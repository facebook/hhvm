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

#include <sstream>
#include <utility>

#include <glog/logging.h>

#include <folly/String.h>
#include <folly/json.h>
#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/debug_thrift_data_difference/debug.h>
#include <thrift/lib/cpp2/debug_thrift_data_difference/pretty_print.h>
#include <thrift/lib/cpp2/folly_dynamic/folly_dynamic.h>

#include <thrift/lib/cpp2/reflection/helpers.h>
#include <thrift/lib/cpp2/reflection/internal/test_helpers.h>
#include <thrift/test/reflection/gen-cpp2/compat_fatal_types.h>
#include <thrift/test/reflection/gen-cpp2/global_fatal_types.h>
#include <thrift/test/reflection/gen-cpp2/reflection_fatal_types.h>

using namespace cpp2;
using facebook::thrift::dynamic_format;
using facebook::thrift::format_adherence;

namespace facebook {
namespace thrift {

template <typename T>
void test_to_from(T const& pod, const folly::dynamic& json) {
  std::ostringstream log;
  try {
    log.str("to_dynamic(PORTABLE):\n");
    const auto actual = to_dynamic(pod, dynamic_format::PORTABLE);
    if (actual != json) {
      log << "actual: " << folly::toPrettyJson(actual) << std::endl
          << "expected: " << folly::toPrettyJson(json);
      LOG(ERROR) << log.str();
    }
    EXPECT_EQ(actual, json);
  } catch (const std::exception&) {
    LOG(ERROR) << log.str();
    throw;
  }
  try {
    log.str("from_dynamic(PORTABLE):\n");
    const auto actual = from_dynamic<T>(json, dynamic_format::PORTABLE);
    if (actual != pod) {
      pretty_print(log << "actual: ", actual);
      log << std::endl;
      pretty_print(log << "expected: ", pod);
      log << std::endl;
      LOG(ERROR) << log.str();
    }
    EXPECT_EQ(actual, pod);
  } catch (const std::exception&) {
    LOG(ERROR) << log.str();
    throw;
  }
  try {
    log.str("from_dynamic(PORTABLE)/to_dynamic(PORTABLE):\n");
    const auto from = from_dynamic<T>(json, dynamic_format::PORTABLE);
    const auto to = to_dynamic(from, dynamic_format::PORTABLE);
    if (json != to) {
      pretty_print(log << "from: ", from);
      log << std::endl
          << "to: " << folly::toPrettyJson(to) << std::endl
          << "expected: " << folly::toPrettyJson(json);
      LOG(ERROR) << log.str();
    }
    EXPECT_EQ(json, to);
  } catch (const std::exception&) {
    LOG(ERROR) << log.str();
    throw;
  }
  try {
    log.str("to_dynamic(PORTABLE)/from_dynamic(PORTABLE):\n");
    const auto to = to_dynamic(pod, dynamic_format::PORTABLE);
    const auto from = from_dynamic<T>(to, dynamic_format::PORTABLE);
    if (pod != from) {
      log << "to: " << folly::toPrettyJson(to) << std::endl;
      pretty_print(log << "from: ", from);
      log << std::endl;
      pretty_print(log << "expected: ", pod);
      log << std::endl;
      LOG(ERROR) << log.str();
    }
    EXPECT_EQ(pod, from);
  } catch (const std::exception&) {
    LOG(ERROR) << log.str();
    throw;
  }
  try {
    log.str("to_dynamic(PORTABLE)/from_dynamic(PORTABLE,LENIENT):\n");
    const auto to = to_dynamic(pod, dynamic_format::PORTABLE);
    const auto from = from_dynamic<T>(
        to, dynamic_format::PORTABLE, format_adherence::LENIENT);
    if (pod != from) {
      log << "to: " << folly::toPrettyJson(to) << std::endl;
      pretty_print(log << "from: ", from);
      log << std::endl;
      pretty_print(log << "expected: ", pod);
      log << std::endl;
      LOG(ERROR) << log.str();
    }
    EXPECT_EQ(pod, from);
  } catch (const std::exception&) {
    LOG(ERROR) << log.str();
    throw;
  }
  try {
    log.str("to_dynamic(PORTABLE)/from_dynamic(JSON_1,LENIENT):\n");
    const auto to = to_dynamic(pod, dynamic_format::PORTABLE);
    const auto from =
        from_dynamic<T>(to, dynamic_format::JSON_1, format_adherence::LENIENT);
    if (pod != from) {
      log << "to: " << folly::toPrettyJson(to) << std::endl;
      pretty_print(log << "from: ", from);
      log << std::endl;
      pretty_print(log << "expected: ", pod);
      log << std::endl;
      LOG(ERROR) << log.str();
    }
    EXPECT_EQ(pod, from);
  } catch (const std::exception&) {
    LOG(ERROR) << log.str();
    throw;
  }
  try {
    log.str("to_dynamic(JSON_1)/from_dynamic(PORTABLE,LENIENT):\n");
    const auto to = to_dynamic(pod, dynamic_format::JSON_1);
    const auto from = from_dynamic<T>(
        to, dynamic_format::PORTABLE, format_adherence::LENIENT);
    if (pod != from) {
      log << "to: " << folly::toPrettyJson(to) << std::endl;
      pretty_print(log << "from: ", from);
      log << std::endl;
      pretty_print(log << "expected: ", pod);
      log << std::endl;
      LOG(ERROR) << log.str();
    }
    EXPECT_EQ(pod, from);
  } catch (const std::exception&) {
    LOG(ERROR) << log.str();
    throw;
  }
  try {
    log.str("to_dynamic(JSON_1)/from_dynamic(JSON_1,LENIENT):\n");
    const auto to = to_dynamic(pod, dynamic_format::JSON_1);
    const auto from =
        from_dynamic<T>(to, dynamic_format::JSON_1, format_adherence::LENIENT);
    if (pod != from) {
      log << "to: " << folly::toPrettyJson(to) << std::endl;
      pretty_print(log << "from: ", from);
      log << std::endl;
      pretty_print(log << "expected: ", pod);
      log << std::endl;
      LOG(ERROR) << log.str();
    }
    EXPECT_EQ(pod, from);
  } catch (const std::exception&) {
    LOG(ERROR) << log.str();
    throw;
  }
}

template <
    typename Struct3,
    typename StructA,
    typename StructB,
    typename Enum1,
    typename Enum2>
std::pair<Struct3, std::string> test_data_1() {
  StructA a1;
  a1.a_ref().ensure();
  *a1.a_ref() = 99;
  a1.b_ref().ensure();
  *a1.b_ref() = "abc";
  StructA a2;
  a2.a_ref().ensure();
  *a2.a_ref() = 1001;
  a2.b_ref().ensure();
  *a2.b_ref() = "foo";
  StructA a3;
  a3.a_ref().ensure();
  *a3.a_ref() = 654;
  a3.b_ref().ensure();
  *a3.b_ref() = "bar";
  StructA a4;
  a4.a_ref().ensure();
  *a4.a_ref() = 9791;
  a4.b_ref().ensure();
  *a4.b_ref() = "baz";
  StructA a5;
  a5.a_ref().ensure();
  *a5.a_ref() = 111;
  a5.b_ref().ensure();
  *a5.b_ref() = "gaz";

  StructB b1;
  b1.c_ref().ensure();
  *b1.c_ref() = 1.23;
  b1.d_ref().ensure();
  *b1.d_ref() = true;
  StructB b2;
  b2.c_ref().ensure();
  *b2.c_ref() = 9.8;
  b2.d_ref().ensure();
  *b2.d_ref() = false;
  StructB b3;
  b3.c_ref().ensure();
  *b3.c_ref() = 10.01;
  b3.d_ref().ensure();
  *b3.d_ref() = true;
  StructB b4;
  b4.c_ref().ensure();
  *b4.c_ref() = 159.73;
  b4.d_ref().ensure();
  *b4.d_ref() = false;
  StructB b5;
  b5.c_ref().ensure();
  *b5.c_ref() = 468.02;
  b5.d_ref().ensure();
  *b5.d_ref() = true;

  Struct3 pod;

  pod.fieldA_ref().ensure();
  *pod.fieldA_ref() = 141;
  pod.fieldB_ref().ensure();
  *pod.fieldB_ref() = "this is a test";
  pod.fieldC_ref().ensure();
  *pod.fieldC_ref() = Enum1::field0;
  pod.fieldD_ref().ensure();
  *pod.fieldD_ref() = Enum2::field1_2;
  pod.fieldE_ref().ensure();
  pod.fieldE_ref()->set_ud(5.6);
  pod.fieldF_ref().ensure();
  pod.fieldF_ref()->set_us_2("this is a variant");
  pod.fieldG_ref().ensure();
  *pod.fieldG_ref()->field0_ref() = 98;
  pod.fieldG_ref()->field1_ref() = "hello, world";
  pod.fieldG_ref()->field2_ref().ensure();
  *pod.fieldG_ref()->field2_ref() = Enum1::field2;
  *pod.fieldG_ref()->field3_ref() = Enum2::field0_2;
  pod.fieldG_ref()->field4_ref() = {};
  pod.fieldG_ref()->field4_ref()->set_ui(19937);
  pod.fieldG_ref()->field5_ref().ensure();
  pod.fieldG_ref()->field5_ref()->set_ue_2(Enum1::field1);
  // fieldH intentionally left empty
  pod.fieldI_ref().ensure();
  *pod.fieldI_ref() = {3, 5, 7, 9};
  pod.fieldJ_ref().ensure();
  *pod.fieldJ_ref() = {"a", "b", "c", "d"};
  pod.fieldK_ref().ensure();
  *pod.fieldK_ref() = {};
  pod.fieldL_ref().ensure();
  pod.fieldL_ref()->push_back(a1);
  pod.fieldL_ref()->push_back(a2);
  pod.fieldL_ref()->push_back(a3);
  pod.fieldL_ref()->push_back(a4);
  pod.fieldL_ref()->push_back(a5);
  pod.fieldM_ref().ensure();
  *pod.fieldM_ref() = {2, 4, 6, 8};
  pod.fieldN_ref().ensure();
  *pod.fieldN_ref() = {"w", "x", "y", "z"};
  pod.fieldO_ref().ensure();
  *pod.fieldO_ref() = {};
  pod.fieldP_ref().ensure();
  *pod.fieldP_ref() = {b1, b2, b3, b4, b5};
  pod.fieldQ_ref().ensure();
  *pod.fieldQ_ref() = {{"a1", a1}, {"a2", a2}, {"a3", a3}};
  pod.fieldR_ref().ensure();
  *pod.fieldR_ref() = {};

  const auto json = folly::stripLeftMargin(R"({
    "fieldA": 141,
    "fieldB": "this is a test",
    "fieldC": "field0",
    "fieldD": "field1_2",
    "fieldE": {
        "ud": 5.6
    },
    "fieldF": {
        "us_2": "this is a variant"
    },
    "fieldG": {
        "field0": 98,
        "field1": "hello, world",
        "field2": "field2",
        "field3": "field0_2",
        "field4": {
            "ui": 19937
        },
        "field5": {
            "ue_2": "field1"
        }
    },
    "fieldH": {},
    "fieldI": [3, 5, 7, 9],
    "fieldJ": ["a", "b", "c", "d"],
    "fieldK": [],
    "fieldL": [
      { "a": 99, "b": "abc" },
      { "a": 1001, "b": "foo" },
      { "a": 654, "b": "bar" },
      { "a": 9791, "b": "baz" },
      { "a": 111, "b": "gaz" }
    ],
    "fieldM": [2, 4, 6, 8],
    "fieldN": ["w", "x", "y", "z"],
    "fieldO": [],
    "fieldP": [
      { "c": 1.23, "d": true },
      { "c": 9.8, "d": false },
      { "c": 10.01, "d": true },
      { "c": 159.73, "d": false },
      { "c": 468.02, "d": true }
    ],
    "fieldQ": {
      "a1": { "a": 99, "b": "abc" },
      "a2": { "a": 1001, "b": "foo" },
      "a3": { "a": 654, "b": "bar" }
    },
    "fieldR": {},
    "fieldS": {}
  })");

