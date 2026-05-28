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

#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <concepts>
#include <utility>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Base concept for Thrift payload types that can be held in
 * `ThriftPayloadVariant`. Each payload provides:
 *
 *   - `T::RocketFrame` typedef naming the rocket frame type (always
 *     `frame::ComposedFrame` post-flat-migration).
 *   - `std::move(t).toRocketFrame()` produces the rocket frame.
 *
 * `toRocketFrame()` may be either noexcept or throwing — the
 * request-response client payload performs metadata serialization
 * inside `toRocketFrame()` and can throw on serializer/allocator
 * failure. The transport adapter catches and surfaces such failures
 * inbound as per-request errors.
 *
 * Lives in its own header to keep the payload headers free of dependencies
 * on consumers of this concept. Refined concepts (request /
 * initial-response) live alongside it so all payload-variant constraints
 * are in one place.
 */
template <typename T>
concept ThriftPayloadConcept =
    requires(T&& t, rocket::server::MetadataProtocol p) {
      {
        std::move(t).toRocketFrame(p)
      } -> std::same_as<typename T::RocketFrame>;
    } &&
    std::same_as<
        typename T::RocketFrame,
        apache::thrift::fast_thrift::frame::ComposedFrame>;

/**
 * Refines `ThriftPayloadConcept` for the 5 initial-request payload types
 * (RR, Fnf, Stream, Sink, Bidi). These open a new exchange and always
 * carry routing metadata — the composite app adapter reads
 * `getRequestRpcMetadata()->name()` to pick a child handler.
 *
 * Today only `ThriftRequestResponsePayload` returns a typed metadata
 * pointer; the other four still hold raw IOBuf metadata and return
 * nullptr. Reshaping them to typed metadata is a future step that's
 * structurally invisible to consumers of this concept.
 */
template <typename T>
concept ThriftRequestPayloadConcept =
    ThriftPayloadConcept<T> && requires(const T& t) {
      {
        t.getRequestRpcMetadata()
      } -> std::same_as<const apache::thrift::RequestRpcMetadata*>;
    };

/**
 * Refines `ThriftPayloadConcept` for the initial-response payload type(s).
 * Currently `ThriftInitialResponsePayload` is the only alternative.
 */
template <typename T>
concept ThriftInitialResponsePayloadConcept =
    ThriftPayloadConcept<T> && requires(const T& t) {
      {
        t.getResponseRpcMetadata()
      } -> std::same_as<const apache::thrift::ResponseRpcMetadata*>;
    };

} // namespace apache::thrift::fast_thrift::thrift
