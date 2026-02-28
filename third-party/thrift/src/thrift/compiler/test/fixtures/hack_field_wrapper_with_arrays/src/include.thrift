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

include "thrift/annotation/hack.thrift"
include "thrift/annotation/scope.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

@scope.Transitive
@hack.Wrapper{name = "\\MyFieldWrapper"}
struct AnnotationStruct {}

struct MyStruct {
  1: MyNestedStruct nested_struct;
}
@hack.Adapter{name = "\\MyAdapterInt"}
typedef i64 i64WithAdapter

@hack.Wrapper{name = "\\MyTypeIntWrapper", extraNamespace = "detail"}
typedef i64 i64WithWrapper

@hack.Adapter{name = "\\MyAdapter1"}
typedef map<string, MyStruct> AdaptedMap

@hack.Adapter{name = "\\MyAdapter1"}
typedef MyStruct AdaptedStruct

struct MyNestedStruct {
  @hack.FieldWrapper{name = "\\MyFieldWrapper"}
  1: i64 wrapped_field;
  @AnnotationStruct
  2: i64 annotated_field;
  @hack.Adapter{name = "\\MyAdapter1"}
  3: i64 adapted_type;
  @hack.FieldWrapper{name = "\\MyFieldWrapper"}
  @hack.Adapter{name = "\\MyAdapter1"}
  4: i64 adapted__and_wrapped_type;
  @hack.FieldWrapper{name = "\\MyFieldWrapper"}
  5: optional i64WithAdapter optional_adapted_and_wrapped_type;
  @hack.FieldWrapper{name = "\\MyFieldWrapper"}
  @hack.SkipCodegen{reason = "invalid map key"}
  6: optional map<MyStruct, string> invalid_key_map;
  @hack.FieldWrapper{name = "\\MyFieldWrapper"}
  7: i64WithWrapper wrapped_type_int;
  @hack.FieldWrapper{name = "\\MyFieldWrapper"}
  8: StructWithWrapper double_wrapped_struct;
  @hack.FieldWrapper{name = "\\MyFieldWrapper"}
  9: optional AdaptedMap optional_adapted_and_wrapped_type_map;
  @hack.FieldWrapper{name = "\\MyFieldWrapper"}
  10: optional AdaptedStruct optional_adapted_and_wrapped_type_struct;
}

struct MyComplexStruct {
  1: map<string, MyStruct> map_of_string_to_MyStruct;
  2: map<string, list<MyStruct>> map_of_string_to_list_of_MyStruct;
  3: map<string, map<string, i32>> map_of_string_to_map_of_string_to_i32;
  4: map<
    string,
    map<string, MyStruct>
  > map_of_string_to_map_of_string_to_MyStruct;
  5: list<
    map<string, list<MyStruct>>
  > list_of_map_of_string_to_list_of_MyStruct;
  6: list<map<string, MyStruct>> list_of_map_of_string_to_MyStruct;
  7: list<
    map<string, StructWithWrapper>
  > list_of_map_of_string_to_StructWithWrapper;
}

service Service1 {
  MyStruct func(1: string arg1, 2: MyStruct arg2);
  MyStruct func1(1: string arg1, 2: MyStruct arg2);
  i64WithWrapper func2(1: StructWithWrapper arg1, 2: i64WithWrapper arg2);
}

@hack.Wrapper{name = "\\MyStructWrapper"}
struct StructWithWrapper {
  1: i64 int_field;
}
