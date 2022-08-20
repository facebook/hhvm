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
#include <folly/Try.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/experimental/coro/AsyncGenerator.h>
#include <thrift/lib/cpp2/async/ClientBufferedStream.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/async/ServerGeneratorStream.h>
#include <thrift/lib/cpp2/async/ServerPublisherStream.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>

namespace yarpl {
namespace flowable {
class ThriftStreamShim;
} // namespace flowable
} // namespace yarpl

namespace apache {
namespace thrift {

template <typename T, bool WithHeader>
class ServerStreamMultiPublisher;

template <typename T>
class ServerStream {
 public:
  using PayloadAndHeader = apache::thrift::detail::PayloadAndHeader<T>;
  using UnorderedHeader = apache::thrift::detail::UnorderedHeader;
  using OrderedHeader = apache::thrift::detail::OrderedHeader;
  using MessageVariant = apache::thrift::detail::MessageVariant<T>;

#if FOLLY_HAS_COROUTINES
  /* implicit */ ServerStream(folly::coro::AsyncGenerator<T&&>&& gen)
      : fn_(apache::thrift::detail::ServerGeneratorStream::
                fromAsyncGenerator<false, T>(std::move(gen))) {}

  /* implicit */ ServerStream(
      folly::coro::AsyncGenerator<MessageVariant&&>&& gen)
      : fn_(apache::thrift::detail::ServerGeneratorStream::
                fromAsyncGenerator<true, T>(std::move(gen))) {}

  using promise_type = folly::coro::detail::AsyncGeneratorPromise<T&&, T>;
#endif

  // Completion callback is optional
  // It may destroy the ServerStreamPublisher object inline
  // It must not call complete() on the publisher object inline
  static std::pair<ServerStream<T>, ServerStreamPublisher<T>> createPublisher(
      folly::Function<void()> onStreamCompleteOrCancel) {
    return createPublisherImpl<false>(std::move(onStreamCompleteOrCancel));
  }
  static std::pair<ServerStream<T>, ServerStreamPublisher<T, true>>
  createPublisherWithHeader(folly::Function<void()> onStreamCompleteOrCancel) {
    return createPublisherImpl<true>(std::move(onStreamCompleteOrCancel));
  }
  static std::pair<ServerStream<T>, ServerStreamPublisher<T>>
  createPublisher() {
    return createPublisher([] {});
  }

  static ServerStream<T> createEmpty() {
    auto pair = createPublisher();
    std::move(pair.second).complete();
    return std::move(pair.first);
  }

  [[deprecated(
      "Use ScopedServerInterfaceThread instead of invoking handler methods "
      "directly. This approach changes the threading model and can hide race "
      "conditions in production code.")]] //
  ClientBufferedStream<T>
  toClientStreamUnsafeDoNotUse(
      folly::EventBase* evb = folly::getUnsafeMutableGlobalEventBase(),
      int32_t bufferSize = 100) &&;

  apache::thrift::detail::ServerStreamFactory operator()(
      folly::Executor::KeepAlive<> serverExecutor,
      apache::thrift::detail::StreamElementEncoder<T>* encode) {
    return fn_(std::move(serverExecutor), encode);
  }

 private:
  explicit ServerStream(apache::thrift::detail::ServerStreamFn<T> fn)
      : fn_(std::move(fn)) {}

  template <bool WithHeader>
  static std::pair<ServerStream<T>, ServerStreamPublisher<T, WithHeader>>
  createPublisherImpl(folly::Function<void()> onStreamCompleteOrCancel) {
    auto pair =
        apache::thrift::detail::ServerPublisherStream<T, WithHeader>::create(
            std::move(onStreamCompleteOrCancel));
    return std::
        make_pair<ServerStream<T>, ServerStreamPublisher<T, WithHeader>>(
            ServerStream<T>(std::move(pair.first)), std::move(pair.second));
  }

  apache::thrift::detail::ServerStreamFn<T> fn_;

  friend class yarpl::flowable::ThriftStreamShim;
  friend class ServerStreamMultiPublisher<T, false>;
  friend class ServerStreamMultiPublisher<T, true>;
};

template <typename Response, typename StreamElement>
struct ResponseAndServerStream {
  using ResponseType = Response;
  using StreamElementType = StreamElement;

  Response response;
  ServerStream<StreamElement> stream;
};
struct ResponseAndServerStreamFactory {
  apache::thrift::SerializedResponse response;
  apache::thrift::detail::ServerStreamFactory stream;
};

} // namespace thrift
} // namespace apache
#include <thrift/lib/cpp2/async/ServerStream-inl.h>
