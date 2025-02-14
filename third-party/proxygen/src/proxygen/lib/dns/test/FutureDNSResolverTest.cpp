/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/dns/FutureDNSResolver.h"

#include <map>
#include <utility>
#include <vector>

#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/portability/GTest.h>

using std::string;

namespace proxygen {
namespace {

template <typename K, typename V>
std::map<V, K> invert(const std::map<K, V>& input) {
  std::map<V, K> result;
  for (const auto& pair : input) {
    result[pair.second] = pair.first;
  }
  return result;
}

class FakeResolverError : public std::runtime_error {
 public:
  explicit FakeResolverError(string message)
      : std::runtime_error(std::move(message)) {
  }
};

class FakeDNSResolver : public DNSResolver {
 public:
  explicit FakeDNSResolver(
      folly::EventBase* evb,
      const std::map<folly::SocketAddress, string>& addrToHostMap,
      std::map<string, std::vector<std::pair<int, string>>>
          domainToMailExchangeMap)
      : evb_{evb},
        addrToHostMap_{addrToHostMap},
        hostToAddrMap_{invert(addrToHostMap)},
        mailExchangeMap_{std::move(domainToMailExchangeMap)} {
    CHECK_EQ(addrToHostMap_.size(), hostToAddrMap_.size());
  }

  ~FakeDNSResolver() override = default;

  void resolveAddress(ResolutionCallback* cb,
                      const folly::SocketAddress& address,
                      std::chrono::milliseconds /* timeout */ =
                          FutureDNSResolver::kDefaultTimeout()) override {
    evb_->runInEventBaseThread([cb, address, this]() {
      auto iter = addrToHostMap_.find(address);
      if (iter != addrToHostMap_.end()) {
        DNSResolver::Answer answer;
        answer.type = DNSResolver::Answer::AnswerType::AT_NAME;
        answer.name = iter->second;
        cb->resolutionSuccess({std::move(answer)});
      } else {
        FakeResolverError error{"Cannot resolve address"};
        cb->resolutionError(folly::exception_wrapper{std::move(error)});
      }
    });
  }

  void resolveHostname(
      ResolutionCallback* cb,
      const std::string& name,
      std::chrono::milliseconds /* timeout */ =
          FutureDNSResolver::kDefaultTimeout(),
      sa_family_t /* family */ = AF_INET,
      TraceEventContext /* teContext */ = TraceEventContext()) override {
    evb_->runInEventBaseThread([cb, name, this]() {
      auto iter = hostToAddrMap_.find(name);
      if (iter != hostToAddrMap_.end()) {
        DNSResolver::Answer answer;
        answer.type = DNSResolver::Answer::AnswerType::AT_ADDRESS;
        answer.address = iter->second;
        cb->resolutionSuccess({std::move(answer)});
      } else {
        FakeResolverError error{"Cannot resolve hostname"};
        cb->resolutionError(folly::exception_wrapper{std::move(error)});
      }
    });
  }

  void resolveMailExchange(ResolutionCallback* cb,
                           const std::string& domain,
                           std::chrono::milliseconds /* timeout */ =
                               FutureDNSResolver::kDefaultTimeout()) override {
    evb_->runInEventBaseThread([cb, domain, this]() {
      auto iter = mailExchangeMap_.find(domain);
      if (iter != mailExchangeMap_.end()) {
        std::vector<std::pair<int, string>> entries = iter->second;
        std::vector<DNSResolver::Answer> answers = {};
        for (const std::pair<int, string>& entry : entries) {
          DNSResolver::Answer answer;
          answer.type = DNSResolver::Answer::AnswerType::AT_MX;
          answer.priority = entry.first;
          answer.name = entry.second;
          answers.push_back(std::move(answer));
        }
        cb->resolutionSuccess(answers);
      } else {
        FakeResolverError error{"Cannot resolve mail exchange info"};
        cb->resolutionError(folly::exception_wrapper{std::move(error)});
      }
    });
  }

  DNSResolver::StatsCollector* getStatsCollector() const override {
    return statsCollector_;
  }

  void setStatsCollector(DNSResolver::StatsCollector* collector) override {
    statsCollector_ = collector;
  }

 private:
  folly::EventBase* evb_;
  std::map<folly::SocketAddress, string> addrToHostMap_;
  std::map<string, folly::SocketAddress> hostToAddrMap_;
  std::map<string, std::vector<std::pair<int, string>>> mailExchangeMap_;
  DNSResolver::StatsCollector* statsCollector_;
};
} // namespace

class FutureDNSResolverTest : public ::testing::Test {
 public:
  ~FutureDNSResolverTest() override = default;

