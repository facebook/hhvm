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
#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/async/StreamPayload.h>

namespace apache::thrift {

class ContextStack;

namespace detail {

class ServerBiDiStreamFactory {
  using StartFunction = folly::Function<void(
      std::shared_ptr<ContextStack>,
      TilePtr&&,
      FirstResponsePayload&&,
      BiDiClientCallback*,
      folly::EventBase*)>;

 public:
  template <typename InputType, typename OutputType>
  explicit ServerBiDiStreamFactory(
      StreamTransformation<InputType, OutputType> streamTransformation,
      SinkElementDecoder<InputType>& decoder,
      StreamElementEncoder<OutputType>& encoder,
      folly::Executor::KeepAlive<> serverExecutor) {
    startFunction_ = [transformFn = std::move(streamTransformation.func),
                      &decoder,
                      &encoder,
                      serverExecutor = std::move(serverExecutor)](
                         std::shared_ptr<ContextStack>,
                         TilePtr&&,
                         FirstResponsePayload&&,
                         BiDiClientCallback*,
                         folly::EventBase*) mutable -> void {
      LOG(FATAL) << "Not implemented";
    };
  }

  void setContextStack(std::shared_ptr<ContextStack> contextStack);

  void setInteraction(TilePtr&& interaction);

  void start(
      FirstResponsePayload&& firstResponsePayload,
      BiDiClientCallback* clientCallback,
      folly::EventBase* eventBase) &&;

 private:
  StartFunction startFunction_;
  std::shared_ptr<ContextStack> contextStack_{nullptr};
  TilePtr interaction_{};
};

} // namespace detail

struct ResponseAndServerBiDiStreamFactory {
  SerializedResponse response;
  detail::ServerBiDiStreamFactory bidiStream;
};

} // namespace apache::thrift
