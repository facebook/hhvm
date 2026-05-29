/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <sys/uio.h>

#include <cstring>
#include <string>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include "mcrouter/lib/IOBufUtil.h"
#include "mcrouter/lib/carbon/CarbonProtocolWriter.h"
#include "mcrouter/lib/carbon/test/Util.h"
#include "mcrouter/lib/carbon/test/gen/CarbonTest.h"
#include "mcrouter/lib/carbon/test/gen/CarbonThriftTest.h"
#include "mcrouter/lib/network/MessageHelpers.h"

using namespace carbon::test::util;

using carbon::test::DummyThriftReply;
using carbon::test::DummyThriftRequest;
using carbon::test::TestReply;
using carbon::test::TestRequest;
using carbon::test::TestRequestStringKey;
using carbon::test::ThriftTestRequest;
using facebook::memcache::coalesceAndGetRange;

namespace {

constexpr auto kKeyLiteral =
    "/region/cluster/abcdefghijklmnopqrstuvwxyz|#|afterhashstop";

template <class Key>
void checkKeyEmpty(const Key& key) {
  const auto emptyRoutingKeyHash = TestRequest().key()->routingKeyHash();

  EXPECT_TRUE(key.empty());
  EXPECT_EQ(0, key.size());
  EXPECT_EQ("", key.fullKey());
  EXPECT_EQ("", key.routingKey());
  EXPECT_EQ("", key.routingPrefix());
  EXPECT_EQ("", key.keyWithoutRoute());
  EXPECT_FALSE(key.hasHashStop());
  EXPECT_EQ(emptyRoutingKeyHash, key.routingKeyHash());
}

template <class Key>
void checkKeyFilledProperly(const Key& key) {
  EXPECT_FALSE(key.empty());
  EXPECT_EQ(std::strlen(kKeyLiteral), key.size());
  EXPECT_EQ(kKeyLiteral, key.fullKey());
  EXPECT_EQ(
      "abcdefghijklmnopqrstuvwxyz|#|afterhashstop",
      key.keyWithoutRoute().str());
  EXPECT_EQ("/region/cluster/", key.routingPrefix().str());
  EXPECT_EQ("abcdefghijklmnopqrstuvwxyz", key.routingKey().str());
  EXPECT_EQ("|#|afterhashstop", key.afterRoutingKey().str());
  EXPECT_NE(0, key.routingKeyHash());
  EXPECT_TRUE(key.hasHashStop());
}

template <class T>
void checkStructWithEnum(
    const T& a,
    const T& b,
    bool equal,
    const std::string& msg) {
  if (equal) {
    EXPECT_EQ(*a.testEnum_ref(), *b.testEnum_ref()) << msg;
  } else {
    EXPECT_NE(*a.testEnum_ref(), *b.testEnum_ref()) << msg;
  }
}

template <class T>
void checkStructWithOptionalEnum(
    const T& a,
    const T& b,
    bool equal,
    const std::string& msg) {
  if (equal) {
    EXPECT_EQ(*a.testEnum_ref(), *b.testEnum_ref()) << msg;
  } else {
    EXPECT_NE(*a.testEnum_ref(), *b.testEnum_ref()) << msg;
  }
  EXPECT_FALSE(
      a.testEmptyEnum_ref().has_value() | b.testEmptyEnum_ref().has_value())
      << msg;
}

template <class T1, class T2>
void testEnumCompatibility(bool expectCompatible) {
  T1 struct1;
  struct1.testEnum() = decltype(struct1.testEnum_ref())::value_type::BBB;

  T2 struct2;
  struct2.testEnum() = decltype(struct2.testEnum_ref())::value_type::BBB;

  checkStructWithEnum(
      struct1,
      serializeAndDeserialize<T2, T1>(struct2),
      expectCompatible,
      fmt::format("{} to {}", typeid(T2).name(), typeid(T1).name()));
  checkStructWithEnum(
      struct2,
      serializeAndDeserialize<T1, T2>(struct1),
      expectCompatible,
      fmt::format("{} to {}", typeid(T1).name(), typeid(T2).name()));
}

template <class T1, class T2>
void testOptionalEnumCompatibility(bool expectCompatible) {
  T1 struct1;
  struct1.testEnum() = decltype(struct1.testEnum_ref())::value_type::BBB;

  T2 struct2;
  struct2.testEnum() = decltype(struct2.testEnum_ref())::value_type::BBB;

  checkStructWithOptionalEnum(
      struct1,
      serializeAndDeserialize<T2, T1>(struct2),
      expectCompatible,
      fmt::format("{} to {}", typeid(T2).name(), typeid(T1).name()));
  checkStructWithOptionalEnum(
      struct2,
      serializeAndDeserialize<T1, T2>(struct1),
      expectCompatible,
      fmt::format("{} to {}", typeid(T1).name(), typeid(T2).name()));
}

template <class Type>
class TestUnionBuilder {
 public:
  explicit TestUnionBuilder(const folly::StringPiece name, Type val)
      : fieldName_(name), fieldVal_(val) {}

