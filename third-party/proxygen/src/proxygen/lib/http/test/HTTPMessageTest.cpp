/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/HTTPMessage.h>

#include <fcntl.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/HTTPPriorityFunctions.h>
#include <proxygen/lib/utils/TestUtils.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>

using namespace proxygen;
using namespace std;

TEST(HTTPMessage, TestParseCookiesSimple) {
  HTTPMessage msg;

  msg.getHeaders().add("Cookie", "id=1256679245; data=0:1234567");
  EXPECT_EQ(msg.getCookie("id"), "1256679245");
  EXPECT_EQ(msg.getCookie("data"), "0:1234567");
  EXPECT_EQ(msg.getCookie("mising"), "");
}

TEST(HTTPMessage, TestParseCookiesSpaces) {
  HTTPMessage msg;

  msg.getHeaders().add("Cookie", " id=1256679245 ;   data=0:1234567  ;");
  EXPECT_EQ(msg.getCookie("id"), "1256679245");
  EXPECT_EQ(msg.getCookie("data"), "0:1234567");
}

TEST(HTTPMessage, TestParseCookiesSingleCookie) {
  HTTPMessage msg;

  msg.getHeaders().add("Cookie", "   user_id=1256679245  ");
  EXPECT_EQ(msg.getCookie("user_id"), "1256679245");
}

TEST(HTTPMessage, TestParseCookiesMultipleCookies) {
  HTTPMessage msg;

  msg.getHeaders().add("Cookie",
                       "id=1256679245; data=0:1234567; same=Always; Name");
  msg.getHeaders().add("Cookie",
                       "id2=1256679245; data2=0:1234567; same=Always; ");
  EXPECT_EQ(msg.getCookie("id"), "1256679245");
  EXPECT_EQ(msg.getCookie("id2"), "1256679245");
  EXPECT_EQ(msg.getCookie("data"), "0:1234567");
  EXPECT_EQ(msg.getCookie("data2"), "0:1234567");
  EXPECT_EQ(msg.getCookie("same"), "Always");
  EXPECT_EQ(msg.getCookie("Name"), "");
}

TEST(HTTPMessage, TestParseQueryParamsSimple) {
  HTTPMessage msg;
  string url =
      "/test?seq=123456&userid=1256679245&dup=1&dup=2&helloWorld"
      "&second=was+it+clear+%28already%29%3F";

  msg.setURL(url);
  EXPECT_EQ(msg.getQueryParam("seq"), "123456");
  EXPECT_EQ(msg.getQueryParam("userid"), "1256679245");
  EXPECT_EQ(msg.getQueryParam("dup"), "2");
  EXPECT_EQ(msg.getQueryParam("helloWorld"), "");
  EXPECT_EQ(msg.getIntQueryParam("dup", 5), 2);
  EXPECT_EQ(msg.getIntQueryParam("abc", 5), 5);
  EXPECT_EQ(msg.getDecodedQueryParam("second"), "was it clear (already)?");
  EXPECT_EQ(msg.getDecodedQueryParam("seq"), "123456");
  EXPECT_EQ(msg.hasQueryParam("seq"), true);
  EXPECT_EQ(msg.hasQueryParam("seq1"), false);
  EXPECT_EQ(msg.getIntQueryParam("dup"), 2);
  EXPECT_ANY_THROW(msg.getIntQueryParam("abc"));
  EXPECT_ANY_THROW(msg.getIntQueryParam("second"));

  const auto& param = msg.getQueryParam("seq");
  msg.setQueryParam("foo", "bar");
  EXPECT_EQ(param, "123456");
  msg.removeQueryParam("foo");
  EXPECT_EQ(param, "123456");
}

TEST(HTTPMessage, TestParseQueryParamsComplex) {
  HTTPMessage msg;
  std::vector<std::vector<std::string>> input = {
      {"", "", ""},
      {"key_and_equal_but_no_value", "=", ""},
      {"only_key", "", ""},
      {"key_and_value", "=", "value"},
      {"key_and_value_2", "=", "value2"},
      {"key_and_value_3", "=", "value3"}};

  for (int i = 0; i < (1 << input.size()); ++i) {
    std::vector<std::vector<std::string>> sub;
    for (size_t j = 0; j < input.size(); ++j) {
      if ((i & (1 << j))) {
        sub.push_back(input[j]);
      }
    }

    sort(sub.begin(), sub.end());
    do {
      bool first = true;
      std::string url = "/test?";
      for (const auto& val : sub) {
        if (first) {
          first = false;
        } else {
          url += "&";
        }

        url += val[0] + val[1] + val[2];
      }

      msg.setURL(url);
      for (const auto& val : sub) {
        if (val[0].empty()) {
          continue;
        }

        EXPECT_EQ(val[2], msg.getQueryParam(val[0]));
      }

    } while (next_permutation(sub.begin(), sub.end()));
  }
}

