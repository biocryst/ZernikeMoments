// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <tuple>
#include <thread>
#include <atomic>
#include <chrono>
#include <type_traits>
#include <cassert>
#include <iterator>
#include <complex>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/lockfree/stack.hpp>
#include <boost/log/common.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/utility/setup/from_stream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/math/special_functions/binomial.hpp>
#include <boost/math/special_functions/factorials.hpp>
#include <boost/math/constants/constants.hpp>

#include <sqlite_modern_cpp.h>

#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>