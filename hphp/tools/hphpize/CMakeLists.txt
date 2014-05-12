GET_DIRECTORY_PROPERTY(SOURCE_INCLUDE_DIRS INCLUDE_DIRECTORIES)

set(HHVM_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")

foreach(dir ${SOURCE_INCLUDE_DIRS})
  if(dir STREQUAL "${HPHP_HOME}/third-party/folly")
    # omit: PREFIX/third-party is a suitable path
  elseif(dir STREQUAL "${HPHP_HOME}/third-party/double-conversion/src")
    # omit: PREFIX/third-party is a suitable path
  else()
    # map HPHP_HOME/hphp to PREFIX/include/hphp
    string(REPLACE
      "${HPHP_HOME}/hphp" # match string
      "${CMAKE_INSTALL_PREFIX}/include/hphp" # replace string
      dir # output variable
      ${dir}) # input

    # map HPHP_HOME/third-party to PREFIX/include/hphp/third-party
    string(REPLACE
      "${HPHP_HOME}/third-party" # match string
      "${CMAKE_INSTALL_PREFIX}/include/hphp/third-party" # replace string
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
install(PROGRAMS hphpize
  DESTINATION bin
  COMPONENT dev)
install(FILES hphpize.cmake
  DESTINATION "lib/hphp/hphpize"
  COMPONENT dev)
