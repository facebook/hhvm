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

} // namespace apache::thrift::conformance
