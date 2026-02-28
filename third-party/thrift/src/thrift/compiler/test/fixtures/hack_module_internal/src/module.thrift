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
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace hack hack.fixtures
namespace hack.module hack.module.internal

@hack.ModuleInternal
typedef i32 int_typedef

@hack.ModuleInternal
enum FooEnum {
}

@hack.ModuleInternal
struct Foo {
  1: i32 i_field;
  @hack.ModuleInternal
  2: string str_field;
}

@hack.ModuleInternal
union FooUnion {
  1: i32 int_field;
  2: string str_field;
}

service TestServiceWithMethodAnnotation {

  @hack.ModuleInternal
  i32 testMethodWithAnnotation();
  void testMethodWithoutAnnotation();
}

@hack.ModuleInternal
service TestServiceWithServiceAnnotation {
  i32 testMethodWithServiceAnnotation();
  void testMethodWithServiceAnnotation2();
}
