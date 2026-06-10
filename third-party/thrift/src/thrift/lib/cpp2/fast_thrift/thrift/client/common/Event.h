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

#include <cstdint>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftClientEventType — discriminator for thrift-client pipeline user
 * events. Each value names a distinct event a pipeline handler may
 * choose to handle.
 */
enum class ThriftClientEventType : std::uint32_t {
  // The transport is draining: the server sent a graceful close
  // (rocket CONNECTION_CLOSE). The transport adapter translates the
  // rocket-native close notification into this event; the
  // pipeline-resident ThriftClientGracefulDrainHandler picks it up and
  // begins draining (reject new requests, let in-flight finish, then
  // deactivate). Not a fault — in-flight work is never failed.
  CloseConnection,
  // Sentinel giving the pipeline its event-type count; must stay last.
  Count,
};

/**
 * ThriftClientEvent — single concrete user-event type broadcast through
 * the thrift client pipeline via PipelineImpl::fireEvent /
 * ContextImpl::fireEvent (wrapped in TypeErasedBox). Handlers that care
 * declare a `static constexpr kSubscribedEvents` listing the event types
 * they handle and implement
 * `onEvent(ctx, ThriftClientEventType, const TypeErasedBox&)` (internal) or
 * `onEvent(ThriftClientEventType, const TypeErasedBox&)` (endpoints). The
 * framework links one hook per subscribed event and passes the event type
 * as the discriminator argument.
 */
struct ThriftClientEvent {
  ThriftClientEventType type;
};

} // namespace apache::thrift::fast_thrift::thrift
