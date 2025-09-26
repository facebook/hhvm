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

/**
 * This header defines user-facing types for BiDi (Bi-directional) methods.
 * Client-side types are: BidirectionalStream, ResponseAndBidirectionalStream.
 * Server-side types are: StreamTransformation, ResponseAndStreamTransformation.
 */

#pragma once

#include <folly/Function.h>
#include <folly/coro/AsyncGenerator.h>
#include <folly/coro/Task.h>
#include <thrift/lib/cpp2/async/ClientBufferedStream.h>
#include <thrift/lib/cpp2/async/Sink.h>

namespace apache::thrift {

/**
 * BidirectionalStream is the Client return type for a BiDi method that does not
 * have an initial response, e.g `sink<T>, stream<U> bidiNoInitialResponse()`
 */
template <typename SinkElement, typename StreamElement>
class BidirectionalStream {
 public:
  using SinkElementType = SinkElement;
  using StreamElementType = StreamElement;

  ClientSink<SinkElement, void> sink;
  ClientBufferedStream<StreamElement> stream;
};

/**
 * ResponseAndBidirectionalStream is the Client return type for a BiDi method
 * that has an initial response, e.g
 * `R, sink<T>, stream<U> bidiWithInitialResponse()`
 */
template <typename Response, typename SinkElement, typename StreamElement>
class ResponseAndBidirectionalStream {
 public:
  using ResponseType = Response;
  using SinkElementType = SinkElement;
  using StreamElementType = StreamElement;

  Response response;
  ClientSink<SinkElement, void> sink;
  ClientBufferedStream<StreamElement> stream;
};

/**
 * StreamTransformation is the Server-side return type for a BiDi method that
 * does not have an initial response:
 * `sink<T>, stream<U> bidiNoInitialResponse()`
 */
template <typename InputElement, typename OutputElement>
struct StreamTransformation {
  using InputType = folly::coro::AsyncGenerator<InputElement&&>;
  using OutputType = folly::coro::AsyncGenerator<OutputElement&&>;
  using Func = folly::Function<OutputType(InputType)>;

  Func func;
};

/**
 * ResponseAndStreamTransformation is the Server-side return type for a BiDi
 * method that has an initial response:
 * `R, sink<T>, stream<U> bidiWithInitialResponse()`
 */
template <typename Response, typename InputElement, typename OutputElement>
struct ResponseAndStreamTransformation {
  Response response;
  StreamTransformation<InputElement, OutputElement> transform;
};

} // namespace apache::thrift
