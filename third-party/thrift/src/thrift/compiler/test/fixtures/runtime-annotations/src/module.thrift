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
include "thrift/annotation/scope.thrift"

@thrift.Experimental
package "facebook.com/thrift/test/fixtures/runtime_annotations"

@scope.Definition
@thrift.RuntimeAnnotation
struct MyAnnotation {}

@MyAnnotation
struct MyStruct {
  @MyAnnotation
  1: i32 field;
}

@MyAnnotation
struct MyUnion {
  @MyAnnotation
  1: i32 field;
}

@MyAnnotation
exception MyException {
  @MyAnnotation
  1: i32 field;
}

@MyAnnotation
enum MyEnum {
  @MyAnnotation
  VALUE = 1,
}
