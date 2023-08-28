/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>

using namespace proxygen;

namespace {
void parseBufWithH1Codec(FakeHTTPCodecCallback& callback,
                         const folly::IOBuf& buf) {
  HTTP1xCodec codec(TransportDirection::DOWNSTREAM);
  codec.setCallback(&callback);
  auto bytesProcessed = codec.onIngress(buf);
  EXPECT_EQ(bytesProcessed, buf.computeChainDataLength());
}
} // namespace

// This test converts a HTTP CONNECT message from HTTP/1.1 to HTTP/2 then back
// to HTTP/1.1 and expects (1) we're conforming to spec after every conversion,
// and (2) we get the same message at the end (i.e., no loss of
// information and no alteration). The reason is that in RFC7540 Section 8.3,
// format of HTTP CONNECT messages is different in H2.
TEST(CrossCodecTest, ConnectRequestConversion) {
  constexpr std::string_view kOriginalRequest =
      ("CONNECT server.example.com:80 HTTP/1.1\r\n"
       "Host: server.example.com:80\r\n"
       "Proxy-Authorization: basic Zm9vOg==\r\n"
       "\r\n");

  auto verifyOriginalRequest = [](const HTTPMessage& msg) {
    EXPECT_EQ(msg.getMethod(), HTTPMethod::CONNECT);
    EXPECT_EQ(msg.getHTTPVersion(), HTTPMessage::kHTTPVersion11);
    EXPECT_EQ(msg.getURL(), "server.example.com:80");
    EXPECT_EQ(msg.getHeaders().getSingleOrEmpty(HTTP_HEADER_HOST),
              "server.example.com:80");
    EXPECT_EQ(
        msg.getHeaders().getSingleOrEmpty(HTTP_HEADER_PROXY_AUTHORIZATION),
        "basic Zm9vOg==");
  };

  // Use H1 codec to parse the message.
  FakeHTTPCodecCallback h1RecvCallback;
  parseBufWithH1Codec(h1RecvCallback,
                      *folly::IOBuf::copyBuffer(kOriginalRequest));
  verifyOriginalRequest(*h1RecvCallback.msg);

  // Then use H2 codec to dump the message.
  folly::IOBufQueue h2OutBuf{folly::IOBufQueue::cacheChainLength()};
  HTTP2Codec h2SendCodec(TransportDirection::UPSTREAM);
  h2SendCodec.generateConnectionPreface(h2OutBuf);
  h2SendCodec.generateSettings(h2OutBuf);
  auto h2SendId = h2SendCodec.createStream();
  h2SendCodec.generateHeader(
      h2OutBuf, h2SendId, *h1RecvCallback.msg, false /* eom */);

  // And use H2 codec to parse the dumped message.
  FakeHTTPCodecCallback h2RecvCallback;
  HTTP2Codec h2RecvCodec(TransportDirection::DOWNSTREAM);
  h2RecvCodec.setCallback(&h2RecvCallback);
  size_t h2BytesProcessed = h2RecvCodec.onIngress(*h2OutBuf.front());
  EXPECT_EQ(h2BytesProcessed, h2OutBuf.chainLength());
  EXPECT_EQ(h2RecvCallback.msg->getMethod(), HTTPMethod::CONNECT);
  EXPECT_EQ(h2RecvCallback.msg->getHTTPVersion(), HTTPMessage::kHTTPVersion11);
  // Per https://httpwg.org/specs/rfc7540.html#CONNECT :path pseudo-header
  // must be omitted.
  EXPECT_EQ(h2RecvCallback.msg->getURL(), "");
  EXPECT_EQ(h2RecvCallback.msg->getHeaders().getSingleOrEmpty(HTTP_HEADER_HOST),
            "server.example.com:80");
  EXPECT_EQ(h2RecvCallback.msg->getHeaders().getSingleOrEmpty(
                HTTP_HEADER_PROXY_AUTHORIZATION),
            "basic Zm9vOg==");

  // Then dump the message with H1 codec.
  folly::IOBufQueue h1OutBuf(folly::IOBufQueue::cacheChainLength());
  HTTP1xCodec h1SendCodec(TransportDirection::UPSTREAM);
  h1SendCodec.generateHeader(
      h1OutBuf, h1SendCodec.createStream(), *h2RecvCallback.msg);

  // Finally, load the buf with H1 codec again, and hopefully we get the same
  // message as original.
  FakeHTTPCodecCallback lastH1RecvCallback;
  parseBufWithH1Codec(lastH1RecvCallback, *h1OutBuf.front());
  verifyOriginalRequest(*lastH1RecvCallback.msg);
}
