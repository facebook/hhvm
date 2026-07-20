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
from typing import Any, Callable, Type

import iobuf.thrift_mutable_types as mutable_types
import iobuf.thrift_types as immutable_types
from folly.iobuf import IOBuf, WritableIOBuf
from iobuf.thrift_types import Moo as MooType
from parameterized import parameterized_class
from thrift.python.mutable_serializer import (
    deserialize as mutable_deserialize,
    serialize_iobuf as mutable_serialize_iobuf,
)
from thrift.python.serializer import deserialize, serialize_iobuf


@parameterized_class(
    ("test_types"),
    [(immutable_types,), (mutable_types,)],
)
class IOBufTests(unittest.TestCase):
    def setUp(self) -> None:
        """
        The `setUp` method performs these assignments with type hints to enable
        pyre when using 'parameterized'. Otherwise, Pyre cannot deduce the types
        behind `test_types`.
        """
        # pyre-ignore[16]: has no attribute `sets_types`
        self.Moo: Type[MooType] = self.test_types.Moo
        # pyrefly: ignore [missing-attribute]
        if self.test_types is mutable_types:
            self._serialize: Callable[..., Any] = mutable_serialize_iobuf
            self._deserialize: Callable[..., Any] = mutable_deserialize
        else:
            self._serialize = serialize_iobuf
            self._deserialize = deserialize

    def test_get_set_struct_field(self) -> None:
        m = self.Moo(
            val=3, ptr=IOBuf(b"abcdef"), buf=IOBuf(b"xyzzy"), opt_ptr=IOBuf(b"pqr")
        )
        m2 = self.Moo(
            val=3, ptr=IOBuf(b"abcdef"), buf=IOBuf(b"xyzzy"), opt_ptr=IOBuf(b"pqr")
        )
        self.assertEqual(m, m2)
        assert m.ptr is not None
        assert m2.ptr is not None
        assert m.opt_ptr is not None
        assert m2.opt_ptr is not None

        self.assertEqual(bytes(m.ptr), bytes(m2.ptr))
        self.assertEqual(b"abcdef", bytes(m.ptr))

        self.assertEqual(bytes(m.buf), bytes(m2.buf))
        self.assertEqual(b"xyzzy", bytes(m.buf))

        self.assertEqual(bytes(m.opt_ptr), bytes(m2.opt_ptr))
        self.assertEqual(b"pqr", bytes(m.opt_ptr))

    def test_non_owning_iobuf_survives_serialize(self) -> None:
        """Regression test: non-owning IOBuf (e.g. from chained IOBuf
        iteration) must survive thrift-python serialization round-trip.

        Non-owning IOBufs created via IOBuf.create() only set _this, not
        _ours. IOBufTypeInfo.to_internal_data copies them to produce an
        owning IOBuf so that the serializer's getIOBuf() can find the
        data via _ours."""
        # Create a chained IOBuf: [b"hello"] -> [b"world"]
        head = WritableIOBuf(bytearray(b"hello"))
        tail = WritableIOBuf(bytearray(b"world"))
        head.append_to_chain(tail)

        # .next returns a non-owning IOBuf (IOBuf.create: _this set, _ours not)
        non_owning = head.next
        assert non_owning is not None
        self.assertEqual(bytes(non_owning), b"world")

        m = self.Moo(val=42, buf=non_owning)

        serialized = self._serialize(m)
        deserialized = self._deserialize(type(m), serialized)

        self.assertEqual(deserialized.val, 42)
        # cloneCoalesced() from the "world" node yields the full chain
        # starting at that position: "world" + "hello".
        self.assertEqual(bytes(deserialized.buf), b"worldhello")

    def test_chained_iobuf_serialize_round_trip(self) -> None:
        """Documents current trunk behavior for an owning chained IOBuf passed
        as a struct field through serialize-deserialize."""
        head = WritableIOBuf(bytearray(b"hello"))
        tail = WritableIOBuf(bytearray(b"world"))
        head.append_to_chain(tail)

        self.assertTrue(head.is_chained)
        self.assertEqual(b"".join(head), b"helloworld")

        m = self.Moo(val=1, buf=head)
        serialized = self._serialize(m)
        deserialized = self._deserialize(type(m), serialized)

        self.assertEqual(deserialized.val, 1)
        result_buf = deserialized.buf
        self.assertIsInstance(result_buf, IOBuf)
        self.assertFalse(result_buf.is_chained)
        self.assertEqual(bytes(result_buf), b"helloworld")
        self.assertEqual(b"".join(result_buf), b"helloworld")
