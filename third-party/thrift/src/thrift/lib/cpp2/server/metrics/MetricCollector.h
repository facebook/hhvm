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

#include <cstdint>
#include <variant>

#include <thrift/lib/cpp2/server/Overload.h>

#pragma once

namespace apache::thrift {

class IMetricCollector {
 public:
  virtual ~IMetricCollector() = default;

  virtual void requestReceived() = 0;

  struct RequestRejectedScope {
    // Reasons
    struct Unknown {};

    struct ServerOverloaded {
      const LoadShedder loadShedder;
    };

    using Reason = std::variant<Unknown, ServerOverloaded>;

    const Reason reason{Unknown{}};
  };

  virtual void requestRejected(const RequestRejectedScope&) = 0;
};

} // namespace apache::thrift
