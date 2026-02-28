/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/AsyncTimeoutSet.h>

#include <folly/ScopeGuard.h>
#include <folly/io/async/Request.h>

using std::chrono::milliseconds;

namespace proxygen {

class SimpleTimeoutClock : public AsyncTimeoutSet::TimeoutClock {
 public:
  std::chrono::milliseconds millisecondsSinceEpoch() override {
    return proxygen::millisecondsSinceEpoch();
  }
};

AsyncTimeoutSet::TimeoutClock& getTimeoutClock() {
  static SimpleTimeoutClock timeoutClock;

  return timeoutClock;
}

AsyncTimeoutSet::Callback::~Callback() {
  if (isScheduled()) {
    cancelTimeout();
  }
}

void AsyncTimeoutSet::Callback::setScheduled(AsyncTimeoutSet* timeoutSet,
                                             Callback* prev) {
  assert(timeoutSet_ == nullptr);
  assert(prev_ == nullptr);
  assert(next_ == nullptr);
  assert(!timePointInitialized(expiration_));

  timeoutSet_ = timeoutSet;
  prev_ = prev;
  next_ = nullptr;
  expiration_ = timeoutSet->timeoutClock_.millisecondsSinceEpoch() +
                timeoutSet_->getInterval();
}

void AsyncTimeoutSet::Callback::cancelTimeoutImpl() {
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
  prev_ = nullptr;
  next_ = nullptr;
  expiration_ = {};
}

AsyncTimeoutSet::AsyncTimeoutSet(folly::TimeoutManager* timeoutManager,
                                 milliseconds intervalMS,
                                 milliseconds atMostEveryN,
                                 TimeoutClock* timeoutClock)
    : folly::AsyncTimeout(timeoutManager),
      timeoutClock_(timeoutClock ? *timeoutClock : getTimeoutClock()),
      head_(nullptr),
      tail_(nullptr),
      interval_(intervalMS),
      atMostEveryN_(atMostEveryN) {
}

AsyncTimeoutSet::AsyncTimeoutSet(folly::TimeoutManager* timeoutManager,
                                 InternalEnum internal,
                                 milliseconds intervalMS,
                                 milliseconds atMostEveryN)
    : folly::AsyncTimeout(timeoutManager, internal),
      timeoutClock_(getTimeoutClock()),
      head_(nullptr),
      tail_(nullptr),
      interval_(intervalMS),
      atMostEveryN_(atMostEveryN) {
}

AsyncTimeoutSet::~AsyncTimeoutSet() {
  // DelayedDestruction should ensure that we are never destroyed while inside
  // a call to timeoutExpired().
  assert(!inTimeoutExpired_);

  // destroy() should have already cleared out the timeout list.
  // It's a bug if anyone tries to keep using the AsyncTimeoutSet after
  // calling destroy, so no new timeouts may have been scheduled since then.
  assert(head_ == nullptr);
  assert(tail_ == nullptr);
}

void AsyncTimeoutSet::destroy() {
  // If there are any timeout callbacks pending, get rid of them without ever
  // invoking them.  This is somewhat undesirable from the callback's
  // perspective (how is it supposed to know that it will never get invoked?).
  // Most users probably only want to destroy a AsyncTimeoutSet when it has no
  // callbacks remaining.  Otherwise they need to implement their own code to
  // take care of cleaning up the callbacks that will never be invoked.

  // cancel from tail to head, to avoid extra calls to headChanged
  while (tail_ != nullptr) {
    tail_->cancelTimeout();
  }

  DelayedDestruction::destroy();
}

void AsyncTimeoutSet::scheduleTimeout(Callback* callback) {
  // Cancel the callback if it happens to be scheduled already.
  callback->cancelTimeout();
  assert(callback->prev_ == nullptr);
  assert(callback->next_ == nullptr);

  callback->context_ = folly::RequestContext::saveContext();

  Callback* old_tail = tail_;
  if (head_ == nullptr) {
    // We don't have any timeouts scheduled already.  We have to schedule
    // ourself.
    assert(tail_ == nullptr);
    assert(!isScheduled());
    if (!inTimeoutExpired_) {
      this->folly::AsyncTimeout::scheduleTimeout(interval_.count());
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

void AsyncTimeoutSet::headChanged() {
  if (inTimeoutExpired_) {
    // timeoutExpired() will always update the scheduling correctly before it
    // returns.  No need to change the state now, since we are just going to
    // change it again later.
    return;
  }

  if (!head_) {
    this->folly::AsyncTimeout::cancelTimeout();
  } else {
    milliseconds delta =
        head_->getTimeRemaining(timeoutClock_.millisecondsSinceEpoch());
    this->folly::AsyncTimeout::scheduleTimeout(delta.count());
  }
}

void AsyncTimeoutSet::timeoutExpired() noexcept {
  // If destroy() is called inside timeoutExpired(), delay actual destruction
  // until timeoutExpired() returns
  DestructorGuard dg(this);

  // timeoutExpired() can only be invoked directly from the event base loop.
  // It should never be invoked recursively.
  //
  // Set inTimeoutExpired_ to true, so that we won't bother rescheduling the
  // main AsyncTimeout inside timeoutExpired().  We'll always make sure this
  // is up-to-date before we return.  This simply prevents us from
  // unnecessarily modifying the main timeout heap multiple times before we
  // return.
  assert(!inTimeoutExpired_);
  inTimeoutExpired_ = true;
  SCOPE_EXIT {
    inTimeoutExpired_ = false;
  };

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
  auto now = timeoutClock_.millisecondsSinceEpoch();

  while (head_ != nullptr) {
    milliseconds delta = head_->getTimeRemaining(now);
    if (delta > milliseconds(0)) {
      if (delta < atMostEveryN_) {
        delta = atMostEveryN_;
      }
      this->folly::AsyncTimeout::scheduleTimeout(delta.count());
      break;
    }

    // Remember the callback to invoke, since calling cancelTimeout()
    // on it will modify head_.
    Callback* cb = head_;
    head_->cancelTimeout();
    folly::RequestContextScopeGuard rctxScopeGuard(cb->context_);
    cb->timeoutExpired();
  }
}

} // namespace proxygen
