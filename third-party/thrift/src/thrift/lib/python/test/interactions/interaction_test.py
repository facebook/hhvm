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
import math
import unittest
from typing import AsyncGenerator

from interaction.thrift_clients import Calculator
from interaction.thrift_types import Point
from thrift.python.client import ClientType, get_client

from .run_interaction import run_interaction


class SpecificError(Exception):
    pass


class InteractionTest(unittest.IsolatedAsyncioTestCase):
    def setUp(self) -> None:
        self.interaction = run_interaction()

    def init_client(self) -> Calculator.Async:
        return get_client(
            Calculator,
            port=self.interaction.getPort(),
            host="::1",
            client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
        )

    def tearDown(self) -> None:
        self.interaction.reset()

    async def test_basic(self) -> None:
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

    async def test_sink(self) -> None:
        async def num_gen(fr: int, to: int, stride: int) -> AsyncGenerator[int, None]:
            for i in range(fr, to, stride):
                await asyncio.sleep(0.1)
                yield i

        # let the Points represent velocity, then their
        # vector sum represents displacement if we ignore calculus
        async def point_gen(
            ax: int, ay: int, steps: int
        ) -> AsyncGenerator[Point, None]:
            step, vx, vy = 0, 0, 0
            while step < steps:
                vx += ax
                vy += ay
                await asyncio.sleep(0.1 / math.hypot(vx, vy))
                yield Point(x=vx, y=vy)

                step += 1

        async with self.init_client() as calc:
            async with calc.createAddition() as add:
                self.assertEqual(await add.getPrimitive(), 0)

                sink = await add.sinkPrimitive()
                fr, to, stride = 2, 9, 2
                final_resp = await sink.sink(num_gen(fr, to, stride))
                expected = sum(range(fr, to, stride))
                self.assertEqual(final_resp, expected)

                self.assertEqual(await add.getPrimitive(), expected)

                initial = await add.getPoint()
                self.assertEqual((0, 0), (initial.x, initial.y))
                sink = await add.sinkPoint()
                final_distance = await sink.sink(point_gen(1, 2, 5))

                final_pos = await add.getPoint()
                self.assertEqual((15, 30), (final_pos.x, final_pos.y))
                self.assertEqual(final_distance, math.hypot(final_pos.x, final_pos.y))

    async def test_multiple_interactions(self) -> None:
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

    async def test_multiple_clients(self) -> None:
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

    async def test_terminate_unused(self) -> None:
        async with self.init_client() as calc:
            async with calc.createAddition() as _:
                pass

    async def test_factory_basic(self) -> None:
        async with self.init_client() as calc:
            result = await calc.newAddition()
            async with result as add:
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

    async def test_factory_init(self) -> None:
        async with self.init_client() as calc:
            start_value = 5
            result, data = await calc.initializedAddition(start_value)
            self.assertEqual(data, start_value)
            async with result as add:
                self.assertEqual(await add.getPrimitive(), 5)

                await add.accumulatePrimitive(2)
                self.assertEqual(await add.getPrimitive(), 7)

                await add.noop()

    async def test_factory_stringify(self) -> None:
        async with self.init_client() as calc:
            start_value = 8
            result, data = await calc.stringifiedAddition(start_value)
            self.assertEqual(data, str(start_value))
            async with result as add:
                self.assertEqual(await add.getPrimitive(), 8)

                await add.accumulatePrimitive(5)
                self.assertEqual(await add.getPrimitive(), 13)

                await add.noop()

    async def test_terminate_client_error(self) -> None:
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
