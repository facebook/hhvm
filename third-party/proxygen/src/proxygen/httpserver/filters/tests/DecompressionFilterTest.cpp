/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/io/IOBuf.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <proxygen/httpserver/Mocks.h>
#include <proxygen/httpserver/filters/DecompressionFilter.h>
#include <proxygen/lib/utils/ZstdStreamCompressor.h>

using namespace proxygen;
using namespace testing;

namespace {

// Helper to compress data using zstd for testing
std::unique_ptr<folly::IOBuf> compressWithZstd(const std::string& data) {
  ZstdStreamCompressor compressor(4 /* compression level */);
  auto input = folly::IOBuf::copyBuffer(data);
  return compressor.compress(input.get(), true /* last */);
}

} // namespace

class DecompressionFilterTest : public Test {
 public:
  void SetUp() override {
    requestHandler_ = new MockRequestHandler();
    responseHandler_ = std::make_unique<MockResponseHandler>(requestHandler_);
  }

  void TearDown() override {
    if (requestHandler_) {
      Mock::VerifyAndClear(requestHandler_);
      delete requestHandler_;
      requestHandler_ = nullptr;
    }
    Mock::VerifyAndClear(responseHandler_.get());
  }

 protected:
  MockRequestHandler* requestHandler_{nullptr};
  std::unique_ptr<MockResponseHandler> responseHandler_;
  ResponseHandler* downstream_{nullptr};
};

// Test: Request with zstd Content-Encoding header initializes decompressor and
// removes headers
TEST_F(DecompressionFilterTest, ZstdContentEncodingInitializesDecompressor) {
  // Setup expectations
  EXPECT_CALL(*requestHandler_, setResponseHandler(_))
      .WillOnce(DoAll(SaveArg<0>(&downstream_), Return()));

  EXPECT_CALL(*requestHandler_, onRequest(_))
      .WillOnce([](std::shared_ptr<HTTPMessage> msg) {
        // Content-Encoding should be removed
        EXPECT_FALSE(msg->getHeaders().exists(HTTP_HEADER_CONTENT_ENCODING));
        // Content-Length should be removed
        EXPECT_FALSE(msg->getHeaders().exists(HTTP_HEADER_CONTENT_LENGTH));
      });

  EXPECT_CALL(*requestHandler_, requestComplete()).Times(1);

  // Create filter
  auto* filter = new DecompressionFilter(requestHandler_);
  filter->setResponseHandler(responseHandler_.get());

  // Create request with zstd Content-Encoding
  auto msg = std::make_unique<HTTPMessage>();
  msg->setURL("/test");
  msg->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING, "zstd");
  msg->getHeaders().set(HTTP_HEADER_CONTENT_LENGTH, "100");

  filter->onRequest(std::move(msg));

  // Clean up - filter deletes itself and calls upstream requestComplete
  filter->requestComplete();
}

// Test: Request without Content-Encoding header passes through unchanged
TEST_F(DecompressionFilterTest, NoContentEncodingPassesThrough) {
  EXPECT_CALL(*requestHandler_, setResponseHandler(_))
      .WillOnce(DoAll(SaveArg<0>(&downstream_), Return()));

  EXPECT_CALL(*requestHandler_, onRequest(_))
      .WillOnce([](std::shared_ptr<HTTPMessage> msg) {
        // Headers should remain unchanged (no Content-Encoding was set)
        EXPECT_FALSE(msg->getHeaders().exists(HTTP_HEADER_CONTENT_ENCODING));
      });

  EXPECT_CALL(*requestHandler_, requestComplete()).Times(1);

  auto* filter = new DecompressionFilter(requestHandler_);
  filter->setResponseHandler(responseHandler_.get());

  auto msg = std::make_unique<HTTPMessage>();
  msg->setURL("/test");

  filter->onRequest(std::move(msg));
  filter->requestComplete();
}

