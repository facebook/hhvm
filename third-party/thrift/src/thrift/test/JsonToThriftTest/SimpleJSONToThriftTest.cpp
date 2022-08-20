/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <fstream>
#include <iostream>
#include <limits>
#include <memory>

#include <folly/portability/GTest.h>

#include <folly/FileUtil.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

#include <thrift/test/JsonToThriftTest/gen-cpp2/myBinaryStruct_types.h>
#include <thrift/test/JsonToThriftTest/gen-cpp2/myBoolStruct_types.h>
#include <thrift/test/JsonToThriftTest/gen-cpp2/myByteStruct_types.h>
#include <thrift/test/JsonToThriftTest/gen-cpp2/myCombinedStructs_types.h>
#include <thrift/test/JsonToThriftTest/gen-cpp2/myComplexStruct_types.h>
#include <thrift/test/JsonToThriftTest/gen-cpp2/myDoubleListStruct_types.h>
#include <thrift/test/JsonToThriftTest/gen-cpp2/myDoubleStruct_types.h>
#include <thrift/test/JsonToThriftTest/gen-cpp2/myEmptyStruct_types.h>
#include <thrift/test/JsonToThriftTest/gen-cpp2/myI16Struct_types.h>
#include <thrift/test/JsonToThriftTest/gen-cpp2/myI32Struct_types.h>
#include <thrift/test/JsonToThriftTest/gen-cpp2/myKeyStruct_types.h>
#include <thrift/test/JsonToThriftTest/gen-cpp2/myMapStruct_types.h>
#include <thrift/test/JsonToThriftTest/gen-cpp2/myMixedStruct_types.h>
#include <thrift/test/JsonToThriftTest/gen-cpp2/mySetStruct_types.h>
#include <thrift/test/JsonToThriftTest/gen-cpp2/myStringStruct_types.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::test;

template <typename T>
static std::string serializeJSON(T dataStruct, string fileName = string()) {
  auto ret = SimpleJSONSerializer::serialize<string>(dataStruct);

  if (!fileName.empty()) {
    folly::writeFile(ret, (fileName + ".json").c_str());
  }

  return ret;
}

template <typename T>
static void deserializeJSON(T& dataStruct, const string& json) {
  LOG(INFO) << json;
  size_t numRead = 0;
  dataStruct = SimpleJSONSerializer::deserialize<T>(json, &numRead);
  EXPECT_EQ(numRead, json.length());
}

template <typename T>
static void testSimpleJSON(T dataStruct) {
  std::string simpleJsonText = serializeJSON(dataStruct);

  T parsedStruct;
  deserializeJSON(parsedStruct, simpleJsonText);

  EXPECT_TRUE(parsedStruct == dataStruct);
}

TEST(SimpleJSONToThriftTest, SimpleJSON_ComplexSerialization) {
  myComplexStruct thriftComplexObj;
  myMixedStruct thriftMixedObj;

  mySimpleStruct thriftSimpleObj;
  *thriftSimpleObj.a_ref() = true;
  *thriftSimpleObj.b_ref() = 120;
  *thriftSimpleObj.c_ref() = 9990;
  *thriftSimpleObj.d_ref() = -9990;
  *thriftSimpleObj.e_ref() = -1;
  *thriftSimpleObj.f_ref() = 0.9;
  *thriftSimpleObj.g_ref() = "Simple String";

  mySuperSimpleStruct superSimple;
  *superSimple.a_ref() = 121;
  thriftMixedObj.a_ref()->push_back(18);
  thriftMixedObj.b_ref()->push_back(superSimple);
  thriftMixedObj.c_ref()->insert(std::make_pair("flame", -8));
  thriftMixedObj.c_ref()->insert(std::make_pair("fire", -191));
  thriftMixedObj.d_ref()->insert(std::make_pair("key1", superSimple));
  thriftMixedObj.e_ref()->insert(88);
  thriftMixedObj.e_ref()->insert(89);

  *thriftComplexObj.a_ref() = thriftSimpleObj;
  thriftComplexObj.b_ref()->push_back(25);
  thriftComplexObj.b_ref()->push_back(24);

  for (int i = 0; i < 3; i++) {
    mySimpleStruct obj;
    *obj.a_ref() = true;
    *obj.b_ref() = 80 + i;
    *obj.c_ref() = 7000 + i;
    *obj.e_ref() = -i;
    *obj.f_ref() = -0.5 * i;
    string elmName = "element" + folly::to<std::string>(i + 1);
    *obj.g_ref() = elmName.c_str();
    thriftComplexObj.c_ref()->insert(std::make_pair(elmName, thriftSimpleObj));
  }

  *thriftComplexObj.e_ref() = EnumTest::EnumTwo;

  testSimpleJSON(thriftMixedObj);
  testSimpleJSON(thriftComplexObj);
}

