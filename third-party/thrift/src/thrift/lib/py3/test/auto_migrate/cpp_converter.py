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


import unittest

import convertible.thrift_types as python_convertible
import testing.thrift_types as python_testing
import thrift.py3.test.cpp_converter_helper as converter
from convertible.types import Color, Nested, Simple, Union
from testing.types import NonCopyable
from thrift.lib.py3.test.auto_migrate.auto_migrate_util import is_auto_migrated


class CppConverterEcho(unittest.TestCase):
    def make_simple(self) -> Simple:
        return Simple(
            intField=42,
            strField="simple",
            intList=[1, 2, 3],
            strSet={"hello", "world"},
            strToIntMap={"one": 1, "two": 2},
            color=Color.GREEN,
            name_="myname",
        )

    def make_nested(self) -> Nested:
        return Nested(
            simpleField=self.make_simple(),
            simpleList=[
                Simple(
                    intField=200,
                    strField="face",
                    intList=[4, 5, 6],
                    strSet={"keep", "calm"},
                    strToIntMap={"three": 3, "four": 4},
                    color=Color.RED,
                    name_="myname",
                ),
                Simple(
                    intField=404,
                    strField="b00k",
                    intList=[7, 8, 9],
                    strSet={"carry", "on"},
                    strToIntMap={"five": 5, "six": 6},
                    color=Color.GREEN,
                    name_="myname",
                ),
            ],
            colorToSimpleMap={
                Color.BLUE: Simple(
                    intField=500,
                    strField="internal",
                    intList=[10],
                    strSet={"server", "error"},
                    strToIntMap={"seven": 7, "eight": 8, "nine": 9},
                    color=Color.BLUE,
                    name_="myname",
                )
            },
        )

    def test_echo_simple(self) -> None:
        echo = converter.echo_simple(self.make_simple())
        self.assertEqual(self.make_simple(), echo)
        expected_type = python_convertible.Simple if is_auto_migrated() else Simple
        self.assertIsInstance(echo, expected_type)

    def test_echo_nested(self) -> None:
        echo = converter.echo_nested(self.make_nested())
        self.assertEqual(self.make_nested(), echo)
        expected_type = python_convertible.Nested if is_auto_migrated() else Nested
        self.assertIsInstance(echo, expected_type)

    def test_noncopyable(self) -> None:
        n = NonCopyable(num=64)
        echo = converter.echo_noncopyable(n)
        self.assertEqual(n, echo)
        self.assertIsNot(n, echo)
        expected_type = (
            python_testing.NonCopyable if is_auto_migrated() else NonCopyable
        )
        self.assertIsInstance(echo, expected_type)

    def test_echo_union(self) -> None:
        for u_factory in [
            lambda: Union(intField=42),
            lambda: Union(strField="hello"),
            lambda: Union(simple_=self.make_simple()),
            lambda: Union(intList=[1, 1, 2, 3, 5, 8]),
        ]:
            echo = converter.echo_union(u_factory())
            self.assertEqual(u_factory(), echo, str(u_factory()))
            expected_type = python_convertible.Union if is_auto_migrated() else Union
            self.assertIsInstance(echo, expected_type)

    def test_constructor_error(self) -> None:
        bad_unicode = b"\xc3\x28"
        s = converter.echo_simple_corrupted(self.make_simple(), bad_unicode)
        # it is not possible to create a thrift object in either thrift-python
        # or C++ that fails conversion due to a bad primitive field.
        # It is possible to set a non-primitive field in C++ that is invalid
        # in thrift-python, but there is no error until field is first accessed
        with self.assertRaises(UnicodeDecodeError):
            s.strField

    def test_type_error(self) -> None:
        with self.assertRaises(TypeError):
            converter.echo_simple("hello")  # type: ignore

        with self.assertRaises(TypeError):
            converter.echo_simple(self.make_nested())  # type: ignore

    def test_cpp_mutation(self) -> None:
        s = self.make_simple()
        initial_strField = s.strField
        cpp_str = converter.try_mutate_simple(s)
        self.assertEqual(cpp_str, "mutated")
        self.assertEqual(s.strField, initial_strField)
