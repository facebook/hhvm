/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <mutex>

#include "mcrouter/lib/fbi/cpp/LogFailure.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

template <class Data>
Observable<Data>::Observable(Data data) : data_(std::move(data)) {}

template <class Data>
typename Observable<Data>::CallbackHandle Observable<Data>::subscribe(
    OnUpdateOldNew callback) {
  return pool_.subscribe(std::move(callback));
}

template <class Data>
typename Observable<Data>::CallbackHandle Observable<Data>::subscribeAndCall(
    OnUpdateOldNew callback) {
  std::shared_lock lck(dataLock_);
  try {
    callback(Data(), data_);
  } catch (const std::exception& e) {
    LOG_FAILURE(
        "mcrouter",
        failure::Category::kOther,
        "Error occured in observable callback: {}",
        e.what());
  } catch (...) {
    LOG_FAILURE(
        "mcrouter",
        failure::Category::kOther,
        "Unknown error occured in observable callback");
  }
  return subscribe(std::move(callback));
}

template <class Data>
Data Observable<Data>::get() {
  std::shared_lock lck(dataLock_);
  return data_;
}

template <class Data>
void Observable<Data>::set(Data data) {
  std::unique_lock lck(dataLock_);
  auto old = std::move(data_);
  data_ = std::move(data);
  // no copy here, because old and data are passed by const reference
  pool_.notify(old, data_);
}

template <class Data>
template <typename... Args>
void Observable<Data>::emplace(Args&&... args) {
  set(Data(std::forward<Args>(args)...));
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
