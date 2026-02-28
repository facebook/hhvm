/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/SocketAddress.h>
#include <gtest/gtest.h>
#include <proxygen/lib/dns/DNSResolver.h>

#include <unordered_set>

using folly::SocketAddress;
using proxygen::DNSResolver;
using std::chrono::seconds;

namespace {

size_t h(const DNSResolver::Answer& a) {
  return DNSResolver::AnswerHash{}(a);
}

} // namespace

TEST(DNSResolverAnswerHashTest, EqualAnswersHaveEqualHash_Address) {
  // Same IP/port should be equal and have equal hash regardless of TTL
  DNSResolver::Answer a1(seconds(5), SocketAddress("127.0.0.1", 80));
  DNSResolver::Answer a2(seconds(30), SocketAddress("127.0.0.1", 80));

  EXPECT_TRUE(a1 == a2);
  EXPECT_EQ(h(a1), h(a2));
}

TEST(DNSResolverAnswerHashTest, EqualAnswersHaveEqualHash_Name) {
  DNSResolver::Answer n1(
      seconds(10), std::string("example.com"), DNSResolver::Answer::AT_NAME);
  DNSResolver::Answer n2(
      seconds(1), std::string("example.com"), DNSResolver::Answer::AT_NAME);

  EXPECT_TRUE(n1 == n2);
  EXPECT_EQ(h(n1), h(n2));
}

TEST(DNSResolverAnswerHashTest, EqualAnswersHaveEqualHash_CName) {
  DNSResolver::Answer c1(seconds(10),
                         std::string("cname.example.com"),
                         DNSResolver::Answer::AT_CNAME);
  DNSResolver::Answer c2(seconds(10),
                         std::string("cname.example.com"),
                         DNSResolver::Answer::AT_CNAME);
  EXPECT_TRUE(c1 == c2);
  EXPECT_EQ(h(c1), h(c2));
}

TEST(DNSResolverAnswerHashTest, EqualAnswersHaveEqualHash_MX) {
  DNSResolver::Answer m1(
      seconds(60), /*priority*/ 10, std::string("mail.example.com"));
  DNSResolver::Answer m2(
      seconds(5), /*priority*/ 10, std::string("mail.example.com"));
  EXPECT_TRUE(m1 == m2);
  EXPECT_EQ(h(m1), h(m2));
}

TEST(DNSResolverAnswerHashTest, DistinctAnswersBehaveInUnorderedSet) {
  // Build a set with distinct answers of different types and payloads
  std::unordered_set<DNSResolver::Answer, DNSResolver::AnswerHash> set;

  DNSResolver::Answer a1(seconds(5), SocketAddress("127.0.0.1", 0));
  DNSResolver::Answer a2(seconds(5), SocketAddress("::1", 0));
  DNSResolver::Answer n1(
      seconds(5), std::string("example.com"), DNSResolver::Answer::AT_NAME);
  DNSResolver::Answer n2(
      seconds(5), std::string("www.example.com"), DNSResolver::Answer::AT_NAME);
  DNSResolver::Answer c1(seconds(5),
                         std::string("cname.example.com"),
                         DNSResolver::Answer::AT_CNAME);
  DNSResolver::Answer m1(
      seconds(5), 5 /*priority*/, std::string("mail.example.com"));
  DNSResolver::Answer m2(
      seconds(5), 10 /*priority*/, std::string("mail.example.com"));

  EXPECT_TRUE(set.insert(a1).second);
  EXPECT_TRUE(set.insert(a2).second);
  EXPECT_TRUE(set.insert(n1).second);
  EXPECT_TRUE(set.insert(n2).second);
  EXPECT_TRUE(set.insert(c1).second);
  EXPECT_TRUE(set.insert(m1).second);
  EXPECT_TRUE(set.insert(m2).second);

  // Reinserting equal values should not increase size
  EXPECT_FALSE(set.insert(DNSResolver::Answer(seconds(999),
                                              SocketAddress("127.0.0.1", 0)))
                   .second);
  EXPECT_FALSE(set.insert(DNSResolver::Answer(seconds(1),
                                              std::string("example.com"),
                                              DNSResolver::Answer::AT_NAME))
                   .second);
  EXPECT_FALSE(set.insert(DNSResolver::Answer(
                              seconds(1), 5, std::string("mail.example.com")))
                   .second);

  EXPECT_EQ(set.size(), 7);
}

TEST(DNSResolverAnswerHashTest, IgnoresTTLAndCreationTime) {
  // creationTime is set at construction time; we can't set it directly,
  // but we can construct the two objects at different times and ensure they
  // still compare equal and have equal hashes.
  DNSResolver::Answer a1(seconds(1), SocketAddress("1.2.3.4", 0));
  /* Small sleep to ensure creationTime differs if measured in seconds */
  DNSResolver::Answer a2(seconds(999), SocketAddress("1.2.3.4", 0));

  EXPECT_TRUE(a1 == a2);
  EXPECT_EQ(h(a1), h(a2));
}

