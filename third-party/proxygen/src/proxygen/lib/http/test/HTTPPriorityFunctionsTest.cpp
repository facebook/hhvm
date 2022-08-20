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
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=5");

  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_EQ(priority->urgency, 5);
  EXPECT_FALSE(priority->incremental);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderUrgencyAndIncremental) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=4,i");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_EQ(priority->urgency, 4);
  EXPECT_TRUE(priority->incremental);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderUrgencyAndIncrementalUppercase) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=4, i");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_EQ(priority->urgency, 4);
  EXPECT_TRUE(priority->incremental);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderUrgencyAndIncrementalTrimSpaces) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=4,  i ");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_EQ(priority->urgency, 4);
  EXPECT_TRUE(priority->incremental);
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderBadUrgency) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "p=3");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderBadUrgencyWithIncremental) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "p=3, i");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());
}

TEST(HTTPPriorityFunctionsTest, PriorityHeaderBadIncremental) {
  HTTPMessage req;
  req.getHeaders().add(HTTP_HEADER_PRIORITY, "u=3, true");
  auto priority = httpPriorityFromHTTPMessage(req);
  EXPECT_FALSE(priority.hasValue());
}
