// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "compute_descriptors.h"

void parallel::recursive_compute(const boost::filesystem::path & input_dir, int max_order, std::size_t queue_size, std::size_t max_thread, sqlite::database & db)
{
    using namespace std;
    using namespace boost::filesystem;
    using namespace logging;

    logger_t & logger = logger_main::get();

    TasksQueue all_voxel_paths{ queue_size };

    vector<thread> working_threads{ max_thread };

    atomic_bool is_stop{ false };

    for (size_t i{ 0 }; i < working_threads.size(); i++)
    {
        working_threads.at(i) = thread(compute_descriptor, ref(all_voxel_paths), max_order, ref(is_stop), ref(db));
    }

    auto iterator = recursive_directory_iterator(input_dir);

    for (const auto & entry : iterator)
    {
        if (is_stop)
        {
            break;
        }

        if (entry.status().type() == file_type::regular_file)
        {
            path local_file{ entry.path() };

            if (local_file.extension() == u8".binvox")
            {
                BOOST_LOG_SEV(logger, severity_t::info) << u8"Found " << local_file << endl;

                path relative_path = relative(local_file, input_dir);

                tuple<path, path> item = std::make_tuple(input_dir, relative_path);

                while (!all_voxel_paths.push(item) && !is_stop)
                {
                    this_thread::sleep_for(500ms);
                }
            }
        }
    }

    while (!(all_voxel_paths.empty() || is_stop))
    {
        this_thread::sleep_for(500ms);
    }

    is_stop = true;

    for (auto & thread : working_threads)
    {
        thread.join();
    }

    BOOST_LOG_SEV(logger, severity_t::info) << u8"Completed" << endl;
}

void parallel::compute_descriptor(TasksQueue & queue, int max_order, std::atomic_bool & is_stop, sqlite::database & db)
{
    using namespace std;
    using namespace boost::filesystem;
    using namespace logging;

    using VoxelType = bool;
    using Container = vector<VoxelType>;
    using DescriptorType = double;

    Container binvox_voxels;
    Container canonical_order_voxels;
    size_t dim{};

    logger_t & logger = logger_main::get();

    tuple<path, path> path_to_voxel;

    while (true)
    {
        if (!queue.pop(path_to_voxel))
        {
            if (is_stop)
            {
                break;
            }

            std::this_thread::sleep_for(50ms);
        }
        else
        {
            path absolute_path = get<0>(path_to_voxel) / get<1>(path_to_voxel);

            BOOST_LOG_SEV(logger, severity_t::debug) << u8"Processing " << absolute_path << endl;

            if (!io::binvox::read_binvox(absolute_path, binvox_voxels, dim))
            {
                BOOST_LOG_SEV(logger, severity_t::warning) << u8"Cannot read binvox from " << absolute_path << endl;
            }
            else
            {
                canonical_order_voxels.resize(binvox_voxels.size());
                binvox::utils::convert_to_canonical_order(binvox_voxels.begin(), canonical_order_voxels.begin(), dim);

                // compute the zernike descriptors
                // This invoke changes voxels data
                ZernikeDescriptor<DescriptorType, Container::iterator> zd(canonical_order_voxels.begin(), dim, max_order);

                auto invs{ zd.get_invariants() };

                try
                {
                    db << u8"INSERT INTO zernike_descriptors (path, file_hash, desc_length, desc_value_size_bytes, descriptor) VALUES(?, ?, ?, ?, ?)"
                        << get<1>(path_to_voxel).string()
                        << u8"1212"
                        << invs.size()
                        << sizeof(DescriptorType)
                        << invs;
                }
                catch (const sqlite::sqlite_exception & exc)
                {
                    BOOST_LOG_SEV(logger, severity_t::warning) << u8"Cannot save invariants to database." << exc.what() << endl << exc.get_extended_code() << endl << exc.get_sql() << endl;
                    is_stop = true;
                    return;
                }

                BOOST_LOG_SEV(logger, severity_t::info) << u8"Save invariants to database." << endl;
            }
        }
    }
}