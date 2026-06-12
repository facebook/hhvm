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

#include <cstddef>
#include <cstdint>

#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftClientEventType — the event enum for thrift-client pipelines.
 *
 * Used as the channel pipeline's EventEnum template parameter: each value
 * names a distinct user event that a pipeline handler or endpoint may
 * subscribe to via `kSubscribedEvents`. `fireEvent(type, ...)` reaches only
 * the subscribers of `type`. Each value carries its own payload as needed, or
 * none for pure signals.
 *
 * The trailing `Count` sentinel gives the pipeline the number of event types
 * at compile time and must remain the last enumerator.
 */
enum class ThriftClientEventType : std::uint32_t {
  // Per-batch write completion relayed up from the rocket pipeline by
  // ThriftClientTransportAdapter. Carries a ThriftClientWriteCompleteEvent.
  WriteComplete,
  // The transport is draining: the server sent a graceful close (rocket
  // CONNECTION_CLOSE). ThriftClientTransportAdapter translates the
  // rocket-native close notification into this event; the pipeline-resident
  // ThriftClientGracefulDrainHandler picks it up and begins draining (reject
  // new requests, let in-flight finish, then deactivate). Not a fault —
  // in-flight work is never failed. Carries no payload — the type alone is the
  // signal.
  CloseConnection,
  // Sentinel: number of event types. Keep last.
  Count,
};

/**
 * Payload for ThriftClientEventType::WriteComplete — the completion of one
 * rocket-frame batch, relayed up from the rocket pipeline. `frameCount` is the
 * number of frames in the batch.
 */
struct ThriftClientWriteCompleteEvent {
  apache::thrift::fast_thrift::transport::WriteCompletionStatus status;
  size_t frameCount;
  size_t bytes;
};

} // namespace apache::thrift::fast_thrift::thrift
