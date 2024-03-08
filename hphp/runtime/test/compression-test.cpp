/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <memory>
#include <string>
#include <vector>

#include <brotli/decode.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <zstd.h>

#include "hphp/runtime/test/test-context.h"

#include "hphp/runtime/base/configs/server.h"
#include "hphp/runtime/server/compression.h"

#include "hphp/util/brotli.h"
#include "hphp/util/gzip.h"
#include "hphp/util/zstd.h"

using namespace testing;

namespace HPHP {
//////////////////////////////////////////////////////////////////////

namespace {
//////////////////////////////////////////////////////////////////////

const std::string kResponse(
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque "
    "sollicitudin felis lorem, id scelerisque libero porta vitae. Orci varius "
    "natoque penatibus et magnis dis parturient montes, nascetur ridiculus "
    "mus. Sed vitae convallis est. Ut a massa a ante commodo convallis. "
    "Praesent condimentum feugiat diam. Vestibulum mattis, nunc ac rutrum "
    "euismod, mauris tellus molestie neque, sed posuere mi libero in nisl. "
    "Proin pharetra tincidunt metus, vel mollis erat fringilla ut.\n\nMaecenas "
    "vel tempor felis. Class aptent taciti sociosqu ad litora torquent per "
    "conubia nostra, per inceptos himenaeos. Quisque pretium pulvinar dictum. "
    "Integer sed vestibulum metus. Donec ullamcorper vel nulla ac ornare. "
    "Curabitur hendrerit, urna id congue suscipit, eros odio feugiat mauris, "
    "nec laoreet ex massa eget elit. Proin aliquet erat ipsum, congue rutrum "
    "eros laoreet et. Pellentesque fringilla erat in elementum sagittis. "
    "Praesent dictum erat ac sodales lobortis. Quisque vulputate nisi libero, "
    "egestas aliquet dolor condimentum et. Suspendisse potenti.\n\nDonec "
    "accumsan mattis diam, a dignissim risus dapibus sed. Quisque rutrum "
    "ultricies gravida. Curabitur porttitor venenatis lacinia. Donec cursus "
    "elementum tellus ut faucibus. Vestibulum euismod pharetra tellus, eget "
    "facilisis velit posuere eget. Fusce dapibus lorem sed mi elementum, non "
    "iaculis erat tincidunt. Sed aliquam iaculis purus, sit amet consectetur "
    "lorem faucibus non. Etiam euismod, odio non iaculis tincidunt, magna nisi "
    "tincidunt massa, non feugiat ligula sem ac diam. Phasellus aliquam libero "
    "faucibus, luctus velit at, eleifend lorem.\n\nSed non finibus urna, vitae "
    "interdum ante. Sed tempus orci id nulla lacinia, quis suscipit massa "
    "luctus. Curabitur ultricies, tellus eget scelerisque aliquet, elit justo "
    "pharetra mauris, sit amet consectetur tellus nunc vel lectus. Duis "
    "consequat ligula vitae volutpat sodales. Donec scelerisque lobortis "
    "augue, dignissim condimentum elit consequat nec. Cras lacus risus, "
    "vulputate id iaculis in, finibus quis sapien. Nam porta lorem sed "
    "placerat convallis. Pellentesque varius, nibh at ultrices vehicula, odio "
    "elit feugiat magna, at tristique urna ligula eget lectus. Duis tempor "
    "quam velit, eget auctor velit eleifend in. Phasellus enim quam, rhoncus "
    "in euismod in, mollis nec ex. Vivamus eu est vel nibh molestie tempor ac "
    "eget est. Morbi blandit leo et blandit mattis.\n\nNullam efficitur urna "
    "vel leo rhoncus, ut sodales tortor eleifend. Aliquam quis arcu velit. "
    "Vivamus at interdum dolor. Mauris consectetur ipsum eget ipsum "
    "sollicitudin, non laoreet sem sodales. Sed sit amet lacinia dolor. Nulla "
    "convallis ex ut est feugiat, eget malesuada lectus faucibus. Duis semper, "
    "purus eu lacinia fringilla, purus leo facilisis velit, eget efficitur "
    "libero orci id est. Morbi sed dolor quis orci blandit iaculis at a odio. "
    "Sed sit amet eros porta, tincidunt turpis et, luctus eros. Aenean tempus "
    "mauris at felis consectetur ultrices. Suspendisse eu mollis diam. Morbi "
    "ante eros, blandit ac semper non, ullamcorper quis ex. Sed finibus elit "
    "ut mi volutpat fermentum. Duis finibus scelerisque aliquet.\n\n");

/**
 * Simplest possible implementation of the ITransportHeaders interface.
 */
struct MockHeaders : ITransportHeaders {
  explicit MockHeaders(HeaderMap req = {}, HeaderMap resp = {})
    : reqHeaders(std::move(req)),
      respHeaders(std::move(resp)) {}

  /* Request header methods */
  Method getMethod() override {
    return Method::Unknown;
  }

  const char *getMethodName() override {
    return nullptr;
  }

  const void *getPostData(size_t &size) override {
    return nullptr;
  }

  const char *getUrl() override {
    return nullptr;
  }

  std::string getCommand() override {
    return "";
  }

  std::string getHeader(const char *name) override {
    static const std::string emptyStr;
    auto it = reqHeaders.find(name);
    if (it == reqHeaders.end()) {
      return emptyStr;
    }
    if (it->second.empty()) {
      return emptyStr;
    }
    return it->second[0];
  }

  const HeaderMap& getHeaders() override {
    return reqHeaders;
  }

  void addRequestHeader(const char *name, const char *value) {
    reqHeaders[name].emplace_back(value);
  }

  /* Response header methods */
  void addHeaderNoLock(const char *name, const char *value) override {
    respHeaders[name].emplace_back(value);
  }

  void addHeader(const char *name, const char *value) override {
    addHeaderNoLock(name, value);
  }