TEST(HTTPMessage, SetInvalidURL) {
  HTTPMessage msg;

  msg.setURL("http://www.foooooooooooooooooooo.com/bar");
  EXPECT_EQ(msg.getPathAsStringPiece(), "/bar");
  msg.setURL("/\t/?tbtkkukgrenncdlvlgbigerblcgjbkgb=1");
  EXPECT_EQ(msg.getPathAsStringPiece(), "");
}

TEST(HTTPMessage, SetURLEmpty) {
  HTTPMessage msg;

  auto res = msg.setURL("");
  EXPECT_FALSE(res.valid());
  EXPECT_EQ(msg.getPathAsStringPiece(), "");
}

TEST(HTTPMessage, SetAbsoluteURLNoPath) {
  HTTPMessage msg;

  auto res = msg.setURL("http://www.foo.com");
  EXPECT_TRUE(res.valid());
  EXPECT_EQ(msg.getPathAsStringPiece(), "/");
  EXPECT_EQ(msg.getPath(), msg.getPathAsStringPiece());
  // getPathAsStringPiece points to constant string and getPath points to copy
  EXPECT_NE(msg.getPath().data(), msg.getPathAsStringPiece().begin());
}

TEST(HTTPMessage, TestHeaderPreservation) {
  HTTPMessage msg;
  HTTPHeaders& hdrs = msg.getHeaders();

  hdrs.add("Jojo", "1");
  hdrs.add("Binky", "2");
  hdrs.add("jOJo", "3");
  hdrs.add("bINKy", "4");

  EXPECT_EQ(hdrs.size(), 4);
  EXPECT_EQ(hdrs.getNumberOfValues("jojo"), 2);
  EXPECT_EQ(hdrs.getNumberOfValues("binky"), 2);
}

TEST(HTTPMessage, TestHeaderRemove) {
  HTTPMessage msg;
  HTTPHeaders& hdrs = msg.getHeaders();

  hdrs.add("Jojo", "1");
  hdrs.add("Binky", "2");
  hdrs.add("jOJo", "3");
  hdrs.add("bINKy", "4");
  hdrs.remove("jojo");

  EXPECT_EQ(hdrs.size(), 2);
  EXPECT_EQ(hdrs.getNumberOfValues("binky"), 2);
}

TEST(HTTPMessage, TestAllVersionsHeaderRemove) {
  HTTPMessage msg;
  HTTPHeaders& hdrs = msg.getHeaders();

  hdrs.add("Jojo_1_2", "1");
  hdrs.add("Binky", "2");
  hdrs.add("Jojo_1-2", "3");
  hdrs.add("Jojo-1_2", "4");
  hdrs.add("Jojo-1-2", "4");
  hdrs.removeAllVersions(HTTP_HEADER_NONE, "Jojo_1_2");
  EXPECT_EQ(hdrs.size(), 1);

  hdrs.add("Content-Length", "1");
  hdrs.add("Content_Length", "2");
  hdrs.removeAllVersions(HTTP_HEADER_CONTENT_LENGTH, "Content-Length");
  EXPECT_EQ(hdrs.size(), 1);
}

TEST(HTTPMessage, TestSetHeader) {
  HTTPMessage msg;
  HTTPHeaders& hdrs = msg.getHeaders();

  hdrs.set("Jojo", "1");
  EXPECT_EQ(hdrs.size(), 1);
  EXPECT_EQ(hdrs.getNumberOfValues("Jojo"), 1);

  hdrs.add("jojo", "2");
  hdrs.add("jojo", "3");
  hdrs.add("bar", "4");
  EXPECT_EQ(hdrs.size(), 4);
  EXPECT_EQ(hdrs.getNumberOfValues("Jojo"), 3);

  hdrs.set("joJO", "5");
  EXPECT_EQ(hdrs.size(), 2);
  EXPECT_EQ(hdrs.getNumberOfValues("Jojo"), 1);
}

