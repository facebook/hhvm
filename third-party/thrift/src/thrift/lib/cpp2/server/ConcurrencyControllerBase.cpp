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

#include <folly/Indestructible.h>
#include <folly/Synchronized.h>
#include <folly/synchronization/CallOnce.h>
#include <thrift/lib/cpp2/server/ConcurrencyControllerBase.h>

namespace apache::thrift {

using Observer = ConcurrencyControllerInterface::Observer;

void ConcurrencyControllerBase::setObserver(
    std::shared_ptr<Observer> observer) {
  observer_ = std::move(observer);
}

void ConcurrencyControllerBase::notifyOnFinishExecution(
    ServerRequest& request) {
  if (observer_) {
    observer_->onFinishExecution(request);
  }
}

} // namespace apache::thrift
