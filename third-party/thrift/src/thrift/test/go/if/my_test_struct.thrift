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

@thrift.AllowLegacyMissingUris
package;

namespace go 'github.com/facebook/fbthrift/thrift/test/go/if/my_test_struct'

enum MyTestEnum {
  FIRST = 1,
  SECOND = 2,
  THIRD = 3,
  FOURTH = 4,
}

struct MyTestStruct {
  1: bool on;
  2: byte b;
  3: i16 int16;
  4: i32 int32;
  5: i64 int64;
  6: double d;
  7: float f;
  8: string st;
  9: binary bin;
  10: map<string, string> stringMap;
  11: list<string> stringList;
  12: set<string> stringSet;
  13: MyTestEnum e;
}
