CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../CMake" ${CMAKE_MODULE_PATH})

find_package(OCaml)
find_package(LZ4)
find_package(LibElf)

if (OCAMLC_FOUND)
  # This is totally the wrong way to do this, but I am tired of fighting with
  # build systems and don't really care to make this work the right way. libelf
  # is all we need right now anyways.
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

  # Xcode/Ninja generators undefined MAKE
  if(NOT MAKE)
    set(MAKE make)
  endif()

  add_custom_target(
    hack
    ALL
    COMMAND
      env OPTBIN="${OCAMLC_OPT_SUFFIX}"
          $(MAKE) EXTRA_INCLUDE_PATHS="${extra_include_paths}"
          EXTRA_LIB_PATHS="${extra_lib_paths}"
          EXTRA_CC_FLAGS="${extra_cc_flags}"
          EXTRA_NATIVE_LIBRARIES="${extra_native_libraries}"
          BYTECODE="${EMIT_OCAML_BYTECODE}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )

  add_custom_target(
    hack_test
    COMMAND
      env OPTBIN="${OCAMLC_OPT_SUFFIX}"
          $(MAKE) test EXTRA_INCLUDE_PATHS="${extra_include_paths}"
          EXTRA_LIB_PATHS="${extra_lib_paths}"
          EXTRA_CC_FLAGS="${extra_cc_flags}"
          EXTRA_NATIVE_LIBRARIES="${extra_native_libraries}"
          BYTECODE="${EMIT_OCAML_BYTECODE}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )

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
