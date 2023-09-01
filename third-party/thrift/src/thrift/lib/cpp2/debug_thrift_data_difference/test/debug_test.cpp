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

#include <thrift/lib/cpp2/debug_thrift_data_difference/debug.h>

#include <thrift/test/reflection/gen-cpp2/reflection_types.h>

#include <folly/portability/GTest.h>

#include <string>
#include <vector>

namespace test_cpp2 {
namespace cpp_reflection {

struct3 test_data() {
  structA a1;
  *a1.a() = 99;
  *a1.b() = "abc";
  structA a2;
  *a2.a() = 1001;
  *a2.b() = "foo";
  structA a3;
  *a3.a() = 654;
  *a3.b() = "bar";
  structA a4;
  *a4.a() = 9791;
  *a4.b() = "baz";
  structA a5;
  *a5.a() = 111;
  *a5.b() = "gaz";

  structB b1;
  *b1.c() = 1.23;
  *b1.d() = true;
  structB b2;
  *b2.c() = 9.8;
  *b2.d() = false;
  structB b3;
  *b3.c() = 10.01;
  *b3.d() = true;
  structB b4;
  *b4.c() = 159.73;
  *b4.d() = false;
  structB b5;
  *b5.c() = 468.02;
  *b5.d() = true;

  struct3 pod;

  *pod.fieldA() = 141;
  *pod.fieldB() = "this is a test";
  *pod.fieldC() = enum1::field0;
  *pod.fieldD() = enum2::field1_2;
  pod.fieldE()->set_ud(5.6);
  pod.fieldF()->set_us_2("this is a variant");
  *pod.fieldG()->field0() = 98;
  pod.fieldG()->field1() = "hello, world";
  *pod.fieldG()->field2() = enum1::field2;
  *pod.fieldG()->field3() = enum2::field0_2;
  pod.fieldG()->field4() = {};
  pod.fieldG()->field4()->set_ui(19937);
  pod.fieldG()->field5()->set_ue_2(enum1::field1);
  // fieldH intentionally left empty
  *pod.fieldI() = {3, 5, 7, 9};
  *pod.fieldJ() = {"a", "b", "c", "d"};
  *pod.fieldK() = {};
  pod.fieldL()->push_back(a1);
  pod.fieldL()->push_back(a2);
  pod.fieldL()->push_back(a3);
  pod.fieldL()->push_back(a4);
  pod.fieldL()->push_back(a5);
  *pod.fieldM() = {2, 4, 6, 8};
  *pod.fieldN() = {"w", "x", "y", "z"};
  *pod.fieldO() = {};
  *pod.fieldP() = {b1, b2, b3, b4, b5};
  *pod.fieldQ() = {{"a1", a1}, {"a2", a2}, {"a3", a3}};
  *pod.fieldR() = {};
  *pod.fieldS() = {{"123", "456"}, {"abc", "ABC"}, {"def", "DEF"}};

  return pod;
}

struct test_callback {
  explicit test_callback(std::vector<std::string>& out) : out_(out) {}

  template <typename T, typename Tag>
  void operator()(
      Tag, T const*, T const*, folly::StringPiece path, folly::StringPiece)
      const {
    out_.emplace_back(path.data(), path.size());
  }

