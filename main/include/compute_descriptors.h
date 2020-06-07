// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "stdafx.h"
#include "binvox_reader.hpp"
#include "ZernikeDescriptor.hpp"
#include "loggers.h"
#include "binvox_utils.hpp"
#include "compute_sha256.h"
#include "sqlite_row.hpp"
#include "path_tree.hpp"

namespace parallel
{
    // Queue stores an absolute path as two parts: parent path and path relative to directory with data.
    using TasksQueue = boost::lockfree::stack <std::tuple<boost::filesystem::path, boost::filesystem::path, std::string>, boost::lockfree::fixed_sized<true>>;

    void recursive_compute(const boost::filesystem::path & input_dir,
        int max_order, std::size_t max_queue_size, std::size_t max_worker_thread, sqlite::database & db);

    void compute_descriptor(TasksQueue & queue, int max_order, std::atomic_bool & is_stop, sqlite::database & db);
}