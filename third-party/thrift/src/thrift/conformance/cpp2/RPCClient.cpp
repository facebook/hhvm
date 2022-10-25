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

#include <exception>
#include <glog/logging.h>
#include <folly/init/Init.h>

#include <folly/experimental/coro/AsyncGenerator.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/Sleep.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>

#include <thrift/conformance/if/gen-cpp2/RPCConformanceService.h>
#include <thrift/conformance/if/gen-cpp2/rpc_types.h>
#include <thrift/lib/cpp/transport/TTransport.h>
#include <thrift/lib/cpp2/async/PooledRequestChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>

DEFINE_int32(port, 7777, "Port for Conformance Verification Server");

using namespace apache::thrift;
using namespace apache::thrift::conformance;
using apache::thrift::transport::TTransportException;

std::unique_ptr<Client<RPCConformanceService>> createClient() {
  return std::make_unique<Client<RPCConformanceService>>(
      PooledRequestChannel::newChannel([](folly::EventBase& eb) {
        return RocketClientChannel::newChannel(
            folly::AsyncTransport::UniquePtr(new folly::AsyncSocket(
                &eb, folly::SocketAddress("::1", FLAGS_port))));
      }));
}

template <class InteractionClientInstruction>
Client<RPCConformanceService>::BasicInteraction createInteraction(
    const InteractionClientInstruction& instruction) {
  auto client = createClient();
  if (instruction.initialSum().has_value()) {
    return folly::coro::blockingWait(
        client->co_basicInteractionFactoryFunction(*instruction.initialSum()));
  } else {
    return client->createBasicInteraction();
  }
}

// =================== Request-Response ===================
RequestResponseBasicClientTestResult runRequestResponseBasicTest(
    const RequestResponseBasicClientInstruction& instruction) {
  RequestResponseBasicClientTestResult result;
  auto client = createClient();
  client->sync_requestResponseBasic(
      result.response().emplace(), *instruction.request());
  return result;
}

RequestResponseDeclaredExceptionClientTestResult
requestResponseDeclaredExceptionTest(
    const RequestResponseDeclaredExceptionClientInstruction& instruction) {
  RequestResponseDeclaredExceptionClientTestResult result;
  auto client = createClient();
  try {
    client->sync_requestResponseDeclaredException(*instruction.request());
  } catch (const UserException& e) {
    result.userException() = e;
  }
  return result;
}

RequestResponseUndeclaredExceptionClientTestResult
requestResponseUndeclaredExceptionTest(
    const RequestResponseUndeclaredExceptionClientInstruction& instruction) {
  RequestResponseUndeclaredExceptionClientTestResult result;
  auto client = createClient();
  try {
    client->sync_requestResponseUndeclaredException(*instruction.request());
  } catch (const TApplicationException& e) {
    result.exceptionMessage() = e.getMessage();
  }
  return result;
}

RequestResponseNoArgVoidResponseClientTestResult
requestResponseNoArgVoidResponseTest(
    const RequestResponseNoArgVoidResponseClientInstruction&) {
  RequestResponseNoArgVoidResponseClientTestResult result;
  auto client = createClient();
  client->sync_requestResponseNoArgVoidResponse();
  return result;
}

RequestResponseTimeoutClientTestResult runRequestResponseTimeoutTest(
    const RequestResponseTimeoutClientInstruction& instruction) {
  RequestResponseTimeoutClientTestResult result;
  Response response;
  apache::thrift::RpcOptions rpcOptions{};
  rpcOptions.setTimeout(std::chrono::milliseconds{*instruction.timeoutMs()});
  auto client = createClient();
  try {
    client->sync_requestResponseTimeout(
        rpcOptions, response, *instruction.request());
  } catch (const TTransportException& e) {
    if (e.getType() ==
        TTransportException::TTransportExceptionType::TIMED_OUT) {
      result.timeoutException() = true;
    }
  }
  return result;
}

// =================== Stream ===================
StreamBasicClientTestResult streamBasicTest(
    StreamBasicClientInstruction& instruction) {
  auto client = createClient();
  apache::thrift::RpcOptions rpcOptions{};
  rpcOptions.setChunkBufferSize(*instruction.bufferSize());
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<StreamBasicClientTestResult> {
        auto gen = (co_await client->co_streamBasic(
                        rpcOptions, *instruction.request()))
                       .toAsyncGenerator();
        StreamBasicClientTestResult result;
        while (auto val = co_await gen.next()) {
          result.streamPayloads()->push_back(std::move(*val));
        }
        co_return result;
      }());
}

