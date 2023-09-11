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

#include <type_traits>
#include <folly/portability/GTest.h>
#include <folly/sorted_vector_types.h>
#include <thrift/conformance/cpp2/internal/AnyStructSerializer.h>
#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/TypeClass.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/test/AdapterTest.h>
#include <thrift/test/testset/Testset.h>

using namespace ::apache::thrift::conformance;

using detail::protocol_reader_t;
using detail::protocol_writer_t;

namespace apache::thrift::op::detail {
namespace {
using apache::thrift::protocol::asValueStruct;
using apache::thrift::protocol::TType;
using detail::typeTagToTType;

TEST(EncodeTest, TypeTagToTType) {
  EXPECT_EQ(typeTagToTType<type::bool_t>, TType::T_BOOL);
  EXPECT_EQ(typeTagToTType<type::byte_t>, TType::T_BYTE);
  EXPECT_EQ(typeTagToTType<type::i16_t>, TType::T_I16);
  EXPECT_EQ(typeTagToTType<type::i32_t>, TType::T_I32);
  EXPECT_EQ(typeTagToTType<type::i64_t>, TType::T_I64);
  EXPECT_EQ(typeTagToTType<type::float_t>, TType::T_FLOAT);
  EXPECT_EQ(typeTagToTType<type::double_t>, TType::T_DOUBLE);
  EXPECT_EQ(typeTagToTType<type::string_t>, TType::T_UTF7);
  EXPECT_EQ(typeTagToTType<type::binary_t>, TType::T_STRING);
  EXPECT_EQ(typeTagToTType<type::enum_t<void>>, TType::T_I32);
  EXPECT_EQ(typeTagToTType<type::struct_t<void>>, TType::T_STRUCT);
  EXPECT_EQ(typeTagToTType<type::exception_t<void>>, TType::T_STRUCT);
  EXPECT_EQ(typeTagToTType<type::union_t<void>>, TType::T_STRUCT);
  EXPECT_EQ(typeTagToTType<type::list<type::string_t>>, TType::T_LIST);
  EXPECT_EQ(
      (typeTagToTType<type::set<type::list<type::bool_t>>>), TType::T_SET);
  EXPECT_EQ(
      (typeTagToTType<type::map<type::i32_t, type::list<type::bool_t>>>),
      TType::T_MAP);
  // test adapted
  EXPECT_EQ(
      (typeTagToTType<type::adapted<void, type::float_t>>), TType::T_FLOAT);
  EXPECT_EQ(
      (typeTagToTType<type::adapted<void, type::list<type::string_t>>>),
      TType::T_LIST);
  EXPECT_EQ(
      (typeTagToTType<type::adapted<void, type::struct_t<void>>>),
      TType::T_STRUCT);
}

template <
    conformance::StandardProtocol Protocol,
    bool ZeroCopy,
    typename Tag,
    typename TypeClass,
    bool IsAdapted = false,
    typename T>
void testSerializedSize(T value) {
  SCOPED_TRACE(folly::pretty_name<Tag>());
  protocol_writer_t<Protocol> writer;
  uint32_t size;
  if constexpr (IsAdapted) {
    using AdaptedTag = type::adapted<test::TemplatedTestAdapter, Tag>;
    size = op::serialized_size<ZeroCopy, AdaptedTag>(
        writer, test::TemplatedTestAdapter::fromThrift(value));
  } else {
    size = op::serialized_size<ZeroCopy, Tag>(writer, value);
  }
  uint32_t expected =
      apache::thrift::detail::pm::protocol_methods<TypeClass, T>::
          template serializedSize<ZeroCopy>(writer, value);
  EXPECT_EQ(size, expected);
}

template <
    conformance::StandardProtocol Protocol,
    typename Tag,
    typename TypeClass,
    bool IsAdapted = false,
    typename T>
void testSerializedSize(T value) {
  testSerializedSize<Protocol, false, Tag, TypeClass, IsAdapted>(value);
  testSerializedSize<Protocol, true, Tag, TypeClass, IsAdapted>(value);
}

template <conformance::StandardProtocol Protocol>
void testSerializedSizeBasicTypes() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  testSerializedSize<Protocol, type::bool_t, type_class::integral>(true);
  testSerializedSize<Protocol, type::byte_t, type_class::integral>((int8_t)1);
  testSerializedSize<Protocol, type::i16_t, type_class::integral>((int16_t)1);
  testSerializedSize<Protocol, type::i32_t, type_class::integral>((int32_t)1);
  testSerializedSize<Protocol, type::i64_t, type_class::integral>((int64_t)1);
  testSerializedSize<Protocol, type::float_t, type_class::floating_point>(1.5f);
  testSerializedSize<Protocol, type::double_t, type_class::floating_point>(1.5);
  testSerializedSize<Protocol, type::string_t, type_class::string>(
      std::string("foo"));
  testSerializedSize<Protocol, type::string_t, type_class::string>(
      folly::StringPiece("foo"));
  testSerializedSize<Protocol, type::string_t, type_class::string>("foo");
  testSerializedSize<Protocol, type::binary_t, type_class::binary>(
      std::string("foo"));
  testSerializedSize<Protocol, type::binary_t, type_class::binary>(
      folly::StringPiece("foo"));
  testSerializedSize<Protocol, type::binary_t, type_class::binary>("foo");
  enum class MyEnum { value = 1 };
  testSerializedSize<Protocol, type::enum_t<MyEnum>, type_class::enumeration>(
      MyEnum::value);
}

template <conformance::StandardProtocol Protocol>
void testSerializedSizeContainers() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  testSerializedSize<
      Protocol,
      type::list<type::bool_t>,
      type_class::list<type_class::integral>>(
      std::vector<bool>{true, false, true});
  testSerializedSize<
      Protocol,
      type::set<type::bool_t>,
      type_class::set<type_class::integral>>(std::set<bool>{true, false});
  testSerializedSize<
      Protocol,
      type::map<type::string_t, type::byte_t>,
      type_class::map<type_class::string, type_class::integral>>(
      std::map<std::string, int8_t>{
          {std::string("foo"), 1}, {std::string("foo"), 2}});
}

