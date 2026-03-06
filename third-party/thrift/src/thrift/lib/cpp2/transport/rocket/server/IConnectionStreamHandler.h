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

#include <thrift/lib/cpp2/async/StreamPayload.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

namespace apache::thrift::rocket {

/**
 * Connection-facing interface for stream/sink/bidi callback handlers.
 *
 * Each handler receives typed frame objects from the connection and
 * processes them internally. The connection constructs frames from raw
 * bytes and dispatches via these virtual calls, eliminating variant_match.
 *
 * Subclasses: RocketStreamClientCallback, RocketSinkClientCallback,
 *             RocketBiDiClientCallback.
 */
class IConnectionStreamHandler {
 public:
  virtual ~IConnectionStreamHandler() = default;

  virtual StreamId streamId() const = 0;

  /**
   * Typed frame handlers — called by the connection after constructing
   * the frame object. Each subclass implements supported frame types and
   * closes the connection for unsupported types.
   */
  virtual void handleFrame(RequestNFrame&&) = 0;
  virtual void handleFrame(CancelFrame&&) = 0;
  virtual void handleFrame(PayloadFrame&&) = 0;
  virtual void handleFrame(ErrorFrame&&) = 0;
  virtual void handleFrame(ExtFrame&&) = 0;

  /**
   * Called when the connection is closing. The handler should notify its
   * server callback and perform any cleanup.
   */
  virtual void handleConnectionClose() = 0;

  /** Called when the connection receives a metadata push for this stream. */
  virtual void handleStreamHeadersPush(HeadersPayload&&) {}

  /** Called when the connection pauses streams due to backpressure. */
  virtual void handlePausedByConnection() {}

  /** Called when the connection resumes streams after backpressure. */
  virtual void handleResumedByConnection() {}
};

} // namespace apache::thrift::rocket
