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

#include <thrift/conformance/data/internal/TestGenerator.h>

using apache::thrift::protocol::serializeObject;

namespace apache::thrift::conformance::data::detail {

[[nodiscard]] std::unique_ptr<folly::IOBuf> serializeThriftStruct(
    const protocol::Object& a, const Protocol& protocol) {
  switch (auto p = protocol.standard()) {
    case StandardProtocol::Compact:
      return serializeObject<apache::thrift::CompactProtocolWriter>(a);
    case StandardProtocol::Binary:
      return serializeObject<apache::thrift::BinaryProtocolWriter>(a);
    default:
      throw std::invalid_argument(
          "Unsupported protocol: " + util::enumNameSafe(p));
  }
}

} // namespace apache::thrift::conformance::data::detail
