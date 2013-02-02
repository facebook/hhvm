auto_sources(files "*.cc" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/src")
add_library(double-conversion STATIC ${files})
