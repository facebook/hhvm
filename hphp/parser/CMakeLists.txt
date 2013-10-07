set(CXX_SOURCES)
auto_sources(files "*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

add_library(hphp_parser STATIC ${CXX_SOURCES})

