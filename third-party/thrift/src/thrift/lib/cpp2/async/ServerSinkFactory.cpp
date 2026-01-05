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

#include <thrift/lib/cpp2/async/ServerSinkFactory.h>

#if FOLLY_HAS_COROUTINES

namespace apache::thrift::detail {

ServerSinkFactory::ServerSinkFactory() : startFunction_(nullptr) {}

ServerSinkFactory::ServerSinkFactory(StartFunction&& fn)
    : startFunction_(std::move(fn)) {}

ServerSinkFactory::ServerSinkFactory(
    SinkConsumerImpl::Consumer&& consumer,
    folly::Executor::KeepAlive<> serverExecutor,
    uint64_t bufferSize,
    std::chrono::milliseconds timeout)
    : bufferSize_{bufferSize}, chunkTimeout_{timeout} {
  startFunction_ = [consumer = std::move(consumer),
                    serverExecutor = std::move(serverExecutor)](
                       uint64_t bufferSize,
                       std::chrono::milliseconds chunkTimeout,
                       folly::EventBase* evb,
                       TilePtr&& interaction,
                       ContextStack::UniquePtr contextStack,
                       FirstResponsePayload&& firstResponsePayload,
                       SinkClientCallback* clientCallback) mutable {
    DCHECK(evb->isInEventBaseThread());
    SinkConsumerImpl sinkConsumer;
    sinkConsumer.consumer = std::move(consumer);
    sinkConsumer.bufferSize = bufferSize;
    sinkConsumer.chunkTimeout = chunkTimeout;
    sinkConsumer.executor = serverExecutor;
    sinkConsumer.interaction = std::move(interaction);
    sinkConsumer.contextStack = std::move(contextStack);

    auto sink =
        new ServerSinkBridge(std::move(sinkConsumer), *evb, clientCallback);
    auto sinkPtr = sink->copy();

    std::ignore = clientCallback->onFirstResponse(
        std::move(firstResponsePayload), evb, sink);
    folly::coro::co_withExecutor(
        std::move(serverExecutor),
        folly::coro::co_invoke(&ServerSinkBridge::start, std::move(sinkPtr)))
        .start();
  };
}

void ServerSinkFactory::start(
    FirstResponsePayload&& payload,
    SinkClientCallback* callback,
    folly::EventBase* evb) {
  startFunction_(
      bufferSize_,
      chunkTimeout_,
      evb,
      std::move(interaction_),
      std::move(contextStack_),
      std::move(payload),
      callback);
}

bool ServerSinkFactory::valid() {
  return startFunction_ != nullptr;
}

std::chrono::milliseconds ServerSinkFactory::getChunkTimeout() {
  return chunkTimeout_;
}

void ServerSinkFactory::setInteraction(TilePtr interaction) {
  interaction_ = std::move(interaction);
}

void ServerSinkFactory::setContextStack(ContextStack::UniquePtr contextStack) {
  contextStack_ = std::move(contextStack);
}

ServerSinkFactory::ServerSinkFactory(
    ConsumerCallback* consumerCallback,
    uint64_t bufferSize,
    std::chrono::milliseconds chunkTimeout)
    : bufferSize_{bufferSize}, chunkTimeout_{chunkTimeout} {
  startFunction_ = [consumerCallback](
                       uint64_t bufferSize,
                       std::chrono::milliseconds chunkTimeout,
                       folly::EventBase* evb,
                       TilePtr&& interaction,
                       ContextStack::UniquePtr contextStack,
                       FirstResponsePayload&& firstResponsePayload,
                       SinkClientCallback* clientCallback) {
    DCHECK(evb->isInEventBaseThread());
    SinkConsumerImpl sinkConsumer;
    sinkConsumer.bufferSize = bufferSize;
    sinkConsumer.chunkTimeout = chunkTimeout;
    sinkConsumer.interaction = std::move(interaction);
    sinkConsumer.contextStack = std::move(contextStack);

    auto sink =
        new ServerSinkBridge(std::move(sinkConsumer), *evb, clientCallback);
    auto sinkPtr = sink->copy();

    std::ignore = clientCallback->onFirstResponse(
        std::move(firstResponsePayload), evb, sink);

    sink->serverPush(bufferSize);
    consumerCallback->provideSink(std::move(sinkPtr));
  };
}

} // namespace apache::thrift::detail

#endif
