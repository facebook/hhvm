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
#include <string>

#include <thrift/lib/cpp2/transport/rocket/client/StreamChannelStatus.h>

namespace apache::thrift::rocket {

class StreamChannelStatusResponse {
 public:
  /* implicit */ StreamChannelStatusResponse(StreamChannelStatus status)
      : status_(status) {}
  StreamChannelStatusResponse(StreamChannelStatus status, std::string errorMsg)
      : status_(status), errorMsg_(std::move(errorMsg)) {}

  StreamChannelStatus getStatus() const { return status_; }
  std::string getErrorMsg() && {
    return std::move(errorMsg_).value_or("Unknown error");
  }

 private:
  StreamChannelStatus status_;
  std::optional<std::string> errorMsg_;
};

} // namespace apache::thrift::rocket
