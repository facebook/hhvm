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

import asyncio
import functools
import logging
import time
import unittest

from thrift.server.TAsyncioServer import (
    ThriftAsyncServerFactory,
    ThriftClientProtocolFactory,
)
from ThriftTest import ThriftTest
from ThriftTest.ttypes import Xception, Xtruct

loop = asyncio.get_event_loop()
loop.set_debug(True)
logging.getLogger("asyncio").setLevel(logging.DEBUG)


class TestHandler(ThriftTest.Iface):
    def __init__(self):
        self.onewaysQueue = asyncio.Queue()

    async def testVoid(self):
        pass

    async def testString(self, s):
        await asyncio.sleep(0)
        return s

    async def testByte(self, b):
        return b

    async def testI16(self, i16):
        return i16

    async def testI32(self, i32):
        return i32

    async def testI64(self, i64):
        return i64

    async def testDouble(self, dub):
        return dub

    async def testStruct(self, thing):
        return thing

    async def testException(self, s):
        if s == "Xception":
            x = Xception()
            x.errorCode = 1001
            x.message = s
            raise x
        elif s == "throw_undeclared":
            raise ValueError("foo")

    async def testOneway(self, seconds):
        t = time.time()
        await asyncio.sleep(seconds)
        await self.onewaysQueue.put((t, time.time(), seconds))

    async def testNest(self, thing):
        return thing

    async def testMap(self, thing):
        return thing

    async def testSet(self, thing):
        return thing

    async def testList(self, thing):
        return thing

    async def testEnum(self, thing):
        return thing

    async def testTypedef(self, thing):
        return thing


def async_test(f):
    @functools.wraps(f)
    def wrapper(*args, **kwargs):
        loop.run_until_complete(f(*args, **kwargs))

    return wrapper


class ThriftAsyncTestCase(unittest.TestCase):
    CLIENT_TYPE = None

    @async_test
    async def setUp(self):
        global loop
        self.host = "127.0.0.1"
        self.handler = TestHandler()
        self.server = await ThriftAsyncServerFactory(
            self.handler,
            interface=self.host,
            port=0,
            loop=loop,
        )
        self.port = self.server.sockets[0].getsockname()[1]
        self.transport, self.protocol = await loop.create_connection(
            ThriftClientProtocolFactory(
                ThriftTest.Client, client_type=self.CLIENT_TYPE
            ),
            host=self.host,
            port=self.port,
        )
        self.client = self.protocol.client

    @async_test
    async def tearDown(self):
        self.protocol.close()
        self.transport.close()
        self.server.close()

    @async_test
    async def testVoid(self):
        result = await self.client.testVoid()
        self.assertEqual(result, None)

    @async_test
    async def testString(self):
        result = await self.client.testString("Python")
        self.assertEqual(result, "Python")

    @async_test
    async def testByte(self):
        result = await self.client.testByte(63)
        self.assertEqual(result, 63)

    @async_test
    async def testI32(self):
        result = await self.client.testI32(-1)
        self.assertEqual(result, -1)
        result = await self.client.testI32(0)
        self.assertEqual(result, 0)

    @async_test
    async def testI64(self):
        result = await self.client.testI64(-34359738368)
        self.assertEqual(result, -34359738368)

    @async_test
    async def testDouble(self):
        result = await self.client.testDouble(-5.235098235)
        self.assertAlmostEqual(result, -5.235098235)

    @async_test
    async def testStruct(self):
        x = Xtruct()
        x.string_thing = "Zero"
        x.byte_thing = 1
        x.i32_thing = -3
        x.i64_thing = -5
        y = await self.client.testStruct(x)

        self.assertEqual(y.string_thing, "Zero")
        self.assertEqual(y.byte_thing, 1)
        self.assertEqual(y.i32_thing, -3)
        self.assertEqual(y.i64_thing, -5)

    @async_test
    async def testException(self):
        await self.client.testException("Safe")
        try:
            await self.client.testException("Xception")
            self.fail("Xception not raised")
        except Xception as x:
            self.assertEqual(x.errorCode, 1001)
            self.assertEqual(x.message, "Xception")  # noqa

        try:
            await self.client.testException("throw_undeclared")
            self.fail("exception not raised")
        except Exception:  # type is undefined
            pass

    @async_test
    async def testOneway(self):
        await self.client.testOneway(2)
        start, end, seconds = await self.handler.onewaysQueue.get()
        self.assertAlmostEqual(seconds, (end - start), places=1)
