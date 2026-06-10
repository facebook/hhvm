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

#include <string_view>

#include <folly/Portability.h>

#if FOLLY_HAS_COROUTINES
#include <thrift/lib/cpp2/async/TwoWayBridgeUtil.h>
#endif
#include <thrift/lib/cpp2/async/ServerSinkBridge.h>

namespace apache::thrift::detail {

#if FOLLY_HAS_COROUTINES

class ServerSinkFactory {
 public:
  using StartFunction = folly::Function<void(
      uint64_t,
      uint64_t,
      std::chrono::milliseconds,
      folly::EventBase*,
      TilePtr&&,
      ContextStack::UniquePtr,
      std::unique_ptr<ThriftSinkLog>,
      FirstResponsePayload&&,
      SinkClientCallback*)>;

  class ConsumerCallback {
   public:
    virtual void provideSink(ServerSinkBridge::Ptr sink) = 0;
    ConsumerCallback() = default;
    virtual ~ConsumerCallback() = default;
  };

  explicit ServerSinkFactory();

  explicit ServerSinkFactory(StartFunction&& fn);

  explicit ServerSinkFactory(
      ConsumerCallback* consumerCallback,
      uint64_t bufferSize,
      uint64_t bufferReplenishThreshold,
      std::chrono::milliseconds timeout);

  /// Template constructor that captures a typed consumer, decoder, and
  /// encode functions — avoiding type erasure of the consumer. Analogous
  /// to ServerBiDiStreamFactory's template constructor.
  template <
      typename SinkType,
      typename FinalResponseType,
      typename EncodeFinalResponseFunc,
      typename EncodeExceptionFunc>
  explicit ServerSinkFactory(
      folly::Function<folly::coro::Task<FinalResponseType>(
          folly::coro::AsyncGenerator<SinkType&&>)>&& consumer,
      SinkElementDecoder<SinkType>& decoder,
      EncodeFinalResponseFunc encodeFinalResponse,
      EncodeExceptionFunc encodeException,
      folly::Executor::KeepAlive<> serverExecutor,
      uint64_t bufferSize,
      uint64_t bufferReplenishThreshold,
      std::chrono::milliseconds timeout)
      : bufferSize_{bufferSize},
        bufferReplenishThreshold_{bufferReplenishThreshold},
        chunkTimeout_{timeout} {
    startFunction_ = [consumer = std::move(consumer),
                      decode = &decoder,
                      encodeFinalResponse = std::move(encodeFinalResponse),
                      encodeException = std::move(encodeException),
                      serverExecutor](
                         uint64_t bufSize,
                         uint64_t bufReplenishThreshold,
                         std::chrono::milliseconds chunkTimeout,
                         folly::EventBase* evb,
                         TilePtr&& interaction,
                         ContextStack::UniquePtr contextStack,
                         std::unique_ptr<ThriftSinkLog> sinkLog,
                         FirstResponsePayload&& firstResponsePayload,
                         SinkClientCallback* clientCallback) mutable {
      DCHECK(evb->isInEventBaseThread());
      SinkBridgeContext sinkConsumer;
      sinkConsumer.bufferSize = bufSize;
      sinkConsumer.bufferReplenishThreshold = bufReplenishThreshold;
      sinkConsumer.chunkTimeout = chunkTimeout;
      sinkConsumer.executor = serverExecutor;
      sinkConsumer.interaction = std::move(interaction);
      sinkConsumer.contextStack = std::move(contextStack);
      sinkConsumer.sinkLog = std::move(sinkLog);

      auto sink =
          new ServerSinkBridge(std::move(sinkConsumer), *evb, clientCallback);
      auto sinkPtr = sink->copy();

      std::ignore = clientCallback->onFirstResponse(
          std::move(firstResponsePayload), evb, sink);

      folly::coro::co_withExecutor(
          std::move(serverExecutor),
          folly::coro::co_invoke(
              [consumer = std::move(consumer),
               decode,
               encodeFinalResponse = std::move(encodeFinalResponse),
               encodeException = std::move(encodeException)](
                  ServerSinkBridge::Ptr bridge) mutable
                  -> folly::coro::Task<void> {
                bridge->serverPush(
                    StreamMessage::RequestN{
                        static_cast<int32_t>(bridge->getBufferSize())});
                try {
                  FinalResponseType finalResponse = co_await consumer(
                      ServerSinkBridge::getInput<SinkType>(
                          bridge->copy(), decode));
                  if (bridge->clientException_) {
                    co_return;
                  }
                  bridge->serverPush(
                      StreamMessage::PayloadOrError{
                          encodeFinalResponse(std::move(finalResponse))});
                } catch (...) {
                  if (bridge->clientException_) {
                    co_return;
                  }
                  bridge->serverPush(
                      StreamMessage::PayloadOrError{encodeException(
                          folly::exception_wrapper(std::current_exception()))});
                }
              },
              std::move(sinkPtr)))
          .start();
    };
  }

  void start(
      FirstResponsePayload&& payload,
      SinkClientCallback* callback,
      folly::EventBase* evb);

  bool valid();

  std::chrono::milliseconds getChunkTimeout();

  void setInteraction(TilePtr interaction);

  void setContextStack(ContextStack::UniquePtr contextStack);

  ContextStack* getContextStack() const { return contextStack_.get(); }

  void setMethodName(std::string_view methodName) { methodName_ = methodName; }

  std::string_view getMethodName() const { return methodName_; }

  void setSinkLog(std::unique_ptr<ThriftSinkLog> sinkLog) {
    sinkLog_ = std::move(sinkLog);
  }

 private:
  StartFunction startFunction_{nullptr};
  uint64_t bufferSize_{};
  uint64_t bufferReplenishThreshold_{};
  std::chrono::milliseconds chunkTimeout_{};
  TilePtr interaction_{};
  ContextStack::UniquePtr contextStack_{nullptr};
  std::string_view methodName_;
  std::unique_ptr<ThriftSinkLog> sinkLog_;
};

#endif

} // namespace apache::thrift::detail
