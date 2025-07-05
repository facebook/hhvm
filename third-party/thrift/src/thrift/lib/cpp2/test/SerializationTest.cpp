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

#include <memory>

#include <fmt/core.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>

using namespace std;
using namespace folly;
using namespace apache::thrift;
using namespace apache::thrift::test;

TestStruct makeTestStruct() {
  TestStruct s;
  *s.s() = "test";
  *s.i() = 48;
  return s;
}

TEST(SerializationTest, CompactSerializerRoundtripPasses) {
  auto s = makeTestStruct();

  folly::IOBufQueue q;
  CompactSerializer::serialize(s, &q);

  TestStruct out;
  CompactSerializer::deserialize(q.front(), out);

  EXPECT_EQ(out, s);
}

TEST(SerializationTest, BinarySerializerRoundtripPasses) {
  auto s = makeTestStruct();

  folly::IOBufQueue q;
  BinarySerializer::serialize(s, &q);

  TestStruct out;
  BinarySerializer::deserialize(q.front(), out);

  EXPECT_EQ(out, s);
}

TEST(SerializationTest, SimpleJSONSerializerRoundtripPasses) {
  auto s = makeTestStruct();

  folly::IOBufQueue q;
  SimpleJSONSerializer::serialize(s, &q);

  TestStruct out;
  SimpleJSONSerializer::deserialize(q.front(), out);

  EXPECT_EQ(out, s);
}

TEST(SerializationTest, MixedRoundtripFails) {
  auto s = makeTestStruct();

  folly::IOBufQueue q;
  CompactSerializer::serialize(s, &q);

  try {
    TestStruct out;
    BinarySerializer::deserialize(q.front(), out);
    FAIL();
  } catch (...) {
    // Should underflow
  }
}

TEST(SerializationTest, DeserializeReturningObjGivenCursor) {
  using serializer = SimpleJSONSerializer;

  auto s = makeTestStruct();
  auto q = serializer::serialize<IOBufQueue>(s);

  Cursor cursor{q.front()};
  auto out = serializer::deserialize<TestStruct>(cursor);
  EXPECT_EQ(s, out);
  EXPECT_TRUE(cursor.isAtEnd());
}

TEST(SerializationTest, DeserializeReturningObjGivenCursorToMiddleOfBuffer) {
  using serializer = SimpleJSONSerializer;

  auto s = makeTestStruct();
  auto str = serializer::serialize<std::string>(s);
  // Copy the serialized data into an IOBuf, with 4 extra bytes on either end.
  IOBuf buf{IOBuf::CREATE, str.size() + (sizeof(uint32_t) * 2)};
  folly::io::Appender appender(&buf, 0);
  appender.writeBE<uint32_t>(12);
  appender(str);
  appender.writeBE<uint32_t>(34);

  // Create a Cursor pointing to the location of the serialized data
  // in the buffer.
  folly::io::Cursor cursor(&buf);
  cursor.skip(sizeof(uint32_t));

  auto out = serializer::deserialize<TestStruct>(cursor);
  EXPECT_EQ(s, out);
  cursor.skip(sizeof(uint32_t));
  EXPECT_TRUE(cursor.isAtEnd());
}

TEST(SerializationTest, DeserializeReturningObjGivenIOBuf) {
  using serializer = SimpleJSONSerializer;

  auto s = makeTestStruct();
  IOBufQueue q;
  serializer::serialize(s, &q);
  auto b = q.move();

  auto out = serializer::deserialize<TestStruct>(b.get());
  EXPECT_EQ(s, out);
}

TEST(SerializationTest, DeserializeReturningObjGivenByteRange) {
  using serializer = SimpleJSONSerializer;

  auto s = makeTestStruct();
  IOBufQueue q;
  serializer::serialize(s, &q);
  auto b = q.move();

  auto out = serializer::deserialize<TestStruct>(ByteRange(b->coalesce()));
  EXPECT_EQ(s, out);
}

TEST(SerializationTest, DeserializeReturningObjGivenStringPiece) {
  using serializer = SimpleJSONSerializer;

  auto s = makeTestStruct();
  IOBufQueue q;
  serializer::serialize(s, &q);
  auto b = q.move();

  auto out = serializer::deserialize<TestStruct>(StringPiece(b->coalesce()));
  EXPECT_EQ(s, out);
}

TEST(SerializationTest, SerializeReturningIOBufQueue) {
  using serializer = SimpleJSONSerializer;
  auto s = makeTestStruct();
  string expected;
  serializer::serialize(s, &expected);
  auto actual = serializer::serialize<IOBufQueue>(s);
  EXPECT_EQ(expected, StringPiece(actual.move()->coalesce()));
}

