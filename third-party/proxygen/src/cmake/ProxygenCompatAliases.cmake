# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

# Helper macro to create a compat alias with multiple dependencies
macro(proxygen_compat_alias _name)
  add_library(${_name} INTERFACE)
  target_link_libraries(${_name} INTERFACE ${ARGN})
  install(TARGETS ${_name} EXPORT proxygen-exports)
  add_library(proxygen::${_name} ALIAS ${_name})
endmacro()

# =============================================================================
# Backwards compatibility aliases (legacy names used by downstream projects)
# =============================================================================

# quicwebtransport: used by moxygen for QUIC WebTransport support
proxygen_compat_alias(quicwebtransport
  proxygen_http_webtransport_quicwebtransport
)

# =============================================================================
# Bundle aliases (group multiple granular targets for convenience)
#
# See TARGETS.txt for the rationale behind these groupings.
# =============================================================================

# Core HTTP types and utilities
proxygen_compat_alias(proxygen_http_core
  proxygen_error
  proxygen_http_status_type
  proxygen_http_http_utils
  proxygen_http_types
)

# HTTP/1.1 and HTTP/2 codecs
proxygen_compat_alias(proxygen_codec
  proxygen_http_codec
  proxygen_http_codec_direction
  proxygen_http_codec_error_code
  proxygen_http_codec_util
)

# HQ codec, framer, H3 errors
proxygen_compat_alias(proxygen_hq_core
  proxygen_http_codec_hq_codec
  proxygen_http_h3_errors
)

# HPACK compression
proxygen_compat_alias(proxygen_hpack
  proxygen_http_codec_compress_hpack
)

# QPACK compression
proxygen_compat_alias(proxygen_qpack
  proxygen_http_codec_compress_qpack
)

# Session base
proxygen_compat_alias(proxygen_http_session
  proxygen_http_session_session
  proxygen_http_session_http_transaction
)

# HQ Session
proxygen_compat_alias(proxygen_hq_session
  proxygen_http_session_hq_session
  proxygen_http_session_hq_upstream_session
  proxygen_http_session_hq_downstream_session
)

# WebTransport
proxygen_compat_alias(proxygen_webtransport
  proxygen_http_webtransport
)

# Combined wt_stream_manager + wt_egress_container
proxygen_compat_alias(proxygen_webtransport_helpers
  proxygen_http_webtransport_wt_stream_manager
  proxygen_http_webtransport_wt_egress_container
)

# Coro client
proxygen_compat_alias(proxygen_coro_client
  proxygen_http_coro_client_http_client_lib
  proxygen_http_coro_client_http_client_connection_cache
)

# Coro server
proxygen_compat_alias(proxygen_coro_server
  proxygen_http_coro_server_coro_acceptor
  proxygen_http_coro_server_coro_httpserver
)

# Coro filters
proxygen_compat_alias(proxygen_coro_filters
  proxygen_http_coro_filters_compression_filter
  proxygen_http_coro_filters_decompression_filter
)

# Connpool
proxygen_compat_alias(proxygen_connpool
  proxygen_http_connpool
  proxygen_http_connpool_session_holder
)

# Utils
proxygen_compat_alias(proxygen_utils
  proxygen_utils_time_util
  proxygen_utils_url
  proxygen_utils_util
)
