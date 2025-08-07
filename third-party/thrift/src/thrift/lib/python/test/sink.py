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

from typing import AsyncGenerator
from unittest import IsolatedAsyncioTestCase

from thrift.python.streaming.sink import ClientSink


class SinkTests(IsolatedAsyncioTestCase):
    def test_create_client_sink(self) -> None:
        sink = ClientSink()
        self.assertIsNotNone(sink)

    async def test_create_async_generator(self) -> None:
        async def iter_alphabet() -> AsyncGenerator[str, None]:
            for c in "abcdefghijklmnopqrstuvwxyz":
                yield c

        sink = ClientSink()
        # we can't test anything meaningful with the inner cpp ClientSink
        # default initialized and disconnected from a RequestChannel.
        with self.assertRaisesRegex(
            AttributeError, "has no attribute '_fbthrift__sink_elem_handler'"
        ):
            await sink.sink(iter_alphabet())
