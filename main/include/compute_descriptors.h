// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#pragma once

#include "stdafx.h"
#include "binvox_reader.hpp"
#include "ZernikeDescriptor.hpp"
#include "loggers.h"
#include "xmlwriter.hpp"
#include "xmlmerger.h"
#include "binvox_utils.hpp"

namespace parallel
{
    using TasksQueue = boost::lockfree::stack <boost::filesystem::path, boost::lockfree::fixed_sized<true>>;

    void recursive_compute(const boost::filesystem::path & input_dir,
        int
        max_order, std::size_t max_queue_size, std::size_t max_worker_thread, const boost::filesystem::path & xml_dir);

    void compute_descriptor(TasksQueue & queue, int max_order, std::atomic_bool & is_stop, boost::filesystem::path & xml_path, const boost::filesystem::path & voxel_root_dir);
}