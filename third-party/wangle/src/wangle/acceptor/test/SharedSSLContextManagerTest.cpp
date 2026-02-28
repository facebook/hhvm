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

#include <array>
#include <functional>

#include <folly/FileUtil.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <folly/testing/TestUtil.h>
#include <wangle/acceptor/FizzConfigUtil.h>
#include <wangle/acceptor/SharedSSLContextManager.h>

using namespace folly;
using namespace testing;

namespace {
// @lint-ignore-every PRIVATEKEY
static const std::string kTestCert1PEM = R"(
-----BEGIN CERTIFICATE-----
MIICFzCCAb6gAwIBAgIJAO6xBdXUFQqgMAkGByqGSM49BAEwaDELMAkGA1UEBhMC
VVMxFTATBgNVBAcMDERlZmF1bHQgQ2l0eTEcMBoGA1UECgwTRGVmYXVsdCBDb21w
YW55IEx0ZDERMA8GA1UECwwIdGVzdC5jb20xETAPBgNVBAMMCHRlc3QuY29tMCAX
DTE2MDMxNjE4MDg1M1oYDzQ3NTQwMjExMTgwODUzWjBoMQswCQYDVQQGEwJVUzEV
MBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0IENvbXBhbnkg
THRkMREwDwYDVQQLDAh0ZXN0LmNvbTERMA8GA1UEAwwIdGVzdC5jb20wWTATBgcq
hkjOPQIBBggqhkjOPQMBBwNCAARZ4vDgSPwytxU2HfQG/wxhsk0uHfr1eUmheqoC
yiQPB7aXZPbFs3JtvhzKc8DZ0rrZIQpkVLAGEIAa5UbuCy32o1AwTjAdBgNVHQ4E
FgQU05wwrHKWuyGM0qAIzeprza/FM9UwHwYDVR0jBBgwFoAU05wwrHKWuyGM0qAI
zeprza/FM9UwDAYDVR0TBAUwAwEB/zAJBgcqhkjOPQQBA0gAMEUCIBofo+kW0kxn
wzvNvopVKr/cFuDzwRKHdozoiZ492g6QAiEAo55BTcbSwBeszWR6Cr8gOCS4Oq7Z
Mt8v4GYjd1KT4fE=
-----END CERTIFICATE-----
)";

static const std::string kTestCert1Key = R"(
-----BEGIN EC PARAMETERS-----
BggqhkjOPQMBBw==
-----END EC PARAMETERS-----
-----BEGIN EC PRIVATE KEY-----
MHcCAQEEIKhuz+7RoCLvsXzcD1+Bq5ahrOViFJmgHiGR3w3OmXEroAoGCCqGSM49
AwEHoUQDQgAEWeLw4Ej8MrcVNh30Bv8MYbJNLh369XlJoXqqAsokDwe2l2T2xbNy
bb4cynPA2dK62SEKZFSwBhCAGuVG7gst9g==
-----END EC PRIVATE KEY-----
)";

auto getCertFile() {
  folly::test::TemporaryFile tmpFile("key_and_cert.pem");
  const auto tmpFilePath = folly::fs::canonical(tmpFile.path()).string();
  auto keyAndCert = kTestCert1Key + kTestCert1PEM;
  folly::writeFile(keyAndCert, tmpFilePath.c_str());
  return tmpFile;
}

wangle::SSLContextConfig getSSLContextConfig() {
  wangle::SSLContextConfig sslCtxConfig;
  sslCtxConfig.sessionContext = "shared_manager_test";
  sslCtxConfig.setCertificateBuf(kTestCert1PEM, kTestCert1Key);
  sslCtxConfig.isDefault = true;
  sslCtxConfig.sessionCacheEnabled = false;
  sslCtxConfig.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  return sslCtxConfig;
  ;
}

std::shared_ptr<wangle::ServerSocketConfig> getServerSocketConfig() {
  auto config = std::make_shared<wangle::ServerSocketConfig>();
  config->sslContextConfigs.emplace_back(getSSLContextConfig());
  return config;
}
} // namespace

