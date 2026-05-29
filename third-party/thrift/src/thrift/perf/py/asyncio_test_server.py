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

import asyncio
import sys
from argparse import ArgumentParser

from thrift.perf.py.asyncio_load_handler import LoadHandler
from thrift.server.TAsyncioServer import ThriftAsyncServerFactory


async def _run_server(port: int) -> None:
    loop = asyncio.get_running_loop()
    handler = LoadHandler()
    server = await ThriftAsyncServerFactory(handler, port=port, loop=loop)
    print("Running Asyncio server on port {}".format(port))

    try:
        await loop.create_future()  # run forever
    except asyncio.CancelledError:
        pass
    finally:
        server.close()


def main():
    parser = ArgumentParser()
    parser.add_argument("--port", default=1234, type=int, help="Port to run on")
    options = parser.parse_args()
    try:
        asyncio.run(_run_server(options.port))
    except KeyboardInterrupt:
        print("Caught SIGINT, exiting")


def invoke_main() -> None:
    sys.exit(main())


if __name__ == "__main__":
    invoke_main()  # pragma: no cover
