/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/CompressionFilter.h"
#include <folly/coro/GmockHelpers.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/http/coro/HTTPFixedSource.h>
#include <proxygen/lib/http/coro/HTTPHybridSource.h>
#include <proxygen/lib/http/coro/HTTPSourceReader.h>
#include <proxygen/lib/http/coro/test/Mocks.h>
#include <proxygen/lib/utils/ZlibStreamCompressor.h>
#include <proxygen/lib/utils/ZstdStreamDecompressor.h>

using namespace testing;
using namespace proxygen;
using namespace proxygen::coro;

namespace proxygen::coro::test {
MATCHER_P(IOBufEquals, expected, "") {
  return folly::IOBufEqualTo()(arg, expected);
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
    zd_ = T::makeDecompressor();
    source_ = new HTTPStreamSource(&evb_);
    source_->setHeapAllocated();
  }

  void TearDown() override {
  }

 protected:
  HTTPStreamSource* source_;
  std::unique_ptr<StreamDecompressor> zd_;
  folly::EventBase evb_;

  void exercise_compression(
      bool expectCompression,
      const std::string& acceptedEncoding,
      const std::string& expectedEncoding,
      std::unique_ptr<HTTPMessage> respMsg,
      std::unique_ptr<folly::IOBuf> maybeChainedRespBody,
      const CompressionFilterUtils::FactoryOptions& compressionOpts) {
    // single iobuf in chain implies non-chunked msg
    bool isResponseChunked = maybeChainedRespBody->isChained();

    // testing response-only path (TODO: request&response path)
    if (!isResponseChunked) {
      // send unchunked response (however, coro doesn't currently support
      // unchunked compression – it will later override ContentLength header)
      EXPECT_FALSE(respMsg->getIsChunked());
      const auto respLen = maybeChainedRespBody->computeChainDataLength();
      respMsg->getHeaders().set(HTTP_HEADER_CONTENT_LENGTH,
                                folly::to<std::string>(respLen));
    }
    respMsg->setIsChunked(isResponseChunked);
    // queue response into source (with trailers to exercise trailing chunk)
    source_->headers(std::move(respMsg), /*eom=*/false);
    source_->body(maybeChainedRespBody->clone(), /*padding=*/0, /*eom=*/false);
    source_->trailers([]() {
      auto msg = std::make_unique<HTTPHeaders>();
      msg->add("foo", "bar");
      return msg;
    }());
    source_->eom();

    // create the compression filter if the client has declared support for it
    HTTPMessage req;
    req.getHeaders().set(HTTP_HEADER_ACCEPT_ENCODING, acceptedEncoding);
    auto filterParams =
        std::make_shared<folly::Optional<CompressionFilterUtils::FilterParams>>(
            CompressionFilterUtils::getFilterParams(req, compressionOpts));

    HTTPSourceReader reader;
    // create compression filter and set it to read from source_; set reader to
    // read from compression filter
    auto* compressionFilter =
        new CompressionFilter(source_, std::move(filterParams));
    compressionFilter->setHeapAllocated();
    reader.setSource(compressionFilter);

    reader.onHeaders(
        [expectCompression, expectedEncoding](
            std::unique_ptr<HTTPMessage> msg, bool /*final*/, bool /*eom*/) {
          if (expectCompression) {
            EXPECT_TRUE(msg->checkForHeaderToken(
                HTTP_HEADER_CONTENT_ENCODING, expectedEncoding.c_str(), false));
          }
          // coro filter currently only supports chunked compression
          EXPECT_EQ(expectCompression, msg->getIsChunked());
          // "Content-Length" is not set on chunked messages
          const auto& headers = msg->getHeaders();
          EXPECT_EQ(msg->getIsChunked(), !headers.exists("Content-Length"));
          return HTTPSourceReader::Continue;
        });

    // Accumulate the body, decompressing it if it's compressed
    std::unique_ptr<folly::IOBuf> decompressedResponseBody;
    reader.onBody([&](BufQueue body, bool eof) -> bool {
      // nothing to process
      if (body.chainLength() == 0) {
        return HTTPSourceReader::Continue;
      }
      auto processedBody =
          expectCompression ? zd_->decompress(body.move().get()) : body.move();
      CHECK(!zd_->hasError()) << "Failed to decompress body!";
      if (decompressedResponseBody) {
        decompressedResponseBody->prependChain(std::move(processedBody));
      } else {
        decompressedResponseBody = std::move(processedBody);
      }
      return HTTPSourceReader::Continue;
    });

    // read source
    co_withExecutor(&evb_, reader.read()).start();
    evb_.loop();

    CHECK(decompressedResponseBody);
    EXPECT_THAT(decompressedResponseBody.get(),
                IOBufEquals(maybeChainedRespBody.get()));
  }

