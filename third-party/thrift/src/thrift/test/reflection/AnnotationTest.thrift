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

include "thrift/annotation/scope.thrift"
include "thrift/annotation/cpp.thrift"

package "apache.org/thrift/test"

@cpp.RuntimeAnnotation
@scope.Field
struct Oncall {
  1: string name;
}

@cpp.RuntimeAnnotation
@scope.Struct
@scope.Field
struct Doc {
  1: string text;
}

@cpp.RuntimeAnnotation
@scope.Field
struct Sensitive {}

@scope.Field
struct Other {}

@Doc{text = "I am a struct"}
struct MyStruct {
  @Oncall{name = "thrift"}
  @Sensitive
  @Other
  1: string field;
}
