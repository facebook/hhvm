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

from folly.iobuf import IOBuf
from iobuf.types import Moo
from thrift.python.serializer import deserialize, serialize_iobuf


class IOBufTests(unittest.TestCase):
    def test_get_set_struct_field(self) -> None:
        m = Moo(val=3, ptr=IOBuf(b"abcdef"), buf=IOBuf(b"xyzzy"), opt_ptr=IOBuf(b"pqr"))
        m2 = Moo(
            val=3, ptr=IOBuf(b"abcdef"), buf=IOBuf(b"xyzzy"), opt_ptr=IOBuf(b"pqr")
        )
        self.assertEqual(m, m2)
        assert m.ptr is not None
        assert m2.ptr is not None
        assert m.opt_ptr is not None
        assert m2.opt_ptr is not None

        self.assertEqual(b"".join(m.ptr), b"".join(m2.ptr))
        self.assertEqual(b"abcdef", b"".join(m.ptr))

        self.assertEqual(b"".join(m.buf), b"".join(m2.buf))
        self.assertEqual(b"xyzzy", b"".join(m.buf))

        self.assertEqual(b"".join(m.opt_ptr), b"".join(m2.opt_ptr))
        self.assertEqual(b"pqr", b"".join(m.opt_ptr))

    def test_iobuf_survives_serialization_round_trip(self) -> None:
        """Regression test: IOBuf fields from py3 structs (non-owning) must
        survive thrift-python serialization. IOBufTypeInfo.to_internal_data
        clones non-owning IOBufs so getIOBuf() can find the data."""
        m = Moo(val=7, buf=IOBuf(b"payload"), ptr=IOBuf(b"pointer"))

        import iobuf.thrift_types as python_types

        python_m = python_types.Moo(val=7, buf=m.buf, ptr=m.ptr)
        assert python_m.ptr is not None
        self.assertEqual(bytes(python_m.buf), b"payload")
        self.assertEqual(bytes(python_m.ptr), b"pointer")

        serialized = serialize_iobuf(python_m)
        deserialized = deserialize(python_types.Moo, serialized)

        self.assertEqual(bytes(deserialized.buf), b"payload")
        assert deserialized.ptr is not None
        self.assertEqual(bytes(deserialized.ptr), b"pointer")