  template <class T, class F>
  bool visitUnionMember(folly::StringPiece fieldName, F&& emplaceFn) {
    if (fieldName == fieldName_) {
      auto& itemRef = emplaceFn();
      itemRef = fieldVal_;
    }
    return true;
  }

 private:
  folly::StringPiece fieldName_;
  Type fieldVal_;
};
} // namespace

TEST(CarbonBasic, staticAsserts) {
  static_assert(!facebook::memcache::HasExptimeTrait<TestRequest>::value, "");
  static_assert(!facebook::memcache::HasFlagsTrait<TestRequest>::value, "");
  static_assert(facebook::memcache::HasKeyTrait<TestRequest>::value, "");
  static_assert(!facebook::memcache::HasValueTrait<TestRequest>::value, "");
  static_assert(TestRequest::typeId == 69, "");

  static_assert(!facebook::memcache::HasExptimeTrait<TestReply>::value, "");
  static_assert(!facebook::memcache::HasFlagsTrait<TestReply>::value, "");
  static_assert(!facebook::memcache::HasKeyTrait<TestReply>::value, "");
  static_assert(!facebook::memcache::HasValueTrait<TestReply>::value, "");
  static_assert(TestReply::typeId == 70, "");

  static_assert(carbon::IsCarbonStruct<TestRequest>::value, "");
  static_assert(!carbon::IsCarbonStruct<int>::value, "");
}

TEST(CarbonBasic, defaultConstructed) {
  TestRequest req;
  TestRequestStringKey req2;

  // key
  checkKeyEmpty(*req.key());
  checkKeyEmpty(*req2.key());

  // bool
  EXPECT_FALSE(*req.testBool());
  // char
  EXPECT_EQ('\0', *req.testChar());
  // int8_t
  EXPECT_EQ(0, *req.testInt8());
  // int16_t
  EXPECT_EQ(0, *req.testInt16());
  // int32_t
  EXPECT_EQ(0, *req.testInt32());
  // int64_t
  EXPECT_EQ(0, *req.testInt64());
  // uint8_t
  EXPECT_EQ(0, *req.testUInt8());
  // uint16_t
  EXPECT_EQ(0, *req.testUInt16());
  // uint32_t
  EXPECT_EQ(0, *req.testUInt32());
  // uint64_t
  EXPECT_EQ(0, *req.testUInt64());

  // float
  EXPECT_EQ(0.0, *req.testFloat());
  // double
  EXPECT_EQ(0.0, *req.testDouble());

  // string
  EXPECT_TRUE(req.testShortString()->empty());
  EXPECT_TRUE(req.testLongString()->empty());
  // IOBuf
  EXPECT_TRUE(req.testIobuf()->empty());

  // List of strings
  EXPECT_TRUE(req.testList()->empty());

  // Vector of vectors
  EXPECT_TRUE(req.testNestedVec()->empty());

  // folly::Optional fields
  EXPECT_FALSE(req.testOptionalIobuf().has_value());

  // optional fields
  EXPECT_FALSE(req.testOptionalKeywordString().has_value());
  EXPECT_FALSE(req.testOptionalKeywordIobuf().has_value());
  EXPECT_FALSE(req.testOptionalKeywordBool().has_value());

  // optional fields field_ref
  EXPECT_FALSE(req.testOptionalKeywordString().has_value());
  EXPECT_FALSE(req.testOptionalKeywordIobuf().has_value());
  EXPECT_FALSE(req.testOptionalKeywordBool().has_value());

  // Unordered map
  EXPECT_TRUE(req.testUMap()->empty());

  // Ordered map
  EXPECT_TRUE(req.testMap()->empty());

  // Complex map
  EXPECT_TRUE(req.testComplexMap()->empty());

  // Unordered set
  EXPECT_TRUE(req.testUSet()->empty());

  // Ordered set
  EXPECT_TRUE(req.testSet()->empty());

  // fields generated for every request (will likely be removed in the future)
  EXPECT_EQ(0, facebook::memcache::getExptimeIfExist(req));
  EXPECT_EQ(0, facebook::memcache::getFlagsIfExist(req));
}

