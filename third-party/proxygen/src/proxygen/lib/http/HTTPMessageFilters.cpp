/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/HTTPMessageFilters.h>

namespace proxygen {

void HTTPMessageFilter::pause() noexcept {
  if (nextElementIsPaused_) {
    return;
  }

  nextElementIsPaused_ = true;

  if (prev_.which() == 0) {
    auto prev = boost::get<HTTPMessageFilter*>(prev_);
    if (prev) {
      prev->pause();
    }
  } else {
    auto prev = boost::get<HTTPSink*>(prev_);
    if (prev) {
      prev->pauseIngress();
    }
  }
}

void HTTPMessageFilter::resume(uint64_t offset) noexcept {
  nextElementIsPaused_ = false;
  if (prev_.which() == 0) {
    auto prev = boost::get<HTTPMessageFilter*>(prev_);
    if (prev) {
      prev->resume(offset);
    }
  } else {
    auto prev = boost::get<HTTPSink*>(prev_);
    if (prev) {
      prev->resumeIngress();
    }
  }
}

} // namespace proxygen
