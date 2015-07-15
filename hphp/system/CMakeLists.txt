auto_sources(IDL_SRCS "*.idl.json" "RECURSE"
             "${CMAKE_CURRENT_SOURCE_DIR}/../system/idl")

add_custom_command(
  OUTPUT "class_map.cpp" "constants.h"
  DEPENDS ${IDL_SRCS} gen-class-map
  COMMAND gen-class-map
  ARGS "--system"
       "${CMAKE_CURRENT_BINARY_DIR}/class_map.cpp"
       "${CMAKE_CURRENT_BINARY_DIR}/constants.h"
       ${IDL_SRCS}
  COMMENT "Generating class_map.cpp and constants.h")

set(CXX_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/class_map.cpp"
                "systemlib.cpp")

add_library(hphp_system STATIC ${CXX_SOURCES})
if (ENABLE_ASYNC_MYSQL)
  add_dependencies(hphp_system webscalesqlclient)
endif ()

auto_sources(files "*.h" "${CMAKE_CURRENT_SOURCE_DIR}")
# constants.h doesn't exist until build time,
# so we need to manually inject it here
list(APPEND files "${CMAKE_CURRENT_SOURCE_DIR}/constants.h")
HHVM_PUBLIC_HEADERS(system ${files})

# Needed to force system/constants.h to generate prior to files which need it
if (ENABLE_ZEND_COMPAT)
  add_dependencies(hphp_ext_zend_compat hphp_system)
endif()

add_dependencies(hphp_runtime_static hphp_system)
add_dependencies(hphp_runtime_ext hphp_system)
add_dependencies(hphp_analysis hphp_system)

FILE(STRINGS "php.txt" SYSTEMLIB_CLASSES)
set(SYSTEMLIB_SRCS)
set(SYSTEMLIB_SRCS_STR)
foreach(cls ${SYSTEMLIB_CLASSES})
  STRING(REGEX REPLACE "[ \t]*#.*" "" cls "${cls}")
  if (NOT "${cls}" STREQUAL "")
    list(APPEND SYSTEMLIB_SRCS "../../${cls}")
    set(SYSTEMLIB_SRCS_STR "${SYSTEMLIB_SRCS_STR} ${cls}")
  endif()
endforeach()

add_custom_target(
  systemlib
  DEPENDS "php.txt" ${SYSTEMLIB_SRCS}
  COMMAND "INSTALL_DIR=${CMAKE_CURRENT_BINARY_DIR}"
          "FBCODE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/../.."
          "${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ext_hhvm/make_systemlib.sh"
          "--install_dir=${CMAKE_CURRENT_BINARY_DIR}"
          "--fbcode_dir=${CMAKE_CURRENT_SOURCE_DIR}/.."
          "${SYSTEMLIB_SRCS_STR}"
  COMMENT "Generating systemlib.php")
