/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HTTP2Codec.h>

#include <folly/io/Cursor.h>
#include <proxygen/lib/http/HTTPHeaderSize.h>
#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/codec/test/HTTP2FramerTest.h>
#include <proxygen/lib/http/codec/test/HTTPParallelCodecTest.h>
#include <proxygen/lib/http/codec/test/MockHTTPCodec.h>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <random>

using namespace proxygen;
using namespace proxygen::compress;
using namespace folly;
using namespace folly::io;
using namespace std;
using namespace testing;

TEST(HTTP2CodecConstantsTest, HTTPContantsAreCommonHeaders) {
  // The purpose of this test is to verify some basic assumptions that should
  // never change but to make clear that the following http2 header constants
  // map to the respective common headers.  Should this test ever fail, the
  // H2Codec would need to be updated in the corresponding places when creating
  // compress/Header objects.
  EXPECT_EQ(HTTPCommonHeaders::hash(headers::kMethod),
            HTTP_HEADER_COLON_METHOD);
  EXPECT_EQ(HTTPCommonHeaders::hash(headers::kScheme),
            HTTP_HEADER_COLON_SCHEME);
  EXPECT_EQ(HTTPCommonHeaders::hash(headers::kPath), HTTP_HEADER_COLON_PATH);
  EXPECT_EQ(HTTPCommonHeaders::hash(headers::kAuthority),
            HTTP_HEADER_COLON_AUTHORITY);
  EXPECT_EQ(HTTPCommonHeaders::hash(headers::kStatus),
            HTTP_HEADER_COLON_STATUS);
}

class HTTP2CodecTest : public HTTPParallelCodecTest {
 public:
  HTTP2CodecTest() : HTTPParallelCodecTest(upstreamCodec_, downstreamCodec_) {
    upstreamCodec_.enableDoubleGoawayDrain();
    downstreamCodec_.enableDoubleGoawayDrain();
  }

  void SetUp() override {
    HTTPParallelCodecTest::SetUp();
    // make it transparent to the tests that we've received a settings frame
    upstreamCodec_.generateSettings(output_);
    parse();
    callbacks_.reset();
  }
  void testHeaderListSize(bool oversized);
  void testFrameSizeLimit(bool oversized);

  void writeHeaders(folly::IOBufQueue& writeBuf,
                    std::unique_ptr<folly::IOBuf> headers,
                    uint32_t stream,
                    folly::Optional<http2::PriorityUpdate> priority,
                    folly::Optional<uint8_t> padding,
                    bool endStream,
                    bool endHeaders) {
    auto headersLen = headers ? headers->computeChainDataLength() : 0;
    auto headerSize = http2::calculatePreHeaderBlockSize(
        false, false, priority.has_value(), padding.has_value());
    auto header = writeBuf.preallocate(headerSize, 32);
    writeBuf.postallocate(headerSize);
    writeBuf.append(std::move(headers));
    http2::writeHeaders((uint8_t*)header.first,
                        header.second,
                        writeBuf,
                        headersLen,
                        stream,
                        priority,
                        padding,
                        endStream,
                        endHeaders);
  }

 protected:
  HTTP2Codec upstreamCodec_{TransportDirection::UPSTREAM};
  HTTP2Codec downstreamCodec_{TransportDirection::DOWNSTREAM};
};

TEST_F(HTTP2CodecTest, IgnoreUnknownSettings) {
  auto numSettings = downstreamCodec_.getIngressSettings()->getNumSettings();
  std::deque<SettingPair> settings;
  for (uint32_t i = 200; i < (200 + 1024); i++) {
    settings.push_back(SettingPair(SettingsId(i), i));
  }
  http2::writeSettings(output_, settings);
  parse();

  EXPECT_EQ(callbacks_.settings, 1);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  EXPECT_EQ(numSettings,
            downstreamCodec_.getIngressSettings()->getNumSettings());
}

// Some tests rely on the fact that we haven't flushed and parsed the preface
// and settings frame.
class HTTP2CodecTestOmitParsePreface : public HTTP2CodecTest {
  void SetUp() override {
    HTTPParallelCodecTest::SetUp();
  }
};

TEST_F(HTTP2CodecTest, NoExHeaders) {
  // do not emit ENABLE_EX_HEADERS setting, if disabled
  SetUpUpstreamTest();

  EXPECT_EQ(callbacks_.settings, 0);
  EXPECT_EQ(callbacks_.numSettings, 0);
  EXPECT_EQ(false, downstreamCodec_.supportsExTransactions());

  parseUpstream();

  EXPECT_EQ(callbacks_.settings, 1);
  // only 3 standard settings: HEADER_TABLE_SIZE, ENABLE_PUSH, MAX_FRAME_SIZE.
  EXPECT_EQ(callbacks_.numSettings, 3);
  EXPECT_EQ(false, downstreamCodec_.supportsExTransactions());
}

TEST_F(HTTP2CodecTest, IgnoreExHeadersSetting) {
  // disable EX_HEADERS on egress
  downstreamCodec_.getEgressSettings()->setSetting(
      SettingsId::ENABLE_EX_HEADERS, 0);
  auto ptr = downstreamCodec_.getEgressSettings()->getSetting(
      SettingsId::ENABLE_EX_HEADERS);
  EXPECT_EQ(0, ptr->value);

  ptr = downstreamCodec_.getIngressSettings()->getSetting(
      SettingsId::ENABLE_EX_HEADERS);
  EXPECT_EQ(nullptr, ptr);
  EXPECT_EQ(false, downstreamCodec_.supportsExTransactions());

  // attempt to enable EX_HEADERS on ingress
  http2::writeSettings(output_,
                       {SettingPair(SettingsId::ENABLE_EX_HEADERS, 1)});
  parse();

  EXPECT_EQ(callbacks_.settings, 1);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  ptr = downstreamCodec_.getIngressSettings()->getSetting(
      SettingsId::ENABLE_EX_HEADERS);
  EXPECT_EQ(nullptr, ptr);
  EXPECT_EQ(false, downstreamCodec_.supportsExTransactions());

  // attempt to disable EX_HEADERS on ingress
  callbacks_.reset();
  http2::writeSettings(output_,
                       {SettingPair(SettingsId::ENABLE_EX_HEADERS, 0)});
  parse();

  EXPECT_EQ(callbacks_.settings, 1);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  ptr = downstreamCodec_.getIngressSettings()->getSetting(
      SettingsId::ENABLE_EX_HEADERS);
  EXPECT_EQ(nullptr, ptr);
  EXPECT_EQ(false, downstreamCodec_.supportsExTransactions());
}

TEST_F(HTTP2CodecTest, EnableExHeadersSetting) {
  // enable EX_HEADERS on egress
  downstreamCodec_.getEgressSettings()->setSetting(
      SettingsId::ENABLE_EX_HEADERS, 1);

  auto ptr = downstreamCodec_.getEgressSettings()->getSetting(
      SettingsId::ENABLE_EX_HEADERS);
  EXPECT_EQ(1, ptr->value);

  ptr = downstreamCodec_.getIngressSettings()->getSetting(
      SettingsId::ENABLE_EX_HEADERS);
  EXPECT_EQ(nullptr, ptr);
  EXPECT_EQ(false, downstreamCodec_.supportsExTransactions());

  // attempt to enable EX_HEADERS on ingress
  http2::writeSettings(output_,
                       {SettingPair(SettingsId::ENABLE_EX_HEADERS, 1)});
  parse();

  EXPECT_EQ(callbacks_.settings, 1);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  ptr = downstreamCodec_.getIngressSettings()->getSetting(
      SettingsId::ENABLE_EX_HEADERS);
  EXPECT_EQ(1, ptr->value);
  EXPECT_EQ(true, downstreamCodec_.supportsExTransactions());

  // attempt to disable EX_HEADERS on ingress
  callbacks_.reset();
  http2::writeSettings(output_,
                       {SettingPair(SettingsId::ENABLE_EX_HEADERS, 0)});
  parse();

  EXPECT_EQ(callbacks_.settings, 1);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  ptr = downstreamCodec_.getIngressSettings()->getSetting(
      SettingsId::ENABLE_EX_HEADERS);
  EXPECT_EQ(0, ptr->value);
  EXPECT_EQ(false, downstreamCodec_.supportsExTransactions());
}

TEST_F(HTTP2CodecTest, InvalidExHeadersSetting) {
  // enable EX_HEADERS on egress
  downstreamCodec_.getEgressSettings()->setSetting(
      SettingsId::ENABLE_EX_HEADERS, 1);

  // attempt to set a invalid ENABLE_EX_HEADERS value
  http2::writeSettings(output_,
                       {SettingPair(SettingsId::ENABLE_EX_HEADERS, 110)});
  parse();

  EXPECT_EQ(callbacks_.sessionErrors, 1);
}

TEST_F(HTTP2CodecTest, BasicHeader) {
  HTTPMessage req = getGetRequest("/guacamole");
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");
  req.getHeaders().add("tab-hdr", "coolio\tv2");
  // Connection header will get dropped
  req.getHeaders().add(HTTP_HEADER_CONNECTION, "Love");
  req.setSecure(true);
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, true /* eom */);

  parse();
  callbacks_.expectMessage(true, 3, "/guacamole");
  EXPECT_TRUE(callbacks_.msg->isSecure());
  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_EQ("coolio", headers.getSingleOrEmpty(HTTP_HEADER_USER_AGENT));
  EXPECT_EQ("coolio\tv2", headers.getSingleOrEmpty("tab-hdr"));
  EXPECT_EQ("www.foo.com", headers.getSingleOrEmpty(HTTP_HEADER_HOST));
}

TEST_F(HTTP2CodecTest, GenerateExtraHeaders) {
  HTTPMessage req = getGetRequest("/fish_taco");
  req.getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "157");
  HTTPHeaders extraHeaders;
  extraHeaders.add(HTTP_HEADER_PRIORITY, "u=0");
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_,
                                id,
                                req,
                                true,
                                nullptr /* headerSize */,
                                std::move(extraHeaders));

  parse();
  // There is also a HOST header
  callbacks_.expectMessage(true, 3, "/fish_taco");
  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_EQ("157", headers.getSingleOrEmpty(HTTP_HEADER_CONTENT_LENGTH));
  EXPECT_EQ("u=0", headers.getSingleOrEmpty(HTTP_HEADER_PRIORITY));
}

TEST_F(HTTP2CodecTest, RequestFromServer) {
  // this is to test EX_HEADERS frame, which carrys the HTTP request initiated
  // by server side
  upstreamCodec_.getEgressSettings()->setSetting(SettingsId::ENABLE_EX_HEADERS,
                                                 1);
  SetUpUpstreamTest();
  proxygen::http2::writeSettings(
      output_, {{proxygen::SettingsId::ENABLE_EX_HEADERS, 1}});

  HTTPMessage req = getGetRequest("/guacamole");
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");
  req.getHeaders().add("tab-hdr", "coolio\tv2");
  // Connection header will get dropped
  req.getHeaders().add(HTTP_HEADER_CONNECTION, "Love");
  req.setSecure(true);

  HTTPCodec::StreamID stream = folly::Random::rand32(10, 1024) * 2;
  HTTPCodec::StreamID controlStream = folly::Random::rand32(10, 1024) * 2 + 1;
  upstreamCodec_.generateExHeader(
      output_, stream, req, HTTPCodec::ExAttributes(controlStream, true), true);

  parseUpstream();
  EXPECT_EQ(controlStream, callbacks_.controlStreamId);
  EXPECT_TRUE(callbacks_.isUnidirectional);
  callbacks_.expectMessage(true, 3, "/guacamole");
  EXPECT_TRUE(callbacks_.msg->isSecure());
  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_EQ("coolio", headers.getSingleOrEmpty(HTTP_HEADER_USER_AGENT));
  EXPECT_EQ("coolio\tv2", headers.getSingleOrEmpty("tab-hdr"));
  EXPECT_EQ("www.foo.com", headers.getSingleOrEmpty(HTTP_HEADER_HOST));
}

TEST_F(HTTP2CodecTestOmitParsePreface, OmitSettingsAfterConnPrefaceError) {
  HTTPMessage req = getGetRequest("/test");
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "rand-user");
  req.setSecure(true);
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, true, /*headerSize=*/nullptr);

  parse();
  EXPECT_EQ(callbacks_.settings, 0);
  EXPECT_EQ(callbacks_.numSettings, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
  EXPECT_EQ(callbacks_.lastParseError->getCodecStatusCode(),
            ErrorCode::PROTOCOL_ERROR);
}

TEST_F(HTTP2CodecTest, ResponseFromClient) {
  // this is to test EX_HEADERS frame, which carrys the HTTP response replied by
  // client side
  downstreamCodec_.getEgressSettings()->setSetting(
      SettingsId::ENABLE_EX_HEADERS, 1);
  proxygen::http2::writeSettings(
      output_, {{proxygen::SettingsId::ENABLE_EX_HEADERS, 1}});

  HTTPMessage resp;
  resp.setStatusCode(200);
  resp.setStatusMessage("nifty-nice");
  resp.getHeaders().add(HTTP_HEADER_CONTENT_TYPE, "x-coolio");

  HTTPCodec::StreamID stream = folly::Random::rand32(10, 1024) * 2;
  HTTPCodec::StreamID controlStream = folly::Random::rand32(10, 1024) * 2 + 1;
  downstreamCodec_.generateExHeader(
      output_,
      stream,
      resp,
      HTTPCodec::ExAttributes(controlStream, true),
      true);

  parse();
  EXPECT_EQ(controlStream, callbacks_.controlStreamId);
  EXPECT_TRUE(callbacks_.isUnidirectional);
  EXPECT_EQ("OK", callbacks_.msg->getStatusMessage());
  callbacks_.expectMessage(true, 2, 200);
  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_EQ("OK", callbacks_.msg->getStatusMessage());
  EXPECT_TRUE(callbacks_.msg->getHeaders().exists(HTTP_HEADER_DATE));
  EXPECT_EQ("x-coolio", headers.getSingleOrEmpty(HTTP_HEADER_CONTENT_TYPE));
}

