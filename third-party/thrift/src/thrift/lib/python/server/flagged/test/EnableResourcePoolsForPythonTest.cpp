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

#include <thrift/lib/python/server/flagged/EnableResourcePoolsForPython.h>
#include <thrift/lib/python/server/flagged/test/EnableResourcePoolsForPythonTest.h>

using namespace apache::thrift;

namespace apache::thrift::python::detail::test {

class EnableResourcePoolsForPythonFlagTest
    : public testing::TestWithParam<bool> {};

TEST_P(EnableResourcePoolsForPythonFlagTest, AreResourcePoolsEnabledForPython) {
  // GIVEN
  bool thriftFlag = GetParam();
  mockEnableResourcePoolsForPython(thriftFlag);

  auto expected = thriftFlag;

  // WHEN
  auto actual = areResourcePoolsEnabledForPython();

  // THEN
  EXPECT_EQ(actual, expected);
}

INSTANTIATE_TEST_CASE_P(
    EnableResourcePoolsForPythonTests,
    EnableResourcePoolsForPythonFlagTest,
    // The resource pool rollout to set enable_resource_pools_for_python
    // is now at 100%. This implies that the flag should be true everwhere.
    // To prepare for the complete removal of flag, and eventually
    // the pre-resource-pool (thread-manager) code, mark resource pools
    // as always true, irrespective of the value of the flag.
    // Even though much of this code now appears dead, leave it as it is
    // for an easy rollback if needed.
    testing::Values(true));

} // namespace apache::thrift::python::detail::test