enum class EmptyEnum : std::int16_t {};

template <conformance::StandardProtocol Protocol>
void testSerializedSizeCppType() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  // test cpp_type with primitive
  testSerializedSize<
      Protocol,
      type::cpp_type<int, type::i16_t>,
      type_class::integral>((int16_t)1);

  // test strongly typed integer
  testSerializedSize<
      Protocol,
      type::cpp_type<EmptyEnum, type::i16_t>,
      type_class::integral>(static_cast<EmptyEnum>(1));

  {
    // test cpp_type with list
    using T = std::deque<int64_t>;
    auto value = T{1, 2, 3};
    using Tag = type::list<type::i64_t>;
    testSerializedSize<
        Protocol,
        type::cpp_type<T, Tag>,
        type_class::list<type_class::integral>>(value);
  }
  {
    // test cpp_type with set
    using T = std::unordered_set<std::string>;
    auto value = T{"foo", "bar"};
    using Tag = type::set<type::string_t>;
    testSerializedSize<
        Protocol,
        type::cpp_type<T, Tag>,
        type_class::set<type_class::string>>(value);
  }
  {
    // test cpp_type with map
    using T = std::unordered_map<std::string, int32_t>;
    auto value = T{{"foo", 1}, {"bar", 2}};
    using Tag = type::map<type::string_t, type::i32_t>;
    testSerializedSize<
        Protocol,
        type::cpp_type<T, Tag>,
        type_class::map<type_class::string, type_class::integral>>(value);
  }
}

template <
    conformance::StandardProtocol Protocol,
    typename Struct,
    typename Tag,
    typename TypeClass,
    bool IsAdapted = false,
    typename T>
