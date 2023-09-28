/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "mcrouter/lib/CacheClientStats.h"
#include "mcrouter/lib/carbon/ExternalCarbonConnectionStats.h"
#include "mcrouter/lib/carbon/connection/CarbonConnectionUtil.h"
#include "mcrouter/lib/fbi/counting_sem.h"
#include "mcrouter/lib/network/AsyncMcClient.h"
#include "mcrouter/lib/network/ThriftTransport.h"

namespace carbon {

template <class Transport>
class Impl;

struct ExternalCarbonConnectionImplOptions {
  size_t maxOutstanding{0};
  bool maxOutstandingError{false};
  uint16_t portOverride{0};
  uint16_t thriftPortOverride{0};
  bool enableLogging{false};
  uint32_t hourlyLogRate{3600};
  uint32_t maxLogBurstSize{500};
  uint32_t logSampleRate{10000};
};

template <class RouterInfo>
class ExternalCarbonConnectionImpl {
 public:
  explicit ExternalCarbonConnectionImpl(
      facebook::memcache::ConnectionOptions connectionOptions,
      ExternalCarbonConnectionImplOptions options =
          ExternalCarbonConnectionImplOptions());

  ~ExternalCarbonConnectionImpl() = default;

  facebook::memcache::CacheClientCounters getStatCounters() const noexcept {
    // TODO: add real stats
    return {};
  }

  std::unordered_map<std::string, std::string> getConfigOptions() {
    // TODO:: add real options
    return std::unordered_map<std::string, std::string>();
  }

  bool healthCheck();

  template <class Request>
  void sendRequestOne(const Request& req, RequestCb<Request> cb);

  template <class Request>
  void sendRequestMulti(
      std::vector<std::reference_wrapper<const Request>>&& reqs,
      RequestCb<Request> cb);

  template <class T>
  std::unique_ptr<T> recreate() {
    LOG(FATAL)
        << "This should not be called, recreation is handled internally.";
    return nullptr; // unreachable, silence compiler errors
  }

 private:
  void makeImpl();

  facebook::memcache::ConnectionOptions connectionOptions_;
  ExternalCarbonConnectionImplOptions options_;
  ExternalCarbonConnectionLoggerOptions loggerOptions_;

  std::unique_ptr<Impl<facebook::memcache::AsyncMcClient>> carbonImpl_;
  std::unique_ptr<Impl<facebook::memcache::ThriftTransport<RouterInfo>>>
      thriftImpl_;
};
} // namespace carbon

#include "ExternalCarbonConnectionImpl-inl.h"