TEST(CarbonBasic, setAndGet) {
  TestRequest req(kKeyLiteral);
  TestRequestStringKey req2(kKeyLiteral);

  // key
  const auto reqKeyPiece = req.key()->fullKey();
  EXPECT_EQ(kKeyLiteral, reqKeyPiece);
  EXPECT_EQ(kKeyLiteral, req.key()->fullKey());
  EXPECT_EQ("abcdefghijklmnopqrstuvwxyz", req.key()->routingKey().str());
  EXPECT_EQ("/region/cluster/", req.key()->routingPrefix().str());
  EXPECT_EQ(
      "abcdefghijklmnopqrstuvwxyz|#|afterhashstop",
      req.key()->keyWithoutRoute().str());
  EXPECT_EQ("|#|afterhashstop", req.key()->afterRoutingKey().str());

  const auto reqKeyPiece2 = req2.key()->fullKey();
  EXPECT_EQ(kKeyLiteral, reqKeyPiece2);
  EXPECT_EQ(kKeyLiteral, req2.key()->fullKey());
  EXPECT_EQ("abcdefghijklmnopqrstuvwxyz", req2.key()->routingKey().str());
  EXPECT_EQ("/region/cluster/", req2.key()->routingPrefix().str());
  EXPECT_EQ(
      "abcdefghijklmnopqrstuvwxyz|#|afterhashstop",
      req2.key()->keyWithoutRoute().str());
  EXPECT_EQ("|#|afterhashstop", req2.key()->afterRoutingKey().str());

  // bool
  req.testBool() = true;
  EXPECT_TRUE(*req.testBool());
  // char
  req.testChar() = 'A';
  EXPECT_EQ('A', *req.testChar());

  // int8_t
  req.testInt8() = kMinInt8;
  EXPECT_EQ(kMinInt8, *req.testInt8());
  // int16_t
  req.testInt16() = kMinInt16;
  EXPECT_EQ(kMinInt16, *req.testInt16());
  // int32_t
  req.testInt32() = kMinInt32;
  EXPECT_EQ(kMinInt32, *req.testInt32());
  // int64_t
  req.testInt64() = kMinInt64;
  EXPECT_EQ(kMinInt64, *req.testInt64());
  // uint8_t
  req.testUInt8() = kMaxUInt8;
  EXPECT_EQ(kMaxUInt8, *req.testUInt8());
  // uint16_t
  req.testUInt16() = kMaxUInt16;
  EXPECT_EQ(kMaxUInt16, *req.testUInt16());
  // uint32_t
  req.testUInt32() = kMaxUInt32;
  EXPECT_EQ(kMaxUInt32, *req.testUInt32());
  // uint64_t
  req.testUInt64() = kMaxUInt64;
  EXPECT_EQ(kMaxUInt64, *req.testUInt64());

  // float
  req.testFloat() = 12345.789f;
  EXPECT_FLOAT_EQ(12345.789f, *req.testFloat());
  // double
  req.testDouble() = 12345.789;
  EXPECT_DOUBLE_EQ(12345.789, *req.testDouble());

  // string
  req.testShortString() = kShortString.str();
  EXPECT_EQ(kShortString, *req.testShortString());
  req.testLongString() = longString();
  EXPECT_EQ(longString(), *req.testLongString());
  // IOBuf
  folly::IOBuf iobuf(folly::IOBuf::COPY_BUFFER, longString());
  req.testIobuf() = iobuf;
  EXPECT_EQ(
      coalesceAndGetRange(iobuf).str(),
      coalesceAndGetRange(req.testIobuf()).str());

  std::vector<std::string> strings = {
      "abcdefg", "xyz", kShortString.str(), longString()};
  req.testList() = strings;
  EXPECT_EQ(strings, *req.testList());

  // Vector of vectors
  std::vector<std::vector<uint64_t>> vectors = {{1, 1, 1}, {2, 2}};
  req.testNestedVec() = vectors;
  EXPECT_EQ(vectors, *req.testNestedVec());

  // folly::Optional fields
  const auto s = longString();
  req.testOptionalIobuf() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, s);
  EXPECT_EQ(s, coalesceAndGetRange(*req.testOptionalIobuf()));
  req.testOptionalBool() = false;
  EXPECT_EQ(false, *req.testOptionalBool());
  std::vector<folly::Optional<std::string>> ovec;
  ovec.emplace_back("hello");

  // optional fields
  const auto lstring = longString();
  req.testOptionalKeywordString() = lstring;
  EXPECT_EQ(lstring, *req.testOptionalKeywordString());
  req.testOptionalKeywordIobuf() =
      folly::IOBuf(folly::IOBuf::COPY_BUFFER, lstring);
  EXPECT_EQ(lstring, coalesceAndGetRange(*req.testOptionalKeywordIobuf()));
  req.testOptionalKeywordBool() = false;
  EXPECT_EQ(false, *req.testOptionalKeywordBool());

  // optional fields ref api
  const auto lstringRef = longString();
  req.testOptionalKeywordString() = lstringRef;
  EXPECT_EQ(lstringRef, *req.testOptionalKeywordString());
  req.testOptionalKeywordIobuf() =
      folly::IOBuf(folly::IOBuf::COPY_BUFFER, lstringRef);
  EXPECT_EQ(lstringRef, coalesceAndGetRange(req.testOptionalKeywordIobuf()));
  req.testOptionalKeywordBool() = false;
  EXPECT_EQ(false, *req.testOptionalKeywordBool());

  // Unordered map
  std::unordered_map<std::string, std::string> stringmap;
  stringmap.insert({"key", "value"});
  req.testUMap() = stringmap;
  EXPECT_EQ(stringmap, *req.testUMap());

  // Ordered map
  std::map<double, double> doublemap;
  doublemap.insert({1.08, 8.3});
  req.testMap() = doublemap;
  EXPECT_EQ(doublemap, *req.testMap());

  // Complex map
  std::map<std::string, std::vector<uint16_t>> complexmap;
  complexmap.insert({"key", {1, 2}});
  req.testComplexMap() = complexmap;
  EXPECT_EQ(complexmap, *req.testComplexMap());

  // Unordered set
  std::unordered_set<std::string> stringset;
  stringset.insert("hello");
  stringset.insert("world");
  req.testUSet() = stringset;
  EXPECT_EQ(stringset, *req.testUSet());

  // Ordered set
  std::set<uint64_t> intset;
  intset.insert(1);
  intset.insert(2);
  req.testSet() = intset;
  EXPECT_EQ(intset, *req.testSet());
}

