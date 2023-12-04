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
import signal
import sys
from argparse import ArgumentParser

from apache.thrift.test.py3.load_handler import LoadTestHandler
from thrift.py3 import ThriftServer


def main():
    parser = ArgumentParser()
    parser.add_argument("--port", default=1234, type=int, help="Port to run on")
    options = parser.parse_args()
    loop = asyncio.get_event_loop()
    handler = LoadTestHandler(loop)
    server = ThriftServer(handler, options.port)
    loop.add_signal_handler(signal.SIGINT, server.stop)
    loop.add_signal_handler(signal.SIGTERM, server.stop)
    print("Running Py3 server on port {}".format(options.port))
    loop.run_until_complete(server.serve())


def invoke_main() -> None:
    sys.exit(main())


if __name__ == "__main__":
    invoke_main()  # pragma: no cover