TEST(SimpleJSONToThriftTest, SimpleJSON_BasicSerialization) {
  mySimpleStruct thriftSimpleObj;
  myDoubleStruct thriftDoubleObj;
  myBoolStruct thriftBoolObj1, thriftBoolObj2;
  myByteStruct thriftByteObj;
  myStringStruct thriftStringObj;
  myI16Struct thriftI16Obj;
  myI32Struct thriftI32Obj;

  *thriftSimpleObj.a_ref() = false;
  *thriftSimpleObj.b_ref() = 87;
  *thriftSimpleObj.c_ref() = 7880;
  *thriftSimpleObj.d_ref() = -7880;
  *thriftSimpleObj.e_ref() = -1;
  *thriftSimpleObj.f_ref() = -0.1;
  *thriftSimpleObj.g_ref() = "T-bone";

  *thriftDoubleObj.a_ref() = 100.5;
  testSimpleJSON(thriftDoubleObj);
  *thriftDoubleObj.a_ref() = numeric_limits<double>::infinity();
  testSimpleJSON(thriftDoubleObj);
  *thriftDoubleObj.a_ref() = -numeric_limits<double>::infinity();
  testSimpleJSON(thriftDoubleObj);

  *thriftBoolObj1.a_ref() = true;
  *thriftBoolObj2.a_ref() = false;

  *thriftByteObj.a_ref() = 115;
  *thriftStringObj.a_ref() = "testing";

  *thriftI16Obj.a_ref() = 4567;
  *thriftI32Obj.a_ref() = 12131415;

  testSimpleJSON(thriftSimpleObj);
  testSimpleJSON(thriftBoolObj1);
  testSimpleJSON(thriftBoolObj2);
  testSimpleJSON(thriftByteObj);
  testSimpleJSON(thriftStringObj);
  testSimpleJSON(thriftI16Obj);
  testSimpleJSON(thriftI32Obj);
}

TEST(SimpleJSONToThriftTest, SimpleJSON_BasicSerializationNan) {
  myDoubleListStruct obj;
  std::vector<double> array = {NAN, -NAN, 0.3333333333};
  *obj.l_ref() = array;

  auto jsonString = serializeJSON(obj);
  myDoubleListStruct parsedStruct;
  deserializeJSON(parsedStruct, jsonString);

  EXPECT_EQ(obj.l_ref()->size(), parsedStruct.l_ref()->size());
  for (size_t i = 0; i < obj.l_ref()->size(); ++i) {
    if (std::isnan(obj.l_ref()[i]) == std::isnan(parsedStruct.l_ref()[i])) {
      continue;
    }
    EXPECT_EQ(obj.l_ref()[i], parsedStruct.l_ref()[i]);
  }

  auto jsonString2 = serializeJSON(parsedStruct);

  // this checks that nan and -nan still have correct '-' information
  EXPECT_EQ(jsonString, jsonString2);
}

