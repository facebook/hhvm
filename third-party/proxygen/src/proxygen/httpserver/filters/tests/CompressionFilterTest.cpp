/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <type_traits>

#include <folly/Conv.h>
#include <folly/ScopeGuard.h>
#include <folly/io/IOBuf.h>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <proxygen/httpserver/Mocks.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <proxygen/httpserver/filters/CompressionFilter.h>
#include <proxygen/lib/utils/ZlibStreamCompressor.h>
#include <proxygen/lib/utils/ZstdStreamDecompressor.h>

using namespace proxygen;
using namespace testing;

MATCHER_P(IOBufEquals,
          expected,
          folly::to<std::string>(
              "IOBuf is ", negation ? "not " : "", "'", expected, "'")) {
  auto iob = arg->clone();
  auto br = iob->coalesce();
  std::string actual(br.begin(), br.end());
  *result_listener << "'" << actual << "'";
  return actual == expected;
}

struct ZlibTest {
  static std::unique_ptr<StreamDecompressor> makeDecompressor() {
    return std::make_unique<ZlibStreamDecompressor>(CompressionType::GZIP);
  }
  static std::string getExpectedEncoding() {
    return "gzip";
  }
  static int32_t getCompressionLevel() {
    return 4 /* default */;
  }
};

struct ZstdTest {
  static std::unique_ptr<StreamDecompressor> makeDecompressor() {
    return std::make_unique<ZstdStreamDecompressor>();
  }
  static std::string getExpectedEncoding() {
    return "zstd";
  }
  static int32_t getCompressionLevel() {
    return 4 /* default */;
  }
};

template <typename T>
class CompressionFilterTest : public Test {
 public:
  using CodecType = T;

  void SetUp() override {
    // requesthandler is the server, responsehandler is the client
    requestHandler_ = new MockRequestHandler();
    responseHandler_ = std::make_unique<MockResponseHandler>(requestHandler_);
    zd_ = T::makeDecompressor();
  }

  void TearDown() override {
    Mock::VerifyAndClear(requestHandler_);
    Mock::VerifyAndClear(responseHandler_.get());

    delete requestHandler_;
  }

 protected:
  CompressionFilter* filter_{nullptr};
  MockRequestHandler* requestHandler_;
  std::unique_ptr<MockResponseHandler> responseHandler_;
  std::unique_ptr<StreamDecompressor> zd_;
  ResponseHandler* downstream_{nullptr};

