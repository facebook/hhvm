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

include "thrift/annotation/python.thrift"

package "thrift.com/python/test/reflection/included_container_typedef"

namespace py3 python_test.reflection

struct ReproStruct {
  1: i32 x;
}

// A named container typedef defined in an *included* file. Referencing it from a
// service method in another file exercised a reflection-codegen bug: the
// generated thrift_services_reflection.py qualified it with the *enclosing*
// program's module instead of this (defining) module.
typedef map<i32, string> ReproDepMap

// Same, but Py3Hidden -- mirrors the real infrasec `acl.IdentitySet`.
@python.Py3Hidden
typedef set<i64> ReproHiddenSet
