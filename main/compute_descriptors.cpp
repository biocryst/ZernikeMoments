// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "compute_descriptors.h"

void parallel::recursive_compute(const boost::filesystem::path& input_dir, int max_order, std::size_t queue_size, std::size_t max_thread, const boost::filesystem::path& xml_dir)
{
    using namespace std;
    using namespace boost::filesystem;
    using namespace logging;

    logger_t& logger = logger_main::get();

    TasksQueue all_voxel_paths{ queue_size };

    vector<thread> working_threads{ max_thread };

    atomic_bool is_stop{ false };

    vector<path> xml_paths{ working_threads.size() };

    for (size_t i{ 0 }; i < working_threads.size(); i++)
    {
        working_threads.at(i) = thread(compute_descriptor, ref(all_voxel_paths), max_order, ref(is_stop), xml_dir, ref(xml_paths.at(i)));
    }

    auto iterator = recursive_directory_iterator(input_dir);

    for (const auto& entry : iterator)
    {
        if (is_stop)
        {
            break;
        }

        if (entry.status().type() == file_type::regular_file)
        {
            path local_file{ entry.path() };

            if (local_file.extension() == ".binvox")
            {
                BOOST_LOG_SEV(logger, severity_t::info) << u8"Found " << local_file << endl;

                while (!all_voxel_paths.push(local_file) && !is_stop)
                {
                    this_thread::sleep_for(500ms);
                }
            }
        }
    }

    while (!all_voxel_paths.empty() && !is_stop)
    {
        this_thread::sleep_for(500ms);
    }

    is_stop = true;

    for (auto& thread : working_threads)
    {
        thread.join();
    }

    BOOST_LOG_SEV(logger, severity_t::info) << u8"Begin merge results" << endl;

    try
    {
        io::xml::XMLMerger  merger{ (xml_dir / u8"result.xml").string(), u8"Voxel" };

        if (!merger.merge_files(xml_paths))
        {
            BOOST_LOG_SEV(logger, severity_t::error) << u8"Cannot merge results" << endl;
        }
    }
    catch (const std::runtime_error & error)
    {
        BOOST_LOG_SEV(logger, severity_t::error) << error.what() << endl;
    }
    catch (const std::invalid_argument & error)
    {
        BOOST_LOG_SEV(logger, severity_t::error) << error.what() << endl;
    }
}

void parallel::compute_descriptor(TasksQueue& queue, int max_order, std::atomic_bool& is_stop, const boost::filesystem::path& xml_dir, boost::filesystem::path& xml_output)
{
    using namespace std;
    using namespace boost::filesystem;
    using namespace logging;

    using VoxelType = bool;
    using Container = vector<VoxelType>;
    using DescriptorType = double;
    using XMLWriterType = io::xml::XMLWriter<DescriptorType>;

    Container voxels;
    size_t dim{};

    path path_to_voxel;

    logger_t& logger = logger_main::get();

    {
        stringstream thread_id;

        thread_id << std::this_thread::get_id();

        xml_output = u8"descriptor_";
        xml_output += thread_id.str();
        xml_output += u8".xml";

        xml_output = xml_dir / xml_output;
    }

    std::unique_ptr<XMLWriterType> writer_ptr;

    try
    {
        writer_ptr = make_unique<XMLWriterType>(xml_output.string());
    }
    catch (std::runtime_error & error)
    {
        BOOST_LOG_SEV(logger, severity_t::error) << error.what() << endl;
        is_stop = true;
        return;
    }

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
            BOOST_LOG_SEV(logger, severity_t::debug) << u8"Processing " << path_to_voxel << endl;

            if (!io::binvox::read_binvox(path_to_voxel, voxels, dim))
            {
                BOOST_LOG_SEV(logger, severity_t::warning) << u8"Cannot read binvox from " << path_to_voxel << endl;
            }
            else
            {
                // compute the zernike descriptors
                // This invoke changes voxels data
                ZernikeDescriptor<DescriptorType, Container::iterator> zd(voxels.begin(), dim, max_order);

                auto invs{ zd.get_invariants() };

                if (!writer_ptr->write_descriptor(path_to_voxel, dim, invs))
                {
                    BOOST_LOG_SEV(logger, severity_t::warning) << u8"Cannot save invariants to xml file: " << xml_output << endl;
                }
                else
                {
                    BOOST_LOG_SEV(logger, severity_t::info) << u8"Save invariants to file: " << xml_output << endl;
                }
            }
        }
    }
}