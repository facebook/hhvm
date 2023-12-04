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
import sys
from argparse import ArgumentParser

from apache.thrift.test.asyncio.asyncio_load_handler import LoadHandler
from thrift.server.TAsyncioServer import ThriftAsyncServerFactory


def main():
    parser = ArgumentParser()
    parser.add_argument("--port", default=1234, type=int, help="Port to run on")
    options = parser.parse_args()
    loop = asyncio.get_event_loop()
    handler = LoadHandler()
    server = loop.run_until_complete(
        ThriftAsyncServerFactory(handler, port=options.port, loop=loop)
    )
    print("Running Asyncio server on port {}".format(options.port))

    try:
        loop.run_forever()
    except KeyboardInterrupt:
        print("Caught SIGINT, exiting")
    finally:
        server.close()
        loop.close()


def invoke_main() -> None:
    sys.exit(main())


if __name__ == "__main__":
    invoke_main()  # pragma: no cover
