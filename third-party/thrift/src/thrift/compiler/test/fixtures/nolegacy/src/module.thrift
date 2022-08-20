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

include "thrift/annotation/thrift.thrift"

@thrift.NoLegacy
package "test.dev/fixtures/nolegacy"

enum TestEnum {
  Value1 = 0,
  Value2 = 1,
}

exception TestError {
  1: TestEnum test_enum;
  2: i32 code;
}

struct TestMixin {
  1: string field1;
}

struct TestStruct {
  1: string bar = "baz";
  2: optional string baropt;
  3: TestError test_error;
  4: TestMixin test_mixin (cpp.mixin);
}

union TestUnion {
  1: TestEnum enumVal;
  2: TestStruct structVal;
}

service MyService {
  TestStruct query(1: TestUnion val);
}
