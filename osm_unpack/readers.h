#ifndef READERS_H_INCLUDED
#define READERS_H_INCLUDED

#include <array>
#include <fstream>
#include <memory>
#include <string>

#include "fileformat.pb.h"
#include "osmformat.pb.h"

#include "bases.h"
#include "wrappers.h"

namespace osm_unpack {

class Reader
{
    std::array<unsigned char, 32 * 1024 * 1024> buf;

    bool skip_past_header(std::istream & is, const std::string & header);
    int uncompress_blob(OSMPBF::PrimitiveBlock & block, std::istream & blob_source);

    std::vector<std::shared_ptr<Node>> nodes_;
    std::vector<std::shared_ptr<Way>> ways_;

    BoundingBox bounding_box_;

    template <typename K, typename V> void insert_values_to_vector(const std::map<K, V> & input, std::vector<V> & output);

public:

    Reader(const std::string & file_name);
    Reader(const char * file_name);

    const std::vector<std::shared_ptr<Node>> nodes() const;
    const std::vector<std::shared_ptr<Way>> ways() const;

    const BoundingBox bounding_box() const;
};

template <typename K, typename V>
inline void Reader::insert_values_to_vector(const std::map<K, V> &input, std::vector<V> &output)
{
    for ( auto it = input.begin() ; it != input.end() ; ++it ) {
        auto[key, val] = *it;
        output.push_back(val);
    }
}

}

#endif