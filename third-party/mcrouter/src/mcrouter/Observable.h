/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>

#include <folly/SharedMutex.h>

#include "mcrouter/CallbackPool.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * Wrapper for Data that allows to register callbacks that will be called
 * on each Data change.
 */
template <class Data>
class Observable {
 public:
  /**
   * Callback function for this Observable. It has two arguments:
   * oldData and newData.
   *
   * @param oldData - value before update
   * @param newData - value after update
   */
  typedef typename CallbackPool<const Data&, const Data&>::OnUpdateFunc
      OnUpdateOldNew;

  /**
   * Callback handle for this Observable. Once it is destroyed corresponding
   * callback will be unsubscribed.
   */
  typedef typename CallbackPool<const Data&, const Data&>::CallbackHandle
      CallbackHandle;

  explicit Observable(Data data = Data());

  /**
   * Adds one more callback function. This callback will be called next time
   * the data is updated. The callback is unsubscribed when returned
   * CallbackHandle is destroyed.
   * NOTE: callbacks can run in parallel. It is up to user to synchronize
   * callbacks.
   */
  inline CallbackHandle subscribe(OnUpdateOldNew callback);

  /**
   * Same as subscribe, but immediatelly calls the callback with
   * Data() as oldData and stored data as newData.
   */
  inline CallbackHandle subscribeAndCall(OnUpdateOldNew callback);

  /**
   * Update stored data. Both data change and callback calls are processed under
   * write lock on data. So one can assume data change and callback calls is an
   * atomic operation.
   */
  inline void set(Data data);

  /**
   * Same as set, but constructs Data inside.
   *
   * @param Args arguments for Data constructor
   */
  template <typename... Args>
  inline void emplace(Args&&... args);

  /**
   * @return copy of stored data
   */
  inline Data get();

 private:
  CallbackPool<const Data&, const Data&> pool_;
  Data data_;

  mutable folly::SharedMutex dataLock_;
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook

#include "Observable-inl.h"
