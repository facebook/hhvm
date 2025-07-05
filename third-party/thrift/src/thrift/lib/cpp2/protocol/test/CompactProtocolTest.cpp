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
#include <folly/CPortability.h>

#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

#include <thrift/lib/cpp2/protocol/test/gen-cpp2/CompactProtocolTestStructs_types.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::test;

FOLLY_DISABLE_UNDEFINED_BEHAVIOR_SANITIZER("undefined")
bool makeInvalidBool() {
  return *reinterpret_cast<const volatile bool*>("\x42");
}

void writeInvalidBoolCheck() {
  auto w = CompactProtocolWriter();
  auto q = folly::IOBufQueue();
  w.setOutput(&q);
  w.writeBool(makeInvalidBool());
  auto s = std::string();
  q.appendToString(s);
  // Die on success.
  CHECK(s != std::string(1, '\1') && s != std::string(1, '\2'))
      << "invalid bool value";
}

TEST(CompactProtocolTest, writeInvalidBool) {
  // writeBool should either throw or write a valid bool. The exact value may
  // depend on the build mode because the optimizer can make use of the UB.
  EXPECT_DEATH({ writeInvalidBoolCheck(); }, "invalid bool value");
}

TEST(CompactProtocolTest, writeStringExactly2GB) {
  auto w = CompactProtocolWriter();
  auto q = folly::IOBufQueue();
  w.setOutput(&q);
  std::string monster((uint32_t)1 << 31, 'x');
  EXPECT_THROW(w.writeString(monster), TProtocolException);
}

TEST(CompactProtocolTest, writeStringExceeds2GB) {
  auto w = CompactProtocolWriter();
  auto q = folly::IOBufQueue();
  w.setOutput(&q);
  std::string monster(((uint32_t)1 << 31) + 100, 'x');
  EXPECT_THROW(w.writeString(monster), TProtocolException);
}

TEST(CompactProtocolTest, writeStringExactly4GB) {
  auto w = CompactProtocolWriter();
  auto q = folly::IOBufQueue();
  w.setOutput(&q);
  std::string monster((uint64_t)1 << 32, 'x');
  EXPECT_THROW(w.writeString(monster), TProtocolException);
}

TEST(CompactProtocolTest, writeStringExceeds4GB) {
  auto w = CompactProtocolWriter();
  auto q = folly::IOBufQueue();
  w.setOutput(&q);
  std::string monster(((uint64_t)1 << 32) + 100, 'x');
  EXPECT_THROW(w.writeString(monster), TProtocolException);
}

// We make this a template so that we can use it both for OriginalStruct and
// UpdatedStruct.
template <typename T>
void initializeOriginalStruct(T& obj) {
  obj.f1() = true;
  obj.f3() = false;
  obj.f6() = 50;
  obj.f8() = 1200;
  obj.f48() = 1300;
  obj.f100() = 1600;
  obj.f20000() = 1.0;
  obj.f20030_ref()->elem() = 0;
  obj.f20032() = "def";
  obj.f20034_ref()->push_back(0);
}

void initializeUpdatedStruct(UpdatedStruct& obj) {
  initializeOriginalStruct(obj);
  obj.f2() = false;
  obj.f4() = true;
  obj.f5() = false;
  obj.f7() = 1100;
  obj.f68() = 1400;
  obj.f88() = 1500;
  obj.f20020() = "abc";
  obj.f20031()->elem() = 1;
  obj.f20033() = "ghi";
  obj.f20035()->push_back(1);
}

TEST(CompactProtocolTest, ParsesOriginalViaRead) {
  Serializer<CompactProtocolReader, CompactProtocolWriter> serializer;

  OriginalStruct original;
  initializeOriginalStruct(original);

  std::string serialized;
  serializer.serialize(original, &serialized);

  for (size_t i = 0; i < serialized.size(); i++) {
    for (size_t j = i; j < serialized.size(); j++) {
      auto buf1 = folly::IOBuf::wrapBuffer(serialized.data(), i);
      auto buf2 = folly::IOBuf::wrapBuffer(serialized.data() + i, j - i);
      auto buf3 = folly::IOBuf::wrapBuffer(
          serialized.data() + j, serialized.size() - j);
      buf1->appendToChain(std::move(buf2));
      buf1->appendToChain(std::move(buf3));

      OriginalStruct deserialized;
      serializer.deserialize(buf1.get(), deserialized);
      EXPECT_EQ(original, deserialized);
    }
  }
}

