#pragma once

#include "stdafx.h"

namespace io
{
	namespace binvox
	{
		using Voxels = std::vector<double>;

		// Original code was imported https://www.patrickmin.com/binvox/read_binvox.cc and slightly modified
		bool read_binvox(const boost::filesystem::path& path_to_file, Voxels& voxels, std::size_t& dim);
	}
}
