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
      std::chrono::milliseconds,
      folly::EventBase*,
      TilePtr&&,
      ContextStack::UniquePtr,
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
      SinkConsumerImpl::Consumer&& consumer,
      folly::Executor::KeepAlive<> serverExecutor,
      uint64_t bufferSize,
      std::chrono::milliseconds timeout);

  explicit ServerSinkFactory(
      ConsumerCallback* consumerCallback,
      uint64_t bufferSize,
      std::chrono::milliseconds timeout);

  void start(
      FirstResponsePayload&& payload,
      SinkClientCallback* callback,
      folly::EventBase* evb);

  bool valid();

  std::chrono::milliseconds getChunkTimeout();

  void setInteraction(TilePtr interaction);

  void setContextStack(ContextStack::UniquePtr contextStack);

 private:
  StartFunction startFunction_{nullptr};
  uint64_t bufferSize_{};
  std::chrono::milliseconds chunkTimeout_{};
  TilePtr interaction_{};
  ContextStack::UniquePtr contextStack_{nullptr};
};

#endif

} // namespace apache::thrift::detail