TEST(HTTPMessage, TestCombine) {
  HTTPMessage msg;
  HTTPHeaders& headers = msg.getHeaders();

  EXPECT_EQ(headers.combine("Combine"), "");

  headers.add("Combine", "first value");
  EXPECT_EQ(headers.combine("Combine"), "first value");

  headers.add("Combine", "second value");
  EXPECT_EQ(headers.combine("Combine"), "first value, second value");

  headers.add("Combine", "third value");
  EXPECT_EQ(headers.combine("Combine"),
            "first value, second value, third value");
  VLOG(4) << msg;
}

TEST(HTTPMessage, TestProxification) {
  HTTPMessage msg;

  folly::SocketAddress dstAddr("192.168.1.1", 1887);
  msg.setDstAddress(dstAddr);
  msg.setLocalIp("10.0.0.1");

  msg.ensureHostHeader();
  msg.setWantsKeepalive(false);

  HTTPHeaders& hdrs = msg.getHeaders();
  EXPECT_EQ("192.168.1.1", hdrs.getSingleOrEmpty(HTTP_HEADER_HOST));
  EXPECT_FALSE(msg.wantsKeepalive());
}

TEST(HTTPMessage, TestSetClientAddress) {
  HTTPMessage msg;

  folly::SocketAddress clientAddr("74.125.127.9", 1987);
  msg.setClientAddress(clientAddr);
  EXPECT_EQ(msg.getClientIP(), "74.125.127.9");
  EXPECT_EQ(msg.getClientPort(), "1987");
  // Now try cached path
  EXPECT_EQ(msg.getClientIP(), "74.125.127.9");
  EXPECT_EQ(msg.getClientPort(), "1987");

  // Test updating client address
  folly::SocketAddress clientAddr2("74.125.127.8", 1988);
  msg.setClientAddress(clientAddr2);
  EXPECT_EQ(msg.getClientIP(), "74.125.127.8");
  EXPECT_EQ(msg.getClientPort(), "1988");
}

TEST(HTTPMessage, BizarreVersions) {
  HTTPMessage msg;

  msg.setHTTPVersion(0, 22);
  EXPECT_EQ(msg.getVersionString(), "0.22");
  msg.setHTTPVersion(10, 1);
  EXPECT_EQ(msg.getVersionString(), "10.1");
}

TEST(HTTPMessage, TestKeepaliveCheck) {
  {
    HTTPMessage msg;
    msg.setHTTPVersion(1, 0);
    EXPECT_FALSE(msg.computeKeepalive());
  }

  {
    HTTPMessage msg;
    msg.setHTTPVersion(1, 1);
    EXPECT_TRUE(msg.computeKeepalive());
  }

  {
    HTTPMessage msg;
    msg.setHTTPVersion(1, 1);
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "close");
    EXPECT_FALSE(msg.computeKeepalive());
  }

  {
    HTTPMessage msg;
    msg.setHTTPVersion(1, 1);
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "ClOsE");
    EXPECT_FALSE(msg.computeKeepalive());
  }

  {
    HTTPMessage msg;
    msg.setHTTPVersion(1, 1);
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "foo,bar");
    EXPECT_TRUE(msg.computeKeepalive());
  }

  {
    HTTPMessage msg;
    msg.setHTTPVersion(1, 1);
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "foo,bar");
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "abc,CLOSE,def");
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "xyz");
    EXPECT_FALSE(msg.computeKeepalive());
  }

  {
    HTTPMessage msg;
    msg.setHTTPVersion(1, 1);
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "foo,bar");
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "abc ,  CLOSE , def");
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "xyz");
    EXPECT_FALSE(msg.computeKeepalive());
  }

  {
    HTTPMessage msg;
    msg.setHTTPVersion(1, 1);
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "  close ");
    EXPECT_FALSE(msg.computeKeepalive());
  }

  {
    HTTPMessage msg;
    msg.setHTTPVersion(1, 1);
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, ",  close ");
    EXPECT_FALSE(msg.computeKeepalive());
  }

  {
    HTTPMessage msg;
    msg.setHTTPVersion(1, 1);
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "  close , ");
    EXPECT_FALSE(msg.computeKeepalive());
  }

  {
    HTTPMessage msg;
    msg.setHTTPVersion(1, 0);
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "Keep-Alive");
    EXPECT_TRUE(msg.computeKeepalive());
  }

  {
    HTTPMessage msg;
    msg.setHTTPVersion(1, 0);
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "keep-alive");
    EXPECT_TRUE(msg.computeKeepalive());
  }

  {
    HTTPMessage msg;
    msg.setHTTPVersion(1, 0);
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "keep-alive");
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "close");
    EXPECT_FALSE(msg.computeKeepalive());
  }

  {
    HTTPMessage msg;
    msg.setHTTPVersion(1, 0);
    msg.getHeaders().add(HTTP_HEADER_CONNECTION, "keep-alive");
    msg.stripPerHopHeaders();
    EXPECT_TRUE(msg.computeKeepalive());
  }
}

