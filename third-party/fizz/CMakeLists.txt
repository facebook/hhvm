cmake_minimum_required(VERSION 2.8.0)

set(FIZZ_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Needed for fcntl w/ F_SETPIPE_SZ
add_definitions("-D_GNU_SOURCE")

add_definitions(-DNO_LIB_GFLAGS)

# Main fizz library files
auto_sources(files "*.cpp" "RECURSE" "${FIZZ_DIR}")
auto_sources(hfiles "*.h" "RECURSE" "${FIZZ_DIR}")

HHVM_REMOVE_MATCHES_FROM_LISTS(files hfiles MATCHES "/test/" "Test.cpp$" "/tool/" "/extensions/javacrypto/")

add_library(fizz STATIC ${files} ${hfiles})
auto_source_group(fizz "${FIZZ_DIR}" ${files} ${hfiles})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR} ${FIZZ_DIR})

add_dependencies(fizz libsodiumMaybeBuild)
target_include_directories(fizz PUBLIC "${LIBSODIUM_INCLUDE_DIRS}")

target_link_libraries(fizz folly boost brotli_enc brotli_dec zstd libsodium ${OPENSSL_LIBRARIES})
