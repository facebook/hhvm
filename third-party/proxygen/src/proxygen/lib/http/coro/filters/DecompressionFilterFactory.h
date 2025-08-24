/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/filters/FilterFactory.h"
#include "proxygen/lib/http/coro/filters/ServerFilterFactory.h"

namespace proxygen::coro {

class ClientDecompressionFilterFactory : public FilterFactory {
 public:
  std::pair<HTTPSourceFilter*, HTTPSourceFilter*> makeFilters() override;
};

class ServerDecompressionFilterFactory : public ServerFilterFactory {
 public:
  void onServerStart(folly::EventBase*) noexcept override {
  }

  void onServerStop() noexcept override {
  }

  std::pair<HTTPSourceFilter*, HTTPSourceFilter*> makeFilters() override;
};

} // namespace proxygen::coro