TEST(HTTPMessage, TestHeaderStripPerHop) {
  HTTPMessage msg;

  msg.getHeaders().add(HTTP_HEADER_CONNECTION, "a, b, c");
  msg.getHeaders().add(HTTP_HEADER_CONNECTION, "d");
  msg.getHeaders().add(HTTP_HEADER_CONNECTION, ",,,,");
  msg.getHeaders().add(HTTP_HEADER_CONNECTION, " , , , ,");
  msg.getHeaders().add(HTTP_HEADER_CONNECTION, ", e");
  msg.getHeaders().add(HTTP_HEADER_CONNECTION, " f ,\tg\t, \r\n\th ");
  msg.getHeaders().add("Keep-Alive", "true");
  msg.getHeaders().add("Priority", "u=5, i");

  msg.getHeaders().add("a", "1");
  msg.getHeaders().add("b", "2");
  msg.getHeaders().add("c", "3");
  msg.getHeaders().add("d", "4");
  msg.getHeaders().add("e", "5");
  msg.getHeaders().add("f", "6");
  msg.getHeaders().add("g", "7");
  msg.getHeaders().add("h", "8");

  EXPECT_EQ(msg.getHeaders().size(), 16);
  msg.stripPerHopHeaders();
  EXPECT_EQ(msg.getHeaders().size(), 1);
  EXPECT_EQ(msg.getStrippedPerHopHeaders().size(), 15);
  msg.stripPerHopHeaders(/* stripPriority */ true);
  EXPECT_EQ(msg.getHeaders().size(), 0);
  EXPECT_EQ(msg.getStrippedPerHopHeaders().size(), 1);
}

TEST(HTTPMessage, TestEmptyName) {
  HTTPMessage msg;
  EXPECT_DEATH_NO_CORE(msg.getHeaders().set("", "empty name?!"), ".*");
}

TEST(HTTPMessage, TestMethod) {
  HTTPMessage msg;

  msg.setMethod(HTTPMethod::GET);
  EXPECT_EQ("GET", msg.getMethodString());
  EXPECT_EQ(HTTPMethod::GET == msg.getMethod(), true);

  msg.setMethod("FOO");
  EXPECT_EQ("FOO", msg.getMethodString());
  EXPECT_EQ(folly::none, msg.getMethod());

  msg.setMethod(HTTPMethod::CONNECT);
  EXPECT_EQ("CONNECT", msg.getMethodString());
  EXPECT_EQ(HTTPMethod::CONNECT == msg.getMethod(), true);
}

void testPathAndQuery(const string& url,
                      const string& expectedPath,
                      const string& expectedQuery) {
  HTTPMessage msg;
  msg.setURL(url);

  EXPECT_EQ(msg.getURL(), url);
  EXPECT_EQ(msg.getPathAsStringPiece(), expectedPath);
  EXPECT_EQ(msg.getPath(), expectedPath);
  EXPECT_EQ(msg.getQueryStringAsStringPiece(), expectedQuery);
  EXPECT_EQ(msg.getQueryString(), expectedQuery);
}

TEST(GetPathAndQuery, ParseURL) {
  testPathAndQuery("http://localhost:80/foo?bar#qqq", "/foo", "bar");
  testPathAndQuery("localhost:80/foo?bar#qqq", "/foo", "bar");
  testPathAndQuery("localhost", "/", "");
  testPathAndQuery("/f/o/o?bar#qqq", "/f/o/o", "bar");
  testPathAndQuery("#?hello", "", "");
}

TEST(HTTPMessage, CheckHeaderForToken) {
  HTTPMessage msg;
  std::vector<std::pair<std::string, bool>> tests{{"close", true},
                                                  {"xclose", false},
                                                  {"closex", false},
                                                  {"close, foo", true},
                                                  {"close, ", true},
                                                  {"foo, close", true},
                                                  {", close", true},
                                                  {"close, close, ", true}};

  for (auto& test : tests) {
    msg.getHeaders().set(HTTP_HEADER_CONNECTION, test.first);
    EXPECT_TRUE(msg.checkForHeaderToken(
                    HTTP_HEADER_CONNECTION, "close", false) == test.second);
  }
}

