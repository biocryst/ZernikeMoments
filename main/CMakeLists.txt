
find_package(Boost 1.71 REQUIRED COMPONENTS filesystem program_options log)
find_package(LibXml2 REQUIRED)

add_executable(zernike3d ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/include/binvox_reader.hpp ${CMAKE_CURRENT_SOURCE_DIR}/include/stdafx.h ${CMAKE_CURRENT_SOURCE_DIR}/src/compute_descriptors.cpp ${CMAKE_CURRENT_SOURCE_DIR}/include/compute_descriptors.h ${CMAKE_CURRENT_SOURCE_DIR}/include/loggers.h ${CMAKE_CURRENT_SOURCE_DIR}/include/xmlwriter.hpp ${CMAKE_CURRENT_SOURCE_DIR}/src/xmlmerger.cpp ${CMAKE_CURRENT_SOURCE_DIR}/include/xmlmerger.h ${CMAKE_CURRENT_SOURCE_DIR}/include/binvox_utils.hpp)

target_compile_features(zernike3d PRIVATE cxx_std_14)
target_compile_definitions(zernike3d PRIVATE ${LIBXML2_DEFINITIONS})
target_include_directories(zernike3d PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${LIBXML2_INCLUDE_DIR})
target_precompile_headers(zernike3d PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/stdafx.h)
target_link_libraries(zernike3d PRIVATE 3DZM PUBLIC Boost::log PUBLIC Boost::boost PUBLIC Boost::filesystem PUBLIC Boost::program_options PRIVATE ${LIBXML2_LIBRARIES}) 
