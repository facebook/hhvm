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

get_target_property(OPAM_EXECUTABLE opam IMPORTED_LOCATION)

add_custom_command(
  OUTPUT "${OPAM_STAMP_FILE}"
  DEPENDS opam opam_setup.sh
  COMMAND
    ${CMAKE_CURRENT_SOURCE_DIR}/opam_setup.sh
    "${OPAM_EXECUTABLE}"
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
  get_target_property(LZ4_INCLUDE_DIRS lz4 INTERFACE_INCLUDE_DIRECTORIES)
  list(APPEND extra_include_paths ${LZ4_INCLUDE_DIRS})
  # If LZ4_FOUND is false either we didn't find lz4 or we found it but it's the
  # wrong version.  We can't just add the new path and a native_lib because we
  # can't control the order (and -l won't accept the raw path to the lib).  By
  # doing it this way we specify the path explicitly.
  get_target_property(LZ4_LIBS lz4 INTERFACE_LINK_LIBRARIES)
  list(APPEND extra_link_opts ${LZ4_LIBS})
endif()

get_target_property(ZSTD_INCLUDE_DIRS zstd INTERFACE_INCLUDE_DIRECTORIES)
list(APPEND extra_include_paths ${ZSTD_INCLUDE_DIRS})
get_target_property(ZSTD_LIBS zstd INTERFACE_LINK_LIBRARIES)
list(APPEND extra_link_opts ${ZSTD_LIBS})

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

  add_dependencies(${name} ocaml rustc cargo zstd)
  if (NOT SKIP_OPAM)
    add_dependencies(${name} opam_setup)
  endif()
endfunction()

invoke_dune(hack_dune_debug debug)
invoke_dune(hack_dune_test test)
invoke_dune(hack_dune all)

set(CARGO_BUILD "${CMAKE_SOURCE_DIR}/hphp/hack/scripts/build_rust_to_ocaml.sh")

add_custom_target(
  hack_ffi
  COMMAND
   . "${CMAKE_CURRENT_BINARY_DIR}/dev_env.sh" &&
    ${CARGO_BUILD} hack_parser_ffi rust_parser_ffi &&
    ${CARGO_BUILD} rust_facts_ffi rust_facts_ffi &&
    ${CARGO_BUILD} compile_ffi compile_ffi
  COMMENT "Compiling Rust FFI"
)

add_dependencies(hack_ffi rustc cargo)
# Not a true dependency, but we need to make sure we don't have two cargo/rustc
# processes building the FFI files at the same time; this leads to intermittent
# failures, which for some reason are particularly common on our Azure MacOS
# build jobs
add_dependencies(hack_dune hack_ffi)

if (NOT LZ4_FOUND)
  add_dependencies(hack_dune lz4)
  add_dependencies(hack_dune_debug lz4)
  add_dependencies(hack_dune_test lz4)
  add_dependencies(hack_ffi lz4)
endif()

if (NOT PC_SQLITE3_FOUND)
  add_dependencies(hack_dune sqlite3)
  add_dependencies(hack_dune_debug sqlite3)
  add_dependencies(hack_dune_test sqlite3)
  add_dependencies(hack_ffi sqlite3)
endif()

# Intentionally not using `hack_dune_debug` as it generates output files of a
# different format (bytecode instead of raw executables, which is useful if
# you're working with Hack, but means that e.g. hhvm can't find
# `hh_single_compile` in the source tree. Keep it around, but require it to be
# explicitly used
add_custom_target(hack ALL DEPENDS hack_dune)
add_custom_target(hack_test DEPENDS hack_dune_test)

# libcompile_ffi_stubs.a, librust_facts_ffi_stubs.a, and
# librust_parser_ffi_stubs.a need to be exposed as library targets to cmake
# for other targets (e.g. hhvm) to depend on.
set(LIBCOMPILE_PATH "${CMAKE_BINARY_DIR}/hphp/hack/libcompile_ffi_stubs.a")
add_library(libcompile_ffi_stubs STATIC IMPORTED GLOBAL)
add_dependencies(libcompile_ffi_stubs hack_ffi)
set_target_properties(libcompile_ffi_stubs PROPERTIES IMPORTED_LOCATION ${LIBCOMPILE_PATH})

set(LIBRUST_PATH "${CMAKE_BINARY_DIR}/hphp/hack/librust_facts_ffi_stubs.a")
add_library(librust_facts_ffi_stubs STATIC IMPORTED GLOBAL)
add_dependencies(librust_facts_ffi_stubs hack_ffi)
set_target_properties(librust_facts_ffi_stubs PROPERTIES IMPORTED_LOCATION ${LIBRUST_PATH})

set(LIBRUST_PARSER_PATH "${CMAKE_BINARY_DIR}/hphp/hack/librust_parser_ffi_stubs.a")
add_library(librust_parser_ffi_stubs STATIC IMPORTED GLOBAL)
add_dependencies(librust_parser_ffi_stubs hack_ffi)
set_target_properties(librust_parser_ffi_stubs PROPERTIES IMPORTED_LOCATION ${LIBRUST_PARSER_PATH})

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
