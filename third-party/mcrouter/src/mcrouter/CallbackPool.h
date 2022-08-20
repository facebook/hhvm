/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>
#include <memory>

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * Simple thread-safe class to handle pool of callbacks.
 * One can register callback with 'subscribe' and call all registered callbacks
 * with 'notify', order of calls in undefined. Exceptions in callbacks are
 * caught and logged.
 *
 * @param Args arguments to pass to callbacks on 'notify'.
 *        NOTE: arguments would be copied for each callback. Use const reference
 *        or pointer type to avoid copy.
 */
template <typename... Args>
class CallbackPool {
 private:
  struct CallbackHandleImpl;
  struct Data;

 public:
  /**
   * Callback function for this CallbackPool.
   */
  typedef std::function<void(Args...)> OnUpdateFunc;

  /**
   * Callback handle for this CallbackPool. Once it is destroyed corresponding
   * callback will be unsubscribed.
   */
  typedef std::unique_ptr<CallbackHandleImpl> CallbackHandle;

  CallbackPool();

  /**
   * Adds one more callback function. This callback will be called
   * on next 'notify' call. The callback is unsubscribed when returned
   * CallbackHandle is destroyed.
   */
  CallbackHandle subscribe(OnUpdateFunc callback);

  /**
   * Call all subscribed callbacks. Order of callbacks is undefined.
   */
  void notify(Args... args);

  /**
   * Return true if no callback subscribed in the CallbackPool
   */
  bool empty();

 private:
  std::shared_ptr<Data> data_;
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "CallbackPool-inl.h"
