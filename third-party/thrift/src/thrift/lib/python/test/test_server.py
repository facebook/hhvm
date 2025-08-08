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

from pathlib import Path

from thrift.py3.server import SocketAddress
from thrift.python.server import ServiceInterface, ThriftServer


class TestServer:
    server: ThriftServer
    serve_task: asyncio.Task | None

    def __init__(
        self,
        handler: ServiceInterface,
        ip: str | None = None,
        path: Path | None = None,
    ) -> None:
        self.server = ThriftServer(handler, ip=ip, path=path)
        self.serve_task = None

    async def __aenter__(self) -> SocketAddress:
        self.serve_task = asyncio.get_event_loop().create_task(self.server.serve())
        return await self.server.get_address()

    async def __aexit__(self, *_exc_info: object) -> None:
        self.server.stop()
        assert self.serve_task is not None
        await self.serve_task