TEST_F(HTTP2CodecTest, ExHeadersWithPriority) {
  downstreamCodec_.getEgressSettings()->setSetting(
      SettingsId::ENABLE_EX_HEADERS, 1);
  proxygen::http2::writeSettings(
      output_, {{proxygen::SettingsId::ENABLE_EX_HEADERS, 1}});

  auto req = getGetRequest();
  // Test empty path
  req.setURL("");
  auto pri = HTTPMessage::HTTP2Priority(0, false, 7);
  req.setHTTP2Priority(pri);
  upstreamCodec_.generateExHeader(
      output_, 3, req, HTTPCodec::ExAttributes(1, true));

  parse();
  EXPECT_EQ(callbacks_.msg->getHTTP2Priority(), pri);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, DuplicateExHeaders) {
  downstreamCodec_.getEgressSettings()->setSetting(
      SettingsId::ENABLE_EX_HEADERS, 1);
  proxygen::http2::writeSettings(
      output_, {{proxygen::SettingsId::ENABLE_EX_HEADERS, 1}});

  auto req = getGetRequest();
  upstreamCodec_.generateExHeader(
      output_, 3, req, HTTPCodec::ExAttributes(1, true), /*eom=*/false);
  upstreamCodec_.generateExHeader(
      output_, 3, req, HTTPCodec::ExAttributes(1, true), /*eom=*/true);

  parse();
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
}

TEST_F(HTTP2CodecTest, IgnoreExHeadersIfNotEnabled) {
  downstreamCodec_.getEgressSettings()->setSetting(
      SettingsId::ENABLE_EX_HEADERS, 0);

  HTTPMessage req = getGetRequest("/guacamole");
  downstreamCodec_.generateExHeader(
      output_, 3, req, HTTPCodec::ExAttributes(1, true));

  parse();
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BadHeaders) {
  static const std::string v1("GET");
  static const std::string v2("/");
  static const std::string v3("http");
  static const std::string v4("foo.com");
  static const vector<proxygen::compress::Header> reqHeaders = {
      Header::makeHeaderForTest(headers::kMethod, v1),
      Header::makeHeaderForTest(headers::kPath, v2),
      Header::makeHeaderForTest(headers::kScheme, v3),
      Header::makeHeaderForTest(headers::kAuthority, v4),
  };

  HPACKCodec headerCodec(TransportDirection::UPSTREAM);
  HTTPCodec::StreamID stream = 1;
  // missing fields (missing authority is OK)
  for (size_t i = 0; i < reqHeaders.size(); i++, stream += 2) {
    std::vector<proxygen::compress::Header> allHeaders = reqHeaders;
    allHeaders.erase(allHeaders.begin() + i);
    auto encodedHeaders = headerCodec.encode(allHeaders);
    writeHeaders(output_,
                 std::move(encodedHeaders),
                 stream,
                 folly::none,
                 http2::kNoPadding,
                 true,
                 true);
  }
  // dup fields
  std::string v("foomonkey");
  for (size_t i = 0; i < reqHeaders.size(); i++, stream += 2) {
    std::vector<proxygen::compress::Header> allHeaders = reqHeaders;
    auto h = allHeaders[i];
    h.value = &v;
    allHeaders.push_back(h);
    auto encodedHeaders = headerCodec.encode(allHeaders);
    writeHeaders(output_,
                 std::move(encodedHeaders),
                 stream,
                 folly::none,
                 http2::kNoPadding,
                 true,
                 true);
  }

  parse();
  EXPECT_EQ(callbacks_.messageBegin, 1 + 7);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.messageComplete, 1);
  EXPECT_EQ(callbacks_.streamErrors, 7);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BadPseudoHeaders) {
  static const std::string v1("POST");
  static const std::string v2("http");
  static const std::string n3("foo");
  static const std::string v3("bar");
  static const std::string v4("/");
  static const vector<proxygen::compress::Header> reqHeaders = {
      Header::makeHeaderForTest(headers::kMethod, v1),
      Header::makeHeaderForTest(headers::kScheme, v2),
      Header::makeHeaderForTest(n3, v3),
      Header::makeHeaderForTest(headers::kPath, v4),
  };

  HPACKCodec headerCodec(TransportDirection::UPSTREAM);
  HTTPCodec::StreamID stream = 1;
  std::vector<proxygen::compress::Header> allHeaders = reqHeaders;
  auto encodedHeaders = headerCodec.encode(allHeaders);
  writeHeaders(output_,
               std::move(encodedHeaders),
               stream,
               folly::none,
               http2::kNoPadding,
               true,
               true);

  parse();
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.lastParseError->getProxygenError(), kErrorParseHeader);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BadHeaderValues) {
  static const std::string v1("--1");
  static const std::string v2("\13\10protocol-attack");
  static const std::string v3("\13");
  static const std::string v4("abc.com\\13\\10");
  static const vector<proxygen::compress::Header> reqHeaders = {
      Header::makeHeaderForTest(headers::kMethod, v1),
      Header::makeHeaderForTest(headers::kPath, v2),
      Header::makeHeaderForTest(headers::kScheme, v3),
      Header::makeHeaderForTest(headers::kAuthority, v4),
  };

  HPACKCodec headerCodec(TransportDirection::UPSTREAM);
  HTTPCodec::StreamID stream = 1;
  for (size_t i = 0; i < reqHeaders.size(); i++, stream += 2) {
    std::vector<proxygen::compress::Header> allHeaders;
    allHeaders.push_back(reqHeaders[i]);
    auto encodedHeaders = headerCodec.encode(allHeaders);
    writeHeaders(output_,
                 std::move(encodedHeaders),
                 stream,
                 folly::none,
                 http2::kNoPadding,
                 true,
                 true);
  }

  parse();
  EXPECT_EQ(callbacks_.messageBegin, 4);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 4);
  EXPECT_EQ(callbacks_.lastParseError->getProxygenError(), kErrorParseHeader);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, HostAuthority) {
  static const std::string v1("GET");
  static const std::string v2("/");
  static const std::string v3("http");
  static const std::string v4("foo.com");

  static const vector<proxygen::compress::Header> reqHeaders = {
      Header::makeHeaderForTest(headers::kMethod, v1),
      Header::makeHeaderForTest(headers::kPath, v2),
      Header::makeHeaderForTest(headers::kScheme, v3),
      Header::makeHeaderForTest(headers::kAuthority, v4),
  };

  HTTPCodec::StreamID stream = 1;
  for (auto i = 0; i < 2; i++, stream += 2) {
    auto allHeaders = reqHeaders;
    std::string v5(i == 0 ? v4 : "nope");
    allHeaders.emplace_back(HTTP_HEADER_HOST, v5);
    HPACKCodec headerCodec(TransportDirection::UPSTREAM);
    auto encodedHeaders = headerCodec.encode(allHeaders);
    writeHeaders(output_,
                 std::move(encodedHeaders),
                 stream,
                 folly::none,
                 http2::kNoPadding,
                 true,
                 true);
    parse();
    if (i == 0) {
      // same value, ok
      callbacks_.expectMessage(true, stream, "/");
      const auto& headers = callbacks_.msg->getHeaders();
      EXPECT_EQ(v4, headers.getSingleOrEmpty(HTTP_HEADER_HOST));
    } else {
      // different values, error
      EXPECT_EQ(callbacks_.streamErrors, 1);
      EXPECT_EQ(callbacks_.lastParseError->getHttpStatusCode(), 400);
    }
  }
}

TEST_F(HTTP2CodecTest, HighAscii) {
  auto g =
      folly::makeGuard([this] { downstreamCodec_.setStrictValidation(false); });
  downstreamCodec_.setStrictValidation(true);
  HTTPMessage req1 = getGetRequest("/guacamole\xff");
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(
      output_, id, req1, true, nullptr /* headerSize */);
  HTTPMessage req2 = getGetRequest("/guacamole");
  req2.getHeaders().set(HTTP_HEADER_HOST, std::string("foo.com\xff"));
  auto id2 = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(
      output_, id2, req2, true, nullptr /* headerSize */);
  HTTPMessage req3 = getGetRequest("/guacamole");
  req3.getHeaders().set(folly::StringPiece("Foo\xff"), "bar");
  auto id3 = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(
      output_, id3, req3, true, nullptr /* headerSize */);
  HTTPMessage req4 = getGetRequest("/guacamole");
  req4.getHeaders().set("Foo", std::string("bar\xff"));
  auto id4 = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(
      output_, id4, req4, true, nullptr /* headerSize */);

  parse();
  EXPECT_EQ(callbacks_.messageBegin, 4);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 4);
  EXPECT_EQ(callbacks_.lastParseError->getProxygenError(), kErrorParseHeader);
  EXPECT_EQ(callbacks_.sessionErrors, 0);

  HTTPMessage req5 = getGetRequest("/guacamole");
  req5.getHeaders().set(HTTP_HEADER_USER_AGENT, "ꪶ𝛸ꫂ_𝐹𝛩𝑅𝐶𝛯_𝑉2");
  auto id5 = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(
      output_, id5, req5, true, nullptr /* headerSize */);
  callbacks_.reset();
  parse();
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.lastParseError->getProxygenError(), kErrorParseHeader);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, EmptyPath) {
  auto g =
      folly::makeGuard([this] { downstreamCodec_.setStrictValidation(false); });
  downstreamCodec_.setStrictValidation(true);
  HTTPMessage req1 = getGetRequest("");
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(
      output_, id, req1, true, nullptr /* headerSize */);
  parse();
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.lastParseError->getProxygenError(), kErrorParseHeader);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

/**
 * Ingress bytes with an empty header name
 */
const uint8_t kBufEmptyHeader[] = {
    0x00, 0x00, 0x1d, 0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x82,
    0x87, 0x44, 0x87, 0x62, 0x6b, 0x46, 0x41, 0xd2, 0x7a, 0x0b,
    0x41, 0x89, 0xf1, 0xe3, 0xc2, 0xf2, 0x9c, 0xeb, 0x90, 0xf4,
    0xff, 0x40, 0x80, 0x84, 0x2d, 0x35, 0xa7, 0xd7};

TEST_F(HTTP2CodecTest, EmptyHeaderName) {
  output_.append(IOBuf::copyBuffer(kBufEmptyHeader, sizeof(kBufEmptyHeader)));
  parse();
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.lastParseError->getProxygenError(), kErrorParseHeader);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BasicConnect) {
  std::string authority = "myhost:1234";
  HTTPMessage request;
  request.setMethod(HTTPMethod::CONNECT);
  request.getHeaders().add(proxygen::HTTP_HEADER_HOST, authority);
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, request, false /* eom */);

  parse();
  callbacks_.expectMessage(false, 1, "");
  EXPECT_EQ(HTTPMethod::CONNECT, callbacks_.msg->getMethod());
  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_EQ(authority, headers.getSingleOrEmpty(proxygen::HTTP_HEADER_HOST));
}

TEST_F(HTTP2CodecTest, BadConnect) {
  std::string v1 = "CONNECT";
  std::string v2 = "somehost:576";
  std::vector<proxygen::compress::Header> goodHeaders = {
      Header::makeHeaderForTest(headers::kMethod, v1),
      Header::makeHeaderForTest(headers::kAuthority, v2),
  };

  // See https://tools.ietf.org/html/rfc7540#section-8.3
  std::string v3 = "/foobar";
  std::vector<proxygen::compress::Header> badHeaders = {
      Header::makeHeaderForTest(headers::kScheme, headers::kHttp),
      Header::makeHeaderForTest(headers::kPath, v3),
  };

  HPACKCodec headerCodec(TransportDirection::UPSTREAM);
  HTTPCodec::StreamID stream = 1;

  for (size_t i = 0; i < badHeaders.size(); i++, stream += 2) {
    auto allHeaders = goodHeaders;
    allHeaders.push_back(badHeaders[i]);
    auto encodedHeaders = headerCodec.encode(allHeaders);
    writeHeaders(output_,
                 std::move(encodedHeaders),
                 stream,
                 folly::none,
                 http2::kNoPadding,
                 true,
                 true);
  }

  parse();
  EXPECT_EQ(callbacks_.messageBegin, badHeaders.size());
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, badHeaders.size());
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, TemplateDrivenConnect) {
  // See https://fburl.com/nql0na8x for definition
  std::string method = "CONNECT";
  std::string authority = "request-proxy.example";
  std::string path = "/proxy?target_host=192.0.2.1,2001:db8::1&tcp_port=443";
  std::string protocol = "connect-tcp";

  std::vector<proxygen::compress::Header> headers = {
      Header::makeHeaderForTest(headers::kMethod, method),
      Header::makeHeaderForTest(headers::kAuthority, authority),
      Header::makeHeaderForTest(headers::kScheme, headers::kHttps),
      Header::makeHeaderForTest(headers::kPath, path),
      Header::makeHeaderForTest(headers::kProtocol, protocol),
  };

  HPACKCodec headerCodec(TransportDirection::UPSTREAM);
  HTTPCodec::StreamID stream = 1;

  auto encodedHeaders = headerCodec.encode(headers);
  writeHeaders(output_,
               std::move(encodedHeaders),
               stream,
               folly::none /* priority */,
               http2::kNoPadding /* padding */,
               true,
               true);

  parse();
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(HTTPMethod::CONNECT, callbacks_.msg->getMethod());
  EXPECT_EQ(path, callbacks_.msg->getURL());
  const auto& parsedHeaders = callbacks_.msg->getHeaders();
  EXPECT_EQ(authority,
            parsedHeaders.getSingleOrEmpty(proxygen::HTTP_HEADER_HOST));
  EXPECT_EQ(protocol, *callbacks_.msg->getUpgradeProtocol());
}

