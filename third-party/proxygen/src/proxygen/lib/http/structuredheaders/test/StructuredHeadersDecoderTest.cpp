/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/structuredheaders/StructuredHeadersDecoder.h>

#include <folly/portability/GTest.h>
#include <string>

namespace proxygen {

class StructuredHeadersDecoderTest : public testing::Test {};

TEST_F(StructuredHeadersDecoderTest, TestItem) {
  std::string input = "645643";
  StructuredHeadersDecoder shd(input);

  StructuredHeaderItem item;
  shd.decodeItem(item);

  EXPECT_EQ(item.tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(item, int64_t(645643));
}

TEST_F(StructuredHeadersDecoderTest, TestList) {
  std::string input = "\"cookies\", 3.1415    , 74657";
  StructuredHeadersDecoder shd(input);

  std::vector<StructuredHeaderItem> v;
  shd.decodeList(v);
  EXPECT_EQ(v.size(), 3);

  EXPECT_EQ(v[0].tag, StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(v[1].tag, StructuredHeaderItem::Type::DOUBLE);
  EXPECT_EQ(v[2].tag, StructuredHeaderItem::Type::INT64);

  EXPECT_EQ(v[0], std::string("cookies"));
  EXPECT_EQ(v[1], 3.1415);
  EXPECT_EQ(v[2], int64_t(74657));
}

TEST_F(StructuredHeadersDecoderTest, TestListBeginningWhitespace) {
  std::string input = "   19   , 95";
  StructuredHeadersDecoder shd(input);

  std::vector<StructuredHeaderItem> v;
  shd.decodeList(v);
  EXPECT_EQ(v.size(), 2);

  EXPECT_EQ(v[0].tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(v[1].tag, StructuredHeaderItem::Type::INT64);

  EXPECT_EQ(v[0], int64_t(19));
  EXPECT_EQ(v[1], int64_t(95));
}

TEST_F(StructuredHeadersDecoderTest, TestListEndingWhitespace) {
  std::string input = "19   , 95    ";
  StructuredHeadersDecoder shd(input);

  std::vector<StructuredHeaderItem> v;
  shd.decodeList(v);
  EXPECT_EQ(v.size(), 2);

  EXPECT_EQ(v[0].tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(v[1].tag, StructuredHeaderItem::Type::INT64);

  EXPECT_EQ(v[0], int64_t(19));
  EXPECT_EQ(v[1], int64_t(95));
}

TEST_F(StructuredHeadersDecoderTest, TestListNoWhitespace) {
  std::string input = "19,95";
  StructuredHeadersDecoder shd(input);

  std::vector<StructuredHeaderItem> v;
  shd.decodeList(v);
  EXPECT_EQ(v.size(), 2);

  EXPECT_EQ(v[0].tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(v[1].tag, StructuredHeaderItem::Type::INT64);

  EXPECT_EQ(v[0], int64_t(19));
  EXPECT_EQ(v[1], int64_t(95));
}

TEST_F(StructuredHeadersDecoderTest, TestListOneItem) {
  std::string input = "*Zm9vZA==*";
  StructuredHeadersDecoder shd(input);

  std::vector<StructuredHeaderItem> v;
  shd.decodeList(v);
  EXPECT_EQ(v.size(), 1);

  EXPECT_EQ(v[0].tag, StructuredHeaderItem::Type::BINARYCONTENT);

  EXPECT_EQ(v[0], std::string("food"));
}

TEST_F(StructuredHeadersDecoderTest, TestDictionaryManyElts) {
  std::string input = "age=87  ,  weight=150.8 ,   name=\"John Doe\"";
  StructuredHeadersDecoder shd(input);

  std::unordered_map<std::string, StructuredHeaderItem> m;
  shd.decodeDictionary(m);
  EXPECT_EQ(m.size(), 3);

  EXPECT_EQ(m["age"].tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(m["weight"].tag, StructuredHeaderItem::Type::DOUBLE);
  EXPECT_EQ(m["name"].tag, StructuredHeaderItem::Type::STRING);

  EXPECT_EQ(m["age"], int64_t(87));
  EXPECT_EQ(m["weight"], 150.8);
  EXPECT_EQ(m["name"], std::string("John Doe"));
}

TEST_F(StructuredHeadersDecoderTest, TestDictionaryOneElt) {
  std::string input = "bagel=*YXZvY2Fkbw==*";
  StructuredHeadersDecoder shd(input);

  std::unordered_map<std::string, StructuredHeaderItem> m;
  shd.decodeDictionary(m);
  EXPECT_EQ(m.size(), 1);

  EXPECT_EQ(m["bagel"].tag, StructuredHeaderItem::Type::BINARYCONTENT);
  EXPECT_EQ(m["bagel"], std::string("avocado"));
}

TEST_F(StructuredHeadersDecoderTest, TestParamListOneElt) {
  std::string input = "abc_123;a=1;b=2";
  StructuredHeadersDecoder shd(input);

  ParameterisedList pl;
  shd.decodeParameterisedList(pl);
  EXPECT_EQ(pl.size(), 1);
  EXPECT_EQ(pl[0].identifier, "abc_123");
  EXPECT_EQ(pl[0].parameterMap.size(), 2);
  EXPECT_EQ(pl[0].parameterMap["a"], int64_t(1));
  EXPECT_EQ(pl[0].parameterMap["b"], int64_t(2));
}

TEST_F(StructuredHeadersDecoderTest, TestParamListManyElts) {
  std::string input = "a_13;a=1;b=2; c_4, ghi;q=\"9\";r=*bWF4IGlzIGF3ZXNvbWU=*";
  StructuredHeadersDecoder shd(input);

  ParameterisedList pl;
  shd.decodeParameterisedList(pl);
  EXPECT_EQ(pl.size(), 2);

  EXPECT_EQ(pl[0].identifier, "a_13");
  EXPECT_EQ(pl[0].parameterMap.size(), 3);
  EXPECT_EQ(pl[0].parameterMap["a"].tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(pl[0].parameterMap["b"].tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(pl[0].parameterMap["c_4"].tag, StructuredHeaderItem::Type::BOOLEAN);
  EXPECT_EQ(pl[0].parameterMap["c_4"], true);
  EXPECT_EQ(pl[0].parameterMap["a"], int64_t(1));
  EXPECT_EQ(pl[0].parameterMap["b"], int64_t(2));

  EXPECT_EQ(pl[1].identifier, "ghi");
  EXPECT_EQ(pl[1].parameterMap.size(), 2);
  EXPECT_EQ(pl[1].parameterMap["q"].tag, StructuredHeaderItem::Type::STRING);
  EXPECT_EQ(pl[1].parameterMap["r"].tag,
            StructuredHeaderItem::Type::BINARYCONTENT);
  EXPECT_EQ(pl[1].parameterMap["q"], std::string("9"));
  EXPECT_EQ(pl[1].parameterMap["r"], std::string("max is awesome"));
}

TEST_F(StructuredHeadersDecoderTest, TestParamListNoParams) {
  std::string input = "apple12, cat14, dog22";
  StructuredHeadersDecoder shd(input);

  ParameterisedList pl;
  shd.decodeParameterisedList(pl);
  EXPECT_EQ(pl.size(), 3);
  EXPECT_EQ(pl[0].identifier, "apple12");
  EXPECT_EQ(pl[0].parameterMap.size(), 0);

  EXPECT_EQ(pl[1].identifier, "cat14");
  EXPECT_EQ(pl[1].parameterMap.size(), 0);

  EXPECT_EQ(pl[2].identifier, "dog22");
  EXPECT_EQ(pl[2].parameterMap.size(), 0);
}

TEST_F(StructuredHeadersDecoderTest, TestParamListWhitespace) {
  std::string input = "am_95    ;    abc=11.8   ,    foo      ";
  StructuredHeadersDecoder shd(input);

  ParameterisedList pl;
  shd.decodeParameterisedList(pl);
  EXPECT_EQ(pl.size(), 2);

  EXPECT_EQ(pl[0].identifier, "am_95");
  EXPECT_EQ(pl[0].parameterMap.size(), 1);
  EXPECT_EQ(pl[0].parameterMap["abc"].tag, StructuredHeaderItem::Type::DOUBLE);
  EXPECT_EQ(pl[0].parameterMap["abc"], 11.8);

  EXPECT_EQ(pl[1].identifier, "foo");
  EXPECT_EQ(pl[1].parameterMap.size(), 0);
}

TEST_F(StructuredHeadersDecoderTest, TestParamListNullValues) {
  std::string input = "beverages;water;juice, food;pizza;burger";
  StructuredHeadersDecoder shd(input);

  ParameterisedList pl;
  shd.decodeParameterisedList(pl);
  EXPECT_EQ(pl.size(), 2);

  EXPECT_EQ(pl[0].identifier, "beverages");
  EXPECT_EQ(pl[0].parameterMap.size(), 2);
  EXPECT_EQ(pl[0].parameterMap["water"].tag,
            StructuredHeaderItem::Type::BOOLEAN);
  EXPECT_EQ(pl[0].parameterMap["water"], true);
  EXPECT_EQ(pl[0].parameterMap["juice"].tag,
            StructuredHeaderItem::Type::BOOLEAN);
  EXPECT_EQ(pl[0].parameterMap["juice"], true);

  EXPECT_EQ(pl[1].identifier, "food");
  EXPECT_EQ(pl[1].parameterMap.size(), 2);
  EXPECT_EQ(pl[1].parameterMap["pizza"].tag,
            StructuredHeaderItem::Type::BOOLEAN);
  EXPECT_EQ(pl[1].parameterMap["burger"].tag,
            StructuredHeaderItem::Type::BOOLEAN);
}

TEST_F(StructuredHeadersDecoderTest, PriorityWithIncremental) {
  StructuredHeaders::Dictionary dict;
  std::array<char, 6> input = {'u', '=', '5', ',', ' ', 'i'};
  StructuredHeadersDecoder decoder(
      folly::StringPiece(input.data(), input.size()));
  auto ret = decoder.decodeDictionary(dict);
  EXPECT_EQ(ret, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(2, dict.size());
  EXPECT_EQ(dict["u"].tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(dict["u"], (int64_t)5);
  EXPECT_EQ(dict["i"].tag, StructuredHeaderItem::Type::BOOLEAN);
  EXPECT_EQ(dict["i"], true);
}

TEST_F(StructuredHeadersDecoderTest, PriorityWithoutIncremental) {
  std::string priority = "u=5";
  StructuredHeadersDecoder decoder(priority);
  StructuredHeaders::Dictionary dict;
  auto ret = decoder.decodeDictionary(dict);
  EXPECT_EQ(ret, StructuredHeaders::DecodeError::OK);
  EXPECT_EQ(1, dict.size());
  EXPECT_EQ(dict["u"].tag, StructuredHeaderItem::Type::INT64);
  EXPECT_EQ(dict["u"], (int64_t)5);
  EXPECT_EQ(dict.find("i"), dict.end());
}

TEST_F(StructuredHeadersDecoderTest, PriorityWithoutIncrementalHasComma) {
  std::string priority = "u=5,";
  StructuredHeadersDecoder decoder(priority);
  StructuredHeaders::Dictionary dict;
  auto ret = decoder.decodeDictionary(dict);
  EXPECT_NE(ret, StructuredHeaders::DecodeError::OK);
}

TEST_F(StructuredHeadersDecoderTest, SpaceOnlyNoCrash) {
  StructuredHeaders::Dictionary dict;
  std::array<char, 1> input = {' '};
  StructuredHeadersDecoder decoder(
      folly::StringPiece(input.data(), input.size()));
  // NO crash:
  (void)decoder.decodeDictionary(dict);
}
} // namespace proxygen
