/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/client/CoroDNSResolver.h"

#include <folly/coro/BlockingWait.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/portability/GTest.h>

#include "proxygen/lib/dns/DNSResolver.h"
#include "proxygen/lib/dns/test/MockDNSResolver.h"

using namespace testing;
using namespace proxygen;
using namespace proxygen::coro;
using namespace std::chrono;

namespace proxygen::coro::test {

class CoroDNSResolverTest : public Test {
 protected:
  folly::EventBase* evb_ = folly::EventBaseManager::get()->getEventBase();

  // Create a mock resolver object, inject it into CoroDNSResolver, and expect a
  // call to its resolveHostname method with an action
  void expectResolveHostname(
      std::function<void(DNSResolver::ResolutionCallback*)> action) {
    auto* mockResolver = new MockDNSResolver();

    EXPECT_CALL(*mockResolver, resolveHostname(_, _, _, _, _))
        .WillOnce(
            ::testing::Invoke([&](DNSResolver::ResolutionCallback* cb,
                                  const std::string&,
                                  std::chrono::milliseconds,
                                  sa_family_t,
                                  const TraceEventContext&) { action(cb); }));

    coro::CoroDNSResolver::resetDNSResolverInstance(
        evb_, proxygen::DNSResolver::UniquePtr(mockResolver));
  }
};

TEST_F(CoroDNSResolverTest, testSuccess) {
  co_withExecutor(
      evb_,
      [this](folly::EventBase* evb) -> folly::coro::Task<void> {
        expectResolveHostname([](DNSResolver::ResolutionCallback* cb) {
          cb->resolutionSuccess({
              DNSResolver::Answer(
                  std::chrono::seconds(10),
                  folly::SocketAddress(
                      "2a03:2880:f134:0183:face:b00c:0000:25de", 0)),
              DNSResolver::Answer(std::chrono::seconds(10),
                                  folly::SocketAddress("31.13.93.35", 0)),
          });
        });

        auto addresses = co_await co_awaitTry(
            CoroDNSResolver::resolveHost(evb, "www.facebook.com", seconds(1)));
        EXPECT_FALSE(addresses.hasException());
        EXPECT_NE(addresses->primary, folly::SocketAddress());
        EXPECT_TRUE(addresses->fallback);
        EXPECT_NE(addresses->fallback, folly::SocketAddress());
        EXPECT_NE(addresses->primary.getFamily(),
                  addresses->fallback->getFamily());
        EXPECT_EQ(addresses->primary.getFamily(), AF_INET6);
      }(evb_))
      .start();
}

TEST_F(CoroDNSResolverTest, testNoNames) {
  co_withExecutor(
      evb_,
      [this](folly::EventBase* evb) -> folly::coro::Task<void> {
        expectResolveHostname([](DNSResolver::ResolutionCallback* cb) {
          folly::exception_wrapper ew =
              folly::make_exception_wrapper<DNSResolver::Exception>(
                  DNSResolver::NODATA, "err");
          cb->resolutionError(ew);
        });

        auto addresses = co_await co_awaitTry(CoroDNSResolver::resolveHost(
            evb, "ktufictvvthhuvlbh.com", seconds(1)));
        EXPECT_TRUE(addresses.hasException());
        EXPECT_EQ(
            addresses.tryGetExceptionObject<DNSResolver::Exception>()->status(),
            DNSResolver::NODATA);
      }(evb_))
      .start();
}

TEST_F(CoroDNSResolverTest, testMultipleReturnedDomains) {
  co_withExecutor(
      evb_,
      [this](folly::EventBase* evb) -> folly::coro::Task<void> {
        expectResolveHostname([](DNSResolver::ResolutionCallback* cb) {
          cb->resolutionSuccess({
              DNSResolver::Answer(
                  std::chrono::seconds(1),
                  folly::SocketAddress(
                      "2a03:2880:f134:0183:face:b00c:0000:25de", 0)),
              DNSResolver::Answer(std::chrono::seconds(1),
                                  folly::SocketAddress("31.13.93.35", 0)),
          });
        });

        auto addresses = co_await co_awaitTry(
            CoroDNSResolver::resolveHost(evb, "www.facebook.com", seconds(1)));
        EXPECT_FALSE(addresses.hasException());
        auto address = addresses.value();
        EXPECT_TRUE(address.fallback);
      }(evb_))
      .start();
}

TEST_F(CoroDNSResolverTest, testAllReturnedDomains) {
  co_withExecutor(
      evb_,
      [this](folly::EventBase* evb) -> folly::coro::Task<void> {
        expectResolveHostname([](DNSResolver::ResolutionCallback* cb) {
          cb->resolutionSuccess({
              DNSResolver::Answer(
                  std::chrono::seconds(1),
                  folly::SocketAddress(
                      "2a03:2880:f134:0183:face:b00c:0000:25de", 0)),
              DNSResolver::Answer(std::chrono::seconds(1),
                                  folly::SocketAddress("31.13.93.35", 0)),
          });
        });

        auto addresses = co_await co_awaitTry(CoroDNSResolver::resolveHostAll(
            evb, "www.facebook.com", seconds(1)));
        EXPECT_FALSE(addresses.hasException());
        EXPECT_GT(addresses.value().size(), 1);
      }(evb_))
      .start();
}

TEST_F(CoroDNSResolverTest, UserSuppliedDnsResolver) {
  // user supplying custom DNSResolver should never call into the global
  // DNSResolver
  auto* globalDnsResolver = new MockDNSResolver();
  coro::CoroDNSResolver::resetDNSResolverInstance(
      evb_, DNSResolver::UniquePtr(globalDnsResolver));
  EXPECT_CALL(*globalDnsResolver,
              resolveHostname(_, "www.facebook.com", _, _, _))
      .Times(0);

  MockDNSResolver resolver;
  EXPECT_CALL(resolver, resolveHostname(_, "www.facebook.com", _, _, _))
      .WillOnce(WithArgs<0>(Invoke([](DNSResolver::ResolutionCallback* cb) {
        cb->resolutionSuccess({DNSResolver::Answer{
            std::chrono::seconds(0), folly::SocketAddress("0.0.0.0", 0)}});
      })));

  folly::coro::blockingWait(
      CoroDNSResolver::resolveHost(
          evb_, "www.facebook.com", std::chrono::milliseconds(100), &resolver),
      evb_);

  coro::CoroDNSResolver::resetDNSResolverInstance(evb_, nullptr);
}

TEST(SingletonTest, DnsSingletonShutdown) {
  // request global IOExecutor backed by singleton (order matters, this
  // construction must occur first)
  auto* evb = folly::getGlobalIOExecutor()->getEventBase();
  auto keepalive = folly::Executor::getKeepAliveToken(evb);

  folly::via(evb).then([evb, keepalive](auto&&) {
    // coroutine to invoke CoroDNSResolver::resolveHost(...)
    auto t =
        folly::coro::co_invoke([evb, keepalive]() -> folly::coro::Task<void> {
          // DNSModule singleton will be requested here
          auto res =
              co_await folly::coro::co_awaitTry(CoroDNSResolver::resolveHost(
                  evb, "www.facebook.com", std::chrono::seconds(1)));
          // should return an exception on shutdown
          EXPECT_TRUE(res.hasException());
          co_return;
        });

    /**
     * run dns resolution after 100ms, gives enough time for shutdown to
     * request destruction of CoroDNSResolver singleton prior to executor/evb
     * (reverse construction order)
     */
    evb->runAfterDelay(
        [t = std::move(t), evb, keepalive]() mutable {
          co_withExecutor(evb, std::move(t)).start();
        },
        100);
  });
}

} // namespace proxygen::coro::test
