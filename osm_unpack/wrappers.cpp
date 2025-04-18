#include "wrappers.h"

#include <algorithm>
#include <sstream>

#include "transformers.h"

osm_unpack::Node::Node(const int64_t &id, const int64_t &lat, const int64_t &lon):
    id_(id), lat_(lat), lon_(lon) {}

const std::string osm_unpack::Way::tags_to_string() const
{
    std::stringstream ss;
    ss << "{";
    auto it = tags_.begin();
    if ( it == tags_.end() ) {
        goto finish;
    }
    while ( true ) {
        auto[k_id, v_id] = *it;
        ss << strings_->at(k_id) << ": " << strings_->at(v_id);
        if ( ++it == tags_.end() ) {
            break;
        }
        ss << ", ";
    }
    finish:
    ss << "}";
    return ss.str();
}

const int64_t osm_unpack::Node::lat() const
{
    return this->lat_;
}

const int64_t osm_unpack::Node::lon() const
{
    return this->lon_;
}

const std::shared_ptr<std::vector<std::string>> osm_unpack::Node::strings()
{
    return this->strings_;
}

const int osm_unpack::Node::ways_size() const
{
    return this->ways_.size();
}

const std::string osm_unpack::Node::to_string() const
{
    std::stringstream ss;
    ss << "Node("
        << "id: " << id_ << ", "
        << "lat: " << lat_ << ", "
        << "lon: " << lon_ << ", "
        << ")";
    return ss.str();
}

void osm_unpack::PrimitiveBlock::unpack_nodes(const OSMPBF::PrimitiveGroup &pbf_group)
{
    auto pbf_nodes_size = pbf_group.nodes_size();
    if ( pbf_nodes_size == 0 ) {
        return;
    }

    auto nodes = pbf_group.nodes();
    auto node_it = nodes.begin();
    for ( int counter = 0 ; counter < pbf_nodes_size ; ++counter ) {
        auto pbf_node = *node_it++;

        const int64_t id = pbf_node.id();
        const int64_t lat = decode_coordinate(pbf_node.lat(), lat_offset_);
        const int64_t lon = decode_coordinate(pbf_node.lon(), lon_offset_);

        auto node = std::shared_ptr<Node>(new Node(id, lat, lon));

        auto keys = pbf_node.keys();
        auto vals = pbf_node.vals();
        for ( auto key_it = keys.begin(), val_it = vals.begin() ; key_it != keys.end() ; ++key_it, ++val_it ) {
            auto const& key = *key_it;
            auto const& value = *val_it;
            node->tags_.emplace(key, value);
        }

        node->strings_ = this->strings;

        auto[node_entry, success] = this->nodes_.insert(std::make_pair(id, node));
    }
}

void osm_unpack::PrimitiveBlock::unpack_dense(const OSMPBF::PrimitiveGroup &pbf_group)
{
    if ( ! pbf_group.has_dense() ) {
        return;
    }

    auto pbf_nodes = pbf_group.dense();
    const int64_t pbf_nodes_size = pbf_nodes.id_size();

    auto id_it = osm_unpack::StatefulIterator(pbf_nodes.id().begin(), pbf_nodes.id().end(), std::plus<const int64_t>{});
    auto encoded_lat_it = osm_unpack::StatefulIterator(pbf_nodes.lat().begin(), pbf_nodes.lat().end(), std::plus<const int64_t>{});
    auto encoded_lon_it = osm_unpack::StatefulIterator(pbf_nodes.lon().begin(), pbf_nodes.lon().end(), std::plus<const int64_t>{});

    auto keys_vals_it = pbf_nodes.keys_vals().begin();

    for ( int counter = 0 ; counter < pbf_nodes_size; ++counter ) {

        const int64_t id = *id_it++;
        const double lat = decode_coordinate(*encoded_lat_it++, lat_offset_);
        const double lon = decode_coordinate(*encoded_lon_it++, lon_offset_);

        auto node = std::shared_ptr<Node>(new Node(id, lat, lon));

        while ( keys_vals_it != pbf_nodes.keys_vals().end() ) {
            if ( *keys_vals_it == 0 ) {
                ++keys_vals_it;
                break;
            }
            auto key = *keys_vals_it++;
            auto value = *keys_vals_it++;
            node->tags_.emplace(key, value);
        }

        node->strings_ = this->strings;

        auto[node_entry, success] = this->nodes_.insert(std::make_pair(id, node));
    }
}

