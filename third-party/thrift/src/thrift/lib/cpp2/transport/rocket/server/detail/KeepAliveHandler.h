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

#include <thrift/lib/cpp2/transport/rocket/core/FrameUtil.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/OutgoingFrameHandler.h>

namespace apache::thrift::rocket {

template <typename ConnectionT, template <typename> class ConnectionAdapter>
class KeepAliveHandler {
  using Connection = ConnectionAdapter<ConnectionT>;
  using OutgoingFrameHandler = apache::thrift::rocket::
      OutgoingFrameHandler<ConnectionT, ConnectionAdapter>;

 public:
  KeepAliveHandler(const KeepAliveHandler&) = delete;
  KeepAliveHandler(KeepAliveHandler&&) = delete;
  KeepAliveHandler& operator=(const KeepAliveHandler&) = delete;
  KeepAliveHandler& operator=(KeepAliveHandler&&) = delete;
  ~KeepAliveHandler() = default;
  explicit KeepAliveHandler(Connection& connection) noexcept
      : connection_(&connection) {}

  bool handle(KeepAliveFrame&& frame) noexcept {
    KeepAliveFrame keepAliveFrame{std::move(frame)};

    if (keepAliveFrame.hasRespondFlag()) {
      connection_->sendFrame(std::move(keepAliveFrame));
    }
    return true;
  }

 private:
  Connection* connection_;
};

} // namespace apache::thrift::rocket