void HTTP2CodecTest::testHeaderListSize(bool oversized) {
  if (oversized) {
    auto settings = downstreamCodec_.getEgressSettings();
    settings->setSetting(SettingsId::MAX_HEADER_LIST_SIZE, 37);
  }

  HTTPMessage req = getGetRequest("/guacamole");
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");
  req.getHeaders().add("x-long-long-header",
                       "supercalafragalisticexpialadoshus");
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, true /* eom */);

  parse();
  // session error
  EXPECT_EQ(callbacks_.messageBegin, oversized ? 0 : 1);
  EXPECT_EQ(callbacks_.headersComplete, oversized ? 0 : 1);
  EXPECT_EQ(callbacks_.messageComplete, oversized ? 0 : 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, oversized ? 1 : 0);
}

void HTTP2CodecTest::testFrameSizeLimit(bool oversized) {
  HTTPMessage req = getBigGetRequest("/guacamole");
  auto settings = downstreamCodec_.getEgressSettings();

  if (oversized) {
    // trick upstream for sending a 2x bigger HEADERS frame
    settings->setSetting(SettingsId::MAX_FRAME_SIZE,
                         http2::kMaxFramePayloadLengthMin * 2);
    downstreamCodec_.generateSettings(output_);
    parseUpstream();
  }

  settings->setSetting(SettingsId::MAX_FRAME_SIZE,
                       http2::kMaxFramePayloadLengthMin);
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, true /* eom */);

  parse();
  // session error
  EXPECT_EQ(callbacks_.messageBegin, oversized ? 0 : 1);
  EXPECT_EQ(callbacks_.headersComplete, oversized ? 0 : 1);
  EXPECT_EQ(callbacks_.messageComplete, oversized ? 0 : 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, oversized ? 1 : 0);
}

TEST_F(HTTP2CodecTest, NormalSizeHeader) {
  testHeaderListSize(false);
}

TEST_F(HTTP2CodecTest, OversizedHeader) {
  testHeaderListSize(true);
}

TEST_F(HTTP2CodecTest, NormalSizeFrame) {
  testFrameSizeLimit(false);
}

TEST_F(HTTP2CodecTest, OversizedFrame) {
  testFrameSizeLimit(true);
}

TEST_F(HTTP2CodecTest, BigHeaderCompressed) {
  SetUpUpstreamTest();
  auto settings = downstreamCodec_.getEgressSettings();
  settings->setSetting(SettingsId::MAX_HEADER_LIST_SIZE, 37);
  downstreamCodec_.generateSettings(output_);
  parseUpstream();

  HTTPMessage req = getGetRequest("/guacamole");
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, true /* eom */);

  parse();
  // session error
  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
}

TEST_F(HTTP2CodecTest, BasicHeaderReply) {
  SetUpUpstreamTest();
  upstreamCodec_.createStream();
  HTTPMessage resp;
  resp.setStatusCode(200);
  resp.setStatusMessage("nifty-nice");
  resp.getHeaders().add(HTTP_HEADER_CONTENT_TYPE, "x-coolio");
  downstreamCodec_.generateHeader(output_, 1, resp);
  downstreamCodec_.generateEOM(output_, 1);

  parseUpstream();
  callbacks_.expectMessage(true, 2, 200);
  const auto& headers = callbacks_.msg->getHeaders();
  // HTTP/2 doesnt support serialization - instead you get the default
  EXPECT_EQ("OK", callbacks_.msg->getStatusMessage());
  EXPECT_TRUE(callbacks_.msg->getHeaders().exists(HTTP_HEADER_DATE));
  EXPECT_EQ("x-coolio", headers.getSingleOrEmpty(HTTP_HEADER_CONTENT_TYPE));
}

TEST_F(HTTP2CodecTest, DontDoubleDate) {
  SetUpUpstreamTest();
  upstreamCodec_.createStream();
  HTTPMessage resp;
  resp.setStatusCode(200);
  resp.setStatusMessage("nifty-nice");
  resp.getHeaders().add(HTTP_HEADER_DATE, "Today!");
  resp.getHeaders().add(HTTP_HEADER_CONTENT_TYPE, "x-coolio");
  downstreamCodec_.generateHeader(output_, 1, resp);
  downstreamCodec_.generateEOM(output_, 1);

  parseUpstream();
  callbacks_.expectMessage(true, 2, 200);
  const auto& headers = callbacks_.msg->getHeaders();
  // HTTP/2 doesnt support serialization - instead you get the default
  EXPECT_EQ("OK", callbacks_.msg->getStatusMessage());
  EXPECT_EQ(1,
            callbacks_.msg->getHeaders().getNumberOfValues(HTTP_HEADER_DATE));
  EXPECT_EQ("x-coolio", headers.getSingleOrEmpty(HTTP_HEADER_CONTENT_TYPE));
}

TEST_F(HTTP2CodecTest, BadHeadersReply) {
  static const std::string v1("200");
  static const vector<proxygen::compress::Header> respHeaders = {
      Header::makeHeaderForTest(headers::kStatus, v1),
  };

  HPACKCodec headerCodec(TransportDirection::DOWNSTREAM);
  HTTPCodec::StreamID stream = 1;
  // missing fields (missing authority is OK)
  for (size_t i = 0; i < respHeaders.size(); i++, stream += 2) {
    std::vector<proxygen::compress::Header> allHeaders = respHeaders;
    allHeaders.erase(allHeaders.begin() + i);
    auto encodedHeaders = headerCodec.encode(allHeaders);
    writeHeaders(output_,
                 std::move(encodedHeaders),
                 stream,
                 folly::none,
                 http2::kNoPadding,
                 true,
                 true);
  }
  // dup fields
  std::string v("foomonkey");
  for (size_t i = 0; i < respHeaders.size(); i++, stream += 2) {
    std::vector<proxygen::compress::Header> allHeaders = respHeaders;
    auto h = allHeaders[i];
    h.value = &v;
    allHeaders.push_back(h);
    auto encodedHeaders = headerCodec.encode(allHeaders);
    writeHeaders(output_,
                 std::move(encodedHeaders),
                 stream,
                 folly::none,
                 http2::kNoPadding,
                 true,
                 true);
  }

  parse();
  EXPECT_EQ(callbacks_.messageBegin, 2);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 2);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, StripLwsHeaderValue) {
  static std::string lwsString("\r\n\t ");
  // first request
  HTTPMessage req = getGetRequest("/test");
  req.getHeaders().add(HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE,
                       " application/x-fb");
  // test header value just whitespace
  req.getHeaders().add(HTTPHeaderCode::HTTP_HEADER_USER_AGENT, lwsString);
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, /*eom=*/true);

  auto parseAndCheckExpectations = [&]() {
    parse();
    EXPECT_EQ(callbacks_.messageBegin, 1);
    EXPECT_EQ(callbacks_.headersComplete, 1);
    EXPECT_EQ(callbacks_.messageComplete, 1);
    EXPECT_EQ(callbacks_.streamErrors, 0);
    EXPECT_EQ(callbacks_.sessionErrors, 0);
    ASSERT_TRUE(callbacks_.msg);
  };

  parseAndCheckExpectations();
  auto parsedHeaders = callbacks_.msg->extractHeaders();
  EXPECT_TRUE(parsedHeaders.exists(HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE));
  EXPECT_TRUE(parsedHeaders.exists(HTTPHeaderCode::HTTP_HEADER_USER_AGENT));
  EXPECT_TRUE(
      parsedHeaders.getSingleOrEmpty(HTTPHeaderCode::HTTP_HEADER_USER_AGENT)
          .empty());
  EXPECT_EQ(
      parsedHeaders.getSingleOrEmpty(HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE),
      "application/x-fb");
  callbacks_.reset();

  // second request
  HPACKCodec headerCodec(TransportDirection::UPSTREAM);
  static std::string kContentTypeValue("\tapplication/x-fb");
  static const std::string v1("GET");
  static const std::string v2("/");
  static const std::string v3("http");
  static const std::string v4("foo.com");
  std::vector<proxygen::compress::Header> reqHeaders = {
      Header::makeHeaderForTest(headers::kMethod, v1),
      Header::makeHeaderForTest(headers::kPath, v2),
      Header::makeHeaderForTest(headers::kScheme, v3),
      Header::makeHeaderForTest(headers::kAuthority, v4),
      Header(HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE, kContentTypeValue),
      Header(HTTPHeaderCode::HTTP_HEADER_USER_AGENT, lwsString)};

  auto encodedHeaders = headerCodec.encode(reqHeaders);
  id = upstreamCodec_.createStream();
  writeHeaders(output_,
               std::move(encodedHeaders),
               id,
               folly::none,
               http2::kNoPadding,
               true,
               true);

  parseAndCheckExpectations();
  parsedHeaders = callbacks_.msg->extractHeaders();
  EXPECT_TRUE(parsedHeaders.exists(HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE));
  EXPECT_TRUE(parsedHeaders.exists(HTTPHeaderCode::HTTP_HEADER_USER_AGENT));
  EXPECT_TRUE(
      parsedHeaders.getSingleOrEmpty(HTTPHeaderCode::HTTP_HEADER_USER_AGENT)
          .empty());
  EXPECT_EQ(
      parsedHeaders.getSingleOrEmpty(HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE),
      "application/x-fb");
}

TEST_F(HTTP2CodecTest, Cookies) {
  HTTPMessage req = getGetRequest("/guacamole");
  req.getHeaders().add("Cookie", "chocolate-chip=1");
  req.getHeaders().add("Cookie", "rainbow-chip=2");
  req.getHeaders().add("Cookie", "butterscotch=3");
  req.getHeaders().add("Cookie", "oatmeal-raisin=4");
  req.setSecure(true);
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req);

  parse();
  callbacks_.expectMessage(false, 2, "/guacamole");
  EXPECT_EQ(callbacks_.msg->getCookie("chocolate-chip"), "1");
  EXPECT_EQ(callbacks_.msg->getCookie("rainbow-chip"), "2");
  EXPECT_EQ(callbacks_.msg->getCookie("butterscotch"), "3");
  EXPECT_EQ(callbacks_.msg->getCookie("oatmeal-raisin"), "4");
}

TEST_F(HTTP2CodecTest, BasicContinuation) {
  HTTPMessage req = getBigGetRequest();
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req);

  parse();
  callbacks_.expectMessage(false, -1, "/");
#ifndef NDEBUG
  EXPECT_GT(downstreamCodec_.getReceivedFrameCount(), 1);
#endif
  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_EQ("coolio", headers.getSingleOrEmpty(HTTP_HEADER_USER_AGENT));
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BasicContinuationEndStream) {
  // CONTINUATION with END_STREAM flag set on the preceding HEADERS frame
  HTTPMessage req = getBigGetRequest();
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, true /* eom */);

  parse();
  callbacks_.expectMessage(true, -1, "/");
#ifndef NDEBUG
  EXPECT_GT(downstreamCodec_.getReceivedFrameCount(), 1);
#endif
  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_EQ("coolio", headers.getSingleOrEmpty(HTTP_HEADER_USER_AGENT));
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.messageComplete, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BadContinuation) {
  // CONTINUATION with no preceding HEADERS
  auto fakeHeaders = makeBuf(5);
  http2::writeContinuation(output_, 3, true, std::move(fakeHeaders));

  parse();
  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
}

TEST_F(HTTP2CodecTest, MissingContinuation) {
  IOBufQueue output(IOBufQueue::cacheChainLength());
  HTTPMessage req = getBigGetRequest();

  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req);
  // empirically determined the size of continuation frame, and strip it
  output_.trimEnd(http2::kFrameHeaderSize + 4134);

  // insert a non-continuation (but otherwise valid) frame
  http2::writeGoaway(output_, 17, ErrorCode::ENHANCE_YOUR_CALM);

  parse();
  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
#ifndef NDEBUG
  // frames = 3 to account for settings frame after H2 conn preface
  EXPECT_EQ(downstreamCodec_.getReceivedFrameCount(), 3);
#endif
}

TEST_F(HTTP2CodecTest, MissingContinuationBadFrame) {
  IOBufQueue output(IOBufQueue::cacheChainLength());
  HTTPMessage req = getBigGetRequest();
  upstreamCodec_.generateHeader(output_, 1, req);

  // empirically determined the size of continuation frame, and fake it
  output_.trimEnd(http2::kFrameHeaderSize + 4134);

  // insert an invalid frame
  auto frame = makeBuf(http2::kFrameHeaderSize + 4134);
  *((uint32_t*)frame->writableData()) = 0xfa000000;
  output_.append(std::move(frame));

  parse();
  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
#ifndef NDEBUG
  // frames = 3 to account for settings frame after H2 conn preface
  EXPECT_EQ(downstreamCodec_.getReceivedFrameCount(), 3);
#endif
}

