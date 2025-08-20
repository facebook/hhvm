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

#include <thrift/lib/cpp2/protocol/Object.h>

#include <set>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/conformance/cpp2/Protocol.h>
#include <thrift/conformance/cpp2/Testing.h>
#include <thrift/conformance/cpp2/internal/AnyStructSerializer.h>
#include <thrift/conformance/data/ValueGenerator.h>
#include <thrift/conformance/if/gen-cpp2/conformance_types_custom_protocol.h>
#include <thrift/conformance/if/gen-cpp2/protocol_types.h>
#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/BadFieldAccess.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/detail/Object.h>
#include <thrift/lib/cpp2/test/gen-cpp2/ObjectTest_types.h>
#include <thrift/lib/cpp2/type/BaseType.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>
#include <thrift/lib/cpp2/type/TypeRegistry.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>
#include <thrift/lib/thrift/gen-cpp2/standard_types.h>
#include <thrift/test/gen-cpp2/adapter_types.h>
#include <thrift/test/testset/Testset.h>

// TODO: Remove this. Specify namespace explicitly instead.
using namespace ::apache::thrift::conformance;

using detail::protocol_reader_t;
using detail::protocol_writer_t;

namespace apache::thrift::protocol {
namespace {

namespace testset = apache::thrift::test::testset;

template <typename Protocol>
MaskedDecodeResult parseObjectWithTest(
    const folly::IOBuf& buf, Mask mask, bool string_to_binary = true) {
  auto ret = parseObject<Protocol>(buf, mask, string_to_binary);
  auto v =
      parseObjectWithoutExcludedData<Protocol>(buf, mask, string_to_binary);
  EXPECT_EQ(ret.included, v);
  return ret;
}

TEST(ObjectTest, Example) {
  using facebook::thrift::lib::test::Bar;

  Bar bar;
  bar.field_3() = {"foo", "bar", "baz"};
  bar.field_4()->field_1() = 42;
  bar.field_4()->field_2() = "Everything";

  auto serialized = CompactSerializer::serialize<folly::IOBufQueue>(bar).move();

  // We can parse arbitrary serialized thrift blob into Protocol Object
  Object obj = parseObject<CompactSerializer::ProtocolReader>(*serialized);

  // We can re-serialize it back to Protocol Object
  // Note: there is no guarantee that serialized data is byte-wise idential.
  serialized = serializeObject<CompactSerializer::ProtocolWriter>(obj);

  // Test round-trip.
  EXPECT_EQ(CompactSerializer::deserialize<Bar>(serialized.get()), bar);

  // Test constructing the same Object manually
  Object foo;
  foo[FieldId{1}].ensure_i32() = 42;
  foo[FieldId{2}].ensure_binary() = *folly::IOBuf::copyBuffer("Everything");

  Object obj2;
  obj2[FieldId{10}] = asValueStruct<type::list<type::binary_t>>(*bar.field_3());
  obj2[FieldId{20}].ensure_object() = foo;

  EXPECT_EQ(obj, obj2);
}

TEST(ObjectTest, TypeEnforced) {
  // Always a bool when bool_t is used, without ambiguity.
  // Pointers implicitly converts to bools.
  Value value = asValueStruct<type::bool_t>("");
  ASSERT_EQ(value.getType(), Value::Type::boolValue);
  EXPECT_TRUE(value.as_bool());
}

TEST(ObjectTest, Empty) {
  Value value;
  value.emplace_object();
  value.as_object().members().reset();
  serializeValue<CompactProtocolWriter>(value);
  serializeObject<CompactProtocolWriter>(value.as_object());
}

TEST(ObjectTest, Bool) {
  Value value = asValueStruct<type::bool_t>(20);
  ASSERT_EQ(value.getType(), Value::Type::boolValue);
  EXPECT_TRUE(value.as_bool());

  value = asValueStruct<type::bool_t>(0);
  ASSERT_EQ(value.getType(), Value::Type::boolValue);
  EXPECT_FALSE(value.as_bool());
}

TEST(ObjectTest, Byte) {
  Value value = asValueStruct<type::byte_t>(7u);
  ASSERT_EQ(value.getType(), Value::Type::byteValue);
  EXPECT_EQ(value.as_byte(), 7);
}

TEST(ObjectTest, I16) {
  Value value = asValueStruct<type::i16_t>(7u);
  ASSERT_EQ(value.getType(), Value::Type::i16Value);
  EXPECT_EQ(value.as_i16(), 7);
}

TEST(ObjectTest, I32) {
  Value value = asValueStruct<type::i32_t>(7u);
  ASSERT_EQ(value.getType(), Value::Type::i32Value);
  EXPECT_EQ(value.as_i32(), 7);
}

TEST(ObjectTest, I64) {
  Value value = asValueStruct<type::i64_t>(7u);
  ASSERT_EQ(value.getType(), Value::Type::i64Value);
  EXPECT_EQ(value.as_i64(), 7);
}

TEST(ObjectTest, Enum) {
  enum class MyEnum { kValue = 7 };
  Value value = asValueStruct<type::enum_c>(MyEnum::kValue);
  ASSERT_EQ(value.getType(), Value::Type::i32Value);
  EXPECT_EQ(value.as_i32(), 7);

  value = asValueStruct<type::enum_c>(static_cast<MyEnum>(2));
  ASSERT_EQ(value.getType(), Value::Type::i32Value);
  EXPECT_EQ(value.as_i32(), 2);

  value = asValueStruct<type::enum_c>(21u);
  ASSERT_EQ(value.getType(), Value::Type::i32Value);
  EXPECT_EQ(value.as_i32(), 21);
}

TEST(ObjectTest, Float) {
  Value value = asValueStruct<type::float_t>(1.5);
  ASSERT_EQ(value.getType(), Value::Type::floatValue);
  EXPECT_EQ(value.as_float(), 1.5f);
}

TEST(ObjectTest, Double) {
  Value value = asValueStruct<type::double_t>(1.5f);
  ASSERT_EQ(value.getType(), Value::Type::doubleValue);
  EXPECT_EQ(value.as_double(), 1.5);
}

TEST(ObjectTest, String) {
  Value value = asValueStruct<type::string_t>("hi");
  ASSERT_EQ(value.getType(), Value::Type::stringValue);
  EXPECT_EQ(value.as_string(), "hi");
}

TEST(ObjectTest, Binary) {
  Value value = asValueStruct<type::binary_t>("hi");
  ASSERT_EQ(value.getType(), Value::Type::binaryValue);
  EXPECT_EQ(toString(value.as_binary()), "hi");
}

TEST(ObjectTest, List) {
  std::vector<int> data = {1, 4, 2};
  Value value = asValueStruct<type::list<type::i16_t>>(data);
  ASSERT_EQ(value.getType(), Value::Type::listValue);
  ASSERT_EQ(value.as_list().size(), data.size());
  for (size_t i = 0; i < data.size(); ++i) {
    EXPECT_EQ(value.as_list()[i], asValueStruct<type::i16_t>(data[i]));
  }

  // Works with other containers
  std::set<int> s(data.begin(), data.end());
  value = asValueStruct<type::list<type::i16_t>>(s);
  std::sort(data.begin(), data.end());
  ASSERT_EQ(value.getType(), Value::Type::listValue);
  ASSERT_EQ(value.as_list().size(), data.size());
  for (size_t i = 0; i < data.size(); ++i) {
    EXPECT_EQ(value.as_list()[i], asValueStruct<type::i16_t>(data[i]));
  }

  // Works with cpp_type type tag
  Value value2 =
      asValueStruct<type::cpp_type<std::set<int>, type::list<type::i16_t>>>(s);
  EXPECT_EQ(value, value2);
}

TEST(ObjectTest, Set) {
  std::set<int> data = {1, 4, 2};
  Value value = asValueStruct<type::set<type::i16_t>>(data);
  ASSERT_EQ(value.getType(), Value::Type::setValue);
  ASSERT_EQ(value.as_set().size(), data.size());
  for (const Value& v : value.as_set()) {
    ASSERT_TRUE(data.contains(v.as_i16()));
  }

  // Works with other containers
  value = asValueStruct<type::set<type::i16_t>>(
      std::vector<int>(data.begin(), data.end()));
  ASSERT_EQ(value.getType(), Value::Type::setValue);
  ASSERT_EQ(value.as_set().size(), data.size());
  for (const Value& v : value.as_set()) {
    ASSERT_TRUE(data.contains(v.as_i16()));
  }
}

TEST(ObjectTest, Map) {
  std::map<std::string, int> data = {{"one", 1}, {"four", 4}, {"two", 2}};
  Value value = asValueStruct<type::map<type::string_t, type::byte_t>>(data);
  ASSERT_EQ(value.getType(), Value::Type::mapValue);
  ASSERT_EQ(value.as_map().size(), data.size());
  for (const auto& entry : data) {
    auto itr = value.as_map().find(asValueStruct<type::string_t>(entry.first));
    ASSERT_NE(itr, value.as_map().end());
    EXPECT_EQ(itr->second, asValueStruct<type::byte_t>(entry.second));
  }

  // Works with other containers.
  std::vector<std::pair<std::string, int>> otherData(data.begin(), data.end());
  value = asValueStruct<type::map<type::string_t, type::byte_t>>(otherData);
  ASSERT_EQ(value.getType(), Value::Type::mapValue);
  ASSERT_EQ(value.as_map().size(), data.size());
  for (const auto& entry : data) {
    auto itr = value.as_map().find(asValueStruct<type::string_t>(entry.first));
    ASSERT_NE(itr, value.as_map().end());
    EXPECT_EQ(itr->second, asValueStruct<type::byte_t>(entry.second));
  }
}

TEST(ObjectTest, Struct) {
  // TODO(afuller): Use a struct that covers more cases.
  auto protocol = ::apache::thrift::conformance::Protocol("hi").asStruct();
  Value value = asValueStruct<type::union_c>(protocol);
  ASSERT_EQ(value.getType(), Value::Type::objectValue);
  const Object& object = value.as_object();
  EXPECT_EQ(object.members()->size(), 2);
  EXPECT_EQ(
      object.members()->at(1),
      asValueStruct<type::enum_c>(
          ::apache::thrift::conformance::StandardProtocol::Custom));
  EXPECT_EQ(object.members()->at(2), asValueStruct<type::binary_t>("hi"));
}

TEST(ObjectTest, StructWithList) {
  testset::struct_with<type::list<type::i32_t>> s;
  std::vector<int> listValues = {1, 2, 3};
  s.field_1() = listValues;
  Value value = asValueStruct<type::struct_c>(s);
  ASSERT_EQ(value.getType(), Value::Type::objectValue);
  const Object& object = value.as_object();
  EXPECT_EQ(object.members()->size(), 1);
  EXPECT_EQ(
      object.members()->at(1),
      asValueStruct<type::list<type::i32_t>>(listValues));
}

TEST(ObjectTest, StructWithMap) {
  testset::struct_with<type::map<type::string_t, type::i32_t>> s;
  std::map<std::string, int> mapValues = {{"one", 1}, {"four", 4}, {"two", 2}};
  s.field_1() = mapValues;
  Value value = asValueStruct<type::struct_c>(s);
  ASSERT_EQ(value.getType(), Value::Type::objectValue);
  const Object& object = value.as_object();
  EXPECT_EQ(object.members()->size(), 1);
  auto val = asValueStruct<type::map<type::binary_t, type::i32_t>>(mapValues);
  EXPECT_EQ(object.members()->at(1), val);
}

TEST(ObjectTest, StructWithSet) {
  testset::struct_with<type::set<type::i64_t>> s;
  std::set<std::int64_t> setValues = {1, 2, 3};
  s.field_1() = setValues;
  Value value = asValueStruct<type::struct_c>(s);
  ASSERT_EQ(value.getType(), Value::Type::objectValue);
  const Object& object = value.as_object();
  EXPECT_EQ(object.members()->size(), 1);
  EXPECT_EQ(
      object.members()->at(1),
      asValueStruct<type::set<type::i64_t>>(setValues));
}

TEST(ObjectTest, ProtocolValueToThriftValueStructureMissingField) {
  using facebook::thrift::lib::test::Foo;
  // Field 4 does not exist in Foo.
  Object obj;
  obj[FieldId{1}] = asValueStruct<type::i32_t>(1);
  obj[FieldId{4}] = asValueStruct<type::i32_t>(42);
  Foo foo;
  EXPECT_FALSE(
      detail::ProtocolValueToThriftValue<type::struct_t<Foo>>{}(obj, foo));
  EXPECT_EQ(foo.field_1(), 1);
}

TEST(ObjectTest, ProtocolValueToThriftValueNestedStructure) {
  using facebook::thrift::lib::test::Foo;
  using facebook::thrift::lib::test::MyStruct;

  // Field 4 does not exist in Foo.
  Value faultyFooVal;
  faultyFooVal.emplace_object();
  faultyFooVal.as_object()[FieldId{1}] = asValueStruct<type::i32_t>(1);
  faultyFooVal.as_object()[FieldId{4}] = asValueStruct<type::i32_t>(42);

  {
    Object myStructObj;
    myStructObj[FieldId{1}] = faultyFooVal;

    MyStruct myStruct;
    EXPECT_FALSE(detail::ProtocolValueToThriftValue<type::struct_t<MyStruct>>{}(
        myStructObj, myStruct));
    EXPECT_TRUE(myStruct.foo().has_value());
    EXPECT_EQ(myStruct.foo().value().field_1(), 1);
  }
  {
    Object myStructObj;
    myStructObj[FieldId{2}].emplace_list().push_back(faultyFooVal);

    MyStruct myStruct;
    EXPECT_FALSE(detail::ProtocolValueToThriftValue<type::struct_t<MyStruct>>{}(
        myStructObj, myStruct));
    EXPECT_TRUE(myStruct.foo_vector().has_value());
    EXPECT_EQ(
        myStruct,
        CompactSerializer::deserialize<MyStruct>(
            serializeObject<CompactProtocolWriter>(myStructObj)->toString()));
  }
  {
    Object myStructObj;
    myStructObj[FieldId{3}].emplace_set().insert(faultyFooVal);

    MyStruct myStruct;
    EXPECT_FALSE(detail::ProtocolValueToThriftValue<type::struct_t<MyStruct>>{}(
        myStructObj, myStruct));
    EXPECT_TRUE(myStruct.foo_set().has_value());
    EXPECT_EQ(
        myStruct,
        CompactSerializer::deserialize<MyStruct>(
            serializeObject<CompactProtocolWriter>(myStructObj)->toString()));
  }
  {
    Object myStructObj;
    myStructObj[FieldId{4}].emplace_map().emplace(
        faultyFooVal, asValueStruct<type::i32_t>(42));

    MyStruct myStruct;
    EXPECT_FALSE(detail::ProtocolValueToThriftValue<type::struct_t<MyStruct>>{}(
        myStructObj, myStruct));
    EXPECT_TRUE(myStruct.foo_key_map().has_value());
    EXPECT_EQ(
        myStruct,
        CompactSerializer::deserialize<MyStruct>(
            serializeObject<CompactProtocolWriter>(myStructObj)->toString()));
  }
  {
    Object myStructObj;
    myStructObj[FieldId{5}].emplace_map().emplace(
        asValueStruct<type::i32_t>(42), faultyFooVal);

    MyStruct myStruct;
    EXPECT_FALSE(detail::ProtocolValueToThriftValue<type::struct_t<MyStruct>>{}(
        myStructObj, myStruct));
    EXPECT_TRUE(myStruct.foo_value_map().has_value());
    EXPECT_EQ(
        myStruct,
        CompactSerializer::deserialize<MyStruct>(
            serializeObject<CompactProtocolWriter>(myStructObj)->toString()));
  }
  {
    Object myStructObj;
    myStructObj[FieldId{6}] = faultyFooVal;

    MyStruct myStruct;
    EXPECT_FALSE(detail::ProtocolValueToThriftValue<type::struct_t<MyStruct>>{}(
        myStructObj, myStruct));
    EXPECT_TRUE(myStruct.foo_adapted().has_value());
    EXPECT_EQ(myStruct.foo_adapted().value().value.field_1(), 1);
  }
  {
    Object myStructObj;
    myStructObj[FieldId{7}].emplace_list().push_back(faultyFooVal);

    MyStruct myStruct;
    EXPECT_FALSE(detail::ProtocolValueToThriftValue<type::struct_t<MyStruct>>{}(
        myStructObj, myStruct));
    EXPECT_TRUE(myStruct.foo_vector_adapted().has_value());
    EXPECT_EQ(
        myStruct,
        CompactSerializer::deserialize<MyStruct>(
            serializeObject<CompactProtocolWriter>(myStructObj)->toString()));
  }
  {
    Object myStructObj;
    myStructObj[FieldId{8}].emplace_list().push_back(faultyFooVal);

    MyStruct myStruct;
    EXPECT_FALSE(detail::ProtocolValueToThriftValue<type::struct_t<MyStruct>>{}(
        myStructObj, myStruct));
    EXPECT_TRUE(myStruct.foo_adapted_vector().has_value());
    EXPECT_EQ(
        myStruct,
        CompactSerializer::deserialize<MyStruct>(
            serializeObject<CompactProtocolWriter>(myStructObj)->toString()));
  }
  {
    Object myStructObj;
    myStructObj[FieldId{9}].emplace_map().emplace(
        asValueStruct<type::i32_t>(42), faultyFooVal);

    MyStruct myStruct;
    EXPECT_FALSE(detail::ProtocolValueToThriftValue<type::struct_t<MyStruct>>{}(
        myStructObj, myStruct));
    EXPECT_TRUE(myStruct.foo_value_cpp_map().has_value());
    EXPECT_EQ(
        myStruct,
        CompactSerializer::deserialize<MyStruct>(
            serializeObject<CompactProtocolWriter>(myStructObj)->toString()));
  }
}

TEST(ObjectTest, DirectlyAdaptedStruct) {
  apache::thrift::test::basic::DirectlyAdaptedStruct s;
  s.value.data() = 42;
  using Tag = type::adapted<
      ::apache::thrift::test::TemplatedTestAdapter,
      type::struct_t<
          apache::thrift::test::basic::detail::DirectlyAdaptedStruct>>;
  Value value = asValueStruct<Tag>(s);
  ASSERT_TRUE(value.is_object());
  const Object& object = value.as_object();
  EXPECT_EQ(object.members()->size(), 1);
  EXPECT_EQ(object.members()->at(1), asValueStruct<type::i64_t>(42));

  apache::thrift::test::basic::DirectlyAdaptedStruct s2;
  detail::ProtocolValueToThriftValue<Tag>{}(value, s2);
  EXPECT_EQ(s, s2);
}

TEST(ObjectTest, parseObject) {
  folly::IOBufQueue iobufQueue;
  testset::struct_with<type::set<type::i64_t>> thriftStruct;
  std::set<std::int64_t> setValues = {1, 2, 3};
  thriftStruct.field_1() = setValues;
  BinarySerializer::serialize(thriftStruct, &iobufQueue);
  auto serialized = iobufQueue.move();
  auto object = parseObject<BinarySerializer::ProtocolReader>(*serialized);
  EXPECT_EQ(object.members()->size(), 1);
  EXPECT_EQ(
      object.members()->at(1),
      asValueStruct<type::set<type::i64_t>>(setValues));
}

TEST(ObjectTest, serializeObject) {
  folly::IOBufQueue iobufQueue;
  testset::struct_with<type::set<type::i64_t>> thriftStruct;
  std::set<std::int64_t> setValues = {1, 2, 3};
  thriftStruct.field_1() = setValues;
  BinarySerializer::serialize(thriftStruct, &iobufQueue);
  auto expected = iobufQueue.move();
  auto object = parseObject<BinarySerializer::ProtocolReader>(*expected);
  auto actual = serializeObject<BinarySerializer::ProtocolWriter>(object);
  auto objectd = parseObject<BinarySerializer::ProtocolReader>(*actual);
  EXPECT_EQ(objectd.members()->size(), 1);
  EXPECT_EQ(
      objectd.members()->at(1),
      asValueStruct<type::set<type::i64_t>>(setValues));
}

TEST(ObjectTest, ValueUnionTypeMatch) {
  EXPECT_EQ(
      static_cast<type::BaseType>(Value::Type::boolValue),
      type::BaseType::Bool);
  EXPECT_EQ(
      static_cast<type::BaseType>(Value::Type::byteValue),
      type::BaseType::Byte);
  EXPECT_EQ(
      static_cast<type::BaseType>(Value::Type::i16Value), type::BaseType::I16);
  EXPECT_EQ(
      static_cast<type::BaseType>(Value::Type::i32Value), type::BaseType::I32);
  EXPECT_EQ(
      static_cast<type::BaseType>(Value::Type::i64Value), type::BaseType::I64);
  EXPECT_EQ(
      static_cast<type::BaseType>(Value::Type::floatValue),
      type::BaseType::Float);
  EXPECT_EQ(
      static_cast<type::BaseType>(Value::Type::doubleValue),
      type::BaseType::Double);
  EXPECT_EQ(
      static_cast<type::BaseType>(Value::Type::stringValue),
      type::BaseType::String);
  EXPECT_EQ(
      static_cast<type::BaseType>(Value::Type::binaryValue),
      type::BaseType::Binary);
  EXPECT_EQ(
      static_cast<type::BaseType>(Value::Type::listValue),
      type::BaseType::List);
  EXPECT_EQ(
      static_cast<type::BaseType>(Value::Type::setValue), type::BaseType::Set);
  EXPECT_EQ(
      static_cast<type::BaseType>(Value::Type::mapValue), type::BaseType::Map);
  EXPECT_EQ(
      static_cast<type::BaseType>(Value::Type::objectValue),
      type::BaseType::Struct);
}

template <typename ParseObjectTestCase>
class TypedParseObjectTest : public testing::Test {};

template <::apache::thrift::conformance::StandardProtocol Protocol, typename T>
std::unique_ptr<folly::IOBuf> serialize(T& s) {
  folly::IOBufQueue iobufQueue;
  protocol_writer_t<Protocol> writer;
  writer.setOutput(&iobufQueue);
  s.write(&writer);
  auto iobuf = iobufQueue.move();
  return iobuf;
}

template <
    ::apache::thrift::conformance::StandardProtocol Protocol,
    typename Tag,
    typename T>
void testParseObject() {
  T testsetValue;
  for (const auto& val : data::ValueGenerator<Tag>::getKeyValues()) {
    SCOPED_TRACE(val.name);
    testsetValue.field_1_ref() = val.value;
    auto object = asObject(testsetValue);

    auto iobuf = serialize<Protocol, T>(testsetValue);
    auto objFromParseObject = parseObject<protocol_reader_t<Protocol>>(*iobuf);
    EXPECT_EQ(objFromParseObject, object);
  }
}

type::StandardProtocol convertStandardProtocol(
    conformance::StandardProtocol prot) {
  return prot == conformance::StandardProtocol::Binary
      ? type::StandardProtocol::Binary
      : type::StandardProtocol::Compact;
}

template <
    ::apache::thrift::conformance::StandardProtocol Protocol,
    typename Tag,
    typename T>
void testWithMask(bool testSerialize) {
  T testsetValue;
  for (const auto& val : data::ValueGenerator<Tag>::getKeyValues()) {
    SCOPED_TRACE(val.name);
    testsetValue.field_1_ref() = val.value;
    auto object = asObject(testsetValue);

    auto reserialize = [&](MaskedDecodeResult& result) {
      auto reserialized = serializeObject<protocol_writer_t<Protocol>>(
          result.included, result.excluded);
      Object finalObj = parseObject<protocol_reader_t<Protocol>>(*reserialized);
      EXPECT_EQ(finalObj, object);
    };

    auto iobuf = serialize<Protocol, T>(testsetValue);
    {
      // parseObject with allMask should parse the entire object.
      auto result =
          parseObjectWithTest<protocol_reader_t<Protocol>>(*iobuf, allMask());
      if (testSerialize) {
        reserialize(result);
      } else { // manually check the result
        EXPECT_EQ(result.included, object);
        MaskedProtocolData expected;
        expected.protocol() = convertStandardProtocol(Protocol);
        EXPECT_EQ(result.excluded, expected);
      }
    }
    {
      // parseObject with noneMask should parse nothing.
      auto result =
          parseObjectWithTest<protocol_reader_t<Protocol>>(*iobuf, noneMask());
      if (testSerialize) {
        reserialize(result);
      } else { // manually check the result
        EXPECT_EQ(result.included, Object{});
        EXPECT_EQ(
            *result.excluded.protocol(), convertStandardProtocol(Protocol));
        auto& values = *result.excluded.values();
        auto& encodedValue =
            detail::getByValueId(values, *result.excluded.data()->full_ref());
        auto objFromExcluded =
            parseObject<protocol_reader_t<Protocol>>(*encodedValue.data());
        EXPECT_EQ(objFromExcluded, object);
      }
    }
    {
      // parseObject with Mask = includes{1: allMask()}
      Mask mask;
      mask.includes().emplace()[1] = allMask();
      auto result =
          parseObjectWithTest<protocol_reader_t<Protocol>>(*iobuf, mask);
      if (testSerialize) {
        reserialize(result);
      } else { // manually check the result
        EXPECT_EQ(result.included.size(), 1);
        EXPECT_EQ(result.included.at(FieldId{1}), object.at(FieldId{1}));
        EXPECT_EQ(
            *result.excluded.protocol(), convertStandardProtocol(Protocol));
      }
    }
  }
}

template <typename Tag>
bool hasEmptyContainer(const type::standard_type<Tag>& value) {
  if constexpr (type::is_a_v<Tag, type::container_c>) {
    if (value.size() == 0) {
      return true;
    }
  }
  if constexpr (type::is_a_v<Tag, type::map<type::all_c, type::container_c>>) {
    for (const auto& [mapkey, mapval] : value) {
      if (mapval.size() == 0) {
        return true;
      }
    }
  }
  return false;
}

// The tests cases to run.
using ParseObjectTestCases = ::testing::Types<
    type::bool_t,
    type::byte_t,
    type::i16_t,
    type::i32_t,
    type::i64_t,
    type::float_t,
    type::double_t,
    type::binary_t,
    type::string_t,
    type::list<type::byte_t>,
    type::list<type::i16_t>,
    type::list<type::i32_t>,
    type::list<type::i64_t>,
    type::list<type::float_t>,
    type::list<type::double_t>,
    type::set<type::i64_t>,
    type::set<type::string_t>,
    type::map<type::string_t, type::i64_t>,
    type::map<type::i64_t, type::double_t>,
    type::map<type::i64_t, type::set<type::string_t>>>;

TYPED_TEST_SUITE(TypedParseObjectTest, ParseObjectTestCases);

TYPED_TEST(TypedParseObjectTest, ParseSerializedSameAsDirectObject) {
  testParseObject<
      ::apache::thrift::conformance::StandardProtocol::Binary,
      TypeParam,
      testset::struct_with<TypeParam>>();
  testParseObject<
      ::apache::thrift::conformance::StandardProtocol::Compact,
      TypeParam,
      testset::struct_with<TypeParam>>();
  testParseObject<
      ::apache::thrift::conformance::StandardProtocol::Binary,
      TypeParam,
      testset::union_with<TypeParam>>();
  testParseObject<
      ::apache::thrift::conformance::StandardProtocol::Compact,
      TypeParam,
      testset::union_with<TypeParam>>();
}

TYPED_TEST(TypedParseObjectTest, ParseObjectWithMask) {
  testWithMask<
      ::apache::thrift::conformance::StandardProtocol::Binary,
      TypeParam,
      testset::struct_with<TypeParam>>(false);
  testWithMask<
      ::apache::thrift::conformance::StandardProtocol::Compact,
      TypeParam,
      testset::struct_with<TypeParam>>(false);
}

TYPED_TEST(TypedParseObjectTest, SerializeObjectWithMask) {
  testWithMask<
      ::apache::thrift::conformance::StandardProtocol::Binary,
      TypeParam,
      testset::struct_with<TypeParam>>(true);
  testWithMask<
      ::apache::thrift::conformance::StandardProtocol::Compact,
      TypeParam,
      testset::struct_with<TypeParam>>(true);
}

TEST(Object, invalid_object) {
  {
    Object obj;
    obj[FieldId{0}].ensure_list() = {
        asValueStruct<type::i32_t>(1), asValueStruct<type::i64_t>(1)};
    EXPECT_THROW(
        serializeObject<CompactSerializer::ProtocolWriter>(obj),
        TProtocolException);
  }
  {
    Object obj;
    obj[FieldId{0}].ensure_set() = {
        asValueStruct<type::i32_t>(1), asValueStruct<type::i64_t>(1)};
    EXPECT_THROW(
        serializeObject<CompactSerializer::ProtocolWriter>(obj),
        TProtocolException);
  }
  {
    Object obj;
    obj[FieldId{0}].ensure_map() = {
        {asValueStruct<type::i32_t>(1), asValueStruct<type::i32_t>(1)},
        {asValueStruct<type::i32_t>(2), asValueStruct<type::i64_t>(1)}};
    EXPECT_THROW(
        serializeObject<CompactSerializer::ProtocolWriter>(obj),
        TProtocolException);
  }
  {
    Object obj;
    obj[FieldId{0}].ensure_map() = {
        {asValueStruct<type::i32_t>(1), asValueStruct<type::i32_t>(1)},
        {asValueStruct<type::i64_t>(1), asValueStruct<type::i32_t>(1)}};
    EXPECT_THROW(
        serializeObject<CompactSerializer::ProtocolWriter>(obj),
        TProtocolException);
  }
}

TEST(Object, uri) {
  EXPECT_EQ(uri<Object>(), "facebook.com/thrift/protocol/Object");
  EXPECT_EQ(uri<Value>(), "facebook.com/thrift/protocol/Value");
}

TEST(Object, Wrapper) {
  Object object;
  EXPECT_TRUE(object.empty());
  object[FieldId{0}];
  EXPECT_FALSE(object.empty());
  object[FieldId{2}];
  EXPECT_EQ(object.size(), 2);
  EXPECT_EQ(&object[FieldId{0}], &object[FieldId{0}]);
  EXPECT_EQ(&object[FieldId{2}], &object[FieldId{2}]);
  EXPECT_EQ(&object.at(FieldId{0}), &object[FieldId{0}]);
  EXPECT_EQ(&object.at(FieldId{2}), &object[FieldId{2}]);
  EXPECT_EQ(object.if_contains(FieldId{0}), &object[FieldId{0}]);
  EXPECT_EQ(object.if_contains(FieldId{2}), &object[FieldId{2}]);

  EXPECT_TRUE(object.contains(FieldId{0}));
  EXPECT_FALSE(object.contains(FieldId{1}));
  EXPECT_TRUE(object.contains(FieldId{2}));
  EXPECT_THROW(object.at(FieldId{1}), std::out_of_range);

  EXPECT_EQ(object.erase(FieldId{0}), 1);
  EXPECT_FALSE(object.contains(FieldId{0}));
  EXPECT_TRUE(object.contains(FieldId{2}));
  EXPECT_EQ(object.size(), 1);

  EXPECT_EQ(object.erase(FieldId{1}), 0);
  EXPECT_EQ(object.size(), 1);
  EXPECT_FALSE(object.empty());

  EXPECT_EQ(object.erase(FieldId{2}), 1);
  EXPECT_EQ(object.size(), 0);
  EXPECT_TRUE(object.empty());
}

TEST(Value, Wrapper) {
  Object obj;
  obj[FieldId{100}] = asValueStruct<type::string_t>("200");

  const std::vector<Value> l = {
      asValueStruct<type::i32_t>(10), asValueStruct<type::i32_t>(20)};

  const folly::F14VectorSet<Value> s = {
      asValueStruct<type::i32_t>(30), asValueStruct<type::i32_t>(40)};

  const folly::F14FastMap<Value, Value> m = {
      {asValueStruct<type::i32_t>(50), asValueStruct<type::i32_t>(60)},
      {asValueStruct<type::i32_t>(70), asValueStruct<type::i32_t>(80)}};

  Value value;

#define FBTHRIFT_TEST_THRIFT_VALUE_TYPE(TYPE, VALUE)                   \
  do {                                                                 \
    EXPECT_FALSE(value.is_##TYPE());                                   \
    EXPECT_THROW(value.as_##TYPE(), apache::thrift::bad_field_access); \
    EXPECT_EQ(value.if_##TYPE(), nullptr);                             \
    value.ensure_##TYPE() = VALUE;                                     \
    EXPECT_TRUE(value.is_##TYPE());                                    \
    value.emplace_##TYPE() = VALUE;                                    \
    EXPECT_TRUE(value.is_##TYPE());                                    \
    value.emplace_##TYPE(VALUE);                                       \
    EXPECT_TRUE(value.is_##TYPE());                                    \
    EXPECT_EQ(value.as_##TYPE(), VALUE);                               \
    EXPECT_EQ(*value.if_##TYPE(), VALUE);                              \
  } while (false)

  FBTHRIFT_TEST_THRIFT_VALUE_TYPE(bool, true);
  FBTHRIFT_TEST_THRIFT_VALUE_TYPE(byte, 1);
  FBTHRIFT_TEST_THRIFT_VALUE_TYPE(i16, 2);
  FBTHRIFT_TEST_THRIFT_VALUE_TYPE(i32, 3);
  FBTHRIFT_TEST_THRIFT_VALUE_TYPE(i64, 4);
  FBTHRIFT_TEST_THRIFT_VALUE_TYPE(float, 5);
  FBTHRIFT_TEST_THRIFT_VALUE_TYPE(double, 6);
  FBTHRIFT_TEST_THRIFT_VALUE_TYPE(string, "7");
  FBTHRIFT_TEST_THRIFT_VALUE_TYPE(object, obj);
  FBTHRIFT_TEST_THRIFT_VALUE_TYPE(list, l);
  FBTHRIFT_TEST_THRIFT_VALUE_TYPE(set, s);
  FBTHRIFT_TEST_THRIFT_VALUE_TYPE(map, m);

#undef FBTHRIFT_VALUE_TEST_TYPE

  // `binary` type requires special code since IOBuf doesn't have operator==
  const auto buf = *folly::IOBuf::copyBuffer("90");
  EXPECT_FALSE(value.is_binary());
  EXPECT_THROW(value.as_binary(), apache::thrift::bad_field_access);
  EXPECT_EQ(value.if_binary(), nullptr);
  value.ensure_binary() = buf;
  EXPECT_TRUE(value.is_binary());
  EXPECT_TRUE(folly::IOBufEqualTo{}(value.as_binary(), buf));
  EXPECT_TRUE(folly::IOBufEqualTo{}(*value.if_binary(), buf));
}

TEST(Value, IsIntrinsicDefaultTrue) {
  EXPECT_TRUE(isIntrinsicDefault(asValueStruct<type::bool_t>(false)));
  EXPECT_TRUE(isIntrinsicDefault(asValueStruct<type::byte_t>(0)));
  EXPECT_TRUE(isIntrinsicDefault(asValueStruct<type::i16_t>(0)));
  EXPECT_TRUE(isIntrinsicDefault(asValueStruct<type::i32_t>(0)));
  EXPECT_TRUE(isIntrinsicDefault(asValueStruct<type::i64_t>(0)));
  EXPECT_TRUE(isIntrinsicDefault(asValueStruct<type::float_t>(0.0)));
  EXPECT_TRUE(isIntrinsicDefault(asValueStruct<type::double_t>(0.0)));
  EXPECT_TRUE(isIntrinsicDefault(asValueStruct<type::string_t>("")));
  EXPECT_TRUE(isIntrinsicDefault(asValueStruct<type::binary_t>("")));
  EXPECT_TRUE(
      isIntrinsicDefault(asValueStruct<type::list<type::string_t>>({})));
  EXPECT_TRUE(isIntrinsicDefault(asValueStruct<type::set<type::i64_t>>({})));
  EXPECT_TRUE(isIntrinsicDefault(
      asValueStruct<type::map<type::i32_t, type::string_t>>({})));
  testset::struct_with<type::map<type::string_t, type::i32_t>> s;
  s.field_1() = std::map<std::string, int>{};
  Object obj = asObject(s);
  EXPECT_TRUE(isIntrinsicDefault(obj));
}

TEST(Value, IsIntrinsicDefaultFalse) {
  EXPECT_FALSE(isIntrinsicDefault(asValueStruct<type::bool_t>(true)));
  EXPECT_FALSE(isIntrinsicDefault(asValueStruct<type::byte_t>(1)));
  EXPECT_FALSE(isIntrinsicDefault(asValueStruct<type::i16_t>(1)));
  EXPECT_FALSE(isIntrinsicDefault(asValueStruct<type::i32_t>(1)));
  EXPECT_FALSE(isIntrinsicDefault(asValueStruct<type::i64_t>(1)));
  EXPECT_FALSE(isIntrinsicDefault(asValueStruct<type::float_t>(0.5)));
  EXPECT_FALSE(isIntrinsicDefault(asValueStruct<type::double_t>(0.5)));
  EXPECT_FALSE(isIntrinsicDefault(asValueStruct<type::string_t>("foo")));
  EXPECT_FALSE(isIntrinsicDefault(asValueStruct<type::binary_t>("foo")));
  EXPECT_FALSE(
      isIntrinsicDefault(asValueStruct<type::list<type::string_t>>({"foo"})));
  EXPECT_FALSE(
      isIntrinsicDefault(asValueStruct<type::set<type::i64_t>>({1, 2, 3})));
  EXPECT_FALSE(
      isIntrinsicDefault(asValueStruct<type::map<type::i32_t, type::string_t>>(
          {{1, "foo"}, {2, "bar"}})));
  testset::struct_with<type::map<type::string_t, type::i32_t>> s;
  s.field_1() = std::map<std::string, int>{{"foo", 1}, {"bar", 2}};
  EXPECT_FALSE(isIntrinsicDefault(asObject(s)));
}

template <typename ProtocolReader, typename Tag>
Value parseValueFromEncodedValue(const EncodedValue& encodedValue) {
  auto baseType = type::detail::getBaseType(Tag{});
  EXPECT_EQ(encodedValue.wireType().value(), baseType);
  return parseValue<ProtocolReader, Tag>(encodedValue.data().value(), false);
}

// Tests parseObject (and serializeObject if testSerialize) with mask.
template <::apache::thrift::conformance::StandardProtocol Protocol>
void testParseObjectWithMask(bool testSerialize) {
  Object obj, foo, bar, expected;
  // obj{1: 3,
  //     2: {1: "foo"}
  //     3: {5: {1: "foo"},
  //         6: true}}
  foo[FieldId{1}].emplace_string("foo");
  bar[FieldId{5}].emplace_object(foo);
  bar[FieldId{6}].emplace_bool(true);
  obj[FieldId{1}].emplace_i16(3);
  obj[FieldId{2}].emplace_object(foo);
  obj[FieldId{3}].emplace_object(bar);

  // masks obj[2] and obj[3][6]
  Mask mask;
  auto& includes = mask.includes().emplace();
  includes[2] = allMask();
  includes[3].includes().emplace()[6] = allMask();

  // expected{2: {1: "foo"}
  //          3: {6: true}}
  expected[FieldId{2}].emplace_object(foo);
  expected[FieldId{3}].ensure_object()[FieldId{6}].emplace_bool(true);

  // serialize the object and deserialize with mask
  auto serialized = protocol::serializeObject<protocol_writer_t<Protocol>>(obj);
  MaskedDecodeResult result = parseObjectWithTest<protocol_reader_t<Protocol>>(
      *serialized, mask, false);

  if (testSerialize) {
    // test serializeObject with mask
    auto reserialized = protocol::serializeObject<protocol_writer_t<Protocol>>(
        result.included, result.excluded);
    Object finalObj =
        parseObject<protocol_reader_t<Protocol>>(*reserialized, false);
    EXPECT_EQ(finalObj, obj);
    return;
  }

  // manually check the result
  EXPECT_EQ(*result.excluded.protocol(), convertStandardProtocol(Protocol));
  EXPECT_EQ(result.included, expected);
  auto& values = *result.excluded.values();
  EXPECT_EQ(values.size(), 2);
  // Excluded should contain obj[1] and obj[3][5].
  auto& excludedFields = result.excluded.data()->fields().value();
  EXPECT_EQ(excludedFields.size(), 2);
  auto& i16Encoded = detail::getByValueId(
      values, excludedFields.at(FieldId{1}).full().value());

  {
    Value v =
        parseValueFromEncodedValue<protocol_reader_t<Protocol>, type::i16_t>(
            i16Encoded);
    EXPECT_EQ(v.as_i16(), 3);
  }
  auto& nestedExcludedFields = excludedFields.at(FieldId{3}).fields().value();
  EXPECT_EQ(nestedExcludedFields.size(), 1);
  auto& objectEncoded = detail::getByValueId(
      values, nestedExcludedFields.at(FieldId{5}).full().value());
  {
    Value v =
        parseValueFromEncodedValue<protocol_reader_t<Protocol>, type::struct_c>(
            objectEncoded);
    EXPECT_EQ(v.as_object(), foo);
  }
}

template <::apache::thrift::conformance::StandardProtocol Protocol>
void testSerializeObjectWithMask() {
  Object obj, foo;
  // obj{1: {1: "foo",
  //         2: "bar"},
  //     2: 2,
  //     3: 3}
  foo[FieldId{1}].emplace_string("foo");
  foo[FieldId{2}].emplace_string("bar");
  obj[FieldId{1}].emplace_object(foo);
  obj[FieldId{2}].emplace_i32(2);
  obj[FieldId{3}].emplace_i32(3);

  // masks obj[1][1] and obj[2]
  Mask mask;
  auto& includes = mask.includes().emplace();
  includes[1].includes().emplace()[1] = allMask();
  includes[2] = allMask();

  // serialize the object and deserialize with mask
  auto serialized = protocol::serializeObject<protocol_writer_t<Protocol>>(obj);
  MaskedDecodeResult result = parseObjectWithTest<protocol_reader_t<Protocol>>(
      *serialized, mask, false);
  {
    Object expected, bar;
    // expected{1: {1: "foo"},
    //          2: 2}
    bar[FieldId{1}].emplace_string("foo");
    expected[FieldId{1}].emplace_object(bar);
    expected[FieldId{2}].emplace_i32(2);
    EXPECT_EQ(result.included, expected);
  }

  {
    // reserialize with the unmodified object
    auto reserialized = protocol::serializeObject<protocol_writer_t<Protocol>>(
        result.included, result.excluded);
    Object finalObj =
        parseObject<protocol_reader_t<Protocol>>(*reserialized, false);
    EXPECT_EQ(finalObj, obj);
  }

  {
    // reserialize with the modified object
    Object modified, baz;
    // modified{1: {3: "baz"},
    //          4: 4}
    baz[FieldId{3}].emplace_string("baz");
    modified[FieldId{1}].emplace_object(baz);
    modified[FieldId{4}].emplace_i32(4);

    Object expected, bar;
    // expected{1: {2: "bar",
    //              3: "baz"},
    //          3: 3,
    //          4: 4}
    bar[FieldId{2}].emplace_string("bar");
    bar[FieldId{3}].emplace_string("baz");
    expected[FieldId{1}].emplace_object(bar);
    expected[FieldId{3}].emplace_i32(3);
    expected[FieldId{4}].emplace_i32(4);

    auto reserialized = protocol::serializeObject<protocol_writer_t<Protocol>>(
        modified, result.excluded);
    Object finalObj =
        parseObject<protocol_reader_t<Protocol>>(*reserialized, false);
    EXPECT_EQ(finalObj, expected);
  }
}

template <::apache::thrift::conformance::StandardProtocol Protocol>
void testSerializeObjectWithMaskError() {
  Object obj, foo;
  // obj{1: {1: "foo"}}
  foo[FieldId{1}].emplace_string("foo");
  obj[FieldId{1}].emplace_object(foo);

  {
    // MaskedData[1] is full, which should be fields.
    MaskedProtocolData protocolData;
    protocolData.protocol() = convertStandardProtocol(Protocol);
    MaskedData& maskedData = protocolData.data().value();
    maskedData.fields().ensure()[FieldId{1}].full() = type::ValueId{1};

    EXPECT_THROW(
        protocol::serializeObject<protocol_writer_t<Protocol>>(
            obj, protocolData),
        std::runtime_error);
  }
  {
    // MaskedData[2] is fields, which should be full.
    MaskedProtocolData protocolData;
    protocolData.protocol() = convertStandardProtocol(Protocol);
    MaskedData& maskedData = protocolData.data().value();
    maskedData.fields().ensure()[FieldId{2}].fields().ensure();

    EXPECT_THROW(
        protocol::serializeObject<protocol_writer_t<Protocol>>(
            obj, protocolData),
        std::runtime_error);
  }
}

TEST(Object, ParseObjectWithMask) {
  testParseObjectWithMask<
      ::apache::thrift::conformance::StandardProtocol::Compact>(false);
  testParseObjectWithMask<
      ::apache::thrift::conformance::StandardProtocol::Binary>(false);
}

TEST(Object, SerializeObjectWithMask) {
  testParseObjectWithMask<
      ::apache::thrift::conformance::StandardProtocol::Compact>(true);
  testParseObjectWithMask<
      ::apache::thrift::conformance::StandardProtocol::Binary>(true);
  testSerializeObjectWithMask<
      ::apache::thrift::conformance::StandardProtocol::Compact>();
  testSerializeObjectWithMask<
      ::apache::thrift::conformance::StandardProtocol::Binary>();
  testSerializeObjectWithMaskError<
      ::apache::thrift::conformance::StandardProtocol::Compact>();
  testSerializeObjectWithMaskError<
      ::apache::thrift::conformance::StandardProtocol::Binary>();
}

// called by testParseObjectWithMapMask when testSerialize=true
template <::apache::thrift::conformance::StandardProtocol Protocol>
void testSerializeObjectWithMapMask(MaskedDecodeResult& result, Object& obj) {
  {
    // test serializeObject with mask
    // reserialize with the unmodified object
    auto reserialized = protocol::serializeObject<protocol_writer_t<Protocol>>(
        result.included, result.excluded);
    Object finalObj =
        parseObject<protocol_reader_t<Protocol>>(*reserialized, false);
    EXPECT_EQ(finalObj, obj);
  }
  {
    // reserialize with the modified object
    Object modified;
    // modified{1: map{10: {},
    //                 30: {"foo": 1}}}
    // This tests when map is empty and types are determined from MaskedData.
    modified[FieldId{1}] = asValueStruct<
        type::map<type::i16_t, type::map<type::string_t, type::i32_t>>>(
        {{10, {}}, {30, {{"foo", 1}}}});

    Object expected;
    // expected{1: map{10: {"bar": 2},
    //                 20: {"baz": 3},
    //                 30: {"foo": 1}}}
    expected[FieldId{1}] = asValueStruct<
        type::map<type::i16_t, type::map<type::string_t, type::i32_t>>>(
        {{10, {{"bar", 2}}}, {20, {{"baz", 3}}}, {30, {{"foo", 1}}}});

    auto reserialized = protocol::serializeObject<protocol_writer_t<Protocol>>(
        modified, result.excluded);
    Object finalObj =
        parseObject<protocol_reader_t<Protocol>>(*reserialized, false);
    EXPECT_EQ(finalObj, expected);
  }
}

template <::apache::thrift::conformance::StandardProtocol Protocol>
void testParseObjectWithMapMask(bool testSerialize) {
  Object obj;
  // obj{1: map{10: {"foo": 1,
  //                 "bar": 2},
  //            20: {"baz": 3}},
  //     2: set{1, 2, 3}}
  obj[FieldId{1}] = asValueStruct<
      type::map<type::i16_t, type::map<type::string_t, type::i32_t>>>(
      {{10, {{"foo", 1}, {"bar", 2}}}, {20, {{"baz", 3}}}});
  std::set<int> set = {1, 2, 3};
  obj[FieldId{2}] = asValueStruct<type::set<type::byte_t>>(set);
  auto serialized = protocol::serializeObject<protocol_writer_t<Protocol>>(obj);

  // masks obj[1][10]["foo"] and obj[2]
  Mask mask;
  int16_t key10 = 10;
  int16_t key20 = 20;
  std::string keyFoo = "foo";
  std::string keyBar = "bar";
  auto& includes = mask.includes().emplace();
  includes[1]
      .includes_map()
      .emplace()[key10]
      .includes_string_map()
      .emplace()[keyFoo] = allMask();
  // This is treated as allMask() as the type is set. It tests the edge case
  // that a set field may have a map mask, since extractMaskFromPatch cannot
  // determine if a patch is for map or set for some operators.
  includes[2].excludes_map().emplace()[99] = allMask();

  Object expected;
  // expected{1: map{10: {"foo": 1}},
  //          2: set{1, 2, 3}}
  expected[FieldId{1}] = asValueStruct<
      type::map<type::i16_t, type::map<type::string_t, type::i32_t>>>(
      {{10, {{"foo", 1}}}});
  expected[FieldId{2}] = asValueStruct<type::set<type::byte_t>>(set);

  // serialize the object and deserialize with mask
  MaskedDecodeResult result = parseObjectWithTest<protocol_reader_t<Protocol>>(
      *serialized, mask, false);

  if (testSerialize) {
    testSerializeObjectWithMapMask<Protocol>(result, obj);
    return;
  }

  // manually check the result
  EXPECT_EQ(result.included, expected);
  EXPECT_EQ(*result.excluded.protocol(), convertStandardProtocol(Protocol));
  auto& values = *result.excluded.values();
  EXPECT_EQ(values.size(), 2); // map[10]["bar"] and map[20]
  auto& keys = *result.excluded.keys();
  EXPECT_EQ(keys.size(), 3); // 10, 20, and "bar"

  auto getKeyValueId = [&](const Value& key) {
    auto it = std::find(keys.begin(), keys.end(), key);
    EXPECT_NE(it, keys.end()); // It should find the value.
    return type::ValueId{apache::thrift::util::i32ToZigzag(it - keys.begin())};
  };

  // Excluded should contain map[10]["bar"] and map[20]
  auto& excludedKeys =
      result.excluded.data()->fields()->at(FieldId{1}).values().value();
  EXPECT_EQ(excludedKeys.size(), 2);
  // check map[20]
  {
    auto& mapEncoded = detail::getByValueId(
        values,
        excludedKeys.at(getKeyValueId(asValueStruct<type::i16_t>(key20)))
            .full_ref()
            .value());
    Value v =
        parseValueFromEncodedValue<protocol_reader_t<Protocol>, type::map_c>(
            mapEncoded);
    EXPECT_EQ(
        v.as_map(),
        obj[FieldId{1}].as_map()[asValueStruct<type::i16_t>(key20)].as_map());
  }
  // check map[10]["bar"]
  {
    auto& nestedExcludedKeys =
        excludedKeys.at(getKeyValueId(asValueStruct<type::i16_t>(key10)))
            .values_ref()
            .value();
    EXPECT_EQ(nestedExcludedKeys.size(), 1);
    auto& i32Encoded = detail::getByValueId(
        values,
        nestedExcludedKeys
            .at(getKeyValueId(asValueStruct<type::string_t>(keyBar)))
            .full_ref()
            .value());
    Value v =
        parseValueFromEncodedValue<protocol_reader_t<Protocol>, type::i32_t>(
            i32Encoded);
    EXPECT_EQ(v.as_i32(), 2);
  }
}

TEST(ObjectTest, ToDynamic) {
  Value v;
  v.ensure_bool() = true;
  EXPECT_EQ(toDynamic(v), true);
  EXPECT_EQ(toDynamic(v).type(), folly::dynamic::Type::BOOL);
  v.ensure_byte() = 10;
  EXPECT_EQ(toDynamic(v), 10);
  EXPECT_EQ(toDynamic(v).type(), folly::dynamic::Type::INT64);
  v.ensure_i16() = 20;
  EXPECT_EQ(toDynamic(v), 20);
  EXPECT_EQ(toDynamic(v).type(), folly::dynamic::Type::INT64);
  v.ensure_i32() = 30;
  EXPECT_EQ(toDynamic(v), 30);
  EXPECT_EQ(toDynamic(v).type(), folly::dynamic::Type::INT64);
  v.ensure_i64() = 40;
  EXPECT_EQ(toDynamic(v), 40);
  EXPECT_EQ(toDynamic(v).type(), folly::dynamic::Type::INT64);
  v.ensure_float() = 50;
  EXPECT_EQ(toDynamic(v), float(50));
  EXPECT_EQ(toDynamic(v).type(), folly::dynamic::Type::DOUBLE);
  v.ensure_double() = 60;
  EXPECT_EQ(toDynamic(v), double(60));
  EXPECT_EQ(toDynamic(v).type(), folly::dynamic::Type::DOUBLE);
  v.ensure_string() = "70";
  EXPECT_EQ(toDynamic(v), "70");
  EXPECT_EQ(toDynamic(v).type(), folly::dynamic::Type::STRING);
  v = asValueStruct<type::binary_t>("80");
  EXPECT_EQ(toDynamic(v), "80");
  EXPECT_EQ(toDynamic(v).type(), folly::dynamic::Type::STRING);
  v = Value();
  EXPECT_EQ(toDynamic(v), nullptr);
  EXPECT_EQ(toDynamic(v).type(), folly::dynamic::Type::NULLT);

  v.ensure_float() = NAN;
  EXPECT_TRUE(std::isnan(toDynamic(v).asDouble()));
  EXPECT_EQ(toDynamic(v).type(), folly::dynamic::Type::DOUBLE);

  std::vector<int> vec = {1, 4, 2};
  v = asValueStruct<type::list<type::i16_t>>(vec);
  EXPECT_EQ(toDynamic(v), folly::dynamic::array(1, 4, 2));
  EXPECT_EQ(toDynamic(v).type(), folly::dynamic::Type::ARRAY);

  v = asValueStruct<type::set<type::i16_t>>({1, 4, 2});

  v = asValueStruct<type::map<type::string_t, type::string_t>>(
      {{"1", "10"}, {"4", "40"}, {"2", "20"}});
  EXPECT_EQ(
      toDynamic(v),
      folly::dynamic(folly::dynamic::object("4", "40")("1", "10")("2", "20")));
  EXPECT_EQ(toDynamic(v).type(), folly::dynamic::Type::OBJECT);

  v.ensure_object();
  v.as_object()[FieldId{10}].ensure_string() = "100";
  v.as_object()[FieldId{40}].ensure_string() = "400";
  v.as_object()[FieldId{20}].ensure_string() = "200";
  EXPECT_EQ(
      toDynamic(v),
      folly::dynamic(
          folly::dynamic::object("40", "400")("10", "100")("20", "200")));
  EXPECT_EQ(toDynamic(v).type(), folly::dynamic::Type::OBJECT);

  Value v2;
  v2.ensure_object()[FieldId{30}].ensure_string() = "300";
  v2.as_object()[FieldId{50}] = v;
  EXPECT_EQ(
      toDynamic(v2),
      folly::dynamic(folly::dynamic::object("30", "300")("50", toDynamic(v))));
  EXPECT_EQ(toDynamic(v).type(), folly::dynamic::Type::OBJECT);

  v = asValueStruct<type::list<type::i16_t>>(vec);
  v2.ensure_map()[v] = asValueStruct<type::i32_t>(10);
  EXPECT_THROW(toDynamic(v2), std::runtime_error);
}

template <::apache::thrift::conformance::StandardProtocol Protocol>
void testSerializeObjectWithMapMaskError() {
  Object obj;
  // obj{1: map{1: "foo"}}
  obj[FieldId{1}] =
      asValueStruct<type::map<type::i32_t, type::string_t>>({{1, "foo"}});

  {
    // MaskedData[1] is full, which should be values.
    MaskedProtocolData protocolData;
    protocolData.protocol() = convertStandardProtocol(Protocol);
    MaskedData& maskedData = protocolData.data().value();
    maskedData.fields().ensure()[FieldId{1}].full() = type::ValueId{1};

    EXPECT_THROW(
        protocol::serializeObject<protocol_writer_t<Protocol>>(
            obj, protocolData),
        std::runtime_error);
  }
  {
    // MaskedData[1][2] is values, which should be full.
    MaskedProtocolData protocolData;
    protocolData.protocol() = convertStandardProtocol(Protocol);
    MaskedData& maskedData = protocolData.data().value();
    auto& keys = protocolData.keys().ensure();
    keys.push_back(asValueStruct<type::i32_t>(2));
    type::ValueId keyValueId =
        type::ValueId{apache::thrift::util::i32ToZigzag(keys.size() - 1)};
    maskedData.fields()
        .ensure()[FieldId{1}]
        .values()
        .ensure()[keyValueId]
        .values()
        .ensure();

    EXPECT_THROW(
        protocol::serializeObject<protocol_writer_t<Protocol>>(
            obj, protocolData),
        std::runtime_error);
  }
}

TEST(Object, ParseObjectWithMapMask) {
  testParseObjectWithMapMask<
      ::apache::thrift::conformance::StandardProtocol::Compact>(false);
  testParseObjectWithMapMask<
      ::apache::thrift::conformance::StandardProtocol::Binary>(false);
}

TEST(Object, SerializeObjectWithMapMask) {
  testParseObjectWithMapMask<
      ::apache::thrift::conformance::StandardProtocol::Compact>(true);
  testParseObjectWithMapMask<
      ::apache::thrift::conformance::StandardProtocol::Binary>(true);
  testSerializeObjectWithMapMaskError<
      ::apache::thrift::conformance::StandardProtocol::Compact>();
  testSerializeObjectWithMapMaskError<
      ::apache::thrift::conformance::StandardProtocol::Binary>();
}

template <::apache::thrift::conformance::StandardProtocol Protocol>
void testParseObjectWithTwoMasks() {
  Object obj, foo;
  // obj{1: {1: "foo",
  //         2: "bar"},
  //     2: 2,
  //     3: 3,
  //     4: map{10: {"foo": 1,
  //                 "bar": 2},
  //            20: {"baz": 3}}}
  foo[FieldId{1}].emplace_string("foo");
  foo[FieldId{2}].emplace_string("bar");
  obj[FieldId{1}].emplace_object(foo);
  obj[FieldId{2}].emplace_i32(2);
  obj[FieldId{3}].emplace_i32(3);
  obj[FieldId{4}] = asValueStruct<
      type::map<type::i16_t, type::map<type::string_t, type::i32_t>>>(
      {{10, {{"foo", 1}, {"bar", 2}}}, {20, {{"baz", 3}}}});

  int16_t key10 = 10;
  int16_t key20 = 20;
  std::string keyFoo = "foo";
  std::string keyBaz = "baz";

  // masks obj[2] and obj[4][10]["foo"]
  Mask readMask;
  {
    auto& includes = readMask.includes().emplace();
    includes[2] = allMask();
    includes[4]
        .includes_map()
        .emplace()[key10]
        .includes_string_map()
        .emplace()[keyFoo] = allMask();
  }

  // masks obj[1][1], obj[3], obj[4][10], and obj[4][20]["baz"]
  Mask writeMask;
  {
    auto& includes = writeMask.includes().emplace();
    includes[1].includes().emplace()[1] = allMask();
    includes[3] = allMask();
    auto& includes_map = includes[4].includes_map().emplace();
    includes_map[key10] = allMask();
    includes_map[key20].includes_string_map().emplace()[keyBaz] = allMask();
  }

  // serialize the object and deserialize with mask
  auto serialized = protocol::serializeObject<protocol_writer_t<Protocol>>(obj);
  MaskedDecodeResult result = parseObject<protocol_reader_t<Protocol>>(
      *serialized, readMask, writeMask, false);
  {
    Object expected;
    // expected{1: {},
    //          2: 2,
    //          4: map{10: {"foo": 1}
    //                 20: {}}}
    expected[FieldId{1}].emplace_object();
    expected[FieldId{2}].emplace_i32(2);
    expected[FieldId{4}] = asValueStruct<
        type::map<type::i16_t, type::map<type::string_t, type::i32_t>>>(
        {{10, {{"foo", 1}}}, {20, {}}});
    EXPECT_EQ(result.included, expected);
  }

  {
    // reserialize with the object and MaskedData
    Object expected, bar;
    // expected{1: {2: "bar"},
    //          2: 2,
    //          4: map{10: {"foo": 1},
    //                 20: {}}}
    bar[FieldId{2}].emplace_string("bar");
    expected[FieldId{1}].emplace_object(bar);
    expected[FieldId{2}].emplace_i32(2);
    expected[FieldId{4}] = asValueStruct<
        type::map<type::i16_t, type::map<type::string_t, type::i32_t>>>(
        {{10, {{"foo", 1}}}, {20, {}}});
    auto reserialized = protocol::serializeObject<protocol_writer_t<Protocol>>(
        result.included, result.excluded);
    Object finalObj =
        parseObject<protocol_reader_t<Protocol>>(*reserialized, false);
    EXPECT_EQ(finalObj, expected);
  }
}

TEST(Object, ParseObjectWithTwoMasks) {
  testParseObjectWithTwoMasks<
      ::apache::thrift::conformance::StandardProtocol::Compact>();
  testParseObjectWithTwoMasks<
      ::apache::thrift::conformance::StandardProtocol::Binary>();
}

TEST(Object, ToType) {
  using namespace type;
  Value v;
  v.ensure_bool() = true;
  EXPECT_EQ(toType(v), Type::create<bool_t>());
  v.ensure_byte() = 1;
  EXPECT_EQ(toType(v), Type::create<byte_t>());
  v.ensure_i16() = 1;
  EXPECT_EQ(toType(v), Type::create<i16_t>());
  v.ensure_i32() = 1;
  EXPECT_EQ(toType(v), Type::create<i32_t>());
  v.ensure_i64() = 1;
  EXPECT_EQ(toType(v), Type::create<i64_t>());
  v.ensure_float() = 1;
  EXPECT_EQ(toType(v), Type::create<float_t>());
  v.ensure_double() = 1;
  EXPECT_EQ(toType(v), Type::create<double_t>());
  v.ensure_string() = "1";
  EXPECT_EQ(toType(v), Type::create<string_t>());
  v.ensure_binary();
  EXPECT_EQ(toType(v), Type::create<binary_t>());

  Value elem;
  elem.ensure_i32() = 20;
  v.ensure_list() = {elem};
  EXPECT_EQ(toType(v), Type::create<list_c>(Type::create<i32_t>()));
  v.ensure_list().clear();
  EXPECT_EQ(toType(v), Type::create<list_c>(Type{}));

  v.ensure_set() = {elem};
  EXPECT_EQ(toType(v), Type::create<set_c>(Type::create<i32_t>()));
  v.ensure_set().clear();
  EXPECT_EQ(toType(v), Type::create<set_c>(Type{}));

  Value key, value;
  key.ensure_i32() = 10;
  value.ensure_string() = "10";
  v.ensure_map() = {{key, value}};
  EXPECT_EQ(
      toType(v),
      Type::create<map_c>(Type::create<i32_t>(), Type::create<string_t>()));
  v.ensure_map().clear();
  EXPECT_EQ(toType(v), Type::create<map_c>(Type{}, Type{}));

  Value obj;
  obj.ensure_object();
  obj.as_object().type() = "facebook.com/to/obj";
  obj.as_object()[FieldId{1}] = elem;
  EXPECT_EQ(toType(obj), Type::create<struct_c>("facebook.com/to/obj"));
}

TEST(ToAnyTest, simple) {
  using facebook::thrift::lib::test::Bar;

  Bar bar;
  bar.field_3() = {"foo", "bar", "baz"};
  bar.field_4()->field_1() = 42;
  bar.field_4()->field_2() = "Everything";

  auto any = type::TypeRegistry::generated().store(
      bar, type::StandardProtocol::Compact);
  auto serialized = CompactSerializer::serialize<folly::IOBufQueue>(bar).move();
  Value value;
  value.ensure_object() =
      parseObject<CompactSerializer::ProtocolReader>(*serialized);
  auto newAny = toAny(
      value,
      type::Type::create<type::struct_t<Bar>>(),
      type::StandardProtocol::Compact);
  EXPECT_EQ(newAny.type(), any.type());
  EXPECT_EQ(newAny.protocol(), type::StandardProtocol::Compact);
  EXPECT_EQ(
      parseObject<CompactSerializer::ProtocolReader>(newAny.data()),
      value.as_object());
  // TODO(dokwon): Enable this when we wrap Thrift Any with Adapter.
  // EXPECT_EQ(any, toAny<CompactSerializer::ProtocolWriter>(value));
}

TEST(ObjectTest, FromValueStruct) {
  Value value;
  value.emplace_bool() = true;
  EXPECT_TRUE(fromValueStruct<type::bool_t>(value));
  value.emplace_byte() = 10;
  EXPECT_EQ(fromValueStruct<type::byte_t>(value), 10);
  value.emplace_i16() = 20;
  EXPECT_EQ(fromValueStruct<type::i16_t>(value), 20);
  value.emplace_i32() = 30;
  EXPECT_EQ(fromValueStruct<type::i32_t>(value), 30);
  value.emplace_i64() = 40;
  EXPECT_EQ(fromValueStruct<type::i64_t>(value), 40);
  value.emplace_float() = 50;
  EXPECT_EQ(fromValueStruct<type::float_t>(value), 50);
  value.emplace_double() = 60;
  EXPECT_EQ(fromValueStruct<type::double_t>(value), 60);

  Value v1;
  v1.emplace_i32() = 10;
  Value v2;
  v2.emplace_i32() = 20;
  Value v3;
  v3.emplace_i32() = 30;
  Value v4;
  v4.emplace_i32() = 40;

  // List
  value.emplace_list() = {v1, v3, v2, v4};
  EXPECT_EQ(
      fromValueStruct<type::list<type::i32_t>>(value),
      (std::vector<std::int32_t>{10, 30, 20, 40}));

  // Set
  value.emplace_set() = {v1, v3, v2, v4};
  EXPECT_EQ(
      fromValueStruct<type::set<type::i32_t>>(value),
      (std::set<std::int32_t>{10, 20, 30, 40}));

  // Map
  value.emplace_map() = {{v1, v3}, {v2, v4}};
  EXPECT_EQ(
      (fromValueStruct<type::map<type::i32_t, type::i32_t>>(value)),
      (std::map<std::int32_t, std::int32_t>{{10, 30}, {20, 40}}));

  // Struct
  using facebook::thrift::lib::test::Bar;
  using Tag = type::struct_t<Bar>;

  Bar bar;
  bar.field_3() = {"foo", "bar", "baz"};
  bar.field_4()->field_1() = 42;
  bar.field_4()->field_2() = "Everything";

  EXPECT_EQ(fromValueStruct<Tag>(asValueStruct<Tag>(bar)), bar);
  EXPECT_EQ(fromObjectStruct<Tag>(asValueStruct<Tag>(bar).as_object()), bar);
}

template <class Tag>
void testSerializeValue(
    const type::native_type<Tag>& t, bool deterministic = true) {
  // Test whether serializeValue output the correct size.
  // If result is deterministic, also test whether result matches op::decode's
  folly::IOBufQueue queue1, queue2;
  CompactProtocolWriter writer1, writer2;
  writer1.setOutput(&queue1);
  writer2.setOutput(&queue2);

  auto size1 = detail::serializeValue(writer1, asValueStruct<Tag>(t));
  auto size2 = op::encode<Tag>(writer2, t);

  auto buf1 = queue1.moveAsValue();
  auto buf2 = queue2.moveAsValue();

  EXPECT_EQ(size1, buf1.computeChainDataLength());
  EXPECT_EQ(size2, buf2.computeChainDataLength());

  CompactProtocolReader reader1, reader2;
  reader1.setInput(&buf1);
  reader2.setInput(&buf2);

  type::native_type<Tag> t1, t2, t3;
  op::decode<Tag>(reader1, t1);
  op::decode<Tag>(reader2, t2);
  detail::ProtocolValueToThriftValue<Tag>{}(asValueStruct<Tag>(t), t3);

  EXPECT_EQ(t, t1);
  EXPECT_EQ(t, t2);
  EXPECT_EQ(t, t3);

  EXPECT_TRUE(!deterministic || folly::IOBufEqualTo{}(buf1, buf2));
}

TEST(ObjectTest, SerializeValueSize) {
  testSerializeValue<type::bool_t>(false);
  testSerializeValue<type::bool_t>(true);
  testSerializeValue<type::byte_t>(10);
  testSerializeValue<type::i16_t>(20);
  testSerializeValue<type::i32_t>(30);
  testSerializeValue<type::i64_t>(40);
  testSerializeValue<type::float_t>(50.0);
  testSerializeValue<type::double_t>(60.0);

  testSerializeValue<type::list<type::i32_t>>({10, 20, 30, 40});
  testSerializeValue<type::set<type::i32_t>>({10, 20, 30, 40}, false);
  testSerializeValue<type::map<type::i32_t, type::string_t>>(
      {{10, "10"}, {20, "20"}}, false);

  // Struct
  using facebook::thrift::lib::test::Bar;

  Bar bar;
  bar.field_3() = {"foo", "bar", "baz"};
  bar.field_4()->field_1() = 42;
  bar.field_4()->field_2() = "Everything";

  // Result is not deterministic since field id can be out of order
  testSerializeValue<type::struct_t<Bar>>(bar, false);
}

TEST(ObjectTest, Adapter) {
  using test::basic::AdaptTestStruct;
  AdaptTestStruct s;
  s.timeout() = std::chrono::seconds(1);
  s.data()->value = 10;
  s.indirectionString()->val = "20";
  s.double_wrapped_integer()->value.value = 30;
  s.custom()->val = 40;
  using Tag = type::struct_t<AdaptTestStruct>;
  EXPECT_EQ(fromObjectStruct<Tag>(asValueStruct<Tag>(s).as_object()), s);
}

TEST(ObjectTest, ToThriftStructTypeMismatch) {
  using facebook::thrift::lib::test::Bar;
  using Tag = type::struct_t<Bar>;

  Bar bar;
  bar.field_3() = {"foo", "bar", "baz"};
  bar.field_4()->field_1() = 42;
  bar.field_4()->field_2() = "Everything";

  {
    auto obj = asValueStruct<Tag>(bar).as_object();
    obj[FieldId{10}].as_list()[2].emplace_i32();
    auto bar2 = fromObjectStruct<Tag>(obj);
    EXPECT_TRUE(bar2.field_3()->empty());
    EXPECT_EQ(bar2.field_4(), bar.field_4());
  }
  {
    auto obj = asValueStruct<Tag>(bar).as_object();
    obj[FieldId{20}].as_object()[FieldId{2}].emplace_i32();
    auto bar2 = fromObjectStruct<Tag>(obj);
    EXPECT_EQ(bar2.field_3(), bar.field_3());
    EXPECT_EQ(bar2.field_4()->field_1(), 42);
    EXPECT_EQ(bar2.field_4()->field_2(), "");
  }
}

TEST(ObjectTest, MaybeAny) {
  using facebook::thrift::lib::test::Bar;
  auto any = type::AnyData::toAny(Bar{}).toThrift();
  auto obj = asValueStruct<type::struct_t<type::AnyStruct>>(any).as_object();
  EXPECT_TRUE(maybeAny(obj));
  {
    auto obj2 = obj;
    obj2[FieldId{4}].emplace_binary();
    EXPECT_FALSE(maybeAny(obj2));
  }
  {
    auto obj2 = obj;
    obj2[FieldId{5}].emplace_binary();
    EXPECT_TRUE(maybeAny(obj2));
  }
  {
    auto obj2 = obj;
    obj2.erase(FieldId{1});
    EXPECT_FALSE(maybeAny(obj2));
  }
  {
    auto obj2 = obj;
    obj2.erase(FieldId{2});
    EXPECT_FALSE(maybeAny(obj2));
  }
  {
    auto obj2 = obj;
    obj2.erase(FieldId{3});
    EXPECT_FALSE(maybeAny(obj2));
  }
  {
    auto obj2 = obj;
    obj2[FieldId{1}].emplace_binary();
    EXPECT_FALSE(maybeAny(obj2));
  }
  {
    auto obj2 = obj;
    obj2[FieldId{2}].emplace_binary();
    EXPECT_FALSE(maybeAny(obj2));
  }
  {
    auto obj2 = obj;
    obj2[FieldId{3}].emplace_object();
    EXPECT_FALSE(maybeAny(obj2));
  }

  // Test if Field{1} does not look like a TypeStruct
  {
    auto obj2 = obj;
    obj2[FieldId{1}].as_object()[FieldId{1}].emplace_binary();
    EXPECT_FALSE(maybeAny(obj2));
  }
  {
    auto obj2 = obj;
    obj2[FieldId{1}].as_object()[FieldId{1}].emplace_binary();
    EXPECT_FALSE(maybeAny(obj2));
  }

  obj = asValueStruct<type::struct_t<Bar>>(Bar{}).as_object();
  EXPECT_FALSE(maybeAny(obj));
}

TEST(ObjectTest, EmptyIOBufPtr) {
  auto obj = asValueStruct<type::struct_c>(
                 facebook::thrift::lib::test::EmptyIOBufPtr{})
                 .as_object();
  EXPECT_EQ(obj.size(), 1);
  EXPECT_EQ(obj[FieldId{1}].as_binary().to<std::string>(), "");
}

} // namespace
} // namespace apache::thrift::protocol
