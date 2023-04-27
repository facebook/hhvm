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

namespace py3 thrift.python.test

include "thrift/annotation/python.thrift"
include "thrift/annotation/scope.thrift"

@python.Adapter{
  name = "thrift.python.test.adapters.datetime.DatetimeAdapter",
  typeHint = "datetime.datetime",
}
@scope.Transitive
struct AsDatetime {
  1: string signature;
}

@AsDatetime{signature = "DatetimeTypedef"}
typedef i32 Datetime

@python.Adapter{
  name = "thrift.python.test.adapters.atoi.AtoiAdapter",
  typeHint = "int",
}
typedef string AdaptedInt2
typedef AdaptedInt2 AdaptedInt

@python.Adapter{
  name = "thrift.python.test.adapters.noop.Wrapper",
  typeHint = "thrift.python.test.adapters.noop.Wrapped[]",
}
struct Baz {
  1: string name;
}

@python.Adapter{
  name = "thrift.python.test.adapters.noop.Wrapper",
  typeHint = "thrift.python.test.adapters.noop.Wrapped[]",
}
typedef bool WrappedBool

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
}

union Bar {
  1: string baz;
  @AsDatetime{signature = "DatetimeField"}
  2: i32 ts;
}

@AsDatetime{signature = "DatetimeConstant"}
const i32 NINETEEN_EIGHTY_FOUR = 441792000;
