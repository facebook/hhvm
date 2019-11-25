cmake_minimum_required(VERSION 2.8.0)
project(re2 CXX C)

auto_sources(sources "*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/src/re2")
SET(util_sources
  src/util/rune.cc
  src/util/strutil.cc
)

set(RE2_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src" CACHE STRING "RE2 include path")
# Blow away the global include directories - eg both hhvm and re2 have a
# util/arena.h
SET_PROPERTY(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "${RE2_INCLUDE_DIR}")
add_library(re2 STATIC ${util_sources} ${sources})