TEST(CompactProtocolTest, ParsesUpdatedViaRead) {
  Serializer<CompactProtocolReader, CompactProtocolWriter> serializer;

  OriginalStruct original;
  initializeOriginalStruct(original);

  UpdatedStruct updated;
  initializeUpdatedStruct(updated);

  std::string serialized;
  serializer.serialize(updated, &serialized);

  for (size_t i = 0; i < serialized.size(); i++) {
    for (size_t j = i; j < serialized.size(); j++) {
      auto buf1 = folly::IOBuf::wrapBuffer(serialized.data(), i);
      auto buf2 = folly::IOBuf::wrapBuffer(serialized.data() + i, j - i);
      auto buf3 = folly::IOBuf::wrapBuffer(
          serialized.data() + j, serialized.size() - j);
      buf1->appendToChain(std::move(buf2));
      buf1->appendToChain(std::move(buf3));

      OriginalStruct deserialized;
      serializer.deserialize(buf1.get(), deserialized);
      EXPECT_EQ(original, deserialized);
    }
  }
}

TEST(CompactProtocolReaderTest, ParsesOriginalManually) {
  Serializer<CompactProtocolReader, CompactProtocolWriter> serializer;

  OriginalStruct original;
  initializeOriginalStruct(original);

  std::string serialized;
  serializer.serialize(original, &serialized);

  std::string unusedFieldNameString;

  for (size_t i = 0; i < serialized.size(); i++) {
    for (size_t j = i; j < serialized.size(); j++) {
      auto buf1 = folly::IOBuf::wrapBuffer(serialized.data(), i);
      auto buf2 = folly::IOBuf::wrapBuffer(serialized.data() + i, j - i);
      auto buf3 = folly::IOBuf::wrapBuffer(
          serialized.data() + j, serialized.size() - j);
      buf1->appendToChain(std::move(buf2));
      buf1->appendToChain(std::move(buf3));

      CompactProtocolReader reader;
      reader.setInput(buf1.get());

      TType fieldType;
      TType listElemType;
      uint32_t listSize;
      int16_t fieldId;

      reader.readStructBegin(unusedFieldNameString);

      // f1
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_BOOL);
      EXPECT_EQ(fieldId, 1);
      bool f1;
      reader.readBool(f1);
      EXPECT_EQ(true, f1);

      // f3
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_BOOL);
      EXPECT_EQ(fieldId, 3);
      bool f3;
      reader.readBool(f3);
      EXPECT_EQ(false, f3);

      // f6
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_BYTE);
      EXPECT_EQ(fieldId, 6);
      int8_t f6;
      reader.readByte(f6);
      EXPECT_EQ(50, f6);

      // f8
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_I16);
      EXPECT_EQ(fieldId, 8);
      int16_t f8;
      reader.readI16(f8);
      EXPECT_EQ(1200, f8);

      // f48
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_I32);
      EXPECT_EQ(fieldId, 48);
      int32_t f48;
      reader.readI32(f48);
      EXPECT_EQ(1300, f48);

      // f100
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_I64);
      EXPECT_EQ(fieldId, 100);
      int64_t f100;
      reader.readI64(f100);
      EXPECT_EQ(1600, f100);

      // f20000
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_DOUBLE);
      EXPECT_EQ(fieldId, 20000);
      double f20000;
      reader.readDouble(f20000);
      EXPECT_EQ(1.0, f20000);

      // f20030
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_STRUCT);
      EXPECT_EQ(fieldId, 20030);
      reader.readStructBegin(unusedFieldNameString);
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_I64);
      EXPECT_EQ(fieldId, 1);
      int64_t f20030_1;
      reader.readI64(f20030_1);
      EXPECT_EQ(0, f20030_1);
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_STOP);
      EXPECT_EQ(fieldId, 0);
      reader.readStructEnd();

      // f20032
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_STRING);
      EXPECT_EQ(fieldId, 20032);
      std::string f20032;
      reader.readString(f20032);
      EXPECT_EQ("def", f20032);

      // f20034
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_LIST);
      EXPECT_EQ(fieldId, 20034);
      reader.readListBegin(listElemType, listSize);
      EXPECT_EQ(listElemType, T_I32);
      EXPECT_EQ(listSize, 1);
      int32_t f20034_0;
      reader.readI32(f20034_0);
      EXPECT_EQ(f20034_0, 0);
      reader.readListEnd();

      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_STOP);
      EXPECT_EQ(fieldId, 0);
      reader.readStructEnd();

      size_t pos = reader.getCursor().getCurrentPosition();
      EXPECT_EQ(serialized.size(), pos);
      EXPECT_FALSE(reader.getCursor().canAdvance(1));
    }
  }
}

