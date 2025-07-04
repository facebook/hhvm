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
#include <folly/coro/AsyncGenerator.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Sleep.h>
#include <thrift/conformance/RpcStructComparator.h>
#include <thrift/conformance/Utils.h>
#include <thrift/conformance/if/gen-cpp2/rpc_types.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

namespace apache::thrift::conformance {
template <typename ClientType>
using RPCRequestParam = std::conditional_t<
    std::is_same_v<ClientType, apache::thrift::Client<RPCConformanceService>>,
    Request,
    ServerInstruction>;

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
  if (!op::identical<protocol::Object::Tag>(
          actual.toThrift(), expected.toThrift())) {
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

  auto actual = AnyRegistry::generated().load<protocol::Value>(*res.result());
  auto expected = AnyRegistry::generated().load<protocol::Value>(expectedAny);

  // This is needed right now to handle string/binary ambiguation and to treat
  // all string/binary as binary.
  auto rexpected = protocol::parseValue<CompactProtocolReader>(
      *protocol::serializeValue<CompactProtocolWriter>(expected),
      static_cast<type::BaseType>(expected.getType()));

  if (actual != rexpected) {
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
    Client<RPCStatelessConformanceService>& client,
    const RequestResponseBasicClientInstruction& instruction) {
  ServerTestResult result;
  client.sync_requestResponseBasic(result, *instruction.request());
  return result;
}

template <typename ClientType>
RequestResponseDeclaredExceptionClientTestResult
runRequestResponseDeclaredException(
    ClientType& client, const RPCRequestParam<ClientType>& request) {
  RequestResponseDeclaredExceptionClientTestResult result;
  try {
    client.sync_requestResponseDeclaredException(request);
  } catch (const UserException& ue) {
    result.userException() = ue;
  }
  return result;
}

template <typename ClientType>
RequestResponseUndeclaredExceptionClientTestResult
runRequestResponseUndeclaredException(
    ClientType& client, const RPCRequestParam<ClientType>& request) {
  RequestResponseUndeclaredExceptionClientTestResult result;
  try {
    client.sync_requestResponseUndeclaredException(request);
  } catch (const TApplicationException& e) {
    result.exceptionMessage() = e.getMessage();
  }
  return result;
}
template <typename ClientType>
RequestResponseNoArgVoidResponseClientTestResult
runRequestResponseNoArgVoidResponse(ClientType& client) {
  RequestResponseNoArgVoidResponseClientTestResult result;
  client.sync_requestResponseNoArgVoidResponse();
  return result;
}

// =================== Stream ===================

template <typename ClientType>
StreamBasicClientTestResult runStreamBasic(
    ClientType& client,
    const StreamBasicClientInstruction& instruction,
    const RPCRequestParam<ClientType>& request) {
  apache::thrift::RpcOptions rpcOptions{};
  rpcOptions.setChunkBufferSize(*instruction.bufferSize());
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<StreamBasicClientTestResult> {
        auto gen = (co_await client.co_streamBasic(rpcOptions, request))
                       .toAsyncGenerator();
        StreamBasicClientTestResult result;
        while (auto val = co_await gen.next()) {
          result.streamPayloads()->push_back(std::move(*val));
        }
        co_return result;
      }());
}

template <typename ClientType>
StreamInitialResponseClientTestResult runStreamInitialResponse(
    ClientType& client, const RPCRequestParam<ClientType>& request) {
  StreamInitialResponseClientTestResult testResult;
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    auto result = co_await client.co_streamInitialResponse(request);
    testResult.initialResponse() = std::move(result.response);
    auto gen = std::move(result.stream).toAsyncGenerator();
    while (auto val = co_await gen.next()) {
      testResult.streamPayloads()->push_back(std::move(*val));
    }
  }());
  return testResult;
}

template <typename ClientType>
StreamDeclaredExceptionClientTestResult runStreamDeclaredException(
    ClientType& client, const RPCRequestParam<ClientType>& request) {
  StreamDeclaredExceptionClientTestResult testResult;
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    auto gen = (co_await client.co_streamDeclaredException(request))
                   .toAsyncGenerator();
    try {
      co_await gen.next();
    } catch (const UserException& e) {
      testResult.userException() = e;
    }
  }());
  return testResult;
}

