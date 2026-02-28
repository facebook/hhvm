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
#include <folly/container/Foreach.h>

#include <thrift/lib/cpp2/BadFieldAccess.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>
#include <thrift/test/gen-cpp2/optionals_types.h>
#include <thrift/test/gen-cpp2/optionals_types_custom_protocol.h>

using namespace apache::thrift;

template <class T>
static std::string objToJSON(T& obj) {
  SimpleJSONProtocolWriter writer;
  const auto size = obj.serializedSize(&writer);
  folly::IOBufQueue queue(IOBufQueue::cacheChainLength());
  writer.setOutput(&queue, size);
  obj.write(&writer);
  auto buf = queue.move();
  auto ret = buf->to<std::string>();
  return ret;
}

template <class T>
static T jsonToObj(const std::string& json) {
  SimpleJSONProtocolReader reader;
  T ret;
  ret = {};
  auto iobuf = folly::IOBuf::copyBuffer(json);
  reader.setInput(iobuf.get());
  ret.read(&reader);
  return ret;
}

TEST(OptionalsTest, SerDesTests) {
  std::string json1;
  std::string json2;

  cpp2::HasOptionals obj1;
  cpp2::HasOptionals obj2;
  obj1 = {};
  obj2 = {};

  // first try with only the default fields, leave all optionals empty
  obj1.int64Def() = 42;
  obj1.stringDef() = "hello";
  obj1.setDef() = std::set<int64_t>{10, 20, 30};
  obj1.listDef() = std::vector<int64_t>{40, 50, 60};
  obj1.mapDef() = std::map<int64_t, int64_t>{{100, 101}, {102, 103}};
  obj1.enumDef() = cpp2::HasOptionalsTestEnum::FOO;
  obj1.structDef() = cpp2::HasOptionalsExtra();
  obj1.structDef() = {};
  obj1.structDef()->extraInt64Def() = 69;
  obj1.structDef()->extraStringDef() = "world";
  obj1.structDef()->extraSetDef() = std::set<int64_t>{210, 220, 230};
  obj1.structDef()->extraListDef() = std::vector<int64_t>{240, 250, 260};
  obj1.structDef()->extraMapDef() =
      std::map<int64_t, int64_t>{{1000, 1001}, {1002, 1003}};
  obj1.structDef()->extraEnumDef() = cpp2::HasOptionalsTestEnum::BAR;
  json1 = objToJSON(obj1);

  obj2 = jsonToObj<cpp2::HasOptionals>(json1);

  EXPECT_EQ(obj1, obj2);
  json2 = objToJSON(obj2);
  EXPECT_EQ(json1, json2);

  // Then try with the required fields, leave all optionals empty
  obj1.int64Req() = 42;
  obj1.stringReq() = "helloREQUIRED";
  obj1.setReq() = std::set<int64_t>{10, 20, 30};
  obj1.listReq() = std::vector<int64_t>{40, 50, 60};
  obj1.mapReq() = std::map<int64_t, int64_t>{{100, 101}, {102, 103}};
  obj1.enumReq() = cpp2::HasOptionalsTestEnum::FOO;
  obj1.structReq() = cpp2::HasOptionalsExtra();
  obj1.structReq() = {};
  obj1.structReq()->extraInt64Req() = 69;
  obj1.structReq()->extraStringReq() = "world";
  obj1.structReq()->extraSetReq() = std::set<int64_t>{210, 220, 230};
  obj1.structReq()->extraListReq() = std::vector<int64_t>{240, 250, 260};
  obj1.structReq()->extraMapReq() =
      std::map<int64_t, int64_t>{{1000, 1001}, {1002, 1003}};
  obj1.structReq()->extraEnumReq() = cpp2::HasOptionalsTestEnum::BAR;
  json1 = objToJSON(obj1);

  obj2 = jsonToObj<cpp2::HasOptionals>(json1);

  EXPECT_EQ(obj1, obj2);
  json2 = objToJSON(obj2);
  EXPECT_EQ(json1, json2);

  // now set optionals
  obj1.int64Opt() = 42;
  obj1.stringOpt().value_unchecked() = "helloOPTIONAL";
  obj1.setOpt() = std::set<int64_t>{10, 20, 30};
  obj1.listOpt() = std::vector<int64_t>{40, 50, 60};
  obj1.mapOpt() = std::map<int64_t, int64_t>{{100, 101}, {102, 103}};
  obj1.enumOpt() = cpp2::HasOptionalsTestEnum::FOO;
  obj1.structOpt() = cpp2::HasOptionalsExtra();
  obj1.structOpt() = {};
  obj1.structOpt()->extraInt64Opt() = 69;
  obj1.structOpt()->extraStringOpt() = "world";
  obj1.structOpt()->extraSetOpt() = std::set<int64_t>{210, 220, 230};
  obj1.structOpt()->extraListOpt() = std::vector<int64_t>{240, 250, 260};
  obj1.structOpt()->extraMapOpt() =
      std::map<int64_t, int64_t>{{1000, 1001}, {1002, 1003}};
  obj1.structOpt()->extraEnumOpt() = cpp2::HasOptionalsTestEnum::BAR;

  // Note: we did NOT set all the __isset's for the above!
  // Verify optionals WITHOUT isset are not serialized.
  json1 = objToJSON(obj1);
  EXPECT_EQ(std::string::npos, json1.find("helloOPTIONAL"));

  // ok, set the __isset's properly
  apache::thrift::ensure_isset_unsafe(obj1.stringOpt());

  json1 = objToJSON(obj1);
  EXPECT_NE(std::string::npos, json1.find("helloOPTIONAL"));
  obj2 = jsonToObj<cpp2::HasOptionals>(json1);
  EXPECT_EQ(obj1, obj2);
  json2 = objToJSON(obj2);
  EXPECT_EQ(json1, json2);
}

