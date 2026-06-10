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

package "facebook.com/testing"

namespace py3 ""

// This struct is intentionally NOT annotated with
// `@python.EnableUnsafeIssetInspection`. The thrift_library compiling this file
// passes `thrift_python_options = ["enable_locally_set_field_inspection"]`, which
// enables `get_locally_set_fields()` for every struct in the library.
struct StructWithIssetInspectionOption {
  1: i32 int_field;
  2: optional string opt_str_field;
  3: bool bool_field;
  4: optional list<i32> opt_list_field;
}
