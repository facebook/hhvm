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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <thrift/compiler/whisker/expected.h>

#include <memory>
#include <tuple>
#include <vector>

namespace whisker {

TEST(ExpectedTest, construct_default) {
  expected<int, int> e;
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(e, 0);
}

TEST(ExpectedTest, construct_inplace) {
  {
    expected<int, int> e(std::in_place, 1);
    EXPECT_TRUE(e.has_value());
    EXPECT_EQ(e.value(), 1);
  }
  {
    expected<std::tuple<int, int>, int> e(std::in_place, 0, 1);
    EXPECT_TRUE(e.has_value());
    EXPECT_EQ(std::get<0>(*e), 0);
    EXPECT_EQ(std::get<1>(*e), 1);
  }
}

TEST(ExpectedTest, construct_error) {
  expected<int, int> e = unexpected(1);
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), 1);
}

TEST(ExpectedTest, copy_construct_from_unexpected) {
  // implicit
  {
    unexpected<int> u(1);
    expected<long, long> e = u;
    EXPECT_FALSE(e.has_value());
    EXPECT_EQ(e.error(), 1);
  }
  // explicit
  {
    struct S {
      explicit S(int i) : value(i) {}
      int value;
    };
    unexpected<int> u{42};
    expected<int, S> e{u};
    EXPECT_FALSE(e.has_value());
    EXPECT_EQ(e.error().value, 42);
  }
}

TEST(ExpectedTest, move_construct_from_unexpected) {
  // implicit
  {
    expected<long, long> e = unexpected<int>(1);
    EXPECT_FALSE(e.has_value());
    EXPECT_EQ(e.error(), 1);
  }
  // explicit
  {
    struct S {
      explicit S(int i) : value(i) {}
      int value;
    };
    expected<int, S> e{unexpected<int>(42)};
    EXPECT_FALSE(e.has_value());
    EXPECT_EQ(e.error().value, 42);
  }
}

TEST(ExpectedTest, construct_error_unexpect) {
  expected<int, int> e(unexpect, 1);
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), 1);
}

TEST(ExpectedTest, construct_initializer_list) {
  expected<std::vector<int>, int> e(std::in_place, {0, 1, 2});
  EXPECT_TRUE(e.has_value());
  EXPECT_THAT(e.value(), testing::ElementsAre(0, 1, 2));
}

TEST(ExpectedTest, construct_error_initializer_list) {
  expected<int, std::vector<int>> e(unexpect, {0, 1, 2});
  EXPECT_FALSE(e.has_value());
  EXPECT_THAT(e.error(), testing::ElementsAre(0, 1, 2));
}

TEST(ExpectedTest, emplace) {
  expected<int, std::unique_ptr<int>> e;
  e.emplace(1);
  EXPECT_TRUE(e.has_value());
  EXPECT_EQ(std::move(e).value(), 1);
}

TEST(ExpectedTest, emplace_initializer_list) {
  struct vec_wrapper {
    explicit vec_wrapper(std::initializer_list<int> ilist) noexcept
        : wrapped(ilist) {}
    std::vector<int> wrapped;
  };
  expected<vec_wrapper, int> e = unexpected(1);
  e.emplace({0, 1, 2});
  EXPECT_TRUE(e.has_value());
  EXPECT_THAT(std::move(e).value().wrapped, testing::ElementsAre(0, 1, 2));
}

TEST(ExpectedTest, swap) {
  expected<int, int> e1 = 1;
  expected<int, int> e2 = unexpected(2);
  swap(e1, e2);
  EXPECT_FALSE(e1.has_value());
  EXPECT_EQ(e1.error(), 2);
  EXPECT_TRUE(e2.has_value());
  EXPECT_EQ(e2, 1);
}

