CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../CMake" ${CMAKE_MODULE_PATH})

option(BUILD_HACK "True if we should build the Hack typechecker." ON)

if (NOT BUILD_HACK)
  message(STATUS "Skipping hack")
  return()
endif()

message(STATUS "Building hack")

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_LIST_DIR}")
  set(HACK_ONLY_BUILD TRUE)
endif()

# Xcode/Ninja generators undefined MAKE
if(NOT MAKE)
  set(MAKE make)
endif()

add_custom_target(
  opam.stamp
  ALL
  COMMAND
    ${CMAKE_CURRENT_SOURCE_DIR}/opam_setup.sh
    "${OCAML_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
add_dependencies(opam.stamp ocaml)

if (SKIP_OPAM)
  set(OPAMROOT "~/.opam")
else ()
  set(OPAMROOT "${CMAKE_CURRENT_SOURCE_DIR}/src/_build/.opam")
endif()

add_custom_target(
  hack
  ALL
  COMMAND
        export OPAMROOT=${OPAMROOT} &&
        export OCAMLFIND_COMMANDS="-ocamlc=${OCAMLC_EXECUTABLE} -ocamlopt=${OCAMLOPT_EXECUTABLE}" &&
        PATH="${TP_BUILD_DIR}/ocaml/build/bin:$(PATH)"
        opam config exec --
        $(MAKE)
        BYTECODE="${EMIT_OCAML_BYTECODE}"
        OCAML="${OCAML_EXECUTABLE}"
        OCAMLC="${OCAMLC_EXECUTABLE}"
        OCAMLBUILD="${OCAMLBUILD_EXECUTABLE}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
add_dependencies(hack ocaml)
if (NOT SKIP_OPAM)
  add_dependencies(hack opam.stamp)
endif()

add_custom_target(
  hack_test
  COMMAND
        export OPAMROOT=${OPAMROOT} &&
        export OCAMLFIND_COMMANDS="-ocamlc=${OCAMLC_EXECUTABLE} -ocamlopt=${OCAMLOPT_EXECUTABLE}" &&
        PATH="${TP_BUILD_DIR}/ocaml/build/bin:$(PATH)"
        opam config exec --
        $(MAKE) test
        BYTECODE="${EMIT_OCAML_BYTECODE}"
        OCAML="${OCAML_EXECUTABLE}"
        OCAMLC="${OCAMLC_EXECUTABLE}"
        OCAMLBUILD="${OCAMLBUILD_EXECUTABLE}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
add_dependencies(hack_test ocaml opam.stamp)

if(NOT LZ4_FOUND)
  # if the system does not have lz4, make sure that the one in public_tl
  # gets built
  add_dependencies(hack lz4)
  add_dependencies(hack_test lz4)
endif()

if(NOT PC_SQLITE3_FOUND)
  # if the system does not have sqlite3, make sure that the one in public_tl
  # gets built
  add_dependencies(hack sqlite3)
  add_dependencies(hack_test sqlite3)
endif()

configure_file(
  "src/options/buildOptions.ml.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/options/buildOptions.ml"
  ESCAPE_QUOTES
)

configure_file(
  "src/fsnotify/dune.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/fsnotify/dune"
)

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hh_client
  DESTINATION bin
  COMPONENT dev)

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hh_server
  DESTINATION bin
  COMPONENT dev)

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hackfmt
  DESTINATION bin
  COMPONENT dev)

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hh_parse
  DESTINATION bin
  COMPONENT dev)

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hh_single_compile
  DESTINATION bin
  COMPONENT dev)
