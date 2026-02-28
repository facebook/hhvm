/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Memory.h>
#include <folly/portability/GTest.h>

#include "proxygen/lib/dns/SyncDNSResolver.h"
#include "proxygen/lib/dns/test/MockDNSResolver.h"

namespace proxygen {

class SyncDNSResolverTest : public testing::Test {
 public:
  void SetUp() override {
    resolver_ = new MockDNSResolver();
    syncResolver_ =
        std::make_unique<SyncDNSResolver>(DNSResolver::UniquePtr(resolver_));
    evb_ = syncResolver_->getEventBase();
  }

  void TearDown() override {
    syncResolver_.reset();
    evb_ = nullptr;
  }

 protected:
  MockDNSResolver* resolver_; // unowned
  std::unique_ptr<SyncDNSResolver> syncResolver_;
  folly::EventBase* evb_;
};

} // namespace proxygen
