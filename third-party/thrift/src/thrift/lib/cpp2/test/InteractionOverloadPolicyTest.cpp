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
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/AlwaysAllowSheddingInteractionOverloadPolicy.h>
#include <thrift/lib/cpp2/async/TerminateInteractionOverloadPolicy.h>

using namespace ::testing;
using namespace apache::thrift;

THRIFT_FLAG_DECLARE_string(interaction_overload_protection_policy);

TEST(InteractionOverloadPolicyTest, CreateFromThriftFlag) {
  THRIFT_FLAG_SET_MOCK(interaction_overload_protection_policy, "default");
  auto policy = InteractionOverloadPolicy::createFromThriftFlag();
  EXPECT_TRUE(dynamic_cast<DefaultInteractionOverloadPolicy*>(policy.get()));

  THRIFT_FLAG_SET_MOCK(interaction_overload_protection_policy, "terminate");
  policy = InteractionOverloadPolicy::createFromThriftFlag();
  EXPECT_TRUE(dynamic_cast<TerminateInteractionOverloadPolicy*>(policy.get()));

  THRIFT_FLAG_SET_MOCK(interaction_overload_protection_policy, "random_string");
  policy = InteractionOverloadPolicy::createFromThriftFlag();
  EXPECT_TRUE(dynamic_cast<DefaultInteractionOverloadPolicy*>(policy.get()));
}
