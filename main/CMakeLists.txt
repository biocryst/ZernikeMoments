find_package(Boost 1.71 REQUIRED COMPONENTS filesystem program_options log)
find_package(SQLite3 REQUIRED)

# sqlite_modern
set(SQLITE_MODERN_CPP_INCLUDE_DIR "" CACHE PATH "A path to directory which contains sqlite_modern_cpp.h file. See official repositiry: https://github.com/SqliteModernCpp/sqlite_modern_cpp")

# PicoSHA2 
set(PicoSHA2_INCLUDE_DIR "" CACHE PATH "A path to directory which contains picosha2.h file. See official repositiry: https://github.com/okdshin/PicoSHA2")

if(NOT PicoSHA2_INCLUDE_DIR)
message(FATAL_ERROR "You need set path to PicoSHA2 header dir. Set PicoSHA2_INCLUDE_DIR")
endif()

if(NOT SQLITE_MODERN_CPP_INCLUDE_DIR)
message(FATAL_ERROR "You need set path to sqlite_modern_cpp header dir. Set SQLITE_MODERN_CPP_INCLUDE_DIR")
endif()

find_file(SQLITE_MODERN_CPP_HEADER sqlite_modern_cpp.h ${SQLITE_MODERN_CPP_INCLUDE_DIR})

if(NOT SQLITE_MODERN_CPP_HEADER)
	message(FATAL_ERROR "Can't find sqlite_modern_cpp.h in ${SQLITE_MODERN_CPP_INCLUDE_DIR}")
else()
	message(STATUS "Found SQLite modern cpp: ${SQLITE_MODERN_CPP_HEADER}")
endif()

find_file(PicoSHA2_HEADER picosha2.h ${PicoSHA2_INCLUDE_DIR})

if(NOT PicoSHA2_HEADER)
	message(FATAL_ERROR "Can't find picosha2.h in ${PicoSHA2_INCLUDE_DIR}")
else()
	message(STATUS "Found PicoSHA2: ${PicoSHA2_HEADER}")
endif()


add_executable(zernike3d 
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/include/binvox_reader.hpp 
	${CMAKE_CURRENT_SOURCE_DIR}/include/stdafx.h 
	${CMAKE_CURRENT_SOURCE_DIR}/src/compute_descriptors.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/include/compute_descriptors.h 
	${CMAKE_CURRENT_SOURCE_DIR}/include/loggers.h 
	${CMAKE_CURRENT_SOURCE_DIR}/include/binvox_utils.hpp  
	${CMAKE_CURRENT_SOURCE_DIR}/include/compute_sha256.h 
	${CMAKE_CURRENT_SOURCE_DIR}/include/path_tree.hpp 
	${CMAKE_CURRENT_SOURCE_DIR}/src/compute_sha256.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/sqlite_row.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/db.h
)
target_compile_features(zernike3d PRIVATE cxx_std_14)
target_compile_definitions(zernike3d PRIVATE BOOST_ALL_DYN_LINK)
target_include_directories(zernike3d PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${SQLITE_MODERN_CPP_INCLUDE_DIR} PRIVATE ${PicoSHA2_INCLUDE_DIR})
target_precompile_headers(zernike3d PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/stdafx.h)
target_link_libraries(zernike3d PRIVATE 3DZM PRIVATE SQLite::SQLite3 PRIVATE Boost::log PRIVATE Boost::boost PRIVATE Boost::filesystem PRIVATE Boost::program_options PRIVATE Boost::dynamic_linking )
