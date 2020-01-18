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
#include "binvox_reader.h"

using namespace boost::program_options;
using namespace boost::filesystem;
using namespace std;

constexpr char* order_arg_name{ "max-order" };
constexpr char* order_arg_short_name{ "n" };
constexpr char* dir_arg_name{ "dir" };
constexpr char* dir_arg_short_name{ "d" };

auto parse_cli_args(int argc, char** argv)
{
	string dir{ dir_arg_name };
	dir += ',';
	dir += dir_arg_short_name;

	string order{ order_arg_name };
	order += ',';
	order += order_arg_short_name;

	options_description desc{ "Program options for descriptors. Create .inv file with descriptors for each binvox in input directory.\nSee: Novotni M., Klein R. 3D zernike descriptors for content based shape retrieval New York, New York, USA: ACM Press, 2003. 216 с." };
	desc.add_options()
		("help,h", "-d path_to_directory -n max_order")
		(dir.c_str(), value<string>()->required(), "Path to directory with .binvox files.")
		(order.c_str(), value<int>()->required(), "Maximum order of Zernike moments. N in original paper.")
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
	catch (error & err)
	{
		cerr << err.what() << endl;
		return 1;
	}

	path input_directory{ args[dir_arg_name].as<string>() };
	int max_order{ args[order_arg_name].as<int>() };

	forward_list<path> all_voxels_files;

	{
		auto iterator = recursive_directory_iterator(input_directory);

		for (const auto& entry : iterator)
		{
			if (entry.status().type() == file_type::directory_file)
			{
				cout << "Scanning " << entry.path() << endl;
			}
			else if (entry.status().type() == file_type::regular_file)
			{
				path local_file{ entry.path() };

				if (local_file.extension() == ".binvox")
				{
					cout << "Found " << local_file << endl;
					all_voxels_files.push_front(std::move(local_file));
				}
			}
		}
	}

	// .inv file name
	for (const auto& path_to_voxel : all_voxels_files)
	{
		vector<double> voxels;
		size_t dim{};

		if (!io::binvox::read_binvox(path_to_voxel, voxels, dim))
		{
			cout << "Cannot read binvox from " << path_to_voxel << endl;
			continue;
		}

		path new_path = path_to_voxel;

		new_path.replace_extension(".inv");

		// compute the zernike descriptors
		ZernikeDescriptor<double, double> zd(voxels.data(), dim, max_order);

		cout << "Saving invariants file: " << new_path << endl;
		zd.SaveInvariants(new_path.string());
	}
}