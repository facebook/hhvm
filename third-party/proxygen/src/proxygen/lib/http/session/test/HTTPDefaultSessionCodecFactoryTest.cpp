/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HTTPDefaultSessionCodecFactory.h>

#include <memory>

#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/codec/HTTP2Constants.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/services/AcceptorConfiguration.h>

using namespace proxygen;

TEST(HTTPDefaultSessionCodecFactoryTest, GetCodecH2) {
  auto conf = std::make_shared<AcceptorConfiguration>();
  // If set directly on the acceptor, we should always return the H2C version.
  conf->plaintextProtocol = "h2c";
  HTTPDefaultSessionCodecFactory factory(std::move(conf));
  auto codec = factory.getCodec(
      "http/1.1", TransportDirection::DOWNSTREAM, false /* isTLS */);
  HTTP2Codec* httpCodec = dynamic_cast<HTTP2Codec*>(codec.get());
  EXPECT_NE(httpCodec, nullptr);
  EXPECT_EQ(httpCodec->getProtocol(), CodecProtocol::HTTP_2);

  // On a somewhat contrived example, if TLS we should return the version
  // negotiated through ALPN.
  codec = factory.getCodec(
      "http/1.1", TransportDirection::UPSTREAM, true /* isTLS */);
  HTTP1xCodec* http1Codec = dynamic_cast<HTTP1xCodec*>(codec.get());
  EXPECT_NE(http1Codec, nullptr);
  EXPECT_EQ(http1Codec->getProtocol(), CodecProtocol::HTTP_1_1);
}

TEST(HTTPDefaultSessionCodecFactoryTest, GetCodec) {
  auto conf = std::make_shared<AcceptorConfiguration>();
  HTTPDefaultSessionCodecFactory factory(std::move(conf));

  // Empty protocol should default to http/1.1
  auto codec =
      factory.getCodec("", TransportDirection::DOWNSTREAM, false /* isTLS */);
  HTTP1xCodec* http1Codec = dynamic_cast<HTTP1xCodec*>(codec.get());
  EXPECT_NE(http1Codec, nullptr);
  EXPECT_EQ(http1Codec->getProtocol(), CodecProtocol::HTTP_1_1);

  codec =
      factory.getCodec("h2", TransportDirection::DOWNSTREAM, false /* isTLS */);
  HTTP2Codec* httpCodec = dynamic_cast<HTTP2Codec*>(codec.get());
  EXPECT_NE(httpCodec, nullptr);
  EXPECT_EQ(httpCodec->getProtocol(), CodecProtocol::HTTP_2);

  // Not supported protocols should return nullptr.
  codec = factory.getCodec(
      "not/supported", TransportDirection::DOWNSTREAM, false /* isTLS */);
  EXPECT_EQ(codec, nullptr);
}

struct TestParams {
  bool strict;
  std::string plaintextProto;
};

class HTTPDefaultSessionCodecFactoryValidationTest
    : public ::testing::TestWithParam<TestParams> {};

TEST_P(HTTPDefaultSessionCodecFactoryValidationTest, StrictValidation) {
  auto conf = std::make_shared<AcceptorConfiguration>();
  conf->plaintextProtocol = GetParam().plaintextProto;
  auto isPlaintextProtocolEmpty = conf->plaintextProtocol.empty();
  HTTPDefaultSessionCodecFactory factory(std::move(conf));
  bool strict = GetParam().strict;
  factory.setConfigFn([strict] {
    HTTPCodecFactory::CodecConfig config;
    config.strictValidation = strict;
    return config;
  });

  auto codec = factory.getCodec(
      http2::kProtocolString, TransportDirection::DOWNSTREAM, false);
  HTTP2Codec upstream(TransportDirection::UPSTREAM);
  HTTPMessage req;
  folly::IOBufQueue output{folly::IOBufQueue::cacheChainLength()};
  req.setURL("/foo\xff");
  upstream.generateConnectionPreface(output);
  upstream.generateSettings(output);
  upstream.generateHeader(output, upstream.createStream(), req, true, nullptr);
  FakeHTTPCodecCallback callbacks;
  codec->setCallback(&callbacks);
  codec->onIngress(*output.front());
  EXPECT_EQ(callbacks.messageBegin, 1);
  EXPECT_EQ(callbacks.headersComplete, strict ? 0 : 1);
  EXPECT_EQ(callbacks.streamErrors, strict ? 1 : 0);
  output.reset();

  if (isPlaintextProtocolEmpty) {
    callbacks.reset();
    codec = factory.getCodec("http/1.1", TransportDirection::DOWNSTREAM, true);
    codec->setCallback(&callbacks);
    codec->onIngress(
        *folly::IOBuf::copyBuffer("GET /foo\xff HTTP/1.1\r\n\r\n"));
    EXPECT_EQ(callbacks.messageBegin, 1);
    EXPECT_EQ(callbacks.headersComplete, strict ? 0 : 1);
    EXPECT_EQ(callbacks.streamErrors, strict ? 1 : 0);
  }
}

INSTANTIATE_TEST_SUITE_P(
    HTTPDefaultSessionCodecFactoryTest,
    HTTPDefaultSessionCodecFactoryValidationTest,
    ::testing::Values(TestParams({true, std::string()}),
                      TestParams({true, std::string("h2c")}),
                      TestParams({false, std::string("")}),
                      TestParams({false, std::string("h2c")})));
