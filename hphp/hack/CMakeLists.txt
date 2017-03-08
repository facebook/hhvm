CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../CMake" ${CMAKE_MODULE_PATH})

option(BUILD_HACK "True if we should build the Hack typechecker." ON)

if (NOT BUILD_HACK)
  message(STATUS "Skipping hack")

else()
  message(STATUS "Building hack")

  find_package(LZ4)
  find_package(LibElf)

  # This is totally the wrong way to do this, but I am tired of fighting with
  # build systems and don't really care to make this work the right way. libelf,
  # lz4 and sqlite3 all we need right now anyways.
  unset(extra_include_paths)
  unset(extra_lib_paths)
  unset(extra_cc_flags)

  # Allows '#include "hphp/path/to/library/"' paths to start from hphp
  # project directory  which is consistent with fbmake's include paths.

  IF(HPHP_HOME)
    list(APPEND extra_include_paths ${HPHP_HOME})
  ELSE()
    list(APPEND extra_include_paths ${CMAKE_CURRENT_SOURCE_DIR}/../..)
  ENDIF()

  foreach(lib ${LIBELF_INCLUDE_DIRS})
    list(APPEND extra_include_paths ${lib})
  endforeach()
  if(ELF_GETSHDRSTRNDX)
    list(APPEND extra_cc_flags -DHAVE_ELF_GETSHDRSTRNDX)
  endif()

  list(APPEND extra_cc_flags -pthread)

  foreach(lib ${LIBELF_LIBRARIES})
    get_filename_component(pth ${lib} PATH)
    list(APPEND extra_lib_paths ${pth})
  endforeach()

  if(LZ4_FOUND)
    list(APPEND extra_include_paths ${LZ4_INCLUDE_DIR})
    get_filename_component(pth ${LZ4_LIBRARY} PATH)
    list(APPEND extra_lib_paths ${pth})
  else()
    list(APPEND extra_include_paths "${TP_DIR}/lz4/src/lib")
    list(APPEND extra_lib_paths "${TP_BUILD_DIR}/lz4")
  endif()
  list(APPEND extra_native_libraries "lz4")

  if(PC_SQLITE3_FOUND)
    list(APPEND extra_include_paths ${LIBSQLITE3_INCLUDE_DIR})
    get_filename_component(pth ${LIBSQLITE3_LIBRARY} PATH)
    list(APPEND extra_lib_paths ${pth})
  else()
    list(APPEND extra_include_paths "${TP_DIR}/libsqlite3")
    list(APPEND extra_lib_paths "${TP_BUILD_DIR}/libsqlite3")
  endif()
  list(APPEND extra_native_libraries "sqlite3")

  # Xcode/Ninja generators undefined MAKE
  if(NOT MAKE)
    set(MAKE make)
  endif()

  add_custom_target(
    hack
    ALL
    COMMAND
          $(MAKE) EXTRA_INCLUDE_PATHS="${extra_include_paths}"
          EXTRA_LIB_PATHS="${extra_lib_paths}"
          EXTRA_CC_FLAGS="${extra_cc_flags}"
          EXTRA_NATIVE_LIBRARIES="${extra_native_libraries}"
          BYTECODE="${EMIT_OCAML_BYTECODE}"
          OCAML="${OCAML_EXECUTABLE}"
          OCAMLC="${OCAMLC_EXECUTABLE}"
          OCAMLBUILD="${OCAMLBUILD_EXECUTABLE}"
          OCAMLBUILD_FLAGS="-ocamlc=${OCAMLC_EXECUTABLE},-ocamlopt=${OCAMLOPT_EXECUTABLE}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )
  add_dependencies(hack ocaml)

  add_custom_target(
    hack_test
    COMMAND
          $(MAKE) test EXTRA_INCLUDE_PATHS="${extra_include_paths}"
          EXTRA_LIB_PATHS="${extra_lib_paths}"
          EXTRA_CC_FLAGS="${extra_cc_flags}"
          EXTRA_NATIVE_LIBRARIES="${extra_native_libraries}"
          BYTECODE="${EMIT_OCAML_BYTECODE}"
          OCAML="${OCAML_EXECUTABLE}"
          OCAMLC="${OCAMLC_EXECUTABLE}"
          OCAMLBUILD="${OCAMLBUILD_EXECUTABLE}"
          OCAMLBUILD_FLAGS="-ocamlc=${OCAMLC_EXECUTABLE},-ocamlopt=${OCAMLOPT_EXECUTABLE}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )
  add_dependencies(hack_test ocaml)

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

  install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hh_client
    DESTINATION bin
    COMPONENT dev)

  install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hh_server
    DESTINATION bin
    COMPONENT dev)

  install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hh_format
    DESTINATION bin
    COMPONENT dev)

  install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/h2tp
    DESTINATION bin
    COMPONENT dev)

  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/h2tp/resources/
    DESTINATION share/hhvm/hack/hacklib
    COMPONENT dev)
endif()

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hh_client
  DESTINATION bin
  COMPONENT dev)

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hh_server
  DESTINATION bin
  COMPONENT dev)

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hackfmt
  DESTINATION bin
  COMPONENT dev)

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hh_format
  DESTINATION bin
  COMPONENT dev)

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/h2tp
  DESTINATION bin
  COMPONENT dev)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/h2tp/resources/
  DESTINATION share/hhvm/hack/hacklib
  COMPONENT dev)
