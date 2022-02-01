cmake_minimum_required(VERSION 3.10)
project(squangle CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# TODO when Ubuntu 18.04 support is dropped:
# - use CONFIGURE_DEPENDS in these globs

file(GLOB_RECURSE SOURCES "squangle/*.cpp")

add_library(squangle STATIC ${SOURCES})
target_include_directories(squangle PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

foreach(DEP Boost fbmysqlclient re2)
  set("${DEP}_INCLUDE_DIR" "${DEP}_INCLUDE_DIR-notfound" CACHE PATH "Path to ${DEP}'s includes")
  target_include_directories(squangle PRIVATE "${${DEP}_INCLUDE_DIR}")
endforeach()

find_package(folly REQUIRED)
target_link_libraries(squangle Folly::folly)

install(TARGETS squangle LIBRARY ARCHIVE DESTINATION lib)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/" DESTINATION include FILES_MATCHING PATTERN "*.h")

