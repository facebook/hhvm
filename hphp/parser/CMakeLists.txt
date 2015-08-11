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

# Through a series of header includes, hphp.tab.cpp ends up depending on
# all-scan-decl.h, but CMake can't figure out that dependency, and so (depending
# on parallelism etc) may or may not generate that header first. Duct tape in
# the dependency... if someone really understands what's going on, feel free to
# fix properly :-P
add_dependencies(hphp_parser all-scan-header)

if (ENABLE_COTIRE)
  cotire(hphp_parser)
endif()
