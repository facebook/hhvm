set(CXX_SOURCES "systemlib.cpp")

add_library(hphp_system STATIC ${CXX_SOURCES})
add_dependencies(hphp_system hphp_util)

auto_sources(files "*.h" "${CMAKE_CURRENT_SOURCE_DIR}")
HHVM_PUBLIC_HEADERS(system ${files})

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

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/systemlib.php
  DEPENDS "php.txt" ${SYSTEMLIB_SRCS}
  COMMAND "INSTALL_DIR=${CMAKE_CURRENT_BINARY_DIR}"
          "FBCODE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/../.."
          "${CMAKE_CURRENT_SOURCE_DIR}/make_systemlib.sh"
          "--install_dir=${CMAKE_CURRENT_BINARY_DIR}"
          "--fbcode_dir=${CMAKE_CURRENT_SOURCE_DIR}/.."
          "${SYSTEMLIB_SRCS_STR}"
  COMMENT "Generating systemlib.php")

add_custom_target(
  systemlib
  DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/systemlib.php
  generated_systemlib
)
