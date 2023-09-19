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


from __future__ import annotations

import asyncio
import unittest
from typing import Any

from binary.thrift_clients import BinaryService
from binary.thrift_services import BinaryServiceInterface
from binary.thrift_types import Binaries, BinaryUnion
from folly.iobuf import IOBuf
from thrift.py3.server import SocketAddress
from thrift.python.client import get_client
from thrift.python.server import ServiceInterface, ThriftServer


class BinaryTests(unittest.TestCase):
    def test_various_binary_types(self) -> None:
        val = Binaries(
            no_special_type=b"abcdef",
            iobuf_val=IOBuf(b"mnopqr"),
            iobuf_ptr=IOBuf(b"ghijkl"),
            fbstring=b"stuvwx",
            nonstandard_type=b"yzabcd",
        )
        self.assertEqual(val.no_special_type, b"abcdef")
        self.assertEqual(bytes(val.iobuf_val), b"mnopqr")
        assert val.iobuf_ptr is not None
        self.assertEqual(bytes(val.iobuf_ptr), b"ghijkl")
        self.assertEqual(val.fbstring, b"stuvwx")
        self.assertEqual(val.nonstandard_type, b"yzabcd")

    def test_binary_union(self) -> None:
        val = BinaryUnion(iobuf_val=IOBuf(b"mnopqr"))
        self.assertEqual(bytes(val.iobuf_val), b"mnopqr")


class BinaryHandler(BinaryServiceInterface):
    # pyre-fixme[2]: Parameter must be annotated.
    def __init__(self, unit_test) -> None:
        # pyre-fixme[4]: Attribute must be annotated.
        self.unit_test = unit_test

    async def sendRecvBinaries(self, val: Binaries) -> Binaries:
        self.unit_test.assertEqual(val.no_special_type, b"c1")
        self.unit_test.assertEqual(bytes(val.iobuf_val), b"c2")
        assert val.iobuf_ptr is not None
        self.unit_test.assertEqual(bytes(val.iobuf_ptr), b"c3")
        self.unit_test.assertEqual(val.fbstring, b"c4")
        self.unit_test.assertEqual(val.nonstandard_type, b"c5")
        return Binaries(
            no_special_type=b"s1",
            iobuf_val=IOBuf(b"s2"),
            iobuf_ptr=IOBuf(b"s3"),
            fbstring=b"s4",
            nonstandard_type=b"s5",
        )

    async def sendRecvBinary(self, val: bytes) -> bytes:
        self.unit_test.assertEqual(val, b"cv1")
        return b"sv1"

    async def sendRecvIOBuf(self, val: IOBuf) -> IOBuf:
        self.unit_test.assertEqual(bytes(val), b"cv2")
        return IOBuf(b"sv2")

    async def sendRecvIOBufPtr(self, val: IOBuf) -> IOBuf:
        self.unit_test.assertEqual(bytes(val), b"cv3")
        return IOBuf(b"sv3")

    async def sendRecvFbstring(self, val: bytes) -> bytes:
        self.unit_test.assertEqual(val, b"cv4")
        return b"sv4"

    async def sendRecvBuffer(self, val: bytes) -> bytes:
        self.unit_test.assertEqual(val, b"cv5")
        return b"sv5"

    async def sendRecBinaryUnion(self, val: BinaryUnion) -> BinaryUnion:
        self.unit_test.assertEqual(bytes(val.iobuf_val), b"cv6")
        return BinaryUnion(iobuf_val=IOBuf(b"sv6"))


# pyre-fixme[13]: Attribute `serve_task` is never initialized.
class TestServer:
    server: ThriftServer
    serve_task: asyncio.Task

    def __init__(self, *, ip: str, handler: ServiceInterface) -> None:
        self.server = ThriftServer(handler, ip=ip, path=None)

    async def __aenter__(self) -> SocketAddress:
        self.serve_task = asyncio.get_event_loop().create_task(self.server.serve())
        return await self.server.get_address()

    # pyre-fixme[2]: Parameter must be annotated.
    async def __aexit__(self, *exc_info) -> None:
        self.server.stop()
        await self.serve_task


class ClientBinaryServerTests(unittest.IsolatedAsyncioTestCase):
    async def test_send_recv(self) -> None:
        async with TestServer(handler=BinaryHandler(self), ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(BinaryService, host=ip, port=port) as client:
                # pyre-fixme[33]: Given annotation cannot be `Any`.
                val: Any
                val = await client.sendRecvBinaries(
                    Binaries(
                        no_special_type=b"c1",
                        iobuf_val=IOBuf(b"c2"),
                        iobuf_ptr=IOBuf(b"c3"),
                        fbstring=b"c4",
                        nonstandard_type=b"c5",
                    )
                )
                self.assertEqual(val.no_special_type, b"s1")
                self.assertEqual(bytes(val.iobuf_val), b"s2")
                assert val.iobuf_ptr is not None
                self.assertEqual(bytes(val.iobuf_ptr), b"s3")
                self.assertEqual(val.fbstring, b"s4")
                self.assertEqual(val.nonstandard_type, b"s5")

                val = await client.sendRecvBinary(b"cv1")
                self.assertEqual(val, b"sv1")

                val = await client.sendRecvIOBuf(IOBuf(b"cv2"))
                self.assertEqual(bytes(val), b"sv2")

                val = await client.sendRecvIOBufPtr(IOBuf(b"cv3"))
                self.assertEqual(bytes(val), b"sv3")

                val = await client.sendRecvFbstring(b"cv4")
                self.assertEqual(val, b"sv4")

                val = await client.sendRecvBuffer(b"cv5")
                self.assertEqual(val, b"sv5")

                bu = BinaryUnion(iobuf_val=IOBuf(b"cv6"))
                val = await client.sendRecBinaryUnion(bu)
                self.assertEqual(bytes(val.iobuf_val), b"sv6")
