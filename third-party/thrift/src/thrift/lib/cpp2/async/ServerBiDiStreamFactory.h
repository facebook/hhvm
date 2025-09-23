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

#include <thrift/lib/cpp2/async/BiDiStream.h>
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/async/StreamPayload.h>

namespace apache::thrift {

class BiDiServerCallback;
class ContextStack;

namespace detail {

class ServerBiDiStreamFactory {
  using TaskFactory = folly::Function<folly::coro::Task<void>(
      std::shared_ptr<ContextStack>,
      TilePtr&&,
      FirstResponsePayload&&,
      BiDiServerCallback*,
      folly::EventBase*)>;

 public:
  template <typename InputType, typename OutputType>
  explicit ServerBiDiStreamFactory(
      StreamTransformation<InputType, OutputType> streamTransformation,
      SinkElementDecoder<InputType>& decoder,
      StreamElementEncoder<OutputType>& encoder) {
    taskFactory_ = [transformFn = std::move(streamTransformation.func),
                    decoder = decoder,
                    encoder = encoder](
                       std::shared_ptr<ContextStack>,
                       TilePtr&&,
                       FirstResponsePayload&&,
                       BiDiServerCallback*,
                       folly::EventBase*) mutable -> folly::coro::Task<void> {
      LOG(FATAL) << "Not implemented";
      co_return;
    };
  }

  void setContextStack(std::shared_ptr<ContextStack> contextStack);

  void setInteraction(TilePtr&& interaction);

  folly::coro::Task<void> start(
      FirstResponsePayload&& payload,
      BiDiServerCallback* cb,
      folly::EventBase* eb) &&;

 private:
  // taskFactory_ is analogous to ServerStreamFn, the difference here is simply
  // where we define the function. For ServerStream, this is in
  // ServerStreamBridge::fromAsyncGenerator
  TaskFactory taskFactory_;
  std::shared_ptr<ContextStack> contextStack_{nullptr};
  TilePtr interaction_{};
};

} // namespace detail

} // namespace apache::thrift
