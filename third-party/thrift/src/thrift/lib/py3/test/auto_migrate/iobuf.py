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