  void addHeader(const String& header) override {
    String name;
    const char *value;
    if (splitHeader(header, name, value)) {
      addHeader(name.data(), value);
    }
  }

  void replaceHeader(const char *name, const char *value) override {
    removeHeader(name);
    addHeader(name, value);
  }
  void replaceHeader(const String& header) override {
    String name;
    const char *value;
    if (splitHeader(header, name, value)) {
      replaceHeader(name.data(), value);
    }
  }

  void removeHeader(const char *name) override {
    respHeaders.erase(name);
  }

  void removeAllHeaders() override {
    respHeaders.clear();
  }

  void getResponseHeaders(HeaderMap &headers) override {
    headers = respHeaders;
  }

  void addToCommaSeparatedHeader(const char* name, const char* value) override {
    assertx(name && *name);
    assertx(value);
    const auto it = respHeaders.find(name);
    if (it != respHeaders.end() && !it->second.empty()) {
      it->second[0] = it->second[0] + std::string(", ") + value;
    } else {
      addHeader(name, value);
    }
  }

 private:
  bool splitHeader(const String& header, String &name, const char *&value) {
    int pos = header.find(':');

    if (pos != String::npos) {
      name = header.substr(0, pos);
      value = header.data() + pos;

      do {
        value++;
      } while (*value == ' ');

      return true;
    }
    return false;
  }

  HeaderMap reqHeaders;
  HeaderMap respHeaders;
};

struct MockResponseCompressor : ResponseCompressor {
  explicit MockResponseCompressor(ITransportHeaders* headers = &mh)
    : ResponseCompressor(headers) {}
  MOCK_METHOD0(enable, void());
  MOCK_METHOD0(disable, void());
  MOCK_CONST_METHOD0(isEnabled, bool());
  MOCK_METHOD0(isAccepted, bool());
  MOCK_CONST_METHOD0(isCompressed, bool());
  MOCK_CONST_METHOD0(encodingName, const char*());
  MOCK_METHOD3(compressResponse, StringHolder(const char*, int, bool));

 private:
  static MockHeaders mh;
};

MockHeaders MockResponseCompressor::mh{};

template <typename Compressor>
std::string compress(Compressor* compressor, const std::string& data) {
  EXPECT_NE(compressor, nullptr);
  auto compressed = compressor->compressResponse(
    data.data(), data.size(), true);
  EXPECT_NE(compressed.data(), nullptr);
  int len = compressed.size();
  EXPECT_GT(len, 0);
  EXPECT_LT(len, data.size());
  EXPECT_TRUE(compressor->isCompressed());
  return std::string(compressed.data(), len);
}

template <typename Compressor>
void compressExpectFailure(
    Compressor* compressor, const std::string& data) {
  EXPECT_NE(compressor, nullptr);
  auto compressed = compressor->compressResponse(
    data.data(), data.size(), true);
  EXPECT_EQ(compressed.data(), nullptr);
  EXPECT_EQ(compressed.size(), 0);
  EXPECT_FALSE(compressor->isCompressed());
}

template <typename Compressor>
std::string compressChunked(
    Compressor* compressor,
    const std::string& data,
    size_t chunkSize = 100) {
  EXPECT_NE(compressor, nullptr);
  std::string compressed;
  for (size_t pos = 0; pos < kResponse.size(); pos += chunkSize) {
    auto last = pos + chunkSize >= kResponse.size();
    auto compressedChunk = compressor->compressResponse(
      kResponse.data() + pos, last ? kResponse.size() - pos : chunkSize, last);
    EXPECT_NE(compressedChunk.data(), nullptr);
    EXPECT_GT(compressedChunk.size(), 0);
    compressed.append(compressedChunk.data(), compressedChunk.size());
  }
  EXPECT_TRUE(compressor->isCompressed());
  return compressed;
}

void decompressGzip(std::string compressed, const std::string& expected) {
  int len = compressed.size();
  auto uncompressed = gzdecode(compressed.data(), len);
  EXPECT_NE(uncompressed, nullptr);
  EXPECT_EQ(len, expected.size());
  std::string uncompressedStr(uncompressed, len);
  EXPECT_EQ(uncompressedStr, expected);
}

void decompressBrotli(std::string compressed, const std::string& expected) {
  std::string decompressed;
  size_t decompressed_size = expected.size();
  decompressed.resize(decompressed_size);
  auto result = BrotliDecoderDecompress(
      compressed.size(),
      reinterpret_cast<const uint8_t *>(compressed.data()),
      &decompressed_size,
      reinterpret_cast<uint8_t*>(&(decompressed[0])));
  EXPECT_EQ(result, BROTLI_DECODER_RESULT_SUCCESS);
  EXPECT_EQ(decompressed_size, expected.size());
  decompressed.resize(decompressed_size);
  EXPECT_EQ(decompressed, expected);
}

void decompressZstd(std::string compressed, const std::string& expected) {
  std::string decompressed;
  decompressed.resize(expected.size() + 1);

  auto ret = ZSTD_decompress(
      &(decompressed[0]),
      decompressed.size(),
      compressed.data(),
      compressed.size());
  EXPECT_FALSE(ZSTD_isError(ret)) << ZSTD_getErrorName(ret);

  EXPECT_EQ(ret, expected.size());
  decompressed.resize(ret);
  EXPECT_EQ(decompressed, expected);
}

struct ResponseCompressorTest : Test {
 protected:
  void SetUp() override {
    Cfg::Server::GzipCompressionLevel = 0;
    Cfg::Server::BrotliCompressionEnabled = false;
    Cfg::Server::BrotliChunkedCompressionEnabled = false;
    Cfg::Server::ZstdCompressionEnabled = false;
    Cfg::Server::AddVaryEncoding = true;
  }

  void TearDown() override {}

