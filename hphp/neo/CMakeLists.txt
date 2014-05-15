set(CXX_SOURCES)
auto_sources(files "*.c" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

add_library(neo STATIC ${CXX_SOURCES})

auto_sources(files "*.h" "RECURSE")
HHVM_PUBLIC_HEADERS(neo ${files})