void osm_unpack::PrimitiveBlock::unpack_ways(const OSMPBF::PrimitiveGroup & pbf_group)
{
    auto ways_size = pbf_group.ways_size();
    if ( ways_size == 0 ) {
        return;
    }

    auto pbf_ways = pbf_group.ways();
    auto pbf_way_it = pbf_ways.begin();

    for ( int counter = 0 ; counter < pbf_group.ways_size() ; ++counter, ++pbf_way_it ) {
        auto pbf_way = *pbf_way_it;
        auto way = std::shared_ptr<Way>(new osm_unpack::Way());
        way->strings_ = this->strings;
        auto refs = osm_unpack::StatefulIterator(pbf_way.refs().begin(), pbf_way.refs().end(), std::plus<const int64_t>{});
        for ( int ref_counter = 0 ; ref_counter < pbf_way.refs_size() ; ++ref_counter ) {
            auto node_it = nodes_.find(*refs++);
            if ( node_it != nodes_.end() ) {
                auto[node_id, node] = *node_it;
                way->push_node(node);
            }
        }

        auto keys = pbf_way.keys();
        auto vals = pbf_way.vals();

        for ( auto key_it = keys.begin(), val_it = vals.begin() ; key_it != keys.end() ; ++key_it, ++val_it ) {
            auto const& key = *key_it;
            auto const& value = *val_it;
            way->tags_.emplace(key, value);
        }

        way->strings_ = this->strings;
        
        this->ways_.push_back(way);
    }
}

const int64_t osm_unpack::PrimitiveBlock::decode_coordinate(const int64_t &coordinate, const int64_t &offset) const
{
    return ( coordinate * granularity_ ) + offset;
}

osm_unpack::PrimitiveBlock::PrimitiveBlock(const OSMPBF::PrimitiveBlock & pbf_block,
                                           std::map<int64_t, std::shared_ptr<osm_unpack::Node>> & nodes,
                                           std::vector<std::shared_ptr<osm_unpack::Way>> & ways):
    nodes_(nodes),
    ways_(ways),
    strings(std::make_shared<std::vector<std::string>>(pbf_block.stringtable().s().begin(), pbf_block.stringtable().s().end())),
    granularity_(pbf_block.granularity()),
    lat_offset_(pbf_block.lat_offset()),
    lon_offset_(pbf_block.lon_offset())
{
    for ( auto const& pbf_group : pbf_block.primitivegroup() ) {
        this->unpack_nodes(pbf_group);
        this->unpack_dense(pbf_group);
        this->unpack_ways(pbf_group);
    }
}

std::optional<std::shared_ptr<osm_unpack::Node>> osm_unpack::PrimitiveBlock::find(const int64_t &id)
{
    auto it = nodes_.find(id);
    if ( it == nodes_.end() ) {
        return {};
    }
    return it->second;
}

osm_unpack::BoundingBox::BoundingBox():
    top(0), bottom(0), left(0), right(0) {}

osm_unpack::BoundingBox::BoundingBox(const std::vector<std::shared_ptr<osm_unpack::Node>> & nodes):
    top(0), bottom(90000000000), left(180000000000), right(0)
{
    for ( auto const& node : nodes ) {
        auto lat = node->lat();
        auto lon = node->lon();
        top = top < lat ? lat : top;
        bottom = bottom > lat ? lat : bottom;
        left = left > lon ? lon : left;
        right = right < lon ? lon : right;
    }
}

osm_unpack::Way::Way() {}

void osm_unpack::Way::push_node(std::shared_ptr<osm_unpack::Node> & node)
{
    nodes_.push_back(node);
    node->ways_.push_back(shared_from_this());
}

const std::vector<std::shared_ptr<osm_unpack::Node>> osm_unpack::Way::nodes() const
{
    return this->nodes_;
}
