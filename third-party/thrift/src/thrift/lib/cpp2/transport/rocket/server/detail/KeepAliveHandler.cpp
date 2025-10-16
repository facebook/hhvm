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

#include <thrift/lib/cpp2/transport/rocket/core/server/KeepAliveHandler.h>

namespace apache::thrift::rocket {
KeepAliveHandler::KeepAliveHandler(
    ConnectionState& connectionState,
    OutgoingFrameHandler& outgoingFrameHandler)
    : connectionState_(connectionState),
      outgoingFrameHandler_(outgoingFrameHandler) {}

bool KeepAliveHandler::handle(KeepAliveFrame&& frame) noexcept {
  KeepAliveFrame keepAliveFrame{std::move(frame)};
  if (keepAliveFrame.hasRespondFlag()) {
    outgoingFrameHandler_.handle(std::move(frame), connectionState_);
  }
  return true;
}

} // namespace apache::thrift::rocket