TEST(SimpleJSONToThriftTest, SimpleStructMissingNonRequiredField) {
  // jsonSimpleT1 is to test whether __isset is set properly, given
  // that all the required field has value: field a's value is missing
  string jsonSimpleT(
      "{\"c\":16,\"d\":32,\"e\":64,\"b\":8,"
      "\"f\":0.99,\"g\":\"Hello\"}");
  mySimpleStruct thriftSimpleObj;

  deserializeJSON(thriftSimpleObj, jsonSimpleT);

  EXPECT_TRUE(!thriftSimpleObj.a_ref().has_value());
  EXPECT_EQ(*thriftSimpleObj.b_ref(), 8);
  EXPECT_TRUE(thriftSimpleObj.b_ref().has_value());
  // field c doesn't have __isset field, since it is required.
  EXPECT_EQ(*thriftSimpleObj.c_ref(), 16);
  EXPECT_EQ(*thriftSimpleObj.d_ref(), 32);
  EXPECT_TRUE(thriftSimpleObj.d_ref().has_value());
  EXPECT_EQ(*thriftSimpleObj.e_ref(), 64);
  EXPECT_TRUE(thriftSimpleObj.e_ref().has_value());
  EXPECT_EQ(*thriftSimpleObj.f_ref(), 0.99);
  EXPECT_TRUE(thriftSimpleObj.f_ref().has_value());
  EXPECT_EQ(*thriftSimpleObj.g_ref(), "Hello");
  EXPECT_TRUE(thriftSimpleObj.g_ref().has_value());
}

TEST(SimpleJSONToThriftTest, NegativeBoundaryCase) {
  string jsonByteTW("{\"a\":-129}");
  myByteStruct thriftByteObjW;
  EXPECT_THROW(
      deserializeJSON(thriftByteObjW, jsonByteTW), apache::thrift::TException);

  string jsonByteT("{\"a\":-128}");
  myByteStruct thriftByteObj;
  deserializeJSON(thriftByteObj, jsonByteT);
  EXPECT_EQ(*thriftByteObj.a_ref(), -128);

  string jsonI16TW("{\"a\":-32769}");
  myI16Struct thriftI16ObjW;
  EXPECT_THROW(
      deserializeJSON(thriftI16ObjW, jsonI16TW), apache::thrift::TException);

  string jsonI16T("{\"a\":-32768}");
  myI16Struct thriftI16Obj;
  deserializeJSON(thriftI16Obj, jsonI16T);
  EXPECT_EQ(*thriftI16Obj.a_ref(), -32768);

  string jsonI32TW("{\"a\":-2147483649}");
  myI32Struct thriftI32ObjW;
  try {
    deserializeJSON(thriftI32ObjW, jsonI32TW);
    cout << serializeJSON(thriftI32ObjW) << endl;
    ADD_FAILURE();
  } catch (apache::thrift::TException&) {
  }

  string jsonI32T("{\"a\":-2147483648}");
  myI32Struct thriftI32Obj;
  deserializeJSON(thriftI32Obj, jsonI32T);
  EXPECT_EQ(*thriftI32Obj.a_ref(), -2147483648);
}

TEST(SimpleJSONToThriftTest, PassingWrongType) {
  string jsonI32T("{\"a\":\"hello\"}");
  myI32Struct thriftI32Obj;
  EXPECT_THROW(deserializeJSON(thriftI32Obj, jsonI32T), std::exception);
}

