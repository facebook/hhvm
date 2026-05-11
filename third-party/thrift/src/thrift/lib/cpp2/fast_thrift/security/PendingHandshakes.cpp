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

#include <thrift/lib/cpp2/fast_thrift/security/PendingHandshakes.h>

#include <thrift/lib/cpp2/fast_thrift/security/FizzHandshakeHelper.h>

namespace apache::thrift::fast_thrift::security {

void PendingHandshakes::cancelAll() {
  // Drain into a local map: helper->cancel() routes back through
  // remove(this), and we don't want that erasing to mutate the map we're
  // iterating. After the loop the local map's HelperPtr deleters call
  // each helper's destroy(); with no DestructorGuards live by then,
  // that becomes an immediate delete.
  decltype(helpers_) drained;
  drained.swap(helpers_);
  for (auto& [_, helper] : drained) {
    folly::DelayedDestruction::DestructorGuard guard(helper.get());
    helper->cancel();
  }
}

} // namespace apache::thrift::fast_thrift::security
