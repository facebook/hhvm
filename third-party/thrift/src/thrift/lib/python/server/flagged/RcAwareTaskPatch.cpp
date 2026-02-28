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

#include <thrift/lib/python/server/flagged/RcAwareTaskPatch.h>

THRIFT_FLAG_DEFINE_bool(rc_aware_task_patch_enabled, false);

namespace apache::thrift::python::detail {

bool isRcAwareTaskPatchEnabled() {
  return THRIFT_FLAG(rc_aware_task_patch_enabled);
}

void mockRcAwareTaskPatchEnabled(bool enabled) {
  THRIFT_FLAG_SET_MOCK(rc_aware_task_patch_enabled, enabled);
}

} // namespace apache::thrift::python::detail