namespace wangle {
class TestAcceptor : public Acceptor {
 public:
  using Acceptor::getFizzPeeker;
  explicit TestAcceptor(std::shared_ptr<const ServerSocketConfig> accConfig)
      : Acceptor(std::move(accConfig)) {}
  std::shared_ptr<SSLContextManager> getContextManager() const {
    return sslCtxManager_;
  }
};

class SharedSSLContextManagerTest : public ::testing::Test {
 public:
  void initialize(std::shared_ptr<const ServerSocketConfig> config) {
    manager_ =
        std::make_shared<SharedSSLContextManagerImpl<wangle::FizzConfigUtil>>(
            config);
    for (auto i = 0; i < N; ++i) {
      acceptors_[i] = initTestAcceptor(config);
      manager_->addAcceptor(acceptors_[i]);
    }
  }

 protected:
  static constexpr size_t N = 2;
  struct Context {
    std::shared_ptr<SSLContextManager> ctxManager;
    std::shared_ptr<const fizz::server::FizzServerContext> fizzContext;
  };

  struct State {
    Context manager;
    std::array<Context, N> acceptors;
  };

  State getCurrentState() const {
    State state;
    state.manager.ctxManager = manager_->getContextManager();
    state.manager.fizzContext = manager_->getFizzContext();
    for (auto i = 0; i < N; ++i) {
      state.acceptors[i].ctxManager = acceptors_[i]->getContextManager();
      state.acceptors[i].fizzContext =
          acceptors_[i]->getFizzPeeker()->getContext();
    }
    return state;
  }

  void checkPreCondition() {
    auto state = getCurrentState();
    EXPECT_NE(nullptr, state.manager.ctxManager);
    EXPECT_NE(nullptr, state.manager.fizzContext);
    for (auto i = 0; i < N; ++i) {
      EXPECT_NE(nullptr, state.acceptors[i].ctxManager);
      EXPECT_NE(nullptr, state.acceptors[i].fizzContext);
      EXPECT_NE(state.manager.ctxManager, state.acceptors[i].ctxManager);
      EXPECT_NE(state.manager.fizzContext, state.acceptors[i].fizzContext);
    }
  }

  void checkSuccessPostCondition(const State& prevState) {
    auto state = getCurrentState();
    EXPECT_NE(prevState.manager.ctxManager, state.manager.ctxManager);
    EXPECT_NE(prevState.manager.fizzContext, state.manager.fizzContext);
    for (auto i = 0; i < N; ++i) {
      EXPECT_NE(
          prevState.acceptors[i].ctxManager, state.acceptors[i].ctxManager);
      EXPECT_NE(
          prevState.acceptors[i].fizzContext, state.acceptors[i].fizzContext);
      EXPECT_EQ(state.manager.ctxManager, state.acceptors[i].ctxManager);
      EXPECT_EQ(state.manager.fizzContext, state.acceptors[i].fizzContext);
    }
  }

  void checkFailurePostCondition(const State& prevState) {
    auto state = getCurrentState();
    EXPECT_EQ(prevState.manager.ctxManager, state.manager.ctxManager);
    EXPECT_EQ(prevState.manager.fizzContext, state.manager.fizzContext);
    for (auto i = 0; i < N; ++i) {
      EXPECT_EQ(
          prevState.acceptors[i].ctxManager, state.acceptors[i].ctxManager);
      EXPECT_EQ(
          prevState.acceptors[i].fizzContext, state.acceptors[i].fizzContext);
    }
  }

  void runUpdateAndChecks(
      std::function<void()> update,
      std::function<void()> checkPostCondition) {
    checkPreCondition();
    update();
    for (auto i = 0; i < N; ++i) {
      evb_.loop();
    }
    checkPostCondition();

    for (auto i = 0; i < N; ++i) {
      acceptors_[i]->forceStop();
      evb_.loop();
    }
  }

