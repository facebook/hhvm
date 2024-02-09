/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <sys/uio.h>

#include <cstring>
#include <string>

#include <gtest/gtest.h>
#include <string.h>

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
  const auto emptyRoutingKeyHash = TestRequest().key_ref()->routingKeyHash();

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
      folly::sformat("{} to {}", typeid(T2).name(), typeid(T1).name()));
  checkStructWithEnum(
      struct2,
      serializeAndDeserialize<T1, T2>(struct1),
      expectCompatible,
      folly::sformat("{} to {}", typeid(T1).name(), typeid(T2).name()));
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
      folly::sformat("{} to {}", typeid(T2).name(), typeid(T1).name()));
  checkStructWithOptionalEnum(
      struct2,
      serializeAndDeserialize<T1, T2>(struct1),
      expectCompatible,
      folly::sformat("{} to {}", typeid(T1).name(), typeid(T2).name()));
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
  checkKeyEmpty(*req.key_ref());
  checkKeyEmpty(*req2.key_ref());

  // bool
  EXPECT_FALSE(*req.testBool_ref());
  // char
  EXPECT_EQ('\0', *req.testChar_ref());
  // int8_t
  EXPECT_EQ(0, *req.testInt8_ref());
  // int16_t
  EXPECT_EQ(0, *req.testInt16_ref());
  // int32_t
  EXPECT_EQ(0, *req.testInt32_ref());
  // int64_t
  EXPECT_EQ(0, *req.testInt64_ref());
  // uint8_t
  EXPECT_EQ(0, *req.testUInt8_ref());
  // uint16_t
  EXPECT_EQ(0, *req.testUInt16_ref());
  // uint32_t
  EXPECT_EQ(0, *req.testUInt32_ref());
  // uint64_t
  EXPECT_EQ(0, *req.testUInt64_ref());

  // float
  EXPECT_EQ(0.0, *req.testFloat_ref());
  // double
  EXPECT_EQ(0.0, *req.testDouble_ref());

  // string
  EXPECT_TRUE(req.testShortString_ref()->empty());
  EXPECT_TRUE(req.testLongString_ref()->empty());
  // IOBuf
  EXPECT_TRUE(req.testIobuf_ref()->empty());

  // List of strings
  EXPECT_TRUE(req.testList_ref()->empty());

  // Vector of vectors
  EXPECT_TRUE(req.testNestedVec_ref()->empty());

  // folly::Optional fields
  EXPECT_FALSE(req.testOptionalIobuf_ref().has_value());

  // optional fields
  EXPECT_FALSE(req.testOptionalKeywordString_ref().has_value());
  EXPECT_FALSE(req.testOptionalKeywordIobuf_ref().has_value());
  EXPECT_FALSE(req.testOptionalKeywordBool_ref().has_value());

  // optional fields field_ref
  EXPECT_FALSE(req.testOptionalKeywordString_ref().has_value());
  EXPECT_FALSE(req.testOptionalKeywordIobuf_ref().has_value());
  EXPECT_FALSE(req.testOptionalKeywordBool_ref().has_value());

  // Unordered map
  EXPECT_TRUE(req.testUMap_ref()->empty());

  // Ordered map
  EXPECT_TRUE(req.testMap_ref()->empty());

  // Complex map
  EXPECT_TRUE(req.testComplexMap_ref()->empty());

  // Unordered set
  EXPECT_TRUE(req.testUSet_ref()->empty());

  // Ordered set
  EXPECT_TRUE(req.testSet_ref()->empty());

  // fields generated for every request (will likely be removed in the future)
  EXPECT_EQ(0, facebook::memcache::getExptimeIfExist(req));
  EXPECT_EQ(0, facebook::memcache::getFlagsIfExist(req));
}

