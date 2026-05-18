find_package(mcrouter CONFIG REQUIRED)

add_library(mcrouter INTERFACE)

get_target_property(
  MCROUTER_INCLUDE_DIRS
  mcrouter::mcrouter_memcache_service_thrift
  INTERFACE_INCLUDE_DIRECTORIES
)

if(NOT MCROUTER_INCLUDE_DIRS)
  message(FATAL_ERROR "mcrouter package did not expose include directories")
endif()

list(REMOVE_DUPLICATES MCROUTER_INCLUDE_DIRS)

target_include_directories(
  mcrouter
  SYSTEM
  INTERFACE
  ${MCROUTER_INCLUDE_DIRS}
)

target_link_libraries(
  mcrouter
  INTERFACE
  mcrouter::mcroutercore
)
