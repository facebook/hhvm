/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/filters/FilterFactory.h"

namespace proxygen::coro {

class ServerFilterFactory : public FilterFactory {
 public:
  virtual ~ServerFilterFactory() override = default;
  /**
   * Invoked in each thread server is going to handle requests
   * before we start handling requests. Can be used to setup
   * thread-local setup for each thread (stats and such).
   */
  virtual void onServerStart(folly::EventBase* evb) noexcept = 0;

  /**
   * Invoked in each handler thread after all the connections are
   * drained from that thread. Can be used to tear down thread-local setup.
   */
  virtual void onServerStop() noexcept = 0;
};

} // namespace proxygen::coro
