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
    COMMAND ${CMAKE_COMMAND} -E env RUSTC=${RUSTC_EXE} CARGO_HOME=${HHVM_RENDER_CONFIG_SPEC_CARGO_HOME}
      ${CARGO_EXE} run --quiet -- ${HHVM_RENDER_CONFIG_SPEC_TYPE} ${HHVM_RENDER_CONFIG_SPEC_OUTPUT_PATH} ${CMAKE_SOURCE_DIR}/hphp/doc/configs.specification
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/hphp/tools/configs
    DEPENDS ${CMAKE_SOURCE_DIR}/hphp/doc/configs.specification rustc cargo
    VERBATIM
  )

  add_custom_target(hhvm_render_config_section_${HHVM_RENDER_CONFIG_SPEC_TYPE}
    DEPENDS ${HHVM_RENDER_CONFIG_SPEC_CONFIG_SOURCES} ${HHVM_RENDER_CONFIG_SPEC_CONFIG_HEADERS})

  add_dependencies(${TARGET} hhvm_render_config_section_${HHVM_RENDER_CONFIG_SPEC_TYPE})
  target_sources(${TARGET} PRIVATE ${HHVM_RENDER_CONFIG_SPEC_CONFIG_SOURCES})
endfunction()
