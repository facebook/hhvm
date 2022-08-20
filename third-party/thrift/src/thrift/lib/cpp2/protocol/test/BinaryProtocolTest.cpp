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

#include <folly/CPortability.h>
#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;

namespace {

class BinaryProtocolTest : public testing::Test {};

TEST_F(BinaryProtocolTest, readInvalidBool) {
  uint8_t data[] = {0, 1, 2};
  auto buf = folly::IOBuf::wrapBufferAsValue(folly::range(data));

  BinaryProtocolReader inprot;
  bool value{};
  inprot.setInput(&buf);
  inprot.readBool(value);
  EXPECT_EQ(false, value) << "sanity check";
  inprot.readBool(value);
  EXPECT_EQ(true, value) << "sanity check";
  EXPECT_THROW(inprot.readBool(value), TProtocolException);
}

FOLLY_DISABLE_UNDEFINED_BEHAVIOR_SANITIZER("undefined")
bool makeInvalidBool() {
  return *reinterpret_cast<const volatile bool*>("\x42");
}

TEST_F(BinaryProtocolTest, writeInvalidBool) {
  auto w = BinaryProtocolWriter();
  auto q = folly::IOBufQueue();
  w.setOutput(&q);
  // writeBool should either fail CHECK or write a valid bool.
  EXPECT_DEATH(
      {
        w.writeBool(makeInvalidBool());
        auto s = std::string();
        q.appendToString(s);
        CHECK(s != std::string(1, '\0')); // Die on success.
      },
      "Check failed");
}

} // namespace
