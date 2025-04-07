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

#include <thrift/conformance/cpp2/internal/AnyStructSerializer.h>
#include <thrift/lib/cpp2/protocol/NativeObject.h>
#include <thrift/test/testset/gen-cpp2/testset_types.h>

using namespace ::testing;
namespace testset = apache::thrift::test::testset;
namespace conformance = apache::thrift::conformance;
namespace experimental = apache::thrift::protocol::experimental;

using StandardProtocol = apache::thrift::conformance::StandardProtocol;
using NativeValue = experimental::Value;
using Object = experimental::Object;
using Bytes = experimental::Bytes;
using String = experimental::String;
using ValueHolder = experimental::ValueHolder;

// ---- Random utils ---- //
template <typename T>
T random_val();

template <>
experimental::Bool random_val<experimental::Bool>() {
  return folly::Random::randBool(0.5);
}

template <>
experimental::I8 random_val<experimental::I8>() {
  return folly::Random::rand32();
}

template <>
experimental::I16 random_val<experimental::I16>() {
  return folly::Random::rand32();
}

template <>
experimental::I32 random_val<experimental::I32>() {
  return folly::Random::rand32();
}

template <>
experimental::I64 random_val<experimental::I64>() {
  return folly::Random::rand64();
}

template <>
experimental::Float random_val<experimental::Float>() {
  return folly::Random::randDouble01();
}

template <>
experimental::Double random_val<experimental::Double>() {
  return folly::Random::randDouble01();
}

template <>
std::string random_val<std::string>() {
  std::string str;
  str.resize(16);
  folly::Random::secureRandom(str.data(), str.size());
  return str;
}

template <>
Bytes random_val<Bytes>() {
  return Bytes::fromStdString(random_val<std::string>());
}

template <>
std::vector<std::int32_t> random_val<std::vector<std::int32_t>>() {
  std::vector<std::int32_t> vec;
  for (size_t i = 0; i < 10; ++i) {
    vec.push_back(random_val<std::int32_t>());
  }
  return vec;
}

template <>
std::set<std::int32_t> random_val<std::set<std::int32_t>>() {
  std::set<std::int32_t> val;
  for (size_t i = 0; i < 10; ++i) {
    val.insert(random_val<std::int32_t>());
  }
  return val;
}

template <>
std::map<std::string, std::int32_t>
random_val<std::map<std::string, std::int32_t>>() {
  std::map<std::string, std::int32_t> val;
  for (size_t i = 0; i < 10; ++i) {
    val.emplace(random_val<std::string>(), random_val<std::int32_t>());
  }
  return val;
}

template <>
testset::struct_i32 random_val<testset::struct_i32>() {
  using T = std::decay_t<decltype(testset::struct_i32{}.field_1().value())>;
  testset::struct_i32 val{};
  val.field_1().emplace(random_val<T>());
  return val;
}

// ---- ValueHolder testcases ---- //

TEST(ValueHolderTest, empty) {
  ValueHolder val{};
  std::ignore = val;
}

template <typename T, typename Wrapper = T>
void test_value_holder_type() {
  Wrapper orig_val{random_val<T>()};
  ValueHolder val{NativeValue{orig_val}};
  const NativeValue& v = val;
  ASSERT_TRUE(v.is_type<T>());
  ASSERT_EQ(v.as_type<T>(), orig_val);
  ValueHolder val2{std::move(val)};
  const NativeValue& v2 = val2;
  ASSERT_TRUE(v2.is_type<T>());
  ASSERT_EQ(v2.as_type<T>(), orig_val);
  ValueHolder val3{v2};
  const NativeValue& v3 = val3;
  ASSERT_TRUE(v3.is_type<T>());
  ASSERT_EQ(v3.as_type<T>(), orig_val);
  ASSERT_EQ(v3, v2);
}

TEST(ValueHolderTest, Bool) {
  test_value_holder_type<experimental::Bool>();
}

TEST(ValueHolderTest, Byte) {
  test_value_holder_type<experimental::I8>();
}