 protected:
  MockHeaders mh;
};

//////////////////////////////////////////////////////////////////////
}

/*************************
 * Accept-Encoding Tests *
 *************************/

TEST_F(ResponseCompressorTest, testAcceptEncodingNoHeader) {
  EXPECT_FALSE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingEmptyHeader) {
  mh.addRequestHeader("Accept-Encoding", "");
  EXPECT_FALSE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingEmptySpaceHeader) {
  mh.addRequestHeader("Accept-Encoding", " ");
  EXPECT_FALSE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingOtherValue) {
  mh.addRequestHeader("Accept-Encoding", "bar");
  EXPECT_FALSE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingOtherValuePrefix) {
  mh.addRequestHeader("Accept-Encoding", "fooo");
  EXPECT_FALSE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingPrefix) {
  mh.addRequestHeader("Accept-Encoding", "fo");
  EXPECT_FALSE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingOtherValues) {
  mh.addRequestHeader("Accept-Encoding", "bar, quux");
  EXPECT_FALSE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingOtherValuesWithOptions) {
  mh.addRequestHeader("Accept-Encoding", "bar ; some_opt=foo, quux;q=2.5;foo");
  EXPECT_FALSE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncoding) {
  mh.addRequestHeader("Accept-Encoding", "foo");
  EXPECT_TRUE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingSpace) {
  mh.addRequestHeader("Accept-Encoding", " \t foo\t");
  EXPECT_TRUE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingMultipleValues) {
  mh.addRequestHeader("Accept-Encoding", "bar, foo, quux");
  EXPECT_TRUE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingOptions) {
  mh.addRequestHeader("Accept-Encoding", "foo;q=1");
  EXPECT_TRUE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingOptionsSpace) {
  mh.addRequestHeader("Accept-Encoding", "   foo \t; q=1\t;  x=y \t ");
  EXPECT_TRUE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingOptionsSpacedTokens) {
  mh.addRequestHeader("Accept-Encoding", "   foo \t; q = 1\t;  x=y \t ");
  EXPECT_TRUE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingWeirdHeader) {
  mh.addRequestHeader("Accept-Encoding", ";;;;,;;;;,,,foo;;,;;,;  ;, ,;");
  EXPECT_TRUE(acceptsEncoding(&mh, "foo"));
}


TEST_F(ResponseCompressorTest, testAcceptEncodingMultipleWeirdHeader) {
  mh.addRequestHeader("Accept-Encoding", ";;,;;foo;;bar;,,yes;,;,;;,;  ;,no,;");
  EXPECT_FALSE(acceptsEncoding(&mh, "foo"));
  EXPECT_FALSE(acceptsEncoding(&mh, "bar"));
  EXPECT_TRUE(acceptsEncoding(&mh, "yes"));
  EXPECT_TRUE(acceptsEncoding(&mh, "no"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingCases) {
  mh.addRequestHeader("Accept-Encoding", "iLoVeTOCoMPResSReSPONSES");
  EXPECT_TRUE(acceptsEncoding(&mh, "ilovetocompressresponses"));
  EXPECT_TRUE(acceptsEncoding(&mh, "iloVETOCOMPRESSRESPONSes"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingBrokenToken) {
  mh.addRequestHeader("Accept-Encoding", "   fo o \t; q=1\t;  x=y \t ");
  EXPECT_FALSE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingAllSemicolon) {
  mh.addRequestHeader("Accept-Encoding", ";;;;;;;;");
  EXPECT_FALSE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingAllComma) {
  mh.addRequestHeader("Accept-Encoding", ",,,,,");
  EXPECT_FALSE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingAllSemicolonSpaced) {
  mh.addRequestHeader("Accept-Encoding", ";;; ;;   \t; ;\t;");
  EXPECT_FALSE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testAcceptEncodingAllCommaSpaced) {
  mh.addRequestHeader("Accept-Encoding", "\t\t,      \t, ,, ,");
  EXPECT_FALSE(acceptsEncoding(&mh, "foo"));
}

TEST_F(ResponseCompressorTest, testReasonableDictionaryEncodingScenario) {
  mh.addRequestHeader("Accept-Encoding","coolencode;d=3, zstd, gzip;q=0.5");
  EXPECT_TRUE(acceptsEncoding(&mh, "coolencode"));
  EXPECT_TRUE(acceptsEncoding(&mh, "zstd"));
  EXPECT_TRUE(acceptsEncoding(&mh, "gzip"));
}

TEST_F(ResponseCompressorTest, testSemicolonCommaAdjacent) {
  mh.addRequestHeader("Accept-Encoding","foo;d=3;, zstd;,");
  EXPECT_TRUE(acceptsEncoding(&mh, "foo"));
  EXPECT_TRUE(acceptsEncoding(&mh, "zstd"));
}

/**************
 * Gzip Tests *
 **************/

TEST_F(ResponseCompressorTest, testGzipAccepted) {
  mh.addRequestHeader("Accept-Encoding", "gzip");
  auto compressor = std::make_unique<GzipResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isAccepted());
  compressExpectFailure(compressor.get(), kResponse); // because not enabled
}

TEST_F(ResponseCompressorTest, testGzipAcceptedMultiple) {
  mh.addRequestHeader("Accept-Encoding", "foo, gzip, baz");
  auto compressor = std::make_unique<GzipResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isAccepted());
}

TEST_F(ResponseCompressorTest, testGzipAcceptedQFactor) {
  mh.addRequestHeader("Accept-Encoding", "gzip;q=2.0");
  auto compressor = std::make_unique<GzipResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isAccepted());
}

TEST_F(ResponseCompressorTest, testGzipNotAcceptedEmpty) {
  mh.addRequestHeader("Accept-Encoding", "");
  auto compressor = std::make_unique<GzipResponseCompressor>(&mh);
  EXPECT_FALSE(compressor->isAccepted());
}

TEST_F(ResponseCompressorTest, testGzipNotAcceptedOther) {
  mh.addRequestHeader("Accept-Encoding", "foo");
  auto compressor = std::make_unique<GzipResponseCompressor>(&mh);
  EXPECT_FALSE(compressor->isAccepted());
}

TEST_F(ResponseCompressorTest, testGzipNotAcceptedOtherMultiple) {
  mh.addRequestHeader("Accept-Encoding", "foo, bar, baz");
  auto compressor = std::make_unique<GzipResponseCompressor>(&mh);
  EXPECT_FALSE(compressor->isAccepted());
}

TEST_F(ResponseCompressorTest, testGzipEnabled) {
  Cfg::Server::GzipCompressionLevel = 6;
  mh.addRequestHeader("Accept-Encoding", "gzip");
  auto compressor = std::make_unique<GzipResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isEnabled());
  compress(compressor.get(), kResponse);
}

TEST_F(ResponseCompressorTest, testGzipNotEnabled) {
  mh.addRequestHeader("Accept-Encoding", "gzip");
  auto compressor = std::make_unique<GzipResponseCompressor>(&mh);
  EXPECT_FALSE(compressor->isEnabled());
  compressExpectFailure(compressor.get(), kResponse);
}

TEST_F(ResponseCompressorTest, testGzipEncodingName) {
  Cfg::Server::GzipCompressionLevel = 6;
  mh.addRequestHeader("Accept-Encoding", "gzip");
  auto compressor = std::make_unique<GzipResponseCompressor>(&mh);
  EXPECT_EQ(std::string(compressor->encodingName()), "gzip");
}

TEST_F(ResponseCompressorTest, testGzipCompression) {
  Cfg::Server::GzipCompressionLevel = 6;
  mh.addRequestHeader("Accept-Encoding", "gzip");
  auto compressor = std::make_unique<GzipResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isEnabled());
  EXPECT_TRUE(compressor->isAccepted());

