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
from signal import SIGINT, SIGTERM

import click
from example.chatroom.thrift_services import EchoInterface
from thrift.python.server import ThriftServer


class EchoHandler(EchoInterface):
    async def echo(self, message: str):
        return message


async def async_main(port):
    loop = asyncio.get_event_loop()
    server = ThriftServer(EchoHandler(), port=port)
    serve_task = loop.create_task(server.serve())
    for signal in [SIGINT, SIGTERM]:
        loop.add_signal_handler(signal, server.stop)
    addr = await server.get_address()
    print(f"Listening on {addr}")
    await serve_task


@click.command()
@click.option("--port", default=7778, help="Listening port of remote thrift server")
def main(port):
    # TODO: due to some bug, using asyncio.run() will cause process to stuck on exit, to be fixed
    asyncio.get_event_loop().run_until_complete(async_main(port))


if __name__ == "__main__":
    main()
