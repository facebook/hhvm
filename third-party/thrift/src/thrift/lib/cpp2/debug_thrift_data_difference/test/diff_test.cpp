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

#include <thrift/lib/cpp2/debug_thrift_data_difference/diff.h>

#include <thrift/test/reflection/gen-cpp2/reflection_types.h>

#include <folly/String.h>
#include <folly/portability/GTest.h>

#include <sstream>
#include <string>
#include <vector>

using namespace test_cpp2::cpp_reflection;

static struct3 test_data() {
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

#define TEST_IMPL(LHS, RHS, EXPECTED)                        \
  do {                                                       \
    using namespace facebook::thrift;                        \
    const auto& expected = folly::stripLeftMargin(EXPECTED); \
    std::ostringstream actualStream;                         \
    debug_thrift_data_difference(                            \
        LHS, RHS, make_diff_output_callback(actualStream));  \
    EXPECT_EQ(expected, actualStream.str());                 \
  } while (false)

TEST(Diff, equal) {
  TEST_IMPL(test_data(), test_data(), "");
}

TEST(Diff, Failure) {
  auto pod = test_data();
  struct3 pod1, pod2;
  *pod1.fieldR()["a"].c() = 1;
  *pod1.fieldR()["b"].c() = 2;
  *pod1.fieldR()["c"].c() = 3;
  *pod1.fieldR()["d"].c() = 4;
  *pod2.fieldR()["d"].c() = 4;
  *pod2.fieldR()["c"].c() = 3;
  *pod2.fieldR()["b"].c() = 2;
  *pod2.fieldR()["a"].c() = 1;
  TEST_IMPL(pod1, pod2, "");
}

TEST(Diff, fieldA) {
  auto pod = test_data();
  *pod.fieldA() = 90;
  TEST_IMPL(pod, test_data(), R"(
    $.fieldA:
    - 90
    + 141
    )");
  *pod.fieldA() = 141;
  TEST_IMPL(pod, test_data(), "");
}

TEST(Diff, fieldB) {
  auto pod = test_data();
  *pod.fieldB() = "should mismatch";
  TEST_IMPL(pod, test_data(), R"(
    $.fieldB:
    - "should mismatch"
    + "this is a test"
  )");
  *pod.fieldB() = "this is a test";
  TEST_IMPL(pod, test_data(), "");
}

TEST(Diff, fieldC) {
  auto pod = test_data();
  *pod.fieldC() = enum1::field2;
  TEST_IMPL(pod, test_data(), R"(
    $.fieldC:
    - field2
    + field0
  )");
  *pod.fieldC() = enum1::field0;
  TEST_IMPL(pod, test_data(), "");
}

TEST(Diff, fieldE) {
  auto pod = test_data();
  pod.fieldE() = {};
  TEST_IMPL(pod, test_data(), R"(
    $.fieldE.ud:
    + 5.6
  )");
  TEST_IMPL(test_data(), pod, R"(
    $.fieldE.ud:
    - 5.6
  )");
  pod.fieldE()->set_ui(5);
  TEST_IMPL(pod, test_data(), R"(
    $.fieldE.ui:
    - 5
    $.fieldE.ud:
    + 5.6
  )");
  pod.fieldE() = {};
  TEST_IMPL(pod, test_data(), R"(
    $.fieldE.ud:
    + 5.6
  )");
  pod.fieldE()->set_ud(4);
  TEST_IMPL(pod, test_data(), R"(
    $.fieldE.ud:
    - 4
    + 5.6
  )");
  pod.fieldE()->set_ud(5.6);
  TEST_IMPL(pod, test_data(), "");
}

TEST(Diff, fieldH) {
  auto pod = test_data();
  pod.fieldH()->set_ui_2(3);
  TEST_IMPL(pod, test_data(), R"(
    $.fieldH.ui_2:
    - 3
  )");
  pod.fieldH() = {};
  TEST_IMPL(pod, test_data(), "");
}

TEST(Diff, fieldI) {
  auto pod = test_data();
  pod.fieldI()[0] = 4;
  TEST_IMPL(pod, test_data(), R"(
    $.fieldI[0]:
    - 4
    + 3
  )");
  pod.fieldI()[0] = 3;
  TEST_IMPL(pod, test_data(), "");
  pod.fieldI()[2] = 10;
  TEST_IMPL(pod, test_data(), R"(
    $.fieldI[2]:
    - 10
    + 7
  )");
  pod.fieldI()->push_back(11);
  TEST_IMPL(pod, test_data(), R"(
    $.fieldI[2]:
    - 10
    + 7
    $.fieldI[4]:
    - 11
  )");
  pod.fieldI()->clear();
  TEST_IMPL(pod, test_data(), R"(
    $.fieldI[0]:
    + 3
    $.fieldI[1]:
    + 5
    $.fieldI[2]:
    + 7
    $.fieldI[3]:
    + 9
  )");
}

TEST(Diff, fieldM) {
  auto pod = test_data();
  pod.fieldM()->clear();
  TEST_IMPL(
      pod,
      test_data(),
      R"(
        $.fieldM[2]:
        + 2
        $.fieldM[4]:
        + 4
        $.fieldM[6]:
        + 6
        $.fieldM[8]:
        + 8
      )");
  pod.fieldM()->insert(11);
  pod.fieldM()->insert(12);
  pod.fieldM()->insert(13);
  pod.fieldM()->insert(14);
  TEST_IMPL(
      pod,
      test_data(),
      R"(
        $.fieldM[11]:
        - 11
        $.fieldM[12]:
        - 12
        $.fieldM[13]:
        - 13
        $.fieldM[14]:
        - 14
        $.fieldM[2]:
        + 2
        $.fieldM[4]:
        + 4
        $.fieldM[6]:
        + 6
        $.fieldM[8]:
        + 8
      )");
  *pod.fieldM() = *test_data().fieldM();
  TEST_IMPL(pod, test_data(), "");
}

TEST(Diff, fieldQ) {
  auto pod = test_data();
  pod.fieldQ()->clear();
  TEST_IMPL(
      pod,
      test_data(),
      R"(
        $.fieldQ["a1"]:
        + <struct>{
        +   a: 99,
        +   b: "abc"
        + }
        $.fieldQ["a2"]:
        + <struct>{
        +   a: 1001,
        +   b: "foo"
        + }
        $.fieldQ["a3"]:
        + <struct>{
        +   a: 654,
        +   b: "bar"
        + }
      )");
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
      R"(
        $.fieldQ["A1"]:
        - <struct>{
        -   a: 1,
        -   b: "1"
        - }
        $.fieldQ["A2"]:
        - <struct>{
        -   a: 2,
        -   b: "2"
        - }
        $.fieldQ["A3"]:
        - <struct>{
        -   a: 3,
        -   b: "3"
        - }
        $.fieldQ["a1"]:
        + <struct>{
        +   a: 99,
        +   b: "abc"
        + }
        $.fieldQ["a2"]:
        + <struct>{
        +   a: 1001,
        +   b: "foo"
        + }
        $.fieldQ["a3"]:
        + <struct>{
        +   a: 654,
        +   b: "bar"
        + }
      )");
  *pod.fieldQ() = *test_data().fieldQ();
  TEST_IMPL(pod, test_data(), "");
}