TEST(CarbonBasic, setAndGet) {
  TestRequest req(kKeyLiteral);
  TestRequestStringKey req2(kKeyLiteral);

  // key
  const auto reqKeyPiece = req.key_ref()->fullKey();
  EXPECT_EQ(kKeyLiteral, reqKeyPiece);
  EXPECT_EQ(kKeyLiteral, req.key_ref()->fullKey());
  EXPECT_EQ("abcdefghijklmnopqrstuvwxyz", req.key_ref()->routingKey().str());
  EXPECT_EQ("/region/cluster/", req.key_ref()->routingPrefix().str());
  EXPECT_EQ(
      "abcdefghijklmnopqrstuvwxyz|#|afterhashstop",
      req.key_ref()->keyWithoutRoute().str());
  EXPECT_EQ("|#|afterhashstop", req.key_ref()->afterRoutingKey().str());

  const auto reqKeyPiece2 = req2.key_ref()->fullKey();
  EXPECT_EQ(kKeyLiteral, reqKeyPiece2);
  EXPECT_EQ(kKeyLiteral, req2.key_ref()->fullKey());
  EXPECT_EQ("abcdefghijklmnopqrstuvwxyz", req2.key_ref()->routingKey().str());
  EXPECT_EQ("/region/cluster/", req2.key_ref()->routingPrefix().str());
  EXPECT_EQ(
      "abcdefghijklmnopqrstuvwxyz|#|afterhashstop",
      req2.key_ref()->keyWithoutRoute().str());
  EXPECT_EQ("|#|afterhashstop", req2.key_ref()->afterRoutingKey().str());

  // bool
  req.testBool_ref() = true;
  EXPECT_TRUE(*req.testBool_ref());
  // char
  req.testChar_ref() = 'A';
  EXPECT_EQ('A', *req.testChar_ref());

  // int8_t
  req.testInt8_ref() = kMinInt8;
  EXPECT_EQ(kMinInt8, *req.testInt8_ref());
  // int16_t
  req.testInt16_ref() = kMinInt16;
  EXPECT_EQ(kMinInt16, *req.testInt16_ref());
  // int32_t
  req.testInt32_ref() = kMinInt32;
  EXPECT_EQ(kMinInt32, *req.testInt32_ref());
  // int64_t
  req.testInt64_ref() = kMinInt64;
  EXPECT_EQ(kMinInt64, *req.testInt64_ref());
  // uint8_t
  req.testUInt8_ref() = kMaxUInt8;
  EXPECT_EQ(kMaxUInt8, *req.testUInt8_ref());
  // uint16_t
  req.testUInt16_ref() = kMaxUInt16;
  EXPECT_EQ(kMaxUInt16, *req.testUInt16_ref());
  // uint32_t
  req.testUInt32_ref() = kMaxUInt32;
  EXPECT_EQ(kMaxUInt32, *req.testUInt32_ref());
  // uint64_t
  req.testUInt64_ref() = kMaxUInt64;
  EXPECT_EQ(kMaxUInt64, *req.testUInt64_ref());

  // float
  req.testFloat_ref() = 12345.789f;
  EXPECT_FLOAT_EQ(12345.789f, *req.testFloat_ref());
  // double
  req.testDouble_ref() = 12345.789;
  EXPECT_DOUBLE_EQ(12345.789, *req.testDouble_ref());

  // string
  req.testShortString_ref() = kShortString.str();
  EXPECT_EQ(kShortString, *req.testShortString_ref());
  req.testLongString_ref() = longString();
  EXPECT_EQ(longString(), *req.testLongString_ref());
  // IOBuf
  folly::IOBuf iobuf(folly::IOBuf::COPY_BUFFER, longString());
  req.testIobuf_ref() = iobuf;
  EXPECT_EQ(
      coalesceAndGetRange(iobuf).str(),
      coalesceAndGetRange(req.testIobuf_ref()).str());

  std::vector<std::string> strings = {
      "abcdefg", "xyz", kShortString.str(), longString()};
  req.testList_ref() = strings;
  EXPECT_EQ(strings, *req.testList_ref());

  // Vector of vectors
  std::vector<std::vector<uint64_t>> vectors = {{1, 1, 1}, {2, 2}};
  req.testNestedVec_ref() = vectors;
  EXPECT_EQ(vectors, *req.testNestedVec_ref());

  // folly::Optional fields
  const auto s = longString();
  req.testOptionalIobuf_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, s);
  EXPECT_EQ(s, coalesceAndGetRange(*req.testOptionalIobuf_ref()));
  req.testOptionalBool_ref() = false;
  EXPECT_EQ(false, *req.testOptionalBool_ref());
  std::vector<folly::Optional<std::string>> ovec;
  ovec.emplace_back(folly::Optional<std::string>("hello"));

  // optional fields
  const auto lstring = longString();
  req.testOptionalKeywordString_ref() = lstring;
  EXPECT_EQ(lstring, *req.testOptionalKeywordString_ref());
  req.testOptionalKeywordIobuf_ref() =
      folly::IOBuf(folly::IOBuf::COPY_BUFFER, lstring);
  EXPECT_EQ(lstring, coalesceAndGetRange(*req.testOptionalKeywordIobuf_ref()));
  req.testOptionalKeywordBool_ref() = false;
  EXPECT_EQ(false, *req.testOptionalKeywordBool_ref());

  // optional fields ref api
  const auto lstringRef = longString();
  req.testOptionalKeywordString_ref() = lstringRef;
  EXPECT_EQ(lstringRef, *req.testOptionalKeywordString_ref());
  req.testOptionalKeywordIobuf_ref() =
      folly::IOBuf(folly::IOBuf::COPY_BUFFER, lstringRef);
  EXPECT_EQ(
      lstringRef, coalesceAndGetRange(req.testOptionalKeywordIobuf_ref()));
  req.testOptionalKeywordBool_ref() = false;
  EXPECT_EQ(false, *req.testOptionalKeywordBool_ref());

  // Unordered map
  std::unordered_map<std::string, std::string> stringmap;
  stringmap.insert({"key", "value"});
  req.testUMap_ref() = stringmap;
  EXPECT_EQ(stringmap, *req.testUMap_ref());

  // Ordered map
  std::map<double, double> doublemap;
  doublemap.insert({1.08, 8.3});
  req.testMap_ref() = doublemap;
  EXPECT_EQ(doublemap, *req.testMap_ref());

  // Complex map
  std::map<std::string, std::vector<uint16_t>> complexmap;
  complexmap.insert({"key", {1, 2}});
  req.testComplexMap_ref() = complexmap;
  EXPECT_EQ(complexmap, *req.testComplexMap_ref());

  // Unordered set
  std::unordered_set<std::string> stringset;
  stringset.insert("hello");
  stringset.insert("world");
  req.testUSet_ref() = stringset;
  EXPECT_EQ(stringset, *req.testUSet_ref());

  // Ordered set
  std::set<uint64_t> intset;
  intset.insert(1);
  intset.insert(2);
  req.testSet_ref() = intset;
  EXPECT_EQ(intset, *req.testSet_ref());
}

