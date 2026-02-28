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

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <thrift/lib/cpp2/BadFieldAccess.h>
#include <thrift/lib/cpp2/BoxedValuePtr.h>
#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/gen/module_types_h.h>
#include <thrift/lib/cpp2/op/Clear.h>

#include <gtest/gtest.h>
#include <folly/Traits.h>

using apache::thrift::bad_field_access;
using apache::thrift::field_ref;
using apache::thrift::intern_boxed_field_ref;
using apache::thrift::optional_boxed_field_ref;
using apache::thrift::optional_field_ref;
using apache::thrift::terse_field_ref;
using apache::thrift::terse_intern_boxed_field_ref;
using apache::thrift::detail::boxed_ptr;
using apache::thrift::detail::boxed_value;
using apache::thrift::detail::boxed_value_ptr;

// A struct which is assignable but not constructible from int or other types
// to test forwarding in field_ref::operator=.
struct IntAssignable {
  IntAssignable& operator=(int v) noexcept {
    value = v;
    return *this;
  }

  int value = 0;
};

// A struct assignable from std::string with a non-noexcept assignment operator
// to test conditional noexcept in field_ref::operator=.
struct StringAssignable {
  StringAssignable& operator=(const std::string& v) {
    value = v;
    return *this;
  }

  std::string value;
};

struct Nested {
  int value = 0;
};

// just enough to test move semantics with field_ref
struct NullableString : private std::unique_ptr<std::string> {
  using std::unique_ptr<std::string>::unique_ptr;
  using std::unique_ptr<std::string>::operator=;

  bool operator<(const NullableString& other) const {
    if (this->get() == nullptr) {
      return other.get() != nullptr;
    }
    if (other.get() == nullptr) {
      return false;
    }
    return *this->get() < *other.get();
  }
};

class TestStruct {
 public:
  field_ref<std::string&> name() {
    return {name_, __isset.at(folly::index_constant<0>())};
  }

  field_ref<const std::string&> name() const {
    return {name_, __isset.at(folly::index_constant<0>())};
  }

  optional_field_ref<std::string&> opt_name() & {
    return {name_, __isset.at(folly::index_constant<0>())};
  }

  optional_field_ref<std::string&&> opt_name() && {
    return {std::move(name_), __isset.at(folly::index_constant<0>())};
  }

  optional_field_ref<const std::string&> opt_name() const& {
    return {name_, __isset.at(folly::index_constant<0>())};
  }

  optional_field_ref<const std::string&&> opt_name() const&& {
    return {std::move(name_), __isset.at(folly::index_constant<0>())};
  }

  terse_field_ref<std::string&> terse_name() & { return {name_}; }

  terse_field_ref<std::string&&> terse_name() && { return {std::move(name_)}; }

  terse_field_ref<const std::string&> terse_name() const& { return {name_}; }

  terse_field_ref<const std::string&&> terse_name() const&& {
    return {static_cast<const std::string&&>(name_)};
  }

  field_ref<IntAssignable&> int_assign() {
    return {int_assign_, __isset.at(folly::index_constant<1>())};
  }

  optional_field_ref<IntAssignable&> opt_int_assign() {
    return {int_assign_, __isset.at(folly::index_constant<1>())};
  }

  terse_field_ref<IntAssignable&> terse_int_assign() { return {int_assign_}; }

  optional_field_ref<std::shared_ptr<int>&> ptr_ref() {
    return {ptr_, __isset.at(folly::index_constant<2>())};
  }

  field_ref<int&> int_val() {
    return {int_val_, __isset.at(folly::index_constant<3>())};
  }

  optional_field_ref<int&> opt_int_val() {
    return {int_val_, __isset.at(folly::index_constant<3>())};
  }

  field_ref<std::unique_ptr<int>&> uptr() & {
    return {uptr_, __isset.at(folly::index_constant<4>())};
  }

  field_ref<std::unique_ptr<int>&&> uptr() && {
    return {std::move(uptr_), __isset.at(folly::index_constant<4>())};
  }

  optional_field_ref<std::unique_ptr<int>&> opt_uptr() & {
    return {uptr_, __isset.at(folly::index_constant<4>())};
  }

  optional_field_ref<std::unique_ptr<int>&&> opt_uptr() && {
    return {std::move(uptr_), __isset.at(folly::index_constant<4>())};
  }

  field_ref<std::vector<bool>&> vec() & {
    return {vec_, __isset.at(folly::index_constant<5>())};
  }

  field_ref<std::vector<bool>&&> vec() && {
    return {std::move(vec_), __isset.at(folly::index_constant<5>())};
  }

  field_ref<const std::vector<bool>&> vec() const& {
    return {vec_, __isset.at(folly::index_constant<5>())};
  }

  field_ref<const std::vector<bool>&&> vec() const&& {
    return {std::move(vec_), __isset.at(folly::index_constant<5>())};
  }

  terse_field_ref<std::vector<bool>&> terse_vec() & { return {vec_}; }

  terse_field_ref<std::vector<bool>&&> terse_vec() && {
    return {std::move(vec_)};
  }

  terse_field_ref<const std::vector<bool>&> terse_vec() const& {
    return {vec_};
  }

  terse_field_ref<const std::vector<bool>&&> terse_vec() const&& {
    return {std::move(vec_)};
  }

  optional_field_ref<Nested&> opt_nested() & {
    return {nested_, __isset.at(folly::index_constant<6>())};
  }

  terse_field_ref<int&> terse_int_val() & { return {int_val_}; }

  terse_field_ref<int&&> terse_int_val() && { return {std::move(int_val_)}; }

  terse_field_ref<const int&> terse_int_val() const& { return {int_val_}; }

  terse_field_ref<const int&&> terse_int_val() const&& {
    return {static_cast<const int&&>(int_val_)};
  }

  field_ref<std::map<NullableString, int>&> map() & {
    return {map_, __isset.at(folly::index_constant<7>())};
  }

