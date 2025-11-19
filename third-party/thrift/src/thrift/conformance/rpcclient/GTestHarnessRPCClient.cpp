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

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string_view>

#include <fmt/core.h>
#include <folly/Subprocess.h>
#include <folly/coro/AsyncGenerator.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Sleep.h>
#include <folly/futures/Future.h>
#include <thrift/conformance/PluggableFunctions.h>
#include <thrift/conformance/RpcStructComparator.h>
#include <thrift/conformance/Utils.h>
#include <thrift/conformance/if/gen-cpp2/RPCConformanceService.h>
#include <thrift/lib/cpp2/async/Sink.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
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
    res = *testCase_.serverInstruction()->requestResponseBasic()->response();
    serverResult_.requestResponseBasic().emplace().request() = *req;
  }

  void requestResponseDeclaredException(std::unique_ptr<Request> req) override {
    serverResult_.requestResponseDeclaredException().emplace().request() = *req;
    throw can_throw(*testCase_.serverInstruction()
                         ->requestResponseDeclaredException()
                         ->userException());
  }

  void requestResponseUndeclaredException(
      std::unique_ptr<Request> req) override {
    serverResult_.requestResponseUndeclaredException().emplace().request() =
        *req;
    throw std::runtime_error(*testCase_.serverInstruction()
                                  ->requestResponseUndeclaredException()
                                  ->exceptionMessage());
  }

  void requestResponseNoArgVoidResponse() override {
    serverResult_.requestResponseNoArgVoidResponse().emplace();
  }

  void requestResponseTimeout(
      Response&, std::unique_ptr<Request> req) override {
    serverResult_.requestResponseTimeout().emplace().request() = *req;
    folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
      co_await folly::coro::sleep(
          std::chrono::milliseconds(*testCase_.serverInstruction()
                                         ->requestResponseTimeout()
                                         ->timeoutMs()));
    }());
  }

  // =================== Stream ===================
  apache::thrift::ServerStream<Response> streamBasic(
      std::unique_ptr<Request> req) override {
    serverResult_.streamBasic().emplace().request() = *req;
    for (auto payload :
         *testCase_.serverInstruction()->streamBasic()->streamPayloads()) {
      co_yield std::move(payload);
    }
  }

  apache::thrift::ServerStream<Response> streamChunkTimeout(
      std::unique_ptr<Request> req) override {
    serverResult_.streamChunkTimeout().emplace().request() = *req;
    for (auto payload : *testCase_.serverInstruction()
                             ->streamChunkTimeout()
                             ->streamPayloads()) {
      co_yield std::move(payload);
    }
    co_await folly::coro::sleep(
        std::chrono::milliseconds(*testCase_.serverInstruction()
                                       ->streamChunkTimeout()
                                       ->chunkTimeoutMs()));
  }

  apache::thrift::ResponseAndServerStream<Response, Response>
  streamInitialResponse(std::unique_ptr<Request> req) override {
    serverResult_.streamInitialResponse().emplace().request() = *req;
    auto stream = folly::coro::co_invoke(
        [&]() -> folly::coro::AsyncGenerator<Response&&> {
          for (auto payload : *testCase_.serverInstruction()
                                   ->streamInitialResponse()
                                   ->streamPayloads()) {
            co_yield std::move(payload);
          }
        });

    return {
        *testCase_.serverInstruction()
             ->streamInitialResponse()
             ->initialResponse(),
        std::move(stream)};
  }

  apache::thrift::ServerStream<Response> streamCreditTimeout(
      std::unique_ptr<Request> req) override {
    serverResult_.streamCreditTimeout().emplace().request() = *req;
    for (auto payload : *testCase_.serverInstruction()
                             ->streamCreditTimeout()
                             ->streamPayloads()) {
      co_yield std::move(payload);
    }
  }

  apache::thrift::ServerStream<Response> streamDeclaredException(
      std::unique_ptr<Request> req) override {
    serverResult_.streamDeclaredException().emplace().request() = *req;
    throw *testCase_.serverInstruction()
        ->streamDeclaredException()
        ->userException();
    co_return;
  }

  apache::thrift::ServerStream<Response> streamUndeclaredException(
      std::unique_ptr<Request> req) override {
    serverResult_.streamUndeclaredException().emplace().request() = *req;
    throw std::runtime_error(*testCase_.serverInstruction()
                                  ->streamUndeclaredException()
                                  ->exceptionMessage());
    co_return;
  }

  apache::thrift::ServerStream<Response> streamInitialDeclaredException(
      std::unique_ptr<Request> req) override {
    serverResult_.streamInitialDeclaredException().emplace().request() = *req;
    throw *testCase_.serverInstruction()
        ->streamInitialDeclaredException()
        ->userException();
  }

  apache::thrift::ServerStream<Response> streamInitialUndeclaredException(
      std::unique_ptr<Request> req) override {
    serverResult_.streamInitialUndeclaredException().emplace().request() = *req;
    throw std::runtime_error(*testCase_.serverInstruction()
                                  ->streamInitialUndeclaredException()
                                  ->exceptionMessage());
  }

  apache::thrift::ServerStream<Response> streamInitialTimeout(
      std::unique_ptr<Request> req) override {
    serverResult_.streamInitialTimeout().emplace().request() = *req;
    std::this_thread::sleep_for(
        std::chrono::milliseconds(*testCase_.serverInstruction()
                                       ->streamInitialTimeout()
                                       ->timeoutMs()));
    return ServerStream<Response>::createEmpty();
  }

  // =================== Sink ===================
  apache::thrift::SinkConsumer<Request, Response> sinkBasic(
      std::unique_ptr<Request> req) override {
    serverResult_.sinkBasic().emplace().request() = *req;
    return apache::thrift::SinkConsumer<Request, Response>{
        [&](folly::coro::AsyncGenerator<Request&&> gen)
            -> folly::coro::Task<Response> {
          while (auto item = co_await gen.next()) {
            serverResult_.sinkBasic()->sinkPayloads()->push_back(
                std::move(*item));
          }
          co_return *testCase_.serverInstruction()
              ->sinkBasic()
              ->finalResponse();
        },
        static_cast<uint64_t>(
            *testCase_.serverInstruction()->sinkBasic()->bufferSize())};
  }

  apache::thrift::ResponseAndSinkConsumer<Response, Request, Response>
  sinkInitialResponse(std::unique_ptr<Request> req) override {
    serverResult_.sinkInitialResponse().emplace().request() = *req;

    return {
        *testCase_.serverInstruction()
             ->sinkInitialResponse()
             ->initialResponse(),
        apache::thrift::SinkConsumer<Request, Response>{
            [&](folly::coro::AsyncGenerator<Request&&> gen)
                -> folly::coro::Task<Response> {
              while (auto item = co_await gen.next()) {
                serverResult_.sinkInitialResponse()->sinkPayloads()->push_back(
                    std::move(*item));
              }
              co_return *testCase_.serverInstruction()
                  ->sinkInitialResponse()
                  ->finalResponse();
            },
            static_cast<uint64_t>(*testCase_.serverInstruction()
                                       ->sinkInitialResponse()
                                       ->bufferSize())}};
  }

  apache::thrift::SinkConsumer<Request, Response> sinkDeclaredException(
      std::unique_ptr<Request> req) override {
    auto& result = serverResult_.sinkDeclaredException().emplace();
    result.request() = *req;
    return {
        [&](folly::coro::AsyncGenerator<Request&&> gen)
            -> folly::coro::Task<Response> {
          try {
            std::ignore = co_await gen.next();
            throw std::logic_error("Publisher didn't throw");
          } catch (const UserException& e) {
            result.userException() = e;
            throw;
          } catch (...) {
            throw std::logic_error(
                fmt::format(
                    "Publisher threw undeclared exception: {}",
                    folly::exception_wrapper(std::current_exception()).what()));
          }
        },
        static_cast<uint64_t>(*testCase_.serverInstruction()
                                   ->sinkDeclaredException()
                                   ->bufferSize())};
  }

  apache::thrift::SinkConsumer<Request, Response> sinkUndeclaredException(
      std::unique_ptr<Request> req) override {
    auto& result = serverResult_.sinkUndeclaredException().emplace();
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
          }
        },
        static_cast<uint64_t>(*testCase_.serverInstruction()
                                   ->sinkUndeclaredException()
                                   ->bufferSize())};
  }
  // =================== Interactions ===================
  class BasicInteraction : public BasicInteractionIf {
   public:
    BasicInteraction(
        const RpcTestCase& testCase,
        ServerTestResult& result,
        int32_t initialSum = 0)
        : testCase_(testCase), serverResult_(result), sum_(initialSum) {}
    void init() override {}
    int32_t add(int32_t toAdd) override {
      sum_ += toAdd;
      return sum_;
    }
    folly::coro::Task<void> co_onTermination() override {
      switch (testCase_.serverInstruction()->getType()) {
        case ServerInstruction::Type::interactionTermination:
          serverResult_.interactionTermination()
              .ensure()
              .terminationReceived() = true;
          break;
        default:; // do nothing
      }
      co_return;
    }

   private:
    const RpcTestCase& testCase_;
    ServerTestResult& serverResult_;
    int32_t sum_;
  };

  std::unique_ptr<BasicInteractionIf> createBasicInteraction() override {
    switch (testCase_.serverInstruction()->getType()) {
      case ServerInstruction::Type::interactionConstructor:
        serverResult_.interactionConstructor().emplace().constructorCalled() =
            true;
        break;
      case ServerInstruction::Type::interactionPersistsState:
        serverResult_.interactionPersistsState().emplace();
        break;
      case ServerInstruction::Type::interactionTermination:
        serverResult_.interactionTermination().emplace();
        break;
      default:
        throw std::runtime_error(
            "BasicInteraction constructor called unexpectedly");
    }
    return std::make_unique<BasicInteraction>(testCase_, serverResult_);
  }

  apache::thrift::TileAndResponse<BasicInteractionIf, void>
  basicInteractionFactoryFunction(int32_t initialSum) override {
    switch (testCase_.serverInstruction()->getType()) {
      case ServerInstruction::Type::interactionFactoryFunction:
        serverResult_.interactionFactoryFunction().emplace().initialSum() =
            initialSum;
        break;
      case ServerInstruction::Type::interactionPersistsState:
        serverResult_.interactionPersistsState().emplace();
        break;
      case ServerInstruction::Type::interactionTermination:
        serverResult_.interactionTermination().emplace();
        break;
      default:
        throw std::runtime_error(
            "BasicInteraction factory function called unexpectedly");
    }
    return {std::make_unique<BasicInteraction>(
        testCase_, serverResult_, initialSum)};
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

void createClient(
    std::string_view serviceName, std::string ipAddress, std::string port) {
  auto client = create_rpc_conformance_setup_service_client_(serviceName);
  folly::coro::blockingWait(
      client->co_createRPCConformanceServiceClient(ipAddress, port));
}

class RPCClientConformanceTest : public testing::Test {
 public:
  RPCClientConformanceTest(
      std::string_view clientCmd,
      const TestSuite& suite,
      const conformance::Test& test,
      const TestCase& testCase,
      bool conforming,
      bool connectViaServer)
      : suite_(suite),
        test_(test),
        testCase_(testCase),
        conforming_(conforming),
        handler_(
            std::make_shared<ConformanceVerificationServer>(*testCase_.rpc())),
        server_(
            handler_,
            connectViaServer ? get_server_ip_addr_() : "::1",
            0,
            apache::thrift::ScopedServerInterfaceThread::ServerConfigCb(
                [&](apache::thrift::ThriftServer& server) {
                  if (connectViaServer) {
                    std::ignore = update_server_props_(server);
                  }
                })),
        connectViaServer_(connectViaServer) {
    try {
      auto port = folly::to<std::string>(server_.getPort());
      if (testCase_.rpc()->serverInstruction()->streamCreditTimeout()) {
        server_.getThriftServer().setStreamExpireTime(
            std::chrono::milliseconds{*testCase_.rpc()
                                           ->serverInstruction()
                                           ->streamCreditTimeout()
                                           ->streamExpireTime()});
      }
      if (connectViaServer_) {
        createClient(clientCmd, server_.getAddress().getAddressStr(), port);
      } else {
        auto cmd = std::string(clientCmd);
        auto args = cmd.ends_with(".jar")
            ? std::vector<std::string>{"/usr/bin/env", "java", "-jar", cmd}
            : std::vector<std::string>{cmd};
        args.push_back("--port");
        args.push_back(port);
        clientProcess_ = launch_client_process_(args);
      }
    } catch (const std::exception& e) {
      verifyConformanceResult(
          testing::AssertionFailure() << "Unexpected Error " << e.what());
    }
  }

 protected:
  void TestBody() override { verifyConformanceResult(runTest()); }

  void TearDown() override {
    if (!connectViaServer_) {
      clientProcess_.sendSignal(SIGINT);
      clientProcess_.waitOrTerminateOrKill(
          std::chrono::seconds(10), std::chrono::seconds(10));
    }
  }

 private:
  void verifyConformanceResult(testing::AssertionResult conforming) {
    if (conforming_) {
      EXPECT_TRUE(conforming) << "For more detail see:"
                              << std::endl
                              // Most specific to least specific.
                              << genTagLinks(testCase_) << genTagLinks(test_)
                              << genTagLinks(suite_);
    } else {
      EXPECT_FALSE(conforming)
          << "If intentional, please remove the associated entry from:"
          << std::endl
          // TODO: create separate nonconforming.txt file for client rpc tests
          << "    thrift/conformance/data/nonconforming.txt" << std::endl;
    }
  }
  testing::AssertionResult runTest() {
    try { // Wait for client to fetch test case
      bool getTestReceived =
          handler_->getTestReceived().wait(std::chrono::seconds(10));

      // End test if client was unable to fetch test case
      if (!getTestReceived) {
        return testing::AssertionFailure()
            << "client failed to fetch test case";
      }

      // Wait for result from client
      folly::Try<ClientTestResult> actualClientResult =
          handler_->clientResult().within(std::chrono::seconds(10)).getTry();

      // End test if result was not received
      if (actualClientResult.hasException()) {
        return testing::AssertionFailure() << actualClientResult.exception();
      }

      auto& expectedClientResult = *testCase_.rpc()->clientTestResult();
      if (!equal(*actualClientResult, expectedClientResult)) {
        return testing::AssertionFailure()
            << "\nExpected client result: " << jsonify(expectedClientResult)
            << "\nActual client result: " << jsonify(*actualClientResult);
      }

      auto& actualServerResult = handler_->serverResult();
      auto& expectedServerResult = *testCase_.rpc()->serverTestResult();
      if (!equal(actualServerResult, expectedServerResult)) {
        return testing::AssertionFailure()
            << "\nExpected server result: " << jsonify(expectedServerResult)
            << "\nActual server result: " << jsonify(actualServerResult);
      }

      return testing::AssertionSuccess();
    } catch (const std::exception& e) {
      return testing::AssertionFailure() << "Unexpected error " << e.what();
    }
  }

  const TestSuite& suite_;
  const conformance::Test& test_;
  const TestCase& testCase_;
  bool conforming_;
  std::shared_ptr<ConformanceVerificationServer> handler_;
  apache::thrift::ScopedServerInterfaceThread server_;
  folly::Subprocess clientProcess_;
  bool connectViaServer_ = false;
};

void registerTests(
    std::string_view category,
    const TestSuite& suite,
    const std::set<std::string>& nonconforming,
    std::pair<std::string_view, bool> clientCmd,
    const char* file,
    int line) {
  for (const auto& test : *suite.tests()) {
    for (const auto& testCase : *test.testCases()) {
      std::string suiteName =
          fmt::format("{}/{}/{}", category, *suite.name(), *testCase.name());
      std::string fullName = fmt::format("{}.{}", suiteName, *test.name());
      bool conforming = nonconforming.find(fullName) == nonconforming.end();
      registerTest(
          suiteName.c_str(),
          test.name()->c_str(),
          nullptr,
          conforming ? nullptr : "nonconforming",
          file,
          line,
          [clientCmd, &suite, &test, &testCase, conforming]() {
            return new RPCClientConformanceTest(
                clientCmd.first,
                suite,
                test,
                testCase,
                conforming,
                clientCmd.second);
          });
    }
  }
}

} // namespace apache::thrift::conformance
