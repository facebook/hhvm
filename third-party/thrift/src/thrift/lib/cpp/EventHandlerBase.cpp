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

#include <thrift/lib/cpp2/runtime/Init.h>

using std::remove;
using std::shared_ptr;
using std::vector;

namespace apache::thrift {

void EventHandlerBase::addEventHandler(
    const std::shared_ptr<TProcessorEventHandler>& handler) {
  if (!handlers_) {
    handlers_ = std::make_shared<
        std::vector<std::shared_ptr<TProcessorEventHandler>>>();
  }
  handlers_->push_back(handler);
}

folly::Range<std::shared_ptr<TProcessorEventHandler>*>
EventHandlerBase::getEventHandlers() const {
  if (!handlers_) {
    return {};
  }
  return folly::range(*handlers_);
}

const std::shared_ptr<std::vector<std::shared_ptr<TProcessorEventHandler>>>&
EventHandlerBase::getEventHandlersSharedPtr() const {
  return handlers_;
}

TProcessorBase::TProcessorBase() {
  std::shared_lock lock{getRWMutex()};

  auto& desired = getHandlers();
  if (desired.empty()) {
    return;
  }
  if (!handlers_) {
    handlers_ = std::make_shared<
        std::vector<std::shared_ptr<TProcessorEventHandler>>>();
  }
  folly::grow_capacity_by(*handlers_, desired.size());
  for (const auto& handler : desired) {
    addEventHandler(handler);
  }
}

void TProcessorBase::addProcessorEventHandler_deprecated(
    std::shared_ptr<TProcessorEventHandler> handler) {
  if (!handler) {
    return;
  }
  std::unique_lock lock{getRWMutex()};
  assert(
      find(getHandlers().begin(), getHandlers().end(), handler) ==
      getHandlers().end());
  getHandlers().emplace_back(std::move(handler));
}

void TProcessorBase::removeProcessorEventHandler(
    std::shared_ptr<TProcessorEventHandler> handler) {
  std::unique_lock lock{getRWMutex()};
  assert(
      find(getHandlers().begin(), getHandlers().end(), handler) !=
      getHandlers().end());
  getHandlers().erase(
      remove(getHandlers().begin(), getHandlers().end(), handler),
      getHandlers().end());
}

folly::SharedMutex& TProcessorBase::getRWMutex() {
  static auto* mutex = new folly::SharedMutex{};
  return *mutex;
}

vector<folly::not_null_shared_ptr<TProcessorEventHandler>>&
TProcessorBase::getHandlers() {
  static vector<folly::not_null_shared_ptr<TProcessorEventHandler>> handlers;
  return handlers;
}

TClientBase::TClientBase() : TClientBase(Options()) {}

TClientBase::TClientBase(Options options) {
  if (!options.includeGlobalLegacyClientHandlers) {
    return;
  }

  // Automatically ask all registered factories to produce an event
  // handler, and attach the handlers
  std::shared_lock lock{getRWMutex()};

  auto& handlers = getHandlers();
  folly::Range<std::shared_ptr<TProcessorEventHandler>*> globalHandlers;
  if (apache::thrift::runtime::wasInitialized()) {
    // If we reach this point, it's likely that an AsyncClient object is being
    // used very early in the program's lifetime, possibly before the main
    // function has been called. In such situations, calling
    // TProcessorEventHandlers can lead to circular dependencies and may result
    // in a deadlock at startup, which can be difficult to debug.
    globalHandlers =
        apache::thrift::runtime::getGlobalLegacyClientEventHandlers();
  }
  size_t capacity = handlers.size() + globalHandlers.size();

  if (capacity != 0) {
    // Initialize the handlers_ in the ctor to be owner of vector object.
    handlers_ = std::make_shared<
        std::vector<std::shared_ptr<TProcessorEventHandler>>>();
    // production data suggests reserving capacity here.
    handlers_->reserve(capacity);

    for (const auto& handler : globalHandlers) {
      addEventHandler(handler);
    }
    for (const auto& handler : handlers) {
      addEventHandler(handler);
    }
  }
}

void TClientBase::addClientEventHandler(
    std::shared_ptr<TProcessorEventHandler> handler) {
  if (!handler) {
    return;
  }
  std::unique_lock lock{getRWMutex()};
  assert(
      find(getHandlers().begin(), getHandlers().end(), handler) ==
      getHandlers().end());
  getHandlers().emplace_back(std::move(handler));
}

void TClientBase::removeClientEventHandler(
    std::shared_ptr<TProcessorEventHandler> handler) {
  std::unique_lock lock{getRWMutex()};
  assert(
      find(getHandlers().begin(), getHandlers().end(), handler) !=
      getHandlers().end());
  getHandlers().erase(
      remove(getHandlers().begin(), getHandlers().end(), handler),
      getHandlers().end());
}

folly::SharedMutex& TClientBase::getRWMutex() {
  static auto* mutex = new folly::SharedMutex{};
  return *mutex;
}

vector<folly::not_null_shared_ptr<TProcessorEventHandler>>&
TClientBase::getHandlers() {
  static vector<folly::not_null_shared_ptr<TProcessorEventHandler>> handlers;
  return handlers;
}

} // namespace apache::thrift