  field_ref<std::map<NullableString, int>&&> map() && {
    return {std::move(map_), __isset.at(folly::index_constant<7>())};
  }

  field_ref<const std::map<NullableString, int>&> map() const& {
    return {map_, __isset.at(folly::index_constant<7>())};
  }

  field_ref<const std::map<NullableString, int>&&> map() const&& {
    return {std::move(map_), __isset.at(folly::index_constant<7>())};
  }

 private:
  std::string name_ = "default";
  IntAssignable int_assign_;
  std::shared_ptr<int> ptr_;
  int int_val_;
  std::unique_ptr<int> uptr_;
  std::vector<bool> vec_;
  Nested nested_;
  std::map<NullableString, int> map_;

  apache::thrift::detail::isset_bitset<8> __isset{};
};

class TestStructBoxedValuePtr {
 public:
  auto opt_name() & {
    return optional_boxed_field_ref<boxed_value_ptr<std::string>&>{name_};
  }

  auto opt_name() && {
    return optional_boxed_field_ref<boxed_value_ptr<std::string>&&>{
        std::move(name_)};
  }

  auto opt_name() const& {
    return optional_boxed_field_ref<const boxed_value_ptr<std::string>&>{name_};
  }

  auto opt_name() const&& {
    return optional_boxed_field_ref<const boxed_value_ptr<std::string>&&>{
        std::move(name_)};
  }

  auto opt_int_assign() {
    return optional_boxed_field_ref<boxed_value_ptr<IntAssignable>&>{
        int_assign_};
  }

  auto ptr_ref() {
    return optional_boxed_field_ref<boxed_value_ptr<std::shared_ptr<int>>&>{
        ptr_};
  }

  auto opt_int_val() {
    return optional_boxed_field_ref<boxed_value_ptr<int>&>{int_val_};
  }

  auto opt_uptr() & {
    return optional_boxed_field_ref<boxed_value_ptr<std::unique_ptr<int>>&>{
        uptr_};
  }

  auto opt_uptr() && {
    return optional_boxed_field_ref<boxed_value_ptr<std::unique_ptr<int>>&&>{
        std::move(uptr_)};
  }

  auto opt_nested() & {
    return optional_boxed_field_ref<boxed_value_ptr<Nested>&>{nested_};
  }

 private:
  boxed_value_ptr<std::string> name_;
  boxed_value_ptr<std::string> name_default_ = "default";
  boxed_value_ptr<IntAssignable> int_assign_;
  boxed_value_ptr<IntAssignable> int_assign_default_ = IntAssignable{};
  boxed_value_ptr<std::shared_ptr<int>> ptr_;
  boxed_value_ptr<int> int_val_;
  boxed_value_ptr<int> int_val_default_ = 0;
  boxed_value_ptr<std::unique_ptr<int>> uptr_;
  boxed_value_ptr<Nested> nested_;
};

class ThriftStruct {
 public:
  using __fbthrift_cpp2_type = ThriftStruct;
  static constexpr bool __fbthrift_cpp2_is_union = false;
  terse_field_ref<std::string&> name() & { return {name_}; }
  terse_field_ref<const std::string&> name() const& { return {name_}; }
  terse_field_ref<std::string&&> name() && { return {std::move(name_)}; }
  terse_field_ref<const std::string&&> name() const&& {
    return {std::move(name_)};
  }
  void __fbthrift_clear() { name_.clear(); }

 private:
  std::string name_ = "default";

  friend bool operator==(const ThriftStruct& lhs, const ThriftStruct& rhs) {
    return lhs.name_ == rhs.name_;
  }

  friend void PrintTo(const ThriftStruct& obj, std::ostream* os) {
    *os << "ThriftStruct{name=\"" << obj.name_ << "\"}";
  }
};

namespace {

enum class FieldQualifier { Fill, Optional, Terse };

} // namespace

template <FieldQualifier Qual>
class TestStructInternBoxedValue {
 public:
  static constexpr FieldQualifier qual = Qual;

  auto struct_field1() & {
    if constexpr (Qual == FieldQualifier::Fill) {
      return intern_boxed_field_ref<boxed_value<ThriftStruct>&>{
          __fbthrift_struct_field1,
          apache::thrift::op::getDefault<ThriftStruct>,
          __isset.at(folly::index_constant<0>())};
    } else {
      return terse_intern_boxed_field_ref<boxed_value<ThriftStruct>&>{
          __fbthrift_struct_field1,
          apache::thrift::op::getDefault<ThriftStruct>};
    }
  }
  auto struct_field1() && {
    if constexpr (Qual == FieldQualifier::Fill) {
      return intern_boxed_field_ref<boxed_value<ThriftStruct>&&>{
          std::move(__fbthrift_struct_field1),
          apache::thrift::op::getDefault<ThriftStruct>,
          __isset.at(folly::index_constant<0>())};
    } else {
      return terse_intern_boxed_field_ref<boxed_value<ThriftStruct>&&>{
          std::move(__fbthrift_struct_field1),
          apache::thrift::op::getDefault<ThriftStruct>};
    }
  }
  auto struct_field1() const& {
    if constexpr (Qual == FieldQualifier::Fill) {
      return intern_boxed_field_ref<const boxed_value<ThriftStruct>&>{
          std::as_const(__fbthrift_struct_field1),
          apache::thrift::op::getDefault<ThriftStruct>,
          __isset.at(folly::index_constant<0>())};
    } else {
      return terse_intern_boxed_field_ref<const boxed_value<ThriftStruct>&>{
          std::as_const(__fbthrift_struct_field1),
          apache::thrift::op::getDefault<ThriftStruct>};
    }
  }
  auto struct_field1() const&& {
    if constexpr (Qual == FieldQualifier::Fill) {
      return intern_boxed_field_ref<const boxed_value<ThriftStruct>&&>{
          std::move(__fbthrift_struct_field1),
          apache::thrift::op::getDefault<ThriftStruct>,
          __isset.at(folly::index_constant<0>())};
    } else {
      return terse_intern_boxed_field_ref<const boxed_value<ThriftStruct>&&>{
          std::move(__fbthrift_struct_field1),
          apache::thrift::op::getDefault<ThriftStruct>};
    }
  }

