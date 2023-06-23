#!/usr/bin/env fbpython
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

import unittest
from sys import float_info, getrefcount
from typing import Callable

# @manual=//thrift/lib/python/capi/test:marshal_fixture
from thrift.python.marshal import marshal_fixture as fixture

# @manual=//thrift/lib/python/capi/test:marshal_fixture
from thrift.python.marshal.marshal_fixture import (
    INT32_MAX,
    INT32_MIN,
    INT64_MAX,
    INT64_MIN,
    UINT32_MAX,
    UINT64_MAX,
)


class MarshalFixture(unittest.TestCase):
    def assert_type_error(self, fn: Callable[[object], object], *args: object) -> None:
        for x in args:
            with self.assertRaises(TypeError):
                fn(x)

    def assert_overflow(self, fn: Callable[[object], object], *args: object) -> None:
        for x in args:
            with self.assertRaises(OverflowError):
                fn(x)


class TestMarshalPrimitives(MarshalFixture):
    def test_int32(self) -> None:
        for x in (0, -1, INT32_MIN, INT32_MAX):
            self.assertEqual(x, fixture.roundtrip_int32(x))
        self.assert_type_error(fixture.roundtrip_int32, None, "oops")
        self.assert_overflow(fixture.roundtrip_int32, INT32_MIN - 1, INT32_MAX + 1)

    def test_int64(self) -> None:
        for x in (0, -1, INT64_MIN, INT64_MAX):
            self.assertEqual(x, fixture.roundtrip_int64(x))
        self.assert_type_error(fixture.roundtrip_int64, None, "oops")
        self.assert_overflow(fixture.roundtrip_int64, INT64_MIN - 1, INT64_MAX + 1)

    def test_uint32(self) -> None:
        for x in (0, UINT32_MAX):
            self.assertEqual(x, fixture.roundtrip_uint32(x))
        self.assert_type_error(fixture.roundtrip_uint32, None, "oops")
        self.assert_overflow(fixture.roundtrip_uint32, -1, UINT32_MAX + 1)

    def test_uint64(self) -> None:
        for x in (0, UINT64_MAX):
            self.assertEqual(x, fixture.roundtrip_uint64(x))
        self.assert_type_error(fixture.roundtrip_uint64, None, "oops")
        self.assert_overflow(fixture.roundtrip_uint64, -1, UINT64_MAX + 1)

    def test_float32(self) -> None:
        max_float32 = (2 - 2**-23) * 2.0**127
        for x in (-max_float32, -1.0, 0.0, max_float32):
            self.assertEqual(x, fixture.roundtrip_float(x))
        self.assert_type_error(fixture.roundtrip_float, None, "oops")
        self.assert_overflow(
            fixture.roundtrip_float, max_float32 * 2.0, -max_float32 * 2.0
        )

    def test_float64(self) -> None:
        for x in (-float_info.max, -1.0, 0.0, float_info.max):
            self.assertEqual(x, fixture.roundtrip_double(x))
        self.assert_type_error(fixture.roundtrip_double, None, "oops")

    def test_bool(self) -> None:
        for x in (True, False):
            self.assertEqual(x, fixture.roundtrip_bool(x))
        self.assert_type_error(fixture.roundtrip_bool, None, "oops", 1, 1.0)

    def test_bytes(self) -> None:
        for x in (b"", b"bytes", b""):
            self.assertEqual(x, fixture.roundtrip_bytes(x))
        self.assert_type_error(fixture.roundtrip_bytes, None, "oops", 1, 1.0)

    def test_unicode(self) -> None:
        for x in ("", "unicode", b"\xE2\x82\xAC".decode()):
            self.assertEqual(x, fixture.roundtrip_unicode(x))
        self.assert_type_error(fixture.roundtrip_unicode, None, b"oops", 1, 1.0)

        self.assertEqual("€", fixture.make_unicode(b"\xE2\x82\xAC"))
        with self.assertRaises(UnicodeDecodeError):
            fixture.make_unicode(b"\xE2\x82")


class TestMarshalList(MarshalFixture):
    # Use the internal representation, which is tuple for lists
    def test_int32_list(self) -> None:
        # store refcounts of singletons for leak checks
        int_refcount = getrefcount(-1)
        empty_tuple_refcount = getrefcount(())

        def make_list():
            return (0, -1, INT32_MIN, INT32_MAX)

        self.assertEqual(make_list(), fixture.roundtrip_int32_list(make_list()))
        self.assertEqual((), fixture.roundtrip_int32_list(()))
        # no leaks!
        self.assertEqual(int_refcount, getrefcount(-1))
        self.assertEqual(empty_tuple_refcount, getrefcount(()))

    def test_bool_list(self) -> None:
        # store refcounts of singletons for leak checks
        false_refcount = getrefcount(False)
        empty_tuple_refcount = getrefcount(())

        def make_list():
            return (True, False, False, False, True, False)

        self.assertEqual(make_list(), fixture.roundtrip_bool_list(make_list()))
        self.assertEqual((), fixture.roundtrip_bool_list(()))
        # no leaks!
        self.assertEqual(false_refcount, getrefcount(False))
        self.assertEqual(empty_tuple_refcount, getrefcount(()))

    def test_double_list(self) -> None:
        # no float singletons afaik
        empty_tuple_refcount = getrefcount(())

        def make_list():
            return (-1.0, 0.0, -float_info.max, float_info.max)

        self.assertEqual(make_list(), fixture.roundtrip_double_list(make_list()))
        self.assertEqual((), fixture.roundtrip_double_list(()))
        # no leaks!
        self.assertEqual(empty_tuple_refcount, getrefcount(()))

    def test_bytes_list(self) -> None:
        # empty bytes is a singleton like empty tuple
        empty_refcount = getrefcount(b"")
        empty_tuple_refcount = getrefcount(())

        def make_list():
            return (b"", b"-1", b"wef2", b"\xE2\x82\xAC")

        self.assertEqual(make_list(), fixture.roundtrip_bytes_list(make_list()))
        self.assertEqual((), fixture.roundtrip_bytes_list(()))
        # no leaks!
        self.assertEqual(empty_refcount, getrefcount(b""))
        self.assertEqual(empty_tuple_refcount, getrefcount(()))

    def test_unicode_list(self) -> None:
        # empty str is a singleton like empty tuple
        empty_refcount = getrefcount("")
        empty_tuple_refcount = getrefcount(())

        def make_list():
            return ("", "-1", "€", "", b"\xE2\x82\xAC".decode())

        self.assertEqual(make_list(), fixture.roundtrip_unicode_list(make_list()))
        self.assertEqual((), fixture.roundtrip_unicode_list(()))
        # no leaks!
        self.assertEqual(empty_refcount, getrefcount(""))
        self.assertEqual(empty_tuple_refcount, getrefcount(()))

        with self.assertRaises(UnicodeDecodeError):
            fixture.make_unicode_list((b"", b"", b"", b"", b"\xE2\x82"))
        # The empty str created before error are not leaked
        self.assertEqual(empty_refcount, getrefcount(""))
