auto_sources(IDL_SRCS "*.idl.json" "RECURSE" "${HPHP_HOME}/hphp/idl")
string(REPLACE ";" " " IDL_SRCS_STR "${IDL_SRCS}")

set(CONSTDEF "${CMAKE_CURRENT_SOURCE_DIR}/globals/constdef.json")

add_custom_command(OUTPUT "class_map.cpp" "constants.h"
                   DEPENDS ${CONSTDEF} ${IDL_SRCS} gen-class-map
                   COMMAND "${HPHP_HOME}/hphp/tools/gen-ext-hhvm/gen-class-map.sh"
                   ARGS "${CMAKE_CURRENT_SOURCE_DIR}/class_map.cpp"
                        "${CMAKE_CURRENT_SOURCE_DIR}/constants.h"
                        ${CONSTDEF} ${IDL_SRCS_STR}
                   COMMENT "Generating class_map.cpp and constants.h")

set(CXX_SOURCES "class_map.cpp" "lib/systemlib.cpp")

add_library(hphp_system STATIC ${CXX_SOURCES})

# Needed to force system/constants.h to generate prior to files which need it
add_dependencies(hphp_runtime_static hphp_system)
add_dependencies(hphp_analysis hphp_system)

