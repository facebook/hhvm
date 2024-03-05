#!/usr/bin/env python3
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

import convertible.thrift_types as python_types
import convertible.ttypes as py_deprecated_types
import convertible.types as py3_types
from thrift.util.converter import to_py_struct


class Py3ToPyDeprecatedConverterTest(unittest.TestCase):
    def test_simple(self) -> None:
        simple = py3_types.Simple(
            intField=42,
            strField="simple",
            intList=[1, 2, 3],
            strSet={"hello", "world"},
            strToIntMap={"one": 1, "two": 2},
            color=py3_types.Color.GREEN,
            name_="renamed",
        )._to_py_deprecated()
        self.assertEqual(simple.intField, 42)
        self.assertEqual(simple.strField, "simple")
        self.assertEqual(simple.intList, [1, 2, 3])
        self.assertEqual(simple.strSet, {"hello", "world"})
        self.assertEqual(simple.strToIntMap, {"one": 1, "two": 2})
        self.assertEqual(simple.color, py_deprecated_types.Color.GREEN)
        self.assertEqual(simple.name, "renamed")

    def test_nested(self) -> None:
        nested = py3_types.Nested(
            simpleField=py3_types.Simple(
                intField=42,
                strField="simple",
                intList=[1, 2, 3],
                strSet={"hello", "world"},
                strToIntMap={"one": 1, "two": 2},
                color=py3_types.Color.NONE,
            ),
            simpleList=[
                py3_types.Simple(
                    intField=200,
                    strField="face",
                    intList=[4, 5, 6],
                    strSet={"keep", "calm"},
                    strToIntMap={"three": 3, "four": 4},
                    color=py3_types.Color.RED,
                ),
                py3_types.Simple(
                    intField=404,
                    strField="b00k",
                    intList=[7, 8, 9],
                    strSet={"carry", "on"},
                    strToIntMap={"five": 5, "six": 6},
                    color=py3_types.Color.GREEN,
                ),
            ],
            colorToSimpleMap={
                py3_types.Color.BLUE: py3_types.Simple(
                    intField=500,
                    strField="internal",
                    intList=[10],
                    strSet={"server", "error"},
                    strToIntMap={"seven": 7, "eight": 8, "nine": 9},
                    color=py3_types.Color.BLUE,
                )
            },
        )._to_py_deprecated()
        self.assertEqual(nested.simpleField.intField, 42)
        self.assertEqual(nested.simpleList[0].intList, [4, 5, 6])
        self.assertEqual(nested.simpleList[1].strSet, {"carry", "on"})
        self.assertEqual(
            nested.colorToSimpleMap[py_deprecated_types.Color.BLUE].color,
            py_deprecated_types.Color.BLUE,
        )

    def test_simple_union(self) -> None:
        simple_union = py3_types.Union(intField=42)._to_py_deprecated()
        self.assertEqual(simple_union.get_intField(), 42)

    def test_union_with_containers(self) -> None:
        union_with_list = py3_types.Union(intList=[1, 2, 3])._to_py_deprecated()
        self.assertEqual(union_with_list.get_intList(), [1, 2, 3])

    def test_complex_union(self) -> None:
        complex_union = py3_types.Union(
            simple_=py3_types.Simple(
                intField=42,
                strField="simple",
                intList=[1, 2, 3],
                strSet={"hello", "world"},
                strToIntMap={"one": 1, "two": 2},
                color=py3_types.Color.NONE,
                name_="renamed",
            )
        )._to_py_deprecated()
        self.assertEqual(complex_union.get_simpleField().intField, 42)
        self.assertEqual(complex_union.get_simpleField().name, "renamed")

    def test_struct_with_mismatching_field(self) -> None:
        tomayto = py3_types.Tomayto(
            to=42,
            mayto="blah",
        )
        tomahto = to_py_struct(
            py_deprecated_types.Tomahto,
            tomayto,
        )
        self.assertEqual(tomahto.to, 42)
        self.assertIsNone(tomahto.mahto)

    def test_union_with_mismatching_field(self) -> None:
        po = to_py_struct(
            py_deprecated_types.Potahto,
            py3_types.Potayto(
                po=42,
            ),
        )
        self.assertEqual(po.getType(), py_deprecated_types.Potahto.PO)
        self.assertEqual(po.get_po(), 42)

        tah = to_py_struct(
            py_deprecated_types.Potahto,
            py3_types.Potayto(
                tay="tay",
            ),
        )
        self.assertEqual(tah.getType(), py_deprecated_types.Potahto.__EMPTY__)

        to = to_py_struct(
            py_deprecated_types.Potahto,
            py3_types.Potayto(
                to=True,
            ),
        )
        self.assertEqual(to.getType(), py_deprecated_types.Potahto.TO)
        self.assertEqual(to.get_to(), True)

    def test_enum(self) -> None:
        self.assertEqual(
            py3_types.Color.RED._to_py_deprecated(),
            py_deprecated_types.Color.RED,
        )


