option(BUILD_HACK "True if we should build the Hack typechecker." ON)

if (NOT BUILD_HACK)
  message(STATUS "Skipping hack")
  return()
endif()

message(STATUS "Building hack")

find_package(LZ4)
find_package(LibElf)

# native_libraries: values for `-l` flags
# lib_paths: values for `-L` flags (directories)
# extra_link_opts: opaque options passed to the linker
#
# We need extra_link_opts for:
# - static libraries
# - anything built from third-party: cmake gives us the link flags
unset(extra_include_paths)
unset(extra_native_libraries)
unset(extra_lib_paths)
unset(extra_link_opts)
unset(extra_cc_flags)

# Allows '#include "hphp/path/to/library/"' paths to start from hphp
# project directory  which is consistent with fbmake's include paths.
list(APPEND extra_include_paths ${HPHP_HOME})
list(APPEND extra_cc_flags -pthread)

# Xcode/Ninja generators undefined MAKE
if(NOT MAKE)
  set(MAKE make)
endif()

add_custom_command(
  OUTPUT opam.stamp
  DEPENDS ocaml opam_setup.sh
  COMMAND
    ${CMAKE_CURRENT_SOURCE_DIR}/opam_setup.sh
    "${OCAML_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}"
    && cmake -E touch opam.stamp
)
add_custom_target(opam_setup DEPENDS opam.stamp opam_setup.sh)

if (SKIP_OPAM)
  set(OPAMROOT "~/.opam")
else ()
  set(OPAMROOT "${CMAKE_CURRENT_SOURCE_DIR}/_build/opam")
endif()

if(LZ4_FOUND)
  list(APPEND extra_include_paths ${LZ4_INCLUDE_DIR})
  get_filename_component(pth ${LZ4_LIBRARY} DIRECTORY)
  list(APPEND extra_lib_paths ${pth})
  list(APPEND extra_native_libraries "lz4")
else()
  list(APPEND extra_include_paths "${TP_DIR}/lz4/src/lib")
  # If LZ4_FOUND is false either we didn't find lz4 or we found it but it's the
  # wrong version.  We can't just add the new path and a native_lib because we
  # can't control the order (and -l won't accept the raw path to the lib).  By
  # doing it this way we specify the path explicitly.
  list(APPEND extra_link_opts "$<TARGET_LINKER_FILE:lz4>")
endif()

if(PC_SQLITE3_FOUND)
  list(APPEND extra_include_paths ${LIBSQLITE3_INCLUDE_DIR})
  get_filename_component(pth ${LIBSQLITE3_LIBRARY} DIRECTORY)
  list(APPEND extra_lib_paths ${pth})
  list(APPEND extra_native_libraries "sqlite3")
else()
  list(APPEND extra_include_paths "${TP_DIR}/libsqlite3")
  list(APPEND extra_link_opts "$<TARGET_LINKER_FILE:sqlite3>")
endif()

# ARGV2 (optional unnamed argument) is expected to only be set to ALL
function(invoke_dune name target)
  add_custom_target(
      ${name}
      ${ARGV2}
      COMMAND
        export OPAMROOT=${OPAMROOT} &&
        export OCAMLFIND_COMMANDS="-ocamlc=${OCAMLC_EXECUTABLE} -ocamlopt=${OCAMLOPT_EXECUTABLE}" &&
        export PYTHONPATH="${HPHP_HOME}" && # needed for verify.py for `hack_dune_test`
        PATH="${TP_BUILD_DIR}/ocaml/build/bin:$(PATH)"
        opam config exec --
        $(MAKE) --makefile=Makefile.dune ${target}
        CMAKE_INSTALL_FULL_SYSCONFDIR="${CMAKE_INSTALL_FULL_SYSCONFDIR}"
        CMAKE_INSTALL_FULL_BINDIR="${CMAKE_INSTALL_FULL_BINDIR}"
        EXTRA_INCLUDE_PATHS="${extra_include_paths}"
        EXTRA_LINK_OPTS="${extra_link_opts}"
        EXTRA_LIB_PATHS="${extra_lib_paths}"
        EXTRA_NATIVE_LIBRARIES="${extra_native_libraries}"
        BYTECODE="${EMIT_OCAML_BYTECODE}"
        OCAML="${OCAML_EXECUTABLE}"
        OCAMLC="${OCAMLC_EXECUTABLE}"
        OCAMLBUILD="${OCAMLBUILD_EXECUTABLE}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

  add_dependencies(${name} ocaml)
  if (NOT SKIP_OPAM)
    add_dependencies(${name} opam_setup)
  endif()
endfunction()

invoke_dune(hack_dune_debug debug)
invoke_dune(hack_dune_test test)
invoke_dune(hack_dune all)

if (NOT LZ4_FOUND)
  add_dependencies(hack_dune lz4)
  add_dependencies(hack_dune_debug lz4)
  add_dependencies(hack_dune_test lz4)
endif()

if (NOT PC_SQLITE3_FOUND)
  add_dependencies(hack_dune sqlite3)
  add_dependencies(hack_dune_debug sqlite3)
  add_dependencies(hack_dune_test sqlite3)
endif()

get_property(DEBUG_CONFIGURATIONS GLOBAL PROPERTY DEBUG_CONFIGURATIONS)
list(FIND DEBUG_CONFIGURATIONS "${CMAKE_BUILD_TYPE}" DEBUG_IDX)
if (DEBUG_IDX EQUAL -1)
  add_custom_target(hack ALL DEPENDS hack_dune)
else()
  add_custom_target(hack ALL DEPENDS hack_dune_debug)
endif()
add_custom_target(hack_test DEPENDS hack_dune_test)

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
