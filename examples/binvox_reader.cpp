#pragma once

#include "binvox_reader.h"

bool io::binvox::read_binvox(const boost::filesystem::path& path_to_file, Voxels& voxels, std::size_t& dim)
{
	using byte = unsigned char;

	std::ifstream input{ path_to_file.string(), std::ios::in | std::ios::binary };

	auto print_error = [&input]()
	{
		if (input.eof())
		{
			std::cerr << "Unexpected end of file." << std::endl;
		}
		else if ((input.rdstate() & std::ifstream::badbit) != 0)
		{
			std::cerr << "Either no characters were extracted, or the characters extracted could not be interpreted as a valid value of the appropriate type." << std::endl;
		}
		else if ((input.rdstate() & std::ifstream::failbit) != 0)
		{
			std::cerr << "Error on stream." << std::endl;
		}
	};

	if (!input.is_open())
	{
		std::cerr << "Cannot open file " << path_to_file << std::endl;
		return false;
	}

	// read header
	std::string line;

	input >> line;  // #binvox

	if (!input.good())
	{
		print_error();
		return false;
	}

	if (line != "#binvox") {
		std::cerr << "Error: first line reads [" << line << "] instead of [#binvox]" << std::endl;
		return false;
	}

	int version{};

	input >> version;

	if (!input.good())
	{
		print_error();
		return false;
	}

	std::cout << "Reading binvox version: " << version << std::endl;

	std::size_t depth{ 0 }, height{}, width{};

	bool done{ false };

	while (input.good() && !done) {
		input >> line;

		if (!input.good())
		{
			print_error();
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
				print_error();
				return false;
			}

			if (depth != height || depth != width)
			{
				std::cerr << "Voxel has unequal dimensions." << std::endl;
				return false;
			}
			else
			{
				dim = depth;
			}
		}
		else
		{
			std::cerr << "  unrecognized keyword [" << line << "], skipping" << std::endl;
			char c;
			do
			{  // skip until end of line
				c = input.get();
			} while (input.good() && (c != '\n'));

			if (!input.good())
			{
				print_error();
				return false;
			}
		}
	}

	if (!done) {
		std::cerr << "Error reading header" << std::endl;
		return false;
	}
	if (depth == 0) {
		std::cerr << "Missing dimensions in header." << std::endl;
		return false;
	}

	std::size_t size = width * height * depth;

	voxels.resize(size);
	std::fill(voxels.begin(), voxels.end(), 0.0f);

	//
	// read voxel data
	//
	byte value{}, count{};

	std::size_t index{ 0 }, end_index{ 0 }, nr_voxels{ 0 };

	input.unsetf(std::ifstream::skipws);  // need to read every byte now (!)
	input >> value;  // read the linefeed char

	if (!input.good())
	{
		print_error();
		return false;
	}

	while ((end_index < size) && input.good()) {
		input >> value >> count;

		if (!input.good())
		{
			print_error();
			return false;
		}
		else
		{
			end_index = index + count;

			if (end_index > size)
			{
				std::cerr << "Too many values in voxel. Size is incorrect" << std::endl;
				return false;
			}

			for (std::size_t i{ index }; i < end_index; i++)
			{
				voxels.at(i) = static_cast<Voxels::value_type>(value);
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