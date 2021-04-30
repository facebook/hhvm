set(CXX_SOURCES)
auto_sources(files "*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

set(HEADER_SOURCES)
auto_sources(files "*.h" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND HEADER_SOURCES ${files})
HHVM_PUBLIC_HEADERS(parser ${files})

set(GROUP_SOURCES)
list(APPEND GROUP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}" ${CXX_SOURCES}
  ${HEADER_SOURCES})

add_library(hphp_parser STATIC ${CXX_SOURCES} ${HEADER_SOURCES})
target_link_libraries(hphp_parser hphp_util)
auto_source_group("hphp_parser" ${GROUP_SOURCES})
set_target_properties(hphp_parser PROPERTIES COMPILE_FLAGS "")

if (ENABLE_COTIRE)
  cotire(hphp_parser)
endif()
