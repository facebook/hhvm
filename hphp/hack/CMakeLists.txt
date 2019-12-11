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

if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  set(DUNE_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/_build")
  set(OPAM_STAMP_FILE "_build/opam.stamp")
else()
  set(DUNE_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")
  set(OPAM_STAMP_FILE "opam.stamp")
endif()
set(HACK_BUILD_ROOT "${DUNE_BUILD_DIR}/default")

add_custom_command(
  OUTPUT "${OPAM_STAMP_FILE}"
  DEPENDS ocaml opam_setup.sh
  COMMAND
    ${CMAKE_CURRENT_SOURCE_DIR}/opam_setup.sh
    "${OCAML_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${DUNE_BUILD_DIR}"
    && cmake -E touch "${OPAM_STAMP_FILE}"
)
add_custom_target(opam_setup DEPENDS "${OPAM_STAMP_FILE}" opam_setup.sh)

if (SKIP_OPAM)
  set(OPAMROOT "~/.opam")
else ()
  set(OPAMROOT "${DUNE_BUILD_DIR}/opam")
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
  get_target_property(LZ4_DIR lz4 BINARY_DIR)
  list(APPEND extra_link_opts "${LZ4_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}lz4${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

if(PC_SQLITE3_FOUND)
  list(APPEND extra_include_paths ${LIBSQLITE3_INCLUDE_DIR})
  get_filename_component(pth ${LIBSQLITE3_LIBRARY} DIRECTORY)
  list(APPEND extra_lib_paths ${pth})
  list(APPEND extra_native_libraries "sqlite3")
else()
  list(APPEND extra_include_paths "${TP_DIR}/libsqlite3")
  get_target_property(SQLITE_DIR sqlite3 BINARY_DIR)
  list(APPEND extra_link_opts "${SQLITE_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}sqlite3${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

get_target_property(RUSTC_EXE rustc LOCATION)
get_target_property(CARGO_EXE cargo LOCATION)
get_filename_component(RUSTC_BIN_DIR "${RUSTC_EXE}" DIRECTORY)
get_filename_component(CARGO_BIN_DIR "${CARGO_EXE}" DIRECTORY)

function(invoke_dune name target)
  add_custom_target(
      ${name}
      COMMAND
        . "${CMAKE_CURRENT_BINARY_DIR}/dev_env.sh" &&
        opam config exec --
        $(MAKE) --makefile=Makefile.dune ${target}
        BYTECODE="${EMIT_OCAML_BYTECODE}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

  add_dependencies(${name} ocaml rustc cargo)
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

# Intentionally not using `hack_dune_debug` as it generates output files of a
# different format (bytecode instead of raw executables, which is useful if
# you're working with Hack, but means that e.g. hhvm can't find
# `hh_single_compile` in the source tree. Keep it around, but require it to be
# explicitly used
add_custom_target(hack ALL DEPENDS hack_dune)
add_custom_target(hack_test DEPENDS hack_dune_test)

configure_file(dev_env.sh.in dev_env.sh ESCAPE_QUOTES @ONLY)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/hh_client
  DESTINATION bin
  COMPONENT dev)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/hh_server
  DESTINATION bin
  COMPONENT dev)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/hackfmt
  DESTINATION bin
  COMPONENT dev)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/hh_parse
  DESTINATION bin
  COMPONENT dev)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/hh_single_compile
  DESTINATION bin
  COMPONENT dev)