void testSerializedSizeObject(T value) {
  Struct s;
  s.field_1_ref() = value;
  testSerializedSize<Protocol, Tag, TypeClass, IsAdapted>(s);
}

template <conformance::StandardProtocol Protocol>
void testSerializedSizeStruct() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  using Struct =
      test::testset::struct_with<type::map<type::string_t, type::i32_t>>;
  std::map<std::string, int> mapValues = {{"one", 1}, {"four", 4}, {"two", 2}};
  testSerializedSizeObject<
      Protocol,
      Struct,
      type::struct_t<Struct>,
      type_class::structure>(mapValues);
  using Union = test::testset::union_with<type::set<type::string_t>>;
  std::set<std::string> setValues = {"foo", "bar", "baz"};
  testSerializedSizeObject<
      Protocol,
      Union,
      type::union_t<Union>,
      type_class::structure>(setValues);
  using Exception = test::testset::exception_with<type::i64_t>;
  testSerializedSizeObject<
      Protocol,
      Exception,
      type::exception_t<Exception>,
      type_class::structure>(1);
}

template <conformance::StandardProtocol Protocol>
void testSerializedSizeAdapted() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  testSerializedSize<Protocol, type::string_t, type_class::string, true>("foo");
  testSerializedSize<
      Protocol,
      type::set<type::i32_t>,
      type_class::set<type_class::integral>,
      true>(std::set<int32_t>{1, 2, 3});
  using Struct = test::testset::struct_with<type::i32_t>;
  testSerializedSizeObject<
      Protocol,
      Struct,
      type::struct_t<Struct>,
      type_class::structure,
      true>(1);
  using Union = test::testset::union_with<type::i32_t>;
  testSerializedSizeObject<
      Protocol,
      Union,
      type::union_t<Union>,
      type_class::structure,
      true>(1);
}

TEST(SerializedSizeTest, SerializedSizeBasicTypes) {
  testSerializedSizeBasicTypes<conformance::StandardProtocol::Binary>();
  testSerializedSizeBasicTypes<conformance::StandardProtocol::Compact>();
}

TEST(SerializedSizeTest, SerializedSizeContainers) {
  testSerializedSizeContainers<conformance::StandardProtocol::Binary>();
  testSerializedSizeContainers<conformance::StandardProtocol::Compact>();
}

TEST(SerializedSizeTest, SerializedSizeCppType) {
  testSerializedSizeCppType<conformance::StandardProtocol::Binary>();
  testSerializedSizeCppType<conformance::StandardProtocol::Compact>();
}

TEST(SerializedSizeTest, SerializedSizeStruct) {
  testSerializedSizeStruct<conformance::StandardProtocol::Binary>();
  testSerializedSizeStruct<conformance::StandardProtocol::Compact>();
}

TEST(SerializedSizeTest, SerializedSizeAdapted) {
  testSerializedSizeAdapted<conformance::StandardProtocol::Binary>();
  testSerializedSizeAdapted<conformance::StandardProtocol::Compact>();
}

template <conformance::StandardProtocol Protocol, typename Tag, typename T>
protocol::Value encodeAndParseValue(
    T value, TType ttype, bool string_to_binary = true) {
  protocol_writer_t<Protocol> writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);
  encode<Tag>(writer, value);
  protocol_reader_t<Protocol> reader;
  auto serialized = queue.move();
  reader.setInput(serialized.get());
  return protocol::detail::parseValue(reader, ttype, string_to_binary);
}

template <conformance::StandardProtocol Protocol, typename Tag, typename T>
void testEncode(T value, bool string_to_binary = true) {
  SCOPED_TRACE(folly::pretty_name<Tag>());
  auto result = encodeAndParseValue<Protocol, Tag>(
      value, typeTagToTType<Tag>, string_to_binary);
  EXPECT_EQ(result, asValueStruct<Tag>(value));
}

