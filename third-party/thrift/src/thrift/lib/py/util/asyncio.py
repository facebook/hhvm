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

from thrift.server.TAsyncioServer import ThriftClientProtocolFactory
from thrift.util.Decorators import protocol_manager


class async_protocol_manager:
    def __init__(self, coro):
        """
        Given a coro from create_connection create a context manager
        around the protocol returned
        """
        self.coro = coro

    def __await__(self):
        async def as_protocol_manager():
            _, protocol = await self.coro
            return protocol_manager(protocol)

        return as_protocol_manager().__await__()

    __iter__ = __await__

    async def __aenter__(self):
        _, self.protocol = await self.coro
        return self.protocol.client

    async def __aexit__(self, exc_type, exc, tb):
        self.protocol.close()


def create_client(
    client_klass,
    *,
    host=None,
    port=None,
    sock=None,
    loop=None,
    timeouts=None,
    client_type=None,
    ssl=None,
):
    """
    create an asyncio thrift client and return an async context
    manager that can be used as follows:

    async with create_client(smc2_client, port=1421) as smc:
        await smc.getStatus()

    This can be used in the old way:

    with (await create_client(smc2_client, port=1421)) as smc:
        await smc.getStatus()

    or even the old deprecated way:

    with (yield from create_client(smc2_client, port=1421) as smc:
        yield from smc.getStatus()

    :param client_klass: thrift Client class
    :param host: hostname/ip, None = loopback
    :param port: port number
    :param sock: socket.socket object
    :param loop: asyncio event loop
    :returns: an Async Context Manager
    """
    if not loop:
        loop = asyncio.get_event_loop()

    coro = loop.create_connection(
        ThriftClientProtocolFactory(
            client_klass,
            loop=loop,
            timeouts=timeouts,
            client_type=client_type,
        ),
        host=host,
        port=port,
        sock=sock,
        ssl=ssl,
    )
    return async_protocol_manager(coro)


def call_as_future(f, loop, *args, **kwargs):
    """call_as_future(callable, *args, **kwargs) -> asyncio.Task

    Like asyncio.ensure_future() but takes any callable and converts
    it to a coroutine function first.
    """
    if not asyncio.iscoroutinefunction(f):
        f = asyncio.coroutine(f)

    return asyncio.ensure_future(f(*args, **kwargs), loop=loop)
