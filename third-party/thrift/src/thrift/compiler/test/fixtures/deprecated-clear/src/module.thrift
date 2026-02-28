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

namespace cpp2 apache.thrift.test

enum MyEnum {
  ME0 = 0,
  ME1 = 1,
}

struct StructWithDefaultStruct {
  1: bool bool_field = true;
  2: byte byte_field = 1;
  3: i16 short_field = 1;
  4: i32 int_field = 1;
  5: i64 long_field = 1;
  6: float float_field = 1.0;
  7: double double_field = 1.0;
  8: string string_field = "1";
  9: binary binary_field = "1";
  10: MyEnum enum_field = MyEnum.ME1;
  11: list<i16> list_field = [1];
  12: set<i16> set_field = [1];
  13: map<i16, i16> map_field = {1: 1};
}