  auto struct_field2() & {
    if constexpr (Qual == FieldQualifier::Fill) {
      return intern_boxed_field_ref<boxed_value<ThriftStruct>&>{
          __fbthrift_struct_field2,
          apache::thrift::op::getDefault<ThriftStruct>,
          __isset.at(folly::index_constant<1>())};
    } else {
      return terse_intern_boxed_field_ref<boxed_value<ThriftStruct>&>{
          __fbthrift_struct_field2,
          apache::thrift::op::getDefault<ThriftStruct>};
    }
  }
  auto struct_field2() && {
    if constexpr (Qual == FieldQualifier::Fill) {
      return intern_boxed_field_ref<boxed_value<ThriftStruct>&&>{
          std::move(__fbthrift_struct_field2),
          apache::thrift::op::getDefault<ThriftStruct>,
          __isset.at(folly::index_constant<1>())};
    } else {
      return terse_intern_boxed_field_ref<boxed_value<ThriftStruct>&&>{
          std::move(__fbthrift_struct_field2),
          apache::thrift::op::getDefault<ThriftStruct>};
    }
  }
  auto struct_field2() const& {
    if constexpr (Qual == FieldQualifier::Fill) {
      return intern_boxed_field_ref<const boxed_value<ThriftStruct>&>{
          std::as_const(__fbthrift_struct_field2),
          apache::thrift::op::getDefault<ThriftStruct>,
          __isset.at(folly::index_constant<1>())};
    } else {
      return terse_intern_boxed_field_ref<const boxed_value<ThriftStruct>&>{
          std::as_const(__fbthrift_struct_field2),
          apache::thrift::op::getDefault<ThriftStruct>};
    }
  }
  auto struct_field2() const&& {
    if constexpr (Qual == FieldQualifier::Fill) {
      return intern_boxed_field_ref<const boxed_value<ThriftStruct>&&>{
          std::move(__fbthrift_struct_field2),
          apache::thrift::op::getDefault<ThriftStruct>,
          __isset.at(folly::index_constant<1>())};
    } else {
      return terse_intern_boxed_field_ref<const boxed_value<ThriftStruct>&&>{
          std::move(__fbthrift_struct_field2),
          apache::thrift::op::getDefault<ThriftStruct>};
    }
  }

 private:
  boxed_value<ThriftStruct> __fbthrift_struct_field1{
      boxed_value<ThriftStruct>::fromStaticConstant(
          &apache::thrift::op::getDefault<ThriftStruct>())};
  boxed_value<ThriftStruct> __fbthrift_struct_field2{
      boxed_value<ThriftStruct>::fromStaticConstant(
          &apache::thrift::op::getDefault<ThriftStruct>())};
  apache::thrift::detail::isset_bitset<2> __isset{};
};

template <class FieldRef>
bool is_set(FieldRef s) {
  EXPECT_EQ(
      s.is_set(),
      apache::thrift::is_non_optional_field_set_manually_or_by_serializer(s));
  return s.is_set();
}

// TODO(dokwon): Clean up FieldRefTest using TYPED_TEST.
TEST(field_ref_test, access_default_value) {
  auto s = TestStruct();
  EXPECT_EQ(*s.name(), "default");
}

TEST(field_ref_test, has_value) {
  auto s = TestStruct();
  EXPECT_FALSE(is_set(s.name()));
  s.name() = "foo";
  EXPECT_TRUE(is_set(s.name()));
}

TEST(field_ref_test, assign) {
  auto s = TestStruct();
  EXPECT_FALSE(is_set(s.name()));
  EXPECT_EQ(*s.name(), "default");
  s.name() = "foo";
  EXPECT_TRUE(is_set(s.name()));
  EXPECT_EQ(*s.name(), "foo");
}

TEST(field_ref_test, copy_from) {
  auto s = TestStruct();
  auto s2 = TestStruct();
  s.name() = "foo";
  s.name().copy_from(s2.name());
  EXPECT_FALSE(is_set(s.name()));
  s2.name() = "foo";
  s.name().copy_from(s2.name());
  EXPECT_TRUE(is_set(s.name()));
  EXPECT_EQ(*s.name(), "foo");
}

TEST(field_ref_test, copy_from_const) {
  auto s = TestStruct();
  auto s2 = TestStruct();
  const auto& s_const = s2;
  s2.name() = "foo";
  s.name().copy_from(s_const.name());
  EXPECT_TRUE(is_set(s.name()));
  EXPECT_EQ(*s.name(), "foo");
}

TEST(field_ref_test, copy_from_other_type) {
  auto s = TestStruct();
  auto s2 = TestStruct();
  s2.int_val() = 42;
  s.int_assign().copy_from(s2.int_val());
  EXPECT_TRUE(is_set(s.int_assign()));
  EXPECT_EQ(s.int_assign()->value, 42);
}

template <template <typename> class FieldRef>
void check_is_assignable() {
  using IntAssignableRef = FieldRef<IntAssignable&>;
  static_assert(std::is_assignable<IntAssignableRef, int>::value);
  static_assert(!std::is_assignable<IntAssignableRef, std::string>::value);
  static_assert(std::is_nothrow_assignable<IntAssignableRef, int>::value);

  using StringAssignableRef = FieldRef<StringAssignable&>;
  static_assert(!std::is_nothrow_assignable<StringAssignableRef&, int>::value);
}

TEST(field_ref_test, is_assignable) {
  check_is_assignable<field_ref>();
}

TEST(field_ref_test, assign_forwards) {
  auto s = TestStruct();
  s.int_assign() = 42;
  EXPECT_TRUE(is_set(s.int_assign()));
  EXPECT_EQ(s.int_assign()->value, 42);
}

