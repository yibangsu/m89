cmake_minimum_required(VERSION 3.13.1)

project(m89 VERSION 1.0)

# need to define the target first
add_executable(m89 
    "${CMAKE_SOURCE_DIR}/src/m89.c")

configure_file(Config.h.in Config.h)
target_include_directories(m89 PUBLIC 
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_SOURCE_DIR}/include")