TEST(OptionalsTest, ValueUncheckedTest) {
  cpp2::HasOptionals obj;
  obj.stringOpt().value_unchecked() = "helloOPTIONAL";
  EXPECT_FALSE(obj.stringOpt().has_value());
  EXPECT_THROW(obj.stringOpt().value(), bad_field_access);
  EXPECT_EQ(obj.stringOpt().value_unchecked(), "helloOPTIONAL");
}

TEST(OptionalsTest, EqualityTests) {
  cpp2::HasOptionals obj1;
  cpp2::HasOptionals obj2;
  obj1 = {};
  obj2 = {};

  // for each of the fields:
  // * set a required field, expect equal.
  // * set an optional field on one; expect not equal.
  // * the the optional field on the other one; equal again.

  // both completely empty
  EXPECT_EQ(obj1, obj2);

  obj1.int64Def() = 1;
  obj2.int64Def() = 1;
  EXPECT_EQ(obj1, obj2);
  obj1.int64Req() = 2;
  EXPECT_NE(obj1, obj2);
  obj2.int64Req() = 2;
  EXPECT_EQ(obj1, obj2);
  obj1.int64Opt() = 3;
  EXPECT_NE(obj1, obj2);
  obj2.int64Opt() = 3;
  EXPECT_EQ(obj1, obj2);

  obj1.stringDef() = "hello";
  obj2.stringDef() = "hello";
  EXPECT_EQ(obj1, obj2);
  obj1.stringReq() = "foo";
  EXPECT_NE(obj1, obj2);
  obj2.stringReq() = "foo";
  EXPECT_EQ(obj1, obj2);
  obj1.stringOpt() = "world";
  EXPECT_NE(obj1, obj2);
  obj2.stringOpt() = "world";
  EXPECT_EQ(obj1, obj2);

  obj1.setDef() = std::set<int64_t>{1, 2};
  obj2.setDef() = std::set<int64_t>{1, 2};
  EXPECT_EQ(obj1, obj2);
  obj1.setReq() = std::set<int64_t>{3, 4};
  EXPECT_NE(obj1, obj2);
  obj2.setReq() = std::set<int64_t>{3, 4};
  EXPECT_EQ(obj1, obj2);
  obj1.setOpt() = std::set<int64_t>{5, 6};
  EXPECT_NE(obj1, obj2);
  obj2.setOpt() = std::set<int64_t>{5, 6};
  EXPECT_EQ(obj1, obj2);

  obj1.listDef() = std::vector<int64_t>{7, 8};
  obj2.listDef() = std::vector<int64_t>{7, 8};
  EXPECT_EQ(obj1, obj2);
  obj1.listReq() = std::vector<int64_t>{9, 10};
  EXPECT_NE(obj1, obj2);
  obj2.listReq() = std::vector<int64_t>{9, 10};
  EXPECT_EQ(obj1, obj2);
  obj1.listOpt() = std::vector<int64_t>{11, 12};
  EXPECT_NE(obj1, obj2);
  obj2.listOpt() = std::vector<int64_t>{11, 12};
  EXPECT_EQ(obj1, obj2);

  obj1.mapDef() = std::map<int64_t, int64_t>{{13, 14}, {15, 16}};
  obj2.mapDef() = std::map<int64_t, int64_t>{{13, 14}, {15, 16}};
  EXPECT_EQ(obj1, obj2);
  obj1.mapReq() = std::map<int64_t, int64_t>{{17, 18}, {19, 20}};
  EXPECT_NE(obj1, obj2);
  obj2.mapReq() = std::map<int64_t, int64_t>{{17, 18}, {19, 20}};
  EXPECT_EQ(obj1, obj2);
  obj1.mapOpt() = std::map<int64_t, int64_t>{{21, 22}, {23, 24}};
  EXPECT_NE(obj1, obj2);
  obj2.mapOpt() = std::map<int64_t, int64_t>{{21, 22}, {23, 24}};
  EXPECT_EQ(obj1, obj2);

  obj1.enumDef() = cpp2::HasOptionalsTestEnum::FOO;
  obj2.enumDef() = cpp2::HasOptionalsTestEnum::FOO;
  EXPECT_EQ(obj1, obj2);
  obj1.enumReq() = cpp2::HasOptionalsTestEnum::BAR;
  EXPECT_NE(obj1, obj2);
  obj2.enumReq() = cpp2::HasOptionalsTestEnum::BAR;
  EXPECT_EQ(obj1, obj2);
  obj1.enumOpt() = cpp2::HasOptionalsTestEnum::BAZ;
  EXPECT_NE(obj1, obj2);
  obj2.enumOpt() = cpp2::HasOptionalsTestEnum::BAZ;
  EXPECT_EQ(obj1, obj2);

  obj1.structDef() = cpp2::HasOptionalsExtra();
  obj1.structDef() = {};
  obj2.structDef() = cpp2::HasOptionalsExtra();
  obj2.structDef() = {};
  EXPECT_EQ(obj1, obj2);
  obj1.structReq() = cpp2::HasOptionalsExtra();
  obj1.structReq() = {};
  obj2.structReq() = cpp2::HasOptionalsExtra();
  obj2.structReq() = {};
  EXPECT_EQ(obj1, obj2);
  obj1.structOpt() = cpp2::HasOptionalsExtra();
  obj1.structOpt() = {};
  apache::thrift::ensure_isset_unsafe(obj1.structOpt());
  EXPECT_NE(obj1, obj2);
  obj2.structOpt() = cpp2::HasOptionalsExtra();
  obj2.structOpt() = {};
  apache::thrift::ensure_isset_unsafe(obj2.structOpt());
  EXPECT_EQ(obj1, obj2);

  // just one more test: try required/optional fields in the optional struct
  // to verify that recursive checking w/ optional fields works.
  // Don't bother testing all the nested struct's fields, this is enough.
  obj1.structOpt()->extraInt64Opt() = 666;
  obj2.structOpt()->extraInt64Opt() = 666;
  EXPECT_EQ(obj1, obj2);

  obj1.structOpt()->extraInt64Opt() = 1;
  EXPECT_NE(obj1, obj2);
  obj2.structOpt()->extraInt64Opt() = 1;
  EXPECT_EQ(obj1, obj2);

  obj1.structOpt()->extraInt64Def() = 2;
  EXPECT_NE(obj1, obj2);
  obj2.structOpt()->extraInt64Def() = 2;
  EXPECT_EQ(obj1, obj2);

  obj1.structOpt()->extraInt64Req() = 3;
  EXPECT_NE(obj1, obj2);
  obj2.structOpt()->extraInt64Req() = 3;
  EXPECT_EQ(obj1, obj2);

  obj1.structDef()->extraInt64Opt() = 4;
  EXPECT_NE(obj1, obj2);
  obj2.structDef()->extraInt64Opt() = 4;
  EXPECT_EQ(obj1, obj2);

  obj1.structDef()->extraInt64Def() = 5;
  EXPECT_NE(obj1, obj2);
  obj2.structDef()->extraInt64Def() = 5;
  EXPECT_EQ(obj1, obj2);

  obj1.structDef()->extraInt64Req() = 6;
  EXPECT_NE(obj1, obj2);
  obj2.structDef()->extraInt64Req() = 6;
  EXPECT_EQ(obj1, obj2);

  obj1.structReq()->extraInt64Opt() = 7;
  EXPECT_NE(obj1, obj2);
  obj2.structReq()->extraInt64Opt() = 7;
  EXPECT_EQ(obj1, obj2);

  obj1.structReq()->extraInt64Def() = 8;
  EXPECT_NE(obj1, obj2);
  obj2.structReq()->extraInt64Def() = 8;
  EXPECT_EQ(obj1, obj2);

  obj1.structReq()->extraInt64Req() = 9;
  EXPECT_NE(obj1, obj2);
  obj2.structReq()->extraInt64Req() = 9;
  EXPECT_EQ(obj1, obj2);
}

