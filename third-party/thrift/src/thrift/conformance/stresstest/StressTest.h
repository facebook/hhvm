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

#include <thrift/conformance/stresstest/client/StressTestBase.h>
#include <thrift/conformance/stresstest/client/StressTestRegistry.h>

// TODO: Consider adding autodeps support for the custom `thrift_stress_test`
// rule. Until then, extra dependencies must be added manually. For convenience
// reasons, we include a few useful coroutine headers here for use in stress
// test definitions.
#include <folly/coro/AsyncScope.h>
#include <folly/coro/Collect.h>
#include <folly/coro/Sleep.h>
#include <folly/coro/Task.h>

#define THRIFT_STRESS_TEST(name)                                            \
  class THRIFT_STRESS_TEST__##name                                          \
      : public ::apache::thrift::stress::StressTestBase {                   \
   public:                                                                  \
    ::folly::coro::Task<void> runWorkload(                                  \
        ::apache::thrift::stress::StressTestClient*) const override;        \
                                                                            \
   private:                                                                 \
    static bool isRegistered_;                                              \
  };                                                                        \
  bool THRIFT_STRESS_TEST__##name::isRegistered_ =                          \
      ::apache::thrift::stress::StressTestRegistry::getInstance().add(      \
          #name,                                                            \
          []() { return std::make_unique<THRIFT_STRESS_TEST__##name>(); }); \
  ::folly::coro::Task<void> THRIFT_STRESS_TEST__##name::runWorkload(        \
      ::apache::thrift::stress::StressTestClient* client) const
