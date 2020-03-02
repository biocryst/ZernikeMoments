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
        xml_paths.at(i) = xml_dir;
        working_threads.at(i) = thread(compute_descriptor, ref(all_voxel_paths), max_order, ref(is_stop), ref(xml_paths.at(i)), input_dir);
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
        io::xml::XMLMerger  merger{ (xml_dir / u8"result.xml").string(), u8"Voxel", u8"Voxels" };

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

void parallel::compute_descriptor(TasksQueue& queue, int max_order, std::atomic_bool& is_stop, boost::filesystem::path& xml_path, const boost::filesystem::path& voxel_root_dir)
{
    using namespace std;
    using namespace boost::filesystem;
    using namespace logging;

    using VoxelType = bool;
    using Container = vector<VoxelType>;
    using DescriptorType = double;
    using XMLWriterType = io::xml::XMLWriter<DescriptorType>;

    Container binvox_voxels;
    Container canonical_order_voxels;
    size_t dim{};

    path path_to_voxel;

    logger_t& logger = logger_main::get();

    if (!boost::filesystem::is_directory(xml_path))
    {
        BOOST_LOG_SEV(logger, severity_t::error) << u8"Cannot start thread because input path to XML is not initialized by directory." << endl;
        is_stop = true;
        return;
    }

    {
        stringstream thread_id;

        thread_id << std::this_thread::get_id();

        boost::filesystem::path file_path{ u8"descriptor_" };
        file_path += thread_id.str();
        file_path += u8".xml";

        xml_path /= file_path;
    }

    std::unique_ptr<XMLWriterType> writer_ptr;

    try
    {
        writer_ptr = make_unique<XMLWriterType>(xml_path.string(), voxel_root_dir.string());
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

            std::this_thread::sleep_for(50ms);
        }
        else
        {
            BOOST_LOG_SEV(logger, severity_t::debug) << u8"Processing " << path_to_voxel << endl;

            if (!io::binvox::read_binvox(path_to_voxel, binvox_voxels, dim))
            {
                BOOST_LOG_SEV(logger, severity_t::warning) << u8"Cannot read binvox from " << path_to_voxel << endl;
            }
            else
            {
                canonical_order_voxels.resize(binvox_voxels.size());
                binvox::utils::convert_to_canonical_order(binvox_voxels.begin(), canonical_order_voxels.begin(), dim);

                // compute the zernike descriptors
                // This invoke changes voxels data
                ZernikeDescriptor<DescriptorType, Container::iterator> zd(canonical_order_voxels.begin(), dim, max_order);

                auto invs{ zd.get_invariants() };

                if (!writer_ptr->write_descriptor(path_to_voxel, dim, invs))
                {
                    BOOST_LOG_SEV(logger, severity_t::warning) << u8"Cannot save invariants to xml file: " << xml_path << endl;
                }
                else
                {
                    BOOST_LOG_SEV(logger, severity_t::info) << u8"Save invariants to file: " << xml_path << endl;
                }
            }
        }
    }
}