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

#pragma once
#include <folly/Try.h>
#include <thrift/lib/cpp/ContextStack.h>
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>

namespace apache::thrift::detail {

struct ServerStreamFactory {
  template <typename F>
  explicit ServerStreamFactory(F&& fn) : fn_(std::forward<F>(fn)) {}

  void setInteraction(TilePtr&& interaction) {
    interaction_ = std::move(interaction);
  }

  void setContextStack(ContextStack::UniquePtr contextStack) {
    contextStack_ = std::shared_ptr<ContextStack>(std::move(contextStack));
  }

  void operator()(
      FirstResponsePayload&& payload,
      StreamClientCallback* cb,
      folly::EventBase* eb) {
    fn_(std::move(payload),
        cb,
        eb,
        std::move(interaction_),
        std::move(contextStack_));
  }

  explicit operator bool() { return !!fn_; }

 private:
  folly::Function<void(
      FirstResponsePayload&&,
      StreamClientCallback*,
      folly::EventBase*,
      TilePtr&&,
      std::shared_ptr<ContextStack>)>
      fn_;
  TilePtr interaction_;
  std::shared_ptr<ContextStack> contextStack_;
};

template <typename T>
using ServerStreamFn = folly::Function<ServerStreamFactory(
    folly::Executor::KeepAlive<>,
    apache::thrift::detail::StreamElementEncoder<T>*)>;

} // namespace apache::thrift::detail
