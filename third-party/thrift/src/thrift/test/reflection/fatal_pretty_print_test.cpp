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

#include <thrift/lib/cpp2/reflection/pretty_print.h>

#include <folly/String.h>

#include <thrift/test/reflection/gen-cpp2/reflection_fatal_types.h>

#include <sstream>
#include <type_traits>

#include <folly/portability/GTest.h>

using output_result = std::false_type;

namespace test_cpp2 {
namespace cpp_reflection {

std::string adjust(std::string input) {
  return folly::rtrimWhitespace(folly::stripLeftMargin(std::move(input))).str();
}

#define TEST_IMPL_TC(TC, Expected, ...)                          \
  do {                                                           \
    std::ostringstream out;                                      \
    if constexpr (std::is_void_v<TC>) {                          \
      apache::thrift::pretty_print(out, __VA_ARGS__);            \
    } else {                                                     \
      apache::thrift::pretty_print<TC>(out, __VA_ARGS__);        \
    }                                                            \
    const auto actual = out.str();                               \
                                                                 \
    if (output_result::value) {                                  \
      std::cout << "actual output: " << actual << std::endl      \
                << "expected output: " << Expected << std::endl; \
    }                                                            \
                                                                 \
    EXPECT_EQ(Expected, actual);                                 \
  } while (false)

#define TEST_IMPL(Expected, ...) TEST_IMPL_TC(void, Expected, __VA_ARGS__)

TEST(FatalPrettyPrint, pretty_print) {
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

  structC c1;
  *c1.a() = 47;
  *c1.b() = "hello, world";
  *c1.c() = 132.98;
  *c1.d() = true;

  *c1.e() = enum1::field1;
  *c1.f() = enum2::field0_2;
  c1.g()->set_us("this is a test");

  // c1.h intentionally left empty
  c1.i()->set_a(a1);

  // c1.j intentionally left empty
  *c1.j1() = {2, 4, 6, 8};
  *c1.j2() = {enum1::field0, enum1::field1, enum1::field2};
  *c1.j3() = {a1, a2, a3, a4};

  // c1.k intentionally left empty
  *c1.k1() = {3, 5, 7, 9};
  *c1.k2() = {enum2::field0_2, enum2::field1_2, enum2::field2_2};
  *c1.k3() = {b1, b2, b3, b4};

  // c1.l intentionally left empty
  *c1.l1() = {{2, 3}, {4, 5}, {6, 7}, {8, 9}};
  *c1.l2() = {{12, enum1::field0}, {34, enum1::field1}, {56, enum1::field2}};
  *c1.l3() = {{89, b1}, {78, b2}, {67, b3}, {56, b4}};

  *c1.m1() = {{enum1::field0, 3}, {enum1::field1, 5}, {enum1::field2, 7}};
  *c1.m2() = {
      {enum1::field0, enum2::field0_2},
      {enum1::field1, enum2::field1_2},
      {enum1::field2, enum2::field2_2},
  };
  *c1.m3() = {{enum1::field0, b1}, {enum1::field1, b2}, {enum1::field2, b3}};

  c1.n1()["abc"] = 3;
  c1.n1()["def"] = 5;
  c1.n1()["ghi"] = 7;
  c1.n1()["jkl"] = 9;
  c1.n2()["mno"] = enum1::field0;
  c1.n2()["pqr"] = enum1::field1;
  c1.n2()["stu"] = enum1::field2;
  c1.n3()["vvv"] = b1;
  c1.n3()["www"] = b2;
  c1.n3()["xxx"] = b3;
  c1.n3()["yyy"] = b4;

  c1.o1()[a1] = 3;
  c1.o1()[a2] = 5;
  c1.o1()[a3] = 7;
  c1.o1()[a4] = 9;
  c1.o2()[a1] = enum1::field0;
  c1.o2()[a2] = enum1::field1;
  c1.o2()[a3] = enum1::field2;
  c1.o3()[a1] = b1;
  c1.o3()[a2] = b2;
  c1.o3()[a3] = b3;
  c1.o3()[a4] = b4;

  TEST_IMPL(
      "<struct>{\n"
      "  a: 47,\n"
      "  b: \"hello, world\",\n"
      "  c: 132.98,\n"
      "  d: true,\n"
      "  e: field1,\n"
      "  f: field0_2,\n"
      "  g: <variant>{\n"
      "    us: \"this is a test\"\n"
      "  },\n"
      "  h: <variant>{},\n"
      "  i: <variant>{\n"
      "    a: <struct>{\n"
      "      a: 99,\n"
      "      b: \"abc\"\n"
      "    }\n"
      "  },\n"
      "  j: <list>[],\n"
      "  j1: <list>[\n"
      "    0: 2,\n"
      "    1: 4,\n"
      "    2: 6,\n"
      "    3: 8\n"
      "  ],\n"
      "  j2: <list>[\n"
      "    0: field0,\n"
      "    1: field1,\n"
      "    2: field2\n"
      "  ],\n"
      "  j3: <list>[\n"
      "    0: <struct>{\n"
      "      a: 99,\n"
      "      b: \"abc\"\n"
      "    },\n"
      "    1: <struct>{\n"
      "      a: 1001,\n"
      "      b: \"foo\"\n"
      "    },\n"
      "    2: <struct>{\n"
      "      a: 654,\n"
      "      b: \"bar\"\n"
      "    },\n"
      "    3: <struct>{\n"
      "      a: 9791,\n"
      "      b: \"baz\"\n"
      "    }\n"
      "  ],\n"
      "  k: <set>{},\n"
      "  k1: <set>{\n"
      "    3,\n"
      "    5,\n"
      "    7,\n"
      "    9\n"
      "  },\n"
      "  k2: <set>{\n"
      "    field0_2,\n"
      "    field1_2,\n"
      "    field2_2\n"
      "  },\n"
      "  k3: <set>{\n"
      "    <struct>{\n"
      "      c: 1.23,\n"
      "      d: true\n"
      "    },\n"
      "    <struct>{\n"
      "      c: 9.8,\n"
      "      d: false\n"
      "    },\n"
      "    <struct>{\n"
      "      c: 10.01,\n"
      "      d: true\n"
      "    },\n"
      "    <struct>{\n"
      "      c: 159.73,\n"
      "      d: false\n"
      "    }\n"
      "  },\n"
      "  l: <map>{},\n"
      "  l1: <map>{\n"
      "    2: 3,\n"
      "    4: 5,\n"
      "    6: 7,\n"
      "    8: 9\n"
      "  },\n"
      "  l2: <map>{\n"
      "    12: field0,\n"
      "    34: field1,\n"
      "    56: field2\n"
      "  },\n"
      "  l3: <map>{\n"
      "    56: <struct>{\n"
      "      c: 159.73,\n"
      "      d: false\n"
      "    },\n"
      "    67: <struct>{\n"
      "      c: 10.01,\n"
      "      d: true\n"
      "    },\n"
      "    78: <struct>{\n"
      "      c: 9.8,\n"
      "      d: false\n"
      "    },\n"
      "    89: <struct>{\n"
      "      c: 1.23,\n"
      "      d: true\n"
      "    }\n"
      "  },\n"
      "  m1: <map>{\n"
      "    field0: 3,\n"
      "    field1: 5,\n"
      "    field2: 7\n"
      "  },\n"
      "  m2: <map>{\n"
      "    field0: field0_2,\n"
      "    field1: field1_2,\n"
      "    field2: field2_2\n"
      "  },\n"
      "  m3: <map>{\n"
      "    field0: <struct>{\n"
      "      c: 1.23,\n"
      "      d: true\n"
      "    },\n"
      "    field1: <struct>{\n"
      "      c: 9.8,\n"
      "      d: false\n"
      "    },\n"
      "    field2: <struct>{\n"
      "      c: 10.01,\n"
      "      d: true\n"
      "    }\n"
      "  },\n"
      "  n1: <map>{\n"
      "    \"abc\": 3,\n"
      "    \"def\": 5,\n"
      "    \"ghi\": 7,\n"
      "    \"jkl\": 9\n"
      "  },\n"
      "  n2: <map>{\n"
      "    \"mno\": field0,\n"
      "    \"pqr\": field1,\n"
      "    \"stu\": field2\n"
      "  },\n"
      "  n3: <map>{\n"
      "    \"vvv\": <struct>{\n"
      "      c: 1.23,\n"
      "      d: true\n"
      "    },\n"
      "    \"www\": <struct>{\n"
      "      c: 9.8,\n"
      "      d: false\n"
      "    },\n"
      "    \"xxx\": <struct>{\n"
      "      c: 10.01,\n"
      "      d: true\n"
      "    },\n"
      "    \"yyy\": <struct>{\n"
      "      c: 159.73,\n"
      "      d: false\n"
      "    }\n"
      "  },\n"
      "  o1: <map>{\n"
      "    <struct>{\n"
      "      a: 99,\n"
      "      b: \"abc\"\n"
      "    }: 3,\n"
      "    <struct>{\n"
      "      a: 654,\n"
      "      b: \"bar\"\n"
      "    }: 7,\n"
      "    <struct>{\n"
      "      a: 1001,\n"
      "      b: \"foo\"\n"
      "    }: 5,\n"
      "    <struct>{\n"
      "      a: 9791,\n"
      "      b: \"baz\"\n"
      "    }: 9\n"
      "  },\n"
      "  o2: <map>{\n"
      "    <struct>{\n"
      "      a: 99,\n"
      "      b: \"abc\"\n"
      "    }: field0,\n"
      "    <struct>{\n"
      "      a: 654,\n"
      "      b: \"bar\"\n"
      "    }: field2,\n"
      "    <struct>{\n"
      "      a: 1001,\n"
      "      b: \"foo\"\n"
      "    }: field1\n"
      "  },\n"
      "  o3: <map>{\n"
      "    <struct>{\n"
      "      a: 99,\n"
      "      b: \"abc\"\n"
      "    }: <struct>{\n"
      "      c: 1.23,\n"
      "      d: true\n"
      "    },\n"
      "    <struct>{\n"
      "      a: 654,\n"
      "      b: \"bar\"\n"
      "    }: <struct>{\n"
      "      c: 10.01,\n"
      "      d: true\n"
      "    },\n"
      "    <struct>{\n"
      "      a: 1001,\n"
      "      b: \"foo\"\n"
      "    }: <struct>{\n"
      "      c: 9.8,\n"
      "      d: false\n"
      "    },\n"
      "    <struct>{\n"
      "      a: 9791,\n"
      "      b: \"baz\"\n"
      "    }: <struct>{\n"
      "      c: 159.73,\n"
      "      d: false\n"
      "    }\n"
      "  }\n"
      "}",
      c1);

  TEST_IMPL(
      "===><struct>{\n"
      "===>*-=.|a: 47,\n"
      "===>*-=.|b: \"hello, world\",\n"
      "===>*-=.|c: 132.98,\n"
      "===>*-=.|d: true,\n"
      "===>*-=.|e: field1,\n"
      "===>*-=.|f: field0_2,\n"
      "===>*-=.|g: <variant>{\n"
      "===>*-=.|*-=.|us: \"this is a test\"\n"
      "===>*-=.|},\n"
      "===>*-=.|h: <variant>{},\n"
      "===>*-=.|i: <variant>{\n"
      "===>*-=.|*-=.|a: <struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 99,\n"
      "===>*-=.|*-=.|*-=.|b: \"abc\"\n"
      "===>*-=.|*-=.|}\n"
      "===>*-=.|},\n"
      "===>*-=.|j: <list>[],\n"
      "===>*-=.|j1: <list>[\n"
      "===>*-=.|*-=.|0: 2,\n"
      "===>*-=.|*-=.|1: 4,\n"
      "===>*-=.|*-=.|2: 6,\n"
      "===>*-=.|*-=.|3: 8\n"
      "===>*-=.|],\n"
      "===>*-=.|j2: <list>[\n"
      "===>*-=.|*-=.|0: field0,\n"
      "===>*-=.|*-=.|1: field1,\n"
      "===>*-=.|*-=.|2: field2\n"
      "===>*-=.|],\n"
      "===>*-=.|j3: <list>[\n"
      "===>*-=.|*-=.|0: <struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 99,\n"
      "===>*-=.|*-=.|*-=.|b: \"abc\"\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|1: <struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 1001,\n"
      "===>*-=.|*-=.|*-=.|b: \"foo\"\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|2: <struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 654,\n"
      "===>*-=.|*-=.|*-=.|b: \"bar\"\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|3: <struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 9791,\n"
      "===>*-=.|*-=.|*-=.|b: \"baz\"\n"
      "===>*-=.|*-=.|}\n"
      "===>*-=.|],\n"
      "===>*-=.|k: <set>{},\n"
      "===>*-=.|k1: <set>{\n"
      "===>*-=.|*-=.|3,\n"
      "===>*-=.|*-=.|5,\n"
      "===>*-=.|*-=.|7,\n"
      "===>*-=.|*-=.|9\n"
      "===>*-=.|},\n"
      "===>*-=.|k2: <set>{\n"
      "===>*-=.|*-=.|field0_2,\n"
      "===>*-=.|*-=.|field1_2,\n"
      "===>*-=.|*-=.|field2_2\n"
      "===>*-=.|},\n"
      "===>*-=.|k3: <set>{\n"
      "===>*-=.|*-=.|<struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 1.23,\n"
      "===>*-=.|*-=.|*-=.|d: true\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|<struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 9.8,\n"
      "===>*-=.|*-=.|*-=.|d: false\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|<struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 10.01,\n"
      "===>*-=.|*-=.|*-=.|d: true\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|<struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 159.73,\n"
      "===>*-=.|*-=.|*-=.|d: false\n"
      "===>*-=.|*-=.|}\n"
      "===>*-=.|},\n"
      "===>*-=.|l: <map>{},\n"
      "===>*-=.|l1: <map>{\n"
      "===>*-=.|*-=.|2: 3,\n"
      "===>*-=.|*-=.|4: 5,\n"
      "===>*-=.|*-=.|6: 7,\n"
      "===>*-=.|*-=.|8: 9\n"
      "===>*-=.|},\n"
      "===>*-=.|l2: <map>{\n"
      "===>*-=.|*-=.|12: field0,\n"
      "===>*-=.|*-=.|34: field1,\n"
      "===>*-=.|*-=.|56: field2\n"
      "===>*-=.|},\n"
      "===>*-=.|l3: <map>{\n"
      "===>*-=.|*-=.|56: <struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 159.73,\n"
      "===>*-=.|*-=.|*-=.|d: false\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|67: <struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 10.01,\n"
      "===>*-=.|*-=.|*-=.|d: true\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|78: <struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 9.8,\n"
      "===>*-=.|*-=.|*-=.|d: false\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|89: <struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 1.23,\n"
      "===>*-=.|*-=.|*-=.|d: true\n"
      "===>*-=.|*-=.|}\n"
      "===>*-=.|},\n"
      "===>*-=.|m1: <map>{\n"
      "===>*-=.|*-=.|field0: 3,\n"
      "===>*-=.|*-=.|field1: 5,\n"
      "===>*-=.|*-=.|field2: 7\n"
      "===>*-=.|},\n"
      "===>*-=.|m2: <map>{\n"
      "===>*-=.|*-=.|field0: field0_2,\n"
      "===>*-=.|*-=.|field1: field1_2,\n"
      "===>*-=.|*-=.|field2: field2_2\n"
      "===>*-=.|},\n"
      "===>*-=.|m3: <map>{\n"
      "===>*-=.|*-=.|field0: <struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 1.23,\n"
      "===>*-=.|*-=.|*-=.|d: true\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|field1: <struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 9.8,\n"
      "===>*-=.|*-=.|*-=.|d: false\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|field2: <struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 10.01,\n"
      "===>*-=.|*-=.|*-=.|d: true\n"
      "===>*-=.|*-=.|}\n"
      "===>*-=.|},\n"
      "===>*-=.|n1: <map>{\n"
      "===>*-=.|*-=.|\"abc\": 3,\n"
      "===>*-=.|*-=.|\"def\": 5,\n"
      "===>*-=.|*-=.|\"ghi\": 7,\n"
      "===>*-=.|*-=.|\"jkl\": 9\n"
      "===>*-=.|},\n"
      "===>*-=.|n2: <map>{\n"
      "===>*-=.|*-=.|\"mno\": field0,\n"
      "===>*-=.|*-=.|\"pqr\": field1,\n"
      "===>*-=.|*-=.|\"stu\": field2\n"
      "===>*-=.|},\n"
      "===>*-=.|n3: <map>{\n"
      "===>*-=.|*-=.|\"vvv\": <struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 1.23,\n"
      "===>*-=.|*-=.|*-=.|d: true\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|\"www\": <struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 9.8,\n"
      "===>*-=.|*-=.|*-=.|d: false\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|\"xxx\": <struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 10.01,\n"
      "===>*-=.|*-=.|*-=.|d: true\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|\"yyy\": <struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 159.73,\n"
      "===>*-=.|*-=.|*-=.|d: false\n"
      "===>*-=.|*-=.|}\n"
      "===>*-=.|},\n"
      "===>*-=.|o1: <map>{\n"
      "===>*-=.|*-=.|<struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 99,\n"
      "===>*-=.|*-=.|*-=.|b: \"abc\"\n"
      "===>*-=.|*-=.|}: 3,\n"
      "===>*-=.|*-=.|<struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 654,\n"
      "===>*-=.|*-=.|*-=.|b: \"bar\"\n"
      "===>*-=.|*-=.|}: 7,\n"
      "===>*-=.|*-=.|<struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 1001,\n"
      "===>*-=.|*-=.|*-=.|b: \"foo\"\n"
      "===>*-=.|*-=.|}: 5,\n"
      "===>*-=.|*-=.|<struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 9791,\n"
      "===>*-=.|*-=.|*-=.|b: \"baz\"\n"
      "===>*-=.|*-=.|}: 9\n"
      "===>*-=.|},\n"
      "===>*-=.|o2: <map>{\n"
      "===>*-=.|*-=.|<struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 99,\n"
      "===>*-=.|*-=.|*-=.|b: \"abc\"\n"
      "===>*-=.|*-=.|}: field0,\n"
      "===>*-=.|*-=.|<struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 654,\n"
      "===>*-=.|*-=.|*-=.|b: \"bar\"\n"
      "===>*-=.|*-=.|}: field2,\n"
      "===>*-=.|*-=.|<struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 1001,\n"
      "===>*-=.|*-=.|*-=.|b: \"foo\"\n"
      "===>*-=.|*-=.|}: field1\n"
      "===>*-=.|},\n"
      "===>*-=.|o3: <map>{\n"
      "===>*-=.|*-=.|<struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 99,\n"
      "===>*-=.|*-=.|*-=.|b: \"abc\"\n"
      "===>*-=.|*-=.|}: <struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 1.23,\n"
      "===>*-=.|*-=.|*-=.|d: true\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|<struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 654,\n"
      "===>*-=.|*-=.|*-=.|b: \"bar\"\n"
      "===>*-=.|*-=.|}: <struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 10.01,\n"
      "===>*-=.|*-=.|*-=.|d: true\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|<struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 1001,\n"
      "===>*-=.|*-=.|*-=.|b: \"foo\"\n"
      "===>*-=.|*-=.|}: <struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 9.8,\n"
      "===>*-=.|*-=.|*-=.|d: false\n"
      "===>*-=.|*-=.|},\n"
      "===>*-=.|*-=.|<struct>{\n"
      "===>*-=.|*-=.|*-=.|a: 9791,\n"
      "===>*-=.|*-=.|*-=.|b: \"baz\"\n"
      "===>*-=.|*-=.|}: <struct>{\n"
      "===>*-=.|*-=.|*-=.|c: 159.73,\n"
      "===>*-=.|*-=.|*-=.|d: false\n"
      "===>*-=.|*-=.|}\n"
      "===>*-=.|}\n"
      "===>}",
      c1,
      "*-=.|",
      "===>");
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
  Structure v;
  const char* expectedStr1 = R"(
    <struct>{
      aStruct: null,
      aList: null,
      aSet: null,
      aMap: null,
      aUnion: null,
      anOptionalStruct: null,
      anOptionalList: null,
      anOptionalSet: null,
      anOptionalMap: null,
      anOptionalUnion: null
    })";
  v.aStruct_ref() = nullptr;
  v.aList_ref() = nullptr;
  v.aSet_ref() = nullptr;
  v.aMap_ref() = nullptr;
  v.aUnion_ref() = nullptr;
  v.anOptionalStruct_ref() = nullptr;
  v.anOptionalList_ref() = nullptr;
  v.anOptionalSet_ref() = nullptr;
  v.anOptionalMap_ref() = nullptr;
  v.anOptionalUnion_ref() = nullptr;
  TEST_IMPL(adjust(expectedStr1), v);

