#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

namespace io
{
	namespace binvox
	{
		bool read_binvox(std::string path_to_file, std::vector<unsigned char>& voxels, std::size_t & dim);
	}
}