template <conformance::StandardProtocol Protocol>
void testEncodeBasicTypes() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  testEncode<Protocol, type::bool_t>(true);
  testEncode<Protocol, type::byte_t>(1);
  testEncode<Protocol, type::i16_t>(1);
  testEncode<Protocol, type::i32_t>(1);
  testEncode<Protocol, type::i64_t>(1);
  testEncode<Protocol, type::float_t>(1.5);
  testEncode<Protocol, type::double_t>(1.5);
  testEncode<Protocol, type::string_t>(std::string("foo"), false);
  testEncode<Protocol, type::string_t>(folly::StringPiece("foo"), false);
  testEncode<Protocol, type::string_t>("foo", false);
  testEncode<Protocol, type::binary_t>("foo");
  testEncode<Protocol, type::enum_t<int>>(1);
}

template <conformance::StandardProtocol Protocol>
void testEncodeContainers() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  testEncode<Protocol, type::list<type::enum_t<int>>>(
      std::vector<int32_t>{1, 2, 3});
  testEncode<Protocol, type::set<type::bool_t>>(std::set<bool>{true, false});
  testEncode<Protocol, type::map<type::string_t, type::byte_t>>(
      std::map<std::string, int8_t>{{"foo", 1}, {"bar", 2}}, false);
}

template <conformance::StandardProtocol Protocol>
void testEncodeCppType() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  {
    // test cpp_type with primitive type
    auto result =
        encodeAndParseValue<Protocol, type::cpp_type<int, type::i16_t>>(
            1, TType::T_I16);
    EXPECT_EQ(result, asValueStruct<type::i16_t>(1));
  }
  {
    // test strongly typed integer
    auto result =
        encodeAndParseValue<Protocol, type::cpp_type<EmptyEnum, type::i16_t>>(
            static_cast<EmptyEnum>(1), TType::T_I16);
    EXPECT_EQ(result, asValueStruct<type::i16_t>(1));
  }
  {
    // test cpp_type with list
    using T = std::list<int64_t>;
    auto value = T{1, 2, 3};
    using Tag = type::list<type::i64_t>;
    auto result = encodeAndParseValue<Protocol, type::cpp_type<T, Tag>>(
        value, TType::T_LIST);
    EXPECT_EQ(result, asValueStruct<Tag>(value));
  }
  {
    // test cpp_type with set
    using T = std::unordered_set<std::string>;
    auto value = T{"foo", "bar"};
    using Tag = type::set<type::string_t>;
    auto result = encodeAndParseValue<Protocol, type::cpp_type<T, Tag>>(
        value, TType::T_SET, false);
    EXPECT_EQ(result, asValueStruct<Tag>(value));
  }
  {
    // test cpp_type with map
    using T = std::unordered_map<std::string, int32_t>;
    auto value = T{{"foo", 1}, {"bar", 2}};
    using Tag = type::map<type::string_t, type::i32_t>;
    auto result = encodeAndParseValue<Protocol, type::cpp_type<T, Tag>>(
        value, TType::T_MAP, false);
    EXPECT_EQ(result, asValueStruct<Tag>(value));
  }
}

// If IsAdapted is true, test op::encode with the given object with
// type::adapted<test::TemplatedTestAdapter, Tag> tag and fromThrift.
template <
    conformance::StandardProtocol Protocol,
    typename Struct,
    typename Tag,
    bool IsAdapted = false,
    typename T>
void testEncodeObject(T value) {
  SCOPED_TRACE(folly::pretty_name<Tag>());
  Struct foo;
  foo.field_1_ref() = value;
  protocol_writer_t<Protocol> w1, w2;
  folly::IOBufQueue o1, o2;
  w1.setOutput(&o1);
  w2.setOutput(&o2);
  if constexpr (IsAdapted) {
    using AdaptedTag = type::adapted<test::TemplatedTestAdapter, Tag>;
    encode<AdaptedTag>(w1, test::TemplatedTestAdapter::fromThrift(foo));
  } else {
    encode<Tag>(w1, foo);
  }
  foo.write(&w2);
  EXPECT_TRUE(folly::IOBufEqualTo{}(*o1.move(), *o2.move()));
}

