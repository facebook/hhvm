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

@thrift.AllowLegacyMissingUris
package;

namespace cpp2 apache.thrift.conformance
namespace go thrift.conformance.rpc
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
  5: RequestResponseTimeoutServerTestResult requestResponseTimeout;
  100: StreamBasicServerTestResult streamBasic;
  101: StreamChunkTimeoutServerTestResult streamChunkTimeout;
  102: StreamInitialResponseServerTestResult streamInitialResponse;
  103: StreamCreditTimeoutServerTestResult streamCreditTimeout;
  104: StreamDeclaredExceptionServerTestResult streamDeclaredException;
  105: StreamUndeclaredExceptionServerTestResult streamUndeclaredException;
  106: StreamInitialDeclaredExceptionServerTestResult streamInitialDeclaredException;
  107: StreamInitialUndeclaredExceptionServerTestResult streamInitialUndeclaredException;
  108: StreamInitialTimeoutServerTestResult streamInitialTimeout;
  200: SinkBasicServerTestResult sinkBasic;
  201: SinkChunkTimeoutServerTestResult sinkChunkTimeout;
  202: SinkInitialResponseServerTestResult sinkInitialResponse;
  203: SinkDeclaredExceptionServerTestResult sinkDeclaredException;
  204: SinkUndeclaredExceptionServerTestResult sinkUndeclaredException;
  205: SinkInitialDeclaredExceptionServerTestResult sinkInitialDeclaredException;
  206: SinkServerDeclaredExceptionServerTestResult sinkServerDeclaredException;
  300: InteractionConstructorServerTestResult interactionConstructor;
  301: InteractionFactoryFunctionServerTestResult interactionFactoryFunction;
  302: InteractionPersistsStateServerTestResult interactionPersistsState;
  303: InteractionTerminationServerTestResult interactionTermination;
}

