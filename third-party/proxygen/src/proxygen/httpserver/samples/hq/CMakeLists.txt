# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

# Manually maintained - granular targets for HQ server libraries
# These break the dependency on monolithic proxygen for downstream projects (moxygen)
# Target names are simplified; CMAKE_TARGET_OVERRIDES in generate_cmake.py maps
# auto-generated dep names to these clean names.

proxygen_add_library(proxygen_hq_logger_helper
  SRCS
    HQLoggerHelper.cpp
  EXPORTED_DEPS
    mvfst::mvfst_logging_file_qlogger
    mvfst::mvfst_logging_qlogger
)

proxygen_add_library(proxygen_hq_insecure_verifier
  EXPORTED_DEPS
    fizz::fizz
)

proxygen_add_library(proxygen_hq_server
  SRCS
    FizzContext.cpp
    HQParams.cpp
    HQServer.cpp
  DEPS
    proxygen_hq_logger_helper
    proxygen_http_session_hq_downstream_session
    mvfst::mvfst_common_udpsocket_folly_async_udp_socket
    fizz::fizz
    Folly::folly_file_util
    Folly::folly_random
    Folly::folly_io_socket_option_map
  EXPORTED_DEPS
    proxygen_http_http_headers
    proxygen_http_codec_codec
    proxygen_http_session_hq_session
    proxygen_http_session_http_transaction
    proxygen_http_session_server
    mvfst::mvfst_constants
    mvfst::mvfst_api_stream_async_transport
    mvfst::mvfst_api_transport
    mvfst::mvfst_common_events_folly_eventbase
    mvfst::mvfst_congestion_control_congestion_controller_factory
    mvfst::mvfst_fizz_client_handshake_psk_cache
    mvfst::mvfst_server_quic_handshake_socket_holder
    mvfst::mvfst_server_server
    mvfst::mvfst_state_transport_settings
    wangle::wangle_ssl_ssl_config
    Folly::folly_network_address
    Folly::folly_optional
    Folly::folly_io_async_async_base
)

proxygen_add_library(proxygen_hq_samples
  SRCS
    H2Server.cpp
    HQClient.cpp
    HQCommandLine.cpp
    HQServerModule.cpp
    SampleHandlers.cpp
  DEPS
    proxygen_hq_logger_helper
    proxygen_hq_insecure_verifier
    proxygen_http_synchronized_quic_lrucache
    proxygen_http_session_hq_session
    proxygen_webtransport
    proxygen_transport_persistent_quic_psk_cache
    proxygen_utils_logging
    proxygen_utils_util_inl
    mvfst::mvfst_constants
    mvfst::mvfst_client_client
    mvfst::mvfst_common_udpsocket_folly_async_udp_socket
    mvfst::mvfst_congestion_control_congestion_controller_factory
    mvfst::mvfst_fizz_client_handshake
    Folly::folly_io_async_scoped_event_base_thread
    Folly::folly_json_dynamic
    Folly::folly_portability_gflags
  EXPORTED_DEPS
    proxygen_hq_server
    proxygenhttpserver
    proxygen_hq_devious_baton
    proxygencurl
    proxygen_http_http_utils
    proxygen_http_message
    proxygen_http_codec_codec
    proxygen_http_session_hq_upstream_session
    proxygen_http_session_http_transaction
    proxygen_http_session_client
    proxygen_utils_safe_path
    mvfst::mvfst_api_stream_async_transport
    mvfst::mvfst_api_transport
    mvfst::mvfst_common_events_folly_eventbase
    mvfst::mvfst_common_events_highres_quic_timer
    Folly::folly_conv
    Folly::folly_file
    Folly::folly_file_util
    Folly::folly_format
    Folly::folly_memory
    Folly::folly_random
    Folly::folly_string
    Folly::folly_synchronized
    Folly::folly_thread_local
    Folly::folly_executors_global_executor
    Folly::folly_futures_core
    Folly::folly_io_iobuf
    Folly::folly_io_async_async_base
    Folly::folly_io_async_event_base_manager
    ${GFLAG_DEPENDENCIES}
    glog::glog
)

add_subdirectory(devious)