TEST(SimpleJSONToThriftTest, Whitespace) {
  // tests if \n \r \t and space are ignored properly
  string jsonSimpleT(
      "\n\r\t {\n\r\t \"c\"\n\r\t :\n\r\t 16,\"d\":32"
      ",\"e\":64\t "
      ", \n\r\t\"b\":\r\t\n 8"
      ",\"f\": \n\r\t0.99\r"
      ",\r\"g\" :  \"Hello\"\n\r\t "
      "}");

  mySimpleStruct thriftSimpleObj;
  deserializeJSON(thriftSimpleObj, jsonSimpleT);

  string jsonComplexT(
      "{\"a\":" + jsonSimpleT +
      ","
      "\"b\":\t\n\r [\n\t\r 3,2,1\r\t \n] \t\n\r,"
      "\"c\":\n\r\t { \t\n\r \"key1\":" +
      jsonSimpleT +
      "  ,     \"key2\": {\"c\":20,"
      "\"d\":320,\"f\":0.001}}\r\r\t\t\n\n   \n}");
  myComplexStruct thriftComplexObj;

  deserializeJSON(thriftComplexObj, jsonComplexT);

  EXPECT_TRUE(!thriftComplexObj.a_ref()->a_ref().has_value());
  EXPECT_EQ(*thriftComplexObj.a_ref()->b_ref(), 8);
  EXPECT_TRUE(thriftComplexObj.a_ref()->b_ref().has_value());
  // field c doesn't have __isset field, since it is required.
  EXPECT_EQ(*thriftComplexObj.a_ref()->c_ref(), 16);
  EXPECT_EQ(*thriftComplexObj.a_ref()->d_ref(), 32);
  EXPECT_TRUE(thriftComplexObj.a_ref()->d_ref().has_value());
  EXPECT_EQ(*thriftComplexObj.a_ref()->e_ref(), 64);
  EXPECT_TRUE(thriftComplexObj.a_ref()->e_ref().has_value());
  EXPECT_EQ(*thriftComplexObj.a_ref()->f_ref(), 0.99);
  EXPECT_TRUE(thriftComplexObj.a_ref()->f_ref().has_value());
  EXPECT_EQ(*thriftComplexObj.a_ref()->g_ref(), "Hello");
  EXPECT_TRUE(thriftComplexObj.a_ref()->g_ref().has_value());

  EXPECT_EQ(thriftComplexObj.b_ref()[0], 3);
  EXPECT_EQ(thriftComplexObj.b_ref()[1], 2);
  EXPECT_EQ(thriftComplexObj.b_ref()[2], 1);

  EXPECT_TRUE(!thriftComplexObj.c_ref()["key1"].a_ref().has_value());
  EXPECT_EQ(*thriftComplexObj.c_ref()["key1"].b_ref(), 8);
  EXPECT_TRUE(thriftComplexObj.c_ref()["key1"].b_ref().has_value());
  // field c doesn't have __isset field, since it is required.
  EXPECT_EQ(*thriftComplexObj.c_ref()["key1"].c_ref(), 16);
  EXPECT_EQ(*thriftComplexObj.c_ref()["key1"].d_ref(), 32);
  EXPECT_TRUE(thriftComplexObj.c_ref()["key1"].d_ref().has_value());
  EXPECT_EQ(*thriftComplexObj.c_ref()["key1"].e_ref(), 64);
  EXPECT_TRUE(thriftComplexObj.c_ref()["key1"].e_ref().has_value());
  EXPECT_EQ(*thriftComplexObj.c_ref()["key1"].f_ref(), 0.99);
  EXPECT_TRUE(thriftComplexObj.c_ref()["key1"].f_ref().has_value());
  EXPECT_EQ(*thriftComplexObj.c_ref()["key1"].g_ref(), "Hello");
  EXPECT_TRUE(thriftComplexObj.c_ref()["key1"].g_ref().has_value());

  EXPECT_EQ(*thriftComplexObj.c_ref()["key2"].c_ref(), 20);
  EXPECT_EQ(*thriftComplexObj.c_ref()["key2"].d_ref(), 320);
  EXPECT_EQ(*thriftComplexObj.c_ref()["key2"].f_ref(), 0.001);
}
// fields in JSON that are not present in the thrift type spec
TEST(SimpleJSONToThriftTest, MissingField) {
  string jsonSimpleT(
      "{\"c\":16,\"d\":32,\"e\":64,\"b\":8,"
      "\"f\":0.99,\"g\":\"Hello\",\"extra\":12}");
  mySimpleStruct thriftSimpleObj;
  deserializeJSON(thriftSimpleObj, jsonSimpleT);

  EXPECT_TRUE(!thriftSimpleObj.a_ref().has_value());
  EXPECT_EQ(*thriftSimpleObj.b_ref(), 8);
  EXPECT_TRUE(thriftSimpleObj.b_ref().has_value());
  // field c doesn't have __isset field, since it is required.
  EXPECT_EQ(*thriftSimpleObj.c_ref(), 16);
  EXPECT_EQ(*thriftSimpleObj.d_ref(), 32);
  EXPECT_TRUE(thriftSimpleObj.d_ref().has_value());
  EXPECT_EQ(*thriftSimpleObj.e_ref(), 64);
  EXPECT_TRUE(thriftSimpleObj.e_ref().has_value());
  EXPECT_EQ(*thriftSimpleObj.f_ref(), 0.99);
  EXPECT_TRUE(thriftSimpleObj.f_ref().has_value());
  EXPECT_EQ(*thriftSimpleObj.g_ref(), "Hello");
  EXPECT_TRUE(thriftSimpleObj.g_ref().has_value());

  // checks that a map is skipped properly
  string jsonEmptyListT("{\"e\":[1, 0.13]}");
  myEmptyStruct thriftEmptyListObj;
  deserializeJSON(thriftEmptyListObj, jsonEmptyListT);

  // checks that a map is skipped properly
  string jsonEmptyMapT("{\"m\":{\"1\":2, \"3\":13}}");
  myEmptyStruct thriftEmptyMapObj;
  deserializeJSON(thriftEmptyMapObj, jsonEmptyMapT);

  // checks that all fields are skipped properly
  string jsonEmptyT(
      "{\"a\": 1,\"b\":-0.1,\"c\":false,\"d\": true"
      ",\"e\":[ 0.3,1],\"f\":{ \"g\":\"abc\",\"h\":\"def\"}"
      ",\"i\":[[ ],[]],\"j\":{}}");
  myEmptyStruct thriftEmptyObj;
  deserializeJSON(thriftEmptyObj, jsonEmptyT);

  // checks that all fields are skipped properly
  string jsonNestedT(
      "{\"a\":" + jsonEmptyT + ",\"b\":[" + jsonEmptyT + "," + jsonEmptyT +
      "]"
      ",\"c\":-123}");
  myNestedEmptyStruct thriftNestedObj;
  deserializeJSON(thriftNestedObj, jsonNestedT);
  EXPECT_TRUE(thriftNestedObj.a_ref().has_value());
  EXPECT_TRUE(thriftNestedObj.b_ref().has_value());
  EXPECT_TRUE(thriftNestedObj.c_ref().has_value());
  EXPECT_EQ(*thriftNestedObj.c_ref(), -123);
}