  auto compressed = compress(compressor.get(), kResponse);
  decompressGzip(compressed, kResponse);
}

TEST_F(ResponseCompressorTest, testGzipChunkedCompression) {
  Cfg::Server::GzipCompressionLevel = 6;
  mh.addRequestHeader("Accept-Encoding", "gzip");
  auto compressor = std::make_unique<GzipResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isEnabled());
  EXPECT_TRUE(compressor->isAccepted());
  auto compressed = compressChunked(compressor.get(), kResponse);
  decompressGzip(compressed, kResponse);
}

/****************
 * Brotli Tests *
 ****************/

TEST_F(ResponseCompressorTest, testBrotliAccepted) {
  mh.addRequestHeader("Accept-Encoding", "br");
  auto compressor = std::make_unique<BrotliResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isAccepted());
  compressExpectFailure(compressor.get(), kResponse); // because not enabled
}

TEST_F(ResponseCompressorTest, testBrotliAcceptedMultiple) {
  mh.addRequestHeader("Accept-Encoding", "foo, br, baz");
  auto compressor = std::make_unique<BrotliResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isAccepted());
}

TEST_F(ResponseCompressorTest, testBrotliAcceptedQFactor) {
  mh.addRequestHeader("Accept-Encoding", "br;q=2.0");
  auto compressor = std::make_unique<BrotliResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isAccepted());
}

TEST_F(ResponseCompressorTest, testBrotliNotAcceptedEmpty) {
  mh.addRequestHeader("Accept-Encoding", "");
  auto compressor = std::make_unique<BrotliResponseCompressor>(&mh);
  EXPECT_FALSE(compressor->isAccepted());
}

TEST_F(ResponseCompressorTest, testBrotliNotAcceptedOther) {
  mh.addRequestHeader("Accept-Encoding", "foo");
  auto compressor = std::make_unique<BrotliResponseCompressor>(&mh);
  EXPECT_FALSE(compressor->isAccepted());
}

TEST_F(ResponseCompressorTest, testBrotliNotAcceptedOtherMultiple) {
  mh.addRequestHeader("Accept-Encoding", "foo, bar, baz");
  auto compressor = std::make_unique<BrotliResponseCompressor>(&mh);
  EXPECT_FALSE(compressor->isAccepted());
}

TEST_F(ResponseCompressorTest, testBrotliEnabled) {
  Cfg::Server::BrotliCompressionEnabled = true;
  Cfg::Server::BrotliChunkedCompressionEnabled = true;
  auto compressor = std::make_unique<BrotliResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isEnabled());
}

TEST_F(ResponseCompressorTest, testBrotliEnabledOneShot) {
  Cfg::Server::BrotliCompressionEnabled = true;
  auto compressor = std::make_unique<BrotliResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isEnabled());
}

TEST_F(ResponseCompressorTest, testBrotliEnabledChunked) {
  Cfg::Server::BrotliChunkedCompressionEnabled = true;
  auto compressor = std::make_unique<BrotliResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isEnabled());
}

TEST_F(ResponseCompressorTest, testBrotliNotEnabled) {
  auto compressor = std::make_unique<BrotliResponseCompressor>(&mh);
  EXPECT_FALSE(compressor->isEnabled());
}

TEST_F(ResponseCompressorTest, testBrotliEncodingName) {
  Cfg::Server::BrotliCompressionEnabled = true;
  Cfg::Server::BrotliChunkedCompressionEnabled = true;
  mh.addRequestHeader("Accept-Encoding", "br");
  auto compressor = std::make_unique<BrotliResponseCompressor>(&mh);
  EXPECT_EQ(std::string(compressor->encodingName()), "br");
}

TEST_F(ResponseCompressorTest, testBrotliCompression) {
  Cfg::Server::BrotliCompressionEnabled = true;
  Cfg::Server::BrotliChunkedCompressionEnabled = true;
  mh.addRequestHeader("Accept-Encoding", "br");
  auto compressor = std::make_unique<BrotliResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isEnabled());
  EXPECT_TRUE(compressor->isAccepted());
  auto compressed = compress(compressor.get(), kResponse);
  decompressBrotli(compressed, kResponse);
}

