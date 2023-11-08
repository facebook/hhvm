/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <exception>

namespace proxygen {

// Based on the IETF Proxy-Status HTTP Header Field draft in
//   https://tools.ietf.org/html/draft-ietf-httpbis-proxy-status-00

#define STATUS_TYPES_GEN(x)                                                    \
  /* STANDARD STATUS TYPES */                                                  \
  x(connection_read_timeout), x(connection_refused), x(connection_terminated), \
      x(connection_timeout), x(connection_write_timeout),                      \
      x(connnection_limit_reached), x(destination_ip_prohibited),              \
      x(destination_ip_unroutable), x(destination_not_found),                  \
      x(destination_unavailable), x(dns_resolution_error), x(dns_error),       \
      x(dns_timeout), x(http_protocol_error), x(http_request_denied),          \
      x(http_request_error), x(http_response_body_size),                       \
      x(http_response_content_coding), x(http_response_header_block_size),     \
      x(http_response_header_size), x(http_response_incomplete),               \
      x(http_response_status), x(http_response_timeout),                       \
      x(http_response_transfer_coding), x(http_upgrade_failed),                \
      x(proxy_internal_response), x(proxy_internal_error),                     \
      x(proxy_loop_detected), x(tls_error), x(tls_expired_peer_certificate),   \
      x(tls_handshake_error), x(tls_missing_proxy_certificate),                \
      x(tls_rejected_proxy_certificate), x(tls_unexpected_peer_certificate),   \
      x(tls_unexpected_peer_identity), x(tls_untrusted_peer_certificate),      \
      x(http_response_ok), /* FB STATUS TYPES */ x(async_request_error),       \
      x(client_read_error), x(client_timeout), x(http_body_before_headers),    \
      x(http_body_parsing_error), x(http_eom_before_headers),                  \
      x(http_headers_parsing_error), x(http_partial_reliability_disabled),     \
      x(http_trailers_before_headers), x(no_server_available),                 \
      x(proxy_adaptive_rate_limit), x(request_rate_limited),                   \
      x(sc_channel_invalid_argument), x(sc_channel_unknown_error),             \
      x(sc_eom_before_headers), x(sc_upstream_timeout),                        \
      x(sc_runtime_exception), x(sc_content_integrity_error),                  \
      x(live_head_error), x(redirect_connect_error),                           \
      x(redirect_limit_exceeded), x(redirect_pool_error),                      \
      x(redirect_request_too_large), x(server_connection_error),               \
      x(server_timeout), x(server_write_error), x(telephoto_error),            \
      x(wasm_invocation_error), x(transcode_server_error),                     \
      x(fbvp_channel_error), x(server_internal_error), x(invalid_pool),        \
      x(qoe_error), x(sc_downstream_error), x(content_integrity),              \
      x(bad_request),                                                          \
      /* APP-SPECIFIC STATUS TYPES */ x(manifest_invalid_status),              \
      x(manifest_is_empty), x(manifest_parsing_error),                         \
      x(manifest_missing_representation), x(manifest_with_0_bitrate),          \
      x(manifest_with_no_tracks), x(manifest_with_wrong_track),                \
      x(cache_lease_queue_hard_timeout), x(cache_purge), x(cache_error),       \
      x(proxy_cache_fill_shed), x(ENUM_COUNT), x(takedown_direct_response)

#define STATUS_TYPE_ENUM(statusType) statusType

enum class StatusType { STATUS_TYPES_GEN(STATUS_TYPE_ENUM) };

const char* getStatusTypeString(StatusType statusType);

class ExceptionWithStatusType : public std::exception {
 public:
  ExceptionWithStatusType(int statusCode, StatusType statusType)
      : statusCode_(statusCode), statusType_(statusType) {
  }
  int getStatusCode() const {
    return statusCode_;
  }
  StatusType getStatusType() const {
    return statusType_;
  }
  const char* what() const noexcept {
    return getStatusTypeString(statusType_);
  }

 protected:
  int statusCode_;
  StatusType statusType_;
};

} // namespace proxygen
