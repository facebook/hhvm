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

#include "hphp/runtime/server/compression.h"

#include <boost/algorithm/string.hpp>
#include <folly/String.h>

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/ini-setting.h"

#include "hphp/util/brotli.h"
#include "hphp/util/gzip.h"
#include "hphp/util/logger.h"
#include "hphp/util/zstd.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// TODO(felixh): @nocommit: evaluate for memory leaks. (what is the convention
// for who is responsible for freeing returned strings???)

namespace {
bool isOff(const String& s) {
  return s.size() == 3 && bstrcaseeq(s.data(), "off", 3);
}
bool isOn(const String& s) {
  return s.size() == 2 && bstrcaseeq(s.data(), "on", 2);
}
void finalizeCompressionOnOff(int8_t& state, const char* ini_key) {
  if (state == 0) {
    return;
  }

  String value;
  IniSetting::Get(ini_key, value);
  if (state == -1) {
    /* default off, can opt in */
    state = isOn(value) ? 1 : 0;
  } else /* state == 1 */ {
    /* default on, can opt out */
    state = isOff(value) ? 0 : 1;
  }
}
} // anonymous namespace


const char*
ResponseCompressor::ENCODING_TYPE_TO_NAME[CompressionType::Max + 1] =
    { "br", "br", "zstd", "gzip", "", "" };

ResponseCompressor::ResponseCompressor(ITransportHeaders* headers)
  : m_encodingType(CompressionType::Max),
    m_compressionDecision(CompressionDecision::NotDecidedYet),
    m_headers(headers) {
  assertx(m_headers);

  enableCompression();
}

void ResponseCompressor::enableCompression() {
  assertx(m_compressionDecision == CompressionDecision::NotDecidedYet);
  m_compressionEnabled[CompressionType::Brotli] =
      RuntimeOption::BrotliCompressionEnabled;
  m_compressionEnabled[CompressionType::BrotliChunked] =
      RuntimeOption::BrotliChunkedCompressionEnabled;
  m_compressionEnabled[CompressionType::Zstd] =
      RuntimeOption::ZstdCompressionEnabled;
  m_compressionEnabled[CompressionType::Gzip] =
      RuntimeOption::GzipCompressionLevel ? 1 : 0;
}

void ResponseCompressor::disableCompression() {
  assertx(m_compressionDecision == CompressionDecision::NotDecidedYet);
  for (int i = 0; i < CompressionType::Max; ++i) {
    m_compressionEnabled[i] = 0;
  }
}

bool ResponseCompressor::isCompressionEnabled() const {
  return m_compressionEnabled[CompressionType::Brotli] ||
         m_compressionEnabled[CompressionType::BrotliChunked] ||
         m_compressionEnabled[CompressionType::Zstd] ||
         m_compressionEnabled[CompressionType::Gzip];
}

bool ResponseCompressor::acceptCompression() const {
  return acceptsEncoding("br") ||
         acceptsEncoding("zstd") ||
         acceptsEncoding("gzip");
}

bool ResponseCompressor::isCompressed() const {
  assertx(m_compressionDecision != CompressionDecision::NotDecidedYet);
  return m_encodingType != CompressionType::None;
}

void ResponseCompressor::decideCompression(int size, bool chunkedEncoding) {
  assertx(m_compressionDecision == CompressionDecision::NotDecidedYet);
  m_compressionDecision = CompressionDecision::Decided;

  m_chunkedEncoding = chunkedEncoding;

  // needs to be done here rather than at construction time since these are
  // things we allow userspace to set up until the first packet goes out.
  finalizeCompressionOnOff(
      m_compressionEnabled[CompressionType::Brotli], "brotli.compression");
  finalizeCompressionOnOff(
      m_compressionEnabled[CompressionType::BrotliChunked],
      "brotli.chunked_compression");
  finalizeCompressionOnOff(
      m_compressionEnabled[CompressionType::Zstd], "zstd.compression");
  finalizeCompressionOnOff(
      m_compressionEnabled[CompressionType::Gzip], "zlib.output_compression");

  m_encodingType = CompressionType::None;

  // Gzip has 20 bytes header, so anything smaller than a few bytes probably
  // wouldn't benefit much from compression
  if (m_chunkedEncoding || size > 50) {
    if (acceptsEncoding("zstd") &&
        m_compressionEnabled[CompressionType::Zstd]) {
      // for the moment, prefer zstd
      m_encodingType = CompressionType::Zstd;
    } else if (m_chunkedEncoding &&
               acceptsEncoding("br") &&
               m_compressionEnabled[CompressionType::BrotliChunked]) {
      m_encodingType = CompressionType::Brotli;
    } else if (!m_chunkedEncoding &&
               acceptsEncoding("br") &&
               m_compressionEnabled[CompressionType::Brotli]) {
      m_encodingType = CompressionType::Brotli;
    } else if (acceptsEncoding("gzip") &&
               m_compressionEnabled[CompressionType::Gzip]) {
      m_encodingType = CompressionType::Gzip;
    }
  }
}

bool ResponseCompressor::acceptsEncoding(const char *encoding) const {
  // Examples of valid encodings that we want to accept
  // gzip;q=1.0, identity; q=0.5, *;q=0
  // compress;q=0.5, gzip;q=1.0
  // For now, we don't care about the qvalue

  assertx(encoding && *encoding);
  auto header = m_headers->getHeader("Accept-Encoding");

  // Handle leading and trailing quotes
  size_t len = header.length();
  if (len >= 2
      && ((header[0] == '"' && header[len-1] == '"')
      || (header[0] == '\'' && header[len-1] == '\''))) {
    header = header.substr(1, len - 2);
  }

  // Split the header by ','
  std::vector<std::string> cTokens;
  folly::split(',', header, cTokens);
  for (size_t i = 0; i < cTokens.size(); ++i) {
    // Then split by ';'
    auto& cToken = cTokens[i];
    std::vector<std::string> scTokens;
    folly::split(';', cToken, scTokens);
    assertx(scTokens.size() > 0);
    // lhs contains the encoding
    // rhs, if it exists, contains the qvalue
    std::string lhs = boost::trim_copy(scTokens[0]);
    if (strcasecmp(lhs.c_str(), encoding) == 0) {
      return true;
    }
  }
  return false;
}

