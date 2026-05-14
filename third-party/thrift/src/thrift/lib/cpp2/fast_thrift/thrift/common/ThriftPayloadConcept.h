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

#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrameVariant.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <concepts>
#include <utility>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Base concept for Thrift payload types that can be held in
 * `ThriftPayloadVariant`. Each payload provides:
 *
 *   - `T::RocketFrame` typedef naming the matching `frame::Composed*Frame`.
 *   - `std::move(t).toRocketFrame()` produces the rocket frame.
 *
 * `toRocketFrame()` may be either noexcept or throwing — the
 * request-response client payload performs metadata serialization
 * inside `toRocketFrame()` and can throw on serializer/allocator
 * failure. The transport adapter catches and surfaces such failures
 * inbound as per-request errors.
 *
 * The rocket frame returned by `toRocketFrame()` must satisfy the
 * `frame::ComposedFrameConcept` (i.e., it can be held in a
 * `ComposedFrameVariant`). Together this lets the variant expose a single
 * `toRocketFrame()` accessor that returns a `ComposedFrameVariant<...>` of
 * all the alternatives' rocket frames — eliminating the runtime switch in
 * the transport adapter.
 *
 * Lives in its own header to break the dependency cycle between the
 * variant template (which constrains its alternatives via this base
 * concept) and the payload headers. Refined concepts (request /
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
    apache::thrift::fast_thrift::frame::ComposedFrameConcept<
        typename T::RocketFrame>;

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
