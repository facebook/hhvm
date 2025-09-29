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

namespace py3 thrift.test.thrift_python

struct TestStruct {
  1: string unqualified_string;
  2: optional string optional_string;
}

union TestUnion {
  1: string string_field;
  2: i32 int_field;
  3: TestStruct struct_field;
}

# This union is used to illustrate the (potentially dangerous) behavior of the
# (immutable) thrift-python union factory method: fromValue(), which tries to
# infer the field to set from the first field that can hold that value.
union TestUnionAmbiguousFromValueIntBool {
  1: i32 int_field;
  2: bool bool_field;
}

# See TestUnionAmbiguousFromValueIntBool above.
union TestUnionAmbiguousFromValueBoolInt {
  1: bool bool_field;
  2: i32 int_field;
}

# See TestUnionAmbiguousFromValueIntBool above.
union TestUnionAmbiguousFromValueFloatInt {
  1: float float_field;
  2: i32 int_field;
}

# This union intentionaly uses a field name that is used by the (immutable)
# thrift-python union class implementation to expose an enumeration with all
# possible fields. See union_test.py for the resulting behavior.
#
union TestUnionAmbiguousTypeFieldName {
  @python.Name{name = "Type_"}
  1: i32 Type;
}

# This union intentionaly uses field names that are used internally by the
# (immutable) thrift-python union class implementation. See union_test.py for
# the resulting behavior.
union TestUnionAmbiguousValueFieldName {
  @python.Name{name = "type_"}
  1: i32 type;
  @python.Name{name = "value_"}
  2: i32 value;
}
# NOTE: Error on import (due to fake "EMPTY" enum value):
# TypeError: Attempted to reuse key: 'EMPTY'
# union TestUnionWithEmptyFieldName {
#   1: i32 EMPTY;
# }

union TestUnionAdaptedTypes {
  @python.Adapter{
    name = "thrift.python.test.adapters.datetime.DatetimeAdapter",
    typeHint = "datetime.datetime",
  }
  1: i32 adapted_i32_to_datetime;

  @python.Adapter{
    name = "thrift.python.test.adapters.atoi.AtoiAdapter",
    typeHint = "int",
  }
  2: string adapted_string_to_i32;

  3: i32 non_adapted_i32;
}

union DefaultFieldUnion {
  1: i32 default_int = 1;
  2: i32 useless_int_default = 0;
  4: list<i32> default_list = [1];
  5: list<i32> useless_list_default = [];
}

union TestUnionContainerTypes {
  1: i32 int_field;
  2: list<i32> list_i32;
  3: set<i32> set_i32;
  4: map<i32, string> map_i32_str;
}
