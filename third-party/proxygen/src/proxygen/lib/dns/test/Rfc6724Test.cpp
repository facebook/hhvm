/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/dns/test/SyncDNSResolverTest.h"

#include "proxygen/lib/dns/Rfc6724.h"

using namespace proxygen;
using namespace testing;

using folly::SocketAddress;
using std::chrono::seconds;

// RFC 6724 section 10.2
TEST(SyncDNSResolver, Basic102) {
  {
    std::vector<SocketAddress> addrs{SocketAddress("198.51.100.121", 0),
                                     SocketAddress("2001:db8:1::1", 0)};
    std::vector<SocketAddress> expected{SocketAddress("2001:db8:1::1", 0),
                                        SocketAddress("198.51.100.121", 0)};
    SocketAddress src1("2001:db8:1::2", 0);
    rfc6724_sort(addrs, &src1);
    for (size_t i = 0; i < expected.size(); ++i) {
      EXPECT_EQ(expected[i], addrs[i]);
    }
  }

  {
    std::vector<SocketAddress> addrs{SocketAddress("2001:db8:1::1", 0),
                                     SocketAddress("2002:c633:6401::1", 0)};
    std::vector<SocketAddress> expected{SocketAddress("2002:c633:6401::1", 0),
                                        SocketAddress("2001:db8:1::1", 0)};
    SocketAddress src1("2002:c633:6401::2", 0);
    rfc6724_sort(addrs, &src1);
    for (size_t i = 0; i < expected.size(); ++i) {
      EXPECT_EQ(expected[i], addrs[i]);
    }
  }

  {
    std::vector<SocketAddress> addrs{SocketAddress("2001:db8:1::1", 0),
                                     SocketAddress("fe80::1", 0)};
    std::vector<SocketAddress> expected{SocketAddress("fe80::1", 0),
                                        SocketAddress("2001:db8:1::1", 0)};
    SocketAddress src1("fe80::2", 0);
    rfc6724_sort(addrs, &src1);
    for (size_t i = 0; i < expected.size(); ++i) {
      EXPECT_EQ(expected[i], addrs[i]);
    }
  }
}

TEST_F(SyncDNSResolverTest, Sorted) {
  std::vector<DNSResolver::Answer> results{
      DNSResolver::Answer(seconds(5), SocketAddress("198.51.100.121", 0)),
      DNSResolver::Answer(seconds(5), SocketAddress("2001:db8:1::1", 0))};

  std::vector<SocketAddress> expected{SocketAddress("2001:db8:1::1", 0),
                                      SocketAddress("198.51.100.121", 0)};

  EXPECT_CALL(*resolver_, resolveHostname(_, Eq("www.facebook.com"), _, _, _))
      .WillOnce(DoAll(Invoke([&](DNSResolver::ResolutionCallback* cb,
                                 const std::string& /*hostname*/,
                                 std::chrono::milliseconds /*timeout*/,
                                 sa_family_t /*family*/,
                                 TraceEventContext /*teContext*/) {
                        cb->resolutionSuccess(results);
                      }),
                      Return()));

  auto actual = syncResolver_->resolveHostname(
      "www.facebook.com", seconds(5), AF_UNSPEC, true);
  ASSERT_EQ(expected.size(), actual.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    ASSERT_EQ(expected[i], actual[i]);
  }
}

TEST_F(SyncDNSResolverTest, SortedLocal) {
  std::vector<DNSResolver::Answer> results{
      DNSResolver::Answer(seconds(5), SocketAddress("127.0.0.1", 0)),
      DNSResolver::Answer(seconds(5), SocketAddress("::1", 0))};

  std::vector<SocketAddress> expected{SocketAddress("::1", 0),
                                      SocketAddress("127.0.0.1", 0)};

  EXPECT_CALL(*resolver_, resolveHostname(_, Eq("www.facebook.com"), _, _, _))
      .WillOnce(DoAll(Invoke([&](DNSResolver::ResolutionCallback* cb,
                                 const std::string& /*hostname*/,
                                 std::chrono::milliseconds /*timeout*/,
                                 sa_family_t /*family*/,
                                 TraceEventContext /*teContext*/) {
                        cb->resolutionSuccess(results);
                      }),
                      Return()));

  auto actual = syncResolver_->resolveHostname(
      "www.facebook.com", seconds(5), AF_UNSPEC, true);
  ASSERT_EQ(expected.size(), actual.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    ASSERT_EQ(expected[i], actual[i]);
  }
}

TEST_F(SyncDNSResolverTest, Unsorted) {
  std::vector<DNSResolver::Answer> expected{
      DNSResolver::Answer(seconds(5), SocketAddress("127.0.0.1", 0)),
      DNSResolver::Answer(seconds(5), SocketAddress("::1", 0))};

  EXPECT_CALL(*resolver_, resolveHostname(_, Eq("www.facebook.com"), _, _, _))
      .WillOnce(DoAll(Invoke([&](DNSResolver::ResolutionCallback* cb,
                                 const std::string& /*hostname*/,
                                 std::chrono::milliseconds /*timeout*/,
                                 sa_family_t /*family*/,
                                 TraceEventContext /*teContext*/) {
                        cb->resolutionSuccess(expected);
                      }),
                      Return()));

  auto actual = syncResolver_->resolveHostname(
      "www.facebook.com", seconds(5), AF_UNSPEC, false);
  ASSERT_EQ(expected.size(), actual.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    ASSERT_EQ(expected[i].address, actual[i]);
  }
}
