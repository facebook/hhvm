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

#include <thrift/conformance/rpcclient/GTestHarnessRPCClient.h>

#include <memory>
#include <stdexcept>

#include <fmt/core.h>
#include <folly/Subprocess.h>
#include <folly/experimental/coro/AsyncGenerator.h>
#include <folly/experimental/coro/Sleep.h>
#include <folly/futures/Future.h>
#include <thrift/conformance/RpcStructComparator.h>
#include <thrift/conformance/Utils.h>
#include <thrift/conformance/if/gen-cpp2/RPCConformanceService.h>
#include <thrift/lib/cpp2/async/Sink.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

namespace apache::thrift::conformance {

class ConformanceVerificationServer
    : public apache::thrift::ServiceHandler<RPCConformanceService> {
 public:
  explicit ConformanceVerificationServer(const RpcTestCase& testCase)
      : testCase_(testCase) {}

  void getTestCase(RpcTestCase& testCase) override {
    testCase = testCase_;
    getTestReceivedPromise_.setValue();
  }

  void sendTestResult(std::unique_ptr<ClientTestResult> result) override {
    clientResultPromise_.setValue(*result);
  }

  // =================== Request-Response ===================
  void requestResponseBasic(
      Response& res, std::unique_ptr<Request> req) override {
    res =
        *testCase_.serverInstruction()->requestResponseBasic_ref()->response();
    serverResult_.requestResponseBasic_ref().emplace().request() = *req;
  }

  void requestResponseDeclaredException(std::unique_ptr<Request> req) override {
    serverResult_.requestResponseDeclaredException_ref().emplace().request() =
        *req;
    throw can_throw(*testCase_.serverInstruction()
                         ->requestResponseDeclaredException_ref()
                         ->userException());
  }

  void requestResponseUndeclaredException(
      std::unique_ptr<Request> req) override {
    serverResult_.requestResponseUndeclaredException_ref().emplace().request() =
        *req;
    throw std::runtime_error(*testCase_.serverInstruction()
                                  ->requestResponseUndeclaredException_ref()
                                  ->exceptionMessage());
  }

  void requestResponseNoArgVoidResponse() override {
    serverResult_.requestResponseNoArgVoidResponse_ref().emplace();
  }

  // =================== Stream ===================
  apache::thrift::ServerStream<Response> streamBasic(
      std::unique_ptr<Request> req) override {
    serverResult_.streamBasic_ref().emplace().request() = *req;
    for (auto payload :
         *testCase_.serverInstruction()->streamBasic_ref()->streamPayloads()) {
      co_yield std::move(payload);
    }
  }

  apache::thrift::ServerStream<Response> streamChunkTimeout(
      std::unique_ptr<Request> req) override {
    serverResult_.streamChunkTimeout_ref().emplace().request() = *req;
    for (auto payload : *testCase_.serverInstruction()
                             ->streamChunkTimeout_ref()
                             ->streamPayloads()) {
      co_yield std::move(payload);
    }
    co_await folly::coro::sleep(
        std::chrono::milliseconds(*testCase_.serverInstruction()
                                       ->streamChunkTimeout_ref()
                                       ->chunkTimeoutMs()));
  }

  apache::thrift::ResponseAndServerStream<Response, Response>
  streamInitialResponse(std::unique_ptr<Request> req) override {
    serverResult_.streamInitialResponse_ref().emplace().request() = *req;
    auto stream = folly::coro::co_invoke(
        [&]() -> folly::coro::AsyncGenerator<Response&&> {
          for (auto payload : *testCase_.serverInstruction()
                                   ->streamInitialResponse_ref()
                                   ->streamPayloads()) {
            co_yield std::move(payload);
          }
        });

    return {
        *testCase_.serverInstruction()
             ->streamInitialResponse_ref()
             ->initialResponse(),
        std::move(stream)};
  }

  // =================== Sink ===================
  apache::thrift::SinkConsumer<Request, Response> sinkBasic(
      std::unique_ptr<Request> req) override {
    serverResult_.sinkBasic_ref().emplace().request() = *req;
    return apache::thrift::SinkConsumer<Request, Response>{
        [&](folly::coro::AsyncGenerator<Request&&> gen)
            -> folly::coro::Task<Response> {
          while (auto item = co_await gen.next()) {
            serverResult_.sinkBasic_ref()->sinkPayloads()->push_back(
                std::move(*item));
          }
          co_return *testCase_.serverInstruction()
              ->sinkBasic_ref()
              ->finalResponse();
        },
        static_cast<uint64_t>(
            *testCase_.serverInstruction()->sinkBasic_ref()->bufferSize())};
  }

  folly::SemiFuture<folly::Unit> getTestReceived() {
    return getTestReceivedPromise_.getSemiFuture();
  }

  folly::SemiFuture<ClientTestResult> clientResult() {
    return clientResultPromise_.getSemiFuture();
  }

  const ServerTestResult& serverResult() { return serverResult_; }

 private:
  const RpcTestCase& testCase_;
  folly::Promise<folly::Unit> getTestReceivedPromise_;
  folly::Promise<ClientTestResult> clientResultPromise_;
  ServerTestResult serverResult_;
};

class RPCClientConformanceTest : public testing::Test {
 public:
  RPCClientConformanceTest(
      std::string_view clientCmd, const TestCase* testCase, bool conforming)
      : testCase_(*testCase),
        conforming_(conforming),
        handler_(std::make_shared<ConformanceVerificationServer>(
            *testCase_.rpc_ref())),
        server_(handler_) {
    clientProcess_ = folly::Subprocess(std::vector<std::string>{
        std::string(clientCmd),
        "--port",
        folly::to<std::string>(server_.getPort())});
  }

