/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/DefaultHTTPCodecFactory.h>

#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/codec/HTTP2Constants.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>

using namespace proxygen;

TEST(DefaultHTTPCodecFactoryTest, GetCodec) {
  DefaultHTTPCodecFactory factory(false);

  auto codec = factory.getCodec(
      http2::kProtocolString, TransportDirection::UPSTREAM, true);
  HTTP2Codec* http2Codec = dynamic_cast<HTTP2Codec*>(codec.get());
  EXPECT_NE(http2Codec, nullptr);

  codec = factory.getCodec("http/1.1", TransportDirection::UPSTREAM, true);
  HTTP1xCodec* http1xCodec = dynamic_cast<HTTP1xCodec*>(codec.get());
  EXPECT_NE(http1xCodec, nullptr);

  codec = factory.getCodec("", TransportDirection::UPSTREAM, true);
  http1xCodec = dynamic_cast<HTTP1xCodec*>(codec.get());
  EXPECT_NE(http1xCodec, nullptr);

  codec = factory.getCodec("not/supported", TransportDirection::UPSTREAM, true);
  http1xCodec = dynamic_cast<HTTP1xCodec*>(codec.get());
  EXPECT_NE(http1xCodec, nullptr);
}

class DefaultHTTPCodecFactoryValidationTest
    : public ::testing::TestWithParam<bool> {};

TEST_P(DefaultHTTPCodecFactoryValidationTest, StrictValidation) {
  DefaultHTTPCodecFactory factory(false);
  bool strict = GetParam();
  factory.setStrictValidationFn([strict] { return strict; });

  auto codec = factory.getCodec(
      http2::kProtocolString, TransportDirection::DOWNSTREAM, true);
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

  callbacks.reset();
  codec = factory.getCodec("http/1.1", TransportDirection::DOWNSTREAM, true);
  codec->setCallback(&callbacks);
  codec->onIngress(*folly::IOBuf::copyBuffer("GET /foo\xff HTTP/1.1\r\n\r\n"));
  EXPECT_EQ(callbacks.messageBegin, 1);
  EXPECT_EQ(callbacks.headersComplete, strict ? 0 : 1);
  EXPECT_EQ(callbacks.streamErrors, strict ? 1 : 0);
}

INSTANTIATE_TEST_SUITE_P(DefaultHTTPCodecFactoryTest,
                         DefaultHTTPCodecFactoryValidationTest,
                         ::testing::Values(true, false));
