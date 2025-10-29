set(CXX_SOURCES "systemlib.cpp")

add_library(hphp_system STATIC ${CXX_SOURCES})
target_link_libraries(hphp_system hphp_util proxygen hhvm_base_headers)

auto_sources(files "*.h" "${CMAKE_CURRENT_SOURCE_DIR}")
HHVM_PUBLIC_HEADERS(system ${files})

FILE(STRINGS ${CMAKE_SOURCE_DIR}/hphp/runtime/ext/core/php.txt SYSTEMLIB_CLASSES)
set(SYSTEMLIB_SRCS)

foreach(cls ${SYSTEMLIB_CLASSES})
  STRING(REGEX REPLACE "[ \t]*#.*" "" cls "${cls}")
  if (NOT "${cls}" STREQUAL "")
    list(APPEND SYSTEMLIB_SRCS "${CMAKE_SOURCE_DIR}/${cls}")
  endif()
endforeach()

set(CORE_SYSTEMLIB_FILE "${CMAKE_SOURCE_DIR}/hphp/runtime/ext/core/ext_core.php")

add_custom_command(
  OUTPUT ${CORE_SYSTEMLIB_FILE}
  DEPENDS "php.txt" ${SYSTEMLIB_SRCS}
  COMMAND "${CMAKE_SOURCE_DIR}/hphp/runtime/ext/core/make_systemlib.sh"
          "${CMAKE_SOURCE_DIR}/hphp/runtime/ext/core"
          "ext_core.php"
          ${SYSTEMLIB_SRCS}
  COMMENT "Generating systemlib.php"
  VERBATIM
  )

add_custom_target(
  systemlib
  DEPENDS
  ${CORE_SYSTEMLIB_FILE}
  generated_systemlib
)
