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
    StreamBasicClientTestResult,
    StreamChunkTimeoutClientTestResult,
    StreamCreditTimeoutClientTestResult,
    StreamInitialResponseClientTestResult,
    UserException,
)
from thrift.python.client import ClientType, get_client
from thrift.python.exceptions import (
    ApplicationError,
    ApplicationErrorType,
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
            ClientInstruction.Type.streamBasic: self.__stream_basic_test,
            ClientInstruction.Type.streamInitialResponse: self.__stream_initial_response_test,
            ClientInstruction.Type.streamChunkTimeout: self.__stream_chunk_timeout_test,
            ClientInstruction.Type.streamCreditTimeout: self.__stream_credit_timeout_test,
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
            # TODO: T132887459 timeout should be passed via rpc options
            await asyncio.wait_for(
                client.requestResponseTimeout(
                    req=instruction.requestResponseTimeout.request
                ),
                timeout=float(instruction.requestResponseTimeout.timeoutMs / 1000.0),
            )
        except asyncio.TimeoutError:
            # TODO: T132887459 specifically marking it as False because as per
            # conformance design we are supposed to catch TransportError.
            # Catch below is a valid branch. Revise when rpc options support is ready
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

    async def __stream_basic_test(self, client, instruction: ClientInstruction) -> None:
        # TODO: T132887459 buffer size should be passed via rpc options
        streamResponse = await client.streamBasic(req=instruction.streamBasic.request)

        payload = []
        async for chunk in streamResponse:
            payload.append(chunk)

        clientTestResult = ClientTestResult.fromValue(
            StreamBasicClientTestResult(streamPayloads=payload)
        )
        await client.sendTestResult(result=clientTestResult)

    async def __stream_initial_response_test(
        self, client, instruction: ClientInstruction
    ) -> None:
        response, streamResponse = await client.streamInitialResponse(
            req=instruction.streamInitialResponse.request
        )

        payload = []
        async for chunk in streamResponse:
            payload.append(chunk)

        clientTestResult = ClientTestResult.fromValue(
            StreamInitialResponseClientTestResult(
                streamPayloads=payload, initialResponse=response
            )
        )
        await client.sendTestResult(result=clientTestResult)

    async def __stream_chunk_timeout_test(
        self, client, instruction: ClientInstruction
    ) -> None:
        # TODO: T132887459 timeout should be passed via rpc options
        receivedTimeoutException = False
        payload = []
        try:
            streamResponse = await asyncio.wait_for(
                client.streamChunkTimeout(req=instruction.streamChunkTimeout.request),
                timeout=float(instruction.streamChunkTimeout.chunkTimeoutMs / 1000.0),
            )
            async for chunk in streamResponse:
                payload.append(chunk)
        except asyncio.TimeoutError:
            # TODO: T132887459 specifically marking it as False because as per
            # conformance design we are supposed to catch TransportError.
            # Catch below is a valid branch. Revise when rpc options support is ready
            receivedTimeoutException = False
        except TransportError as error:
            if error.type == TransportErrorType.TIMED_OUT:
                receivedTimeoutException = True

        clientTestResult = ClientTestResult.fromValue(
            StreamChunkTimeoutClientTestResult(
                streamPayloads=payload, chunkTimeoutException=receivedTimeoutException
            )
        )
        await client.sendTestResult(result=clientTestResult)

    async def __stream_credit_timeout_test(
        self, client, instruction: ClientInstruction
    ) -> None:
        receivedTimeoutException = False
        streamResponse = await client.streamCreditTimeout(
            req=instruction.streamCreditTimeout.request
        )
        # TODO: T132887459. This test is failing for some reason.
        # maybe try to specify chunkbuffersize=0 (as in fbcode/thrift/conformance/cpp2/RPCClient.cpp)
        # but that has to be done via rpc options
        try:
            await streamResponse.__anext__()
            # Sleep longer than the stream expiration time so that the server
            # will run out of credit and throw a credit timeout exception
            await asyncio.sleep(
                float(instruction.streamCreditTimeout.creditTimeoutMs / 1000.0)
            )
            await streamResponse.__anext__()
            raise RuntimeError(
                "Should not get here. Should catch server's ApplicationError instead"
            )
        except ApplicationError as error:
            if error.type == ApplicationErrorType.TIMED_OUT:
                receivedTimeoutException = True

        clientTestResult = ClientTestResult.fromValue(
            StreamCreditTimeoutClientTestResult(
                creditTimeoutException=receivedTimeoutException
            )
        )
        await client.sendTestResult(result=clientTestResult)

    async def run_test(self, port: int) -> None:
        async with get_client(
            RPCConformanceService,
            host=self.__HOST_ADDRESS,
            port=port,
            client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
        ) as client:
            test = await client.getTestCase()
            instruction = RpcTestClient.__get_client_instruction(test)
            await self.dispatch_table[instruction.get_type()](client, instruction)


@click.command()
@click.option("--port", default=7777, help="RPC conformance test server port")
def main(port):
    testClient = RpcTestClient()
    asyncio.run(testClient.run_test(port))


if __name__ == "__main__":
    main()