StreamChunkTimeoutClientTestResult streamChunkTimeoutTest(
    StreamChunkTimeoutClientInstruction& instruction) {
  auto client = createClient();
  StreamChunkTimeoutClientTestResult result;
  apache::thrift::RpcOptions rpcOptions{};
  rpcOptions.setChunkTimeout(
      std::chrono::milliseconds{*instruction.chunkTimeoutMs()});
  try {
    folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
      auto gen = (co_await client->co_streamChunkTimeout(
                      rpcOptions, *instruction.request()))
                     .toAsyncGenerator();
      while (auto val = co_await gen.next()) {
        result.streamPayloads()->push_back(std::move(*val));
      }
    }());
  } catch (const TTransportException& e) {
    if (e.getType() ==
        TTransportException::TTransportExceptionType::TIMED_OUT) {
      result.chunkTimeoutException() = true;
    }
  }
  return result;
}

StreamInitialResponseClientTestResult streamInitialResponseTest(
    const StreamInitialResponseClientInstruction& instruction) {
  auto client = createClient();
  StreamInitialResponseClientTestResult testResult;
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    auto result =
        co_await client->co_streamInitialResponse(*instruction.request());
    testResult.initialResponse() = std::move(result.response);
    auto gen = std::move(result.stream).toAsyncGenerator();
    while (auto val = co_await gen.next()) {
      testResult.streamPayloads()->push_back(std::move(*val));
    }
  }());
  return testResult;
}

StreamCreditTimeoutClientTestResult streamCreditTimeoutTest(
    const StreamCreditTimeoutClientInstruction& instruction) {
  auto client = createClient();
  StreamCreditTimeoutClientTestResult result;
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    auto gen = (co_await client->co_streamCreditTimeout(
                    RpcOptions().setChunkBufferSize(0), *instruction.request()))
                   .toAsyncGenerator();
    try {
      co_await gen.next();
      // Sleep longer than the stream expiration time, so that the server
      // will run out of credit and throw a credit timeout.
      co_await folly::coro::sleep(
          std::chrono::milliseconds{*instruction.creditTimeoutMs()});
      co_await gen.next();
    } catch (const TApplicationException& e) {
      if (e.getType() ==
          TApplicationException::TApplicationExceptionType::TIMEOUT) {
        result.creditTimeoutException() = true;
      }
    }
  }());
  return result;
}

StreamDeclaredExceptionClientTestResult streamDeclaredExceptionTest(
    StreamDeclaredExceptionClientInstruction& instruction) {
  auto client = createClient();
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<StreamDeclaredExceptionClientTestResult> {
        auto gen = (co_await client->co_streamDeclaredException(
                        *instruction.request()))
                       .toAsyncGenerator();
        StreamDeclaredExceptionClientTestResult result;
        try {
          co_await gen.next();
        } catch (const UserException& e) {
          result.userException() = e;
        }
        co_return result;
      }());
}

StreamUndeclaredExceptionClientTestResult streamUndeclaredExceptionTest(
    StreamUndeclaredExceptionClientInstruction& instruction) {
  auto client = createClient();
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<StreamUndeclaredExceptionClientTestResult> {
        auto gen = (co_await client->co_streamUndeclaredException(
                        *instruction.request()))
                       .toAsyncGenerator();
        StreamUndeclaredExceptionClientTestResult result;
        try {
          co_await gen.next();
        } catch (const TApplicationException& e) {
          result.exceptionMessage() = e.getMessage();
        }
        co_return result;
      }());
}

// =================== Sink ===================
SinkBasicClientTestResult sinkBasicTest(
    SinkBasicClientInstruction& instruction) {
  auto client = createClient();
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<SinkBasicClientTestResult> {
        auto sink = co_await client->co_sinkBasic(*instruction.request());
        auto finalResponse =
            co_await sink.sink([&]() -> folly::coro::AsyncGenerator<Request&&> {
              for (auto& payload : *instruction.sinkPayloads()) {
                co_yield std::move(payload);
              }
            }());
        SinkBasicClientTestResult result;
        result.finalResponse() = std::move(finalResponse);
        co_return result;
      }());
}

// =================== Interactions ===================
InteractionConstructorClientTestResult interactionConstructorTest(
    InteractionConstructorClientInstruction&) {
  auto client = createClient();
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<InteractionConstructorClientTestResult> {
        auto interaction = client->createBasicInteraction();
        co_await interaction.co_init();
        co_return InteractionConstructorClientTestResult();
      }());
}

InteractionFactoryFunctionClientTestResult interactionFactoryFunctionTest(
    InteractionFactoryFunctionClientInstruction& instruction) {
  auto client = createClient();
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<InteractionFactoryFunctionClientTestResult> {
        co_await client->co_basicInteractionFactoryFunction(
            *instruction.initialSum());
        co_return InteractionFactoryFunctionClientTestResult();
      }());
}