TEST(HTTPHeaders, AddStringPiece) {
  const char foo[] = "name:value";
  HTTPHeaders headers;

  folly::StringPiece str(foo);
  folly::StringPiece name = str.split_step(':');
  headers.add(name, str);
  EXPECT_EQ("value", headers.getSingleOrEmpty("name"));
}

TEST(HTTPHeaders, InitializerList) {
  HTTPHeaders hdrs;

  hdrs.add({{"name", "value"}});
  hdrs.add({{HTTP_HEADER_CONNECTION, "close"}});
  hdrs.add(
      {{"a", "b"}, {HTTP_HEADER_CONNECTION, "foo"}, {HTTP_HEADER_SERVER, "x"}});

  EXPECT_EQ("value", hdrs.getSingleOrEmpty("name"));
  EXPECT_EQ("close, foo", hdrs.combine(HTTP_HEADER_CONNECTION));
  EXPECT_EQ("x", hdrs.getSingleOrEmpty(HTTP_HEADER_SERVER));
}

TEST(HTTPHeaders, InitializerListStringPiece) {
  HTTPHeaders hdrs;

  const char* foo = "name:value";
  folly::StringPiece str(foo);
  folly::StringPiece name = str.split_step(':');
  hdrs.add({{name, str}, {HTTP_HEADER_CONNECTION, str}});

  EXPECT_EQ("value", hdrs.getSingleOrEmpty("name"));
  EXPECT_EQ("value", hdrs.getSingleOrEmpty(HTTP_HEADER_CONNECTION));
}

void testRemoveQueryParam(const string& url,
                          const string& queryParam,
                          const string& expectedUrl,
                          const string& expectedQuery) {
  HTTPMessage msg;
  msg.setURL(url);
  bool expectedChange = (url != expectedUrl);
  EXPECT_EQ(msg.removeQueryParam(queryParam), expectedChange);

  EXPECT_EQ(msg.getURL(), expectedUrl);
  EXPECT_EQ(msg.getQueryStringAsStringPiece(), expectedQuery);
}

TEST(HTTPMessage, RemoveQueryParamTests) {
  // Query param present
  testRemoveQueryParam("http://localhost:80/foo?param1=a&param2=b#qqq",
                       "param2",
                       "http://localhost:80/foo?param1=a#qqq",
                       "param1=a");
  // Query param not present
  testRemoveQueryParam("http://localhost/foo?param1=a&param2=b#qqq",
                       "param3",
                       "http://localhost/foo?param1=a&param2=b#qqq",
                       "param1=a&param2=b");
  // No scheme
  testRemoveQueryParam("localhost:80/foo?param1=a&param2=b#qqq",
                       "param2",
                       "localhost:80/foo?param1=a#qqq",
                       "param1=a");
  // Just hostname as URL and empty query param
  testRemoveQueryParam("localhost", "param2", "localhost", "");
  testRemoveQueryParam("localhost", "", "localhost", "");
  // Just path as URL
  testRemoveQueryParam("/f/o/o?bar#qqq", "bar", "/f/o/o#qqq", "");
}

void testSetQueryParam(const string& url,
                       const string& queryParam,
                       const string& paramValue,
                       const string& expectedUrl,
                       const string& expectedQuery) {
  HTTPMessage msg;
  msg.setURL(url);
  bool expectedChange = (url != expectedUrl);
  EXPECT_EQ(msg.setQueryParam(queryParam, paramValue), expectedChange);

  EXPECT_EQ(msg.getURL(), expectedUrl);
  EXPECT_EQ(msg.getQueryStringAsStringPiece(), expectedQuery);
}

TEST(HTTPMessage, SetQueryParamTests) {
  // Overwrite existing parameter
  testSetQueryParam("http://localhost:80/foo?param1=a&param2=b#qqq",
                    "param2",
                    "true",
                    "http://localhost:80/foo?param1=a&param2=true#qqq",
                    "param1=a&param2=true");
  // Add a query parameter
  testSetQueryParam("http://localhost/foo?param1=a&param2=b#qqq",
                    "param3",
                    "true",
                    "http://localhost/foo?param1=a&param2=b&param3=true#qqq",
                    "param1=a&param2=b&param3=true");
  // Add a query parameter, should be alphabetical order
  testSetQueryParam("localhost:80/foo?param1=a&param3=c#qqq",
                    "param2",
                    "b",
                    "localhost:80/foo?param1=a&param2=b&param3=c#qqq",
                    "param1=a&param2=b&param3=c");
  // Add a query parameter when no query parameters exist
  testSetQueryParam("localhost:80/foo#qqq",
                    "param2",
                    "b",
                    "localhost:80/foo?param2=b#qqq",
                    "param2=b");
}