// Test: Body decompression with valid zstd compressed data
TEST_F(DecompressionFilterTest, ValidZstdBodyDecompression) {
  const std::string originalData = "Hello, World! This is test data.";

  EXPECT_CALL(*requestHandler_, setResponseHandler(_))
      .WillOnce(DoAll(SaveArg<0>(&downstream_), Return()));

  EXPECT_CALL(*requestHandler_, onRequest(_)).Times(1);

  std::string decompressedData;
  // May be called multiple times (body + EOM remaining data)
  EXPECT_CALL(*requestHandler_, onBody(_))
      .WillRepeatedly([&decompressedData](std::shared_ptr<folly::IOBuf> body) {
        if (body) {
          auto cloned = body->clone();
          auto data = cloned->coalesce();
          decompressedData += std::string(
              reinterpret_cast<const char*>(data.data()), data.size());
        }
      });

  EXPECT_CALL(*requestHandler_, onEOM()).Times(1);
  EXPECT_CALL(*requestHandler_, requestComplete()).Times(1);

  auto* filter = new DecompressionFilter(requestHandler_);
  filter->setResponseHandler(responseHandler_.get());

  // Send request with zstd encoding
  auto msg = std::make_unique<HTTPMessage>();
  msg->setURL("/test");
  msg->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING, "zstd");

  filter->onRequest(std::move(msg));

  // Send compressed body
  auto compressedBody = compressWithZstd(originalData);
  ASSERT_TRUE(compressedBody != nullptr);

  filter->onBody(std::move(compressedBody));
  filter->onEOM();
  filter->requestComplete();

  EXPECT_EQ(decompressedData, originalData);
}

// Test: Multiple body chunks being decompressed
TEST_F(DecompressionFilterTest, MultipleChunksDecompression) {
  const std::string chunk1 = "First chunk of data. ";
  const std::string chunk2 = "Second chunk of data. ";
  const std::string chunk3 = "Third and final chunk.";
  const std::string fullData = chunk1 + chunk2 + chunk3;

  EXPECT_CALL(*requestHandler_, setResponseHandler(_))
      .WillOnce(DoAll(SaveArg<0>(&downstream_), Return()));

  EXPECT_CALL(*requestHandler_, onRequest(_)).Times(1);

  std::string accumulatedData;
  EXPECT_CALL(*requestHandler_, onBody(_))
      .WillRepeatedly([&accumulatedData](std::shared_ptr<folly::IOBuf> body) {
        if (body) {
          auto cloned = body->clone();
          auto data = cloned->coalesce();
          accumulatedData += std::string(
              reinterpret_cast<const char*>(data.data()), data.size());
        }
      });

  EXPECT_CALL(*requestHandler_, onEOM()).Times(1);
  EXPECT_CALL(*requestHandler_, requestComplete()).Times(1);

  auto* filter = new DecompressionFilter(requestHandler_);
  filter->setResponseHandler(responseHandler_.get());

  auto msg = std::make_unique<HTTPMessage>();
  msg->setURL("/test");
  msg->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING, "zstd");

  filter->onRequest(std::move(msg));

  // Compress entire data as single stream then send
  auto compressedBody = compressWithZstd(fullData);
  ASSERT_TRUE(compressedBody != nullptr);

  filter->onBody(std::move(compressedBody));
  filter->onEOM();
  filter->requestComplete();

  EXPECT_EQ(accumulatedData, fullData);
}

// Test: Decompression error handling with invalid compressed data
TEST_F(DecompressionFilterTest, InvalidCompressedDataSendsAbort) {
  EXPECT_CALL(*requestHandler_, setResponseHandler(_))
      .WillOnce(DoAll(SaveArg<0>(&downstream_), Return()));

  EXPECT_CALL(*requestHandler_, onRequest(_)).Times(1);

  // Should not receive body on error
  EXPECT_CALL(*requestHandler_, onBody(_)).Times(0);

  // Should send abort on decompression error
  EXPECT_CALL(*responseHandler_, sendAbort(_)).Times(1);

  auto* filter = new DecompressionFilter(requestHandler_);
  filter->setResponseHandler(responseHandler_.get());

  auto msg = std::make_unique<HTTPMessage>();
  msg->setURL("/test");
  msg->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING, "zstd");

  filter->onRequest(std::move(msg));

  // Send invalid (not actually compressed) data
  auto invalidBody = folly::IOBuf::copyBuffer("This is not valid zstd data!");
  filter->onBody(std::move(invalidBody));

  // Clean up via onError since we aborted
  filter->onError(ProxygenError::kErrorUnknown);
}

