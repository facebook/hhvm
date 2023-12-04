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

#include <algorithm>
#include <thrift/lib/cpp/EventHandlerBase.h>

using std::remove;
using std::shared_ptr;
using std::vector;

using RWMutex = folly::SharedMutex;
using RLock = RWMutex::ReadHolder;
using WLock = RWMutex::WriteHolder;

namespace apache {
namespace thrift {

void EventHandlerBase::addEventHandler(
    const std::shared_ptr<TProcessorEventHandler>& handler) {
  if (!handlers_) {
    handlers_ = std::make_shared<
        std::vector<std::shared_ptr<TProcessorEventHandler>>>();
  }
  handlers_->push_back(handler);
}

void EventHandlerBase::addNotNullEventHandler(
    const folly::not_null_shared_ptr<TProcessorEventHandler>& handler) {
  if (!handlers_) {
    handlers_ = std::make_shared<
        std::vector<std::shared_ptr<TProcessorEventHandler>>>();
  }

  handlers_->push_back(
      static_cast<const std::shared_ptr<TProcessorEventHandler>>(handler));
}

folly::Range<std::shared_ptr<TProcessorEventHandler>*>
EventHandlerBase::getEventHandlers() const {
  if (!handlers_) {
    return {};
  }
  return folly::range(*handlers_);
}

TProcessorBase::TProcessorBase() {}

void TProcessorBase::addProcessorEventHandler(
    std::shared_ptr<TProcessorEventHandler> handler) {
  if (!handler) {
    return;
  }
  WLock lock{getRWMutex()};
  assert(
      find(getHandlers().begin(), getHandlers().end(), handler) ==
      getHandlers().end());
  getHandlers().push_back(std::move(handler));
}

void TProcessorBase::removeProcessorEventHandler(
    std::shared_ptr<TProcessorEventHandler> handler) {
  WLock lock{getRWMutex()};
  assert(
      find(getHandlers().begin(), getHandlers().end(), handler) !=
      getHandlers().end());
  getHandlers().erase(
      remove(getHandlers().begin(), getHandlers().end(), handler),
      getHandlers().end());
}

RWMutex& TProcessorBase::getRWMutex() {
  static auto* mutex = new RWMutex{};
  return *mutex;
}

vector<folly::not_null_shared_ptr<TProcessorEventHandler>>&
TProcessorBase::getHandlers() {
  static vector<folly::not_null_shared_ptr<TProcessorEventHandler>> handlers;
  return handlers;
}

TClientBase::TClientBase() : TClientBase(Options()) {}

TClientBase::TClientBase(Options options) {
  if (!options.includeGlobalEventHandlers) {
    return;
  }

  // Automatically ask all registered factories to produce an event
  // handler, and attach the handlers
  RLock lock{getRWMutex()};

  auto& handlers = getHandlers();
  size_t capacity = handlers.size();

  if (capacity != 0) {
    // Initialize the handlers_ in the ctor to be owner of vector object.
    handlers_ = std::make_shared<
        std::vector<std::shared_ptr<TProcessorEventHandler>>>();
    // production data suggests reserving capacity here.
    handlers_->reserve(capacity);

    for (const auto& handler : handlers) {
      addNotNullEventHandler(handler);
    }
  }
}

void TClientBase::addClientEventHandler(
    std::shared_ptr<TProcessorEventHandler> handler) {
  if (!handler) {
    return;
  }
  WLock lock{getRWMutex()};
  assert(
      find(getHandlers().begin(), getHandlers().end(), handler) ==
      getHandlers().end());
  getHandlers().push_back(std::move(handler));
}

void TClientBase::removeClientEventHandler(
    std::shared_ptr<TProcessorEventHandler> handler) {
  WLock lock{getRWMutex()};
  assert(
      find(getHandlers().begin(), getHandlers().end(), handler) !=
      getHandlers().end());
  getHandlers().erase(
      remove(getHandlers().begin(), getHandlers().end(), handler),
      getHandlers().end());
}

RWMutex& TClientBase::getRWMutex() {
  static auto* mutex = new RWMutex{};
  return *mutex;
}

vector<folly::not_null_shared_ptr<TProcessorEventHandler>>&
TClientBase::getHandlers() {
  static vector<folly::not_null_shared_ptr<TProcessorEventHandler>> handlers;
  return handlers;
}

} // namespace thrift
} // namespace apache