TEST(SimpleJSONToThriftTest, BoundaryCase) {
  // jsonSimpleT2 is to test that the generated code does NOT throw exeption
  // if the required field doesn't have value, since required is being
  // deprecated.
  string jsonSimpleT(
      "{\"a\":true,\"d\":32,\"e\":64,\"b\":8,"
      "\"f\":0.99,\"g\":\"Hello\"}");
  mySimpleStruct thriftSimpleObj;
  deserializeJSON(thriftSimpleObj, jsonSimpleT);

  string jsonByteTW("{\"a\":128}");
  myByteStruct thriftByteObjW;
  EXPECT_THROW(
      deserializeJSON(thriftByteObjW, jsonByteTW), apache::thrift::TException);

  string jsonByteT("{\"a\":127}");
  myByteStruct thriftByteObj;
  deserializeJSON(thriftByteObj, jsonByteT);
  EXPECT_EQ(*thriftByteObj.a_ref(), 127);

  string jsonI16TW("{\"a\":32768}");
  myI16Struct thriftI16ObjW;
  EXPECT_THROW(
      deserializeJSON(thriftI16ObjW, jsonI16TW), apache::thrift::TException);

  string jsonI16T("{\"a\":32767}");
  myI16Struct thriftI16Obj;
  deserializeJSON(thriftI16Obj, jsonI16T);
  EXPECT_EQ(*thriftI16Obj.a_ref(), 32767);

  string jsonI32TW("{\"a\":2147483648}");
  myI32Struct thriftI32ObjW;
  EXPECT_THROW(
      deserializeJSON(thriftI32ObjW, jsonI32TW), apache::thrift::TException);

  string jsonI32T("{\"a\":2147483647}");
  myI32Struct thriftI32Obj;
  deserializeJSON(thriftI32Obj, jsonI32T);
  EXPECT_EQ(*thriftI32Obj.a_ref(), 2147483647);

  string jsonBoolTW("{\"a\":2}");
  myBoolStruct thriftBoolObjW;
  EXPECT_THROW(deserializeJSON(thriftBoolObjW, jsonBoolTW), std::exception);
}

