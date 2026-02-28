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
#include <utility>
#include <vector>

#include <folly/Random.h>

#include "mcrouter/lib/CacheClientStats.h"
#include "mcrouter/lib/carbon/connection/CarbonConnectionUtil.h"

namespace carbon {

template <class If>
class PooledCarbonConnectionImpl {
 public:
  explicit PooledCarbonConnectionImpl(
      std::vector<std::unique_ptr<If>> connections,
      bool splitBatchedRequests = false)
      : connections_(std::move(connections)),
        splitBatchedRequests_(splitBatchedRequests) {}

  template <class Request>
  void sendRequestOne(const Request& req, RequestCb<Request> cb) {
    connections_[folly::Random::rand32(connections_.size())]->sendRequestOne(
        req, std::move(cb));
  }

  template <class Request>
  void sendRequestMulti(
      std::vector<std::reference_wrapper<const Request>>&& reqs,
      RequestCb<Request> cb) {
    if (splitBatchedRequests_) {
      auto clientId = folly::Random::rand32(connections_.size());
      for (const Request& req : reqs) {
        connections_[clientId]->sendRequestOne(req, cb);
        clientId = (clientId + 1) % connections_.size();
      }
    } else {
      connections_[folly::Random::rand32(connections_.size())]
          ->sendRequestMulti(std::move(reqs), std::move(cb));
    }
  }

  facebook::memcache::CacheClientCounters getStatCounters() const noexcept {
    facebook::memcache::CacheClientCounters ret;
    for (auto& connection : connections_) {
      ret += connection->getStatCounters();
    }
    return ret;
  }

  std::unordered_map<std::string, std::string> getConfigOptions() {
    return std::unordered_map<std::string, std::string>();
  }

  bool healthCheck() {
    for (auto& connection : connections_) {
      if (!connection->healthCheck()) {
        return false;
      }
    }
    return true;
  }

  template <class Impl>
  std::unique_ptr<If> recreate() {
    std::vector<std::unique_ptr<If>> newConnections;
    for (size_t i = 0; i < connections_.size(); ++i) {
      newConnections.push_back(connections_[i]->recreate());
    }
    return std::make_unique<Impl>(
        std::move(newConnections), splitBatchedRequests_);
  }

 private:
  std::vector<std::unique_ptr<If>> connections_;
  bool splitBatchedRequests_;
};
} // namespace carbon
