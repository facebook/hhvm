/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/dns/DNSModule.h"

namespace proxygen {

class MockDNSModule : public DNSModule {
 public:
  MockDNSModule();

  ~MockDNSModule() override;

  void setMockDNSResolver(DNSResolver::UniquePtr r) {
    resolver = std::move(r);
  }

  DNSResolver::UniquePtr provideDNSResolver(
      folly::EventBase* /*eventBase*/) override {
    return std::move(resolver);
  }

 private:
  DNSResolver::UniquePtr resolver;
};

} // namespace proxygen
