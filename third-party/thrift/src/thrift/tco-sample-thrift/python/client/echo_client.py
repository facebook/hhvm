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
from thrift.python.client import ClientType, get_client


async def async_main(host, port, http):
    async with get_client(
        Echo,
        host=host,
        port=port,
        path=("/" if http else None),
        client_type=(
            ClientType.THRIFT_HTTP2_CLIENT_TYPE
            if http
            else ClientType.THRIFT_ROCKET_CLIENT_TYPE
        ),
    ) as client:
        while True:
            text = input("> ")
            resp = await client.echo(text)
            print(f"< {resp}")


@click.command()
@click.option("--host", default="localhost", help="Hostname of remote thrift server")
@click.option("--port", default=7778, help="Listening port of remote thrift server")
@click.option("--http/--no-http", default=False, help="Use HTTP2 client")
def main(*args, **kwargs):
    asyncio.run(async_main(*args, **kwargs))


if __name__ == "__main__":
    main()
