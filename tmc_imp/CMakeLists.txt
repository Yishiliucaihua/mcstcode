cmake_minimum_required (VERSION 3.28)
project (mcst_imp)

SET(CMAKE_BUILD_TYPE "Release")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -std=c++17")

include_directories (src)
aux_source_directory (src SRC_LIST)
add_executable(mcst_imp ${SRC_LIST})