template <typename ClientType>
StreamUndeclaredExceptionClientTestResult runStreamUndeclaredException(
    ClientType& client, const RPCRequestParam<ClientType>& request) {
  StreamUndeclaredExceptionClientTestResult testResult;
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    auto gen = (co_await client.co_streamUndeclaredException(request))
                   .toAsyncGenerator();
    try {
      co_await gen.next();
    } catch (const TApplicationException& e) {
      testResult.exceptionMessage() = e.getMessage();
    }
  }());
  return testResult;
}

template <typename ClientType>
StreamInitialDeclaredExceptionClientTestResult
runStreamInitialDeclaredException(
    ClientType& client, const RPCRequestParam<ClientType>& request) {
  StreamInitialDeclaredExceptionClientTestResult testResult;
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    try {
      co_await client.co_streamInitialDeclaredException(request);
    } catch (const UserException& e) {
      testResult.userException() = e;
    }
  }());
  return testResult;
}

template <typename ClientType>
StreamInitialUndeclaredExceptionClientTestResult
runStreamInitialUndeclaredException(
    ClientType& client, const RPCRequestParam<ClientType>& request) {
  StreamInitialUndeclaredExceptionClientTestResult testResult;
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    try {
      co_await client.co_streamInitialUndeclaredException(request);
    } catch (const TApplicationException& e) {
      testResult.exceptionMessage() = e.getMessage();
    }
  }());
  return testResult;
}

