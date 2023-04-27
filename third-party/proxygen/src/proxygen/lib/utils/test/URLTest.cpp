/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/URL.h>

#include <folly/portability/GTest.h>

using namespace proxygen;

TEST(URLTest, Root) {
  URL u("http", "www.facebook.com", 0);
  EXPECT_TRUE(u.isValid());
  EXPECT_EQ(u.getUrl(), "http://www.facebook.com/");
}

TEST(URLTest, CapitalSheme) {
  URL u("HTTPS", "www.facebook.com", 0);
  EXPECT_TRUE(u.isValid());
  EXPECT_EQ(u.getUrl(), "https://www.facebook.com/");
}

TEST(URLTest, Invalid) {
  URL u1("https://www.facebook.com/foo\xff", true, URL::Mode::STRICT);
  EXPECT_FALSE(u1.isValid());
  EXPECT_EQ(u1.getHost(), "");
  EXPECT_EQ(u1.getPath(), "");
  URL u2("https://www.facebook.com/foo\xff", true, URL::Mode::STRICT_COMPAT);
  EXPECT_TRUE(u2.isValid());
  EXPECT_EQ(u2.getHost(), "www.facebook.com");
  EXPECT_EQ(u2.getPath(), "/foo\xff");
}

TEST(URLTest, NonHTTPScheme) {
  URL u1("masque://www.facebook.com/foo", true, URL::Mode::STRICT);
  // Invalid, but host is still set
  EXPECT_FALSE(u1.isValid());
  EXPECT_EQ(u1.getHost(), "www.facebook.com");
  EXPECT_EQ(u1.getPath(), "/foo");
}

TEST(URLTest, GetPort) {
  URL u1("https://www.facebook.com/foo", true, URL::Mode::STRICT);
  EXPECT_TRUE(u1.isValid());
  EXPECT_EQ(u1.getPort(), 443);
  EXPECT_EQ(u1.getHostAndPortOmitDefault(), "www.facebook.com");

  URL u2("http://www.facebook.com/foo", true, URL::Mode::STRICT);
  EXPECT_TRUE(u2.isValid());
  EXPECT_EQ(u2.getPort(), 80);
  EXPECT_EQ(u2.getHostAndPortOmitDefault(), "www.facebook.com");

  URL u3("http://www.facebook.com:8081/foo", true, URL::Mode::STRICT);
  EXPECT_TRUE(u3.isValid());
  EXPECT_EQ(u3.getPort(), 8081);
  EXPECT_EQ(u3.getHostAndPortOmitDefault(), "www.facebook.com:8081");

  URL u4("https://www.facebook.com:8081/foo", true, URL::Mode::STRICT);
  EXPECT_TRUE(u4.isValid());
  EXPECT_EQ(u4.getPort(), 8081);
  EXPECT_EQ(u4.getHostAndPortOmitDefault(), "www.facebook.com:8081");
}