TEST_F(HTTP2CodecTest, BadContinuationStream) {
  HTTPMessage req = getBigGetRequest();
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req);

  // empirically determined the size of continuation frame, and fake it
  output_.trimEnd(http2::kFrameHeaderSize + 4134);
  auto fakeHeaders = makeBuf(4134);
  http2::writeContinuation(output_, 3, true, std::move(fakeHeaders));

  parse();
  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
#ifndef NDEBUG
  // frames = 3 to account for settings frame after H2 conn preface
  EXPECT_EQ(downstreamCodec_.getReceivedFrameCount(), 3);
#endif
}

TEST_F(HTTP2CodecTest, FrameTooLarge) {
  writeFrameHeaderManual(output_, 1 << 15, 0, 0, 1);

  parse();
  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
  EXPECT_TRUE(callbacks_.lastParseError->hasCodecStatusCode());
  EXPECT_EQ(callbacks_.lastParseError->getCodecStatusCode(),
            ErrorCode::FRAME_SIZE_ERROR);
}

TEST_F(HTTP2CodecTest, DataFrameZeroLengthWithEOM) {
  HTTPMessage req = getGetRequest();
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");

  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req);
  // Write Zero length Data frame (just a Frame header) with end of stream set
  writeFrameHeaderManual(output_,
                         0,
                         (uint8_t)http2::FrameType::DATA,
                         (uint8_t)http2::Flags::END_STREAM,
                         1);

  parse();

  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.messageComplete, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, UnknownFrameType) {
  HTTPMessage req = getGetRequest();
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");

  // unknown frame type 17
  writeFrameHeaderManual(output_, 17, 37, 0, 1);
  output_.append("wicked awesome!!!");
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req);

  parse();
  callbacks_.expectMessage(false, 2, ""); // + host
}

TEST_F(HTTP2CodecTest, JunkAfterConnError) {
  HTTPMessage req = getGetRequest();
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");

  // write headers frame for stream 0
  writeFrameHeaderManual(output_, 0, (uint8_t)http2::FrameType::HEADERS, 0, 0);
  // now write a valid headers frame, should never be parsed
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req);

  parse();
  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
}

TEST_F(HTTP2CodecTest, BasicData) {
  string data("abcde");
  auto buf = folly::IOBuf::copyBuffer(data.data(), data.length());
  upstreamCodec_.generateBody(
      output_, 2, std::move(buf), HTTPCodec::NoPadding, true);

  parse();
  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 1);
  EXPECT_EQ(callbacks_.bodyCalls, 1);
  EXPECT_EQ(callbacks_.bodyLength, 5);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  EXPECT_EQ(callbacks_.data_.move()->moveToFbString(), data);
}

TEST_F(HTTP2CodecTest, LongData) {
  // Hack the max frame size artificially low
  HTTPSettings* settings = (HTTPSettings*)upstreamCodec_.getIngressSettings();
  settings->setSetting(SettingsId::MAX_FRAME_SIZE, 16);
  auto buf = makeBuf(100);
  upstreamCodec_.generateBody(
      output_, 1, buf->clone(), HTTPCodec::NoPadding, true);

  parse();
  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 1);
  EXPECT_EQ(callbacks_.bodyCalls, 7);
  EXPECT_EQ(callbacks_.bodyLength, 100);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  EXPECT_EQ(callbacks_.data_.move()->moveToFbString(), buf->moveToFbString());
}

TEST_F(HTTP2CodecTest, PushPromiseContinuation) {
  auto settings = upstreamCodec_.getEgressSettings();
  settings->setSetting(SettingsId::ENABLE_PUSH, 1);
  upstreamCodec_.generateSettings(output_);
  upstreamCodec_.createStream();

  SetUpUpstreamTest();
  // Hack the max frame size artificially low
  settings = (HTTPSettings*)downstreamCodec_.getIngressSettings();
  settings->setSetting(SettingsId::MAX_FRAME_SIZE, 5);
  HTTPHeaderSize size;
  HTTPMessage req = getGetRequest();
  req.getHeaders().add("foomonkey", "george");
  downstreamCodec_.generatePushPromise(output_, 2, req, 1, false, &size);

  parseUpstream();
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.pushId, 2);
  EXPECT_EQ(callbacks_.assocStreamId, 1);
  EXPECT_EQ(callbacks_.headersCompleteId, 2);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, MalformedPaddingLength) {
  const uint8_t badInput[] = {0x50,
                              0x52,
                              0x49,
                              0x20,
                              0x2a,
                              0x20,
                              0x48,
                              0x54,
                              0x54,
                              0x50,
                              0x2f,
                              0x32,
                              0x2e,
                              0x30,
                              0x0d,
                              0x0a,
                              0x0d,
                              0x0a,
                              0x53,
                              0x4d,
                              0x0d,
                              0x0a,
                              0x0d,
                              0x0a,
                              0x00,
                              0x00,
                              0x7e,
                              0x00,
                              0x6f,
                              0x6f,
                              0x6f,
                              0x6f,
                              // The padding length byte below is 0x82 (130
                              // in decimal) which is greater than the length
                              // specified by the header's length field, 126
                              0x01,
                              0x82,
                              0x87,
                              0x44,
                              0x87,
                              0x92,
                              0x97,
                              0x92,
                              0x92,
                              0x92,
                              0x7a,
                              0x0b,
                              0x41,
                              0x89,
                              0xf1,
                              0xe3,
                              0xc0,
                              0xf2,
                              0x9c,
                              0xdd,
                              0x90,
                              0xf4,
                              0xff,
                              0x40,
                              0x80,
                              0x84,
                              0x2d,
                              0x35,
                              0xa7,
                              0xd7};
  output_.reset();
  output_.append(badInput, sizeof(badInput));
  EXPECT_EQ(output_.chainLength(), sizeof(badInput));

  EXPECT_FALSE(parse());
}

TEST_F(HTTP2CodecTest, MalformedPadding) {
  const uint8_t badInput[] = {0x00, 0x00, 0x0d, 0x01, 0xbe, 0x63, 0x0d, 0x0a,
                              0x0d, 0x0a, 0x00, 0x73, 0x00, 0x00, 0x06, 0x08,
                              0x72, 0x00, 0x24, 0x00, 0xfa, 0x4d, 0x0d};
  output_.append(badInput, sizeof(badInput));

  EXPECT_FALSE(parse());
}

TEST_F(HTTP2CodecTestOmitParsePreface, NoAppByte) {
  const uint8_t noAppByte[] = {
      0x50, 0x52, 0x49, 0x20, 0x2a, 0x20, 0x48, 0x54, 0x54, 0x50, 0x2f,
      0x32, 0x2e, 0x30, 0x0d, 0x0a, 0x0d, 0x0a, 0x53, 0x4d, 0x0d, 0x0a,
      0x0d, 0x0a, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x56, 0x00, 0x5d, 0x00, 0x00, 0x00, 0x01, 0x55, 0x00};
  output_.reset();
  output_.append(noAppByte, sizeof(noAppByte));
  EXPECT_EQ(output_.chainLength(), sizeof(noAppByte));

  EXPECT_TRUE(parse());
  EXPECT_EQ(callbacks_.settings, 1);
  EXPECT_EQ(callbacks_.numSettings, 0);
  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.bodyCalls, 1);
  EXPECT_EQ(callbacks_.bodyLength, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, DataFramePartialDataOnFrameHeaderCall) {
  using namespace testing;
  NiceMock<MockHTTPCodecCallback> mockCallback;
  EXPECT_CALL(mockCallback, onFrameHeader(_, _, _, _, _));

  const size_t bufSize = 10;
  auto buf = makeBuf(bufSize);
  const size_t padding = 10;
  upstreamCodec_.generateBody(output_, 1, buf->clone(), padding, true);
  // 9 (frame header) + 1 (padding length) + 10 (body) + 10 (padding)
  EXPECT_EQ(output_.chainLength(), 30);

  downstreamCodec_.setCallback(&mockCallback);

  auto ingress = output_.move();
  ingress->coalesce();
  // Copy partial byte to a new buffer
  auto ingress1 = IOBuf::copyBuffer(ingress->data(), 34);
  downstreamCodec_.onIngress(*ingress1);
}

TEST_F(HTTP2CodecTest, DataFramePartialDataWithNoAppByte) {
  const size_t bufSize = 10;
  auto buf = makeBuf(bufSize);
  const size_t padding = 10;
  upstreamCodec_.generateBody(output_, 1, buf->clone(), padding, true);
  // 9 (frame header) + 1 (padding length) + 10 (body) + 10 (padding)
  EXPECT_EQ(output_.chainLength(), 30);

  auto ingress = output_.move();
  ingress->coalesce();
  // Copy up to the padding length byte to a new buffer
  auto ingress1 = IOBuf::copyBuffer(ingress->data(), 10);
  size_t parsed = downstreamCodec_.onIngress(*ingress1);
  // The 10th byte is the padding length byte which should not be parsed
  EXPECT_EQ(parsed, 9);
  // Copy from the padding length byte to the end
  auto ingress2 = IOBuf::copyBuffer(ingress->data() + 9, 21);
  parsed = downstreamCodec_.onIngress(*ingress2);
  // The padding length byte should be parsed this time along with 10 bytes of
  // application data and 10 bytes of padding
  EXPECT_EQ(parsed, 21);

  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 1);
  EXPECT_EQ(callbacks_.bodyCalls, 1);
  EXPECT_EQ(callbacks_.bodyLength, bufSize);
  // Total padding is the padding length byte and the padding bytes
  EXPECT_EQ(callbacks_.paddingBytes, padding + 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  EXPECT_EQ(callbacks_.data_.move()->moveToFbString(), buf->moveToFbString());
}

TEST_F(HTTP2CodecTest, BasicRst) {
  upstreamCodec_.generateRstStream(output_, 2, ErrorCode::ENHANCE_YOUR_CALM);
  parse();
  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.bodyCalls, 0);
  EXPECT_EQ(callbacks_.aborts, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BasicRstInvalidCode) {
  upstreamCodec_.generateRstStream(output_, 2, ErrorCode::STREAM_CLOSED);
  parse();
  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.bodyCalls, 0);
  EXPECT_EQ(callbacks_.aborts, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BasicPing) {
  upstreamCodec_.generatePingRequest(output_);
  upstreamCodec_.generatePingReply(output_, 17);

  uint64_t pingReq;
  parse([&](IOBuf* ingress) {
    folly::io::Cursor c(ingress);
    c.skip(http2::kFrameHeaderSize);
    pingReq = c.read<uint64_t>();
  });

  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.bodyCalls, 0);
  EXPECT_EQ(callbacks_.recvPingRequest, pingReq);
  EXPECT_EQ(callbacks_.recvPingReply, 17);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BasicWindow) {
  // This test would fail if the codec had window state
  upstreamCodec_.generateWindowUpdate(output_, 0, 10);
  upstreamCodec_.generateWindowUpdate(output_, 0, http2::kMaxWindowUpdateSize);
  upstreamCodec_.generateWindowUpdate(output_, 1, 12);
  upstreamCodec_.generateWindowUpdate(output_, 1, http2::kMaxWindowUpdateSize);

  parse();
  EXPECT_EQ(callbacks_.windowUpdateCalls, 4);
  EXPECT_EQ(callbacks_.windowUpdates[0],
            std::vector<uint32_t>({10, http2::kMaxWindowUpdateSize}));
  EXPECT_EQ(callbacks_.windowUpdates[1],
            std::vector<uint32_t>({12, http2::kMaxWindowUpdateSize}));
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, ZeroWindow) {
  auto streamID = HTTPCodec::StreamID(1);
  // First generate a frame with delta=1 so as to pass the checks, and then
  // hack the frame so that delta=0 without modifying other checks
  upstreamCodec_.generateWindowUpdate(output_, streamID, 1);
  output_.trimEnd(http2::kFrameWindowUpdateSize);
  QueueAppender appender(&output_, http2::kFrameWindowUpdateSize);
  appender.writeBE<uint32_t>(0);

  parse();
  // This test doesn't ensure that RST_STREAM is generated
  EXPECT_EQ(callbacks_.windowUpdateCalls, 0);
  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.lastParseError->getCodecStatusCode(),
            ErrorCode::PROTOCOL_ERROR);
}

TEST_F(HTTP2CodecTest, BasicGoaway) {
  std::unique_ptr<folly::IOBuf> debugData =
      folly::IOBuf::copyBuffer("debugData");
  upstreamCodec_.generateGoaway(
      output_, 17, ErrorCode::ENHANCE_YOUR_CALM, std::move(debugData));

  parse();
  EXPECT_EQ(callbacks_.goaways, 1);
  EXPECT_EQ(callbacks_.data_.move()->moveToFbString(), "debugData");
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BadGoaway) {
  std::unique_ptr<folly::IOBuf> debugData =
      folly::IOBuf::copyBuffer("debugData");
  upstreamCodec_.generateGoaway(
      output_, 17, ErrorCode::ENHANCE_YOUR_CALM, std::move(debugData));
  auto bytes =
      upstreamCodec_.generateGoaway(output_, 27, ErrorCode::ENHANCE_YOUR_CALM);
  ;
  EXPECT_EQ(bytes, 0);
}