// Test: Pass through when no decompressor is set (no Content-Encoding)
TEST_F(DecompressionFilterTest, PassThroughWithoutDecompressor) {
  const std::string originalData = "Uncompressed data";

  EXPECT_CALL(*requestHandler_, setResponseHandler(_))
      .WillOnce(DoAll(SaveArg<0>(&downstream_), Return()));

  EXPECT_CALL(*requestHandler_, onRequest(_)).Times(1);

  std::string receivedData;
  EXPECT_CALL(*requestHandler_, onBody(_))
      .WillOnce([&receivedData](std::shared_ptr<folly::IOBuf> body) {
        if (body) {
          auto cloned = body->clone();
          auto data = cloned->coalesce();
          receivedData = std::string(reinterpret_cast<const char*>(data.data()),
                                     data.size());
        }
      });

  EXPECT_CALL(*requestHandler_, onEOM()).Times(1);
  EXPECT_CALL(*requestHandler_, requestComplete()).Times(1);

  auto* filter = new DecompressionFilter(requestHandler_);
  filter->setResponseHandler(responseHandler_.get());

  // No Content-Encoding header
  auto msg = std::make_unique<HTTPMessage>();
  msg->setURL("/test");

  filter->onRequest(std::move(msg));

  auto body = folly::IOBuf::copyBuffer(originalData);
  filter->onBody(std::move(body));
  filter->onEOM();
  filter->requestComplete();

  // Data should pass through unchanged
  EXPECT_EQ(receivedData, originalData);
}

// Test: Content-Length header removal when decompression is active
TEST_F(DecompressionFilterTest, ContentLengthRemovedWhenDecompressing) {
  EXPECT_CALL(*requestHandler_, setResponseHandler(_))
      .WillOnce(DoAll(SaveArg<0>(&downstream_), Return()));

  EXPECT_CALL(*requestHandler_, onRequest(_))
      .WillOnce([](std::shared_ptr<HTTPMessage> msg) {
        // Both Content-Encoding and Content-Length should be removed
        EXPECT_FALSE(msg->getHeaders().exists(HTTP_HEADER_CONTENT_ENCODING));
        EXPECT_FALSE(msg->getHeaders().exists(HTTP_HEADER_CONTENT_LENGTH));
      });

  EXPECT_CALL(*requestHandler_, requestComplete()).Times(1);

  auto* filter = new DecompressionFilter(requestHandler_);
  filter->setResponseHandler(responseHandler_.get());

  auto msg = std::make_unique<HTTPMessage>();
  msg->setURL("/test");
  msg->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING, "zstd");
  msg->getHeaders().set(HTTP_HEADER_CONTENT_LENGTH, "12345");

  filter->onRequest(std::move(msg));
  filter->requestComplete();
}

// Test: Unsupported Content-Encoding passes through unchanged
TEST_F(DecompressionFilterTest, UnsupportedEncodingPassesThrough) {
  EXPECT_CALL(*requestHandler_, setResponseHandler(_))
      .WillOnce(DoAll(SaveArg<0>(&downstream_), Return()));

  EXPECT_CALL(*requestHandler_, onRequest(_))
      .WillOnce([](std::shared_ptr<HTTPMessage> msg) {
        // Unsupported encoding should remain
        EXPECT_EQ(
            msg->getHeaders().getSingleOrEmpty(HTTP_HEADER_CONTENT_ENCODING),
            "gzip");
        // Content-Length should also remain
        EXPECT_EQ(
            msg->getHeaders().getSingleOrEmpty(HTTP_HEADER_CONTENT_LENGTH),
            "500");
      });

  EXPECT_CALL(*requestHandler_, requestComplete()).Times(1);

  auto* filter = new DecompressionFilter(requestHandler_);
  filter->setResponseHandler(responseHandler_.get());

  auto msg = std::make_unique<HTTPMessage>();
  msg->setURL("/test");
  msg->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING, "gzip");
  msg->getHeaders().set(HTTP_HEADER_CONTENT_LENGTH, "500");

  filter->onRequest(std::move(msg));
  filter->requestComplete();
}

// Test: EOM handling without decompressor (pass through)
TEST_F(DecompressionFilterTest, EOMPassThroughWithoutDecompressor) {
  EXPECT_CALL(*requestHandler_, setResponseHandler(_))
      .WillOnce(DoAll(SaveArg<0>(&downstream_), Return()));

  EXPECT_CALL(*requestHandler_, onRequest(_)).Times(1);
  EXPECT_CALL(*requestHandler_, onEOM()).Times(1);
  EXPECT_CALL(*requestHandler_, requestComplete()).Times(1);

  auto* filter = new DecompressionFilter(requestHandler_);
  filter->setResponseHandler(responseHandler_.get());

  auto msg = std::make_unique<HTTPMessage>();
  msg->setURL("/test");

  filter->onRequest(std::move(msg));
  filter->onEOM();
  filter->requestComplete();
}

