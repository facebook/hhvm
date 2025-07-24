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


from __future__ import annotations

import copy

import types

import unittest
from typing import Type

import testing.thrift_mutable_types as mutable_test_types
import testing.thrift_types as immutable_test_types

import thrift.python.mutable_serializer as mutable_serializer
import thrift.python.serializer as immutable_serializer

from parameterized import parameterized_class

from testing.thrift_types import (
    Color as ColorType,
    EmptyError as EmptyErrorType,
    HardError as HardErrorType,
    SimpleError as SimpleErrorType,
    UnfriendlyError as UnfriendlyErrorType,
    UnusedError as UnusedErrorType,
    ValueOrError as ValueOrErrorType,
)
from thrift.python.exceptions import (
    ApplicationError,
    ApplicationErrorType,
    Error,
    GeneratedError,
    TransportError,
    TransportErrorType,
)
from thrift.python.types import Struct


class TypeIndependentTests(unittest.TestCase):
    """
    There is only one `Error`, `ApplicationError`, etc. We do not create
    mutable versions of them. Both mutable and immutable types utilize them.
    """

    def test_cython_enum_scope(self) -> None:
        self.assertEqual(ApplicationErrorType(6), ApplicationErrorType.INTERNAL_ERROR)

    def test_invalid_error_type(self) -> None:
        with self.assertRaises(TypeError):
            ApplicationError("msg", "legit error message")  # pyre-ignore
        self.assertEqual(
            TransportError(
                type=0,  # pyre-ignore
                message="transport error",
                errno=5,
                options=1,
            ).type,
            TransportErrorType.UNKNOWN,
        )
        with self.assertRaises(TypeError):
            TransportError(
                type="oops",  # pyre-ignore
                message="transport error",
                errno=5,
                options=1,
            )
        with self.assertRaises(TypeError):
            TransportError(
                type=100,  # pyre-ignore
                message="transport error",
                errno=5,
                options=1,
            )

        # allow this for backwards-compatibility reasons
        self.assertEqual(
            TransportError(
                type=None,  # pyre-ignore
                message="transport error",
                errno=5,
                options=1,
            ).type,
            TransportErrorType.UNKNOWN,
        )

    def test_application_error_fmt(self) -> None:
        self.assertEqual(
            f"{ApplicationErrorType.UNKNOWN}", "ApplicationErrorType.UNKNOWN"
        )
        err = ApplicationError(ApplicationErrorType.UNKNOWN, "oops")
        self.assertIsInstance(err.type, ApplicationErrorType)
        self.assertEqual(f"{err.type}", "ApplicationErrorType.UNKNOWN")

    def test_create_transporterror_should_set_correct_values(self) -> None:
        t = TransportError(
            type=TransportErrorType.CORRUPTED_DATA,
            message="transport error",
            errno=5,
            options=1,
        )
        self.assertEqual(t.type, TransportErrorType.CORRUPTED_DATA)
        self.assertEqual(t.message, "transport error")
        self.assertEqual(t.errno, 5)
        self.assertEqual(t.options, 1)


class ImmutableExceptionTests(unittest.TestCase):
    def test_hashability(self) -> None:
        hash(immutable_test_types.UnusedError())
        hash(immutable_test_types.EmptyError())


