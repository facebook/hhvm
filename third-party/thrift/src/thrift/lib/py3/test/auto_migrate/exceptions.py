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

import types
import unittest

from testing.types import Color, HardError, SimpleError, UnfriendlyError, UnusedError
from thrift.lib.py3.test.auto_migrate.auto_migrate_util import is_auto_migrated
from thrift.lib.py3.test.exception_helper import (
    simulate_HardError,
    simulate_UnusedError,
)
from thrift.py3.exceptions import (
    ApplicationError,
    ApplicationErrorType,
    Error,
    GeneratedError,
    ProtocolError,
    ProtocolErrorType,
    TransportError,
    TransportErrorType,
    TransportOptions,
)
from thrift.py3.serializer import deserialize, serialize_iobuf
from thrift.py3.types import Struct


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

    def test_application_error_fmt(self) -> None:
        self.assertEqual(
            f"{ApplicationErrorType.UNKNOWN}", "ApplicationErrorType.UNKNOWN"
        )
        err = ApplicationError(ApplicationErrorType.UNKNOWN, "oops")
        self.assertIsInstance(err.type, ApplicationErrorType)
        self.assertEqual(f"{err.type}", "ApplicationErrorType.UNKNOWN")

    def test_creation_optional_from_python(self) -> None:
        msg = "something broke"
        UnusedError()
        # pyre-fixme[19]: Expected 0 positional arguments.
        x = UnusedError(msg)
        y = UnusedError(message=msg)
        self.assertEqual(x, y)
        self.assertEqual(x.args, (msg,))
        self.assertEqual(x.message, msg)
        self.assertEqual(str(x), msg)
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
        self.assertEqual(x.args, (msg, code))
        self.assertEqual(str(x), msg)
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

    def test_instance(self) -> None:
        self.assertIsInstance(HardError(errortext="err", code=2), GeneratedError)
        self.assertIsInstance(HardError(errortext="err", code=2), Error)
        self.assertIsInstance(HardError(errortext="err", code=2), HardError)
        self.assertNotIsInstance(HardError(errortext="err", code=2), SimpleError)
        self.assertNotIsInstance(3, HardError)
        self.assertNotIsInstance(3, GeneratedError)
        self.assertNotIsInstance(HardError(errortext="err", code=2), Struct)
        self.assertTrue(issubclass(HardError, GeneratedError))
        self.assertTrue(issubclass(HardError, Error))
        self.assertTrue(issubclass(HardError, HardError))
        self.assertFalse(issubclass(HardError, SimpleError))
        self.assertFalse(issubclass(int, HardError))
        self.assertFalse(issubclass(int, GeneratedError))
        self.assertFalse(issubclass(HardError, Struct))

    def test_subclass_not_allow_inheritance(self) -> None:
        thrift_python_err = (
            r"Inheritance from generated thrift exception .+ is deprecated"
        )
        cython_err = (
            r"type '.+' is not an acceptable base type"
            if not hasattr(HardError, "_FBTHRIFT__PYTHON_CLASS")
            else r"Inheritance of thrift-generated .+ from TestSubclass is deprecated."
        )
        err_regex = thrift_python_err if is_auto_migrated() else cython_err

        with self.assertRaisesRegex(TypeError, err_regex):
            types.new_class("TestSubclass", bases=(HardError,))

    def test_subclass_allow_inheritance(self) -> None:
        c = SubclassError(message="halp")
        self.assertIsInstance(c, UnusedError)
        self.assertIsInstance(c, GeneratedError)
        self.assertIsInstance(UnusedError(), GeneratedError)
        self.assertEqual(c.message, "halp")

        for catch in (SubclassError, UnusedError, GeneratedError):
            with self.assertRaises(catch):
                raise SubclassError()

    def test_subclass_allow_inheritance_ancestor(self) -> None:
        c = SubSubclassError(message="halp")
        self.assertIsInstance(c, SubclassError)
        self.assertIsInstance(c, UnusedError)
        self.assertIsInstance(c, GeneratedError)
        self.assertEqual(c.message, "halp")

        for catch in (SubSubclassError, SubclassError, UnusedError, GeneratedError):
            with self.assertRaises(catch):
                raise SubSubclassError()


class SubclassError(UnusedError):
    pass


class SubSubclassError(SubclassError):
    pass
