add_library(3DZM INTERFACE)
target_compile_features(3DZM INTERFACE cxx_std_14)
target_include_directories(3DZM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
