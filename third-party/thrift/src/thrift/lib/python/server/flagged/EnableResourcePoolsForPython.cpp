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

#include <thrift/lib/cpp2/Flags.h>

#include <thrift/lib/python/server/flagged/EnableResourcePoolsForPython.h>

THRIFT_FLAG_DEFINE_bool(enable_resource_pools_for_python, true);

namespace apache::thrift::python::detail {

bool areResourcePoolsEnabledForPython() {
  // The resource pool rollout to set enable_resource_pools_for_python
  // is now at 100%. This implies that the flag should be true everwhere.
  // To prepare for the complete removal of flag, and eventually
  // the pre-resource-pool (thread-manager) code, mark resource pools
  // as always true, irrespective of the value of the flag.
  // Even though much of this code now appears dead, leave it as it is
  // for an easy rollback if needed.
  return true;
}

} // namespace apache::thrift::python::detail