TEST_F(ResponseCompressorTest, testBrotliChunkedCompression) {
  Cfg::Server::BrotliCompressionEnabled = true;
  Cfg::Server::BrotliChunkedCompressionEnabled = true;
  mh.addRequestHeader("Accept-Encoding", "br");
  auto compressor = std::make_unique<BrotliResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isEnabled());
  EXPECT_TRUE(compressor->isAccepted());
  auto compressed = compressChunked(compressor.get(), kResponse);
  decompressBrotli(compressed, kResponse);
}

/**************
 * Zstd Tests *
 **************/

TEST_F(ResponseCompressorTest, testZstdAccepted) {
  mh.addRequestHeader("Accept-Encoding", "zstd");
  auto compressor = std::make_unique<ZstdResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isAccepted());
  compressExpectFailure(compressor.get(), kResponse); // because not enabled
}

TEST_F(ResponseCompressorTest, testZstdAcceptedMultiple) {
  mh.addRequestHeader("Accept-Encoding", "foo, zstd, baz");
  auto compressor = std::make_unique<ZstdResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isAccepted());
}

TEST_F(ResponseCompressorTest, testZstdAcceptedQFactor) {
  mh.addRequestHeader("Accept-Encoding", "zstd;q=2.0");
  auto compressor = std::make_unique<ZstdResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isAccepted());
}

TEST_F(ResponseCompressorTest, testZstdNotAcceptedEmpty) {
  mh.addRequestHeader("Accept-Encoding", "");
  auto compressor = std::make_unique<ZstdResponseCompressor>(&mh);
  EXPECT_FALSE(compressor->isAccepted());
}

TEST_F(ResponseCompressorTest, testZstdNotAcceptedOther) {
  mh.addRequestHeader("Accept-Encoding", "foo");
  auto compressor = std::make_unique<ZstdResponseCompressor>(&mh);
  EXPECT_FALSE(compressor->isAccepted());
}

TEST_F(ResponseCompressorTest, testZstdNotAcceptedOtherMultiple) {
  mh.addRequestHeader("Accept-Encoding", "foo, bar, baz");
  auto compressor = std::make_unique<ZstdResponseCompressor>(&mh);
  EXPECT_FALSE(compressor->isAccepted());
}

TEST_F(ResponseCompressorTest, testZstdEnabled) {
  Cfg::Server::ZstdCompressionEnabled = true;
  mh.addRequestHeader("Accept-Encoding", "zstd");
  auto compressor = std::make_unique<ZstdResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isEnabled());
  compress(compressor.get(), kResponse);
}

TEST_F(ResponseCompressorTest, testZstdNotEnabled) {
  mh.addRequestHeader("Accept-Encoding", "zstd");
  auto compressor = std::make_unique<ZstdResponseCompressor>(&mh);
  EXPECT_FALSE(compressor->isEnabled());
  compressExpectFailure(compressor.get(), kResponse);
}

TEST_F(ResponseCompressorTest, testZstdEncodingName) {
  Cfg::Server::ZstdCompressionEnabled = true;
  mh.addRequestHeader("Accept-Encoding", "zstd");
  auto compressor = std::make_unique<ZstdResponseCompressor>(&mh);
  EXPECT_EQ(std::string(compressor->encodingName()), "zstd");
}

TEST_F(ResponseCompressorTest, testZstdCompression) {
  Cfg::Server::ZstdCompressionEnabled = true;
  mh.addRequestHeader("Accept-Encoding", "zstd");
  auto compressor = std::make_unique<ZstdResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isEnabled());
  EXPECT_TRUE(compressor->isAccepted());

  auto compressed = compress(compressor.get(), kResponse);
  decompressZstd(compressed, kResponse);
}

TEST_F(ResponseCompressorTest, testZstdChunkedCompression) {
  Cfg::Server::ZstdCompressionEnabled = true;
  mh.addRequestHeader("Accept-Encoding", "zstd");
  auto compressor = std::make_unique<ZstdResponseCompressor>(&mh);
  EXPECT_TRUE(compressor->isEnabled());
  EXPECT_TRUE(compressor->isAccepted());
  auto compressed = compressChunked(compressor.get(), kResponse);
  decompressZstd(compressed, kResponse);
}

TEST_F(ResponseCompressorTest, testHeaderSettings) {
  mh.addToCommaSeparatedHeader("Vary", "Accept-Encoding");
  HeaderMap responseHeaders;
  mh.getResponseHeaders(responseHeaders);
  EXPECT_EQ(responseHeaders["Vary"][0], "Accept-Encoding");
  mh.addToCommaSeparatedHeader("Vary", "Other Stuff");
  mh.getResponseHeaders(responseHeaders);
  EXPECT_EQ(responseHeaders["Vary"][0], "Accept-Encoding, Other Stuff");
}

/********************
 * Manager Tests *
 ********************/

TEST_F(ResponseCompressorTest, testManagerConstruction) {
  auto dispatcher = std::make_unique<ResponseCompressorManager>(&mh);
}

TEST_F(ResponseCompressorTest, testManagerConstructionWithMock) {
  auto mockPtr = std::make_unique<MockResponseCompressor>();
  auto& mock = *mockPtr;
  std::vector<std::unique_ptr<ResponseCompressor>> vec;
  vec.push_back(std::move(mockPtr));

  EXPECT_CALL(mock, enable()).Times(1);

  auto manager = std::make_unique<ResponseCompressorManager>(
      &mh, std::move(vec));
}