TEST(CarbonTest, serializeDeserialize) {
  // Fill in a request
  TestRequest outRequest("abcdefghijklmnopqrstuvwxyz");
  outRequest.testBool_ref() = true;
  outRequest.testChar_ref() = 'A';
  outRequest.testInt8_ref() = kMinInt8;
  outRequest.testInt16_ref() = kMinInt16;
  outRequest.testInt32_ref() = kMinInt32;
  outRequest.testInt64_ref() = kMinInt64;
  outRequest.testUInt8_ref() = kMaxUInt8;
  outRequest.testUInt16_ref() = kMaxUInt16;
  outRequest.testUInt32_ref() = kMaxUInt32;
  outRequest.testUInt64_ref() = kMaxUInt64;
  outRequest.testFloat_ref() = 12345.678f;
  outRequest.testDouble_ref() = 12345.678;
  outRequest.testShortString_ref() = kShortString.str();
  outRequest.testLongString_ref() = longString();
  outRequest.testIobuf_ref() =
      folly::IOBuf(folly::IOBuf::COPY_BUFFER, kShortString);
  // List of strings
  outRequest.testList_ref() = {
      "abcdefg", "xyz", kShortString.str(), longString()};
  // Other optional field gets a value of zero length
  outRequest.testOptionalIobuf_ref() =
      folly::IOBuf(folly::IOBuf::COPY_BUFFER, "");

  outRequest.testNestedVec_ref()->push_back({100, 2000});

  outRequest.testUMap_ref()->insert({"hello", "world"});
  outRequest.testMap_ref()->insert({1.08, 8.3});
  outRequest.testF14FastMap_ref()->insert({"hello", "F14FastMap"});
  outRequest.testF14NodeMap_ref()->insert({"hello", "F14NodeMap"});
  outRequest.testF14ValueMap_ref()->insert({"hello", "F14ValueMap"});
  outRequest.testF14VectorMap_ref()->insert({"hello", "F14VectorMap"});
  outRequest.testComplexMap_ref()->insert({"key", {1, 2}});

  outRequest.testUSet_ref()->insert("hello");
  outRequest.testSet_ref()->insert(123);
  outRequest.testF14FastSet_ref()->insert("hello F14FastSet");
  outRequest.testF14NodeSet_ref()->insert("hello F14NodeSet");
  outRequest.testF14ValueSet_ref()->insert("hello F14ValueSet");
  outRequest.testF14VectorSet_ref()->insert("hello F14VectorSet");
  outRequest.testOptionalBool_ref() = false;
  outRequest.testIOBufList_ref()->emplace_back(folly::IOBuf());
  const auto inRequest = serializeAndDeserialize(outRequest);
  expectEqTestRequest(outRequest, inRequest);
}

