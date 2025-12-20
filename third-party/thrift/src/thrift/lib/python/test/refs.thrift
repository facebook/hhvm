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

@thrift.AllowLegacyMissingUris
package;

namespace py3 python_test

struct ComplexRef {
  1: string name;
  2: optional ComplexRef ref;
  3: optional list<i16> list_basetype_ref;
  4: optional list<ComplexRef> list_recursive_ref;
  5: optional set<i16> set_basetype_ref;
  6: optional set<ComplexRef> set_recursive_ref;
  7: optional map<i16, i16> map_basetype_ref;
  8: optional map<i16, ComplexRef> map_recursive_ref;
  9: optional list<ComplexRef> list_shared_ref;
  10: optional set<ComplexRef> set_const_shared_ref;
  11: optional ComplexRef recursive;
}

struct Circular {
  1: string val;
  2: optional Circular child;
  // This makes any init of struct fail with stack overflow
  // from infinite recursion in `getStandardDefaultValueForType`
  // 3: Circular unqualified_child;
}
