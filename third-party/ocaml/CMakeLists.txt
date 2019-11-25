# Either we couldn't find a reasonable version of ocamlc or it was too old.
message(STATUS "Building ocaml from third-party")

set(OCAML_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/build")
set(OCAML_SRC "${CMAKE_CURRENT_BINARY_DIR}/ocaml_src.stamp")
set(OCAML "${OCAML_PREFIX}/bin/ocaml")
set(OCAMLC "${OCAML_PREFIX}/bin/ocamlc.opt")
set(OCAMLOPT "${OCAML_PREFIX}/bin/ocamlopt.opt")
set(OPAM_SRC "${CMAKE_CURRENT_BINARY_DIR}/opam_src.stamp")
set(OPAM "${OCAML_PREFIX}/bin/opam")
set(HACK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../hphp/hack")

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  add_custom_command(
    OUTPUT "${OCAML_SRC}"
    COMMAND cmake -E touch "${OCAML_SRC}"
  )
else()
  # OCaml 4.05 has a custom configure script that doesn't support out of source
  # builds; 4.07 uses autotools so hopefully we don't need this when we next
  # update ocaml
  add_custom_command(
    OUTPUT "${OCAML_SRC}"
    COMMAND
      cmake -E copy_directory
      "${CMAKE_CURRENT_SOURCE_DIR}/src"
      "${CMAKE_CURRENT_BINARY_DIR}/src"
      && cmake -E touch "${OCAML_SRC}"
  )
endif()


# TODO: what about dependencies so we rebuild ocaml when it changes?
# NOTE: The weirdness building world.opt twice is because (at least on arm64)
# ocaml (4.03) seems to have parallel build dependency problems.
add_custom_command(
  OUTPUT ${OCAML} ${OCAMLC} ${OCAMLOPT}
  COMMAND ./configure -prefix "${OCAML_PREFIX}" -no-graph
  COMMAND \$\(MAKE\) -k world.opt || true
  COMMAND \$\(MAKE\) -j1 world.opt
  COMMAND \$\(MAKE\) install
  DEPENDS "${OCAML_SRC}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/src"
  COMMENT "Compiling ocaml")

add_custom_command(
  OUTPUT "${OPAM_SRC}"
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/fetch_and_setup_src.sh" "${HACK_DIR}"
  DEPENDS ./fetch_and_setup_src.sh
  COMMENT "Fetch opam full source code from internet")

# `./configure` instead of `${CMAKE_CURRENT_SOURCE_DIR}/configure` because
# opam's source is extracted in the build directory
add_custom_command(
  OUTPUT ${OPAM}
  COMMAND /bin/bash -c "\
    PATH='${OCAML_PREFIX}/bin:$ENV{PATH}'\
    ./configure \
    --prefix='${OCAML_PREFIX}' \
  "
  COMMAND /bin/bash -c "\
    PATH='${OCAML_PREFIX}/bin:$ENV{PATH}' \
    \$\(MAKE\) -j1 lib-ext \
  "
  COMMAND /bin/bash -c "\
    PATH='${OCAML_PREFIX}/bin:$ENV{PATH}' \
    \$\(MAKE\) -j1 \
  "
  COMMAND /bin/bash -c "\
    PATH='${OCAML_PREFIX}/bin:$ENV{PATH}' \
    \$\(MAKE\) install \
  "
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/opam"
  DEPENDS ${OPAM_SRC} ${OCAMLC}
  COMMENT "Compiling opam")

set(OCAMLC_FOUND TRUE)

add_custom_target(ocaml
  DEPENDS ${OCAMLC} ${OCAMLOPT} ${OPAM})

set(OCAML_EXECUTABLE ${OCAML} CACHE FILEPATH "path to ocaml" FORCE)
mark_as_advanced(OCAML_EXECUTABLE)
set(OCAMLC_EXECUTABLE ${OCAMLC} CACHE FILEPATH "path to ocamlc" FORCE)
mark_as_advanced(OCAMLC_EXECUTABLE)
set(OCAMLOPT_EXECUTABLE ${OCAMLOPT} CACHE FILEPATH "path to ocamlopt" FORCE)
mark_as_advanced(OCAMLOPT_EXECUTABLE)
set(OPAM_EXECUTABLE ${OPAM} CACHE FILEPATH "path to opam" FORCE)
mark_as_advanced(OPAM_EXECUTABLE)
