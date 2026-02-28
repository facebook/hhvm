/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/util/CancellableBaton.h"
#include <folly/logging/xlog.h>

namespace proxygen::coro {

class Refcount {
 public:
  explicit Refcount(size_t n = 0) : count_(n) {
    if (count_ == 0) {
      baton_.signal();
    }
  }

  size_t count() const {
    return count_;
  }

  void incRef() {
    if (count_++ == 0) {
      baton_.reset();
    }
  }

  void decRef() {
    XCHECK_GT(count_, 0UL);
    if (--count_ == 0) {
      baton_.signal();
    }
  }

  folly::coro::Task<TimedBaton::Status> zeroRefs() {
    return baton_.wait();
  }

 protected:
  detail::CancellableBaton baton_;

 private:
  size_t count_;
};

} // namespace proxygen::coro
