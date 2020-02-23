// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*

                          3D Zernike Moments
    Copyright (C) 2003 by Computer Graphics Group, University of Bonn
           http://www.cg.cs.uni-bonn.de/project-pages/3dsearch/

Code by Marcin Novotni:     marcin@cs.uni-bonn.de

for more information, see the paper:

@inproceedings{novotni-2003-3d,
    author = {M. Novotni and R. Klein},
    title = {3{D} {Z}ernike Descriptors for Content Based Shape Retrieval},
    booktitle = {The 8th ACM Symposium on Solid Modeling and Applications},
    pages = {216--225},
    year = {2003},
    month = {June},
    institution = {Universit\"{a}t Bonn},
    conference = {The 8th ACM Symposium on Solid Modeling and Applications, June 16-20, Seattle, WA}
}
 *---------------------------------------------------------------------------*
 *                                                                           *
 *                                License                                    *
 *                                                                           *
 *  This library is free software; you can redistribute it and/or modify it  *
 *  under the terms of the GNU Library General Public License as published   *
 *  by the Free Software Foundation, version 2.                              *
 *                                                                           *
 *  This library is distributed in the hope that it will be useful, but      *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *  Library General Public License for more details.                         *
 *                                                                           *
 *  You should have received a copy of the GNU Library General Public        *
 *  License along with this library; if not, write to the Free Software      *
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                *
 *                                                                           *
\*===========================================================================*/

/*
    This is a demonstration of how the classes may be used to generate the
    3D Zernike descriptors from a given input binary file containing the
    voxel grid representation of the object.

    Notice that in the below case, the file contains a cubic grid, i.e. the
    x-, y-, and z-dimensions are equal (such setting should satisfy most needs).
*/

#include "stdafx.h"
#include "compute_descriptors.h"

using namespace boost::program_options;
using namespace boost::filesystem;

constexpr char* order_arg_name{ u8"max-order" };
constexpr char* order_arg_short_name{ u8"n" };
constexpr char* dir_arg_name{ u8"dir" };
constexpr char* dir_arg_short_name{ u8"d" };
constexpr char* thread_arg_name{ u8"threads" };
constexpr char* thread_arg_short_name{ u8"t" };
constexpr char* queue_arg_name{ u8"queue-size" };
constexpr char* queue_arg_short_name{ u8"s" };
constexpr char* log_sett_arg_name{ u8"logconf" };
constexpr char* log_sett_short_arh_name{ u8"l" };
constexpr char* xml_dir_arg_name{ u8"output-dir" };
constexpr char* xml_short_arg_name{ u8"o" };

bool init_logg_settings_from_file(const path& path_to_config)
{
    using std::ifstream;
    using std::cerr;
    using std::endl;

    ifstream settings(path_to_config.string());

    if (!settings.is_open())
    {
        cerr << u8"Could not open " << path_to_config << endl;
        return false;
    }

    // Read the settings and initialize logging library
    try
    {
        boost::log::init_from_stream(settings);
    }
    catch (std::exception & exc)
    {
        cerr << exc.what() << endl;
        settings.close();
        return false;
    }

    settings.close();

    boost::log::core::get()->add_global_attribute(u8"TimeStamp", boost::log::attributes::local_clock());
    boost::log::core::get()->add_global_attribute(u8"ThreadID", boost::log::attributes::current_thread_id());

    return true;
}

std::tuple<variables_map, options_description> parse_cli_args(int argc, char** argv)
{
    using std::string;

    string dir{ dir_arg_name };
    dir += ',';
    dir += dir_arg_short_name;

    string order{ order_arg_name };
    order += ',';
    order += order_arg_short_name;

    string thred_arg{ thread_arg_name };
    thred_arg += ',';
    thred_arg += thread_arg_short_name;

    string queue_arg{ queue_arg_name };
    queue_arg += ',';
    queue_arg += queue_arg_short_name;

    string log_arg{ log_sett_arg_name };
    log_arg += ',';
    log_arg += log_sett_short_arh_name;

    string xml_arg{ xml_dir_arg_name };
    xml_arg += ',';
    xml_arg += xml_short_arg_name;

    options_description desc{ u8"Program options for descriptors. Create .inv file with descriptors for each binvox in input directory.\nSee: Novotni M., Klein R. 3D zernike descriptors for content based shape retrieval New York, New York, USA: ACM Press, 2003. 216 с." };
    desc.add_options()
        (u8"help,h", u8"-d path_to_directory -n max_order")
        (dir.c_str(), value<string>(), u8"Path to directory with .binvox files.")
        (order.c_str(), value<int>(), u8"Maximum order of Zernike moments. N in original paper.")
        (thred_arg.c_str(), value<int>()->default_value(2), u8"Maximum number of threads for descriptor computing.")
        (queue_arg.c_str(), value<int>()->default_value(500), u8"Maximum size of queue of file paths when recursive scanning directory. If size of queue is greater than parameter then scanning thread sleeps.")
        (log_arg.c_str(), value<string>()->default_value(u8"logsettings.ini"), u8"Path to file with log config. See https://www.boost.org/doc/libs/1_72_0/libs/log/doc/html/log/detailed/utilities.html#log.detailed.utilities.setup.settings_file")
        (xml_arg.c_str(), value<string>()->default_value(u8"xml-desc"), u8"Path to output directory to store XML with results")
        ;

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    return make_tuple(vm, desc);
}

bool validate_args(const variables_map& args, const options_description& desc)
{
    using std::cout;
    using std::cerr;
    using std::endl;
    using std::string;

    {
        path input_dir{ args[dir_arg_name].as<string>() };

        if (status(input_dir).type() != file_type::directory_file)
        {
            cerr << input_dir << u8" is not directory or does not exist." << endl;
            return false;
        }
    }

    {
        int max_order{ args[order_arg_name].as<int>() };

        if (max_order <= 0)
        {
            cerr << u8"Maximum order must be positive. Actual value is " << max_order << endl;
            return false;
        }
    }

    {
        int n_thread{ args[thread_arg_name].as<int>() };

        if (n_thread <= 0)
        {
            cerr << u8"Number of thread must be positive. Actual value is " << n_thread << endl;
            return false;
        }
    }

    {
        int queue_size{ args[queue_arg_name].as<int>() };

        if (queue_size <= 0)
        {
            cerr << u8"Queue size must be positive. Actual value is " << queue_size << endl;
            return false;
        }
    }

    {
        path log_sett{ args[log_sett_arg_name].as<string>() };

        if (status(log_sett).type() != file_type::regular_file)
        {
            cerr << log_sett << u8" is not file or does not exist." << endl;
            return false;
        }
    }

    {
        path xml_dir{ args[xml_dir_arg_name].as<string>() };

        if (exists(xml_dir))
        {
            if (status(xml_dir).type() != file_type::directory_file)
            {
                cerr << xml_dir_arg_name << u8" is not directory or does not exist." << endl;
                return false;
            }
        }
    }

    return true;
}

void clear()
{
    xmlCleanupParser();
    boost::log::core::get()->remove_all_sinks();
}

int main(int argc, char** argv)
{
    using std::cout;
    using std::endl;
    using std::cerr;
    using std::string;

    LIBXML_TEST_VERSION

        variables_map args;

    try
    {
        auto res_tuple = parse_cli_args(argc, argv);
        args = std::get<0>(res_tuple);
        auto desc = std::get<1>(res_tuple);

        if (args.count(u8"help"))
        {
            cout << desc << endl;
            return 0;
        }

        if (!validate_args(args, desc))
        {
            cout << desc << endl;
            return 1;
        }
    }
    catch (const error & err)
    {
        cerr << err.what() << endl;
        return 1;
    }

    if (!init_logg_settings_from_file(args[log_sett_arg_name].as<string>()))
    {
        return 1;
    }

    path input_directory{ args[dir_arg_name].as<string>() };
    int max_order{ args[order_arg_name].as<int>() };
    int queue_size{ args[queue_arg_name].as<int>() };
    int thread_count{ args[thread_arg_name].as<int>() };
    path xml_dir{ args[xml_dir_arg_name].as<string>() };

    logging::logger_t& logger = logging::logger_main::get();

    try
    {
        if (!exists(xml_dir))
        {
            BOOST_LOG_SEV(logger, logging::severity_t::info) << u8"Create directory: " << xml_dir << endl;

            if (!create_directories(xml_dir))
            {
                BOOST_LOG_SEV(logger, logging::severity_t::error) << u8"Cannot create directories" << endl;

                clear();

                return 1;
            }
        }

        for (auto& path : directory_iterator(xml_dir))
        {
            if (path.path().extension() == u8".xml")
            {
                BOOST_LOG_SEV(logger, logging::severity_t::info) << u8"Delete " << path << endl;
                remove(path);
            }
        }
    }
    catch (const boost::filesystem::filesystem_error & exc)
    {
        BOOST_LOG_SEV(logger, logging::severity_t::error) << exc.what() << endl;
        clear();
        return 1;
    }

    parallel::recursive_compute(input_directory, max_order, queue_size, thread_count, xml_dir);

    clear();

    return 0;
}