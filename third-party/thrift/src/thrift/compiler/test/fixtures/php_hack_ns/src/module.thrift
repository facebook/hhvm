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

@hack.ConstantsClass{name = "AnnotatedPhpConstants"}
@hack.NamePrefix{prefix = "ProgramPrefixed"}
package "test.dev/foo/php/ns"

const i32 ANSWER = 42;

@hack.NamePrefix{prefix = "Prefixed"}
typedef string TestTypedef

enum Status {
  Unknown = 0,
}

@hack.NamePrefix{prefix = "Prefixed"}
union TestUnion {
  1: string string_value;
}

@hack.NamePrefix{prefix = "Prefixed", apply_on_getName = false}
exception TestException {
  1: string message;
}

@hack.NamePrefix{prefix = "Double_", apply_on_getName = false}
@hack.Name{name = "Prefixed_"}
struct TestStruct {
  1: string str_value;
  2: TestTypedef typedef_value;
  3: TestUnion union_value;
}

@hack.NamePrefix{prefix = "Deprecated_"}
service FooHackService {
  Status fetchStatus(1: TestStruct request);
  TestStruct ping(1: string str_arg) throws (1: TestException ex);
}