TEST(field_ref_test, construct_const_from_mutable) {
  auto s = TestStruct();
  s.name() = "foo";
  field_ref<std::string&> name = s.name();
  field_ref<const std::string&> const_name = name;
  EXPECT_TRUE(is_set(const_name));
  EXPECT_EQ(*const_name, "foo");
}

template <typename T>
constexpr bool is_const_ref() {
  return std::is_reference<T>::value &&
      std::is_const<std::remove_reference_t<T>>::value;
}

TEST(field_ref_test, const_accessors) {
  TestStruct s;
  s.name() = "bar";
  field_ref<const std::string&> name = s.name();
  EXPECT_EQ(*name, "bar");
  EXPECT_EQ(name.value(), "bar");
  EXPECT_EQ(name->size(), 3);
  static_assert(is_const_ref<decltype(*name)>());
  static_assert(is_const_ref<decltype(name.value())>());
  static_assert(is_const_ref<decltype(*name.operator->())>());
}

TEST(field_ref_test, mutable_accessors) {
  TestStruct s;
  field_ref<std::string&> name = s.name();
  *name = "foo";
  EXPECT_EQ(*name, "foo");
  name.value() = "bar";
  EXPECT_EQ(*name, "bar");
  name->assign("baz");
  EXPECT_EQ(*name, "baz");

  // Field is not marked as set but that's OK for unqualified field.
  EXPECT_FALSE(is_set(name));
}

TEST(field_ref_test, ensure) {
  TestStruct s;
  s.name().value() = "foo";
  EXPECT_FALSE(is_set(s.name()));
  s.name().ensure();
  EXPECT_TRUE(is_set(s.name()));
  EXPECT_EQ(s.name(), "foo");
}

using expander = int[];

template <template <typename, typename> class F, typename T, typename... Args>
void do_gen_pairs() {
  (void)expander{(F<T, Args>(), 0)...};
}

// Generates all possible pairs of types (T1, T2) from T (cross product) and
// invokes F<T1, T2>().
template <template <typename, typename> class F, typename... T>
void gen_pairs() {
  (void)expander{(do_gen_pairs<F, T, T...>(), 0)...};
}

template <template <typename, typename> class F>
void test_conversions() {
  gen_pairs<F, int&, const int&, int&&, const int&&>();
}

template <typename From, typename To>
struct FieldRefConversionChecker {
  static_assert(
      std::is_convertible<From, To>() ==
          std::is_convertible<field_ref<From>, field_ref<To>>(),
      "inconsistent implicit conversion");
};

TEST(field_ref_test, conversions) {
  test_conversions<FieldRefConversionChecker>();
}

TEST(field_ref_test, copy_list_initialization) {
  TestStruct s;
  s.name() = {};
}

TEST(field_ref_test, emplace) {
  TestStruct s;
  s.name().emplace({'f', 'o', 'o'});
  EXPECT_EQ(s.name().value(), "foo");
  s.name().emplace({'b', 'a', 'r'});
  EXPECT_EQ(s.name().value(), "bar");
  s.name().emplace({'b', 'a', 'z'}, std::allocator<char>());
  EXPECT_EQ(s.name().value(), "baz");
}

TEST(field_ref_test, move) {
  TestStruct s;
  s.uptr() = std::make_unique<int>(42);
  auto rawp = s.uptr()->get();
  std::unique_ptr<int> p = *std::move(s).uptr();
  EXPECT_TRUE(!*s.uptr());
  EXPECT_EQ(p.get(), rawp);
}

TEST(field_ref_test, subscript) {
  TestStruct s;
  const auto& cs = s;
  s.vec() = {false};
  EXPECT_FALSE(s.vec()[0]);
  s.vec()[0] = true;
  EXPECT_TRUE(cs.vec()[0]);
  s.terse_vec()[0] = false;
  EXPECT_FALSE(cs.terse_vec()[0]);
}

TEST(field_ref_test, subscript_map) {
  TestStruct s;
  auto key = [] { return NullableString{new std::string{"foo"}}; };
  field_ref map = s.map();
  terse_field_ref<decltype(map.value())> tmap{*s.map()};
  apache::thrift::required_field_ref<decltype(map.value())> rmap{*s.map()};
  map[key()] = 42;
  EXPECT_EQ(map[key()], 42);
  tmap[key()] = 43;
  EXPECT_EQ(tmap[key()], 43);
  rmap[key()] = 44;
  EXPECT_EQ(rmap[key()], 44);
}

TEST(field_ref_test, as_const) {
  TestStruct s;
  auto name_ref = s.name();
  static_assert(
      std::is_same_v<decltype(name_ref)::reference_type, std::string&>);
  auto const_name_ref = name_ref.as_const();
  static_assert(std::is_same_v<
                decltype(const_name_ref)::reference_type,
                const std::string&>);

  auto opt_name_ref = s.opt_name();
  static_assert(
      std::is_same_v<decltype(opt_name_ref)::reference_type, std::string&>);
  auto const_opt_name_ref = opt_name_ref.as_const();
  static_assert(std::is_same_v<
                decltype(const_opt_name_ref)::reference_type,
                const std::string&>);

  auto terse_name_ref = s.terse_name();
  static_assert(
      std::is_same_v<decltype(terse_name_ref)::reference_type, std::string&>);
  auto const_terse_name_ref = terse_name_ref.as_const();
  static_assert(std::is_same_v<
                decltype(const_terse_name_ref)::reference_type,
                const std::string&>);
}

template <typename T>
class optional_field_ref_typed_test : public testing::Test {
 public:
  using Struct = T;
};

using OptionalFieldRefTestTypes =
    ::testing::Types<TestStruct, TestStructBoxedValuePtr>;
TYPED_TEST_CASE(optional_field_ref_typed_test, OptionalFieldRefTestTypes);