  return std::make_pair(pod, json);
}

TEST(FollyDynamic, to_from_dynamic) {
  const auto data = test_data_1<
      test_cpp2::cpp_reflection::struct3,
      test_cpp2::cpp_reflection::structA,
      test_cpp2::cpp_reflection::structB,
      test_cpp2::cpp_reflection::enum1,
      test_cpp2::cpp_reflection::enum2>();
  const auto pod = data.first;
  const auto json = folly::parseJson(data.second);

  test_to_from(pod, json);
}

TEST(FollyDynamic, booleans) {
  const auto decode = [](const char* json) {
    return from_dynamic<test_cpp2::cpp_reflection::structB>(
        folly::parseJson(json), dynamic_format::PORTABLE);
  };

  test_cpp2::cpp_reflection::structB expected;
  *expected.c() = 1.3;
  *expected.d() = true;

  EXPECT_EQ(expected, decode(R"({ "c": 1.3, "d": 1})"));
  EXPECT_EQ(expected, decode(R"({ "c": 1.3, "d": 100})"));
  EXPECT_EQ(expected, decode(R"({ "c": 1.3, "d": true})"));
}

TEST(FollyDynamic, to_from_dynamic_compat) {
  const auto data = test_data_1<
      test_cpp2::cpp_compat::compat_struct3,
      test_cpp2::cpp_compat::compat_structA,
      test_cpp2::cpp_compat::compat_structB,
      test_cpp2::cpp_compat::compat_enum1,
      test_cpp2::cpp_compat::compat_enum2>();
  const auto pod = data.first;
  const auto json = folly::parseJson(data.second);

  test_to_from(pod, json);
}

TEST(FollyDynamic, to_from_dynamic_global) {
  const auto data = test_data_1<
      ::global_struct3,
      ::global_structA,
      ::global_structB,
      ::global_enum1,
      ::global_enum2>();
  const auto pod = data.first;
  const auto json = folly::parseJson(data.second);

  test_to_from(pod, json);
}

TEST(FollyDynamic, to_from_dynamic_binary) {
  folly::dynamic actl = folly::dynamic::object;
  folly::dynamic expt = folly::dynamic::object;

  // to
  test_cpp2::cpp_reflection::struct_binary a;
  *a.bi() = "123abc";

  actl = to_dynamic(a, dynamic_format::PORTABLE);
  expt = folly::dynamic::object("bi", "123abc");

  EXPECT_EQ(expt, actl);

  // from
  auto obj = from_dynamic<test_cpp2::cpp_reflection::struct_binary>(
      folly::dynamic::object("bi", "123abc"), dynamic_format::PORTABLE);
  EXPECT_EQ("123abc", *obj.bi());
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
  using namespace test_cpp2::cpp_reflection;
  using namespace std::string_literals;
  std::initializer_list<std::string> hello = {"Hello"s};
  std::map<std::string, std::string> helloWorld{{"Hello"s, "World"s}};

  Structure v;
  const auto rawJson = folly::stripLeftMargin(R"({
    "aStruct": {
      "a": 0,
      "b": ""
    },
    "aList": [
      "Hello"
    ],
    "aSet": [
      "Hello"
    ],
    "aMap": {
      "Hello": "World"
    },
    "aUnion": {},
    "anOptionalStruct": null,
    "anOptionalList": null,
    "anOptionalSet": null,
    "anOptionalMap": null,
    "anOptionalUnion": null
  })");
  v.aStruct_ref() = Helper::template build<structA>();
  v.aList_ref() = Helper::template build<std::vector<std::string>>(hello);
  v.aSet_ref() = Helper::template build<std::set<std::string>>(hello);
  v.aMap_ref() =
      Helper::template build<std::map<std::string, std::string>>(helloWorld);
  v.aUnion_ref() = Helper::template build<unionA>();
  v.anOptionalStruct_ref() = nullptr;
  v.anOptionalList_ref() = nullptr;
  v.anOptionalSet_ref() = nullptr;
  v.anOptionalMap_ref() = nullptr;
  v.anOptionalUnion_ref() = nullptr;
  const auto json = folly::parseJson(rawJson);
  test_to_from(v, json);

