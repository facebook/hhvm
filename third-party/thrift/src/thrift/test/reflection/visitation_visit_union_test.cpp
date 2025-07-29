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

#include <any>
#include <gtest/gtest.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_for_each_field.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_visit_by_thrift_field_metadata.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_visit_union.h>
#include <thrift/test/gen-cpp2/UnionFieldRef_for_each_field.h>
#include <thrift/test/gen-cpp2/UnionFieldRef_visit_union.h>

namespace apache::thrift::test {
namespace {

struct VisitUnionAdapter {
  template <class... Args>
  void operator()(Args&&... args) const {
    apache::thrift::visit_union(std::forward<Args>(args)...);
  }
};

struct ForEachFieldAdapter {
  template <class T, class F>
  void operator()(T&& t, F&& f) const {
    apache::thrift::for_each_field(
        std::forward<T>(t), [&](auto&& meta, auto&& ref) {
          if (folly::to_underlying(t.getType()) == meta.id_ref()) {
            f(meta, *ref);
          }
        });
  }
};

struct VisitByThriftIdAdapter {
  template <class T, class F>
  void operator()(T&& t, F f) const {
    if (t.getType() != T::Type::__EMPTY__) {
      apache::thrift::metadata::ThriftField field;
      apache::thrift::for_each_field(
          std::forward<T>(t), [&](auto&& meta, auto&& ref) {
            if (ref) {
              field = meta;
            }
          });

      apache::thrift::visit_by_thrift_field_metadata(
          std::forward<T>(t), field, std::move(f));
    }
  }
};

template <class Adapter>
struct VisitUnionTest : ::testing::Test {
  static constexpr Adapter adapter;
};

using Adapters = ::testing::Types<VisitUnionAdapter, ForEachFieldAdapter>;
TYPED_TEST_CASE(VisitUnionTest, Adapters);

TYPED_TEST(VisitUnionTest, basic) {
  Basic a;
  TestFixture::adapter(a, [&](auto&&, auto&&) { FAIL(); });

  static const std::string str = "foo";
  a.str() = str;
  TestFixture::adapter(a, [](auto&& meta, auto&& v) {
    EXPECT_EQ(*meta.name_ref(), "str");
    EXPECT_EQ(
        meta.type_ref()->getType(), metadata::ThriftType::Type::t_primitive);
    EXPECT_EQ(*meta.id_ref(), 2);
    EXPECT_EQ(*meta.is_optional_ref(), false);
    if constexpr (std::is_same_v<decltype(v), std::string&>) {
      EXPECT_EQ(v, str);
    } else {
      FAIL();
    }
  });

  static const int64_t int64 = 42LL << 42;
  a.int64() = int64;
  TestFixture::adapter(a, [](auto&& meta, auto&& v) {
    EXPECT_EQ(*meta.name_ref(), "int64");
    EXPECT_EQ(
        meta.type_ref()->getType(), metadata::ThriftType::Type::t_primitive);
    EXPECT_EQ(*meta.id_ref(), 1);
    EXPECT_EQ(*meta.is_optional_ref(), false);
    EXPECT_EQ(typeid(v), typeid(int64_t));
    if constexpr (std::is_same_v<decltype(v), int64_t&>) {
      EXPECT_EQ(v, int64);
    } else {
      FAIL();
    }
  });

  static const std::vector<int32_t> list_i32 = {3, 1, 2};
  a.list_i32() = list_i32;
  TestFixture::adapter(a, [](auto&& meta, auto&& v) {
    EXPECT_EQ(*meta.name_ref(), "list_i32");
    EXPECT_EQ(meta.type_ref()->getType(), metadata::ThriftType::Type::t_list);
    EXPECT_EQ(*meta.id_ref(), 4);
    EXPECT_EQ(*meta.is_optional_ref(), false);
    if constexpr (std::is_same_v<decltype(v), std::vector<int32_t>&>) {
      EXPECT_EQ(v, list_i32);
    } else {
      FAIL();
    }
  });
}

TYPED_TEST(VisitUnionTest, Metadata) {
  Basic a;
  a.int64() = 42;
  TestFixture::adapter(a, [](auto&& m, auto&&) {
    // ThriftType itself is union, we can visit it like ordinary thrift union
    TestFixture::adapter(*m.type_ref(), [](auto&& meta, std::any value) {
      EXPECT_EQ(*meta.name_ref(), "t_primitive");
      EXPECT_EQ(meta.type_ref()->getType(), metadata::ThriftType::Type::t_enum);
      EXPECT_EQ(
          std::any_cast<metadata::ThriftPrimitiveType>(value),
          metadata::ThriftPrimitiveType::THRIFT_I64_TYPE);
    });
  });
}

struct TestPassCallableByValue {
  int i = 0;
  template <class... Args>
  void operator()(Args&&...) {
    ++i;
  }
};

TYPED_TEST(VisitUnionTest, PassCallableByReference) {
  TestPassCallableByValue f;
  Basic a;
  a.int64() = 42;
  TestFixture::adapter(a, folly::copy(f));
  EXPECT_EQ(f.i, 0);
  TestFixture::adapter(a, std::ref(f));
  EXPECT_EQ(f.i, 1);
  TestFixture::adapter(a, f);
  EXPECT_EQ(f.i, 2);
}

template <class T>
constexpr bool kIsString =
    std::is_same_v<folly::remove_cvref_t<T>, std::string>;

TEST(VisitUnionTest, CppRef) {
  CppRef r;
  CppRef r2;
  r2.str() = "42";
  r.set_cppref(r2);
  bool typeMatches = false;
  apache::thrift::visit_union(r, [&typeMatches](auto&&, auto&& r2) {
    if constexpr (!kIsString<decltype(r2)>) {
      apache::thrift::visit_union(r2, [&typeMatches](auto&&, auto&& v) {
        if constexpr (kIsString<decltype(v)>) {
          typeMatches = true;
          EXPECT_EQ(v, "42");
        }
      });
    }
  });
  EXPECT_TRUE(typeMatches);
}

TEST(VisitUnionTest, NonVoid) {
  DuplicateType r;
  r.set_str1("boo");
  auto callable = [](auto&&, auto&& r2) {
    if constexpr (std::is_same_v<
                      folly::remove_cvref_t<decltype(r2)>,
                      std::string>) {
      return std::move(r2);
    }
    return std::string("list");
  };
  EXPECT_EQ(apache::thrift::visit_union(r, callable), "boo");
}

} // namespace
} // namespace apache::thrift::test