TEST(SerializationTest, SerializeAppendsToString) {
  using Serializer = SimpleJSONSerializer;
  auto s = makeTestStruct();
  string prefix = "existing_text";

  string target = prefix;
  Serializer::serialize(s, &target);

  folly::StringPiece source = target;
  EXPECT_TRUE(source.removePrefix(prefix));
  TestStruct check;
  Serializer::deserialize(source, check);
  EXPECT_EQ(check, s);
}

TEST(SerializationTest, SerializeAppendsToFBString) {
  using Serializer = SimpleJSONSerializer;
  auto s = makeTestStruct();
  for (fbstring prefix = "An existing string. "; prefix.size() < 2000;
       prefix += prefix) {
    fbstring target = prefix;
    Serializer::serialize(s, &target);

    folly::StringPiece source = target;
    EXPECT_TRUE(source.removePrefix(prefix));
    TestStruct check;
    Serializer::deserialize(source, check);
    EXPECT_EQ(check, s);
  }
}

TEST(SerializationTest, SerializeReturningString) {
  using serializer = SimpleJSONSerializer;
  auto s = makeTestStruct();
  string expected;
  serializer::serialize(s, &expected);
  auto actual = serializer::serialize<string>(s);
  EXPECT_EQ(expected, actual);
}

TEST(SerializationTest, SerializeReturningFBString) {
  using serializer = SimpleJSONSerializer;
  auto s = makeTestStruct();
  fbstring expected;
  serializer::serialize(s, &expected);
  auto actual = serializer::serialize<fbstring>(s);
  EXPECT_EQ(expected, actual);
}

TestStructRecursive makeTestStructRecursive(size_t levels) {
  unique_ptr<TestStructRecursive> s;
  for (size_t i = levels; i > 0; --i) {
    auto t = make_unique<TestStructRecursive>();
    *t->tag() = fmt::format("level-{}", i);
    t->cdr() = std::move(s);
    s = std::move(t);
  }
  TestStructRecursive ret;
  *ret.tag() = "level-0";
  ret.cdr() = std::move(s);
  return ret;
}

size_t getRecDepth(const TestStructRecursive& s) {
  auto p = &s;
  size_t depth = 0;
  while ((p = p->cdr().get())) {
    ++depth;
  }
  return depth;
}

TEST(SerializationTest, RecursiveNoDepthCompactSerializerRoundtripPasses) {
  auto s = makeTestStructRecursive(0);

  folly::IOBufQueue q;
  CompactSerializer::serialize(s, &q);

  TestStructRecursive out;
  CompactSerializer::deserialize(q.front(), out);

  EXPECT_EQ(s, out);
}

TEST(SerializationTest, RecursiveDeepCompactSerializerRoundtripPasses) {
  auto s = makeTestStructRecursive(6);
  EXPECT_EQ(6, getRecDepth(s));

  folly::IOBufQueue q;
  CompactSerializer::serialize(s, &q);

  TestStructRecursive out;
  CompactSerializer::deserialize(q.front(), out);

  EXPECT_EQ(s, out);
}

TEST(SerializationTest, RecursiveNoDepthBinarySerializerRoundtripPasses) {
  auto s = makeTestStructRecursive(0);

  folly::IOBufQueue q;
  BinarySerializer::serialize(s, &q);

  TestStructRecursive out;
  BinarySerializer::deserialize(q.front(), out);

  EXPECT_EQ(s, out);
}

TEST(SerializationTest, RecursiveDeepBinarySerializerRoundtripPasses) {
  auto s = makeTestStructRecursive(6);
  EXPECT_EQ(6, getRecDepth(s));

  folly::IOBufQueue q;
  BinarySerializer::serialize(s, &q);

  TestStructRecursive out;
  BinarySerializer::deserialize(q.front(), out);

  EXPECT_EQ(s, out);
}

TEST(SerializationTest, RecursiveNoDepthSimpleJSONSerializerRoundtripPasses) {
  auto s = makeTestStructRecursive(0);

  folly::IOBufQueue q;
  SimpleJSONSerializer::serialize(s, &q);

  TestStructRecursive out;
  SimpleJSONSerializer::deserialize(q.front(), out);

  EXPECT_EQ(s, out);
}

TEST(SerializationTest, RecursiveDeepSimpleJSONSerializerRoundtripPasses) {
  auto s = makeTestStructRecursive(6);
  EXPECT_EQ(6, getRecDepth(s));

  folly::IOBufQueue q;
  SimpleJSONSerializer::serialize(s, &q);

  TestStructRecursive out;
  SimpleJSONSerializer::deserialize(q.front(), out);

  EXPECT_EQ(s, out);
}