  v.anOptionalStruct_ref() = Helper::template build<structA>();
  v.anOptionalList_ref() =
      Helper::template build<std::vector<std::string>>(hello);
  v.anOptionalSet_ref() = Helper::template build<std::set<std::string>>(hello);
  v.anOptionalMap_ref() =
      Helper::template build<std::map<std::string, std::string>>(helloWorld);
  v.anOptionalUnion_ref() = Helper::template build<unionA>();
  const auto rawJson2 = R"(
    {
      "aStruct": {
        "a": 0,
        "b": ""
      },
      "aList": [
        "Hello"
      ],
      "aSet": [
        "Hello"
      ],
      "aMap": {
        "Hello": "World"
      },
      "aUnion": {},
      "anOptionalStruct": {
        "a": 0,
        "b": ""
      },
      "anOptionalList": [
        "Hello"
      ],
      "anOptionalSet": [
        "Hello"
      ],
      "anOptionalMap": {
        "Hello": "World"
      },
      "anOptionalUnion": {}
    })";
  const auto json2 = folly::parseJson(rawJson2);
  test_to_from(v, json2);
}

TEST(PrettyPrint, to_from_struct_ref_unique) {
  ref_test<test_cpp2::cpp_reflection::hasRefUniqueSimple, UniqueHelper>();
}

