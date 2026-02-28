/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/Result.h"
#include <folly/portability/GTest.h>
#include <folly/test/TestUtils.h>
#include <string>

using namespace watchman;

TEST(Result, empty) {
  Result<bool> b;

  EXPECT_TRUE(b.empty()) << "default constructed and empty";

  EXPECT_THROW(b.throwIfError(), std::logic_error);
  EXPECT_THROW(b.value(), std::logic_error);
  EXPECT_THROW(b.error(), std::logic_error)
      << "error throws logic error for empty result";
}

TEST(Result, simple_value) {
  auto b = makeResult(true);

  EXPECT_FALSE(b.empty());
  EXPECT_TRUE(b.hasValue());
  EXPECT_TRUE(b.value());

  Result<bool> copyOfB(b);

  EXPECT_FALSE(b.empty()) << "b is not empty after being copied";
  EXPECT_FALSE(copyOfB.empty()) << "copyOfB is not empty";
  EXPECT_TRUE(copyOfB.hasValue()) << "copyOfB has a value";
  EXPECT_TRUE(copyOfB.value()) << "copyOfB holds true";

  Result<bool> movedB(std::move(b));

  EXPECT_TRUE(b.empty()) << "b empty after move";
  EXPECT_FALSE(movedB.empty()) << "movedB is not empty";
  EXPECT_TRUE(movedB.hasValue()) << "movedB has a value";
  EXPECT_TRUE(movedB.value()) << "movedB holds true";

  b = movedB;
  EXPECT_FALSE(b.empty()) << "b is not empty after copying";
  EXPECT_TRUE(b.hasValue()) << "b has a value";
  EXPECT_TRUE(b.value()) << "b holds true";

  b = std::move(copyOfB);
  EXPECT_FALSE(b.empty()) << "b is not empty after copying";
  EXPECT_TRUE(b.hasValue()) << "b has a value";
  EXPECT_TRUE(b.value()) << "b holds true";
  EXPECT_TRUE(copyOfB.empty()) << "copyOfB is empty after being moved";
}

TEST(Result, error) {
  auto a = makeResultWith([] { return std::string("noice"); });
  EXPECT_TRUE(a.hasValue()) << "got a value";
  EXPECT_EQ(a.value(), "noice") << "got our string out";
  using atype = decltype(a);
  auto is_string = std::is_same<typename atype::value_type, std::string>::value;
  EXPECT_TRUE(is_string) << "a has std::string as a value type";

  auto b = makeResultWith([] { throw std::runtime_error("w00t"); });

  EXPECT_TRUE(b.hasError()) << "we got an exception contained";

  EXPECT_THROW_RE(b.throwIfError(), std::runtime_error, "w00t");

  using btype = decltype(b);
  auto is_unit = std::is_same<typename btype::value_type, folly::Unit>::value;
  EXPECT_TRUE(is_unit) << "b has Unit as a value type";

  auto c = makeResultWith([] {
    if (false) {
      return 42;
    }
    throw std::runtime_error("gah");
  });

  using ctype = decltype(c);
  auto is_int = std::is_same<typename ctype::value_type, int>::value;
  EXPECT_TRUE(is_int) << "c has int as a value type";

  EXPECT_TRUE(c.hasError()) << "c has an error";
}

TEST(Result, non_exception_error_type) {
  Result<std::string, int> result("hello");

  EXPECT_TRUE(result.hasValue()) << "has value";
  EXPECT_EQ(result.value(), "hello");

  result = Result<std::string, int>(42);
  EXPECT_TRUE(result.hasError()) << "holding error";
  EXPECT_EQ(result.error(), 42) << "holding 42";

  EXPECT_THROW(result.throwIfError(), std::logic_error);
}
