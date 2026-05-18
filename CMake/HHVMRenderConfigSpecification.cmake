# Generate C++ source files and headers from doc/configs.specification
# using the generate_configs tool.
#
# Supported arguments:
# TARGET: The target to add the generated files to.
# TYPE: An output type supported by generate_configs ("hackc", "defs" or "loader").
# OUTPUT_PATH: The directory where the generated files will be placed.
function (HHVM_RENDER_CONFIG_SPECIFICATION TARGET)
  cmake_parse_arguments("HHVM_RENDER_CONFIG_SPEC" "" "TYPE;OUTPUT_PATH" "" ${ARGN})

  get_target_property(CARGO_EXE cargo LOCATION)
  get_target_property(RUSTC_EXE rustc LOCATION)

  set(HHVM_TOOLS_CONFIGS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/hphp/tools/configs)
  set(HHVM_TOOLS_CONFIGS_CARGO_DIR ${CMAKE_BINARY_DIR}/hphp/tools/configs/cargo)
  set(HHVM_TOOLS_CONFIGS_CARGO_TOML ${HHVM_TOOLS_CONFIGS_CARGO_DIR}/Cargo.toml)
  set(HHVM_TOOLS_CONFIGS_MAIN_RS ${HHVM_TOOLS_CONFIGS_CARGO_DIR}/generate_configs.rs)
  set(HHVM_TOOLS_CONFIGS_LIB_RS ${HHVM_TOOLS_CONFIGS_CARGO_DIR}/generate_configs_lib.rs)
  set(HHVM_TOOLS_CONFIGS_CARGO_TARGET_DIR ${CMAKE_BINARY_DIR}/hphp/tools/configs/target)

  file(MAKE_DIRECTORY ${HHVM_TOOLS_CONFIGS_CARGO_DIR})
  get_property(HHVM_TOOLS_CONFIGS_CARGO_GENERATED GLOBAL
    PROPERTY HHVM_TOOLS_CONFIGS_CARGO_GENERATED)
  if (NOT HHVM_TOOLS_CONFIGS_CARGO_GENERATED)
    string(CONCAT HHVM_TOOLS_CONFIGS_CARGO_TOML_CONTENT
      "[package]\n"
      "name = \"generate_configs\"\n"
      "version = \"0.0.0\"\n"
      "edition = \"2021\"\n"
      "\n"
      "[lib]\n"
      "name = \"generate_configs_lib\"\n"
      "path = \"generate_configs_lib.rs\"\n"
      "\n"
      "[[bin]]\n"
      "name = \"generate_configs\"\n"
      "path = \"generate_configs.rs\"\n"
      "\n"
      "[dependencies]\n"
      "clap = { version = \"4\", features = [\"derive\"] }\n"
      "convert_case = \"0.4.0\"\n"
      "nom = \"8\"\n"
      "nom-language = \"0.1.0\"\n"
      "\n"
      "[build-dependencies]\n"
      "anyhow = \"1.0.86\"\n"
      "cc = \"1.0.90\"\n"
    )
    string(CONCAT HHVM_TOOLS_CONFIGS_MAIN_RS_CONTENT
      "include!(r#\"${HHVM_TOOLS_CONFIGS_SOURCE_DIR}/generate_configs.rs\"#);\n"
    )
    string(CONCAT HHVM_TOOLS_CONFIGS_LIB_RS_CONTENT
      "include!(r#\"${HHVM_TOOLS_CONFIGS_SOURCE_DIR}/generate_configs_lib.rs\"#);\n"
    )
    file(GENERATE OUTPUT ${HHVM_TOOLS_CONFIGS_CARGO_TOML}
      CONTENT "${HHVM_TOOLS_CONFIGS_CARGO_TOML_CONTENT}")
    file(GENERATE OUTPUT ${HHVM_TOOLS_CONFIGS_MAIN_RS}
      CONTENT "${HHVM_TOOLS_CONFIGS_MAIN_RS_CONTENT}")
    file(GENERATE OUTPUT ${HHVM_TOOLS_CONFIGS_LIB_RS}
      CONTENT "${HHVM_TOOLS_CONFIGS_LIB_RS_CONTENT}")
    set_property(GLOBAL PROPERTY HHVM_TOOLS_CONFIGS_CARGO_GENERATED TRUE)
  endif()

  execute_process(
    COMMAND python3 ${CMAKE_SOURCE_DIR}/hphp/tools/configs/get_config_sections.py ${CMAKE_SOURCE_DIR}/hphp/doc/configs.specification
    OUTPUT_VARIABLE HHVM_RENDER_CONFIG_SPEC_CONFIG_SECTIONS
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if ("${HHVM_RENDER_CONFIG_SPEC_TYPE}" STREQUAL "hackc")
    list(APPEND HHVM_RENDER_CONFIG_SPEC_CONFIG_HEADERS ${HHVM_RENDER_CONFIG_SPEC_OUTPUT_PATH}/options_gen.h)
  elseif("${HHVM_RENDER_CONFIG_SPEC_TYPE}" STREQUAL "loader")
    foreach(SECTION ${HHVM_RENDER_CONFIG_SPEC_CONFIG_SECTIONS})
      list(APPEND HHVM_RENDER_CONFIG_SPEC_CONFIG_HEADERS ${HHVM_RENDER_CONFIG_SPEC_OUTPUT_PATH}/${SECTION}-loader.h)
      list(APPEND HHVM_RENDER_CONFIG_SPEC_CONFIG_SOURCES ${HHVM_RENDER_CONFIG_SPEC_OUTPUT_PATH}/${SECTION}-loader.cpp)
    endforeach()

    list(
      APPEND HHVM_RENDER_CONFIG_SPEC_CONFIG_HEADERS
      ${HHVM_RENDER_CONFIG_SPEC_OUTPUT_PATH}/repo-global-data-generated.h
      ${HHVM_RENDER_CONFIG_SPEC_OUTPUT_PATH}/unit-cache-generated.h
      ${HHVM_RENDER_CONFIG_SPEC_OUTPUT_PATH}/repo-option-flags-generated.h
    )

    list(APPEND HHVM_RENDER_CONFIG_SPEC_CONFIG_SOURCES ${HHVM_RENDER_CONFIG_SPEC_OUTPUT_PATH}/configs-generated.cpp)
  elseif("${HHVM_RENDER_CONFIG_SPEC_TYPE}" STREQUAL "defs")
    foreach(SECTION ${HHVM_RENDER_CONFIG_SPEC_CONFIG_SECTIONS})
      list(APPEND HHVM_RENDER_CONFIG_SPEC_CONFIG_HEADERS ${HHVM_RENDER_CONFIG_SPEC_OUTPUT_PATH}/${SECTION}.h)
      list(APPEND HHVM_RENDER_CONFIG_SPEC_CONFIG_SOURCES ${HHVM_RENDER_CONFIG_SPEC_OUTPUT_PATH}/${SECTION}.cpp)
    endforeach()
  else()
    message(FATAL_ERROR "Invalid output type: ${HHVM_RENDER_CONFIG_SPEC_TYPE}")
  endif()

  set(HHVM_RENDER_CONFIG_SPEC_CARGO_HOME ${CMAKE_BINARY_DIR}/hphp/tools/configs/.cargo)

  add_custom_command(
    OUTPUT ${HHVM_RENDER_CONFIG_SPEC_CONFIG_SOURCES} ${HHVM_RENDER_CONFIG_SPEC_CONFIG_HEADERS}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${HHVM_RENDER_CONFIG_SPEC_OUTPUT_PATH}
    COMMAND
      ${CMAKE_COMMAND} -E env
      RUSTC=${RUSTC_EXE}
      CARGO_HOME=${HHVM_RENDER_CONFIG_SPEC_CARGO_HOME}
      CARGO_TARGET_DIR=${HHVM_TOOLS_CONFIGS_CARGO_TARGET_DIR}
      ${CARGO_EXE}
      run
      --manifest-path ${HHVM_TOOLS_CONFIGS_CARGO_TOML}
      --quiet
      --bin generate_configs
      --
      ${HHVM_RENDER_CONFIG_SPEC_TYPE}
      ${HHVM_RENDER_CONFIG_SPEC_OUTPUT_PATH}
      ${CMAKE_SOURCE_DIR}/hphp/doc/configs.specification
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
      ${CMAKE_SOURCE_DIR}/hphp/doc/configs.specification
      ${HHVM_TOOLS_CONFIGS_CARGO_TOML}
      ${HHVM_TOOLS_CONFIGS_MAIN_RS}
      ${HHVM_TOOLS_CONFIGS_LIB_RS}
      ${HHVM_TOOLS_CONFIGS_SOURCE_DIR}/generate_configs.rs
      ${HHVM_TOOLS_CONFIGS_SOURCE_DIR}/generate_configs_lib.rs
      rustc
      cargo
    VERBATIM
  )

  add_custom_target(hhvm_render_config_section_${HHVM_RENDER_CONFIG_SPEC_TYPE}
    DEPENDS ${HHVM_RENDER_CONFIG_SPEC_CONFIG_SOURCES} ${HHVM_RENDER_CONFIG_SPEC_CONFIG_HEADERS})

  add_dependencies(${TARGET} hhvm_render_config_section_${HHVM_RENDER_CONFIG_SPEC_TYPE})
  target_sources(${TARGET} PRIVATE ${HHVM_RENDER_CONFIG_SPEC_CONFIG_SOURCES})
endfunction()
