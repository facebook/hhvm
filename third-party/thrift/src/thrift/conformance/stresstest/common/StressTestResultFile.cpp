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

#include <thrift/conformance/stresstest/common/StressTestResultFile.h>

#include <folly/FileUtil.h>
#include <thrift/conformance/stresstest/if/gen-cpp2/StressTestResult_types_custom_protocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

namespace apache::thrift::stress {
namespace {

template <typename Result>
void writeResult(const std::string& path, const Result& result) {
  auto json = SimpleJSONSerializer::serialize<std::string>(result);
  json.push_back('\n');
  folly::writeFileAtomic(path, json);
}

} // namespace

void writeStressTestResult(
    const std::string& path, const ClientResult& result) {
  writeResult(path, result);
}

} // namespace apache::thrift::stress
