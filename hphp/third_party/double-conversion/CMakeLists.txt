auto_sources(files "*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/src")
add_library(double-conversion STATIC ${files})
