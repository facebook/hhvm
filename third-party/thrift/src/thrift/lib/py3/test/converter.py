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
from thrift.py3.converter import to_py3_struct
from thrift.py3.types import BadEnum, Struct


class PyDeprecatedToPy3ConverterTest(unittest.TestCase):
    def test_simple(self) -> None:
        simple = py_deprecated_types.Simple(
            intField=42,
            strField="simple",
            intList=[1, 2, 3],
            strSet={"hello", "world"},
            strToIntMap={"one": 1, "two": 2},
            color=py_deprecated_types.Color.GREEN,
            name="myname",
        )._to_py3()
        self.assertEqual(simple.intField, 42)
        self.assertEqual(simple.strField, "simple")
        self.assertEqual(simple.intList, [1, 2, 3])
        self.assertEqual(simple.strSet, {"hello", "world"})
        self.assertEqual(simple.strToIntMap, {"one": 1, "two": 2})
        self.assertEqual(simple.color, py3_types.Color.GREEN)
        self.assertEqual(simple.name_, "myname")

    def test_nested(self) -> None:
        nested = py_deprecated_types.Nested(
            simpleField=py_deprecated_types.Simple(
                intField=42,
                strField="simple",
                intList=[1, 2, 3],
                strSet={"hello", "world"},
                strToIntMap={"one": 1, "two": 2},
                color=py_deprecated_types.Color.NONE,
                name="myname",
            ),
            simpleList=[
                py_deprecated_types.Simple(
                    intField=200,
                    strField="face",
                    intList=[4, 5, 6],
                    strSet={"keep", "calm"},
                    strToIntMap={"three": 3, "four": 4},
                    color=py_deprecated_types.Color.RED,
                    name="myname",
                ),
                py_deprecated_types.Simple(
                    intField=404,
                    strField="b00k",
                    intList=[7, 8, 9],
                    strSet={"carry", "on"},
                    strToIntMap={"five": 5, "six": 6},
                    color=py_deprecated_types.Color.GREEN,
                    name="myname",
                ),
            ],
            colorToSimpleMap={
                py_deprecated_types.Color.BLUE: py_deprecated_types.Simple(
                    intField=500,
                    strField="internal",
                    intList=[10],
                    strSet={"server", "error"},
                    strToIntMap={"seven": 7, "eight": 8, "nine": 9},
                    color=py_deprecated_types.Color.BLUE,
                    name="myname",
                )
            },
        )._to_py3()
        self.assertEqual(nested.simpleField.intField, 42)
        self.assertEqual(nested.simpleList[0].intList, [4, 5, 6])
        self.assertEqual(nested.simpleList[1].strSet, {"carry", "on"})
        self.assertEqual(
            nested.colorToSimpleMap[py3_types.Color.BLUE].color, py3_types.Color.BLUE
        )

    def test_simple_union(self) -> None:
        simple_union = py_deprecated_types.Union(intField=42)._to_py3()
        self.assertEqual(simple_union.type, py3_types.Union.Type.intField)
        self.assertEqual(simple_union.value, 42)

    def test_union_with_py3_name_annotation(self) -> None:
        simple_union = py_deprecated_types.Union(name="myname")._to_py3()
        self.assertEqual(simple_union.type, py3_types.Union.Type.name_)
        self.assertEqual(simple_union.value, "myname")

    def test_union_with_containers(self) -> None:
        union_with_list = py_deprecated_types.Union(intList=[1, 2, 3])._to_py3()
        self.assertEqual(union_with_list.type, py3_types.Union.Type.intList)
        self.assertEqual(union_with_list.value, [1, 2, 3])

    def test_complex_union(self) -> None:
        complex_union = py_deprecated_types.Union(
            simpleField=py_deprecated_types.Simple(
                intField=42,
                strField="simple",
                intList=[1, 2, 3],
                strSet={"hello", "world"},
                strToIntMap={"one": 1, "two": 2},
                color=py_deprecated_types.Color.NONE,
            )
        )._to_py3()
        self.assertEqual(complex_union.type, py3_types.Union.Type.simple_)
        self.assertEqual(complex_union.simple_.intField, 42)

    def test_optional_defaults(self) -> None:
        converted = py_deprecated_types.OptionalDefaultsStruct()._to_py3()
        # pyre-fixme[6]: Expected `HasIsSet[Variable[thrift.py3.py3_types._T]]` for 1st
        #  param but got `OptionalDefaultsStruct`.
        self.assertFalse(Struct.isset(converted).sillyString)
        # pyre-fixme[6]: Expected `HasIsSet[Variable[thrift.py3.py3_types._T]]` for 1st
        #  param but got `OptionalDefaultsStruct`.
        self.assertFalse(Struct.isset(converted).sillyColor)

    def test_struct_with_mismatching_field(self) -> None:
        tomayto = py_deprecated_types.Tomayto(
            to=42,
            mayto="blah",
        )
        tomahto = to_py3_struct(
            py3_types.Tomahto,
            tomayto,
        )
        self.assertEqual(tomahto.to, 42)
        self.assertEqual(tomahto.mahto, "mahto")

    def test_union_with_mismatching_field(self) -> None:
        po = to_py3_struct(
            py3_types.Potahto,
            py_deprecated_types.Potayto(
                po=42,
            ),
        )
        self.assertEqual(po.type, py3_types.Potahto.Type.po)
        self.assertEqual(po.value, 42)

        tah = to_py3_struct(
            py3_types.Potahto,
            py_deprecated_types.Potayto(
                tay="tay",
            ),
        )
        self.assertEqual(tah.type, py3_types.Potahto.Type.EMPTY)

        to = to_py3_struct(
            py3_types.Potahto,
            py_deprecated_types.Potayto(
                to=True,
            ),
        )
        self.assertEqual(to.type, py3_types.Potahto.Type.to)
        self.assertEqual(to.value, True)

    def test_py_bad_enum(self) -> None:
        simple = py_deprecated_types.Simple(
            intField=42,
            strField="simple",
            intList=[1, 2, 3],
            strSet={"hello", "world"},
            strToIntMap={"one": 1, "two": 2},
            color=1234,  # pyre-ignore[6]: In call `py_deprecated_types.Simple.__init__`, for 6th parameter `color` expected `Optional[Color]` but got `int`.
            name="myname",
        )._to_py3()
        self.assertIsInstance(simple.color, BadEnum)
        self.assertEqual(
            simple.color.enum,
            py3_types.Color,
        )

        self.assertEqual(int(simple.color), 1234)


