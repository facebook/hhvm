set(C_SOURCES)
auto_sources(files "*.c" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND C_SOURCES ${files})

set(HEADER_SOURCES)
auto_sources(files "*.h" "RECURSE")
list(APPEND HEADER_SOURCES ${files})
HHVM_PUBLIC_HEADERS(neo ${files})

add_library(neo STATIC ${C_SOURCES} ${HEADER_SOURCES})
auto_source_group("neo" "${CMAKE_CURRENT_SOURCE_DIR}"
  ${C_SOURCES} ${HEADER_SOURCES})

if (ENABLE_COTIRE)
  cotire(neo)
endif()