  // Helper method to convert a vector of strings to an IOBuf chain
  // specifically create a chain because the chain pieces are chunks
  std::unique_ptr<folly::IOBuf> createResponseChain(
      const std::vector<std::string>& bodyStrings) {
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

  // create compression object with provided options
  CompressionFilterUtils::FactoryOptions createCompressionOpts(
      int32_t compressionLevel = T::getCompressionLevel(),
      uint32_t minimumCompressionSize = 1,
      bool disableCompressionForThisEncoding = false) {
    CompressionFilterUtils::FactoryOptions opts{};
    opts.zlibCompressionLevel = compressionLevel;
    opts.minimumCompressionSize = minimumCompressionSize;
    opts.compressibleContentTypes = std::make_shared<std::set<std::string>>(
        std::set<std::string>{"text/html"});
    opts.enableZstd = true;
    if (disableCompressionForThisEncoding) {
      if (CodecType::getExpectedEncoding() == "gzip") {
        opts.enableGzip = false;
      }
      if (CodecType::getExpectedEncoding() == "zstd") {
        opts.enableZstd = false;
      }
    }
    return opts;
  }

  // create http resp message object with provided headers
  std::unique_ptr<HTTPMessage> createResponse(
      std::string contentType, bool respAlreadyCompressed = false) {
    // create response message & headers
    auto msg = makeResponse(200);
    msg->getHeaders().set(HTTP_HEADER_CONTENT_TYPE, contentType);
    // response already compressed, signal via header to skip filter
    if (respAlreadyCompressed) {
      msg->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING, "gzip");
    }
    return msg;
  }
};

using CompressionCodecs = ::testing::Types<ZlibTest, ZstdTest>;

TYPED_TEST_SUITE(CompressionFilterTest, CompressionCodecs);

// Basic smoke test
TYPED_TEST(CompressionFilterTest, NonchunkedCompression) {
  using Codec = typename TestFixture::CodecType;
  const std::vector<std::string> chunks = {"Hello World"};
  auto defaultOpts = this->createCompressionOpts();
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        /*expectCompression=*/true,
        /*acceptedEncoding=*/Codec::getExpectedEncoding(),
        /*expectedEncoding=*/Codec::getExpectedEncoding(),
        /*respMsg=*/this->createResponse("text/html"),
        /*maybeChainedRespBody=*/this->createResponseChain(chunks),
        /*compressionOpts=*/defaultOpts);
  });
}

TYPED_TEST(CompressionFilterTest, ChunkedCompression) {
  using Codec = typename TestFixture::CodecType;
  const std::vector<std::string> chunks = {"Hello", " World"};
  auto defaultOpts = this->createCompressionOpts();
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        /*expectCompression=*/true,
        /*acceptedEncoding=*/Codec::getExpectedEncoding(),
        /*expectedEncoding=*/Codec::getExpectedEncoding(),
        /*respMsg=*/this->createResponse("text/html"),
        /*maybeChainedRespBody=*/this->createResponseChain(chunks),
        /*compressionOpts=*/defaultOpts);
  });
}

TYPED_TEST(CompressionFilterTest, ParameterizedContenttype) {
  using Codec = typename TestFixture::CodecType;
  const std::vector<std::string> chunks = {"Hello World"};
  auto defaultOpts = this->createCompressionOpts();
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        /*expectCompression=*/true,
        /*acceptedEncoding=*/Codec::getExpectedEncoding(),
        /*expectedEncoding=*/Codec::getExpectedEncoding(),
        /*respMsg=*/this->createResponse("text/html; param1"),
        /*maybeChainedRespBody=*/this->createResponseChain(chunks),
        /*compressionOpts=*/defaultOpts);
  });
}

