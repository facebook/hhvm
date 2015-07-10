set(C_SOURCES)
auto_sources(files "*.c" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND C_SOURCES ${files})

set(CXX_SOURCES)
auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

set(HEADER_SOURCES)
auto_sources(files "*.h" "RECURSE")
list(APPEND HEADER_SOURCES ${files})
HHVM_PUBLIC_HEADERS(zend ${files})

add_library(hphp_zend STATIC ${C_SOURCES} ${CXX_SOURCES} ${HEADER_SOURCES})
auto_source_group("hphp_zend" "${CMAKE_CURRENT_SOURCE_DIR}"
  ${C_SOURCES} ${CXX_SOURCES} ${HEADER_SOURCES})
