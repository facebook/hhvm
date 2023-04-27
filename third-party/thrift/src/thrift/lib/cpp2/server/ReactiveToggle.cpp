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

#include <thrift/lib/cpp2/server/ReactiveToggle.h>

namespace apache::thrift::server {

ReactiveToggleSource::ReactiveToggleSource()
    : context_(std::make_shared<ReactiveToggleContext>()) {}

ReactiveToggle::CallbackHandle::~CallbackHandle() {
  cancelAndJoin();
}

ReactiveToggle::CallbackHandle::CallbackHandle(
    std::shared_ptr<ReactiveToggleContext> ctx, folly::Function<void(bool)> f)
    : context_(ctx) {
  node_ = std::make_unique<ReactiveToggleContext::CallbackNode>(std::move(f));

  auto lCallbacks = context_->callbacks.lock();

  lCallbacks->push_back(*node_);
  (node_->func)(context_->on.load());
}

void ReactiveToggle::CallbackHandle::cancelAndJoin() {
  if (!context_) {
    return;
  }

  {
    // wait till the existing callback
    // finishes running
    auto lCallbacks = context_->callbacks.lock();
    auto iter = lCallbacks->iterator_to(*node_);
    // here the memory is not freed yet
    lCallbacks->erase(iter);
    node_->removed = true;
  }

  // past this point this callback
  // is invalidated
  node_.reset();
  context_.reset();
}

void ReactiveToggleSource::set(bool on) {
  // overwrite on value
  // if value is not changed
  // we do nothing
  auto oldOn = context_->on.exchange(on);
  if (oldOn == on) {
    return;
  }

  // execute the callback once
  auto& lCallbacks = *context_->callbacks.lock();
  for (auto& elem : lCallbacks) {
    elem.func(on);
  }
}

ReactiveToggle::ReactiveToggle(const ReactiveToggleSource& source)
    : context_(source.context_) {}

bool ReactiveToggle::get() const {
  if (context_) {
    return context_->on.load();
  }

  return false;
}

ReactiveToggle::CallbackHandle ReactiveToggle::addCallback(
    folly::Function<void(bool)> f) {
  return CallbackHandle(context_, std::move(f));
}

} // namespace apache::thrift::server
