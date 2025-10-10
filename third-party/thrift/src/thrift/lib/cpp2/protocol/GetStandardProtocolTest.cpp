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

#include <concepts>

namespace apache::thrift::protocol {

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

TEST(GetStandardProtocolTest, Json) {
  EXPECT_EQ(
      get_standard_protocol<JSONProtocolReader>, type::StandardProtocol::Json);
  EXPECT_EQ(
      get_standard_protocol<JSONProtocolWriter>, type::StandardProtocol::Json);
}

TEST(GetStandardProtocolTest, SimpleJson) {
  EXPECT_EQ(
      get_standard_protocol<SimpleJSONProtocolReader>,
      type::StandardProtocol::SimpleJson);
  EXPECT_EQ(
      get_standard_protocol<SimpleJSONProtocolWriter>,
      type::StandardProtocol::SimpleJson);
}

TEST(GetStandardProtocolTest, ProtocolReaderForBinary) {
  EXPECT_TRUE((std::same_as<
               ProtocolReaderFor<type::StandardProtocol::Binary>,
               BinaryProtocolReader>));
}

TEST(GetStandardProtocolTest, ProtocolReaderForCompact) {
  EXPECT_TRUE((std::same_as<
               ProtocolReaderFor<type::StandardProtocol::Compact>,
               CompactProtocolReader>));
}

TEST(GetStandardProtocolTest, ProtocolReaderForJson) {
  EXPECT_TRUE((std::same_as<
               ProtocolReaderFor<type::StandardProtocol::Json>,
               JSONProtocolReader>));
}

TEST(GetStandardProtocolTest, ProtocolReaderForSimpleJson) {
  EXPECT_TRUE((std::same_as<
               ProtocolReaderFor<type::StandardProtocol::SimpleJson>,
               SimpleJSONProtocolReader>));
}

TEST(GetStandardProtocolTest, ProtocolWriterForBinary) {
  EXPECT_TRUE((std::same_as<
               ProtocolWriterFor<type::StandardProtocol::Binary>,
               BinaryProtocolWriter>));
}

TEST(GetStandardProtocolTest, ProtocolWriterForCompact) {
  EXPECT_TRUE((std::same_as<
               ProtocolWriterFor<type::StandardProtocol::Compact>,
               CompactProtocolWriter>));
}

TEST(GetStandardProtocolTest, ProtocolWriterForJson) {
  EXPECT_TRUE((std::same_as<
               ProtocolWriterFor<type::StandardProtocol::Json>,
               JSONProtocolWriter>));
}

TEST(GetStandardProtocolTest, ProtocolWriterForSimpleJson) {
  EXPECT_TRUE((std::same_as<
               ProtocolWriterFor<type::StandardProtocol::SimpleJson>,
               SimpleJSONProtocolWriter>));
}

} // namespace apache::thrift::protocol