TYPED_TEST(CompressionFilterTest, MixedcaseContenttype) {
  using Codec = typename TestFixture::CodecType;
  const std::vector<std::string> chunks = {"Hello World"};
  auto defaultOpts = this->createCompressionOpts();
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        /*expectCompression=*/true,
        /*acceptedEncoding=*/Codec::getExpectedEncoding(),
        /*expectedEncoding=*/Codec::getExpectedEncoding(),
        /*respMsg=*/this->createResponse("Text/Html; param1"),
        /*maybeChainedRespBody=*/this->createResponseChain(chunks),
        /*compressionOpts=*/defaultOpts);
  });
}

// Client supports multiple possible compression encodings
TYPED_TEST(CompressionFilterTest, MultipleAcceptedEncodings) {
  using Codec = typename TestFixture::CodecType;
  const std::vector<std::string> chunks = {"Hello World"};
  auto defaultOpts = this->createCompressionOpts();
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        /*expectCompression=*/true,
        /*acceptedEncoding=*/Codec::getExpectedEncoding() +
            ", identity, deflate",
        /*expectedEncoding=*/Codec::getExpectedEncoding(),
        /*respMsg=*/this->createResponse("text/html"),
        /*maybeChainedRespBody=*/this->createResponseChain(chunks),
        /*compressionOpts=*/defaultOpts);
  });
}

// Server skips compressing if the response is already compressed
TYPED_TEST(CompressionFilterTest, ResponseAlreadyCompressedTest) {
  using Codec = typename TestFixture::CodecType;
  auto compressor =
      std::make_unique<ZlibStreamCompressor>(CompressionType::GZIP, 4);
  auto fakeCompressed = folly::IOBuf::copyBuffer("helloimsupposedlycompressed");
  auto compressionOpts =
      this->createCompressionOpts(/*compressionLevel=*/4,
                                  /*minimumCompressionSize=*/1);
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        /*expectCompression=*/false,
        /*acceptedEncoding=*/Codec::getExpectedEncoding(),
        /*expectedEncoding=*/Codec::getExpectedEncoding(),
        /*respMsg=*/
        this->createResponse("text/html", /*responseAlreadyCompressed=*/true),
        /*maybeChainedRespBody=*/std::move(fakeCompressed),
        /*compressionOpts=*/compressionOpts);
  });
}

TYPED_TEST(CompressionFilterTest, MultipleAcceptedEncodingsQvalues) {
  using Codec = typename TestFixture::CodecType;
  const std::vector<std::string> chunks = {"Hello World"};
  auto defaultOpts = this->createCompressionOpts();
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        /*expectCompression=*/true,
        /*acceptedEncoding=*/Codec::getExpectedEncoding() + "; q=.7;, identity",
        /*expectedEncoding=*/Codec::getExpectedEncoding(),
        /*respMsg=*/this->createResponse("text/html"),
        /*maybeChainedRespBody=*/this->createResponseChain(chunks),
        /*compressionOpts=*/defaultOpts);
  });
}

TYPED_TEST(CompressionFilterTest, NoCompressibleAcceptedEncodings) {
  const std::vector<std::string> chunks = {"Hello World"};
  auto defaultOpts = this->createCompressionOpts();
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        /*expectCompression=*/false,
        /*acceptedEncoding=*/"identity; q=.7;",
        /*expectedEncoding=*/"",
        /*respMsg=*/this->createResponse("text/html"),
        /*maybeChainedRespBody=*/this->createResponseChain(chunks),
        /*compressionOpts=*/defaultOpts);
  });
}

TYPED_TEST(CompressionFilterTest, MissingAcceptedEncodings) {
  const std::vector<std::string> chunks = {"Hello World"};
  auto defaultOpts = this->createCompressionOpts();
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        /*expectCompression=*/false,
        /*acceptedEncoding=*/"",
        /*expectedEncoding=*/"",
        /*respMsg=*/this->createResponse("text/html"),
        /*maybeChainedRespBody=*/this->createResponseChain(chunks),
        /*compressionOpts=*/defaultOpts);
  });
}

