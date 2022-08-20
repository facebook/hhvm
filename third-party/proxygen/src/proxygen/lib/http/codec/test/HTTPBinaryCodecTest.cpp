/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HTTPBinaryCodec.h>
#include <quic/codec/QuicInteger.h>

#include <folly/String.h>
#include <folly/portability/GTest.h>

namespace proxygen::test {

class HTTPBinaryCodecForTest : public HTTPBinaryCodec {
 public:
  explicit HTTPBinaryCodecForTest(TransportDirection direction)
      : HTTPBinaryCodec{direction} {
  }
  ParseResult parseFramingIndicator(folly::io::Cursor& cursor,
                                    bool& request,
                                    bool& knownLength) {
    return HTTPBinaryCodec::parseFramingIndicator(cursor, request, knownLength);
  }

  ParseResult parseRequestControlData(folly::io::Cursor& cursor,
                                      size_t remaining,
                                      HTTPMessage& msg) {
    return HTTPBinaryCodec::parseRequestControlData(cursor, remaining, msg);
  }

  ParseResult parseResponseControlData(folly::io::Cursor& cursor,
                                       size_t remaining,
                                       HTTPMessage& msg) {
    return HTTPBinaryCodec::parseResponseControlData(cursor, remaining, msg);
  }

  ParseResult parseHeaders(folly::io::Cursor& cursor,
                           size_t remaining,
                           HeaderDecodeInfo& decodeInfo) {
    return HTTPBinaryCodec::parseHeaders(cursor, remaining, decodeInfo);
  }

  ParseResult parseContent(folly::io::Cursor& cursor,
                           size_t remaining,
                           HTTPMessage& msg) {
    return HTTPBinaryCodec::parseContent(cursor, remaining, msg);
  }

  folly::IOBuf& getMsgBody() {
    return *msgBody_;
  }
};

class HTTPBinaryCodecCallback : public HTTPCodec::Callback {
 public:
  HTTPBinaryCodecCallback() = default;

  void onMessageBegin(HTTPCodec::StreamID /*stream*/,
                      HTTPMessage* /*msg*/) override {
  }
  void onPushMessageBegin(HTTPCodec::StreamID /*stream*/,
                          HTTPCodec::StreamID /*assocStream*/,
                          HTTPMessage* /*msg*/) override {
  }
  void onHeadersComplete(HTTPCodec::StreamID /*stream*/,
                         std::unique_ptr<HTTPMessage> msg) override {
    msg_ = std::move(msg);
  }
  void onBody(HTTPCodec::StreamID /*stream*/,
              std::unique_ptr<folly::IOBuf> chain,
              uint16_t /*padding*/) override {
    if (chain) {
      body_ = chain->clone();
    }
  }
  void onChunkHeader(HTTPCodec::StreamID /*stream*/,
                     size_t /*length*/) override {
  }
  void onChunkComplete(HTTPCodec::StreamID /*stream*/) override {
  }
  void onTrailersComplete(HTTPCodec::StreamID /*stream*/,
                          std::unique_ptr<HTTPHeaders> trailers) override {
    trailers_ = std::move(trailers);
  }
  void onMessageComplete(HTTPCodec::StreamID /*stream*/,
                         bool /*upgrade*/) override {
  }
  void onError(HTTPCodec::StreamID /*stream*/,
               const HTTPException& error,
               bool /*newTxn*/) override {
    error_ = std::make_unique<HTTPException>(error);
  }

  std::unique_ptr<HTTPMessage> msg_;
  std::unique_ptr<folly::IOBuf> body_;
  std::unique_ptr<HTTPHeaders> trailers_;
  std::unique_ptr<HTTPException> error_;
};

class HTTPBinaryCodecTest : public ::testing::Test {
 protected:
  void SetUp() override {
    downstreamBinaryCodec_ = std::make_unique<HTTPBinaryCodecForTest>(
        TransportDirection::DOWNSTREAM);
  }

  void TearDown() override {
  }

