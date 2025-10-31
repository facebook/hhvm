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

#include <thrift/lib/cpp2/protocol/VirtualProtocol.h>

#include <stdexcept>

#include <folly/Conv.h>
#include <folly/lang/Switch.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache::thrift {

std::unique_ptr<VirtualReaderBase> makeVirtualReader(ProtocolType type) {
  FOLLY_NON_EXHAUSTIVE_SWITCH({
    switch (type) {
      case ProtocolType::T_BINARY_PROTOCOL:
        return std::make_unique<VirtualReader<BinaryProtocolReader>>();
      case ProtocolType::T_COMPACT_PROTOCOL:
        return std::make_unique<VirtualReader<CompactProtocolReader>>();
      default:
        throw std::invalid_argument(
            folly::to<std::string>("Invalid protocol type ", type));
    }
  });
}

} // namespace apache::thrift