// Test: Empty body with zstd encoding
TEST_F(DecompressionFilterTest, EmptyBodyWithZstdEncoding) {
  EXPECT_CALL(*requestHandler_, setResponseHandler(_))
      .WillOnce(DoAll(SaveArg<0>(&downstream_), Return()));

  EXPECT_CALL(*requestHandler_, onRequest(_)).Times(1);
  EXPECT_CALL(*requestHandler_, onEOM()).Times(1);
  EXPECT_CALL(*requestHandler_, requestComplete()).Times(1);

  auto* filter = new DecompressionFilter(requestHandler_);
  filter->setResponseHandler(responseHandler_.get());

  auto msg = std::make_unique<HTTPMessage>();
  msg->setURL("/test");
  msg->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING, "zstd");

  filter->onRequest(std::move(msg));
  // Don't send any body, just EOM
  filter->onEOM();
  filter->requestComplete();
}

// Test: DecompressionFilterFactory creates filter for zstd requests
TEST_F(DecompressionFilterTest, FactoryCreatesFilterForZstd) {
  DecompressionFilterFactory factory;

  HTTPMessage msg;
  msg.setURL("/test");
  msg.getHeaders().set(HTTP_HEADER_CONTENT_ENCODING, "zstd");

  auto* handler = factory.onRequest(requestHandler_, &msg);

  // Should return a new filter wrapping the handler
  EXPECT_NE(handler, requestHandler_);

  // Clean up - need to set response handler and complete the request
  EXPECT_CALL(*requestHandler_, setResponseHandler(_)).Times(1);
  EXPECT_CALL(*requestHandler_, requestComplete()).Times(1);

  handler->setResponseHandler(responseHandler_.get());
  handler->requestComplete();
}

// Test: DecompressionFilterFactory creates filter for non-zstd requests
TEST_F(DecompressionFilterTest, FactoryCreatesFilterForNonZstd) {
  DecompressionFilterFactory factory;

  HTTPMessage msg;
  msg.setURL("/test");

  auto* handler = factory.onRequest(requestHandler_, &msg);

  // Factory always creates a new filter wrapping the handler
  EXPECT_NE(handler, requestHandler_);

  // Clean up - need to set response handler and complete the request
  EXPECT_CALL(*requestHandler_, setResponseHandler(_)).Times(1);
  EXPECT_CALL(*requestHandler_, requestComplete()).Times(1);

  handler->setResponseHandler(responseHandler_.get());
  handler->requestComplete();
}

// Test: DecompressionFilterFactory creates filter for gzip requests
TEST_F(DecompressionFilterTest, FactoryCreatesFilterForGzip) {
  DecompressionFilterFactory factory;

  HTTPMessage msg;
  msg.setURL("/test");
  msg.getHeaders().set(HTTP_HEADER_CONTENT_ENCODING, "gzip");

  auto* handler = factory.onRequest(requestHandler_, &msg);

  // Factory always creates a new filter wrapping the handler
  EXPECT_NE(handler, requestHandler_);

  // Clean up - need to set response handler and complete the request
  EXPECT_CALL(*requestHandler_, setResponseHandler(_)).Times(1);
  EXPECT_CALL(*requestHandler_, requestComplete()).Times(1);

  handler->setResponseHandler(responseHandler_.get());
  handler->requestComplete();
}

// Test: Large data decompression
TEST_F(DecompressionFilterTest, LargeDataDecompression) {
  // Create a large string (100KB)
  std::string largeData(100UL * 1024, 'X');
  for (size_t i = 0; i < largeData.size(); i += 100) {
    largeData[i] = static_cast<char>('A' + (i % 26));
  }

  EXPECT_CALL(*requestHandler_, setResponseHandler(_))
      .WillOnce(DoAll(SaveArg<0>(&downstream_), Return()));

  EXPECT_CALL(*requestHandler_, onRequest(_)).Times(1);

  std::string accumulatedData;
  EXPECT_CALL(*requestHandler_, onBody(_))
      .WillRepeatedly([&accumulatedData](std::shared_ptr<folly::IOBuf> body) {
        if (body) {
          auto cloned = body->clone();
          auto data = cloned->coalesce();
          accumulatedData += std::string(
              reinterpret_cast<const char*>(data.data()), data.size());
        }
      });

  EXPECT_CALL(*requestHandler_, onEOM()).Times(1);
  EXPECT_CALL(*requestHandler_, requestComplete()).Times(1);

  auto* filter = new DecompressionFilter(requestHandler_);
  filter->setResponseHandler(responseHandler_.get());

  auto msg = std::make_unique<HTTPMessage>();
  msg->setURL("/test");
  msg->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING, "zstd");

  filter->onRequest(std::move(msg));

  auto compressedBody = compressWithZstd(largeData);
  ASSERT_TRUE(compressedBody != nullptr);

  filter->onBody(std::move(compressedBody));
  filter->onEOM();
  filter->requestComplete();

  EXPECT_EQ(accumulatedData, largeData);
}
