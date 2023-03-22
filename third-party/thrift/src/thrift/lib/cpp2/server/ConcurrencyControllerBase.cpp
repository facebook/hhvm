/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <atomic>
#include <folly/Indestructible.h>
#include <folly/Synchronized.h>
#include <folly/synchronization/CallOnce.h>
#include <thrift/lib/cpp2/server/ConcurrencyControllerBase.h>

namespace apache::thrift {

using Observer = ConcurrencyControllerBase::Observer;

auto& getObserverStorage() {
  class Storage {
   public:
    void set(std::shared_ptr<Observer> o) {
      folly::call_once(observerSetFlag_, [&] { instance_ = std::move(o); });
    }
    Observer* get() {
      if (!folly::test_once(observerSetFlag_)) {
        return {};
      }
      return instance_.get();
    }

   private:
    folly::once_flag observerSetFlag_;
    std::shared_ptr<Observer> instance_;
  };

  static auto observer = folly::Indestructible<Storage>();
  return *observer;
}

Observer* ConcurrencyControllerBase::getGlobalObserver() {
  return getObserverStorage().get();
}

void ConcurrencyControllerBase::setGlobalObserver(
    std::shared_ptr<Observer> observer) {
  getObserverStorage().set(std::move(observer));
}

void ConcurrencyControllerBase::setObserver(std::unique_ptr<Observer> ob) {
  observer_ = std::move(ob);
}

void ConcurrencyControllerBase::notifyOnFinishExecution(
    ServerRequest& request) {
  // use local observer
  if (observer_) {
    observer_->onFinishExecution(request);
  } else {
    // use global observer
    if (auto observer = getGlobalObserver()) {
      observer->onFinishExecution(request);
    }
  }
}

} // namespace apache::thrift
