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

#include <thrift/conformance/rpcserver/RPCServerConformanceHandler.h>

#include <chrono>
#include <exception>

#include <folly/experimental/coro/AsyncGenerator.h>

namespace apache::thrift::conformance {

// =================== Request-Response ===================
void RPCServerConformanceHandler::requestResponseBasic(
    Response& res, std::unique_ptr<Request> req) {
  result_.requestResponseBasic_ref().emplace().request() = *req;
  res = *testCase_->serverInstruction_ref()
             ->requestResponseBasic_ref()
             ->response();
}

void RPCServerConformanceHandler::requestResponseDeclaredException(
    std::unique_ptr<Request> req) {
  result_.requestResponseDeclaredException_ref().emplace().request() = *req;
  throw can_throw(*testCase_->serverInstruction()
                       ->requestResponseDeclaredException_ref()
                       ->userException());
}

void RPCServerConformanceHandler::requestResponseUndeclaredException(
    std::unique_ptr<Request> req) {
  result_.requestResponseUndeclaredException_ref().emplace().request() = *req;
  throw std::runtime_error(*testCase_->serverInstruction()
                                ->requestResponseUndeclaredException_ref()
                                ->exceptionMessage());
}

void RPCServerConformanceHandler::requestResponseNoArgVoidResponse() {
  result_.requestResponseNoArgVoidResponse_ref().emplace();
}

// =================== Stream ===================
apache::thrift::ServerStream<Response> RPCServerConformanceHandler::streamBasic(
    std::unique_ptr<Request> req) {
  result_.streamBasic_ref().emplace().request() = *req;
  for (auto& payload :
       *testCase_->serverInstruction()->streamBasic_ref()->streamPayloads()) {
    co_yield std::move(payload);
  }
}

apache::thrift::ResponseAndServerStream<Response, Response>
RPCServerConformanceHandler::streamInitialResponse(
    std::unique_ptr<Request> req) {
  result_.streamInitialResponse_ref().emplace().request() = *req;
  auto stream =
      folly::coro::co_invoke([&]() -> folly::coro::AsyncGenerator<Response&&> {
        for (auto& payload : *testCase_->serverInstruction()
                                  ->streamInitialResponse_ref()
                                  ->streamPayloads()) {
          co_yield std::move(payload);
        }
      });

  return {
      *testCase_->serverInstruction()
           ->streamInitialResponse_ref()
           ->initialResponse(),
      std::move(stream)};
}

apache::thrift::ServerStream<Response>
RPCServerConformanceHandler::streamDeclaredException(
    std::unique_ptr<Request> req) {
  result_.streamDeclaredException_ref().emplace().request() = *req;
  throw *testCase_->serverInstruction()
      ->streamDeclaredException_ref()
      ->userException();
  co_return;
}

apache::thrift::ServerStream<Response>
RPCServerConformanceHandler::streamUndeclaredException(
    std::unique_ptr<Request> req) {
  result_.streamUndeclaredException_ref().emplace().request() = *req;
  throw std::runtime_error(*testCase_->serverInstruction()
                                ->streamUndeclaredException_ref()
                                ->exceptionMessage());
  co_return;
}

apache::thrift::ServerStream<Response>
RPCServerConformanceHandler::streamInitialDeclaredException(
    std::unique_ptr<Request> req) {
  result_.streamInitialDeclaredException_ref().emplace().request() = *req;
  throw *testCase_->serverInstruction()
      ->streamInitialDeclaredException_ref()
      ->userException();
}

apache::thrift::ServerStream<Response>
RPCServerConformanceHandler::streamInitialUndeclaredException(
    std::unique_ptr<Request> req) {
  result_.streamInitialUndeclaredException_ref().emplace().request() = *req;
  throw std::runtime_error(*testCase_->serverInstruction()
                                ->streamInitialUndeclaredException_ref()
                                ->exceptionMessage());
}

// =================== Sink ===================
apache::thrift::SinkConsumer<Request, Response>
RPCServerConformanceHandler::sinkBasic(std::unique_ptr<Request> req) {
  result_.sinkBasic_ref().emplace().request() = *req;
  return apache::thrift::SinkConsumer<Request, Response>{
      [&](folly::coro::AsyncGenerator<Request&&> gen)
          -> folly::coro::Task<Response> {
        while (auto item = co_await gen.next()) {
          result_.sinkBasic_ref()->sinkPayloads()->push_back(std::move(*item));
        }
        co_return *testCase_->serverInstruction()
            ->sinkBasic_ref()
            ->finalResponse();
      },
      static_cast<uint64_t>(
          *testCase_->serverInstruction()->sinkBasic_ref()->bufferSize())};
}

apache::thrift::SinkConsumer<Request, Response>
RPCServerConformanceHandler::sinkChunkTimeout(std::unique_ptr<Request> req) {
  result_.sinkChunkTimeout_ref().emplace().request() = *req;
  return apache::thrift::SinkConsumer<Request, Response>{
      [&](folly::coro::AsyncGenerator<Request&&> gen)
          -> folly::coro::Task<Response> {
        try {
          while (auto item = co_await gen.next()) {
            result_.sinkChunkTimeout_ref()->sinkPayloads()->push_back(
                std::move(*item));
          }
        } catch (const apache::thrift::TApplicationException& e) {
          if (e.getType() ==
              TApplicationException::TApplicationExceptionType::TIMEOUT) {
            result_.sinkChunkTimeout_ref()->chunkTimeoutException() = true;
          }
        }
        co_return *testCase_->serverInstruction()
            ->sinkChunkTimeout_ref()
            ->finalResponse();
      }}
      .setChunkTimeout(std::chrono::milliseconds{*testCase_->serverInstruction()
                                                      ->sinkChunkTimeout_ref()
                                                      ->chunkTimeoutMs()});
}

// =================== Interactions ===================
std::unique_ptr<RPCServerConformanceHandler::BasicInteractionIf>
RPCServerConformanceHandler::createBasicInteraction() {
  switch (testCase_->serverInstruction()->getType()) {
    case ServerInstruction::Type::interactionConstructor:
      result_.interactionConstructor_ref().emplace().constructorCalled() = true;
      break;
    case ServerInstruction::Type::interactionPersistsState:
      result_.interactionPersistsState_ref().emplace();
      break;
    case ServerInstruction::Type::interactionTermination:
      result_.interactionTermination_ref().emplace();
      break;
    default:
      throw std::runtime_error(
          "BasicInteraction constructor called unexpectedly");
  }
  return std::make_unique<BasicInteraction>(*testCase_, result_);
}

apache::thrift::
    TileAndResponse<RPCServerConformanceHandler::BasicInteractionIf, void>
    RPCServerConformanceHandler::basicInteractionFactoryFunction(
        int32_t initialSum) {
  switch (testCase_->serverInstruction()->getType()) {
    case ServerInstruction::Type::interactionFactoryFunction:
      result_.interactionFactoryFunction_ref().emplace().initialSum() =
          initialSum;
      break;
    case ServerInstruction::Type::interactionPersistsState:
      result_.interactionPersistsState_ref().emplace();
      break;
    case ServerInstruction::Type::interactionTermination:
      result_.interactionTermination_ref().emplace();
      break;
    default:
      throw std::runtime_error(
          "BasicInteraction factory function called unexpectedly");
  }
  return {std::make_unique<BasicInteraction>(*testCase_, result_, initialSum)};
}

} // namespace apache::thrift::conformance
