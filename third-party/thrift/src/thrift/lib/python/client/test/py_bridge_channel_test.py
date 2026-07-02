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

import asyncio
from unittest import IsolatedAsyncioTestCase

from thrift.python.client.py_bridge.py_bridge_channel import get_bridged_async_client
from thrift.python.serializer import Protocol, serialize
from thrift.python.test.thrift_clients import EchoService, TestService
from thrift.python.test.thrift_types import _fbthrift_EchoService_echo_result


# Test-only Compact message-envelope helpers (production enveloping is C++).
def _write_varint32(buf: bytearray, value: int) -> None:
    # 32-bit varint: the mask is intentional (seqid / name length only).
    value &= 0xFFFFFFFF
    while True:
        byte = value & 0x7F
        value >>= 7
        buf.append(byte | 0x80 if value else byte)
        if not value:
            return


def _read_varint(data: bytes, pos: int) -> tuple[int, int]:
    result = 0
    shift = 0
    while True:
        byte = data[pos]
        pos += 1
        result |= (byte & 0x7F) << shift
        if not (byte & 0x80):
            return result, pos
        shift += 7


def _compact_reply(method: str, body: bytes) -> bytes:
    # 0x82 proto id; version (fbthrift VERSION_N = 2) | (REPLY type << 5).
    out = bytearray([0x82, 0x02 | (2 << 5)])
    _write_varint32(out, 0)  # seqid
    name = method.encode("utf-8")
    _write_varint32(out, len(name))
    out += name
    out += body
    return bytes(out)


def _compact_method(data: bytes) -> str:
    # Mirror of the Compact CALL envelope `LegacySerializedRequest` writes:
    # [0]=proto id, [1]=version|type, then varint seqid, varint name length,
    # name bytes. Start at byte 2 (after proto id + version|type byte).
    _seqid, pos = _read_varint(data, 2)
    name_len, pos = _read_varint(data, pos)
    return data[pos : pos + name_len].decode("utf-8")


class _CannedHandler:
    """A ChannelHandler that records requests and returns a fixed reply."""

    def __init__(self, reply: bytes = b"") -> None:
        self.reply = reply
        self.requests: list[tuple[bytes, int]] = []
        self.called: asyncio.Event = asyncio.Event()

    async def send_request(self, enveloped_request: bytes, rpc_kind: int) -> bytes:
        self.requests.append((enveloped_request, rpc_kind))
        self.called.set()
        return self.reply


class PyBridgeChannelTest(IsolatedAsyncioTestCase):
    async def test_unary_typed_roundtrip(self) -> None:
        body = serialize(
            _fbthrift_EchoService_echo_result(success="HELLO"),
            protocol=Protocol.COMPACT,
        )
        handler = _CannedHandler(_compact_reply("echo", body))
        async with get_bridged_async_client(EchoService, handler) as client:
            result = await client.echo("hi")
        self.assertEqual(result, "HELLO")
        # The C++ channel enveloped the request as a CALL carrying the method.
        self.assertEqual(_compact_method(handler.requests[0][0]), "echo")

    async def test_oneway_fire_and_forget(self) -> None:
        handler = _CannedHandler()
        async with get_bridged_async_client(TestService, handler) as client:
            await client.oneway()
            # oneway returns before the fire-and-forget send task runs; wait for
            # the handler to actually record the request rather than guessing at
            # how many event-loop turns the chain takes.
            await asyncio.wait_for(handler.called.wait(), timeout=5)
        self.assertEqual(len(handler.requests), 1)
        self.assertEqual(_compact_method(handler.requests[0][0]), "oneway")