TEST(SimpleJSONToThriftTest, DoubleExponents) {
  string jsonDouble("{\"a\":21.47483647e9}");
  myDoubleStruct thriftDoubleObj;
  deserializeJSON(thriftDoubleObj, jsonDouble);
  EXPECT_EQ(*thriftDoubleObj.a_ref(), 21.47483647e9);
}

TEST(SimpleJSONToThriftTest, ComplexTypeMissingRequiredFieldInMember) {
  string jsonT(
      "{\"a\":true,\"c\":16,\"d\":32,\"e\":64,\"b\":8,"
      "\"f\":0.99,\"g\":\"Hello\"}");
  string jsonComplexT(
      "{\"a\":" + jsonT + ",\"b\":[3,2,1],\"c\":{\"key1\":" + jsonT +
      ",\"key2\":{\"d\":320,\"f\":0.001}}}");

  myComplexStruct thriftComplexObj;
  try {
    deserializeJSON(thriftComplexObj, jsonComplexT);
  } catch (apache::thrift::TException&) {
    // We don't enforce required anymore, thus shouldn't throw.
    ADD_FAILURE();
  }
}

TEST(SimpleJSONToThriftTest, ComplexTypeTest) {
  string jsonT(
      "{\"a\":true,\"c\":16,\"d\":32,\"e\":64,\"b\":8,"
      "\"f\":0.99,\"g\":\"Hello\"}");
  string jsonComplexT(
      "{\"a\":" + jsonT + ",\"b\":[3,2,1],\"c\":{\"key1\":" + jsonT +
      ",\"key2\":{\"c\":20, \"d\":320,\"f\":0.001}}}");

  myComplexStruct thriftComplexObj;
  deserializeJSON(thriftComplexObj, jsonComplexT);

  EXPECT_EQ(*thriftComplexObj.a_ref()->b_ref(), 8);
  EXPECT_EQ(*thriftComplexObj.a_ref()->c_ref(), 16);
  EXPECT_EQ(*thriftComplexObj.a_ref()->d_ref(), 32);
  EXPECT_EQ(*thriftComplexObj.a_ref()->e_ref(), 64);
  EXPECT_EQ(*thriftComplexObj.a_ref()->f_ref(), 0.99);
  EXPECT_EQ(*thriftComplexObj.a_ref()->g_ref(), "Hello");

  EXPECT_EQ(thriftComplexObj.b_ref()[0], 3);
  EXPECT_EQ(thriftComplexObj.b_ref()[1], 2);
  EXPECT_EQ(thriftComplexObj.b_ref()[2], 1);

  EXPECT_EQ(*thriftComplexObj.c_ref()["key1"].b_ref(), 8);
  EXPECT_EQ(*thriftComplexObj.c_ref()["key1"].c_ref(), 16);
  EXPECT_EQ(*thriftComplexObj.c_ref()["key1"].d_ref(), 32);
  EXPECT_EQ(*thriftComplexObj.c_ref()["key1"].e_ref(), 64);
  EXPECT_EQ(*thriftComplexObj.c_ref()["key1"].f_ref(), 0.99);
  EXPECT_EQ(*thriftComplexObj.c_ref()["key1"].g_ref(), "Hello");
  EXPECT_EQ(*thriftComplexObj.c_ref()["key2"].c_ref(), 20);
  EXPECT_EQ(*thriftComplexObj.c_ref()["key2"].d_ref(), 320);
  EXPECT_EQ(*thriftComplexObj.c_ref()["key2"].f_ref(), 0.001);
}

