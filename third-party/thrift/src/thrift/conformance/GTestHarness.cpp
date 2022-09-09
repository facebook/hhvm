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

#include <chrono>
#include <stdexcept>

#include <folly/experimental/coro/AsyncGenerator.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/Sleep.h>
#include <thrift/conformance/RpcStructComparator.h>
#include <thrift/conformance/Utils.h>
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

  auto toJson = [](const Object& obj) {
    folly::json::serialization_opts opts;
    opts.pretty_formatting = true;
    opts.sort_keys = true;
    opts.allow_non_string_keys = true;
    opts.allow_nan_inf = true;
    return folly::json::serialize(protocol::toDynamic(obj), opts);
  };

  Object actual = parseAny(*res.value());
  Object expected = parseAny(expectedAny);
  if (!op::identical<type::struct_t<Object>>(actual, expected)) {
    // TODO(afuller): Report out the delta
    return testing::AssertionFailure()
        << "\nInput: " << toJson(parseAny(*roundTrip.request()->value()))
        << "\nExpected: " << toJson(expected) << "\nActual: " << toJson(actual);
  }
  return testing::AssertionSuccess();
}

testing::AssertionResult runPatchTest(
    Client<ConformanceService>& client, const PatchOpTestCase& patchTestCase) {
  PatchOpResponse res;

  try {
    client.sync_patch(res, *patchTestCase.request());
  } catch (const apache::thrift::TApplicationException&) {
    return testing::AssertionFailure();
  }

  const Any& expectedAny = *patchTestCase.result();

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

  Object actual = parseAny(*res.result());
  Object expected = parseAny(expectedAny);
  if (!op::identical<type::struct_t<Object>>(actual, expected)) {
    // TODO: Report out the delta
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

ServerTestResult runRequestResponseBasic(
    Client<BasicRPCConformanceService>& client,
    const RequestResponseBasicClientInstruction& instruction) {
  ServerTestResult result;
  client.sync_requestResponseBasic(result, *instruction.request());
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

RequestResponseDeclaredExceptionClientTestResult
runRequestResponseDeclaredException(
    BasicRPCConformanceServiceAsyncClient& client,
    const ServerInstruction& serverInstruction) {
  RequestResponseDeclaredExceptionClientTestResult result;
  try {
    client.sync_requestResponseDeclaredException(serverInstruction);
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

RequestResponseUndeclaredExceptionClientTestResult
runRequestResponseUndeclaredException(
    BasicRPCConformanceServiceAsyncClient& client,
    const ServerInstruction& serverInstruction) {
  RequestResponseUndeclaredExceptionClientTestResult result;
  try {
    client.sync_requestResponseUndeclaredException(serverInstruction);
  } catch (const TApplicationException& e) {
    result.exceptionMessage() = e.getMessage();
  }
  return result;
}

RequestResponseNoArgVoidResponseClientTestResult
runRequestResponseNoArgVoidResponse(RPCConformanceServiceAsyncClient& client) {
  RequestResponseNoArgVoidResponseClientTestResult result;
  client.sync_requestResponseNoArgVoidResponse();
  return result;
}

RequestResponseNoArgVoidResponseClientTestResult
runRequestResponseNoArgVoidResponse(
    BasicRPCConformanceServiceAsyncClient& client) {
  RequestResponseNoArgVoidResponseClientTestResult result;
  client.sync_requestResponseNoArgVoidResponse();
  return result;
}

// =================== Stream ===================
StreamBasicClientTestResult runStreamBasic(
    RPCConformanceServiceAsyncClient& client,
    const StreamBasicClientInstruction& instruction) {
  apache::thrift::RpcOptions rpcOptions{};
  rpcOptions.setChunkBufferSize(*instruction.bufferSize());
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<StreamBasicClientTestResult> {
        auto gen =
            (co_await client.co_streamBasic(rpcOptions, *instruction.request()))
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

SinkChunkTimeoutClientTestResult runSinkChunkTimeout(
    RPCConformanceServiceAsyncClient& client,
    const SinkChunkTimeoutClientInstruction& instruction) {
  SinkChunkTimeoutClientTestResult result;
  try {
    folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
      auto sink = co_await client.co_sinkChunkTimeout(*instruction.request());
      auto finalResponse =
          co_await sink.sink([&]() -> folly::coro::AsyncGenerator<Request&&> {
            for (auto payload : *instruction.sinkPayloads()) {
              co_yield std::move(payload);
            }
            co_await folly::coro::sleep(
                std::chrono::milliseconds{*instruction.chunkTimeoutMs()});
          }());
    }());
  } catch (const apache::thrift::TApplicationException& e) {
    if (e.getType() ==
        TApplicationException::TApplicationExceptionType::TIMEOUT) {
      result.chunkTimeoutException() = true;
    }
  }
  return result;
}

ClientTestResult runClientSteps(
    Client<RPCConformanceService>& client,
    const ClientInstruction& clientInstruction) {
  ClientTestResult result;
  switch (clientInstruction.getType()) {
    case ClientInstruction::Type::requestResponseBasic:
      result.requestResponseBasic_ref() = runRequestResponseBasic(
          client, *clientInstruction.requestResponseBasic_ref());
      break;
    case ClientInstruction::Type::requestResponseDeclaredException:
      result.requestResponseDeclaredException_ref() =
          runRequestResponseDeclaredException(
              client,
              *clientInstruction.requestResponseDeclaredException_ref());
      break;
    case ClientInstruction::Type::requestResponseUndeclaredException:
      result.requestResponseUndeclaredException_ref() =
          runRequestResponseUndeclaredException(
              client,
              *clientInstruction.requestResponseUndeclaredException_ref());
      break;
    case ClientInstruction::Type::requestResponseNoArgVoidResponse:
      result.requestResponseNoArgVoidResponse_ref() =
          runRequestResponseNoArgVoidResponse(client);
      break;
    case ClientInstruction::Type::streamBasic:
      result.streamBasic_ref() =
          runStreamBasic(client, *clientInstruction.streamBasic_ref());
      break;
    case ClientInstruction::Type::streamInitialResponse:
      result.streamInitialResponse_ref() = runStreamInitialResponse(
          client, *clientInstruction.streamInitialResponse_ref());
      break;
    case ClientInstruction::Type::sinkBasic:
      result.sinkBasic_ref() =
          runSinkBasic(client, *clientInstruction.sinkBasic_ref());
      break;
    case ClientInstruction::Type::sinkChunkTimeout:
      result.sinkChunkTimeout_ref() = runSinkChunkTimeout(
          client, *clientInstruction.sinkChunkTimeout_ref());
      break;
    default:
      throw std::runtime_error("Invalid TestCase Type.");
  }
  return result;
}

testing::AssertionResult runRpcTest(
    Client<RPCConformanceService>& client, const RpcTestCase& rpc) {
  try {
    client.sync_sendTestCase(rpc);
    auto actualClientResult = runClientSteps(client, *rpc.clientInstruction());
    if (!equal(actualClientResult, *rpc.clientTestResult())) {
      return testing::AssertionFailure()
          << "\nExpected client result: " << jsonify(*rpc.clientTestResult())
          << "\nActual client result: " << jsonify(actualClientResult);
    }

    // Get result from server
    ServerTestResult actualServerResult;
    client.sync_getTestResult(actualServerResult);
    if (actualServerResult != *rpc.serverTestResult()) {
      return testing::AssertionFailure()
          << "\nExpected server result: " << jsonify(*rpc.serverTestResult())
          << "\nActual server result: " << jsonify(actualServerResult);
    }
    return testing::AssertionSuccess();
  } catch (...) {
    return testing::AssertionFailure();
  }
}

testing::AssertionResult runBasicRpcTest(
    Client<BasicRPCConformanceService>& client, const RpcTestCase& rpc) {
  try {
    ClientTestResult result;
    auto clientInstruction = *rpc.clientInstruction();
    auto serverInstruction = *rpc.serverInstruction();
    ServerTestResult actualServerTestResult;
    switch (clientInstruction.getType()) {
      case ClientInstruction::Type::requestResponseBasic:
        actualServerTestResult = runRequestResponseBasic(
            client, *clientInstruction.requestResponseBasic_ref());

        if (actualServerTestResult != *rpc.serverTestResult()) {
          return testing::AssertionFailure();
        } else {
          return testing::AssertionSuccess();
        }
        break;
      case ClientInstruction::Type::requestResponseDeclaredException:
        result.requestResponseDeclaredException_ref() =
            runRequestResponseDeclaredException(client, serverInstruction);
        break;
      case ClientInstruction::Type::requestResponseUndeclaredException:
        result.requestResponseUndeclaredException_ref() =
            runRequestResponseUndeclaredException(client, serverInstruction);
        break;
      case ClientInstruction::Type::requestResponseNoArgVoidResponse:
        result.requestResponseNoArgVoidResponse_ref() =
            runRequestResponseNoArgVoidResponse(client);
        break;
      default:
        throw std::runtime_error("Invalid TestCase Type.");
    }

    if (!equal(result, *rpc.clientTestResult())) {
      return testing::AssertionFailure();
    }
    return testing::AssertionSuccess();

  } catch (...) {
    return testing::AssertionFailure();
  }
}

} // namespace apache::thrift::conformance