TEST(HTTPMessage, TestCheckForHeaderToken) {
  HTTPMessage msg;
  HTTPHeaders& headers = msg.getHeaders();

  headers.add(HTTP_HEADER_CONNECTION, "HTTP2-Settings");
  EXPECT_TRUE(
      msg.checkForHeaderToken(HTTP_HEADER_CONNECTION, "HTTP2-Settings", false));
  EXPECT_FALSE(
      msg.checkForHeaderToken(HTTP_HEADER_CONNECTION, "http2-settings", true));
}

TEST(HTTPMessage, TestProtocolStringHTTPVersion) {
  HTTPMessage msg;
  msg.setHTTPVersion(1, 1);

  EXPECT_EQ(msg.getProtocolString(), "1.1");
}

TEST(HTTPMessage, TestProtocolStringAdvancedProtocol) {
  HTTPMessage msg;
  std::string advancedProtocol = "h2";
  msg.setAdvancedProtocolString(advancedProtocol);
  EXPECT_EQ(msg.getProtocolString(), advancedProtocol);
}

TEST(HTTPMessage, TestExtractTrailers) {
  HTTPMessage msg;
  auto trailers = std::make_unique<HTTPHeaders>();
  HTTPHeaders* rawPointer = trailers.get();
  trailers->add("The-trailer", "something");
  msg.setTrailers(std::move(trailers));
  auto trailers2 = msg.extractTrailers();
  EXPECT_EQ(rawPointer, trailers2.get());
  EXPECT_EQ(nullptr, msg.getTrailers());
}

TEST(HTTPMessage, TestMoveCopy) {
  HTTPMessage m1;
  m1.setURL(std::string(32, 'a'));
  HTTPMessage m2;
  m2.setURL(std::string(32, 'b'));
  m2 = m1;
  m2 = std::move(m1);
}

namespace {
const size_t kInitialVectorReserve = 16;
}

TEST(HTTPHeaders, GrowTest) {
  HTTPHeaders headers;
  for (size_t i = 0; i < kInitialVectorReserve * 2; i++) {
    headers.add(folly::to<std::string>(i), std::string(50, 'a' + i));
  }
  EXPECT_EQ(headers.getSingleOrEmpty("0")[0], 'a');
  EXPECT_EQ(headers.getSingleOrEmpty("25")[0], 'z');
}

TEST(HTTPHeaders, ClearTest) {
  HTTPHeaders headers;
  for (size_t i = 0; i < kInitialVectorReserve * 2; i++) {
    headers.add(folly::to<std::string>(i), std::string(50, 'a' + i));
  }
  EXPECT_EQ(headers.getSingleOrEmpty("25")[0], 'z');
  headers.removeAll();
  EXPECT_EQ(headers.size(), 0);
  for (size_t i = 0; i < kInitialVectorReserve * 2; i++) {
    headers.add(folly::to<std::string>(i), std::string(50, 'A' + i));
  }
  EXPECT_EQ(headers.getSingleOrEmpty("25")[0], 'Z');
}

void copyAndMoveTest(size_t multiplier) {
  HTTPHeaders headers;
  for (size_t i = 0; i < kInitialVectorReserve * multiplier; i++) {
    headers.add(folly::to<std::string>(i), std::string(50, 'a' + i));
  }
  HTTPHeaders headers2(headers);
  EXPECT_EQ(headers2.getSingleOrEmpty("3")[0], 'd');
  HTTPHeaders headers3;
  headers3.add("blown", "away");
  headers3 = headers2;
  EXPECT_EQ(headers2.getSingleOrEmpty("4")[0], 'e');
  HTTPHeaders headers4(std::move(headers3));
  EXPECT_EQ(headers4.getSingleOrEmpty("5")[0], 'f');
  HTTPHeaders headers5;
  headers5.add("blown", "away");
  headers5 = std::move(headers4);
  EXPECT_EQ(headers5.getSingleOrEmpty("6")[0], 'g');
}

