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
using namespace std;
\

constexpr char* order_arg_name{ "max-order" };
constexpr char* order_arg_short_name{ "n" };
constexpr char* dir_arg_name{ "dir" };
constexpr char* dir_arg_short_name{ "d" };
constexpr char* thread_arg_name{ "threads" };
constexpr char* thread_arg_short_name{ "t" };
constexpr char* queue_arg_name{ "queue-size" };
constexpr char* queue_arg_short_name{ "s" };
constexpr char* log_sett_arg_name{ "logconf" };
constexpr char* log_sett_short_arh_name{ "l" };

void init_logg_settings_from_file(const path& path_to_config)
{
    std::ifstream settings(path_to_config.string());

    if (!settings.is_open())
    {
        std::cerr << "Could not open " << path_to_config << std::endl;
    }

    // Read the settings and initialize logging library
    boost::log::init_from_stream(settings);

    settings.close();

    // Add some attributes
    boost::log::core::get()->add_global_attribute("TimeStamp", boost::log::attributes::local_clock());
    boost::log::core::get()->add_global_attribute("ThreadID", boost::log::attributes::current_thread_id());
}

auto parse_cli_args(int argc, char** argv)
{
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

    options_description desc{ "Program options for descriptors. Create .inv file with descriptors for each binvox in input directory.\nSee: Novotni M., Klein R. 3D zernike descriptors for content based shape retrieval New York, New York, USA: ACM Press, 2003. 216 с." };
    desc.add_options()
        ("help,h", "-d path_to_directory -n max_order")
        (dir.c_str(), value<string>(), "Path to directory with .binvox files.")
        (order.c_str(), value<int>(), "Maximum order of Zernike moments. N in original paper.")
        (thred_arg.c_str(), value<int>()->default_value(2), "Maximum number of threads.")
        (queue_arg.c_str(), value<int>()->default_value(500), "Maximum size of queue of file paths when recursive scanning directory. If size of queue is greater than parameter then scanning thread sleeps.")
        (log_arg.c_str(), value<string>()->default_value("logsettings.ini"), "Path to file with log config. See https://www.boost.org/doc/libs/1_72_0/libs/log/doc/html/log/detailed/utilities.html#log.detailed.utilities.setup.settings_file");
    ;

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    return make_tuple(vm, desc);
}

bool validate_args(const variables_map& args, const options_description& desc)
{
    if (args.count(dir_arg_name) != 1)
    {
        cout << dir_arg_name << " occurred multiple times." << endl;
        return false;
    }

    if (args.count(order_arg_name) != 1)
    {
        cout << order_arg_name << " occurred multiple times." << endl;
        return false;
    }

    if (args.count(thread_arg_name) != 1)
    {
        cout << order_arg_name << " occurred multiple times." << endl;
        return false;
    }

    if (args.count(queue_arg_name) != 1)
    {
        cout << order_arg_name << " occurred multiple times." << endl;
        return false;
    }

    path input_dir{ args[dir_arg_name].as<string>() };

    if (status(input_dir).type() != file_type::directory_file)
    {
        cerr << input_dir << " is not directory or does not exist." << endl;
        return false;
    }

    int max_order{ args[order_arg_name].as<int>() };

    if (max_order <= 0)
    {
        cerr << "Maximum order must be positive. Actual value is " << max_order << endl;
        return false;
    }

    int n_thread{ args[thread_arg_name].as<int>() };

    if (n_thread <= 0)
    {
        cerr << "Number of thread must be positive. Actual value is " << max_order << endl;
        return false;
    }

    int queue_size{ args[queue_arg_name].as<int>() };

    if (queue_size <= 0)
    {
        cerr << "Queue size must be positive. Actual value is " << max_order << endl;
        return false;
    }

    path log_sett{ args[log_sett_arg_name].as<string>() };

    if (status(log_sett).type() != file_type::regular_file)
    {
        cerr << log_sett_arg_name << " is not file or does not exist." << endl;
        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    variables_map args;

    try
    {
        auto res_tuple = parse_cli_args(argc, argv);
        args = get<0>(res_tuple);
        auto desc = get<1>(res_tuple);

        if (args.count("help"))
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

    init_logg_settings_from_file(args[log_sett_arg_name].as<string>());

    path input_directory{ args[dir_arg_name].as<string>() };
    int max_order{ args[order_arg_name].as<int>() };
    int queue_size{ args[queue_arg_name].as<int>() };
    int thread_count{ args[thread_arg_name].as<int>() };

    parallel::recursive_compute(input_directory, max_order, queue_size, thread_count);

    return 0;
}