TEST(CarbonTest, serializeDeserialize) {
  // Fill in a request
  TestRequest outRequest("abcdefghijklmnopqrstuvwxyz");
  outRequest.testBool() = true;
  outRequest.testChar() = 'A';
  outRequest.testInt8() = kMinInt8;
  outRequest.testInt16() = kMinInt16;
  outRequest.testInt32() = kMinInt32;
  outRequest.testInt64() = kMinInt64;
  outRequest.testUInt8() = kMaxUInt8;
  outRequest.testUInt16() = kMaxUInt16;
  outRequest.testUInt32() = kMaxUInt32;
  outRequest.testUInt64() = kMaxUInt64;
  outRequest.testFloat() = 12345.678f;
  outRequest.testDouble() = 12345.678;
  outRequest.testShortString() = kShortString.str();
  outRequest.testLongString() = longString();
  outRequest.testIobuf() =
      folly::IOBuf(folly::IOBuf::COPY_BUFFER, kShortString);
  // List of strings
  outRequest.testList() = {"abcdefg", "xyz", kShortString.str(), longString()};
  // Other optional field gets a value of zero length
  outRequest.testOptionalIobuf() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "");

  outRequest.testNestedVec()->push_back({100, 2000});

  outRequest.testUMap()->insert({"hello", "world"});
  outRequest.testMap()->insert({1.08, 8.3});
  outRequest.testF14FastMap()->insert({"hello", "F14FastMap"});
  outRequest.testF14NodeMap()->insert({"hello", "F14NodeMap"});
  outRequest.testF14ValueMap()->insert({"hello", "F14ValueMap"});
  outRequest.testF14VectorMap()->insert({"hello", "F14VectorMap"});
  outRequest.testComplexMap()->insert({"key", {1, 2}});

  outRequest.testUSet()->insert("hello");
  outRequest.testSet()->insert(123);
  outRequest.testF14FastSet()->insert("hello F14FastSet");
  outRequest.testF14NodeSet()->insert("hello F14NodeSet");
  outRequest.testF14ValueSet()->insert("hello F14ValueSet");
  outRequest.testF14VectorSet()->insert("hello F14VectorSet");
  outRequest.testOptionalBool() = false;
  outRequest.testIOBufList()->emplace_back();
  const auto inRequest = serializeAndDeserialize(outRequest);
  expectEqTestRequest(outRequest, inRequest);
}

