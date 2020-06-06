// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "stdafx.h"

namespace hash
{
    bool compute_sha256(const boost::filesystem::path & path, std::vector<unsigned char> & buffer, std::string & hash);
}