class PythonToPy3ConverterTest(unittest.TestCase):
    def make_simple_python(self) -> python_types.Simple:
        return python_types.Simple(
            intField=42,
            strField="simple",
            intList=[1, 2, 3],
            strSet={"hello", "world"},
            strToIntMap={"one": 1, "two": 2},
            color=python_types.Color.GREEN,
            name_="myname",
        )

    def assert_simple(self, simple: py3_types.Simple) -> None:
        self.assertEqual(simple.intField, 42)
        self.assertEqual(simple.strField, "simple")
        self.assertEqual(simple.intList, [1, 2, 3])
        self.assertEqual(simple.strSet, {"hello", "world"})
        self.assertEqual(simple.strToIntMap, {"one": 1, "two": 2})
        self.assertEqual(simple.color, py3_types.Color.GREEN)
        self.assertEqual(simple.name_, "myname")
        self.assertIsInstance(simple.empty, BadEnum)
        self.assertEqual(int(simple.empty), 0)

    def test_simple(self) -> None:
        self.assert_simple(self.make_simple_python()._to_py3())

    def test_simple_capi(self) -> None:
        self.assert_simple(py3_types.Simple.from_python(self.make_simple_python()))

    def make_nested_python(self) -> python_types.Nested:
        return python_types.Nested(
            simpleField=self.make_simple_python(),
            simpleList=[
                python_types.Simple(
                    intField=200,
                    strField="face",
                    intList=[4, 5, 6],
                    strSet={"keep", "calm"},
                    strToIntMap={"three": 3, "four": 4},
                    color=python_types.Color.RED,
                    name_="myname",
                ),
                python_types.Simple(
                    intField=404,
                    strField="b00k",
                    intList=[7, 8, 9],
                    strSet={"carry", "on"},
                    strToIntMap={"five": 5, "six": 6},
                    color=python_types.Color.GREEN,
                    name_="myname",
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
                    name_="myname",
                )
            },
        )

    def test_nested(self) -> None:
        nested = self.make_nested_python()._to_py3()
        self.assertEqual(nested.simpleField.intField, 42)
        self.assertEqual(nested.simpleList[0].intList, [4, 5, 6])
        self.assertEqual(nested.simpleList[1].strSet, {"carry", "on"})
        self.assertEqual(
            nested.colorToSimpleMap[py3_types.Color.BLUE].color, py3_types.Color.BLUE
        )

    def test_nested_capi(self) -> None:
        self.assertEqual(
            self.make_nested_python()._to_py3(),
            py3_types.Nested.from_python(self.make_nested_python()),
        )

    def test_simple_union(self) -> None:
        simple_union = python_types.Union(intField=42)._to_py3()
        self.assertEqual(simple_union.type, py3_types.Union.Type.intField)
        self.assertEqual(simple_union.value, 42)

    def test_simple_union_capi(self) -> None:
        simple_union = py3_types.Union.from_python(python_types.Union(intField=42))
        self.assertEqual(simple_union.type, py3_types.Union.Type.intField)
        self.assertEqual(simple_union.value, 42)

    def test_union_with_py3_name_annotation(self) -> None:
        simple_union = python_types.Union(name_="myname")._to_py3()
        self.assertEqual(simple_union.type, py3_types.Union.Type.name_)
        self.assertEqual(simple_union.value, "myname")

    def test_union_with_py3_name_annotation_capi(self) -> None:
        simple_union = py3_types.Union.from_python(python_types.Union(name_="myname"))
        self.assertEqual(simple_union.type, py3_types.Union.Type.name_)
        self.assertEqual(simple_union.value, "myname")

    def test_union_with_containers(self) -> None:
        union_with_list = python_types.Union(intList=[1, 2, 3])._to_py3()
        self.assertEqual(union_with_list.type, py3_types.Union.Type.intList)
        self.assertEqual(union_with_list.value, [1, 2, 3])

    def test_union_with_containers_capi(self) -> None:
        union_with_list = py3_types.Union.from_python(
            python_types.Union(intList=[1, 2, 3])
        )
        self.assertEqual(union_with_list.type, py3_types.Union.Type.intList)
        self.assertEqual(union_with_list.value, [1, 2, 3])

    def test_complex_union(self) -> None:
        complex_union = python_types.Union(
            simple_=self.make_simple_python(),
        )._to_py3()
        self.assertEqual(complex_union.type, py3_types.Union.Type.simple_)
        self.assertEqual(complex_union.simple_.intField, 42)

    def test_complex_union_capi(self) -> None:
        complex_union = py3_types.Union.from_python(
            python_types.Union(
                simple_=self.make_simple_python(),
            )
        )
        self.assertEqual(complex_union.type, py3_types.Union.Type.simple_)
        self.assertEqual(complex_union.simple_.intField, 42)

    def test_optional_defaults(self) -> None:
        converted = python_types.OptionalDefaultsStruct()._to_py3()
        # pyre-fixme[6]: Expected `HasIsSet[Variable[thrift.py3.py3_types._T]]` for 1st
        #  param but got `OptionalDefaultsStruct`.
        self.assertFalse(Struct.isset(converted).sillyString)
        # pyre-fixme[6]: Expected `HasIsSet[Variable[thrift.py3.py3_types._T]]` for 1st
        #  param but got `OptionalDefaultsStruct`.
        self.assertFalse(Struct.isset(converted).sillyColor)

    def test_struct_with_mismatching_field(self) -> None:
        tomayto = python_types.Tomayto(
            to=42,
            mayto="blah",
        )
        tomahto = to_py3_struct(
            py3_types.Tomahto,
            tomayto,
        )
        self.assertEqual(tomahto.to, 42)
        self.assertEqual(tomahto.mahto, "mahto")

    def test_union_with_mismatching_field(self) -> None:
        po = to_py3_struct(
            py3_types.Potahto,
            python_types.Potayto(
                po=42,
            ),
        )
        self.assertEqual(po.type, py3_types.Potahto.Type.po)
        self.assertEqual(po.value, 42)

        tah = to_py3_struct(
            py3_types.Potahto,
            python_types.Potayto(
                tay="tay",
            ),
        )
        self.assertEqual(tah.type, py3_types.Potahto.Type.EMPTY)

        to = to_py3_struct(
            py3_types.Potahto,
            python_types.Potayto(
                to=True,
            ),
        )
        self.assertEqual(to.type, py3_types.Potahto.Type.to)
        self.assertEqual(to.value, True)

    def test_enum(self) -> None:
        self.assertEqual(
            python_types.Color.RED._to_py3(),
            py3_types.Color.RED,
        )


class NoneToPy3ConverterTest(unittest.TestCase):
    def test_optional_type(self) -> None:
        self.assertIsNone(to_py3_struct(py3_types.Simple, None))
        self.assertIsNone(
            to_py3_struct(py3_types.Simple, python_types.Nested().optionalSimple)
        )

        with self.assertRaises(AttributeError):
            # make sure pyre complains
            # pyre-ignore[16]: Optional type has no attribute `intField`.
            to_py3_struct(
                py3_types.Simple, python_types.Nested().optionalSimple
            ).intField


class Py3ToPy3ConverterTest(unittest.TestCase):
    def test_should_return_self(self) -> None:
        simple = py3_types.Simple()
        self.assertIs(
            simple,
            to_py3_struct(
                py3_types.Simple,
                simple,
            ),
        )
