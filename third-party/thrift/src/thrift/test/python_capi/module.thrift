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

include "thrift/annotation/cpp.thrift"
include "thrift/lib/thrift/patch.thrift"

cpp_include "thrift/test/python_capi/adapter.h"

package "thrift.org/test/python_capi"

enum MyEnum {
  MyValue1 = 0,
  MyValue2 = 1,
}

enum AnnoyingEnum {
  FOO = 1 (cpp.name = "l0O1"),
  BAR = 2 (cpp.name = "FuBaR"),
} (cpp.name = "NormalDecentEnum")

@patch.GeneratePatch
struct MyStruct {
  1: i64 inty;
  2: string stringy;
  3: MyDataItem myItemy;
  4: MyEnum myEnumy;
  5: bool booly (cpp.name = "boulet");
  6: list<float> floatListy;
  7: map<binary, string> strMappy;
  8: set<i32> intSetty;
}

@patch.GeneratePatch
struct MyDataItem {
  1: string s;
}

@cpp.Adapter{name = "::thrift::test::lib::StructDoubler"}
@scope.Transitive
struct TransitiveDoubler {}

@TransitiveDoubler
struct DoubledPair {
  1: string s;
  2: i32 x;
}

struct StringPair {
  1: string normal;
  @cpp.Adapter{name = "::thrift::test::lib::StringDoubler"}
  2: string doubled;
}

struct EmptyStruct {} (cpp.name = "VapidStruct")

union MyUnion {
  1: MyEnum myEnum;
  2: MyStruct myStruct;
  3: MyDataItem myDataItem;
  4: set<i64> intSet;
  5: list<double> doubleList;
  6: map<binary, string> strMap;
} (cpp.name = "OurUnion")
