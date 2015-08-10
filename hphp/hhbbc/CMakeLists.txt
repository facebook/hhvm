set(CXX_SOURCES)
auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

set(HEADER_SOURCES)
auto_sources(files "*.h" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND HEADER_SOURCES ${files})
HHVM_PUBLIC_HEADERS(hhbc ${files})


# remove anything in a test folder
HHVM_REMOVE_MATCHES_FROM_LISTS(CXX_SOURCES HEADER_SOURCES MATCHES "/test/")

add_library(hphp_hhbbc STATIC ${CXX_SOURCES} ${HEADER_SOURCES})
auto_source_group("hphp_hhbbc" "${CMAKE_CURRENT_SOURCE_DIR}"
  ${CXX_SOURCES} ${HEADER_SOURCES})
add_dependencies(hphp_hhbbc hphp_system)
if (ENABLE_COTIRE)
  cotire(hphp_hhbbc)
endif()
