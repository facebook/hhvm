cmake_minimum_required(VERSION 2.8.0)

set(FMT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fmt/")

# Main fmt library files
auto_sources(files "*.cc" "RECURSE" "${FMT_DIR}/src")
auto_sources(hfiles "*.h" "RECURSE" "${FMT_DIR}/include")
include_directories("${FMT_DIR}/include")

add_library(fmt STATIC ${files} ${hfiles})
auto_source_group(fmt "${FMT_DIR}" ${files} ${hfiles})
