find_package(OCaml)

if (OCAMLC_FOUND)
  # This is totally the wrong way to do this, but I am tired of fighting with
  # build systems and don't really care to make this work the right way. libelf
  # is all we need right now ayways.
  unset(hackcflags)
  foreach(lib ${LIBELF_INCLUDE_DIRS})
    list(APPEND hackcflags -ccopt -I${lib})
  endforeach()
  if(ELF_GETSHDRSTRNDX)
    list(APPEND hackcflags -ccopt -DHAVE_ELF_GETSHDRSTRNDX)
  endif()

  foreach(lib ${LIBELF_LIBRARIES})
    get_filename_component(pth ${lib} PATH)
    get_filename_component(base ${lib} NAME_WE)
    string(REGEX REPLACE "^lib" "" libname ${base})
    list(APPEND hackcflags -ccopt -L${pth} -cclib -l${libname})
  endforeach()

  # Xcode/Ninja generators undefined MAKE
  if(NOT MAKE)
    set(MAKE make)
  endif()

  add_custom_target(
    hack
    ALL
    COMMAND $(MAKE) depend &&
      env OPTBIN="${OCAMLC_OPT_SUFFIX}"
          OCAMLCFLAGS_EXTRA="${hackcflags}"
          $(MAKE)
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )

  install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hh_client
    DESTINATION bin
    COMPONENT dev)

  install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/bin/hh_server
    DESTINATION bin
    COMPONENT dev)
endif()
