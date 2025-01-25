#include "readers.h"

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

    std::map<int64_t, osm_unpack::Node> nodes;
    std::map<int64_t, osm_unpack::Way> ways;

    while ( ! map_file.eof() ) {

        OSMPBF::PrimitiveBlock block;
        if ( uncompress_blob(block, map_file) < 0 ) {
            std::cerr << strerror(errno) << std::endl;
        }

        std::cout << "Unpacking block " << counter << std::endl;
        osm_unpack::PrimitiveBlock primitive_block(block, nodes, ways);

        if ( ! skip_past_header(map_file, osm_data_name) ) {
            break;
        }
        counter += 1;
    }
}

osm_unpack::Reader::Reader(const char *file_name):
    Reader(std::string(file_name))
{
    std::cout << file_name << std::endl;
}