TEST_F(HTTP2CodecTest, RstStreamNoError) {
  SetUpUpstreamTest();
  upstreamCodec_.createStream();
  HTTPMessage resp;
  resp.setStatusCode(200);
  resp.setStatusMessage("OK");
  downstreamCodec_.generateHeader(output_, 1, resp);
  downstreamCodec_.generateBody(
      output_, 1, makeBuf(10), HTTPCodec::NoPadding, true);
  downstreamCodec_.generateRstStream(output_, 1, ErrorCode::NO_ERROR);
  parseUpstream();
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.bodyCalls, 1);
  EXPECT_EQ(callbacks_.messageComplete, 1);
  EXPECT_EQ(callbacks_.aborts, 1);
}

TEST_F(HTTP2CodecTest, DoubleGoaway) {
  SetUpUpstreamTest();
  downstreamCodec_.generateGoaway(output_);
  EXPECT_TRUE(downstreamCodec_.isWaitingToDrain());
  EXPECT_TRUE(downstreamCodec_.isReusable());
  EXPECT_TRUE(downstreamCodec_.isStreamIngressEgressAllowed(0));
  EXPECT_TRUE(downstreamCodec_.isStreamIngressEgressAllowed(1));
  EXPECT_TRUE(downstreamCodec_.isStreamIngressEgressAllowed(2));

  EXPECT_TRUE(upstreamCodec_.isStreamIngressEgressAllowed(0));
  EXPECT_TRUE(upstreamCodec_.isStreamIngressEgressAllowed(1));
  EXPECT_TRUE(upstreamCodec_.isStreamIngressEgressAllowed(2));
  EXPECT_TRUE(upstreamCodec_.isReusable());

  parseUpstream();
  EXPECT_TRUE(upstreamCodec_.isStreamIngressEgressAllowed(0));
  EXPECT_TRUE(upstreamCodec_.isStreamIngressEgressAllowed(1));
  EXPECT_TRUE(upstreamCodec_.isStreamIngressEgressAllowed(2));
  EXPECT_FALSE(upstreamCodec_.isReusable());
  EXPECT_EQ(callbacks_.goaways, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);

  downstreamCodec_.generateGoaway(output_, 0, ErrorCode::NO_ERROR);
  EXPECT_FALSE(downstreamCodec_.isWaitingToDrain());
  EXPECT_FALSE(downstreamCodec_.isReusable());
  EXPECT_TRUE(downstreamCodec_.isStreamIngressEgressAllowed(0));
  EXPECT_FALSE(downstreamCodec_.isStreamIngressEgressAllowed(1));
  EXPECT_TRUE(downstreamCodec_.isStreamIngressEgressAllowed(2));

  parseUpstream();
  EXPECT_TRUE(upstreamCodec_.isStreamIngressEgressAllowed(0));
  EXPECT_FALSE(upstreamCodec_.isStreamIngressEgressAllowed(1));
  EXPECT_TRUE(upstreamCodec_.isStreamIngressEgressAllowed(2));
  EXPECT_EQ(callbacks_.goaways, 2);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);

  upstreamCodec_.generateGoaway(output_, 0, ErrorCode::NO_ERROR);
  EXPECT_TRUE(upstreamCodec_.isStreamIngressEgressAllowed(0));
  EXPECT_FALSE(upstreamCodec_.isStreamIngressEgressAllowed(1));
  EXPECT_FALSE(upstreamCodec_.isStreamIngressEgressAllowed(2));
  parse();
  EXPECT_TRUE(downstreamCodec_.isStreamIngressEgressAllowed(0));
  EXPECT_FALSE(downstreamCodec_.isStreamIngressEgressAllowed(1));
  EXPECT_FALSE(downstreamCodec_.isStreamIngressEgressAllowed(2));
}

TEST_F(HTTP2CodecTest, DoubleGoawayWithError) {
  SetUpUpstreamTest();
  std::unique_ptr<folly::IOBuf> debugData =
      folly::IOBuf::copyBuffer("debugData");
  downstreamCodec_.generateGoaway(output_,
                                  std::numeric_limits<int32_t>::max(),
                                  ErrorCode::ENHANCE_YOUR_CALM,
                                  std::move(debugData));
  EXPECT_FALSE(downstreamCodec_.isWaitingToDrain());
  EXPECT_FALSE(downstreamCodec_.isReusable());
  auto ret = downstreamCodec_.generateGoaway(output_, 0, ErrorCode::NO_ERROR);
  EXPECT_EQ(ret, 0);

  parseUpstream();
  EXPECT_EQ(callbacks_.goaways, 1);
  EXPECT_EQ(callbacks_.data_.move()->moveToFbString(), "debugData");
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, GoawayHandling) {
  auto settings = upstreamCodec_.getEgressSettings();
  settings->setSetting(SettingsId::ENABLE_PUSH, 1);
  upstreamCodec_.generateSettings(output_);

  // send request
  HTTPMessage req = getGetRequest();
  HTTPHeaderSize size;
  size.uncompressed = size.compressed = 0;
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, true, &size);
  EXPECT_GT(size.uncompressed, 0);
  parse();
  callbacks_.expectMessage(true, 1, "/");
  callbacks_.reset();

  SetUpUpstreamTest();
  // drain after this message
  downstreamCodec_.generateGoaway(output_, 1, ErrorCode::NO_ERROR);
  parseUpstream();
  // upstream cannot generate id > 1
  upstreamCodec_.generateHeader(output_, 3, req, false, &size);
  EXPECT_EQ(size.uncompressed, 0);
  upstreamCodec_.generateWindowUpdate(output_, 3, 100);
  upstreamCodec_.generateBody(
      output_, 3, makeBuf(10), HTTPCodec::NoPadding, false);
  upstreamCodec_.generatePriority(
      output_, 3, HTTPMessage::HTTP2Priority(0, true, 1));
  upstreamCodec_.generateEOM(output_, 3);
  upstreamCodec_.generateRstStream(output_, 3, ErrorCode::CANCEL);
  EXPECT_EQ(output_.chainLength(), 0);

  // send a push promise that will be rejected by downstream
  req.getHeaders().add("foomonkey", "george");
  downstreamCodec_.generatePushPromise(output_, 2, req, 1, false, &size);
  EXPECT_GT(size.uncompressed, 0);
  HTTPMessage resp;
  resp.setStatusCode(200);
  // send a push response that will be ignored
  downstreamCodec_.generateHeader(output_, 2, resp, false, &size);
  // window update for push doesn't make any sense, but whatever
  downstreamCodec_.generateWindowUpdate(output_, 2, 100);
  downstreamCodec_.generateBody(
      output_, 2, makeBuf(10), HTTPCodec::NoPadding, false);
  writeFrameHeaderManual(output_, 20, (uint8_t)http2::FrameType::DATA, 0, 2);
  output_.append(makeBuf(10));

  // tell the upstream no pushing, and parse the first batch
  IOBufQueue dummy;
  upstreamCodec_.generateGoaway(dummy, 0, ErrorCode::NO_ERROR);
  parseUpstream();

  output_.append(makeBuf(10));
  downstreamCodec_.generatePriority(
      output_, 2, HTTPMessage::HTTP2Priority(0, true, 1));
  downstreamCodec_.generateEOM(output_, 2);
  downstreamCodec_.generateRstStream(output_, 2, ErrorCode::CANCEL);

  // send a response that will be accepted, headers should be ok
  downstreamCodec_.generateHeader(output_, 1, resp, true, &size);
  EXPECT_GT(size.uncompressed, 0);

  // parse the remainder
  parseUpstream();
  callbacks_.expectMessage(true, 1, 200);
}

TEST_F(HTTP2CodecTest, GoawayReply) {
  upstreamCodec_.createStream();
  upstreamCodec_.generateGoaway(output_, 0, ErrorCode::NO_ERROR);

  parse();
  EXPECT_EQ(callbacks_.goaways, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);

  SetUpUpstreamTest();
  HTTPMessage resp;
  resp.setStatusCode(200);
  resp.setStatusMessage("nifty-nice");
  downstreamCodec_.generateHeader(output_, 1, resp);
  downstreamCodec_.generateEOM(output_, 1);
  parseUpstream();
  callbacks_.expectMessage(true, 1, 200);
  EXPECT_TRUE(callbacks_.msg->getHeaders().exists(HTTP_HEADER_DATE));
}