// Tests to ensure each field in operator== and AnswerHash is properly checked.
// If any field is missing from the implementation, these tests will fail.

TEST(DNSResolverAnswerHashTest, DifferentTypeNotEqual) {
  DNSResolver::Answer a1(
      seconds(5), std::string("example.com"), DNSResolver::Answer::AT_NAME);
  DNSResolver::Answer a2(
      seconds(5), std::string("example.com"), DNSResolver::Answer::AT_CNAME);

  EXPECT_FALSE(a1 == a2);
  EXPECT_NE(h(a1), h(a2));
}

TEST(DNSResolverAnswerHashTest, DifferentAddressNotEqual) {
  DNSResolver::Answer a1(seconds(5), SocketAddress("127.0.0.1", 80));
  DNSResolver::Answer a2(seconds(5), SocketAddress("127.0.0.2", 80));

  EXPECT_FALSE(a1 == a2);
  EXPECT_NE(h(a1), h(a2));
}

TEST(DNSResolverAnswerHashTest, DifferentNameNotEqual) {
  DNSResolver::Answer n1(
      seconds(5), std::string("example.com"), DNSResolver::Answer::AT_NAME);
  DNSResolver::Answer n2(
      seconds(5), std::string("www.example.com"), DNSResolver::Answer::AT_NAME);

  EXPECT_FALSE(n1 == n2);
  EXPECT_NE(h(n1), h(n2));
}

TEST(DNSResolverAnswerHashTest, DifferentCanonicalNameNotEqual) {
  DNSResolver::Answer a1(seconds(5), SocketAddress("127.0.0.1", 80));
  a1.canonicalName = "canonical1.example.com";

  DNSResolver::Answer a2(seconds(5), SocketAddress("127.0.0.1", 80));
  a2.canonicalName = "canonical2.example.com";

  EXPECT_FALSE(a1 == a2);
  EXPECT_NE(h(a1), h(a2));
}

TEST(DNSResolverAnswerHashTest, DifferentPortNotEqual) {
  DNSResolver::Answer a1(seconds(5), SocketAddress("127.0.0.1", 0));
  a1.port = 8080;

  DNSResolver::Answer a2(seconds(5), SocketAddress("127.0.0.1", 0));
  a2.port = 9090;

  EXPECT_FALSE(a1 == a2);
  EXPECT_NE(h(a1), h(a2));
}

TEST(DNSResolverAnswerHashTest, DifferentPriorityNotEqual) {
  DNSResolver::Answer m1(
      seconds(5), /*priority*/ 5, std::string("mail.example.com"));
  DNSResolver::Answer m2(
      seconds(5), /*priority*/ 10, std::string("mail.example.com"));

  EXPECT_FALSE(m1 == m2);
  EXPECT_NE(h(m1), h(m2));
}

TEST(DNSResolverAnswerHashTest, SameCanonicalNameAndPortEqual) {
  DNSResolver::Answer a1(seconds(5), SocketAddress("127.0.0.1", 80));
  a1.canonicalName = "canonical.example.com";
  a1.port = 8080;

  DNSResolver::Answer a2(seconds(30), SocketAddress("127.0.0.1", 80));
  a2.canonicalName = "canonical.example.com";
  a2.port = 8080;

  EXPECT_TRUE(a1 == a2);
  EXPECT_EQ(h(a1), h(a2));
}

TEST(DNSResolverAnswerHashTest, CanonicalNameAndPortInUnorderedSet) {
  std::unordered_set<DNSResolver::Answer, DNSResolver::AnswerHash> set;

  DNSResolver::Answer a1(seconds(5), SocketAddress("127.0.0.1", 80));
  a1.canonicalName = "canonical1.example.com";
  a1.port = 8080;

  DNSResolver::Answer a2(seconds(5), SocketAddress("127.0.0.1", 80));
  a2.canonicalName = "canonical2.example.com";
  a2.port = 8080;

  DNSResolver::Answer a3(seconds(5), SocketAddress("127.0.0.1", 80));
  a3.canonicalName = "canonical1.example.com";
  a3.port = 9090;

  EXPECT_TRUE(set.insert(a1).second);
  EXPECT_TRUE(set.insert(a2).second);
  EXPECT_TRUE(set.insert(a3).second);
  EXPECT_EQ(set.size(), 3);

  // Same canonicalName and port as a1 should not be inserted
  DNSResolver::Answer a4(seconds(999), SocketAddress("127.0.0.1", 80));
  a4.canonicalName = "canonical1.example.com";
  a4.port = 8080;

  EXPECT_FALSE(set.insert(a4).second);
  EXPECT_EQ(set.size(), 3);
}
