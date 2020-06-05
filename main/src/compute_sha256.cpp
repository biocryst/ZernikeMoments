#include "compute_sha256.h"

bool hash::compute_sha256(const boost::filesystem::path & path, std::vector<unsigned char> & buffer, std::string & hash)
{
    std::ifstream input_file(path.string(), std::ifstream::in | std::ifstream::binary);

    if (!input_file.is_open())
    {
        return false;
    }

    buffer.resize(picosha2::k_digest_size);

    picosha2::hash256(input_file, buffer.begin(), buffer.end());
    picosha2::bytes_to_hex_string(buffer.begin(), buffer.end(), hash);

    return true;
}