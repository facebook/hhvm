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

include "thrift/annotation/cpp.thrift"

namespace cpp2 apache.thrift.test
namespace py3 thrift.test.lazy_deserialization

struct Foo {
  1: list<double> field1; // fast to skip in CompactProtocol
  2: list<i32> field2; // slow to skip in CompactProtocol
  3: list<double> field3; // fast to skip in CompactProtocol
  4: list<i32> field4; // slow to skip in CompactProtocol
}

// Identical to Foo, except field3 and field4 are lazy
struct LazyFoo {
  1: list<double> field1;
  2: list<i32> field2;
  3: list<double> field3 (cpp.experimental.lazy);
  @cpp.Lazy{ref = true}
  4: list<i32> field4;
}

struct OptionalFoo {
  1: optional list<double> field1;
  2: optional list<i32> field2;
  3: optional list<double> field3;
  4: optional list<i32> field4;
}

struct OptionalLazyFoo {
  1: optional list<double> field1;
  2: optional list<i32> field2;
  3: optional list<double> field3 (cpp.experimental.lazy);
  4: optional list<i32> field4 (cpp.experimental.lazy);
}

struct LazyCppRef {
  1: optional list<i32> field1 (cpp.ref_type = "unique", cpp.experimental.lazy);
  2: optional list<i32> field2 (cpp.ref_type = "shared", cpp.experimental.lazy);
  3: optional list<i32> field3 (
    cpp.ref_type = "shared_const",
    cpp.experimental.lazy,
  );
  4: list<i32> field4 (cpp.ref_type = "unique", cpp.experimental.lazy);
}

// Same as Foo, except adding index field explicitly
// Since we can't use negative as index field, we will change id
// in serialized data manually
struct IndexedFoo {
  100: double serialized_data_size;

  1: list<double> field1;
  2: list<i32> field2;
  3: list<double> field3;
  4: list<i32> field4;

  101: map<i16, i64> field_id_to_size;
}

struct OptionalIndexedFoo {
  100: double serialized_data_size;

  1: optional list<double> field1;
  2: optional list<i32> field2;
  3: optional list<double> field3;
  4: optional list<i32> field4;

  101: map<i16, i64> field_id_to_size;
}

struct Empty {}

const i32 kSizeId = 100;
const i32 kIndexId = 101;
