cmake_minimum_required(VERSION 2.8.0)

set(WANGLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/wangle")

# Needed for fcntl w/ F_SETPIPE_SZ
add_definitions("-D_GNU_SOURCE")

add_definitions(-DNO_LIB_GFLAGS)

# Main wangle library files
auto_sources(files "*.cpp" "RECURSE" "${WANGLE_DIR}")
auto_sources(hfiles "*.h" "RECURSE" "${WANGLE_DIR}")

# No need for tests, Benchmarks, Utils, or most experimental stuff
HHVM_REMOVE_MATCHES_FROM_LISTS(files hfiles MATCHES "/test/" "Test.cpp$" "/example/" "/bootstrap/")

list(REMOVE_ITEM files ${WANGLE_DIR}/channel/FileRegion.cpp)
list(REMOVE_ITEM hfiles ${WANGLE_DIR}/channel/FileRegion.h)

add_library(wangle STATIC ${files} ${hfiles})
auto_source_group(wangle "${WANGLE_DIR}" ${files} ${hfiles})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR} "${TP_DIR}/fizz/src")

target_link_libraries(wangle folly boost fizz ${OPENSSL_LIBRARIES})