TEST_F(HTTP2CodecTest, BasicSetting) {
  auto settings = upstreamCodec_.getEgressSettings();
  settings->setSetting(SettingsId::MAX_CONCURRENT_STREAMS, 37);
  settings->setSetting(SettingsId::INITIAL_WINDOW_SIZE, 12345);
  upstreamCodec_.generateSettings(output_);

  parse();
  EXPECT_EQ(callbacks_.settings, 1);
  EXPECT_EQ(callbacks_.maxStreams, 37);
  EXPECT_EQ(callbacks_.windowSize, 12345);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, SettingsAck) {
  upstreamCodec_.generateSettingsAck(output_);

  parse();
  EXPECT_EQ(callbacks_.settings, 0);
  EXPECT_EQ(callbacks_.settingsAcks, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BadSettings) {
  auto settings = upstreamCodec_.getEgressSettings();
  settings->setSetting(SettingsId::INITIAL_WINDOW_SIZE, 0xffffffff);
  upstreamCodec_.generateSettings(output_);

  parse();
  EXPECT_EQ(callbacks_.settings, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
}

TEST_F(HTTP2CodecTestOmitParsePreface, BadPushSettings) {
  auto settings = downstreamCodec_.getEgressSettings();
  settings->clearSettings();
  settings->setSetting(SettingsId::ENABLE_PUSH, 0);
  SetUpUpstreamTest();

  parseUpstream([&](IOBuf* ingress) {
    EXPECT_EQ(ingress->computeChainDataLength(), http2::kFrameHeaderSize);
  });
  EXPECT_FALSE(upstreamCodec_.supportsPushTransactions());
  // Only way to disable push for downstreamCodec_ is to read
  // ENABLE_PUSH:0 from client
  EXPECT_TRUE(downstreamCodec_.supportsPushTransactions());
  EXPECT_EQ(callbacks_.settings, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, SettingsTableSize) {
  auto settings = upstreamCodec_.getEgressSettings();
  settings->setSetting(SettingsId::HEADER_TABLE_SIZE, 8192);
  upstreamCodec_.generateSettings(output_);

  parse();
  EXPECT_EQ(callbacks_.settings, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  downstreamCodec_.generateSettingsAck(output_);
  parseUpstream();

  callbacks_.reset();

  HTTPMessage resp;
  resp.setStatusCode(200);
  resp.setStatusMessage("nifty-nice");
  resp.getHeaders().add(HTTP_HEADER_CONTENT_TYPE, "x-coolio");
  SetUpUpstreamTest();
  upstreamCodec_.createStream();
  downstreamCodec_.generateHeader(output_, 1, resp);

  parseUpstream();
  callbacks_.expectMessage(false, 2, 200);
  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_TRUE(callbacks_.msg->getHeaders().exists(HTTP_HEADER_DATE));
  EXPECT_EQ("x-coolio", headers.getSingleOrEmpty(HTTP_HEADER_CONTENT_TYPE));
}

TEST_F(HTTP2CodecTestOmitParsePreface, BadSettingsTableSize) {
  auto settings = upstreamCodec_.getEgressSettings();
  settings->setSetting(SettingsId::HEADER_TABLE_SIZE, 8192);
  // This sets the max decoder table size to 8k
  upstreamCodec_.generateSettings(output_);

  parse();
  EXPECT_EQ(callbacks_.settings, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);

  callbacks_.reset();

  // Attempt to set a new max table size.  This is a no-op because the first,
  // setting is unacknowledged.  The upstream encoder will up the table size to
  // 8k per the first settings frame and the HPACK codec will send a code to
  // update the decoder.
  settings->setSetting(SettingsId::HEADER_TABLE_SIZE, 4096);

  HTTPMessage resp;
  resp.setStatusCode(200);
  resp.setStatusMessage("nifty-nice");
  resp.getHeaders().add(HTTP_HEADER_CONTENT_TYPE, "x-coolio");
  SetUpUpstreamTest();
  downstreamCodec_.generateHeader(output_, 1, resp);

  parseUpstream();
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
}

TEST_F(HTTP2CodecTest, SettingsTableSizeEarlyShrink) {
  // Lower size to 2k
  auto settings = upstreamCodec_.getEgressSettings();
  settings->setSetting(SettingsId::HEADER_TABLE_SIZE, 2048);
  upstreamCodec_.generateSettings(output_);

  parse();
  EXPECT_EQ(callbacks_.settings, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  downstreamCodec_.generateSettingsAck(output_);
  // Parsing SETTINGS ack updates upstream decoder to 2k
  parseUpstream();

  callbacks_.reset();

  HTTPMessage resp;
  resp.setStatusCode(200);
  resp.setStatusMessage("nifty-nice");
  resp.getHeaders().add(HTTP_HEADER_CONTENT_TYPE, "x-coolio");
  SetUpUpstreamTest();
  upstreamCodec_.createStream();
  // downstream encoder will send TSU/2k
  downstreamCodec_.generateHeader(output_, 1, resp);

  // sets pending table size to 512, but doesn't update it yet
  settings = upstreamCodec_.getEgressSettings();
  settings->setSetting(SettingsId::HEADER_TABLE_SIZE, 512);
  IOBufQueue tmp{IOBufQueue::cacheChainLength()};
  upstreamCodec_.generateSettings(tmp);

  // Previous code would barf here, since TSU/2k is a violation of the current
  // max=512
  parseUpstream();
  callbacks_.expectMessage(false, 2, 200);
  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_TRUE(callbacks_.msg->getHeaders().exists(HTTP_HEADER_DATE));
  EXPECT_EQ("x-coolio", headers.getSingleOrEmpty(HTTP_HEADER_CONTENT_TYPE));
}

TEST_F(HTTP2CodecTest, ConcurrentStreams) {
  auto settings = upstreamCodec_.getEgressSettings();
  settings->setSetting(SettingsId::ENABLE_PUSH, 1);
  settings->setSetting(SettingsId::MAX_CONCURRENT_STREAMS, 0);
  upstreamCodec_.generateSettings(output_);
  HTTPMessage req = getGetRequest();
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, true /* eom */);

  parse();
  EXPECT_EQ(callbacks_.maxStreams, 0);

  callbacks_.reset();
  SetUpUpstreamTest();
  HTTPMessage resp;
  resp.setStatusCode(200);
  downstreamCodec_.generateHeader(output_, 1, resp, false /* eom */);
  downstreamCodec_.generatePushPromise(output_, 2, req, 1);
  parseUpstream();
  EXPECT_EQ(callbacks_.messageBegin, 2);
  EXPECT_EQ(callbacks_.headersComplete, 2);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  EXPECT_NE(callbacks_.msg, nullptr);
  EXPECT_EQ(callbacks_.pushId, 2);
  EXPECT_EQ(callbacks_.assocStreamId, 1);
  callbacks_.reset();
  downstreamCodec_.generateHeader(output_, 2, resp, true /* eom */);
  parseUpstream();
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.lastParseError->getCodecStatusCode(),
            ErrorCode::REFUSED_STREAM);
}

TEST_F(HTTP2CodecTest, BasicPriority) {
  auto pri = HTTPMessage::HTTP2Priority(0, true, 1);
  upstreamCodec_.generatePriority(output_, 1, pri);

  EXPECT_TRUE(parse());
  EXPECT_EQ(callbacks_.priority, pri);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BadHeaderPriority) {
  HTTPMessage req = getGetRequest();
  req.setHTTP2Priority(HTTPMessage::HTTP2Priority(0, false, 7));
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, true /* eom */);

  // hack ingress with cirular dep
  EXPECT_TRUE(parse([&](IOBuf* ingress) {
    folly::io::RWPrivateCursor c(ingress);
    c.skip(http2::kFrameHeaderSize);
    c.writeBE<uint32_t>(1);
  }));

  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, CircularHeaderPriority) {
  HTTPMessage req = getGetRequest();
  req.setHTTP2Priority(HTTPMessage::HTTP2Priority(1, false, 7));
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, true /* eom */);
}

TEST_F(HTTP2CodecTest, DuplicateBadHeaderPriority) {
  // Sent an initial header with a circular dependency
  HTTPMessage req = getGetRequest();
  req.setHTTP2Priority(HTTPMessage::HTTP2Priority(0, false, 7));
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, true /* eom */);

  // Hack ingress with circular dependency.
  EXPECT_TRUE(parse([&](IOBuf* ingress) {
    folly::io::RWPrivateCursor c(ingress);
    c.skip(http2::kFrameHeaderSize);
    c.writeBE<uint32_t>(1);
  }));

  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.sessionErrors, 0);

  // On the same stream, send another request, interpreted as trailers with
  // a pseudo header which is illegal
  HTTPMessage nextRequest = getGetRequest();
  upstreamCodec_.generateHeader(output_, id, nextRequest, true /* eom */);
  parse();
  EXPECT_EQ(callbacks_.streamErrors, 2);
  EXPECT_EQ(callbacks_.sessionErrors, 0);

  callbacks_.reset();

  // Now send a legit request
  auto id2 = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id2, nextRequest, true /* eom */);
  parse();
  callbacks_.expectMessage(true, 1, "/");
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BadPriority) {
  auto pri = HTTPMessage::HTTP2Priority(0, true, 1);
  upstreamCodec_.generatePriority(output_, 1, pri);

  // hack ingress with cirular dep
  EXPECT_TRUE(parse([&](IOBuf* ingress) {
    folly::io::RWPrivateCursor c(ingress);
    c.skip(http2::kFrameHeaderSize);
    c.writeBE<uint32_t>(1);
  }));

  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

class DummyQueue : public HTTPCodec::PriorityQueue {
 public:
  DummyQueue() {
  }
  ~DummyQueue() override {
  }
  void addPriorityNode(HTTPCodec::StreamID id, HTTPCodec::StreamID) override {
    nodes_.push_back(id);
  }

  std::vector<HTTPCodec::StreamID> nodes_;
};

TEST_F(HTTP2CodecTest, VirtualNodes) {
  DummyQueue queue;
  uint8_t level = 30;
  upstreamCodec_.addPriorityNodes(queue, output_, level);

  EXPECT_TRUE(parse());
  for (int i = 0; i < level; i++) {
    EXPECT_EQ(queue.nodes_[i], upstreamCodec_.mapPriorityToDependency(i));
  }

  // Out-of-range priorites are mapped to the lowest level of virtual nodes.
  EXPECT_EQ(queue.nodes_[level - 1],
            upstreamCodec_.mapPriorityToDependency(level));
  EXPECT_EQ(queue.nodes_[level - 1],
            upstreamCodec_.mapPriorityToDependency(level + 1));
}

TEST_F(HTTP2CodecTest, BasicPushPromise) {
  upstreamCodec_.generateSettings(output_);
  parse();
  EXPECT_FALSE(upstreamCodec_.supportsPushTransactions());
  EXPECT_FALSE(downstreamCodec_.supportsPushTransactions());

  auto settings = upstreamCodec_.getEgressSettings();
  settings->setSetting(SettingsId::ENABLE_PUSH, 1);
  upstreamCodec_.generateSettings(output_);
  parse();
  EXPECT_TRUE(upstreamCodec_.supportsPushTransactions());
  EXPECT_TRUE(downstreamCodec_.supportsPushTransactions());

  SetUpUpstreamTest();
  upstreamCodec_.createStream();
  upstreamCodec_.createStream();
  upstreamCodec_.createStream();

  // Test had previously hardcoded 7
  HTTPCodec::StreamID assocStream = upstreamCodec_.createStream();
  for (auto i = 0; i < 2; i++) {
    // Push promise
    HTTPCodec::StreamID pushStream = downstreamCodec_.createStream();
    HTTPMessage req = getGetRequest();
    req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");
    downstreamCodec_.generatePushPromise(output_, pushStream, req, assocStream);

    parseUpstream();
    callbacks_.expectMessage(false, 2, "/"); // + host
    EXPECT_EQ(callbacks_.assocStreamId, assocStream);
    EXPECT_EQ(callbacks_.headersCompleteId, pushStream);
    auto& headers = callbacks_.msg->getHeaders();
    EXPECT_EQ("coolio", headers.getSingleOrEmpty(HTTP_HEADER_USER_AGENT));
    callbacks_.reset();

    // Actual reply headers
    HTTPMessage resp;
    resp.setStatusCode(200);
    resp.getHeaders().add(HTTP_HEADER_CONTENT_TYPE, "text/plain");
    downstreamCodec_.generateHeader(output_, pushStream, resp);

    parseUpstream();
    callbacks_.expectMessage(false, 2, 200);
    EXPECT_EQ(callbacks_.headersCompleteId, pushStream);
    EXPECT_EQ(callbacks_.assocStreamId, 0);
    EXPECT_TRUE(callbacks_.msg->getHeaders().exists(HTTP_HEADER_DATE));
    EXPECT_EQ("text/plain",
              callbacks_.msg->getHeaders().getSingleOrEmpty(
                  HTTP_HEADER_CONTENT_TYPE));
    callbacks_.reset();
  }
}

TEST_F(HTTP2CodecTest, DuplicatePushPromise) {
  auto settings = upstreamCodec_.getEgressSettings();
  settings->setSetting(SettingsId::ENABLE_PUSH, 1);
  upstreamCodec_.generateSettings(output_);
  parse();
  EXPECT_TRUE(upstreamCodec_.supportsPushTransactions());
  EXPECT_TRUE(downstreamCodec_.supportsPushTransactions());

  SetUpUpstreamTest();

  HTTPCodec::StreamID assocStream = 7;
  HTTPCodec::StreamID pushStream = downstreamCodec_.createStream();
  HTTPMessage req = getGetRequest();
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");
  downstreamCodec_.generatePushPromise(output_, pushStream, req, assocStream);
  downstreamCodec_.generatePushPromise(output_, pushStream, req, assocStream);

  parseUpstream();

  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
}

TEST_F(HTTP2CodecTest, BadPushPromise) {
  // ENABLE_PUSH is now 0 by default
  SetUpUpstreamTest();
  HTTPMessage req = getGetRequest();
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");
  downstreamCodec_.generatePushPromise(output_, 2, req, 1);

  parseUpstream();
  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.assocStreamId, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
}

TEST_F(HTTP2CodecTest, BadPushPromiseResets) {
  auto settings = upstreamCodec_.getEgressSettings();
  settings->setSetting(SettingsId::ENABLE_PUSH, 1);
  upstreamCodec_.generateSettings(output_);
  SetUpUpstreamTest();
  HTTPMessage req = getGetRequest();
  req.getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "100");
  req.getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "200");
  downstreamCodec_.generatePushPromise(output_, 2, req, 1);

  parseUpstream();
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.assocStreamId, 1);
  EXPECT_EQ(callbacks_.streamErrors, 2);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BasicCertificateRequest) {
  uint16_t requestId = 17;
  std::unique_ptr<folly::IOBuf> authRequest =
      folly::IOBuf::copyBuffer("authRequestData");
  upstreamCodec_.generateCertificateRequest(
      output_, requestId, std::move(authRequest));

  parse();
  EXPECT_EQ(callbacks_.certificateRequests, 1);
  EXPECT_EQ(callbacks_.lastCertRequestId, requestId);
  EXPECT_EQ(callbacks_.data_.move()->moveToFbString(), "authRequestData");
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BasicCertificate) {
  uint16_t certId = 17;
  std::unique_ptr<folly::IOBuf> authenticator =
      folly::IOBuf::copyBuffer("authenticatorData");
  upstreamCodec_.generateCertificate(output_, certId, std::move(authenticator));

  parse();
  EXPECT_EQ(callbacks_.certificates, 1);
  EXPECT_EQ(callbacks_.lastCertId, certId);
  EXPECT_EQ(callbacks_.data_.move()->moveToFbString(), "authenticatorData");
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, BadServerPreface) {
  output_.reset();
  downstreamCodec_.generateWindowUpdate(output_, 0, 10);
  parseUpstream();
  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.assocStreamId, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
}

TEST_F(HTTP2CodecTest, Normal1024Continuation) {
  HTTPMessage req = getGetRequest();
  string bigval(8691, '!');
  bigval.append(8691, '@');
  req.getHeaders().add("x-headr", bigval);
  req.setHTTP2Priority(HTTPMessage::HTTP2Priority(0, false, 7));
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req);

  parse();
  callbacks_.expectMessage(false, -1, "/");
  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_EQ(bigval, headers.getSingleOrEmpty("x-headr"));
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);

  upstreamCodec_.generateSettingsAck(output_);
  parse();
  EXPECT_EQ(callbacks_.settingsAcks, 1);
}

TEST_F(HTTP2CodecTest, StreamIdOverflow) {
  HTTP2Codec codec(TransportDirection::UPSTREAM);

  HTTPCodec::StreamID streamId;
  codec.setNextEgressStreamId(std::numeric_limits<int32_t>::max() - 10);
  while (codec.isReusable()) {
    streamId = codec.createStream();
  }
  EXPECT_EQ(streamId, std::numeric_limits<int32_t>::max() - 2);
}

TEST_F(HTTP2CodecTest, TestMultipleDifferentContentLengthHeaders) {
  // Generate a POST request with two Content-Length headers
  // NOTE: getPostRequest already adds the content-length
  HTTPMessage req = getPostRequest();
  req.getHeaders().add(HTTP_HEADER_CONTENT_LENGTH, "300");
  EXPECT_EQ(req.getHeaders().getNumberOfValues(HTTP_HEADER_CONTENT_LENGTH), 2);

  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, true /* eom */);
  parse();

  // Check that the request fails before the codec finishes parsing the headers
  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.lastParseError->getHttpStatusCode(), 400);
}

TEST_F(HTTP2CodecTest, TestMultipleIdenticalContentLengthHeaders) {
  // Generate a POST request with two Content-Length headers
  // NOTE: getPostRequest already adds the content-length
  HTTPMessage req = getPostRequest();
  req.getHeaders().add("content-length", "200");
  EXPECT_EQ(req.getHeaders().getNumberOfValues("content-length"), 2);

  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, true /* eom */);
  parse();

  // Check that the headers parsing completes correctly
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.headersComplete, 1);
}

