set(CXX_SOURCES)
auto_sources(files "*.cc" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

add_definitions("-DUSE_SIMULATOR")
auto_sources(files "*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/a64")
list(APPEND CXX_SOURCES ${files})

add_library(vixl STATIC ${CXX_SOURCES})
