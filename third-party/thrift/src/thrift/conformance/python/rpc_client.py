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

from thrift.conformance.rpc.thrift_clients import RPCConformanceService
from thrift.conformance.rpc.thrift_types import (
    ClientInstruction,
    ClientTestResult,
    RequestResponseBasicClientTestResult,
    RequestResponseDeclaredExceptionClientTestResult,
    RequestResponseNoArgVoidResponseClientTestResult,
    RequestResponseTimeoutClientTestResult,
    RequestResponseUndeclaredExceptionClientTestResult,
    RpcTestCase,
    UserException,
)
from thrift.python.client import get_client
from thrift.python.exceptions import (
    ApplicationError,
    TransportError,
    TransportErrorType,
)


class RpcTestClient:
    __HOST_ADDRESS = "::1"

    @staticmethod
    def __get_client_instruction(test: RpcTestCase) -> ClientInstruction:
        if test is None:
            raise RuntimeError("Null test case")
        instruction = test.clientInstruction
        if instruction is None:
            raise RuntimeError("Null client intstruction")
        return instruction

    def __init__(self):
        self.dispatch_table = {
            ClientInstruction.Type.requestResponseBasic: self.__request_response_basic_test,
            ClientInstruction.Type.requestResponseDeclaredException: self.__request_response_declared_exception_test,
            ClientInstruction.Type.requestResponseUndeclaredException: self.__request_response_undeclared_exception_test,
            ClientInstruction.Type.requestResponseNoArgVoidResponse: self.__request_response_noarg_void_response_test,
            ClientInstruction.Type.requestResponseTimeout: self.__request_response_timeout_test,
        }

    async def __request_response_basic_test(
        self, client, instruction: ClientInstruction
    ) -> None:
        response = await client.requestResponseBasic(
            req=instruction.requestResponseBasic.request
        )
        clientTestResult = ClientTestResult.fromValue(
            RequestResponseBasicClientTestResult(response=response)
        )
        await client.sendTestResult(result=clientTestResult)

    async def __request_response_declared_exception_test(
        self, client, instruction: ClientInstruction
    ) -> None:
        try:
            await client.requestResponseDeclaredException(
                req=instruction.requestResponseDeclaredException.request
            )
        except UserException as e:
            clientTestResult = ClientTestResult.fromValue(
                RequestResponseDeclaredExceptionClientTestResult(userException=e)
            )
            await client.sendTestResult(result=clientTestResult)

    async def __request_response_undeclared_exception_test(
        self, client, instruction: ClientInstruction
    ) -> None:
        try:
            await client.requestResponseUndeclaredException(
                req=instruction.requestResponseUndeclaredException.request
            )
        except ApplicationError as e:
            clientTestResult = ClientTestResult.fromValue(
                RequestResponseUndeclaredExceptionClientTestResult(
                    exceptionMessage=str(e)
                )
            )
            await client.sendTestResult(result=clientTestResult)

    async def __request_response_noarg_void_response_test(
        self, client, instruction: ClientInstruction
    ) -> None:
        await client.requestResponseNoArgVoidResponse()
        clientTestResult = ClientTestResult.fromValue(
            RequestResponseNoArgVoidResponseClientTestResult()
        )
        await client.sendTestResult(result=clientTestResult)

    async def __request_response_timeout_test(
        self, client, instruction: ClientInstruction
    ) -> None:
        receivedTimeoutException = False
        try:
            # TO-DO: T132887459 implement timeout in rpc options
            await asyncio.wait_for(
                client.requestResponseTimeout(
                    req=instruction.requestResponseTimeout.request
                ),
                timeout=float(instruction.requestResponseTimeout.timeoutMs / 1000.0),
            )
        except asyncio.TimeoutError:
            # T132887459: this time out is just needed to complete await
            receivedTimeoutException = False
        except TransportError as error:
            if error.type == TransportErrorType.TIMED_OUT:
                receivedTimeoutException = True
        clientTestResult = ClientTestResult.fromValue(
            RequestResponseTimeoutClientTestResult(
                timeoutException=receivedTimeoutException
            )
        )
        await client.sendTestResult(result=clientTestResult)

    async def run_test(self, port: int) -> None:
        async with get_client(
            RPCConformanceService, host=self.__HOST_ADDRESS, port=port
        ) as client:
            test = await client.getTestCase()
            instruction = RpcTestClient.__get_client_instruction(test)
            await self.dispatch_table[instruction.get_type()](client, instruction)


def get_port() -> int:
    idx = 0
    for arg in sys.argv:
        if arg == "--port":
            break
        idx = idx + 1
    if idx >= len(sys.argv) - 1:
        raise RuntimeError("Port argument is not provided")
    port = int(sys.argv[idx + 1])
    print(f"Port={port}")
    return port


async def main():
    port = get_port()
    testClient = RpcTestClient()
    await testClient.run_test(port)


if __name__ == "__main__":
    asyncio.run(main())
