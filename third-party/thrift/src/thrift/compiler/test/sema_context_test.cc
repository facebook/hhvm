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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thrift/compiler/sema/sema_context.h>

namespace apache::thrift::compiler {

using testing::ThrowsMessage;

TEST(sema_params_test, parse_validation_level) {
  EXPECT_EQ(
      sema_params::parse_validation_level("none"),
      sema_params::validation_level::none);
  EXPECT_EQ(
      sema_params::parse_validation_level("warn"),
      sema_params::validation_level::warn);
  EXPECT_EQ(
      sema_params::parse_validation_level("error"),
      sema_params::validation_level::error);
  EXPECT_THAT(
      []() { sema_params::parse_validation_level("invalid"); },
      ThrowsMessage<std::runtime_error>("Unknown validation level: 'invalid'"));
}

TEST(sema_params_test, validation_level_to_string) {
  EXPECT_EQ(
      "none",
      sema_params::validation_level_to_string(
          sema_params::validation_level::none));
  EXPECT_EQ(
      "warn",
      sema_params::validation_level_to_string(
          sema_params::validation_level::warn));
  EXPECT_EQ(
      "error",
      sema_params::validation_level_to_string(
          sema_params::validation_level::error));
  EXPECT_EQ(
      "<unknown validation_level: 42>",
      sema_params::validation_level_to_string(
          static_cast<sema_params::validation_level>(42)));
}

} // namespace apache::thrift::compiler
