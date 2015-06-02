set(OCAMLC_FOUND FALSE)
set(OCAMLC_OPT_SUFFIX "")

find_program(OCAMLC_EXECUTABLE ocamlc DOC "path to ocamlc")
mark_as_advanced(OCAMLC_EXECUTABLE)

if(OCAMLC_EXECUTABLE)
  message(STATUS "Found ocamlc: ${OCAMLC_EXECUTABLE}")
  execute_process(COMMAND ${OCAMLC_EXECUTABLE} -version
    OUTPUT_VARIABLE OCAMLC_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if ("${OCAMLC_VERSION}" VERSION_LESS "4.01")
    message(FATAL_ERROR "OCaml version ${OCAMLC_VERSION} is too old "
      "to build the Hack typechecker, need at least 4.01. Directions "
      "at https://github.com/facebook/hhvm/wiki/Building-and-Installing-HHVM "
      "may have instructions how to get a newer version for your distro.")
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
  message(FATAL_ERROR "OCaml not found, can not build Hack typechecker")
endif()
