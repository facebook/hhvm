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

#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/ThriftServerConfig.h>
#include <thrift/lib/cpp2/server/overload/IOverloadChecker.h>

namespace apache::thrift {

class CurrentlyOverloadedChecker final : public IOverloadChecker {
 public:
  CurrentlyOverloadedChecker(
      IsOverloadedFunc& isOverloaded,
      ThriftServerConfig& config,
      ThriftServer& server)
      : isOverloaded_(isOverloaded), config_(config), server_(server) {}

 protected:
  folly::Optional<OverloadResult> checkOverload(
      const CheckOverloadParams) override;

 private:
  IsOverloadedFunc& isOverloaded_;
  ThriftServerConfig& config_;
  ThriftServer& server_;
};
} // namespace apache::thrift
