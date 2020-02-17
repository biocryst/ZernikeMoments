add_library(3DZM INTERFACE)
target_sources(3DZM INTERFACE INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/ScaledGeometricalMoments.cpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/ScaledGeometricMoments.h INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/ZernikeDescriptor.h INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/ZernikeMoments.cpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/ZernikeMoments.h INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/ZernikeMoments.inl)
target_compile_features(3DZM INTERFACE cxx_std_14)
target_include_directories(3DZM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
