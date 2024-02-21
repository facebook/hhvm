/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <folly/json/json.h>

#include "mcrouter/lib/IOBufUtil.h"
#include "mcrouter/lib/carbon/CarbonMessageConversionUtils.h"
#include "mcrouter/lib/carbon/test/gen/CarbonTest.h"

TEST(CarbonMessageConversionUtils, toFollyDynamic_Complex) {
  carbon::test::TestRequest r;
  r.key_ref() = carbon::Keys<folly::IOBuf>("/test/key/");
  r.testBool_ref() = true;
  r.testChar_ref() = 'a';
  r.testInt8_ref() = -123;
  r.testInt16_ref() = -7890;
  r.testInt32_ref() = -123456789;
  r.testInt64_ref() = -9876543210123ll;
  r.testUInt8_ref() = 123;
  r.testUInt16_ref() = 7890;
  r.testUInt32_ref() = 123456789;
  r.testUInt64_ref() = 9876543210123ll;
  r.testFloat_ref() = 1.5;
  r.testDouble_ref() = 5.6;
  r.testShortString_ref() = "abcdef";
  r.testLongString_ref() =
      "asdfghjkl;'eqtirgwuifhiivlzkhbvjkhc3978y42h97*&687gba";
  r.testIobuf_ref() = folly::IOBuf(
      folly::IOBuf::CopyBufferOp(), folly::StringPiece("TestTheBuf"));
  r.testList_ref() = std::vector<std::string>({"abc", "bce", "xyz"});
  r.testNestedVec_ref() = {{1, 1, 1}, {2, 2, 2}};
  r.testUMap_ref() = std::unordered_map<std::string, std::string>(
      {{"key", "value"}, {"adele", "beyonce"}});
  r.testMap_ref() = std::map<double, double>({{3.14, 2.7}, {0.577, 0.2}});
  r.testF14FastMap_ref() = folly::F14FastMap<std::string, std::string>(
      {{"hello", "F14"}, {"Fast", "Map"}});
  r.testF14NodeMap_ref() = folly::F14NodeMap<std::string, std::string>(
      {{"hello", "F14"}, {"Node", "Map"}});
  r.testF14ValueMap_ref() = folly::F14ValueMap<std::string, std::string>(
      {{"hello", "F14"}, {"Value", "Map"}});
  r.testF14VectorMap_ref() = folly::F14VectorMap<std::string, std::string>(
      {{"hello", "F14"}, {"Vector", "Map"}});
  r.testComplexMap_ref() = std::map<std::string, std::vector<uint16_t>>(
      {{"hello", {1, 1, 1}}, {"world", {2, 2, 2}}});
  r.testUSet_ref() = std::unordered_set<std::string>({"hello", "world"});
  r.testSet_ref() = std::set<uint64_t>({123, 456});
  r.testF14FastSet_ref() =
      folly::F14FastSet<std::string>({"hello", "F14FastSet"});
  r.testF14NodeSet_ref() =
      folly::F14NodeSet<std::string>({"hello", "F14NodeSet"});
  r.testF14ValueSet_ref() =
      folly::F14ValueSet<std::string>({"hello", "F14ValueSet"});
  r.testF14VectorSet_ref() =
      folly::F14VectorSet<std::string>({"hello", "F14VectorSet"});
  r.testIOBufList_ref() =
      std::vector<folly::IOBuf>({folly::IOBuf(), folly::IOBuf()});

  folly::dynamic expected =
      folly::dynamic::object(
          "dummy2", false)("dummy22", false)("dummy23", false)("dummy31", "")("key", "/test/key/")("testBool", true)("testChar", 97)("testInt8", -123)("testInt16", -7890)("testInt32", -123456789)("testInt64", -9876543210123ll)("testUInt8", 123)("testUInt16", 7890)("testUInt32", 123456789)("testUInt64", 9876543210123ll)("testFloat", 1.5)("testDouble", 5.6)("testShortString", "abcdef")("testLongString", "asdfghjkl;'eqtirgwuifhiivlzkhbvjkhc3978y42h97*&687gba")("testIobuf", "TestTheBuf")("testList", folly::dynamic::array("abc", "bce", "xyz"))("testNestedVec", folly::dynamic::array(folly::dynamic::array(1, 1, 1), folly::dynamic::array(2, 2, 2)))("testUMap", folly::dynamic::object("key", "value")("adele", "beyonce"))("testMap", folly::dynamic::object("3.14", 2.7)("0.577", 0.2))("testF14FastMap", folly::dynamic::object("Fast", "Map")("hello", "F14"))("testF14NodeMap", folly::dynamic::object("Node", "Map")("hello", "F14"))("testF14ValueMap", folly::dynamic::object("Value", "Map")("hello", "F14"))("testF14VectorMap", folly::dynamic::object("Vector", "Map")("hello", "F14"))("testComplexMap", folly::dynamic::object("hello", folly::dynamic::array(1, 1, 1))("world", folly::dynamic::array(2, 2, 2)))("testUSet", folly::dynamic::array("hello", "world"))("testF14FastSet", folly::dynamic::array("F14FastSet", "hello"))("testF14NodeSet", folly::dynamic::array("F14NodeSet", "hello"))("testF14ValueSet", folly::dynamic::array("F14ValueSet", "hello"))("testF14VectorSet", folly::dynamic::array("F14VectorSet", "hello"))("testSet", folly::dynamic::array(123, 456))("testIOBufList", folly::dynamic::array("", ""));

  auto dynamic = carbon::convertToFollyDynamic(r);

  auto set = dynamic.at("testUSet");
  std::sort(set.begin(), set.end());
  dynamic.at("testUSet") = set;

  auto fastset = dynamic.at("testF14FastSet");
  std::sort(fastset.begin(), fastset.end());
  dynamic.at("testF14FastSet") = fastset;

  auto nodeset = dynamic.at("testF14NodeSet");
  std::sort(nodeset.begin(), nodeset.end());
  dynamic.at("testF14NodeSet") = nodeset;

  auto valueset = dynamic.at("testF14ValueSet");
  std::sort(valueset.begin(), valueset.end());
  dynamic.at("testF14ValueSet") = valueset;

  EXPECT_EQ(expected, dynamic);
}