TEST(SimpleJSONToThriftTest, SetTypeTest) {
  string jsonT("{\"a\":[1,2,3]}");
  mySetStruct thriftSetObj;
  deserializeJSON(thriftSetObj, jsonT);
  EXPECT_TRUE(thriftSetObj.a_ref().has_value());
  EXPECT_EQ(thriftSetObj.a_ref()->size(), 3);
  EXPECT_TRUE(thriftSetObj.a_ref()->find(2) != thriftSetObj.a_ref()->end());
  EXPECT_TRUE(thriftSetObj.a_ref()->find(5) == thriftSetObj.a_ref()->end());
}

TEST(SimpleJSONToThriftTest, MixedStructTest) {
  string jsonT(
      "{\"a\":[1],\"b\":[{\"a\":1}],\"c\":{\"hello\":1},"
      "\"d\":{\"hello\":{\"a\":1}},\"e\":[1]}");
  myMixedStruct thriftMixedObj;
  deserializeJSON(thriftMixedObj, jsonT);
  EXPECT_EQ(thriftMixedObj.a_ref()[0], 1);
  EXPECT_EQ(*thriftMixedObj.b_ref()[0].a_ref(), 1);
  EXPECT_EQ(thriftMixedObj.c_ref()["hello"], 1);
  EXPECT_EQ(*thriftMixedObj.d_ref()["hello"].a_ref(), 1);
  EXPECT_TRUE(thriftMixedObj.e_ref()->find(1) != thriftMixedObj.e_ref()->end());
}

TEST(SimpleJSONToThriftTest, MapTypeTest) {
  string stringJson("\"stringMap\": {\"a\":\"A\", \"b\":\"B\"}");
  string boolJson("\"boolMap\": {\"true\":\"True\", \"false\":\"False\"}");
  string byteJson("\"byteMap\": {\"1\":\"one\", \"2\":\"two\"}");
  string doubleJson("\"doubleMap\": {\"0.1\":\"0.one\", \"0.2\":\"0.two\"}");
  string enumJson("\"enumMap\": {\"1\":\"male\", \"2\":\"female\"}");
  string json = "{" + stringJson + "," + boolJson + "," + byteJson + "," +
      doubleJson + "," + enumJson + "}";
  myMapStruct mapStruct;
  deserializeJSON(mapStruct, json);
  EXPECT_EQ(mapStruct.stringMap_ref()->size(), 2);
  EXPECT_EQ(mapStruct.stringMap_ref()["a"], "A");
  EXPECT_EQ(mapStruct.stringMap_ref()["b"], "B");
  EXPECT_EQ(mapStruct.boolMap_ref()->size(), 2);
  EXPECT_EQ(mapStruct.boolMap_ref()[true], "True");
  EXPECT_EQ(mapStruct.boolMap_ref()[false], "False");
  EXPECT_EQ(mapStruct.byteMap_ref()->size(), 2);
  EXPECT_EQ(mapStruct.byteMap_ref()[1], "one");
  EXPECT_EQ(mapStruct.byteMap_ref()[2], "two");
  EXPECT_EQ(mapStruct.doubleMap_ref()->size(), 2);
  EXPECT_EQ(mapStruct.doubleMap_ref()[0.1], "0.one");
  EXPECT_EQ(mapStruct.doubleMap_ref()[0.2], "0.two");
  EXPECT_EQ(mapStruct.enumMap_ref()->size(), 2);
  EXPECT_EQ(mapStruct.enumMap_ref()[Gender::MALE], "male");
  EXPECT_EQ(mapStruct.enumMap_ref()[Gender::FEMALE], "female");
}

