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

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/frozen/Frozen.h>
#include <thrift/lib/cpp2/frozen/test/gen-cpp2/Binary_layouts.h>

using namespace apache::thrift::frozen;
using namespace apache::thrift::test;
using folly::ByteRange;
using folly::StringPiece;

namespace {
byte test[]{0xDE, 0xAD, 0x00, 0xBE, 0xEF};
ByteRange testRange(test, sizeof(test));
StringPiece testString(testRange);

byte test2[]{0xFA, 0xCE, 0xB0, 0x0C};
ByteRange test2Range(test2, sizeof(test2));
} // namespace

TEST(FrozenIOBuf, Thrift2) {
  Binaries b2;
  *b2.normal() = testString.str();
  *b2.iobufptr() = IOBuf::copyBuffer(testRange.data(), testRange.size());

  auto fb2 = freeze(b2);
  EXPECT_EQ(testString, fb2.normal());
  EXPECT_EQ(testRange, fb2.iobufptr());
}

TEST(FrozenIOBuf, IOBufChain) {
  Binaries b2;
  auto buf1 = IOBuf::copyBuffer(testRange.data(), testRange.size());
  auto buf2 = IOBuf::copyBuffer(test2Range.data(), test2Range.size());
  buf1->appendChain(std::move(buf2));
  *b2.iobufptr() = std::move(buf1);

  auto fb2 = freeze(b2);
  EXPECT_EQ(0, fb2.normal().size());
  EXPECT_EQ(9, fb2.iobufptr().size());
  auto combined = fb2.iobufptr();
  EXPECT_TRUE(combined.startsWith(testRange));
  EXPECT_TRUE(combined.endsWith(test2Range));
}

TEST(FrozenIOBuf, IOBufValue) {
  std::string input = "hello";
  Binaries bin;
  *bin.iobuf() = IOBuf(IOBuf::COPY_BUFFER, input);

  auto fbin = freeze(bin);
  EXPECT_EQ(input.size(), fbin.iobuf().size());
  auto fstr = fbin.iobuf();
  EXPECT_EQ(fstr.str(), input);
}
