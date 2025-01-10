/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Exception.h>
#include <folly/String.h>
#include <folly/small_vector.h>
#include <memory>

#include "squangle/mysql_client/ConnectionOptions.h"
#include "squangle/mysql_client/Flags.h"

namespace facebook::common::mysql_client {

class Operation;

ConnectionOptions::ConnectionOptions()
    : connection_timeout_(FLAGS_async_mysql_connect_timeout_micros),
      total_timeout_(FLAGS_async_mysql_timeout_micros * 2),
      query_timeout_(FLAGS_async_mysql_timeout_micros) {}

std::string ConnectionOptions::getDisplayString() const {
  // Reserve 4 + 2 extra elements
  folly::small_vector<std::string, 6> parts;

  parts.push_back(
      fmt::format("conn timeout={}us", connection_timeout_.count()));
  parts.push_back(fmt::format("query timeout={}us", query_timeout_.count()));
  parts.push_back(fmt::format("total timeout={}us", total_timeout_.count()));
  parts.push_back(fmt::format("conn attempts={}", max_attempts_));
  if (dscp_.has_value()) {
    parts.push_back(fmt::format("outbound dscp={}", *dscp_));
  }
  if (ssl_options_provider_ != nullptr) {
    parts.push_back(fmt::format(
        "SSL options provider={}", (void*)ssl_options_provider_.get()));
  }
  if (compression_lib_.has_value()) {
    parts.push_back(fmt::format(
        "compression library={}", (void*)compression_lib_.get_pointer()));
  }

  if (!attributes_.empty()) {
    std::vector<std::string> substrings;
    for (const auto& [key, value] : attributes_) {
      substrings.push_back(fmt::format("{}={}", key, value));
    }

    parts.push_back(fmt::format(
        "connection attributes=[{}]", folly::join(",", substrings)));
  }
  return fmt::format("({})", folly::join(", ", parts));
}

// Sets the differentiated service code point (DSCP) on the underlying
// connection, which has the effect of embedding it into outgoing packet ip
// headers. The value may be used to classify and prioritize said traffic.
//
// Note: A DSCP value is 6 bits and is packed into an 8 bit field. Users must
// specify the unpacked (unshifted) 6-bit value.
//
// Note: This implementation only supports IPv6.
//
// Also known as "Quality of Service" (QoS), "Type of Service", "Class of
// Service" (COS).
//
// See also RFC 2474 [0] and RFC 3542 6.5 (IPv6 sockopt) [1]
// [0]: https://tools.ietf.org/html/rfc2474
// [1]: https://tools.ietf.org/html/rfc3542#section-6.5
ConnectionOptions& ConnectionOptions::setDscp(uint8_t dscp) {
  CHECK_THROW((dscp & 0b11000000) == 0, std::invalid_argument);
  dscp_ = dscp;
  return *this;
}

ConnectionOptions& ConnectionOptions::setCertValidationCallback(
    CertValidatorCallback callback) noexcept {
  certValidationCallback_ = std::move(callback);
  return *this;
}

} // namespace facebook::common::mysql_client
