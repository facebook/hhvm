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

#include <thrift/conformance/stresstest/StressTest.h>

#include <fmt/core.h>
#include <folly/init/Init.h>

#include <thrift/conformance/stresstest/client/TestRunner.h>

namespace apache {
namespace thrift {
namespace stress {

// Stress tests may override the default main() behavior by defining this weak
// symbol.
FOLLY_ATTR_WEAK int customStressTestMain(int argc, char* argv[]);

} // namespace stress
} // namespace thrift
} // namespace apache

using namespace apache::thrift::stress;

int main(int argc, char* argv[]) {
  folly::init(&argc, &argv);

  if (customStressTestMain) {
    return customStressTestMain(argc, argv);
  }

  // create a test runner instance
  auto clientCfg = ClientConfig::createFromFlags();
  TestRunner testRunner(std::move(clientCfg));
  testRunner.runTests();

  return 0;
}
