/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/IntrusiveList.h>
#include <folly/io/async/EventBase.h>

namespace proxygen::coro::detail {

/**
 * An executor that allows changing the underlying eventbase via ::detachEvb and
 * ::attachEvb. Such semantics allows us to suspend all scheduled functions upon
 * detach, and re-schedule them upon attach. This is only possible by exploiting
 * the fact that ::add() via a coroutine executor *in HTTPCoroSession*
 * ::readLoop and ::writeLoop is only ever invoked from within the currently
 * attached EventBase.
 */
class DetachableExecutor : public folly::Executor {
 public:
  explicit DetachableExecutor(folly::EventBase* pEvb)
      : pEvb_(CHECK_NOTNULL(pEvb)) {
  }
  /**
   * DetachableGuard should only be acquired in strategic suspension points
   * (e.g. where a coroutine is most likely to be suspended waiting for some
   * external event to happen) that allow us to migrate eventbases.
   * Consequently, detachEvb should only be invoked when a DetachableGuard has
   * been acquired.
   */
  enum State : uint8_t { Undetachable = 0, Detachable };
  struct DetachableGuard {
    friend class DetachableExecutor;
    ~DetachableGuard() {
      stateRef_ = Undetachable;
    }

   private:
    explicit DetachableGuard(State& stateRef) : stateRef_(stateRef) {
      stateRef_ = Detachable;
    }

   private:
    State& stateRef_;
  };

  [[nodiscard]] State getState() const {
    return state_;
  }

  DetachableGuard acquireGuard() {
    return DetachableGuard{state_};
  }

  void add(folly::Func fn) override;

  void detachEvb();
  void attachEvb(folly::EventBase* evb);

 private:
  using FunctionLoopCallback = folly::EventBase::FunctionLoopCallback;
  struct LoopCallback : public FunctionLoopCallback {
    using FunctionLoopCallback::FunctionLoopCallback;
    ~LoopCallback() override = default;
    folly::IntrusiveListHook listHook_;
  };

  folly::EventBase* pEvb_;
  State state_{Undetachable};
  using FunctionList =
      folly::IntrusiveList<LoopCallback, &LoopCallback::listHook_>;
  FunctionList fnList_;
};

} // namespace proxygen::coro::detail
