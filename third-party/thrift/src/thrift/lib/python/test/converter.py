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

import convertible.thrift_mutable_types as python_mutable_types
import convertible.thrift_types as python_types
import convertible.ttypes as py_deprecated_types
import convertible.types as py3_types
import testing.py_deprecated_asyncio_fallback_test.thrift_types
import testing.py_deprecated_asyncio_fallback_test.ttypes
import testing.py_deprecated_asyncio_test.thrift_types
import testing.py_deprecated_asyncio_test.ttypes
import testing.thrift_types as testing_types
from thrift.python.converter import to_python_struct
from thrift.python.mutable_converter import to_mutable_python_struct_or_union
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

    def test_exception(self) -> None:
        exception = py3_types.SimpleException(
            message="Test error", code=42
        )._to_python()
        self.assertIsInstance(exception, python_types.SimpleException)
        self.assertEqual(exception.message, "Test error")
        self.assertEqual(exception.code, 42)


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

    def test_exception(self) -> None:
        exception = py_deprecated_types.SimpleException(
            message="Test error", code=42
        )._to_python()
        self.assertIsInstance(exception, python_types.SimpleException)
        self.assertEqual(exception.message, "Test error")
        self.assertEqual(exception.code, 42)


class Py3ToMutablePythonConverterTest(unittest.TestCase):
    @unittest.expectedFailure
    def test_exception(self) -> None:
        # pyre-ignore[16]: _to_mutable_python() does not exist on py3 exceptions
        exception = py3_types.SimpleException(
            message="Test error", code=42
        )._to_mutable_python()
        self.assertIsInstance(exception, python_mutable_types.SimpleException)
        self.assertEqual(exception.message, "Test error")
        self.assertEqual(exception.code, 42)


class PyDeprecatedToMutablePythonConverterTest(unittest.TestCase):
    def test_simple(self) -> None:
        simple = py_deprecated_types.Simple(
            intField=42,
            strField="simple",
            intList=[1, 2, 3],
            strSet={"hello", "world"},
            strToIntMap={"one": 1, "two": 2},
            color=py_deprecated_types.Color.GREEN,
            name="myname",
        )._to_mutable_python()
        self.assertEqual(simple.intField, 42)
        self.assertEqual(simple.strField, "simple")
        self.assertEqual(simple.intList, [1, 2, 3])
        self.assertEqual(simple.strSet, {"hello", "world"})
        self.assertEqual(simple.strToIntMap, {"one": 1, "two": 2})
        self.assertEqual(simple.color, python_mutable_types.Color.GREEN)
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
        )._to_mutable_python()
        self.assertEqual(nested.simpleField.intField, 42)
        self.assertEqual(nested.simpleList[0].intList, [4, 5, 6])
        self.assertEqual(nested.simpleList[1].strSet, {"carry", "on"})
        self.assertEqual(
            nested.colorToSimpleMap[python_mutable_types.Color.BLUE].color,
            python_mutable_types.Color.BLUE,
        )

    def test_simple_union(self) -> None:
        simple_union = py_deprecated_types.Union(intField=42)._to_mutable_python()
        self.assertEqual(
            simple_union.fbthrift_current_field,
            python_mutable_types.Union.FbThriftUnionFieldEnum.intField,
        )
        self.assertEqual(simple_union.fbthrift_current_value, 42)

    def test_union_with_py3_name_annotation(self) -> None:
        simple_union = py_deprecated_types.Union(name="myname")._to_mutable_python()
        self.assertEqual(
            simple_union.fbthrift_current_field,
            python_mutable_types.Union.FbThriftUnionFieldEnum.name_,
        )
        self.assertEqual(simple_union.fbthrift_current_value, "myname")

    def test_union_with_containers(self) -> None:
        union_with_list = py_deprecated_types.Union(
            intList=[1, 2, 3]
        )._to_mutable_python()
        self.assertEqual(
            union_with_list.fbthrift_current_field,
            python_mutable_types.Union.FbThriftUnionFieldEnum.intList,
        )
        self.assertEqual(union_with_list.fbthrift_current_value, [1, 2, 3])

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
        )._to_mutable_python()
        self.assertEqual(
            complex_union.fbthrift_current_field,
            python_mutable_types.Union.FbThriftUnionFieldEnum.simple_,
        )
        self.assertEqual(complex_union.simple_.intField, 42)

    def test_struct_with_mismatching_field(self) -> None:
        tomayto = py_deprecated_types.Tomayto(
            to=42,
            mayto="blah",
        )
        tomahto = to_mutable_python_struct_or_union(
            python_mutable_types.Tomahto,
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
        )._to_mutable_python()
        self.assertIsInstance(simple.color, BadEnum)
        self.assertEqual(
            # pyre-ignore[16]: `python_mutable_types.Color` has no attribute `enum`. It's actually a BadEnum.
            simple.color.enum,
            python_mutable_types.Color,
        )

        self.assertEqual(int(simple.color), 1234)

    def test_union_with_mismatching_field(self) -> None:
        po = to_mutable_python_struct_or_union(
            python_mutable_types.Potahto,
            py_deprecated_types.Potayto(
                po=42,
            ),
        )
        self.assertEqual(
            po.fbthrift_current_field,
            python_mutable_types.Potahto.FbThriftUnionFieldEnum.po,
        )
        self.assertEqual(po.fbthrift_current_value, 42)

        tah = to_mutable_python_struct_or_union(
            python_mutable_types.Potahto,
            py_deprecated_types.Potayto(
                tay="tay",
            ),
        )
        self.assertEqual(
            tah.fbthrift_current_field,
            python_mutable_types.Potahto.FbThriftUnionFieldEnum.EMPTY,
        )

        to = to_mutable_python_struct_or_union(
            python_mutable_types.Potahto,
            py_deprecated_types.Potayto(
                to=True,
            ),
        )
        self.assertEqual(
            to.fbthrift_current_field,
            python_mutable_types.Potahto.FbThriftUnionFieldEnum.to,
        )
        self.assertEqual(to.fbthrift_current_value, True)

    @unittest.expectedFailure
    def test_exception(self) -> None:
        exception = py_deprecated_types.SimpleException(
            message="Test error", code=42
        )._to_mutable_python()
        self.assertIsInstance(exception, python_mutable_types.SimpleException)
        self.assertEqual(exception.message, "Test error")
        self.assertEqual(exception.code, 42)