TEST(ValueHolderTest, I16) {
  test_value_holder_type<experimental::I16>();
}

TEST(ValueHolderTest, I32) {
  test_value_holder_type<experimental::I32>();
}

TEST(ValueHolderTest, I64) {
  test_value_holder_type<experimental::I64>();
}

TEST(ValueHolderTest, Float) {
  test_value_holder_type<experimental::Float>();
}

TEST(ValueHolderTest, Double) {
  test_value_holder_type<experimental::Double>();
}

TEST(ValueHolderTest, Bytes) {
  test_value_holder_type<experimental::Bytes>();
}

TEST(ObjectIterator, object_with_primitive_fields) {
  Object obj;
  experimental::I32 int_val_1 = random_val<experimental::I32>();
  obj.emplace(1, NativeValue{int_val_1});
  experimental::I32 int_val_2 = random_val<experimental::I32>();
  obj[2] = NativeValue{int_val_2};
  {
    ASSERT_EQ(obj.size(), 2);
    for (const auto& [field_id, field_val] : obj) {
      ASSERT_TRUE(field_val.is_type<experimental::I32>());
      ASSERT_EQ(
          field_val.as_type<experimental::I32>(),
          field_id == 1 ? int_val_1 : int_val_2);
    }
  }

  {
    Object obj2{obj};
    ASSERT_EQ(obj2.size(), 2);
    for (const auto& [field_id, field_val] : obj2) {
      ASSERT_TRUE(field_val.is_type<experimental::I32>());
      ASSERT_EQ(
          field_val.as_type<experimental::I32>(),
          field_id == 1 ? int_val_1 : int_val_2);
    }
  }

  {
    Object obj3(std::move(obj));
    ASSERT_EQ(obj3.size(), 2);
    for (const auto& [field_id, field_val] : obj3) {
      ASSERT_TRUE(field_val.is_type<experimental::I32>());
      ASSERT_EQ(
          field_val.as_type<experimental::I32>(),
          field_id == 1 ? int_val_1 : int_val_2);
    }
  }
}

// ---- Object testcases ---- //

template <StandardProtocol Protocol, typename T>
std::unique_ptr<folly::IOBuf> serialize(T& s) {
  folly::IOBufQueue iobufQueue;
  conformance::detail::protocol_writer_t<Protocol> writer{};
  writer.setOutput(&iobufQueue);
  s.write(&writer);
  auto iobuf = iobufQueue.move();
  return iobuf;
}

template <StandardProtocol Protocol>
Object deserialize(folly::IOBuf& buf) {
  return experimental::parseObject<
      conformance::detail::protocol_reader_t<Protocol>>(buf);
}

template <StandardProtocol Protocol, typename T>
Object testSerDe(const T& t) {
  auto buf = serialize<Protocol>(t);
  return deserialize<Protocol>(*buf);
}

TEST(NativeObjectTest, empty) {
  const auto empty =
      testSerDe<StandardProtocol::Binary>(testset::struct_empty{});
  ASSERT_TRUE(empty.empty());
}

template <typename T>
void assertFieldType() {
  T t;
  using FieldTy =
      std::remove_cvref_t<typename decltype(t.field_1())::value_type>;
  const auto val = random_val<FieldTy>();
  t.field_1().emplace(val);
  const auto obj = testSerDe<StandardProtocol::Binary>(t);

  ASSERT_EQ(obj.size(), 1);
  const NativeValue& field_obj = obj.at(1);
  ASSERT_TRUE(field_obj.is_type<FieldTy>());
  ASSERT_EQ(field_obj.as_type<FieldTy>(), val);
}

template <typename... Ts>
void assertFieldTypes() {
  (assertFieldType<Ts>(), ...);
}

TEST(NativeObjectTest, struct_with_primitive_fields) {
  assertFieldTypes<
      testset::struct_bool,
      testset::struct_byte,
      testset::struct_i16,
      testset::struct_i32,
      testset::struct_i64,
      testset::struct_float,
      testset::struct_double,
      testset::struct_binary,
      testset::struct_string>();
}
