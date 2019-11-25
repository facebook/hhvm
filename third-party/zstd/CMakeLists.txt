set(ZSTD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/lib")

set(C_SOURCES)
auto_sources(files "*.c" "RECURSE" "${ZSTD_DIR}")
list(APPEND C_SOURCES ${files})
include_directories("${ZSTD_DIR}/common")
add_library(zstd STATIC ${C_SOURCES})
target_include_directories(zstd PUBLIC "${ZSTD_DIR}")
target_include_directories(zstd PUBLIC "${ZSTD_DIR}/dictBuilder")
