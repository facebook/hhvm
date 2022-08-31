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

#include <folly/portability/GTest.h>
#include <thrift/conformance/cpp2/internal/AnyStructSerializer.h>
#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/test/AdapterTest.h>
#include <thrift/test/testset/Testset.h>

using namespace ::apache::thrift::conformance;

using detail::protocol_reader_t;
using detail::protocol_writer_t;

namespace apache::thrift::op {
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
} // namespace
} // namespace apache::thrift::op
