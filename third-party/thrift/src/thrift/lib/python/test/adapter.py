# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# pyre-strict


from __future__ import annotations

import time
import unittest
from datetime import datetime
from unittest.mock import MagicMock

from thrift.python.test.adapters.datetime import DatetimeAdapter

DatetimeAdapter_from_thrift = MagicMock(wraps=DatetimeAdapter.from_thrift)
DatetimeAdapter.from_thrift = DatetimeAdapter_from_thrift
DatetimeAdapter_to_thrift = MagicMock(wraps=DatetimeAdapter.to_thrift)
DatetimeAdapter.to_thrift = DatetimeAdapter_to_thrift
DatetimeAdapter_from_thrift_field = MagicMock(wraps=DatetimeAdapter.from_thrift_field)
DatetimeAdapter.from_thrift_field = DatetimeAdapter_from_thrift_field
DatetimeAdapter_to_thrift_field = MagicMock(wraps=DatetimeAdapter.to_thrift_field)
DatetimeAdapter.to_thrift_field = DatetimeAdapter_to_thrift_field

from thrift.python.test.adapter.thrift_types import (
    AdaptedInt,
    Bar,
    Baz,
    Datetime,
    Foo,
    NINETEEN_EIGHTY_FOUR,
)
from thrift.python.test.adapters.noop import Wrapped

# @manual=//thrift/lib/python/test:adapter_thrift-python-types
from thrift.python.test.adapter.thrift_types import (
    _fbthrift_unadapted_AsDatetime,
    _fbthrift_unadapted_Baz,
    _fbthrift_unadapted_NINETEEN_EIGHTY_FOUR,
)  # isort:skip


