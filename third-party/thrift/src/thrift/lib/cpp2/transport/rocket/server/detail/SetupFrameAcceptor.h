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

#include <utility>
#include <thrift/lib/cpp2/transport/rocket/server/detail/ConnectionAdapter.h>

namespace apache::thrift::rocket {

class SetupFrame;

/**
 * Accepts setup frames and creates a RocketServerHandler.
 */
template <
    typename ConnectionT,
    template <typename> class ConnectionAdapter,
    typename RocketServerHandler>
class SetupFrameAcceptor {
  using Connection = ConnectionAdapter<ConnectionT>;

 public:
  SetupFrameAcceptor(Connection& connection, RocketServerHandler& handler)
      : connection_(&connection), handler_(&handler) {}

  void handle(SetupFrame&& frame) noexcept {
    handler_->handleSetupFrame(
        std::move(frame), *connection_->getWrappedConnection());
  }

 private:
  Connection* connection_;
  RocketServerHandler* handler_;
};

} // namespace apache::thrift::rocket
