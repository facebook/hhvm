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

from __future__ import absolute_import, division, print_function, unicode_literals

import asyncio

from thrift_asyncio.sleep import Sleep as AsyncSleep
from thrift_asyncio.sleep.ttypes import OverflowResult
from thrift_asyncio.tutorial import Calculator as AsyncCalculator


class AsyncCalculatorHandler(AsyncCalculator.Iface):
    async def add(self, n1, n2):
        return 42

    async def calculate(self, logid, work):
        return 0

    async def zip(self):
        print("zip")


class AsyncSleepHandler(AsyncSleep.Iface):
    def __init__(self, loop):
        self._loop = loop

    async def echo(self, message, delay):
        return await asyncio.sleep(delay, result=message)

    async def overflow(self, value):
        # simply return the value in OverflowResult
        return OverflowResult(value=value)