TEST_F(ResponseCompressorTest, testManagerEnableDisable) {
  auto mockPtr = std::make_unique<MockResponseCompressor>();
  auto& mock = *mockPtr;
  std::vector<std::unique_ptr<ResponseCompressor>> vec;
  vec.push_back(std::move(mockPtr));

  EXPECT_CALL(mock, enable()).Times(2);
  EXPECT_CALL(mock, disable()).Times(1);
  EXPECT_CALL(mock, isEnabled())
      .WillOnce(Return(true))
      .WillOnce(Return(false))
      .WillOnce(Return(true));

  auto manager = std::make_unique<ResponseCompressorManager>(
      &mh, std::move(vec));

  EXPECT_EQ(manager->isEnabled(), true);
  manager->disable();
  EXPECT_EQ(manager->isEnabled(), false);
  manager->enable();
  EXPECT_EQ(manager->isEnabled(), true);
}

TEST_F(ResponseCompressorTest, testManagerCompress) {
  auto mockPtr = std::make_unique<MockResponseCompressor>();
  auto& mock = *mockPtr;
  std::vector<std::unique_ptr<ResponseCompressor>> vec;
  vec.push_back(std::move(mockPtr));

  EXPECT_CALL(mock, enable()).Times(1);

  auto manager = std::make_unique<ResponseCompressorManager>(
      &mh, std::move(vec));

  const char *encoding = "xyzzy";

  EXPECT_CALL(mock, isEnabled()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock, isAccepted()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock, compressResponse(kResponse.data(), kResponse.size(), true))
    .WillOnce(InvokeWithoutArgs([]() {
      return StringHolder("foo", 3, FreeType::NoFree); }));
  EXPECT_CALL(mock, isCompressed()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock, encodingName()).WillRepeatedly(Return(encoding));

  auto compressed = compress(manager.get(), kResponse);

  EXPECT_EQ(compressed, "foo");
  EXPECT_EQ(manager->encodingName(), encoding);
}

TEST_F(ResponseCompressorTest, testManagerCompressFailureNotAccepted) {
  auto mockPtr = std::make_unique<MockResponseCompressor>();
  auto& mock = *mockPtr;
  std::vector<std::unique_ptr<ResponseCompressor>> vec;
  vec.push_back(std::move(mockPtr));

  EXPECT_CALL(mock, enable()).Times(1);

  auto manager = std::make_unique<ResponseCompressorManager>(
      &mh, std::move(vec));

  EXPECT_CALL(mock, isEnabled()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock, isAccepted()).WillRepeatedly(Return(false));

  compressExpectFailure(manager.get(), kResponse);
}

TEST_F(ResponseCompressorTest, testManagerCompressFailureNotEnabled) {
  auto mockPtr = std::make_unique<MockResponseCompressor>();
  auto& mock = *mockPtr;
  std::vector<std::unique_ptr<ResponseCompressor>> vec;
  vec.push_back(std::move(mockPtr));

  EXPECT_CALL(mock, enable()).Times(1);

  auto manager = std::make_unique<ResponseCompressorManager>(
      &mh, std::move(vec));

  EXPECT_CALL(mock, isEnabled()).WillRepeatedly(Return(false));
  EXPECT_CALL(mock, isAccepted()).WillRepeatedly(Return(true));

  compressExpectFailure(manager.get(), kResponse);
}

TEST_F(ResponseCompressorTest, testManagerCompressFallbackNotAccepted) {
  auto mockPtr1 = std::make_unique<MockResponseCompressor>();
  auto& mock1 = *mockPtr1;
  auto mockPtr2 = std::make_unique<MockResponseCompressor>();
  auto& mock2 = *mockPtr2;
  std::vector<std::unique_ptr<ResponseCompressor>> vec;
  vec.push_back(std::move(mockPtr1));
  vec.push_back(std::move(mockPtr2));

  EXPECT_CALL(mock1, enable()).Times(1);
  EXPECT_CALL(mock2, enable()).Times(1);

  auto manager = std::make_unique<ResponseCompressorManager>(
      &mh, std::move(vec));

  EXPECT_CALL(mock1, isEnabled()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock1, isAccepted()).WillRepeatedly(Return(false));
  EXPECT_CALL(mock2, isEnabled()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock2, isAccepted()).WillRepeatedly(Return(true));

  EXPECT_CALL(mock2, compressResponse(kResponse.data(), kResponse.size(), true))
    .WillOnce(InvokeWithoutArgs([]() {
      return StringHolder("foo", 3, FreeType::NoFree); }));

  EXPECT_CALL(mock2, isCompressed()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock2, encodingName()).WillRepeatedly(Return("bar"));

  auto compressed = compress(manager.get(), kResponse);

  EXPECT_EQ(compressed, "foo");
}

TEST_F(ResponseCompressorTest, testManagerCompressFallbackNotEnabled) {
  auto mockPtr1 = std::make_unique<MockResponseCompressor>();
  auto& mock1 = *mockPtr1;
  auto mockPtr2 = std::make_unique<MockResponseCompressor>();
  auto& mock2 = *mockPtr2;
  std::vector<std::unique_ptr<ResponseCompressor>> vec;
  vec.push_back(std::move(mockPtr1));
  vec.push_back(std::move(mockPtr2));

  EXPECT_CALL(mock1, enable()).Times(1);
  EXPECT_CALL(mock2, enable()).Times(1);

  auto manager = std::make_unique<ResponseCompressorManager>(
      &mh, std::move(vec));

  EXPECT_CALL(mock1, isEnabled()).WillRepeatedly(Return(false));
  EXPECT_CALL(mock1, isAccepted()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock2, isEnabled()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock2, isAccepted()).WillRepeatedly(Return(true));

  EXPECT_CALL(mock2, compressResponse(kResponse.data(), kResponse.size(), true))
    .WillOnce(InvokeWithoutArgs([]() {
      return StringHolder("foo", 3, FreeType::NoFree); }));

  EXPECT_CALL(mock2, isCompressed()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock2, encodingName()).WillRepeatedly(Return("bar"));

  auto compressed = compress(manager.get(), kResponse);

  EXPECT_EQ(compressed, "foo");
}

