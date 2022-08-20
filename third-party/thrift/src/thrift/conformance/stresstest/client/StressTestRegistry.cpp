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

#include <thrift/conformance/stresstest/client/StressTestRegistry.h>

namespace apache {
namespace thrift {
namespace stress {

StressTestRegistry& StressTestRegistry::getInstance() {
  static folly::Indestructible<StressTestRegistry> instance;
  return *instance;
}

bool StressTestRegistry::add(
    std::string name, folly::Function<Create> createFn) {
  return registry_.try_emplace(name, std::move(createFn)).second;
}

std::vector<std::string> StressTestRegistry::listAll() const {
  std::vector<std::string> ret;
  for (const auto& [name, _] : registry_) {
    ret.push_back(name);
  }
  return ret;
}

std::unique_ptr<StressTestBase> StressTestRegistry::create(
    std::string name) const {
  auto ptr = folly::get_ptr(registry_, name);
  return !ptr ? nullptr : (*ptr)();
}

} // namespace stress
} // namespace thrift
} // namespace apache