namespace {
void testRequestUpgrade(HTTPMessage& req, size_t numConnectionHeaders) {
  HTTP2Codec::requestUpgrade(req);
  EXPECT_EQ(req.getHeaders().getSingleOrEmpty(HTTP_HEADER_UPGRADE), "h2c");
  EXPECT_TRUE(
      req.checkForHeaderToken(HTTP_HEADER_CONNECTION, "Upgrade", false));
  EXPECT_TRUE(req.checkForHeaderToken(
      HTTP_HEADER_CONNECTION, http2::kProtocolSettingsHeader.c_str(), false));
  EXPECT_EQ(req.getHeaders().getNumberOfValues(HTTP_HEADER_CONNECTION),
            numConnectionHeaders);
  EXPECT_GT(req.getHeaders()
                .getSingleOrEmpty(http2::kProtocolSettingsHeader)
                .length(),
            0);
}
} // namespace

TEST_F(HTTP2CodecTest, CleartextUpgrade) {
  HTTPMessage req = getGetRequest("/guacamole");
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");
  testRequestUpgrade(req, 1);
  req.stripPerHopHeaders();

  req.getHeaders().add(HTTP_HEADER_CONNECTION, "Foo");
  testRequestUpgrade(req, 2);
  req.stripPerHopHeaders();

  req.getHeaders().add(HTTP_HEADER_CONNECTION, "Upgrade");
  testRequestUpgrade(req, 2);
  req.stripPerHopHeaders();

  req.getHeaders().add(HTTP_HEADER_CONNECTION, "HTTP2-Settings");
  testRequestUpgrade(req, 2);
}

TEST_F(HTTP2CodecTest, HTTP2SettingsSuccess) {
  HTTPMessage req = getGetRequest("/guacamole");

  // empty settings
  req.getHeaders().add(http2::kProtocolSettingsHeader, "");
  EXPECT_TRUE(downstreamCodec_.onIngressUpgradeMessage(req));

  // real settings (overwrites empty)
  HTTP2Codec::requestUpgrade(req);
  EXPECT_TRUE(downstreamCodec_.onIngressUpgradeMessage(req));
}

TEST_F(HTTP2CodecTest, HTTP2SettingsFailure) {
  HTTPMessage req = getGetRequest("/guacamole");
  // no settings
  EXPECT_FALSE(downstreamCodec_.onIngressUpgradeMessage(req));

  HTTPHeaders& headers = req.getHeaders();

  // Not base64_url settings
  headers.set(http2::kProtocolSettingsHeader, "????");
  EXPECT_FALSE(downstreamCodec_.onIngressUpgradeMessage(req));
  headers.set(http2::kProtocolSettingsHeader, "AAA");
  EXPECT_FALSE(downstreamCodec_.onIngressUpgradeMessage(req));

  // Too big
  string bigSettings((http2::kMaxFramePayloadLength + 1) * 4 / 3, 'A');
  headers.set(http2::kProtocolSettingsHeader, bigSettings);
  EXPECT_FALSE(downstreamCodec_.onIngressUpgradeMessage(req));

  // Malformed (not a multiple of 6)
  headers.set(http2::kProtocolSettingsHeader, "AAAA");
  EXPECT_FALSE(downstreamCodec_.onIngressUpgradeMessage(req));

  // Two headers
  headers.set(http2::kProtocolSettingsHeader, "AAAAAAAA");
  headers.add(http2::kProtocolSettingsHeader, "AAAAAAAA");
  EXPECT_FALSE(downstreamCodec_.onIngressUpgradeMessage(req));
}

TEST_F(HTTP2CodecTest, HTTP2EnableConnect) {
  SetUpUpstreamTest();
  // egress settings have no connect settings.
  auto ws_enable = upstreamCodec_.getEgressSettings()->getSetting(
      SettingsId::ENABLE_CONNECT_PROTOCOL);
  // enable connect settings, and check.
  upstreamCodec_.getEgressSettings()->setSetting(
      SettingsId::ENABLE_CONNECT_PROTOCOL, 1);
  ws_enable = upstreamCodec_.getEgressSettings()->getSetting(
      SettingsId::ENABLE_CONNECT_PROTOCOL);
  EXPECT_EQ(ws_enable->value, 1);
  // generateSettings.
  // pass the buffer to be parsed by the codec and check for ingress settings.
  upstreamCodec_.generateSettings(output_);
  parseUpstream();
  EXPECT_EQ(1, upstreamCodec_.peerHasWebsockets());
}

TEST_F(HTTP2CodecTest, WebsocketUpgrade) {
  HTTPMessage req = getGetRequest("/apples");
  req.setSecure(true);
  req.setEgressWebsocketUpgrade();

  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, false);
  parse();

  EXPECT_TRUE(callbacks_.msg->isIngressWebsocketUpgrade());
  EXPECT_NE(nullptr, callbacks_.msg->getUpgradeProtocol());
  EXPECT_EQ(headers::kWebsocketString, *callbacks_.msg->getUpgradeProtocol());
}

TEST_F(HTTP2CodecTest, WebsocketBadHeader) {
  const std::string kConnect{"CONNECT"};
  const std::string kWebsocketPath{"/websocket"};
  const std::string kSchemeHttps{"https"};
  vector<proxygen::compress::Header> reqHeaders = {
      Header::makeHeaderForTest(headers::kMethod, kConnect),
      Header::makeHeaderForTest(headers::kProtocol, headers::kWebsocketString),
  };
  vector<proxygen::compress::Header> optionalHeaders = {
      Header::makeHeaderForTest(headers::kPath, kWebsocketPath),
      Header::makeHeaderForTest(headers::kScheme, kSchemeHttps),
  };

  HPACKCodec headerCodec(TransportDirection::UPSTREAM);
  int stream = 1;
  for (size_t i = 0; i < optionalHeaders.size(); ++i, stream += 2) {
    auto headers = reqHeaders;
    headers.push_back(optionalHeaders[i]);
    auto encodedHeaders = headerCodec.encode(headers);
    writeHeaders(output_,
                 std::move(encodedHeaders),
                 stream,
                 folly::none,
                 http2::kNoPadding,
                 false,
                 true);
    parse();
  }

  EXPECT_EQ(callbacks_.messageBegin, optionalHeaders.size());
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, optionalHeaders.size());
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, WebsocketDupProtocol) {
  const std::string kConnect{"CONNECT"};
  const std::string kWebsocketPath{"/websocket"};
  const std::string kSchemeHttps{"https"};
  vector<proxygen::compress::Header> headers = {
      Header::makeHeaderForTest(headers::kMethod, kConnect),
      Header::makeHeaderForTest(headers::kProtocol, headers::kWebsocketString),
      Header::makeHeaderForTest(headers::kProtocol, headers::kWebsocketString),
      Header::makeHeaderForTest(headers::kPath, kWebsocketPath),
      Header::makeHeaderForTest(headers::kScheme, kSchemeHttps),
  };
  HPACKCodec headerCodec(TransportDirection::UPSTREAM);
  auto encodedHeaders = headerCodec.encode(headers);
  writeHeaders(output_,
               std::move(encodedHeaders),
               1,
               folly::none,
               http2::kNoPadding,
               false,
               true);
  parse();
  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, WebsocketIncorrectResponse) {
  output_.reset();
  HTTPMessage resp;
  resp.setStatusCode(400);
  resp.setStatusMessage("Bad Request");
  downstreamCodec_.generateHeader(output_, 1, resp);
  parseUpstream();
  EXPECT_EQ(callbacks_.streamErrors, 0);
}

TEST_F(HTTP2CodecTest, WebTransportProtocol) {
  HTTPMessage req;
  req.setMethod(HTTPMethod::CONNECT);
  req.setHTTPVersion(1, 1);
  req.setURL("/wt");
  req.setSecure(true);
  req.setUpgradeProtocol("webtransport");

  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, false);
  parse();

  EXPECT_FALSE(callbacks_.msg->isIngressWebsocketUpgrade());
  EXPECT_NE(nullptr, callbacks_.msg->getUpgradeProtocol());
  EXPECT_EQ("webtransport", *callbacks_.msg->getUpgradeProtocol());
}

TEST_F(HTTP2CodecTest, TestAllEgressFrameTypeCallbacks) {
  class CallbackTypeTracker {
    std::set<uint8_t> types;

   public:
    void add(uint8_t, uint8_t type, uint64_t, uint16_t) {
      types.insert(type);
    }

    bool isAllFrameTypesReceived() {
      http2::FrameType expectedTypes[] = {
          http2::FrameType::DATA,
          http2::FrameType::HEADERS,
          http2::FrameType::PRIORITY,
          http2::FrameType::RST_STREAM,
          http2::FrameType::SETTINGS,
          http2::FrameType::PUSH_PROMISE,
          http2::FrameType::PING,
          http2::FrameType::GOAWAY,
          http2::FrameType::WINDOW_UPDATE,
          http2::FrameType::CONTINUATION,
          http2::FrameType::EX_HEADERS,
      };

      for (http2::FrameType type : expectedTypes) {
        EXPECT_TRUE(types.find(static_cast<uint8_t>(type)) != types.end())
            << "callback missing for type " << static_cast<uint8_t>(type);
      }
      return types.size() == (sizeof(expectedTypes) / sizeof(http2::FrameType));
    }
  };

  CallbackTypeTracker callbackTypeTracker;

  NiceMock<MockHTTPCodecCallback> mockCallback;
  upstreamCodec_.setCallback(&mockCallback);
  downstreamCodec_.setCallback(&mockCallback);
  EXPECT_CALL(mockCallback, onGenerateFrameHeader(_, _, _, _))
      .WillRepeatedly(Invoke(&callbackTypeTracker, &CallbackTypeTracker::add));

  // DATA frame
  string data("abcde");
  auto buf = folly::IOBuf::copyBuffer(data.data(), data.length());
  upstreamCodec_.generateBody(
      output_, 2, std::move(buf), HTTPCodec::NoPadding, true);

  HTTPHeaderSize size;
  size.uncompressed = size.compressed = 0;
  HTTPMessage req = getGetRequest();
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req, true, &size);

  upstreamCodec_.generatePriority(
      output_, 3, HTTPMessage::HTTP2Priority(0, true, 1));
  upstreamCodec_.generateRstStream(output_, 2, ErrorCode::ENHANCE_YOUR_CALM);
  upstreamCodec_.generateSettings(output_);
  downstreamCodec_.generatePushPromise(output_, 2, req, 1);
  upstreamCodec_.generatePingRequest(output_);

  std::unique_ptr<folly::IOBuf> debugData =
      folly::IOBuf::copyBuffer("debugData");
  upstreamCodec_.generateGoaway(
      output_, 17, ErrorCode::ENHANCE_YOUR_CALM, std::move(debugData));

  upstreamCodec_.generateWindowUpdate(output_, 0, 10);

  HTTPCodec::StreamID stream = folly::Random::rand32(10, 1024) * 2;
  HTTPCodec::StreamID controlStream = folly::Random::rand32(10, 1024) * 2 + 1;
  downstreamCodec_.generateExHeader(
      output_, stream, req, HTTPCodec::ExAttributes(controlStream, true));

  // Tests the continuation frame
  req = getBigGetRequest();
  upstreamCodec_.generateHeader(output_, id, req, true /* eom */);

  EXPECT_TRUE(callbackTypeTracker.isAllFrameTypesReceived());
}

TEST_F(HTTP2CodecTest, Trailers) {
  HTTPMessage req = getGetRequest("/guacamole");
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req);

  string data("abcde");
  auto buf = folly::IOBuf::copyBuffer(data.data(), data.length());
  upstreamCodec_.generateBody(
      output_, 1, std::move(buf), HTTPCodec::NoPadding, false /* eom */);

  HTTPHeaders trailers;
  trailers.add("x-trailer-1", "pico-de-gallo");
  upstreamCodec_.generateTrailers(output_, 1, trailers);

  parse();

  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.bodyCalls, 1);
  EXPECT_EQ(callbacks_.bodyLength, 5);
  EXPECT_EQ(callbacks_.trailers, 1);
  EXPECT_NE(nullptr, callbacks_.msg->getTrailers());
  EXPECT_EQ("pico-de-gallo",
            callbacks_.msg->getTrailers()->getSingleOrEmpty("x-trailer-1"));
  EXPECT_EQ(callbacks_.messageComplete, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
#ifndef NDEBUG
  // frames = 4 to account for settings frame after H2 conn preface
  EXPECT_EQ(downstreamCodec_.getReceivedFrameCount(), 4);
#endif
}

TEST_F(HTTP2CodecTest, TrailersWithPseudoHeaders) {
  HTTPMessage req = getGetRequest("/guacamole");
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req);

  string data("abcde");
  auto buf = folly::IOBuf::copyBuffer(data.data(), data.length());
  upstreamCodec_.generateBody(
      output_, 1, std::move(buf), HTTPCodec::NoPadding, false /* eom */);

  HPACKCodec headerCodec(TransportDirection::UPSTREAM);
  std::string post("POST");
  std::vector<proxygen::compress::Header> trailers = {
      Header::makeHeaderForTest(headers::kMethod, post)};
  auto encodedTrailers = headerCodec.encode(trailers);
  writeHeaders(output_,
               std::move(encodedTrailers),
               1,
               folly::none,
               http2::kNoPadding,
               true,
               true);

  parse();

  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.bodyCalls, 1);
  EXPECT_EQ(callbacks_.bodyLength, 5);
  EXPECT_EQ(callbacks_.trailers, 0);
  EXPECT_EQ(nullptr, callbacks_.msg->getTrailers());
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 1);
}

