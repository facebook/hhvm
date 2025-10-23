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
#include <thrift/lib/cpp2/async/ServerBiDiSinkBridge.h>
#include <thrift/lib/cpp2/async/ServerBiDiStreamBridge.h>
#include <thrift/lib/cpp2/async/ServerCallbackStapler.h>
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
    startFunction_ =
        [transformFn = std::move(streamTransformation.func),
         &decoder,
         &encoder,
         serverExecutor = std::move(serverExecutor)](
            // TODO(sazonovk): T239783600 Add support for ContextStack
            std::shared_ptr<ContextStack> /* contextStack */,
            // TODO(sazonovk): T239783647 Add support for Interactions
            TilePtr&& /* interaction */,
            FirstResponsePayload&& payload,
            BiDiClientCallback* clientCb,
            folly::EventBase* evb) mutable -> void {
      auto stapled = new ServerCallbackStapler();
      auto streamBridge = new ServerBiDiStreamBridge(stapled, evb);
      auto sinkBridge = new ServerBiDiSinkBridge(stapled, evb);
      stapled->setSinkServerCallback(sinkBridge);
      stapled->setStreamServerCallback(streamBridge);
      stapled->resetClientCallback(*clientCb);

      // TODO(sazonovk): T239783814 Add the ability to specify buffer size in
      // StreamTransformation
      uint64_t bufferSize = 100;
      sinkBridge->setBufferSize(bufferSize);

      auto task = ServerBiDiStreamBridge::getTask(
          streamBridge->copy(),
          folly::coro::co_invoke(
              std::move(transformFn),
              ServerBiDiSinkBridge::getInput(sinkBridge->copy(), &decoder)),
          &encoder);
      folly::coro::co_withExecutor(serverExecutor, std::move(task)).start();

      std::ignore = clientCb->onFirstResponse(std::move(payload), evb, stapled);

      sinkBridge->serverPush(uint64_t(bufferSize));
      sinkBridge->processClientMessages();
      streamBridge->processClientMessages();
    };
  }

  static ServerBiDiStreamFactory makeNoopFactory();

  void setContextStack(std::shared_ptr<ContextStack> contextStack);

  void setInteraction(TilePtr&& interaction);

  bool valid() const;

  void start(
      FirstResponsePayload&& firstResponsePayload,
      BiDiClientCallback* clientCallback,
      folly::EventBase* eventBase) &&;

 private:
  /**
   * This constructor is only used when the method that initiates the
   * bidirectional stream throws an exception. Calling start() on an
   * ServerBiDiStreamFactory constructed in this manner will be a noop.
   */
  explicit ServerBiDiStreamFactory();

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
