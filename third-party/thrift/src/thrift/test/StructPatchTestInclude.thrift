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

include "thrift/lib/thrift/patch.thrift"

@patch.GeneratePatch
package "facebook.com/thrift/test/patch"

namespace cpp2 apache.thrift.test.patch
namespace py3 thrift.test

struct MyData {
  1: string data1;
  2: i32 data2;
  3: optional string data3;
}

union MyInnerUnion {
  1: string option1;
}

union MyUnion {
  1: string option1;
  2: i32 option2;
  3: MyInnerUnion option3;
}