 protected:
  void TestBody() override {
    // Wait for client to fetch test case
    bool getTestReceived =
        handler_->getTestReceived().wait(std::chrono::seconds(1));
    EXPECT_EQ(conforming_, getTestReceived);

    // End test if client was unable to fetch test case
    if (!getTestReceived) {
      return;
    }

    // Wait for result from client
    folly::Try<ClientTestResult> actualClientResult =
        handler_->clientResult().within(std::chrono::seconds(5)).getTry();

    // End test if result was not received
    if (actualClientResult.hasException()) {
      EXPECT_FALSE(conforming_);
      return;
    }

    auto& expectedClientResult = *testCase_.rpc_ref()->clientTestResult();
    if (!equal(*actualClientResult, expectedClientResult)) {
      EXPECT_FALSE(conforming_);
    }

    auto& actualServerResult = handler_->serverResult();
    auto& expectedServerResult = *testCase_.rpc_ref()->serverTestResult();
    if (actualServerResult != expectedServerResult) {
      EXPECT_FALSE(conforming_);
    }

    EXPECT_TRUE(conforming_);
  }

  void TearDown() override {
    clientProcess_.sendSignal(SIGINT);
    clientProcess_.waitOrTerminateOrKill(
        std::chrono::seconds(1), std::chrono::seconds(1));
  }

 private:
  const TestCase& testCase_;
  bool conforming_;
  std::shared_ptr<ConformanceVerificationServer> handler_;
  apache::thrift::ScopedServerInterfaceThread server_;
  folly::Subprocess clientProcess_;
};

void registerTests(
    std::string_view category,
    const TestSuite* suite,
    const std::set<std::string>& nonconforming,
    std::string_view clientCmd,
    const char* file,
    int line) {
  for (const auto& test : *suite->tests()) {
    for (const auto& testCase : *test.testCases()) {
      std::string suiteName =
          fmt::format("{}/{}/{}", category, *suite->name(), *testCase.name());
      std::string fullName = fmt::format("{}.{}", suiteName, *test.name());
      bool conforming = nonconforming.find(fullName) == nonconforming.end();
      registerTest(
          suiteName.c_str(),
          test.name()->c_str(),
          nullptr,
          conforming ? nullptr : "nonconforming",
          file,
          line,
          [&testCase, clientCmd, conforming]() {
            return new RPCClientConformanceTest(
                clientCmd, &testCase, conforming);
          });
    }
  }
}

} // namespace apache::thrift::conformance
