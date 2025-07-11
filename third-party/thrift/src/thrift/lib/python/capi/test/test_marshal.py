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

# pyre-unsafe

import math
import unittest
from sys import float_info, getrefcount
from typing import Callable

from folly.iobuf import IOBuf
from thrift.python.marshal import marshal_fixture as fixture
from thrift.python.marshal.marshal_fixture import (
    INT16_MAX,
    INT16_MIN,
    INT32_MAX,
    INT32_MIN,
    INT64_MAX,
    INT64_MIN,
    INT8_MAX,
    INT8_MIN,
    UINT16_MAX,
    UINT32_MAX,
    UINT64_MAX,
    UINT8_MAX,
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
    def test_int8(self) -> None:
        for x in (0, -1, INT8_MIN, INT8_MAX):
            self.assertEqual(x, fixture.roundtrip_int8(x))
        self.assert_type_error(fixture.roundtrip_int8, None, "oops")
        self.assert_overflow(fixture.roundtrip_int8, INT8_MIN - 1, INT8_MAX + 1)

    def test_int16(self) -> None:
        for x in (0, -1, INT16_MIN, INT16_MAX):
            self.assertEqual(x, fixture.roundtrip_int16(x))
        self.assert_type_error(fixture.roundtrip_int16, None, "oops")
        self.assert_overflow(fixture.roundtrip_int16, INT16_MIN - 1, INT16_MAX + 1)

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

    def test_uint8(self) -> None:
        for x in (0, UINT8_MAX):
            self.assertEqual(x, fixture.roundtrip_uint8(x))
        self.assert_type_error(fixture.roundtrip_uint8, None, "oops")
        self.assert_overflow(fixture.roundtrip_uint8, -1, UINT8_MAX + 1)

    def test_uint16(self) -> None:
        for x in (0, UINT16_MAX):
            self.assertEqual(x, fixture.roundtrip_uint16(x))
        self.assert_type_error(fixture.roundtrip_uint16, None, "oops")
        self.assert_overflow(fixture.roundtrip_uint16, -1, UINT16_MAX + 1)

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
        for x in (
            -max_float32,
            -1.0,
            0.0,
            max_float32,
            float("inf"),
            float("-inf"),
            float("nan"),
        ):
            if math.isnan(x):  # Special handling for NaN since NaN != NaN
                self.assertTrue(math.isnan(fixture.roundtrip_float(x)))
            else:
                self.assertEqual(x, fixture.roundtrip_float(x))
        self.assert_type_error(fixture.roundtrip_float, None, "oops")
        self.assert_overflow(
            fixture.roundtrip_float, max_float32 * 2.0, -max_float32 * 2.0
        )

    def test_float64(self) -> None:
        for x in (
            -float_info.max,
            -1.0,
            0.0,
            float_info.max,
            float("inf"),
            float("-inf"),
            float("nan"),
        ):
            if math.isnan(x):  # Special handling for NaN since NaN != NaN
                self.assertTrue(math.isnan(fixture.roundtrip_float(x)))
            else:
                self.assertEqual(x, fixture.roundtrip_double(x))
        self.assert_type_error(fixture.roundtrip_double, None, "oops")

    def test_bool(self) -> None:
        for x in (True, False):
            self.assertEqual(x, fixture.roundtrip_bool(x))
        self.assert_type_error(fixture.roundtrip_bool, None, "oops", 1, 1.0)

    def test_bytes(self) -> None:
        for x in (b"", b"bytes", b"\xe2\x82\xac"):
            self.assertEqual(x, fixture.roundtrip_bytes(x))
        self.assert_type_error(fixture.roundtrip_bytes, None, "oops", 1, 1.0)

    def test_unicode(self) -> None:
        for x in ("", "unicode", b"\xe2\x82\xac".decode()):
            self.assertEqual(x, fixture.roundtrip_unicode(x))
        self.assert_type_error(fixture.roundtrip_unicode, None, b"oops", 1, 1.0)

        self.assertEqual("€", fixture.make_unicode(b"\xe2\x82\xac"))
        with self.assertRaises(UnicodeDecodeError):
            fixture.make_unicode(b"\xe2\x82")

    def test_fallible_unicode(self) -> None:
        for x in ("", "unicode", b"\xe2\x82\xac".decode()):
            self.assertEqual(x, fixture.roundtrip_fallible_unicode(x))
        self.assert_type_error(fixture.roundtrip_fallible_unicode, None, 1, 1.0)

        # if the unicode is valid, it gets extracted from `bytes` to std::string by Extractor
        # then it's successfully decoded to unicode `str` by Constructor
        for x in (b"", b"bytes", b"\xe2\x82\xac"):
            self.assertEqual(x.decode(), fixture.roundtrip_fallible_unicode(x))

        # if the unicode is invalid, it gets extracted from `bytes` to std::string by Extractor
        # then unicode decode fails in Constructor, so it just constructs as `bytes`
        bad_unicode = b"\xe2\x82"
        self.assertEqual(bad_unicode, fixture.roundtrip_fallible_unicode(bad_unicode))

    def test_iobuf_stack(self) -> None:
        for b in (b"", b"bytes", b"\xe2\x82\xac"):
            x = IOBuf(memoryview(b))
            self.assertEqual(x, fixture.roundtrip_iobuf_stack(x))
        self.assert_type_error(fixture.roundtrip_iobuf_stack, None, b"oops", 1, 1.0)

    def test_iobuf_heap(self) -> None:
        for b in (b"", b"bytes", b"\xe2\x82\xac"):
            x = IOBuf(memoryview(b))
            self.assertEqual(x, fixture.roundtrip_iobuf_heap(x))
        self.assert_type_error(fixture.roundtrip_iobuf_heap, None, b"oops", 1, 1.0)


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
            return (b"", b"-1", b"wef2", b"\xe2\x82\xac")

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
            return ("", "-1", "€", "", b"\xe2\x82\xac".decode())

        self.assertEqual(make_list(), fixture.roundtrip_unicode_list(make_list()))
        self.assertEqual((), fixture.roundtrip_unicode_list(()))
        # no leaks!
        self.assertEqual(empty_refcount, getrefcount(""))
        self.assertEqual(empty_tuple_refcount, getrefcount(()))

        with self.assertRaises(UnicodeDecodeError):
            fixture.make_unicode_list((b"", b"", b"", b"", b"\xe2\x82"))
        # The empty str created before error are not leaked
        self.assertEqual(empty_refcount, getrefcount(""))


class TestMarshalSet(MarshalFixture):
    # Use the internal representation, which is frozenset
    def test_int32_set(self) -> None:
        # store refcounts of singletons for leak checks
        int_refcount = getrefcount(-1)

        def make_set():
            return frozenset({0, -1, INT32_MIN, INT32_MAX})

        self.assertEqual(make_set(), fixture.roundtrip_int32_set(make_set()))
        self.assertEqual(frozenset(), fixture.roundtrip_int32_set(frozenset()))
        # no leaks!
        self.assertEqual(int_refcount, getrefcount(-1))

    def test_bool_set(self) -> None:
        # store refcounts of singletons for leak checks
        false_refcount = getrefcount(False)

        def make_set():
            return frozenset({True, False})

        self.assertEqual(make_set(), fixture.roundtrip_bool_set(make_set()))
        self.assertEqual(frozenset(), fixture.roundtrip_bool_set(frozenset()))
        # no leaks!
        self.assertEqual(false_refcount, getrefcount(False))

    def test_double_set(self) -> None:
        def make_set():
            return frozenset({-1.0, 0.0, -float_info.max, float_info.max})

        self.assertEqual(make_set(), fixture.roundtrip_double_set(make_set()))
        self.assertEqual(frozenset(), fixture.roundtrip_double_set(frozenset()))

    def test_bytes_set(self) -> None:
        # empty bytes is a singleton like empty tuple
        empty_refcount = getrefcount(b"")

        def make_set():
            return frozenset({b"", b"-1", b"wef2", b"\xe2\x82\xac"})

        self.assertEqual(make_set(), fixture.roundtrip_bytes_set(make_set()))
        self.assertEqual(frozenset(), fixture.roundtrip_bytes_set(frozenset()))
        # no leaks!
        self.assertEqual(empty_refcount, getrefcount(b""))

    def test_unicode_set(self) -> None:
        # empty str is a singleton like empty tuple
        empty_refcount = getrefcount("")

        def make_set():
            return frozenset({"", "-1", "€", b"\xe2\x82\xac".decode()})

        self.assertEqual(make_set(), fixture.roundtrip_unicode_set(make_set()))
        self.assertEqual(frozenset(), fixture.roundtrip_unicode_set(frozenset()))
        # no leaks!
        self.assertEqual(empty_refcount, getrefcount(""))

        with self.assertRaises(UnicodeDecodeError):
            fixture.make_unicode_set(frozenset((b"", b"a", b"c", b"e", b"\xe2\x82")))
        # The empty str created before error are not leaked
        self.assertEqual(empty_refcount, getrefcount(""))


class TestMarshalMap(MarshalFixture):
    # Use the internal representation, which is tuple of (k, v) tuples.
    def test_int32_bool_map(self) -> None:
        # store refcounts of singletons for leak checks
        nil_refcount = getrefcount(0)
        int_refcount = getrefcount(-1)
        true_refcount = getrefcount(True)
        false_refcount = getrefcount(False)

        def make_dict():
            return tuple((x, x % 2 == 0) for x in [INT32_MIN, -1, 0, INT32_MAX])

        self.assertEqual(make_dict(), fixture.roundtrip_int32_bool_map(make_dict()))
        self.assertEqual((), fixture.roundtrip_int32_bool_map(()))
        # no leaks!
        self.assertEqual(nil_refcount, getrefcount(0))
        self.assertEqual(int_refcount, getrefcount(-1))
        self.assertEqual(true_refcount, getrefcount(True))
        self.assertEqual(false_refcount, getrefcount(False))

    def test_byte_float_map(self) -> None:
        # store refcounts of singletons for leak checks
        nil_refcount = getrefcount(0)
        int_refcount = getrefcount(-1)
        ace_refcount = getrefcount(1)

        def make_dict():
            return tuple((x, x / 13.0) for x in [INT8_MIN, -1, 0, 1, INT8_MAX])

        self.assertEqual(make_dict(), fixture.roundtrip_byte_float_map(make_dict()))
        self.assertEqual((), fixture.roundtrip_byte_float_map(()))
        # no leaks!
        self.assertEqual(nil_refcount, getrefcount(0))
        self.assertEqual(int_refcount, getrefcount(-1))
        self.assertEqual(ace_refcount, getrefcount(1))

    def test_bytes_key_map(self) -> None:
        # store refcounts of singletons for leak checks
        nil_refcount = getrefcount(0)
        int_refcount = getrefcount(-1)
        ace_refcount = getrefcount(1)

        def make_dict():
            return ((b"", -1), (b"asdfwe", 0), (b"wdfwe", 1))

        # This fixture uses F14FastMap to exercise the path for containers that don't
        # provide extract(). F14FastMap iter uses LIFO order for better erase semantics
        # and speed, thereby reversing the ordering
        self.assertEqual(
            make_dict(),
            tuple(reversed(fixture.roundtrip_bytes_key_map(make_dict()))),
        )
        self.assertEqual((), fixture.roundtrip_bytes_key_map(()))

        # no leaks!
        self.assertEqual(nil_refcount, getrefcount(0))
        self.assertEqual(int_refcount, getrefcount(-1))
        self.assertEqual(ace_refcount, getrefcount(1))

    def test_bytes_val_map(self) -> None:
        # store refcounts of singletons for leak checks
        nil_refcount = getrefcount(0)
        int_refcount = getrefcount(-1)
        ace_refcount = getrefcount(1)

        def make_dict():
            return ((-1, b""), (0, b"asdfwe"), (1, b"wdfwe"))

        self.assertEqual(make_dict(), fixture.roundtrip_bytes_val_map(make_dict()))
        self.assertEqual((), fixture.roundtrip_bytes_val_map(()))

        # no leaks!
        self.assertEqual(nil_refcount, getrefcount(0))
        self.assertEqual(int_refcount, getrefcount(-1))
        self.assertEqual(ace_refcount, getrefcount(1))

    def test_unicode_key_map(self) -> None:
        # store refcounts of singletons for leak checks
        nil_refcount = getrefcount(0)
        int_refcount = getrefcount(-1)
        ace_refcount = getrefcount(1)

        def make_dict():
            return (("", -1), ("asdfwe", 0), ("wdfwe", 1))

        self.assertEqual(make_dict(), fixture.roundtrip_unicode_key_map(make_dict()))
        self.assertEqual((), fixture.roundtrip_unicode_key_map(()))

        # no leaks!
        self.assertEqual(nil_refcount, getrefcount(0))
        self.assertEqual(int_refcount, getrefcount(-1))
        self.assertEqual(ace_refcount, getrefcount(1))

    def test_unicode_val_map(self) -> None:
        # store refcounts of singletons for leak checks
        nil_refcount = getrefcount(0)
        int_refcount = getrefcount(-1)
        ace_refcount = getrefcount(1)
        empty_refcount = getrefcount("")

        def make_dict():
            return ((-1, ""), (0, "asdfwe"), (1, "wdfwe"))

        # This fixture uses F14FastMap to exercise the path for containers that don't
        # provide extract(). F14FastMap iter uses LIFO order for better erase semantics
        # and speed, thereby reversing the ordering
        self.assertEqual(
            make_dict(),
            tuple(reversed(fixture.roundtrip_unicode_val_map(make_dict()))),
        )
        self.assertEqual((), fixture.roundtrip_unicode_val_map(()))

        # no leaks!
        self.assertEqual(nil_refcount, getrefcount(0))
        self.assertEqual(int_refcount, getrefcount(-1))
        self.assertEqual(ace_refcount, getrefcount(1))

        with self.assertRaises(UnicodeDecodeError):
            fixture.make_unicode_val_map(((-1, b""), (0, b"a"), (1, b"\xe2\x82")))

        self.assertEqual(nil_refcount, getrefcount(0))
        self.assertEqual(int_refcount, getrefcount(-1))
        self.assertEqual(ace_refcount, getrefcount(1))
        self.assertEqual(empty_refcount, getrefcount(""))
