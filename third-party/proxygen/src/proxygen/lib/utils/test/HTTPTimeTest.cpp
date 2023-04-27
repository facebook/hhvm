/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/HTTPTime.h>

#include <folly/portability/GTest.h>

using proxygen::parseHTTPDateTime;

TEST(HTTPTimeTests, InvalidTimeTest) {
  EXPECT_FALSE(parseHTTPDateTime("Hello, World").has_value());
  EXPECT_FALSE(parseHTTPDateTime("Sun, 33 Nov 1994 08:49:37 GMT").has_value());
  EXPECT_FALSE(parseHTTPDateTime("Sun, 06 Nov 1800").has_value());
}

TEST(HTTPTimeTests, ValidTimeTest) {
  // From http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3
  EXPECT_TRUE(parseHTTPDateTime("Sun, 06 Nov 1994 08:49:37 GMT").has_value());
  EXPECT_TRUE(parseHTTPDateTime("Sunday, 06-Nov-94 08:49:37 GMT").has_value());
  EXPECT_TRUE(parseHTTPDateTime("Sun Nov  6 08:49:37 1994").has_value());
}

TEST(HTTPTimeTests, EqualTimeTest) {
  auto a = parseHTTPDateTime("Thu, 07 Mar 2013 08:49:37 GMT");
  EXPECT_TRUE(a.has_value());
  auto b = parseHTTPDateTime("Thursday, 07-Mar-13 08:49:37 GMT");
  EXPECT_TRUE(b.has_value());
  auto c = parseHTTPDateTime("Thu Mar 7 08:49:37 2013");
  EXPECT_TRUE(c.has_value());

  EXPECT_EQ(a.value(), b.value());
  EXPECT_EQ(a.value(), c.value());
  EXPECT_EQ(b.value(), c.value());
}

TEST(HTTPTimeTests, ReallyOldTimeTest) {
  auto a = parseHTTPDateTime("Thu, 07 Mar 1970 08:49:37 GMT");
  EXPECT_TRUE(a.has_value());
  auto b = parseHTTPDateTime("Thu, 07 Mar 1971 08:49:37 GMT");
  EXPECT_TRUE(b.has_value());
  auto c = parseHTTPDateTime("Thu, 07 Mar 1980 08:49:37 GMT");
  EXPECT_TRUE(c.has_value());

  EXPECT_LT(a, b);
  EXPECT_LT(a, c);
  EXPECT_LT(b, c);
}

TEST(HTTPTimeTests, TzToUnixTsTest) {
  auto a = parseHTTPDateTime("Wed, 13 Jun 2018 21:43:49 GMT");
  EXPECT_EQ(a.value(), 1528926229);
  auto b = parseHTTPDateTime("Wed, 31 Dec 1969 23:59:59 GMT");
  EXPECT_EQ(b.value(), -1);
  auto c = parseHTTPDateTime("Thu, 01 Jan 1970 00:00:00 GMT");
  EXPECT_EQ(c.value(), 0);
  auto d = parseHTTPDateTime("Thu, 01 Jan 1970 00:00:01 GMT");
  EXPECT_EQ(d.value(), 1);

  auto e = parseHTTPDateTime("Wed, 13-Jun-18 21:43:49 GMT");
  EXPECT_EQ(e.value(), 1528926229);
  auto f = parseHTTPDateTime("Wed, 31-Dec-69 23:59:59 GMT");
  EXPECT_EQ(f.value(), -1);
  auto g = parseHTTPDateTime("Wed, 01-Jan-70 00:00:00 GMT");
  EXPECT_EQ(g.value(), 0);
  auto h = parseHTTPDateTime("Thu, 01-Jan-70 00:00:01 GMT");
  EXPECT_EQ(h.value(), 1);

  auto i = parseHTTPDateTime("Wed Jun 13 21:43:49 2018");
  EXPECT_EQ(i.value(), 1528926229);
  auto j = parseHTTPDateTime("Wed Dec 31 23:59:59 1969");
  EXPECT_EQ(j.value(), -1);
  auto k = parseHTTPDateTime("Thu Jan 1 00:00:00 1970");
  EXPECT_EQ(k.value(), 0);
  auto l = parseHTTPDateTime("Thu Jan 1 00:00:01 1970");
  EXPECT_EQ(l.value(), 1);

  auto m = parseHTTPDateTime("Tue, 19 Jan 2038 03:14:07 GMT");
  EXPECT_EQ(m.value(), 2147483647);

  auto n = parseHTTPDateTime("Thu, 01 Jan 1970 00:00:01 PST");
  EXPECT_FALSE(n.has_value());
  auto o = parseHTTPDateTime("Thu, 01 Jan 1970 00:00:01");
  EXPECT_FALSE(o.has_value());
}