  void exercise_compression(bool expectCompression,
                            std::string url,
                            std::string acceptedEncoding,
                            std::string expectedEncoding,
                            std::string originalRequestBody,
                            std::string responseContentType,
                            std::unique_ptr<folly::IOBuf> originalResponseBody,
                            int32_t compressionLevel = T::getCompressionLevel(),
                            uint32_t minimumCompressionSize = 1,
                            bool sendCompressedResponse = false,
                            bool disableCompressionForThisEncoding = false) {

    // If there is only one IOBuf, then it's not chunked.
    bool isResponseChunked = originalResponseBody->isChained();
    size_t chunkCount = originalResponseBody->countChainElements();

    // Chunked and compressed responses will have an extra block
    if (isResponseChunked && expectCompression) {
      chunkCount += 1;
    }

    // Request Handler Expectations
    EXPECT_CALL(*requestHandler_, onBody(_)).Times(1);
    EXPECT_CALL(*requestHandler_, onEOM()).Times(1);

    // Need to capture whatever the filter is for ResponseBuilder later
    EXPECT_CALL(*requestHandler_, setResponseHandler(_))
        .WillOnce(DoAll(SaveArg<0>(&downstream_), Return()));

    // Response Handler Expectations
    // Headers are only sent once
    EXPECT_CALL(*responseHandler_, sendHeaders(_))
        .WillOnce(DoAll(Invoke([&](HTTPMessage& msg) {
                          auto& headers = msg.getHeaders();
                          if (expectCompression) {
                            EXPECT_TRUE(msg.checkForHeaderToken(
                                HTTP_HEADER_CONTENT_ENCODING,
                                expectedEncoding.c_str(),
                                false));
                          }

                          if (msg.getIsChunked()) {
                            EXPECT_FALSE(headers.exists("Content-Length"));
                          } else {
                            // Content-Length is not set on chunked messages
                            EXPECT_TRUE(headers.exists("Content-Length"));
                          }
                        }),
                        Return()));

    if (isResponseChunked) {
      // The final chunk has 0 body
      EXPECT_CALL(*responseHandler_, sendChunkHeader(_)).Times(chunkCount);
      EXPECT_CALL(*responseHandler_, sendChunkTerminator()).Times(chunkCount);
    } else {
      EXPECT_CALL(*responseHandler_, sendChunkHeader(_)).Times(0);
      EXPECT_CALL(*responseHandler_, sendChunkTerminator()).Times(0);
    }

    // Accumulate the body, decompressing it if it's compressed
    std::unique_ptr<folly::IOBuf> responseBody;
    EXPECT_CALL(*responseHandler_, sendBody(_))
        .Times(chunkCount)
        .WillRepeatedly(DoAll(
            Invoke([&](std::shared_ptr<folly::IOBuf> body) {
              std::unique_ptr<folly::IOBuf> processedBody;

              if (expectCompression) {
                processedBody = zd_->decompress(body.get());
                ASSERT_FALSE(zd_->hasError()) << "Failed to decompress body!";
              } else {
                processedBody = folly::IOBuf::copyBuffer(
                    body->data(), body->length(), 0, 0);
              }

              if (responseBody) {
                responseBody->prependChain(std::move(processedBody));
              } else {
                responseBody = std::move(processedBody);
              }
            }),
            Return()));

    EXPECT_CALL(*responseHandler_, sendEOM()).Times(1);

    /* Simulate Request/Response  */

    HTTPMessage msg;
    msg.setURL(url);
    msg.getHeaders().set(HTTP_HEADER_ACCEPT_ENCODING, acceptedEncoding);

    std::set<std::string> compressibleTypes = {"text/html"};

    CompressionFilterFactory::Options opts;
    opts.zlibCompressionLevel = compressionLevel;
    opts.minimumCompressionSize = minimumCompressionSize;
    opts.compressibleContentTypes = compressibleTypes;
    opts.enableZstd = true;
    if (disableCompressionForThisEncoding) {
      if (CodecType::getExpectedEncoding() == "gzip") {
        opts.enableGzip = false;
      }
      if (CodecType::getExpectedEncoding() == "zstd") {
        opts.enableZstd = false;
      }
    }
    auto filterFactory = std::make_unique<CompressionFilterFactory>(opts);

    auto filter = filterFactory->onRequest(requestHandler_, &msg);
    filter->setResponseHandler(responseHandler_.get());

    // Send fake request
    filter->onBody(folly::IOBuf::copyBuffer(originalRequestBody));
    filter->onEOM();

    // Send a fake Response
    if (isResponseChunked) {

      ResponseBuilder(downstream_)
          .status(200, "OK")
          .header(HTTP_HEADER_CONTENT_TYPE, responseContentType)
          .send();

      folly::IOBuf* crtBuf;
      crtBuf = originalResponseBody.get();

      do {
        ResponseBuilder(downstream_).body(crtBuf->cloneOne()).send();
        crtBuf = crtBuf->next();
      } while (crtBuf != originalResponseBody.get());

      ResponseBuilder(downstream_).sendWithEOM();

    } else if (sendCompressedResponse) {
      // Send unchunked response
      ResponseBuilder(downstream_)
          .status(200, "OK")
          .header(HTTP_HEADER_CONTENT_TYPE, responseContentType)
          .header(HTTP_HEADER_CONTENT_ENCODING, "gzip")
          .body(originalResponseBody->clone())
          .sendWithEOM();
    } else {
      // Send unchunked response
      ResponseBuilder(downstream_)
          .status(200, "OK")
          .header(HTTP_HEADER_CONTENT_TYPE, responseContentType)
          .body(originalResponseBody->clone())
          .sendWithEOM();
    }

    filter->requestComplete();

    EXPECT_THAT(responseBody, IOBufEquals(originalRequestBody));
  }

  // Helper method to convert a vector of strings to an IOBuf chain
  // specificaly create a chain because the chain pieces are chunks
  std::unique_ptr<folly::IOBuf> createResponseChain(
      std::vector<std::string> const& bodyStrings) {

    std::unique_ptr<folly::IOBuf> responseBodyChain;

    for (auto& s : bodyStrings) {
      auto nextBody = folly::IOBuf::copyBuffer(s.c_str());
      if (responseBodyChain) {
        responseBodyChain->prependChain(std::move(nextBody));
      } else {
        responseBodyChain = std::move(nextBody);
      }
    }

    return responseBodyChain;
  }
};

typedef ::testing::Types<ZlibTest, ZstdTest> CompressionCodecs;

TYPED_TEST_SUITE(CompressionFilterTest, CompressionCodecs);

// Basic smoke test
TYPED_TEST(CompressionFilterTest, NonchunkedCompression) {
  using Codec = typename TestFixture::CodecType;
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(true,
                               std::string("http://locahost/foo.compressme"),
                               Codec::getExpectedEncoding(),
                               Codec::getExpectedEncoding(),
                               std::string("Hello World"),
                               std::string("text/html"),
                               folly::IOBuf::copyBuffer("Hello World"));
  });
}

