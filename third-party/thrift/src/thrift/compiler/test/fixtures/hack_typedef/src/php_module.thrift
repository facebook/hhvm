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

include "php_include.thrift"

namespace php include_typedef

typedef i32 MyI32
typedef php_include.IncludedMyI32 IncludedMyI32
typedef php_include.IncludedFoo IncludedFoo

struct Foo {
  1: MyI32 i_field;
  2: i32 i_field2;
}
