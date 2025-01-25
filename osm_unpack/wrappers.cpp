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

void osm_unpack::PrimitiveBlock::unpack_dense(const OSMPBF::PrimitiveGroup & pbf_group)
{
    auto pbf_nodes = pbf_group.dense();
    const int64_t pbf_nodes_size = pbf_nodes.id_size();

    auto id_it = osm_unpack::StatefulIterator(pbf_nodes.id().begin(), pbf_nodes.id().end(), std::plus<const int64_t>{});
    auto encoded_lat_it = osm_unpack::StatefulIterator(pbf_nodes.lat().begin(), pbf_nodes.lat().end(), std::plus<const int64_t>{});
    auto encoded_lon_it = osm_unpack::StatefulIterator(pbf_nodes.lon().begin(), pbf_nodes.lon().end(), std::plus<const int64_t>{});

    auto keys_vals_it = pbf_nodes.keys_vals().begin();
    
    for ( int counter = 0; counter < pbf_nodes_size; ++counter ) {

        std::unordered_map<std::string, std::string> tags;

        while ( *keys_vals_it != 0 ) {
            auto const& key = strings[*keys_vals_it++];
            auto const& value = strings[*keys_vals_it++];
            tags.emplace(key, value);
        }

        const int64_t id = *id_it++;
        const double lat = decode_coordinate(*encoded_lat_it++, lat_offset);
        const double lon = decode_coordinate(*encoded_lon_it++, lon_offset);

        auto[node_entry, success] = this->nodes_.insert(std::make_pair(id, Node(id, lat, lon, tags)));
    }
}

void osm_unpack::PrimitiveBlock::unpack_ways(const OSMPBF::PrimitiveGroup & pbf_group)
{
    auto pbf_ways = pbf_group.ways();

    for ( auto way: pbf_ways ) {
        auto refs = osm_unpack::StatefulIterator(way.refs().begin(), way.refs().end(), std::plus<const int64_t>{});
        std::vector<Node> found_nodes;
        for ( auto i = 0; i < pbf_group.ways_size(); ++i ) {
            auto ref = refs++;
            auto node_it = nodes_.find(*ref);
            if ( node_it != nodes_.end() ) {
                auto[node_id, node] = *node_it;
                found_nodes.push_back(node);
            }
        }
    }
}

const double osm_unpack::PrimitiveBlock::decode_coordinate(const int64_t &coordinate, const int64_t &offset) const
{
    return (( coordinate * granularity ) + offset) / 1000000000.0;
}

osm_unpack::PrimitiveBlock::PrimitiveBlock(const OSMPBF::PrimitiveBlock & pbf_block):
    strings(pbf_block.stringtable().s().begin(), pbf_block.stringtable().s().end()),
    granularity(pbf_block.granularity()),
    lat_offset(pbf_block.lat_offset()),
    lon_offset(pbf_block.lon_offset())
{
    for ( auto const& pbf_group : pbf_block.primitivegroup() ) {
        this->unpack_dense(pbf_group);
        this->unpack_ways(pbf_group);
    }
}

std::optional<osm_unpack::Node> osm_unpack::PrimitiveBlock::find(const int64_t &id)
{
    auto it = nodes_.find(id);
    if ( it == nodes_.end() ) {
        return {};
    }
    return it->second;
}

std::map<int64_t, osm_unpack::Node>::const_iterator osm_unpack::PrimitiveBlock::nodes_begin() const
{
    return this->nodes_.begin();
}

std::map<int64_t, osm_unpack::Node>::const_iterator osm_unpack::PrimitiveBlock::nodes_end() const
{
    return this->nodes_.end();
}

std::map<int64_t, osm_unpack::Way>::const_iterator osm_unpack::PrimitiveBlock::ways_begin()
{
    return this->ways_.begin();
}

std::map<int64_t, osm_unpack::Way>::const_iterator osm_unpack::PrimitiveBlock::ways_end()
{
    return this->ways_.end();
}
