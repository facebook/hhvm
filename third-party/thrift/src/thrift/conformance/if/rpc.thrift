/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

include "thrift/annotation/thrift.thrift"

namespace cpp2 apache.thrift.conformance
namespace php apache_thrift
namespace py thrift.conformance.rpc
namespace py.asyncio thrift_asyncio.conformance.rpc
namespace py3 thrift.conformance
namespace java.swift org.apache.thrift.conformance

struct RpcTestCase {
  1: ClientInstruction clientInstruction;
  2: ClientTestResult clientTestResult;
  3: ServerInstruction serverInstruction;
  4: ServerTestResult serverTestResult;
}

struct Request {
  1: string data;
  2: optional i32 num;
}

struct Response {
  1: string data;
  2: optional i32 num;
}

exception UserException {
  1: string msg;
}

union ServerTestResult {
  1: RequestResponseBasicServerTestResult requestResponseBasic;
  2: RequestResponseDeclaredExceptionServerTestResult requestResponseDeclaredException;
  3: RequestResponseNoArgVoidResponseServerTestResult requestResponseNoArgVoidResponse;
  4: RequestResponseUndeclaredExceptionServerTestResult requestResponseUndeclaredException;
  100: StreamBasicServerTestResult streamBasic;
  101: StreamChunkTimeoutServerTestResult streamChunkTimeout;
  102: StreamInitialResponseServerTestResult streamInitialResponse;
  200: SinkBasicServerTestResult sinkBasic;
}

union ClientTestResult {
  1: RequestResponseBasicClientTestResult requestResponseBasic;
  2: RequestResponseDeclaredExceptionClientTestResult requestResponseDeclaredException;
  3: RequestResponseNoArgVoidResponseClientTestResult requestResponseNoArgVoidResponse;
  4: RequestResponseUndeclaredExceptionClientTestResult requestResponseUndeclaredException;
  100: StreamBasicClientTestResult streamBasic;
  101: StreamChunkTimeoutClientTestResult streamChunkTimeout;
  102: StreamInitialResponseClientTestResult streamInitialResponse;
  200: SinkBasicClientTestResult sinkBasic;
}

struct RequestResponseBasicServerTestResult {
  1: Request request;
}

struct RequestResponseDeclaredExceptionServerTestResult {
  1: Request request;
}

struct RequestResponseUndeclaredExceptionServerTestResult {
  1: Request request;
}

struct RequestResponseNoArgVoidResponseServerTestResult {}

struct StreamBasicServerTestResult {
  1: Request request;
}

struct StreamChunkTimeoutServerTestResult {
  1: Request request;
}

struct StreamInitialResponseServerTestResult {
  1: Request request;
}

struct SinkBasicServerTestResult {
  1: Request request;
  2: list<Request> sinkPayloads;
}

struct RequestResponseBasicClientTestResult {
  1: Response response;
}

struct RequestResponseNoArgVoidResponseClientTestResult {}

struct RequestResponseDeclaredExceptionClientTestResult {
  // TODO(dokwon): Remove @thrift.Box after fixing incomplete type bug.
  @thrift.Box
  1: optional UserException userException;
}

struct RequestResponseUndeclaredExceptionClientTestResult {
  1: string exceptionMessage;
}

struct StreamBasicClientTestResult {
  1: list<Response> streamPayloads;
}

struct StreamChunkTimeoutClientTestResult {
  1: list<Response> streamPayloads;
  2: bool chunkTimeoutException;
}

struct StreamInitialResponseClientTestResult {
  1: list<Response> streamPayloads;
  2: Response initialResponse;
}

struct SinkBasicClientTestResult {
  1: Response finalResponse;
}

union ClientInstruction {
  1: RequestResponseBasicClientInstruction requestResponseBasic;
  2: RequestResponseDeclaredExceptionClientInstruction requestResponseDeclaredException;
  3: RequestResponseNoArgVoidResponseClientInstruction requestResponseNoArgVoidResponse;
  4: RequestResponseUndeclaredExceptionClientInstruction requestResponseUndeclaredException;
  100: StreamBasicClientInstruction streamBasic;
  101: StreamChunkTimeoutClientInstruction streamChunkTimeout;
  102: StreamInitialResponseClientInstruction streamInitialResponse;
  200: SinkBasicClientInstruction sinkBasic;
}

union ServerInstruction {
  1: RequestResponseBasicServerInstruction requestResponseBasic;
  2: RequestResponseDeclaredExceptionServerInstruction requestResponseDeclaredException;
  3: RequestResponseNoArgVoidResponseServerInstruction requestResponseNoArgVoidResponse;
  4: RequestResponseUndeclaredExceptionServerInstruction requestResponseUndeclaredException;
  100: StreamBasicServerInstruction streamBasic;
  101: StreamChunkTimeoutServerInstruction streamChunkTimeout;
  102: StreamInitialResponseServerInstruction streamInitialResponse;
  200: SinkBasicServerInstruction sinkBasic;
}

struct RequestResponseBasicClientInstruction {
  1: Request request;
}

struct RequestResponseDeclaredExceptionClientInstruction {
  1: Request request;
}

struct RequestResponseUndeclaredExceptionClientInstruction {
  1: Request request;
}

struct RequestResponseNoArgVoidResponseClientInstruction {}

struct StreamBasicClientInstruction {
  1: Request request;
}

struct StreamChunkTimeoutClientInstruction {
  1: Request request;
  2: i32 chunkTimeoutMs;
}

struct StreamInitialResponseClientInstruction {
  1: Request request;
}

struct SinkBasicClientInstruction {
  1: Request request;
  2: list<Request> sinkPayloads;
}

struct RequestResponseBasicServerInstruction {
  1: Response response;
}

struct RequestResponseDeclaredExceptionServerInstruction {
  // TODO(dokwon): Remove @thrift.Box after fixing incomplete type bug.
  @thrift.Box
  1: optional UserException userException;
}

struct RequestResponseUndeclaredExceptionServerInstruction {
  1: string exceptionMessage;
}

struct RequestResponseNoArgVoidResponseServerInstruction {}

struct StreamBasicServerInstruction {
  1: list<Response> streamPayloads;
}

struct StreamChunkTimeoutServerInstruction {
  1: list<Response> streamPayloads;
  2: i32 chunkTimeoutMs;
}

struct StreamInitialResponseServerInstruction {
  1: list<Response> streamPayloads;
  2: Response initialResponse;
}

struct SinkBasicServerInstruction {
  1: Response finalResponse;
  2: i64 bufferSize;
}

service RPCConformanceService {
  // =================== Conformance framework - Only for Server Tests ===================
  void sendTestCase(1: RpcTestCase testCase);
  ServerTestResult getTestResult();

  // =================== Conformance framework - Only for Client Tests ===================
  RpcTestCase getTestCase();
  void sendTestResult(1: ClientTestResult result);

  // =================== Request-Response ===================
  Response requestResponseBasic(1: Request req);
  void requestResponseDeclaredException(1: Request req) throws (
    1: UserException e,
  );
  void requestResponseUndeclaredException(1: Request req);
  void requestResponseNoArgVoidResponse();

  // =================== Stream ===================
  stream<Response> streamBasic(1: Request req);
  stream<Response> streamChunkTimeout(1: Request req);
  Response, stream<Response> streamInitialResponse(1: Request req);

  // =================== Sink ===================
  sink<Request, Response> sinkBasic(1: Request req);
}
