/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/structuredheaders/StructuredHeadersEncoder.h>

#include <folly/Conv.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <unordered_map>

using namespace testing;

namespace proxygen {

class StructuredHeadersEncoderTest : public testing::Test {};

TEST_F(StructuredHeadersEncoderTest, TestInteger) {
  StructuredHeaderItem item;
  int64_t val = 2018;
  item.tag = StructuredHeaderItem::Type::INT64;
  item.value = val;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeItem(item);

  EXPECT_EQ(err, EncodeError::OK);
  EXPECT_EQ(encoder.get(), "2018");
}

TEST_F(StructuredHeadersEncoderTest, TestIntegerNegative) {
  StructuredHeaderItem item;
  int64_t val = -2018;
  item.tag = StructuredHeaderItem::Type::INT64;
  item.value = val;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeItem(item);

  EXPECT_EQ(err, EncodeError::OK);
  EXPECT_EQ(encoder.get(), "-2018");
}

TEST_F(StructuredHeadersEncoderTest, TestBoolean) {
  StructuredHeaderItem item;
  for (auto i = 0; i < 2; i++) {
    bool val = i;
    item.tag = StructuredHeaderItem::Type::BOOLEAN;
    item.value = val;

    StructuredHeadersEncoder encoder;
    auto err = encoder.encodeItem(item);

    EXPECT_EQ(err, EncodeError::OK);
    EXPECT_EQ(encoder.get(), folly::to<std::string>("?", (val ? "1" : "0")));
  }
}

TEST_F(StructuredHeadersEncoderTest, TestFloat) {
  StructuredHeaderItem item;
  double val = 3.1415926535;
  item.tag = StructuredHeaderItem::Type::DOUBLE;
  item.value = val;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeItem(item);

  EXPECT_EQ(err, EncodeError::OK);
  EXPECT_EQ(encoder.get(), "3.1415926535");
}

TEST_F(StructuredHeadersEncoderTest, TestFloatTooMuchPrecision) {
  StructuredHeadersEncoder encoder;
  StructuredHeaderItem item;
  double val = 100000.8392758372647; // has 20 characters
  item.tag = StructuredHeaderItem::Type::DOUBLE;
  item.value = val;

  auto err = encoder.encodeItem(item);

  EXPECT_EQ(err, EncodeError::OK);
  EXPECT_EQ(encoder.get(), "100000.839275837");
}

TEST_F(StructuredHeadersEncoderTest, TestFloatNegative) {
  StructuredHeaderItem item;
  double val = -3.141;
  item.tag = StructuredHeaderItem::Type::DOUBLE;
  item.value = val;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeItem(item);

  EXPECT_EQ(err, EncodeError::OK);
  EXPECT_EQ(encoder.get(), "-3.141");
}

TEST_F(StructuredHeadersEncoderTest, TestString) {
  StructuredHeaderItem item;
  std::string val = "seattle is the best";
  item.tag = StructuredHeaderItem::Type::STRING;
  item.value = val;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeItem(item);

  EXPECT_EQ(err, EncodeError::OK);
  EXPECT_EQ(encoder.get(), "\"seattle is the best\"");
}

TEST_F(StructuredHeadersEncoderTest, TestStringBadContent) {
  StructuredHeaderItem item;
  std::string val = "seattle \n is the best";
  item.tag = StructuredHeaderItem::Type::STRING;
  item.value = val;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeItem(item);

  EXPECT_NE(err, EncodeError::OK);
}

TEST_F(StructuredHeadersEncoderTest, TestStringBackslash) {
  StructuredHeaderItem item;
  std::string val = "seattle \\is the best";
  item.tag = StructuredHeaderItem::Type::STRING;
  item.value = val;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeItem(item);

  EXPECT_EQ(err, EncodeError::OK);
  EXPECT_EQ(encoder.get(), "\"seattle \\\\is the best\"");
}

TEST_F(StructuredHeadersEncoderTest, TestStringQuote) {
  StructuredHeaderItem item;
  std::string val = "seattle \"is the best";
  item.tag = StructuredHeaderItem::Type::STRING;
  item.value = val;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeItem(item);

  EXPECT_EQ(err, EncodeError::OK);
  EXPECT_EQ(encoder.get(), "\"seattle \\\"is the best\"");
}

TEST_F(StructuredHeadersEncoderTest, TestBinaryContent) {
  StructuredHeaderItem item;
  std::string val = "seattle <3";
  item.tag = StructuredHeaderItem::Type::BINARYCONTENT;
  item.value = val;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeItem(item);

  EXPECT_EQ(err, EncodeError::OK);
  EXPECT_EQ(encoder.get(), "*c2VhdHRsZSA8Mw==*");
}

TEST_F(StructuredHeadersEncoderTest, TestIdentifier) {
  std::string result;
  std::string val = "abc_00123";

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeIdentifier(val);

  EXPECT_EQ(err, EncodeError::OK);
  EXPECT_EQ(encoder.get(), "abc_00123");
}

TEST_F(StructuredHeadersEncoderTest, TestIdentifierBadContent) {
  std::string result;
  std::string val = "_abc_00123";

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeIdentifier(val);

  EXPECT_NE(err, EncodeError::OK);
}

TEST_F(StructuredHeadersEncoderTest, TestWrongType) {
  StructuredHeaderItem item;
  double val = 3.1415;
  item.tag = StructuredHeaderItem::Type::INT64;
  item.value = val;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeItem(item);

  EXPECT_NE(err, EncodeError::OK);
}

TEST_F(StructuredHeadersEncoderTest, TestListManyElts) {
  std::vector<StructuredHeaderItem> vec;
  StructuredHeaderItem item;

  item.tag = StructuredHeaderItem::Type::DOUBLE;
  double val1 = 3.14;
  item.value = val1;
  vec.push_back(item);

  item.tag = StructuredHeaderItem::Type::BINARYCONTENT;
  std::string val2 = "pizza";
  item.value = val2;
  vec.push_back(item);

  item.tag = StructuredHeaderItem::Type::INT64;
  int64_t val3 = 65;
  item.value = val3;
  vec.push_back(item);

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeList(vec);

  EXPECT_EQ(err, EncodeError::OK);
  EXPECT_EQ(encoder.get(), "3.14, *cGl6emE=*, 65");
}

TEST_F(StructuredHeadersEncoderTest, TestListOneElt) {
  std::vector<StructuredHeaderItem> vec;
  StructuredHeaderItem item;

  item.tag = StructuredHeaderItem::Type::STRING;
  std::string val1 = "hello world";
  item.value = val1;
  vec.push_back(item);

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeList(vec);

  EXPECT_EQ(err, EncodeError::OK);
  EXPECT_EQ(encoder.get(), "\"hello world\"");
}

TEST_F(StructuredHeadersEncoderTest, TestListEmpty) {
  std::vector<StructuredHeaderItem> vec;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeList(vec);

  EXPECT_NE(err, EncodeError::OK);
}

TEST_F(StructuredHeadersEncoderTest, TestListBadItem) {
  std::vector<StructuredHeaderItem> vec;
  StructuredHeaderItem item;

  item.tag = StructuredHeaderItem::Type::STRING;
  std::string val1 = "hello \x10world";
  item.value = val1;
  vec.push_back(item);

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeList(vec);

  EXPECT_NE(err, EncodeError::OK);
}

TEST_F(StructuredHeadersEncoderTest, TestDictionaryOneElt) {
  StructuredHeaders::Dictionary dict;

  StructuredHeaderItem item1;
  item1.tag = StructuredHeaderItem::Type::DOUBLE;
  double val1 = 2.71;
  item1.value = val1;

  dict["e"] = item1;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeDictionary(dict);

  EXPECT_EQ(err, EncodeError::OK);
  EXPECT_EQ(encoder.get(), "e=2.71");
}

TEST_F(StructuredHeadersEncoderTest, TestDictionaryWithTrueBoolean) {
  StructuredHeaders::Dictionary dict;
  StructuredHeaderItem item;
  item.tag = StructuredHeaderItem::Type::BOOLEAN;
  item.value = true;

  dict["u"] = item;

  StructuredHeadersEncoder encoder;
  EXPECT_EQ(EncodeError::OK, encoder.encodeDictionary(dict));
  EXPECT_EQ(encoder.get(), "u");
}

TEST_F(StructuredHeadersEncoderTest, TestDictionaryWithFalseBoolean) {
  StructuredHeaders::Dictionary dict;
  StructuredHeaderItem item;
  item.tag = StructuredHeaderItem::Type::BOOLEAN;
  item.value = false;

  dict["u"] = item;

  StructuredHeadersEncoder encoder;
  EXPECT_EQ(EncodeError::OK, encoder.encodeDictionary(dict));
  EXPECT_EQ(encoder.get(), "u=?0");
}

TEST_F(StructuredHeadersEncoderTest, TestDictionaryManyElts) {
  StructuredHeaders::Dictionary dict;
  StructuredHeaderItem item;

  item.tag = StructuredHeaderItem::Type::INT64;
  int64_t val1 = 87;
  item.value = val1;
  dict["age"] = item;

  item.tag = StructuredHeaderItem::Type::STRING;
  std::string val2 = "John Doe";
  item.value = val2;
  dict["name"] = item;

  item.tag = StructuredHeaderItem::Type::BINARYCONTENT;
  std::string val3 = "password";
  item.value = val3;
  dict["password"] = item;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeDictionary(dict);

  EXPECT_EQ(err, EncodeError::OK);

  // A dictionary is an unordered mapping, so the ordering of specific elements
  // within the dictionary doesn't matter
  EXPECT_THAT(encoder.get(),
              AnyOf(Eq("age=87, name=\"John Doe\", password=*cGFzc3dvcmQ=*"),
                    Eq("age=87, password=*cGFzc3dvcmQ=*, name=\"John Doe\""),
                    Eq("name=\"John Doe\", age=87, password=*cGFzc3dvcmQ=*"),
                    Eq("name=\"John Doe\", password=*cGFzc3dvcmQ=*, age=87"),
                    Eq("password=*cGFzc3dvcmQ=*, name=\"John Doe\", age=87"),
                    Eq("password=*cGFzc3dvcmQ=*, age=87, name=\"John Doe\"")));
}

TEST_F(StructuredHeadersEncoderTest, TestDictionaryEmpty) {
  StructuredHeaders::Dictionary dict;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeDictionary(dict);

  EXPECT_NE(err, EncodeError::OK);
}

TEST_F(StructuredHeadersEncoderTest, TestDictionaryBadItem) {
  StructuredHeaders::Dictionary dict;

  StructuredHeaderItem item1;
  item1.tag = StructuredHeaderItem::Type::STRING;
  std::string val1 = "hi\nmy name is bob";
  item1.value = val1;

  dict["e"] = item1;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeDictionary(dict);

  EXPECT_NE(err, EncodeError::OK);
}

TEST_F(StructuredHeadersEncoderTest, TestDictionaryBadIdentifier) {
  StructuredHeaders::Dictionary dict;

  StructuredHeaderItem item1;
  item1.tag = StructuredHeaderItem::Type::STRING;
  std::string val1 = "hi";
  item1.value = val1;

  dict["_bad_identifier"] = item1;

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeDictionary(dict);

  EXPECT_NE(err, EncodeError::OK);
}

TEST_F(StructuredHeadersEncoderTest, TestParamListOneElt) {
  ParameterisedList pl;
  std::unordered_map<std::string, StructuredHeaderItem> m;

  StructuredHeaderItem item;

  item.tag = StructuredHeaderItem::Type::INT64;
  int64_t val1 = 1;
  item.value = val1;
  m["abc"] = item;

  ParameterisedIdentifier pident = {"foo", m};

  pl.emplace_back(pident);

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeParameterisedList(pl);

  EXPECT_EQ(err, EncodeError::OK);

  EXPECT_EQ(encoder.get(), "foo; abc=1");
}

TEST_F(StructuredHeadersEncoderTest, TestParamListSuccessiveNulls) {
  ParameterisedList pl;
  std::unordered_map<std::string, StructuredHeaderItem> m;

  StructuredHeaderItem item;

  item.tag = StructuredHeaderItem::Type::NONE;
  m["a"] = item;
  m["b"] = item;

  ParameterisedIdentifier pident = {"foo", m};

  pl.emplace_back(pident);

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeParameterisedList(pl);

  EXPECT_EQ(err, EncodeError::OK);

  EXPECT_THAT(encoder.get(), AnyOf(Eq("foo; a; b"), Eq("foo; b; a")));
}

TEST_F(StructuredHeadersEncoderTest, TestParamListManyElts) {
  ParameterisedList pl;
  std::unordered_map<std::string, StructuredHeaderItem> m1;

  StructuredHeaderItem item;

  item.tag = StructuredHeaderItem::Type::DOUBLE;
  double val1 = 4234.234;
  item.value = val1;
  m1["foo"] = item;

  item.tag = StructuredHeaderItem::Type::BINARYCONTENT;
  std::string val2 = "+++!";
  item.value = val2;
  m1["goo"] = item;

  ParameterisedIdentifier pident1 = {"bar", m1};

  pl.emplace_back(pident1);

  std::unordered_map<std::string, StructuredHeaderItem> m2;

  item.tag = StructuredHeaderItem::Type::NONE;
  m2["foo"] = item;

  item.tag = StructuredHeaderItem::Type::INT64;
  int64_t val4 = 100;
  item.value = val4;
  m2["goo"] = item;

  ParameterisedIdentifier pident2 = {"far", m2};

  pl.emplace_back(pident2);

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeParameterisedList(pl);

  EXPECT_EQ(err, EncodeError::OK);

  // The order of the parameters of a particular identifier doesn't matter,
  // so any of these permutations is acceptable
  EXPECT_THAT(
      encoder.get(),
      AnyOf(Eq("bar; foo=4234.234; goo=*KysrIQ==*, far; foo; goo=100"),
            Eq("bar; foo=4234.234; goo=*KysrIQ==*, far; goo=100; foo"),
            Eq("bar; goo=*KysrIQ==*; foo=4234.234, far; foo; goo=100"),
            Eq("bar; goo=*KysrIQ==*; foo=4234.234, far; goo=100; foo"),
            Eq("far; foo; goo=100, bar; foo=4234.234; goo=*KysrIQ==*"),
            Eq("far; foo; goo=100, bar; goo=*KysrIQ==*; foo=4234.234"),
            Eq("far; goo=100; foo, bar; foo=4234.234; goo=*KysrIQ==*"),
            Eq("far; goo=100; foo, bar; goo=*KysrIQ==*; foo=4234.234")));
}

TEST_F(StructuredHeadersEncoderTest, TestParamListEmpty) {
  ParameterisedList pl;
  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeParameterisedList(pl);

  EXPECT_NE(err, EncodeError::OK);
}

TEST_F(StructuredHeadersEncoderTest, TestParamListBadSecondaryIdentifier) {
  ParameterisedList pl;
  std::unordered_map<std::string, StructuredHeaderItem> m;

  StructuredHeaderItem item;

  item.tag = StructuredHeaderItem::Type::STRING;
  std::string val1 = "ABC";
  item.value = val1;
  m["\nbbb"] = item;

  ParameterisedIdentifier pident = {"foo", m};

  pl.emplace_back(pident);

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeParameterisedList(pl);

  EXPECT_NE(err, EncodeError::OK);
}

TEST_F(StructuredHeadersEncoderTest, TestParamListBadPrimaryIdentifier) {
  ParameterisedList pl;
  std::unordered_map<std::string, StructuredHeaderItem> m;

  StructuredHeaderItem item;

  item.tag = StructuredHeaderItem::Type::INT64;
  int64_t val1 = 143;
  item.value = val1;
  m["abc"] = item;

  ParameterisedIdentifier pident = {"a+++", m};

  pl.emplace_back(pident);

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeParameterisedList(pl);

  EXPECT_NE(err, EncodeError::OK);
}

TEST_F(StructuredHeadersEncoderTest, TestParamListBadItems) {
  ParameterisedList pl;
  std::unordered_map<std::string, StructuredHeaderItem> m;

  StructuredHeaderItem item;

  item.tag = StructuredHeaderItem::Type::STRING;
  std::string val1 = "AB\nC";
  item.value = val1;
  m["bbb"] = item;

  ParameterisedIdentifier pident = {"foo", m};

  pl.emplace_back(pident);

  StructuredHeadersEncoder encoder;
  auto err = encoder.encodeParameterisedList(pl);

  EXPECT_NE(err, EncodeError::OK);
}

} // namespace proxygen
