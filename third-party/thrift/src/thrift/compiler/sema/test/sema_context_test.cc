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

TEST(sema_params_test, parseValidationLevel) {
  EXPECT_EQ(
      sema_params::parseValidationLevel("none"),
      sema_params::ValidationLevel::None);
  EXPECT_EQ(
      sema_params::parseValidationLevel("warn"),
      sema_params::ValidationLevel::Warn);
  EXPECT_EQ(
      sema_params::parseValidationLevel("error"),
      sema_params::ValidationLevel::Error);
  EXPECT_THAT(
      [&]() { sema_params::parseValidationLevel("invalid"); },
      ThrowsMessage<std::runtime_error>("Unknown validation level: 'invalid'"));
}

} // namespace apache::thrift::compiler