TEST(CarbonTest, veryLongString) {
  constexpr uint32_t kVeryLongStringSize = 1 << 30;
  std::string veryLongString(kVeryLongStringSize, 'x');

  TestRequest outRequest(longString());
  outRequest.testLongString() = std::move(veryLongString);
  const auto inRequest = serializeAndDeserialize(outRequest);
  expectEqTestRequest(outRequest, inRequest);
  EXPECT_EQ(kVeryLongStringSize, inRequest.testLongString()->length());
}

TEST(CarbonTest, repeatStorageUsage) {
  std::string testStr(longString());

  carbon::CarbonQueueAppenderStorage storage;
  carbon::CarbonProtocolWriter writer(storage);

  TestRequest outRequest(longString());
  outRequest.testLongString() = std::move(testStr);

  for (int i = 0; i < 100; i++) {
    serialize(outRequest, writer);
    storage.reset();
  }
}

TEST(CarbonTest, veryLongIobuf) {
  constexpr uint32_t kVeryLongIobufSize = 1 << 30;
  folly::IOBuf veryLongIobuf(folly::IOBuf::CREATE, kVeryLongIobufSize);
  std::memset(veryLongIobuf.writableTail(), 'x', kVeryLongIobufSize);
  veryLongIobuf.append(kVeryLongIobufSize);

  TestRequest outRequest(longString());
  outRequest.testIobuf() = std::move(veryLongIobuf);
  const auto inRequest = serializeAndDeserialize(outRequest);
  expectEqTestRequest(outRequest, inRequest);
  EXPECT_EQ(kVeryLongIobufSize, inRequest.testIobuf()->length());
}

TEST(CarbonTest, keysIobuf) {
  {
    TestRequest req;
    checkKeyEmpty(*req.key());
  }
  {
    TestRequest req;

    const folly::IOBuf keyCopy(folly::IOBuf::CopyBufferOp(), kKeyLiteral);
    req.key() = keyCopy;
    checkKeyFilledProperly(*req.key());

    req.key() = "";
    checkKeyEmpty(*req.key());
  }
  {
    TestRequest req;
    checkKeyEmpty(*req.key());

    req.key() = folly::IOBuf(folly::IOBuf::CopyBufferOp(), kKeyLiteral);
    checkKeyFilledProperly(*req.key());
  }
  {
    TestRequest req(kKeyLiteral);
    checkKeyFilledProperly(*req.key());
  }
  {
    TestRequest req{folly::StringPiece(kKeyLiteral)};
    checkKeyFilledProperly(*req.key());
  }
  {
    TestRequest req(folly::IOBuf(folly::IOBuf::CopyBufferOp(), kKeyLiteral));
    checkKeyFilledProperly(*req.key());
  }
}

