/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/coro/HTTPSourceFilter.h>

namespace proxygen::coro {

/**
 * An HTTPSourceFilter that ignores 1xx responses
 */
class Status1xxFilter : public HTTPSourceFilter {
 public:
  /**
   * Read header event from underlying HTTPSource silently ignoring any 1xx
   * responses
   */
  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override;
};
} // namespace proxygen::coro
