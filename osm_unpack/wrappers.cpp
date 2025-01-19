#include "wrappers.h"

#include <algorithm>
#include <sstream>

#include "transformers.h"

osm_unpack::Node::Node(const int64_t &id, const double &lat, const double &lon,
    const std::unordered_map<std::string, std::string> & tags):
    id_(id), lat_(lat), lon_(lon), tags_(tags) {}

const std::string osm_unpack::Node::tags_to_string() const
{
    std::stringstream ss;
    ss << "{";
    auto it = tags_.begin();
    if ( it == tags_.end() ) {
        goto finish;
    }
    while ( true ) {
        auto[k, v] = *it;
        ss << k << ": " << v;
        if ( ++it == tags_.end() ) {
            break;
        }
        ss << ", ";
    }
    finish:
    ss << "}";
    return ss.str();
}

const std::string osm_unpack::Node::to_string() const
{
    std::stringstream ss;
    ss << "Node("
        << "id: " << id_ << ", "
        << "lat: " << lat_ << ", "
        << "lon: " << lon_ << ", "
        << "tags: " << tags_to_string()
        << ")";
    return ss.str();
}

void osm_unpack::PrimitiveGroup::unpack_dense(const OSMPBF::PrimitiveBlock &parent_block)
{
    auto pbf_nodes = primitive_group.dense();
    const int64_t pbf_nodes_size = pbf_nodes.id_size();

    auto strings = parent_block.stringtable().s();

    auto id_it = osm_unpack::StatefulIterable(pbf_nodes.id(), std::plus<int64_t>{}).begin();
    auto encoded_lat_it = osm_unpack::StatefulIterable(pbf_nodes.lat(), std::plus<int64_t>{}).begin();
    auto encoded_lon_it = osm_unpack::StatefulIterable(pbf_nodes.lon(), std::plus<int64_t>{}).begin();

    auto keys_vals_it = pbf_nodes.keys_vals().begin();
    
    for ( int counter = 0; counter < pbf_nodes_size; ++counter ) {

        std::unordered_map<std::string, std::string> tags;

        while ( *keys_vals_it != 0 ) {
            auto k_it = strings.begin() + *keys_vals_it++;
            auto v_it = strings.begin() + *keys_vals_it++;
            tags.emplace(*k_it, *v_it);
        }

        const int64_t id = *id_it++;
        const double lat = decode_coordinate(*encoded_lat_it++, lat_offset);
        const double lon = decode_coordinate(*encoded_lon_it++, lon_offset);

        auto[node_entry, success] = this->nodes_.insert(std::make_pair(id, Node(id, lat, lon, tags)));
    }
}

void osm_unpack::PrimitiveGroup::unpack_ways(const OSMPBF::PrimitiveBlock &parent_block)
{
    auto pbf_ways = primitive_group.ways();

    for ( auto way: pbf_ways ) {
        auto refs = osm_unpack::StatefulIterable(way.refs(), std::plus<int64_t>{});
        std::vector<Node> found_nodes;
        for ( auto ref : refs ) {
            auto node_it = this->nodes_.find(ref);
            if ( node_it != this->nodes_.end() ) {
                found_nodes.push_back(node_it->second);
            }
        }
    }
    std::cout << std::flush;
}

const double osm_unpack::PrimitiveGroup::decode_coordinate(const int64_t &coordinate, const int64_t &offset) const
{
    return (( coordinate * granularity ) + offset) / 1000000000.0;
}

osm_unpack::PrimitiveGroup::PrimitiveGroup(const OSMPBF::PrimitiveBlock & parent_block,
    const OSMPBF::PrimitiveGroup & group):
    primitive_group(group),
    strings(parent_block.stringtable().s()),
    granularity(parent_block.granularity()),
    lat_offset(parent_block.lat_offset()),
    lon_offset(parent_block.lon_offset())
{
    this->unpack_dense(parent_block);
    this->unpack_ways(parent_block);
}

std::optional<osm_unpack::Node> osm_unpack::PrimitiveGroup::find(const int64_t &id)
{
    auto it = nodes_.find(id);
    if ( it == nodes_.end() ) {
        return {};
    }
    return it->second;
}

const std::map<int64_t, osm_unpack::Node> osm_unpack::PrimitiveGroup::nodes() const
{
    return this->nodes_;
}

