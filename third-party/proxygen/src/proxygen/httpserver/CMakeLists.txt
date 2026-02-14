# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

# Manually maintained - generated base with manual additions for compat aliases
# and BUILD_SAMPLES logic. Run generate_cmake.py to update the library targets,
# then re-add the manual section below.

proxygen_add_library(proxygen_httpserver_httpserver_headers
  EXPORTED_DEPS
    proxygen_http_session_http_transaction
    proxygen_utils_compression_filter_utils
    proxygen_utils_stream_decompressor
    Folly::folly_expected
    Folly::folly_function
    Folly::folly_io_async_server_socket
    Folly::folly_network_address
    Folly::folly_scope_guard
)

proxygen_add_library(proxygen_httpserver_decompression_filter
  SRCS
    filters/DecompressionFilter.cpp
  DEPS
    proxygen_utils_zstd_stream_decompressor
  EXPORTED_DEPS
    proxygen_httpserver_httpserver_headers
    proxygen_utils_stream_decompressor
)

proxygen_add_library(proxygen_httpserver
  SRCS
    HTTPServer.cpp
    HTTPServerAcceptor.cpp
    RequestHandlerAdaptor.cpp
    SignalHandler.cpp
  DEPS
    proxygen_http_codec_codec_common
    proxygen_http_codec_http1x_codec
    wangle::wangle_ssl_ssl_context_manager
    Folly::folly_exception_string
    Folly::folly_executors_thread_factory_named_thread_factory
    Folly::folly_io_async_event_base_manager
    Folly::folly_range
    Folly::folly_system_hardware_concurrency
  EXPORTED_DEPS
    proxygen_http_codec_codec_factory
    proxygen_http_session_http_transaction
    proxygen_http_session_server
    proxygen_http_session_session
    proxygen_httpserver_httpserver_headers
    proxygen_services_acceptor_configuration
    wangle::wangle_bootstrap_server_bootstrap
    wangle::wangle_ssl_ssl_context_config
    Folly::folly_executors_io_thread_pool_executor
    Folly::folly_io_async_async_base
    Folly::folly_io_async_async_signal_handler
    Folly::folly_io_socket_option_map
)

proxygen_add_library(proxygen_httpserver_scoped_httpserver
  EXPORTED_DEPS
    proxygen_httpserver
    proxygen_httpserver_httpserver_headers
    Folly::folly_io_async_ssl_context
    Folly::folly_system_thread_name
)

proxygen_add_library(proxygen_httpserver_mocks
  EXPORTED_DEPS
    proxygen_httpserver_httpserver_headers
    Folly::folly_portability_gmock
)

# =============================================================================
# Manually maintained section below
# =============================================================================

# Include granular HQ server targets (unconditional - needed by moxygen)
add_subdirectory(samples/hq)

# Backwards compatibility alias: proxygenhttpserver -> proxygen_httpserver
add_library(proxygenhttpserver INTERFACE)
target_link_libraries(proxygenhttpserver INTERFACE proxygen_httpserver)
install(TARGETS proxygenhttpserver EXPORT proxygen-exports)
add_library(proxygen::proxygenhttpserver ALIAS proxygenhttpserver)

# Backwards compatibility alias for proxygenhqserver
add_library(proxygenhqserver INTERFACE)
target_link_libraries(proxygenhqserver INTERFACE
  proxygen_hq_server
  proxygen_hq_logger_helper
)
install(TARGETS proxygenhqserver EXPORT proxygen-exports)
add_library(proxygen::proxygenhqserver ALIAS proxygenhqserver)

# Backwards compatibility alias for proxygenhqloggerhelper
add_library(proxygenhqloggerhelper INTERFACE)
target_link_libraries(proxygenhqloggerhelper INTERFACE
  proxygen_hq_logger_helper
)
install(TARGETS proxygenhqloggerhelper EXPORT proxygen-exports)
add_library(proxygen::proxygenhqloggerhelper ALIAS proxygenhqloggerhelper)

if (BUILD_SAMPLES)
  add_executable(proxygen_push
    samples/push/PushServer.cpp
    samples/push/PushRequestHandler.cpp
  )
  target_compile_options(
      proxygen_push PRIVATE ${_PROXYGEN_COMMON_COMPILE_OPTIONS})
  target_link_libraries(proxygen_push PUBLIC proxygen proxygen_httpserver)
  install(TARGETS proxygen_push EXPORT proxygen-exports DESTINATION bin)

  add_executable(proxygen_proxy
    samples/proxy/ProxyServer.cpp
    samples/proxy/ProxyHandler.cpp
  )
  target_compile_options(
      proxygen_proxy PRIVATE ${_PROXYGEN_COMMON_COMPILE_OPTIONS})
  target_link_libraries(proxygen_proxy PUBLIC proxygen proxygen_httpserver)
  install(TARGETS proxygen_proxy EXPORT proxygen-exports DESTINATION bin)

  add_executable(proxygen_static
    samples/static/StaticServer.cpp
    samples/static/StaticHandler.cpp
  )
  target_compile_options(
      proxygen_static PRIVATE ${_PROXYGEN_COMMON_COMPILE_OPTIONS})
  target_link_libraries(proxygen_static PUBLIC proxygen proxygen_httpserver)
  install(TARGETS proxygen_static EXPORT proxygen-exports DESTINATION bin)

  add_executable(proxygen_echo
    samples/echo/EchoServer.cpp
    samples/echo/EchoHandler.cpp
  )
  target_compile_options(
      proxygen_echo PRIVATE ${_PROXYGEN_COMMON_COMPILE_OPTIONS})
  target_link_libraries(proxygen_echo PUBLIC proxygen proxygen_httpserver)
  install(TARGETS proxygen_echo EXPORT proxygen-exports DESTINATION bin)

  add_executable(hq
    samples/hq/main.cpp
  )
  target_compile_options(hq PRIVATE ${_PROXYGEN_COMMON_COMPILE_OPTIONS})
  target_link_libraries(hq
    PUBLIC
      proxygen_hq_samples
      proxygen_hq_server
      proxygen_transport_persistent_quic_psk_cache
      proxygen_httpserver
      Folly::folly_init_init
      Folly::folly_portability_gflags
  )
  install(TARGETS hq EXPORT proxygen-exports DESTINATION bin)
endif()

file(
  GLOB_RECURSE PROXYGEN_HTTPSERVER_HEADERS_TOINSTALL
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  *.h
)
list(FILTER PROXYGEN_HTTPSERVER_HEADERS_TOINSTALL EXCLUDE REGEX tests/)
foreach(header ${PROXYGEN_HTTPSERVER_HEADERS_TOINSTALL})
  get_filename_component(header_dir ${header} DIRECTORY)
  install(FILES ${header} DESTINATION include/proxygen/httpserver/${header_dir})
endforeach()

add_subdirectory(tests)
add_subdirectory(filters/tests)
#add_subdirectory(samples/echo/test)
#add_subdirectory(samples/hq/devious/test)
