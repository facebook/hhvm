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
using NativeValue = experimental::NativeValue;

using I8 = experimental::PrimitiveTypes::I8;
using I16 = experimental::PrimitiveTypes::I16;
using I32 = experimental::PrimitiveTypes::I32;
using I64 = experimental::PrimitiveTypes::I64;
using Float = experimental::PrimitiveTypes::Float;
using Double = experimental::PrimitiveTypes::Double;
using NativeObject = experimental::NativeObject;
using Bytes = experimental::PrimitiveTypes::Bytes;
using ValueHolder = experimental::ValueHolder;

// ---- Random utils ---- //
template <typename T>
T random_val();

template <>
bool random_val<bool>() {
  return folly::Random::randBool(0.5);
}

template <>
I8 random_val<I8>() {
  return folly::Random::rand32();
}

template <>
I16 random_val<I16>() {
  return folly::Random::rand32();
}

template <>
I32 random_val<I32>() {
  return folly::Random::rand32();
}

template <>
I64 random_val<I64>() {
  return folly::Random::rand64();
}

template <>
Float random_val<Float>() {
  return folly::Random::randDouble01();
}

template <>
Double random_val<Double>() {
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
  return Bytes{random_val<std::string>()};
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
testset::struct_empty random_val<testset::struct_empty>() {
  return testset::struct_empty{};
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
  test_value_holder_type<bool>();
}

TEST(ValueHolderTest, Byte) {
  test_value_holder_type<I8>();
}

TEST(ValueHolderTest, I16) {
  test_value_holder_type<I16>();
}

TEST(ValueHolderTest, I32) {
  test_value_holder_type<I32>();
}

TEST(ValueHolderTest, I64) {
  test_value_holder_type<I64>();
}

TEST(ValueHolderTest, Float) {
  test_value_holder_type<Float>();
}

TEST(ValueHolderTest, Double) {
  test_value_holder_type<Double>();
}

TEST(ValueHolderTest, Bytes) {
  test_value_holder_type<Bytes>();
}

TEST(ObjectIterator, object_with_primitive_fields) {
  NativeObject obj;
  I32 int_val_1 = random_val<I32>();
  obj.emplace(1, NativeValue{int_val_1});
  I32 int_val_2 = random_val<I32>();
  obj[2] = NativeValue{int_val_2};
  {
    ASSERT_EQ(obj.size(), 2);
    for (const auto& [field_id, field_val] : obj) {
      ASSERT_TRUE(field_val.is_type<I32>());
      ASSERT_EQ(
          field_val.as_type<I32>(), field_id == 1 ? int_val_1 : int_val_2);
    }
  }

  {
    NativeObject obj2{obj};
    ASSERT_EQ(obj2.size(), 2);
    for (const auto& [field_id, field_val] : obj2) {
      ASSERT_TRUE(field_val.is_type<I32>());
      ASSERT_EQ(
          field_val.as_type<I32>(), field_id == 1 ? int_val_1 : int_val_2);
    }
  }

  {
    NativeObject obj3(std::move(obj));
    ASSERT_EQ(obj3.size(), 2);
    for (const auto& [field_id, field_val] : obj3) {
      ASSERT_TRUE(field_val.is_type<I32>());
      ASSERT_EQ(
          field_val.as_type<I32>(), field_id == 1 ? int_val_1 : int_val_2);
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
NativeObject deserialize(folly::IOBuf& buf) {
  return experimental::parseObject<
      conformance::detail::protocol_reader_t<Protocol>>(buf);
}

template <StandardProtocol Protocol, typename T>
NativeObject testSerDe(const T& t) {
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

template <typename T>
void assertListType() {
  T t;
  using ListFieldTy =
      std::remove_cvref_t<typename decltype(t.field_1())::value_type>;
  using ListElemTy = typename ListFieldTy::value_type;

  auto list_val = ListFieldTy{};
  for (int i = 0; i < 10; ++i) {
    list_val.push_back(random_val<ListElemTy>());
  }
  t.field_1().emplace(list_val);

  const auto obj = testSerDe<StandardProtocol::Binary>(t);
  ASSERT_EQ(obj.size(), 1);
  const NativeValue& field_obj = obj.at(1);

  ASSERT_TRUE(field_obj.is_list());
  using ResultListTy = experimental::detail::list_t<ListElemTy>;
  ASSERT_TRUE(field_obj.as_list().is_type<ResultListTy>());
  const auto& objList = field_obj.as_list().as_type<ResultListTy>();
  if constexpr (std::is_same_v<ResultListTy, experimental::ListOf<Bytes>>) {
    ASSERT_EQ(list_val.size(), objList.size());
    for (size_t i = 0; i < list_val.size(); ++i) {
      ASSERT_EQ(Bytes{list_val[i]}, objList[i]);
    }
  } else {
    ASSERT_EQ(objList, list_val);
  }
}

template <typename... Ts>
void assertListTypes() {
  (assertListType<Ts>(), ...);
}

TEST(NativeObjectTest, list_with_primitive_fields) {
  assertListTypes<
      testset::struct_list_bool,
      testset::struct_list_byte,
      testset::struct_list_i16,
      testset::struct_list_i32,
      testset::struct_list_i64,
      testset::struct_list_float,
      testset::struct_list_double,
      testset::struct_list_binary,
      testset::struct_list_string>();
}

template <typename T>
void assertValueListType() {
  T t;
  using OuterListTy =
      std::remove_cvref_t<typename decltype(t.field_1())::value_type>;
  using InnerContainerTy = typename OuterListTy::value_type;
  using ListElemTy = typename InnerContainerTy::value_type;
  constexpr bool isStructTy = std::is_same_v<ListElemTy, testset::struct_empty>;

  auto list_val = OuterListTy{};
  for (int i = 0; i < 10; ++i) {
    auto inner_container = InnerContainerTy{};
    for (int j = 0; j < 10; ++j) {
      if constexpr (isStructTy) {
        inner_container.emplace_back();
      } else {
        inner_container.push_back(random_val<ListElemTy>());
      }
    }
    list_val.push_back(std::move(inner_container));
  }
  t.field_1().emplace(list_val);

  const auto obj = testSerDe<StandardProtocol::Binary>(t);
  ASSERT_EQ(obj.size(), 1);
  const NativeValue& field_obj = obj.at(1);

  ASSERT_TRUE(field_obj.is_list());
  const auto& valueList = field_obj.as_list().as_list_of_value();
  for (size_t i = 0; i < list_val.size(); ++i) {
    ASSERT_TRUE(valueList.at(i).is_list());
    const auto& innerList = valueList.at(i).as_list();

    using ResultingContainerTy = std::conditional_t<
        isStructTy,
        experimental::ListOf<NativeObject>,
        experimental::detail::list_t<ListElemTy>>;

    ASSERT_TRUE(innerList.is_type<ResultingContainerTy>());
    const auto& values = innerList.as_type<ResultingContainerTy>();
    ASSERT_EQ(values.size(), list_val[i].size());
    for (size_t j = 0; j < list_val[i].size(); ++j) {
      if constexpr (!isStructTy) {
        ASSERT_EQ(values.at(j), list_val[i][j]);
      }
    }
  }
}

template <typename... Ts>
void assertValueListTypes() {
  (assertValueListType<Ts>(), ...);
}

TEST(NativeObjectTest, list_with_boxed_fields) {
  assertValueListTypes<
      testset::struct_list_list_bool,
      testset::struct_list_list_byte,
      testset::struct_list_list_i16,
      testset::struct_list_list_i32,
      testset::struct_list_list_i64,
      testset::struct_list_list_float,
      testset::struct_list_list_double,
      testset::struct_list_list_binary,
      testset::struct_list_list_string,
      testset::struct_list_list_struct_empty>();
}

template <typename T>
experimental::SetOf<T> into_set(const std::set<T>& set) {
  experimental::SetOf<T> newSet;
  for (const auto& val : set) {
    newSet.insert(val);
  }
  return newSet;
}

template <typename Key, typename Value>
experimental::detail::map_t<Key, Value> into_map(
    const std::map<Key, Value>& map) {
  experimental::detail::map_t<Key, Value> newMap;
  for (const auto& [key, val] : map) {
    newMap.emplace(key, val);
  }
  return newMap;
}

template <typename T>
void assertSetType() {
  T t;
  using SetFieldTy =
      std::remove_cvref_t<typename decltype(t.field_1())::value_type>;
  using SetElemTy = typename SetFieldTy::value_type;

  auto set_val = SetFieldTy{};
  for (int i = 0; i < 1; ++i) {
    set_val.emplace(random_val<SetElemTy>());
  }
  t.field_1().emplace(set_val);

  const auto obj = testSerDe<StandardProtocol::Binary>(t);
  ASSERT_EQ(obj.size(), 1);
  const NativeValue& field_obj = obj.at(1);

  using ResultSetTy = std::conditional_t<
      apache::thrift::is_thrift_class_v<SetElemTy>,
      experimental::SetOf<NativeObject>,
      experimental::detail::set_t<SetElemTy>>;

  using ResultElemTy = typename ResultSetTy::value_type;

  ASSERT_TRUE(field_obj.is_set());
  const auto& objSet = field_obj.as_set().as_type<ResultSetTy>();
  ASSERT_EQ(objSet.size(), set_val.size());
  for (const auto& item : set_val) {
    if constexpr (std::is_same_v<ResultElemTy, ValueHolder>) {
      const NativeValue& obj_in_set = *objSet.begin();
      if (const auto* list = obj_in_set.if_list();
          list && list->is_list_of_i32()) {
        if constexpr (std::is_same_v<SetElemTy, std::vector<std::int32_t>>) {
          EXPECT_EQ(item, list->as_list_of_i32());
        } else {
          FAIL() << "Expected list type";
        }
      } else if (const auto* set = obj_in_set.if_set();
                 set && set->is_set_of_i32()) {
        if constexpr (std::is_same_v<SetElemTy, std::set<std::int32_t>>) {
          EXPECT_EQ(into_set(item), set->as_set_of_i32());
        } else {
          FAIL() << "Expected set type";
        }
      } else if (const auto* map = obj_in_set.if_map()) {
        const auto* map_string_i32 = map->if_type<experimental::MapOf<
            experimental::Bytes,
            experimental::ValueHolder>>();
        if constexpr (std::is_same_v<
                          SetElemTy,
                          std::map<std::string, std::int32_t>>) {
          EXPECT_EQ(into_map(item), *map_string_i32);
        } else {
          FAIL() << "Expected map type";
        }
      } else {
        ASSERT_TRUE(false);
      }
    } else if constexpr (std::is_same_v<ResultElemTy, NativeObject>) {
      NativeObject strct{};
      ASSERT_TRUE(objSet.contains(strct));
    } else if constexpr (std::is_same_v<ResultElemTy, Bytes>) {
      ASSERT_TRUE(objSet.contains(Bytes{item}));
    } else {
      ASSERT_TRUE(objSet.contains(item));
    }
  }
}

template <typename... Ts>
void assertSetTypes() {
  (assertSetType<Ts>(), ...);
}

TEST(NativeObjectTest, set_fields) {
  assertSetTypes<
      testset::struct_set_bool,
      testset::struct_set_byte,
      testset::struct_set_i16,
      testset::struct_set_i32,
      testset::struct_set_i64,
      testset::struct_set_float,
      testset::struct_set_double,
      testset::struct_set_binary,
      testset::struct_set_string,
      testset::struct_set_struct_empty,
      testset::struct_set_set_i32>();
}

template <typename T>
void assertMapType() {
  T t;
  using MapFieldTy =
      std::remove_cvref_t<typename decltype(t.field_1())::value_type>;
  using MapKeyTy = typename MapFieldTy::key_type;
  using MapValueTy = typename MapFieldTy::mapped_type;

  auto map_value = MapFieldTy{};
  for (int i = 0; i < 10; ++i) {
    map_value.emplace(random_val<MapKeyTy>(), random_val<MapValueTy>());
  }
  t.field_1().emplace(map_value);

  const auto obj = testSerDe<StandardProtocol::Binary>(t);
  ASSERT_EQ(obj.size(), 1);
  const NativeValue& field_obj = obj.at(1);

  using ResultMapTy = experimental::detail::map_t<MapKeyTy, MapValueTy>;
  using ResultValueTy = typename ResultMapTy::mapped_type;

  ASSERT_TRUE(field_obj.is_map());
  const auto& resultMap = field_obj.as_map().as_type<ResultMapTy>();
  ASSERT_EQ(resultMap.size(), map_value.size());

  for (const auto& [key, value] : map_value) {
    if constexpr (std::is_same_v<experimental::ValueHolder, ResultValueTy>) {
      if constexpr (std::is_same_v<MapKeyTy, std::string>) {
        Bytes objKey = Bytes{key};
        const auto it = resultMap.find(objKey);
        ASSERT_NE(it, resultMap.end());
        const experimental::ValueHolder& inner_val = it->second;
        ASSERT_EQ(inner_val.as_type<MapValueTy>(), value);
      } else {
        const auto& primVal = folly::get_or_throw(resultMap, key);
        ASSERT_EQ(
            std::get<experimental::detail::native_value_type_t<MapValueTy>>(
                primVal),
            value);
      }
    } else if constexpr (std::is_same_v<
                             experimental::ValueHolder,
                             ResultValueTy>) {
      const auto& val = folly::get_or_throw(resultMap, key);
      ASSERT_TRUE(val.template is_type<MapValueTy>());
      const auto& wrappedValue = val.template as_type<MapValueTy>();
      ASSERT_EQ(wrappedValue, value);
    } else {
      static_assert(false, "Not implemented");
    }
  }
}

template <typename... Ts>
void assertMapTypes() {
  (assertMapType<Ts>(), ...);
}

TEST(NativeObjectTest, map_string_to_primitive) {
  assertMapTypes<
      testset::struct_map_string_bool,
      testset::struct_map_string_byte,
      testset::struct_map_string_i16,
      testset::struct_map_string_i32,
      testset::struct_map_string_i64,
      testset::struct_map_string_float,
      testset::struct_map_string_double,
      testset::struct_map_string_string,
      testset::struct_map_string_binary>();
}

// ------- Type traits tests to verify the MapOf type system ------- //

template <
    typename ResultKey,
    typename ResultValue,
    typename Key,
    typename Value>
constexpr bool native_map_type_is_v = std::is_same_v<
    experimental::detail::map_t<Key, Value>,
    experimental::MapOf<ResultKey, ResultValue>>;

template <
    typename ResultKey,
    typename ResultValue,
    typename Key,
    typename... Values>
constexpr bool
    native_map_type_is_v<ResultKey, ResultValue, Key, std::tuple<Values...>> =
        (native_map_type_is_v<ResultKey, ResultValue, Key, Values> && ...);

template <typename Key, typename Value>
constexpr bool native_map_is_fallback_v = std::is_same_v<
    experimental::MapOf<ValueHolder, ValueHolder>,
    experimental::detail::map_t<Key, Value>>;

template <typename Key, typename... Values>
constexpr bool native_map_is_fallback_v<Key, std::tuple<Values...>> =
    (native_map_is_fallback_v<Key, Values> && ...);

using PT = experimental::PrimitiveTypes;
using I8 = PT::I8;
using I16 = PT::I16;
using I32 = PT::I32;
using I64 = PT::I64;
using Float = PT::Float;
using Double = PT::Double;
using Bytes = PT::Bytes;
using NativeList = experimental::NativeList;
using NativeSet = experimental::NativeSet;
using NativeMap = experimental::NativeMap;

using primitives_t =
    std::tuple<PT::Bool, I8, I16, I32, I64, Float, Double, Bytes>;
using non_primitives_t = std::tuple<NativeList, NativeSet, NativeMap>;

static_assert(
    native_map_type_is_v<I8, ValueHolder, I8, non_primitives_t>,
    "Specialization of I8 -> ValueHolder");
static_assert(
    native_map_type_is_v<I16, ValueHolder, I16, non_primitives_t>,
    "Specialization of I16 -> ValueHolder");
static_assert(
    native_map_type_is_v<I32, ValueHolder, I32, non_primitives_t>,
    "Specialization of I32 -> ValueHolder");
static_assert(
    native_map_type_is_v<I64, ValueHolder, I64, non_primitives_t>,
    "Specialization of I64 -> ValueHolder");
static_assert(
    native_map_type_is_v<Float, ValueHolder, Float, non_primitives_t>,
    "Specialization of Float -> ValueHolder");
static_assert(
    native_map_type_is_v<Double, ValueHolder, Double, non_primitives_t>,
    "Specialization of Double -> ValueHolder");
static_assert(
    native_map_type_is_v<Bytes, ValueHolder, Bytes, non_primitives_t>,
    "Specialization of Bytes -> ValueHolder");
static_assert(
    native_map_type_is_v<Bytes, ValueHolder, std::string, non_primitives_t>,
    "Specialization of String -> ValueHolder (Converts to Bytes)");
static_assert(
    native_map_is_fallback_v<NativeObject, primitives_t>,
    "Fallback for Object -> PrimitiveValue");
static_assert(
    native_map_is_fallback_v<NativeObject, non_primitives_t>,
    "Fallback for Object -> T");

// ---- Nested container serde ---- //

TEST(NativeObjectTest, nested_map_list) {
  NativeMap map{experimental::MapOf<PT::I32, ValueHolder>{}};
  map.as_type<experimental::MapOf<PT::I32, ValueHolder>>().emplace(
      42, ValueHolder{experimental::make_list_of<PT::Double>(12.0)});
  NativeValue val{std::move(map)};

  auto serialized =
      experimental::serializeValue<apache::thrift::BinaryProtocolWriter>(val);
  NativeValue decoded = experimental::parseValue<
      apache::thrift::BinaryProtocolReader,
      apache::thrift::type::map_c>(*serialized);
  EXPECT_EQ(decoded, val);
}

TEST(NativeObjectTest, nested_map_map) {
  NativeMap map{experimental::MapOf<PT::I32, ValueHolder>{}};
  map.as_type<experimental::MapOf<PT::I32, ValueHolder>>().emplace(
      42, experimental::make_map_of<PT::I32, PT::Double>(1, 2.0));
  NativeValue val{std::move(map)};

  auto serialized =
      experimental::serializeValue<apache::thrift::BinaryProtocolWriter>(val);
  NativeValue decoded = experimental::parseValue<
      apache::thrift::BinaryProtocolReader,
      apache::thrift::type::map_c>(*serialized);
  EXPECT_EQ(decoded, val);
}

TEST(NativeObjectTest, nested_map_set) {
  NativeMap map{experimental::MapOf<PT::I32, ValueHolder>{}};
  // TODO(sadroeck) - Replace with make_set_of once it is implemented
  map.as_type<experimental::MapOf<PT::I32, ValueHolder>>().emplace(
      42, ValueHolder{NativeSet{experimental::SetOf<PT::Double>{12.0}}});
  NativeValue val{std::move(map)};

  auto serialized =
      experimental::serializeValue<apache::thrift::BinaryProtocolWriter>(val);
  NativeValue decoded = experimental::parseValue<
      apache::thrift::BinaryProtocolReader,
      apache::thrift::type::map_c>(*serialized);
  EXPECT_EQ(decoded, val);
}

// ---- make_list_of tests ---- //

TEST(NativeObjectTest, make_list_of_bool) {
  const auto list = experimental::make_list_of(true);
  ASSERT_TRUE(list.is_list_of_bool());
  ASSERT_EQ(list.as_list_of_bool()[0], true);
}

TEST(NativeObjectTest, make_list_of_i8) {
  const auto list = experimental::make_list_of(int8_t{1});
  ASSERT_TRUE(list.is_list_of_i8());
  ASSERT_EQ(list.as_list_of_i8()[0], 1);
}

TEST(NativeObjectTest, make_list_of_i16) {
  const auto list = experimental::make_list_of(int16_t{1});
  ASSERT_TRUE(list.is_list_of_i16());
  ASSERT_EQ(list.as_list_of_i16()[0], 1);
}

TEST(NativeObjectTest, make_list_of_i32) {
  const auto list = experimental::make_list_of(int32_t{1});
  ASSERT_TRUE(list.is_list_of_i32());
  ASSERT_EQ(list.as_list_of_i32()[0], 1);
}

TEST(NativeObjectTest, make_list_of_i64) {
  const auto list = experimental::make_list_of(int64_t{1});
  ASSERT_TRUE(list.is_list_of_i64());
  ASSERT_EQ(list.as_list_of_i64()[0], 1);
}

TEST(NativeObjectTest, make_list_of_float) {
  const auto list = experimental::make_list_of(float{1.0});
  ASSERT_TRUE(list.is_list_of_float());
  ASSERT_EQ(list.as_list_of_float()[0], 1.0);
}

TEST(NativeObjectTest, make_list_of_double) {
  const auto list = experimental::make_list_of(double{1.0});
  ASSERT_TRUE(list.is_list_of_double());
  ASSERT_EQ(list.as_list_of_double()[0], 1.0);
}

TEST(NativeObjectTest, make_list_string) {
  const auto list = experimental::make_list_of(std::string{"foo"});
  ASSERT_TRUE(list.is_list_of_bytes());
  ASSERT_EQ(list.as_list_of_bytes()[0].as_string_view(), "foo");
}

TEST(NativeObjectTest, make_list_of_binary) {
  const auto list = experimental::make_list_of(Bytes{std::string_view{"foo"}});
  ASSERT_TRUE(list.is_list_of_bytes());
  ASSERT_EQ(list.as_list_of_bytes()[0], Bytes{std::string_view{"foo"}});
}

TEST(NativeObjectTest, make_list_of_of_lists) {
  const auto list = experimental::make_list_of(NativeList{});
  ASSERT_TRUE(list.is_list_of_value());
  ASSERT_TRUE(list.as_list_of_value()[0].is_list());
}

TEST(NativeObjectTest, make_list_of_of_sets) {
  const auto list = experimental::make_list_of(NativeSet{});
  ASSERT_TRUE(list.is_list_of_value());
  ASSERT_TRUE(list.as_list_of_value()[0].is_set());
}

TEST(NativeObjectTest, make_list_of_of_maps) {
  const auto list = experimental::make_list_of(NativeMap{});
  ASSERT_TRUE(list.is_list_of_value());
  ASSERT_TRUE(list.as_list_of_value()[0].is_map());
}

TEST(NativeObjectTest, make_list_of_native_value_i32) {
  NativeValue nativeValue = NativeValue{I32{42}};
  const auto list = experimental::make_list_of(nativeValue);
  ASSERT_TRUE(list.is_list_of_i32());
  ASSERT_EQ(list.as_list_of_i32()[0], 42);
}

// ---- asValueStruct testcases ---- //

TEST(NativeObjectAsValueStructTest, bool) {
  testset::struct_bool t;
  t.field_1().emplace(true);
  const auto val = experimental::asValueStruct<apache::thrift::type::bool_t>(
      t.field_1().value());
  ASSERT_TRUE(val.is_bool());
  ASSERT_EQ(val.as_bool(), true);
}

TEST(NativeObjectAsValueStructTest, byte) {
  testset::struct_byte t;
  t.field_1().emplace(42);
  const auto val = experimental::asValueStruct<apache::thrift::type::byte_t>(
      t.field_1().value());
  ASSERT_TRUE(val.is_byte());
  ASSERT_EQ(val.as_byte(), 42);
}

TEST(NativeObjectAsValueStructTest, i16) {
  testset::struct_i16 t;
  t.field_1().emplace(42);
  const auto val = experimental::asValueStruct<apache::thrift::type::i16_t>(
      t.field_1().value());
  ASSERT_TRUE(val.is_i16());
  ASSERT_EQ(val.as_i16(), 42);
}

TEST(NativeObjectAsValueStructTest, i32) {
  testset::struct_i32 t;
  t.field_1().emplace(42);
  const auto val = experimental::asValueStruct<apache::thrift::type::i32_t>(
      t.field_1().value());
  ASSERT_TRUE(val.is_i32());
  ASSERT_EQ(val.as_i32(), 42);
}

TEST(NativeObjectAsValueStructTest, i64) {
  testset::struct_i64 t;
  t.field_1().emplace(42);
  const auto val = experimental::asValueStruct<apache::thrift::type::i64_t>(
      t.field_1().value());
  ASSERT_TRUE(val.is_i64());
  ASSERT_EQ(val.as_i64(), 42);
}

TEST(NativeObjectAsValueStructTest, float) {
  testset::struct_float t;
  t.field_1().emplace(3.1415f);
  const auto val = experimental::asValueStruct<apache::thrift::type::float_t>(
      t.field_1().value());
  ASSERT_TRUE(val.is_float());
  ASSERT_EQ(val.as_float(), 3.1415f);
}

TEST(NativeObjectAsValueStructTest, double) {
  testset::struct_double t;
  t.field_1().emplace(3.141592653589793);
  const auto val = experimental::asValueStruct<apache::thrift::type::double_t>(
      t.field_1().value());
  ASSERT_TRUE(val.is_double());
  ASSERT_EQ(val.as_double(), 3.141592653589793);
}

TEST(NativeObjectAsValueStructTest, string) {
  testset::struct_string t;
  t.field_1().emplace("hello");
  const auto val = experimental::asValueStruct<apache::thrift::type::string_t>(
      t.field_1().value());
  ASSERT_TRUE(val.is_binary());
  ASSERT_EQ(val.as_binary(), "hello");
}

TEST(NativeObjectAsValueStructTest, binary) {
  testset::struct_binary t;
  t.field_1().emplace("hello");
  const auto val = experimental::asValueStruct<apache::thrift::type::binary_t>(
      t.field_1().value());
  ASSERT_TRUE(val.is_binary());
  ASSERT_EQ(val.as_binary().as_string_view(), "hello");
}

TEST(NativeObjectAsValueStructTest, list_of_i16) {
  testset::struct_list_i16 t;
  std::vector<std::int16_t> list_val = {1, 2, 4, 16};
  t.field_1().emplace(list_val);
  const auto val = experimental::asValueStruct<
      apache::thrift::type::list<apache::thrift::type::i16_t>>(
      t.field_1().value());
  ASSERT_TRUE(val.is_list());
  ASSERT_TRUE(val.as_list().is_list_of_i16());
  ASSERT_EQ(val.as_list().as_list_of_i16(), list_val);
}

TEST(NativeObjectAsValueStructTest, list_of_bytes) {
  testset::struct_list_binary t;
  std::vector<std::string> list_val = {"hello", "world", "!"};
  t.field_1().emplace(list_val);
  const auto val = experimental::asValueStruct<
      apache::thrift::type::list<apache::thrift::type::binary_t>>(
      t.field_1().value());
  ASSERT_TRUE(val.is_list());
  ASSERT_TRUE(val.as_list().is_list_of_bytes());
  experimental::ListOf<experimental::PrimitiveTypes::Bytes> list_of_bytes{
      list_val.begin(), list_val.end()};
  ASSERT_EQ(val.as_list().as_list_of_bytes(), list_of_bytes);
}

TEST(NativeObjectAsValueStructTest, set_of_i16) {
  testset::struct_set_i16 t;
  std::set<std::int16_t> set_val = {1, 2, 4, 16};
  t.field_1().emplace(set_val);
  const auto val = experimental::asValueStruct<
      apache::thrift::type::set<apache::thrift::type::i16_t>>(
      t.field_1().value());
  ASSERT_TRUE(val.is_set());
  ASSERT_TRUE(val.as_set().is_set_of_i16());
  experimental::SetOf<experimental::PrimitiveTypes::I16> set_of_i16{
      set_val.begin(), set_val.end()};
  ASSERT_EQ(val.as_set().as_set_of_i16(), set_of_i16);
}

TEST(NativeObjectAsValueStructTest, set_of_bytes) {
  testset::struct_set_binary t;
  std::set<std::string> set_val = {"hello", "world", "!"};
  t.field_1().emplace(set_val);
  const auto val = experimental::asValueStruct<
      apache::thrift::type::set<apache::thrift::type::binary_t>>(
      t.field_1().value());
  ASSERT_TRUE(val.is_set());
  ASSERT_TRUE(val.as_set().is_set_of_bytes());
  experimental::SetOf<experimental::PrimitiveTypes::Bytes> set_of_bytes{};
  for (auto&& elem : set_val) {
    set_of_bytes.emplace(elem);
  }
  ASSERT_EQ(val.as_set().as_set_of_bytes(), set_of_bytes);
}

// ---- NativeSet utilities ---- //

TEST(NativeSetTest, contains) {
  NativeSet set{experimental::SetOf<std::int32_t>{1, 2, 3}};

  // Visitor
  set.visit(
      [](experimental::SetOf<std::int32_t>& set) {
        ASSERT_TRUE(set.contains(1));
        ASSERT_TRUE(set.contains(2));
        ASSERT_TRUE(set.contains(3));
        ASSERT_FALSE(set.contains(4));
      },
      [](auto&&) { FAIL() << "Unexpected set type"; });

  // As
  {
    const auto& setI32 = set.as_set_of_i32();
    ASSERT_TRUE(setI32.contains(1));
    ASSERT_TRUE(setI32.contains(2));
    ASSERT_TRUE(setI32.contains(3));
    ASSERT_FALSE(setI32.contains(4));
  }

  // By NativeValue
  ASSERT_TRUE(set.contains(NativeValue{static_cast<std::int32_t>(1)}));
  ASSERT_TRUE(set.contains(NativeValue{static_cast<std::int32_t>(2)}));
  ASSERT_TRUE(set.contains(NativeValue{static_cast<std::int32_t>(3)}));
  ASSERT_FALSE(set.contains(NativeValue{static_cast<std::int32_t>(4)}));
  ASSERT_FALSE(set.contains(NativeValue{static_cast<std::int64_t>(1)}));
  ASSERT_FALSE(set.contains(NativeValue{static_cast<double>(1.0)}));
}

// ---- NativeMap utilities ---- //

TEST(NativeMapTest, make_map_of) {
  {
    auto map_i32_i64 =
        experimental::make_map_of(std::int32_t{1}, std::int64_t{2});
    EXPECT_EQ(map_i32_i64.size(), 1);
    EXPECT_TRUE((map_i32_i64.is_type<experimental::MapOf<I32, I64>>()));
    const auto& m = map_i32_i64.as_type<experimental::MapOf<I32, I64>>();
    EXPECT_EQ(m.at(1).as_type<PT::I64>(), 2);
  }

  {
    auto map_string_to_listI32 = experimental::make_map_of(
        std::string{"foo"}, experimental::make_list_of(std::int32_t{1}));
    EXPECT_EQ(map_string_to_listI32.size(), 1);
    EXPECT_TRUE((map_string_to_listI32
                     .is_type<experimental::MapOf<PT::Bytes, NativeList>>()));
    const auto& m = map_string_to_listI32
                        .as_type<experimental::MapOf<PT::Bytes, NativeList>>();
    EXPECT_EQ(
        m.at(PT::Bytes{std::string_view{"foo"}}).as_list().as_list_of_i32()[0],
        1);
  }
}

TEST(NativeMapTest, emplace) {
  using M = experimental::detail::map_t<std::int32_t, std::int64_t>;

  // Visitor specialized
  {
    NativeMap map{M{}};
    map.visit(
        [](M& map) {
          map.emplace(1, 2);
          map.emplace(3, 4);
        },
        [](auto&&) { FAIL() << "Unexpected map type"; });
    ASSERT_EQ(map.size(), 2);
  }

  // By NativeValue specialized
  {
    NativeMap map{M{}};
    map.emplace(
        NativeValue{static_cast<std::int32_t>(1)},
        NativeValue{static_cast<std::int64_t>(2)});
    map.emplace(
        NativeValue{static_cast<std::int32_t>(3)},
        NativeValue{static_cast<std::int64_t>(4)});
    ASSERT_EQ(map.size(), 2);
  }

  // emplace specialized
  {
    NativeMap map{};
    map.emplace(static_cast<std::int32_t>(1), static_cast<std::int64_t>(2));
    map.emplace(static_cast<std::int32_t>(3), static_cast<std::int64_t>(4));
    ASSERT_EQ(map.size(), 2);
    ASSERT_TRUE((map.is_type<experimental::MapOf<PT::I32, PT::I64>>()));
  }

  // Emplace generic
  {
    NativeMap map{};
    map.emplace(
        NativeValue{static_cast<std::int32_t>(1)},
        NativeValue{static_cast<std::int64_t>(2)});
    map.emplace(
        NativeValue{static_cast<std::int32_t>(3)},
        NativeValue{static_cast<std::int64_t>(4)});
    ASSERT_EQ(map.size(), 2);
    ASSERT_TRUE((map.is_type<experimental::MapOf<PT::I32, PT::I64>>()));
  }
}

TEST(NativeMapTest, insert_or_assign) {
  using M = experimental::detail::map_t<std::int32_t, std::int64_t>;

  // Visitor specialized
  {
    NativeMap map{M{}};
    map.visit(
        [](M& map) {
          map.insert_or_assign(1, NativeValue{static_cast<std::int64_t>(2)});
          map.insert_or_assign(
              1,
              NativeValue{static_cast<std::int64_t>(
                  3)}); // This should update the value for key 1
        },
        [](auto&&) { FAIL() << "Unexpected map type"; });
    ASSERT_EQ(map.size(), 1);
    ASSERT_EQ((map.as_type<M>().at(1).as_type<PT::I64>()), 3);
  }

  // By NativeValue specialized
  {
    NativeMap map{M{}};
    map.insert_or_assign(
        NativeValue{static_cast<std::int32_t>(1)},
        NativeValue{static_cast<std::int64_t>(2)});
    map.insert_or_assign(
        NativeValue{static_cast<std::int32_t>(1)},
        NativeValue{static_cast<std::int64_t>(3)}); // Update
    ASSERT_EQ(map.size(), 1);
    ASSERT_EQ((map.as_type<M>().at(1).as_type<PT::I64>()), 3);
  }

  // By NativeValue generic
  {
    NativeMap map{};
    map.insert_or_assign(
        static_cast<std::int32_t>(1), static_cast<std::int64_t>(2));
    map.insert_or_assign(
        static_cast<std::int32_t>(1),
        static_cast<std::int64_t>(3)); // Update
    ASSERT_EQ(map.size(), 1);
    ASSERT_TRUE((map.is_type<experimental::MapOf<PT::I32, PT::I64>>()));
    ASSERT_EQ(
        (map.as_type<experimental::MapOf<PT::I32, PT::I64>>()
             .at(1)
             .as_type<PT::I64>()),
        3);
  }
}
