foreach(def ${EXTENSION_IDL_DEFINES})
  add_definitions(${def})
endforeach()

# This will be run after runtime/ext has, so we can use the output of
# that.
add_custom_command(
  OUTPUT "class_map.cpp"
  DEPENDS ${IDL_SRCS} gen-class-map
  COMMAND gen-class-map
  ARGS "--system"
       "${CMAKE_CURRENT_BINARY_DIR}/class_map.cpp"
       ${EXTENSION_IDL_SOURCES}
  COMMENT "Generating class_map.cpp")

set(CXX_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/class_map.cpp"
                "systemlib.cpp")

add_library(hphp_system STATIC ${CXX_SOURCES})
if (ENABLE_ASYNC_MYSQL)
  add_dependencies(hphp_system webscalesqlclient)
endif ()

auto_sources(files "*.h" "${CMAKE_CURRENT_SOURCE_DIR}")
HHVM_PUBLIC_HEADERS(system ${files})

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
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/systemlib.php)
