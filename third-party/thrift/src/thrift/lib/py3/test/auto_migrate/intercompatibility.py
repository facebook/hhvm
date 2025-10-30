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
from pickle import dumps, loads

import testing.thrift_types as python_types
import testing.types as py3_types
from thrift.lib.py3.test.auto_migrate.auto_migrate_util import is_auto_migrated


class PythonCompatibilityTest(unittest.TestCase):
    def test_init_py3_struct_with_python_union(self) -> None:
        python_integers = python_types.Integers(small=2023)
        # pyre-fixme[6]: In call `py3_types.easy.__init__`, for argument `an_int`, expected `Optional[py3_types.Integers]` but got `python_types.Integers`.
        py3_easy = py3_types.easy(an_int=python_integers)
        self.assertEqual(2023, py3_easy.an_int.small)

    def test_init_py3_struct_with_python_exception(self) -> None:
        python_exception = python_types.HardError(errortext="hard error")
        # pyre-ignore[6]: Incompatible parameter type [6]: In call `py3_types.NestedError.__init__`, for argument `val_error`, expected `Optional[py3_types.HardError]` but got `python_types.HardError`.
        py3_nesterror = py3_types.NestedError(val_error=python_exception)
        self.assertIsInstance(py3_nesterror.val_error, py3_types.HardError)
        self.assertEqual("hard error", py3_nesterror.val_error.errortext)

    def test_init_py3_union_with_python_exception(self) -> None:
        python_exception = python_types.HardError(errortext="hard error")
        # pyre-ignore[6]: Incompatible parameter type [6]: In call `py3_types.ValueOrError.__init__`, for argument `error`, expected `Optional[py3_types.HardError]` but got `python_types.HardError`.
        py3_nested_union = py3_types.ValueOrError(error=python_exception)
        self.assertIsInstance(py3_nested_union.error, py3_types.HardError)
        self.assertEqual("hard error", py3_nested_union.error.errortext)

    def test_init_py3_exception_with_python_exception(self) -> None:
        python_exception = python_types.HardError(errortext="hard error")
        # pyre-ignore[6]: Incompatible parameter type [6]: In call `py3_types.NestedHardError.__init__`, for argument `error`, expected `Optional[py3_types.HardError]` but got `python_types.HardError`.
        py3_nested_exception = py3_types.NestedHardError(error=python_exception)
        self.assertIsInstance(py3_nested_exception.error, py3_types.HardError)
        self.assertEqual("hard error", py3_nested_exception.error.errortext)

    def test_update_py3_struct_with_python_union(self) -> None:
        py3_easy = py3_types.easy()
        python_integers = python_types.Integers(small=2023)
        # pyre-fixme[6]: In call `py3_types.easy.__call__`, for argument `an_int`, expected `Optional[py3_types.Integers]` but got `python_types.Integers`
        py3_easy = py3_easy(an_int=python_integers)
        self.assertEqual(2023, py3_easy.an_int.small)

    def test_init_py3_struct_with_python_enum(self) -> None:
        py3_file = py3_types.File(
            permissions=(python_types.Perm.read | python_types.Perm.read),
            type=python_types.Kind.FIFO,
        )
        self.assertEqual(
            py3_types.Perm.read | py3_types.Perm.read,
            py3_file.permissions,
        )
        self.assertEqual(py3_types.Kind.FIFO, py3_file.type)

    def test_update_py3_struct_with_python_enum(self) -> None:
        py3_file = py3_types.File()
        py3_file = py3_file(
            permissions=(python_types.Perm.read | python_types.Perm.read),
            type=python_types.Kind.FIFO,
        )
        self.assertEqual(
            py3_types.Perm.read | py3_types.Perm.read,
            py3_file.permissions,
        )
        self.assertEqual(py3_types.Kind.FIFO, py3_file.type)

    def test_init_py3_union_with_python_struct(self) -> None:
        python_easy = python_types.easy(name="foo")
        # pyre-fixme[6]: Incompatible parameter type. In call `py3_types.ComplexUnion.__init__`, for argument `easy_struct`, expected `Optional[py3_types.easy]` but got `python_types.easy`.Incompatible parameter type
        py3_complex_union = py3_types.ComplexUnion(easy_struct=python_easy)
        self.assertEqual("foo", py3_complex_union.easy_struct.name)


class PicklingCompatibilityTest(unittest.TestCase):
    def make_hard(self) -> python_types.hard:
        return python_types.hard(
            val=42,
            val_list=[1, 1, 2, 3, 5, 8, 13],
            name="foo",
            an_int=python_types.Integers(small=300),
        )

    def assert_hard(self, h: python_types.hard | py3_types.hard) -> None:
        self.assertEqual(42, h.val)
        self.assertEqual([1, 1, 2, 3, 5, 8, 13], h.val_list)
        self.assertEqual("foo", h.name)
        self.assertEqual(300, h.an_int.small)

    def test_python_to_py3_unpickle(self) -> None:
        # !!! This is not true for all time. pickle is a moving target its not a stable serialization format between python versions !!!
        # python_pickled = dumps(self.make_hard())
        # py3_pickled = dumps(self.make_hard()._to_py3())
        # self.assertEqual(self.pickled_python(), python_pickled)
        # if not is_auto_migrated():
        #    self.assertEqual(self.pickled_py3(), py3_pickled)

        # But you should be able unpickle it

        python_hard = loads(self.pickled_python())
        self.assertTrue(
            python_hard.__class__.__module__.endswith(".thrift_types"),
            type(python_hard),
        )
        py3_hard = loads(self.pickled_py3())
        expected_ending = ".thrift_types" if is_auto_migrated() else ".types"
        self.assertTrue(
            py3_hard.__class__.__module__.endswith(expected_ending), type(py3_hard)
        )
        self.assertEqual(py3_hard, python_hard._to_py3())
        self.assertEqual(python_hard, py3_hard._to_python())
        self.assert_hard(py3_hard)
        self.assert_hard(python_hard)

    def pickled_python(self) -> bytes:
        return b"\x80\x04\x95w\x00\x00\x00\x00\x00\x00\x00\x8c\x13thrift.python.types\x94\x8c\x10_unpickle_struct\x94\x93\x94\x8c\x14testing.thrift_types\x94\x8c\x04hard\x94\x93\x94C$\x15T\x19u\x02\x02\x04\x06\n\x10\x1a\x18\x03foo\x1c$\xd8\x04\x00\x18\x0csome default\x00\x94\x86\x94R\x94."

    def pickled_py3(self) -> bytes:
        return b"\x80\x04\x95m\x00\x00\x00\x00\x00\x00\x00\x8c\x15thrift.py3.serializer\x94\x8c\x0bdeserialize\x94\x93\x94\x8c\rtesting.types\x94\x8c\x04hard\x94\x93\x94C$\x15T\x19u\x02\x02\x04\x06\n\x10\x1a\x18\x03foo\x1c$\xd8\x04\x00\x18\x0csome default\x00\x94\x86\x94R\x94."