TEST(PrettyPrint, to_from_struct_ref_shared) {
  ref_test<test_cpp2::cpp_reflection::hasRefSharedSimple, SharedHelper>();
}

TEST(PrettyPrint, to_from_struct_ref_shared_const) {
  ref_test<
      test_cpp2::cpp_reflection::hasRefSharedConstSimple,
      SharedConstHelper>();
}

TEST(FollyDynamic, to_from_variant_ref_unique) {
  test_cpp2::cpp_reflection::variantHasRefUnique pod;

  test_cpp2::cpp_reflection::structA inner;
  inner.a() = 109;
  inner.b() = "testing yo";
  pod.set_aStruct(inner);

  const auto rawJson = folly::stripLeftMargin(R"({
    "aStruct": {
      "a": 109,
      "b": "testing yo"
    }
  })");

  const auto json = folly::parseJson(rawJson);
  test_to_from(pod, json);
}

TEST(PrettyPrint, to_from_struct_box) {
  test_cpp2::cpp_reflection::hasBoxSimple pod;

  auto& inner = pod.anOptionalStruct().ensure();
  inner.a() = 109;
  inner.b() = "testing yo";

  const auto rawJson = folly::stripLeftMargin(R"({
    "anOptionalStruct": {
      "a": 109,
      "b": "testing yo"
    }
  })");

  const auto json = folly::parseJson(rawJson);
  test_to_from(pod, json);
}

} // namespace thrift
} // namespace facebook

