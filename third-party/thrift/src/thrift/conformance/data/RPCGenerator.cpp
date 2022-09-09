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

  auto& rpcTest = testCase.rpc_ref().emplace();
  rpcTest.clientInstruction_ref()
      .emplace()
      .requestResponseBasic_ref()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";
  rpcTest.clientTestResult_ref()
      .emplace()
      .requestResponseBasic_ref()
      .emplace()
      .response()
      .emplace()
      .data() = "world";

  rpcTest.serverInstruction_ref()
      .emplace()
      .requestResponseBasic_ref()
      .emplace()
      .response()
      .emplace()
      .data() = "world";
  rpcTest.serverTestResult_ref()
      .emplace()
      .requestResponseBasic_ref()
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

  auto& rpcTest = testCase.rpc_ref().emplace();
  rpcTest.clientInstruction_ref()
      .emplace()
      .requestResponseDeclaredException_ref()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";
  rpcTest.clientTestResult_ref()
      .emplace()
      .requestResponseDeclaredException_ref()
      .emplace()
      .userException() = userException;

  rpcTest.serverInstruction_ref()
      .emplace()
      .requestResponseDeclaredException_ref()
      .emplace()
      .userException() = userException;
  rpcTest.serverTestResult_ref()
      .emplace()
      .requestResponseDeclaredException_ref()
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

  auto& rpcTest = testCase.rpc_ref().emplace();
  rpcTest.clientInstruction_ref()
      .emplace()
      .requestResponseUndeclaredException_ref()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";
  rpcTest.clientTestResult_ref()
      .emplace()
      .requestResponseUndeclaredException_ref()
      .emplace()
      .exceptionMessage() = "my undeclared exception";

  rpcTest.serverInstruction_ref()
      .emplace()
      .requestResponseUndeclaredException_ref()
      .emplace()
      .exceptionMessage() = "my undeclared exception";
  rpcTest.serverTestResult_ref()
      .emplace()
      .requestResponseUndeclaredException_ref()
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

  auto& rpcTest = testCase.rpc_ref().emplace();
  rpcTest.clientInstruction_ref()
      .emplace()
      .requestResponseNoArgVoidResponse_ref()
      .emplace();
  rpcTest.clientTestResult_ref()
      .emplace()
      .requestResponseNoArgVoidResponse_ref()
      .emplace();

  rpcTest.serverInstruction_ref()
      .emplace()
      .requestResponseNoArgVoidResponse_ref()
      .emplace();
  rpcTest.serverTestResult_ref()
      .emplace()
      .requestResponseNoArgVoidResponse_ref()
      .emplace();

  return ret;
}

