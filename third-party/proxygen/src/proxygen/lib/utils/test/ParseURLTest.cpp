/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/ParseURL.h>

#include <folly/portability/GTest.h>

using proxygen::ParseURL;
using std::string;

void testParseURL(const string& url,
                  const string& expectedScheme,
                  const string& expectedPath,
                  const string& expectedQuery,
                  const string& expectedHost,
                  const uint16_t expectedPort,
                  const string& expectedAuthority,
                  const bool expectedValid = true,
                  const bool strict = false) {
  auto u = ParseURL::parseURLMaybeInvalid(url, strict);

  if (expectedValid) {
    EXPECT_EQ(url, u.url());
    EXPECT_EQ(expectedScheme, u.scheme());
    EXPECT_EQ(expectedPath, u.path());
    EXPECT_EQ(expectedQuery, u.query());
    EXPECT_EQ(expectedHost, u.host());
    EXPECT_EQ(expectedPort, u.port());
    EXPECT_EQ(expectedAuthority, u.authority());
    EXPECT_EQ(expectedValid, u.valid());
  } else {
    // invalid, do not need to test values
    EXPECT_EQ(expectedValid, u.valid());
  }
}

void testHostIsIpAddress(const string& url, const bool expected) {
  auto u = ParseURL::parseURLMaybeInvalid(url);
  EXPECT_TRUE(!expected || u.valid());
  EXPECT_EQ(expected, u.hostIsIPAddress());
}

TEST(ParseURL, HostNoBrackets) {
  auto p = ParseURL::parseURL("/bar");

  EXPECT_EQ("", p->host());
  EXPECT_EQ("", p->hostNoBrackets());
}

TEST(ParseURL, FullyFormedURL) {
  testParseURL("http://localhost:80/foo?bar#qqq",
               "http",
               "/foo",
               "bar",
               "localhost",
               80,
               "localhost:80");
  testParseURL("http://localhost:80/foo?bar",
               "http",
               "/foo",
               "bar",
               "localhost",
               80,
               "localhost:80");
  testParseURL("http://localhost:80/foo",
               "http",
               "/foo",
               "",
               "localhost",
               80,
               "localhost:80");
  testParseURL(
      "http://localhost:80/", "http", "/", "", "localhost", 80, "localhost:80");
  testParseURL(
      "http://localhost:80", "http", "", "", "localhost", 80, "localhost:80");
  testParseURL("http://localhost", "http", "", "", "localhost", 0, "localhost");
  testParseURL("http://[2401:db00:2110:3051:face:0:3f:0]/",
               "http",
               "/",
               "",
               "[2401:db00:2110:3051:face:0:3f:0]",
               0,
               "[2401:db00:2110:3051:face:0:3f:0]");
}

TEST(ParseURL, ValidNonHttpScheme) {
  testParseURL("https://localhost:80/foo?bar#qqq",
               "https",
               "/foo",
               "bar",
               "localhost",
               80,
               "localhost:80");
  testParseURL("rtmp://localhost:80/foo?bar#qqq",
               "rtmp",
               "/foo",
               "bar",
               "localhost",
               80,
               "localhost:80");
  testParseURL("ftp://localhost:80/foo?bar#qqq",
               "ftp",
               "/foo",
               "bar",
               "localhost",
               80,
               "localhost:80");
  testParseURL("proxygen://localhost:80/foo?bar#qqq",
               "proxygen",
               "/foo",
               "bar",
               "localhost",
               80,
               "localhost:80");
  testParseURL("test://localhost:80/foo?bar#qqq",
               "test",
               "/foo",
               "bar",
               "localhost",
               80,
               "localhost:80");
}

TEST(ParseURL, InvalidScheme) {
  testParseURL("test123://localhost:80", "", "", "", "", 0, "", false);
  testParseURL("test.1://localhost:80", "", "", "", "", 0, "", false);
  testParseURL("://localhost:80", "", "", "", "", 0, "", false);
  testParseURL("123://localhost:80", "", "", "", "", 0, "", false);
  testParseURL("---://localhost:80", "", "", "", "", 0, "", false);
}

TEST(ParseURL, NoScheme) {
  testParseURL("localhost:80/foo?bar#qqq",
               "",
               "/foo",
               "bar",
               "localhost",
               80,
               "localhost:80");
  testParseURL("localhost:80/foo?bar",
               "",
               "/foo",
               "bar",
               "localhost",
               80,
               "localhost:80");
  testParseURL(
      "localhost:80/foo", "", "/foo", "", "localhost", 80, "localhost:80");
  testParseURL("localhost:80/", "", "/", "", "localhost", 80, "localhost:80");
  testParseURL("localhost:80", "", "", "", "localhost", 80, "localhost:80");
  testParseURL("localhost", "", "", "", "localhost", 0, "localhost");
}