namespace test_cpp1 {
namespace cpp_compat {} // namespace cpp_compat
} // namespace test_cpp1

TEST(FollyDynamic, optional_string) {
  auto obj = from_dynamic<global_struct1>(
      folly::dynamic::object("field1", "asdf"), dynamic_format::PORTABLE);
  EXPECT_EQ("asdf", *obj.field1());
}

TEST(FollyDynamic, list_from_empty_object) {
  // some dynamic languages (lua, php) conflate empty array and empty object;
  // check that we do not throw in such cases
  using type = global_structC;
  using member_name = fatal::sequence<char, 'j', '3'>;
  using member_meta =
      apache::thrift::get_struct_member_by_name<type, member_name>;
  EXPECT_SAME< // sanity check
      member_meta::type_class,
      apache::thrift::type_class::list<
          apache::thrift::type_class::structure>>();
  auto obj = from_dynamic<type>(
      folly::dynamic::object(
          fatal::z_data<member_name>(), folly::dynamic::object),
      dynamic_format::PORTABLE);
  EXPECT_TRUE(member_meta::is_set(obj));
  EXPECT_EQ(0, member_meta::getter{}(obj).size());
}

TEST(FollyDynamic, set_from_empty_object) {
  // some dynamic languages (lua, php) conflate empty array and empty object;
  // check that we do not throw in such cases
  using type = global_structC;
  using member_name = fatal::sequence<char, 'k', '3'>;
  using member_meta =
      apache::thrift::get_struct_member_by_name<type, member_name>;
  EXPECT_SAME< // sanity check
      member_meta::type_class,
      apache::thrift::type_class::set<apache::thrift::type_class::structure>>();
  auto obj = from_dynamic<type>(
      folly::dynamic::object(
          fatal::z_data<member_name>(), folly::dynamic::object),
      dynamic_format::PORTABLE);
  EXPECT_TRUE(member_meta::is_set(obj));
  EXPECT_EQ(0, member_meta::getter{}(obj).size());
}

