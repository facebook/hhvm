# ----- Generate C++ sources from .thrfit files -----

add_custom_target(mcrouter_thrift1_codegen)

set(CODEGEN_SOURCES_DIR "${CMAKE_CURRENT_BINARY_DIR}/thrift-gen/src")
set(CODEGEN_SOURCES)

macro(mcrouter_thrift1_impl GEN_ARGS SOURCE SOURCE_SUBDIR)
  get_filename_component(BASENAME "${SOURCE}" NAME_WE)
  file(MAKE_DIRECTORY "${CODEGEN_SOURCES_DIR}/${SOURCE_SUBDIR}")
  # Taken from FBThriftCppLibrary.cmake in fbcode_builder
  set(OUTPUTS
    "${CODEGEN_SOURCES_DIR}/${SOURCE_SUBDIR}/gen-cpp2/${BASENAME}_constants.cpp"
    "${CODEGEN_SOURCES_DIR}/${SOURCE_SUBDIR}/gen-cpp2/${BASENAME}_data.cpp"
    "${CODEGEN_SOURCES_DIR}/${SOURCE_SUBDIR}/gen-cpp2/${BASENAME}_types.cpp"
    "${CODEGEN_SOURCES_DIR}/${SOURCE_SUBDIR}/gen-cpp2/${BASENAME}_metadata.cpp"
  )
  if("${BASENAME}" MATCHES "Service$")
    string(REPLACE "Service" "" SERVICE "${BASENAME}")
    list(APPEND OUTPUTS
      "${CODEGEN_SOURCES_DIR}/${SOURCE_SUBDIR}/gen-cpp2/${SERVICE}.cpp"
      "${CODEGEN_SOURCES_DIR}/${SOURCE_SUBDIR}/gen-cpp2/${SERVICE}AsyncClient.cpp"
      "${CODEGEN_SOURCES_DIR}/${SOURCE_SUBDIR}/gen-cpp2/${SERVICE}_processmap_binary.cpp"
      "${CODEGEN_SOURCES_DIR}/${SOURCE_SUBDIR}/gen-cpp2/${SERVICE}_processmap_compact.cpp"
    )
  endif()
  list(APPEND CODEGEN_SOURCES ${OUTPUTS})

  add_custom_command(
    OUTPUT ${OUTPUTS}
    COMMAND
    FBThrift::thrift1 --gen
    "${GEN_ARGS},include-prefix=${SOURCE_SUBDIR}"
    -o "${CODEGEN_SOURCES_DIR}/${SOURCE_SUBDIR}"
    -I "${SOURCE_SUBDIR}" -I "${CMAKE_CURRENT_SOURCE_DIR}"
    "${SOURCE_SUBDIR}/${SOURCE}"
    BYPRODUCTS ${OUTPUTS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_SUBDIR}/${SOURCE}"
  )
  add_custom_target("mcrouter_thrift1_${BASENAME}" DEPENDS ${OUTPUTS})
  add_dependencies(mcrouter_thrift1_codegen "mcrouter_thrift1_${BASENAME}")
endmacro()

macro(carbon_thrift1 SOURCE)
  mcrouter_thrift1_impl("mstch_cpp2:stack_arguments,sync_methods_return_try" "${SOURCE}" mcrouter/lib/carbon)
endmacro()

carbon_thrift1(carbon.thrift)
carbon_thrift1(carbon_result.thrift)

macro(network_thrift1 SOURCE)
  mcrouter_thrift1_impl(
    "mstch_cpp2:stack_arguments,sync_methods_return_try,terse_writes"
    "${SOURCE}"
    mcrouter/lib/network/gen
  )
endmacro()

network_thrift1(Common.thrift)
network_thrift1(Memcache.thrift)
network_thrift1(MemcacheService.thrift) 

# ----- Build a static library from the generated C++ sources ------

add_library(mcrouter_thrift_lib STATIC EXCLUDE_FROM_ALL ${CODEGEN_SOURCES})
add_dependencies(mcrouter_thrift_lib mcrouter_thrift1_codegen)
target_link_libraries(mcrouter_thrift_lib PUBLIC FBThrift::thriftcpp2)
target_include_directories(mcrouter_thrift_lib PUBLIC ${CODEGEN_SOURCES_DIR})
