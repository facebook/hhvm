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

#include <thrift/conformance/data/RPCGenerator.h>

namespace apache::thrift::conformance::data {

namespace {
const uint32_t kMaxFrameSize = (1 << 24) - 1;
const std::string kLargeData = std::string(kMaxFrameSize * 2, 'a');
const uint64_t kDefaultBufferSize = 100;

// =================== Request-Response ===================
Test createRequestResponseBasicTest() {
  Test ret;
  ret.name() = "RequestResponseBasicTest";
  ret.tags()->emplace("spec/protocol/interface/#request-response");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "RequestResponseBasic/Success";

  auto& rpcTest = testCase.rpc().emplace();
  rpcTest.clientInstruction()
      .emplace()
      .requestResponseBasic()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";
  rpcTest.clientTestResult()
      .emplace()
      .requestResponseBasic()
      .emplace()
      .response()
      .emplace()
      .data() = "world";

  rpcTest.serverInstruction()
      .emplace()
      .requestResponseBasic()
      .emplace()
      .response()
      .emplace()
      .data() = "world";
  rpcTest.serverTestResult()
      .emplace()
      .requestResponseBasic()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  return ret;
}

Test createRequestResponseDeclaredExceptionTest() {
  Test ret;
  ret.name() = "RequestResponseDeclaredExceptionTest";
  ret.tags()->emplace(
      "spec/protocol/interface/#declared-response-and-declared-exception");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "RequestResponseDeclaredException/Success";

  UserException userException;
  userException.msg() = "world";

  auto& rpcTest = testCase.rpc().emplace();
  rpcTest.clientInstruction()
      .emplace()
      .requestResponseDeclaredException()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";
  rpcTest.clientTestResult()
      .emplace()
      .requestResponseDeclaredException()
      .emplace()
      .userException() = userException;

  rpcTest.serverInstruction()
      .emplace()
      .requestResponseDeclaredException()
      .emplace()
      .userException() = userException;
  rpcTest.serverTestResult()
      .emplace()
      .requestResponseDeclaredException()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  return ret;
}

Test createRequestResponseUndeclaredExceptionTest() {
  Test ret;
  ret.name() = "RequestResponseUndeclaredExceptionTest";
  ret.tags()->emplace("spec/protocol/interface/#undeclared-exception");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "RequestResponseUndeclaredException/Success";

  auto& rpcTest = testCase.rpc().emplace();
  rpcTest.clientInstruction()
      .emplace()
      .requestResponseUndeclaredException()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";
  rpcTest.clientTestResult()
      .emplace()
      .requestResponseUndeclaredException()
      .emplace()
      .exceptionMessage() = "my undeclared exception";

  rpcTest.serverInstruction()
      .emplace()
      .requestResponseUndeclaredException()
      .emplace()
      .exceptionMessage() = "my undeclared exception";
  rpcTest.serverTestResult()
      .emplace()
      .requestResponseUndeclaredException()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  return ret;
}

Test createRequestResponseNoArgVoidResponse() {
  Test ret;
  ret.name() = "RequestResponseNoArgVoidResponseTest";
  ret.tags()->emplace("spec/protocol/interface/#request-response");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "RequestResponseNoArgVoidResponse/Success";

  auto& rpcTest = testCase.rpc().emplace();
  rpcTest.clientInstruction()
      .emplace()
      .requestResponseNoArgVoidResponse()
      .emplace();
  rpcTest.clientTestResult()
      .emplace()
      .requestResponseNoArgVoidResponse()
      .emplace();

  rpcTest.serverInstruction()
      .emplace()
      .requestResponseNoArgVoidResponse()
      .emplace();
  rpcTest.serverTestResult()
      .emplace()
      .requestResponseNoArgVoidResponse()
      .emplace();

  return ret;
}

Test createRequestResponseTimeoutTest() {
  Test ret;
  ret.name() = "RequestResponseTimeoutTest";
  ret.tags()->emplace("spec/protocol/interface/#client-timeout");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "RequestResponseTimeout/Success";

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction().emplace().requestResponseTimeout().emplace();
  clientInstruction.request().emplace().data() = "hello";
  clientInstruction.timeoutMs() = 100;

  rpcTest.clientTestResult()
      .emplace()
      .requestResponseTimeout()
      .emplace()
      .timeoutException() = true;

  rpcTest.serverInstruction()
      .emplace()
      .requestResponseTimeout()
      .emplace()
      .timeoutMs() = 150;
  rpcTest.serverTestResult()
      .emplace()
      .requestResponseTimeout()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  return ret;
}

Test createRequestResponseFragmentationTest() {
  Test ret;
  ret.name() = "RequestResponseFragmentationTest";
  ret.tags()->emplace("spec/protocol/interface/rocket#request-response");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "RequestResponseFragmentation/Success";

  auto& rpcTest = testCase.rpc().emplace();
  rpcTest.clientInstruction()
      .emplace()
      .requestResponseBasic()
      .emplace()
      .request()
      .emplace()
      .data() = kLargeData;
  rpcTest.clientTestResult()
      .emplace()
      .requestResponseBasic()
      .emplace()
      .response()
      .emplace()
      .data() = kLargeData;

  rpcTest.serverInstruction()
      .emplace()
      .requestResponseBasic()
      .emplace()
      .response()
      .emplace()
      .data() = kLargeData;
  rpcTest.serverTestResult()
      .emplace()
      .requestResponseBasic()
      .emplace()
      .request()
      .emplace()
      .data() = kLargeData;

  return ret;
}

// =================== Stream ===================
Test createStreamBasicTest() {
  Test ret;
  ret.name() = "StreamBasicTest";
  ret.tags()->emplace("spec/protocol/interface/#stream");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "StreamBasic/Success";

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction().emplace().streamBasic().emplace();
  clientInstruction.request().emplace().data() = "hello";
  clientInstruction.bufferSize() = kDefaultBufferSize;

  auto& serverInstruction =
      rpcTest.serverInstruction().emplace().streamBasic().emplace();
  for (int i = 0; i < 100; i++) {
    auto& payload = serverInstruction.streamPayloads()->emplace_back();
    payload.data() = folly::to<std::string>(i);
  }

  rpcTest.clientTestResult()
      .emplace()
      .streamBasic()
      .emplace()
      .streamPayloads()
      .copy_from(serverInstruction.streamPayloads());

  rpcTest.serverTestResult()
      .emplace()
      .streamBasic()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  return ret;
}

Test createStreamChunkTimeoutTest() {
  Test ret;
  ret.name() = "StreamChunkTimeoutTest";
  ret.tags()->emplace("spec/protocol/interface/#stream-chunk-timeout");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "StreamChunkTimeout/Success";

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction().emplace().streamChunkTimeout().emplace();
  clientInstruction.request().emplace().data() = "hello";
  clientInstruction.chunkTimeoutMs() = 100;

  auto& serverInstruction =
      rpcTest.serverInstruction().emplace().streamChunkTimeout().emplace();
  for (int i = 0; i < 100; i++) {
    auto& payload = serverInstruction.streamPayloads()->emplace_back();
    payload.data() = folly::to<std::string>(i);
  }
  serverInstruction.chunkTimeoutMs() = 150;

  auto& clientTestResult =
      rpcTest.clientTestResult().emplace().streamChunkTimeout().emplace();
  clientTestResult.streamPayloads().copy_from(
      serverInstruction.streamPayloads());
  clientTestResult.chunkTimeoutException() = true;

  rpcTest.serverTestResult()
      .emplace()
      .streamChunkTimeout()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  return ret;
}

Test createStreamFragmentationTest() {
  Test ret;
  ret.name() = "StreamFragmentationTest";
  ret.tags()->emplace("spec/protocol/interface/rocket/#stream");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "StreamFragmentation/Success";

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction().emplace().streamBasic().emplace();
  clientInstruction.request().emplace().data() = kLargeData;
  clientInstruction.bufferSize() = kDefaultBufferSize;

  auto& serverInstruction =
      rpcTest.serverInstruction().emplace().streamBasic().emplace();
  serverInstruction.streamPayloads()->emplace_back().data() = kLargeData;

  rpcTest.clientTestResult()
      .emplace()
      .streamBasic()
      .emplace()
      .streamPayloads()
      .copy_from(serverInstruction.streamPayloads());

  rpcTest.serverTestResult()
      .emplace()
      .streamBasic()
      .emplace()
      .request()
      .emplace()
      .data() = kLargeData;

  return ret;
}

Test createStreamInitialResponseTest() {
  Test ret;
  ret.name() = "StreamInitialResponseTest";
  ret.tags()->emplace("spec/protocol/interface/#stream-initial-response");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "StreamInitialResponse/Success";

  auto& rpcTest = testCase.rpc().emplace();
  rpcTest.clientInstruction()
      .emplace()
      .streamInitialResponse()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  auto& serverInstruction =
      rpcTest.serverInstruction().emplace().streamInitialResponse().emplace();
  for (int i = 0; i < 100; i++) {
    auto& payload = serverInstruction.streamPayloads()->emplace_back();
    payload.data() = folly::to<std::string>(i);
  }
  serverInstruction.initialResponse().emplace().data() = "world";

  auto& clientTestResult =
      rpcTest.clientTestResult().emplace().streamInitialResponse().emplace();
  clientTestResult.streamPayloads().copy_from(
      serverInstruction.streamPayloads());
  clientTestResult.initialResponse().emplace().data() = "world";

  rpcTest.serverTestResult()
      .emplace()
      .streamInitialResponse()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  return ret;
}

Test createStreamSubsequentCreditsTest() {
  // Same as StreamBasicTest but the chunk buffer size is smaller than the
  // number of stream payloads, so the client must keep sending subsequent
  // credits in order to receive all stream payloads.
  Test ret;
  ret.name() = "StreamSubsequentCreditsTest";
  ret.tags()->emplace("spec/protocol/interface/#stream-flow-control");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "StreamSubsequentCredits/Success";

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction().emplace().streamBasic().emplace();
  clientInstruction.request().emplace().data() = "hello";
  clientInstruction.bufferSize() = 10;

  auto& serverInstruction =
      rpcTest.serverInstruction().emplace().streamBasic().emplace();
  for (int i = 0; i < 100; i++) {
    auto& payload = serverInstruction.streamPayloads()->emplace_back();
    payload.data() = folly::to<std::string>(i);
  }

  rpcTest.clientTestResult()
      .emplace()
      .streamBasic()
      .emplace()
      .streamPayloads()
      .copy_from(serverInstruction.streamPayloads());

  rpcTest.serverTestResult()
      .emplace()
      .streamBasic()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  return ret;
}

Test createStreamCreditTimeoutTest() {
  Test ret;
  ret.name() = "StreamCreditTimeoutTest";
  ret.tags()->emplace("spec/protocol/interface/#stream-expire-time");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "StreamCreditTimeout/Success";

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction().emplace().streamCreditTimeout().emplace();
  clientInstruction.request().emplace().data() = "hello";
  clientInstruction.creditTimeoutMs() = 100;

  auto& serverInstruction =
      rpcTest.serverInstruction().emplace().streamCreditTimeout().emplace();
  for (int i = 0; i < 100; i++) {
    auto& payload = serverInstruction.streamPayloads()->emplace_back();
    payload.data() = folly::to<std::string>(i);
  }
  serverInstruction.streamExpireTime() = 50;

  auto& clientTestResult =
      rpcTest.clientTestResult().emplace().streamCreditTimeout().emplace();
  clientTestResult.creditTimeoutException() = true;

  rpcTest.serverTestResult()
      .emplace()
      .streamCreditTimeout()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  return ret;
}

Test createStreamDeclaredExceptionTest() {
  Test ret;
  ret.name() = "StreamDeclaredExceptionTest";
  ret.tags()->emplace("spec/protocol/interface/rocket#stream-exception");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "StreamDeclaredException/Success";

  auto& rpcTest = testCase.rpc().emplace();
  rpcTest.clientInstruction()
      .emplace()
      .streamDeclaredException()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  rpcTest.serverInstruction()
      .emplace()
      .streamDeclaredException()
      .emplace()
      .userException()
      .emplace()
      .msg() = "world";

  rpcTest.clientTestResult()
      .emplace()
      .streamDeclaredException()
      .emplace()
      .userException()
      .emplace()
      .msg() = "world";

  rpcTest.serverTestResult()
      .emplace()
      .streamDeclaredException()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  return ret;
}

Test createStreamUndeclaredExceptionTest() {
  Test ret;
  ret.name() = "StreamUndeclaredExceptionTest";
  ret.tags()->emplace("spec/protocol/interface/rocket#stream-exception");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "StreamUndeclaredException/Success";

  auto& rpcTest = testCase.rpc().emplace();
  rpcTest.clientInstruction()
      .emplace()
      .streamUndeclaredException()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  rpcTest.serverInstruction()
      .emplace()
      .streamUndeclaredException()
      .emplace()
      .exceptionMessage() = "world";

  rpcTest.clientTestResult()
      .emplace()
      .streamUndeclaredException()
      .emplace()
      .exceptionMessage() = "world";

  rpcTest.serverTestResult()
      .emplace()
      .streamUndeclaredException()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  return ret;
}

Test createStreamInitialDeclaredExceptionTest() {
  Test ret;
  ret.name() = "StreamInitialDeclaredExceptionTest";
  ret.tags()->emplace("spec/protocol/interface/#stream-initial-response");
  ret.tags()->emplace("spec/protocol/interface/rocket#stream-initial-response");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "StreamInitialDeclaredException/Success";

  auto& rpcTest = testCase.rpc().emplace();
  rpcTest.clientInstruction()
      .emplace()
      .streamInitialDeclaredException()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  rpcTest.serverInstruction()
      .emplace()
      .streamInitialDeclaredException()
      .emplace()
      .userException()
      .emplace()
      .msg() = "world";

  rpcTest.clientTestResult()
      .emplace()
      .streamInitialDeclaredException()
      .emplace()
      .userException()
      .emplace()
      .msg() = "world";

  rpcTest.serverTestResult()
      .emplace()
      .streamInitialDeclaredException()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  return ret;
}

Test createStreamInitialUndeclaredExceptionTest() {
  Test ret;
  ret.name() = "StreamInitialUndeclaredExceptionTest";
  ret.tags()->emplace("spec/protocol/interface/#stream-initial-response");
  ret.tags()->emplace("spec/protocol/interface/rocket#stream-initial-response");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "StreamInitialUndeclaredException/Success";

  auto& rpcTest = testCase.rpc().emplace();
  rpcTest.clientInstruction()
      .emplace()
      .streamInitialUndeclaredException()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  rpcTest.serverInstruction()
      .emplace()
      .streamInitialUndeclaredException()
      .emplace()
      .exceptionMessage() = "world";

  rpcTest.clientTestResult()
      .emplace()
      .streamInitialUndeclaredException()
      .emplace()
      .exceptionMessage() = "world";

  rpcTest.serverTestResult()
      .emplace()
      .streamInitialUndeclaredException()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  return ret;
}

Test createStreamInitialTimeoutTest() {
  Test ret;
  ret.name() = "StreamInitialTimeoutTest";
  ret.tags()->emplace("spec/protocol/interface/#client-timeout");
  ret.tags()->emplace("spec/protocol/interface/#client-detected-exceptions");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "StreamInitialTimeout/Success";

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction().emplace().streamInitialTimeout().emplace();
  clientInstruction.request().emplace().data() = "hello";
  clientInstruction.timeoutMs() = 100;

  rpcTest.serverInstruction()
      .emplace()
      .streamInitialTimeout()
      .emplace()
      .timeoutMs() = 150;

  rpcTest.clientTestResult()
      .emplace()
      .streamInitialTimeout()
      .emplace()
      .timeoutException() = true;

  rpcTest.serverTestResult()
      .emplace()
      .streamInitialTimeout()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  return ret;
}

// =================== Sink ===================
Test createSinkBasicTest() {
  Test ret;
  ret.name() = "SinkBasicTest";
  ret.tags()->emplace("spec/protocol/interface/#sink");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "SinkBasic/Success";

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction().emplace().sinkBasic().emplace();
  clientInstruction.request().emplace().data() = "hello";
  for (int i = 0; i < 100; i++) {
    auto& sinkPayload = clientInstruction.sinkPayloads()->emplace_back();
    sinkPayload.data() = folly::to<std::string>(i);
  }

  rpcTest.clientTestResult()
      .emplace()
      .sinkBasic()
      .emplace()
      .finalResponse()
      .emplace()
      .data() = "world";

  auto& serverInstruction =
      rpcTest.serverInstruction().emplace().sinkBasic().emplace();
  serverInstruction.finalResponse().emplace().data() = "world";
  serverInstruction.bufferSize() = kDefaultBufferSize;

  auto& serverResult =
      rpcTest.serverTestResult().emplace().sinkBasic().emplace();
  serverResult.request().emplace().data() = "hello";
  serverResult.sinkPayloads().copy_from(clientInstruction.sinkPayloads());

  return ret;
}

Test createSinkFragmentationTest() {
  Test ret;
  ret.name() = "SinkFragmentationTest";
  ret.tags()->emplace("spec/protocol/interface/rocket/#sink");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "SinkFragmentation/Success";

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction().emplace().sinkBasic().emplace();
  clientInstruction.request().emplace().data() = kLargeData;
  clientInstruction.sinkPayloads()->emplace_back().data() = kLargeData;

  rpcTest.clientTestResult()
      .emplace()
      .sinkBasic()
      .emplace()
      .finalResponse()
      .emplace()
      .data() = kLargeData;

  auto& serverInstruction =
      rpcTest.serverInstruction().emplace().sinkBasic().emplace();
  serverInstruction.finalResponse().emplace().data() = kLargeData;
  serverInstruction.bufferSize() = kDefaultBufferSize;

  auto& serverResult =
      rpcTest.serverTestResult().emplace().sinkBasic().emplace();
  serverResult.request().emplace().data() = kLargeData;
  serverResult.sinkPayloads().copy_from(clientInstruction.sinkPayloads());

  return ret;
}

Test createSinkSubsequentCreditsTest() {
  // Same as SinkBasicTest but the server buffer size is smaller than the number
  // of sink payloads, so the server must keep sending subsequent credits in
  // order to receive all sink payloads.
  Test ret;
  ret.name() = "SinkSubsequentCreditsTest";
  ret.tags()->emplace("spec/protocol/interface/#sink-flow-control");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "SinkSubsequestCredits/Success";

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction().emplace().sinkBasic().emplace();
  clientInstruction.request().emplace().data() = "hello";
  for (int i = 0; i < 100; i++) {
    auto& sinkPayload = clientInstruction.sinkPayloads()->emplace_back();
    sinkPayload.data() = folly::to<std::string>(i);
  }

  rpcTest.clientTestResult()
      .emplace()
      .sinkBasic()
      .emplace()
      .finalResponse()
      .emplace()
      .data() = "world";

  auto& serverInstruction =
      rpcTest.serverInstruction().emplace().sinkBasic().emplace();
  serverInstruction.finalResponse().emplace().data() = "world";
  serverInstruction.bufferSize() = 10;

  auto& serverResult =
      rpcTest.serverTestResult().emplace().sinkBasic().emplace();
  serverResult.request().emplace().data() = "hello";
  serverResult.sinkPayloads().copy_from(clientInstruction.sinkPayloads());

  return ret;
}

Test createSinkChunkTimeoutTest() {
  Test ret;
  ret.name() = "SinkChunkTimeoutTest";
  ret.tags()->emplace("spec/protocol/interface/#sink-chunk-timeout");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "SinkChunkTimeout/Success";

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction().emplace().sinkChunkTimeout().emplace();
  clientInstruction.request().emplace().data() = "hello";
  for (int i = 0; i < 100; i++) {
    auto& sinkPayload = clientInstruction.sinkPayloads()->emplace_back();
    sinkPayload.data() = folly::to<std::string>(i);
  }
  clientInstruction.chunkTimeoutMs() = 150;

  rpcTest.clientTestResult()
      .emplace()
      .sinkChunkTimeout()
      .emplace()
      .chunkTimeoutException() = true;

  auto& serverInstruction =
      rpcTest.serverInstruction().emplace().sinkChunkTimeout().emplace();
  serverInstruction.finalResponse().emplace().data() = "world";
  serverInstruction.chunkTimeoutMs() = 100;

  auto& serverResult =
      rpcTest.serverTestResult().emplace().sinkChunkTimeout().emplace();
  serverResult.request().emplace().data() = "hello";
  serverResult.sinkPayloads().copy_from(clientInstruction.sinkPayloads());
  serverResult.chunkTimeoutException() = true;

  return ret;
}

Test createSinkInitialResponseTest() {
  Test ret;
  ret.name() = "SinkInitialResponseTest";
  ret.tags()->emplace("spec/protocol/interface/#sink-initial-response");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "SinkInitialResponse/Success";

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction().emplace().sinkInitialResponse().emplace();
  clientInstruction.request().emplace().data() = "hello";
  for (int i = 0; i < 100; i++) {
    auto& sinkPayload = clientInstruction.sinkPayloads()->emplace_back();
    sinkPayload.data() = folly::to<std::string>(i);
  }

  auto& testResult =
      rpcTest.clientTestResult().emplace().sinkInitialResponse().emplace();
  testResult.initialResponse().emplace().data() = "hello";
  testResult.finalResponse().emplace().data() = "world";

  auto& serverInstruction =
      rpcTest.serverInstruction().emplace().sinkInitialResponse().emplace();

  serverInstruction.initialResponse().emplace().data() = "hello";
  serverInstruction.finalResponse().emplace().data() = "world";
  serverInstruction.bufferSize() = kDefaultBufferSize;

  auto& serverResult =
      rpcTest.serverTestResult().emplace().sinkInitialResponse().emplace();
  serverResult.request().emplace().data() = "hello";
  serverResult.sinkPayloads().copy_from(clientInstruction.sinkPayloads());

  return ret;
}

Test createSinkDeclaredExceptionTest() {
  Test ret;
  ret.name() = "SinkDeclaredExceptionTest";
  ret.tags()->emplace("spec/protocol/interface/#sink-declared-exception");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "SinkDeclaredException/Success";

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction().emplace().sinkDeclaredException().emplace();
  clientInstruction.request().emplace().data() = "request";
  clientInstruction.userException().emplace().msg() = "user_exception";

  rpcTest.clientTestResult()
      .emplace()
      .sinkDeclaredException()
      .emplace()
      .sinkThrew() = true;

  rpcTest.serverInstruction()
      .emplace()
      .sinkDeclaredException()
      .emplace()
      .bufferSize() = kDefaultBufferSize;

  auto& serverResult =
      rpcTest.serverTestResult().emplace().sinkDeclaredException().emplace();
  serverResult.request().emplace().data() = "request";
  serverResult.userException().copy_from(clientInstruction.userException());

  return ret;
}

Test createSinkUndeclaredExceptionTest() {
  Test ret;
  ret.name() = "SinkUndeclaredExceptionTest";
  ret.tags()->emplace("spec/protocol/interface/#sink-undeclared-exception");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "SinkUndeclaredException/Success";

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction().emplace().sinkUndeclaredException().emplace();
  clientInstruction.request().emplace().data() = "request";
  clientInstruction.exceptionMessage() = "undeclared_exception";

  rpcTest.clientTestResult()
      .emplace()
      .sinkUndeclaredException()
      .emplace()
      .sinkThrew() = true;

  rpcTest.serverInstruction()
      .emplace()
      .sinkUndeclaredException()
      .emplace()
      .bufferSize() = kDefaultBufferSize;

  auto& serverResult =
      rpcTest.serverTestResult().emplace().sinkUndeclaredException().emplace();
  serverResult.request().copy_from(clientInstruction.request());
  serverResult.exceptionMessage().copy_from(
      clientInstruction.exceptionMessage());

  return ret;
}

// =================== Interactions ===================
Test createInteractionConstructorTest() {
  Test ret;
  ret.name() = "InteractionConstructorTest";
  ret.tags()->emplace("spec/protocol/interface/#constructors-deprecated");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "InteractionConstructorTest/Success";

  auto& rpcTest = testCase.rpc().emplace();
  rpcTest.clientInstruction().emplace().interactionConstructor().emplace();

  rpcTest.clientTestResult().emplace().interactionConstructor().emplace();

  rpcTest.serverInstruction().emplace().interactionConstructor().emplace();

  rpcTest.serverTestResult()
      .emplace()
      .interactionConstructor()
      .emplace()
      .constructorCalled() = true;

  return ret;
}

Test createInteractionFactoryFunctionTest() {
  Test ret;
  ret.name() = "InteractionFactoryFunctionTest";
  ret.tags()->emplace("spec/protocol/interface/#factory-functions");
  ret.tags()->emplace("spec/protocol/interface/rocket/#factory-functions");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "InteractionFactoryFunctionTest/Success";

  constexpr int initialSum = 10;

  auto& rpcTest = testCase.rpc().emplace();
  rpcTest.clientInstruction()
      .emplace()
      .interactionFactoryFunction()
      .emplace()
      .initialSum() = initialSum;

  rpcTest.clientTestResult().emplace().interactionFactoryFunction().emplace();

  rpcTest.serverInstruction().emplace().interactionFactoryFunction().emplace();

  rpcTest.serverTestResult()
      .emplace()
      .interactionFactoryFunction()
      .emplace()
      .initialSum() = initialSum;

  return ret;
}

Test createInteractionConstructorPersistsStateTest() {
  Test ret;
  ret.name() = "InteractionConstructorPersistsStateTest";
  ret.tags()->emplace("spec/protocol/interface/#interactions");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "InteractionConstructorPersistsStateTest/Success";

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction = rpcTest.clientInstruction()
                                .emplace()
                                .interactionPersistsState()
                                .emplace();

  auto& clientTestResult =
      rpcTest.clientTestResult().emplace().interactionPersistsState().emplace();

  int sum = 0;
  for (int i = 1; i <= 5; i++) {
    sum += i;
    clientInstruction.valuesToAdd()->emplace_back(i);
    clientTestResult.responses()->emplace_back(sum);
  }

  rpcTest.serverInstruction().emplace().interactionPersistsState().emplace();

  rpcTest.serverTestResult().emplace().interactionPersistsState().emplace();

  return ret;
}

Test createInteractionFactoryFunctionPersistsStateTest() {
  Test ret;
  ret.name() = "InteractionFactoryFunctionPersistsStateTest";
  ret.tags()->emplace("spec/protocol/interface/#interactions");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "InteractionFactoryFunctionPersistsStateTest/Success";

  constexpr int initialSum = 10;

  auto& rpcTest = testCase.rpc().emplace();
  auto& clientInstruction = rpcTest.clientInstruction()
                                .emplace()
                                .interactionPersistsState()
                                .emplace();
  clientInstruction.initialSum() = initialSum;

  auto& clientTestResult =
      rpcTest.clientTestResult().emplace().interactionPersistsState().emplace();

  int sum = initialSum;
  for (int i = 1; i <= 5; i++) {
    sum += i;
    clientInstruction.valuesToAdd()->emplace_back(i);
    clientTestResult.responses()->emplace_back(sum);
  }

  rpcTest.serverInstruction().emplace().interactionPersistsState().emplace();

  rpcTest.serverTestResult().emplace().interactionPersistsState().emplace();

  return ret;
}

Test createInteractionConstructorTerminationTest() {
  Test ret;
  ret.name() = "InteractionConstructorTerminationTest";
  ret.tags()->emplace("spec/protocol/interface/#termination");
  ret.tags()->emplace("spec/protocol/interface/rocket/#termination");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "InteractionConstructorTerminationTest/Success";

  auto& rpcTest = testCase.rpc().emplace();
  rpcTest.clientInstruction().emplace().interactionTermination().emplace();

  rpcTest.clientTestResult().emplace().interactionTermination().emplace();

  rpcTest.serverInstruction().emplace().interactionTermination().emplace();

  rpcTest.serverTestResult()
      .emplace()
      .interactionTermination()
      .emplace()
      .terminationReceived() = true;

  return ret;
}

Test createInteractionFactoryFunctionTerminationTest() {
  Test ret;
  ret.name() = "InteractionFactoryFunctionTerminationTest";
  ret.tags()->emplace("spec/protocol/interface/#sink-chunk-timeout");
  ret.tags()->emplace("spec/protocol/interface/rocket/#termination");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "InteractionFactoryFunctionTerminationTest/Success";

  auto& rpcTest = testCase.rpc().emplace();
  rpcTest.clientInstruction()
      .emplace()
      .interactionTermination()
      .emplace()
      .initialSum() = 10;

  rpcTest.clientTestResult().emplace().interactionTermination().emplace();

  rpcTest.serverInstruction().emplace().interactionTermination().emplace();

  rpcTest.serverTestResult()
      .emplace()
      .interactionTermination()
      .emplace()
      .terminationReceived() = true;

  return ret;
}

void addCommonRequestResponseTests(TestSuite& suite) {
  // =================== Request-Response ===================
  suite.tests()->push_back(createRequestResponseBasicTest());
  suite.tests()->push_back(createRequestResponseDeclaredExceptionTest());
  suite.tests()->push_back(createRequestResponseUndeclaredExceptionTest());
  suite.tests()->push_back(createRequestResponseNoArgVoidResponse());
  suite.tests()->push_back(createRequestResponseFragmentationTest());
}

void addCommonRPCTests(TestSuite& suite) {
  // =================== Request-Response ===================
  addCommonRequestResponseTests(suite);
  // =================== Stream ===================
  suite.tests()->push_back(createStreamBasicTest());
  suite.tests()->push_back(createStreamFragmentationTest());
  suite.tests()->push_back(createStreamInitialResponseTest());
  suite.tests()->push_back(createStreamSubsequentCreditsTest());
  suite.tests()->push_back(createStreamDeclaredExceptionTest());
  suite.tests()->push_back(createStreamUndeclaredExceptionTest());
  suite.tests()->push_back(createStreamInitialDeclaredExceptionTest());
  suite.tests()->push_back(createStreamInitialUndeclaredExceptionTest());
  // =================== Sink ===================
  suite.tests()->push_back(createSinkBasicTest());
  suite.tests()->push_back(createSinkFragmentationTest());
  suite.tests()->push_back(createSinkSubsequentCreditsTest());
  suite.tests()->push_back(createSinkInitialResponseTest());
  suite.tests()->push_back(createSinkDeclaredExceptionTest());
  suite.tests()->push_back(createSinkUndeclaredExceptionTest());
  // =================== Interactions ===================
  suite.tests()->push_back(createInteractionConstructorTest());
  suite.tests()->push_back(createInteractionFactoryFunctionTest());
  suite.tests()->push_back(createInteractionConstructorPersistsStateTest());
  suite.tests()->push_back(createInteractionFactoryFunctionPersistsStateTest());
  suite.tests()->push_back(createInteractionConstructorTerminationTest());
  suite.tests()->push_back(createInteractionFactoryFunctionTerminationTest());
}

} // namespace

TestSuite createRPCServerTestSuite() {
  TestSuite suite;
  suite.name() = "ThriftRPCServerTest";
  suite.tags()->emplace("spec/protocol/interface/");
  addCommonRPCTests(suite);
  // =================== Sink ===================
  suite.tests()->push_back(createSinkChunkTimeoutTest());
  return suite;
}

TestSuite createBasicRPCServerTestSuite() {
  TestSuite suite;
  suite.name() = "ThriftBasicRPCServerTest";
  suite.tags()->emplace("spec/protocol/interface/");
  addCommonRequestResponseTests(suite);
  return suite;
}

TestSuite createRPCClientTestSuite() {
  TestSuite suite;
  suite.name() = "ThriftRPCClientTest";
  suite.tags()->emplace("spec/protocol/interface/");
  addCommonRPCTests(suite);
  // =================== Request-Response ===================
  suite.tests()->push_back(createRequestResponseTimeoutTest());
  // =================== Stream ===================
  suite.tests()->push_back(createStreamChunkTimeoutTest());
  suite.tests()->push_back(createStreamCreditTimeoutTest());
  suite.tests()->push_back(createStreamInitialTimeoutTest());
  return suite;
}

} // namespace apache::thrift::conformance::data
