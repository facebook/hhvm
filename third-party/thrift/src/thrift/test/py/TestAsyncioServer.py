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

# pyre-unsafe

from unittest.mock import MagicMock

from libfb.py.asyncio.unittest import TestCase
from thrift.server.TAsyncioServer import ThriftAsyncServerFactory
from thrift.Thrift import TProcessor


class TestAsyncioServer(TestCase):
    async def test_factory(self):
        async def wrap(value):
            return value

        protocol_factory = MagicMock()
        loop = MagicMock()
        processor = MagicMock(spec=TProcessor)
        event_handler = MagicMock()
        server = MagicMock()
        sock = MagicMock()
        sock.getsockname.return_value = "foosock"
        server.sockets = [sock]
        loop.create_server.return_value = wrap(server)

        await ThriftAsyncServerFactory(
            processor,
            loop=loop,
            event_handler=event_handler,
            protocol_factory=protocol_factory,
        )

        event_handler.preServe.assert_called_with("foosock")
