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

# Some libraries may need to be built, instead
# of using the system version. Passing
# through CFLAGS isn't great, but works
unset(extra_cc_flags)
unset(extra_cc_lib_flags)

if (NOT USE_GLOBAL_LIBS)
  if (HACK_ONLY_BUILD)
    # Woah - this is special. They're running cmake in the hack directory
    # itself so we don't have our usual set up.
    get_filename_component(HPHP_HOME "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
    set(TP_DIR "${HPHP_HOME}/third-party")
    set(TP_BUILD_DIR ${TP_DIR})

    if(NOT EXISTS "${TP_DIR}/CMakeLists.txt")
      message(FATAL_ERROR
        "${TP_DIR}/CMakeLists.txt missing. Try updating your submodule with:\n"
        "    rm -r ${TP_DIR}\n"
        "    git submodule update --init --recursive\n"
        )
    endif()
  endif()

  find_package(LZ4)
  find_package(LibElf)

  if (HACK_ONLY_BUILD AND NOT LZ4_FOUND)
    add_subdirectory("${TP_DIR}/lz4" "${TP_DIR}/lz4" EXCLUDE_FROM_ALL)
  endif()

  if (HACK_ONLY_BUILD)
    add_subdirectory("${TP_DIR}/ocaml" "${TP_DIR}/ocaml" EXCLUDE_FROM_ALL)
  endif()

  if (HACK_ONLY_BUILD AND NOT PC_SQLITE3_FOUND)
    add_subdirectory(
      "${TP_DIR}/libsqlite3"
      "${TP_DIR}/libsqlite3"
      EXCLUDE_FROM_ALL)
  endif()

  # Allows '#include "hphp/path/to/library/"' paths to start from hphp
  # project directory  which is consistent with fbmake's include paths.

  IF(HPHP_HOME)
    list(APPEND extra_cc_flags "-I${HPHP_HOME}")
  ELSE()
    list(APPEND extra_cc_flags "-I$${CMAKE_CURRENT_SOURCE_DIR}/../..")
  ENDIF()

  list(APPEND extra_cc_flags -pthread)

  if(LZ4_FOUND)
    list(APPEND extra_cc_flags "-I${LZ4_INCLUDE_DIR}")
    get_filename_component(pth ${LZ4_LIBRARY} PATH)
    list(APPEND extra_cc_lib_flags "-L${pth}")
    list(APPEND extra_cc_lib_flags "-llz4")
  else()
    list(APPEND extra_cc_flags "-I$${TP_DIR}/lz4/src/lib")
    # If LZ4_FOUND is false either we didn't find lz4 or we found it but it's the
    # wrong version.  We can't just add the new path and a native_lib because we
    # can't control the order (and -l won't accept the raw path to the lib).  By
    # doing it this way we specify the path explicitly.
    list(APPEND extra_cc_lib_flags "$<TARGET_LINKER_FILE:lz4>")
  endif()

  if(PC_SQLITE3_FOUND)
    list(APPEND extra_cc_flags "-I$${LIBSQLITE3_INCLUDE_DIR}")
    get_filename_component(pth ${LIBSQLITE3_LIBRARY} PATH)
    list(APPEND extra_cc_lib_flags "-L${pth}")
  else()
    list(APPEND extra_cc_flags "-I$${TP_DIR}/libsqlite3")
    list(APPEND extra_cc_lib_flags "-L${TP_BUILD_DIR}/libsqlite3")
  endif()
  list(APPEND extra_cc_lib_flags "-lsqlite3")

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

configure_file (
  "src/heap/config/discover.ml.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/heap/config/discover.ml"
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