TEST(CarbonTest, veryLongString) {
  constexpr uint32_t kVeryLongStringSize = 1 << 30;
  std::string veryLongString(kVeryLongStringSize, 'x');

  TestRequest outRequest(longString());
  outRequest.testLongString_ref() = std::move(veryLongString);
  const auto inRequest = serializeAndDeserialize(outRequest);
  expectEqTestRequest(outRequest, inRequest);
  EXPECT_EQ(kVeryLongStringSize, inRequest.testLongString_ref()->length());
}

TEST(CarbonTest, repeatStorageUsage) {
  std::string testStr(longString());

  carbon::CarbonQueueAppenderStorage storage;
  carbon::CarbonProtocolWriter writer(storage);

  TestRequest outRequest(longString());
  outRequest.testLongString_ref() = std::move(testStr);

  for (int i = 0; i < 100; i++) {
    outRequest.serialize(writer);
    storage.reset();
  }
}

TEST(CarbonTest, veryLongIobuf) {
  constexpr uint32_t kVeryLongIobufSize = 1 << 30;
  folly::IOBuf veryLongIobuf(folly::IOBuf::CREATE, kVeryLongIobufSize);
  std::memset(veryLongIobuf.writableTail(), 'x', kVeryLongIobufSize);
  veryLongIobuf.append(kVeryLongIobufSize);

  TestRequest outRequest(longString());
  outRequest.testIobuf_ref() = std::move(veryLongIobuf);
  const auto inRequest = serializeAndDeserialize(outRequest);
  expectEqTestRequest(outRequest, inRequest);
  EXPECT_EQ(kVeryLongIobufSize, inRequest.testIobuf_ref()->length());
}

TEST(CarbonTest, keysIobuf) {
  {
    TestRequest req;
    checkKeyEmpty(*req.key_ref());
  }
  {
    TestRequest req;

    const folly::IOBuf keyCopy(folly::IOBuf::CopyBufferOp(), kKeyLiteral);
    req.key_ref() = keyCopy;
    checkKeyFilledProperly(*req.key_ref());

    req.key_ref() = "";
    checkKeyEmpty(*req.key_ref());
  }
  {
    TestRequest req;
    checkKeyEmpty(*req.key_ref());

    req.key_ref() = folly::IOBuf(folly::IOBuf::CopyBufferOp(), kKeyLiteral);
    checkKeyFilledProperly(*req.key_ref());
  }
  {
    TestRequest req(kKeyLiteral);
    checkKeyFilledProperly(*req.key_ref());
  }
  {
    TestRequest req{folly::StringPiece(kKeyLiteral)};
    checkKeyFilledProperly(*req.key_ref());
  }
  {
    TestRequest req(folly::IOBuf(folly::IOBuf::CopyBufferOp(), kKeyLiteral));
    checkKeyFilledProperly(*req.key_ref());
  }
}

TEST(CarbonTest, keysString) {
  {
    TestRequestStringKey req;
    checkKeyEmpty(*req.key_ref());
  }
  {
    TestRequestStringKey req;

    const std::string keyCopy(kKeyLiteral);
    req.key_ref() = keyCopy;
    checkKeyFilledProperly(*req.key_ref());

    req.key_ref() = "";
    checkKeyEmpty(*req.key_ref());
  }
  {
    TestRequestStringKey req;
    checkKeyEmpty(*req.key_ref());

    req.key_ref() = kKeyLiteral;
    checkKeyFilledProperly(*req.key_ref());
  }
  {
    TestRequestStringKey req(kKeyLiteral);
    checkKeyFilledProperly(*req.key_ref());
  }
  {
    TestRequest req{folly::StringPiece(kKeyLiteral)};
    checkKeyFilledProperly(*req.key_ref());
  }
  {
    TestRequest req{std::string(kKeyLiteral)};
    checkKeyFilledProperly(*req.key_ref());
  }
}

