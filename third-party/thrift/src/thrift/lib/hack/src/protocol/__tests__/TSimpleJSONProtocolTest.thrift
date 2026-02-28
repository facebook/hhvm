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
 *
 */

package "meta.com/t_simple_json_protocol_test"

namespace php TSimpleJSONProtocolTest

struct StringVal {
  1: string s;
}

struct NumVals {
  1: i32 i;
  2: float f;
  3: map<i32, i32> m;
}

struct BinaryVal {
  1: binary s;
}
