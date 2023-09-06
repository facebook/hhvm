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
#include <exception>
#include <stdexcept>

#include <folly/ExceptionString.h>
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
  } catch (const apache::thrift::TApplicationException& e) {
    return *roundTrip.expectException()
        ? testing::AssertionSuccess()
        : (testing::AssertionFailure()
           << "Unexpected exception: " << folly::exceptionStr(e));
  }

  if (*roundTrip.expectException()) {
    return testing::AssertionFailure() << "No expected exception.";
  }

  const Any& expectedAny = roundTrip.expectedResponse()
      ? *roundTrip.expectedResponse().value_unchecked().value()
      : *roundTrip.request()->value();

  auto parseAny = [](const Any& a) {
    switch (auto protocol = a.protocol().value_or(StandardProtocol::Compact)) {
      case StandardProtocol::Compact:
        return protocol::parseObject<apache::thrift::CompactProtocolReader>(
            *a.data());
      case StandardProtocol::Binary:
        return protocol::parseObject<apache::thrift::BinaryProtocolReader>(
            *a.data());
      default:
        throw std::invalid_argument(
            "Unsupported protocol: " + util::enumNameSafe(protocol));
    }
  };

  auto toJson = [](const protocol::Object& obj) {
    return SimpleJSONSerializer::serialize<std::string>(obj);
  };

  protocol::Object actual = parseAny(*res.value());
  protocol::Object expected = parseAny(expectedAny);
  if (!op::identical<protocol::Object::Tag>(actual, expected)) {
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
        return protocol::parseObject<apache::thrift::CompactProtocolReader>(
            *a.data());
      case StandardProtocol::Binary:
        return protocol::parseObject<apache::thrift::BinaryProtocolReader>(
            *a.data());
      default:
        throw std::invalid_argument(
            "Unsupported protocol: " + util::enumNameSafe(protocol));
    }
  };

  protocol::Object actual = parseAny(*res.result());
  protocol::Object expected = parseAny(expectedAny);
  if (!op::identical<protocol::Object::Tag>(actual, expected)) {
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

StreamDeclaredExceptionClientTestResult runStreamDeclaredException(
    RPCConformanceServiceAsyncClient& client,
    const StreamDeclaredExceptionClientInstruction& instruction) {
  StreamDeclaredExceptionClientTestResult testResult;
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    auto gen =
        (co_await client.co_streamDeclaredException(*instruction.request()))
            .toAsyncGenerator();
    try {
      co_await gen.next();
    } catch (const UserException& e) {
      testResult.userException() = e;
    }
  }());
  return testResult;
}

StreamUndeclaredExceptionClientTestResult runStreamUndeclaredException(
    RPCConformanceServiceAsyncClient& client,
    const StreamUndeclaredExceptionClientInstruction& instruction) {
  StreamUndeclaredExceptionClientTestResult testResult;
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    auto gen =
        (co_await client.co_streamUndeclaredException(*instruction.request()))
            .toAsyncGenerator();
    try {
      co_await gen.next();
    } catch (const TApplicationException& e) {
      testResult.exceptionMessage() = e.getMessage();
    }
  }());
  return testResult;
}

StreamInitialDeclaredExceptionClientTestResult
runStreamInitialDeclaredException(
    RPCConformanceServiceAsyncClient& client,
    const StreamInitialDeclaredExceptionClientInstruction& instruction) {
  StreamInitialDeclaredExceptionClientTestResult testResult;
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    try {
      co_await client.co_streamInitialDeclaredException(*instruction.request());
    } catch (const UserException& e) {
      testResult.userException() = e;
    }
  }());
  return testResult;
}

StreamInitialUndeclaredExceptionClientTestResult
runStreamInitialUndeclaredException(
    RPCConformanceServiceAsyncClient& client,
    const StreamInitialUndeclaredExceptionClientInstruction& instruction) {
  StreamInitialUndeclaredExceptionClientTestResult testResult;
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    try {
      co_await client.co_streamInitialUndeclaredException(
          *instruction.request());
    } catch (const TApplicationException& e) {
      testResult.exceptionMessage() = e.getMessage();
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

// =================== Interactions ===================
InteractionConstructorClientTestResult runInteractionConstructor(
    RPCConformanceServiceAsyncClient& client,
    const InteractionConstructorClientInstruction&) {
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<InteractionConstructorClientTestResult> {
        auto interaction = client.createBasicInteraction();
        co_await interaction.co_init();
        co_return InteractionConstructorClientTestResult();
      }());
}

InteractionFactoryFunctionClientTestResult runInteractionFactoryFunction(
    RPCConformanceServiceAsyncClient& client,
    const InteractionFactoryFunctionClientInstruction& instruction) {
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<InteractionFactoryFunctionClientTestResult> {
        co_await client.co_basicInteractionFactoryFunction(
            *instruction.initialSum());
        co_return InteractionFactoryFunctionClientTestResult();
      }());
}

