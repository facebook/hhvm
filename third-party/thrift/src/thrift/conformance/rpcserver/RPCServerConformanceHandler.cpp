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

#include <folly/coro/AsyncGenerator.h>

namespace apache::thrift::conformance {

// =================== Request-Response ===================
void RPCServerConformanceHandler::requestResponseBasic(
    Response& res, std::unique_ptr<Request> req) {
  result_.requestResponseBasic().emplace().request() = *req;
  res = *testCase_->serverInstruction()->requestResponseBasic()->response();
}

void RPCServerConformanceHandler::requestResponseDeclaredException(
    std::unique_ptr<Request> req) {
  result_.requestResponseDeclaredException().emplace().request() = *req;
  throw can_throw(*testCase_->serverInstruction()
                       ->requestResponseDeclaredException()
                       ->userException());
}

void RPCServerConformanceHandler::requestResponseUndeclaredException(
    std::unique_ptr<Request> req) {
  result_.requestResponseUndeclaredException().emplace().request() = *req;
  throw std::runtime_error(*testCase_->serverInstruction()
                                ->requestResponseUndeclaredException()
                                ->exceptionMessage());
}

void RPCServerConformanceHandler::requestResponseNoArgVoidResponse() {
  result_.requestResponseNoArgVoidResponse().emplace();
}

// =================== Stream ===================
apache::thrift::ServerStream<Response> RPCServerConformanceHandler::streamBasic(
    std::unique_ptr<Request> req) {
  result_.streamBasic().emplace().request() = *req;
  for (auto& payload :
       *testCase_->serverInstruction()->streamBasic()->streamPayloads()) {
    co_yield std::move(payload);
  }
}

apache::thrift::ResponseAndServerStream<Response, Response>
RPCServerConformanceHandler::streamInitialResponse(
    std::unique_ptr<Request> req) {
  result_.streamInitialResponse().emplace().request() = *req;
  auto stream =
      folly::coro::co_invoke([&]() -> folly::coro::AsyncGenerator<Response&&> {
        for (auto& payload : *testCase_->serverInstruction()
                                  ->streamInitialResponse()
                                  ->streamPayloads()) {
          co_yield std::move(payload);
        }
      });

  return {
      *testCase_->serverInstruction()
           ->streamInitialResponse()
           ->initialResponse(),
      std::move(stream)};
}

apache::thrift::ServerStream<Response>
RPCServerConformanceHandler::streamDeclaredException(
    std::unique_ptr<Request> req) {
  result_.streamDeclaredException().emplace().request() = *req;
  throw *testCase_->serverInstruction()
      ->streamDeclaredException()
      ->userException();
  co_return;
}

apache::thrift::ServerStream<Response>
RPCServerConformanceHandler::streamUndeclaredException(
    std::unique_ptr<Request> req) {
  result_.streamUndeclaredException().emplace().request() = *req;
  throw std::runtime_error(*testCase_->serverInstruction()
                                ->streamUndeclaredException()
                                ->exceptionMessage());
  co_return;
}

apache::thrift::ServerStream<Response>
RPCServerConformanceHandler::streamInitialDeclaredException(
    std::unique_ptr<Request> req) {
  result_.streamInitialDeclaredException().emplace().request() = *req;
  throw *testCase_->serverInstruction()
      ->streamInitialDeclaredException()
      ->userException();
}

apache::thrift::ServerStream<Response>
RPCServerConformanceHandler::streamInitialUndeclaredException(
    std::unique_ptr<Request> req) {
  result_.streamInitialUndeclaredException().emplace().request() = *req;
  throw std::runtime_error(*testCase_->serverInstruction()
                                ->streamInitialUndeclaredException()
                                ->exceptionMessage());
}

// =================== Sink ===================
apache::thrift::SinkConsumer<Request, Response>
RPCServerConformanceHandler::sinkBasic(std::unique_ptr<Request> req) {
  result_.sinkBasic().emplace().request() = *req;
  return apache::thrift::SinkConsumer<Request, Response>{
      [&](folly::coro::AsyncGenerator<Request&&> gen)
          -> folly::coro::Task<Response> {
        while (auto item = co_await gen.next()) {
          result_.sinkBasic()->sinkPayloads()->push_back(std::move(*item));
        }
        co_return *testCase_->serverInstruction()->sinkBasic()->finalResponse();
      },
      static_cast<uint64_t>(
          *testCase_->serverInstruction()->sinkBasic()->bufferSize())};
}

apache::thrift::SinkConsumer<Request, Response>
RPCServerConformanceHandler::sinkChunkTimeout(std::unique_ptr<Request> req) {
  result_.sinkChunkTimeout().emplace().request() = *req;
  return apache::thrift::SinkConsumer<Request, Response>{
      [&](folly::coro::AsyncGenerator<Request&&> gen)
          -> folly::coro::Task<Response> {
        try {
          while (auto item = co_await gen.next()) {
            result_.sinkChunkTimeout()->sinkPayloads()->push_back(
                std::move(*item));
          }
        } catch (const apache::thrift::TApplicationException& e) {
          if (e.getType() ==
              TApplicationException::TApplicationExceptionType::TIMEOUT) {
            result_.sinkChunkTimeout()->chunkTimeoutException() = true;
          }
        }
        co_return *testCase_->serverInstruction()
            ->sinkChunkTimeout()
            ->finalResponse();
      }}
      .setChunkTimeout(
          std::chrono::milliseconds{*testCase_->serverInstruction()
                                         ->sinkChunkTimeout()
                                         ->chunkTimeoutMs()});
}

apache::thrift::ResponseAndSinkConsumer<Response, Request, Response>
RPCServerConformanceHandler::sinkInitialResponse(std::unique_ptr<Request> req) {
  result_.sinkInitialResponse().emplace().request() = *req;
  return {
      *testCase_->serverInstruction()->sinkInitialResponse()->initialResponse(),
      apache::thrift::SinkConsumer<Request, Response>{
          [&](folly::coro::AsyncGenerator<Request&&> gen)
              -> folly::coro::Task<Response> {
            while (auto item = co_await gen.next()) {
              result_.sinkInitialResponse()->sinkPayloads()->push_back(
                  std::move(*item));
            }
            co_return *testCase_->serverInstruction()
                ->sinkInitialResponse()
                ->finalResponse();
          },
          static_cast<uint64_t>(*testCase_->serverInstruction()
                                     ->sinkInitialResponse()
                                     ->bufferSize())}};
}

apache::thrift::SinkConsumer<Request, Response>
RPCServerConformanceHandler::sinkDeclaredException(
    std::unique_ptr<Request> req) {
  auto& result = result_.sinkDeclaredException().emplace();
  result.request() = *req;
  return {
      [&](folly::coro::AsyncGenerator<Request&&> gen)
          -> folly::coro::Task<Response> {
        try {
          std::ignore = co_await gen.next();
        } catch (const UserException& e) {
          result.userException() = e;
          throw;
        }
        throw std::logic_error("Publisher didn't throw");
      },
      static_cast<uint64_t>(*testCase_->serverInstruction()
                                 ->sinkDeclaredException()
                                 ->bufferSize())};
}

apache::thrift::SinkConsumer<Request, Response>
RPCServerConformanceHandler::sinkUndeclaredException(
    std::unique_ptr<Request> req) {
  auto& result = result_.sinkUndeclaredException().emplace();
  result.request() = *req;
  return {
      [&](folly::coro::AsyncGenerator<Request&&> gen)
          -> folly::coro::Task<Response> {
        try {
          std::ignore = co_await gen.next();
          throw std::logic_error("Publisher didn't throw");
        } catch (const TApplicationException& e) {
          result.exceptionMessage() = e.getMessage();
          throw;
        } catch (...) {
          result.exceptionMessage() =
              folly::exceptionStr(std::current_exception());
          throw;
        }
      },
      static_cast<uint64_t>(*testCase_->serverInstruction()
                                 ->sinkUndeclaredException()
                                 ->bufferSize())};
}

// =================== Interactions ===================
std::unique_ptr<RPCServerConformanceHandler::BasicInteractionIf>
RPCServerConformanceHandler::createBasicInteraction() {
  switch (testCase_->serverInstruction()->getType()) {
    case ServerInstruction::Type::interactionConstructor:
      result_.interactionConstructor().emplace().constructorCalled() = true;
      break;
    case ServerInstruction::Type::interactionPersistsState:
      result_.interactionPersistsState().emplace();
      break;
    case ServerInstruction::Type::interactionTermination:
      result_.interactionTermination().emplace();
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
      result_.interactionFactoryFunction().emplace().initialSum() = initialSum;
      break;
    case ServerInstruction::Type::interactionPersistsState:
      result_.interactionPersistsState().emplace();
      break;
    case ServerInstruction::Type::interactionTermination:
      result_.interactionTermination().emplace();
      break;
    default:
      throw std::runtime_error(
          "BasicInteraction factory function called unexpectedly");
  }
  return {std::make_unique<BasicInteraction>(*testCase_, result_, initialSum)};
}

} // namespace apache::thrift::conformance
