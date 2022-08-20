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

import struct
import unittest

from thrift.transport.THeaderTransport import (
    BIG_FRAME_MAGIC,
    CLIENT_TYPE,
    MAX_FRAME_SIZE,
    THeaderTransport,
    TRANSFORM,
)
from thrift.transport.TTransport import TMemoryBuffer

MIN_HEADER_SIZE = 128


class BigFrame(unittest.TestCase):
    def round_robin(self, compress=None):
        original = b"A" * MAX_FRAME_SIZE
        mb = TMemoryBuffer()
        trans = THeaderTransport(mb, client_type=CLIENT_TYPE.HEADER)
        trans.set_max_frame_size(MAX_FRAME_SIZE + MIN_HEADER_SIZE)
        if compress:
            trans.add_transform(compress)
        trans.write(original)
        trans.flush()
        frame = mb.getvalue()
        # Cleanup the memory buffer
        mb.close()
        del mb

        if compress is None:
            # Partial Decode the frame and see if its correct size wise
            sz = struct.unpack("!I", frame[:4])[0]
            self.assertEqual(sz, BIG_FRAME_MAGIC)
            sz = struct.unpack("!Q", frame[4:12])[0]
            self.assertEqual(len(frame), sz + 12)

        # Read it back
        mb = TMemoryBuffer(frame)
        trans = THeaderTransport(mb, client_type=CLIENT_TYPE.HEADER)
        trans.set_max_frame_size(len(frame))
        trans.readFrame(0)
        result = trans.read(MAX_FRAME_SIZE)
        mb.close()
        del mb
        self.assertEqual(result, original, "round-robin different from original")

    def test_round_robin(self):
        self.round_robin()
        self.round_robin(TRANSFORM.ZLIB)
        self.round_robin(TRANSFORM.SNAPPY)
        self.round_robin(TRANSFORM.ZSTD)
