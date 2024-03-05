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

import unittest

from testing.thrift_types import (
    Color,
    EmptyError,
    HardError,
    SimpleError,
    UnfriendlyError,
    UnusedError,
    ValueOrError,
)
from thrift.python.exceptions import (
    ApplicationErrorType,
    Error,
    TransportError,
    TransportErrorType,
)
from thrift.python.serializer import deserialize, Protocol, serialize_iobuf


class ExceptionTests(unittest.TestCase):
    def test_hashability(self) -> None:
        hash(UnusedError())
        hash(EmptyError())

    def test_cython_enum_scope(self) -> None:
        self.assertEqual(ApplicationErrorType(6), ApplicationErrorType.INTERNAL_ERROR)

    def test_exception_message_annotation(self) -> None:
        x = UnusedError(message="something broke")
        self.assertEqual(x.message, str(x))
        y = HardError(errortext="WAT!", code=22)
        self.assertEqual(y.errortext, str(y))
        z = UnfriendlyError(errortext="WAT!", code=22)
        self.assertNotEqual(z.errortext, str(z))
        self.assertNotEqual(str(y), str(z))

    def test_creation(self) -> None:
        msg = "something broke"
        UnusedError()
        # pyre-ignore[19]: for test
        x = UnusedError(msg)
        y = UnusedError(message=msg)
        self.assertEqual(x, y)
        self.assertEqual(x.args, y.args)
        self.assertEqual(x.message, y.message)
        self.assertEqual(str(x), str(x))

    def test_raise(self) -> None:
        with self.assertRaises(SimpleError):
            raise SimpleError()

        with self.assertRaises(Error):
            raise SimpleError(Color.red)

        with self.assertRaises(Exception):  # noqa: B017
            raise SimpleError()

        with self.assertRaises(BaseException):
            raise SimpleError()

        x = SimpleError(Color.blue)

        self.assertIsInstance(x, BaseException)
        self.assertIsInstance(x, Exception)
        self.assertIsInstance(x, Error)
        self.assertIsInstance(x, SimpleError)

    def test_str(self) -> None:
        x = UnusedError()
        self.assertEqual(str(x), "")
        x2 = UnusedError(message="hello")
        self.assertEqual(str(x2), "hello")
        y = SimpleError()
        self.assertEqual(str(y), "Color.red")
        y2 = SimpleError(color=Color.red)
        self.assertEqual(str(y2), "Color.red")
        z = UnfriendlyError(errortext="WAT!", code=22)
        self.assertEqual(str(z), "('WAT!', 22)")

    def test_str_deserialized(self) -> None:
        x = deserialize(UnusedError, b"{}", protocol=Protocol.JSON)
        self.assertEqual(str(x), "")
        x2 = deserialize(UnusedError, b'{"message": "hello"}', protocol=Protocol.JSON)
        self.assertEqual(str(x2), "hello")
        y = deserialize(SimpleError, b"{}", protocol=Protocol.JSON)
        self.assertEqual(str(y), "Color.red")
        y2 = deserialize(SimpleError, b'{"color": 0}', protocol=Protocol.JSON)
        self.assertEqual(str(y2), "Color.red")
        z = deserialize(
            UnfriendlyError,
            b'{"errortext": "WAT!", "code": 22}',
            protocol=Protocol.JSON,
        )
        self.assertEqual(str(z), "('WAT!', 22)")

    def test_expr(self) -> None:
        x = UnusedError()
        x = deserialize(UnusedError, b"{}", protocol=Protocol.JSON)
        self.assertEqual(repr(x), "UnusedError(message='')")
        x2 = UnusedError(message="hello")
        self.assertEqual(repr(x2), "UnusedError(message='hello')")
        y = SimpleError()
        self.assertEqual(repr(y), "SimpleError(color=<Color.red: 0>)")
        y2 = SimpleError(color=Color.red)
        self.assertEqual(repr(y2), "SimpleError(color=<Color.red: 0>)")
        z = UnfriendlyError(errortext="WAT!", code=22)
        self.assertEqual(repr(z), "UnfriendlyError(errortext='WAT!', code=22)")

    def test_expr_deserialized(self) -> None:
        x = deserialize(UnusedError, b"{}", protocol=Protocol.JSON)
        self.assertEqual(repr(x), "UnusedError(message='')")
        x2 = deserialize(UnusedError, b'{"message": "hello"}', protocol=Protocol.JSON)
        self.assertEqual(repr(x2), "UnusedError(message='hello')")
        y = deserialize(SimpleError, b"{}", protocol=Protocol.JSON)
        self.assertEqual(repr(y), "SimpleError(color=<Color.red: 0>)")
        y2 = deserialize(SimpleError, b'{"color": 0}', protocol=Protocol.JSON)
        self.assertEqual(repr(y2), "SimpleError(color=<Color.red: 0>)")
        z = deserialize(
            UnfriendlyError,
            b'{"errortext": "WAT!", "code": 22}',
            protocol=Protocol.JSON,
        )
        self.assertEqual(repr(z), "UnfriendlyError(errortext='WAT!', code=22)")

    def test_serialize_deserialize(self) -> None:
        err = HardError(errortext="err", code=2)
        x = ValueOrError(error=err)
        serialized = serialize_iobuf(x)
        y = deserialize(ValueOrError, serialized)
        self.assertIsNot(x, y)
        self.assertEqual(x, y)

        serialized_direct = serialize_iobuf(err)
        deserialized_direct = deserialize(HardError, serialized_direct)
        self.assertIsNot(err, deserialized_direct)
        self.assertEqual(err, deserialized_direct)

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
