option(BUILD_HACK "True if we should build the Hack typechecker." ON)

include(CMakeParseArguments)

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
  set(RUST_FFI_BUILD_ROOT "${DUNE_BUILD_DIR}/rust_ffi")
  set(CARGO_HOME "${DUNE_BUILD_DIR}/cargo_home")
else()
  set(DUNE_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")
  set(RUST_FFI_BUILD_ROOT "${CMAKE_BINARY_DIR}")
  set(CARGO_HOME "${CMAKE_CURRENT_BINARY_DIR}/cargo_home")
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

list(APPEND extra_include_paths ${LIBSQLITE3_INCLUDE_DIR})
get_filename_component(pth ${LIBSQLITE3_LIBRARY} DIRECTORY)
list(APPEND extra_lib_paths ${pth})
list(APPEND extra_native_libraries "sqlite3")

get_target_property(RUSTC_EXE rustc LOCATION)
get_target_property(CARGO_EXE cargo LOCATION)
get_filename_component(RUSTC_BIN_DIR "${RUSTC_EXE}" DIRECTORY)
get_filename_component(CARGO_BIN_DIR "${CARGO_EXE}" DIRECTORY)

function(invoke_dune name target)
  add_custom_target(
      ${name}
      COMMAND
        . "${CMAKE_CURRENT_BINARY_DIR}/dev_env.sh" &&
        opam exec --
        $(MAKE) --makefile=Makefile.dune ${target}
        BYTECODE="${EMIT_OCAML_BYTECODE}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

  add_dependencies(${name} rustc cargo zstd)
  if (NOT SKIP_OPAM)
    add_dependencies(${name} opam_setup)
  endif()
endfunction()

invoke_dune(hack_dune_debug debug)
invoke_dune(hack_dune_test test)
invoke_dune(hack_dune all)

set(INVOKE_CARGO "${CMAKE_SOURCE_DIR}/hphp/hack/scripts/invoke_cargo.sh")

if(DEFINED ENV{HACKDEBUG})
  set(PROFILE "debug")
else()
  set(PROFILE "release")
endif()
set(RUST_FFI_CARGO_TARGET_DIR "${RUST_FFI_BUILD_ROOT}/hphp/hack/rust_ffi")
set(HHVM_RUST_FFI_STATICLIB_NAME "hhvm_ffi_bridge")
set(
  HHVM_RUST_FFI_WORKSPACE_DIR
  "${RUST_FFI_BUILD_ROOT}/hphp/hack/${HHVM_RUST_FFI_STATICLIB_NAME}"
)
set(HHVM_RUST_FFI_CARGO_TOML "${HHVM_RUST_FFI_WORKSPACE_DIR}/Cargo.toml")
set(HHVM_RUST_FFI_LIB_RS "${HHVM_RUST_FFI_WORKSPACE_DIR}/lib.rs")
set(
  HHVM_RUST_FFI_STATICLIB
  "${RUST_FFI_CARGO_TARGET_DIR}/${PROFILE}/${CMAKE_STATIC_LIBRARY_PREFIX}${HHVM_RUST_FFI_STATICLIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
if(DEFINED ENV{HACKDEBUG})
  set(HHVM_RUST_FFI_PROFILE_ARGS)
else()
  set(HHVM_RUST_FFI_PROFILE_ARGS --release)
endif()
file(MAKE_DIRECTORY "${HHVM_RUST_FFI_WORKSPACE_DIR}")
set(COMPILER_FFI_CARGO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/hackc/ffi_bridge")
set(HDF_CARGO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/hdf")
set(
  HHVM_HHBC_DEFS_FFI_CARGO_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/src/hackc/hhvm_cxx/hhvm_hhbc_defs"
)
set(
  HHVM_TYPES_FFI_CARGO_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/src/hackc/hhvm_cxx/hhvm_types"
)
set(PACKAGE_FFI_CARGO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/package/ffi_bridge")
set(PARSER_FFI_CARGO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/parser/ffi_bridge")
set(
  COMPILER_FFI_EXTRA_SRC
  "${CMAKE_CURRENT_SOURCE_DIR}/src/hackc/ffi_bridge/external_decl_provider.cpp"
)
set(
  HHVM_TYPES_FFI_EXTRA_SRC
  "${CMAKE_CURRENT_SOURCE_DIR}/src/hackc/hhvm_cxx/hhvm_types/as-base-ffi.cpp"
)
set(
  HHVM_HHBC_DEFS_FFI_EXTRA_SRC
  "${HHVM_HHBC_DEFS_FFI_CARGO_PATH}/as-hhbc-ffi.cpp"
)
string(CONCAT HHVM_RUST_FFI_CARGO_TOML_CONTENT
  "[package]\n"
  "name = \"${HHVM_RUST_FFI_STATICLIB_NAME}\"\n"
  "version = \"0.0.0\"\n"
  "edition = \"2024\"\n"
  "\n"
  "[lib]\n"
  "path = \"lib.rs\"\n"
  "crate-type = [\"staticlib\"]\n"
  "\n"
  "[dependencies]\n"
  "compiler_ffi = { path = \"${COMPILER_FFI_CARGO_PATH}\" }\n"
  "hdf = { path = \"${HDF_CARGO_PATH}\" }\n"
  "hhvm_hhbc_defs_ffi = { path = \"${HHVM_HHBC_DEFS_FFI_CARGO_PATH}\" }\n"
  "hhvm_types_ffi = { path = \"${HHVM_TYPES_FFI_CARGO_PATH}\" }\n"
  "package_ffi = { path = \"${PACKAGE_FFI_CARGO_PATH}\" }\n"
  "parser_ffi = { path = \"${PARSER_FFI_CARGO_PATH}\" }\n"
)
string(CONCAT HHVM_RUST_FFI_LIB_RS_CONTENT
  "#![allow(unused_extern_crates)]\n"
  "\n"
  "// cxx expects a single Rust staticlib when the final "
  "link is done by CMake.\n"
  "extern crate compiler_ffi;\n"
  "extern crate hdf;\n"
  "extern crate hhvm_hhbc_defs_ffi;\n"
  "extern crate hhvm_types_ffi;\n"
  "extern crate package_ffi;\n"
  "extern crate parser_ffi;\n"
)
file(
  GENERATE
  OUTPUT "${HHVM_RUST_FFI_CARGO_TOML}"
  CONTENT "${HHVM_RUST_FFI_CARGO_TOML_CONTENT}"
)
file(
  GENERATE
  OUTPUT "${HHVM_RUST_FFI_LIB_RS}"
  CONTENT "${HHVM_RUST_FFI_LIB_RS_CONTENT}"
)

set(RUST_OPCODES_DIR "${CMAKE_BINARY_DIR}/hphp/hack/src/hackc")
set(RUST_OPCODES "${RUST_OPCODES_DIR}/opcodes.rs")

set(HHBC_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/src/hackc")
set(HHBC_AST_SRCS
  "${HHBC_PREFIX}/hhbc/adata.rs"
  "${HHBC_PREFIX}/hhbc/attribute.rs"
  "${HHBC_PREFIX}/hhbc/body.rs"
  "${HHBC_PREFIX}/hhbc/class.rs"
  "${HHBC_PREFIX}/hhbc/class_alias.rs"
  "${HHBC_PREFIX}/hhbc/coeffects.rs"
  "${HHBC_PREFIX}/hhbc/constant.rs"
  "${HHBC_PREFIX}/hhbc/function.rs"
  "${HHBC_PREFIX}/hhbc/id.rs"
  "${HHBC_PREFIX}/hhbc/instruct.rs"
  "${HHBC_PREFIX}/hhbc/method.rs"
  "${HHBC_PREFIX}/hhbc/module.rs"
  "${HHBC_PREFIX}/hhbc/param.rs"
  "${HHBC_PREFIX}/hhbc/pos.rs"
  "${HHBC_PREFIX}/hhbc/property.rs"
  "${HHBC_PREFIX}/hhbc/symbol_refs.rs"
  "${HHBC_PREFIX}/hhbc/types.rs"
  "${HHBC_PREFIX}/hhbc/type_const.rs"
  "${HHBC_PREFIX}/hhbc/typedef.rs"
  "${HHBC_PREFIX}/hhbc/typed_value.rs"
  "${HHBC_PREFIX}/hhbc/unit.rs"
  "${HHBC_PREFIX}/hhbc/unit_cbindgen.rs"  # Not in a crate.
  "${RUST_OPCODES}"
)

set(NAMING_SPECIAL_NAMES_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/src/naming")
set(NAMING_SPECIAL_NAMES_SRCS
   "${NAMING_SPECIAL_NAMES_PREFIX}/naming_special_names.rs"
   "${NAMING_SPECIAL_NAMES_PREFIX}/naming_special_names_ffi_cbindgen.rs"
)

set(FFI_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/src/hackc/ffi")
set(FFI_SRCS
   "${FFI_PREFIX}/lib.rs"
   "${FFI_PREFIX}/ffi_ffi_cbindgen.rs"
)

add_custom_command(
  OUTPUT ${RUST_OPCODES}
  COMMAND
    ${CMAKE_COMMAND} -E make_directory "${RUST_OPCODES_DIR}" &&
    . "${CMAKE_CURRENT_BINARY_DIR}/dev_env_rust_only.sh" &&
    ${INVOKE_CARGO} dump-opcodes dump-opcodes --exe &&
    ${INVOKE_CARGO} dump-opcodes dump-opcodes --bin dump_opcodes
      -o "${RUST_OPCODES}"
  COMMENT "Generating Rust opcodes for cbindgen"
  DEPENDS "${OPCODE_DATA}"
)

set(HHBC_AST_HEADER "${RUST_FFI_BUILD_ROOT}/hphp/hack/src/hackc/hhbc-unit.h")
set(FFI_HEADER "${RUST_FFI_BUILD_ROOT}/hphp/hack/src/hackc/ffi.h")
set(
  NAMING_SPECIAL_NAMES_HEADER
  "${RUST_FFI_BUILD_ROOT}/hphp/hack/src/naming/naming-special-names.h"
)
set(
  TYPE_CONSTRAINT_HEADER
  "${CMAKE_SOURCE_DIR}/hphp/runtime/vm/type-constraint-flags.h"
)
set(FCALL_HEADER "${CMAKE_SOURCE_DIR}/hphp/runtime/vm/fcall-args-flags.h")
set(HHBC_HEADER "${CMAKE_SOURCE_DIR}/hphp/runtime/vm/hhbc-shared.h")
set(
  ITER_ARGS_FLAGS_HEADER
  "${CMAKE_SOURCE_DIR}/hphp/runtime/vm/iter-args-flags.h"
)
set(ATTR_HEADER "${CMAKE_SOURCE_DIR}/hphp/runtime/base/attr.h")
set(FFI_EXTRA_HEADER "${CMAKE_SOURCE_DIR}/hphp/hack/src/hackc/ffi/ffi_extra.h")

add_custom_command(
  OUTPUT ${HHBC_AST_HEADER}
  COMMAND
   . "${CMAKE_CURRENT_BINARY_DIR}/dev_env_rust_only.sh" &&
    ${INVOKE_CARGO} ffi_cbindgen ffi_cbindgen --exe &&
    ${INVOKE_CARGO} ffi_cbindgen ffi_cbindgen --bin ffi_cbindgen
      --header "${FFI_HEADER}" --namespaces "HPHP,hackc"
      --includes "${FFI_EXTRA_HEADER}"
      ${FFI_SRCS} &&
    ${INVOKE_CARGO} ffi_cbindgen ffi_cbindgen --bin ffi_cbindgen
      --header "${NAMING_SPECIAL_NAMES_HEADER}" --namespaces "HPHP,hackc,hhbc"
      ${NAMING_SPECIAL_NAMES_SRCS} &&
    ${INVOKE_CARGO} ffi_cbindgen ffi_cbindgen --bin ffi_cbindgen
      --header "${HHBC_AST_HEADER}" --namespaces "HPHP,hackc,hhbc"
      --includes "${FFI_HEADER},${NAMING_SPECIAL_NAMES_HEADER},${TYPE_CONSTRAINT_HEADER},${ATTR_HEADER},${FCALL_HEADER},${HHBC_HEADER},${ITER_ARGS_FLAGS_HEADER}"
      ${HHBC_AST_SRCS}
  DEPENDS rustc cargo "${RUST_OPCODES}"
  COMMENT "Generating hhbc-unit.h"
)

add_custom_target(
  "hhbc_ast_cbindgen"
  DEPENDS ${HHBC_AST_HEADER}
)

add_library("hhbc_ast_header" INTERFACE)
add_dependencies("hhbc_ast_header" "hhbc_ast_cbindgen")

add_custom_target(hackc_options)
include(HHVMRenderConfigSpecification)
HHVM_RENDER_CONFIG_SPECIFICATION(
  hackc_options
  TYPE hackc
  OUTPUT_PATH "${CMAKE_BINARY_DIR}/hphp/hack/src/hackc/compile"
)

# Compiling cxx entrypoints for hhvm
#
# Usage:
#     build_cxx_bridge(
#       name
#       DIR directory
#       [EXTRA_SRCS src [src ...]]
#       [LINK_LIBS lib [lib ...]]
#     )
#
# Where:
#   `name` is the target name of the cxx_bridge.
#   `directory` is the required directory of the cxx_bridge sources.
#   `src` are extra source files to include in the bridge.
#   `lib` are extra link libraries to include in the bridge.
#

function(build_cxx_bridge NAME)
  cmake_parse_arguments(CXX_BRIDGE "" "DIR" "EXTRA_SRCS;LINK_LIBS" ${ARGN})

  if ("${CXX_BRIDGE_DIR}" STREQUAL "")
    message(FATAL_ERROR "Missing DIR parameter")
  endif()
  if (NOT "${CXX_BRIDGE_UNPARSED_ARGUMENTS}" STREQUAL "")
    message(
      FATAL_ERROR
      "Unexpected parameters: ${CXX_BRIDGE_UNPARSED_ARGUMENTS}"
    )
  endif()

  set(FFI_BRIDGE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${CXX_BRIDGE_DIR}")
  set(FFI_BRIDGE_BIN "${RUST_FFI_BUILD_ROOT}/hphp/hack/${CXX_BRIDGE_DIR}")
  set(RUST_PART_CXX "${FFI_BRIDGE_BIN}/${NAME}.cpp")
  set(RUST_PART_HEADER "${FFI_BRIDGE_BIN}/${NAME}.rs.h")
  set(GENERATED "${RUST_FFI_CARGO_TARGET_DIR}/cxxbridge/${NAME}/${NAME}")
  set(GENERATED_CXX "${GENERATED}.rs.cc")
  set(GENERATED_HEADER "${GENERATED}.rs.h")
  set(GENERATED_CXXBRIDGE "${RUST_FFI_CARGO_TARGET_DIR}/cxxbridge")

  add_custom_command(
      OUTPUT
        ${RUST_PART_CXX}
        ${RUST_PART_HEADER}
      COMMAND
        ${CMAKE_COMMAND} -E make_directory "${FFI_BRIDGE_BIN}"
      COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        "${GENERATED_CXX}"
        "${RUST_PART_CXX}"
      COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        "${GENERATED_HEADER}"
        "${RUST_PART_HEADER}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      DEPENDS
        hack_rust_ffi_bridge
        ${GENERATED_CXX}
        ${GENERATED_HEADER}
  )
  add_custom_target(
    "${NAME}_cxx"
    DEPENDS ${RUST_PART_CXX} ${RUST_PART_HEADER}
  )
  add_library("${NAME}" STATIC ${RUST_PART_CXX} ${CXX_BRIDGE_EXTRA_SRCS} )
  add_dependencies(hack_rust_ffi_bridge_targets "${NAME}")
  add_dependencies("${NAME}" "${NAME}_cxx")
  target_link_libraries(
    "${NAME}"
    PUBLIC
      ${CXX_BRIDGE_LINK_LIBS}
  )
  target_include_directories("${NAME}" INTERFACE "${RUST_FFI_BUILD_ROOT}")
  target_include_directories("${NAME}" PRIVATE "${GENERATED_CXXBRIDGE}")
endfunction()

add_custom_command(
  OUTPUT ${HHVM_RUST_FFI_STATICLIB}
  COMMAND
    ${CMAKE_COMMAND} -E make_directory "${RUST_FFI_CARGO_TARGET_DIR}"
  COMMAND
    . "${CMAKE_CURRENT_BINARY_DIR}/dev_env_rust_only.sh" &&
    cd "${HHVM_RUST_FFI_WORKSPACE_DIR}" &&
    "${CARGO_EXE}" build
      --quiet
      --package "${HHVM_RUST_FFI_STATICLIB_NAME}"
      --target-dir "${RUST_FFI_CARGO_TARGET_DIR}"
      ${HHVM_RUST_FFI_PROFILE_ARGS}
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS
    rustc
    cargo
    hackc_options
    "${OPCODE_DATA}"
    "${HHVM_RUST_FFI_CARGO_TOML}"
    "${HHVM_RUST_FFI_LIB_RS}"
)
add_custom_target(hack_rust_ffi_bridge DEPENDS ${HHVM_RUST_FFI_STATICLIB})
add_library(hack_rust_ffi STATIC IMPORTED GLOBAL)
add_dependencies(hack_rust_ffi hack_rust_ffi_bridge)
set_target_properties(
  hack_rust_ffi
  PROPERTIES
  IMPORTED_LOCATION "${HHVM_RUST_FFI_STATICLIB}"
)

add_custom_target(hack_rust_ffi_bridge_targets)

build_cxx_bridge(
  package_ffi
  DIR "src/package/ffi_bridge"
)
build_cxx_bridge(
  parser_ffi
  DIR "src/parser/ffi_bridge"
)
build_cxx_bridge(
  compiler_ffi
  DIR "src/hackc/ffi_bridge"
  EXTRA_SRCS
    "${COMPILER_FFI_EXTRA_SRC}"
  LINK_LIBS hdf
)
build_cxx_bridge(
  hdf
  DIR "src/utils/hdf"
  EXTRA_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/hdf/hdf-wrap.cpp"
  LINK_LIBS folly
)
build_cxx_bridge(
  hhvm_types_ffi
  DIR "src/hackc/hhvm_cxx/hhvm_types"
  EXTRA_SRCS
    "${HHVM_TYPES_FFI_EXTRA_SRC}"
)
build_cxx_bridge(
 hhvm_hhbc_defs_ffi
 DIR "src/hackc/hhvm_cxx/hhvm_hhbc_defs"
 EXTRA_SRCS
   "${HHVM_HHBC_DEFS_FFI_EXTRA_SRC}"
)

if (NOT LZ4_FOUND)
  add_dependencies(hack_dune lz4)
  add_dependencies(hack_dune_debug lz4)
  add_dependencies(hack_dune_test lz4)
endif()

# Intentionally not using `hack_dune_debug` as it generates output files of a
# different format (bytecode instead of raw executables, which is useful if
# you're working with Hack). Keep it around, but require it to be explicitly
# used.
# hack_dune builds the typechecker (hh_server) via OCaml/dune.
# Excluded from ALL because it requires shipit file transformations
# (internal .ml files don't match OSS .mli stubs).
# The hhvm binary doesn't need hack_dune — it only needs the Rust FFI bridges.
add_custom_target(
  hack
  DEPENDS hack_dune hack_rust_ffi_bridge_targets "${OPCODE_DATA}"
)
add_custom_target(hack_test DEPENDS hack_dune_test "${OPCODE_DATA}")

configure_file(dev_env.sh.in dev_env.sh ESCAPE_QUOTES @ONLY)
configure_file(dev_env_common.sh.in dev_env_common.sh ESCAPE_QUOTES @ONLY)
configure_file(dev_env_rust_only.sh.in dev_env_rust_only.sh ESCAPE_QUOTES @ONLY)

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
