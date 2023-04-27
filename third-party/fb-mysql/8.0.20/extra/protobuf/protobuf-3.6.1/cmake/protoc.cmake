set(protoc_files
  ${protobuf_source_dir}/src/google/protobuf/compiler/main.cc
)

add_executable(protoc ${protoc_files})
target_link_libraries(protoc libprotobuf libprotoc)
add_executable(protobuf::protoc ALIAS protoc)

SET_TARGET_PROPERTIES(protoc PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/runtime_output_directory)
