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

#include <thrift/lib/cpp2/type/Protocol.h>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/type/Testing.h>

namespace apache::thrift::type {

namespace {
using test::makeProtocol;

template <StandardProtocol StdProtocol>
void testStandardProtocol(std::string_view expectedName) {
  SCOPED_TRACE(expectedName);

  // 3 ways to get the protocol all return the same value.
  const auto& protocol = Protocol::get<StdProtocol>();
  EXPECT_EQ(Protocol(StdProtocol), protocol);
  const auto asCustom = makeProtocol(std::string(expectedName));
  EXPECT_NE(asCustom, protocol);
  EXPECT_THROW(validateProtocol(asCustom), std::invalid_argument);

  // We get the expected name.
  EXPECT_EQ(protocol.name(), expectedName);
}

TEST(ProtocolTest, ProtocolStruct) {}

TEST(ProtocolTest, Standard) {
  testStandardProtocol<StandardProtocol::Custom>("Custom");
  testStandardProtocol<StandardProtocol::Binary>("Binary");
  testStandardProtocol<StandardProtocol::Compact>("Compact");
  testStandardProtocol<StandardProtocol::Json>("Json");
  testStandardProtocol<StandardProtocol::SimpleJson>("SimpleJson");
}

TEST(ProtocolTest, Empty) {
  Protocol empty;
  validateProtocol(empty);
  EXPECT_EQ(empty.name(), "Custom");
  EXPECT_EQ(empty.standard(), StandardProtocol::Custom);
  EXPECT_EQ(empty.custom(), "");

  EXPECT_EQ(empty, kNoProtocol);
  EXPECT_EQ(empty, makeProtocol(""));
  EXPECT_EQ(empty, Protocol::get<StandardProtocol::Custom>());

  EXPECT_NE(empty, makeProtocol("Custom"));
  EXPECT_THROW(validateProtocol(makeProtocol("Custom")), std::invalid_argument);
}

TEST(ProtocolTest, Unknown) {
  EXPECT_EQ(test::UnknownProtocol().name(), "");
}

TEST(ProtocolTest, Custom) {
  Protocol protocol = makeProtocol("hi");
  EXPECT_EQ(protocol.name(), "hi");
  EXPECT_EQ(protocol.standard(), StandardProtocol::Custom);
  EXPECT_EQ(protocol.custom(), "hi");
  EXPECT_EQ(makeProtocol("hi"), protocol);
  EXPECT_NE(makeProtocol("bye"), protocol);
}

TEST(ProtocolTest, ValidateProtocol) {
  const auto good = "foo.com/my/protocol";
  const auto bad = "foo.com:42/my/protocol";
  EXPECT_THROW(validateProtocol(makeProtocol(bad)), std::invalid_argument);
  validateProtocol(makeProtocol(good));
}

TEST(ProtocolTest, FromName) {
  EXPECT_EQ(
      Protocol::fromName("Binary"), Protocol::get<StandardProtocol::Binary>());
  EXPECT_EQ(
      Protocol::fromName("Compact"),
      Protocol::get<StandardProtocol::Compact>());
  EXPECT_EQ(
      Protocol::fromName("Json"), Protocol::get<StandardProtocol::Json>());
  EXPECT_EQ(
      Protocol::fromName("SimpleJson"),
      Protocol::get<StandardProtocol::SimpleJson>());

  // Case sensitive.
  EXPECT_THROW(Protocol::fromName("binary"), std::invalid_argument);
  // Rejects invalid custom names.
  EXPECT_THROW(
      Protocol::fromName("foo.com:42/my/protocol"), std::invalid_argument);

  // Accepts valid custom names.
  EXPECT_EQ(
      Protocol::fromName("foo.com/my/protocol"),
      makeProtocol("foo.com/my/protocol"));
}

} // namespace
} // namespace apache::thrift::type
