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

#include <thrift/lib/cpp2/transport/rocket/core/UnifexSupport.h>
#include <thrift/lib/cpp2/transport/rocket/core/server/OutgoingFrameHandler.h>

namespace apache::thrift::rocket {

class KeepAliveHandler {
  using async_scope = unifex_support::async_scope;

 public:
  KeepAliveHandler(
      ConnectionState& connectionState,
      OutgoingFrameHandler& outgoingFrameHandler);

  bool handle(KeepAliveFrame&&) noexcept;

 private:
  ConnectionState& connectionState_;
  OutgoingFrameHandler& outgoingFrameHandler_;
};

} // namespace apache::thrift::rocket
