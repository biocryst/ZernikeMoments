
add_executable(zernike3d main.cpp)
target_compile_features(zernike3d PUBLIC cxx_std_14)
target_link_libraries(zernike3d 3DZM)