TEST(CarbonTest, keysString) {
  {
    TestRequestStringKey req;
    checkKeyEmpty(*req.key());
  }
  {
    TestRequestStringKey req;

    const std::string keyCopy(kKeyLiteral);
    req.key() = keyCopy;
    checkKeyFilledProperly(*req.key());

    req.key() = "";
    checkKeyEmpty(*req.key());
  }
  {
    TestRequestStringKey req;
    checkKeyEmpty(*req.key());

    req.key() = kKeyLiteral;
    checkKeyFilledProperly(*req.key());
  }
  {
    TestRequestStringKey req(kKeyLiteral);
    checkKeyFilledProperly(*req.key());
  }
  {
    TestRequest req{folly::StringPiece(kKeyLiteral)};
    checkKeyFilledProperly(*req.key());
  }
  {
    TestRequest req{std::string(kKeyLiteral)};
    checkKeyFilledProperly(*req.key());
  }
}

TEST(CarbonBasic, defaultConstructedFieldRefAPI) {
  TestRequest req;
  TestRequestStringKey req2;

  // key
  checkKeyEmpty(*req.key());
  checkKeyEmpty(*req2.key());

  // bool
  EXPECT_FALSE(*req.testBool());
  // char
  EXPECT_EQ('\0', *req.testChar());

  // int8_t
  EXPECT_EQ(0, *req.testInt8());
  // int16_t
  EXPECT_EQ(0, *req.testInt16());
  // int32_t
  EXPECT_EQ(0, *req.testInt32());
  // int64_t
  EXPECT_EQ(0, *req.testInt64());
  // uint8_t
  EXPECT_EQ(0, *req.testUInt8());
  // uint16_t
  EXPECT_EQ(0, *req.testUInt16());
  // uint32_t
  EXPECT_EQ(0, *req.testUInt32());
  // uint64_t
  EXPECT_EQ(0, *req.testUInt64());

  // float
  EXPECT_EQ(0.0, *req.testFloat());
  // double
  EXPECT_EQ(0.0, *req.testDouble());

  // string
  EXPECT_TRUE(req.testShortString()->empty());
  EXPECT_TRUE(req.testLongString()->empty());
  // IOBuf
  EXPECT_TRUE(req.testIobuf()->empty());

  // List of strings
  EXPECT_TRUE(req.testList()->empty());

  // Vector of vectors
  EXPECT_TRUE(req.testNestedVec()->empty());

  // Unordered map
  EXPECT_TRUE(req.testUMap()->empty());

  // Ordered map
  EXPECT_TRUE(req.testMap()->empty());

  // Complex map
  EXPECT_TRUE(req.testComplexMap()->empty());

  // Unordered set
  EXPECT_TRUE(req.testUSet()->empty());

  // Ordered set
  EXPECT_TRUE(req.testSet()->empty());

  // fields generated for every request (will likely be removed in the future)
  EXPECT_EQ(0, facebook::memcache::getExptimeIfExist(req));
  EXPECT_EQ(0, facebook::memcache::getFlagsIfExist(req));
}