class PythonToMutablePythonConverterTest(unittest.TestCase):
    @unittest.expectedFailure
    def test_exception(self) -> None:
        exception = python_types.SimpleException(
            message="Test error", code=42
        )._to_mutable_python()
        self.assertIsInstance(exception, python_mutable_types.SimpleException)
        self.assertEqual(exception.message, "Test error")
        self.assertEqual(exception.code, 42)


class MutablePythonToMutablePythonConverterTest(unittest.TestCase):
    def test_exception(self) -> None:
        exception = python_mutable_types.SimpleException(message="Test error", code=42)
        self.assertIs(exception, exception._to_mutable_python())


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

    def test_exception(self) -> None:
        exception = python_types.SimpleException(message="Test error", code=42)
        self.assertIs(exception, exception._to_python())


class MutablePythonToPythonConverterTest(unittest.TestCase):
    def test_exception(self) -> None:
        exception = python_mutable_types.SimpleException(
            message="Test error", code=42
        )._to_python()
        self.assertIsInstance(exception, python_types.SimpleException)
        self.assertEqual(exception.message, "Test error")
        self.assertEqual(exception.code, 42)


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


class ToPyDeprecatedConverterTest(unittest.TestCase):
    def test_enum(self) -> None:
        def takes_enum(py_deprecated_color: py_deprecated_types.Color) -> None:
            self.assertEqual(py_deprecated_color, py_deprecated_types.Color.RED)

        python_enum_value = python_types.Color.RED
        takes_enum(python_enum_value._to_py_deprecated())

    def test_converter_to_py_deprecated(self) -> None:
        self.assertIsInstance(
            python_types.Simple()._to_py_deprecated(),
            py_deprecated_types.Simple,
        )
        self.assertIsInstance(
            testing.py_deprecated_asyncio_test.thrift_types.PyDeprecatedAsyncioStruct()._to_py_deprecated(),
            testing.py_deprecated_asyncio_test.ttypes.PyDeprecatedAsyncioStruct,
        )
        self.assertIsInstance(
            testing.py_deprecated_asyncio_fallback_test.thrift_types.PyDeprecatedAsyncioFallbackStruct()._to_py_deprecated(),
            testing.py_deprecated_asyncio_fallback_test.ttypes.PyDeprecatedAsyncioFallbackStruct,
        )

    def test_converter_to_py_deprecated_not_implemented(self) -> None:
        with self.assertRaises(NotImplementedError):
            testing_types.easy()._to_py_deprecated()
