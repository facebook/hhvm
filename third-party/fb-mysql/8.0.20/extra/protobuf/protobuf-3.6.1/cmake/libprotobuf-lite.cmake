set(libprotobuf_lite_files
  ${protobuf_source_dir}/src/google/protobuf/arena.cc
  ${protobuf_source_dir}/src/google/protobuf/arenastring.cc
  ${protobuf_source_dir}/src/google/protobuf/extension_set.cc
  ${protobuf_source_dir}/src/google/protobuf/generated_message_table_driven_lite.cc
  ${protobuf_source_dir}/src/google/protobuf/generated_message_util.cc
  ${protobuf_source_dir}/src/google/protobuf/implicit_weak_message.cc
  ${protobuf_source_dir}/src/google/protobuf/io/coded_stream.cc
  ${protobuf_source_dir}/src/google/protobuf/io/zero_copy_stream.cc
  ${protobuf_source_dir}/src/google/protobuf/io/zero_copy_stream_impl_lite.cc
  ${protobuf_source_dir}/src/google/protobuf/message_lite.cc
  ${protobuf_source_dir}/src/google/protobuf/repeated_field.cc
  ${protobuf_source_dir}/src/google/protobuf/stubs/bytestream.cc
  ${protobuf_source_dir}/src/google/protobuf/stubs/common.cc
  ${protobuf_source_dir}/src/google/protobuf/stubs/int128.cc
  ${protobuf_source_dir}/src/google/protobuf/stubs/io_win32.cc
  ${protobuf_source_dir}/src/google/protobuf/stubs/status.cc
  ${protobuf_source_dir}/src/google/protobuf/stubs/statusor.cc
  ${protobuf_source_dir}/src/google/protobuf/stubs/stringpiece.cc
  ${protobuf_source_dir}/src/google/protobuf/stubs/stringprintf.cc
  ${protobuf_source_dir}/src/google/protobuf/stubs/structurally_valid.cc
  ${protobuf_source_dir}/src/google/protobuf/stubs/strutil.cc
  ${protobuf_source_dir}/src/google/protobuf/stubs/time.cc
  ${protobuf_source_dir}/src/google/protobuf/wire_format_lite.cc
)

set(libprotobuf_lite_includes
  ${protobuf_source_dir}/src/google/protobuf/arena.h
  ${protobuf_source_dir}/src/google/protobuf/arenastring.h
  ${protobuf_source_dir}/src/google/protobuf/extension_set.h
  ${protobuf_source_dir}/src/google/protobuf/generated_message_util.h
  ${protobuf_source_dir}/src/google/protobuf/implicit_weak_message.h
  ${protobuf_source_dir}/src/google/protobuf/io/coded_stream.h
  ${protobuf_source_dir}/src/google/protobuf/io/zero_copy_stream.h
  ${protobuf_source_dir}/src/google/protobuf/io/zero_copy_stream_impl_lite.h
  ${protobuf_source_dir}/src/google/protobuf/message_lite.h
  ${protobuf_source_dir}/src/google/protobuf/repeated_field.h
  ${protobuf_source_dir}/src/google/protobuf/stubs/bytestream.h
  ${protobuf_source_dir}/src/google/protobuf/stubs/common.h
  ${protobuf_source_dir}/src/google/protobuf/stubs/int128.h
  ${protobuf_source_dir}/src/google/protobuf/stubs/once.h
  ${protobuf_source_dir}/src/google/protobuf/stubs/status.h
  ${protobuf_source_dir}/src/google/protobuf/stubs/statusor.h
  ${protobuf_source_dir}/src/google/protobuf/stubs/stringpiece.h
  ${protobuf_source_dir}/src/google/protobuf/stubs/stringprintf.h
  ${protobuf_source_dir}/src/google/protobuf/stubs/strutil.h
  ${protobuf_source_dir}/src/google/protobuf/stubs/time.h
  ${protobuf_source_dir}/src/google/protobuf/wire_format_lite.h
)

add_library(libprotobuf-lite ${protobuf_SHARED_OR_STATIC}
  ${libprotobuf_lite_files} ${libprotobuf_lite_includes})
target_link_libraries(libprotobuf-lite ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(libprotobuf-lite PUBLIC ${protobuf_source_dir}/src)

if(MSVC AND protobuf_BUILD_SHARED_LIBS)
  target_compile_definitions(libprotobuf-lite
    PUBLIC  PROTOBUF_USE_DLLS
    PRIVATE LIBPROTOBUF_EXPORTS)
endif()
set_target_properties(libprotobuf-lite PROPERTIES
    VERSION ${protobuf_VERSION}
    OUTPUT_NAME ${LIB_PREFIX}protobuf-lite
    DEBUG_POSTFIX "${protobuf_DEBUG_POSTFIX}")
add_library(protobuf::libprotobuf-lite ALIAS libprotobuf-lite)

IF(protobuf_BUILD_SHARED_LIBS)
  SET_TARGET_PROPERTIES(libprotobuf-lite PROPERTIES
    DEBUG_POSTFIX ""
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/library_output_directory
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/library_output_directory
    )

  IF(LINUX)
    SET_TARGET_PROPERTIES(libprotobuf-lite
      PROPERTIES LINK_FLAGS
      "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libprotobuf.ver"
      )
  ENDIF()

  IF(WIN32)
    ADD_CUSTOM_COMMAND(TARGET libprotobuf-lite POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_BINARY_DIR}/library_output_directory/${CMAKE_CFG_INTDIR}/$<TARGET_FILE_NAME:libprotobuf-lite>"
      "${CMAKE_BINARY_DIR}/runtime_output_directory/${CMAKE_CFG_INTDIR}/$<TARGET_FILE_NAME:libprotobuf-lite>"
      )

    SET_TARGET_PROPERTIES(libprotobuf-lite PROPERTIES
      DEBUG_POSTFIX "-debug")
    INSTALL_DEBUG_TARGET(libprotobuf-lite
      DESTINATION "${INSTALL_BINDIR}"
      COMPONENT SharedLibraries
      )
  ENDIF()

  INSTALL_PRIVATE_LIBRARY(libprotobuf-lite)

  IF(WITH_ROUTER)
    IF(APPLE OR WIN32)
      INSTALL(TARGETS libprotobuf-lite
        DESTINATION "${ROUTER_INSTALL_PLUGINDIR}" COMPONENT Router
        )
    ELSEIF(UNIX)
      INSTALL(TARGETS libprotobuf-lite
        LIBRARY
        DESTINATION "${ROUTER_INSTALL_LIBDIR}"
        COMPONENT Router
        NAMELINK_SKIP
        )
    ENDIF()
  ENDIF()

ENDIF()
