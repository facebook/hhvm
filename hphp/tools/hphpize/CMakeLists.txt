GET_DIRECTORY_PROPERTY(SOURCE_INCLUDE_DIRS INCLUDE_DIRECTORIES)

set(HHVM_INCLUDE_DIRS "${CMAKE_INSTALL_FULL_INCLUDEDIR}")

foreach(dir ${SOURCE_INCLUDE_DIRS})
  if(dir STREQUAL "${HPHP_HOME}/third-party/folly")
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

file(READ "${HPHP_HOME}/hphp/runtime/ext/extension.h" EXTENSION_H_CONTENTS)
if(EXTENSION_H_CONTENTS MATCHES "#define HHVM_API_VERSION +([0-9]+)")
  set(HHVM_API_VERSION ${CMAKE_MATCH_1})
else()
  message(FATAL_ERROR "Unable to determine API version")
endif()

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