TEST(Diff, fieldG_field0) {
  auto pod = test_data();
  *pod.fieldG()->field0() = 12;
  TEST_IMPL(pod, test_data(), R"(
    $.fieldG.field0:
    - 12
    + 98
  )");
  *pod.fieldG()->field0() = 98;
  TEST_IMPL(pod, test_data(), "");
}

TEST(Diff, fieldG_field1) {
  auto pod = test_data();
  pod.fieldG()->field1() = "should mismatch";
  TEST_IMPL(pod, test_data(), R"(
    $.fieldG.field1:
    - "should mismatch"
    + "hello, world"
  )");
  pod.fieldG()->field1() = "hello, world";
  TEST_IMPL(pod, test_data(), "");
}

TEST(Diff, fieldG_field2) {
  auto pod = test_data();
  *pod.fieldG()->field2() = enum1::field1;
  TEST_IMPL(pod, test_data(), R"(
    $.fieldG.field2:
    - field1
    + field2
  )");
  *pod.fieldG()->field2() = enum1::field2;
  TEST_IMPL(pod, test_data(), "");
}

TEST(Diff, fieldG_field5) {
  auto pod = test_data();
  pod.fieldG()->field5()->set_ui_2(5);
  TEST_IMPL(pod, test_data(), R"(
    $.fieldG.field5.ui_2:
    - 5
    $.fieldG.field5.ue_2:
    + field1
  )");
  pod.fieldG()->field5() = {};
  TEST_IMPL(pod, test_data(), R"(
    $.fieldG.field5.ue_2:
    + field1
  )");
  pod.fieldG()->field5()->set_ue_2(enum1::field0);
  TEST_IMPL(pod, test_data(), R"(
    $.fieldG.field5.ue_2:
    - field0
    + field1
  )");
  pod.fieldG()->field5()->set_ue_2(enum1::field1);
  TEST_IMPL(pod, test_data(), "");
}

