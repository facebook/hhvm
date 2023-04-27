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

#include <string>
#include <vector>

#include <folly/portability/GFlags.h>
#include <thrift/lib/cpp2/server/RequestsRegistry.h>

FOLLY_GFLAGS_DECLARE_uint32(thrift_server_request_debug_log_entries_max);

namespace apache {
namespace thrift {

void appendRequestDebugLog(std::string&&);
void appendRequestDebugLog(const std::string&);
std::vector<std::string> collectRequestDebugLog(
    const std::shared_ptr<folly::RequestContext>&);
std::vector<std::string> collectRequestDebugLog(
    const RequestsRegistry::DebugStub&);

} // namespace thrift
} // namespace apache