template <class InteractionClientInstruction>
Client<RPCConformanceService>::BasicInteraction createInteraction(
    RPCConformanceServiceAsyncClient& client,
    const InteractionClientInstruction& instruction) {
  if (instruction.initialSum().has_value()) {
    return folly::coro::blockingWait(
        client.co_basicInteractionFactoryFunction(*instruction.initialSum()));
  } else {
    return client.createBasicInteraction();
  }
}

InteractionPersistsStateClientTestResult runInteractionPersistsState(
    RPCConformanceServiceAsyncClient& client,
    const InteractionPersistsStateClientInstruction& instruction) {
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<InteractionPersistsStateClientTestResult> {
        auto interaction = createInteraction(client, instruction);
        InteractionPersistsStateClientTestResult result;
        for (auto& arg : *instruction.valuesToAdd()) {
          result.responses()->emplace_back(co_await interaction.co_add(arg));
        }
        co_return result;
      }());
}

InteractionTerminationClientTestResult runInteractionTermination(
    RPCConformanceServiceAsyncClient& client,
    const InteractionTerminationClientInstruction& instruction) {
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<InteractionTerminationClientTestResult> {
        auto interaction = createInteraction(client, instruction);
        co_await interaction.co_init();
        co_return InteractionTerminationClientTestResult();
      }());
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
    case ClientInstruction::Type::streamDeclaredException:
      result.streamDeclaredException_ref() = runStreamDeclaredException(
          client, *clientInstruction.streamDeclaredException_ref());
      break;
    case ClientInstruction::Type::streamUndeclaredException:
      result.streamUndeclaredException_ref() = runStreamUndeclaredException(
          client, *clientInstruction.streamUndeclaredException_ref());
      break;
    case ClientInstruction::Type::streamInitialDeclaredException:
      result.streamInitialDeclaredException_ref() =
          runStreamInitialDeclaredException(
              client, *clientInstruction.streamInitialDeclaredException_ref());
      break;
    case ClientInstruction::Type::streamInitialUndeclaredException:
      result.streamInitialUndeclaredException_ref() =
          runStreamInitialUndeclaredException(
              client,
              *clientInstruction.streamInitialUndeclaredException_ref());
      break;
    case ClientInstruction::Type::sinkBasic:
      result.sinkBasic_ref() =
          runSinkBasic(client, *clientInstruction.sinkBasic_ref());
      break;
    case ClientInstruction::Type::sinkChunkTimeout:
      result.sinkChunkTimeout_ref() = runSinkChunkTimeout(
          client, *clientInstruction.sinkChunkTimeout_ref());
      break;
    case ClientInstruction::Type::interactionConstructor:
      result.interactionConstructor_ref() = runInteractionConstructor(
          client, *clientInstruction.interactionConstructor_ref());
      break;
    case ClientInstruction::Type::interactionFactoryFunction:
      result.interactionFactoryFunction_ref() = runInteractionFactoryFunction(
          client, *clientInstruction.interactionFactoryFunction_ref());
      break;
    case ClientInstruction::Type::interactionPersistsState:
      result.interactionPersistsState_ref() = runInteractionPersistsState(
          client, *clientInstruction.interactionPersistsState_ref());
      break;
    case ClientInstruction::Type::interactionTermination:
      result.interactionTermination_ref() = runInteractionTermination(
          client, *clientInstruction.interactionTermination_ref());
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
  } catch (const std::exception& e) {
    return testing::AssertionFailure()
        << "\nFailed to send RPC test case to server: " << e.what();
  }

  ClientTestResult actualClientResult;
  try {
    actualClientResult = runClientSteps(client, *rpc.clientInstruction());
  } catch (const std::exception& e) {
    return testing::AssertionFailure()
        << "\nFailed to receive RPC client result: " << e.what();
  }

  if (!equal(actualClientResult, *rpc.clientTestResult())) {
    return testing::AssertionFailure()
        << "\nExpected client result: " << jsonify(*rpc.clientTestResult())
        << "\nActual client result: " << jsonify(actualClientResult);
  }

  // Get result from server
  ServerTestResult actualServerResult;
  try {
    client.sync_getTestResult(actualServerResult);
  } catch (const std::exception& e) {
    return testing::AssertionFailure()
        << "\nFailed to receive RPC server result: " << e.what();
  }

  if (actualServerResult != *rpc.serverTestResult()) {
    return testing::AssertionFailure()
        << "\nExpected server result: " << jsonify(*rpc.serverTestResult())
        << "\nActual server result: " << jsonify(actualServerResult);
  }
  return testing::AssertionSuccess();
}

testing::AssertionResult runBasicRpcTest(
    Client<BasicRPCConformanceService>& client, const RpcTestCase& rpc) {
  ClientTestResult result;
  ServerTestResult actualServerTestResult;
  const auto& clientInstruction = *rpc.clientInstruction();
  const auto& serverInstruction = *rpc.serverInstruction();
  try {
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