TEST(CarbonBasic, defaultConstructedFieldRefAPI) {
  TestRequest req;
  TestRequestStringKey req2;

  // key
  checkKeyEmpty(*req.key_ref());
  checkKeyEmpty(*req2.key_ref());

  // bool
  EXPECT_FALSE(*req.testBool_ref());
  // char
  EXPECT_EQ('\0', *req.testChar_ref());

  // int8_t
  EXPECT_EQ(0, *req.testInt8_ref());
  // int16_t
  EXPECT_EQ(0, *req.testInt16_ref());
  // int32_t
  EXPECT_EQ(0, *req.testInt32_ref());
  // int64_t
  EXPECT_EQ(0, *req.testInt64_ref());
  // uint8_t
  EXPECT_EQ(0, *req.testUInt8_ref());
  // uint16_t
  EXPECT_EQ(0, *req.testUInt16_ref());
  // uint32_t
  EXPECT_EQ(0, *req.testUInt32_ref());
  // uint64_t
  EXPECT_EQ(0, *req.testUInt64_ref());

  // float
  EXPECT_EQ(0.0, *req.testFloat_ref());
  // double
  EXPECT_EQ(0.0, *req.testDouble_ref());

  // string
  EXPECT_TRUE(req.testShortString_ref()->empty());
  EXPECT_TRUE(req.testLongString_ref()->empty());
  // IOBuf
  EXPECT_TRUE(req.testIobuf_ref()->empty());

  // List of strings
  EXPECT_TRUE(req.testList_ref()->empty());

  // Vector of vectors
  EXPECT_TRUE(req.testNestedVec_ref()->empty());

  // Unordered map
  EXPECT_TRUE(req.testUMap_ref()->empty());

  // Ordered map
  EXPECT_TRUE(req.testMap_ref()->empty());

  // Complex map
  EXPECT_TRUE(req.testComplexMap_ref()->empty());

  // Unordered set
  EXPECT_TRUE(req.testUSet_ref()->empty());

  // Ordered set
  EXPECT_TRUE(req.testSet_ref()->empty());

  // fields generated for every request (will likely be removed in the future)
  EXPECT_EQ(0, facebook::memcache::getExptimeIfExist(req));
  EXPECT_EQ(0, facebook::memcache::getFlagsIfExist(req));
}