template <conformance::StandardProtocol Protocol>
void testEncodeStruct() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  using Struct =
      test::testset::struct_with<type::map<type::string_t, type::i32_t>>;
  std::map<std::string, int> mapValues = {{"one", 1}, {"four", 4}, {"two", 2}};
  testEncodeObject<Protocol, Struct, type::struct_t<Struct>>(mapValues);
  using Union = test::testset::union_with<type::set<type::string_t>>;
  std::set<std::string> setValues = {"foo", "bar", "baz"};
  testEncodeObject<Protocol, Union, type::union_t<Union>>(setValues);
  using Exception = test::testset::exception_with<type::i64_t>;
  testEncodeObject<Protocol, Exception, type::exception_t<Exception>>(1);
}

template <conformance::StandardProtocol Protocol>
void testEncodeAdapted() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  {
    // test op::encode with adapted struct
    using Struct = test::testset::struct_with<type::i32_t>;
    testEncodeObject<Protocol, Struct, type::struct_t<Struct>, true>(1);
  }
  {
    // test op::encode with adapted primitive type
    using AdaptedTag = type::adapted<test::TemplatedTestAdapter, type::i16_t>;
    auto result = encodeAndParseValue<Protocol, AdaptedTag>(
        test::TemplatedTestAdapter::fromThrift(1), TType::T_I16);
    EXPECT_EQ(result, asValueStruct<type::i16_t>(1));
  }
  {
    // test op::encode with adapted container
    auto value = std::map<std::string, int32_t>{{"foo", 1}, {"bar", 2}};
    using Tag = type::map<type::string_t, type::i32_t>;
    using AdaptedTag = type::adapted<test::TemplatedTestAdapter, Tag>;
    auto result = encodeAndParseValue<Protocol, AdaptedTag>(
        test::TemplatedTestAdapter::fromThrift(value), TType::T_MAP, false);
    EXPECT_EQ(result, asValueStruct<Tag>(value));
  }
  {
    // test op::encode with Adapter::encode optimization
    using AdaptedTag = type::adapted<test::EncodeAdapter, type::i64_t>;
    test::Num value{1};
    auto result =
        encodeAndParseValue<Protocol, AdaptedTag>(value, TType::T_I64);
    EXPECT_EQ(result, asValueStruct<type::i64_t>(1));
  }
}

TEST(EncodeTest, EncodeBasicTypes) {
  testEncodeBasicTypes<conformance::StandardProtocol::Binary>();
  testEncodeBasicTypes<conformance::StandardProtocol::Compact>();
}

TEST(EncodeTest, EncodeContainers) {
  testEncodeContainers<conformance::StandardProtocol::Binary>();
  testEncodeContainers<conformance::StandardProtocol::Compact>();
}

TEST(EncodeTest, EncodeStruct) {
  testEncodeStruct<conformance::StandardProtocol::Binary>();
  testEncodeStruct<conformance::StandardProtocol::Compact>();
}

TEST(EncodeTest, EncodeCppType) {
  testEncodeCppType<conformance::StandardProtocol::Binary>();
  testEncodeCppType<conformance::StandardProtocol::Compact>();
}

TEST(EncodeTest, EncodeAdapted) {
  testEncodeAdapted<conformance::StandardProtocol::Binary>();
  testEncodeAdapted<conformance::StandardProtocol::Compact>();
}

// Encodes the given value with EncodeTag, and decodes the result with DecodeTag
// to DecodeT type using the Protocol.
template <
    conformance::StandardProtocol Protocol,
    typename EncodeTag,
    typename DecodeTag,
    typename DecodeT,
    typename EncodeT>
