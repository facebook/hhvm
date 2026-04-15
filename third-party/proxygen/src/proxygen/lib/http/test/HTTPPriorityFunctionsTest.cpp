/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/HTTPPriorityFunctions.h>

using namespace proxygen;

TEST(HTTPPriorityFunctionsTest, PriorityHeaderUrgencyOnly) {
  HTTPMessage req;

  // urgency only
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=5");
  auto priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 5);
  EXPECT_FALSE(priority->incremental);
  EXPECT_EQ(priority->requiredBps, 0);

  // urgency with other random fields
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "u=3, true");
  priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 3);
  EXPECT_FALSE(priority->incremental);
  EXPECT_EQ(priority->requiredBps, 0);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderIncrementalOnly) {
  HTTPMessage req;

  // incremental only
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "i");
  auto priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 3);
  EXPECT_TRUE(priority->incremental);
  EXPECT_EQ(priority->requiredBps, 0);

  // incremental with other random fields
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "i, true");
  priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 3);
  EXPECT_TRUE(priority->incremental);
  EXPECT_EQ(priority->requiredBps, 0);

  // incremental with int "truthy" value
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "i=?1");
  priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 3);
  EXPECT_TRUE(priority->incremental);
  EXPECT_EQ(priority->requiredBps, 0);

  // incremental with int "falsy" value
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "i=?0");
  priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 3);
  EXPECT_FALSE(priority->incremental);
  EXPECT_EQ(priority->requiredBps, 0);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderWhiteSpace) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "    ");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());

  req.getHeaders().set(HTTP_HEADER_PRIORITY, "u=4,  i ");
  priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 4);
  EXPECT_TRUE(priority->incremental);
  EXPECT_EQ(priority->requiredBps, 0);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderUrgencyAndIncremental) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=4,i");
  auto priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 4);
  EXPECT_TRUE(priority->incremental);
  EXPECT_EQ(priority->requiredBps, 0);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderUrgencyAndIncrementalUppercase) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "U=4, I");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderBadUrgency) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "x=3");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());

  // same as above but with incremental flag
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "x=2, i");
  priority = httpPriorityFromHTTPMessage(req);
  // default to u=3 if urgency is missing but incremental is present
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 3);
  EXPECT_TRUE(priority->incremental);
  EXPECT_EQ(priority->requiredBps, 0);

  // urgency > kMaxPriority should fail
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "u=8");
  priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());

  // urgency < kMinPriority should fail
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "u=-2");
  priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());

  // urgency non-integral type should fail
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "u=banana,i");
  priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderBadIncremental) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=3, i=0");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderBadPaused) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=3, p=0");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderDefaultOrderId) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=3");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_EQ(priority->orderId, 0);
  EXPECT_EQ(priority->requiredBps, 0);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderCustomOrderId) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=3, o=100");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_EQ(priority->urgency, 3);
  EXPECT_EQ(priority->orderId, 100);
  EXPECT_EQ(priority->requiredBps, 0);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderDefaultUnpaused) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=3");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority->paused);
  EXPECT_EQ(priority->requiredBps, 0);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderPaused) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=3, p");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_TRUE(priority->paused);
  EXPECT_EQ(priority->requiredBps, 0);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderRequiredRateValid) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=0, i, r=5000000");
  auto priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 0);
  EXPECT_TRUE(priority->incremental);
  EXPECT_EQ(priority->requiredBps, 5000000);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderRequiredRateZero) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=3, r=0");
  auto priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 3);
  EXPECT_EQ(priority->requiredBps, 0);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderRequiredRateOnly) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "r=1000000");
  auto priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, kDefaultHttpPriorityUrgency);
  EXPECT_FALSE(priority->incremental);
  EXPECT_EQ(priority->requiredBps, 1000000);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderBadRequiredRate) {
  HTTPMessage req;

  // negative value rejects the entire header
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=3, r=-100");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());

  // non-integral type fails at structured header parsing level
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "u=5, i, r=invalid");
  priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());

  // uppercase R fails at structured header parsing level
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "u=3, R=5000000");
  priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderRequiredRateLarge) {
  HTTPMessage req;
  // 100 Gbps
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=0, r=100000000000");
  auto priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 0);
  EXPECT_EQ(priority->requiredBps, 100000000000ULL);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderRequiredRateZeroVsOmitted) {
  HTTPMessage req;
  // r=0 is treated as missing (no required rate)
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=3, r=0");
  auto withZero = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(withZero.hasValue());
  EXPECT_EQ(withZero->requiredBps, 0);
  // r omitted means no required rate (requiredBps == 0)
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "u=3");
  auto withOmitted = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(withOmitted.hasValue());
  EXPECT_EQ(withOmitted->requiredBps, 0);
  // r=0 and omitted are now equivalent
  EXPECT_EQ(withZero->requiredBps, withOmitted->requiredBps);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderAllParameters) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=2, i, o=50, p, r=10000000");
  auto priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 2);
  EXPECT_TRUE(priority->incremental);
  EXPECT_EQ(priority->orderId, 50);
  EXPECT_TRUE(priority->paused);
  EXPECT_EQ(priority->requiredBps, 10000000);
}

TEST(HTTPPriorityFunctionsTest, PriorityToStringWithRequiredRate) {
  HTTPPriority prio(0, true, 0, false, 5000000);
  std::string result = httpPriorityToString(prio);
  EXPECT_EQ(result, "u=0,i,r=5000000");
}

TEST(HTTPPriorityFunctionsTest, PriorityToStringNoRequiredRate) {
  HTTPPriority prio(3, false, 0, false, 0);
  std::string result = httpPriorityToString(prio);
  EXPECT_EQ(result, "u=3");
}

TEST(HTTPPriorityFunctionsTest, PriorityEqualityWithRequiredRate) {
  HTTPPriority a(1, true, 10, false, 5000000);
  HTTPPriority b(1, true, 10, false, 5000000);
  HTTPPriority c(1, true, 10, false, 6000000);
  HTTPPriority d(1, true, 10, false, 0);
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a == c);
  EXPECT_FALSE(a == d);
}
