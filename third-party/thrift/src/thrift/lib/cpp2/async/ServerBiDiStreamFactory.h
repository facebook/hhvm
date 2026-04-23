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
#include <thrift/lib/cpp2/async/StreamMessage.h>
#include <thrift/lib/cpp2/async/StreamPayload.h>
#include <thrift/lib/cpp2/logging/ThriftBiDiLog.h>

namespace apache::thrift {

class ContextStack;

namespace detail {

class ServerBiDiStreamFactory {
  using StartFunction = folly::Function<void(
      std::shared_ptr<ContextStack>,
      TilePtr&&,
      FirstResponsePayload&&,
      BiDiClientCallback*,
      folly::EventBase*,
      std::shared_ptr<ThriftBiDiLog>)>;

 public:
  /**
   * Callback interface for receiving both BiDi bridges.
   * Similar to ServerSinkFactory::ConsumerCallback and
   * ServerGeneratorStreamBridge::ProducerCallback.
   */
  class ServerBiDiCallback {
   public:
    virtual void provideBiDiBridge(
        ServerBiDiStreamBridge::Ptr streamBridge,
        ServerBiDiSinkBridge::Ptr sinkBridge) = 0;
    ServerBiDiCallback() = default;
    virtual ~ServerBiDiCallback() = default;
  };

  /**
   * Constructor for runtimes that want to receive the bridges
   * directly via a callback.
   */
  explicit ServerBiDiStreamFactory(
      ServerBiDiCallback* biDiCallback, uint64_t bufferSize);

  explicit ServerBiDiStreamFactory(
      ServerBiDiCallback* biDiCallback,
      uint64_t bufferSize,
      uint64_t bufferReplenishThreshold);

  template <typename InputType, typename OutputType>
  explicit ServerBiDiStreamFactory(
      StreamTransformation<InputType, OutputType> streamTransformation,
      SinkElementDecoder<InputType>& decoder,
      StreamElementEncoder<OutputType>& encoder,
      folly::Executor::KeepAlive<> serverExecutor,
      std::chrono::milliseconds chunkTimeout = std::chrono::milliseconds{0})
      : chunkTimeout_(chunkTimeout) {
    startFunction_ =
        [transformFn = std::move(streamTransformation.func),
         sinkBufferSize = streamTransformation.bufferSize,
         sinkReplenishThreshold = streamTransformation.bufferReplenishThreshold,
         &decoder,
         &encoder,
         serverExecutor = std::move(serverExecutor)](
            std::shared_ptr<ContextStack> contextStack,
            // TODO(sazonovk): T239783647 Add support for Interactions
            TilePtr&& /* interaction */,
            FirstResponsePayload&& payload,
            BiDiClientCallback* clientCb,
            folly::EventBase* evb,
            std::shared_ptr<ThriftBiDiLog> biDiLog) mutable -> void {
      if (contextStack) {
        contextStack->onBiDiSubscribe();
      }
      if (biDiLog) {
        biDiLog->log(detail::BiDiSubscribeEvent{});
      }

      auto stapled = new ServerCallbackStapler();
      auto streamBridge =
          new ServerBiDiStreamBridge(stapled, evb, contextStack);
      auto sinkBridge = new ServerBiDiSinkBridge(stapled, evb, contextStack);
      stapled->setContextStack(contextStack);
      stapled->setBiDiLog(biDiLog);
      streamBridge->setBiDiLog(biDiLog);
      sinkBridge->setBiDiLog(biDiLog);
      stapled->setSinkServerCallback(sinkBridge);
      stapled->setStreamServerCallback(streamBridge);
      stapled->resetClientCallback(*clientCb);

      DCHECK_GT(sinkBufferSize, 0);
      sinkBridge->setBufferSize(sinkBufferSize);
      sinkBridge->setBufferReplenishThreshold(sinkReplenishThreshold);

      auto task = ServerBiDiStreamBridge::getTask(
          streamBridge->copy(),
          folly::coro::co_invoke(
              std::move(transformFn),
              ServerBiDiSinkBridge::getInput(sinkBridge->copy(), &decoder)),
          &encoder);
      folly::coro::co_withExecutor(serverExecutor, std::move(task)).start();

      std::ignore = clientCb->onFirstResponse(std::move(payload), evb, stapled);

      sinkBridge->serverPush(StreamMessage::RequestN{sinkBufferSize});
      sinkBridge->processClientMessages();
      streamBridge->processClientMessages();
    };
  }

  static ServerBiDiStreamFactory makeNoopFactory();

  void setContextStack(std::shared_ptr<ContextStack> contextStack);

  void setInteraction(TilePtr&& interaction);

  void setMethodName(std::string_view methodName) { methodName_ = methodName; }
  std::string_view getMethodName() const { return methodName_; }

  void setBiDiLog(std::shared_ptr<ThriftBiDiLog> biDiLog) {
    biDiLog_ = std::move(biDiLog);
  }

  bool valid() const;

  std::chrono::milliseconds getChunkTimeout() const { return chunkTimeout_; }

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
  std::chrono::milliseconds chunkTimeout_{0};
  std::string_view methodName_;
  std::shared_ptr<ThriftBiDiLog> biDiLog_;
};

} // namespace detail

struct ResponseAndServerBiDiStreamFactory {
  SerializedResponse response;
  detail::ServerBiDiStreamFactory bidiStream;
};

} // namespace apache::thrift
