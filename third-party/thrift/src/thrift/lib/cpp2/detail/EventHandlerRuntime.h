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

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <thrift/lib/cpp2/Flags.h>

THRIFT_FLAG_DECLARE_string(client_methods_bypass_eventhandlers);

namespace apache::thrift::detail {

/**
 * This class provides runtime support for EventHandlerBase types. It stores
 * inherently global concerns (such as Thrift flags) related to
 * TProcessorEventHandler and TClientBase.
 */
class EventHandlerRuntime {
 private:
  EventHandlerRuntime() = delete;

 public:
  static bool isClientMethodBypassed(
      std::string_view serviceName, std::string_view methodName);

  struct MethodNameSet {
    std::vector<std::string> serviceNames;
    std::vector<std::string> methodNames;
  };
  static void setClientMethodsToBypass(MethodNameSet methods);
};

} // namespace apache::thrift::detail
