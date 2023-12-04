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

from thrift.conformance.rpc.thrift_services import RPCConformanceServiceInterface
from thrift.conformance.rpc.thrift_types import (
    Request,
    RequestResponseBasicServerTestResult,
    RequestResponseDeclaredExceptionServerTestResult,
    RequestResponseNoArgVoidResponseServerTestResult,
    RequestResponseUndeclaredExceptionServerTestResult,
    Response,
    RpcTestCase,
    ServerTestResult,
    UserException,
)
from thrift.python.server import ThriftServer


class Handler(RPCConformanceServiceInterface):
    def __init__(self) -> None:
        self.instruction = None
        self.result = None

    async def sendTestCase(self, testCase: RpcTestCase) -> None:
        self.instruction = testCase.serverInstruction

    async def getTestResult(self) -> ServerTestResult:
        return self.result

    async def requestResponseBasic(self, req: Request) -> Response:
        self.result = ServerTestResult.fromValue(
            RequestResponseBasicServerTestResult(request=req)
        )
        return self.instruction.requestResponseBasic.response

    async def requestResponseDeclaredException(self, req: Request) -> None:
        self.result = ServerTestResult.fromValue(
            RequestResponseDeclaredExceptionServerTestResult(request=req)
        )
        raise UserException(
            msg=self.instruction.requestResponseDeclaredException.userException.msg
        )

    async def requestResponseUndeclaredException(self, req: Request) -> None:
        self.result = ServerTestResult.fromValue(
            RequestResponseUndeclaredExceptionServerTestResult(request=req)
        )
        raise Exception(
            self.instruction.requestResponseUndeclaredException.exceptionMessage
        )

    async def requestResponseNoArgVoidResponse(self) -> None:
        self.result = ServerTestResult.fromValue(
            RequestResponseNoArgVoidResponseServerTestResult()
        )


async def main():
    loop = asyncio.get_event_loop()
    server = ThriftServer(Handler())
    serve_task = asyncio.create_task(server.serve())
    for signal in [SIGINT, SIGTERM]:
        loop.add_signal_handler(signal, server.stop)
    addr = await server.get_address()
    print(f"{addr.port}\n", flush=True)
    await serve_task


def invoke_main() -> None:
    asyncio.run(main())


if __name__ == "__main__":
    invoke_main()  # pragma: no cover
