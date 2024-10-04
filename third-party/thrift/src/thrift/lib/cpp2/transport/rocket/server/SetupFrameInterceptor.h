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

#include <optional>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>

namespace apache {
namespace thrift {

class RequestSetupMetadata;

namespace rocket {
/*
 * An interface used by ThriftServer to provide intercepting setup frames and
 * rejecting connection establishment. The api allows the user to return a
 * specific error message to be propagated back to the client.
 */
class SetupFrameInterceptor {
 public:
  SetupFrameInterceptor() = default;
  virtual ~SetupFrameInterceptor() = default;
  SetupFrameInterceptor(const SetupFrameInterceptor&) = delete;

  /*
   * Whether to accept setup frame or not. If rejected the error message
   * will be returned to the client.
   */
  virtual folly::Expected<folly::Unit, std::runtime_error> acceptSetup(
      const RequestSetupMetadata&, const ConnectionLoggingContext&) = 0;
};

} // namespace rocket
} // namespace thrift
} // namespace apache
