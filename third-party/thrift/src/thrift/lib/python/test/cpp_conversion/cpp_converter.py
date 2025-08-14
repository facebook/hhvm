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

import thrift.python.test.cpp_converter_helper as converter

from convertible.thrift_types import Color, Nested, Simple, Union
from thrift.python.test.annotations.thrift_types import RenamedEmpty


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
        self.assertEqual(self.make_simple(), converter.echo_simple(self.make_simple()))

    def test_echo_nested(self) -> None:
        self.assertEqual(self.make_nested(), converter.echo_nested(self.make_nested()))

    def test_echo_union(self) -> None:
        for ufac in [
            lambda: Union(intField=42),
            lambda: Union(strField="hello"),
            lambda: Union(simple_=self.make_simple()),
            lambda: Union(intList=[1, 1, 2, 3, 5, 8]),
        ]:
            self.assertEqual(ufac(), converter.echo_union(ufac()))

    def test_constructor_error(self) -> None:
        bad_unicode = b"\xc3\x28"
        s = converter.make_simple_corrupted(self.make_simple(), bad_unicode)
        # this shows that the unicode passes through extractor without raising error
        rt = converter.echo_simple(s)
        # it is not possible to create a thrift object in either thrift-python
        # or C++ that fails conversion due to a bad primitive field.
        # It is possible to set a non-primitive field in C++ that is invalid
        # in thrift-python, but there is no error until field is first accessed
        with self.assertRaises(UnicodeDecodeError):
            s.strField
        with self.assertRaises(UnicodeDecodeError):
            rt.strField

        with self.assertRaises(UnicodeDecodeError):
            s.strList
        with self.assertRaises(UnicodeDecodeError):
            rt.strList

        # Currently the conversion from internal data to python value happens
        # for all key-value pairs in a map at access time.
        # If we make this lazy, this step will pass, and the UnicodeDecodeError
        # will be deferred until the corrupted fields are accessed, while the
        # valid unicode fields will become accessible.
        with self.assertRaises(UnicodeDecodeError):
            s.strToStrMap
        with self.assertRaises(UnicodeDecodeError):
            rt.strToStrMap

    def test_type_error(self) -> None:
        with self.assertRaises(TypeError):
            converter.echo_simple("hello")  # type: ignore

        with self.assertRaises(TypeError):
            converter.echo_simple(self.make_nested())  # type: ignore

    def test_RenamedEmpty(self) -> None:
        # pyre-fixme[16]: Module `cpp_converter_helper` has no attribute
        #  `echo_RenamedEmpty`.
        self.assertEqual(RenamedEmpty(), converter.echo_RenamedEmpty(RenamedEmpty()))