TEST(HTTPHeaders, CopyAndMoveTest) {
  copyAndMoveTest(1);
  copyAndMoveTest(2);
  copyAndMoveTest(3);
}

void checkPrettyPrint(const HTTPHeaders& h) {
  EXPECT_EQ(h.size(), 2);
}

TEST(HTTPHeaders, PrettyPrint) {
  HTTPHeaders headers;
  headers.add(HTTP_HEADER_HOST, "www.facebook.com");
  headers.add(HTTP_HEADER_CONNECTION, "close");
  headers.add("Foo", "Bar");
  headers.remove(HTTP_HEADER_CONNECTION);
  checkPrettyPrint(headers);
}

void checkPrettyPrintMsg(const HTTPMessage& m) {
  EXPECT_EQ(m.getMethodString(), "SPECIAL");
}

TEST(HTTPMessage, PrettyPrint) {
  HTTPMessage request;
  request.setURL("/foo/bar.php?n=v");
  request.setHTTPVersion(1, 1);
  request.setMethod("SPECIAL");
  request.setIsChunked(true);
  folly::SocketAddress addr;
  addr.setFromHostPort("localhost", 9999);
  request.setClientAddress(addr);
  request.getClientIP();
  request.getClientPort();
  request.getHeaders().add(HTTP_HEADER_HOST, "www.intstagram.com");
  request.getHeaders().add("Bar", "Foo");
  checkPrettyPrintMsg(request);

  HTTPMessage response;
  response.setStatusCode(418);
  response.setStatusMessage("I am a teapot");
  response.setHTTPVersion(3, 0);
  response.getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "0");
  response.getHeaders().add(HTTP_HEADER_CONTENT_TYPE, "text/plain");
  checkPrettyPrint(response.getHeaders());
}

TEST(HTTPMessage, NoDefaultHTTPPriority) {
  HTTPMessage message;
  EXPECT_FALSE(httpPriorityFromHTTPMessage(message).hasValue());
}

TEST(HTTPMessage, HTTPPrioritySetGet) {
  HTTPMessage message;
  message.setHTTPPriority(HTTPPriority(0, true));
  EXPECT_EQ(0, httpPriorityFromHTTPMessage(message)->urgency);
  EXPECT_TRUE(httpPriorityFromHTTPMessage(message)->incremental);
  auto& priHeader = message.getHeaders().getSingleOrEmpty(HTTP_HEADER_PRIORITY);
  EXPECT_EQ("u=0,i", priHeader);
  auto priHeaderViaGetter = message.getHTTPPriority();
  EXPECT_EQ(0, priHeaderViaGetter->urgency);
  EXPECT_TRUE(priHeaderViaGetter->incremental);

  message.setHTTPPriority(1, false);
  EXPECT_EQ(1, httpPriorityFromHTTPMessage(message)->urgency);
  EXPECT_FALSE(httpPriorityFromHTTPMessage(message)->incremental);
  EXPECT_EQ("u=1", message.getHeaders().getSingleOrEmpty(HTTP_HEADER_PRIORITY));
  priHeaderViaGetter = message.getHTTPPriority();
  EXPECT_EQ(1, priHeaderViaGetter->urgency);
  EXPECT_FALSE(priHeaderViaGetter->incremental);
}

TEST(HTTPMessage, HTTPPrioritySetOutRangeUrgency) {
  HTTPMessage message;
  message.setHTTPPriority(HTTPPriority(kMaxPriority + 10, true));
  EXPECT_EQ(kMaxPriority, httpPriorityFromHTTPMessage(message)->urgency);
  EXPECT_TRUE(httpPriorityFromHTTPMessage(message)->incremental);
  auto& priHeader = message.getHeaders().getSingleOrEmpty(HTTP_HEADER_PRIORITY);
  EXPECT_EQ("u=7,i", priHeader);
  auto priHeaderViaGetter = message.getHTTPPriority();
  EXPECT_EQ(7, priHeaderViaGetter->urgency);
  EXPECT_TRUE(priHeaderViaGetter->incremental);
}

TEST(HTTPHeaders, GetSetOnResize) {
  HTTPHeaders headers;
  for (size_t i = 0; i < kInitialVectorReserve - 1; i++) {
    headers.add(HTTP_HEADER_CONNECTION, "token");
  }
  std::string value(32, 'a');
  headers.add(HTTP_HEADER_SERVER, value);
  EXPECT_EQ(headers.size(), kInitialVectorReserve);
  auto& v = headers.getSingleOrEmpty(HTTP_HEADER_SERVER);
  headers.set(HTTP_HEADER_SERVER, v);

  EXPECT_EQ(headers.getSingleOrEmpty(HTTP_HEADER_SERVER), value);
}

