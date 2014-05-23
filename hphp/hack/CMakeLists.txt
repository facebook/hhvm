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

  unset(hacklibs)
  foreach(lib ${LIBELF_LIBRARIES})
    get_filename_component(pth ${lib} PATH)
    get_filename_component(base ${lib} NAME_WE)
    list(APPEND hacklibs ${pth}/${base}.a)
  endforeach()

  add_custom_target(
    hack
    ALL
    COMMAND make depend &&
      env OPTBIN="${OCAMLC_OPT_SUFFIX}"
          OCAMLCFLAGS_EXTRA="${hackcflags}"
          EXTLIBS="${hacklibs}"
          make
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )
endif()
