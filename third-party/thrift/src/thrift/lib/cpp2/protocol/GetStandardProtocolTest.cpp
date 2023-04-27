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

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/protocol/GetStandardProtocol.h>

namespace apache::thrift::protocol::detail {

TEST(GetStandardProtocolTest, Binary) {
  EXPECT_EQ(
      get_standard_protocol<BinaryProtocolReader>,
      type::StandardProtocol::Binary);
  EXPECT_EQ(
      get_standard_protocol<BinaryProtocolWriter>,
      type::StandardProtocol::Binary);
}

TEST(GetStandardProtocolTest, Compact) {
  EXPECT_EQ(
      get_standard_protocol<CompactProtocolReader>,
      type::StandardProtocol::Compact);
  EXPECT_EQ(
      get_standard_protocol<CompactProtocolWriter>,
      type::StandardProtocol::Compact);
}
} // namespace apache::thrift::protocol::detail
