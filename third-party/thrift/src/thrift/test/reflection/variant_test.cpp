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

#include <thrift/lib/cpp2/reflection/variant.h>

#include <thrift/lib/cpp2/reflection/internal/test_helpers.h>

#include <thrift/test/reflection/gen-cpp2/reflection_fatal_types.h>

#include <folly/portability/GTest.h>

#include <memory>
#include <utility>

namespace test_cpp2 {
namespace cpp_reflection {

TEST(FatalVariant, variant_try_get) {
#define TEST_IMPL2(TYPE, FIELD, OTHER1, OTHER2, OTHER3, VALUE, ...)           \
  do {                                                                        \
    union1 u;                                                                 \
    const union1& c = u;                                                      \
    TYPE __VA_ARGS__ value{VALUE};                                            \
    u.set_##FIELD(value);                                                     \
                                                                              \
    ASSERT_NE(nullptr, apache::thrift::variant_try_get<TYPE>(u));             \
    ASSERT_EQ(union1::Type::FIELD, u.getType());                              \
    EXPECT_EQ(value, *apache::thrift::variant_try_get<TYPE>(u));              \
    ASSERT_EQ(union1::Type::FIELD, u.getType());                              \
    EXPECT_EQ(                                                                \
        std::addressof(u.mutable_##FIELD()),                                  \
        apache::thrift::variant_try_get<TYPE>(u));                            \
    ASSERT_EQ(union1::Type::FIELD, u.getType());                              \
    EXPECT_SAME<TYPE*, decltype(apache::thrift::variant_try_get<TYPE>(u))>(); \
                                                                              \
    ASSERT_NE(nullptr, apache::thrift::variant_try_get<TYPE>(c));             \
    ASSERT_EQ(union1::Type::FIELD, u.getType());                              \
    EXPECT_EQ(value, *apache::thrift::variant_try_get<TYPE>(c));              \
    ASSERT_EQ(union1::Type::FIELD, u.getType());                              \
    EXPECT_EQ(                                                                \
        std::addressof(c.get_##FIELD()),                                      \
        apache::thrift::variant_try_get<TYPE>(c));                            \
    ASSERT_EQ(union1::Type::FIELD, u.getType());                              \
    EXPECT_SAME<                                                              \
        const TYPE*,                                                          \
        decltype(apache::thrift::variant_try_get<TYPE>(c))>();                \
                                                                              \
    EXPECT_EQ(nullptr, apache::thrift::variant_try_get<OTHER1>(u));           \
    ASSERT_EQ(union1::Type::FIELD, u.getType());                              \
    EXPECT_EQ(nullptr, apache::thrift::variant_try_get<OTHER2>(u));           \
    ASSERT_EQ(union1::Type::FIELD, u.getType());                              \
    EXPECT_EQ(nullptr, apache::thrift::variant_try_get<OTHER3>(u));           \
    ASSERT_EQ(union1::Type::FIELD, u.getType());                              \
                                                                              \
    EXPECT_EQ(nullptr, apache::thrift::variant_try_get<OTHER1>(c));           \
    ASSERT_EQ(union1::Type::FIELD, u.getType());                              \
    EXPECT_EQ(nullptr, apache::thrift::variant_try_get<OTHER2>(c));           \
    ASSERT_EQ(union1::Type::FIELD, u.getType());                              \
    EXPECT_EQ(nullptr, apache::thrift::variant_try_get<OTHER3>(c));           \
    ASSERT_EQ(union1::Type::FIELD, u.getType());                              \
  } while (false)

#define TEST_IMPL(...)              \
  do {                              \
    TEST_IMPL2(__VA_ARGS__);        \
    TEST_IMPL2(__VA_ARGS__, const); \
  } while (false)

  TEST_IMPL(std::int32_t, ui, double, std::string, enum1, 10);
  TEST_IMPL(double, ud, std::int32_t, std::string, enum1, 5.6);
  TEST_IMPL(std::string, us, std::int32_t, double, enum1, "test");
  TEST_IMPL(enum1, ue, std::int32_t, double, std::string, enum1::field2);

#undef TEST_IMPL
#undef TEST_IMPL2
}

TEST(FatalVariant, variant_set) {
#define TEST_IMPL2(TYPE, FIELD, VALUE, ...)                              \
  do {                                                                   \
    union1 u;                                                            \
    TYPE value{VALUE};                                                   \
    TYPE const expected{VALUE};                                          \
                                                                         \
    apache::thrift::variant_set(u, __VA_ARGS__(value));                  \
                                                                         \
    ASSERT_EQ(union1::Type::FIELD, u.getType());                         \
    EXPECT_EQ(expected, u.get_##FIELD());                                \
    EXPECT_SAME<                                                         \
        void,                                                            \
        decltype(apache::thrift::variant_set(u, __VA_ARGS__(value)))>(); \
  } while (false)

#define TEST_IMPL(TYPE, FIELD, VALUE)                         \
  do {                                                        \
    TEST_IMPL2(TYPE, FIELD, VALUE);                           \
    TEST_IMPL2(TYPE, FIELD, VALUE, static_cast<const TYPE&>); \
    TEST_IMPL2(TYPE, FIELD, VALUE, std::move);                \
  } while (false)

  TEST_IMPL(std::int32_t, ui, 10);
  TEST_IMPL(double, ud, 5.6);
  TEST_IMPL(std::string, us, "test");
  TEST_IMPL(enum1, ue, enum1::field2);

#undef TEST_IMPL
#undef TEST_IMPL2
}

} // namespace cpp_reflection
} // namespace test_cpp2