TEST(OptionalsTest, emplace) {
  cpp2::HasOptionals obj;
  folly::for_each(
      std::make_tuple(obj.stringOpt(), obj.stringReq(), obj.stringDef()),
      [](auto&& i) {
        EXPECT_EQ(i.emplace(3, 'a'), "aaa");
        EXPECT_EQ(i.value(), "aaa");
        EXPECT_EQ(i.emplace(3, 'b'), "bbb");
        EXPECT_EQ(i.value(), "bbb");
        i.emplace() = "ccc";
        EXPECT_EQ(i.value(), "ccc");
        EXPECT_THROW(i.emplace(std::string(""), 1), std::out_of_range);
        if (typeid(i) ==
            typeid(apache::thrift::required_field_ref<std::string&>)) {
          // Required field always has value
          EXPECT_TRUE(i.has_value());
        } else {
          // C++ Standard requires *this to be empty if `emplace(...)` throws
          EXPECT_FALSE(i.has_value());
        }
      });
}

TEST(DeprecatedOptionalField, NulloptComparisons) {
  cpp2::HasOptionals obj;

  EXPECT_TRUE(obj.int64Opt() == std::nullopt);
  EXPECT_TRUE(std::nullopt == obj.int64Opt());

  obj.int64Opt() = 1;
  EXPECT_FALSE(obj.int64Opt() == std::nullopt);
  EXPECT_FALSE(std::nullopt == obj.int64Opt());

  obj.int64Opt().reset();
  EXPECT_FALSE(obj.int64Opt() != std::nullopt);
  EXPECT_FALSE(std::nullopt != obj.int64Opt());

  obj.int64Opt() = 1;
  EXPECT_TRUE(obj.int64Opt() != std::nullopt);
  EXPECT_TRUE(std::nullopt != obj.int64Opt());
}

