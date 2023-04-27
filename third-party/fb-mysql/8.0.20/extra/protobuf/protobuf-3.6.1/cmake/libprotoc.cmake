set(libprotoc_files
  ${protobuf_source_dir}/src/google/protobuf/compiler/code_generator.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/command_line_interface.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_enum.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_enum_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_extension.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_file.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_generator.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_helpers.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_map_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_message.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_message_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_padding_optimizer.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_primitive_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_service.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_string_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_doc_comment.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_enum.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_enum_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_field_base.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_generator.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_helpers.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_map_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_message.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_message_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_primitive_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_reflection_class.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_repeated_enum_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_repeated_message_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_repeated_primitive_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_source_generator_base.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_wrapper_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_context.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_doc_comment.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_enum.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_enum_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_enum_field_lite.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_enum_lite.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_extension.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_extension_lite.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_file.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_generator.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_generator_factory.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_helpers.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_lazy_message_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_lazy_message_field_lite.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_map_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_map_field_lite.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_message.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_message_builder.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_message_builder_lite.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_message_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_message_field_lite.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_message_lite.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_name_resolver.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_primitive_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_primitive_field_lite.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_service.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_shared_code_generator.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_string_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_string_field_lite.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/js/js_generator.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/js/well_known_types_embed.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_enum.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_enum_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_extension.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_file.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_generator.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_helpers.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_map_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_message.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_message_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_oneof.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_primitive_field.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/php/php_generator.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/plugin.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/plugin.pb.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/python/python_generator.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/ruby/ruby_generator.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/subprocess.cc
  ${protobuf_source_dir}/src/google/protobuf/compiler/zip_writer.cc
)

set(libprotoc_headers
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_enum.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_enum_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_extension.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_file.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_helpers.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_map_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_message.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_message_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_message_layout_helper.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_options.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_padding_optimizer.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_primitive_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_service.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/cpp/cpp_string_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_doc_comment.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_enum.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_enum_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_field_base.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_helpers.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_map_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_message.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_message_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_options.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_primitive_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_reflection_class.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_repeated_enum_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_repeated_message_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_repeated_primitive_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_source_generator_base.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/csharp/csharp_wrapper_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_context.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_doc_comment.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_enum.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_enum_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_enum_field_lite.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_enum_lite.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_extension.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_extension_lite.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_file.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_generator_factory.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_helpers.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_lazy_message_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_lazy_message_field_lite.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_map_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_map_field_lite.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_message.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_message_builder.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_message_builder_lite.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_message_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_message_field_lite.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_message_lite.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_name_resolver.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_options.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_primitive_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_primitive_field_lite.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_service.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_shared_code_generator.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_string_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/java/java_string_field_lite.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_enum.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_enum_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_extension.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_file.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_helpers.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_map_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_message.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_message_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_oneof.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/objectivec/objectivec_primitive_field.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/subprocess.h
  ${protobuf_source_dir}/src/google/protobuf/compiler/zip_writer.h
)

if (MSVC)
set(libprotoc_rc_files
  ${CMAKE_CURRENT_BINARY_DIR}/version.rc
)
endif()

# The custom command using xprotocol_plugin hangs on solaris if
# libprotoc is a shared library.
# TODO: should we make it STATIC on all platforms?
IF(SOLARIS)
  add_library(libprotoc STATIC
    ${libprotoc_files} ${libprotoc_headers})
ELSE()
  add_library(libprotoc ${protobuf_SHARED_OR_STATIC}
    ${libprotoc_files} ${libprotoc_headers})
ENDIF()

target_link_libraries(libprotoc libprotobuf)
if(MSVC AND protobuf_BUILD_SHARED_LIBS)
  target_compile_definitions(libprotoc
    PUBLIC  PROTOBUF_USE_DLLS
    PRIVATE LIBPROTOC_EXPORTS)
endif()
set_target_properties(libprotoc PROPERTIES
    COMPILE_DEFINITIONS LIBPROTOC_EXPORTS
    VERSION ${protobuf_VERSION}
    OUTPUT_NAME ${LIB_PREFIX}protoc
    DEBUG_POSTFIX "${protobuf_DEBUG_POSTFIX}")
add_library(protobuf::libprotoc ALIAS libprotoc)

IF(protobuf_BUILD_SHARED_LIBS AND NOT SOLARIS)
  SET_TARGET_PROPERTIES(libprotoc PROPERTIES
    DEBUG_POSTFIX ""
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/library_output_directory
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/library_output_directory)
  IF(WIN32)
    ADD_CUSTOM_COMMAND(TARGET libprotoc POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_BINARY_DIR}/library_output_directory/${CMAKE_CFG_INTDIR}/libprotoc.dll"
      "${CMAKE_BINARY_DIR}/runtime_output_directory/${CMAKE_CFG_INTDIR}/libprotoc.dll"
      )
  ENDIF()
ENDIF()