TEST(SimpleJSONToThriftTest, EmptyStringTest) {
  string jsonT("{\"a\":\"\"}");
  myStringStruct thriftStringObj;
  deserializeJSON(thriftStringObj, jsonT);
  EXPECT_EQ(*thriftStringObj.a_ref(), "");
}

TEST(SimpleJSONToThriftTest, BinaryTypeTest) {
  string jsonT("{\"a\":\"SSBsb3ZlIEJhc2U2NCEA\"}");
  myBinaryStruct thriftBinaryObj;
  deserializeJSON(thriftBinaryObj, jsonT);
  EXPECT_EQ(*thriftBinaryObj.a_ref(), std::string("I love Base64!\0", 15));
}

TEST(SimpleJSONToThriftTest, CompoundTest) {
  SmallStruct struct1;
  *struct1.bools_ref() = {};
  *struct1.ints_ref() = {};

  SmallStruct struct2;
  *struct1.bools_ref() = {true};
  *struct1.ints_ref() = {1};

  SmallStruct struct3;
  *struct1.bools_ref() = {false, true};
  *struct1.ints_ref() = {1, 2};

  NestedStruct nester;
  *nester.lists_ref() = {{}, {{}, {1}, {2, 3}}, {{4, 5, 6}}};
  *nester.sets_ref() = {{}, {{}, {1}, {2, 3}}, {{4, 5, 6}}};
  nester.maps_ref()["abc"][1] = {struct1, struct1, struct2};
  nester.maps_ref()["abc"][2] = {struct1, struct2, struct3};
  nester.maps_ref()["edf"][-10] = {struct2, struct3, struct3};
  nester.maps_ref()["ghi"] = {};
  nester.maps_ref()["jkl"][0] = {};

  TestStruct stuff;
  *stuff.i1_ref() = 1;
  *stuff.i2_ref() = -2;
  *stuff.i3_ref() = 3;
  *stuff.b1_ref() = true;
  *stuff.b2_ref() = false;
  *stuff.doubles_ref() = {0.0, 1.0, -2.0};
  *stuff.ints_ref() = {0, 1, -2};
  *stuff.m1_ref() = {{"one", 1}, {"two", 2}, {"three", 3}};
  *stuff.m2_ref() = {{0, {}}, {1, {"one"}}, {2, {"one", "two"}}};
  *stuff.structs_ref() = {struct1, struct2, struct2};
  *stuff.n_ref() = nester;
  *stuff.s_ref() = "hello \\u!@#$%^&*()\\r\\\\n\\'\"";

  testSimpleJSON(stuff);

  std::string text = serializeJSON(stuff);
  TestStruct deserialized;
  deserializeJSON(deserialized, text);

  EXPECT_TRUE(stuff == deserialized);
}

TEST(SimpleJSONToThriftTest, MapKeysTests) {
  myKeyStruct mapStruct;
  using Key = std::vector<int>;
  mapStruct.a_ref()[Key{}] = "";
  mapStruct.a_ref()[Key{1}] = "1";
  mapStruct.a_ref()[Key{1, 2, 3}] = "123";

  // currently the implementation does not throw errors on
  // map keys that are lists, maps, sets or structs
  // this may be a desireable feature later on
  try {
    testSimpleJSON(mapStruct);
  } catch (apache::thrift::TException&) {
    ADD_FAILURE();
  }
}
