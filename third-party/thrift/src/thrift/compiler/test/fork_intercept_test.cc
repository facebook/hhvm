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

#include <sys/types.h>
#include <folly/Subprocess.h>
#include <thrift/compiler/test/fork_intercept.h>

namespace apache::thrift::compiler {
namespace {

TEST(ForkInterceptTest, DirectForkInterception) {
  // The death test itself needs to fork, so we allow it here but the process
  // executing the death test will reinstall the intercept
  fork_intercept::set_intercept(false);
  ASSERT_DEATH(
      ([]() {
        fork_intercept::set_intercept(true);
        fork();
      })(),
      "INTERCEPTED: Direct fork\\(\\) override - intercepted fork\\(\\) call");
}

TEST(ForkInterceptTest, FollySubprocessInterception) {
  // This should be allowed by the fork intercept override since
  // folly::Subprocess does not use fork()
  std::vector<std::string> args = {"/bin/echo", "Hello world!"};
  auto proc = folly::Subprocess(args);
  proc.wait();
}

} // namespace
} // namespace apache::thrift::compiler
