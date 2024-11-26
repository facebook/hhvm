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

namespace py3 thrift.python.test

struct TestPropertyAsField {
  # The presence of this field caused a failure due to the conflict
  # with the built-in property. Keep this test to ensure
  # the fix continues to work.
  1: string property;
  2: string break_unless_used_with_renamed_built_in_property;
}
