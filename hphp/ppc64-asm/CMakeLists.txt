set(CXX_SOURCES)
set(HEADER_SOURCES)

auto_sources(files "*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})
auto_sources(files "*.h" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND HEADER_SOURCES ${files})

add_library(ppc64-asm STATIC ${HEADERS_SOURCES} ${CXX_SOURCES})

HHVM_PUBLIC_HEADERS(ppc64-asm ${HEADERS_SOURCES})