TEST(FollyDynamic, map_from_empty_array) {
  // some dynamic languages (lua, php) conflate empty array and empty object;
  // check that we do not throw in such cases
  using type = global_structC;
  using member_name = fatal::sequence<char, 'l', '3'>;
  using member_meta =
      apache::thrift::get_struct_member_by_name<type, member_name>;
  EXPECT_SAME< // sanity check
      member_meta::type_class,
      apache::thrift::type_class::map<
          apache::thrift::type_class::integral,
          apache::thrift::type_class::structure>>();
  auto obj = from_dynamic<type>(
      folly::dynamic::object(
          fatal::z_data<member_name>(), folly::dynamic::array),
      dynamic_format::PORTABLE);
  EXPECT_TRUE(member_meta::is_set(obj));
  EXPECT_EQ(0, member_meta::getter{}(obj).size());
}

TEST(FollyDynamic, from_iobuf) {
  folly::dynamic dyn =
      folly::dynamic::object("buf", "foo")("bufInPlace", "bar");
  auto obj = from_dynamic<test_cpp2::cpp_reflection::StructWithIOBuf>(
      dyn, dynamic_format::PORTABLE);
  EXPECT_EQ((*obj.buf())->moveToFbString(), "foo");
  EXPECT_EQ(obj.bufInPlace()->moveToFbString(), "bar");

  folly::dynamic dynEmpty = folly::dynamic::object();
  auto objEmpty = from_dynamic<test_cpp2::cpp_reflection::StructWithIOBuf>(
      dynEmpty, dynamic_format::PORTABLE);
  EXPECT_FALSE(*objEmpty.buf());
  EXPECT_EQ(objEmpty.bufInPlace()->moveToFbString(), "");
}

