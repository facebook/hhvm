/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/dns/test/SyncDNSResolverTest.h"

using namespace proxygen;
using namespace testing;

using folly::SocketAddress;
using std::chrono::seconds;

TEST_F(SyncDNSResolverTest, Success) {
  std::vector<DNSResolver::Answer> expected{
      DNSResolver::Answer(seconds(5), SocketAddress("::1", 0)),
      DNSResolver::Answer(seconds(5), SocketAddress("127.0.0.1", 0)),
      DNSResolver::Answer(seconds(5), SocketAddress("1.2.3.4", 0))};

  EXPECT_CALL(*resolver_, resolveHostname(_, Eq("www.facebook.com"), _, _, _))
      .WillOnce(DoAll(Invoke([&](DNSResolver::ResolutionCallback* cb,
                                 const std::string& /*hostname*/,
                                 std::chrono::milliseconds /*timeout*/,
                                 sa_family_t /*family*/,
                                 TraceEventContext /*teContext*/) {
                        cb->resolutionSuccess(expected);
                      }),
                      Return()));

  auto actual = syncResolver_->resolveHostname("www.facebook.com");
  ASSERT_EQ(expected.size(), actual.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    ASSERT_EQ(expected[i].address, actual[i]);
  }
}

TEST_F(SyncDNSResolverTest, EmptyResponseIsTreatedAsError) {
  std::vector<DNSResolver::Answer> expected;

  EXPECT_CALL(*resolver_, resolveHostname(_, Eq("www.facebook.com"), _, _, _))
      .WillOnce(DoAll(Invoke([&](DNSResolver::ResolutionCallback* cb,
                                 const std::string& /*hostname*/,
                                 std::chrono::milliseconds /*timeout*/,
                                 sa_family_t /*family*/,
                                 TraceEventContext /*teContext*/) {
                        cb->resolutionSuccess(expected);
                      }),
                      Return()));

  ASSERT_THROW(syncResolver_->resolveHostname("www.facebook.com"),
               proxygen::DNSResolver::Exception);
}

TEST_F(SyncDNSResolverTest, AsyncSuccess) {
  std::vector<DNSResolver::Answer> expected{
      DNSResolver::Answer(seconds(5), SocketAddress("127.0.0.1", 0))};

  EXPECT_CALL(*resolver_, resolveHostname(_, Eq("www.facebook.com"), _, _, _))
      .WillOnce(DoAll(Invoke([&](DNSResolver::ResolutionCallback* cb,
                                 const std::string& /*hostname*/,
                                 std::chrono::milliseconds /*timeout*/,
                                 sa_family_t /*family*/,
                                 TraceEventContext /*teContext*/) {
                        evb_->runInLoop(
                            [&, cb]() { cb->resolutionSuccess(expected); });
                      }),
                      Return()));

  auto actual = syncResolver_->resolveHostname("www.facebook.com");
  ASSERT_EQ(expected[0].address, actual[0]);
}

TEST_F(SyncDNSResolverTest, Failure) {
  EXPECT_CALL(*resolver_, resolveHostname(_, Eq("www.facebook.com"), _, _, _))
      .WillOnce(DoAll(
          Invoke([&](DNSResolver::ResolutionCallback* cb,
                     const std::string& /*hostname*/,
                     std::chrono::milliseconds /*timeout*/,
                     sa_family_t /*family*/,
                     TraceEventContext /*teContext*/) {
            auto ew = folly::make_exception_wrapper<std::runtime_error>("test");
            cb->resolutionError(ew);
          }),
          Return()));

  ASSERT_THROW(syncResolver_->resolveHostname("www.facebook.com"),
               std::runtime_error);
}
