#pragma once

#include "stdafx.h"

namespace hash
{
    bool compute_sha256(const boost::filesystem::path & path, std::vector<unsigned char> & buffer, std::string & hash);
}