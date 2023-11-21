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

include "included.thrift"
include "namespaced.thrift"
include "thrift/annotation/cpp.thrift"

@included.structured_annotation_included{name = 'aba'}
package "test.dev/fixtures/basic-structured-annotations"

@cpp.RuntimeAnnotation
struct runtime_annotation {}

struct structured_annotation_inline {
  1: i64 count;
  2: string name = 'abacaba';
}

struct structured_annotation_with_default {
  1: string name = 'abacabadabacaba';
}

struct structured_annotation_recursive {
  1: string name;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional structured_annotation_recursive recurse;
  3: structured_annotation_forward forward;
}

struct structured_annotation_forward {
  1: i64 count;
}

struct structured_annotation_nested {
  1: string name;
  2: structured_annotation_with_default nest;
}

@structured_annotation_with_default
typedef string annotated_with_default_string

@structured_annotation_inline{count = 1}
@structured_annotation_with_default{name = 'abc'}
typedef string annotated_inline_string

@structured_annotation_inline{count = 2}
typedef i64 annotated_inline_i64

@structured_annotation_inline{count = 2}
@structured_annotation_with_default{}
@structured_annotation_nested{
  name = 'nesty2',
  nest = structured_annotation_with_default{name = 'dcdbdcdadcdbdcd'},
}
@included.structured_annotation_included{name = 'aba'}
@namespaced.structured_annotation_with_namespace{name = 'bac'}
@runtime_annotation
struct MyStruct {
  @structured_annotation_inline{count = 1, name = 'counter'}
  @runtime_annotation
  1: i64 annotated_field;

  2: annotated_inline_string annotated_type;

  @structured_annotation_recursive{
    name = "abc",
    recurse = structured_annotation_recursive{name = "cba"},
    forward = structured_annotation_forward{count = 3},
  }
  3: string annotated_recursive;

  @structured_annotation_nested{name = 'nesty'}
  4: i64 annotated_nested;
}

@structured_annotation_nested{name = 'nesty'}
exception MyException {
  @structured_annotation_with_default
  1: string context;
}

@structured_annotation_nested{
  name = 'nesty',
  nest = structured_annotation_with_default{},
}
union MyUnion {
  @structured_annotation_with_default
  1: annotated_inline_string first;

  @structured_annotation_with_default{name = 'aba'}
  2: annotated_inline_i64 second;
}

@structured_annotation_inline{count = 3}
service MyService {
  @structured_annotation_with_default{}
  annotated_inline_string first();

  @structured_annotation_inline{count = 2}
  bool second(
    @structured_annotation_inline{count = 4}
    1: i64 count,
  );
}

@structured_annotation_inline{count = 4}
enum MyEnum {
  @structured_annotation_with_default{name = 'unknown'}
  UNKNOWN = 0,

  @structured_annotation_with_default{name = 'one'}
  ONE = 1,
}

@structured_annotation_inline{name = 'MyHackEnum'}
const map<string, string> MyConst = {'ENUMERATOR': 'enum', 'CONST': 'const'};
