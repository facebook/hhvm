set(OCAMLC_FOUND FALSE)
set(OCAMLC_OPT_SUFFIX "")

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  message(STATUS "Not on Linux, cannot build Hack typechecker")
  return()
endif()

find_program(OCAMLC_EXECUTABLE ocamlc DOC "path to ocamlc")
mark_as_advanced(OCAMLC_EXECUTABLE)

if(OCAMLC_EXECUTABLE)
  message(STATUS "Found ocamlc: ${OCAMLC_EXECUTABLE}")
  execute_process(COMMAND ${OCAMLC_EXECUTABLE} -version
    OUTPUT_VARIABLE OCAMLC_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if ("${OCAMLC_VERSION}" VERSION_LESS "3.12")
    message(STATUS "OCaml version ${OCAMLC_VERSION} is too old "
      "to build the Hack typechecker, need at least 3.12")
  else()
    set(OCAMLC_FOUND TRUE)

    find_program(OCAMLC_OPT_EXECUTABLE ocamlc.opt DOC "path to ocamlc.opt")
    mark_as_advanced(OCAMLC_OPT_EXECUTABLE)

    if (OCAMLC_OPT_EXECUTABLE)
      message(STATUS "Found ocamlc.opt: ${OCAMLC_OPT_EXECUTABLE}")
      set(OCAMLC_OPT_SUFFIX ".opt")
    else()
      message(STATUS "Could not find ocamlc.opt, "
        "Hack typechecker build will be slow")
    endif()
  endif()
else()
  message(STATUS "OCaml not found, will not build Hack typechecker")
endif()