const char* ResponseCompressor::compressionName(CompressionType type) {
  return ENCODING_TYPE_TO_NAME[static_cast<int>(type)];
}

StringHolder ResponseCompressor::compressResponse(
    const char *data, int len, bool last) {
  bool first = m_compressionDecision == CompressionDecision::NotDecidedYet;
  if (first) {
    decideCompression(len, !last);
  }

  StringHolder response(nullptr, 0);

  switch (m_encodingType) {
  case CompressionType::None:
    // pass
    break;
  case CompressionType::Zstd:
    response = compressZstd(data, len, last);
    break;
  case CompressionType::Brotli:
    response = compressBrotli(data, len, last);
    break;
  case CompressionType::Gzip:
    response = compressGzip(data, len, last);
    break;
  default:
    raise_error("Unsupported compression type selected!");
  }

  if (response.data() == nullptr && m_encodingType != CompressionType::None) {
    if (m_chunkedEncoding && !first) {
      raise_error("Failed to compress and we've already committed to compressing.");
    } else {
      m_encodingType = CompressionType::None;
    }
  }

  if (first) {
    setResponseHeaders();
  }

  return response;
}

void ResponseCompressor::setResponseHeaders() {
  assertx(m_compressionDecision != CompressionDecision::NotDecidedYet);

  if (m_encodingType != CompressionType::None) {
    auto encodingName = compressionName(m_encodingType);
    m_headers->addHeader("Content-Encoding", encodingName);
  }
  if (RuntimeOption::ServerAddVaryEncoding) {
    /*
     * Our response may vary depending on the Accept-Encoding header if
     *  - we compressed it or
     *  - we didn't compress it because this client does not accept compression
     * (The alternative there being that we could have compressed it but made
     * our own choice not to, in which case it doesn't matter what the client
     * tells us it accepts.)
     */
    if (isCompressed() || (isCompressionEnabled() && !acceptCompression())) {
      m_headers->addHeader("Vary", "Accept-Encoding");
    }
  }
}

StringHolder ResponseCompressor::compressGzip(
    const char *data, int size, bool last) {
  StringHolder response(nullptr, 0);

  int compressionLevel = RuntimeOption::GzipCompressionLevel;
  String compressionLevelStr;
  IniSetting::Get("zlib.output_compression_level", compressionLevelStr);
  int level = compressionLevelStr.toInt64();
  if (level > compressionLevel &&
      level <= RuntimeOption::GzipMaxCompressionLevel) {
    compressionLevel = level;
  }
  if (m_gzipCompressor == nullptr) {
    m_gzipCompressor = std::make_unique<GzipCompressor>(
        compressionLevel, CODING_GZIP, true);
  }
  int len = size;
  char *compressedData =
    m_gzipCompressor->compress((const char*)data, len, last);
  if (compressedData) {
    StringHolder deleter(compressedData, len, true);
    if (m_chunkedEncoding || len < size) {
      response = std::move(deleter);
    }
  } else {
    Logger::Error("Unable to compress response: level=%d len=%d",
                  compressionLevel, len);
  }

  return response;
}

StringHolder ResponseCompressor::compressBrotli(
    const char *data, int size, bool last) {
  if (m_brotliCompressor == nullptr) {
    brotli::BrotliParams params;
    params.mode =
        (brotli::BrotliParams::Mode)RuntimeOption::BrotliCompressionMode;

    Variant quality;
    IniSetting::Get("brotli.compression_quality", quality);
    params.quality = quality.asInt64Val();

    Variant lgWindowSize;
    IniSetting::Get("brotli.compression_lgwin", lgWindowSize);
    params.lgwin = lgWindowSize.asInt64Val();
    if (size && !m_chunkedEncoding) {
      // If there is only one block (i.e. non-chunked content) set a maximum
      // brotli window of ceil(log2(size)). This way the reader doesn't have
      // to waste memory constructing a larger window which will never be used.
      params.lgwin = std::min(
          static_cast<unsigned int>(params.lgwin),
          folly::findLastSet(static_cast<unsigned int>(size) - 1));
    }

    m_brotliCompressor = std::make_unique<brotli::BrotliCompressor>(params);
  }

  size_t len = size;
  auto compressedData =
      HPHP::compressBrotli(m_brotliCompressor.get(), data, len, last);
  if (!compressedData) {
    Logger::Error("Unable to compress response to brotli: size=%d", size);
    return StringHolder(nullptr, 0);
  }

  return StringHolder(compressedData, len, true);
}

StringHolder ResponseCompressor::compressZstd(
    const char *data, int size, bool last) {
  if (m_zstdCompressor == nullptr) {
    Variant quality;
    IniSetting::Get("zstd.compression_level", quality);
    auto compression_level = quality.asInt64Val();

    m_zstdCompressor = std::make_unique<ZstdCompressor>(compression_level);
  }

  size_t len = size;
  auto compressedData = m_zstdCompressor->compress(data, len, last);
  if (!compressedData) {
    Logger::Error("Unable to compress response to zstd: size=%d", size);
    return StringHolder(nullptr, 0);
  }

  return StringHolder(compressedData, len, true);
}

///////////////////////////////////////////////////////////////////////////////
}
