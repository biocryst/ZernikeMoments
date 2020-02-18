#include "compute_descriptors.h"
#include "loggers.h"

void parallel::recursive_compute(const boost::filesystem::path& input_dir, int max_order, std::size_t queue_size, std::size_t max_thread)
{
    using namespace std;
    using namespace boost::filesystem;

    logging::logger_t& logger = logging::logger_main::get();

    TasksQueue all_voxel_paths{ queue_size };

    vector<thread> working_threads{ max_thread };

    atomic_bool is_stop{ false };

    for (size_t i{ 0 }; i < working_threads.size(); i++)
    {
        working_threads.at(i) = thread(compute_descriptor, ref(all_voxel_paths), max_order, ref(is_stop));
    }

    auto iterator = recursive_directory_iterator(input_dir);

    for (const auto& entry : iterator)
    {
        if (entry.status().type() == file_type::regular_file)
        {
            path local_file{ entry.path() };

            if (local_file.extension() == ".binvox")
            {
                BOOST_LOG_SEV(logger, logging::severity_t::info) << "Found " << local_file << endl;

                while (!all_voxel_paths.push(local_file))
                {
                    this_thread::sleep_for(500ms);
                }
            }
        }
    }

    while (!all_voxel_paths.empty())
    {
        this_thread::sleep_for(500ms);
    }

    is_stop = true;

    for (auto& thread : working_threads)
    {
        thread.join();
    }
}

void parallel::compute_descriptor(TasksQueue& queue, int max_order, std::atomic_bool& is_stop)
{
    using namespace std;
    using namespace boost::filesystem;

    using VoxelType = bool;
    using Container = vector<VoxelType>;

    Container voxels;
    size_t dim{};

    path path_to_voxel;

    logging::logger_t& logger = logging::logger_main::get();

    while (true)
    {
        if (!queue.pop(path_to_voxel))
        {
            if (is_stop)
            {
                break;
            }

            std::this_thread::sleep_for(100ms);
        }
        else
        {
            if (!io::binvox::read_binvox(path_to_voxel, voxels, dim))
            {
                BOOST_LOG_SEV(logger, logging::severity_t::warning) << "Cannot read binvox from " << path_to_voxel << endl;
            }

            path new_path = path_to_voxel;

            new_path.replace_extension(".inv");

            // compute the zernike descriptors
            // This invoke changes voxels data
            ZernikeDescriptor<double, Container::iterator> zd(voxels.begin(), dim, max_order);

            if (!zd.SaveInvariants(new_path.string()))
            {
                BOOST_LOG_SEV(logger, logging::severity_t::warning) << "Cannot save invariants to file: " << new_path << endl;
            }
            else
            {
                BOOST_LOG_SEV(logger, logging::severity_t::info) << "Save invariants to file: " << new_path << endl;
            }
        }
    }
}