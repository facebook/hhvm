find_package(OCaml)

if (OCAMLC_FOUND)
	add_custom_target(
		hack
		ALL
		COMMAND make depend && env OPTBIN="${OCAMLC_OPT_SUFFIX}" make
		WORKING_DIRECTORY src
	)
endif()