TEST(FollyDynamic, to_iobuf) {
  test_cpp2::cpp_reflection::StructWithIOBuf obj;
  obj.buf() = folly::IOBuf::copyBuffer("foo");
  obj.bufInPlace() = std::move(*folly::IOBuf::copyBuffer("bar"));

  folly::dynamic dyn = to_dynamic(obj, dynamic_format::PORTABLE);
  EXPECT_EQ(dyn["buf"], "foo");
  EXPECT_EQ(dyn["bufInPlace"], "bar");

  test_cpp2::cpp_reflection::StructWithIOBuf objEmpty;
  folly::dynamic dynEmpty = to_dynamic(objEmpty, dynamic_format::PORTABLE);
  EXPECT_TRUE(dynEmpty["buf"].isNull());
  EXPECT_EQ(dynEmpty["bufInPlace"], "");
}

namespace {

class FollyDynamicEnum : public ::testing::Test {
 protected:
  void SetUp() override {
    EXPECT_SAME< // sanity check
        member_meta::type_class,
        apache::thrift::type_class::enumeration>();
  }

  using type = global_structC;
  using member_name = fatal::sequence<char, 'e'>;
  using member_meta =
      apache::thrift::get_struct_member_by_name<type, member_name>;

  std::string member_name_s{fatal::to_instance<std::string, member_name>()};
};
} // namespace

TEST_F(FollyDynamicEnum, from_string_strict) {
  folly::dynamic dyn = folly::dynamic::object(member_name_s, "field0");
  auto obj = from_dynamic<type>(dyn, dynamic_format::PORTABLE);
  EXPECT_TRUE(member_meta::is_set(obj));
  EXPECT_EQ(global_enum1::field0, member_meta::getter{}(obj));
  EXPECT_THROW(
      from_dynamic<type>(dyn, dynamic_format::JSON_1), folly::ConversionError);
}

TEST_F(FollyDynamicEnum, from_integer_strict) {
  folly::dynamic dyn = folly::dynamic::object(member_name_s, 0);
  auto obj = from_dynamic<type>(dyn, dynamic_format::JSON_1);
  EXPECT_TRUE(member_meta::is_set(obj));
  EXPECT_EQ(global_enum1::field0, member_meta::getter{}(obj));
  EXPECT_THROW(
      from_dynamic<type>(dyn, dynamic_format::PORTABLE), std::invalid_argument);
}

TEST_F(FollyDynamicEnum, from_string_lenient) {
  folly::dynamic dyn = folly::dynamic::object(member_name_s, "field0");
  auto obj1 = from_dynamic<type>(
      dyn, dynamic_format::PORTABLE, format_adherence::LENIENT);
  EXPECT_TRUE(member_meta::is_set(obj1));
  EXPECT_EQ(global_enum1::field0, member_meta::getter{}(obj1));
  auto obj2 = from_dynamic<type>(
      dyn, dynamic_format::JSON_1, format_adherence::LENIENT);
  EXPECT_TRUE(member_meta::is_set(obj2));
  EXPECT_EQ(global_enum1::field0, member_meta::getter{}(obj2));
}

TEST_F(FollyDynamicEnum, from_integer_lenient) {
  folly::dynamic dyn = folly::dynamic::object(member_name_s, 0);
  auto obj1 = from_dynamic<type>(
      dyn, dynamic_format::PORTABLE, format_adherence::LENIENT);
  EXPECT_TRUE(member_meta::is_set(obj1));
  EXPECT_EQ(global_enum1::field0, member_meta::getter{}(obj1));
  auto obj2 = from_dynamic<type>(
      dyn, dynamic_format::JSON_1, format_adherence::LENIENT);
  EXPECT_TRUE(member_meta::is_set(obj2));
  EXPECT_EQ(global_enum1::field0, member_meta::getter{}(obj2));
}

TEST(FromDynamic, struct_with_vector_bool) {
  folly::dynamic v = folly::dynamic::object();
  v["values"] = folly::dynamic::array(true, false, true, false);
  auto res = from_dynamic<test_cpp2::cpp_reflection::StructWithVectorBool>(
      v, dynamic_format::PORTABLE, format_adherence::LENIENT);
  EXPECT_EQ(res.values(), std::vector<bool>({true, false, true, false}));
}