TEST_F(ResponseCompressorTest, testManagerCompressFallbackFirstFailed) {
  auto mockPtr1 = std::make_unique<MockResponseCompressor>();
  auto& mock1 = *mockPtr1;
  auto mockPtr2 = std::make_unique<MockResponseCompressor>();
  auto& mock2 = *mockPtr2;
  std::vector<std::unique_ptr<ResponseCompressor>> vec;
  vec.push_back(std::move(mockPtr1));
  vec.push_back(std::move(mockPtr2));

  EXPECT_CALL(mock1, enable()).Times(1);
  EXPECT_CALL(mock2, enable()).Times(1);

  auto manager = std::make_unique<ResponseCompressorManager>(
      &mh, std::move(vec));

  const char *encoding1 = "baz";
  const char *encoding2 = "qux";

  EXPECT_CALL(mock1, isEnabled()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock1, isAccepted()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock1, encodingName()).WillRepeatedly(Return(encoding1));
  EXPECT_CALL(mock2, isEnabled()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock2, isAccepted()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock2, encodingName()).WillRepeatedly(Return(encoding2));

  EXPECT_CALL(mock1, compressResponse(kResponse.data(), kResponse.size(), true))
    .WillOnce(InvokeWithoutArgs([]() { return StringHolder{}; }));
  EXPECT_CALL(mock2, compressResponse(kResponse.data(), kResponse.size(), true))
    .WillOnce(InvokeWithoutArgs([]() {
      return StringHolder("foo", 3, FreeType::NoFree); }));

  EXPECT_CALL(mock2, isCompressed()).WillRepeatedly(Return(true));

  auto compressed = compress(manager.get(), kResponse);

  EXPECT_EQ(compressed, "foo");
  EXPECT_EQ(manager->encodingName(), encoding2);
}

TEST_F(ResponseCompressorTest, testManagerCompressChunked) {
  auto mockPtr = std::make_unique<MockResponseCompressor>();
  auto& mock = *mockPtr;
  std::vector<std::unique_ptr<ResponseCompressor>> vec;
  vec.push_back(std::move(mockPtr));

  EXPECT_CALL(mock, enable()).Times(1);

  auto manager = std::make_unique<ResponseCompressorManager>(
      &mh, std::move(vec));

  const char *encoding = "xyzzy";

  EXPECT_CALL(mock, isEnabled()).WillOnce(Return(true));
  EXPECT_CALL(mock, isAccepted()).WillOnce(Return(true));
  EXPECT_CALL(mock, compressResponse(_, _, _))
    .WillRepeatedly(InvokeWithoutArgs([]() {
      return StringHolder("foo", 3, FreeType::NoFree);}));
  EXPECT_CALL(mock, isCompressed()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock, encodingName()).WillRepeatedly(Return(encoding));

  auto compressed = compressChunked(manager.get(), kResponse);

  EXPECT_EQ(compressed.find("foo"), 0);
  EXPECT_EQ(compressed.rfind("foo"), compressed.size() - 3);
  EXPECT_EQ(manager->encodingName(), encoding);
}

TEST_F(ResponseCompressorTest,
       testManagerCompressChunkedFallbackFirstChunk) {
  auto mockPtr1 = std::make_unique<MockResponseCompressor>();
  auto& mock1 = *mockPtr1;
  auto mockPtr2 = std::make_unique<MockResponseCompressor>();
  auto& mock2 = *mockPtr2;
  std::vector<std::unique_ptr<ResponseCompressor>> vec;
  vec.push_back(std::move(mockPtr1));
  vec.push_back(std::move(mockPtr2));

  EXPECT_CALL(mock1, enable()).Times(1);
  EXPECT_CALL(mock2, enable()).Times(1);

  auto manager = std::make_unique<ResponseCompressorManager>(
      &mh, std::move(vec));

  EXPECT_CALL(mock1, isEnabled()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock1, isAccepted()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock2, isEnabled()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock2, isAccepted()).WillRepeatedly(Return(true));

  EXPECT_CALL(mock1, compressResponse(_, _, _))
    .WillOnce(InvokeWithoutArgs([]() { return StringHolder{}; }));
  EXPECT_CALL(mock2, compressResponse(_, _, _))
    .WillRepeatedly(InvokeWithoutArgs([]() {
      return StringHolder("foo", 3, FreeType::NoFree); }));

  EXPECT_CALL(mock2, isCompressed()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock2, encodingName()).WillRepeatedly(Return("bar"));

  auto compressed = compressChunked(manager.get(), kResponse);

  EXPECT_EQ(compressed.find("foo"), 0);
  EXPECT_EQ(compressed.rfind("foo"), compressed.size() - 3);
}

TEST_F(ResponseCompressorTest, testManagerCompressChunkedFailSecondChunk) {
  auto mockPtr1 = std::make_unique<MockResponseCompressor>();
  auto& mock1 = *mockPtr1;
  auto mockPtr2 = std::make_unique<MockResponseCompressor>();
  auto& mock2 = *mockPtr2;
  std::vector<std::unique_ptr<ResponseCompressor>> vec;
  vec.push_back(std::move(mockPtr1));
  vec.push_back(std::move(mockPtr2));

  EXPECT_CALL(mock1, enable()).Times(1);
  EXPECT_CALL(mock2, enable()).Times(1);

  auto manager = std::make_unique<ResponseCompressorManager>(
      &mh, std::move(vec));

  EXPECT_CALL(mock1, isEnabled()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock1, isAccepted()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock1, isCompressed()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock1, encodingName()).WillRepeatedly(Return("bar"));

  EXPECT_CALL(mock1, compressResponse(_, _, _))
    .WillOnce(InvokeWithoutArgs([]() {
      return StringHolder("foo", 3, FreeType::NoFree); }))
    .WillOnce(InvokeWithoutArgs([]() { return StringHolder{}; }));
  EXPECT_CALL(mock2, compressResponse(_, _, _))
    .WillRepeatedly(InvokeWithoutArgs([]() {
      return StringHolder("bar", 3, FreeType::NoFree); }));

  auto compressed1 = manager->compressResponse(kResponse.data(), 100, false);
  EXPECT_NE(compressed1.data(), nullptr);
  EXPECT_NE(compressed1.size(), 0);
  std::string compressedStr1(compressed1.data(), compressed1.size());
  EXPECT_EQ(compressedStr1, "foo");

  // now that it's committed to sending the response via this impl, if the impl
  // returns an error, no recovery is possible. Ensure that we fail hard.

  EXPECT_THROW(
      manager->compressResponse(kResponse.data() + 100, 200, false),
      std::exception);

  EXPECT_FALSE(manager->isCompressed());
  EXPECT_EQ(manager->encodingName(), nullptr);
}

