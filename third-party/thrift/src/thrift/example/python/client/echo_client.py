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

import click
from example.chatroom.thrift_clients import Echo
from thrift.python.client import get_client


async def async_main(host, port):
    async with get_client(Echo, host=host, port=port) as client:
        while True:
            text = input("> ")
            resp = await client.echo(text)
            print(f"< {resp}")


@click.command()
@click.option("--host", default="localhost", help="Hostname of remote thrift server")
@click.option("--port", default=7777, help="Listening port of remote thrift server")
def main(*args, **kwargs):
    asyncio.run(async_main(*args, **kwargs))


if __name__ == "__main__":
    main()
