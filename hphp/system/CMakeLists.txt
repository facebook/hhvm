set(CXX_SOURCES "systemlib.cpp")

add_library(hphp_system STATIC ${CXX_SOURCES})
target_link_libraries(hphp_system hphp_util proxygen hhvm_base_headers)

auto_sources(files "*.h" "${CMAKE_CURRENT_SOURCE_DIR}")
HHVM_PUBLIC_HEADERS(system ${files})

set(SYSTEMLIB_LIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/php.txt")
if(NOT EXISTS "${SYSTEMLIB_LIST_FILE}")
  set(SYSTEMLIB_LIST_FILE "${CMAKE_SOURCE_DIR}/hphp/runtime/ext/core/php.txt")
endif()

FILE(STRINGS "${SYSTEMLIB_LIST_FILE}" SYSTEMLIB_CLASSES)
set(SYSTEMLIB_SRCS_ABS)
foreach(cls ${SYSTEMLIB_CLASSES})
  STRING(REGEX REPLACE "[ \t]*#.*" "" cls "${cls}")
  if (NOT "${cls}" STREQUAL "")
    set(systemlib_src "${cls}")
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${systemlib_src}")
      STRING(
        REGEX REPLACE "^hphp/system/php/" "hphp/runtime/ext/core/php/"
        systemlib_src "${systemlib_src}"
      )
    endif()
    list(APPEND SYSTEMLIB_SRCS_ABS "${CMAKE_SOURCE_DIR}/${systemlib_src}")
  endif()
endforeach()

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/systemlib.php
  DEPENDS
    "${SYSTEMLIB_LIST_FILE}"
    ${SYSTEMLIB_SRCS_ABS}
    "${CMAKE_SOURCE_DIR}/hphp/runtime/ext/core/make_systemlib.sh"
  COMMAND
    "${CMAKE_SOURCE_DIR}/hphp/runtime/ext/core/make_systemlib.sh"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "systemlib.php"
    ${SYSTEMLIB_SRCS_ABS}
  COMMENT "Generating systemlib.php")

add_custom_target(
  systemlib
  DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/systemlib.php
  generated_systemlib
)
