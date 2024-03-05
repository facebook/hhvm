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

import unittest

import convertible.thrift_types as python_types
import convertible.ttypes as py_deprecated_types
import convertible.types as py3_types
from thrift.python.converter import to_python_struct
from thrift.python.types import BadEnum


class Py3ToPythonConverterTest(unittest.TestCase):
    def test_simple(self) -> None:
        simple = py3_types.Simple(
            intField=42,
            strField="simple",
            intList=[1, 2, 3],
            strSet={"hello", "world"},
            strToIntMap={"one": 1, "two": 2},
            color=py3_types.Color.GREEN,
            name_="myname",
        )._to_python()
        self.assertEqual(simple.intField, 42)
        self.assertEqual(simple.strField, "simple")
        self.assertEqual(simple.intList, [1, 2, 3])
        self.assertEqual(simple.strSet, {"hello", "world"})
        self.assertEqual(simple.strToIntMap, {"one": 1, "two": 2})
        self.assertEqual(simple.color, python_types.Color.GREEN)
        self.assertEqual(simple.name_, "myname")

    def test_nested(self) -> None:
        nested = py3_types.Nested(
            simpleField=py3_types.Simple(
                intField=42,
                strField="simple",
                intList=[1, 2, 3],
                strSet={"hello", "world"},
                strToIntMap={"one": 1, "two": 2},
                color=py3_types.Color.NONE,
                name_="myname",
            ),
            simpleList=[
                py3_types.Simple(
                    intField=200,
                    strField="face",
                    intList=[4, 5, 6],
                    strSet={"keep", "calm"},
                    strToIntMap={"three": 3, "four": 4},
                    color=py3_types.Color.RED,
                    name_="myname",
                ),
                py3_types.Simple(
                    intField=404,
                    strField="b00k",
                    intList=[7, 8, 9],
                    strSet={"carry", "on"},
                    strToIntMap={"five": 5, "six": 6},
                    color=py3_types.Color.GREEN,
                    name_="myname",
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
                    name_="myname",
                )
            },
        )._to_python()
        self.assertEqual(nested.simpleField.intField, 42)
        self.assertEqual(nested.simpleList[0].intList, [4, 5, 6])
        self.assertEqual(nested.simpleList[1].strSet, {"carry", "on"})
        self.assertEqual(
            nested.colorToSimpleMap[python_types.Color.BLUE].color,
            python_types.Color.BLUE,
        )

    def test_simple_union(self) -> None:
        simple_union = py3_types.Union(intField=42)._to_python()
        self.assertEqual(simple_union.type, python_types.Union.Type.intField)
        self.assertEqual(simple_union.value, 42)

    def test_union_with_py3_name_annotation(self) -> None:
        simple_union = py3_types.Union(name_="myname")._to_python()
        self.assertEqual(simple_union.type, python_types.Union.Type.name_)
        self.assertEqual(simple_union.value, "myname")

    def test_union_with_containers(self) -> None:
        union_with_list = py3_types.Union(intList=[1, 2, 3])._to_python()
        self.assertEqual(union_with_list.type, python_types.Union.Type.intList)
        self.assertEqual(union_with_list.value, [1, 2, 3])

    def test_complex_union(self) -> None:
        complex_union = py3_types.Union(
            simple_=py3_types.Simple(
                intField=42,
                strField="simple",
                intList=[1, 2, 3],
                strSet={"hello", "world"},
                strToIntMap={"one": 1, "two": 2},
                color=py3_types.Color.NONE,
            )
        )._to_python()
        self.assertEqual(complex_union.type, python_types.Union.Type.simple_)
        self.assertEqual(complex_union.simple_.intField, 42)

    def test_struct_with_mismatching_field(self) -> None:
        tomayto = py3_types.Tomayto(
            to=42,
            mayto="blah",
        )
        tomahto = to_python_struct(
            python_types.Tomahto,
            tomayto,
        )
        self.assertEqual(tomahto.to, 42)
        self.assertEqual(tomahto.mahto, "mahto")

    def test_union_with_mismatching_field(self) -> None:
        po = to_python_struct(
            python_types.Potahto,
            py3_types.Potayto(
                po=42,
            ),
        )
        self.assertEqual(po.type, python_types.Potahto.Type.po)
        self.assertEqual(po.value, 42)

        tah = to_python_struct(
            python_types.Potahto,
            py3_types.Potayto(
                tay="tay",
            ),
        )
        self.assertEqual(tah.type, python_types.Potahto.Type.EMPTY)

        to = to_python_struct(
            python_types.Potahto,
            py3_types.Potayto(
                to=True,
            ),
        )
        self.assertEqual(to.type, python_types.Potahto.Type.to)
        self.assertEqual(to.value, True)

    def test_enum(self) -> None:
        self.assertEqual(
            py3_types.Color.RED._to_python(),
            python_types.Color.RED,
        )