TEST(CompactProtocolReaderTest, ParsesUpdatedManually) {
  Serializer<CompactProtocolReader, CompactProtocolWriter> serializer;

  OriginalStruct original;
  initializeOriginalStruct(original);

  UpdatedStruct updated;
  initializeUpdatedStruct(updated);

  std::string serialized;
  serializer.serialize(updated, &serialized);

  std::string unusedFieldNameString;

  for (size_t i = 0; i < serialized.size(); i++) {
    for (size_t j = i; j < serialized.size(); j++) {
      auto buf1 = folly::IOBuf::wrapBuffer(serialized.data(), i);
      auto buf2 = folly::IOBuf::wrapBuffer(serialized.data() + i, j - i);
      auto buf3 = folly::IOBuf::wrapBuffer(
          serialized.data() + j, serialized.size() - j);
      buf1->appendToChain(std::move(buf2));
      buf1->appendToChain(std::move(buf3));

      CompactProtocolReader reader;
      reader.setInput(buf1.get());

      TType fieldType;
      TType listElemType;
      uint32_t listSize;
      int16_t fieldId;

      reader.readStructBegin(unusedFieldNameString);

      // f1
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_BOOL);
      EXPECT_EQ(fieldId, 1);
      bool f1;
      reader.readBool(f1);
      EXPECT_EQ(true, f1);

      // f2 (skipped)
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_BOOL);
      EXPECT_EQ(fieldId, 2);
      reader.skip(fieldType);

      // f3
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_BOOL);
      EXPECT_EQ(fieldId, 3);
      bool f3;
      reader.readBool(f3);
      EXPECT_EQ(false, f3);

      // f4 (skipped)
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_BOOL);
      EXPECT_EQ(fieldId, 4);
      reader.skip(fieldType);

      // f5 (skipped)
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_BOOL);
      EXPECT_EQ(fieldId, 5);
      reader.skip(fieldType);

      // f6
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_BYTE);
      EXPECT_EQ(fieldId, 6);
      int8_t f6;
      reader.readByte(f6);
      EXPECT_EQ(50, f6);

      // f7
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_I32);
      EXPECT_EQ(fieldId, 7);
      reader.skip(fieldType);

      // f8
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_I16);
      EXPECT_EQ(fieldId, 8);
      int16_t f8;
      reader.readI16(f8);
      EXPECT_EQ(1200, f8);

      // f48
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_I32);
      EXPECT_EQ(fieldId, 48);
      int32_t f48;
      reader.readI32(f48);
      EXPECT_EQ(1300, f48);

      // f68 (skipped)
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_I32);
      EXPECT_EQ(fieldId, 68);
      reader.skip(fieldType);

      // f88 (skipped)
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_I32);
      EXPECT_EQ(fieldId, 88);
      reader.skip(fieldType);

      // f100
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_I64);
      EXPECT_EQ(fieldId, 100);
      int64_t f100;
      reader.readI64(f100);
      EXPECT_EQ(1600, f100);

      // f20000
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_DOUBLE);
      EXPECT_EQ(fieldId, 20000);
      double f20000;
      reader.readDouble(f20000);
      EXPECT_EQ(1.0, f20000);

      // f20020 (skipped)
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_STRING);
      EXPECT_EQ(fieldId, 20020);
      reader.skip(fieldType);

      // f20030
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_STRUCT);
      EXPECT_EQ(fieldId, 20030);
      reader.readStructBegin(unusedFieldNameString);
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_I64);
      EXPECT_EQ(fieldId, 1);
      int64_t f20030_1;
      reader.readI64(f20030_1);
      EXPECT_EQ(0, f20030_1);
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_STOP);
      EXPECT_EQ(fieldId, 0);
      reader.readStructEnd();

      // f20031 (skipped)
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_STRUCT);
      EXPECT_EQ(fieldId, 20031);
      reader.skip(fieldType);

      // f20032
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_STRING);
      EXPECT_EQ(fieldId, 20032);
      std::string f20032;
      reader.readString(f20032);
      EXPECT_EQ("def", f20032);

      // f20033 (skipped)
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_STRING);
      EXPECT_EQ(fieldId, 20033);
      reader.skip(fieldType);

      // f20034
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_LIST);
      EXPECT_EQ(fieldId, 20034);
      reader.readListBegin(listElemType, listSize);
      EXPECT_EQ(listElemType, T_I32);
      EXPECT_EQ(listSize, 1);
      int32_t f20034_0;
      reader.readI32(f20034_0);
      EXPECT_EQ(f20034_0, 0);
      reader.readListEnd();

      // f20035 (skipped)
      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_LIST);
      EXPECT_EQ(fieldId, 20035);
      reader.skip(fieldType);

      reader.readFieldBegin(unusedFieldNameString, fieldType, fieldId);
      EXPECT_EQ(fieldType, T_STOP);
      EXPECT_EQ(fieldId, 0);
      reader.readStructEnd();

      size_t pos = reader.getCursor().getCurrentPosition();
      EXPECT_EQ(serialized.size(), pos);
      EXPECT_FALSE(reader.getCursor().canAdvance(1));
    }
  }
}
