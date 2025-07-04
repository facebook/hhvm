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

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/core.h>
#include <gtest/gtest.h>
#include <folly/FileUtil.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <folly/Subprocess.h>
#include <folly/Synchronized.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/lang/Exception.h>
#include <folly/stop_watch.h>
#include <thrift/conformance/Utils.h>
#include <thrift/conformance/cpp2/AnyRegistry.h>
#include <thrift/conformance/if/gen-cpp2/ConformanceServiceAsyncClient.h>
#include <thrift/conformance/if/gen-cpp2/rpc_clients.h>
#include <thrift/conformance/if/gen-cpp2/test_suite_types.h>
#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/PooledRequestChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/op/Compare.h>
#include <thrift/lib/cpp2/protocol/Object.h>

// Registers the given conformance test suites with gtest using the
// client providers in clientFns.
#define THRIFT_CONFORMANCE_TEST(suites, clientFns, nonconforming)           \
  static ::apache::thrift::conformance::detail::ConformanceTestRegistration \
      __suite_reg_##__LINE__(                                               \
          suites, clientFns, nonconforming, __FILE__, __LINE__)

namespace apache::thrift::conformance {

// A map from name to client provider.
template <typename Client>
using client_fn_map = std::map<std::string_view, std::function<Client&()>>;

enum class ChannelType {
  Header = 1,
  Rocket,
};

// Creates a client for the localhost.
template <typename Client>
std::unique_ptr<Client> createClient(int port, ChannelType channelType) {
  return std::make_unique<Client>(PooledRequestChannel::newChannel(
      [=](folly::EventBase& eb) -> PooledRequestChannel::ImplPtr {
        folly::AsyncTransport::UniquePtr socket(
            new folly::AsyncSocket(&eb, folly::SocketAddress("::1", port)));
        switch (channelType) {
          case ChannelType::Header:
            return HeaderClientChannel::newChannel(std::move(socket));
          case ChannelType::Rocket:
            return RocketClientChannel::newChannel(std::move(socket));
          default:
            throw std::invalid_argument(
                "Unknown channel type: " + std::to_string(int(channelType)));
        }
      }));
}

// Creates a client for the localhost.
template <typename Client>
std::unique_ptr<Client> createClient(std::string_view) {
  throw std::invalid_argument("Unimplemented Method createClient");
}

// Bundles a server process and client.
template <typename Client>
class ClientAndServer {
 public:
  explicit ClientAndServer(std::string cmd, ChannelType channelType)
      : server_(
            std::vector<std::string>{std::move(cmd)},
            folly::Subprocess::Options().pipeStdout()),
        channelType_(channelType) {
    LOG(INFO) << "Starting binary: " << cmd;
    std::string port;
    server_.communicate(
        folly::Subprocess::readLinesCallback(
            [&port](int, folly::StringPiece s) {
              port = std::string(s);
              return true;
            }),
        [](int, int) { return true; });
    LOG(INFO) << "Using port: " << port;
    client_ = createClient<Client>(folly::to<int>(port), channelType_);
  }

  ~ClientAndServer() {
    server_.sendSignal(SIGINT);
    server_.waitOrTerminateOrKill(
        std::chrono::seconds(1), std::chrono::seconds(1));
  }

  Client& getClient() { return *client_; }

 private:
  folly::Subprocess server_;
  std::unique_ptr<Client> client_;
  ChannelType channelType_;
};

// Creates a map from name to client provider, using lazily initalized
// ClientAndServers.
template <typename Client>
client_fn_map<Client> getServers(ChannelType channelType) {
  auto cmds = parseCmds(getEnvOrThrow("THRIFT_CONFORMANCE_SERVER_BINARIES"));
  client_fn_map<Client> result;
  for (const auto& entry : cmds) {
    result.emplace(
        entry.first,
        [name = std::string(entry.first),
         cmd = std::string(entry.second),
         channelType]() -> Client& {
          static folly::Synchronized<std::map<
              std::string_view,
              std::unique_ptr<ClientAndServer<Client>>>>
              clients;
          auto lockedClients = clients.wlock();

          // Get or create ClientAndServer in the static map.
          auto itr = lockedClients->find(name);
          if (itr == lockedClients->end()) {
            itr = lockedClients->emplace_hint(
                itr,
                name,
                std::make_unique<ClientAndServer<Client>>(cmd, channelType));
          }
          return itr->second->getClient();
        });
  }
  auto servers = parseCmds(getEnvOrThrow("THRIFT_CONFORMANCE_SERVERS"));
  for (const auto& entry : servers) {
    result.emplace(
        entry.first,
        [name = std::string(entry.first),
         server = std::string(entry.second)]() -> Client& {
          static folly::Synchronized<
              std::map<std::string_view, std::unique_ptr<Client>>>
              clients;
          auto lockedClients = clients.wlock();

          // Get or create Client in the static map.
          auto itr = lockedClients->find(name);
          if (itr == lockedClients->end()) {
            itr = lockedClients->emplace_hint(
                itr, name, createClient<Client>(server));
          }
          return *itr->second;
        });
  }
  return result;
}

testing::AssertionResult runRoundTripTest(
    Client<ConformanceService>& client, const RoundTripTestCase& roundTrip);

testing::AssertionResult runPatchTest(
    Client<ConformanceService>& client, const PatchOpTestCase& patchTestCase);

testing::AssertionResult runRpcTest(
    Client<RPCConformanceService>& client, const RpcTestCase& rpc);

testing::AssertionResult runStatelessRpcTest(
    Client<RPCStatelessConformanceService>& client, const RpcTestCase& rpc);

template <typename Client>
class ConformanceTest : public testing::Test {
 public:
  ConformanceTest(
      Client& client,
      const TestSuite& suite,
      const conformance::Test& test,
      const TestCase& testCase,
      bool conforming)
      : client_(client),
        suite_(suite),
        test_(test),
        testCase_(testCase),
        conforming_(conforming) {}

