cmake_minimum_required(VERSION 2.8.12)
project(example-c++)

set(CMAKE_CXX_STANDARD 11)

include_directories(../../src/include)
link_directories(../../build/src)

add_definitions(-DONNX_ML=1)
add_definitions(-DONNX_NAMESPACE=onnx)
add_executable(example main.cpp)
target_link_libraries(example duckdb)
