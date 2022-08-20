/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

namespace cpp2 thrift.test

cpp_include "thrift/test/CustomStruct.h"

struct MyStruct {
  1: string stringData;
  2: i32 intData;
}

union MyUnion {
  1: string stringData;
  2: i32 intData;
}

typedef MyStruct (cpp.type = "MyCustomStruct") SpecializedStruct
typedef MyUnion (cpp.type = "MyCustomUnion") SpecializedUnion

struct Container {
  1: SpecializedStruct myStruct;
  2: SpecializedUnion myUnion1;
  3: SpecializedUnion myUnion2;
  4: list<SpecializedStruct> myStructList;
  5: list<SpecializedUnion> myUnionList;
  6: map<i32, SpecializedStruct> myStructMap;
  7: map<i32, SpecializedUnion> myUnionMap;
  8: map<SpecializedStruct, string> myRevStructMap;
  9: map<SpecializedUnion, string> myRevUnionMap;
}

service CustomStruct {
  SpecializedStruct echoStruct(1: SpecializedStruct s);
  SpecializedUnion echoUnion(1: SpecializedUnion u);
}
