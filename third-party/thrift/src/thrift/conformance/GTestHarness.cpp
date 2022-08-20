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

#include <thrift/conformance/GTestHarness.h>

#include <stdexcept>

#include <folly/experimental/coro/AsyncGenerator.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <thrift/conformance/RpcStructComparator.h>
#include <thrift/conformance/if/gen-cpp2/rpc_types.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

namespace apache::thrift::conformance {

testing::AssertionResult runRoundTripTest(
    Client<ConformanceService>& client, const RoundTripTestCase& roundTrip) {
  RoundTripResponse res;
  try {
    client.sync_roundTrip(res, *roundTrip.request());
  } catch (const apache::thrift::TApplicationException&) {
    return *roundTrip.expectException() ? testing::AssertionSuccess()
                                        : testing::AssertionFailure();
  }

  if (*roundTrip.expectException()) {
    return testing::AssertionFailure();
  }

  const Any& expectedAny = roundTrip.expectedResponse()
      ? *roundTrip.expectedResponse().value_unchecked().value()
      : *roundTrip.request()->value();

  auto parseAny = [](const Any& a) {
    switch (auto protocol = a.protocol().value_or(StandardProtocol::Compact)) {
      case StandardProtocol::Compact:
        return parseObject<apache::thrift::CompactProtocolReader>(*a.data());
      case StandardProtocol::Binary:
        return parseObject<apache::thrift::BinaryProtocolReader>(*a.data());
      default:
        throw std::invalid_argument(
            "Unsupported protocol: " + util::enumNameSafe(protocol));
    }
  };

  Object actual = parseAny(*res.value());
  Object expected = parseAny(expectedAny);
  if (!op::identical<type::struct_t<Object>>(actual, expected)) {
    // TODO(afuller): Report out the delta
    return testing::AssertionFailure();
  }
  return testing::AssertionSuccess();
}

// =================== Request-Response ===================
RequestResponseBasicClientTestResult runRequestResponseBasic(
    Client<RPCConformanceService>& client,
    const RequestResponseBasicClientInstruction& instruction) {
  RequestResponseBasicClientTestResult result;
  client.sync_requestResponseBasic(
      result.response().emplace(), *instruction.request());
  return result;
}

RequestResponseDeclaredExceptionClientTestResult
runRequestResponseDeclaredException(
    RPCConformanceServiceAsyncClient& client,
    const RequestResponseDeclaredExceptionClientInstruction& instruction) {
  RequestResponseDeclaredExceptionClientTestResult result;
  try {
    client.sync_requestResponseDeclaredException(*instruction.request());
  } catch (const UserException& ue) {
    result.userException() = ue;
  }
  return result;
}

RequestResponseUndeclaredExceptionClientTestResult
runRequestResponseUndeclaredException(
    RPCConformanceServiceAsyncClient& client,
    const RequestResponseUndeclaredExceptionClientInstruction& instruction) {
  RequestResponseUndeclaredExceptionClientTestResult result;
  try {
    client.sync_requestResponseUndeclaredException(*instruction.request());
  } catch (const TApplicationException& e) {
    result.exceptionMessage() = e.getMessage();
  }
  return result;
}

RequestResponseNoArgVoidResponseClientTestResult
runRequestResponseNoArgVoidResponse(
    RPCConformanceServiceAsyncClient& client,
    const RequestResponseNoArgVoidResponseClientInstruction&) {
  RequestResponseNoArgVoidResponseClientTestResult result;
  client.sync_requestResponseNoArgVoidResponse();
  return result;
}

// =================== Stream ===================
StreamBasicClientTestResult runStreamBasic(
    RPCConformanceServiceAsyncClient& client,
    const StreamBasicClientInstruction& instruction) {
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<StreamBasicClientTestResult> {
        auto gen = (co_await client.co_streamBasic(*instruction.request()))
                       .toAsyncGenerator();
        StreamBasicClientTestResult result;
        while (auto val = co_await gen.next()) {
          result.streamPayloads()->push_back(std::move(*val));
        }
        co_return result;
      }());
}

StreamInitialResponseClientTestResult runStreamInitialResponse(
    RPCConformanceServiceAsyncClient& client,
    const StreamInitialResponseClientInstruction& instruction) {
  StreamInitialResponseClientTestResult testResult;
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    auto result =
        co_await client.co_streamInitialResponse(*instruction.request());
    testResult.initialResponse() = std::move(result.response);
    auto gen = std::move(result.stream).toAsyncGenerator();
    while (auto val = co_await gen.next()) {
      testResult.streamPayloads()->push_back(std::move(*val));
    }
  }());
  return testResult;
}

// =================== Sink ===================
SinkBasicClientTestResult runSinkBasic(
    RPCConformanceServiceAsyncClient& client,
    const SinkBasicClientInstruction& instruction) {
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<SinkBasicClientTestResult> {
        auto sink = co_await client.co_sinkBasic(*instruction.request());
        auto finalResponse =
            co_await sink.sink([&]() -> folly::coro::AsyncGenerator<Request&&> {
              for (auto payload : *instruction.sinkPayloads()) {
                co_yield std::move(payload);
              }
            }());
        SinkBasicClientTestResult result;
        result.finalResponse() = std::move(finalResponse);
        co_return result;
      }());
}

ClientTestResult runClientSteps(
    Client<RPCConformanceService>& client,
    const ClientInstruction& clientInstruction) {
  ClientTestResult result;
  switch (clientInstruction.getType()) {
    case ClientInstruction::Type::requestResponseBasic:
      result.set_requestResponseBasic(runRequestResponseBasic(
          client, *clientInstruction.requestResponseBasic_ref()));
      break;
    case ClientInstruction::Type::requestResponseDeclaredException:
      result.set_requestResponseDeclaredException(
          runRequestResponseDeclaredException(
              client,
              *clientInstruction.requestResponseDeclaredException_ref()));
      break;
    case ClientInstruction::Type::requestResponseUndeclaredException:
      result.set_requestResponseUndeclaredException(
          runRequestResponseUndeclaredException(
              client,
              *clientInstruction.requestResponseUndeclaredException_ref()));
      break;
    case ClientInstruction::Type::requestResponseNoArgVoidResponse:
      result.set_requestResponseNoArgVoidResponse(
          runRequestResponseNoArgVoidResponse(
              client,
              *clientInstruction.requestResponseNoArgVoidResponse_ref()));
      break;
    case ClientInstruction::Type::streamBasic:
      result.set_streamBasic(
          runStreamBasic(client, *clientInstruction.streamBasic_ref()));
      break;
    case ClientInstruction::Type::streamInitialResponse:
      result.set_streamInitialResponse(runStreamInitialResponse(
          client, *clientInstruction.streamInitialResponse_ref()));
      break;
    case ClientInstruction::Type::sinkBasic:
      result.set_sinkBasic(
          runSinkBasic(client, *clientInstruction.sinkBasic_ref()));
      break;
    default:
      throw std::runtime_error("Invalid TestCase Type.");
  }
  return result;
}

testing::AssertionResult runRpcTest(
    Client<RPCConformanceService>& client, const RpcTestCase& rpc) {
  client.sync_sendTestCase(rpc);
  auto actualClientResult = runClientSteps(client, *rpc.clientInstruction());
  if (!equal(actualClientResult, *rpc.clientTestResult())) {
    return testing::AssertionFailure();
  }

  // Get result from server
  ServerTestResult actualServerResult;
  client.sync_getTestResult(actualServerResult);
  if (actualServerResult != *rpc.serverTestResult()) {
    return testing::AssertionFailure();
  }
  return testing::AssertionSuccess();
}

} // namespace apache::thrift::conformance
