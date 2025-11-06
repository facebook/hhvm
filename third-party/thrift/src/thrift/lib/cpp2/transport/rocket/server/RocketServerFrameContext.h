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

#include <folly/io/async/HHWheelTimer.h>

#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/server/IRocketServerConnection.h>

namespace folly {
class EventBase;
class IOBuf;
} // namespace folly

namespace apache::thrift::rocket {

class RocketException;
class RocketSinkClientCallback;
class RocketStreamClientCallback;

class RocketServerFrameContext {
 public:
  RocketServerFrameContext(
      IRocketServerConnection& connection, StreamId streamId);
  RocketServerFrameContext(RocketServerFrameContext&& other) noexcept;
  RocketServerFrameContext& operator=(RocketServerFrameContext&&) = delete;
  ~RocketServerFrameContext();

  void sendPayload(
      Payload&& payload,
      Flags flags,
      apache::thrift::MessageChannel::SendCallbackPtr cb);
  void sendError(
      RocketException&& rex,
      apache::thrift::MessageChannel::SendCallbackPtr cb);

  folly::EventBase& getEventBase() const;

  StreamId streamId() const { return streamId_; }

  IRocketServerConnection& connection() {
    DCHECK(connection_);
    return *connection_;
  }

  void unsetMarkRequestComplete() {
    DCHECK(markRequestComplete_);
    markRequestComplete_ = false;
  }

  void onFullFrame(RequestResponseFrame&& fullFrame) &&;
  void onFullFrame(RequestFnfFrame&& fullFrame) &&;
  void onFullFrame(RequestStreamFrame&& fullFrame) &&;
  void onFullFrame(RequestChannelFrame&& fullFrame) &&;

 private:
  IRocketServerConnection* connection_{nullptr};
  const StreamId streamId_;
  bool markRequestComplete_{true};
};

} // namespace apache::thrift::rocket
