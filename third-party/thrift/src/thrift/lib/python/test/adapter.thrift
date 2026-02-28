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

package "thrift.com/python/test"
namespace py3 python_test

include "thrift/annotation/python.thrift"
include "thrift/annotation/scope.thrift"
include "thrift/annotation/thrift.thrift"
include "thrift/lib/python/test/dependency.thrift"

@python.Adapter{
  name = "thrift.python.test.adapters.datetime.DatetimeAdapter",
  typeHint = "datetime.datetime",
}
@scope.Transitive
struct AsDatetime {
  1: string signature;
}

@thrift.AllowLegacyTypedefUri
@AsDatetime{signature = "DatetimeTypedef"}
typedef i32 Datetime

@thrift.AllowLegacyTypedefUri
@python.Adapter{
  name = "thrift.python.test.adapters.atoi.AtoiAdapter",
  typeHint = "int",
}
typedef string AdaptedInt2
@thrift.AllowLegacyTypedefUri
typedef AdaptedInt2 AdaptedInt

@python.Adapter{
  name = "thrift.python.test.adapters.noop.Wrapper",
  typeHint = "thrift.python.test.adapters.noop.Wrapped[]",
}
struct Baz {
  1: string name;
}

@thrift.AllowLegacyTypedefUri
@python.Adapter{
  name = "thrift.python.test.adapters.noop.Wrapper",
  typeHint = "thrift.python.test.adapters.noop.Wrapped[]",
}
typedef bool WrappedBool

@python.Adapter{
  name = "thrift.python.test.adapters.noop.Wrapper",
  typeHint = "thrift.python.test.adapters.noop.Wrapped[]",
}
@scope.Transitive
struct AnnotationWithContainers {
  1: list<string> names;
  2: map<string, i32> counts;
}

struct Foo {
  @AsDatetime{signature = "DatetimeField"}
  1: i32 created_at;
  2: Datetime updated_at;
  @AsDatetime
  3: AdaptedInt another_time;
  4: list<AdaptedInt> int_list;
  5: set<AdaptedInt> int_set;
  6: map<AdaptedInt, Datetime> int_to_datetime_map;
  @python.Adapter{
    name = "thrift.python.test.adapters.atoi.ItoaListAdapter",
    typeHint = "typing.Sequence[str]",
  }
  7: list<AdaptedInt> adapted_list;
  8: Baz baz;
  @python.Adapter{
    name = "thrift.python.test.adapters.atoi.ItoaNestedListAdapter",
    typeHint = "typing.Sequence[typing.Sequence[typing.Mapping[str, str]]]",
  }
  9: list<list<map<AdaptedInt, AdaptedInt>>> adapted_list_nested;
  10: list<list<AdaptedInt>> int_list_list;
  11: WrappedBool wrapped_bool = true;
  @python.Adapter{
    name = "thrift.python.test.adapters.noop.Wrapper",
    typeHint = "thrift.python.test.adapters.noop.Wrapped[]",
  }
  12: WrappedBool double_wrapped_bool = true;
  @AnnotationWithContainers{names = ["bar", "baz"], counts = {"c": 3}}
  13: i32 abc;
}

union Bar {
  1: string baz;
  @AsDatetime{signature = "DatetimeField"}
  2: i32 ts;
}

@AsDatetime{signature = "DatetimeConstant"}
const i32 NINETEEN_EIGHTY_FOUR = 441792000;

union UnionMapWithAdaptedKeyAndValueTypes {
  1: map<AdaptedInt2, WrappedBool> field_1;
  2: map<AdaptedInt2, WrappedBool> field_2;
}

union UnionMapWithInvariantAdaptedValueTypes {
  1: map<AdaptedInt2, Baz> field_1;
  2: map<AdaptedInt2, Baz> field_2;
}

// Test case: uses an adapter-annotated struct from an included file.
// This should trigger import of the included adapter module in generated .pyi file.
struct UsesIncludedAdapter {
  @dependency.IncludedAdapter{signature = "IncludedAdapterField"}
  1: i32 adapted_field;
}

// Test case: const annotated with adapter from included file
@dependency.IncludedAdapter{signature = "IncludedAdapterConstant"}
const i32 CONST_WITH_INCLUDED_ADAPTER = 123456789;
