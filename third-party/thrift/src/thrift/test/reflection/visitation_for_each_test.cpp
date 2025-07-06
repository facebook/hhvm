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

#include <thrift/test/reflection/gen-cpp2/reflection_for_each_field.h>
#include <thrift/test/reflection/gen-cpp2/reflection_visit_by_thrift_field_metadata.h> // @manual

#include <gtest/gtest.h>
#include <folly/Overload.h>

#include <typeindex>

using apache::thrift::field_ref;
using apache::thrift::optional_field_ref;
using apache::thrift::required_field_ref;
using std::deque;
using std::is_same_v;
using std::string;
using std::type_index;
using std::vector;

namespace test_cpp2 {
namespace cpp_reflection {
namespace {

struct ForEachFieldAdapter {
  template <class... Args>
  void operator()(Args&&... args) const {
    apache::thrift::for_each_field(std::forward<Args>(args)...);
  }
};

struct VisitByThriftIdAdapter {
  template <class T, class F>
  void operator()(T&& t, F f) const {
    for (auto&& meta : *::apache::thrift::get_struct_metadata<std::decay_t<T>>()
                            .fields_ref()) {
      apache::thrift::visit_by_thrift_field_metadata(
          std::forward<T>(t), meta, [&](auto&& ref) { f(meta, ref); });
    }
  }

