/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/coro/Transport.h>

namespace folly::coro {
class ReadCallback;
};

namespace proxygen::coro::detail {

/**
 * A transport that implements ::detachEvb & ::attachEvb; assumes the parent
 * task awaits ::read() on a DetachableExecutor
 */
class Transport : public folly::coro::Transport {
  using folly::coro::Transport::Transport;

 private:
  void detachEventBase() override;
  void attachEventBase(folly::EventBase* evb) override;
  // represents the suspended read callback; set by detachEvb & reset by
  // attachEvb
  folly::coro::ReadCallback* readCB_{nullptr};
};

} // namespace proxygen::coro::detail
