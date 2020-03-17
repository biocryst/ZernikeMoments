// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "stdafx.h"
#include "loggers.h"

namespace io
{
    namespace binvox
    {
        // Original code was imported https://www.patrickmin.com/binvox/read_binvox.cc and slightly modified
        template<typename VoxelType>
        bool read_binvox(const boost::filesystem::path& path_to_file, std::vector<VoxelType>& voxels, std::size_t& dim)
        {
            static_assert(std::is_integral<VoxelType>::value || std::is_floating_point<VoxelType>::value, "Voxel type must be integral or float");

            logging::logger_t& logger = logging::logger_io::get();

            using byte = unsigned char;

            dim = 0;

            std::ifstream input{ path_to_file.string(), std::ios::in | std::ios::binary };

            if (!input.is_open())
            {
                BOOST_LOG_SEV(logger, logging::severity_t::trace) << "Cannot open file " << path_to_file << std::endl;
                return false;
            }

            // read header
            std::string line;

            input >> line;  // #binvox

            if (!input.good())
            {
                return false;
            }

            if (line != "#binvox")
            {
                BOOST_LOG_SEV(logger, logging::severity_t::trace) << "Error: first line reads [" << line << "] instead of [#binvox]. Probably it is not binvox format." << std::endl;
                return false;
            }

            int version{};

            input >> version;

            if (!input.good())
            {
                return false;
            }

            BOOST_LOG_SEV(logger, logging::severity_t::trace) << "Reading binvox version: " << version << std::endl;

            std::size_t depth{ 0 }, height{}, width{};

            bool done{ false };

            while (input.good() && !done)
            {
                input >> line;

                if (!input.good())
                {
                    return false;
                }

                if (line == "data")
                {
                    done = true;
                }
                else if (line == "dim")
                {
                    input >> depth >> height >> width;

                    if (!input.good())
                    {
                        return false;
                    }

                    if (depth != height || depth != width)
                    {
                        BOOST_LOG_SEV(logger, logging::severity_t::trace) << "Voxel has unequal dimensions." << std::endl;
                        return false;
                    }
                    else
                    {
                        dim = depth;
                    }
                }
                else
                {
                    BOOST_LOG_SEV(logger, logging::severity_t::trace) << "Unrecognized keyword [" << line << "], skipping" << std::endl;
                    char c;
                    do
                    {  // skip until end of line
                        c = input.get();
                    } while (input.good() && (c != '\n'));

                    if (!input.good())
                    {
                        return false;
                    }
                }
            }

            if (!done)
            {
                BOOST_LOG_SEV(logger, logging::severity_t::trace) << "Error reading header" << std::endl;
                return false;
            }

            if (depth == 0)
            {
                BOOST_LOG_SEV(logger, logging::severity_t::trace) << "Missing dimensions in header." << std::endl;
                return false;
            }

            std::size_t size = width * height * depth;

            voxels.resize(size);

            //
            // read voxel data
            //
            byte value{}, count{};

            std::size_t index{ 0 }, end_index{ 0 }, nr_voxels{ 0 };

            input.unsetf(std::ifstream::skipws);  // need to read every byte now (!)
            input >> value;  // read the linefeed char

            if (!input.good())
            {
                return false;
            }

            while ((end_index < size) && input.good())
            {
                input >> value >> count;

                if (!input.good())
                {
                    return false;
                }
                else
                {
                    end_index = index + count;

                    if (end_index > size)
                    {
                        BOOST_LOG_SEV(logger, logging::severity_t::trace) << "Too many values in voxel. Size is incorrect" << std::endl;
                        return false;
                    }

                    for (std::size_t i{ index }; i < end_index; i++)
                    {
                        voxels.at(i) = static_cast<VoxelType>(value);
                    }

                    if (value)
                    {
                        nr_voxels += count;
                    }

                    index = end_index;
                }
            }

            input.close();

            BOOST_LOG_SEV(logger, logging::severity_t::trace) << "Read " << nr_voxels << " voxels" << std::endl;

            return true;
        }
    }
}