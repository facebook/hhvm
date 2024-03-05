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

from testing.types import Color, HardError, SimpleError, UnfriendlyError, UnusedError
from thrift.py3.exceptions import (
    ApplicationError,
    ApplicationErrorType,
    Error,
    ProtocolError,
    ProtocolErrorType,
    TransportError,
    TransportErrorType,
    TransportOptions,
)
from thrift.py3.serializer import deserialize, serialize_iobuf

from .exception_helper import simulate_HardError, simulate_UnusedError


class ExceptionTests(unittest.TestCase):
    def test_hashability(self) -> None:
        hash(UnusedError())

    def test_creation_optional_from_c(self) -> None:
        msg = "this is what happened"
        x = simulate_UnusedError(msg)
        self.assertIsInstance(x, UnusedError)
        self.assertIn(msg, str(x))
        self.assertIn(msg, x.args)
        self.assertEqual(msg, x.message)
        self.assertEqual(UnusedError(*x.args), x)

    def test_exception_message_annotation(self) -> None:
        x = UnusedError(message="something broke")
        self.assertEqual(x.message, str(x))
        # pyre-fixme[19]: Expected 0 positional arguments.
        y = HardError("WAT!", 22)
        self.assertEqual(y.errortext, str(y))
        # pyre-fixme[19]: Expected 0 positional arguments.
        z = UnfriendlyError("WAT!", 22)
        self.assertNotEqual(z.errortext, str(z))
        self.assertNotEqual(str(y), str(z))

    def test_creation_optional_from_python(self) -> None:
        msg = "something broke"
        UnusedError()
        # pyre-fixme[19]: Expected 0 positional arguments.
        x = UnusedError(msg)
        y = UnusedError(message=msg)
        self.assertEqual(x, y)
        self.assertEqual(x.args, y.args)
        self.assertEqual(x.message, y.message)
        self.assertEqual(str(x), str(x))

    def test_creation_required_from_c(self) -> None:
        msg = "ack!"
        code = 22
        x = simulate_HardError(msg, code)
        self.assertIsInstance(x, HardError)
        self.assertIn(msg, str(x))
        self.assertIn(msg, x.args)
        self.assertIn(code, x.args)
        self.assertEqual(code, x.code)
        self.assertEqual(msg, x.errortext)
        self.assertEqual(x, HardError(*x.args))

    def test_creation_required_from_python(self) -> None:
        msg = "ack!"
        code = 22
        # pyre-fixme[19]: Expected 0 positional arguments.
        w = HardError(msg)
        self.assertEqual(w.code, 0)
        # pyre-fixme[19]: Expected 0 positional arguments.
        x = HardError(msg, code)
        # pyre-fixme[19]: Expected 0 positional arguments.
        y = HardError(msg, code=code)
        self.assertEqual(x, y)
        self.assertEqual(x.args, y.args)
        self.assertEqual(x.errortext, y.errortext)
        self.assertEqual(x.code, y.code)
        self.assertEqual(str(x), str(y))

    def test_raise(self) -> None:
        with self.assertRaises(SimpleError):
            raise SimpleError()

        with self.assertRaises(Error):
            raise SimpleError(Color.red)

        with self.assertRaises(Exception):
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

    def test_create_from_python(self) -> None:
        a = ApplicationError(type=ApplicationErrorType.INVALID_TRANSFORM, message="abc")
        self.assertEqual(a.type, ApplicationErrorType.INVALID_TRANSFORM)
        self.assertEqual(a.message, "abc")

        p = ProtocolError(type=ProtocolErrorType.NEGATIVE_SIZE, message="qed")
        self.assertEqual(p.type, ProtocolErrorType.NEGATIVE_SIZE)
        self.assertEqual(p.message, "qed")

        t = TransportError(
            type=TransportErrorType.CORRUPTED_DATA,
            message="der",
            errno=5,
            options=TransportOptions.CHANNEL_IS_VALID,
        )
        self.assertEqual(t.type, TransportErrorType.CORRUPTED_DATA)
        self.assertEqual(t.message, "der")
        self.assertEqual(t.errno, 5)
        self.assertEqual(t.options, TransportOptions.CHANNEL_IS_VALID)

    def test_serialize_deserialize(self) -> None:
        err = HardError(errortext="err", code=2)
        serialized = serialize_iobuf(err)
        deserialized = deserialize(HardError, serialized)
        self.assertIsNot(err, deserialized)
        self.assertEqual(err, deserialized)
