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

set(CARGO_BUILD "${CMAKE_SOURCE_DIR}/hphp/hack/scripts/build_rust_to_ocaml.sh")

if(DEFINED ENV{HACKDEBUG})
  set(PROFILE "debug")
else()
  set(PROFILE "release")
endif()

set(HHBC_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/src/hhbc")
set(HHBC_AST_SRCS
  "hhbc_by_ref/hhas_adata.rs"
  "hhbc_by_ref/hhas_attribute.rs"
  "hhbc_by_ref/hhas_body.rs"
  "hhbc_by_ref/hhas_class.rs"
  "hhbc_by_ref/hhas_coeffects.rs"
  "hhbc_by_ref/hhas_constant.rs"
  "hhbc_by_ref/hhas_function.rs"
  "hhbc_by_ref/hhas_method.rs"
  "hhbc_by_ref/hhas_param.rs"
  "hhbc_by_ref/hhas_pos.rs"
  "hhbc_by_ref/hhas_program.rs"
  "hhbc_by_ref/hhas_property.rs"
  "hhbc_by_ref/hhas_record_def.rs"
  "hhbc_by_ref/hhas_symbol_refs.rs"
  "hhbc_by_ref/hhas_type.rs"
  "hhbc_by_ref/hhas_type_const.rs"
  "hhbc_by_ref/hhas_typedef.rs"
  "hhbc_by_ref/hhbc_ast.rs"
  "hhbc_by_ref/hhbc_ast_ffi_cbindgen.rs"
  "hhbc_by_ref/hhbc_id.rs"
  "emitter/instruction_sequence.rs"
  "emitter/label.rs"
  "emitter/local.rs"
  "emitter/symbol_refs_state.rs"
  "emitter/typed_value.rs"
)
foreach(src ${HHBC_AST_SRCS})
  string(APPEND HHBC_AST_SRC_ARG "${HHBC_PREFIX}/${src},")
endforeach()

set(NAMING_SPECIAL_NAMES_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/src/naming")
set(NAMING_SPECIAL_NAMES_SRCS
   "naming_special_names.rs"
   "naming_special_names_ffi_cbindgen.rs"
)
foreach(src ${NAMING_SPECIAL_NAMES_SRCS})
  string(APPEND NAMING_SPECIAL_NAMES_SRC_ARG "${NAMING_SPECIAL_NAMES_PREFIX}/${src},")
endforeach()

set(FFI_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/ffi")
set(FFI_SRCS
   "ffi.rs"
   "ffi_ffi_cbindgen.rs"
)
foreach(src ${FFI_SRCS})
  string(APPEND FFI_SRC_ARG "${FFI_PREFIX}/${src},")
endforeach()

set(HHBC_AST_HEADER "${RUST_FFI_BUILD_ROOT}/hphp/hack/src/hhbc/hhbc-ast.h")
set(FFI_HEADER "${RUST_FFI_BUILD_ROOT}/hphp/hack/src/utils/ffi.h")
set(NAMING_SPECIAL_NAMES_HEADER "${RUST_FFI_BUILD_ROOT}/hphp/hack/src/naming/naming-special-names.h")
add_custom_command(
  OUTPUT ${HHBC_AST_HEADER}
  COMMAND
   . "${CMAKE_CURRENT_BINARY_DIR}/dev_env_rust_only.sh" &&
    ${CARGO_BUILD} ffi_cbindgen ffi_cbindgen --exe
      --header "${FFI_HEADER}" --srcs "${FFI_SRC_ARG}"
      --namespaces "HPHP,hackc" &&
    ${CARGO_BUILD} ffi_cbindgen ffi_cbindgen --exe
      --header "${NAMING_SPECIAL_NAMES_HEADER}" --srcs "${NAMING_SPECIAL_NAMES_SRC_ARG}" --namespaces "HPHP,hackc,hhbc"
      --namespaces "HPHP,hackc" &&
    ${CARGO_BUILD} ffi_cbindgen ffi_cbindgen --exe
      --header "${HHBC_AST_HEADER}" --srcs "${HHBC_AST_SRC_ARG}" --namespaces "HPHP,hackc,hhbc"
      --includes "${FFI_HEADER},${NAMING_SPECIAL_NAMES_HEADER}"
  DEPENDS rustc cargo
  COMMENT "Generating hhbc-ast.h"
)

add_custom_target(
  "hhbc_ast_cbindgen"
  DEPENDS ${HHBC_AST_HEADER}
)

add_library("hhbc_ast_header" INTERFACE)
add_dependencies("hhbc_ast_header" "hhbc_ast_cbindgen")

add_custom_target(hack_rust_ffi_bridge_targets)

# Compiling cxx entrypoints for hhvm
function(build_cxx_bridge NAME FFI_BRIDGE_DIR)
  set(FFI_BRIDGE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FFI_BRIDGE_DIR}")
  set(FFI_BRIDGE_BIN "${RUST_FFI_BUILD_ROOT}/hphp/hack/${FFI_BRIDGE_DIR}")

  set(RUST_PART_LIB "${FFI_BRIDGE_BIN}/${PROFILE}/${CMAKE_STATIC_LIBRARY_PREFIX}${NAME}_ffi_bridge${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set(RUST_PART_CXX "${FFI_BRIDGE_BIN}/${NAME}_ffi_bridge.cpp")
  set(RUST_PART_HEADER "${FFI_BRIDGE_BIN}/${NAME}_ffi_bridge.rs")
  set(GENERATED "${FFI_BRIDGE_BIN}/cxxbridge/${NAME}_ffi_bridge/${NAME}_ffi_bridge")

  add_custom_command(
      OUTPUT ${RUST_PART_CXX}
      COMMAND
        ${CMAKE_COMMAND} -E make_directory "${FFI_BRIDGE_BIN}" &&
        . "${CMAKE_CURRENT_BINARY_DIR}/dev_env_rust_only.sh" &&
        ${CARGO_BUILD} "${NAME}_ffi_bridge" "${NAME}_ffi_bridge" --cxx "${FFI_BRIDGE_BIN}" &&
        ${CMAKE_COMMAND} -E copy "${GENERATED}.rs.cc" "${RUST_PART_CXX}" &&
        ${CMAKE_COMMAND} -E copy "${GENERATED}.rs.h" "${RUST_PART_HEADER}"
      WORKING_DIRECTORY ${FFI_BRIDGE_SRC}
      DEPENDS rustc cargo
  )
  add_custom_target(
    "${NAME}_cxx"
    DEPENDS ${RUST_PART_CXX}
  )

  add_library("${NAME}_ffi_bridge" STATIC ${RUST_PART_CXX})
  add_dependencies("${NAME}_ffi_bridge" rustc cargo "${NAME}_cxx")
  add_dependencies(hack_rust_ffi_bridge_targets "${NAME}_ffi_bridge")
  target_link_libraries("${NAME}_ffi_bridge" PUBLIC ${RUST_PART_LIB})
  # `-iquote` is like `-I` (or target_include_directories()`), except:
  # - it takes precedence over `-I`
  # - it only applies to `#include "foo"`, not `#include <foo>`
  target_compile_options("${NAME}_ffi_bridge" INTERFACE "-iquote" "${RUST_FFI_BUILD_ROOT}")
endfunction()

build_cxx_bridge(rust_parser "src/parser/ffi_bridge")
build_cxx_bridge(rust_compile "src/hhbc/ffi_bridge")

if (NOT LZ4_FOUND)
  add_dependencies(hack_dune lz4)
  add_dependencies(hack_dune_debug lz4)
  add_dependencies(hack_dune_test lz4)
endif()

# Intentionally not using `hack_dune_debug` as it generates output files of a
# different format (bytecode instead of raw executables, which is useful if
# you're working with Hack, but means that e.g. hhvm can't find
# `hh_single_compile` in the source tree. Keep it around, but require it to be
# explicitly used
add_custom_target(hack ALL DEPENDS hack_dune)
add_custom_target(hack_test DEPENDS hack_dune_test)

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