TYPED_TEST(CompressionFilterTest, ChunkedCompression) {
  using Codec = typename TestFixture::CodecType;
  std::vector<std::string> chunks = {"Hello", " World"};
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(true,
                               std::string("http://locahost/foo.compressme"),
                               Codec::getExpectedEncoding(),
                               Codec::getExpectedEncoding(),
                               std::string("Hello World"),
                               std::string("text/html"),
                               this->createResponseChain(chunks));
  });
}

TYPED_TEST(CompressionFilterTest, ParameterizedContenttype) {
  using Codec = typename TestFixture::CodecType;
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(true,
                               std::string("http://locahost/foo.compressme"),
                               Codec::getExpectedEncoding(),
                               Codec::getExpectedEncoding(),
                               std::string("Hello World"),
                               std::string("text/html; param1"),
                               folly::IOBuf::copyBuffer("Hello World"));
  });
}

TYPED_TEST(CompressionFilterTest, MixedcaseContenttype) {
  using Codec = typename TestFixture::CodecType;
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(true,
                               std::string("http://locahost/foo.compressme"),
                               Codec::getExpectedEncoding(),
                               Codec::getExpectedEncoding(),
                               std::string("Hello World"),
                               std::string("Text/Html; param1"),
                               folly::IOBuf::copyBuffer("Hello World"));
  });
}

// Client supports multiple possible compression encodings
TYPED_TEST(CompressionFilterTest, MultipleAcceptedEncodings) {
  using Codec = typename TestFixture::CodecType;
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        true,
        std::string("http://locahost/foo.compressme"),
        Codec::getExpectedEncoding() + ", identity, deflate",
        Codec::getExpectedEncoding(),
        std::string("Hello World"),
        std::string("text/html"),
        folly::IOBuf::copyBuffer("Hello World"));
  });
}

// Server skips compressing if the response is already compressed
TYPED_TEST(CompressionFilterTest, ResponseAlreadyCompressedTest) {
  using Codec = typename TestFixture::CodecType;
  auto compressor =
      std::make_unique<ZlibStreamCompressor>(CompressionType::GZIP, 4);
  auto fakeCompressed = folly::IOBuf::copyBuffer("helloimsupposedlycompressed");
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(false,
                               std::string("http://locahost/foo.compressme"),
                               Codec::getExpectedEncoding(),
                               Codec::getExpectedEncoding(),
                               std::string("helloimsupposedlycompressed"),
                               std::string("text/html"),
                               std::move(fakeCompressed),
                               4,
                               1,
                               true /*SendCompressedResponse*/);
  });
}

TYPED_TEST(CompressionFilterTest, MultipleAcceptedEncodingsQvalues) {
  using Codec = typename TestFixture::CodecType;
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        true,
        std::string("http://locahost/foo.compressme"),
        Codec::getExpectedEncoding() + "; q=.7;, identity",
        Codec::getExpectedEncoding(),
        std::string("Hello World"),
        std::string("text/html"),
        folly::IOBuf::copyBuffer("Hello World"));
  });
}

TYPED_TEST(CompressionFilterTest, NoCompressibleAcceptedEncodings) {
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(false,
                               std::string("http://locahost/foo.compressme"),
                               std::string("identity; q=.7;"),
                               std::string(""),
                               std::string("Hello World"),
                               std::string("text/html"),
                               folly::IOBuf::copyBuffer("Hello World"));
  });
}

TYPED_TEST(CompressionFilterTest, MissingAcceptedEncodings) {
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(false,
                               std::string("http://locahost/foo.compressme"),
                               std::string(""),
                               std::string(""),
                               std::string("Hello World"),
                               std::string("text/html"),
                               folly::IOBuf::copyBuffer("Hello World"));
  });
}

// Content is of an-uncompressible content-type
TYPED_TEST(CompressionFilterTest, UncompressibleContenttype) {
  using Codec = typename TestFixture::CodecType;
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(false,
                               std::string("http://locahost/foo.nocompress"),
                               Codec::getExpectedEncoding(),
                               std::string(""),
                               std::string("Hello World"),
                               std::string("image/jpeg"),
                               folly::IOBuf::copyBuffer("Hello World"));
  });
}

TYPED_TEST(CompressionFilterTest, UncompressibleContenttypeParam) {
  using Codec = typename TestFixture::CodecType;
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(false,
                               std::string("http://locahost/foo.nocompress"),
                               Codec::getExpectedEncoding(),
                               std::string(""),
                               std::string("Hello World"),
                               std::string("application/jpeg; param1"),
                               folly::IOBuf::copyBuffer("Hello World"));
  });
}

// Content is under the minimum compression size
TYPED_TEST(CompressionFilterTest, TooSmallToCompress) {
  using Codec = typename TestFixture::CodecType;
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(false,
                               std::string("http://locahost/foo.smallfry"),
                               Codec::getExpectedEncoding(),
                               std::string(""),
                               std::string("Hello World"),
                               std::string("text/html"),
                               folly::IOBuf::copyBuffer("Hello World"),
                               Codec::getCompressionLevel(),
                               1000);
  });
}

