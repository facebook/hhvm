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

#include <gtest/gtest.h>

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/common/CompactVariant.h>

#include <memory>
#include <string>

namespace apache::thrift::fast_thrift {

// =============================================================================
// Basic Construction
// =============================================================================

TEST(CompactVariantTest, DefaultConstructIsValueless) {
  CompactVariant<int, double> v;
  EXPECT_TRUE(v.valueless());
  EXPECT_EQ(v.index(), 0);
}

TEST(CompactVariantTest, ConstructFromFirstType) {
  CompactVariant<int, double> v(42);
  EXPECT_FALSE(v.valueless());
  EXPECT_TRUE(v.is<int>());
  EXPECT_FALSE(v.is<double>());
  EXPECT_EQ(v.get<int>(), 42);
}

TEST(CompactVariantTest, ConstructFromSecondType) {
  CompactVariant<int, double> v(3.14);
  EXPECT_TRUE(v.is<double>());
  EXPECT_DOUBLE_EQ(v.get<double>(), 3.14);
}

TEST(CompactVariantTest, ConstructFromMoveOnly) {
  auto buf = folly::IOBuf::copyBuffer("hello");
  auto* rawPtr = buf.get();

  CompactVariant<std::unique_ptr<folly::IOBuf>, int> v(std::move(buf));
  EXPECT_TRUE(v.is<std::unique_ptr<folly::IOBuf>>());
  EXPECT_EQ(v.get<std::unique_ptr<folly::IOBuf>>().get(), rawPtr);
}

// =============================================================================
// Emplace
// =============================================================================

TEST(CompactVariantTest, EmplaceReplacesValue) {
  CompactVariant<int, double> v(42);
  EXPECT_TRUE(v.is<int>());

  v.emplace<double>(2.71);
  EXPECT_TRUE(v.is<double>());
  EXPECT_DOUBLE_EQ(v.get<double>(), 2.71);
}

TEST(CompactVariantTest, EmplaceOnValueless) {
  CompactVariant<int, std::string> v;
  EXPECT_TRUE(v.valueless());

  v.emplace<std::string>("test");
  EXPECT_TRUE(v.is<std::string>());
  EXPECT_EQ(v.get<std::string>(), "test");
}

// =============================================================================
// Assignment
// =============================================================================

TEST(CompactVariantTest, AssignFromValue) {
  CompactVariant<int, double> v(42);
  v = 3.14;
  EXPECT_TRUE(v.is<double>());
  EXPECT_DOUBLE_EQ(v.get<double>(), 3.14);
}

TEST(CompactVariantTest, AssignFromMoveOnly) {
  CompactVariant<std::unique_ptr<folly::IOBuf>, int> v(123);
  EXPECT_TRUE(v.is<int>());

  v = folly::IOBuf::copyBuffer("world");
  EXPECT_TRUE(v.is<std::unique_ptr<folly::IOBuf>>());
}

// =============================================================================
// Move
// =============================================================================

TEST(CompactVariantTest, MoveConstruct) {
  CompactVariant<int, std::string> v(std::string("hello"));
  EXPECT_TRUE(v.is<std::string>());

  CompactVariant<int, std::string> v2(std::move(v));
  EXPECT_TRUE(v2.is<std::string>());
  EXPECT_EQ(v2.get<std::string>(), "hello");
  EXPECT_TRUE(v.valueless()); // NOLINT: testing moved-from state
}

TEST(CompactVariantTest, MoveAssign) {
  CompactVariant<int, double> v(42);
  CompactVariant<int, double> v2(1.0);

  v2 = std::move(v);
  EXPECT_TRUE(v2.is<int>());
  EXPECT_EQ(v2.get<int>(), 42);
  EXPECT_TRUE(v.valueless()); // NOLINT: testing moved-from state
}

TEST(CompactVariantTest, MoveConstructMoveOnly) {
  auto buf = folly::IOBuf::copyBuffer("test");
  auto* rawPtr = buf.get();

  CompactVariant<std::unique_ptr<folly::IOBuf>, int> v(std::move(buf));
  CompactVariant<std::unique_ptr<folly::IOBuf>, int> v2(std::move(v));

  EXPECT_TRUE(v2.is<std::unique_ptr<folly::IOBuf>>());
  EXPECT_EQ(v2.get<std::unique_ptr<folly::IOBuf>>().get(), rawPtr);
  EXPECT_TRUE(v.valueless()); // NOLINT: testing moved-from state
}

// =============================================================================
// Destruction
// =============================================================================

TEST(CompactVariantTest, DestroysCurrent) {
  bool destroyed = false;
  struct Tracer {
    bool* flag;
    explicit Tracer(bool* f) : flag(f) {}
    Tracer(Tracer&& o) noexcept : flag(o.flag) { o.flag = nullptr; }
    Tracer& operator=(Tracer&&) noexcept = default;
    ~Tracer() {
      if (flag) {
        *flag = true;
      }
    }
  };

  {
    CompactVariant<Tracer, int> v{Tracer(&destroyed)};
    EXPECT_FALSE(destroyed);
  }
  EXPECT_TRUE(destroyed);
}

TEST(CompactVariantTest, EmplaceDestroysPrevious) {
  bool destroyed = false;
  struct Tracer {
    bool* flag;
    explicit Tracer(bool* f) : flag(f) {}
    Tracer(Tracer&& o) noexcept : flag(o.flag) { o.flag = nullptr; }
    Tracer& operator=(Tracer&&) noexcept = default;
    ~Tracer() {
      if (flag) {
        *flag = true;
      }
    }
  };

  CompactVariant<Tracer, int> v{Tracer(&destroyed)};
  EXPECT_FALSE(destroyed);

  v.emplace<int>(42);
  EXPECT_TRUE(destroyed);
  EXPECT_TRUE(v.is<int>());
}

// =============================================================================
// Free Functions
// =============================================================================

TEST(CompactVariantTest, HoldsAlternative) {
  CompactVariant<int, double, std::string> v(42);
  EXPECT_TRUE(holds_alternative<int>(v));
  EXPECT_FALSE(holds_alternative<double>(v));
  EXPECT_FALSE(holds_alternative<std::string>(v));
}

TEST(CompactVariantTest, FreeGet) {
  CompactVariant<int, double> v(42);
  EXPECT_EQ(get<int>(v), 42);

  const auto& cv = v;
  EXPECT_EQ(get<int>(cv), 42);
}

// =============================================================================
// Size Verification
// =============================================================================

TEST(CompactVariantTest, SizeIsSmallerThanStdVariant) {
  using StdV = std::variant<int, double>;
  using CompactV = CompactVariant<int, double>;

  // CompactVariant: max(sizeof(int), sizeof(double)) + 1 = 9
  // std::variant is typically 16 (8 storage + 8 index)
  EXPECT_EQ(sizeof(CompactV), sizeof(double) + 1);
  EXPECT_LT(sizeof(CompactV), sizeof(StdV));
}

TEST(CompactVariantTest, SizeWithLargeType) {
  struct Big {
    char data[48];
  };
  using V = CompactVariant<Big, int>;

  // 48 + 1 = 49
  EXPECT_EQ(sizeof(V), 49);
}

// =============================================================================
// exception_wrapper (folly type used in ThriftResponseMessage)
// =============================================================================

TEST(CompactVariantTest, ExceptionWrapper) {
  CompactVariant<int, folly::exception_wrapper> v(
      folly::make_exception_wrapper<std::runtime_error>("test error"));

  EXPECT_TRUE(v.is<folly::exception_wrapper>());
  auto& ew = v.get<folly::exception_wrapper>();
  EXPECT_TRUE(ew.is_compatible_with<std::runtime_error>());
}

} // namespace apache::thrift::fast_thrift
