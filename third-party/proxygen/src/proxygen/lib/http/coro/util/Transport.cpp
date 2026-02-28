/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/io/async/EventBase.h>
#include <folly/io/coro/TransportCallbacks.h>
#include <folly/logging/xlog.h>

#include "proxygen/lib/http/coro/util/Transport.h"

namespace proxygen::coro::detail {

void Transport::detachEventBase() {
  XCHECK(eventBase_->isInEventBaseThread());
  XCHECK_EQ(readCB_, nullptr);
  /**
   * static_cast<folly::coro::ReadCallback> necessary to ::cancelTimeout below
   * and ::rescheduleTimeout in attachEventBase. This is ok because the contract
   * here is that folly::coro::Transport owns the AsyncTransport and is the only
   * code installing and uninstalling ReadCallbacks.
   */
  readCB_ =
      static_cast<folly::coro::ReadCallback*>(transport_->getReadCallback());
  XLOG(DBG6) << __func__ << "; readCB_=" << readCB_;
  transport_->setReadCB(nullptr); // ok if already uninstalled
  if (readCB_) {
    readCB_->cancelTimeout();
  }
  transport_->detachEventBase();
}

void Transport::attachEventBase(folly::EventBase* evb) {
  XLOG(DBG6) << __func__ << "; readCB_=" << readCB_;
  eventBase_ = evb;
  transport_->attachEventBase(evb);
  if (auto* readCB = std::exchange(readCB_, nullptr)) {
    transport_->setReadCB(readCB);
    readCB->scheduleTimeout(evb->timer());
  }
}

} // namespace proxygen::coro::detail
