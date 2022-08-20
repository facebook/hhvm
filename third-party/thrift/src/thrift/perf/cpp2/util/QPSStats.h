/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <glog/logging.h>
#include <folly/ThreadCachedInt.h>
#include <thrift/perf/cpp2/util/Counter.h>

namespace facebook {
namespace thrift {
namespace benchmarks {

class QPSStats {
 public:
  void printStats(double secsSinceLastPrint) {
    double totalQPS = 0;
    for (auto& pair : counters_) {
      totalQPS += pair.second->print(secsSinceLastPrint);
    }
    LOG(INFO) << std::scientific << " | TOTAL QPS: " << totalQPS;
  }

  void registerCounter(std::string name) {
    // TODO: Each thread in the Runner creates an instance of an Operation.
    // Each instance of the operation calls registerCounter with given name.
    // So this function is being called as the number of threads.
    // We should make this function to be called per type of the Operation, not
    // per instance.
    std::lock_guard<std::mutex> guard(mutex_);
    counters_.emplace(name, std::make_unique<Counter>(name));
  }

  void add(std::string& name) { ++(*counters_[name]); }

  void add(std::string& name, uint32_t sz) { (*counters_[name]) += sz; }

 private:
  std::map<std::string, std::unique_ptr<Counter>> counters_;
  std::mutex mutex_;
};

} // namespace benchmarks
} // namespace thrift
} // namespace facebook
