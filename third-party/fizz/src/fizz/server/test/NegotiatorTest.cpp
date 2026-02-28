/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/server/Negotiator.h>

namespace fizz {
namespace server {
namespace test {

TEST(NegotiatorTest, TestSingle) {
  std::vector<std::vector<int>> server = {{1}};
  std::vector<int> client = {1};

  EXPECT_EQ(*negotiate(server, client), 1);
}

TEST(NegotiatorTest, TestSingleMismatch) {
  std::vector<std::vector<int>> server = {{1}};
  std::vector<int> client = {2};

  EXPECT_FALSE(negotiate(server, client).has_value());
}

TEST(NegotiatorTest, TestServerPref) {
  std::vector<std::vector<int>> server = {{1}, {2}, {3}};
  std::vector<int> client = {3, 1, 2};

  EXPECT_EQ(*negotiate(server, client), 1);
}

TEST(NegotiatorTest, TestServerPrefTie) {
  std::vector<std::vector<int>> server = {{1}, {2, 4}, {3}};
  std::vector<int> client = {5, 6, 4, 2};

  EXPECT_EQ(*negotiate(server, client), 4);
}

TEST(NegotiateTest, TestSingleOrdering) {
  std::vector<int> server = {1, 2, 3};
  std::vector<int> client = {3, 1, 2};

  EXPECT_EQ(*negotiate(server, client), 1);
}

TEST(NegotiateTest, TestSingleOrderingLast) {
  std::vector<int> server = {4, 5, 3};
  std::vector<int> client = {3, 1, 2};

  EXPECT_EQ(*negotiate(server, client), 3);
}

TEST(NegotiateTest, TestSingleOrderingNoMatch) {
  std::vector<int> server = {1, 5, 6};
  std::vector<int> client = {3, 4, 2};

  EXPECT_FALSE(negotiate(server, client).has_value());
}

TEST(NegotiateTest, TestServerEmptyTier) {
  std::vector<std::vector<int>> server = {{}, {4}};
  std::vector<int> client = {3, 4, 2};

  EXPECT_EQ(*negotiate(server, client), 4);
}
} // namespace test
} // namespace server
} // namespace fizz
