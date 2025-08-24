/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/util/DetachableExecutor.h"

#include <folly/logging/xlog.h>

namespace proxygen::coro::detail {

void DetachableExecutor::add(folly::Func fn) {
  XLOG(DBG8) << __func__ << "; pEvb_" << pEvb_;
  // must be "attached" and running in evb thread
  XCHECK(pEvb_ && pEvb_->isInEventBaseThread());
  auto loopCallback = std::make_unique<LoopCallback>(std::move(fn));
  fnList_.push_back(*loopCallback);
  pEvb_->runInLoop(loopCallback.release()); // deleted after invocation
}

void DetachableExecutor::detachEvb() {
  XLOG(DBG4) << __func__ << "; pEvb_=" << pEvb_;
  XCHECK(pEvb_->isInEventBaseThread());
  XCHECK_EQ(state_, State::Detachable);
  pEvb_ = nullptr;
  for (auto& loopCallback : fnList_) {
    loopCallback.cancelLoopCallback();
  }
}

void DetachableExecutor::attachEvb(folly::EventBase* evb) {
  XLOG(DBG4) << __func__ << "; evb=" << evb;
  XCHECK(evb->isInEventBaseThread());
  pEvb_ = evb;
  for (auto& loopCallback : fnList_) {
    pEvb_->runInLoop(&loopCallback);
  }
}

} // namespace proxygen::coro::detail
