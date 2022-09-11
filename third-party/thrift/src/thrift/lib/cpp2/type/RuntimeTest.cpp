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

#include <thrift/lib/cpp2/type/Runtime.h>

#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include <folly/io/IOBuf.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/thrift/gen-cpp2/standard_types.h>

namespace apache::thrift::type {
namespace {

// Test the exact case of AlignedPtr used by RuntimeType.
TEST(AlignedPtr, InfoPointer) {
  using info_pointer = detail::AlignedPtr<const detail::TypeInfo, 2>;
  EXPECT_EQ(~info_pointer::kMask, 3); // Everything except the last 2 bits.
  const auto* fullPtr = (const detail::TypeInfo*)(info_pointer::kMask);

  info_pointer empty, full{fullPtr, 3};
  EXPECT_FALSE(empty.get<0>());
  EXPECT_FALSE(empty.get<1>());
  EXPECT_EQ(empty.get(), nullptr);
  EXPECT_TRUE(full.get<0>());
  EXPECT_TRUE(full.get<1>());
  EXPECT_EQ(full.get(), fullPtr);

  empty.set<0>();
  full.clear<0>();
  EXPECT_TRUE(empty.get<0>());
  EXPECT_FALSE(empty.get<1>());
  EXPECT_EQ(empty.get(), nullptr);
  EXPECT_FALSE(full.get<0>());
  EXPECT_TRUE(full.get<1>());
  EXPECT_EQ(full.get(), fullPtr);

  empty.set<1>();
  full.clear<1>();
  EXPECT_TRUE(empty.get<0>());
  EXPECT_TRUE(empty.get<1>());
  EXPECT_EQ(empty.get(), nullptr);
  EXPECT_FALSE(full.get<0>());
  EXPECT_FALSE(full.get<1>());
  EXPECT_EQ(full.get(), fullPtr);

  empty.clear<0>();
  full.set<0>();
  EXPECT_FALSE(empty.get<0>());
  EXPECT_TRUE(empty.get<1>());
  EXPECT_EQ(empty.get(), nullptr);
  EXPECT_TRUE(full.get<0>());
  EXPECT_FALSE(full.get<1>());
  EXPECT_EQ(full.get(), fullPtr);
}

TEST(RuntimeRefTest, Void) {
  Ref ref;
  EXPECT_EQ(ref.type(), Type::get<void_t>());
  EXPECT_TRUE(ref.empty());
  ref.clear();
  EXPECT_TRUE(ref.empty());
  EXPECT_THROW(ref.get(FieldId{1}), std::logic_error);
  EXPECT_THROW(ref.get("field1"), std::logic_error);

  EXPECT_THROW(ref.as<string_t>(), std::bad_any_cast);
  EXPECT_TRUE(ref.tryAs<string_t>() == nullptr);
}

TEST(RuntimeRefTest, Int) {
  int32_t value = 1;
  Ref ref = Ref::to<i32_t>(value);
  EXPECT_EQ(ref.type(), Type::get<i32_t>());
  EXPECT_FALSE(ref.empty());
  EXPECT_TRUE(ref.add(ref));
  EXPECT_EQ(value, 2);
  EXPECT_EQ(ref, Ref::to<i32_t>(2));
  EXPECT_EQ(ref, Value::of<i32_t>(2));

  ref.clear();
  EXPECT_TRUE(ref.empty());
  EXPECT_EQ(value, 0);
  EXPECT_EQ(ref.as<i32_t>(), 0);
  EXPECT_EQ(ref, Ref::to<i32_t>(0));
  EXPECT_EQ(ref, Value::create<i32_t>());
}

TEST(RuntimeRefTest, List) {
  std::vector<std::string> value;
  std::string elem = "hi";
  auto ref = Ref::to<list<string_t>>(value);
  EXPECT_TRUE(ref.empty());
  EXPECT_EQ(ref.size(), 0);
  ref.append(Ref::to<string_t>(elem));
  EXPECT_THAT(value, ::testing::ElementsAre("hi"));

  EXPECT_FALSE(ref.empty());
  EXPECT_EQ(ref.size(), 1);
  EXPECT_THROW(ref[FieldId{1}], std::logic_error);
  EXPECT_THROW(ref["field1"], std::logic_error);
  EXPECT_EQ(ref[0], Ref::to<string_t>("hi"));
  EXPECT_EQ(ref[Ordinal{1}], Ref::to<string_t>("hi"));
  EXPECT_THROW(ref[1], std::out_of_range);
  EXPECT_THROW(ref.add(Ref::to<string_t>(value[0])), std::runtime_error);
  EXPECT_THROW(ref[Ref::to<i32_t>(0)], std::logic_error);

  ref.clear();
  EXPECT_TRUE(ref.empty());
  EXPECT_TRUE(value.empty());
}

TEST(RuntimeRefTest, Set) {
  std::set<std::string> value;
  auto ref = Ref::to<set<string_t>>(value);
  EXPECT_TRUE(ref.empty());
  EXPECT_EQ(ref.size(), 0);
  EXPECT_TRUE(ref.add("hi"));
  EXPECT_THAT(value, ::testing::ElementsAre("hi"));

  EXPECT_FALSE(ref.empty());
  EXPECT_EQ(ref.size(), 1);
  EXPECT_THROW(ref.get(FieldId{1}), std::runtime_error);
  EXPECT_THROW(ref.get("hi"), std::runtime_error);
  EXPECT_THROW(ref.get(Ref::to<string_t>("hi")), std::runtime_error);

  ref.clear();
  EXPECT_TRUE(ref.empty());
  EXPECT_TRUE(value.empty());
}

TEST(RuntimeRefTest, Map) {
  std::map<std::string, int> value;
  auto ref = Ref::to<map<string_t, i32_t>>(value);
  EXPECT_TRUE(ref.empty());
  EXPECT_EQ(ref.size(), 0);
  EXPECT_FALSE(ref.put("one", 1));
  EXPECT_EQ(value["one"], 1);

  EXPECT_TRUE(ref.put("one", 2));
  EXPECT_EQ(value["one"], 2);

  EXPECT_FALSE(ref.empty());
  EXPECT_EQ(ref.size(), 1);
  EXPECT_THROW(ref.put(FieldId{1}, 2), std::logic_error);
  EXPECT_THROW(ref[FieldId{1}], std::logic_error);
  EXPECT_EQ(ref["one"], 2);
  EXPECT_THROW(ref["two"], std::out_of_range);

  ref.clear();
  EXPECT_TRUE(ref.empty());
  EXPECT_TRUE(value.empty());
}

TEST(RuntimeRefTest, Map_Add) {
  std::map<std::string, int> value;
  auto ref = Ref::to<map<string_t, i32_t>>(value);

  // ensure = add if not present.
  EXPECT_EQ(ref.ensure("zero"), 0);
  EXPECT_EQ(ref.ensure("one", 1), 1);
  EXPECT_EQ(ref.ensure("one", 2), 1);
  EXPECT_EQ(value["zero"], 0);
  EXPECT_EQ(value["one"], 1);
}

TEST(RuntimeRefTest, Struct) {
  type::UriStruct actual;
  using Tag = type::struct_t<type::UriStruct>;
  auto ref = Ref::to<Tag>(actual);
  EXPECT_FALSE(ref.empty());
  EXPECT_EQ(ref.size(), 5);

  EXPECT_TRUE(ref.put(FieldId{1}, "foo"));
  EXPECT_EQ(*actual.scheme(), "foo");
  ref.clear(FieldId{1});
  EXPECT_EQ(*actual.scheme(), "");

  EXPECT_TRUE(ref.put("scheme", "bar"));
  EXPECT_EQ(*actual.scheme(), "bar");
  ref.clear("scheme");
  EXPECT_EQ(*actual.scheme(), "");

  EXPECT_THROW(ref.clear(FieldId{}), std::out_of_range);
  EXPECT_THROW(ref.clear("bad"), std::out_of_range);
  EXPECT_THROW(ref.put(FieldId{}, ""), std::out_of_range);
  EXPECT_THROW(ref.put("bad", ""), std::out_of_range);
}

TEST(RuntimeRefTest, Identical) {
  float value = 1.0f;
  auto ref = Ref::to<float_t>(value);
  EXPECT_FALSE(ref.empty());
  ref.clear();
  float zero = 0.0;
  float negZero = -0.0;
  double dblZero = 0.0;
  EXPECT_TRUE(ref.empty());
  EXPECT_TRUE(ref.identical(zero));
  EXPECT_FALSE(ref.identical(negZero));
  EXPECT_FALSE(ref.identical(dblZero));
}

TEST(RuntimeRefTest, ConstRef) {
  constexpr int32_t one = 1;
  auto ref = Ref::to<i32_t>(one);
  EXPECT_FALSE(ref.empty());
  EXPECT_TRUE(ref.identical(1));
  // Cannot be modified.
  EXPECT_THROW(ref.clear(), std::logic_error);
}

TEST(RuntimeValueTest, Void) {
  Value value;
  EXPECT_EQ(value.type(), Type::get<void_t>());
  EXPECT_TRUE(value.empty());
  value.clear();
  EXPECT_TRUE(value.empty());

  EXPECT_THROW(value.as<string_t>(), std::bad_any_cast);
  EXPECT_TRUE(value.tryAs<string_t>() == nullptr);
}

TEST(RuntimeValueTest, Int) {
  Value value = Value::of<i32_t>(1);
  EXPECT_EQ(value.type(), Type::get<i32_t>());
  EXPECT_FALSE(value.empty());
  value.clear();
  EXPECT_TRUE(value.empty());

  EXPECT_EQ(value.as<i32_t>(), 0);
  value.as<i32_t>() = 2;
  EXPECT_FALSE(value.empty());
}

TEST(RuntimeValueTest, List) {
  Value value;
  value = Value::create<list<string_t>>();
  EXPECT_TRUE(value.empty());
  value.as<list<string_t>>().emplace_back("hi");
  EXPECT_FALSE(value.empty());
  Value other(value);
  EXPECT_FALSE(other.empty());
  value.clear();
  EXPECT_TRUE(value.empty());
  EXPECT_TRUE(value.as<list<string_t>>().empty());
  value = other;
  EXPECT_FALSE(value.empty());
}

TEST(RuntimeValueTest, Identical) {
  Value value;
  value = Value::of<float_t>(1.0f);
  EXPECT_FALSE(value.empty());
  value.clear();
  EXPECT_TRUE(value.empty());
  EXPECT_TRUE(value.identical(0.0f));
  EXPECT_FALSE(value.identical(-0.0f));
  EXPECT_FALSE(value.identical(0.0));
}

TEST(RuntimeValueTest, CppType_List) {
  using T = std::list<std::string>;
  using Tag = cpp_type<T, type::list<string_t>>;
  auto list = Value::create<Tag>();
  list.append("foo");
  EXPECT_EQ(list[0], Ref::to<string_t>("foo"));
}

TEST(RuntimeValueTest, CppType_Map) {
  using T = std::unordered_map<std::string, int>;
  using Tag = cpp_type<T, type::map<string_t, i32_t>>;

  auto map = Value::create<Tag>();
  map.put("foo", 2);
  Ref value = map["foo"];
  EXPECT_EQ(value.as<i32_t>(), 2);
  EXPECT_GT(value, 1);
  EXPECT_EQ(value, value);

  auto otherMap = Value::create<Tag>();
  EXPECT_NE(map, otherMap);
  otherMap.put("bar", 2);
  EXPECT_NE(map, otherMap);

  EXPECT_THROW(map < otherMap, std::logic_error);
}

TEST(RuntimeValueTest, Float_InterOp) {
  EXPECT_EQ(Ref::to<double_t>(2.0), Ref::to<float_t>(2.0f));
  EXPECT_LT(Ref::to<float_t>(1.0), Ref::to<double_t>(2.0f));
  EXPECT_GT(
      Ref::to<double_t>(std::numeric_limits<double>::infinity()),
      Ref::to<float_t>(-std::numeric_limits<float>::infinity()));
}

TEST(RuntimeValueTest, StringBinary_InterOp) {
  using STag = type::string_t;
  using BTag = type::cpp_type<folly::IOBuf, type::binary_t>;

  auto stringHi = Value::of<STag>("hi");
  auto stringBye = Value::of<STag>("bye");

  auto binaryHiBuf = folly::IOBuf::wrapBufferAsValue("hi", 2);
  auto binaryHi = Ref::to<BTag>(binaryHiBuf);
  auto binaryByeBuf = folly::IOBuf::wrapBufferAsValue("bye", 3);
  auto binaryBye = ConstRef::to<BTag>(binaryByeBuf);

  // Compare.
  EXPECT_EQ(stringHi, binaryHi);
  EXPECT_EQ(binaryBye, stringBye);
  EXPECT_NE(binaryHi, stringBye);
  EXPECT_GT(stringHi, binaryBye);

  // TODO(afuller): Support 'assign' op.
  // binaryHi = stringBye;
  // EXPECT_LT(binaryHi, stringHi);
  // stringHi.assign(binaryBye);
  // EXPECT_EQ(stringHi.type(), stringBye.type());
  // EXPECT_EQ(binaryHi, stringHi);
  // stringHi = binaryBye;
  // EXPECT_NE(stringHi.type(), stringBye.type());
  // EXPECT_EQ(binaryHi, stringHi);
}

} // namespace
} // namespace apache::thrift::type
