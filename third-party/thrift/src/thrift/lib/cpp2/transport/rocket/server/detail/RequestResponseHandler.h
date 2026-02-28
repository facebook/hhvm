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

namespace apache::thrift::rocket {

template <typename ConnectionT, template <typename> class ConnectionAdapter>
class RequestResponseHandler {
  using Connection = ConnectionAdapter<ConnectionT>;

 public:
  explicit RequestResponseHandler(Connection* connection) noexcept
      : connection_(connection) {}

  void handle(RequestResponseFrame&& frame) noexcept {
    auto streamId = frame.streamId();
    if (UNLIKELY(frame.hasFollows())) {
      connection_->emplacePartialFrame(streamId, std::move(frame));
    } else {
      RocketServerFrameContext(*connection_->getWrappedConnection(), streamId)
          .onFullFrame(std::move(frame));
    }
  }

 private:
  Connection* connection_;
};

} // namespace apache::thrift::rocket
