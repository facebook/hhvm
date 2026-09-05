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

#include <concepts>
#include <cstddef>

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

namespace apache::thrift::fast_thrift::transport {

namespace detail {
// Stand-in sink, used only to check the Parser concept. Real sinks are
// deduced at the call site so the drain loop inlines.
struct ConceptSink {
  channel_pipeline::Result operator()(channel_pipeline::BytesPtr&&) noexcept;
};
} // namespace detail

/**
 * Owns a connection's inbound buffer and turns the socket byte stream into
 * whole messages.
 *
 * Buffer ownership and message extraction live in the same object because the
 * parser is the only party that knows how many bytes the next message needs:
 * it sizes the buffer it is about to be read into, then splits messages out of
 * that same buffer. Splitting those roles across the transport and a
 * downstream pipeline handler costs an allocation plus a copy per read, since
 * the transport would have to guess a size before anything had been parsed.
 *
 * A parser is per-connection, is only touched on that connection's EventBase,
 * and never throws — errors come back as Result::Error.
 *
 * `consume`/`consumeBuffer` return the sink's first non-Success result. Bytes
 * not yet emitted stay buffered for the next call, so Backpressure is
 * resumable. The sink may close the connection re-entrantly, so a parser must
 * not touch itself after a sink call returns non-Success.
 *
 * `setIOBufFactory` is called once, when the transport is given its pipeline,
 * to route buffer allocation through the pipeline's allocator. The factory
 * outlives the parser.
 */
template <typename P>
concept Parser = requires(
    P parser,
    void** bufReturn,
    size_t* lenReturn,
    size_t len,
    channel_pipeline::BytesPtr buf,
    folly::IOBufFactory* factory,
    detail::ConceptSink sink) {
  { parser.getReadBuffer(bufReturn, lenReturn) } noexcept;
  {
    parser.consume(len, sink)
  } noexcept -> std::same_as<channel_pipeline::Result>;
  {
    parser.consumeBuffer(std::move(buf), sink)
  } noexcept -> std::same_as<channel_pipeline::Result>;
  { parser.setIOBufFactory(factory) } noexcept;
  { parser.reset() } noexcept;
};

} // namespace apache::thrift::fast_thrift::transport
