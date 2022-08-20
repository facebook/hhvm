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

#pragma once

#include <thrift/conformance/if/gen-cpp2/RPCConformanceService.h>
#include <thrift/lib/cpp2/async/ServerStream.h>
#include <thrift/lib/cpp2/async/Sink.h>

namespace apache::thrift::conformance {

class RPCServerConformanceHandler
    : public apache::thrift::ServiceHandler<RPCConformanceService> {
 public:
  // =================== Request-Response ===================
  void requestResponseBasic(
      Response& res, std::unique_ptr<Request> req) override;

  void requestResponseDeclaredException(std::unique_ptr<Request> req) override;

  void requestResponseUndeclaredException(
      std::unique_ptr<Request> req) override;

  void requestResponseNoArgVoidResponse() override;

  // =================== Stream ===================
  apache::thrift::ServerStream<Response> streamBasic(
      std::unique_ptr<Request> req) override;

  apache::thrift::ResponseAndServerStream<Response, Response>
  streamInitialResponse(std::unique_ptr<Request> req) override;

  // =================== Sink ===================
  apache::thrift::SinkConsumer<Request, Response> sinkBasic(
      std::unique_ptr<Request> req) override;

  // =================== Test Utils ===================
  void sendTestCase(std::unique_ptr<RpcTestCase> req) override {
    testCase_ = std::move(req);
  }

  void getTestResult(ServerTestResult& res) override { res = result_; }

 private:
  std::unique_ptr<RpcTestCase> testCase_;
  ServerTestResult result_;
};

} // namespace apache::thrift::conformance
