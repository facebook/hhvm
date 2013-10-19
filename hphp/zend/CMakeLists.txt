set(C_SOURCES)
set(CXX_SOURCES)
auto_sources(files "*.c" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND C_SOURCES ${files})
auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

add_library(hphp_zend STATIC ${C_SOURCES} ${CXX_SOURCES})
