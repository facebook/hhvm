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

#include <folly/executors/FunctionScheduler.h>

#include <thrift/conformance/stresstest/client/ClientConfig.h>
#include <thrift/conformance/stresstest/client/StressTestBase.h>
#include <thrift/conformance/stresstest/common/StressTestStats.h>

namespace apache::thrift::stress {

class ClientRunner;

class TestRunner {
 public:
  explicit TestRunner(ClientConfig cfg);
  ~TestRunner() { functionScheduler_.shutdown(); }

  const std::vector<std::string>& getSelectedTests() const;
  const std::vector<std::string>& getAvailableTests() const;
  bool isRegistered(std::string testName) const;

  std::unique_ptr<StressTestBase> instantiate(std::string testName) const;

  StressTestStats run(std::string testName);
  StressTestStats run(std::unique_ptr<StressTestBase> test);

  void runTests();

  void scheduleContinuousStats(ClientRunner& runner);

 private:
  const ClientConfig cfg_;
  const std::vector<std::string> availableTests_;
  std::vector<std::string> selectedTests_;
  folly::FunctionScheduler functionScheduler_;

  void runContinuously();
  void runContinuously(std::unique_ptr<StressTestBase> test);
  void runFixedTime();
  void runFixedCount();
};

} // namespace apache::thrift::stress
