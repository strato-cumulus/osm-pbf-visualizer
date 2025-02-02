#ifndef WRAPPERS_H_INCLUDED
#define WRAPPERS_H_INCLUDED

#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "osmformat.pb.h"

#include "transformers.h"

namespace osm_unpack {

class PrimitiveBlock;

class WrapperBase
{
protected:

    std::unordered_map<std::string, std::string> tags_;

    template<class OutContainer, class InContainer>
    void unpack(OutContainer & out_container,
        typename OutContainer::iterator out_it,
        const InContainer & in_container);

    template<class OutContainer, class InKeyContainer, class InValueContainer>
    void unpack(OutContainer & out_container,
        typename OutContainer::iterator out_it,
        const InKeyContainer & in_keys,
        const InValueContainer & in_values);

    template<class OutContainer, class InContainer, class DictContainer>
    void translate(OutContainer & out_container,
        const InContainer & in_container,
        const DictContainer & dict_container);

    template<class InContainer, class DictContainer>
    void unpack_tags(const InContainer & in_container,
        const DictContainer & dict_container);
};

class Way;

class Node : WrapperBase {

    int64_t id_;
    double lat_;
    double lon_;

    std::shared_ptr<std::vector<std::string>> strings_;
    std::unordered_map<uint32_t, uint32_t> tags_;
    std::vector<std::weak_ptr<Way>> ways_;

    friend class PrimitiveBlock;
    friend class Way;

    const std::string tags_to_string() const;

public:

    Node(const int64_t & id, const int64_t & lat, const int64_t & lon,
        const std::unordered_map<uint32_t, uint32_t> & tags);

    const int64_t lat() const;
    const int64_t lon() const;

    const std::shared_ptr<std::vector<std::string>> strings();
    const int ways_size() const;

    const std::string to_string() const;
};

class Way: protected WrapperBase, public std::enable_shared_from_this<Way>
{
    std::vector<std::shared_ptr<Node>> nodes_;

    friend class Node;

public:

    Way();

    void push_node(std::shared_ptr<Node> & node);

    const std::vector<std::shared_ptr<Node>> nodes() const;
};

struct BoundingBox
{
    int64_t top, bottom, left, right;

    BoundingBox();
    BoundingBox(const std::vector<std::shared_ptr<Node>> & nodes);
};

class PrimitiveBlock: protected WrapperBase {

    std::shared_ptr<std::vector<std::string>> strings;

    int32_t granularity_;
    int64_t lat_offset_;
    int64_t lon_offset_;

    std::map<int64_t, std::shared_ptr<Node>> & nodes_;
    std::vector<std::shared_ptr<Way>> & ways_;

    void unpack_nodes(const OSMPBF::PrimitiveGroup & pbf_group);
    void unpack_dense(const OSMPBF::PrimitiveGroup & pbf_group);
    void unpack_ways(const OSMPBF::PrimitiveGroup & pbf_group);

    const int64_t decode_coordinate(const int64_t & coordinate, const int64_t & offset) const;

public:

    PrimitiveBlock(const OSMPBF::PrimitiveBlock & parent_block,
        std::map<int64_t, std::shared_ptr<osm_unpack::Node>> & nodes, std::vector<std::shared_ptr<osm_unpack::Way>> & ways);

    std::optional<std::shared_ptr<Node>> find(const int64_t & id);
};

template <class OutContainer, class InContainer>
inline void WrapperBase::unpack(OutContainer &out_container, typename OutContainer::iterator out_it, const InContainer &in_container)
{
    std::insert_iterator<OutContainer> inserter = std::inserter(out_container, out_it);

    for ( auto in_it = in_container.begin(); in_it != in_container.end(); ++in_it ) {
        *inserter = in_it;
    }
}

template <class OutContainer, class InKeyContainer, class InValueContainer>
inline void WrapperBase::unpack(OutContainer &out_container,
                                typename OutContainer::iterator out_it,
                                const InKeyContainer &in_keys,
                                const InValueContainer &in_values)
{
    std::insert_iterator<OutContainer> inserter = std::inserter(out_container, out_it);

    auto k_it = in_keys.begin();
    auto v_it = in_values.begin();
    
    while ( k_it != in_keys.end() ) {
        *inserter = std::make_pair(*k_it++, *v_it++);
    }
}

template <class OutContainer, class InContainer, class DictContainer>
inline void WrapperBase::translate(OutContainer &out_container, const InContainer &in_container, const DictContainer &dict_container)
{
    auto inserter = std::inserter(out_container, out_container.begin());
    std::transform(in_container.begin(), in_container.end(), out_container.begin(), [&](const int & offset) {
        auto dict_it = dict_container.begin();
        std::advance(dict_it, offset);
        return *dict_it;
    });
}

template <class InContainer, class DictContainer>
inline void WrapperBase::unpack_tags(const InContainer &in_container, const DictContainer &dict_container)
{
    std::vector<std::string> keys, vals;
    translate(keys, in_container.keys(), dict_container);
    translate(vals, in_container.vals(), dict_container);
    for ( auto k_it = keys.begin(), v_it = keys.end(); k_it != keys.end(); k_it++, v_it++) {
        tags_.emplace(*k_it, *v_it);
    }
}

}

#endif