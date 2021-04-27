GET_DIRECTORY_PROPERTY(SOURCE_INCLUDE_DIRS INCLUDE_DIRECTORIES)

# Ensure that we pick up the includes for hhvm and folly
get_property(HHVM_TARGET_INCLUDE_DIRS TARGET hhvm PROPERTY INCLUDE_DIRECTORIES)
get_property(FOLLY_TARGET_INCLUDE_DIRS TARGET folly PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
list(APPEND SOURCE_INCLUDE_DIRS ${HHVM_TARGET_INCLUDE_DIRS})
list(APPEND SOURCE_INCLUDE_DIRS ${FOLLY_TARGET_INCLUDE_DIRS})
list(REMOVE_DUPLICATES SOURCE_INCLUDE_DIRS)

set(HHVM_INCLUDE_DIRS "${CMAKE_INSTALL_FULL_INCLUDEDIR}")

foreach(dir ${SOURCE_INCLUDE_DIRS})
  if(dir STREQUAL "${HPHP_HOME}/third-party/folly")
    # omit: PREFIX/third-party is a suitable path
  elseif(dir STREQUAL "${HPHP_HOME}/third-party/boost")
      # omit: PREFIX/third-party is a suitable path
  elseif(dir STREQUAL "${HPHP_HOME}/third-party/double-conversion/src")
    # omit: PREFIX/third-party is a suitable path
  elseif(dir STREQUAL "${HPHP_HOME}")
    # omit: base dir should not be included
  else()
    # map HPHP_HOME/hphp to CMAKE_INSTALL_FULL_INCLUDEDIR/hphp
    string(REPLACE
      "${HPHP_HOME}/hphp" # match string
      "${CMAKE_INSTALL_FULL_INCLUDEDIR}/hphp" # replace string
      dir # output variable
      ${dir}) # input

    # map HPHP_HOME/third-party to CMAKE_INSTALL_FULL_INCLUDEDIR/hphp/third-party
    string(REPLACE
      "${HPHP_HOME}/third-party" # match string
      "${CMAKE_INSTALL_FULL_INCLUDEDIR}/hphp/third-party" # replace string
      dir # output variable
      ${dir}) # input

    list(APPEND HHVM_INCLUDE_DIRS ${dir})
  endif()
endforeach()

GET_DIRECTORY_PROPERTY(HHVM_DEFINITIONS COMPILE_DEFINITIONS)

# Ensure that we pick up the definitions for hhvm and folly
get_property(HHVM_TARGET_DEFINES TARGET hhvm PROPERTY COMPILE_DEFINITIONS)
get_property(FOLLY_TARGET_DEFINES TARGET folly PROPERTY INTERFACE_COMPILE_DEFINITIONS)
list(APPEND HHVM_DEFINITIONS ${HHVM_TARGET_DEFINES})
list(APPEND HHVM_DEFINITIONS ${FOLLY_TARGET_DEFINES})
list(REMOVE_DUPLICATES HHVM_DEFINITIONS)

file(READ "${HPHP_HOME}/hphp/runtime/version.h" VERSION_H_CONTENTS)
if(VERSION_H_CONTENTS MATCHES "# define HHVM_VERSION_MAJOR +([0-9]+)")
  set(HHVM_VERSION_MAJOR ${CMAKE_MATCH_1})
else()
  message(FATAL_ERROR "Unable to determine API version")
endif()
if(VERSION_H_CONTENTS MATCHES "# define HHVM_VERSION_MINOR +([0-9]+)")
  set(HHVM_VERSION_MINOR ${CMAKE_MATCH_1})
else()
  message(FATAL_ERROR "Unable to determine API version")
endif()
math(EXPR HHVM_VERSION_BRANCH "((${HHVM_VERSION_MAJOR} * 10000)|(${HHVM_VERSION_MINOR} * 100))")

configure_file(hphpize.cmake.in hphpize.cmake ESCAPE_QUOTES @ONLY)
configure_file(hphpize.in hphpize ESCAPE_QUOTES @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/hphpize
  DESTINATION bin
  COMPONENT dev)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hphpize.cmake
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/hhvm/hphpize"
  COMPONENT dev)
install(FILES ${HPHP_HOME}/hphp/test/run
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/hhvm/hphpize"
  COMPONENT dev)
