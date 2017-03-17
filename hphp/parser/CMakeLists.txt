set(CXX_SOURCES)
auto_sources(files "*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

set(HEADER_SOURCES)
auto_sources(files "*.h" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND HEADER_SOURCES ${files})
HHVM_PUBLIC_HEADERS(parser ${files})

set(GROUP_SOURCES)
list(APPEND GROUP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}" ${CXX_SOURCES}
  ${HEADER_SOURCES})

add_library(hphp_parser STATIC ${CXX_SOURCES} ${HEADER_SOURCES})
set(COMPILER_HEADER "${HPHP_HOME}/hphp/compiler/parser/parser.h")
auto_source_group("hphp_parser" ${GROUP_SOURCES} ${COMPILER_HEADER})
set_target_properties(hphp_parser PROPERTIES COMPILE_FLAGS
  "-DPARSER_DEFINITIONS_HEADER='\"${COMPILER_HEADER}\"'")

add_library(hphp_facts_parser STATIC ${CXX_SOURCES} ${HEADER_SOURCES})
set(FACTS_HEADER "${HPHP_HOME}/hphp/runtime/ext/factparse/parser.h")
auto_source_group("hphp_facts_parser" ${GROUP_SOURCES} ${FACTS_HEADER})
set_target_properties(hphp_facts_parser PROPERTIES COMPILE_FLAGS
  "-DPARSER_DEFINITIONS_HEADER='\"${FACTS_HEADER}\"'")

if (ENABLE_COTIRE)
  cotire(hphp_parser)
  cotire(hphp_facts_parser)
endif()