TEST(ParseURL, NoSchemeIP) {
  testParseURL("1.2.3.4:54321/foo?bar#qqq",
               "",
               "/foo",
               "bar",
               "1.2.3.4",
               54321,
               "1.2.3.4:54321");
  testParseURL("[::1]:80/foo?bar", "", "/foo", "bar", "[::1]", 80, "[::1]:80");
  testParseURL("[::1]/foo?bar", "", "/foo", "bar", "[::1]", 0, "[::1]");
}

TEST(ParseURL, PathOnly) {
  testParseURL("/f/o/o?bar#qqq", "", "/f/o/o", "bar", "", 0, "");
  testParseURL("/f/o/o?bar", "", "/f/o/o", "bar", "", 0, "");
  testParseURL("/f/o/o", "", "/f/o/o", "", "", 0, "");
  testParseURL("/", "", "/", "", "", 0, "");
  testParseURL("?foo=bar", "", "", "foo=bar", "", 0, "");
  testParseURL("?#", "", "", "", "", 0, "");
  testParseURL("#/foo/bar", "", "", "", "", 0, "");
}

TEST(ParseURL, QueryIsURL) {
  testParseURL("/?ids=http://vlc.afreecodec.com/download/",
               "",
               "/",
               "ids=http://vlc.afreecodec.com/download/",
               "",
               0,
               "");
  testParseURL("/plugins/facepile.php?href=http://www.vakan.nl/hotels",
               "",
               "/plugins/facepile.php",
               "href=http://www.vakan.nl/hotels",
               "",
               0,
               "");
}

TEST(ParseURL, InvalidURL) {
  testParseURL("http://tel:198433511/", "", "", "", "", 0, "", false);
  testParseURL("localhost:80/foo#bar?qqq", "", "", "", "", 0, "", false);
  testParseURL("#?", "", "", "", "", 0, "", false);
  testParseURL("#?hello", "", "", "", "", 0, "", false);
  testParseURL("[::1/foo?bar", "", "", "", "", 0, "", false);
  testParseURL("", "", "", "", "", 0, "", false);
  testParseURL("http://tel:198433511/test\n", "", "", "", "", 0, "", false);
  testParseURL("/test\n", "", "", "", "", 0, "", false);
  testParseURL(
      "http://foo.com/test\xff", "", "", "", "", 0, "", false, /*strict=*/true);
  testParseURL("http://foo.com/test\xff",
               "http",
               "/test\xff",
               "",
               "foo.com",
               0,
               "foo.com",
               true,
               /*strict=*/false);
  testParseURL("test\xff", "", "", "", "", 0, "", false, /*strict=*/true);
  testParseURL(
      "/test\xff", "", "/test\xff", "", "", 0, "", true, /*strict=*/false);
}

TEST(ParseURL, IsHostIPAddress) {
  testHostIsIpAddress("http://127.0.0.1:80", true);
  testHostIsIpAddress("127.0.0.1", true);
  testHostIsIpAddress("http://[::1]:80", true);
  testHostIsIpAddress("[::1]", true);
  testHostIsIpAddress("[::AB]", true);

  testHostIsIpAddress("http://localhost:80", false);
  testHostIsIpAddress("http://localhost", false);
  testHostIsIpAddress("localhost", false);
  testHostIsIpAddress("1.2.3.-1", false);
  testHostIsIpAddress("1.2.3.999", false);
  testHostIsIpAddress("::1", false);
  testHostIsIpAddress("[::99999999]", false);

  // invalid url
  testHostIsIpAddress("", false);
  testHostIsIpAddress("127.0.0.1:80/foo#bar?qqq", false);
}

TEST(ParseURL, PortOverflow) {
  std::string url("http://foo:12345");
  auto u =
      ParseURL::parseURL(folly::StringPiece(url.data(), url.size() - 4), true);
  EXPECT_EQ(u->port(), 1);
}

TEST(ParseURL, GetQueryParam) {
  auto u = ParseURL::parseURL("localhost/?foo=1&bar=2&baz&bazz=3&bak=");
  ASSERT_TRUE(u.hasValue());
  EXPECT_EQ(u->getQueryParam("foo"), "1");
  EXPECT_EQ(u->getQueryParam("bar"), "2");
  EXPECT_EQ(u->getQueryParam("baz"), "");
  EXPECT_EQ(u->getQueryParam("bazz"), "3");
  EXPECT_EQ(u->getQueryParam("bak"), "");
  EXPECT_FALSE(u->getQueryParam("fooo").has_value());
}