class PyDeprecatedToPythonConverterTest(unittest.TestCase):
    def test_simple(self) -> None:
        simple = py_deprecated_types.Simple(
            intField=42,
            strField="simple",
            intList=[1, 2, 3],
            strSet={"hello", "world"},
            strToIntMap={"one": 1, "two": 2},
            color=py_deprecated_types.Color.GREEN,
            name="myname",
        )._to_python()
        self.assertEqual(simple.intField, 42)
        self.assertEqual(simple.strField, "simple")
        self.assertEqual(simple.intList, [1, 2, 3])
        self.assertEqual(simple.strSet, {"hello", "world"})
        self.assertEqual(simple.strToIntMap, {"one": 1, "two": 2})
        self.assertEqual(simple.color, python_types.Color.GREEN)
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
        )._to_python()
        self.assertEqual(nested.simpleField.intField, 42)
        self.assertEqual(nested.simpleList[0].intList, [4, 5, 6])
        self.assertEqual(nested.simpleList[1].strSet, {"carry", "on"})
        self.assertEqual(
            nested.colorToSimpleMap[python_types.Color.BLUE].color,
            python_types.Color.BLUE,
        )

    def test_simple_union(self) -> None:
        simple_union = py_deprecated_types.Union(intField=42)._to_python()
        self.assertEqual(simple_union.type, python_types.Union.Type.intField)
        self.assertEqual(simple_union.value, 42)

    def test_union_with_py3_name_annotation(self) -> None:
        simple_union = py_deprecated_types.Union(name="myname")._to_python()
        self.assertEqual(simple_union.type, python_types.Union.Type.name_)
        self.assertEqual(simple_union.value, "myname")

    def test_union_with_containers(self) -> None:
        union_with_list = py_deprecated_types.Union(intList=[1, 2, 3])._to_python()
        self.assertEqual(union_with_list.type, python_types.Union.Type.intList)
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
        )._to_python()
        self.assertEqual(complex_union.type, python_types.Union.Type.simple_)
        self.assertEqual(complex_union.simple_.intField, 42)

    def test_struct_with_mismatching_field(self) -> None:
        tomayto = py_deprecated_types.Tomayto(
            to=42,
            mayto="blah",
        )
        tomahto = to_python_struct(
            python_types.Tomahto,
            tomayto,
        )
        self.assertEqual(tomahto.to, 42)
        self.assertEqual(tomahto.mahto, "mahto")

    def test_py_bad_enum(self) -> None:
        simple = py_deprecated_types.Simple(
            intField=42,
            strField="simple",
            intList=[1, 2, 3],
            strSet={"hello", "world"},
            strToIntMap={"one": 1, "two": 2},
            color=1234,  # pyre-ignore[6]: In call `py_deprecated_types.Simple.__init__`, for 6th parameter `color` expected `Optional[Color]` but got `int`.
            name="myname",
        )._to_python()
        self.assertIsInstance(simple.color, BadEnum)
        self.assertEqual(
            # pyre-ignore[16]: `python_types.Color` has no attribute `enum`. It's actually a BadEnum.
            simple.color.enum,
            python_types.Color,
        )

        self.assertEqual(int(simple.color), 1234)

    def test_union_with_mismatching_field(self) -> None:
        po = to_python_struct(
            python_types.Potahto,
            py_deprecated_types.Potayto(
                po=42,
            ),
        )
        self.assertEqual(po.type, python_types.Potahto.Type.po)
        self.assertEqual(po.value, 42)

        tah = to_python_struct(
            python_types.Potahto,
            py_deprecated_types.Potayto(
                tay="tay",
            ),
        )
        self.assertEqual(tah.type, python_types.Potahto.Type.EMPTY)

        to = to_python_struct(
            python_types.Potahto,
            py_deprecated_types.Potayto(
                to=True,
            ),
        )
        self.assertEqual(to.type, python_types.Potahto.Type.to)
        self.assertEqual(to.value, True)


class PythonToPythonConverterTest(unittest.TestCase):
    def test_should_return_self(self) -> None:
        simple = python_types.Simple()
        self.assertIs(
            simple,
            to_python_struct(
                python_types.Simple,
                simple,
            ),
        )


class NoneToPythonConverterTest(unittest.TestCase):
    def test_optional_type(self) -> None:
        self.assertIsNone(to_python_struct(python_types.Simple, None))
        self.assertIsNone(
            to_python_struct(python_types.Simple, py3_types.Nested().optionalSimple)
        )

        with self.assertRaises(AttributeError):
            # make sure pyre complains
            # pyre-ignore[16]: Optional type has no attribute `intField`.
            to_python_struct(
                python_types.Simple, py3_types.Nested().optionalSimple
            ).intField