TEST(Diff, fieldS) {
  auto pod = test_data();
  *pod.fieldS() = {{"123", "456"}, {"abc", "ABC"}, {"ghi", "GHI"}};
  TEST_IMPL(pod, test_data(), R"(
    $.fieldS["0x676869"]:
    - "0x474849"
    $.fieldS["0x646566"]:
    + "0x444546"
  )");
}

TEST(Diff, struct_binary) {
  struct_binary lhs;
  *lhs.bi() = "hello";
  struct_binary rhs;
  *rhs.bi() = "world";

  TEST_IMPL(lhs, rhs, R"(
    $.bi:
    - "0x68656c6c6f"
    + "0x776f726c64"
  )");
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
  TEST_IMPL(allNull, allDefault, R"(
    $.anOptionalStruct:
    + <struct>{
    +   a: 0,
    +   b: ""
    + }
    $.anOptionalList:
    + <list>[]
    $.anOptionalSet:
    + <set>{}
    $.anOptionalMap:
    + <map>{}
    $.anOptionalUnion:
    + <variant>{}
  )");
  TEST_IMPL(allDefault, allNull, R"(
    $.anOptionalStruct:
    - <struct>{
    -   a: 0,
    -   b: ""
    - }
    $.anOptionalList:
    - <list>[]
    $.anOptionalSet:
    - <set>{}
    $.anOptionalMap:
    - <map>{}
    $.anOptionalUnion:
    - <variant>{}
  )");
}

TEST(Diff, struct_ref_unique) {
  ref_test<hasRefUnique, UniqueHelper>();
}

TEST(Diff, ref_shared) {
  ref_test<hasRefShared, SharedHelper>();
}

TEST(Diff, ref_shared_const) {
  ref_test<hasRefSharedConst, SharedConstHelper>();
}

TEST(Diff, optional_members) {
  struct1 field1Set;
  field1Set.field1() = "1";
  struct1 field1Unset;
  struct1 field1SetButNotIsset;
  field1SetButNotIsset.field1() = "2";
  apache::thrift::unset_unsafe(field1SetButNotIsset.field1());
  struct1 field1SetDefault;
  apache::thrift::ensure_isset_unsafe(field1SetDefault.field1());
  TEST_IMPL(field1Set, field1Unset, R"(
    $.field1:
    - "1"
  )");
  TEST_IMPL(field1Unset, field1Set, R"(
    $.field1:
    + "1"
  )");
  TEST_IMPL(field1Set, field1SetButNotIsset, R"(
    $.field1:
    - "1"
  )");
  TEST_IMPL(field1SetButNotIsset, field1Set, R"(
    $.field1:
    + "1"
  )");
  TEST_IMPL(field1Unset, field1SetButNotIsset, "");
  TEST_IMPL(field1SetButNotIsset, field1Unset, "");
  TEST_IMPL(field1Set, field1SetDefault, R"(
    $.field1:
    - "1"
    + ""
  )");
  TEST_IMPL(field1SetDefault, field1Set, R"(
    $.field1:
    - ""
    + "1"
  )");
  TEST_IMPL(field1SetDefault, field1SetButNotIsset, R"(
    $.field1:
    - ""
  )");
  TEST_IMPL(field1SetButNotIsset, field1SetDefault, R"(
    $.field1:
    + ""
  )");
  TEST_IMPL(field1Unset, field1SetDefault, R"(
    $.field1:
    + ""
  )");
  TEST_IMPL(field1SetDefault, field1Unset, R"(
    $.field1:
    - ""
  )");
}

TEST(Diff, adapters) {
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
      R"(
        $.meta:
        - "non-meta"
        + "meta"
        $.typeAdapted.field:
        - -2
        + 2
        $.fieldAdapted.field:
        - -3
        + 3
        $.typeAdapted2:
        - -4
        + 4
        $.DoubleAdapted:
        - -5
        + 5
      )");
}

#undef TEST_IMPL
