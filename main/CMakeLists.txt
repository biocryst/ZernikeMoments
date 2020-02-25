
find_package(Boost 1.71 REQUIRED COMPONENTS filesystem program_options log)
find_package(LibXml2 REQUIRED)

set(_precompiled_header "stdafx.h")

add_executable(zernike3d main.cpp binvox_reader.hpp stdafx.cpp stdafx.h compute_descriptors.cpp compute_descriptors.h loggers.h xmlwriter.hpp xmlmerger.cpp xmlmerger.h binvox_utils.hpp)

if(MSVC)
target_compile_options(zernike3d PRIVATE /Yu${_precompiled_header} PRIVATE /FI${_precompiled_header})
set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS /Yc${_precompiled_header})
endif()

target_compile_features(zernike3d PUBLIC cxx_std_14)
target_compile_definitions(zernike3d PRIVATE PRIVATE ${LIBXML2_DEFINITIONS})
target_include_directories(zernike3d PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} PRIVATE ${LIBXML2_INCLUDE_DIR})
target_link_libraries(zernike3d PRIVATE 3DZM PUBLIC Boost::log PUBLIC Boost::boost PUBLIC Boost::filesystem PUBLIC Boost::program_options PRIVATE ${LIBXML2_LIBRARIES}) 
