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

// =================== Request-Response ===================
Test createRequestResponseBasicTest() {
  Test ret;
  ret.name() = "RequestResponseBasicTest";

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

Test createRequestResponseFragmentationTest() {
  Test ret;
  ret.name() = "RequestResponseFragmentationTest";

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

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "StreamBasic/Success";

  auto& rpcTest = testCase.rpc_ref().emplace();
  rpcTest.clientInstruction_ref()
      .emplace()
      .streamBasic_ref()
      .emplace()
      .request()
      .emplace()
      .data() = "hello";

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

  auto& testCase = ret.testCases()->emplace_back();
  testCase.name() = "StreamFragmentation/Success";

  auto& rpcTest = testCase.rpc_ref().emplace();
  rpcTest.clientInstruction_ref()
      .emplace()
      .streamBasic_ref()
      .emplace()
      .request()
      .emplace()
      .data() = kLargeData;

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

// =================== Sink ===================
Test createSinkBasicTest() {
  Test ret;
  ret.name() = "SinkBasicTest";

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
  serverInstruction.bufferSize() = 1000;

  auto& serverResult =
      rpcTest.serverTestResult_ref().emplace().sinkBasic_ref().emplace();
  serverResult.request().emplace().data() = "hello";
  serverResult.sinkPayloads().copy_from(clientInstruction.sinkPayloads());

  return ret;
}

Test createSinkFragmentationTest() {
  Test ret;
  ret.name() = "SinkFragmentationTest";

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
  serverInstruction.bufferSize() = 1000;

  auto& serverResult =
      rpcTest.serverTestResult_ref().emplace().sinkBasic_ref().emplace();
  serverResult.request().emplace().data() = kLargeData;
  serverResult.sinkPayloads().copy_from(clientInstruction.sinkPayloads());

  return ret;
}

Test createSinkSubsequentCreditsTest() {
  // Same as SinkBasicTest but server buffer size is smaller than number of sink
  // payloads, so server must keep sending subsequent credits in order to
  // receive all sink payloads
  Test ret;
  ret.name() = "SinkSubsequentCreditsTest";

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

void addCommonRPCTests(TestSuite& suite) {
  // =================== Request-Response ===================
  suite.tests()->push_back(createRequestResponseBasicTest());
  suite.tests()->push_back(createRequestResponseDeclaredExceptionTest());
  suite.tests()->push_back(createRequestResponseUndeclaredExceptionTest());
  suite.tests()->push_back(createRequestResponseNoArgVoidResponse());
  suite.tests()->push_back(createRequestResponseFragmentationTest());
  // =================== Stream ===================
  suite.tests()->push_back(createStreamBasicTest());
  suite.tests()->push_back(createStreamFragmentationTest());
  suite.tests()->push_back(createStreamInitialResponseTest());
  // =================== Sink ===================
  suite.tests()->push_back(createSinkBasicTest());
  suite.tests()->push_back(createSinkFragmentationTest());
  suite.tests()->push_back(createSinkSubsequentCreditsTest());
}

} // namespace

TestSuite createRPCServerTestSuite() {
  TestSuite suite;
  suite.name() = "ThriftRPCServerTest";
  addCommonRPCTests(suite);
  return suite;
}

TestSuite createRPCClientTestSuite() {
  TestSuite suite;
  suite.name() = "ThriftRPCClientTest";
  addCommonRPCTests(suite);
  // =================== Stream ===================
  suite.tests()->push_back(createStreamChunkTimeoutTest());
  return suite;
}

} // namespace apache::thrift::conformance::data