TEST(CarbonBasic, setAndGetFieldRefAPI) {
  TestRequest req(kKeyLiteral);
  TestRequestStringKey req2(kKeyLiteral);

  // key
  const auto reqKeyPiece = req.key()->fullKey();
  EXPECT_EQ(kKeyLiteral, reqKeyPiece);
  EXPECT_EQ(kKeyLiteral, req.key()->fullKey());
  EXPECT_EQ("abcdefghijklmnopqrstuvwxyz", req.key()->routingKey().str());
  EXPECT_EQ("/region/cluster/", req.key()->routingPrefix().str());
  EXPECT_EQ(
      "abcdefghijklmnopqrstuvwxyz|#|afterhashstop",
      req.key()->keyWithoutRoute().str());
  EXPECT_EQ("|#|afterhashstop", req.key()->afterRoutingKey().str());

  const auto reqKeyPiece2 = req2.key()->fullKey();
  EXPECT_EQ(kKeyLiteral, reqKeyPiece2);
  EXPECT_EQ(kKeyLiteral, req2.key()->fullKey());
  EXPECT_EQ("abcdefghijklmnopqrstuvwxyz", req2.key()->routingKey().str());
  EXPECT_EQ("/region/cluster/", req2.key()->routingPrefix().str());
  EXPECT_EQ(
      "abcdefghijklmnopqrstuvwxyz|#|afterhashstop",
      req2.key()->keyWithoutRoute().str());
  EXPECT_EQ("|#|afterhashstop", req2.key()->afterRoutingKey().str());

  // bool
  req.testBool() = true;
  EXPECT_TRUE(*req.testBool());
  // char
  req.testChar() = 'A';
  EXPECT_EQ('A', *req.testChar());

  // int8_t
  req.testInt8() = kMinInt8;
  EXPECT_EQ(kMinInt8, *req.testInt8());
  // int16_t
  req.testInt16() = kMinInt16;
  EXPECT_EQ(kMinInt16, *req.testInt16());
  // int32_t
  req.testInt32() = kMinInt32;
  EXPECT_EQ(kMinInt32, *req.testInt32());
  // int64_t
  req.testInt64() = kMinInt64;
  EXPECT_EQ(kMinInt64, *req.testInt64());
  // uint8_t
  req.testUInt8() = kMaxUInt8;
  EXPECT_EQ(kMaxUInt8, *req.testUInt8());
  // uint16_t
  req.testUInt16() = kMaxUInt16;
  EXPECT_EQ(kMaxUInt16, *req.testUInt16());
  // uint32_t
  req.testUInt32() = kMaxUInt32;
  EXPECT_EQ(kMaxUInt32, *req.testUInt32());
  // uint64_t
  req.testUInt64() = kMaxUInt64;
  EXPECT_EQ(kMaxUInt64, *req.testUInt64());

  // float
  req.testFloat() = 12345.789f;
  EXPECT_FLOAT_EQ(12345.789f, *req.testFloat());
  // double
  req.testDouble() = 12345.789;
  EXPECT_DOUBLE_EQ(12345.789, *req.testDouble());

  // string
  req.testShortString() = kShortString.str();
  EXPECT_EQ(kShortString, *req.testShortString());
  req.testLongString() = longString();
  EXPECT_EQ(longString(), *req.testLongString());
  // IOBuf
  folly::IOBuf iobuf(folly::IOBuf::COPY_BUFFER, longString());
  req.testIobuf() = iobuf;
  EXPECT_EQ(
      coalesceAndGetRange(iobuf).str(),
      coalesceAndGetRange(req.testIobuf()).str());

  std::vector<std::string> strings = {
      "abcdefg", "xyz", kShortString.str(), longString()};
  req.testList() = strings;
  EXPECT_EQ(strings, *req.testList());

  // optionals
  EXPECT_FALSE(req.testOptionalKeywordString().has_value());
  EXPECT_FALSE(req.testOptionalKeywordIobuf().has_value());
  EXPECT_FALSE(req.testOptionalKeywordBool().has_value());

  // optionals field_ref
  EXPECT_FALSE(req.testOptionalKeywordString().has_value());
  EXPECT_FALSE(req.testOptionalKeywordIobuf().has_value());
  EXPECT_FALSE(req.testOptionalKeywordBool().has_value());

  // Vector of vectors
  std::vector<std::vector<uint64_t>> vectors = {{1, 1, 1}, {2, 2}};
  req.testNestedVec() = vectors;
  EXPECT_EQ(vectors, *req.testNestedVec());

  // Unordered map
  std::unordered_map<std::string, std::string> stringmap;
  stringmap.insert({"key", "value"});
  req.testUMap() = stringmap;
  EXPECT_EQ(stringmap, *req.testUMap());

  // Ordered map
  std::map<double, double> doublemap;
  doublemap.insert({1.08, 8.3});
  req.testMap() = doublemap;
  EXPECT_EQ(doublemap, *req.testMap());

  // Complex map
  std::map<std::string, std::vector<uint16_t>> complexmap;
  complexmap.insert({"key", {1, 2}});
  req.testComplexMap() = complexmap;
  EXPECT_EQ(complexmap, *req.testComplexMap());

  // Unordered set
  std::unordered_set<std::string> stringset;
  stringset.insert("hello");
  stringset.insert("world");
  req.testUSet() = stringset;
  EXPECT_EQ(stringset, *req.testUSet());

  // Ordered set
  std::set<uint64_t> intset;
  intset.insert(1);
  intset.insert(2);
  req.testSet() = intset;
  EXPECT_EQ(intset, *req.testSet());
}