TEST(OptionalsTest, Equality) {
  cpp2::HasOptionals obj;
  obj.int64Opt() = 1;
  EXPECT_EQ(obj.int64Opt(), 1);
  EXPECT_NE(obj.int64Opt(), 2);
  EXPECT_EQ(1, obj.int64Opt());
  EXPECT_NE(2, obj.int64Opt());
  obj.int64Opt().reset();
  EXPECT_NE(obj.int64Opt(), 1);
  EXPECT_NE(1, obj.int64Opt());
}

TEST(OptionalsTest, Comparison) {
  cpp2::HasOptionals obj;
  obj.int64Opt() = 2;
  EXPECT_LT(obj.int64Opt(), 3);
  EXPECT_LE(obj.int64Opt(), 2);
  EXPECT_LE(obj.int64Opt(), 3);
  EXPECT_LT(1, obj.int64Opt());
  EXPECT_LE(1, obj.int64Opt());
  EXPECT_LE(2, obj.int64Opt());

  EXPECT_GT(obj.int64Opt(), 1);
  EXPECT_GE(obj.int64Opt(), 1);
  EXPECT_GE(obj.int64Opt(), 2);
  EXPECT_GT(3, obj.int64Opt());
  EXPECT_GE(2, obj.int64Opt());
  EXPECT_GE(3, obj.int64Opt());

  obj.int64Opt().reset();
  EXPECT_LT(obj.int64Opt(), -1);
  EXPECT_LE(obj.int64Opt(), -1);
  EXPECT_GT(-1, obj.int64Opt());
  EXPECT_GE(-1, obj.int64Opt());
}