TEST(optional_field_ref_test, access_default_value) {
  auto s = TestStruct();
  EXPECT_THROW(*s.opt_name(), bad_field_access);
  EXPECT_EQ(s.opt_name().value_unchecked(), "default");
}

TYPED_TEST(optional_field_ref_typed_test, assign) {
  auto s = typename TestFixture::Struct();
  EXPECT_FALSE(s.opt_name().has_value());
  s.opt_name() = "foo";
  EXPECT_TRUE(s.opt_name().has_value());
  EXPECT_EQ(*s.opt_name(), "foo");
}

TYPED_TEST(optional_field_ref_typed_test, copy_from) {
  auto s = typename TestFixture::Struct();
  auto s2 = typename TestFixture::Struct();
  s.opt_name() = "foo";
  s.opt_name().copy_from(s2.opt_name());
  EXPECT_FALSE(s.opt_name().has_value());
  s2.opt_name() = "foo";
  s.opt_name().copy_from(s2.opt_name());
  EXPECT_TRUE(s.opt_name().has_value());
  EXPECT_EQ(*s.opt_name(), "foo");
}

TYPED_TEST(optional_field_ref_typed_test, copy_from_const) {
  auto s = typename TestFixture::Struct();
  auto s2 = typename TestFixture::Struct();
  const auto& s_const = s2;
  s2.opt_name() = "foo";
  s.opt_name().copy_from(s_const.opt_name());
  EXPECT_TRUE(s.opt_name().has_value());
  EXPECT_EQ(*s.opt_name(), "foo");
}

TYPED_TEST(optional_field_ref_typed_test, copy_from_other_type) {
  auto s = typename TestFixture::Struct();
  auto s2 = typename TestFixture::Struct();
  s2.opt_int_val() = 42;
  s.opt_int_assign().copy_from(s2.opt_int_val());
  EXPECT_TRUE(s.opt_int_assign().has_value());
  EXPECT_EQ(s.opt_int_assign()->value, 42);
}

TYPED_TEST(optional_field_ref_typed_test, move_from) {
  auto s = typename TestFixture::Struct();
  auto s2 = typename TestFixture::Struct();
  s.opt_name() = "foo";
  s.opt_name().move_from(s2.opt_name());
  EXPECT_FALSE(s.opt_name().has_value());
  EXPECT_FALSE(s2.opt_name().has_value());
  s2.opt_name() = "foo";
  s.opt_name().move_from(s2.opt_name());
  EXPECT_TRUE(s.opt_name().has_value());
  EXPECT_EQ(*s.opt_name(), "foo");
  using is_optional_field_ref =
      std::is_same<typename TestFixture::Struct, TestStruct>;
  EXPECT_EQ(s2.opt_name().has_value(), is_optional_field_ref::value);
}

TYPED_TEST(optional_field_ref_typed_test, is_assignable) {
  check_is_assignable<optional_field_ref>();
}

TYPED_TEST(optional_field_ref_typed_test, assign_forwards) {
  auto s = typename TestFixture::Struct();
  s.opt_int_assign() = 42;
  EXPECT_TRUE(s.opt_int_assign().has_value());
  EXPECT_EQ(s.opt_int_assign()->value, 42);
}

TYPED_TEST(optional_field_ref_typed_test, reset) {
  auto s = typename TestFixture::Struct();
  EXPECT_FALSE(s.ptr_ref().has_value());
  s.ptr_ref().reset();
  EXPECT_FALSE(s.ptr_ref().has_value());
  auto ptr = std::make_shared<int>(42);
  s.ptr_ref() = ptr;
  EXPECT_TRUE(s.ptr_ref().has_value());
  EXPECT_EQ(ptr.use_count(), 2);
  s.ptr_ref().reset();
  EXPECT_FALSE(s.ptr_ref().has_value());
  EXPECT_EQ(ptr.use_count(), 1);
}

TYPED_TEST(optional_field_ref_typed_test, construct_const_from_mutable) {
  auto s = typename TestFixture::Struct();
  s.opt_name() = "foo";
  auto name = s.opt_name();
  const auto const_name = name;
  EXPECT_TRUE(const_name.has_value());
  EXPECT_EQ(*const_name, "foo");
}

TYPED_TEST(optional_field_ref_typed_test, const_accessors) {
  typename TestFixture::Struct s;
  s.opt_name() = "bar";
  auto name = std::as_const(s).opt_name();
  EXPECT_EQ(*name, "bar");
  EXPECT_EQ(name.value(), "bar");
  EXPECT_EQ(name->size(), 3);
  static_assert(is_const_ref<decltype(*name)>());
  static_assert(is_const_ref<decltype(name.value())>());
  static_assert(is_const_ref<decltype(*name.operator->())>());
  s.opt_uptr() = std::make_unique<int>(42);
  std::move(s).opt_uptr()->get();
}

TYPED_TEST(optional_field_ref_typed_test, mutable_accessors) {
  typename TestFixture::Struct s;
  s.opt_name() = "initial";
  auto name = s.opt_name();
  *name = "foo";
  EXPECT_EQ(*name, "foo");
  name.value() = "bar";
  EXPECT_EQ(*name, "bar");
  name->assign("baz");
  EXPECT_EQ(*name, "baz");
  std::move(s).opt_name()->assign("qux");
  EXPECT_EQ(*name, "qux");
  EXPECT_TRUE(name.has_value());
}

TYPED_TEST(optional_field_ref_typed_test, value_or) {
  typename TestFixture::Struct s;
  EXPECT_EQ("foo", s.opt_name().value_or("foo"));
  s.opt_name() = "bar";
  EXPECT_EQ("bar", s.opt_name().value_or("foo"));
  s.opt_name().reset();
  EXPECT_EQ("", s.opt_name().value_or({}));
  s.opt_name() = "bar";
  EXPECT_EQ("bar", std::move(s).opt_name().value_or("foo"));
  EXPECT_EQ("", s.opt_name().value_or("foo"));
}