TEST(SerializationTest, StringOverloads) {
  auto s = makeTestStruct();

  std::string str;
  CompactSerializer::serialize(s, &str);

  {
    TestStruct out;
    CompactSerializer::deserialize(str, out);
    EXPECT_EQ(out, s);
  }
}

namespace {
// Large enough to ensure externally allocated buffer and to make sure
// IOBufQueue::insert doesn't copy instead of linking
constexpr size_t kBufSize = 8192;

template <class Serializer>
void testIOBufSharingUnmanagedBuffer() {
  char tmp[kBufSize];
  memcpy(tmp, "hello", 5);
  TestStructIOBuf s;
  *s.buf() = folly::IOBuf(folly::IOBuf::WRAP_BUFFER, tmp, sizeof(tmp));

  for (unsigned int i = 0; i < 4; ++i) {
    ExternalBufferSharing serializationSharing =
        i & 1 ? SHARE_EXTERNAL_BUFFER : COPY_EXTERNAL_BUFFER;
    ExternalBufferSharing deserializationSharing =
        i & 2 ? SHARE_EXTERNAL_BUFFER : COPY_EXTERNAL_BUFFER;

    folly::IOBufQueue q;
    Serializer::serialize(s, &q, serializationSharing);

    TestStructIOBuf s2;
    Serializer::deserialize(q.front(), s2, deserializationSharing);

    size_t size = 0;
    for (auto& br : *s2.buf()) {
      if (br.empty()) {
        continue;
      }
      if (i == 3) {
        // Expect only one non-empty buffer, which must be ours
        EXPECT_EQ(size, 0);
        EXPECT_EQ(s.buf()->data(), br.data());
        EXPECT_EQ(s.buf()->length(), br.size());
      } else {
        EXPECT_NE(s.buf()->data(), br.data());
      }
      size += br.size();
    }
    EXPECT_EQ(s.buf()->length(), size);
    s2.buf()->coalesce();
    EXPECT_EQ(0, memcmp(s2.buf()->data(), s.buf()->data(), s.buf()->length()));
  }
}

template <class Serializer>
void testIOBufSharingManagedBuffer() {
  TestStructIOBuf s;
  *s.buf() = folly::IOBuf(folly::IOBuf::CREATE, kBufSize);
  memcpy(s.buf()->writableTail(), "hello", 5);
  s.buf()->append(kBufSize);

  for (unsigned int i = 0; i < 4; ++i) {
    ExternalBufferSharing serializationSharing =
        i & 1 ? SHARE_EXTERNAL_BUFFER : COPY_EXTERNAL_BUFFER;
    ExternalBufferSharing deserializationSharing =
        i & 2 ? SHARE_EXTERNAL_BUFFER : COPY_EXTERNAL_BUFFER;

    folly::IOBufQueue q;
    Serializer::serialize(s, &q, serializationSharing);

    TestStructIOBuf s2;
    Serializer::deserialize(q.front(), s2, deserializationSharing);

    size_t size = 0;
    for (auto& br : *s2.buf()) {
      if (br.empty()) {
        continue;
      }
      // Expect only one non-empty buffer, which must be ours
      EXPECT_EQ(size, 0);
      EXPECT_EQ(s.buf()->data(), br.data());
      EXPECT_EQ(s.buf()->length(), br.size());
      size += br.size();
    }
  }
}

} // namespace

TEST(SerializationTest, CompactSerializerIOBufSharingUnmanagedBuffer) {
  testIOBufSharingUnmanagedBuffer<CompactSerializer>();
}

TEST(SerializationTest, BinarySerializerIOBufSharingUnmanagedBuffer) {
  testIOBufSharingUnmanagedBuffer<BinarySerializer>();
}

TEST(SerializationTest, CompactSerializerIOBufSharingManagedBuffer) {
  testIOBufSharingManagedBuffer<CompactSerializer>();
}

TEST(SerializationTest, BinarySerializerIOBufSharingManagedBuffer) {
  testIOBufSharingManagedBuffer<BinarySerializer>();
}

