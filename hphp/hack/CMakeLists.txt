find_package(OCaml)

if (OCAMLC_FOUND)
  add_custom_target(
    hack
    ALL
    COMMAND CMAKE=1 make depend &&
            env OPTBIN="${OCAMLC_OPT_SUFFIX}" CMAKE=1 make
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )
endif()