TYPED_TEST(optional_field_ref_typed_test, bad_field_access) {
  typename TestFixture::Struct s;
  auto name = s.opt_name();
  EXPECT_THROW(*name = "foo", bad_field_access);
  EXPECT_THROW(name.value() = "bar", bad_field_access);
  EXPECT_THROW(name->assign("baz"), bad_field_access);
}

TYPED_TEST(optional_field_ref_typed_test, convert_to_bool) {
  typename TestFixture::Struct s;
  if (auto name = s.opt_name()) {
    EXPECT_TRUE(false);
  }
  s.opt_name() = "foo";
  if (auto name = s.opt_name()) {
    // Do nothing.
  } else {
    EXPECT_TRUE(false);
  }
  EXPECT_FALSE((std::is_convertible<decltype(s.opt_name()), bool>::value));
}

template <typename From, typename To>
struct OptionalFieldRefConversionChecker {
  static_assert(
      std::is_convertible<From, To>() ==
          std::is_convertible<
              optional_field_ref<From>,
              optional_field_ref<To>>(),
      "inconsistent implicit conversion");
};

template <typename From, typename To>
struct OptionalBoxedFieldRefConversionChecker {
  static_assert(
      std::is_convertible<From, To>::value ==
          std::is_convertible<
              optional_boxed_field_ref<From>,
              optional_boxed_field_ref<To>>::value,
      "inconsistent implicit conversion");
};

template <template <typename, typename> class F>
void test_conversions_boxed_value_ptr() {
  gen_pairs<
      F,
      boxed_value_ptr<int>&,
      const boxed_value_ptr<int>&,
      boxed_value_ptr<int>&&,
      const boxed_value_ptr<int>&&>();
}

TEST(optional_field_ref_test, conversions) {
  {
    test_conversions<OptionalFieldRefConversionChecker>();
    TestStruct s;
    optional_field_ref<std::string&> lvalue_ref = s.opt_name();
    EXPECT_FALSE(lvalue_ref);
    optional_field_ref<std::string&&> rvalue_ref =
        static_cast<optional_field_ref<std::string&&>>(lvalue_ref);
    EXPECT_FALSE(rvalue_ref);
    optional_field_ref<const std::string&&> crvalue_ref =
        static_cast<optional_field_ref<const std::string&&>>(lvalue_ref);
    EXPECT_FALSE(crvalue_ref);
  }

  {
    test_conversions_boxed_value_ptr<OptionalBoxedFieldRefConversionChecker>();
    TestStructBoxedValuePtr s;
    optional_boxed_field_ref<boxed_value_ptr<std::string>&> lvalue_ref =
        s.opt_name();
    EXPECT_FALSE(lvalue_ref);
    optional_boxed_field_ref<boxed_value_ptr<std::string>&&> rvalue_ref =
        static_cast<optional_boxed_field_ref<boxed_value_ptr<std::string>&&>>(
            lvalue_ref);
    EXPECT_FALSE(rvalue_ref);
    optional_boxed_field_ref<const boxed_value_ptr<std::string>&&> crvalue_ref =
        static_cast<
            optional_boxed_field_ref<const boxed_value_ptr<std::string>&&>>(
            lvalue_ref);
    EXPECT_FALSE(crvalue_ref);
  }
}

TYPED_TEST(optional_field_ref_typed_test, copy_list_initialization) {
  typename TestFixture::Struct s;
  s.opt_name() = {};
}

TYPED_TEST(optional_field_ref_typed_test, emplace) {
  typename TestFixture::Struct s;
  s.opt_name().emplace({'f', 'o', 'o'});
  EXPECT_EQ(s.opt_name().value(), "foo");
  s.opt_name().emplace({'b', 'a', 'r'});
  EXPECT_EQ(s.opt_name().value(), "bar");
  s.opt_name().emplace({'b', 'a', 'z'}, std::allocator<char>());
  EXPECT_EQ(s.opt_name().value(), "baz");
}

TYPED_TEST(optional_field_ref_typed_test, move) {
  typename TestFixture::Struct s;
  s.opt_uptr() = std::make_unique<int>(42);
  auto rawp = s.opt_uptr()->get();
  std::unique_ptr<int> p = *std::move(s).opt_uptr();
  EXPECT_TRUE(!*s.opt_uptr());
  EXPECT_EQ(p.get(), rawp);
}

TYPED_TEST(optional_field_ref_typed_test, get_pointer) {
  typename TestFixture::Struct s;
  EXPECT_EQ(apache::thrift::get_pointer(s.opt_name()), nullptr);
  s.opt_name() = "foo";
  EXPECT_EQ(*apache::thrift::get_pointer(s.opt_name()), "foo");
  s.opt_name() = "bar";
  EXPECT_EQ(*apache::thrift::get_pointer(s.opt_name()), "bar");
  s.opt_name().reset();
  EXPECT_EQ(apache::thrift::get_pointer(s.opt_name()), nullptr);
}

TYPED_TEST(optional_field_ref_typed_test, ensure) {
  typename TestFixture::Struct s;
  EXPECT_FALSE(s.opt_name().has_value());
  EXPECT_EQ(s.opt_name().ensure(), "");
  EXPECT_TRUE(s.opt_name());
}

TEST(optional_field_ref_test, ensure_isset_unsafe) {
  TestStruct s;
  s.opt_name().value_unchecked() = "foo";
  apache::thrift::ensure_isset_unsafe(s.opt_name());
  EXPECT_EQ(s.opt_name().value(), "foo");
}

TEST(optional_field_ref_test, ensure_isset_unsafe_deprecated) {
  TestStruct s;
  s.opt_name().value_unchecked() = "foo";
  apache::thrift::ensure_isset_unsafe_deprecated(s.opt_name());
  EXPECT_EQ(s.opt_name().value(), "foo");
}

