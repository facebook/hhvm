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

#include <chrono>
#include <memory>

#include <folly/Portability.h>
#include <folly/Try.h>
#include <folly/coro/AsyncGenerator.h>
#include <folly/coro/Task.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/async/ClientSinkBridge.h>
#include <thrift/lib/cpp2/async/ServerSinkBridge.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>

namespace apache::thrift {

class FOLLY_EXPORT SinkThrew : public TApplicationException {
 public:
  using TApplicationException::TApplicationException;
};

template <typename T, typename R = void>
class ClientSink {
#if FOLLY_HAS_COROUTINES
  using PayloadSerializer = apache::thrift::detail::StreamElementEncoder<T>*;
  using FinalResponseDeserializer =
      folly::Try<R> (*)(folly::Try<StreamPayload>&&);

 public:
  ClientSink() = default;

  ClientSink(
      apache::thrift::detail::ClientSinkBridge::ClientPtr impl,
      PayloadSerializer serializer,
      FinalResponseDeserializer deserializer)
      : impl_(std::move(impl)),
        serializer_(std::move(serializer)),
        deserializer_(std::move(deserializer)) {}

  ClientSink(const ClientSink&) = delete;
  ClientSink& operator=(const ClientSink&) = delete;

  ClientSink(ClientSink&&) = default;
  ClientSink& operator=(ClientSink&& sink) noexcept {
    impl_ = std::move(sink.impl_);
    serializer_ = std::move(sink.serializer_);
    deserializer_ = std::move(sink.deserializer_);
    return *this;
  }

  folly::coro::Task<R> sink(folly::coro::AsyncGenerator<T&&> generator) {
    folly::exception_wrapper sinkError;
    auto finalResponse = co_await folly::coro::co_nothrow(
        detail::ClientSinkBridge::Ptr(impl_.release())
            ->sink(
                [this, &sinkError](folly::coro::AsyncGenerator<T&&> _generator)
                    -> folly::coro::AsyncGenerator<
                        folly::Try<StreamPayload>&&> {
                  while (true) {
                    auto item =
                        co_await folly::coro::co_awaitTry(_generator.next());
                    if (item.hasException()) {
                      sinkError = item.exception();
                      co_yield (*serializer_)(std::move(item.exception()));
                      co_return;
                    }
                    if (!item->has_value()) {
                      co_return;
                    }
                    co_yield (*serializer_)(std::move(**item));
                  }
                  co_return;
                }(std::move(generator))));

    if (finalResponse.hasException()) {
      co_yield folly::coro::co_error(
          deserializer_(std::move(finalResponse)).exception());
    }
    if (sinkError) {
      co_yield folly::coro::co_error(SinkThrew(sinkError.what().toStdString()));
    }

    co_return deserializer_(std::move(finalResponse)).value();
  }

 private:
  apache::thrift::detail::ClientSinkBridge::ClientPtr impl_;
  PayloadSerializer serializer_;
  FinalResponseDeserializer deserializer_;
#endif
};

struct SinkOptions {
  std::chrono::milliseconds chunkTimeout;
};

template <typename SinkElement, typename FinalResponse>
struct SinkConsumer {
#if FOLLY_HAS_COROUTINES
  using Consumer = folly::Function<folly::coro::Task<FinalResponse>(
      folly::coro::AsyncGenerator<SinkElement&&>)>;
  Consumer consumer;
  uint64_t bufferSize{100};
  SinkOptions sinkOptions{std::chrono::milliseconds(0)};
  SinkConsumer&& setChunkTimeout(const std::chrono::milliseconds& timeout) && {
    sinkOptions.chunkTimeout = timeout;
    return std::move(*this);
  }
  SinkConsumer& setChunkTimeout(const std::chrono::milliseconds& timeout) & {
    sinkOptions.chunkTimeout = timeout;
    return *this;
  }
#endif
};

template <typename Response, typename SinkElement, typename FinalResponse>
class ResponseAndClientSink {
 public:
  using ResponseType = Response;
  using SinkElementType = SinkElement;
  using FinalResponseType = FinalResponse;

  Response response;
  ClientSink<SinkElement, FinalResponse> sink;
};

template <typename Response, typename SinkElement, typename FinalResponse>
class ResponseAndSinkConsumer {
 public:
  using ResponseType = Response;
  using SinkElementType = SinkElement;
  using FinalResponseType = FinalResponse;

  Response response;
  SinkConsumer<SinkElement, FinalResponse> sinkConsumer;
};

template <typename T>
class ClientBufferedStream;

template <typename Response, typename SinkElement, typename StreamElement>
class ResponseAndBidirectionalStream {
 public:
  using ResponseType = Response;
  using SinkElementType = SinkElement;
  using StreamElementType = StreamElement;

  Response response;
  ClientSink<SinkElement> sink;
  ClientBufferedStream<StreamElement> stream;
};

template <typename SinkElement, typename StreamElement>
class BidirectionalStream {
 public:
  using SinkElementType = SinkElement;
  using StreamElementType = StreamElement;

  ClientSink<SinkElement> sink;
  ClientBufferedStream<StreamElement> stream;
};

} // namespace apache::thrift
