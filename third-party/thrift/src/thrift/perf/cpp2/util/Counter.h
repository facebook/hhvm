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
#include <string>
#include <glog/logging.h>
#include <folly/ThreadCachedInt.h>

namespace facebook {
namespace thrift {
namespace benchmarks {

class Counter {
 public:
  explicit Counter(std::string name)
      : name_(name), value_(0, 10000), lastQueryCount_(0), maxPerSec_(0) {}

  Counter& operator+=(uint32_t inc) {
    value_ += inc;
    return *this;
  }

  Counter& operator++() {
    ++value_;
    return *this;
  }

  double print(double secsSinceLastPrint) {
    double queryCount_ = value_.readFull();
    double lastSecAvg = (queryCount_ - lastQueryCount_) / secsSinceLastPrint;
    lastQueryCount_ = queryCount_;
    maxPerSec_ = std::max(maxPerSec_, lastSecAvg);
    if (queryCount_ > 0) { // Don't print unused counters
      LOG(INFO) << std::scientific << " | QPS: " << lastSecAvg
                << " | Max: " << maxPerSec_ << " | Total: " << queryCount_
                << " | Operation: " << name_;
    }
    return lastSecAvg;
  }

 private:
  std::string name_;
  folly::ThreadCachedInt<uint32_t> value_;
  double lastQueryCount_;
  double maxPerSec_;
};

} // namespace benchmarks
} // namespace thrift
} // namespace facebook
