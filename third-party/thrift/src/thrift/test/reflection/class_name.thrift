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

package "apache.org/thrift/test"

include "thrift/annotation/cpp.thrift"

struct MyStruct {
  1: i32 field;
}

union MyUnion {
  1: i32 field;
}

permanent client exception MyException {
  1: i32 field;
}

@cpp.Name{value = "RenamedMyStruct"}
struct MyStruct2 {
  1: i32 field;
}

@cpp.Name{value = "RenamedMyUnion"}
union MyUnion2 {
  1: i32 field;
}

@cpp.Name{value = "RenamedMyException"}
permanent client exception MyException2 {
  1: string message;
}