  template <class T, class F>
  void operator()(T&& t1, T&& t2, F f) const {
    using namespace apache::thrift;
    for (auto&& meta : *get_struct_metadata<std::decay_t<T>>().fields_ref()) {
      visit_by_thrift_field_metadata(
          std::forward<T>(t1), meta, [&](auto&& ref1) {
            visit_by_thrift_field_metadata(
                std::forward<T>(t2), meta, [&](auto&& ref2) {
                  if constexpr (std::
                                    is_same_v<decltype(ref1), decltype(ref2)>) {
                    f(meta, ref1, ref2);
                  } else {
                    ASSERT_TRUE(false);
                  }
                });
          });
    }
  }
};

template <class Adapter>
struct ForEachFieldTest : ::testing::Test {
  static constexpr Adapter adapter;
};

using Adapters = ::testing::Types<ForEachFieldAdapter, VisitByThriftIdAdapter>;
TYPED_TEST_CASE(ForEachFieldTest, Adapters);

TYPED_TEST(ForEachFieldTest, test_metadata) {
  struct1 s;
  TestFixture::adapter(s, [i = 0](auto& meta, auto&& ref) mutable {
    EXPECT_EQ(*meta.id_ref(), 1 << i);
    EXPECT_EQ(*meta.name_ref(), "field" + std::to_string(i));
    EXPECT_EQ(
        meta.type_ref()->getType(),
        (vector{
            apache::thrift::metadata::ThriftType::Type::t_primitive,
            apache::thrift::metadata::ThriftType::Type::t_primitive,
            apache::thrift::metadata::ThriftType::Type::t_enum,
            apache::thrift::metadata::ThriftType::Type::t_enum,
            apache::thrift::metadata::ThriftType::Type::t_union,
            apache::thrift::metadata::ThriftType::Type::t_union,
        })[i]);
    EXPECT_EQ(
        *meta.is_optional_ref(),
        (vector{false, true, false, false, true, false})[i]);
    EXPECT_EQ(
        type_index(typeid(ref)),
        (vector<type_index>{
            typeid(required_field_ref<int32_t&>),
            typeid(optional_field_ref<string&>),
            typeid(field_ref<enum1&>),
            typeid(required_field_ref<enum2&>),
            typeid(optional_field_ref<union1&>),
            typeid(field_ref<union2&>),
        })[i]);

    // required field always has value
    EXPECT_EQ(
        ref.has_value(), (vector{true, false, false, true, false, false})[i]);
    ++i;
  });
}

TYPED_TEST(ForEachFieldTest, modify_field) {
  struct1 s;
  s.field0() = 10;
  s.field1() = "20";
  s.field2() = enum1::field0;
  s.field3() = enum2::field1_2;
  s.field4().emplace().set_us("foo");
  s.field5().emplace().set_us_2("bar");
  auto run = folly::overload(
      [](int32_t& ref) {
        EXPECT_EQ(ref, 10);
        ref = 20;
      },
      [](string& ref) {
        EXPECT_EQ(ref, "20");
        ref = "30";
      },
      [](enum1& ref) {
        EXPECT_EQ(ref, enum1::field0);
        ref = enum1::field1;
      },
      [](enum2& ref) {
        EXPECT_EQ(ref, enum2::field1_2);
        ref = enum2::field2_2;
      },
      [](union1& ref) {
        EXPECT_EQ(ref.get_us(), "foo");
        ref.set_ui(20);
      },
      [](union2& ref) {
        EXPECT_EQ(ref.get_us_2(), "bar");
        ref.set_ui_2(30);
      },
      [](auto&) { EXPECT_TRUE(false) << "type mismatch"; });
  TestFixture::adapter(s, [run](auto&, auto&& ref) {
    EXPECT_TRUE(ref.has_value());
    run(*ref);
  });
  EXPECT_EQ(s.field0(), 20);
  EXPECT_EQ(s.field1(), "30");
  EXPECT_EQ(s.field2(), enum1::field1);
  EXPECT_EQ(s.field3(), enum2::field2_2);
  EXPECT_EQ(s.field4()->get_ui(), 20);
  EXPECT_EQ(s.field5()->get_ui_2(), 30);
}

TYPED_TEST(ForEachFieldTest, test_cpp_ref_unique) {
  hasRefUnique s;
  deque<string> names = {
      "aStruct",
      "aList",
      "aSet",
      "aMap",
      "aUnion",
      "anOptionalStruct",
      "anOptionalList",
      "anOptionalSet",
      "anOptionalMap",
      "anOptionalUnion",
  };
  TestFixture::adapter(s, [&, i = 0](auto& meta, auto&& ref) mutable {
    EXPECT_EQ(*meta.name_ref(), names[i++]);
    if constexpr (is_same_v<decltype(*ref), deque<string>&>) {
      if (*meta.is_optional_ref()) {
        ref.reset(new decltype(names)(names));
      }
    }
  });

  // for cpp.ref, unqualified field has value by default
  EXPECT_TRUE(s.aStruct());
  EXPECT_TRUE(s.aList()->empty());
  EXPECT_TRUE(s.aSet()->empty());
  EXPECT_TRUE(s.aMap()->empty());
  EXPECT_TRUE(s.aUnion());

  EXPECT_FALSE(s.anOptionalStruct());
  EXPECT_EQ(*s.anOptionalList(), names);
  EXPECT_FALSE(s.anOptionalSet());
  EXPECT_FALSE(s.anOptionalMap());
  EXPECT_FALSE(s.anOptionalUnion());
}

TYPED_TEST(ForEachFieldTest, test_reference_type) {
  struct1 s;
  TestFixture::adapter(s, [](auto& meta, auto&& ref) {
    switch (*meta.id_ref()) {
      case 1:
        EXPECT_TRUE((is_same_v<decltype(*ref), int32_t&>));
        break;
      case 2:
        EXPECT_TRUE((is_same_v<decltype(*ref), string&>));
        break;
      case 4:
        EXPECT_TRUE((is_same_v<decltype(*ref), enum1&>));
        break;
      case 8:
        EXPECT_TRUE((is_same_v<decltype(*ref), enum2&>));
        break;
      case 16:
        EXPECT_TRUE((is_same_v<decltype(*ref), union1&>));
        break;
      case 32:
        EXPECT_TRUE((is_same_v<decltype(*ref), union2&>));
        break;
      default:
        EXPECT_TRUE(false) << *meta.name_ref() << " " << *meta.id_ref();
    }
  });
  TestFixture::adapter(std::move(s), [](auto& meta, auto&& ref) {
    switch (*meta.id_ref()) {
      case 1:
        EXPECT_TRUE((is_same_v<decltype(*ref), int32_t&&>));
        break;
      case 2:
        EXPECT_TRUE((is_same_v<decltype(*ref), string&&>));
        break;
      case 4:
        EXPECT_TRUE((is_same_v<decltype(*ref), enum1&&>));
        break;
      case 8:
        EXPECT_TRUE((is_same_v<decltype(*ref), enum2&&>));
        break;
      case 16:
        EXPECT_TRUE((is_same_v<decltype(*ref), union1&&>));
        break;
      case 32:
        EXPECT_TRUE((is_same_v<decltype(*ref), union2&&>));
        break;
      default:
        EXPECT_TRUE(false) << *meta.name_ref() << " " << *meta.id_ref();
    }
  });
  const struct1 t;
  TestFixture::adapter(t, [](auto& meta, auto&& ref) {
    switch (*meta.id_ref()) {
      case 1:
        EXPECT_TRUE((is_same_v<decltype(*ref), const int32_t&>));
        break;
      case 2:
        EXPECT_TRUE((is_same_v<decltype(*ref), const string&>));
        break;
      case 4:
        EXPECT_TRUE((is_same_v<decltype(*ref), const enum1&>));
        break;
      case 8:
        EXPECT_TRUE((is_same_v<decltype(*ref), const enum2&>));
        break;
      case 16:
        EXPECT_TRUE((is_same_v<decltype(*ref), const union1&>));
        break;
      case 32:
        EXPECT_TRUE((is_same_v<decltype(*ref), const union2&>));
        break;
      default:
        EXPECT_TRUE(false) << *meta.name_ref() << " " << *meta.id_ref();
    }
  });
}

TYPED_TEST(ForEachFieldTest, test_two_structs_document) {
  structA thrift1;
  thrift1.a() = 10;
  thrift1.b() = "20";
  structA thrift2 = thrift1;

  TestFixture::adapter(
      thrift1,
      thrift2,
      [](const apache::thrift::metadata::ThriftField& meta,
         auto field_ref1,
         auto field_ref2) {
        EXPECT_EQ(field_ref1, field_ref2) << *meta.name() << " mismatch";
      });
}

TYPED_TEST(ForEachFieldTest, test_two_structs_assignment) {
  struct1 s, t;
  s.field0() = 10;
  s.field1() = "11";
  t.field0() = 20;
  t.field1() = "22";
  auto run = folly::overload(
      [](auto& meta,
         required_field_ref<int32_t&> r1,
         required_field_ref<int32_t&> r2) {
        EXPECT_EQ(r1, 10);
        EXPECT_EQ(r2, 20);
        r1 = 30;
        r2 = 40;

        EXPECT_EQ(*meta.name_ref(), "field0");
        EXPECT_EQ(*meta.is_optional_ref(), false);
      },
      [](auto& meta,
         optional_field_ref<string&> r1,
         optional_field_ref<string&> r2) {
        EXPECT_EQ(r1, "11");
        EXPECT_EQ(r2, "22");
        r1 = "33";
        r2 = "44";

        EXPECT_EQ(*meta.name_ref(), "field1");
        EXPECT_EQ(*meta.is_optional_ref(), true);
      },
      [](auto&&...) {});
  TestFixture::adapter(s, t, run);
  EXPECT_EQ(s.field0(), 30);
  EXPECT_EQ(s.field1(), "33");
  EXPECT_EQ(t.field0(), 40);
  EXPECT_EQ(t.field1(), "44");
}

struct TestPassCallableByValue {
  int i = 0;
  template <class... Args>
  void operator()(Args&&...) {
    ++i;
  }
};

TYPED_TEST(ForEachFieldTest, PassCallableByValue) {
  TestPassCallableByValue f;
  TestFixture::adapter(struct1{}, f);
  EXPECT_EQ(f.i, 0);
  TestFixture::adapter(struct1{}, struct1{}, f);
  EXPECT_EQ(f.i, 0);
  TestFixture::adapter(struct1{}, std::ref(f));
  EXPECT_EQ(f.i, 6);
  TestFixture::adapter(struct1{}, struct1{}, std::ref(f));
  EXPECT_EQ(f.i, 12);
}

} // namespace
} // namespace cpp_reflection
} // namespace test_cpp2