Test createRequestResponseTimeoutTest() {
  Test ret;
  ret.name() = "RequestResponseTimeoutTest";
  ret.tags()->emplace("spec/protocol/interface/#client-timeout");

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "RequestResponseTimeout/Success";

  auto& rpcTest = testCase.rpc_ref().emplace();
  auto& clientInstruction = rpcTest.clientInstruction_ref()
                                .emplace()
                                .requestResponseTimeout_ref()
                                .emplace();
  clientInstruction.request().emplace().data() = "hello";
  clientInstruction.timeoutMs() = 100;

  rpcTest.clientTestResult_ref()
      .emplace()
      .requestResponseTimeout_ref()
      .emplace()
      .timeoutException() = true;

  rpcTest.serverInstruction_ref()
      .emplace()
      .requestResponseTimeout_ref()
      .emplace()
      .timeoutMs() = 150;
  rpcTest.serverTestResult_ref()
      .emplace()
      .requestResponseTimeout_ref()
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

  auto& rpcTest = testCase.rpc_ref().emplace();
  rpcTest.clientInstruction_ref()
      .emplace()
      .requestResponseBasic_ref()
      .emplace()
      .request()
      .emplace()
      .data() = kLargeData;
  rpcTest.clientTestResult_ref()
      .emplace()
      .requestResponseBasic_ref()
      .emplace()
      .response()
      .emplace()
      .data() = kLargeData;

  rpcTest.serverInstruction_ref()
      .emplace()
      .requestResponseBasic_ref()
      .emplace()
      .response()
      .emplace()
      .data() = kLargeData;
  rpcTest.serverTestResult_ref()
      .emplace()
      .requestResponseBasic_ref()
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

  auto& rpcTest = testCase.rpc_ref().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction_ref().emplace().streamBasic_ref().emplace();
  clientInstruction.request().emplace().data() = "hello";
  clientInstruction.bufferSize() = kDefaultBufferSize;

  auto& serverInstruction =
      rpcTest.serverInstruction_ref().emplace().streamBasic_ref().emplace();
  for (int i = 0; i < 100; i++) {
    auto& payload = serverInstruction.streamPayloads()->emplace_back();
    payload.data() = folly::to<std::string>(i);
  }

  rpcTest.clientTestResult_ref()
      .emplace()
      .streamBasic_ref()
      .emplace()
      .streamPayloads()
      .copy_from(serverInstruction.streamPayloads());

  rpcTest.serverTestResult_ref()
      .emplace()
      .streamBasic_ref()
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

  auto& rpcTest = testCase.rpc_ref().emplace();
  auto& clientInstruction = rpcTest.clientInstruction_ref()
                                .emplace()
                                .streamChunkTimeout_ref()
                                .emplace();
  clientInstruction.request().emplace().data() = "hello";
  clientInstruction.chunkTimeoutMs() = 100;

  auto& serverInstruction = rpcTest.serverInstruction_ref()
                                .emplace()
                                .streamChunkTimeout_ref()
                                .emplace();
  for (int i = 0; i < 100; i++) {
    auto& payload = serverInstruction.streamPayloads()->emplace_back();
    payload.data() = folly::to<std::string>(i);
  }
  serverInstruction.chunkTimeoutMs() = 150;

  auto& clientTestResult = rpcTest.clientTestResult_ref()
                               .emplace()
                               .streamChunkTimeout_ref()
                               .emplace();
  clientTestResult.streamPayloads().copy_from(
      serverInstruction.streamPayloads());
  clientTestResult.chunkTimeoutException() = true;

  rpcTest.serverTestResult_ref()
      .emplace()
      .streamChunkTimeout_ref()
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

  auto& rpcTest = testCase.rpc_ref().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction_ref().emplace().streamBasic_ref().emplace();
  clientInstruction.request().emplace().data() = kLargeData;
  clientInstruction.bufferSize() = kDefaultBufferSize;

  auto& serverInstruction =
      rpcTest.serverInstruction_ref().emplace().streamBasic_ref().emplace();
  serverInstruction.streamPayloads()->emplace_back().data() = kLargeData;

  rpcTest.clientTestResult_ref()
      .emplace()
      .streamBasic_ref()
      .emplace()
      .streamPayloads()
      .copy_from(serverInstruction.streamPayloads());

  rpcTest.serverTestResult_ref()
      .emplace()
      .streamBasic_ref()
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

  auto& rpcTest = testCase.rpc_ref().emplace();
  rpcTest.clientInstruction_ref()
      .emplace()
      .streamInitialResponse_ref()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

  auto& serverInstruction = rpcTest.serverInstruction_ref()
                                .emplace()
                                .streamInitialResponse_ref()
                                .emplace();
  for (int i = 0; i < 100; i++) {
    auto& payload = serverInstruction.streamPayloads()->emplace_back();
    payload.data() = folly::to<std::string>(i);
  }
  serverInstruction.initialResponse().emplace().data() = "world";

  auto& clientTestResult = rpcTest.clientTestResult_ref()
                               .emplace()
                               .streamInitialResponse_ref()
                               .emplace();
  clientTestResult.streamPayloads().copy_from(
      serverInstruction.streamPayloads());
  clientTestResult.initialResponse().emplace().data() = "world";

  rpcTest.serverTestResult_ref()
      .emplace()
      .streamInitialResponse_ref()
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

  auto& rpcTest = testCase.rpc_ref().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction_ref().emplace().streamBasic_ref().emplace();
  clientInstruction.request().emplace().data() = "hello";
  clientInstruction.bufferSize() = 10;

  auto& serverInstruction =
      rpcTest.serverInstruction_ref().emplace().streamBasic_ref().emplace();
  for (int i = 0; i < 100; i++) {
    auto& payload = serverInstruction.streamPayloads()->emplace_back();
    payload.data() = folly::to<std::string>(i);
  }

  rpcTest.clientTestResult_ref()
      .emplace()
      .streamBasic_ref()
      .emplace()
      .streamPayloads()
      .copy_from(serverInstruction.streamPayloads());

  rpcTest.serverTestResult_ref()
      .emplace()
      .streamBasic_ref()
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

  auto& rpcTest = testCase.rpc_ref().emplace();
  auto& clientInstruction = rpcTest.clientInstruction_ref()
                                .emplace()
                                .streamCreditTimeout_ref()
                                .emplace();
  clientInstruction.request().emplace().data() = "hello";
  clientInstruction.creditTimeoutMs() = 100;

  auto& serverInstruction = rpcTest.serverInstruction_ref()
                                .emplace()
                                .streamCreditTimeout_ref()
                                .emplace();
  for (int i = 0; i < 100; i++) {
    auto& payload = serverInstruction.streamPayloads()->emplace_back();
    payload.data() = folly::to<std::string>(i);
  }
  serverInstruction.streamExpireTime() = 10;

  auto& clientTestResult = rpcTest.clientTestResult_ref()
                               .emplace()
                               .streamCreditTimeout_ref()
                               .emplace();
  clientTestResult.creditTimeoutException() = true;

  rpcTest.serverTestResult_ref()
      .emplace()
      .streamCreditTimeout_ref()
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

  auto& rpcTest = testCase.rpc_ref().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction_ref().emplace().sinkBasic_ref().emplace();
  clientInstruction.request().emplace().data() = "hello";
  for (int i = 0; i < 100; i++) {
    auto& sinkPayload = clientInstruction.sinkPayloads()->emplace_back();
    sinkPayload.data() = folly::to<std::string>(i);
  }

  rpcTest.clientTestResult_ref()
      .emplace()
      .sinkBasic_ref()
      .emplace()
      .finalResponse()
      .emplace()
      .data() = "world";

  auto& serverInstruction =
      rpcTest.serverInstruction_ref().emplace().sinkBasic_ref().emplace();
  serverInstruction.finalResponse().emplace().data() = "world";
  serverInstruction.bufferSize() = kDefaultBufferSize;

  auto& serverResult =
      rpcTest.serverTestResult_ref().emplace().sinkBasic_ref().emplace();
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

  auto& rpcTest = testCase.rpc_ref().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction_ref().emplace().sinkBasic_ref().emplace();
  clientInstruction.request().emplace().data() = kLargeData;
  clientInstruction.sinkPayloads()->emplace_back().data() = kLargeData;

  rpcTest.clientTestResult_ref()
      .emplace()
      .sinkBasic_ref()
      .emplace()
      .finalResponse()
      .emplace()
      .data() = kLargeData;

  auto& serverInstruction =
      rpcTest.serverInstruction_ref().emplace().sinkBasic_ref().emplace();
  serverInstruction.finalResponse().emplace().data() = kLargeData;
  serverInstruction.bufferSize() = kDefaultBufferSize;

  auto& serverResult =
      rpcTest.serverTestResult_ref().emplace().sinkBasic_ref().emplace();
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

  auto& rpcTest = testCase.rpc_ref().emplace();
  auto& clientInstruction =
      rpcTest.clientInstruction_ref().emplace().sinkBasic_ref().emplace();
  clientInstruction.request().emplace().data() = "hello";
  for (int i = 0; i < 100; i++) {
    auto& sinkPayload = clientInstruction.sinkPayloads()->emplace_back();
    sinkPayload.data() = folly::to<std::string>(i);
  }

  rpcTest.clientTestResult_ref()
      .emplace()
      .sinkBasic_ref()
      .emplace()
      .finalResponse()
      .emplace()
      .data() = "world";

  auto& serverInstruction =
      rpcTest.serverInstruction_ref().emplace().sinkBasic_ref().emplace();
  serverInstruction.finalResponse().emplace().data() = "world";
  serverInstruction.bufferSize() = 10;

  auto& serverResult =
      rpcTest.serverTestResult_ref().emplace().sinkBasic_ref().emplace();
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

  auto& rpcTest = testCase.rpc_ref().emplace();
  auto& clientInstruction = rpcTest.clientInstruction_ref()
                                .emplace()
                                .sinkChunkTimeout_ref()
                                .emplace();
  clientInstruction.request().emplace().data() = "hello";
  for (int i = 0; i < 100; i++) {
    auto& sinkPayload = clientInstruction.sinkPayloads()->emplace_back();
    sinkPayload.data() = folly::to<std::string>(i);
  }
  clientInstruction.chunkTimeoutMs() = 150;

  rpcTest.clientTestResult_ref()
      .emplace()
      .sinkChunkTimeout_ref()
      .emplace()
      .chunkTimeoutException() = true;

  auto& serverInstruction = rpcTest.serverInstruction_ref()
                                .emplace()
                                .sinkChunkTimeout_ref()
                                .emplace();
  serverInstruction.finalResponse().emplace().data() = "world";
  serverInstruction.chunkTimeoutMs() = 100;

  auto& serverResult =
      rpcTest.serverTestResult_ref().emplace().sinkChunkTimeout_ref().emplace();
  serverResult.request().emplace().data() = "hello";
  serverResult.sinkPayloads().copy_from(clientInstruction.sinkPayloads());
  serverResult.chunkTimeoutException() = true;

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
  // =================== Sink ===================
  suite.tests()->push_back(createSinkBasicTest());
  suite.tests()->push_back(createSinkFragmentationTest());
  suite.tests()->push_back(createSinkSubsequentCreditsTest());
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
  return suite;
}

} // namespace apache::thrift::conformance::data
