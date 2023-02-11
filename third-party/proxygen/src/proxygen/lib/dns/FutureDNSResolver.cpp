/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/dns/FutureDNSResolver.h"

namespace proxygen {
namespace {
class FutureDNSResolutionCallback : public DNSResolver::ResolutionCallback {
 public:
  explicit FutureDNSResolutionCallback(
      folly::Promise<std::vector<DNSResolver::Answer>> promise)
      : promise_{std::move(promise)} {
  }

  void resolutionError(const folly::exception_wrapper& ew) noexcept override {
    promise_.setException(ew);
    delete this;
  }

  void resolutionSuccess(
      std::vector<DNSResolver::Answer> answers) noexcept override {
    promise_.setValue(std::move(answers));
    delete this;
  }

 private:
  folly::Promise<std::vector<DNSResolver::Answer>> promise_;
};
} // namespace

FutureDNSResolver::FutureDNSResolver(folly::EventBase* evb,
                                     DNSResolver::UniquePtr resolver)
    : evb_{evb}, resolver_{std::move(resolver)} {
  CHECK_NE(static_cast<folly::EventBase*>(nullptr), evb_)
      << "EventBase must not be null";
  CHECK_NE(static_cast<DNSResolver*>(nullptr), resolver_.get())
      << "DNS resolver must not be null";
}

FutureDNSResolver::~FutureDNSResolver() = default;

folly::Future<std::vector<DNSResolver::Answer>>
FutureDNSResolver::resolveAddress(const folly::SocketAddress& address,
                                  std::chrono::milliseconds timeout) {
  folly::Promise<std::vector<DNSResolver::Answer>> promise;
  auto future = promise.getFuture();
  auto callback = new FutureDNSResolutionCallback(std::move(promise));
  evb_->runInEventBaseThread(
      [address, callback, resolver = resolver_, timeout]() {
        resolver->resolveAddress(callback, address, timeout);
      });
  return future;
}

folly::Future<std::vector<DNSResolver::Answer>>
FutureDNSResolver::resolveHostname(const std::string& name,
                                   std::chrono::milliseconds timeout,
                                   sa_family_t family,
                                   TraceEventContext teCtx) {
  folly::Promise<std::vector<DNSResolver::Answer>> promise;
  auto future = promise.getFuture();
  auto callback = new FutureDNSResolutionCallback(std::move(promise));
  evb_->runInEventBaseThread(
      [callback, family, name, resolver = resolver_, teCtx, timeout]() {
        resolver->resolveHostname(callback, name, timeout, family, teCtx);
      });
  return future;
}

DNSResolver::StatsCollector* FutureDNSResolver::getStatsCollector() const {
  return resolver_->getStatsCollector();
}
void FutureDNSResolver::setStatsCollector(
    DNSResolver::StatsCollector* collector) {
  resolver_->setStatsCollector(collector);
}

} // namespace proxygen