DecodeT encodeAndDecode(EncodeT value) {
  protocol_writer_t<Protocol> writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);
  encode<EncodeTag>(writer, value);

  protocol_reader_t<Protocol> reader;
  auto serialized = queue.move();
  reader.setInput(serialized.get());
  DecodeT result;
  decode<DecodeTag>(reader, result);
  return result;
}

template <conformance::StandardProtocol Protocol, typename Tag, typename T>
void testDecode(T value) {
  SCOPED_TRACE(folly::pretty_name<Tag>());
  EXPECT_EQ(
      (encodeAndDecode<Protocol, Tag, Tag, decltype(value)>(value)), value);
}

template <conformance::StandardProtocol Protocol>
void testDecodeBasicTypes() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  testDecode<Protocol, type::bool_t>(true);
  testDecode<Protocol, type::byte_t>((int8_t)1);
  testDecode<Protocol, type::i16_t>((int16_t)11);
  testDecode<Protocol, type::i32_t>((int32_t)11);
  testDecode<Protocol, type::i64_t>((int64_t)11);
  testDecode<Protocol, type::float_t>(1.5f);
  testDecode<Protocol, type::double_t>(1.5);
  testDecode<Protocol, type::string_t>(std::string("foo"));
  testDecode<Protocol, type::binary_t>(std::string("foo"));
  enum class MyEnum { value = 1 };
  testDecode<Protocol, type::enum_t<MyEnum>>(MyEnum::value);
}

template <conformance::StandardProtocol Protocol>
void testDecodeContainers() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  testDecode<Protocol, type::list<type::enum_t<int>>>(
      std::vector<int32_t>{1, 2, 3});
  testDecode<Protocol, type::list<type::bool_t>>(
      std::vector<bool>{true, false, true});
  testDecode<Protocol, type::set<type::bool_t>>(std::set<bool>{true, false});
  testDecode<Protocol, type::set<type::bool_t>>(
      std::unordered_set<bool>{true, false});
  testDecode<Protocol, type::map<type::string_t, type::byte_t>>(
      std::map<std::string, int8_t>{
          {std::string("foo"), 1}, {std::string("foo"), 2}});
  testDecode<Protocol, type::map<type::string_t, type::byte_t>>(
      std::unordered_map<std::string, int8_t>{
          {std::string("foo"), 1}, {std::string("foo"), 2}});
  testDecode<Protocol, type::set<type::byte_t>>(
      folly::sorted_vector_set<int8_t>{3, 1, 2});
  testDecode<Protocol, type::map<type::string_t, type::byte_t>>(
      folly::sorted_vector_map<std::string, int8_t>{
          {std::string("foo"), 1}, {std::string("foo"), 2}});

  // Test if it skips when value type doesn't match.
  {
    auto result = encodeAndDecode<
        Protocol,
        type::list<type::i32_t>,
        type::list<type::string_t>,
        std::vector<std::string>>(std::vector<int32_t>{1, 2, 3});
    EXPECT_TRUE(result.empty());
  }
  {
    auto result = encodeAndDecode<
        Protocol,
        type::set<type::i32_t>,
        type::set<type::i64_t>,
        std::set<int64_t>>(std::set<int32_t>{1, 2, 3});
    EXPECT_TRUE(result.empty());
  }
  {
    auto result = encodeAndDecode<
        Protocol,
        type::map<type::i32_t, type::bool_t>,
        type::map<type::string_t, type::bool_t>,
        std::map<std::string, bool>>(
        std::map<int32_t, bool>{{1, true}, {2, false}});
    EXPECT_TRUE(result.empty());
  }
  {
    auto result = encodeAndDecode<
        Protocol,
        type::map<type::string_t, type::bool_t>,
        type::map<type::string_t, type::i32_t>,
        std::map<std::string, int32_t>>(
        std::map<std::string, bool>{{"1", true}, {"2", false}});
    EXPECT_TRUE(result.empty());
  }
}

template <
    conformance::StandardProtocol Protocol,
    typename Struct,
    typename Tag,
    bool IsAdapted = false,
    typename T>
