/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include "thrift/lib/cpp/async/TAsyncTimeoutSet.h"

#include "thrift/lib/cpp/concurrency/Util.h"

#include "folly/ScopeGuard.h"

#include <cassert>

using apache::thrift::concurrency::Util;
using std::chrono::milliseconds;

namespace apache { namespace thrift { namespace async {

TAsyncTimeoutSet::Callback::~Callback() {
  if (isScheduled()) {
    cancelTimeout();
  }
}

void TAsyncTimeoutSet::Callback::setScheduled(TAsyncTimeoutSet* timeoutSet,
                                              Callback* prev) {
  assert(timeoutSet_ == nullptr);
  assert(expiration_ == milliseconds(0));
  assert(prev_ == nullptr);
  assert(next_ == nullptr);

  timeoutSet_ = timeoutSet;
  expiration_ = milliseconds(Util::currentTime()) + timeoutSet_->getInterval();
  prev_ = prev;
  next_ = nullptr;
}

void TAsyncTimeoutSet::Callback::cancelTimeoutImpl() {
  if (next_ == nullptr) {
    assert(timeoutSet_->tail_ == this);
    timeoutSet_->tail_ = prev_;
  } else {
    assert(timeoutSet_->tail_ != this);
    next_->prev_ = prev_;
  }

  if (prev_ == nullptr) {
    assert(timeoutSet_->head_ == this);
    timeoutSet_->head_ = next_;
    timeoutSet_->headChanged();
  } else {
    assert(timeoutSet_->head_ != this);
    prev_->next_ = next_;
  }

  timeoutSet_ = nullptr;
  expiration_ = milliseconds(0);
  prev_ = nullptr;
  next_ = nullptr;
}

TAsyncTimeoutSet::TAsyncTimeoutSet(TEventBase* eventBase,
                                   std::chrono::milliseconds intervalMS,
                                   std::chrono::milliseconds atMostEveryN)
  : TAsyncTimeout(eventBase),
    interval_(intervalMS),
    atMostEveryN_(atMostEveryN),
    inTimeoutExpired_(false),
    head_(nullptr),
    tail_(nullptr) {
}

TAsyncTimeoutSet::TAsyncTimeoutSet(TEventBase* eventBase,
                                   InternalEnum internal,
                                   std::chrono::milliseconds intervalMS,
                                   std::chrono::milliseconds atMostEveryN)
    : TAsyncTimeout(eventBase, internal),
      interval_(intervalMS),
      atMostEveryN_(atMostEveryN),
      inTimeoutExpired_(false),
      head_(nullptr),
      tail_(nullptr) {
}

TAsyncTimeoutSet::~TAsyncTimeoutSet() {
  // TDelayedDestruction should ensure that we are never destroyed while inside
  // a call to timeoutExpired().
  assert(!inTimeoutExpired_);

  // destroy() should have already cleared out the timeout list.
  // It's a bug if anyone tries to keep using the TAsyncTimeoutSet after
  // calling destroy, so no new timeouts may have been scheduled since then.
  assert(head_ == nullptr);
  assert(tail_ == nullptr);
}

void TAsyncTimeoutSet::destroy() {
  // If there are any timeout callbacks pending, get rid of them without ever
  // invoking them.  This is somewhat undesirable from the callback's
  // perspective (how is it supposed to know that it will never get invoked?).
  // Most users probably only want to destroy a TAsyncTimeoutSet when it has no
  // callbacks remaining.  Otherwise they need to implement their own code to
  // take care of cleaning up the callbacks that will never be invoked.

  while (head_ != nullptr) {
    head_->cancelTimeout();
  }

  TDelayedDestruction::destroy();
}

void TAsyncTimeoutSet::scheduleTimeout(Callback* callback) {
  // Cancel the callback if it happens to be scheduled already.
  callback->cancelTimeout();
  assert(callback->prev_ == nullptr);
  assert(callback->next_ == nullptr);

  callback->context_ = RequestContext::saveContext();

  Callback* old_tail = tail_;
  if (head_ == nullptr) {
    // We don't have any timeouts scheduled already.  We have to schedule
    // ourself.
    assert(tail_ == nullptr);
    assert(!isScheduled());
    if (!inTimeoutExpired_) {
      this->TAsyncTimeout::scheduleTimeout(interval_.count());
    }
    head_ = callback;
    tail_ = callback;
  } else {
    assert(inTimeoutExpired_ || isScheduled());
    assert(tail_->next_ == nullptr);
    tail_->next_ = callback;
    tail_ = callback;
  }

  // callback->prev_ = tail_;
  callback->setScheduled(this, old_tail);
}

void TAsyncTimeoutSet::headChanged() {
  if (inTimeoutExpired_) {
    // timeoutExpired() will always update the scheduling correctly before it
    // returns.  No need to change the state now, since we are just going to
    // change it again later.
    return;
  }

  if (!head_) {
    this->TAsyncTimeout::cancelTimeout();
  } else {
    milliseconds delta =
      head_->getTimeRemaining(milliseconds(Util::currentTime()));
    this->TAsyncTimeout::scheduleTimeout(delta.count());
  }
}

void TAsyncTimeoutSet::timeoutExpired() noexcept {
  // If destroy() is called inside timeoutExpired(), delay actual destruction
  // until timeoutExpired() returns
  DestructorGuard dg(this);

  // timeoutExpired() can only be invoked directly from the event base loop.
  // It should never be invoked recursively.
  //
  // Set inTimeoutExpired_ to true, so that we won't bother rescheduling the
  // main TAsyncTimeout inside timeoutExpired().  We'll always make sure this
  // is up-to-date before we return.  This simply prevents us from
  // unnecessarily modifying the main timeout heap multiple times before we
  // return.
  assert(!inTimeoutExpired_);
  inTimeoutExpired_ = true;
  SCOPE_EXIT { inTimeoutExpired_ = false; };

  // Get the current time.
  // For now we only compute the current time at the start of the loop.
  // If a callback takes a very long time to execute its timeoutExpired()
  // method, this value could potentially get stale.
  //
  // However, this should be rare, and it doesn't seem worth the overhead of
  // recomputing the current time each time around the loop.  If the value does
  // go stale, we won't invoke as many callbacks as we could.  They will have
  // to wait until the next call to timeoutExpired().  However, we could also
  // end up rescheduling the next timeoutExpired() call a bit late if now gets
  // stale.  If we find that this becomes a problem in practice we could be
  // more smart about when we recompute the current time.
  milliseconds now(Util::currentTime());

  while (head_ != nullptr) {
    milliseconds delta = head_->getTimeRemaining(now);
    if (delta > milliseconds(0)) {
      if (delta < atMostEveryN_) {
        delta = atMostEveryN_;
      }
      this->TAsyncTimeout::scheduleTimeout(delta.count());
      break;
    }

    // Remember the callback to invoke, since calling cancelTimeout()
    // on it will modify head_.
    Callback* cb = head_;
    head_->cancelTimeout();
    auto old_ctx =
      RequestContext::setContext(cb->context_);
    cb->timeoutExpired();
    RequestContext::setContext(old_ctx);
  }
}

}}} // apache::thrift::async
