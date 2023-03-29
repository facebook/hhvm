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

  // urgency with other random fields
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "u=3, true");
  priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 3);
  EXPECT_FALSE(priority->incremental);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderIncrementalOnly) {
  HTTPMessage req;

  // incremental only
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "i");
  auto priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 3);
  EXPECT_TRUE(priority->incremental);

  // incremental with other random fields
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "i, true");
  priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 3);
  EXPECT_TRUE(priority->incremental);

  // incremental with int "truthy" value
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "i=?1");
  priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 3);
  EXPECT_TRUE(priority->incremental);

  // incremental with int "falsy" value
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "i=?0");
  priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 3);
  EXPECT_FALSE(priority->incremental);
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
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderUrgencyAndIncremental) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=4,i");
  auto priority = httpPriorityFromHTTPMessage(req);
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 4);
  EXPECT_TRUE(priority->incremental);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderUrgencyAndIncrementalUppercase) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "U=4, I");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderBadUrgency) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "p=3");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());

  // same as above but with incremental flag
  req.getHeaders().set(HTTP_HEADER_PRIORITY, "p=2, i");
  priority = httpPriorityFromHTTPMessage(req);
  // default to u=3 if urgency is missing but incremental is present
  ASSERT_TRUE(priority.hasValue());
  EXPECT_EQ(priority->urgency, 3);
  EXPECT_TRUE(priority->incremental);

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

TEST(HTTPPriorityFunctionsTest, PriorityHeaderBadPriority) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=3, i=0");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderDefaultOrderId) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=3");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_EQ(priority->orderId, 0);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderCustomOrderId) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=3, o=100");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_EQ(priority->urgency, 3);
  EXPECT_EQ(priority->orderId, 100);
}