TEST_F(ResponseCompressorTest, testGetResponseHeadersCompressed) {
  auto mockPtr = std::make_unique<MockResponseCompressor>(&mh);
  auto& mock = *mockPtr;
  std::vector<std::unique_ptr<ResponseCompressor>> vec;
  vec.push_back(std::move(mockPtr));
  auto manager = std::make_unique<ResponseCompressorManager>(
      &mh, std::move(vec));

  EXPECT_CALL(mock, isCompressed()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock, encodingName()).WillRepeatedly(Return("xyzzy"));
  EXPECT_CALL(mock, isEnabled()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock, isAccepted()).WillRepeatedly(Return(true));

  EXPECT_CALL(mock, compressResponse(kResponse.data(), kResponse.size(), true))
    .WillOnce(InvokeWithoutArgs([]() {
      return StringHolder("foo", 3, FreeType::NoFree); }));
  auto compressed = manager->compressResponse(
      kResponse.data(), kResponse.size(), true);

  HeaderMap expectedHeaders;
  expectedHeaders["Content-Encoding"].emplace_back("xyzzy");
  expectedHeaders["Vary"].emplace_back("Accept-Encoding");

  HeaderMap actualHeaders;
  mh.getResponseHeaders(actualHeaders);

  EXPECT_EQ(actualHeaders, expectedHeaders);
}

TEST_F(ResponseCompressorTest, testGetResponseHeadersCompressedNoVary) {
  Cfg::Server::AddVaryEncoding = false;
  auto mockPtr = std::make_unique<MockResponseCompressor>(&mh);
  auto& mock = *mockPtr;
  std::vector<std::unique_ptr<ResponseCompressor>> vec;
  vec.push_back(std::move(mockPtr));
  auto manager = std::make_unique<ResponseCompressorManager>(
      &mh, std::move(vec));

  EXPECT_CALL(mock, isCompressed()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock, encodingName()).WillRepeatedly(Return("xyzzy"));
  EXPECT_CALL(mock, isEnabled()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock, isAccepted()).WillRepeatedly(Return(true));

  EXPECT_CALL(mock, compressResponse(kResponse.data(), kResponse.size(), true))
    .WillOnce(InvokeWithoutArgs([]() {
      return StringHolder("foo", 3, FreeType::NoFree); }));
  auto compressed = manager->compressResponse(
      kResponse.data(), kResponse.size(), true);

  HeaderMap expectedHeaders;
  expectedHeaders["Content-Encoding"].emplace_back("xyzzy");

  HeaderMap actualHeaders;
  mh.getResponseHeaders(actualHeaders);

  EXPECT_EQ(actualHeaders, expectedHeaders);
}

TEST_F(ResponseCompressorTest, testGetResponseHeadersNotEnabled) {
  auto mockPtr = std::make_unique<MockResponseCompressor>(&mh);
  auto& mock = *mockPtr;
  std::vector<std::unique_ptr<ResponseCompressor>> vec;
  vec.push_back(std::move(mockPtr));
  auto manager = std::make_unique<ResponseCompressorManager>(
      &mh, std::move(vec));

  EXPECT_CALL(mock, isCompressed()).WillRepeatedly(Return(false));
  EXPECT_CALL(mock, encodingName()).WillRepeatedly(Return(nullptr));
  EXPECT_CALL(mock, isEnabled()).WillRepeatedly(Return(false));
  EXPECT_CALL(mock, isAccepted()).WillRepeatedly(Return(true));

  auto compressed = manager->compressResponse(
      kResponse.data(), kResponse.size(), true);

  HeaderMap expectedHeaders;

  HeaderMap actualHeaders;
  mh.getResponseHeaders(actualHeaders);

  EXPECT_EQ(actualHeaders, expectedHeaders);
}

TEST_F(ResponseCompressorTest, testGetResponseHeadersNotAccepted) {
  auto mockPtr = std::make_unique<MockResponseCompressor>(&mh);
  auto& mock = *mockPtr;
  std::vector<std::unique_ptr<ResponseCompressor>> vec;
  vec.push_back(std::move(mockPtr));
  auto manager = std::make_unique<ResponseCompressorManager>(
      &mh, std::move(vec));

  EXPECT_CALL(mock, isCompressed()).WillRepeatedly(Return(false));
  EXPECT_CALL(mock, encodingName()).WillRepeatedly(Return(nullptr));
  EXPECT_CALL(mock, isEnabled()).WillRepeatedly(Return(true));
  EXPECT_CALL(mock, isAccepted()).WillRepeatedly(Return(false));

  auto compressed = manager->compressResponse(
      kResponse.data(), kResponse.size(), true);

  HeaderMap expectedHeaders;
  expectedHeaders["Vary"].emplace_back("Accept-Encoding");

  HeaderMap actualHeaders;
  mh.getResponseHeaders(actualHeaders);

  EXPECT_EQ(actualHeaders, expectedHeaders);
}

//////////////////////////////////////////////////////////////////////
}
