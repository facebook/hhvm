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
 *
 */

include "TypeAndStructWrapperTest.thrift"
include "thrift/annotation/hack.thrift"

package "meta.com/thrift/wrapper_test"

namespace hack "WrapperTest"

struct ModuleMyStruct {
  1: i64 int_field;
}

union MyUnion {
  @TypeAndStructWrapperTest.AnnotationStruct
  1: i64 union_annotated_field;
  @hack.Adapter{name = "\AdapterTestIntToString"}
  3: i64 union_adapted_type;
}

exception MyException {
  1: i64 code;
  2: string message;
  @TypeAndStructWrapperTest.AnnotationStruct
  3: string annotated_message;
}

service Service {
  i32 func(1: string arg1, 2: TypeAndStructWrapperTest.TestMyStruct arg2);
}