union ClientTestResult {
  1: RequestResponseBasicClientTestResult requestResponseBasic;
  2: RequestResponseDeclaredExceptionClientTestResult requestResponseDeclaredException;
  3: RequestResponseNoArgVoidResponseClientTestResult requestResponseNoArgVoidResponse;
  4: RequestResponseUndeclaredExceptionClientTestResult requestResponseUndeclaredException;
  5: RequestResponseTimeoutClientTestResult requestResponseTimeout;
  100: StreamBasicClientTestResult streamBasic;
  101: StreamChunkTimeoutClientTestResult streamChunkTimeout;
  102: StreamInitialResponseClientTestResult streamInitialResponse;
  103: StreamCreditTimeoutClientTestResult streamCreditTimeout;
  104: StreamDeclaredExceptionClientTestResult streamDeclaredException;
  105: StreamUndeclaredExceptionClientTestResult streamUndeclaredException;
  106: StreamInitialDeclaredExceptionClientTestResult streamInitialDeclaredException;
  107: StreamInitialUndeclaredExceptionClientTestResult streamInitialUndeclaredException;
  108: StreamInitialTimeoutClientTestResult streamInitialTimeout;
  200: SinkBasicClientTestResult sinkBasic;
  201: SinkChunkTimeoutClientTestResult sinkChunkTimeout;
  202: SinkInitialResponseClientTestResult sinkInitialResponse;
  203: SinkDeclaredExceptionClientTestResult sinkDeclaredException;
  204: SinkUndeclaredExceptionClientTestResult sinkUndeclaredException;
  205: SinkInitialDeclaredExceptionClientTestResult sinkInitialDeclaredException;
  206: SinkServerDeclaredExceptionClientTestResult sinkServerDeclaredException;
  300: InteractionConstructorClientTestResult interactionConstructor;
  301: InteractionFactoryFunctionClientTestResult interactionFactoryFunction;
  302: InteractionPersistsStateClientTestResult interactionPersistsState;
  303: InteractionTerminationClientTestResult interactionTermination;
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

struct RequestResponseTimeoutServerTestResult {
  1: Request request;
}

struct StreamBasicServerTestResult {
  1: Request request;
}

struct StreamChunkTimeoutServerTestResult {
  1: Request request;
}

struct StreamInitialResponseServerTestResult {
  1: Request request;
}

struct StreamCreditTimeoutServerTestResult {
  1: Request request;
}

struct StreamDeclaredExceptionServerTestResult {
  1: Request request;
}

struct StreamUndeclaredExceptionServerTestResult {
  1: Request request;
}

struct StreamInitialDeclaredExceptionServerTestResult {
  1: Request request;
}

struct StreamInitialUndeclaredExceptionServerTestResult {
  1: Request request;
}

struct StreamInitialTimeoutServerTestResult {
  1: Request request;
}

struct SinkBasicServerTestResult {
  1: Request request;
  2: list<Request> sinkPayloads;
}

struct SinkChunkTimeoutServerTestResult {
  1: Request request;
  2: list<Request> sinkPayloads;
  3: bool chunkTimeoutException;
}

struct SinkInitialResponseServerTestResult {
  1: Request request;
  2: list<Request> sinkPayloads;
}

struct SinkDeclaredExceptionServerTestResult {
  1: Request request;
  2: optional UserException userException;
}

struct SinkUndeclaredExceptionServerTestResult {
  1: Request request;
  2: optional string exceptionMessage;
}

struct SinkInitialDeclaredExceptionServerTestResult {
  1: Request request;
  2: optional string exceptionMessage;
}

struct SinkServerDeclaredExceptionServerTestResult {
  1: Request request;
  2: optional string exceptionMessage;
}

struct InteractionConstructorServerTestResult {
  1: bool constructorCalled;
}

struct InteractionFactoryFunctionServerTestResult {
  1: i32 initialSum;
}

struct InteractionPersistsStateServerTestResult {}

struct InteractionTerminationServerTestResult {
  1: bool terminationReceived;
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

struct RequestResponseTimeoutClientTestResult {
  1: bool timeoutException;
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

struct StreamCreditTimeoutClientTestResult {
  1: bool creditTimeoutException;
}

struct StreamDeclaredExceptionClientTestResult {
  // TODO(dokwon): Remove @thrift.Box after fixing incomplete type bug.
  @thrift.Box
  1: optional UserException userException;
}

struct StreamUndeclaredExceptionClientTestResult {
  1: string exceptionMessage;
}

struct StreamInitialDeclaredExceptionClientTestResult {
  // TODO(dokwon): Remove @thrift.Box after fixing incomplete type bug.
  @thrift.Box
  1: optional UserException userException;
}

struct StreamInitialUndeclaredExceptionClientTestResult {
  1: string exceptionMessage;
}

struct StreamInitialTimeoutClientTestResult {
  1: bool timeoutException;
}

struct SinkBasicClientTestResult {
  1: Response finalResponse;
}

struct SinkChunkTimeoutClientTestResult {
  1: bool chunkTimeoutException;
}

struct SinkInitialResponseClientTestResult {
  1: Response initialResponse;
  2: Response finalResponse;
}

struct SinkDeclaredExceptionClientTestResult {
  1: bool sinkThrew;
}

struct SinkUndeclaredExceptionClientTestResult {
  1: bool sinkThrew;
}

struct SinkInitialDeclaredExceptionClientTestResult {
  1: bool sinkThrew;
}

struct SinkServerDeclaredExceptionClientTestResult {
  1: bool sinkThrew;
}

struct InteractionConstructorClientTestResult {}

struct InteractionFactoryFunctionClientTestResult {}

struct InteractionPersistsStateClientTestResult {
  1: list<i32> responses;
}

struct InteractionTerminationClientTestResult {}

union ClientInstruction {
  1: RequestResponseBasicClientInstruction requestResponseBasic;
  2: RequestResponseDeclaredExceptionClientInstruction requestResponseDeclaredException;
  3: RequestResponseNoArgVoidResponseClientInstruction requestResponseNoArgVoidResponse;
  4: RequestResponseUndeclaredExceptionClientInstruction requestResponseUndeclaredException;
  5: RequestResponseTimeoutClientInstruction requestResponseTimeout;
  100: StreamBasicClientInstruction streamBasic;
  101: StreamChunkTimeoutClientInstruction streamChunkTimeout;
  102: StreamInitialResponseClientInstruction streamInitialResponse;
  103: StreamCreditTimeoutClientInstruction streamCreditTimeout;
  104: StreamDeclaredExceptionClientInstruction streamDeclaredException;
  105: StreamUndeclaredExceptionClientInstruction streamUndeclaredException;
  106: StreamInitialDeclaredExceptionClientInstruction streamInitialDeclaredException;
  107: StreamInitialUndeclaredExceptionClientInstruction streamInitialUndeclaredException;
  108: StreamInitialTimeoutClientInstruction streamInitialTimeout;
  200: SinkBasicClientInstruction sinkBasic;
  201: SinkChunkTimeoutClientInstruction sinkChunkTimeout;
  202: SinkInitialResponseClientInstruction sinkInitialResponse;
  203: SinkDeclaredExceptionClientInstruction sinkDeclaredException;
  204: SinkUndeclaredExceptionClientInstruction sinkUndeclaredException;
  205: SinkInitialDeclaredExceptionClientInstruction sinkInitialDeclaredException;
  206: SinkServerDeclaredExceptionClientInstruction sinkServerDeclaredException;
  300: InteractionConstructorClientInstruction interactionConstructor;
  301: InteractionFactoryFunctionClientInstruction interactionFactoryFunction;
  302: InteractionPersistsStateClientInstruction interactionPersistsState;
  303: InteractionTerminationClientInstruction interactionTermination;
}

union ServerInstruction {
  1: RequestResponseBasicServerInstruction requestResponseBasic;
  2: RequestResponseDeclaredExceptionServerInstruction requestResponseDeclaredException;
  3: RequestResponseNoArgVoidResponseServerInstruction requestResponseNoArgVoidResponse;
  4: RequestResponseUndeclaredExceptionServerInstruction requestResponseUndeclaredException;
  5: RequestResponseTimeoutServerInstruction requestResponseTimeout;
  100: StreamBasicServerInstruction streamBasic;
  101: StreamChunkTimeoutServerInstruction streamChunkTimeout;
  102: StreamInitialResponseServerInstruction streamInitialResponse;
  103: StreamCreditTimeoutServerInstruction streamCreditTimeout;
  104: StreamDeclaredExceptionServerInstruction streamDeclaredException;
  105: StreamUndeclaredExceptionServerInstruction streamUndeclaredException;
  106: StreamInitialDeclaredExceptionServerInstruction streamInitialDeclaredException;
  107: StreamInitialUndeclaredExceptionServerInstruction streamInitialUndeclaredException;
  108: StreamInitialTimeoutServerInstruction streamInitialTimeout;
  200: SinkBasicServerInstruction sinkBasic;
  201: SinkChunkTimeoutServerInstruction sinkChunkTimeout;
  202: SinkInitialResponseServerInstruction sinkInitialResponse;
  203: SinkDeclaredExceptionServerInstruction sinkDeclaredException;
  204: SinkUndeclaredExceptionServerInstruction sinkUndeclaredException;
  205: SinkInitialDeclaredExceptionServerInstruction sinkInitialDeclaredException;
  206: SinkServerDeclaredExceptionServerInstruction sinkServerDeclaredException;
  300: InteractionConstructorServerInstruction interactionConstructor;
  301: InteractionFactoryFunctionServerInstruction interactionFactoryFunction;
  302: InteractionPersistsStateServerInstruction interactionPersistsState;
  303: InteractionTerminationServerInstruction interactionTermination;
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

struct RequestResponseTimeoutClientInstruction {
  1: Request request;
  2: i64 timeoutMs;
}

struct StreamBasicClientInstruction {
  1: Request request;
  2: i64 bufferSize;
}

struct StreamChunkTimeoutClientInstruction {
  1: Request request;
  2: i64 chunkTimeoutMs;
}

struct StreamInitialResponseClientInstruction {
  1: Request request;
}

struct StreamCreditTimeoutClientInstruction {
  1: Request request;
  2: i64 creditTimeoutMs;
}

struct StreamDeclaredExceptionClientInstruction {
  1: Request request;
}

struct StreamUndeclaredExceptionClientInstruction {
  1: Request request;
}

struct StreamInitialDeclaredExceptionClientInstruction {
  1: Request request;
}

struct StreamInitialUndeclaredExceptionClientInstruction {
  1: Request request;
}

struct StreamInitialTimeoutClientInstruction {
  1: Request request;
  2: i64 timeoutMs;
}

struct SinkBasicClientInstruction {
  1: Request request;
  2: list<Request> sinkPayloads;
}

struct SinkChunkTimeoutClientInstruction {
  1: Request request;
  2: list<Request> sinkPayloads;
  3: i64 chunkTimeoutMs;
}

struct SinkInitialResponseClientInstruction {
  1: Request request;
  2: list<Request> sinkPayloads;
}

struct SinkDeclaredExceptionClientInstruction {
  1: Request request;
  2: optional UserException userException;
}

struct SinkUndeclaredExceptionClientInstruction {
  1: Request request;
  2: optional string exceptionMessage;
}

struct SinkInitialDeclaredExceptionClientInstruction {
  1: Request request;
  2: optional string exceptionMessage;
}

struct SinkServerDeclaredExceptionClientInstruction {
  1: Request request;
  2: optional string exceptionMessage;
}

struct InteractionConstructorClientInstruction {}

struct InteractionFactoryFunctionClientInstruction {
  1: i32 initialSum;
}

struct InteractionPersistsStateClientInstruction {
  1: optional i32 initialSum;
  2: list<i32> valuesToAdd;
}

struct InteractionTerminationClientInstruction {
  1: optional i32 initialSum;
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

struct RequestResponseTimeoutServerInstruction {
  1: i64 timeoutMs;
}

struct StreamBasicServerInstruction {
  1: list<Response> streamPayloads;
}

struct StreamChunkTimeoutServerInstruction {
  1: list<Response> streamPayloads;
  2: i64 chunkTimeoutMs;
}

struct StreamInitialResponseServerInstruction {
  1: list<Response> streamPayloads;
  2: Response initialResponse;
}

struct StreamCreditTimeoutServerInstruction {
  1: list<Response> streamPayloads;
  2: i64 streamExpireTime;
}

struct StreamDeclaredExceptionServerInstruction {
  // TODO(dokwon): Remove @thrift.Box after fixing incomplete type bug.
  @thrift.Box
  1: optional UserException userException;
}

struct StreamUndeclaredExceptionServerInstruction {
  1: string exceptionMessage;
}

struct StreamInitialDeclaredExceptionServerInstruction {
  // TODO(dokwon): Remove @thrift.Box after fixing incomplete type bug.
  @thrift.Box
  1: optional UserException userException;
}

struct StreamInitialUndeclaredExceptionServerInstruction {
  1: string exceptionMessage;
}

struct StreamInitialTimeoutServerInstruction {
  1: i64 timeoutMs;
}

struct SinkBasicServerInstruction {
  1: Response finalResponse;
  2: i64 bufferSize;
}

struct SinkChunkTimeoutServerInstruction {
  1: Response finalResponse;
  2: i64 chunkTimeoutMs;
}

struct SinkInitialResponseServerInstruction {
  1: Response initialResponse;
  2: Response finalResponse;
  3: i64 bufferSize;
}

struct SinkDeclaredExceptionServerInstruction {
  1: i64 bufferSize;
}

struct SinkUndeclaredExceptionServerInstruction {
  1: i64 bufferSize;
}

struct SinkInitialDeclaredExceptionServerInstruction {
  1: i64 bufferSize;
}

struct SinkServerDeclaredExceptionServerInstruction {
  1: i64 bufferSize;
}

struct InteractionConstructorServerInstruction {}

struct InteractionFactoryFunctionServerInstruction {}

struct InteractionPersistsStateServerInstruction {}

struct InteractionTerminationServerInstruction {}

interaction BasicInteraction {
  void init();
  // adds i to the cumulative sum and returns the new value
  i32 add(1: i32 i);
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
  Response requestResponseTimeout(1: Request req);

  // =================== Stream ===================
  stream<Response> streamBasic(1: Request req);
  stream<Response> streamChunkTimeout(1: Request req);
  Response, stream<Response> streamInitialResponse(1: Request req);
  stream<Response> streamCreditTimeout(1: Request req);
  stream<Response throws (1: UserException e)> streamDeclaredException(
    1: Request req,
  );
  stream<Response> streamUndeclaredException(1: Request req);
  stream<Response> streamInitialDeclaredException(1: Request req) throws (
    1: UserException e,
  );
  stream<Response> streamInitialUndeclaredException(1: Request req);
  stream<Response> streamInitialTimeout(1: Request req);

  // =================== Sink ===================
  sink<Request, Response> sinkBasic(1: Request req);
  sink<Request, Response> sinkChunkTimeout(1: Request req);
  Response, sink<Request, Response> sinkInitialResponse(1: Request req);
  # Service function may throw an expected exception
  sink<Request, Response> sinkInitialDeclaredException(1: Request req) throws (
    1: UserException e,
  );
  # Sink exception, client side may throw an expected exception and send to server
  sink<Request throws (1: UserException e), Response> sinkDeclaredException(
    1: Request req,
  );
  # Sink exception, server side may throw an expected exception and send to client
  sink<
    Request,
    Response throws (1: UserException e)
  > sinkServerDeclaredException(1: Request req);
  sink<Request, Response> sinkUndeclaredException(1: Request req);

  // =================== Interactions ===================
  performs BasicInteraction;
  BasicInteraction basicInteractionFactoryFunction(i32 initialSum);
}

service RPCStatelessConformanceService {
  // =================== Request-Response ===================
  ServerTestResult requestResponseBasic(1: Request req);
  void requestResponseDeclaredException(
    1: ServerInstruction serverInstr,
  ) throws (1: UserException e);
  void requestResponseUndeclaredException(1: ServerInstruction serverInstr);
  void requestResponseNoArgVoidResponse();

  // =================== Stream ===================
  stream<Response> streamBasic(1: ServerInstruction serverInstr);
  Response, stream<Response> streamInitialResponse(
    1: ServerInstruction serverInstr,
  );
  stream<Response throws (1: UserException e)> streamDeclaredException(
    1: ServerInstruction serverInstr,
  );
  stream<Response> streamUndeclaredException(1: ServerInstruction serverInstr);
  stream<Response> streamInitialDeclaredException(
    1: ServerInstruction serverInstr,
  ) throws (1: UserException e);
  stream<Response> streamInitialUndeclaredException(
    1: ServerInstruction serverInstr,
  );

  // =================== Sink ===================
  sink<Request, Response> sinkBasic(1: ServerInstruction serverInstr);
  sink<Request, Response> sinkChunkTimeout(1: ServerInstruction serverInstr);
  Response, sink<Request, Response> sinkInitialResponse(
    1: ServerInstruction serverInstr,
  );
  # Service function may throw an expected exception
  sink<Request, Response> sinkInitialDeclaredException(
    1: ServerInstruction serverInstr,
  ) throws (1: UserException e);
  # Sink exception, client side may throw an expected exception and send to server
  sink<Request throws (1: UserException e), Response> sinkDeclaredException(
    1: ServerInstruction serverInstr,
  );
  # Sink exception, server side may throw an expected exception and send to client
  sink<
    Request,
    Response throws (1: UserException e)
  > sinkServerDeclaredException(1: ServerInstruction serverInstr);
  sink<Request, Response> sinkUndeclaredException(
    1: ServerInstruction serverInstr,
  );
}
