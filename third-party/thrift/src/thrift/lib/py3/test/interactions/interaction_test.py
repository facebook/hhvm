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

import asyncio
import unittest

from blank_interaction.services import BlankServiceInterface
from interaction.clients import Calculator
from interaction.types import Point
from thrift.py3.client import ClientType, get_client

from .run_interaction import run_interaction


class InteractionTest(unittest.TestCase):
    def setUp(self) -> None:
        self.interaction = run_interaction()

    def init_client(self) -> Calculator:
        return get_client(
            Calculator,
            port=self.interaction.getPort(),
            host="::1",
            client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
        )

    def tearDown(self) -> None:
        self.interaction.reset()

    def test_basic(self) -> None:
        async def inner_test() -> None:
            async with self.init_client() as calc:
                self.assertEqual(await calc.addPrimitive(0, 0), 0)
                async with calc.createAddition() as add:
                    self.assertEqual(await add.getPrimitive(), 0)

                    await add.accumulatePrimitive(1)
                    self.assertEqual(await add.getPrimitive(), 1)

                    point = await add.getPoint()
                    self.assertEqual(point.x, 0)
                    self.assertEqual(point.y, 0)

                    newPoint = Point(x=2, y=3)
                    await add.accumulatePoint(newPoint)

                    point = await add.getPoint()
                    self.assertEqual(point.x, 2)
                    self.assertEqual(point.y, 3)

                    await add.noop()

        asyncio.run(inner_test())

    def test_multiple_interactions(self) -> None:
        async def inner_test() -> None:
            async with self.init_client() as calc:
                self.assertEqual(await calc.addPrimitive(0, 0), 0)
                async with calc.createAddition() as add:
                    self.assertEqual(await add.getPrimitive(), 0)

                    await add.accumulatePrimitive(1)
                    self.assertEqual(await add.getPrimitive(), 1)

                async with calc.createAddition() as add:
                    self.assertEqual(await add.getPrimitive(), 0)

                    await add.accumulatePrimitive(2)
                    self.assertEqual(await add.getPrimitive(), 2)

        asyncio.run(inner_test())

    def test_multiple_clients(self) -> None:
        async def inner_test() -> None:
            async with self.init_client() as calc:
                self.assertEqual(await calc.addPrimitive(0, 0), 0)
                async with calc.createAddition() as add:
                    self.assertEqual(await add.getPrimitive(), 0)

                    await add.accumulatePrimitive(1)
                    self.assertEqual(await add.getPrimitive(), 1)

            async with self.init_client() as calc:
                self.assertEqual(await calc.addPrimitive(0, 1), 1)
                async with calc.createAddition() as add:
                    self.assertEqual(await add.getPrimitive(), 0)

                    await add.accumulatePrimitive(2)
                    self.assertEqual(await add.getPrimitive(), 2)

        asyncio.run(inner_test())

    def test_terminate_unused(self) -> None:
        async def inner_test() -> None:
            async with self.init_client() as calc:
                async with calc.createAddition() as _:
                    pass

        asyncio.run(inner_test())

    def test_terminate_client_error(self) -> None:
        class SpecificError(Exception):
            pass

        async def inner_test() -> None:
            try:
                async with self.init_client() as calc:
                    self.assertEqual(await calc.addPrimitive(0, 0), 0)
                    async with calc.createAddition() as add:
                        await add.accumulatePrimitive(1)
                        raise SpecificError("Generic error")
            except SpecificError:
                pass
            else:
                self.fail("Didn't throw SpecificError")

        asyncio.run(inner_test())