TEST(optional_field_ref_test, alias) {
  TestStruct s;
  auto ref = s.opt_nested();
  auto aliased = apache::thrift::alias_isset(
      ref, [](auto& ref) -> auto& { return ref.value; });
  EXPECT_FALSE(aliased.has_value());
  EXPECT_FALSE(ref.has_value());
  aliased = 5;
  EXPECT_EQ(*aliased, 5);
  EXPECT_EQ(ref->value, 5);
  aliased.reset();
  EXPECT_FALSE(aliased.has_value());
  EXPECT_FALSE(ref.has_value());
}

TYPED_TEST(optional_field_ref_typed_test, copy_from_optional) {
  auto s = typename TestFixture::Struct();
  s.opt_name() = "foo";
  auto empty = std::optional<std::string>();
  s.opt_name().from_optional(empty);
  EXPECT_FALSE(s.opt_name().has_value());
  auto foo = std::optional<std::string>("foo");
  s.opt_name().from_optional(foo);
  EXPECT_TRUE(s.opt_name().has_value());
  EXPECT_EQ(*s.opt_name(), "foo");
}

TYPED_TEST(optional_field_ref_typed_test, move_from_optional) {
  auto s = typename TestFixture::Struct();
  auto p = std::make_optional(std::make_unique<int>(42));
  auto rawp = p->get();
  s.opt_uptr().from_optional(std::move(p));
  EXPECT_TRUE(p.has_value());
  EXPECT_EQ(p->get(), nullptr);
  EXPECT_EQ(s.opt_uptr()->get(), rawp);
  p.reset();
  s.opt_uptr().from_optional(std::move(p));
  EXPECT_FALSE(p.has_value());
  EXPECT_FALSE(s.opt_uptr().has_value());
}

TYPED_TEST(optional_field_ref_typed_test, to_optional) {
  typename TestFixture::Struct s;
  std::optional<std::string> opt = s.opt_name().to_optional();
  EXPECT_FALSE(opt);
  s.opt_name() = "foo";
  opt = s.opt_name().to_optional();
  EXPECT_TRUE(opt);
  EXPECT_EQ(*opt, "foo");
}

TYPED_TEST(optional_field_ref_typed_test, rvalue_ref_method) {
  typename TestFixture::Struct s;
  auto ref = std::move(s).opt_uptr();
  static_assert(
      std::is_rvalue_reference_v<typename decltype(ref)::reference_type>);
  ref = std::make_unique<int>(10);
  EXPECT_EQ(**ref, 10);
  std::unique_ptr<int> p = *ref;
  EXPECT_EQ(*p, 10);
  EXPECT_FALSE(*ref);
}

TEST(optional_boxed_field_ref_test, move_to_unique_ptr) {
  TestStructBoxedValuePtr s;
  s.opt_name().emplace();
  EXPECT_TRUE(s.opt_name().has_value());

  const std::string* addr = &s.opt_name().value();
  std::unique_ptr<std::string> ptr =
      apache::thrift::move_to_unique_ptr(s.opt_name());
  EXPECT_EQ(addr, ptr.get());
  EXPECT_FALSE(s.opt_name().has_value());
}

TEST(optional_boxed_field_ref_test, assign_from_unique_ptr) {
  TestStructBoxedValuePtr s;

  auto p = std::make_unique<std::string>("42");
  const std::string* addr = &*p;

  apache::thrift::assign_from_unique_ptr(s.opt_name(), std::move(p));

  EXPECT_TRUE(s.opt_name().has_value());
  EXPECT_EQ(&s.opt_name().value(), addr);
}

TEST(terse_field_ref_test, access_default_value) {
  auto s = TestStruct();
  EXPECT_EQ(s.terse_name(), "default");
  EXPECT_EQ(s.terse_int_val(), 0);
}

TEST(terse_field_ref_test, is_assignable) {
  check_is_assignable<terse_field_ref>();
}

TEST(terse_field_ref_test, assign) {
  auto s = TestStruct();
  EXPECT_EQ(*s.terse_name(), "default");
  s.terse_name() = "foo";
  EXPECT_EQ(*s.terse_name(), "foo");
}

TEST(terse_field_ref_test, copy_from) {
  auto s = TestStruct();
  auto s2 = TestStruct();
  s2.terse_name() = "foo";
  s.terse_name().copy_from(s2.terse_name());
  EXPECT_EQ(*s.terse_name(), "foo");
}

TEST(terse_field_ref_test, move_from) {
  auto s = TestStruct();
  auto s2 = TestStruct();
  s2.terse_name() = "foo";
  s.terse_name().move_from(s2.terse_name());
  EXPECT_EQ(*s.terse_name(), "foo");
}

TEST(terse_field_ref_test, access) {
  auto s = TestStruct();
  EXPECT_EQ(*s.terse_name(), "default");
  EXPECT_EQ(s.terse_name().value(), "default");
}

TEST(terse_field_ref_test, subscript) {
  TestStruct s;
  s.terse_vec() = {false};
  EXPECT_FALSE(s.terse_vec()[0]);
  s.terse_vec()[0] = true;
  EXPECT_TRUE(s.terse_vec()[0]);
}

TEST(terse_field_ref_test, assign_forwards) {
  auto s = TestStruct();
  s.terse_int_assign() = 42;
  EXPECT_EQ(s.terse_int_assign()->value, 42);
}

TEST(terse_field_ref_test, emplace) {
  TestStruct s;
  s.terse_name().emplace({'f', 'o', 'o'});
  EXPECT_EQ(s.terse_name(), "foo");
  s.terse_name().emplace({'b', 'a', 'r'});
  EXPECT_EQ(s.terse_name(), "bar");
  s.terse_name().emplace({'b', 'a', 'z'}, std::allocator<char>());
  EXPECT_EQ(s.terse_name(), "baz");
}

template <typename From, typename To>
struct TerseFieldRefConversionChecker {
  static_assert(
      std::is_convertible<From, To>() ==
          std::is_convertible<terse_field_ref<From>, terse_field_ref<To>>(),
      "inconsistent implicit conversion");
};

