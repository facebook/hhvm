/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/carbon/ExternalCarbonConnectionStats.h"

#pragma once

// Forward declaration
namespace facebook {
namespace memcache {
namespace mcrouter {
class ProxyRequestContext;
struct RequestLoggerContext;
class RequestClass;
} // namespace mcrouter
} // namespace memcache
} // namespace facebook

namespace carbon {

class NoopAdditionalLogger {
 public:
  explicit NoopAdditionalLogger(
      const facebook::memcache::mcrouter::ProxyRequestContext&) {}

  template <class Request>
  void logBeforeRequestSent(
      const Request&,
      const facebook::memcache::mcrouter::RequestLoggerContext&) {}

  template <class Request>
  void log(
      const Request&,
      const typename Request::reply_type&,
      const facebook::memcache::mcrouter::RequestLoggerContext&) {}

  template <class KeyType>
  bool mayLog(
      const carbon::Keys<KeyType>& /* unused */,
      const facebook::memcache::mcrouter::RequestClass& /* unused */,
      const carbon::Result& /* unused */,
      const int64_t /* unused */) const {
    return false;
  }
};

class NoopNoBeforeAdditionalLogger {
 public:
  explicit NoopNoBeforeAdditionalLogger(
      const facebook::memcache::mcrouter::ProxyRequestContext&) {}

  template <class Request>
  void log(
      const Request&,
      const typename Request::reply_type&,
      const facebook::memcache::mcrouter::RequestLoggerContext&) {}

  template <class KeyType>
  bool mayLog(
      const carbon::Keys<KeyType>& /* unused */,
      const facebook::memcache::mcrouter::RequestClass& /* unused */,
      const carbon::Result& /* unused */,
      const int64_t /* unused */) const {
    return false;
  }
};

class NoopExternalConnectionAdditionalLogger {
 public:
  explicit NoopExternalConnectionAdditionalLogger(
      carbon::ExternalCarbonConnectionLoggerOptions&) {}

  void log(carbon::ExternalCarbonConnectionStats&) {}

  bool shouldLog() const {
    return false;
  }

  void setEnabledStatus(bool /* unused */) {}
  bool getEnabledStatus() {
    return false;
  }
};

} // namespace carbon
