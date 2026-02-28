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

#pragma once

#include <functional>

#include <gtest/gtest.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <thrift/conformance/if/gen-cpp2/test_suite_types.h>

// Registers the given conformance test suites with gtest
#define THRIFT_RPC_CLIENT_CONFORMANCE_TEST(suites, clientCmds, nonconforming) \
  static ::apache::thrift::conformance::detail::                              \
      RPCClientConformanceTestRegistration                                    \
      __suite_reg_##__LINE__(                                                 \
          suites, clientCmds, nonconforming, __FILE__, __LINE__)

namespace apache::thrift::conformance {

// Registers a test suite with gtest.
void registerTests(
    std::string_view name,
    const TestSuite& suite,
    const std::set<std::string>& nonconforming,
    std::pair<std::string_view, bool> clientCmd,
    const char* file = "",
    int line = 0);

namespace detail {

class RPCClientConformanceTestRegistration {
 public:
  RPCClientConformanceTestRegistration(
      std::vector<TestSuite> suites,
      std::map<std::string_view, std::pair<std::string_view, bool>> clientCmds,
      const std::set<std::string>& nonconforming,
      const char* file = "",
      int line = 0)
      : suites_(std::move(suites)) {
    for (const auto& entry : clientCmds) {
      for (const auto& suite : suites_) {
        registerTests(
            entry.first, suite, nonconforming, entry.second, file, line);
      }
    }
  }

 private:
  const std::vector<TestSuite> suites_;
};

} // namespace detail
} // namespace apache::thrift::conformance