// =================== Sink ===================
template <typename ClientType>
SinkBasicClientTestResult runSinkBasic(
    ClientType& client,
    const SinkBasicClientInstruction& instruction,
    const RPCRequestParam<ClientType>& request) {
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<SinkBasicClientTestResult> {
        auto sink = co_await client.co_sinkBasic(request);
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

template <typename ClientType>
SinkChunkTimeoutClientTestResult runSinkChunkTimeout(
    ClientType& client,
    const SinkChunkTimeoutClientInstruction& instruction,
    const RPCRequestParam<ClientType>& request) {
  SinkChunkTimeoutClientTestResult result;
  try {
    folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
      auto sink = co_await client.co_sinkChunkTimeout(request);
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

template <typename ClientType>
SinkInitialResponseClientTestResult runSinkInitialResponse(
    ClientType& client,
    const SinkInitialResponseClientInstruction& instruction,
    const RPCRequestParam<ClientType>& request) {
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<SinkInitialResponseClientTestResult> {
        SinkInitialResponseClientTestResult result;
        auto sinkAndResponse = co_await client.co_sinkInitialResponse(request);
        result.initialResponse() = sinkAndResponse.response;
        auto finalResponse = co_await sinkAndResponse.sink.sink(
            [&]() -> folly::coro::AsyncGenerator<Request&&> {
              for (auto payload : *instruction.sinkPayloads()) {
                co_yield std::move(payload);
              }
            }());
        result.finalResponse() = std::move(finalResponse);
        co_return result;
      }());
}

template <typename ClientType>
SinkDeclaredExceptionClientTestResult runSinkDeclaredException(
    ClientType& client,
    const SinkDeclaredExceptionClientInstruction& instruction,
    const RPCRequestParam<ClientType>& request) {
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<SinkDeclaredExceptionClientTestResult> {
        auto sink = co_await client.co_sinkDeclaredException(request);
        auto finalResponse = co_await folly::coro::co_awaitTry(
            sink.sink([&]() -> folly::coro::AsyncGenerator<Request&&> {
              throw *instruction.userException();
              co_return;
            }()));
        SinkDeclaredExceptionClientTestResult result;
        result.sinkThrew() = finalResponse.template hasException<SinkThrew>();
        co_return result;
      }());
}

template <typename ClientType>
SinkUndeclaredExceptionClientTestResult runSinkUndeclaredException(
    ClientType& client,
    const SinkUndeclaredExceptionClientInstruction& instruction,
    const RPCRequestParam<ClientType>& request) {
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<SinkUndeclaredExceptionClientTestResult> {
        auto sink = co_await client.co_sinkUndeclaredException(request);
        auto finalResponse = co_await folly::coro::co_awaitTry(
            sink.sink([&]() -> folly::coro::AsyncGenerator<Request&&> {
              throw std::runtime_error(*instruction.exceptionMessage());
              co_return;
            }()));
        SinkUndeclaredExceptionClientTestResult result;
        result.sinkThrew() = finalResponse.template hasException<SinkThrew>();
        co_return result;
      }());
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
      result.requestResponseBasic() = runRequestResponseBasic(
          client, *clientInstruction.requestResponseBasic());
      break;
    case ClientInstruction::Type::requestResponseDeclaredException:
      result.requestResponseDeclaredException() =
          runRequestResponseDeclaredException(
              client,
              *clientInstruction.requestResponseDeclaredException()->request());
      break;
    case ClientInstruction::Type::requestResponseUndeclaredException:
      result.requestResponseUndeclaredException() =
          runRequestResponseUndeclaredException(
              client,
              *clientInstruction.requestResponseUndeclaredException()
                   ->request());
      break;
    case ClientInstruction::Type::requestResponseNoArgVoidResponse:
      result.requestResponseNoArgVoidResponse() =
          runRequestResponseNoArgVoidResponse(client);
      break;
    case ClientInstruction::Type::streamBasic: {
      auto instruction = *clientInstruction.streamBasic();
      result.streamBasic() =
          runStreamBasic(client, instruction, *instruction.request());
      break;
    }
    case ClientInstruction::Type::streamInitialResponse: {
      auto instruction = *clientInstruction.streamInitialResponse();
      result.streamInitialResponse() =
          runStreamInitialResponse(client, *instruction.request());
      break;
    }
    case ClientInstruction::Type::streamDeclaredException: {
      auto instruction = *clientInstruction.streamDeclaredException();
      result.streamDeclaredException() =
          runStreamDeclaredException(client, *instruction.request());
      break;
    }
    case ClientInstruction::Type::streamUndeclaredException: {
      auto instruction = *clientInstruction.streamUndeclaredException();
      result.streamUndeclaredException() =
          runStreamUndeclaredException(client, *instruction.request());
      break;
    }
    case ClientInstruction::Type::streamInitialDeclaredException: {
      auto instruction = *clientInstruction.streamInitialDeclaredException();
      result.streamInitialDeclaredException() =
          runStreamInitialDeclaredException(client, *instruction.request());
      break;
    }
    case ClientInstruction::Type::streamInitialUndeclaredException: {
      auto instruction = *clientInstruction.streamInitialUndeclaredException();
      result.streamInitialUndeclaredException() =
          runStreamInitialUndeclaredException(client, *instruction.request());
      break;
    }
    case ClientInstruction::Type::sinkBasic: {
      auto instruction = *clientInstruction.sinkBasic();
      result.sinkBasic() =
          runSinkBasic(client, instruction, *instruction.request());
      break;
    }
    case ClientInstruction::Type::sinkChunkTimeout: {
      auto instruction = *clientInstruction.sinkChunkTimeout();
      result.sinkChunkTimeout() =
          runSinkChunkTimeout(client, instruction, *instruction.request());
      break;
    }
    case ClientInstruction::Type::sinkInitialResponse: {
      auto instruction = *clientInstruction.sinkInitialResponse();
      result.sinkInitialResponse() =
          runSinkInitialResponse(client, instruction, *instruction.request());
      break;
    }
    case ClientInstruction::Type::sinkDeclaredException: {
      auto instruction = *clientInstruction.sinkDeclaredException();
      result.sinkDeclaredException() =
          runSinkDeclaredException(client, instruction, *instruction.request());
      break;
    }
    case ClientInstruction::Type::sinkUndeclaredException: {
      auto instruction = *clientInstruction.sinkUndeclaredException();
      result.sinkUndeclaredException() = runSinkUndeclaredException(
          client, instruction, *instruction.request());
      break;
    }
    case ClientInstruction::Type::interactionConstructor:
      result.interactionConstructor() = runInteractionConstructor(
          client, *clientInstruction.interactionConstructor());
      break;
    case ClientInstruction::Type::interactionFactoryFunction:
      result.interactionFactoryFunction() = runInteractionFactoryFunction(
          client, *clientInstruction.interactionFactoryFunction());
      break;
    case ClientInstruction::Type::interactionPersistsState:
      result.interactionPersistsState() = runInteractionPersistsState(
          client, *clientInstruction.interactionPersistsState());
      break;
    case ClientInstruction::Type::interactionTermination:
      result.interactionTermination() = runInteractionTermination(
          client, *clientInstruction.interactionTermination());
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

  if (!equal(actualServerResult, *rpc.serverTestResult())) {
    return testing::AssertionFailure()
        << "\nExpected server result: " << jsonify(*rpc.serverTestResult())
        << "\nActual server result: " << jsonify(actualServerResult);
  }
  return testing::AssertionSuccess();
}

testing::AssertionResult runStatelessRpcTest(
    Client<RPCStatelessConformanceService>& client, const RpcTestCase& rpc) {
  ClientTestResult result;
  ServerTestResult actualServerTestResult;
  const auto& clientInstruction = *rpc.clientInstruction();
  const auto& serverInstruction = *rpc.serverInstruction();
  try {
    switch (clientInstruction.getType()) {
      case ClientInstruction::Type::requestResponseBasic:
        actualServerTestResult = runRequestResponseBasic(
            client, *clientInstruction.requestResponseBasic());

        if (actualServerTestResult != *rpc.serverTestResult()) {
          return testing::AssertionFailure();
        } else {
          return testing::AssertionSuccess();
        }
        break;
      case ClientInstruction::Type::requestResponseDeclaredException:
        result.requestResponseDeclaredException() =
            runRequestResponseDeclaredException(client, serverInstruction);
        break;
      case ClientInstruction::Type::requestResponseUndeclaredException:
        result.requestResponseUndeclaredException() =
            runRequestResponseUndeclaredException(client, serverInstruction);
        break;
      case ClientInstruction::Type::requestResponseNoArgVoidResponse:
        result.requestResponseNoArgVoidResponse() =
            runRequestResponseNoArgVoidResponse(client);
        break;
      case ClientInstruction::Type::streamBasic: {
        auto instruction = *clientInstruction.streamBasic();
        result.streamBasic() =
            runStreamBasic(client, instruction, serverInstruction);
        break;
      }
      case ClientInstruction::Type::streamInitialResponse: {
        auto instruction = *clientInstruction.streamInitialResponse();
        result.streamInitialResponse() =
            runStreamInitialResponse(client, serverInstruction);
        break;
      }
      case ClientInstruction::Type::streamDeclaredException: {
        auto instruction = *clientInstruction.streamDeclaredException();
        result.streamDeclaredException() =
            runStreamDeclaredException(client, serverInstruction);
        break;
      }
      case ClientInstruction::Type::streamUndeclaredException: {
        auto instruction = *clientInstruction.streamUndeclaredException();
        result.streamUndeclaredException() =
            runStreamUndeclaredException(client, serverInstruction);
        break;
      }
      case ClientInstruction::Type::streamInitialDeclaredException: {
        auto instruction = *clientInstruction.streamInitialDeclaredException();
        result.streamInitialDeclaredException() =
            runStreamInitialDeclaredException(client, serverInstruction);
        break;
      }
      case ClientInstruction::Type::streamInitialUndeclaredException: {
        auto instruction =
            *clientInstruction.streamInitialUndeclaredException();
        result.streamInitialUndeclaredException() =
            runStreamInitialUndeclaredException(client, serverInstruction);
        break;
      }
      case ClientInstruction::Type::sinkBasic: {
        auto instruction = *clientInstruction.sinkBasic();
        result.sinkBasic() =
            runSinkBasic(client, instruction, serverInstruction);
        break;
      }
      case ClientInstruction::Type::sinkChunkTimeout: {
        auto instruction = *clientInstruction.sinkChunkTimeout();
        result.sinkChunkTimeout() =
            runSinkChunkTimeout(client, instruction, serverInstruction);
        break;
      }
      case ClientInstruction::Type::sinkInitialResponse: {
        auto instruction = *clientInstruction.sinkInitialResponse();
        result.sinkInitialResponse() =
            runSinkInitialResponse(client, instruction, serverInstruction);
        break;
      }
      case ClientInstruction::Type::sinkDeclaredException: {
        auto instruction = *clientInstruction.sinkDeclaredException();
        result.sinkDeclaredException() =
            runSinkDeclaredException(client, instruction, serverInstruction);
        break;
      }
      case ClientInstruction::Type::sinkUndeclaredException: {
        auto instruction = *clientInstruction.sinkUndeclaredException();
        result.sinkUndeclaredException() =
            runSinkUndeclaredException(client, instruction, serverInstruction);
        break;
      }
      case ClientInstruction::Type::interactionConstructor:
      case ClientInstruction::Type::interactionFactoryFunction:
      case ClientInstruction::Type::interactionPersistsState:
      case ClientInstruction::Type::interactionTermination:
        return testing::AssertionFailure()
            << "Interaction Tests are not supported in Stateless RPC";
      default:
        throw std::runtime_error("Invalid TestCase Type.");
    }

    if (!equal(result, *rpc.clientTestResult())) {
      return testing::AssertionFailure();
    }
    return testing::AssertionSuccess();

  } catch (const std::exception& e) {
    return testing::AssertionFailure() << "\nUnexpected Error : " << e.what();
  }
}

} // namespace apache::thrift::conformance
