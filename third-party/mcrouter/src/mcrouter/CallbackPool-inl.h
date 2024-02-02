/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <set>

#include <folly/SharedMutex.h>
#include <glog/logging.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

/* CallbackPool::CallbackHandle */
template <typename... Args>
struct CallbackPool<Args...>::CallbackHandleImpl {
 public:
  CallbackHandleImpl(const CallbackHandleImpl&) = delete;
  CallbackHandleImpl& operator=(const CallbackHandleImpl&) = delete;
  ~CallbackHandleImpl() {
    std::unique_lock lck(data_->callbackLock);
    data_->callbacks.erase(this);
  }

 private:
  friend class CallbackPool;

  std::shared_ptr<Data> data_;
  const OnUpdateFunc func_;

  CallbackHandleImpl(std::shared_ptr<Data> data, OnUpdateFunc func)
      : data_(std::move(data)), func_(std::move(func)) {
    std::unique_lock lck(data_->callbackLock);
    data_->callbacks.insert(this);
  }
};

/* CallbackPool::Data */
template <typename... Args>
struct CallbackPool<Args...>::Data {
  std::set<CallbackHandleImpl*> callbacks;
  mutable folly::SharedMutex callbackLock;
};

/* CallbackPool */

template <typename... Args>
CallbackPool<Args...>::CallbackPool() : data_(std::make_shared<Data>()) {}

template <typename... Args>
void CallbackPool<Args...>::notify(Args... args) {
  std::shared_lock lck(data_->callbackLock);
  for (auto& it : data_->callbacks) {
    try {
      it->func_(args...);
    } catch (const std::exception& e) {
      LOG(ERROR) << "Error occured in callback: " << e.what();
    } catch (...) {
      LOG(ERROR) << "Unknown error occured in callback";
    }
  }
}

template <typename... Args>
typename CallbackPool<Args...>::CallbackHandle CallbackPool<Args...>::subscribe(
    OnUpdateFunc callback) {
  return std::unique_ptr<CallbackHandleImpl>(
      new CallbackHandleImpl(data_, std::move(callback)));
}

template <typename... Args>
bool CallbackPool<Args...>::empty() {
  std::shared_lock lck(data_->callbackLock);
  return data_->callbacks.empty();
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
