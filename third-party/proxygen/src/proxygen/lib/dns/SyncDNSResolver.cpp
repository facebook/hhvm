/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/dns/SyncDNSResolver.h"

#include "proxygen/lib/dns/DNSModule.h"
#include "proxygen/lib/dns/NaiveResolutionCallback.h"
#include "proxygen/lib/dns/Rfc6724.h"

#include <barrier>

namespace proxygen {
namespace {

void startEventBaseThread(std::thread& thread, folly::EventBase& evb) {
  auto barrier = std::make_shared<std::barrier<>>(2);
  thread = std::thread([&evb, barrier]() mutable {
    evb.runInLoop([=]() mutable {
      barrier->arrive_and_wait();
      barrier.reset();
    });

    evb.loopForever();
    evb.loop();
  });

  barrier->arrive_and_wait();
}

} // namespace

// public constructor
SyncDNSResolver::SyncDNSResolver() {
  startEventBaseThread(thread_, evb_);
  resolver_ = DNSModule::get()->provideDNSResolver(&evb_);
}

SyncDNSResolver::SyncDNSResolver(DNSResolver::UniquePtr resolver) {
  // Start a thread running event loop before wiring up the resolver.
  startEventBaseThread(thread_, evb_);
  resolver_ = std::move(resolver);
}

// public destructor
SyncDNSResolver::~SyncDNSResolver() {
  evb_.runInEventBaseThread([this]() {
    resolver_.reset();
    evb_.terminateLoopSoon();
  });

  thread_.join();
}

// public
std::vector<folly::SocketAddress> SyncDNSResolver::resolveHostname(
    const std::string& hostname,
    std::chrono::milliseconds timeout, // 100ms
    sa_family_t family,                // = AF_UNSPEC,
    bool rfc6724sort /* = true */) {

  std::vector<folly::SocketAddress> addrs;
  folly::exception_wrapper ew;

  auto barrier = std::make_shared<std::barrier<>>(2);
  evb_.runInEventBaseThread([&, barrier]() mutable {
    auto cb = new NaiveResolutionCallback(
        [&, barrier](std::vector<DNSResolver::Answer>&& answers,
                     const folly::exception_wrapper&& exptr) {
          for (const auto& answer : answers) {
            if (answer.type == DNSResolver::Answer::AT_ADDRESS) {
              addrs.push_back(answer.address);
            }
          }
          if (addrs.empty()) {
            if (exptr) {
              ew = std::move(exptr);
            } else {
              ew = DNSResolver::makeNoNameException();
            }
          }
          barrier->arrive_and_wait();
        });

    resolver_->resolveHostname(cb, hostname, timeout, family);
  });

  barrier->arrive_and_wait();
  if (ew) {
    ew.throw_exception();
  }

  if (rfc6724sort) {
    rfc6724_sort(addrs);
  }
  return addrs;
}

} // namespace proxygen
