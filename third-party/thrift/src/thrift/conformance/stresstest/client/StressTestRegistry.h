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

#include <folly/Function.h>
#include <folly/Indestructible.h>
#include <thrift/conformance/stresstest/client/StressTestBase.h>

namespace apache {
namespace thrift {
namespace stress {

/**
 * Registry for stress tests
 */
class StressTestRegistry {
 public:
  using Create = std::unique_ptr<StressTestBase>() const;

  static StressTestRegistry& getInstance();

  bool add(std::string name, folly::Function<Create> createFn);

  std::vector<std::string> listAll() const;

  std::unique_ptr<StressTestBase> create(std::string name) const;

 private:
  friend class folly::Indestructible<StressTestRegistry>;
  StressTestRegistry() = default;
  StressTestRegistry(StressTestRegistry&&) = delete;
  StressTestRegistry& operator=(StressTestRegistry&&) = delete;

  std::unordered_map<std::string, folly::Function<Create>> registry_;
};

} // namespace stress
} // namespace thrift
} // namespace apache
