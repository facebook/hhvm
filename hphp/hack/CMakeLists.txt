CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)

find_package(OCaml)
find_package(LZ4)
find_package(LibElf)
find_package(Memfd)

if (OCAMLC_FOUND)
  # This is totally the wrong way to do this, but I am tired of fighting with
  # build systems and don't really care to make this work the right way. libelf
  # and lz4 are all we need right now anyways.
  unset(extra_include_paths)
  unset(extra_lib_paths)
  unset(extra_cc_flags)
  foreach(lib ${LIBELF_INCLUDE_DIRS})
    list(APPEND extra_include_paths ${lib})
  endforeach()
  if(ELF_GETSHDRSTRNDX)
    list(APPEND extra_cc_flags -DHAVE_ELF_GETSHDRSTRNDX)
  endif()

  if(HAVE_MEMFD_CREATE)
    list(APPEND extra_cc_flags -DHAVE_MEMFD_CREATE)
  endif()

  if(HAVE_DECL___NR_MEMFD_CREATE)
    list(APPEND extra_cc_flags -DHAVE_DECL___NR_MEMFD_CREATE)
  endif()

  list(APPEND extra_cc_flags -DOSS_SMALL_HH_TABLE_POWS)
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
    list(APPEND extra_include_paths "${TP_DIR}/lz4")
    list(APPEND extra_lib_paths "${TP_BUILD_DIR}/lz4")
  endif()
  list(APPEND extra_native_libraries "lz4")

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
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )

  if(NOT LZ4_FOUND)
    # if the system does not have lz4, make sure that the one in public_tld
    # gets built
    add_dependencies(hack lz4)
    add_dependencies(hack_test lz4)
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
