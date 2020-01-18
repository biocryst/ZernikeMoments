#pragma once

#include "BinvoxReader.h"


bool io::binvox::read_binvox(std::string path_to_file, std::vector<unsigned char>& voxels, std::size_t & dim)
{
    using byte = unsigned char;

    std::ifstream input{ path_to_file, std::ios::in | std::ios::binary };

    if (!input.is_open())
    {
        std::cerr << "Cannot open file '" << path_to_file << '\'' << std::endl;
        return false;
    }

    //
    // read header
    //
    std::string line;

    input >> line;  // #binvox
    
    if (line.compare("#binvox") != 0) {
        std::cout << "Error: first line reads [" << line << "] instead of [#binvox]" << std::endl;
        return false;
    }

    int version;

    input >> version;

    std::cout << "reading binvox version " << version << std::endl;

    std::size_t depth{0}, height{}, width{};
    bool done{ false };

    while (input.good() && !done) {
        input >> line;
        if (line.compare("data") == 0)
            done = true;
        else if (line.compare("dim") == 0) {
            input >> depth >> height >> width;

            if (depth != height || depth != width)
            {
                std::cerr << "Voxel has unequal dimensions " << std::endl;
                return 1;
            }
            else
            {
                dim = depth;
            }
        }
        else {
            std::cout << "  unrecognized keyword [" << line << "], skipping" << std::endl;
            char c;
            do {  // skip until end of line
                c = input.get();
            } 
            while (input.good() && (c != '\n'));
        }
    }

    if (!done) {
        std::cout << "  error reading header" << std::endl;
        return false;
    }
    if (depth == 0) {
        std::cout << "  missing dimensions in header" << std::endl;
        return false;
    }

    std::size_t size = width * height * depth;

    voxels.resize(size);

    //
    // read voxel data
    //
    byte value{};
    byte count{};

    std::size_t index{ 0 }, end_index{ 0 }, nr_voxels{ 0 };

    input.unsetf(std::ios::skipws);  // need to read every byte now (!)
    input >> value;  // read the linefeed char

    while ((end_index < size) && input.good()) {
        input >> value >> count;

        if (input.good()) {
            end_index = index + count;
            if (end_index > size)
            {
                std::cerr << "Too many values in voxel. Size is incorrcet" << std::endl;
                return false;
            }

            for (std::size_t i{ index }; i < end_index; i++)
            {
                voxels.at(i) = value;
            }

            if (value)
            {
                nr_voxels += count;
            }

            index = end_index;
        } 
    }

    input.close();
    
    std::cout << "Read " << nr_voxels << " voxels" << std::endl;

    return true;
}