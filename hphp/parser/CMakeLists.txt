set(CXX_SOURCES)
auto_sources(files "*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

set(HEADER_SOURCES)
auto_sources(files "*.h" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND HEADER_SOURCES ${files})
HHVM_PUBLIC_HEADERS(parser ${files})

add_library(hphp_parser STATIC ${CXX_SOURCES} ${HEADER_SOURCES})
auto_source_group("hphp_parser" "${CMAKE_CURRENT_SOURCE_DIR}"
  ${CXX_SOURCES} ${HEADER_SOURCES})

if (ENABLE_COTIRE)
  cotire(hphp_parser)
endif()
