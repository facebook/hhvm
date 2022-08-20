#!/usr/bin/env python3
# Copyright (c) Facebook, Inc. and its affiliates.
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
import pickle
import time
from concurrent.futures import ProcessPoolExecutor

from apache.thrift.test.asyncio.load import LoadTest
from apache.thrift.test.asyncio.load.ttypes import LoadError


def us_to_sec(microseconds):
    return microseconds / 1000000


def burn_in_executor(us):
    """
    The "busy wait" functions are CPU bound; to prevent blocking
    they need to be run in an executor. This function
    provides the implementation.
    """
    start = time.time()
    end = start + us_to_sec(us)
    while time.time() < end:
        pass


class LoadHandler(LoadTest.Iface):
    def __init__(self, loop=None):
        super().__init__()
        self.loop = loop or asyncio.get_event_loop()
        self.pool = ProcessPoolExecutor()
        pickle.DEFAULT_PROTOCOL = pickle.HIGHEST_PROTOCOL

    def noop(self):
        pass

    def onewayNoop(self):
        pass

    def asyncNoop(self):
        pass

    async def sleep(self, us):
        await asyncio.sleep(us_to_sec(us))

    async def onewaySleep(self, us):
        await asyncio.sleep(us_to_sec(us))

    async def burn(self, us):
        return await self.loop.run_in_executor(self.pool, burn_in_executor, us)

    async def onewayBurn(self, us):
        return await self.loop.run_in_executor(self.pool, burn_in_executor, us)

    def badSleep(self, us):
        # "bad" because it sleeps on the main thread
        # tests how the infrastructure responds to poorly
        # written client requests
        time.sleep(us_to_sec(us))

    def badBurn(self, us):
        return burn_in_executor(us)

    def throwError(self, code):
        raise LoadError(code=code)

    def throwUnexpected(self, code):
        raise LoadError(code=code)

    def send(self, data):
        pass

    def onewaySend(self, data):
        pass

    def recv(self, bytes):
        return "a" * bytes

    def sendrecv(self, data, recvBytes):
        return "a" * recvBytes

    def echo(self, data):
        return data

    def add(self, a, b):
        return a + b

    def largeContainer(self, data):
        pass

    async def iterAllFields(self, data):
        for item in data:
            _ = item.stringField
            for _ in item.stringList:
                pass
        return data