@parameterized_class(
    ("test_types", "serializer_module"),
    [
        (immutable_test_types, immutable_serializer),
        (mutable_test_types, mutable_serializer),
    ],
)
class ExceptionTests(unittest.TestCase):
    def setUp(self) -> None:
        """
        The `setUp` method performs these assignments with type hints to enable
        pyre when using 'parameterized'. Otherwise, Pyre cannot deduce the types
        behind `test_types`.
        """
        # pyre-ignore[16]: has no attribute `test_types`
        self.UnusedError: Type[UnusedErrorType] = self.test_types.UnusedError
        self.HardError: Type[HardErrorType] = self.test_types.HardError
        self.UnfriendlyError: Type[UnfriendlyErrorType] = (
            self.test_types.UnfriendlyError
        )
        self.SimpleError: Type[SimpleErrorType] = self.test_types.SimpleError
        self.Color: Type[ColorType] = self.test_types.Color
        self.ValueOrError: Type[ValueOrErrorType] = self.test_types.ValueOrError
        self.is_mutable_run: bool = self.test_types.__name__.endswith(
            "thrift_mutable_types"
        )
        # pyre-ignore[16]: has no attribute `serializer_module`
        self.serializer: types.ModuleType = self.serializer_module

    def test_exception_message_annotation(self) -> None:
        x = self.UnusedError(message="something broke")
        self.assertEqual(x.message, str(x))
        y = self.HardError(errortext="WAT!", code=22)
        self.assertEqual(y.errortext, str(y))
        z = self.UnfriendlyError(errortext="WAT!", code=22)
        self.assertNotEqual(z.errortext, str(z))
        self.assertNotEqual(str(y), str(z))

    def test_creation(self) -> None:
        msg = "something broke"
        self.UnusedError()
        # pyre-ignore[19]: for test
        x = self.UnusedError(msg)
        y = self.UnusedError(message=msg)
        self.assertEqual(x, y)
        self.assertEqual(x.args, y.args)
        self.assertEqual(x.message, y.message)
        self.assertEqual(str(x), str(x))

    def test_raise(self) -> None:
        with self.assertRaises(self.SimpleError):
            raise self.SimpleError()

        with self.assertRaises(Error):
            raise self.SimpleError(self.Color.red)

        with self.assertRaises(Exception):  # noqa: B017
            raise self.SimpleError()

        with self.assertRaises(BaseException):
            raise self.SimpleError()

        x = self.SimpleError(self.Color.blue)

        self.assertIsInstance(x, BaseException)
        self.assertIsInstance(x, Exception)
        self.assertIsInstance(x, Error)
        self.assertIsInstance(x, self.SimpleError)

    def test_str(self) -> None:
        x = self.UnusedError()
        self.assertEqual(str(x), "")
        x2 = self.UnusedError(message="hello")
        self.assertEqual(str(x2), "hello")
        y = self.SimpleError()
        self.assertEqual(str(y), "Color.red")
        y2 = self.SimpleError(color=self.Color.red)
        self.assertEqual(str(y2), "Color.red")
        z = self.UnfriendlyError(errortext="WAT!", code=22)
        self.assertEqual(str(z), "('WAT!', 22)")

    def test_str_deserialized(self) -> None:
        x = self.serializer.deserialize(
            self.UnusedError, b"{}", protocol=self.serializer.Protocol.JSON
        )
        self.assertEqual(str(x), "")
        x2 = self.serializer.deserialize(
            self.UnusedError,
            b'{"message": "hello"}',
            protocol=self.serializer.Protocol.JSON,
        )
        self.assertEqual(str(x2), "hello")
        y = self.serializer.deserialize(
            self.SimpleError, b"{}", protocol=self.serializer.Protocol.JSON
        )
        self.assertEqual(str(y), "Color.red")
        y2 = self.serializer.deserialize(
            self.SimpleError, b'{"color": 0}', protocol=self.serializer.Protocol.JSON
        )
        self.assertEqual(str(y2), "Color.red")
        z = self.serializer.deserialize(
            self.UnfriendlyError,
            b'{"errortext": "WAT!", "code": 22}',
            protocol=self.serializer.Protocol.JSON,
        )
        self.assertEqual(str(z), "('WAT!', 22)")

    def test_expr(self) -> None:
        x = self.UnusedError()
        x = self.serializer.deserialize(
            self.UnusedError, b"{}", protocol=self.serializer.Protocol.JSON
        )
        self.assertEqual(repr(x), "UnusedError(message='')")
        x2 = self.UnusedError(message="hello")
        self.assertEqual(repr(x2), "UnusedError(message='hello')")
        y = self.SimpleError()
        self.assertEqual(repr(y), "SimpleError(color=<Color.red: 0>)")
        y2 = self.SimpleError(color=self.Color.red)
        self.assertEqual(repr(y2), "SimpleError(color=<Color.red: 0>)")
        z = self.UnfriendlyError(errortext="WAT!", code=22)
        self.assertEqual(repr(z), "UnfriendlyError(errortext='WAT!', code=22)")

    def test_expr_deserialized(self) -> None:
        x = self.serializer.deserialize(
            self.UnusedError, b"{}", protocol=self.serializer.Protocol.JSON
        )
        self.assertEqual(repr(x), "UnusedError(message='')")
        x2 = self.serializer.deserialize(
            self.UnusedError,
            b'{"message": "hello"}',
            protocol=self.serializer.Protocol.JSON,
        )
        self.assertEqual(repr(x2), "UnusedError(message='hello')")
        y = self.serializer.deserialize(
            self.SimpleError, b"{}", protocol=self.serializer.Protocol.JSON
        )
        self.assertEqual(repr(y), "SimpleError(color=<Color.red: 0>)")
        y2 = self.serializer.deserialize(
            self.SimpleError, b'{"color": 0}', protocol=self.serializer.Protocol.JSON
        )
        self.assertEqual(repr(y2), "SimpleError(color=<Color.red: 0>)")
        z = self.serializer.deserialize(
            self.UnfriendlyError,
            b'{"errortext": "WAT!", "code": 22}',
            protocol=self.serializer.Protocol.JSON,
        )
        self.assertEqual(repr(z), "UnfriendlyError(errortext='WAT!', code=22)")

    def test_serialize_deserialize(self) -> None:
        if self.is_mutable_run:
            # `ValueOrError` is an union, MutableUnion implementation is not complete yet
            return

        err = self.HardError(errortext="err", code=2)
        x = self.ValueOrError(error=err)
        serialized = self.serializer.serialize_iobuf(x)
        y = self.serializer.deserialize(self.ValueOrError, serialized)
        self.assertIsNot(x, y)
        self.assertEqual(x, y)

        serialized_direct = self.serializer.serialize_iobuf(err)
        deserialized_direct = self.serializer.deserialize(
            self.HardError, serialized_direct
        )
        self.assertIsNot(err, deserialized_direct)
        self.assertEqual(err, deserialized_direct)

    def test_instance(self) -> None:
        self.assertIsInstance(HardErrorType(errortext="err", code=2), GeneratedError)
        self.assertIsInstance(HardErrorType(errortext="err", code=2), Error)
        self.assertIsInstance(HardErrorType(errortext="err", code=2), HardErrorType)
        self.assertNotIsInstance(
            HardErrorType(errortext="err", code=2), SimpleErrorType
        )
        self.assertNotIsInstance(3, HardErrorType)
        self.assertNotIsInstance(3, GeneratedError)
        self.assertNotIsInstance(HardErrorType(errortext="err", code=2), Struct)
        self.assertTrue(issubclass(HardErrorType, GeneratedError))
        self.assertTrue(issubclass(HardErrorType, Error))
        self.assertTrue(issubclass(HardErrorType, HardErrorType))
        self.assertFalse(issubclass(HardErrorType, SimpleErrorType))
        self.assertFalse(issubclass(int, HardErrorType))
        self.assertFalse(issubclass(int, GeneratedError))
        self.assertFalse(issubclass(HardErrorType, Struct))

    def test_copy(self) -> None:
        x = self.HardError(errortext="err", code=2)
        y = copy.copy(x)
        self.assertEqual(x, y)
        self.assertIs(x.errortext, y.errortext)
        if self.is_mutable_run:
            self.assertIsNot(x, y)
        else:
            self.assertIs(x, y)

    def test_deepcopy(self) -> None:
        x = self.HardError(errortext="err", code=2)
        y = copy.deepcopy(x)
        self.assertEqual(x, y)

        if self.is_mutable_run:
            self.assertIsNot(x, y)
            # strings are different, but ints are same (small int optimization)
            self.assertIsNot(x.errortext, y.errortext)
        else:
            self.assertIs(x, y)