  std::shared_ptr<TestAcceptor> initTestAcceptor(
      std::shared_ptr<const ServerSocketConfig> config) {
    auto acceptor = std::make_shared<TestAcceptor>(std::move(config));
    acceptor->init(nullptr, &evb_);
    return acceptor;
  }

  EventBase evb_;
  std::shared_ptr<SharedSSLContextManager> manager_;
  std::array<std::shared_ptr<TestAcceptor>, N> acceptors_;
};

TEST_F(SharedSSLContextManagerTest, TestUpdateTLSTicketKeys) {
  auto config = getServerSocketConfig();
  config->initialTicketSeeds = {{"01"}, {"02"}, {"03"}};
  initialize(std::move(config));

  auto update = [this]() {
    TLSTicketKeySeeds seeds{{"02"}, {"03"}, {"04"}};
    manager_->updateTLSTicketKeys(seeds);
  };

  runUpdateAndChecks(update, [this, state = getCurrentState()]() {
    this->checkSuccessPostCondition(state);
  });
}

TEST_F(SharedSSLContextManagerTest, TestUpdateTLSTicketKeysFailure) {
  auto tmpFile = getCertFile();
  const auto tmpFilePath = folly::fs::canonical(tmpFile.path()).string();
  auto config = getServerSocketConfig();
  config->sslContextConfigs.front().setCertificate(
      tmpFilePath, tmpFilePath, "");
  initialize(std::move(config));

  std::string corruptedCert =
      "-----BEGIN CERTIFICATE-----\n"
      "bad certificate \n"
      "-----END CERTIFICATE-----\n";
  folly::writeFile(corruptedCert, tmpFilePath.c_str());
  auto update = [this]() {
    TLSTicketKeySeeds seeds{{"02"}, {"03"}, {"04"}};
    manager_->updateTLSTicketKeys(seeds);
  };

  runUpdateAndChecks(update, [this, state = getCurrentState()]() {
    this->checkFailurePostCondition(state);
  });
}

TEST_F(SharedSSLContextManagerTest, TestReloadSSLContextConfigs) {
  initialize(getServerSocketConfig());

  auto update = [this]() { manager_->reloadSSLContextConfigs(); };

  runUpdateAndChecks(update, [this, state = getCurrentState()]() {
    this->checkSuccessPostCondition(state);
  });
}

TEST_F(SharedSSLContextManagerTest, TestReloadSSLContextConfigsFailure) {
  auto tmpFile = getCertFile();
  const auto tmpFilePath = folly::fs::canonical(tmpFile.path()).string();
  auto config = getServerSocketConfig();
  config->sslContextConfigs.front().setCertificate(
      tmpFilePath, tmpFilePath, "");
  initialize(std::move(config));

  std::string corruptedCert =
      "-----BEGIN CERTIFICATE-----\n"
      "bad certificate \n"
      "-----END CERTIFICATE-----\n";
  folly::writeFile(corruptedCert, tmpFilePath.c_str());
  auto update = [this]() { manager_->reloadSSLContextConfigs(); };

  runUpdateAndChecks(update, [this, state = getCurrentState()]() {
    this->checkFailurePostCondition(state);
  });
}

TEST_F(SharedSSLContextManagerTest, TestUpdateSSLConfigAndReloadContexts) {
  initialize(getServerSocketConfig());

  auto update = [this]() {
    manager_->updateSSLConfigAndReloadContexts(getSSLContextConfig());
  };

  runUpdateAndChecks(update, [this, state = getCurrentState()]() {
    this->checkSuccessPostCondition(state);
  });
}

TEST_F(
    SharedSSLContextManagerTest,
    TestUpdateSSLConfigAndReloadContextsFailure) {
  initialize(getServerSocketConfig());

  auto update = [this]() {
    auto sslContextConfig = getSSLContextConfig();
    sslContextConfig.setCertificateBuf("bad cert", "bad key");
    manager_->updateSSLConfigAndReloadContexts(std::move(sslContextConfig));
  };

  runUpdateAndChecks(update, [this, state = getCurrentState()]() {
    this->checkFailurePostCondition(state);
  });
}
} // namespace wangle