void testDecodeObject(T value) {
  Struct s;
  s.field_1_ref() = value;
  if constexpr (IsAdapted) {
    using AdaptedTag = type::adapted<test::TemplatedTestAdapter, Tag>;
    testDecode<Protocol, AdaptedTag>(test::TemplatedTestAdapter::fromThrift(s));
  } else {
    testDecode<Protocol, Tag>(s);
  }
}

template <conformance::StandardProtocol Protocol>
void testDecodeStruct() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  using Struct =
      test::testset::struct_with<type::map<type::string_t, type::i32_t>>;
  std::map<std::string, int> mapValues = {{"one", 1}, {"four", 4}, {"two", 2}};
  testDecodeObject<Protocol, Struct, type::struct_t<Struct>>(mapValues);
  using Union = test::testset::union_with<type::set<type::string_t>>;
  std::set<std::string> setValues = {"foo", "bar", "baz"};
  testDecodeObject<Protocol, Union, type::union_t<Union>>(setValues);
  using Exception = test::testset::exception_with<type::i64_t>;
  testDecodeObject<Protocol, Exception, type::exception_t<Exception>>(1);
}

template <conformance::StandardProtocol Protocol>
void testDecodeCppType() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  {
    // test strongly typed integer
    testDecode<Protocol, type::cpp_type<EmptyEnum, type::i16_t>>(
        static_cast<EmptyEnum>(1));
  }
  {
    // test cpp_type with list
    using T = std::deque<int64_t>;
    auto value = T{1, 2, 3};
    using Tag = type::list<type::i64_t>;
    testDecode<Protocol, type::cpp_type<T, Tag>>(value);
  }
  {
    // test cpp_type with set
    using T = std::unordered_set<std::string>;
    auto value = T{"foo", "bar"};
    using Tag = type::set<type::string_t>;
    testDecode<Protocol, type::cpp_type<T, Tag>>(value);
  }
  {
    // test cpp_type with map
    using T = std::unordered_map<std::string, int32_t>;
    auto value = T{{"foo", 1}, {"bar", 2}};
    using Tag = type::map<type::string_t, type::i32_t>;
    testDecode<Protocol, type::cpp_type<T, Tag>>(value);
  }
}

template <
    conformance::StandardProtocol Protocol,
    typename Struct,
    typename Tag,
    typename T>
void testEncodeAndDecodeFieldAdapted(T value) {
  SCOPED_TRACE(folly::pretty_name<Tag>());
  // s is used as a placeholder.
  Struct s;
  const test::AdaptedWithContext<T, Struct, 0> adapted{value};
  using AdaptedTag = type::adapted<test::TemplatedTestFieldAdapter, Tag>;
  using FieldTag = type::field<AdaptedTag, FieldContext<Struct, 0>>;

  protocol_writer_t<Protocol> writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);
  encode<AdaptedTag>(writer, adapted);

  protocol_reader_t<Protocol> reader;
  auto serialized = queue.move();
  reader.setInput(serialized.get());
  test::AdaptedWithContext<T, Struct, 0> result;
  decode<FieldTag>(reader, result, s);
  EXPECT_EQ(adapted, result);

  using AdaptedTag2 = type::adapted<test::EncodeFieldAdapter, Tag>;
  using FieldTag2 = type::field<AdaptedTag2, FieldContext<Struct, 0>>;
  encode<AdaptedTag2>(writer, adapted);

  auto serialized2 = queue.move();
  reader.setInput(serialized2.get());
  decode<FieldTag2>(reader, result, s);
  EXPECT_EQ(adapted, result);
}

