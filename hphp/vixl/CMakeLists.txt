set(CXX_SOURCES)
set(HEADER_SOURCES)

auto_sources(files "*.cc" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

if(NOT IS_AARCH64)
  add_definitions("-DHPHP_VIXL")
endif()

auto_sources(files "*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/aarch64")
# Exclude simulator-related .cc files
list(FILTER files EXCLUDE REGEX "simulator-aarch64\\.cc$")
list(FILTER files EXCLUDE REGEX "debugger-aarch64\\.cc$")
list(FILTER files EXCLUDE REGEX "logic-aarch64\\.cc$")
list(APPEND CXX_SOURCES ${files})

auto_sources(files "*.h" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND HEADER_SOURCES ${files})
HHVM_PUBLIC_HEADERS(vixl ${files})

auto_sources(files "*.h" "${CMAKE_CURRENT_SOURCE_DIR}/aarch64")
list(APPEND HEADER_SOURCES ${files})
HHVM_PUBLIC_HEADERS(vixl ${files})

add_library(vixl STATIC ${CXX_SOURCES} ${HEADER_SOURCES})
target_compile_definitions(vixl PRIVATE
  HPHP_VIXL VIXL_INCLUDE_TARGET_AARCH64 VIXL_CODE_BUFFER_MALLOC)
target_link_libraries(vixl hphp_util)
auto_source_group("vixl" "${CMAKE_CURRENT_SOURCE_DIR}"
  ${CXX_SOURCES} ${HEADER_SOURCES})