TEST(CarbonBasic, setAndGetFieldRefAPI) {
  TestRequest req(kKeyLiteral);
  TestRequestStringKey req2(kKeyLiteral);

  // key
  const auto reqKeyPiece = req.key_ref()->fullKey();
  EXPECT_EQ(kKeyLiteral, reqKeyPiece);
  EXPECT_EQ(kKeyLiteral, req.key_ref()->fullKey());
  EXPECT_EQ("abcdefghijklmnopqrstuvwxyz", req.key_ref()->routingKey().str());
  EXPECT_EQ("/region/cluster/", req.key_ref()->routingPrefix().str());
  EXPECT_EQ(
      "abcdefghijklmnopqrstuvwxyz|#|afterhashstop",
      req.key_ref()->keyWithoutRoute().str());
  EXPECT_EQ("|#|afterhashstop", req.key_ref()->afterRoutingKey().str());

  const auto reqKeyPiece2 = req2.key_ref()->fullKey();
  EXPECT_EQ(kKeyLiteral, reqKeyPiece2);
  EXPECT_EQ(kKeyLiteral, req2.key_ref()->fullKey());
  EXPECT_EQ("abcdefghijklmnopqrstuvwxyz", req2.key_ref()->routingKey().str());
  EXPECT_EQ("/region/cluster/", req2.key_ref()->routingPrefix().str());
  EXPECT_EQ(
      "abcdefghijklmnopqrstuvwxyz|#|afterhashstop",
      req2.key_ref()->keyWithoutRoute().str());
  EXPECT_EQ("|#|afterhashstop", req2.key_ref()->afterRoutingKey().str());

  // bool
  req.testBool_ref() = true;
  EXPECT_TRUE(*req.testBool_ref());
  // char
  req.testChar_ref() = 'A';
  EXPECT_EQ('A', *req.testChar_ref());

  // int8_t
  req.testInt8_ref() = kMinInt8;
  EXPECT_EQ(kMinInt8, *req.testInt8_ref());
  // int16_t
  req.testInt16_ref() = kMinInt16;
  EXPECT_EQ(kMinInt16, *req.testInt16_ref());
  // int32_t
  req.testInt32_ref() = kMinInt32;
  EXPECT_EQ(kMinInt32, *req.testInt32_ref());
  // int64_t
  req.testInt64_ref() = kMinInt64;
  EXPECT_EQ(kMinInt64, *req.testInt64_ref());
  // uint8_t
  req.testUInt8_ref() = kMaxUInt8;
  EXPECT_EQ(kMaxUInt8, *req.testUInt8_ref());
  // uint16_t
  req.testUInt16_ref() = kMaxUInt16;
  EXPECT_EQ(kMaxUInt16, *req.testUInt16_ref());
  // uint32_t
  req.testUInt32_ref() = kMaxUInt32;
  EXPECT_EQ(kMaxUInt32, *req.testUInt32_ref());
  // uint64_t
  req.testUInt64_ref() = kMaxUInt64;
  EXPECT_EQ(kMaxUInt64, *req.testUInt64_ref());

  // float
  req.testFloat_ref() = 12345.789f;
  EXPECT_FLOAT_EQ(12345.789f, *req.testFloat_ref());
  // double
  req.testDouble_ref() = 12345.789;
  EXPECT_DOUBLE_EQ(12345.789, *req.testDouble_ref());

  // string
  req.testShortString_ref() = kShortString.str();
  EXPECT_EQ(kShortString, *req.testShortString_ref());
  req.testLongString_ref() = longString();
  EXPECT_EQ(longString(), *req.testLongString_ref());
  // IOBuf
  folly::IOBuf iobuf(folly::IOBuf::COPY_BUFFER, longString());
  req.testIobuf_ref() = iobuf;
  EXPECT_EQ(
      coalesceAndGetRange(iobuf).str(),
      coalesceAndGetRange(req.testIobuf_ref()).str());

  std::vector<std::string> strings = {
      "abcdefg", "xyz", kShortString.str(), longString()};
  req.testList_ref() = strings;
  EXPECT_EQ(strings, *req.testList_ref());

  // optionals
  EXPECT_FALSE(req.testOptionalKeywordString_ref().has_value());
  EXPECT_FALSE(req.testOptionalKeywordIobuf_ref().has_value());
  EXPECT_FALSE(req.testOptionalKeywordBool_ref().has_value());

  // optionals field_ref
  EXPECT_FALSE(req.testOptionalKeywordString_ref().has_value());
  EXPECT_FALSE(req.testOptionalKeywordIobuf_ref().has_value());
  EXPECT_FALSE(req.testOptionalKeywordBool_ref().has_value());

  // Vector of vectors
  std::vector<std::vector<uint64_t>> vectors = {{1, 1, 1}, {2, 2}};
  req.testNestedVec_ref() = vectors;
  EXPECT_EQ(vectors, *req.testNestedVec_ref());

  // Unordered map
  std::unordered_map<std::string, std::string> stringmap;
  stringmap.insert({"key", "value"});
  req.testUMap_ref() = stringmap;
  EXPECT_EQ(stringmap, *req.testUMap_ref());

  // Ordered map
  std::map<double, double> doublemap;
  doublemap.insert({1.08, 8.3});
  req.testMap_ref() = doublemap;
  EXPECT_EQ(doublemap, *req.testMap_ref());

  // Complex map
  std::map<std::string, std::vector<uint16_t>> complexmap;
  complexmap.insert({"key", {1, 2}});
  req.testComplexMap_ref() = complexmap;
  EXPECT_EQ(complexmap, *req.testComplexMap_ref());

  // Unordered set
  std::unordered_set<std::string> stringset;
  stringset.insert("hello");
  stringset.insert("world");
  req.testUSet_ref() = stringset;
  EXPECT_EQ(stringset, *req.testUSet_ref());

  // Ordered set
  std::set<uint64_t> intset;
  intset.insert(1);
  intset.insert(2);
  req.testSet_ref() = intset;
  EXPECT_EQ(intset, *req.testSet_ref());
}

