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
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>

#include <thrift/test/gen-cpp2/EnumTest_types.h>
#include <thrift/test/gen-cpp2/QualifiedEnumTest_types.h>

using namespace apache::thrift;
using namespace cpp2;

TEST(QualifiedEnums, Defaults) {
  MyQualifiedStruct empty;

  BinaryProtocolWriter protWriter;
  size_t bufSize =
      Cpp2Ops<MyQualifiedStruct>::serializedSize(&protWriter, &empty);
  folly::IOBufQueue queue;
  protWriter.setOutput(&queue, bufSize);
  Cpp2Ops<MyQualifiedStruct>::write(&protWriter, &empty);

  auto buf = queue.move();
  BinaryProtocolReader protReader;
  protReader.setInput(buf.get());
  MyQualifiedStruct actual;
  Cpp2Ops<MyQualifiedStruct>::read(&protReader, &actual);

  EXPECT_EQ(MyQualifiedEnum::BAR, actual.field1().value_unchecked());
  EXPECT_EQ(MyQualifiedEnum::FOO, actual.field2().value_unchecked());
  EXPECT_EQ(MyEnum1::ME1_1, actual.field3().value_unchecked());
  EXPECT_EQ(MyEnum1::ME1_1, actual.field4().value_unchecked());
  EXPECT_EQ(MyEnum4::ME4_A, actual.field5().value_unchecked());
}

TEST(QualifiedEnums, BitwiseOps) {
  using EnumType = MyBitMaskEnum;
  EXPECT_EQ(sizeof(int32_t), sizeof(EnumType));
  EXPECT_EQ(EnumType::kBar, EnumType::kBar & EnumType(11));
  EXPECT_EQ(EnumType(5), EnumType::kFoo | EnumType::kBaz);
  EXPECT_EQ(EnumType(5), EnumType(3) ^ EnumType(6));
  EXPECT_EQ(EnumType(-1), ~EnumType(0));
}

TEST(QualifiedEnums, BitwiseOpsShort) {
  using EnumType = MyBitMaskEnumShort;
  EXPECT_EQ(sizeof(int16_t), sizeof(EnumType));
  EXPECT_EQ(EnumType::kBar, EnumType::kBar & EnumType(11));
  EXPECT_EQ(EnumType(5), EnumType::kFoo | EnumType::kBaz);
  EXPECT_EQ(EnumType(5), EnumType(3) ^ EnumType(6));
  EXPECT_EQ(EnumType(-1), ~EnumType(0));
}
