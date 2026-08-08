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

package "test.dev/fixtures/hack_final_structs"

struct FinalStruct {
  1: string value;
  2: optional map<string, i64> entries;
}

@hack.MigrationBlockingAllowInheritance
struct OpenStruct {
  1: string value;
  2: optional map<string, i64> entries;
}

union FinalUnion {
  1: string str_value;
  2: i64 int_value;
}

@hack.MigrationBlockingAllowInheritance
union OpenUnion {
  1: string str_value;
  2: i64 int_value;
}

exception FinalException {
  1: string message;
}

@hack.MigrationBlockingAllowInheritance
exception OpenException {
  1: string message;
}

@hack.StructAsTrait
struct StructAsTraitCannotBeFinal {
  1: string value;
}

@hack.MigrationBlockingAllowInheritance
@hack.Attributes{attributes = ["OtherAttribute"]}
struct OpenStructWithOtherAttributes {
  1: string value;
}

service FinalStructsService {
  FinalStruct getStruct(1: OpenStruct request) throws (1: FinalException err);
}
