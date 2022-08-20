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

from thrift.perf.load.services import LoadTestInterface
from thrift.perf.load.types import LoadError


def us_to_sec(microseconds):
    return microseconds / 1000000


def burn_in_executor(us):
    start = time.time()
    end = start + us_to_sec(us)
    while time.time() < end:
        pass


class LoadTestHandler(LoadTestInterface):
    def __init__(self, loop=None):
        super().__init__()
        self.loop = loop or asyncio.get_event_loop()
        self.pool = ProcessPoolExecutor()
        pickle.DEFAULT_PROTOCOL = pickle.HIGHEST_PROTOCOL

    async def noop(self):
        pass

    async def onewayNoop(self):
        pass

    async def asyncNoop(self):
        pass

    async def sleep(self, us):
        await asyncio.sleep(us_to_sec(us))

    async def onewaySleep(self, us):
        await asyncio.sleep(us_to_sec(us))

    async def burn(self, us):
        return await self.loop.run_in_executor(self.pool, burn_in_executor, us)

    async def onewayBurn(self, us):
        return await self.loop.run_in_executor(self.pool, burn_in_executor, us)

    async def badSleep(self, us):
        # "bad" because it sleeps on the main thread
        time.sleep(us_to_sec(us))

    async def badBurn(self, us):
        return burn_in_executor(us)

    async def throwError(self, code):
        raise LoadError(code=code)

    async def throwUnexpected(self, code):
        raise LoadError(code=code)

    async def send(self, data):
        pass

    async def onewaySend(self, data):
        pass

    async def recv(self, bytes):
        return "a" * bytes

    async def sendrecv(self, data, recvBytes):
        return "a" * recvBytes

    async def echo(self, data):
        return data

    async def add(self, a, b):
        return a + b

    async def largeContainer(self, data):
        pass

    async def iterAllFields(self, data):
        for item in data:
            _ = item.stringField
            for _ in item.stringList:
                pass
        return data