TEST(OptionalsTest, UnsetUnsafe) {
  cpp2::HasOptionals obj;
  EXPECT_FALSE(obj.int64Def().is_set());
  obj.int64Def() = 1;
  EXPECT_TRUE(obj.int64Def().is_set());
  unset_unsafe(obj.int64Def());
  EXPECT_FALSE(obj.int64Def().is_set());
  EXPECT_EQ(obj.int64Def(), 1);

  obj.int64Opt() = 2;
  EXPECT_TRUE(obj.int64Opt().has_value());
  unset_unsafe(obj.int64Opt());
  EXPECT_FALSE(obj.int64Opt().has_value());
  EXPECT_EQ(obj.int64Opt().value_unchecked(), 2);
}

TEST(OptionalsTest, RefForUnqualifiedField) {
  cpp2::HasOptionals obj;
  EXPECT_TRUE(obj.int64Req().has_value());
  obj.int64Req() = 42;
  EXPECT_TRUE(obj.int64Req().has_value());
  EXPECT_EQ(obj.int64Req(), 42);

  EXPECT_TRUE(obj.stringReq().has_value());
  obj.stringReq() = "foo";
  EXPECT_TRUE(obj.stringReq().has_value());
  EXPECT_EQ(obj.stringReq(), "foo");
}

TEST(OptionalsTest, MoveFrom) {
  cpp2::HasOptionals obj1;
  cpp2::HasOptionals obj2;

  obj1.int64Opt() = 1;
  obj2.int64Opt().move_from(obj1.int64Opt());
  EXPECT_EQ(obj2.int64Opt(), 1);

  obj1.int64Opt() = 2;
  obj2.int64Opt().move_from(std::move(obj1).int64Opt());
  EXPECT_EQ(obj2.int64Opt(), 2);

  obj1.int64Opt() = 3;
  obj2.int64Opt().move_from(std::move(obj1).int64Opt());
  EXPECT_EQ(obj2.int64Opt().value(), 3);

  obj1.int64Opt() = 4;
  obj2.int64Opt().move_from(std::move(obj1).int64Opt());
  EXPECT_EQ(obj2.int64Opt().value(), 4);
}

TEST(OptionalsTest, CopyFrom) {
  cpp2::HasOptionals obj1;
  cpp2::HasOptionals obj2;

  obj1.int64Opt() = 1;
  obj2.int64Opt().copy_from(obj1.int64Opt());
  EXPECT_EQ(obj1.int64Opt().value(), 1);
  EXPECT_EQ(obj2.int64Opt().value(), 1);

  obj1.int64Opt() = 2;
  obj2.int64Opt().copy_from(obj1.int64Opt());
  EXPECT_EQ(obj1.int64Opt().value(), 2);
  EXPECT_EQ(obj2.int64Opt().value(), 2);

  obj1.int64Opt() = 3;
  obj2.int64Opt().copy_from(obj1.int64Opt());
  EXPECT_EQ(obj1.int64Opt().value(), 3);
  EXPECT_EQ(obj2.int64Opt().value(), 3);

  obj1.int64Opt() = 4;
  obj2.int64Opt().copy_from(obj1.int64Opt());
  EXPECT_EQ(obj1.int64Opt().value(), 4);
  EXPECT_EQ(obj2.int64Opt().value(), 4);
}

TEST(OptionalsTest, AddRef) {
  cpp2::HasOptionals obj;
  static_assert(
      std::is_same_v<decltype(obj.int64Opt()), optional_field_ref<int64_t&>>);
  static_assert(std::is_same_v<
                decltype(std::as_const(obj).int64Opt()),
                optional_field_ref<const int64_t&>>);
  static_assert(std::is_same_v<
                decltype(std::move(obj).int64Opt()),
                optional_field_ref<int64_t&&>>);
  static_assert(std::is_same_v<
                decltype(std::move(std::as_const(obj)).int64Opt()),
                optional_field_ref<const int64_t&>>);
  obj.int64Opt() = 42;
  EXPECT_EQ(obj.int64Opt(), 42);
  auto value = std::map<int64_t, int64_t>{{1, 2}};
  obj.mapOpt() = value;
  EXPECT_EQ(*obj.mapOpt(), value);
}