// Content is of an-uncompressible content-type
TYPED_TEST(CompressionFilterTest, UncompressibleContenttype) {
  using Codec = typename TestFixture::CodecType;
  const std::vector<std::string> chunks = {"Hello World"};
  auto defaultOpts = this->createCompressionOpts();
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        /*expectCompression=*/false,
        /*acceptedEncoding=*/Codec::getExpectedEncoding(),
        /*expectedEncoding=*/"",
        /*respMsg=*/this->createResponse("image/jpeg"),
        /*maybeChainedRespBody=*/this->createResponseChain(chunks),
        /*compressionOpts=*/defaultOpts);
  });
}

TYPED_TEST(CompressionFilterTest, UncompressibleContenttypeParam) {
  using Codec = typename TestFixture::CodecType;
  const std::vector<std::string> chunks = {"Hello World"};
  auto defaultOpts = this->createCompressionOpts();
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        /*expectCompression=*/false,
        /*acceptedEncoding=*/Codec::getExpectedEncoding(),
        /*expectedEncoding=*/"",
        /*respMsg=*/this->createResponse("application/jpeg; param1"),
        /*maybeChainedRespBody=*/this->createResponseChain(chunks),
        /*compressionOpts=*/defaultOpts);
  });
}

// Content is under the minimum compression size
TYPED_TEST(CompressionFilterTest, TooSmallToCompress) {
  using Codec = typename TestFixture::CodecType;
  const std::vector<std::string> chunks = {"Hello World"};
  auto compressionOpts = this->createCompressionOpts(
      /*compressionLevel=*/Codec::getCompressionLevel(),
      /*minimumCompressionSize=*/1000);
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        /*expectCompression=*/false,
        /*acceptedEncoding=*/Codec::getExpectedEncoding(),
        /*expectedEncoding=*/"",
        /*respMsg=*/this->createResponse("text/html"),
        /*maybeChainedRespBody=*/this->createResponseChain(chunks),
        /*compressionOpts=*/compressionOpts);
  });
}

TYPED_TEST(CompressionFilterTest, SmallChunksCompress) {
  // Expect this to compress despite being small because can't tell the content
  // length when we're chunked
  using Codec = typename TestFixture::CodecType;
  std::vector<std::string> chunks = {"Hello", " World"};
  auto compressionOpts = this->createCompressionOpts(
      /*compressionLevel=*/Codec::getCompressionLevel(),
      /*minimumCompressionSize=*/1000);
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        /*expectCompression=*/true,
        /*acceptedEncoding=*/Codec::getExpectedEncoding(),
        /*expectedEncoding=*/Codec::getExpectedEncoding(),
        /*respMsg=*/this->createResponse("text/html"),
        /*maybeChainedRespBody=*/this->createResponseChain(chunks),
        /*compressionOpts=*/compressionOpts);
  });
}

TYPED_TEST(CompressionFilterTest, MinimumCompressSizeEqualToRequestSize) {
  using Codec = typename TestFixture::CodecType;
  const std::vector<std::string> chunks = {"Hello World"};
  auto compressionOpts = this->createCompressionOpts(
      /*compressionLevel=*/Codec::getCompressionLevel(),
      /*minimumCompressionSize=*/chunks[0].size());
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        /*expectCompression=*/true,
        /*acceptedEncoding=*/Codec::getExpectedEncoding(),
        /*expectedEncoding=*/Codec::getExpectedEncoding(),
        /*respMsg=*/this->createResponse("text/html"),
        /*maybeChainedRespBody=*/this->createResponseChain(chunks),
        /*compressionOpts=*/compressionOpts);
  });
}

TYPED_TEST(CompressionFilterTest, CompressionDisabledForEncoding) {
  using Codec = typename TestFixture::CodecType;
  const std::vector<std::string> chunks = {"Hello World"};
  auto compressionOpts = this->createCompressionOpts(
      /*compressionLevel=*/1,
      /*minimumCompressionSize=*/1,
      /*disableCompressionForThisEncoding=*/true);
  ASSERT_NO_FATAL_FAILURE({
    this->exercise_compression(
        /*expectCompression=*/false,
        /*acceptedEncoding=*/Codec::getExpectedEncoding(),
        /*expectedEncoding=*/Codec::getExpectedEncoding(),
        /*respMsg=*/this->createResponse("text/html"),
        /*maybeChainedRespBody=*/this->createResponseChain(chunks),
        /*compressionOpts=*/compressionOpts);
  });
}
} // namespace proxygen::coro::test