  using namespace std::string_literals;
  std::initializer_list<std::string> hello = {"Hello"s};
  std::unordered_map<std::string, std::string> helloWorld{{"Hello"s, "World"s}};
  v.aStruct_ref() = Helper::template build<structA>();
  v.anOptionalStruct_ref() = Helper::template build<structA>();
  v.aList_ref() = Helper::template build<std::deque<std::string>>(hello);
  v.anOptionalList_ref() =
      Helper::template build<std::deque<std::string>>(hello);
  v.aSet_ref() = Helper::template build<std::unordered_set<std::string>>(hello);
  v.anOptionalSet_ref() =
      Helper::template build<std::unordered_set<std::string>>(hello);
  v.aMap_ref() =
      Helper::template build<std::unordered_map<std::string, std::string>>(
          helloWorld);
  v.anOptionalMap_ref() =
      Helper::template build<std::unordered_map<std::string, std::string>>(
          helloWorld);
  v.aUnion_ref() = Helper::template build<unionA>();
  v.anOptionalUnion_ref() = Helper::template build<unionA>();
  const char* expectedStr2 = R"(
    <struct>{
      aStruct: <struct>{
        a: 0,
        b: ""
      },
      aList: <list>[
        0: "Hello"
      ],
      aSet: <set>{
        "Hello"
      },
      aMap: <map>{
        "Hello": "World"
      },
      aUnion: <variant>{},
      anOptionalStruct: <struct>{
        a: 0,
        b: ""
      },
      anOptionalList: <list>[
        0: "Hello"
      ],
      anOptionalSet: <set>{
        "Hello"
      },
      anOptionalMap: <map>{
        "Hello": "World"
      },
      anOptionalUnion: <variant>{}
    })";
  TEST_IMPL(adjust(expectedStr2), v);
}

