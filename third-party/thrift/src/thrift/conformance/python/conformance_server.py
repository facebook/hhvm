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

from thrift.conformance.conformance.thrift_services import ConformanceServiceInterface
from thrift.conformance.patch_data.thrift_types import PatchOpRequest, PatchOpResponse
from thrift.conformance.serialization.thrift_types import (
    RoundTripRequest,
    RoundTripResponse,
)
from thrift.conformance.test_suite.thrift_types import TestCase
from thrift.python.conformance.omni_registry import OmniAnyRegistry
from thrift.python.server import ThriftServer


class Handler(ConformanceServiceInterface):
    async def roundTrip(self, request: RoundTripRequest) -> RoundTripResponse:
        registry = OmniAnyRegistry()
        obj = registry.load(request.value)
        protocol = request.targetProtocol
        if protocol is None:
            protocol = request.value.protocol
        return RoundTripResponse(value=registry.store(obj, protocol))

    async def patch(self, request: PatchOpRequest) -> PatchOpResponse:
        raise NotImplementedError

    async def sendTestCase(self, testCase: TestCase):
        pass


async def main():
    # Ensure OmniAnyRegistry is initialized before we start ThriftServer
    OmniAnyRegistry()

    server = ThriftServer(Handler())
    serve_task = asyncio.create_task(server.serve())
    addr = await server.get_address()
    print(addr.port, flush=True)
    print("\n", flush=True)
    try:
        await serve_task
    finally:
        server.stop()


def invoke_main() -> None:
    asyncio.get_event_loop().run_until_complete(main())


if __name__ == "__main__":
    invoke_main()  # pragma: no cover