 private:
  std::vector<std::string>& out_;
};

#define TEST_IMPL_TAG(Tag, LHS, RHS, ...)                       \
  do {                                                          \
    const std::vector<std::string> expected{__VA_ARGS__};       \
                                                                \
    std::vector<std::string> actual;                            \
    actual.reserve(expected.size());                            \
                                                                \
    if constexpr (std::is_void_v<Tag>) {                        \
      EXPECT_EQ(                                                \
          expected.empty(),                                     \
          (facebook::thrift::debug_thrift_data_difference(      \
              LHS, RHS, test_callback(actual))));               \
    } else {                                                    \
      EXPECT_EQ(                                                \
          expected.empty(),                                     \
          (facebook::thrift::debug_thrift_data_difference<Tag>( \
              LHS, RHS, test_callback(actual))));               \
    }                                                           \
                                                                \
    EXPECT_EQ(expected, actual);                                \
  } while (false)

#define TEST_IMPL(LHS, RHS, ...) TEST_IMPL_TAG(void, LHS, RHS, __VA_ARGS__)

TEST(Debug, equal) {
  TEST_IMPL(test_data(), test_data());
}

TEST(Debug, fieldA) {
  auto pod = test_data();
  *pod.fieldA() = 90;
  TEST_IMPL(pod, test_data(), "$.fieldA");
  *pod.fieldA() = 141;
  TEST_IMPL(pod, test_data());
}

TEST(Debug, fieldB) {
  auto pod = test_data();
  *pod.fieldB() = "should mismatch";
  TEST_IMPL(pod, test_data(), "$.fieldB");
  *pod.fieldB() = "this is a test";
  TEST_IMPL(pod, test_data());
}

TEST(Debug, fieldC) {
  auto pod = test_data();
  *pod.fieldC() = enum1::field2;
  TEST_IMPL(pod, test_data(), "$.fieldC");
  *pod.fieldC() = enum1::field0;
  TEST_IMPL(pod, test_data());
}

TEST(Debug, fieldE) {
  auto pod = test_data();
  pod.fieldE()->set_ui(5);
  TEST_IMPL(
      pod, test_data(), "$.fieldE.ui" /* missing */, "$.fieldE.ud" /* extra */);
  pod.fieldE() = {};
  TEST_IMPL(pod, test_data(), "$.fieldE.ud" /* extra */);
  pod.fieldE()->set_ud(4);
  TEST_IMPL(pod, test_data(), "$.fieldE.ud" /* changed */);
  pod.fieldE()->set_ud(5.6);
  TEST_IMPL(pod, test_data());
}

TEST(Debug, fieldH) {
  auto pod = test_data();
  pod.fieldH()->set_ui_2(3);
  TEST_IMPL(pod, test_data(), "$.fieldH.ui_2" /* extra */);
  pod.fieldH() = {};
  TEST_IMPL(pod, test_data());
}

TEST(Debug, fieldI) {
  auto pod = test_data();
  pod.fieldI()[0] = 4;
  TEST_IMPL(pod, test_data(), "$.fieldI[0]");
  pod.fieldI()[0] = 3;
  TEST_IMPL(pod, test_data());
  pod.fieldI()[2] = 10;
  TEST_IMPL(pod, test_data(), "$.fieldI[2]");
  pod.fieldI()->push_back(11);
  TEST_IMPL(pod, test_data(), "$.fieldI[2]", "$.fieldI[4]");
  pod.fieldI()->clear();
  TEST_IMPL(
      pod,
      test_data(),
      "$.fieldI[0]",
      "$.fieldI[1]",
      "$.fieldI[2]",
      "$.fieldI[3]");
}

TEST(Debug, fieldM) {
  auto pod = test_data();
  pod.fieldM()->clear();
  TEST_IMPL(
      pod,
      test_data(),
      "$.fieldM[2]" /* extra */,
      "$.fieldM[4]" /* extra */,
      "$.fieldM[6]" /* extra */,
      "$.fieldM[8]" /* extra */);
  pod.fieldM()->insert(11);
  pod.fieldM()->insert(12);
  pod.fieldM()->insert(13);
  pod.fieldM()->insert(14);
  TEST_IMPL(
      pod,
      test_data(),
      "$.fieldM[11]" /* missing */,
      "$.fieldM[12]" /* missing */,
      "$.fieldM[13]" /* missing */,
      "$.fieldM[14]" /* missing */,
      "$.fieldM[2]" /* extra */,
      "$.fieldM[4]" /* extra */,
      "$.fieldM[6]" /* extra */,
      "$.fieldM[8]" /* extra */);
  *pod.fieldM() = *test_data().fieldM();
  TEST_IMPL(pod, test_data());
}

TEST(Debug, fieldQ) {
  auto pod = test_data();
  pod.fieldQ()->clear();
  TEST_IMPL(
      pod,
      test_data(),
      "$.fieldQ[\"a1\"]" /* extra */,
      "$.fieldQ[\"a2\"]" /* extra */,
      "$.fieldQ[\"a3\"]" /* extra */);
  structA a1;
  *a1.a() = 1;
  *a1.b() = "1";
  structA a2;
  *a2.a() = 2;
  *a2.b() = "2";
  structA a3;
  *a3.a() = 3;
  *a3.b() = "3";
  pod.fieldQ()["A1"] = a1;
  pod.fieldQ()["A2"] = a2;
  pod.fieldQ()["A3"] = a3;
  TEST_IMPL(
      pod,
      test_data(),
      "$.fieldQ[\"A1\"]" /* missing */,
      "$.fieldQ[\"A2\"]" /* missing */,
      "$.fieldQ[\"A3\"]" /* missing */,
      "$.fieldQ[\"a1\"]" /* extra */,
      "$.fieldQ[\"a2\"]" /* extra */,
      "$.fieldQ[\"a3\"]" /* extra */);
  *pod.fieldQ() = *test_data().fieldQ();
  TEST_IMPL(pod, test_data());
}

TEST(Debug, fieldG_field0) {
  auto pod = test_data();
  *pod.fieldG()->field0() = 12;
  TEST_IMPL(pod, test_data(), "$.fieldG.field0");
  *pod.fieldG()->field0() = 98;
  TEST_IMPL(pod, test_data());
}

TEST(Debug, fieldG_field1) {
  auto pod = test_data();
  pod.fieldG()->field1() = "should mismatch";
  TEST_IMPL(pod, test_data(), "$.fieldG.field1");
  pod.fieldG()->field1() = "hello, world";
  TEST_IMPL(pod, test_data());
}

TEST(Debug, fieldG_field2) {
  auto pod = test_data();
  *pod.fieldG()->field2() = enum1::field1;
  TEST_IMPL(pod, test_data(), "$.fieldG.field2");
  *pod.fieldG()->field2() = enum1::field2;
  TEST_IMPL(pod, test_data());
}

TEST(Debug, fieldG_field5) {
  auto pod = test_data();
  pod.fieldG()->field5()->set_ui_2(5);
  TEST_IMPL(
      pod,
      test_data(),
      "$.fieldG.field5.ui_2" /* missing */,
      "$.fieldG.field5.ue_2" /* extra */);
  pod.fieldG()->field5() = {};
  TEST_IMPL(pod, test_data(), "$.fieldG.field5.ue_2" /* extra */);
  pod.fieldG()->field5()->set_ue_2(enum1::field0);
  TEST_IMPL(pod, test_data(), "$.fieldG.field5.ue_2" /* changed */);
  pod.fieldG()->field5()->set_ue_2(enum1::field1);
  TEST_IMPL(pod, test_data());
}

TEST(Debug, fieldS) {
  auto pod = test_data();
  *pod.fieldS() = {{"123", "456"}, {"abc", "ABC"}, {"ghi", "GHI"}};
  TEST_IMPL(
      pod,
      test_data(),
      "$.fieldS[\"0x676869\"]" /* missing */,
      "$.fieldS[\"0x646566\"]" /* extra  */);
}

TEST(Debug, struct_binary) {
  struct_binary lhs;
  *lhs.bi() = "hello";
  struct_binary rhs;
  *rhs.bi() = "world";

  TEST_IMPL(lhs, rhs, "$.bi");
}

namespace {
struct UniqueHelper {
  template <typename T, typename... Args>
  static std::unique_ptr<T> build(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
  }
};

struct SharedHelper {
  template <typename T, typename... Args>
  static std::shared_ptr<T> build(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
  }
};

struct SharedConstHelper {
  template <typename T, typename... Args>
  static std::shared_ptr<const T> build(Args&&... args) {
    return std::make_shared<const T>(std::forward<Args>(args)...);
  }
};
} // namespace

template <typename Structure, typename Helper>
void ref_test() {
  Structure allNull;
  allNull.aStruct_ref() = Helper::template build<structA>();
  allNull.aList_ref() = Helper::template build<std::deque<std::string>>();
  allNull.aSet_ref() =
      Helper::template build<std::unordered_set<std::string>>();
  allNull.aMap_ref() =
      Helper::template build<std::unordered_map<std::string, std::string>>();
  allNull.aUnion_ref() = Helper::template build<unionA>();
  allNull.anOptionalStruct_ref() = nullptr;
  allNull.anOptionalList_ref() = nullptr;
  allNull.anOptionalSet_ref() = nullptr;
  allNull.anOptionalMap_ref() = nullptr;
  allNull.anOptionalUnion_ref() = nullptr;

  Structure allDefault;
  allDefault.aStruct_ref() = Helper::template build<structA>();
  allDefault.anOptionalStruct_ref() = Helper::template build<structA>();
  allDefault.aList_ref() = Helper::template build<std::deque<std::string>>();
  allDefault.anOptionalList_ref() =
      Helper::template build<std::deque<std::string>>();
  allDefault.aSet_ref() =
      Helper::template build<std::unordered_set<std::string>>();
  allDefault.anOptionalSet_ref() =
      Helper::template build<std::unordered_set<std::string>>();
  allDefault.aMap_ref() =
      Helper::template build<std::unordered_map<std::string, std::string>>();
  allDefault.anOptionalMap_ref() =
      Helper::template build<std::unordered_map<std::string, std::string>>();
  allDefault.aUnion_ref() = Helper::template build<unionA>();
  allDefault.anOptionalUnion_ref() = Helper::template build<unionA>();
  TEST_IMPL(
      allNull,
      allDefault,
      "$.anOptionalStruct" /* extra */,
      "$.anOptionalList" /* extra */,
      "$.anOptionalSet" /* extra */,
      "$.anOptionalMap" /* extra */,
      "$.anOptionalUnion" /* extra */);
  TEST_IMPL(
      allDefault,
      allNull,
      "$.anOptionalStruct" /* missing */,
      "$.anOptionalList" /* missing */,
      "$.anOptionalSet" /* missing */,
      "$.anOptionalMap" /* missing */,
      "$.anOptionalUnion" /* missing */);
}

TEST(Debug, struct_ref_unique) {
  ref_test<hasRefUnique, UniqueHelper>();
}

TEST(Debug, ref_shared) {
  ref_test<hasRefShared, SharedHelper>();
}

TEST(Debug, ref_shared_const) {
  ref_test<hasRefSharedConst, SharedConstHelper>();
}

TEST(Debug, optional_members) {
  struct1 field1Set;
  field1Set.field1() = 2;
  struct1 field1Unset;
  struct1 field1SetButNotIsset;
  field1SetButNotIsset.field1().value_unchecked() = "2";
  struct1 field1SetDefault;
  apache::thrift::ensure_isset_unsafe(field1SetDefault.field1());
  TEST_IMPL(field1Set, field1Unset, "$.field1" /* missing */);
  TEST_IMPL(field1Unset, field1Set, "$.field1" /* extra */);
  TEST_IMPL(field1Set, field1SetButNotIsset, "$.field1" /* missing */);
  TEST_IMPL(field1SetButNotIsset, field1Set, "$.field1" /* extra */);
  TEST_IMPL(field1Unset, field1SetButNotIsset);
  TEST_IMPL(field1SetButNotIsset, field1Unset);
  TEST_IMPL(field1Set, field1SetDefault, "$.field1" /* different */);
  TEST_IMPL(field1SetDefault, field1Set, "$.field1" /* different */);
  TEST_IMPL(field1SetDefault, field1SetButNotIsset, "$.field1" /* missing */);
  TEST_IMPL(field1SetButNotIsset, field1SetDefault, "$.field1" /* extra */);
  TEST_IMPL(field1Unset, field1SetDefault, "$.field1" /* extra */);
  TEST_IMPL(field1SetDefault, field1Unset, "$.field1" /* missing */);
}

TEST(Debug, tc_binary_iobuf) {
  using tc = apache::thrift::type::binary_t;
  std::string x = "hello";
  std::string y = "world";
  auto xb = folly::IOBuf::wrapBuffer(x.data(), x.size());
  auto yb = folly::IOBuf::wrapBuffer(y.data(), y.size());
  xb->prependChain(std::move(yb));
  auto zb = xb->cloneCoalesced();
  TEST_IMPL_TAG(tc, xb->to<std::string>(), zb->to<std::string>());
  TEST_IMPL_TAG(tc, xb->to<std::string>(), std::string("blah"), "$");
  TEST_IMPL_TAG(tc, xb->to<folly::fbstring>(), zb->to<folly::fbstring>());
  TEST_IMPL_TAG(tc, xb->to<folly::fbstring>(), folly::fbstring("blah"), "$");
  TEST_IMPL_TAG(tc, *xb, *zb);
  TEST_IMPL_TAG(tc, xb, zb);
  TEST_IMPL_TAG(tc, xb, std::unique_ptr<folly::IOBuf>(), "$");
  TEST_IMPL_TAG(tc, xb, std::make_unique<folly::IOBuf>(), "$");
}

TEST(Debug, adapters) {
  using apache::thrift::test::AdaptedWithContext;
  using apache::thrift::test::Wrapper;

  IntStruct intStruct;

  StructWithAdaptedField x;
  x.meta() = "non-meta";
  intStruct.field() = -2;
  x.typeAdapted() = Wrapper<IntStruct>{.value = intStruct};
  intStruct.field() = -3;
  x.fieldAdapted() =
      AdaptedWithContext<IntStruct, StructWithAdaptedField, 3>{intStruct};
  x.typeAdapted2() = Wrapper<int64_t>{.value = -4};
  x.DoubleAdapted() =
      AdaptedWithContext<Wrapper<int64_t>, StructWithAdaptedField, 5>{
          Wrapper<int64_t>{.value = -5}};

  StructWithAdaptedField y;
  y.meta() = "meta";
  intStruct.field() = 2;
  y.typeAdapted() = Wrapper<IntStruct>{.value = intStruct};
  intStruct.field() = 3;
  y.fieldAdapted() =
      AdaptedWithContext<IntStruct, StructWithAdaptedField, 3>{intStruct};
  y.typeAdapted2() = Wrapper<int64_t>{.value = 4};
  y.DoubleAdapted() =
      AdaptedWithContext<Wrapper<int64_t>, StructWithAdaptedField, 5>{
          Wrapper<int64_t>{.value = 5}};

  TEST_IMPL(
      x,
      y,
      "$.meta",
      "$.typeAdapted.field",
      "$.fieldAdapted.field",
      "$.typeAdapted2",
      "$.DoubleAdapted");
}

#undef TEST_IMPL

} // namespace cpp_reflection
} // namespace test_cpp2