class PythonToPyDeprecatedConverterTest(unittest.TestCase):
    def test_simple(self) -> None:
        simple = python_types.Simple(
            intField=42,
            strField="simple",
            intList=[1, 2, 3],
            strSet={"hello", "world"},
            strToIntMap={"one": 1, "two": 2},
            color=python_types.Color.GREEN,
            name_="renamed",
        )._to_py_deprecated()
        self.assertEqual(simple.intField, 42)
        self.assertEqual(simple.strField, "simple")
        self.assertEqual(simple.intList, [1, 2, 3])
        self.assertEqual(simple.strSet, {"hello", "world"})
        self.assertEqual(simple.strToIntMap, {"one": 1, "two": 2})
        self.assertEqual(simple.color, py_deprecated_types.Color.GREEN)
        self.assertEqual(simple.name, "renamed")

    def test_nested(self) -> None:
        nested = python_types.Nested(
            simpleField=python_types.Simple(
                intField=42,
                strField="simple",
                intList=[1, 2, 3],
                strSet={"hello", "world"},
                strToIntMap={"one": 1, "two": 2},
                color=python_types.Color.NONE,
            ),
            simpleList=[
                python_types.Simple(
                    intField=200,
                    strField="face",
                    intList=[4, 5, 6],
                    strSet={"keep", "calm"},
                    strToIntMap={"three": 3, "four": 4},
                    color=python_types.Color.RED,
                ),
                python_types.Simple(
                    intField=404,
                    strField="b00k",
                    intList=[7, 8, 9],
                    strSet={"carry", "on"},
                    strToIntMap={"five": 5, "six": 6},
                    color=python_types.Color.GREEN,
                ),
            ],
            colorToSimpleMap={
                python_types.Color.BLUE: python_types.Simple(
                    intField=500,
                    strField="internal",
                    intList=[10],
                    strSet={"server", "error"},
                    strToIntMap={"seven": 7, "eight": 8, "nine": 9},
                    color=python_types.Color.BLUE,
                )
            },
        )._to_py_deprecated()
        self.assertEqual(nested.simpleField.intField, 42)
        self.assertEqual(nested.simpleList[0].intList, [4, 5, 6])
        self.assertEqual(nested.simpleList[1].strSet, {"carry", "on"})
        self.assertEqual(
            nested.colorToSimpleMap[py_deprecated_types.Color.BLUE].color,
            py_deprecated_types.Color.BLUE,
        )

    def test_simple_union(self) -> None:
        simple_union = python_types.Union(intField=42)._to_py_deprecated()
        self.assertEqual(simple_union.get_intField(), 42)

    def test_union_with_containers(self) -> None:
        union_with_list = python_types.Union(intList=[1, 2, 3])._to_py_deprecated()
        self.assertEqual(union_with_list.get_intList(), [1, 2, 3])

    def test_complex_union(self) -> None:
        complex_union = python_types.Union(
            simple_=python_types.Simple(
                intField=42,
                strField="simple",
                intList=[1, 2, 3],
                strSet={"hello", "world"},
                strToIntMap={"one": 1, "two": 2},
                color=python_types.Color.NONE,
                name_="renamed",
            )
        )._to_py_deprecated()
        self.assertEqual(complex_union.get_simpleField().intField, 42)
        self.assertEqual(complex_union.get_simpleField().name, "renamed")

    def test_struct_with_mismatching_field(self) -> None:
        tomayto = python_types.Tomayto(
            to=42,
            mayto="blah",
        )
        tomahto = to_py_struct(
            py_deprecated_types.Tomahto,
            tomayto,
        )
        self.assertEqual(tomahto.to, 42)
        self.assertIsNone(tomahto.mahto)

    def test_union_with_mismatching_field(self) -> None:
        po = to_py_struct(
            py_deprecated_types.Potahto,
            python_types.Potayto(
                po=42,
            ),
        )
        self.assertEqual(po.getType(), py_deprecated_types.Potahto.PO)
        self.assertEqual(po.get_po(), 42)

        tah = to_py_struct(
            py_deprecated_types.Potahto,
            python_types.Potayto(
                tay="tay",
            ),
        )
        self.assertEqual(tah.getType(), py_deprecated_types.Potahto.__EMPTY__)

        to = to_py_struct(
            py_deprecated_types.Potahto,
            python_types.Potayto(
                to=True,
            ),
        )
        self.assertEqual(to.getType(), py_deprecated_types.Potahto.TO)
        self.assertEqual(to.get_to(), True)

    def test_enum(self) -> None:
        self.assertEqual(
            python_types.Color.RED._to_py_deprecated(),
            py_deprecated_types.Color.RED,
        )


class NoneToPyDeprecatedConverterTest(unittest.TestCase):
    def test_none(self) -> None:
        self.assertIsNone(to_py_struct(py_deprecated_types.Simple, None))

    def test_optional_type(self) -> None:
        self.assertIsNone(
            to_py_struct(
                py_deprecated_types.Simple, python_types.Nested().optionalSimple
            )
        )

        with self.assertRaises(AttributeError):
            # make sure pyre complains
            # pyre-ignore[16]: Optional type has no attribute `intField`.
            to_py_struct(
                py_deprecated_types.Simple, python_types.Nested().optionalSimple
            ).intField


class PyDeprecatedToPyDeprecatedConverterTest(unittest.TestCase):
    def test_should_return_self(self) -> None:
        simple = py_deprecated_types.Simple()
        self.assertIs(
            simple,
            to_py_struct(
                py_deprecated_types.Simple,
                simple,
            ),
        )
