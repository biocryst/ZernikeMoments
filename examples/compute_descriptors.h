#pragma once

#include "stdafx.h"
#include "binvox_reader.hpp"

namespace parallel
{
	using TasksQueue = boost::lockfree::stack < boost::filesystem::path, boost::lockfree::fixed_sized<true>>;

	void recursive_compute(const boost::filesystem::path& input_dir, int
		max_order, std::size_t max_queue_size, std::size_t max_worker_thread);

	void compute_descriptor(TasksQueue& queue, int max_order, std::atomic_bool& is_stop);
}