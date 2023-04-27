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

#include <thrift/compiler/sema/const_checker.h>

#include <cmath>
#include <limits>

#include <folly/Portability.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/compiler/ast/t_base_type.h>
#include <thrift/compiler/ast/t_const_value.h>

namespace apache {
namespace thrift {
namespace compiler {

TEST(ConstCheckerTest, IsValidCustomDefaultIntegerOverAndUnderFlow) {
  auto custom_byte_over = t_const_value(
      static_cast<int64_t>(std::numeric_limits<int8_t>::max()) + 1);
  auto custom_byte_under = t_const_value(
      static_cast<int64_t>(std::numeric_limits<int8_t>::min()) - 1);
  auto custom_short_over = t_const_value(
      static_cast<int64_t>(std::numeric_limits<int16_t>::max()) + 1);
  auto custom_short_under = t_const_value(
      static_cast<int64_t>(std::numeric_limits<int16_t>::min()) - 1);
  auto custom_integer_over = t_const_value(
      static_cast<int64_t>(std::numeric_limits<int32_t>::max()) + 1);
  auto custom_integer_under = t_const_value(
      static_cast<int64_t>(std::numeric_limits<int32_t>::min()) - 1);
  EXPECT_FALSE(detail::is_valid_custom_default_integer(
      &t_base_type::t_byte(), &custom_byte_over));
  EXPECT_FALSE(detail::is_valid_custom_default_integer(
      &t_base_type::t_byte(), &custom_byte_under));
  EXPECT_FALSE(detail::is_valid_custom_default_integer(
      &t_base_type::t_i16(), &custom_short_over));
  EXPECT_FALSE(detail::is_valid_custom_default_integer(
      &t_base_type::t_i16(), &custom_short_under));
  EXPECT_FALSE(detail::is_valid_custom_default_integer(
      &t_base_type::t_i32(), &custom_integer_over));
  EXPECT_FALSE(detail::is_valid_custom_default_integer(
      &t_base_type::t_i32(), &custom_integer_under));
}

TEST(ConstCheckerTest, IsValidCustomDefaultIntegerEdge) {
  auto custom_byte_upper =
      t_const_value(static_cast<int64_t>(std::numeric_limits<int8_t>::max()));
  auto custom_byte_lower =
      t_const_value(static_cast<int64_t>(std::numeric_limits<int8_t>::min()));
  auto custom_short_upper =
      t_const_value(static_cast<int64_t>(std::numeric_limits<int16_t>::max()));
  auto custom_short_lower =
      t_const_value(static_cast<int64_t>(std::numeric_limits<int16_t>::min()));
  auto custom_integer_upper =
      t_const_value(static_cast<int64_t>(std::numeric_limits<int32_t>::max()));
  auto custom_integer_lower =
      t_const_value(static_cast<int64_t>(std::numeric_limits<int32_t>::min()));
  EXPECT_TRUE(detail::is_valid_custom_default_integer(
      &t_base_type::t_byte(), &custom_byte_upper));
  EXPECT_TRUE(detail::is_valid_custom_default_integer(
      &t_base_type::t_byte(), &custom_byte_lower));
  EXPECT_TRUE(detail::is_valid_custom_default_integer(
      &t_base_type::t_i16(), &custom_short_upper));
  EXPECT_TRUE(detail::is_valid_custom_default_integer(
      &t_base_type::t_i16(), &custom_short_lower));
  EXPECT_TRUE(detail::is_valid_custom_default_integer(
      &t_base_type::t_i32(), &custom_integer_upper));
  EXPECT_TRUE(detail::is_valid_custom_default_integer(
      &t_base_type::t_i32(), &custom_integer_lower));
}

TEST(ConstCheckerTest, IsValidCustomDefaultIntegerAssert) {
  auto custom_long = t_const_value(std::numeric_limits<int64_t>::min());
  auto custom_double = t_const_value();
  custom_double.set_double(1.0);

  if (folly::kIsDebug) {
    EXPECT_DEATH(
        detail::is_valid_custom_default_integer(
            &t_base_type::t_i64(), &custom_long),
        "");
    EXPECT_DEATH(
        detail::is_valid_custom_default_integer(
            &t_base_type::t_float(), &custom_double),
        "");
    EXPECT_DEATH(
        detail::is_valid_custom_default_integer(
            &t_base_type::t_double(), &custom_double),
        "");
  }
}

TEST(ConstCheckerTest, IsValidCustomDefaultFloatOverAndUnderFlow) {
  auto custom_float_over = t_const_value();
  custom_float_over.set_double(std::nextafter(
      static_cast<double>(std::numeric_limits<float>::max()),
      std::numeric_limits<double>::max()));
  auto custom_float_under = t_const_value();
  custom_float_under.set_double(std::nextafter(
      static_cast<double>(std::numeric_limits<float>::lowest()),
      std::numeric_limits<double>::lowest()));
  EXPECT_FALSE(detail::is_valid_custom_default_float(&custom_float_over));
  EXPECT_FALSE(detail::is_valid_custom_default_float(&custom_float_under));
}

TEST(ConstCheckerTest, IsValidCustomDefaultFloatEdge) {
  auto custom_float_upper = t_const_value();
  custom_float_upper.set_double(std::numeric_limits<float>::max());
  auto custom_float_lower = t_const_value();
  custom_float_lower.set_double(std::numeric_limits<float>::lowest());
  EXPECT_TRUE(detail::is_valid_custom_default_float(&custom_float_upper));
  EXPECT_TRUE(detail::is_valid_custom_default_float(&custom_float_lower));
}

TEST(ConstCheckerTest, IsValidCustomDefaultFloatWithIntegerValue) {
  auto custom_float = t_const_value(1e8);
  auto custom_double = t_const_value();
  EXPECT_TRUE(detail::is_valid_custom_default_float_with_integer_value<float>(
      &custom_float));
  EXPECT_TRUE(detail::is_valid_custom_default_float_with_integer_value<double>(
      &custom_double));
}

TEST(ConstCheckerTest, IsValidCustomDefaultFloatWithIntegerValueEdge) {
  auto custom_float_precision_loss_upper =
      t_const_value(static_cast<int64_t>(1e8) + 1);
  auto custom_float_precision_loss_lower =
      t_const_value(-static_cast<int64_t>(1e8) + 1);
  auto custom_double_precision_loss_upper =
      t_const_value(static_cast<int64_t>(1e16) + 1);
  auto custom_double_precision_loss_lower =
      t_const_value(-static_cast<int64_t>(1e16) + 1);
  EXPECT_FALSE(detail::is_valid_custom_default_float_with_integer_value<float>(
      &custom_float_precision_loss_upper));
  EXPECT_FALSE(detail::is_valid_custom_default_float_with_integer_value<float>(
      &custom_float_precision_loss_lower));
  EXPECT_FALSE(detail::is_valid_custom_default_float_with_integer_value<double>(
      &custom_double_precision_loss_upper));
  EXPECT_FALSE(detail::is_valid_custom_default_float_with_integer_value<double>(
      &custom_double_precision_loss_lower));
}

} // namespace compiler
} // namespace thrift
} // namespace apache
