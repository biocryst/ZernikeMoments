# 3D Zernike Descriptors

## Description of the method

Fork of GPL code writen by Marcin Novotni (Uni-Bonn) and provided as supplementary material with the paper:
M. Novotni, R. Klein "Shape Retrieval using 3D Zernike Descriptors" Computer Aided Design 2004; 36(11):1047-1062

Downloaded from: http://cg.cs.uni-bonn.de/project-pages/3dsearch/downloads.html#3D%20Zernike%20Descriptors

Copy of the original copyright notice:
```
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
```

## Requirements

1. CMake 3.10 or higher
2. Boost.Filesystem, Boost.Program_options, Boost.Log, Boost.Math, Boost.Lockfree, Libxml2.
3. Compiler with C++14.

## Building

For simplicity you can install [vcpkg](https://github.com/microsoft/vcpkg).

Create a new directory `build`.

### Generating project

Run CMake:
```
cmake -DCMAKE_BUILD_TYPE=Release  -S . -B .\build
```

If you have vcpkg, then:
```
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="<path_to_installed_vcpkg_dir>\vcpkg\scripts\buildsystems\vcpkg.cmake"  -S . -B .\build
```

Example for Visual Studio 2019:
```
cmake -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="<path_to_installed_vcpkg_dir>\vcpkg\scripts\buildsystems\vcpkg.cmake" -S . -B .\build

```

### Compilation

Run:
```
cmake --build .\build --target ALL_BUILD --config Release
```

### Remarks

CMake supports only precompiled header for MSVS compiler. In this case, all files include implicitly `stdafx.h`.

On Linux you may need to add follow lines:
```
find_package(Boost 1.71 REQUIRED COMPONENTS filesystem program_options log_setup log)

target_link_libraries(zernike3d PRIVATE 3DZM PUBLIC Boost::log_setup PUBLIC Boost::log PUBLIC Boost::boost PUBLIC Boost::filesystem PUBLIC Boost::program_options PRIVATE ${LIBXML2_LIBRARIES})
```

## How to use

1. Copy `.\main\logsettings.ini` to directory with executable file of program.
2. Run program: `.\zernike3d.exe -d <path_to_directory_with_binvox> -n 20 -t 4`.

The program computes Zernike Descriptors for all binvox files in the directory and subdirectories. It saves results in XML file `/xml-desc/results.xml`. By default directory `xml-desc` is in the same directory where executable is. For more information see: `.\zernike3d.exe --help`.


## Voxelization

You can use [this repository](https://github.com/KernelA/cuda_voxelizer) for getting binvox voxels.
