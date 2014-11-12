find_package(OCaml)

if (OCAMLC_FOUND)
  # This is totally the wrong way to do this, but I am tired of fighting with
  # build systems and don't really care to make this work the right way. libelf
  # is all we need right now ayways.
  unset(extra_include_paths)
  unset(extra_lib_paths)
  unset(extra_cc_flags)
  foreach(lib ${LIBELF_INCLUDE_DIRS})
    list(APPEND extra_include_paths ${lib})
  endforeach()
  if(ELF_GETSHDRSTRNDX)
    list(APPEND extra_cc_flags -DHAVE_ELF_GETSHDRSTRNDX)
  endif()

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
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )

  install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hh_client
    DESTINATION bin
    COMPONENT dev)

  install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hh_server
    DESTINATION bin
    COMPONENT dev)
endif()
