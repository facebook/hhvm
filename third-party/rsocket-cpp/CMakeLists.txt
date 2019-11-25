cmake_minimum_required(VERSION 2.8.0)

set(RSOCKET_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

if (OPENSSL_HAVE_ALPN)
  # Main rsocket_cpp library files
  auto_sources(files "*.cpp" "RECURSE" "${RSOCKET_CPP_DIR}")
  auto_sources(hfiles "*.h" "RECURSE" "${RSOCKET_CPP_DIR}")
else()
  auto_sources(files "*.cpp" "RECURSE" "${RSOCKET_CPP_DIR}/yarpl")
  auto_sources(hfiles "*.h" "RECURSE" "${RSOCKET_CPP_DIR}/yarpl")
endif()

HHVM_REMOVE_MATCHES_FROM_LISTS(files hfiles MATCHES "/devtools/" "/benchmarks/" "/examples/" "/test/" "/perf/")

include_directories("${RSOCKET_CPP_DIR}")
add_library(rsocket_cpp STATIC ${files} ${hfiles})
auto_source_group(rsocket_cpp "${RSOCKET_CPP_DIR}" ${files} ${hfiles})
target_link_libraries(rsocket_cpp folly)