 protected:
  void SetUp() override {
    DNSResolver::UniquePtr proxygenResolver;
    std::map<folly::SocketAddress, string> addrToHostMap{
        {folly::SocketAddress(folly::IPAddress("1.2.3.4"), 0), "1234.com"},
        {folly::SocketAddress(folly::IPAddress("2.3.4.5"), 0), "2345.com"},
    };
    std::map<string, std::vector<std::pair<int, string>>>
        domainToMailExchangeMap{
            {"1234.com",
             {
                 {10, "mail1.1234.com"},
                 {20, "mail2.1234.com"},
                 {30, "mail3.1234.com"},
             }},
            {"2345.com",
             {
                 {100, "mailone.2345.com"},
                 {100, "mailtwo.2345.com"},
             }},
        };
    proxygenResolver.reset(
        new FakeDNSResolver(evb_, addrToHostMap, domainToMailExchangeMap));
    resolver_ =
        std::make_unique<FutureDNSResolver>(evb_, std::move(proxygenResolver));
  }

  using AnswerType = DNSResolver::Answer::AnswerType;

  folly::EventBaseManager* ebm_{folly::EventBaseManager::get()};
  folly::EventBase* evb_{ebm_->getEventBase()};
  std::unique_ptr<FutureDNSResolver> resolver_;
};

TEST_F(FutureDNSResolverTest, TestResolveAddressSuccess) {
  auto addr1 = folly::SocketAddress(folly::IPAddress("1.2.3.4"), 0);
  auto answers1 = resolver_->resolveAddress(addr1).getVia(evb_);
  EXPECT_EQ(1, answers1.size());
  EXPECT_EQ(AnswerType::AT_NAME, answers1[0].type);
  EXPECT_EQ("1234.com", answers1[0].name);

  auto addr2 = folly::SocketAddress(folly::IPAddress("2.3.4.5"), 0);
  auto answers2 = resolver_->resolveAddress(addr2).getVia(evb_);
  EXPECT_EQ(1, answers2.size());
  EXPECT_EQ(AnswerType::AT_NAME, answers2[0].type);
  EXPECT_EQ("2345.com", answers2[0].name);
}

TEST_F(FutureDNSResolverTest, TestResolveAddressFail) {
  auto addr = folly::SocketAddress(folly::IPAddress("6.7.8.9"), 0);
  EXPECT_THROW(
      { resolver_->resolveAddress(addr).getVia(evb_); }, FakeResolverError);
}

TEST_F(FutureDNSResolverTest, TestResolveHostnameSuccess) {
  auto addr1 = folly::SocketAddress(folly::IPAddress("1.2.3.4"), 0);
  auto answers1 = resolver_->resolveHostname("1234.com").getVia(evb_);
  EXPECT_EQ(1, answers1.size());
  EXPECT_EQ(AnswerType::AT_ADDRESS, answers1[0].type);
  EXPECT_EQ(addr1, answers1[0].address);

  auto addr2 = folly::SocketAddress(folly::IPAddress("2.3.4.5"), 0);
  auto answers2 = resolver_->resolveHostname("2345.com").getVia(evb_);
  EXPECT_EQ(1, answers2.size());
  EXPECT_EQ(AnswerType::AT_ADDRESS, answers2[0].type);
  EXPECT_EQ(addr2, answers2[0].address);
}

TEST_F(FutureDNSResolverTest, TestResolveHostnameFail) {
  EXPECT_THROW(
      { resolver_->resolveHostname("unknown-host.com").getVia(evb_); },
      FakeResolverError);
}

TEST_F(FutureDNSResolverTest, TestResolveMailExchangeSuccess) {
  auto host11 = "mail1.1234.com";
  auto priority11 = 10;
  auto host12 = "mail2.1234.com";
  auto priority12 = 20;
  auto host13 = "mail3.1234.com";
  auto priority13 = 30;
  auto answers1 = resolver_->resolveMailExchange("1234.com").getVia(evb_);
  EXPECT_EQ(3, answers1.size());
  EXPECT_EQ(AnswerType::AT_MX, answers1[0].type);
  EXPECT_EQ(host11, answers1[0].name);
  EXPECT_EQ(priority11, answers1[0].priority);
  EXPECT_EQ(AnswerType::AT_MX, answers1[1].type);
  EXPECT_EQ(host12, answers1[1].name);
  EXPECT_EQ(priority12, answers1[1].priority);
  EXPECT_EQ(AnswerType::AT_MX, answers1[2].type);
  EXPECT_EQ(host13, answers1[2].name);
  EXPECT_EQ(priority13, answers1[2].priority);

  auto host21 = "mailone.2345.com";
  auto priority21 = 100;
  auto host22 = "mailtwo.2345.com";
  auto priority22 = 100;
  auto answers2 = resolver_->resolveMailExchange("2345.com").getVia(evb_);
  EXPECT_EQ(2, answers2.size());
  EXPECT_EQ(AnswerType::AT_MX, answers2[0].type);
  EXPECT_EQ(host21, answers2[0].name);
  EXPECT_EQ(priority21, answers2[0].priority);
  EXPECT_EQ(AnswerType::AT_MX, answers2[1].type);
  EXPECT_EQ(host22, answers2[1].name);
  EXPECT_EQ(priority22, answers2[1].priority);
}

TEST_F(FutureDNSResolverTest, TestResolveMailExchangeFail) {
  EXPECT_THROW(
      { resolver_->resolveMailExchange("unknown-host.com").getVia(evb_); },
      FakeResolverError);
}
} // namespace proxygen
