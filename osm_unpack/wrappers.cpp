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

void osm_unpack::PrimitiveBlock::unpack_nodes(const OSMPBF::PrimitiveGroup &pbf_group)
{
    auto pbf_nodes_size = pbf_group.nodes_size();
    if ( pbf_nodes_size == 0 ) {
        return;
    }

    std::cout << "Unpacking " << pbf_nodes_size << " Nodes\n";

    auto nodes = pbf_group.nodes();
    auto node_it = nodes.begin();
    for ( int counter = 0 ; counter < pbf_nodes_size ; ++counter ) {
        auto pbf_node = *node_it++;

        std::unordered_map<std::string, std::string> tags;

        auto keys = pbf_node.keys();
        auto vals = pbf_node.vals();
        for ( auto key_it = keys.begin(), val_it = vals.begin() ; key_it != keys.end() ; ++key_it, ++val_it ) {
            auto const& key = strings[*key_it];
            auto const& value = strings[*val_it];
            tags.emplace(key, value);
        }

        const int64_t id = pbf_node.id();
        const double lat = decode_coordinate(pbf_node.lat(), lat_offset);
        const double lon = decode_coordinate(pbf_node.lon(), lon_offset);

        auto[node_entry, success] = this->nodes_.insert(std::make_pair(id, Node(id, lat, lon, tags)));
    }
}

void osm_unpack::PrimitiveBlock::unpack_dense(const OSMPBF::PrimitiveGroup &pbf_group)
{
    if ( ! pbf_group.has_dense() ) {
        return;
    }

    auto pbf_nodes = pbf_group.dense();
    const int64_t pbf_nodes_size = pbf_nodes.id_size();

    std::cout << "Unpacking " << pbf_nodes_size << " DenseNodes\n";

    auto id_it = osm_unpack::StatefulIterator(pbf_nodes.id().begin(), pbf_nodes.id().end(), std::plus<const int64_t>{});
    auto encoded_lat_it = osm_unpack::StatefulIterator(pbf_nodes.lat().begin(), pbf_nodes.lat().end(), std::plus<const int64_t>{});
    auto encoded_lon_it = osm_unpack::StatefulIterator(pbf_nodes.lon().begin(), pbf_nodes.lon().end(), std::plus<const int64_t>{});

    auto keys_vals_it = pbf_nodes.keys_vals().begin();
    
    for ( int counter = 0 ; counter < pbf_nodes_size; ++counter ) {

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
    auto ways_size = pbf_group.ways_size();
    if ( ways_size == 0 ) {
        return;
    }

    std::cout << "Unpacking " << ways_size << " Ways\n";

    auto pbf_ways = pbf_group.ways();
    auto pbf_way_it = pbf_ways.begin();

    for ( int counter = 0 ; counter < pbf_group.ways_size() ; ++counter, ++pbf_way_it ) {
        auto pbf_way = *pbf_way_it;
        auto refs = osm_unpack::StatefulIterator(pbf_way.refs().begin(), pbf_way.refs().end(), std::plus<const int64_t>{});
        std::vector<Node> found_nodes;
        for ( int ref_counter = 0 ; ref_counter < pbf_way.refs_size() ; ++ref_counter ) {
            auto node_it = nodes_.find(*refs++);
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
        this->unpack_nodes(pbf_group);
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