  std::unique_ptr<HTTPBinaryCodecForTest> downstreamBinaryCodec_;
};

TEST_F(HTTPBinaryCodecTest, testParseFramingIndicatorSuccess) {
  // Test Known Length Request
  const std::vector<uint8_t> framingIndicatorKnownRequest{0x00};
  auto framingIndicatorIOBuf = folly::IOBuf::wrapBuffer(
      folly::ByteRange(framingIndicatorKnownRequest.data(),
                       framingIndicatorKnownRequest.size()));
  folly::io::Cursor cursor(framingIndicatorIOBuf.get());

  bool request = false;
  bool knownLength = false;
  EXPECT_EQ(downstreamBinaryCodec_
                ->parseFramingIndicator(cursor, request, knownLength)
                .value(),
            1);
  EXPECT_EQ(request, true);
  EXPECT_EQ(knownLength, true);

  // Test Indeterminate Length Response
  const std::vector<uint8_t> framingIndicatorIndeterminateResponse{0x03};
  framingIndicatorIOBuf = folly::IOBuf::wrapBuffer(
      folly::ByteRange(framingIndicatorIndeterminateResponse.data(),
                       framingIndicatorIndeterminateResponse.size()));
  cursor = folly::io::Cursor(framingIndicatorIOBuf.get());

  EXPECT_EQ(downstreamBinaryCodec_
                ->parseFramingIndicator(cursor, request, knownLength)
                .error(),
            "Unsupported indeterminate length Binary HTTP Request");
  EXPECT_EQ(request, false);
  EXPECT_EQ(knownLength, false);
}

TEST_F(HTTPBinaryCodecTest, testParseFramingIndicatorFailure) {
  // Test Invalid Framing Indicator
  const std::vector<uint8_t> framingIndicatorInvalidResponse{0x04};
  auto framingIndicatorIOBuf = folly::IOBuf::wrapBuffer(
      folly::ByteRange(framingIndicatorInvalidResponse.data(),
                       framingIndicatorInvalidResponse.size()));
  folly::io::Cursor cursor(framingIndicatorIOBuf.get());

  bool request = false;
  bool knownLength = false;
  EXPECT_EQ(downstreamBinaryCodec_
                ->parseFramingIndicator(cursor, request, knownLength)
                .error(),
            "Invalid Framing Indicator: 4");
}

TEST_F(HTTPBinaryCodecTest, testParseRequestControlDataSuccess) {
  // Format is `.GET.https.www.example.com./hello.txt` where `.` represents the
  // length of each subsequent string
  const std::vector<uint8_t> controlDataRequest{
      0x03, 0x47, 0x45, 0x54, 0x05, 0x68, 0x74, 0x74, 0x70, 0x73,
      0x0f, 0x77, 0x77, 0x77, 0x2e, 0x65, 0x78, 0x61, 0x6d, 0x70,
      0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x0a, 0x2f, 0x68, 0x65,
      0x6c, 0x6c, 0x6f, 0x2e, 0x74, 0x78, 0x74};
  auto controlDataIOBuf = folly::IOBuf::wrapBuffer(
      folly::ByteRange(controlDataRequest.data(), controlDataRequest.size()));
  folly::io::Cursor cursor(controlDataIOBuf.get());

  HTTPMessage msg;
  EXPECT_EQ(
      downstreamBinaryCodec_
          ->parseRequestControlData(cursor, controlDataRequest.size(), msg)
          .value(),
      controlDataRequest.size());
  EXPECT_EQ(msg.isSecure(), true);
  EXPECT_EQ(msg.getMethod(), proxygen::HTTPMethod::GET);
  EXPECT_EQ(msg.getURL(), "/hello.txt");
}

TEST_F(HTTPBinaryCodecTest, testParseRequestControlDataFailure) {
  // Format is `.GET.https.www.example.com./hello.txt` where `.` before
  // /hello.txt is value 11 instead of value 10, which should cause the parsing
  // to error
  const std::vector<uint8_t> controlDataInvalidRequest{
      0x03, 0x47, 0x45, 0x54, 0x05, 0x68, 0x74, 0x74, 0x70, 0x73,
      0x0f, 0x77, 0x77, 0x77, 0x2e, 0x65, 0x78, 0x61, 0x6d, 0x70,
      0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x0b, 0x2f, 0x68, 0x65,
      0x6c, 0x6c, 0x6f, 0x2e, 0x74, 0x78, 0x74};
  auto controlDataIOBuf = folly::IOBuf::wrapBuffer(folly::ByteRange(
      controlDataInvalidRequest.data(), controlDataInvalidRequest.size()));
  folly::io::Cursor cursor(controlDataIOBuf.get());

  HTTPMessage msg;
  EXPECT_EQ(downstreamBinaryCodec_
                ->parseRequestControlData(
                    cursor, controlDataInvalidRequest.size(), msg)
                .error(),
            "Failure to parse: path");

  // Format is `.GET.httpt.www.example.com./hello.txt` where `.` represents the
  // length of each subsequent string.
  const std::vector<uint8_t> controlDataInvalidScheme{
      0x03, 0x47, 0x45, 0x54, 0x05, 0x68, 0x74, 0x74, 0x70, 0x74,
      0x0f, 0x77, 0x77, 0x77, 0x2e, 0x65, 0x78, 0x61, 0x6d, 0x70,
      0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x0a, 0x2f, 0x68, 0x65,
      0x6c, 0x6c, 0x6f, 0x2e, 0x74, 0x78, 0x74};
  controlDataIOBuf = folly::IOBuf::wrapBuffer(folly::ByteRange(
      controlDataInvalidScheme.data(), controlDataInvalidScheme.size()));
  cursor = folly::io::Cursor(controlDataIOBuf.get());

  EXPECT_EQ(downstreamBinaryCodec_
                ->parseRequestControlData(
                    cursor, controlDataInvalidScheme.size(), msg)
                .error(),
            "Failure to parse: scheme. Should be 'http' or 'https'");

  // Format is `.GET.https.www.example.com.hello.tx[\x1]` where `.` represents
  // the length of each subsequent string.
  const std::vector<uint8_t> controlDataInvalidPath{
      0x03, 0x47, 0x45, 0x54, 0x05, 0x68, 0x74, 0x74, 0x70, 0x73, 0x0f, 0x77,
      0x77, 0x77, 0x2e, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63,
      0x6f, 0x6d, 0x09, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x2e, 0x74, 0x78, 0x01};
  controlDataIOBuf = folly::IOBuf::wrapBuffer(folly::ByteRange(
      controlDataInvalidPath.data(), controlDataInvalidPath.size()));
  cursor = folly::io::Cursor(controlDataIOBuf.get());

  EXPECT_EQ(
      downstreamBinaryCodec_
          ->parseRequestControlData(cursor, controlDataInvalidPath.size(), msg)
          .error(),
      "Failure to parse: invalid URL path 'hello.tx\x1'");
}

TEST_F(HTTPBinaryCodecTest, testParseResponseControlDataSuccess) {
  // Reponse Code 200 OK
  folly::IOBufQueue controlDataIOBuf;
  folly::io::QueueAppender appender(&controlDataIOBuf, 0);
  auto parsedBytes = quic::encodeQuicInteger(200, [&appender](auto val) {
                       appender.writeBE(val);
                     }).value();
  folly::io::Cursor cursor(controlDataIOBuf.front());

  HTTPMessage msg;
  EXPECT_EQ(
      downstreamBinaryCodec_->parseResponseControlData(cursor, parsedBytes, msg)
          .value(),
      parsedBytes);
  EXPECT_EQ(msg.getStatusCode(), 200);
}

TEST_F(HTTPBinaryCodecTest, testParseResponseControlDataFailure) {
  // Invalid Status Code
  folly::IOBufQueue controlInvalidDataIOBuf;
  folly::io::QueueAppender appender(&controlInvalidDataIOBuf, 0);
  auto parsedBytes = quic::encodeQuicInteger(600, [&appender](auto val) {
                       appender.writeBE(val);
                     }).value();
  folly::io::Cursor cursor(controlInvalidDataIOBuf.front());

  HTTPMessage msg;
  EXPECT_EQ(
      downstreamBinaryCodec_->parseResponseControlData(cursor, parsedBytes, msg)
          .error(),
      "Invalid response status code: 600");

  folly::IOBufQueue controlInvalidDataInformationalResponseIOBuf;
  appender = folly::io::QueueAppender(
      &controlInvalidDataInformationalResponseIOBuf, 0);
  parsedBytes = quic::encodeQuicInteger(101, [&appender](auto val) {
                  appender.writeBE(val);
                }).value();
  cursor =
      folly::io::Cursor(controlInvalidDataInformationalResponseIOBuf.front());

  EXPECT_EQ(
      downstreamBinaryCodec_->parseResponseControlData(cursor, parsedBytes, msg)
          .error(),
      "Invalid response status code: 101");
}

TEST_F(HTTPBinaryCodecTest, testParseHeadersSuccess) {
  // Format is `..user-agent.curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l
  // zlib/1.2.3.host.www.example.com.accept-language.en, mi` where `.`
  // represents the length of the each subsequent string. The first `.` is
  // actually a Quic Integer that takes 2 bytes (and encodes the length of the
  // overall header as 108)
  const std::vector<uint8_t> headers{
      0x40, 0x6c, 0x0a, 0x75, 0x73, 0x65, 0x72, 0x2d, 0x61, 0x67, 0x65,
      0x6e, 0x74, 0x34, 0x63, 0x75, 0x72, 0x6c, 0x2f, 0x37, 0x2e, 0x31,
      0x36, 0x2e, 0x33, 0x20, 0x6c, 0x69, 0x62, 0x63, 0x75, 0x72, 0x6c,
      0x2f, 0x37, 0x2e, 0x31, 0x36, 0x2e, 0x33, 0x20, 0x4f, 0x70, 0x65,
      0x6e, 0x53, 0x53, 0x4c, 0x2f, 0x30, 0x2e, 0x39, 0x2e, 0x37, 0x6c,
      0x20, 0x7a, 0x6c, 0x69, 0x62, 0x2f, 0x31, 0x2e, 0x32, 0x2e, 0x33,
      0x04, 0x68, 0x6f, 0x73, 0x74, 0x0f, 0x77, 0x77, 0x77, 0x2e, 0x65,
      0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x0f,
      0x61, 0x63, 0x63, 0x65, 0x70, 0x74, 0x2d, 0x6c, 0x61, 0x6e, 0x67,
      0x75, 0x61, 0x67, 0x65, 0x06, 0x65, 0x6e, 0x2c, 0x20, 0x6d, 0x69};
  auto headersIOBuf = folly::IOBuf::wrapBuffer(
      folly::ByteRange(headers.data(), headers.size()));
  folly::io::Cursor cursor(headersIOBuf.get());

  HeaderDecodeInfo decodeInfo;
  decodeInfo.init(true /* request */,
                  false /* isRequestTrailers */,
                  true /* validate */,
                  false /* strictValidation */,
                  false /* allowEmptyPath */);
  EXPECT_EQ(
      downstreamBinaryCodec_->parseHeaders(cursor, headers.size(), decodeInfo)
          .value(),
      headers.size());

  HTTPHeaders httpHeaders = decodeInfo.msg->getHeaders();
  EXPECT_EQ(httpHeaders.exists("user-agent"), true);
  EXPECT_EQ(httpHeaders.exists("host"), true);
  EXPECT_EQ(httpHeaders.exists("accept-language"), true);
  EXPECT_EQ(httpHeaders.getSingleOrEmpty("user-agent"),
            "curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3");
  EXPECT_EQ(httpHeaders.getSingleOrEmpty("host"), "www.example.com");
  EXPECT_EQ(httpHeaders.getSingleOrEmpty("accept-language"), "en, mi");
}

TEST_F(HTTPBinaryCodecTest, testParseHeadersFailure) {
  // Number of headers should be >= 1
  const std::vector<uint8_t> invalidHeadersCount{0x00};
  auto headersIOBuf = folly::IOBuf::wrapBuffer(
      folly::ByteRange(invalidHeadersCount.data(), invalidHeadersCount.size()));
  folly::io::Cursor cursor(headersIOBuf.get());

  HeaderDecodeInfo decodeInfo;
  decodeInfo.init(true /* request */,
                  false /* isRequestTrailers */,
                  true /* validate */,
                  false /* strictValidation */,
                  false /* allowEmptyPath */);
  EXPECT_EQ(
      downstreamBinaryCodec_
          ->parseHeaders(cursor, invalidHeadersCount.size(), decodeInfo)
          .error(),
      "Number of headers (key value pairs) should be >= 1. Header count is 0");

  // Format is `..user-agent.curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l
  // zlib/1.2.3.host.www.example.com.accept-language.en, mi` where the `.` after
  // accept-language is value 7 instead of 6 which should cause parsing to fail
  const std::vector<uint8_t> invalidHeadersLength{
      0x40, 0x6c, 0x0a, 0x75, 0x73, 0x65, 0x72, 0x2d, 0x61, 0x67, 0x65,
      0x6e, 0x74, 0x34, 0x63, 0x75, 0x72, 0x6c, 0x2f, 0x37, 0x2e, 0x31,
      0x36, 0x2e, 0x33, 0x20, 0x6c, 0x69, 0x62, 0x63, 0x75, 0x72, 0x6c,
      0x2f, 0x37, 0x2e, 0x31, 0x36, 0x2e, 0x33, 0x20, 0x4f, 0x70, 0x65,
      0x6e, 0x53, 0x53, 0x4c, 0x2f, 0x30, 0x2e, 0x39, 0x2e, 0x37, 0x6c,
      0x20, 0x7a, 0x6c, 0x69, 0x62, 0x2f, 0x31, 0x2e, 0x32, 0x2e, 0x33,
      0x04, 0x68, 0x6f, 0x73, 0x74, 0x0f, 0x77, 0x77, 0x77, 0x2e, 0x65,
      0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x0f,
      0x61, 0x63, 0x63, 0x65, 0x70, 0x74, 0x2d, 0x6c, 0x61, 0x6e, 0x67,
      0x75, 0x61, 0x67, 0x65, 0x07, 0x65, 0x6e, 0x2c, 0x20, 0x6d, 0x69};
  headersIOBuf = folly::IOBuf::wrapBuffer(folly::ByteRange(
      invalidHeadersLength.data(), invalidHeadersLength.size()));
  cursor = folly::io::Cursor(headersIOBuf.get());

  EXPECT_EQ(downstreamBinaryCodec_
                ->parseHeaders(cursor, invalidHeadersLength.size(), decodeInfo)
                .error(),
            "Failure to parse: headerValue");

  // Format is `..a.b` where the first `.` represents a too long length
  const std::vector<uint8_t> invalidHeadersUnderflow{
      0x09, 0x01, 0x61, 0x01, 0x62};
  headersIOBuf = folly::IOBuf::wrapBuffer(folly::ByteRange(
      invalidHeadersUnderflow.data(), invalidHeadersUnderflow.size()));
  cursor = folly::io::Cursor(headersIOBuf.get());

  EXPECT_EQ(
      downstreamBinaryCodec_
          ->parseHeaders(cursor, invalidHeadersUnderflow.size(), decodeInfo)
          .error(),
      "Header parsing underflowed! Headers length in bytes (9) is "
      "inconsistent with remaining buffer length (4)");

  // Format is `.` where the first `.` represents an underflowed quic integer
  const std::vector<uint8_t> invalidHeadersUnderflowQuic{0x99};
  headersIOBuf = folly::IOBuf::wrapBuffer(folly::ByteRange(
      invalidHeadersUnderflowQuic.data(), invalidHeadersUnderflowQuic.size()));
  cursor = folly::io::Cursor(headersIOBuf.get());

  EXPECT_EQ(
      downstreamBinaryCodec_
          ->parseHeaders(cursor, invalidHeadersUnderflowQuic.size(), decodeInfo)
          .error(),
      "Failure to parse number of headers");
}

TEST_F(HTTPBinaryCodecTest, testParseContentSuccess) {
  // Format is `.hello`
  const std::vector<uint8_t> content{
      0x07, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x0d, 0x0a};
  auto contentIOBuf = folly::IOBuf::wrapBuffer(
      folly::ByteRange(content.data(), content.size()));
  folly::io::Cursor cursor(contentIOBuf.get());

  HTTPMessage msg;
  EXPECT_EQ(
      downstreamBinaryCodec_->parseContent(cursor, content.size(), msg).value(),
      content.size());
  EXPECT_EQ(downstreamBinaryCodec_->getMsgBody().moveToFbString().toStdString(),
            "hello\r\n");
}

TEST_F(HTTPBinaryCodecTest, testParseContentFailure) {
  // Format is `.hello` where . is value 8 instead of 7
  const std::vector<uint8_t> contentInvalid{
      0x08, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x0d, 0x0a};
  auto contentIOBuf = folly::IOBuf::wrapBuffer(
      folly::ByteRange(contentInvalid.data(), contentInvalid.size()));
  folly::io::Cursor cursor(contentIOBuf.get());

  HTTPMessage msg;
  EXPECT_EQ(
      downstreamBinaryCodec_->parseContent(cursor, contentInvalid.size(), msg)
          .error(),
      "Failure to parse content");
}

TEST_F(HTTPBinaryCodecTest, testOnIngressSuccess) {
  // Format is `..GET.https.www.example.com./hello.txt..user-agent.curl/7.16.3
  // libcurl/7.16.3 OpenSSL/0.9.7l
  // zlib/1.2.3.host.www.example.com.accept-language.en, mi`
  const std::vector<uint8_t> binaryHTTPMessage{
      0x00, 0x03, 0x47, 0x45, 0x54, 0x05, 0x68, 0x74, 0x74, 0x70, 0x73, 0x0f,
      0x77, 0x77, 0x77, 0x2e, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e,
      0x63, 0x6f, 0x6d, 0x0a, 0x2f, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x2e, 0x74,
      0x78, 0x74, 0x40, 0x6c, 0x0a, 0x75, 0x73, 0x65, 0x72, 0x2d, 0x61, 0x67,
      0x65, 0x6e, 0x74, 0x34, 0x63, 0x75, 0x72, 0x6c, 0x2f, 0x37, 0x2e, 0x31,
      0x36, 0x2e, 0x33, 0x20, 0x6c, 0x69, 0x62, 0x63, 0x75, 0x72, 0x6c, 0x2f,
      0x37, 0x2e, 0x31, 0x36, 0x2e, 0x33, 0x20, 0x4f, 0x70, 0x65, 0x6e, 0x53,
      0x53, 0x4c, 0x2f, 0x30, 0x2e, 0x39, 0x2e, 0x37, 0x6c, 0x20, 0x7a, 0x6c,
      0x69, 0x62, 0x2f, 0x31, 0x2e, 0x32, 0x2e, 0x33, 0x04, 0x68, 0x6f, 0x73,
      0x74, 0x0f, 0x77, 0x77, 0x77, 0x2e, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c,
      0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x0f, 0x61, 0x63, 0x63, 0x65, 0x70, 0x74,
      0x2d, 0x6c, 0x61, 0x6e, 0x67, 0x75, 0x61, 0x67, 0x65, 0x06, 0x65, 0x6e,
      0x2c, 0x20, 0x6d, 0x69, 0x00, 0x00};
  auto binaryHTTPMessageIOBuf = folly::IOBuf::wrapBuffer(
      folly::ByteRange(binaryHTTPMessage.data(), binaryHTTPMessage.size()));
  folly::io::Cursor cursor(binaryHTTPMessageIOBuf.get());

  HTTPBinaryCodecCallback callback;
  downstreamBinaryCodec_->setCallback(&callback);
  downstreamBinaryCodec_->onIngress(*binaryHTTPMessageIOBuf);
  downstreamBinaryCodec_->onIngressEOF();

  // Check onError was not called for the callback
  EXPECT_EQ(callback.error_, nullptr);

  // Check msg and header fields
  EXPECT_EQ(callback.msg_->isSecure(), true);
  EXPECT_EQ(callback.msg_->getMethod(), proxygen::HTTPMethod::GET);
  EXPECT_EQ(callback.msg_->getURL(), "/hello.txt");
  HTTPHeaders httpHeaders = callback.msg_->getHeaders();
  EXPECT_EQ(httpHeaders.exists("user-agent"), true);
  EXPECT_EQ(httpHeaders.exists("host"), true);
  EXPECT_EQ(httpHeaders.exists("accept-language"), true);
  EXPECT_EQ(httpHeaders.getSingleOrEmpty("user-agent"),
            "curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3");
  EXPECT_EQ(httpHeaders.getSingleOrEmpty("host"), "www.example.com");
  EXPECT_EQ(httpHeaders.getSingleOrEmpty("accept-language"), "en, mi");
}

TEST_F(HTTPBinaryCodecTest, testOnIngressSuccessForControlData) {
  // Format is `..GET.https.www.example.com./`
  const std::vector<uint8_t> binaryHTTPMessage{
      0x00, 0x03, 0x47, 0x45, 0x54, 0x05, 0x68, 0x74, 0x74,
      0x70, 0x73, 0x0b, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c,
      0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x01, 0x2f};
  auto binaryHTTPMessageIOBuf = folly::IOBuf::wrapBuffer(
      folly::ByteRange(binaryHTTPMessage.data(), binaryHTTPMessage.size()));
  folly::io::Cursor cursor(binaryHTTPMessageIOBuf.get());

  HTTPBinaryCodecCallback callback;
  downstreamBinaryCodec_->setCallback(&callback);
  downstreamBinaryCodec_->onIngress(*binaryHTTPMessageIOBuf);
  downstreamBinaryCodec_->onIngressEOF();

  // Check onError was not called for the callback
  EXPECT_EQ(callback.error_, nullptr);

  // Check msg and header fields
  EXPECT_EQ(callback.msg_->isSecure(), true);
  EXPECT_EQ(callback.msg_->getMethod(), proxygen::HTTPMethod::GET);
  EXPECT_EQ(callback.msg_->getURL(), "/");
}

TEST_F(HTTPBinaryCodecTest, testOnIngressFailure) {
  // Format is `..GET.https.www.example.com./hello.txt..user-agent.curl/7.16.3
  // libcurl/7.16.3 OpenSSL/0.9.7l
  // zlib/1.2.3.host.www.example.com.accept-language.en, mi` where the last `.`
  // is value 10 instead of 6
  const std::vector<uint8_t> binaryInvalidHTTPMessage{
      0x00, 0x03, 0x47, 0x45, 0x54, 0x05, 0x68, 0x74, 0x74, 0x70, 0x73, 0x0f,
      0x77, 0x77, 0x77, 0x2e, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e,
      0x63, 0x6f, 0x6d, 0x0a, 0x2f, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x2e, 0x74,
      0x78, 0x74, 0x40, 0x6c, 0x0a, 0x75, 0x73, 0x65, 0x72, 0x2d, 0x61, 0x67,
      0x65, 0x6e, 0x74, 0x34, 0x63, 0x75, 0x72, 0x6c, 0x2f, 0x37, 0x2e, 0x31,
      0x36, 0x2e, 0x33, 0x20, 0x6c, 0x69, 0x62, 0x63, 0x75, 0x72, 0x6c, 0x2f,
      0x37, 0x2e, 0x31, 0x36, 0x2e, 0x33, 0x20, 0x4f, 0x70, 0x65, 0x6e, 0x53,
      0x53, 0x4c, 0x2f, 0x30, 0x2e, 0x39, 0x2e, 0x37, 0x6c, 0x20, 0x7a, 0x6c,
      0x69, 0x62, 0x2f, 0x31, 0x2e, 0x32, 0x2e, 0x33, 0x04, 0x68, 0x6f, 0x73,
      0x74, 0x0f, 0x77, 0x77, 0x77, 0x2e, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c,
      0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x0f, 0x61, 0x63, 0x63, 0x65, 0x70, 0x74,
      0x2d, 0x6c, 0x61, 0x6e, 0x67, 0x75, 0x61, 0x67, 0x65, 0x0a, 0x65, 0x6e,
      0x2c, 0x20, 0x6d, 0x69, 0x00, 0x00};
  auto binaryHTTPMessageIOBuf = folly::IOBuf::wrapBuffer(folly::ByteRange(
      binaryInvalidHTTPMessage.data(), binaryInvalidHTTPMessage.size()));
  folly::io::Cursor cursor(binaryHTTPMessageIOBuf.get());

  HTTPBinaryCodecCallback callback;
  downstreamBinaryCodec_->setCallback(&callback);
  downstreamBinaryCodec_->onIngress(*binaryHTTPMessageIOBuf);
  downstreamBinaryCodec_->onIngressEOF();

  // Check onError was called with the correct error
  EXPECT_EQ(std::string(callback.error_.get()->what()),
            "Invalid Message: Failure to parse: headerValue");
}

TEST_F(HTTPBinaryCodecTest, testGenerateHeaders) {
  // Create HTTPMessage and encode it to a buffer
  HTTPMessage msgEncoded;
  msgEncoded.setMethod("GET");
  msgEncoded.setSecure(true);
  msgEncoded.setURL("/hello.txt");
  HTTPHeaders& headersEncoded = msgEncoded.getHeaders();
  headersEncoded.set("user-agent",
                     "curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3");
  headersEncoded.set("host", "www.example.com");
  headersEncoded.set("accept-language", "en, mi");

  folly::IOBufQueue writeBuffer;
  downstreamBinaryCodec_->generateHeader(writeBuffer, 0, msgEncoded);

  // Now, decode the HTTPMessage from the buffer and check values
  HTTPBinaryCodecCallback callback;
  downstreamBinaryCodec_->setCallback(&callback);
  downstreamBinaryCodec_->onIngress(*writeBuffer.front());
  downstreamBinaryCodec_->onIngressEOF();

  EXPECT_EQ(callback.msg_->getMethod(), msgEncoded.getMethod());
  EXPECT_EQ(callback.msg_->isSecure(), msgEncoded.isSecure());
  EXPECT_EQ(callback.msg_->getURL(), msgEncoded.getURL());
  auto headersDecoded = callback.msg_->getHeaders();
  EXPECT_EQ(headersDecoded.size(), headersEncoded.size());
  headersEncoded.forEach([&headersDecoded](const std::string& headerName,
                                           const std::string& headerValue) {
    EXPECT_EQ(headersDecoded.exists(headerName), true);
    EXPECT_EQ(headersDecoded.getSingleOrEmpty(headerName), headerValue);
  });
}

TEST_F(HTTPBinaryCodecTest, testGenerateBody) {
  // Create Test Body and encode
  std::string body = "Sample Test Body!";
  std::unique_ptr<folly::IOBuf> testBody =
      folly::IOBuf::wrapBuffer(body.data(), body.size());

  folly::IOBufQueue writeBuffer;
  downstreamBinaryCodec_->generateBody(writeBuffer, 0, std::move(testBody));

  // Decode Test Body and check
  folly::io::Cursor cursor(writeBuffer.front());
  HTTPMessage msg;
  EXPECT_EQ(downstreamBinaryCodec_->parseContent(cursor, 18, msg).value(), 18);
  EXPECT_EQ(downstreamBinaryCodec_->getMsgBody().moveToFbString().toStdString(),
            "Sample Test Body!");
}

TEST_F(HTTPBinaryCodecTest, testEncodeAndDecodeRequest) {
  // Create full request encode it to a buffer
  folly::IOBufQueue writeBuffer;

  // Encode Framing Indicator, Control Data, and Headers
  HTTPMessage msgEncoded;
  msgEncoded.setMethod("POST");
  msgEncoded.setSecure(false);
  msgEncoded.setURL("/hello.txt");
  HTTPHeaders& headersEncoded = msgEncoded.getHeaders();
  headersEncoded.set("user-agent",
                     "curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3");
  headersEncoded.set("host", "www.example.com");
  headersEncoded.set("accept-language", "en, mi");
  downstreamBinaryCodec_->generateHeader(writeBuffer, 0, msgEncoded);

  // Encode Body
  std::string body = "Sample Test Body!";
  std::unique_ptr<folly::IOBuf> testBody =
      folly::IOBuf::wrapBuffer(body.data(), body.size());
  downstreamBinaryCodec_->generateBody(writeBuffer, 0, std::move(testBody));

  // Encode Trailing Headers
  std::unique_ptr<HTTPHeaders> trailersEncoded =
      std::make_unique<HTTPHeaders>();
  trailersEncoded->set("test-trailer", "test-trailer-value");
  msgEncoded.setTrailers(std::move(trailersEncoded));
  downstreamBinaryCodec_->generateTrailers(
      writeBuffer, 0, *msgEncoded.getTrailers());

  // Now, decode the request and check values
  HTTPBinaryCodecCallback callback;
  downstreamBinaryCodec_->setCallback(&callback);
  downstreamBinaryCodec_->onIngress(*writeBuffer.front());
  downstreamBinaryCodec_->onIngressEOF();

  EXPECT_EQ(callback.msg_->getMethod(), msgEncoded.getMethod());
  EXPECT_EQ(callback.msg_->isSecure(), msgEncoded.isSecure());
  EXPECT_EQ(callback.msg_->getURL(), msgEncoded.getURL());
  auto headersDecoded = callback.msg_->getHeaders();
  EXPECT_EQ(headersDecoded.size(), headersEncoded.size());
  headersEncoded.forEach([&headersDecoded](const std::string& headerName,
                                           const std::string& headerValue) {
    EXPECT_EQ(headersDecoded.exists(headerName), true);
    EXPECT_EQ(headersDecoded.getSingleOrEmpty(headerName), headerValue);
  });

  EXPECT_EQ(callback.body_->moveToFbString().toStdString(),
            "Sample Test Body!");

  auto trailersDecoded = *callback.trailers_;
  EXPECT_EQ(trailersDecoded.size(), 1);
  EXPECT_EQ(trailersDecoded.exists("test-trailer"), true);
  EXPECT_EQ(trailersDecoded.getSingleOrEmpty("test-trailer"),
            "test-trailer-value");
}

} // namespace proxygen::test
