set(EXT_CORE_SYSTEMLIB_OUT "${CMAKE_BINARY_DIR}/hphp/runtime/ext/core/ext_core.php")

file(STRINGS "${HRE_CURRENT_EXT_PATH}/php.txt" EXT_CORE_SYSTEMLIB_CLASSES)
set(EXT_CORE_SYSTEMLIB_SRCS_ABS)
foreach(cls ${EXT_CORE_SYSTEMLIB_CLASSES})
  string(REGEX REPLACE "[ \t]*#.*" "" cls "${cls}")
  if(NOT "${cls}" STREQUAL "")
    set(ext_core_systemlib_src "${cls}")
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${ext_core_systemlib_src}")
      string(
        REGEX REPLACE
        "^hphp/system/php/"
        "hphp/runtime/ext/core/php/"
        ext_core_systemlib_src
        "${ext_core_systemlib_src}"
      )
    endif()
    list(APPEND EXT_CORE_SYSTEMLIB_SRCS_ABS "${CMAKE_SOURCE_DIR}/${ext_core_systemlib_src}")
  endif()
endforeach()

add_custom_command(
  OUTPUT "${EXT_CORE_SYSTEMLIB_OUT}"
  DEPENDS
    "${HRE_CURRENT_EXT_PATH}/php.txt"
    "${HRE_CURRENT_EXT_PATH}/make_systemlib.sh"
    ${EXT_CORE_SYSTEMLIB_SRCS_ABS}
  COMMAND
    "${HRE_CURRENT_EXT_PATH}/make_systemlib.sh"
    "${CMAKE_BINARY_DIR}/hphp/runtime/ext/core"
    "ext_core.php"
    ${EXT_CORE_SYSTEMLIB_SRCS_ABS}
  COMMENT "Generating ext_core.php"
  VERBATIM
)

add_custom_target(
  ext_core_generated_systemlib
  DEPENDS
    "${EXT_CORE_SYSTEMLIB_OUT}"
)
add_dependencies(generated_systemlib ext_core_generated_systemlib)

HHVM_DEFINE_EXTENSION("core" REQUIRED
  SOURCES
    ext_core.cpp
    ext_core_closure.cpp
  HEADERS
    ext_core.h
  SYSTEMLIB
    "${EXT_CORE_SYSTEMLIB_OUT}"
)