class AdapterTest(unittest.TestCase):
    def test_round_trip(self) -> None:
        now = datetime.fromtimestamp(int(time.time()))
        foo = Foo(created_at=now)
        self.assertIsInstance(foo.created_at, datetime)
        self.assertEqual(foo.created_at, now)

    def test_update(self) -> None:
        now = datetime.fromtimestamp(int(time.time()))
        foo = Foo(created_at=now)
        future = datetime.fromtimestamp(int(time.time()) + 100)
        foo = foo(created_at=future)
        self.assertEqual(foo.created_at, future)

    def test_union(self) -> None:
        now = datetime.fromtimestamp(int(time.time()))
        bar = Bar(ts=now)
        self.assertEqual(bar.ts, now)

    def test_struct_adapter_called_with_transitive_annotation(self) -> None:
        DatetimeAdapter_from_thrift.reset_mock()
        DatetimeAdapter_to_thrift.reset_mock()
        now_ts = int(time.time())
        now = datetime.fromtimestamp(now_ts)
        foo = Foo(updated_at=now)
        DatetimeAdapter_to_thrift.assert_called_once_with(
            now,
            transitive_annotation=_fbthrift_unadapted_AsDatetime(
                signature="DatetimeTypedef"
            ),
        )
        foo.updated_at
        DatetimeAdapter_from_thrift.assert_called_once_with(
            now_ts,
            transitive_annotation=_fbthrift_unadapted_AsDatetime(
                signature="DatetimeTypedef"
            ),
        )

    def test_field_adapter_called_with_field_id_and_trasitive_annotation(self) -> None:
        DatetimeAdapter_from_thrift_field.reset_mock()
        DatetimeAdapter_to_thrift_field.reset_mock()
        now_ts = int(time.time())
        now = datetime.fromtimestamp(now_ts)
        foo = Foo(created_at=now)
        DatetimeAdapter_to_thrift_field.assert_called_once_with(
            now,
            1,
            foo,
            transitive_annotation=_fbthrift_unadapted_AsDatetime(
                signature="DatetimeField"
            ),
        )
        foo.created_at
        DatetimeAdapter_from_thrift_field.assert_called_once_with(
            now_ts,
            1,
            foo,
            transitive_annotation=_fbthrift_unadapted_AsDatetime(
                signature="DatetimeField"
            ),
        )

        DatetimeAdapter_from_thrift_field.reset_mock()
        DatetimeAdapter_to_thrift_field.reset_mock()
        bar = Bar(ts=now)
        DatetimeAdapter_to_thrift_field.assert_called_once_with(
            now,
            2,
            bar,
            transitive_annotation=_fbthrift_unadapted_AsDatetime(
                signature="DatetimeField"
            ),
        )
        bar.ts
        DatetimeAdapter_from_thrift_field.assert_called_once_with(
            now_ts,
            2,
            bar,
            transitive_annotation=_fbthrift_unadapted_AsDatetime(
                signature="DatetimeField"
            ),
        )

    def test_typedef_field(self) -> None:
        now = datetime.fromtimestamp(int(time.time()))
        foo = Foo(updated_at=now)
        self.assertIsInstance(foo.updated_at, datetime)
        self.assertEqual(foo.updated_at, now)

    def test_double_adapters(self) -> None:
        now = datetime.fromtimestamp(int(time.time()))
        foo = Foo(another_time=now)
        self.assertIsInstance(foo.another_time, datetime)
        self.assertEqual(foo.another_time, now)

    def test_typedefs(self) -> None:
        self.assertEqual(datetime, Datetime)
        self.assertEqual(int, AdaptedInt)

    def test_adapted_type_in_containers(self) -> None:
        int_list = [1, 2, 3]
        int_list_list = [[1, 2, 3], [4, 5, 6]]
        int_set = {4, 5, 6}
        int_to_datetime_map = {42: datetime.fromtimestamp(int(time.time()))}
        foo = Foo(
            int_list=int_list,
            int_set=int_set,
            int_to_datetime_map=int_to_datetime_map,
            int_list_list=int_list_list,
        )
        self.assertEqual(foo.int_list, int_list)
        self.assertEqual(foo.int_set, int_set)
        self.assertEqual(foo.int_to_datetime_map, int_to_datetime_map)
        self.assertEqual(foo.int_list_list, int_list_list)

    def test_adapted_container_of_adapted_type(self) -> None:
        str_list = ["1", "1", "2", "3", "5", "8", "13"]
        foo = Foo(adapted_list=str_list)
        self.assertEqual(foo.adapted_list, str_list)

    def test_adapted_nested_container_of_adapted_type(self) -> None:
        str_list = [
            [{"1": "2", "3": "4"}, {"1": "5"}],
            [{"2": "6"}],
            [{"3": "5"}, {"8": "13"}],
        ]
        foo = Foo(adapted_list_nested=str_list)
        self.assertEqual(foo.adapted_list_nested, str_list)

    def test_directly_annotated(self) -> None:
        foo = Foo()
        self.assertEqual(Baz, Wrapped[_fbthrift_unadapted_Baz])
        self.assertIsInstance(foo.baz, Wrapped)
        self.assertIsInstance(foo.baz.value, _fbthrift_unadapted_Baz)

    def test_adapted_variable(self) -> None:
        self.assertEqual(NINETEEN_EIGHTY_FOUR, datetime(1984, 1, 1))
        self.assertEqual(_fbthrift_unadapted_NINETEEN_EIGHTY_FOUR, 441792000)

        DatetimeAdapter_from_thrift.assert_called_once_with(
            441792000,
            transitive_annotation=_fbthrift_unadapted_AsDatetime(
                signature="DatetimeConstant"
            ),
            constant_uri="thrift.com/python/test/NINETEEN_EIGHTY_FOUR",
        )

    def test_adapted_field_with_default_value(self) -> None:
        foo = Foo()
        self.assertIs(True, foo.wrapped_bool.value)
        self.assertIs(True, foo.double_wrapped_bool.value.value)

    def test_adapted_field_with_container_value_annotation(self) -> None:
        foo = Foo()
        self.assertIs(0, foo.abc.value)