TEST_F(HTTP2CodecTest, TrailersNoBody) {
  HTTPMessage req = getGetRequest("/guacamole");
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req);

  HTTPHeaders trailers;
  trailers.add("x-trailer-1", "pico-de-gallo");
  upstreamCodec_.generateTrailers(output_, id, trailers);

  parse();

  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.bodyCalls, 0);
  EXPECT_EQ(callbacks_.bodyLength, 0);
  EXPECT_EQ(callbacks_.trailers, 1);
  EXPECT_NE(nullptr, callbacks_.msg->getTrailers());
  EXPECT_EQ("pico-de-gallo",
            callbacks_.msg->getTrailers()->getSingleOrEmpty("x-trailer-1"));
  EXPECT_EQ(callbacks_.messageComplete, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
#ifndef NDEBUG
  // frames = 3 to account for settings frame after H2 conn preface
  EXPECT_EQ(downstreamCodec_.getReceivedFrameCount(), 3);
#endif
}

TEST_F(HTTP2CodecTest, TrailersContinuation) {
  HTTPMessage req = getGetRequest("/guacamole");
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req);

  HTTPHeaders trailers;
  trailers.add("x-trailer-1", "pico-de-gallo");
  trailers.add("x-huge-trailer",
               std::string(http2::kMaxFramePayloadLengthMin, '!'));
  upstreamCodec_.generateTrailers(output_, 1, trailers);

  parse();

  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.messageComplete, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  EXPECT_NE(callbacks_.msg, nullptr);
  EXPECT_EQ(callbacks_.trailers, 1);
  EXPECT_NE(callbacks_.msg->getTrailers(), nullptr);
  EXPECT_EQ("pico-de-gallo",
            callbacks_.msg->getTrailers()->getSingleOrEmpty("x-trailer-1"));
  EXPECT_EQ(std::string(http2::kMaxFramePayloadLengthMin, '!'),
            callbacks_.msg->getTrailers()->getSingleOrEmpty("x-huge-trailer"));
#ifndef NDEBUG
  // frames = 4 to account for settings frame after H2 conn preface
  EXPECT_EQ(downstreamCodec_.getReceivedFrameCount(), 4);
#endif
}

TEST_F(HTTP2CodecTest, TrailersReply) {
  SetUpUpstreamTest();
  upstreamCodec_.createStream();
  HTTPMessage resp;
  resp.setStatusCode(200);
  resp.setStatusMessage("nifty-nice");
  resp.getHeaders().add(HTTP_HEADER_CONTENT_TYPE, "x-coolio");
  downstreamCodec_.generateHeader(output_, 1, resp);

  string data("abcde");
  auto buf = folly::IOBuf::copyBuffer(data.data(), data.length());
  downstreamCodec_.generateBody(
      output_, 1, std::move(buf), HTTPCodec::NoPadding, false);

  HTTPHeaders trailers;
  trailers.add("x-trailer-1", "pico-de-gallo");
  trailers.add("x-trailer-2", "chicken-kyiv");
  downstreamCodec_.generateTrailers(output_, 1, trailers);

  parseUpstream();

  callbacks_.expectMessage(true, 2, 200);
  EXPECT_EQ(callbacks_.bodyCalls, 1);
  EXPECT_EQ(callbacks_.bodyLength, 5);
  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_TRUE(callbacks_.msg->getHeaders().exists(HTTP_HEADER_DATE));
  EXPECT_EQ("x-coolio", headers.getSingleOrEmpty(HTTP_HEADER_CONTENT_TYPE));
  EXPECT_EQ(1, callbacks_.trailers);
  EXPECT_NE(nullptr, callbacks_.msg->getTrailers());
  EXPECT_EQ("pico-de-gallo",
            callbacks_.msg->getTrailers()->getSingleOrEmpty("x-trailer-1"));
  EXPECT_EQ("chicken-kyiv",
            callbacks_.msg->getTrailers()->getSingleOrEmpty("x-trailer-2"));
#ifndef NDEBUG
  EXPECT_EQ(upstreamCodec_.getReceivedFrameCount(), 4);
#endif
}

TEST_F(HTTP2CodecTest, TrailersReplyEmpty) {
  SetUpUpstreamTest();
  upstreamCodec_.createStream();
  HTTPMessage resp;
  resp.setStatusCode(200);
  resp.setStatusMessage("nifty-nice");
  resp.getHeaders().add(HTTP_HEADER_CONTENT_TYPE, "x-coolio");
  downstreamCodec_.generateHeader(output_, 1, resp);

  string data("abcde");
  auto buf = folly::IOBuf::copyBuffer(data.data(), data.length());
  downstreamCodec_.generateBody(
      output_, 1, std::move(buf), HTTPCodec::NoPadding, false);

  HTTPHeaders trailers;
  auto trailerSz = downstreamCodec_.generateTrailers(output_, 1, trailers);
  // Just a frame header (DATA + END_STREAM).
  EXPECT_EQ(trailerSz, 9);

  parseUpstream();

  callbacks_.expectMessage(true, 2, 200);
  EXPECT_EQ(callbacks_.bodyCalls, 1);
  EXPECT_EQ(callbacks_.bodyLength, 5);
  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_TRUE(callbacks_.msg->getHeaders().exists(HTTP_HEADER_DATE));
  EXPECT_EQ("x-coolio", headers.getSingleOrEmpty(HTTP_HEADER_CONTENT_TYPE));
  EXPECT_EQ(0, callbacks_.trailers);
  EXPECT_EQ(nullptr, callbacks_.msg->getTrailers());
#ifndef NDEBUG
  EXPECT_EQ(upstreamCodec_.getReceivedFrameCount(), 4);
#endif
}

TEST_F(HTTP2CodecTest, TrailersReplyWithNoData) {
  SetUpUpstreamTest();
  upstreamCodec_.createStream();
  HTTPMessage resp;
  resp.setStatusCode(200);
  resp.setStatusMessage("nifty-nice");
  resp.getHeaders().add(HTTP_HEADER_CONTENT_TYPE, "x-coolio");
  downstreamCodec_.generateHeader(output_, 1, resp);

  HTTPHeaders trailers;
  trailers.add("x-trailer-1", "pico-de-gallo");
  downstreamCodec_.generateTrailers(output_, 1, trailers);

  parseUpstream();

  callbacks_.expectMessage(true, 2, 200);
  EXPECT_EQ(callbacks_.bodyCalls, 0);
  EXPECT_EQ(callbacks_.bodyLength, 0);
  const auto& headers = callbacks_.msg->getHeaders();
  EXPECT_TRUE(callbacks_.msg->getHeaders().exists(HTTP_HEADER_DATE));
  EXPECT_EQ("x-coolio", headers.getSingleOrEmpty(HTTP_HEADER_CONTENT_TYPE));
  EXPECT_EQ(1, callbacks_.trailers);
  EXPECT_NE(nullptr, callbacks_.msg->getTrailers());
  EXPECT_EQ("pico-de-gallo",
            callbacks_.msg->getTrailers()->getSingleOrEmpty("x-trailer-1"));
#ifndef NDEBUG
  EXPECT_EQ(upstreamCodec_.getReceivedFrameCount(), 3);
#endif
}

TEST_F(HTTP2CodecTest, TrailersReplyWithPseudoHeaders) {
  SetUpUpstreamTest();
  upstreamCodec_.createStream();
  HTTPMessage resp;
  resp.setStatusCode(200);
  resp.setStatusMessage("nifty-nice");
  resp.getHeaders().add(HTTP_HEADER_CONTENT_TYPE, "x-coolio");
  downstreamCodec_.generateHeader(output_, 1, resp);

  string data("abcde");
  auto buf = folly::IOBuf::copyBuffer(data.data(), data.length());
  downstreamCodec_.generateBody(
      output_, 1, std::move(buf), HTTPCodec::NoPadding, false);

  HPACKCodec headerCodec(TransportDirection::DOWNSTREAM);
  std::string post("POST");
  std::vector<proxygen::compress::Header> trailers = {
      Header::makeHeaderForTest(headers::kMethod, post)};
  auto encodedTrailers = headerCodec.encode(trailers);
  writeHeaders(output_,
               std::move(encodedTrailers),
               1,
               folly::none,
               http2::kNoPadding,
               true,
               true);
  parseUpstream();

  // Unfortunately, you get 2x messageBegin calls for parse error in
  // upstream trailers
  EXPECT_EQ(callbacks_.messageBegin, 2);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.trailers, 0);
  EXPECT_EQ(nullptr, callbacks_.msg->getTrailers());
  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, TrailersReplyContinuation) {
  SetUpUpstreamTest();
  upstreamCodec_.createStream();
  HTTPMessage resp;
  resp.setStatusCode(200);
  downstreamCodec_.generateHeader(output_, 1, resp);

  HTTPHeaders trailers;
  trailers.add("x-trailer-1", "pico-de-gallo");
  trailers.add("x-huge-trailer",
               std::string(http2::kMaxFramePayloadLengthMin, '!'));
  downstreamCodec_.generateTrailers(output_, 1, trailers);

  parseUpstream();

  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.messageComplete, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
  EXPECT_NE(callbacks_.msg, nullptr);
  EXPECT_EQ(callbacks_.msg->getStatusCode(), 200);
  EXPECT_EQ(1, callbacks_.trailers);
  EXPECT_NE(nullptr, callbacks_.msg->getTrailers());
  EXPECT_EQ("pico-de-gallo",
            callbacks_.msg->getTrailers()->getSingleOrEmpty("x-trailer-1"));
  EXPECT_EQ(std::string(http2::kMaxFramePayloadLengthMin, '!'),
            callbacks_.msg->getTrailers()->getSingleOrEmpty("x-huge-trailer"));
#ifndef NDEBUG
  EXPECT_EQ(upstreamCodec_.getReceivedFrameCount(), 4);
#endif
}

TEST_F(HTTP2CodecTest, TrailersReplyMissingContinuation) {
  SetUpUpstreamTest();
  upstreamCodec_.createStream();
  HTTPMessage resp;
  resp.setStatusCode(200);
  downstreamCodec_.generateHeader(output_, 1, resp);

  HTTPHeaders trailers;
  trailers.add("x-trailer-1", "pico-de-gallo");
  trailers.add("x-huge-trailer",
               std::string(http2::kMaxFramePayloadLengthMin, '!'));
  downstreamCodec_.generateTrailers(output_, 1, trailers);
  // empirically determined the size of continuation frame, and strip it
  output_.trimEnd(http2::kFrameHeaderSize + 4132);

  // insert a non-continuation (but otherwise valid) frame
  http2::writeGoaway(output_, 17, ErrorCode::ENHANCE_YOUR_CALM);

  parseUpstream();

  EXPECT_EQ(callbacks_.messageBegin, 1);
  EXPECT_EQ(callbacks_.headersComplete, 1);
  EXPECT_EQ(callbacks_.messageComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
#ifndef NDEBUG
  EXPECT_EQ(upstreamCodec_.getReceivedFrameCount(), 4);
#endif
}

TEST_F(HTTP2CodecTest, TrailersNotLatest) {
  HTTPMessage req = getGetRequest("/guacamole");
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");
  auto id = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id, req);
  auto id2 = upstreamCodec_.createStream();
  upstreamCodec_.generateHeader(output_, id2, req);

  HTTPHeaders trailers;
  trailers.add("x-trailer-1", "pico-de-gallo");
  upstreamCodec_.generateTrailers(output_, id, trailers);
  upstreamCodec_.generateHeader(output_, id2, req);

  parse();

  EXPECT_EQ(callbacks_.messageBegin, 2);
  EXPECT_EQ(callbacks_.headersComplete, 2);
  EXPECT_EQ(callbacks_.bodyCalls, 0);
  EXPECT_EQ(callbacks_.trailers, 1);
  EXPECT_NE(nullptr, callbacks_.msg->getTrailers());
  EXPECT_EQ("pico-de-gallo",
            callbacks_.msg->getTrailers()->getSingleOrEmpty("x-trailer-1"));
  EXPECT_EQ(callbacks_.messageComplete, 1);
  EXPECT_EQ(callbacks_.streamErrors, 1);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}

TEST_F(HTTP2CodecTest, ResponseWithoutRequest) {
  HTTPMessage resp;
  resp.setStatusCode(200);
  downstreamCodec_.generateHeader(output_, 1, resp);

  parseUpstream();

  EXPECT_EQ(callbacks_.messageBegin, 0);
  EXPECT_EQ(callbacks_.headersComplete, 0);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 1);
}

class HTTP2CodecDoubleGoawayTest : public HTTPParallelCodecTest {
 public:
  HTTP2CodecDoubleGoawayTest()
      : HTTPParallelCodecTest(upstreamCodec_, downstreamCodec_) {
  }

  void SetUp() override {
    HTTPParallelCodecTest::SetUp();
  }

 protected:
  HTTP2Codec upstreamCodec_{TransportDirection::UPSTREAM};
  HTTP2Codec downstreamCodec_{TransportDirection::DOWNSTREAM};
};

TEST_F(HTTP2CodecDoubleGoawayTest, EnableDoubleGoawayAfterClose) {
  SetUpUpstreamTest();
  upstreamCodec_.generateGoaway(output_);
  // Now a no-op
  upstreamCodec_.enableDoubleGoawayDrain();
  parseUpstream();
  EXPECT_EQ(callbacks_.goaways, 1);
  EXPECT_EQ(callbacks_.streamErrors, 0);
  EXPECT_EQ(callbacks_.sessionErrors, 0);
}
