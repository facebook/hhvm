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

#include <thrift/conformance/stresstest/util/Util.h>

#include <folly/memory/MallctlHelper.h>

namespace apache {
namespace thrift {
namespace stress {

namespace {
size_t* getStatsPtr(const char* name) {
  size_t* ptr{nullptr};
  if (folly::usingJEMalloc()) {
    folly::mallctlRead(name, &ptr);
  }
  return ptr;
}
} // namespace

void resetMemoryStats() {
  if (folly::usingJEMalloc()) {
    mallctl("prof.reset", nullptr, nullptr, nullptr, 0);
  }
}

size_t getThreadMemoryUsage() {
  static size_t* allocatedp = getStatsPtr("thread.allocatedp");
  static size_t* deallocatedp = getStatsPtr("thread.deallocatedp");
  return allocatedp && deallocatedp ? (*allocatedp - *deallocatedp) : 0;
}

} // namespace stress
} // namespace thrift
} // namespace apache