TEST(CarbonBasic, setAndGetFieldRefAPIThrift) {
  DummyThriftRequest req(kKeyLiteral);

  // key
  const auto reqKeyPiece = req.key_ref()->fullKey();
  EXPECT_EQ(kKeyLiteral, reqKeyPiece);
  EXPECT_EQ(kKeyLiteral, req.key_ref()->fullKey());
  EXPECT_EQ("abcdefghijklmnopqrstuvwxyz", req.key_ref()->routingKey().str());
  EXPECT_EQ("/region/cluster/", req.key_ref()->routingPrefix().str());
  EXPECT_EQ(
      "abcdefghijklmnopqrstuvwxyz|#|afterhashstop",
      req.key_ref()->keyWithoutRoute().str());
  EXPECT_EQ("|#|afterhashstop", req.key_ref()->afterRoutingKey().str());

  // bool
  req.testBool_ref() = true;
  EXPECT_TRUE(*(req.testBool_ref()));

  // int8_t
  req.testInt8_ref() = kMinInt8;
  EXPECT_EQ(kMinInt8, *(req.testInt8_ref()));
  // int16_t
  req.testInt16_ref() = kMinInt16;
  EXPECT_EQ(kMinInt16, *(req.testInt16_ref()));
  // int32_t
  req.testInt32_ref() = kMinInt32;
  EXPECT_EQ(kMinInt32, *(req.testInt32_ref()));
  // int64_t
  req.testInt64_ref() = kMinInt64;
  EXPECT_EQ(kMinInt64, *(req.testInt64_ref()));
  // uint8_t
  req.testUInt8_ref() = kMaxUInt8;
  EXPECT_EQ(kMaxUInt8, *(req.testUInt8_ref()));
  // uint16_t
  req.testUInt16_ref() = kMaxUInt16;
  EXPECT_EQ(kMaxUInt16, *(req.testUInt16_ref()));
  // uint32_t
  req.testUInt32_ref() = kMaxUInt32;
  EXPECT_EQ(kMaxUInt32, *(req.testUInt32_ref()));
  // uint64_t
  req.testUInt64_ref() = kMaxUInt64;
  EXPECT_EQ(kMaxUInt64, *(req.testUInt64_ref()));

  // float
  req.testFloat_ref() = 12345.789f;
  EXPECT_FLOAT_EQ(12345.789f, *(req.testFloat_ref()));
  // double
  req.testDouble_ref() = 12345.789;
  EXPECT_DOUBLE_EQ(12345.789, *(req.testDouble_ref()));

  // optionals field_ref
  const auto lstringRef = longString();
  req.testOptionalKeywordString_ref() = lstringRef;
  EXPECT_EQ(lstringRef, *req.testOptionalKeywordString_ref());
  req.testOptionalKeywordIobuf_ref() =
      folly::IOBuf(folly::IOBuf::COPY_BUFFER, lstringRef);
  EXPECT_EQ(
      lstringRef, coalesceAndGetRange(req.testOptionalKeywordIobuf_ref()));
  req.testOptionalKeywordBool_ref() = false;
  EXPECT_EQ(false, *req.testOptionalKeywordBool_ref());

  // string
  req.testShortString_ref() = kShortString.str();
  EXPECT_EQ(kShortString, *(req.testShortString_ref()));
  req.testLongString_ref() = longString();
  EXPECT_EQ(longString(), *(req.testLongString_ref()));
  // IOBuf
  folly::IOBuf iobuf(folly::IOBuf::COPY_BUFFER, longString());
  req.testIobuf_ref() = iobuf;
  EXPECT_EQ(
      coalesceAndGetRange(iobuf).str(),
      coalesceAndGetRange(req.testIobuf_ref()).str());

  std::vector<std::string> strings = {
      "abcdefg", "xyz", kShortString.str(), longString()};
  req.testList_ref() = strings;
  EXPECT_EQ(strings, *(req.testList_ref()));
}

TEST(CarbonBasic, mixinsFieldRefAPIThrift) {
  ThriftTestRequest req;
  EXPECT_EQ(0, *req.base_ref()->myBaseStruct_ref()->baseInt64Member_ref());

  req.base_ref()->myBaseStruct_ref()->baseInt64Member_ref() = 12345;
  // Exercise the different ways we can access the mixed-in baseInt64Member
  EXPECT_EQ(12345, *req.base_ref()->myBaseStruct_ref()->baseInt64Member_ref());
  EXPECT_EQ(12345, *req.base_ref()->baseInt64Member_ref());
  EXPECT_EQ(12345, *req.myBaseStruct_ref()->baseInt64Member_ref());
  EXPECT_EQ(12345, *req.baseInt64Member_ref());
}
