#include "readers.h"

#include <chrono>
#include <zlib.h>

bool osm_unpack::Reader::skip_past_header(std::istream &is, const std::string &header)
{
    char current_ch;
    int match_pos = 0;

    while ( ! is.eof() ) {

        is >> current_ch;

        if ( current_ch == header[match_pos++] ) {
            if ( match_pos >= header.size() ) {
                return true;
            }
        }
        else {
            match_pos = 0;
        }
    }
    return false;
}

int osm_unpack::Reader::uncompress_blob(OSMPBF::PrimitiveBlock &block, std::istream &blob_source)
{
    OSMPBF::Blob blob;

    int uc_ret = 0;

    blob.ParseFromIstream(&blob_source);
    if ( blob.has_zlib_data() ) {
        const std::string & data = blob.zlib_data();
        unsigned long raw_size = static_cast<unsigned long>(blob.raw_size());
        uc_ret = uncompress(&buf[0], &raw_size, (const unsigned char *)data.c_str(), data.size());
        if ( uc_ret == 0 ) {
            return block.ParseFromArray(&buf[0], blob.raw_size());
        }
    }
    return uc_ret;
}

osm_unpack::Reader::Reader(const std::string & file_name)
{
    auto start = std::chrono::steady_clock::now();

    std::string osm_header_name("OSMHeader");
    std::string osm_data_name("OSMData");

    std::ifstream map_file;
    map_file.open(file_name, std::ios::in | std::ios::binary);

    if ( ! skip_past_header(map_file, osm_header_name) ) {
        std::cerr << "Header not in file" << std::endl;
        exit(1);
    }    

    OSMPBF::BlobHeader header;
    header.ParseFromIstream(&map_file);

    if ( ! skip_past_header(map_file, osm_data_name) ) {
        std::cerr << "Data not in file" << std::endl;
        exit(1);
    }

    int counter = 0;

    std::map<int64_t, std::shared_ptr<Node>> nodes;

    while ( ! map_file.eof() ) {

        OSMPBF::PrimitiveBlock block;
        if ( uncompress_blob(block, map_file) < 0 ) {
            std::cerr << strerror(errno) << std::endl;
        }

#ifdef DEBUG
        std::cout << "Unpacking block " << counter << std::endl;
#endif

        osm_unpack::PrimitiveBlock primitive_block(block, nodes, this->ways_);

        if ( ! skip_past_header(map_file, osm_data_name) ) {
            break;
        }
        counter += 1;
    }

    // filter out nodes which are not part of any way
    for ( auto const&[id, node] : nodes ) {
        if ( node->ways_size() > 0 ) {
            this->nodes_.push_back(node);
        }
    }

    this->bounding_box_ = BoundingBox(this->nodes_);
    auto elapsed = std::chrono::steady_clock::now() - start;
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();

    std::cout << "Processed the map file in " << elapsed_seconds / 60 << "m " << elapsed_seconds % 60 << "s.\n";
}

osm_unpack::Reader::Reader(const char *file_name):
    Reader(std::string(file_name))
{}

const std::vector<std::shared_ptr<osm_unpack::Node>> osm_unpack::Reader::nodes() const
{
    return this->nodes_;
}

const std::vector<std::shared_ptr<osm_unpack::Way>> osm_unpack::Reader::ways() const
{
    return this->ways_;
}

const osm_unpack::BoundingBox osm_unpack::Reader::bounding_box() const
{
    return this->bounding_box_;
}
