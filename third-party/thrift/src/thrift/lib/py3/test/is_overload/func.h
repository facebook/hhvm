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

#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

namespace thrift::py3::test {

bool isOverloaded(
    const apache::thrift::transport::THeader::StringToStringMap& /* headers */,
    const std::string& method_name) {
  return method_name == "overloaded_method";
}

bool checkOverload(
    const std::shared_ptr<apache::thrift::ThriftServer> server,
    const std::string method_name) {
  // dummy test doesn't use the headers, so pass nullptr
  auto ret = server->checkOverload({}, method_name);
  // ret will contain the error code if there is an overload
  // otherwise, it will return no value
  return ret.hasValue();
}
} // namespace thrift::py3::test