TEST(CarbonBasic, setAndGetFieldRefAPIThrift) {
  DummyThriftRequest req(kKeyLiteral);

  // key
  const auto reqKeyPiece = req.key()->fullKey();
  EXPECT_EQ(kKeyLiteral, reqKeyPiece);
  EXPECT_EQ(kKeyLiteral, req.key()->fullKey());
  EXPECT_EQ("abcdefghijklmnopqrstuvwxyz", req.key()->routingKey().str());
  EXPECT_EQ("/region/cluster/", req.key()->routingPrefix().str());
  EXPECT_EQ(
      "abcdefghijklmnopqrstuvwxyz|#|afterhashstop",
      req.key()->keyWithoutRoute().str());
  EXPECT_EQ("|#|afterhashstop", req.key()->afterRoutingKey().str());

  // bool
  req.testBool() = true;
  EXPECT_TRUE(*(req.testBool()));

  // int8_t
  req.testInt8() = kMinInt8;
  EXPECT_EQ(kMinInt8, *(req.testInt8()));
  // int16_t
  req.testInt16() = kMinInt16;
  EXPECT_EQ(kMinInt16, *(req.testInt16()));
  // int32_t
  req.testInt32() = kMinInt32;
  EXPECT_EQ(kMinInt32, *(req.testInt32()));
  // int64_t
  req.testInt64() = kMinInt64;
  EXPECT_EQ(kMinInt64, *(req.testInt64()));
  // uint8_t
  req.testUInt8() = kMaxUInt8;
  EXPECT_EQ(kMaxUInt8, *(req.testUInt8()));
  // uint16_t
  req.testUInt16() = kMaxUInt16;
  EXPECT_EQ(kMaxUInt16, *(req.testUInt16()));
  // uint32_t
  req.testUInt32() = kMaxUInt32;
  EXPECT_EQ(kMaxUInt32, *(req.testUInt32()));
  // uint64_t
  req.testUInt64() = kMaxUInt64;
  EXPECT_EQ(kMaxUInt64, *(req.testUInt64()));

  // float
  req.testFloat() = 12345.789f;
  EXPECT_FLOAT_EQ(12345.789f, *(req.testFloat()));
  // double
  req.testDouble() = 12345.789;
  EXPECT_DOUBLE_EQ(12345.789, *(req.testDouble()));

  // optionals field_ref
  const auto lstringRef = longString();
  req.testOptionalKeywordString() = lstringRef;
  EXPECT_EQ(lstringRef, *req.testOptionalKeywordString());
  req.testOptionalKeywordIobuf() =
      folly::IOBuf(folly::IOBuf::COPY_BUFFER, lstringRef);
  EXPECT_EQ(lstringRef, coalesceAndGetRange(req.testOptionalKeywordIobuf()));
  req.testOptionalKeywordBool() = false;
  EXPECT_EQ(false, *req.testOptionalKeywordBool());

  // string
  req.testShortString() = kShortString.str();
  EXPECT_EQ(kShortString, *(req.testShortString()));
  req.testLongString() = longString();
  EXPECT_EQ(longString(), *(req.testLongString()));
  // IOBuf
  folly::IOBuf iobuf(folly::IOBuf::COPY_BUFFER, longString());
  req.testIobuf() = iobuf;
  EXPECT_EQ(
      coalesceAndGetRange(iobuf).str(),
      coalesceAndGetRange(req.testIobuf()).str());

  std::vector<std::string> strings = {
      "abcdefg", "xyz", kShortString.str(), longString()};
  req.testList() = strings;
  EXPECT_EQ(strings, *(req.testList()));
}

TEST(CarbonBasic, mixinsFieldRefAPIThrift) {
  ThriftTestRequest req;
  EXPECT_EQ(0, *req.base()->myBaseStruct()->baseInt64Member());

  req.base()->myBaseStruct()->baseInt64Member() = 12345;
  // Exercise the different ways we can access the mixed-in baseInt64Member
  EXPECT_EQ(12345, *req.base()->myBaseStruct()->baseInt64Member());
  EXPECT_EQ(12345, *req.base()->baseInt64Member());
  EXPECT_EQ(12345, *req.myBaseStruct()->baseInt64Member());
  EXPECT_EQ(12345, *req.baseInt64Member());
}
