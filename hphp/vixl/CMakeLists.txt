set(CXX_SOURCES)
set(HEADER_SOURCES)

auto_sources(files "*.cc" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

if(NOT IS_AARCH64)
  add_definitions("-DUSE_SIMULATOR")
endif()

auto_sources(files "*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/a64")
list(APPEND CXX_SOURCES ${files})

auto_sources(files "*.h" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND HEADER_SOURCES ${files})
HHVM_PUBLIC_HEADERS(vixl ${files})

auto_sources(files "*.h" "${CMAKE_CURRENT_SOURCE_DIR}/a64")
list(APPEND HEADER_SOURCES ${files})
HHVM_PUBLIC_HEADERS(vixl ${files})

add_library(vixl STATIC ${CXX_SOURCES} ${HEADER_SOURCES})
auto_source_group("vixl" "${CMAKE_CURRENT_SOURCE_DIR}"
  ${CXX_SOURCES} ${HEADER_SOURCES})