TEST(HTTPHeaders, MoveFromTest) {
  HTTPHeaders h1;
  HTTPHeaders h2(std::move(h1));
  EXPECT_FALSE(h1.exists(HTTP_HEADER_CONNECTION));
  h1.forEachValueOfHeader(HTTP_HEADER_HOST, [](const std::string&) {
    CHECK(false) << "Unreachable";
    return false;
  });
  h1.add(HTTP_HEADER_CONNECTION, "close");

  HTTPHeaders h3;
  h3 = std::move(h1); // move assignment
  EXPECT_EQ(h1.size(), 0);
  EXPECT_EQ(h3.size(), 1);
}

TEST(HTTPMessage, DefaultSchemeHttp) {
  HTTPMessage message;
  EXPECT_EQ(message.getScheme(), "http");
  EXPECT_FALSE(message.isSecure());
}

TEST(HTTPMessage, SchemeHttps) {
  HTTPMessage message;
  message.setSecure(true);
  EXPECT_EQ(message.getScheme(), "https");
  EXPECT_TRUE(message.isSecure());
  message.setSecure(false);
  EXPECT_EQ(message.getScheme(), "http");
  EXPECT_FALSE(message.isSecure());
}

TEST(HTTPMessage, SchemeMasque) {
  HTTPMessage message;
  message.setMasque();
  EXPECT_EQ(message.getScheme(), "masque");
  EXPECT_TRUE(message.isSecure());
  // Masque is already secure, setting secure again has no effect
  message.setSecure(true);
  EXPECT_EQ(message.getScheme(), "masque");
  EXPECT_TRUE(message.isSecure());
  // Masque must be secure, so unsetting secure falls back to http://
  message.setSecure(false);
  EXPECT_EQ(message.getScheme(), "http");
  EXPECT_FALSE(message.isSecure());
}

TEST(HTTPMessage, StrictMode) {
  HTTPMessage message;
  EXPECT_FALSE(message.setURL("/foo\xff", /*strict=*/true));
  EXPECT_EQ(message.getURL(), "/foo\xff");
  EXPECT_EQ(message.getPath(), "");

  message.setURL("/");
  // Not strict mode, high ascii OK
  EXPECT_TRUE(message.setQueryString("a=b\xff"));
  EXPECT_EQ(message.getURL(), "/?a=b\xff");
  EXPECT_EQ(message.getPath(), "/");
  EXPECT_EQ(message.getQueryString(), "a=b\xff");

  EXPECT_TRUE(message.setQueryString("a=b"));
  EXPECT_EQ(message.getURL(), "/?a=b");
  EXPECT_FALSE(message.setQueryString("a=b\xff", /*strict=*/true));
  EXPECT_EQ(message.getURL(), "/?a=b\xff");
  EXPECT_EQ(message.getQueryString(), "");

  EXPECT_TRUE(message.setQueryString("a=b"));
  EXPECT_FALSE(message.setQueryParam("c", "d\xff", /*strict=*/true));
  EXPECT_EQ(message.getURL(), "/?a=b&c=d\xff");
  EXPECT_EQ(message.getQueryString(), "");
  EXPECT_TRUE(message.setQueryParam("c", "d\xff", /*strict=*/false));
  EXPECT_EQ(message.getURL(), "/?a=b&c=d\xff");

  // Can remove a param without failing the strict parsing
  EXPECT_TRUE(message.setURL("/?a=b&c=d\xff"));
  EXPECT_TRUE(message.removeQueryParam("a"));
  EXPECT_TRUE(message.removeQueryParam("c"));
}

#ifdef NDEBUG
// This fails DCHECKs in debug mode, throws in opt
TEST(HTTPMessage, BadAPIUsage) {
  HTTPMessage req;
  req.setURL("/");
  EXPECT_THROW(req.getStatusCode(), std::runtime_error);
  EXPECT_EQ(req.getURL(), "/");

  HTTPMessage resp;
  resp.setStatusCode(200);
  EXPECT_THROW(resp.getQueryString(), std::runtime_error);
  EXPECT_EQ(resp.getStatusCode(), 200);
}
#endif