TEST(SerializationTest, UnsignedIntStruct) {
  TestUnsignedIntStruct s;

  static_assert(
      std::is_same<decltype(s.u8())::value_type, uint8_t>::value,
      "Unexpected type for s.u8");
  static_assert(
      std::is_same<decltype(s.u16())::value_type, uint16_t>::value,
      "Unexpected type for s.u16");
  static_assert(
      std::is_same<decltype(s.u32())::value_type, uint32_t>::value,
      "Unexpected type for s.u32");
  static_assert(
      std::is_same<decltype(s.u64())::value_type, uint64_t>::value,
      "Unexpected type for s.u64");

  *s.u8() = 128U;
  *s.u16() = 32768U;
  *s.u32() = 2147483648UL;
  *s.u64() = 9223372036854775808ULL;

  folly::IOBufQueue q;
  CompactSerializer::serialize(s, &q);

  TestUnsignedIntStruct out;
  CompactSerializer::deserialize(q.front(), out);

  EXPECT_EQ(out, s);
}

TEST(SerializationTest, UnsignedIntUnion) {
  TestUnsignedIntUnion u;

  static_assert(
      std::is_same<decltype(u.get_u8()), const uint8_t&>::value,
      "Unexpected return value type for u.get_u8()");
  static_assert(
      std::is_same<decltype(u.get_u16()), const uint16_t&>::value,
      "Unexpected return value type for u.get_u16()");
  static_assert(
      std::is_same<decltype(u.get_u32()), const uint32_t&>::value,
      "Unexpected return value type for u.get_u32()");
  static_assert(
      std::is_same<decltype(u.get_u64()), const uint64_t&>::value,
      "Unexpected return value type for s.get_u64()");

  u.u64() = 9223372036854775808ULL;

  folly::IOBufQueue q;
  CompactSerializer::serialize(u, &q);

  TestUnsignedIntUnion out;
  CompactSerializer::deserialize(q.front(), out);

  EXPECT_EQ(out, u);
}

TEST(SerializationTest, UnsignedInt32ListStruct) {
  TestUnsignedInt32ListStruct s;

  static_assert(
      std::is_same<decltype(s.l())::value_type, std::vector<uint32_t>>::value,
      "Unexpected type for s.l");

  s.l()->push_back(1073741824UL);
  s.l()->push_back(2147483648UL);
  s.l()->push_back(3221225472UL);
  s.l()->push_back(4294967295UL);

  folly::IOBufQueue q;
  CompactSerializer::serialize(s, &q);

  TestUnsignedInt32ListStruct out;
  CompactSerializer::deserialize(q.front(), out);

  EXPECT_EQ(out, s);
}

TEST(SerializationTest, UnsignedIntMap) {
  TestUnsignedIntMapStruct s;

  static_assert(
      std::is_same<decltype(s.m())::value_type, std::map<uint32_t, uint64_t>>::
          value,
      "Unexpected type for s.m");

  s.m()[1073741824UL] = 4611686018427387904ULL;
  s.m()[2147483648UL] = 9223372036854775808ULL;
  s.m()[3221225472UL] = 13835058055282163712ULL;
  s.m()[4294967295UL] = 18446744073709551615ULL;

  folly::IOBufQueue q;
  CompactSerializer::serialize(s, &q);

  TestUnsignedIntMapStruct out;
  CompactSerializer::deserialize(q.front(), out);

  EXPECT_EQ(out, s);
}

template <class Serializer, class ProtocolWriter>
void testSerializedSizeZC() {
  for (size_t testSize : {10, 5000}) {
    TestStructIOBuf s;
    *s.buf() = folly::IOBuf(IOBuf::CREATE, testSize);
    *s.i() = 0x7fffffff;
    memset(s.buf()->writableTail(), 'a', testSize);
    s.buf()->append(testSize);
    folly::IOBufQueue q;
    Serializer::serialize(s, &q);
    auto iob = q.move();
    size_t realSize = iob->computeChainDataLength();
    size_t realSizeNotIncludingTestSizedIOB = 0;
    for (auto& p : *iob) {
      if (p.size() != testSize) {
        realSizeNotIncludingTestSizedIOB += p.size();
      }
    }
    ProtocolWriter w;
    EXPECT_GE(s.serializedSize(&w), realSize);
    EXPECT_GE(s.serializedSizeZC(&w), realSizeNotIncludingTestSizedIOB);
    if (testSize <= 4 << 10) { // Less than MAX_PACK_COPY
      EXPECT_GE(s.serializedSizeZC(&w), realSize);
    }
  }
}

TEST(SerializationTest, BinarySerializedSizeZC) {
  testSerializedSizeZC<BinarySerializer, BinaryProtocolWriter>();
}

TEST(SerializationTest, CompactSerializedSizeZC) {
  testSerializedSizeZC<CompactSerializer, CompactProtocolWriter>();
}