TEST(terse_field_ref_test, conversions) {
  test_conversions<TerseFieldRefConversionChecker>();
}

template <template <typename> class FieldRef>
void check_is_assignable_boxed() {
  using IntAssignableRef = FieldRef<boxed_value_ptr<IntAssignable>&>;
  static_assert(std::is_assignable<IntAssignableRef, int>::value);
  static_assert(!std::is_assignable<IntAssignableRef, std::string>::value);
}

const ThriftStruct* getInternDefaultAddress() {
  return &apache::thrift::op::getDefault<ThriftStruct>();
}

template <typename T>
class intern_boxed_field_ref_typed_test : public testing::Test {
 public:
  using Struct = T;
};

using InternBoxedFieldRefTypedTest = ::testing::Types<
    TestStructInternBoxedValue<FieldQualifier::Fill>,
    TestStructInternBoxedValue<FieldQualifier::Terse>>;
TYPED_TEST_CASE(
    intern_boxed_field_ref_typed_test, InternBoxedFieldRefTypedTest);

TYPED_TEST(intern_boxed_field_ref_typed_test, access) {
  typename TestFixture::Struct s;
  EXPECT_EQ(
      &*std::as_const(s).struct_field1(), &*std::as_const(s).struct_field2());
  EXPECT_EQ(&*std::as_const(s).struct_field1(), getInternDefaultAddress());
  EXPECT_NE(&*std::as_const(s).struct_field1(), &*s.struct_field2());
  EXPECT_NE(&*s.struct_field1(), &*s.struct_field2());
  EXPECT_NE(&*std::as_const(s).struct_field1(), getInternDefaultAddress());
  EXPECT_NE(&*std::as_const(s).struct_field2(), getInternDefaultAddress());
}

TYPED_TEST(intern_boxed_field_ref_typed_test, shallow_copy_from) {
  typename TestFixture::Struct s;
  s.struct_field1()->name() = "foo";
  // 's' owns 'struct_field1' but not 'struct_field2'.
  EXPECT_NE(
      &*std::as_const(s).struct_field1(), &*std::as_const(s).struct_field2());
  s.struct_field1().copy_from(std::as_const(s).struct_field2());
  // Performs shallow copy as 'struct_field2' is not owned.
  EXPECT_EQ(
      &*std::as_const(s).struct_field1(), &*std::as_const(s).struct_field2());
  EXPECT_EQ(&*std::as_const(s).struct_field1(), getInternDefaultAddress());
}

TYPED_TEST(intern_boxed_field_ref_typed_test, deep_copy_from) {
  typename TestFixture::Struct s;
  s.struct_field1()->name() = "foo";
  // 's' owns 'struct_field1' but not 'struct_field2'.
  EXPECT_NE(
      &*std::as_const(s).struct_field1(), &*std::as_const(s).struct_field2());
  s.struct_field2().copy_from(std::as_const(s).struct_field1());
  // Performs deep copy as 'struct_field2' is not owned.
  EXPECT_NE(
      &*std::as_const(s).struct_field1(), &*std::as_const(s).struct_field2());
  EXPECT_NE(&*std::as_const(s).struct_field1(), getInternDefaultAddress());
  EXPECT_NE(&*std::as_const(s).struct_field2(), getInternDefaultAddress());
}

TYPED_TEST(intern_boxed_field_ref_typed_test, reset) {
  typename TestFixture::Struct s;
  s.struct_field1()->name() = "foo";
  // 's' owns 'struct_field1' but not 'struct_field2'.
  EXPECT_NE(
      &*std::as_const(s).struct_field1(), &*std::as_const(s).struct_field2());
  s.struct_field1().reset();
  EXPECT_EQ(
      &*std::as_const(s).struct_field1(), &*std::as_const(s).struct_field2());
  EXPECT_EQ(&*std::as_const(s).struct_field1(), getInternDefaultAddress());
}

TYPED_TEST(intern_boxed_field_ref_typed_test, emplace) {
  typename TestFixture::Struct s;
  s.struct_field1().emplace();
  if constexpr (TestFixture::Struct::qual == FieldQualifier::Fill) {
    EXPECT_TRUE(is_set(s.struct_field1()));
  }
  EXPECT_NE(
      &*std::as_const(s).struct_field1(), &*std::as_const(s).struct_field2());
  EXPECT_NE(&*std::as_const(s).struct_field1(), getInternDefaultAddress());
}

TYPED_TEST(intern_boxed_field_ref_typed_test, has_value) {
  typename TestFixture::Struct s;
  if constexpr (TestFixture::Struct::qual == FieldQualifier::Fill) {
    EXPECT_FALSE(is_set(s.struct_field1()));
    s.struct_field1() = ThriftStruct();
    EXPECT_TRUE(is_set(s.struct_field1()));
  }
}

TYPED_TEST(intern_boxed_field_ref_typed_test, ensure) {
  typename TestFixture::Struct s;
  s.struct_field1().value().name() = "foo";
  if constexpr (TestFixture::Struct::qual == FieldQualifier::Fill) {
    EXPECT_FALSE(is_set(s.struct_field1()));
  }
  s.struct_field1().ensure();
  if constexpr (TestFixture::Struct::qual == FieldQualifier::Fill) {
    EXPECT_TRUE(is_set(s.struct_field1()));
  }
  EXPECT_EQ(s.struct_field1()->name(), "foo");
}

TYPED_TEST(intern_boxed_field_ref_typed_test, assignment) {
  typename TestFixture::Struct s;
  s.struct_field1() = apache::thrift::op::getDefault<ThriftStruct>();
  // Although this is setting with the default value, it is not interned.
  EXPECT_NE(&*std::as_const(s).struct_field1(), getInternDefaultAddress());
  EXPECT_EQ(
      *std::as_const(s).struct_field1(),
      apache::thrift::op::getDefault<ThriftStruct>());
}
