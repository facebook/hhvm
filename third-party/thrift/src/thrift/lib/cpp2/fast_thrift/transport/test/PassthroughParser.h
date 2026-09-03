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

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/transport/Parser.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

namespace apache::thrift::fast_thrift::transport::test {

/**
 * Parser that does no framing: each read is forwarded to the pipeline as-is.
 *
 * For tests and benchmarks that exercise the transport itself, or that move
 * raw bytes with no protocol on the wire. Production pipelines pair the
 * transport with a parser for whatever protocol they actually speak, so this
 * deliberately does not live alongside the Parser concept.
 */
class PassthroughParser {
 public:
  static constexpr size_t kDefaultMinBufferSize = 4096;
  static constexpr size_t kDefaultMaxBufferSize = 65536;

  explicit PassthroughParser(
      size_t minBufferSize = kDefaultMinBufferSize,
      size_t maxBufferSize = kDefaultMaxBufferSize) noexcept
      : minBufferSize_(minBufferSize), maxBufferSize_(maxBufferSize) {}

  void getReadBuffer(void** bufReturn, size_t* lenReturn) noexcept {
    auto ret = readBufQueue_.preallocate(minBufferSize_, maxBufferSize_);
    *bufReturn = ret.first;
    *lenReturn = ret.second;
  }

  template <typename Sink>
  channel_pipeline::Result consume(size_t len, Sink&& sink) noexcept {
    readBufQueue_.postallocate(len);
    return sink(readBufQueue_.move());
  }

  template <typename Sink>
  channel_pipeline::Result consumeBuffer(
      channel_pipeline::BytesPtr buf, Sink&& sink) noexcept {
    return sink(std::move(buf));
  }

  void setIOBufFactory(folly::IOBufFactory* factory) noexcept {
    readBufQueue_.setIOBufFactory(factory);
  }

  void reset() noexcept { readBufQueue_.reset(); }

 private:
  size_t minBufferSize_;
  size_t maxBufferSize_;
  folly::IOBufQueue readBufQueue_{folly::IOBufQueue::cacheChainLength()};
};

static_assert(Parser<PassthroughParser>);

/**
 * The unframed transport used by tests and benchmarks, standing in for the
 * `TransportHandler` alias the production header no longer provides.
 */
using PassthroughTransportHandler =
    TransportHandlerT<NoOpWriteCompleteEventFactory, PassthroughParser>;

} // namespace apache::thrift::fast_thrift::transport::test
