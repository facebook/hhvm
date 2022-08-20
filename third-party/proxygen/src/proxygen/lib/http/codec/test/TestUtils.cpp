/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/test/TestUtils.h>

#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/codec/HTTP2Constants.h>

#include <boost/optional/optional_io.hpp>
#include <folly/Random.h>
#include <folly/io/Cursor.h>

using namespace folly::io;
using namespace std;
using namespace testing;

namespace proxygen {

const HTTPSettings kDefaultIngressSettings{
    {SettingsId::INITIAL_WINDOW_SIZE, 65536}};

std::unique_ptr<folly::IOBuf> makeBuf(uint32_t size) {
  auto out = folly::IOBuf::create(size);
  out->append(size);
  // fill with random junk
  RWPrivateCursor cursor(out.get());
  while (cursor.length() >= 8) {
    cursor.write<uint64_t>(folly::Random::rand64());
  }
  while (cursor.length()) {
    cursor.write<uint8_t>((uint8_t)folly::Random::rand32());
  }
  return out;
}

std::unique_ptr<testing::NiceMock<MockHTTPCodec>> makeMockParallelCodec(
    TransportDirection dir) {
  auto codec = std::make_unique<testing::NiceMock<MockHTTPCodec>>();
  EXPECT_CALL(*codec, supportsParallelRequests())
      .WillRepeatedly(testing::Return(true));
  EXPECT_CALL(*codec, getProtocol())
      .WillRepeatedly(testing::Return(CodecProtocol::HTTP_2));
  EXPECT_CALL(*codec, isReusable()).WillRepeatedly(testing::Return(true));
  EXPECT_CALL(*codec, getTransportDirection())
      .WillRepeatedly(testing::Return(dir));
  EXPECT_CALL(*codec, getIngressSettings())
      .WillRepeatedly(testing::Return(&kDefaultIngressSettings));
  return codec;
}

std::unique_ptr<testing::NiceMock<MockHTTPCodec>>
makeDownstreamParallelCodec() {
  return makeMockParallelCodec(TransportDirection::DOWNSTREAM);
}

std::unique_ptr<testing::NiceMock<MockHTTPCodec>> makeUpstreamParallelCodec() {
  return makeMockParallelCodec(TransportDirection::UPSTREAM);
}

HTTPMessage getGetRequest(const std::string& url) {
  HTTPMessage req;
  req.setMethod("GET");
  req.setURL(url);
  req.setHTTPVersion(1, 1);
  req.getHeaders().set(HTTP_HEADER_HOST, "www.foo.com");
  return req;
}

HTTPMessage getBigGetRequest(const std::string& url) {
  HTTPMessage req;
  req.setMethod("GET");
  req.setURL(url);
  req.setHTTPVersion(1, 1);
  req.getHeaders().set(HTTP_HEADER_HOST, "www.foo.com");
  req.getHeaders().add(HTTP_HEADER_USER_AGENT, "coolio");
  req.getHeaders().add("x-huge-header",
                       std::string(http2::kMaxFramePayloadLengthMin, '!'));
  return req;
}

std::unique_ptr<HTTPMessage> makeGetRequest() {
  return std::make_unique<HTTPMessage>(getGetRequest());
}

HTTPMessage getPostRequest(uint32_t contentLength) {
  HTTPMessage req;
  req.setMethod("POST");
  req.setURL<string>("/");
  req.setHTTPVersion(1, 1);
  req.getHeaders().set(HTTP_HEADER_HOST, "www.foo.com");
  req.getHeaders().set(HTTP_HEADER_CONTENT_LENGTH,
                       folly::to<string>(contentLength));
  return req;
}

HTTPMessage getChunkedPostRequest() {
  HTTPMessage req;
  req.setMethod("POST");
  req.setURL<string>("/");
  req.setHTTPVersion(1, 1);
  req.setIsChunked(true);
  req.getHeaders().set(HTTP_HEADER_HOST, "www.foo.com");
  req.getHeaders().set(HTTP_HEADER_TRANSFER_ENCODING, "chunked");
  return req;
}

std::unique_ptr<HTTPMessage> makePostRequest(uint32_t contentLength) {
  return std::make_unique<HTTPMessage>(getPostRequest(contentLength));
}

HTTPMessage getPubRequest(const std::string& url) {
  HTTPMessage req;
  req.setMethod("PUB");
  req.setURL(url);
  req.setHTTPVersion(1, 1);
  req.getHeaders().set(HTTP_HEADER_HOST, "www.foo.com");
  return req;
}

HTTPMessage getResponse(uint32_t code, uint32_t bodyLen) {
  HTTPMessage resp;
  resp.setStatusCode(code);
  if (bodyLen > 0) {
    resp.getHeaders().set(HTTP_HEADER_CONTENT_LENGTH,
                          folly::to<string>(bodyLen));
  }
  return resp;
}

std::unique_ptr<HTTPMessage> makeResponse(uint16_t statusCode) {
  auto resp = std::make_unique<HTTPMessage>();
  resp->setStatusCode(statusCode);
  resp->setHTTPVersion(1, 1);
  return resp;
}

std::tuple<std::unique_ptr<HTTPMessage>, std::unique_ptr<folly::IOBuf>>
makeResponse(uint16_t statusCode, size_t len) {
  auto resp = makeResponse(statusCode);
  resp->getHeaders().set(HTTP_HEADER_CONTENT_LENGTH, folly::to<string>(len));
  return std::make_pair(std::move(resp), makeBuf(len));
}

HTTPMessage getUpgradeRequest(const std::string& upgradeHeader,
                              HTTPMethod method,
                              uint32_t bodyLen) {
  HTTPMessage req = getGetRequest();
  req.setMethod(method);
  if (upgradeHeader.find(http2::kProtocolCleartextString) !=
      std::string::npos) {
    HTTP2Codec::requestUpgrade(req);
  }
  req.getHeaders().set(HTTP_HEADER_UPGRADE, upgradeHeader);
  if (bodyLen > 0) {
    req.getHeaders().set(HTTP_HEADER_CONTENT_LENGTH,
                         folly::to<std::string>(bodyLen));
  }
  return req;
}

HTTPMessage getResponseWithInvalidBodyLength() {
  HTTPMessage resp;
  resp.setStatusCode(200);
  auto bodyLen = "invalid";
  resp.getHeaders().set(HTTP_HEADER_CONTENT_LENGTH, bodyLen);
  return resp;
}

bool isH3GreaseId(uint64_t id) {
  if (id < 0x21 || id > 0x3FFFFFFFFFFFFFFF) {
    return false;
  }
  return (((id - 0x21) % 0x1F) == 0);
}

void fakeMockCodec(MockHTTPCodec& codec) {
  // For each generate* function, write some data to the chain
  EXPECT_CALL(codec, generateHeader(_, _, _, _, _, _))
      .WillRepeatedly(Invoke(
          [](folly::IOBufQueue& writeBuf,
             HTTPCodec::StreamID /*stream*/,
             const HTTPMessage& /*msg*/,
             bool /*eom*/,
             HTTPHeaderSize* /*size*/,
             folly::Optional<HTTPHeaders>) { writeBuf.append(makeBuf(10)); }));

  EXPECT_CALL(codec, generatePushPromise(_, _, _, _, _, _))
      .WillRepeatedly(Invoke(
          [](folly::IOBufQueue& writeBuf,
             HTTPCodec::StreamID /*stream*/,
             const HTTPMessage& /*msg*/,
             HTTPCodec::StreamID /*assocStream*/,
             bool /*eom*/,
             HTTPHeaderSize* /*size*/) { writeBuf.append(makeBuf(10)); }));

  EXPECT_CALL(codec, generateBody(_, _, _, _, _))
      .WillRepeatedly(Invoke([](folly::IOBufQueue& writeBuf,
                                HTTPCodec::StreamID /*stream*/,
                                std::shared_ptr<folly::IOBuf> chain,
                                folly::Optional<uint8_t> /*padding*/,
                                bool /*eom*/) {
        auto len = chain->computeChainDataLength();
        writeBuf.append(chain->clone());
        return len;
      }));

  EXPECT_CALL(codec, generateChunkHeader(_, _, _))
      .WillRepeatedly(Invoke([](folly::IOBufQueue& writeBuf,
                                HTTPCodec::StreamID /*stream*/,
                                size_t length) {
        writeBuf.append(makeBuf(length));
        return length;
      }));

  EXPECT_CALL(codec, generateChunkTerminator(_, _))
      .WillRepeatedly(Invoke(
          [](folly::IOBufQueue& writeBuf, HTTPCodec::StreamID /*stream*/) {
            writeBuf.append(makeBuf(4));
            return 4;
          }));

  EXPECT_CALL(codec, generateTrailers(_, _, _))
      .WillRepeatedly(Invoke([](folly::IOBufQueue& writeBuf,
                                HTTPCodec::StreamID /*stream*/,
                                const HTTPHeaders& /*trailers*/) {
        writeBuf.append(makeBuf(30));
        return 30;
      }));

  EXPECT_CALL(codec, generateEOM(_, _))
      .WillRepeatedly(Invoke(
          [](folly::IOBufQueue& writeBuf, HTTPCodec::StreamID /*stream*/) {
            writeBuf.append(makeBuf(6));
            return 6;
          }));

  EXPECT_CALL(codec, generateRstStream(_, _, _))
      .WillRepeatedly(Invoke([](folly::IOBufQueue& writeBuf,
                                HTTPCodec::StreamID /*stream*/,
                                ErrorCode /*code*/) {
        writeBuf.append(makeBuf(6));
        return 6;
      }));

  EXPECT_CALL(codec, generateGoaway(_, _, _, _))
      .WillRepeatedly(Invoke([](folly::IOBufQueue& writeBuf,
                                uint32_t /*lastStream*/,
                                ErrorCode,
                                std::shared_ptr<folly::IOBuf>) {
        writeBuf.append(makeBuf(6));
        return 6;
      }));

  EXPECT_CALL(codec, generatePingRequest(_))
      .WillRepeatedly(Invoke([](folly::IOBufQueue& writeBuf) {
        writeBuf.append(makeBuf(6));
        return 6;
      }));

  EXPECT_CALL(codec, generatePingReply(_, _))
      .WillRepeatedly(Invoke([](folly::IOBufQueue& writeBuf, uint64_t /*id*/) {
        writeBuf.append(makeBuf(6));
        return 6;
      }));

  EXPECT_CALL(codec, generateSettings(_))
      .WillRepeatedly(Invoke([](folly::IOBufQueue& writeBuf) {
        writeBuf.append(makeBuf(6));
        return 6;
      }));

  EXPECT_CALL(codec, generateWindowUpdate(_, _, _))
      .WillRepeatedly(Invoke([](folly::IOBufQueue& writeBuf,
                                HTTPCodec::StreamID /*stream*/,
                                uint32_t /*delta*/) {
        writeBuf.append(makeBuf(6));
        return 6;
      }));

  EXPECT_CALL(codec, generateCertificateRequest(_, _, _))
      .WillRepeatedly(Invoke([](folly::IOBufQueue& writeBuf,
                                uint16_t /*requestId*/,
                                std::shared_ptr<folly::IOBuf>) {
        writeBuf.append(makeBuf(6));
        return 6;
      }));

  EXPECT_CALL(codec, generateCertificate(_, _, _))
      .WillRepeatedly(Invoke([](folly::IOBufQueue& writeBuf,
                                uint16_t /*certId*/,
                                std::shared_ptr<folly::IOBuf>) {
        writeBuf.append(makeBuf(6));
        return 6;
      }));
}
} // namespace proxygen