template <conformance::StandardProtocol Protocol>
void testDecodeAdapted() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  using AdaptedTag = type::adapted<test::TemplatedTestAdapter, type::string_t>;
  testDecode<Protocol, AdaptedTag>(
      test::TemplatedTestAdapter::fromThrift(std::string()));
  using Struct = test::testset::struct_with<type::i32_t>;
  testDecodeObject<Protocol, Struct, type::struct_t<Struct>, true>(1);
  using Union = test::testset::union_with<type::i32_t>;
  testDecodeObject<Protocol, Union, type::union_t<Union>, true>(1);
  testDecode<Protocol, type::adapted<test::EncodeAdapter, type::i64_t>>(
      test::Num{1});
  using Struct64 = test::testset::struct_with<type::i64_t>;
  testEncodeAndDecodeFieldAdapted<Protocol, Struct64, type::i64_t>(
      static_cast<int64_t>(42));
}

TEST(DecodeTest, DecodeBasicTypes) {
  testDecodeBasicTypes<conformance::StandardProtocol::Binary>();
  testDecodeBasicTypes<conformance::StandardProtocol::Compact>();
}

TEST(DecodeTest, DecodeContainers) {
  testDecodeContainers<conformance::StandardProtocol::Binary>();
  testDecodeContainers<conformance::StandardProtocol::Compact>();
}

TEST(DecodeTest, DecodeStruct) {
  testDecodeStruct<conformance::StandardProtocol::Binary>();
  testDecodeStruct<conformance::StandardProtocol::Compact>();
}

TEST(DecodeTest, DecodeCppType) {
  testDecodeCppType<conformance::StandardProtocol::Binary>();
  testDecodeCppType<conformance::StandardProtocol::Compact>();
}

TEST(DecodeTest, DecodeAdapted) {
  testDecodeAdapted<conformance::StandardProtocol::Binary>();
  testDecodeAdapted<conformance::StandardProtocol::Compact>();
}
} // namespace

enum { UseWrite, UseStructEncode };

struct MockStruct {
  static const bool __fbthrift_cpp2_gen_json = false;
  template <class T>
  uint32_t write(T&&) const {
    return UseWrite;
  }
};

struct JSONMockStruct {
  static const bool __fbthrift_cpp2_gen_json = true;
  template <class T>
  uint32_t write(T&&) const {
    return UseWrite;
  }
};

template <>
struct StructEncode<MockStruct> {
  template <typename T, typename U>
  uint32_t operator()(T&&, const U&) const {
    return UseStructEncode;
  }
};

template <>
struct StructEncode<JSONMockStruct> {
  template <typename T, typename U>
  uint32_t operator()(T&&, const U&) const {
    return UseStructEncode;
  }
};

TEST(EncodeTest, EncodeMethod) {
  CompactProtocolWriter compact;
  BinaryProtocolWriter binary;
  SimpleJSONProtocolWriter simpleJson;
  JSONProtocolWriter json;
  DebugProtocolWriter debug;
  MockStruct mock;
  JSONMockStruct jsonMock;
  auto encodeMockStruct = op::encode<type::struct_t<MockStruct>>;
  auto encodeJSONMockStruct = op::encode<type::struct_t<JSONMockStruct>>;

  EXPECT_EQ(encodeMockStruct(compact, mock), UseWrite);
  EXPECT_EQ(encodeMockStruct(binary, mock), UseWrite);
  EXPECT_EQ(encodeMockStruct(simpleJson, mock), UseStructEncode);
  EXPECT_EQ(encodeMockStruct(json, mock), UseStructEncode);
  EXPECT_EQ(encodeMockStruct(debug, mock), UseStructEncode);
  EXPECT_EQ(encodeJSONMockStruct(compact, jsonMock), UseWrite);
  EXPECT_EQ(encodeJSONMockStruct(binary, jsonMock), UseWrite);
  EXPECT_EQ(encodeJSONMockStruct(simpleJson, jsonMock), UseWrite);
  EXPECT_EQ(encodeJSONMockStruct(json, jsonMock), UseStructEncode);
  EXPECT_EQ(encodeJSONMockStruct(debug, jsonMock), UseStructEncode);
}

} // namespace apache::thrift::op::detail