TYPED_TEST(CompressionFilterTest, SmallChunksCompress) {
  // Expect this to compress despite being small because can't tell the content
  // length when we're chunked
  using Codec = typename TestFixture::CodecType;
  std::vector<std::string> chunks = {"Hello", " World"};
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(true,
                               std::string("http://locahost/foo.compressme"),
                               Codec::getExpectedEncoding(),
                               Codec::getExpectedEncoding(),
                               std::string("Hello World"),
                               std::string("text/html"),
                               this->createResponseChain(chunks),
                               Codec::getCompressionLevel(),
                               1000);
  });
}

TYPED_TEST(CompressionFilterTest, MinimumCompressSizeEqualToRequestSize) {
  using Codec = typename TestFixture::CodecType;
  auto requestBody = std::string("Hello World");
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(true,
                               std::string("http://locahost/foo.compressme"),
                               Codec::getExpectedEncoding(),
                               Codec::getExpectedEncoding(),
                               requestBody,
                               std::string("text/html"),
                               folly::IOBuf::copyBuffer(requestBody),
                               Codec::getCompressionLevel(),
                               requestBody.length());
  });
}

TYPED_TEST(CompressionFilterTest, CompressionDisabledForEncoding) {
  using Codec = typename TestFixture::CodecType;
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(false,
                               std::string("http://locahost/foo.compressme"),
                               Codec::getExpectedEncoding(),
                               Codec::getExpectedEncoding(),
                               std::string("Hello World"),
                               std::string("text/html"),
                               folly::IOBuf::copyBuffer("Hello World"),
                               1,
                               1,
                               false,
                               true);
  });
}

TYPED_TEST(CompressionFilterTest, NoResponseBody) {
  using Codec = typename TestFixture::CodecType;

  std::string acceptedEncoding = Codec::getExpectedEncoding();
  std::string expectedEncoding = Codec::getExpectedEncoding();
  std::string url = std::string("http://locahost/foo.compressme");
  std::string responseContentType = std::string("text/html");
  int32_t compressionLevel = Codec::getCompressionLevel();
  uint32_t minimumCompressionSize = 0;

  auto& requestHandler = this->requestHandler_;
  auto& downstream = this->downstream_;
  auto& responseHandler = this->responseHandler_;

  ASSERT_NO_FATAL_FAILURE({
    // Request Handler Expectations
    EXPECT_CALL(*requestHandler, onEOM()).Times(1);

    // Need to capture whatever the filter is for ResponseBuilder later
    EXPECT_CALL(*requestHandler, setResponseHandler(_))
        .WillOnce(DoAll(SaveArg<0>(&downstream), Return()));

    // Response Handler Expectations
    // Headers are only sent once
    EXPECT_CALL(*responseHandler, sendHeaders(_))
        .WillOnce(DoAll(Invoke([&](HTTPMessage& msg) {
                          auto& headers = msg.getHeaders();
                          EXPECT_TRUE(msg.checkForHeaderToken(
                              HTTP_HEADER_CONTENT_ENCODING,
                              expectedEncoding.c_str(),
                              false));
                          if (msg.getIsChunked()) {
                            EXPECT_FALSE(headers.exists("Content-Length"));
                          } else {
                            // Content-Length is not set on chunked messages
                            EXPECT_TRUE(headers.exists("Content-Length"));
                          }
                        }),
                        Return()));

    EXPECT_CALL(*responseHandler, sendEOM()).Times(1);

    /* Simulate Request/Response where no body message received */
    HTTPMessage msg;
    msg.setURL(url);
    msg.getHeaders().set(HTTP_HEADER_ACCEPT_ENCODING, acceptedEncoding);

    std::set<std::string> compressibleTypes = {"text/html"};

    CompressionFilterFactory::Options opts;
    auto& optCompressionLevel = Codec::getExpectedEncoding() == "gzip"
                                    ? opts.zlibCompressionLevel
                                    : opts.zstdCompressionLevel;
    optCompressionLevel = compressionLevel;
    opts.minimumCompressionSize = minimumCompressionSize;
    opts.compressibleContentTypes = compressibleTypes;
    opts.enableZstd = true;
    auto filterFactory = std::make_unique<CompressionFilterFactory>(opts);

    auto filter = filterFactory->onRequest(requestHandler, &msg);
    filter->setResponseHandler(responseHandler.get());

    // Send fake request
    filter->onEOM();

    ResponseBuilder(downstream)
        .status(200, "OK")
        .header(HTTP_HEADER_CONTENT_TYPE, responseContentType)
        .send();

    ResponseBuilder(downstream).sendWithEOM();

    filter->requestComplete();
  });
}