TEST(FatalPrettyPrint, struct_ref_unique) {
  ref_test<hasRefUnique, UniqueHelper>();
}

TEST(FatalPrettyPrint, ref_shared) {
  ref_test<hasRefShared, SharedHelper>();
}

TEST(FatalPrettyPrint, ref_shared_const) {
  ref_test<hasRefSharedConst, SharedConstHelper>();
}

TEST(FatalPrettyPrint, optional_member) {
  struct1 v;
  v.field1() = 4;

  const char* expectedStr = R"(
    <struct>{
      field0: 0,
      field1: "\004",
      field2: field0,
      field3: field0_2,
      field5: <variant>{}
    })";
  TEST_IMPL(adjust(expectedStr), v);
}

TEST(FatalPrettyPrint, escape_strings) {
  structA s;
  s.b() = "foo\nbar";
  const char* expectedStr = R"(
    <struct>{
      a: 0,
      b: "foo\nbar"
    })";
  TEST_IMPL(adjust(expectedStr), s);
}

TEST(FatalPrettyPrint, variant_ref_unique) {
  variantHasRefUnique v;
  const char* expectedStr1 = R"(
    <variant>{
      aStruct: null
    })";
  v.set_aStruct() = nullptr;
  TEST_IMPL(adjust(expectedStr1), v);

  v.set_aStruct();
  const char* expectedStr2 = R"(
    <variant>{
      aStruct: <struct>{
        a: 0,
        b: ""
      }
    })";
  TEST_IMPL(adjust(expectedStr2), v);
}

TEST(FatalPrettyPrint, tc_binary_iobuf) {
  using tc = apache::thrift::type_class::binary;
  std::string x = "hello";
  std::string y = "world";
  auto xb = folly::IOBuf::wrapBuffer(x.data(), x.size());
  auto yb = folly::IOBuf::wrapBuffer(y.data(), y.size());
  xb->prependChain(std::move(yb));
  const char* expectedStr = R"("0x68656c6c6f776f726c64")"; // hex("helloworld")
  TEST_IMPL_TC(tc, expectedStr, xb->to<std::string>());
  TEST_IMPL_TC(tc, expectedStr, xb->to<folly::fbstring>());
  TEST_IMPL_TC(tc, expectedStr, xb);
  TEST_IMPL_TC(tc, expectedStr, *xb);
  TEST_IMPL_TC(tc, R"()", std::unique_ptr<folly::IOBuf>());
  TEST_IMPL_TC(tc, R"("0x")", std::make_unique<folly::IOBuf>());
}

} // namespace cpp_reflection
} // namespace test_cpp2