InteractionPersistsStateClientTestResult interactionPersistsStateTest(
    InteractionPersistsStateClientInstruction& instruction) {
  auto client = createClient();
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<InteractionPersistsStateClientTestResult> {
        auto interaction = createInteraction(instruction);
        InteractionPersistsStateClientTestResult result;
        for (auto& arg : *instruction.valuesToAdd()) {
          result.responses()->emplace_back(co_await interaction.co_add(arg));
        }
        co_return result;
      }());
}

InteractionTerminationClientTestResult interactionTerminationTest(
    InteractionTerminationClientInstruction& instruction) {
  auto client = createClient();
  return folly::coro::blockingWait(
      [&]() -> folly::coro::Task<InteractionTerminationClientTestResult> {
        auto interaction = createInteraction(instruction);
        co_await interaction.co_init();
        co_return InteractionTerminationClientTestResult();
      }());
}

int main(int argc, char** argv) {
  folly::init(&argc, &argv);

  RpcTestCase testCase;
  auto client = createClient();
  client->sync_getTestCase(testCase);
  auto& clientInstruction = *testCase.clientInstruction_ref();

  ClientTestResult result;
  switch (clientInstruction.getType()) {
    case ClientInstruction::Type::requestResponseBasic:
      result.requestResponseBasic_ref() = runRequestResponseBasicTest(
          *clientInstruction.requestResponseBasic_ref());
      break;
    case ClientInstruction::Type::requestResponseDeclaredException:
      result.requestResponseDeclaredException_ref() =
          requestResponseDeclaredExceptionTest(
              *clientInstruction.requestResponseDeclaredException_ref());
      break;
    case ClientInstruction::Type::requestResponseUndeclaredException:
      result.requestResponseUndeclaredException_ref() =
          requestResponseUndeclaredExceptionTest(
              *clientInstruction.requestResponseUndeclaredException_ref());
      break;
    case ClientInstruction::Type::requestResponseNoArgVoidResponse:
      result.requestResponseNoArgVoidResponse_ref() =
          requestResponseNoArgVoidResponseTest(
              *clientInstruction.requestResponseNoArgVoidResponse_ref());
      break;
    case ClientInstruction::Type::requestResponseTimeout:
      result.requestResponseTimeout_ref() = runRequestResponseTimeoutTest(
          *clientInstruction.requestResponseTimeout_ref());
      break;
    case ClientInstruction::Type::streamBasic:
      result.streamBasic_ref() =
          streamBasicTest(*clientInstruction.streamBasic_ref());
      break;
    case ClientInstruction::Type::streamChunkTimeout:
      result.streamChunkTimeout_ref() =
          streamChunkTimeoutTest(*clientInstruction.streamChunkTimeout_ref());
      break;
    case ClientInstruction::Type::streamInitialResponse:
      result.streamInitialResponse_ref() = streamInitialResponseTest(
          *clientInstruction.streamInitialResponse_ref());
      break;
    case ClientInstruction::Type::streamCreditTimeout:
      result.streamCreditTimeout_ref() =
          streamCreditTimeoutTest(*clientInstruction.streamCreditTimeout_ref());
      break;
    case ClientInstruction::Type::streamDeclaredException:
      result.streamDeclaredException_ref() = streamDeclaredExceptionTest(
          *clientInstruction.streamDeclaredException_ref());
      break;
    case ClientInstruction::Type::streamUndeclaredException:
      result.streamUndeclaredException_ref() = streamUndeclaredExceptionTest(
          *clientInstruction.streamUndeclaredException_ref());
      break;
    case ClientInstruction::Type::sinkBasic:
      result.sinkBasic_ref() =
          sinkBasicTest(*clientInstruction.sinkBasic_ref());
      break;
    case ClientInstruction::Type::interactionConstructor:
      result.interactionConstructor_ref() = interactionConstructorTest(
          *clientInstruction.interactionConstructor_ref());
      break;
    case ClientInstruction::Type::interactionFactoryFunction:
      result.interactionFactoryFunction_ref() = interactionFactoryFunctionTest(
          *clientInstruction.interactionFactoryFunction_ref());
      break;
    case ClientInstruction::Type::interactionPersistsState:
      result.interactionPersistsState_ref() = interactionPersistsStateTest(
          *clientInstruction.interactionPersistsState_ref());
      break;
    case ClientInstruction::Type::interactionTermination:
      result.interactionTermination_ref() = interactionTerminationTest(
          *clientInstruction.interactionTermination_ref());
      break;
    default:
      throw std::runtime_error("Invalid TestCase Type.");
  }

  client->sync_sendTestResult(result);
  return 0;
}
