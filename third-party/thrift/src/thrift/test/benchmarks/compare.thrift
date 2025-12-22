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

struct SubSubStruct {
  1: i64 subsubfield_1;
  2: i64 subsubfield_2;
  3: double subsubfield_3;
  4: double subsubfield_4;
  5: i32 subsubfield_5;
  6: i32 subsubfield_6;
  7: bool subsubfield_7;
}

struct SubStruct {
  1: SubSubStruct subfield_1;
  2: SubSubStruct subfield_2;
  3: SubSubStruct subfield_3;
  4: SubSubStruct subfield_4;
  5: SubSubStruct subfield_5;
}

struct MyNestedStruct {
  1: SubStruct field_1;
  2: SubStruct field_2;
  3: SubStruct field_3;
}

struct MyStruct {
  1: string field_1;
  2: string field_2;
  3: string field_3;
  4: string field_4;
  5: string field_5;
  6: string field_6;
  7: string field_7;
  8: string field_8;
  9: string field_9;
  10: string field_10;
  11: string field_11;
  12: string field_12;
  13: string field_13;
  14: string field_14;
  15: string field_15;
  16: string field_16;
  17: string field_17;
  18: string field_18;
  19: string field_19;
  20: string field_20;
  21: string field_21;
  22: string field_22;
  23: string field_23;
  24: string field_24;
  25: string field_25;
  26: string field_26;
  27: string field_27;
  28: string field_28;
  29: string field_29;
}

union MyUnion {
  1: string field_1;
  2: string field_2;
  3: string field_3;
  4: string field_4;
  5: string field_5;
  6: string field_6;
  7: string field_7;
  8: string field_8;
  9: string field_9;
  10: string field_10;
  11: string field_11;
  12: string field_12;
  13: string field_13;
  14: string field_14;
  15: string field_15;
  16: string field_16;
  17: string field_17;
  18: string field_18;
  19: string field_19;
  20: string field_20;
  21: string field_21;
  22: string field_22;
  23: string field_23;
  24: string field_24;
  25: string field_25;
  26: string field_26;
  27: string field_27;
  28: string field_28;
  29: string field_29;
}