 protected:
  void TestBody() override {
    testing::AssertionResult conforming = runTestCase(client_, testCase_);
    if (conforming_) {
      EXPECT_TRUE(conforming)
          // Most specific to least specific.
          << genDescription(testCase_) << genDescription(test_)
          << genDescription(suite_) << std::endl
          << "For more detail see:" << std::endl
          << genTagLinks(testCase_) << genTagLinks(test_)
          << genTagLinks(suite_);
    } else {
      EXPECT_FALSE(conforming)
          << "If intentional, please remove the associated entry from:"
          << std::endl
          << "    thrift/conformance/data/nonconforming.txt" << std::endl;
    }
  }

 private:
  Client& client_;
  const TestSuite& suite_;
  const conformance::Test& test_;
  const TestCase& testCase_;
  const bool conforming_;
};

// Runs a conformance test case against the given client.
template <typename Client>
testing::AssertionResult runTestCase(Client& client, const TestCase& testCase) {
  switch (testCase.test()->getType()) {
    case TestCaseUnion::Type::roundTrip:
      if constexpr (std::is_same_v<
                        Client,
                        apache::thrift::Client<ConformanceService>>) {
        return runRoundTripTest(client, *testCase.roundTrip());
      }
      return testing::AssertionFailure() << "Invalid test client.";
    case TestCaseUnion::Type::rpc:
      if constexpr (std::is_same_v<
                        Client,
                        apache::thrift::Client<RPCConformanceService>>) {
        return runRpcTest(client, *testCase.rpc());
      } else if constexpr (std::is_same_v<
                               Client,
                               apache::thrift::Client<
                                   RPCStatelessConformanceService>>) {
        return runStatelessRpcTest(client, *testCase.rpc());
      }
      return testing::AssertionFailure() << "Invalid test client.";
    case TestCaseUnion::Type::objectPatch:
      if constexpr (std::is_same_v<
                        Client,
                        apache::thrift::Client<ConformanceService>>) {
        return runPatchTest(client, *testCase.objectPatch());
      }
      return testing::AssertionFailure() << "Invalid test client.";
    default:
      return testing::AssertionFailure()
          << "Unsupported test case type: "
          << util::enumNameSafe(testCase.test()->getType());
  }
}

// Registers a test suite with gtest.
template <typename Client>
void registerTests(
    std::string_view category,
    const TestSuite& suite,
    const std::set<std::string>& nonconforming,
    std::function<Client&()> clientFn,
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
          [clientFn, &suite, &test, &testCase, conforming]() {
            return new ConformanceTest<Client>(
                clientFn(), suite, test, testCase, conforming);
          });
    }
  }
}

namespace detail {

template <typename Client>
class ConformanceTestRegistration {
 public:
  ConformanceTestRegistration(
      std::vector<TestSuite> suites,
      client_fn_map<Client> clientFns,
      const std::set<std::string>& nonconforming,
      const char* file = "",
      int line = 0)
      : suites_(std::move(suites)) {
    for (const auto& entry : clientFns) {
      for (const auto& suite : suites_) {
        registerTests<Client>(
            entry.first, suite, nonconforming, entry.second, file, line);
      }
    }
  }

 private:
  const std::vector<TestSuite> suites_;
};

} // namespace detail
} // namespace apache::thrift::conformance