TEST(ExpectedTest, value_error) {
  expected<int, int> e = unexpected(1);
  EXPECT_FALSE(e.has_value());
  EXPECT_EQ(e.error(), 1);
  EXPECT_THROW(
      {
        try {
          e.value();
        } catch (const bad_expected_access<int>& ex) {
          EXPECT_EQ(ex.error(), 1);
          throw;
        };
      },
      bad_expected_access<int>);
}

TEST(ExpectedTest, assign) {
  expected<int, int> e1 = 42;
  expected<int, int> e2 = 17;
  expected<int, int> e3 = 21;
  expected<int, int> e4 = unexpected(42);
  expected<int, int> e5 = unexpected(17);
  expected<int, int> e6 = unexpected(21);

  e1 = e2;
  EXPECT_TRUE(e1);
  EXPECT_EQ(*e1, 17);
  EXPECT_TRUE(e2);
  EXPECT_EQ(*e2, 17);

  e1 = std::move(e2);
  EXPECT_TRUE(e1);
  EXPECT_EQ(*e1, 17);
  // @lint-ignore CLANGTIDY bugprone-use-after-move
  EXPECT_TRUE(e2);
  EXPECT_EQ(*e2, 17);

  e1 = 42;
  EXPECT_TRUE(e1);
  EXPECT_EQ(*e1, 42);

  auto unex = unexpected(12);
  e1 = unex;
  EXPECT_FALSE(e1);
  EXPECT_EQ(e1.error(), 12);

  e1 = unexpected(42);
  EXPECT_FALSE(e1);
  EXPECT_EQ(e1.error(), 42);

  e1 = e3;
  EXPECT_TRUE(e1);
  EXPECT_EQ(*e1, 21);

  e4 = e5;
  EXPECT_FALSE(e4);
  EXPECT_EQ(e4.error(), 17);

  e4 = std::move(e6);
  EXPECT_FALSE(e4);
  EXPECT_EQ(e4.error(), 21);

  e4 = e1;
  EXPECT_TRUE(e4);
  EXPECT_EQ(*e4, 21);
}

TEST(ExpectedTest, comparison) {
  expected<int, int> e1 = 42;
  expected<int, int> e2 = 42;
  expected<int, int> e3 = unexpected(42);
  expected<short, short> e4 = unexpected(42);

  EXPECT_EQ(e1, e2);
  EXPECT_EQ(e1, 42);
  EXPECT_EQ(e1, short(42));
  EXPECT_NE(e1, e3);
  EXPECT_NE(e3, e2);
  EXPECT_NE(e3, 42);
  EXPECT_EQ(e3, unexpected(42));
  EXPECT_EQ(e3, unexpected(short(42)));
  EXPECT_EQ(e3, e4);
}

TEST(ExpectedTest, LWG_3836) {
  struct BaseError {};
  struct DerivedError : BaseError {};

  expected<bool, DerivedError> e1(false);
  expected<bool, BaseError> e2(e1);
  // should not convert using operator bool()
  EXPECT_EQ(e2.value(), false);
}

TEST(ExpectedTest, visit_variant) {
  expected<int, std::variant<std::string, long>> o =
      unexpected<std::string>("yikes!");
  EXPECT_EQ(
      visit(
          o,
          [](int) { return 0; },
          [](const std::string& str) { return int(str.length()); },
          [](long) { return 2; }),
      std::string("yikes!").length());

  EXPECT_EQ(
      visit(
          std::move(o),
          [](int) -> std::string { return ""; },
          [](std::string&& str) -> std::string { return std::move(str); },
          [](long) -> std::string { return ""; }),
      "yikes!");
}

TEST(ExpectedTest, visit_nonvariant) {
  expected<int, std::string> o = unexpected<std::string>("yikes!");
  EXPECT_EQ(
      visit(
          o,
          [](int) { return 0; },
          [](const std::string& str) { return int(str.length()); }),
      std::string("yikes!").length());

  EXPECT_EQ(
      visit(
          std::move(o),
          [](int) -> std::string { return ""; },
          [](std::string&& str) -> std::string { return std::move(str); }),
      "yikes!");
}

} // namespace whisker
