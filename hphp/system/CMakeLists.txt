set(CXX_SOURCES "systemlib.cpp")

add_library(hphp_system STATIC ${CXX_SOURCES})
target_link_libraries(hphp_system hphp_util proxygen hhvm_base_headers)

auto_sources(files "*.h" "${CMAKE_CURRENT_SOURCE_DIR}")
HHVM_PUBLIC_HEADERS(system ${files})

FILE(STRINGS "php.txt" SYSTEMLIB_CLASSES)
set(SYSTEMLIB_SRCS)
set(SYSTEMLIB_SRCS_STR)
foreach(cls ${SYSTEMLIB_CLASSES})
  STRING(REGEX REPLACE "[ \t]*#.*" "" cls "${cls}")
  if (NOT "${cls}" STREQUAL "")
    list(APPEND SYSTEMLIB_SRCS "../../${cls}")
    set(
      SYSTEMLIB_SRCS_STR
      "${SYSTEMLIB_SRCS_STR} ${CMAKE_CURRENT_SOURCE_DIR}/../../${cls}"
    )
  endif()
endforeach()

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/systemlib.php
  DEPENDS "php.txt" ${SYSTEMLIB_SRCS}
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/make_systemlib.sh"
          "${CMAKE_CURRENT_BINARY_DIR}/systemlib.php"
          "${SYSTEMLIB_SRCS_STR}"
  COMMENT "Generating systemlib.php")

add_custom_target(
  systemlib
  DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/systemlib.php
  generated_systemlib
)
