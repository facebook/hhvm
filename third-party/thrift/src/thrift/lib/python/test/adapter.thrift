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

@python.Adapter{
  name = "thrift.python.test.adapters.datetime.DatetimeAdapter",
  typeHint = "datetime.datetime",
}
typedef i32 Datetime

@python.Adapter{
  name = "thrift.python.test.adapters.atoi.AtoiAdapter",
  typeHint = "int",
}
typedef string AdaptedInt

@python.Adapter{
  name = "thrift.python.test.adapters.noop.Wrapper",
  typeHint = "thrift.python.test.adapters.noop.Wrapped",
}
struct Baz {
  1: string name;
}

struct Foo {
  @python.Adapter{
    name = "thrift.python.test.adapters.datetime.DatetimeAdapter",
    typeHint = "datetime.datetime",
  }
  1: i32 created_at;
  2: Datetime updated_at;
  @python.Adapter{
    name = "thrift.python.test.adapters.datetime.DatetimeAdapter",
    typeHint = "datetime.datetime",
  }
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
}

union Bar {
  1: string baz;
  @python.Adapter{
    name = "thrift.python.test.adapters.datetime.DatetimeAdapter",
    typeHint = "datetime.datetime",
  }
  2: i32 ts;
}
