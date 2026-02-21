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

include "thrift/lib/thrift/any.thrift"
include "thrift/lib/thrift/patch.thrift"

package "apache.org/thrift/protocol"

@patch.GeneratePatchNew
union MyUnion {
  1: string s;
  2: i32 i;
  3: MyStruct strct;
  4: map<i32, MyStruct> m;
}

@patch.GeneratePatchNew
struct MyStruct {
  1: any.Any any;
}

@patch.GeneratePatchNew
struct Sets {
  1: set<string> stringSet;
  2: set<binary> binarySet;
}

struct StructWithAny {
  1: any.Any any;
}

// Structs for testing custom default value preservation in patch conversion
@patch.GeneratePatchNew
struct CustomDefaultOptions {
  1: bool enabled = true;
  2: i32 thresholdPercent = 90;
  3: bool allowEmpty = false;
}

@patch.GeneratePatchNew
struct OuterStructWithCustomDefaults {
  1: string name;
  2: optional CustomDefaultOptions options;
}