TEST(CarbonMessageConversionUtils, fromFollyDynamic_Complex) {
  const std::string jsonStr = R"json(
    {
      "key": "sampleKey",

      "testBool": true,
      "testChar": 9,

      "testInt8": -8,
      "testInt16": -16,
      "testInt32": -32,
      "testInt64": -64,
      "testUInt8": 8,
      "testUInt16": 16,
      "testUInt32": 32,
      "testUInt64": 64,

      "testFloat": 12.356,
      "testDouble": 35.98765,

      "testLongString": "this is a very long and nice string in a json file 12",
      "testIobuf": "iobuf string here...",

      "testList": [
        "string 1",
        "s2"
      ],

      "testOptionalString": "I exist!",

      "testNestedVec": [
        [ 17, 26 ],
        [],
        [ 32 ]
      ],

      "testF14FastSet": [
        "hello",
        "F14FastSet"
      ],
      "testF14NodeSet": [
        "hello",
        "F14NodeSet"
      ],
      "testF14ValueSet": [
        "hello",
        "F14ValueSet"
      ],
      "testF14VectorSet": [
        "hello",
        "F14VectorSet"
      ],

      "testMap": {
        10.7: 11.8,
        30.567: 31.789
      },
      "testF14FastMap": {
        "hello": "F14",
        "Fast": "Map"
      },
      "testF14NodeMap": {
        "hello": "F14",
        "Node": "Map"
      },
      "testF14ValueMap": {
        "hello": "F14",
        "Value": "Map"
      },
      "testF14VectorMap": {
        "hello": "F14",
        "Vector": "Map"
      },

      "testComplexMap": {
        "v1": [ 10 ],
        "ve2": [ 20, 30 ],
        "vec03": [ 50, 70, 90 ]
      }
    }
  )json";

  size_t numErrors = 0;
  auto onError = [&numErrors](folly::StringPiece name, folly::StringPiece msg) {
    std::cerr << "ERROR: " << name << ": " << msg << std::endl;
    numErrors++;
  };

  folly::json::serialization_opts jsonOpts;
  jsonOpts.allow_non_string_keys = true;
  folly::dynamic json = folly::parseJson(jsonStr, jsonOpts);

  carbon::test::TestRequest r;
  carbon::convertFromFollyDynamic(json, r, std::move(onError));

  EXPECT_EQ(0, numErrors);

  EXPECT_EQ("sampleKey", r.key_ref()->fullKey());

  EXPECT_EQ(-8, *r.testInt8_ref());
  EXPECT_EQ(-16, *r.testInt16_ref());
  EXPECT_EQ(-32, *r.testInt32_ref());
  EXPECT_EQ(-64, *r.testInt64_ref());
  EXPECT_EQ(8, *r.testUInt8_ref());
  EXPECT_EQ(16, *r.testUInt16_ref());
  EXPECT_EQ(32, *r.testUInt32_ref());
  EXPECT_EQ(64, *r.testUInt64_ref());

  EXPECT_FLOAT_EQ(12.356, *r.testFloat_ref());
  EXPECT_DOUBLE_EQ(35.98765, *r.testDouble_ref());

  EXPECT_EQ(
      "this is a very long and nice string in a json file 12",
      *r.testLongString_ref());
  const folly::IOBuf expectedIobuf(
      folly::IOBuf::CopyBufferOp(), folly::StringPiece("iobuf string here..."));
  EXPECT_TRUE(folly::IOBufEqualTo()(expectedIobuf, *r.testIobuf_ref()));

  ASSERT_EQ(2, r.testList_ref()->size());
  EXPECT_EQ("string 1", r.testList_ref()[0]);
  EXPECT_EQ("s2", r.testList_ref()[1]);

  ASSERT_TRUE(r.testOptionalString_ref().has_value());
  EXPECT_EQ("I exist!", r.testOptionalString_ref().value());

  ASSERT_EQ(3, r.testNestedVec_ref()->size());
  ASSERT_EQ(2, r.testNestedVec_ref()[0].size());
  EXPECT_EQ(0, r.testNestedVec_ref()[1].size());
  ASSERT_EQ(1, r.testNestedVec_ref()[2].size());
  EXPECT_EQ(17, r.testNestedVec_ref()[0][0]);
  EXPECT_EQ(26, r.testNestedVec_ref()[0][1]);
  EXPECT_EQ(32, r.testNestedVec_ref()[2][0]);

  ASSERT_EQ(2, r.testF14FastSet_ref()->size());
  EXPECT_NE(
      r.testF14FastSet_ref()->find("hello"), r.testF14FastSet_ref()->end());
  EXPECT_NE(
      r.testF14FastSet_ref()->find("F14FastSet"),
      r.testF14FastSet_ref()->end());

  ASSERT_EQ(2, r.testF14NodeSet_ref()->size());
  EXPECT_NE(
      r.testF14NodeSet_ref()->find("hello"), r.testF14NodeSet_ref()->end());
  EXPECT_NE(
      r.testF14NodeSet_ref()->find("F14NodeSet"),
      r.testF14NodeSet_ref()->end());

  ASSERT_EQ(2, r.testF14ValueSet_ref()->size());
  EXPECT_NE(
      r.testF14ValueSet_ref()->find("hello"), r.testF14ValueSet_ref()->end());
  EXPECT_NE(
      r.testF14ValueSet_ref()->find("F14ValueSet"),
      r.testF14ValueSet_ref()->end());

  ASSERT_EQ(2, r.testF14VectorSet_ref()->size());
  EXPECT_NE(
      r.testF14VectorSet_ref()->find("hello"), r.testF14VectorSet_ref()->end());
  EXPECT_NE(
      r.testF14VectorSet_ref()->find("F14VectorSet"),
      r.testF14VectorSet_ref()->end());

  ASSERT_EQ(2, r.testMap_ref()->size());
  EXPECT_EQ(11.8, r.testMap_ref()[10.7]);
  EXPECT_EQ(31.789, r.testMap_ref()[30.567]);

  ASSERT_EQ(2, r.testF14FastMap_ref()->size());
  EXPECT_EQ("F14", r.testF14FastMap_ref()["hello"]);
  EXPECT_EQ("Map", r.testF14FastMap_ref()["Fast"]);

  ASSERT_EQ(2, r.testF14NodeMap_ref()->size());
  EXPECT_EQ("F14", r.testF14NodeMap_ref()["hello"]);
  EXPECT_EQ("Map", r.testF14NodeMap_ref()["Node"]);

  ASSERT_EQ(2, r.testF14ValueMap_ref()->size());
  EXPECT_EQ("F14", r.testF14ValueMap_ref()["hello"]);
  EXPECT_EQ("Map", r.testF14ValueMap_ref()["Value"]);

  ASSERT_EQ(2, r.testF14VectorMap_ref()->size());
  EXPECT_EQ("F14", r.testF14VectorMap_ref()["hello"]);
  EXPECT_EQ("Map", r.testF14VectorMap_ref()["Vector"]);

  ASSERT_EQ(3, r.testComplexMap_ref()->size());
  ASSERT_EQ(1, r.testComplexMap_ref()["v1"].size());
  EXPECT_EQ(10, r.testComplexMap_ref()["v1"][0]);
  ASSERT_EQ(2, r.testComplexMap_ref()["ve2"].size());
  EXPECT_EQ(20, r.testComplexMap_ref()["ve2"][0]);
  EXPECT_EQ(30, r.testComplexMap_ref()["ve2"][1]);
  ASSERT_EQ(3, r.testComplexMap_ref()["vec03"].size());
  EXPECT_EQ(50, r.testComplexMap_ref()["vec03"][0]);
  EXPECT_EQ(70, r.testComplexMap_ref()["vec03"][1]);
  EXPECT_EQ(90, r.testComplexMap_ref()["vec03"][2]);
}

TEST(CarbonMessageConversionUtils, fromFollyDynamic_Errors) {
  const std::string jsonStr = R"json(
    {
      "key": 75,

      "int32Member": "abc",

      "testChar": "ab",

      "testList": [
        "string 1",
        7
      ],

      "testNestedVec": [
        [],
        [ 18, "abc" ]
      ]
    }
  )json";

  size_t numErrors = 0;
  auto onError = [&numErrors](
                     folly::StringPiece fieldName, folly::StringPiece msg) {
    numErrors++;
    std::cerr << fieldName << ": " << msg << std::endl;
  };

  carbon::test::TestRequest r;
  carbon::convertFromFollyDynamic(
      folly::parseJson(jsonStr), r, std::move(onError));

  EXPECT_EQ(4, numErrors);

  ASSERT_EQ(1, r.testList_ref()->size());
  EXPECT_EQ("string 1", r.testList_ref()[0]);

  ASSERT_EQ(2, r.testNestedVec_ref()->size());
  EXPECT_EQ(0, r.testNestedVec_ref()[0].size());
  ASSERT_EQ(1, r.testNestedVec_ref()[1].size());
  EXPECT_EQ(18, r.testNestedVec_ref()[1][0]);
}
