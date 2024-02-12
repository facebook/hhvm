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

#include <strings.h>

#include <folly/Random.h>

#include "hphp/runtime/base/configs/server.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/ini-setting.h"

#include "hphp/util/brotli.h"
#include "hphp/util/gzip.h"
#include "hphp/util/logger.h"
#include "hphp/util/zstd.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
const StaticString s_brotliCQ("brotli.compression_quality");
const StaticString s_brotliCL("brotli.compression_lgwin");
const StaticString s_brotliC("brotli.compression");
const StaticString s_brotliCC("brotli.chunked_compression");

const StaticString s_zstdCL("zstd.compression_level");
const StaticString s_zstdCR("zstd.checksum_rate");
const StaticString s_zstdWL("zstd.window_log");
const StaticString s_zstdBS("zstd.target_block_size");
const StaticString s_zstdC("zstd.compression");

const StaticString s_zlibOCL("zlib.output_compression_level");
const StaticString s_zlibOC("zlib.output_compression");

bool isOff(const String& s) {
  return s.size() == 3 && bstrcaseeq(s.data(), "off", 3);
}
bool isOn(const String& s) {
  return s.size() == 2 && bstrcaseeq(s.data(), "on", 2);
}
void finalizeCompressionOnOff(int8_t& state, const StaticString& ini_key) {
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

std::vector<std::unique_ptr<ResponseCompressor>> makeImpls(ITransportHeaders* headers) {
  std::vector<std::unique_ptr<ResponseCompressor>> impls;
  impls.push_back(std::make_unique<ZstdResponseCompressor>(headers));
  impls.push_back(std::make_unique<BrotliResponseCompressor>(headers));
  impls.push_back(std::make_unique<GzipResponseCompressor>(headers));
  return impls;
}

// Returns whether or not a vector of CompressionEncodingPair accepts encoding
bool acceptsCompressionHeaderEncoding(
    const std::vector<CompressionEncodingPair>& encodingHeader,
    const char* encoding) {
  assertx(encoding && *encoding);
  for (const auto& elem : encodingHeader) {
    if (strcasecmp(elem.encoding.data(), encoding) == 0) {
      return true;
    }
  }

  return false;
}

bool acceptsEncoding(folly::StringPiece header, const char *encoding) {
  // Examples of valid encodings that we want to accept
  // gzip;q=1.0, identity; q=0.5, *;q=0
  // compress;q=0.5, gzip;q=1.0
  // For now, we don't care about the qvalue

  // TODO: handle *
  // TODO: handle priorities
  // TODO: handle q=0 disabling

  assertx(encoding && *encoding);
  std::vector<CompressionEncodingPair> parsedHeader = parseEncoding(header);
  return acceptsCompressionHeaderEncoding(parsedHeader, encoding);
}

std::unique_ptr<ResponseCompressor> makeGzipResponseCompressor(
    ITransportHeaders *headers) {
  return std::make_unique<GzipResponseCompressor>(headers);
}

std::unique_ptr<ResponseCompressor> makeBrotliResponseCompressor(
    ITransportHeaders *headers) {
  return std::make_unique<BrotliResponseCompressor>(headers);
}

std::unique_ptr<ResponseCompressor> makeZstdResponseCompressor(
    ITransportHeaders *headers) {
  return std::make_unique<ZstdResponseCompressor>(headers);
}

struct StaticInitTrigger {
  StaticInitTrigger() {
    // priorities as described in header file comment
    ResponseCompressorManager::addImpl(makeZstdResponseCompressor, 30);
    ResponseCompressorManager::addImpl(makeBrotliResponseCompressor, 20);
    ResponseCompressorManager::addImpl(makeGzipResponseCompressor, 10);
  }
};

const StaticInitTrigger trigger;
} // anonymous namespace

std::vector<CompressionEncodingParam> parseParams(
    const std::string& allParams) {
  std::vector<CompressionEncodingParam> ret;
  std::vector<folly::StringPiece> allParamsSplit;
  folly::split(';', allParams, allParamsSplit);

  for (auto& paramValue : allParamsSplit) {
    CompressionEncodingParam p;
    auto paramName = paramValue.split_step('=');
    p.param = std::string(folly::trimWhitespace(paramName));
    p.value = std::string(folly::trimWhitespace(paramValue));
    ret.emplace_back(std::move(p));
  }

  return ret;
}

std::vector<CompressionEncodingPair> parseEncoding(folly::StringPiece header) {
  // Handle leading and trailing quotes
  std::vector<CompressionEncodingPair> encodingHeader;
  size_t len = header.size();
  if (len >= 2
      && ((header[0] == '"' && header[len-1] == '"')
      || (header[0] == '\'' && header[len-1] == '\''))) {
    header = folly::StringPiece(header.data() + 1, len - 2);
  }

  // Split the header by ','
  std::vector<folly::StringPiece> cTokens;
  folly::split(',', header, cTokens, true);

  for (size_t i = 0; i < cTokens.size(); ++i) {
    // Then split by first ';'
    auto encoding = cTokens[i].split_step(';');
    auto trimmedEncoding = folly::trimWhitespace(std::move(encoding));
    CompressionEncodingPair pair;
    pair.encoding = std::string(trimmedEncoding);
    if (!cTokens[i].empty()) {
      pair.params = std::string(cTokens[i]);
    }
    encodingHeader.push_back(pair);

  }
  return encodingHeader;
}

bool acceptsEncoding(ITransportHeaders* headers, const char *encoding) {
  const auto& map = headers->getHeaders();
  auto it = map.find("Accept-Encoding");
  if (it == map.end()) {
    return false;
  }
  const auto& vec = it->second;
  if (vec.empty()) {
    return false;
  }
  return acceptsEncoding(it->second[0], encoding);
}

ResponseCompressor::ResponseCompressor(ITransportHeaders *headers)
  : m_headers(headers),
    m_accepted(-1) {
  assertx(m_headers);
}

bool ResponseCompressor::isAccepted() {
  if (m_accepted >= 0) {
    return m_accepted > 0;
  }
  m_accepted = acceptsEncoding(m_headers, encodingName());
  return m_accepted;
}

/*************
 * Gzip Impl *
 *************/

GzipResponseCompressor::GzipResponseCompressor(ITransportHeaders* headers)
  : ResponseCompressor(headers),
    m_enabled(RuntimeOption::GzipCompressionLevel > 0) {}

void GzipResponseCompressor::enable() {
  m_enabled = RuntimeOption::GzipCompressionLevel > 0;
}

void GzipResponseCompressor::disable() {
  m_enabled = false;
}

GzipCompressor* GzipResponseCompressor::getCompressor() {
  if (!m_compressor) {
    finalizeCompressionOnOff(m_enabled, s_zlibOC);
    if (!isEnabled()) {
      return nullptr;
    }
    int compressionLevel = RuntimeOption::GzipCompressionLevel;
    String compressionLevelStr;
    IniSetting::Get(s_zlibOCL, compressionLevelStr);
    int level = compressionLevelStr.toInt64();
    if (level > compressionLevel &&
        level <= RuntimeOption::GzipMaxCompressionLevel) {
      compressionLevel = level;
    }
    m_compressor = std::make_unique<GzipCompressor>(
        compressionLevel, CODING_GZIP, true);
  }

  return m_compressor.get();
}

StringHolder GzipResponseCompressor::compressResponse(
    const char *data, int len, bool last) {
  auto compressor = getCompressor();
  if (!compressor) {
    // We just decided not to use this compressor. This doesn't necessarily
    // imply an error: gzip could have simply been disabled by the request
    // userland. Return null but don't log an error.
    return StringHolder{};
  }
  auto compressedData = compressor->compress(data, len, last);
  if (!compressedData) {
    m_compressor.reset();
    Logger::Error("Unable to compress response: len=%d", len);
    return StringHolder{};
  }
  return compressedData;
}

/***************
 * Brotli Impl *
 ***************/

BrotliResponseCompressor::BrotliResponseCompressor(ITransportHeaders* headers)
  : ResponseCompressor(headers),
    m_enabled(RuntimeOption::BrotliCompressionEnabled),
    m_chunkedEnabled(RuntimeOption::BrotliChunkedCompressionEnabled) {}

void BrotliResponseCompressor::enable() {
  m_enabled = RuntimeOption::BrotliCompressionEnabled;
  m_chunkedEnabled = RuntimeOption::BrotliChunkedCompressionEnabled;
}

void BrotliResponseCompressor::disable() {
  m_enabled = false;
  m_chunkedEnabled = false;
}

BrotliCompressor* BrotliResponseCompressor::getCompressor(
    int size, bool last) {
  if (!m_compressor) {
    if (last) {
      finalizeCompressionOnOff(m_enabled, s_brotliC);
      m_chunkedEnabled = false;
    } else {
      m_enabled = false;
      finalizeCompressionOnOff(m_chunkedEnabled, s_brotliCC);
    }
    if (!isEnabled()) {
      return nullptr;
    }
    BrotliEncoderMode mode =
        (BrotliEncoderMode)RuntimeOption::BrotliCompressionMode;

    Variant qualityVar;
    IniSetting::Get(s_brotliCQ, qualityVar);
    uint32_t quality = static_cast<uint32_t>(qualityVar.asInt64Val());


    Variant windowSizeVar;
    IniSetting::Get(s_brotliCL, windowSizeVar);
    uint32_t windowSize = static_cast<uint32_t>(windowSizeVar.asInt64Val());
    if (size && !m_chunkedEnabled) {
      // If there is only one block (i.e. non-chunked content) set a maximum
      // brotli window of ceil(log2(size)). This way the reader doesn't have
      // to waste memory constructing a larger window which will never be used.

      // First, calculate the window size. It must be at least
      // BROTLI_MIN_WINDOW_BITS large.
      auto calculatedSize = std::max(
        folly::findLastSet(static_cast<uint32_t>(size) - 1),
        static_cast<uint32_t>(BROTLI_MIN_WINDOW_BITS));

      // Then, use whichever is smaller: configured value or calculated one.
      windowSize = std::min(windowSize, calculatedSize);
    }

    m_compressor = std::make_unique<BrotliCompressor>(mode, quality, windowSize);
  }

  return m_compressor.get();
}

StringHolder BrotliResponseCompressor::compressResponse(
    const char *data, int len, bool last) {
  auto compressor = getCompressor(len, last);
  if (!compressor) {
    // We just decided not to use this compressor. This doesn't necessarily
    // imply an error: brotli could have simply been disabled by the request
    // userland. Return null but don't log an error.
    return StringHolder{};
  }
  size_t size = len;
  auto compressedData = compressor->compress(data, size, last);
  if (!compressedData) {
    m_compressor.reset();
    Logger::Error("Unable to compress response to brotli: len=%d", len);
    return StringHolder{};
  }
  return compressedData;
}

/*************
 * Zstd Impl *
 *************/

ZstdResponseCompressor::ZstdResponseCompressor(ITransportHeaders* headers)
  : ResponseCompressor(headers),
    m_enabled(RuntimeOption::ZstdCompressionEnabled) {}

void ZstdResponseCompressor::enable() {
  m_enabled = RuntimeOption::ZstdCompressionEnabled;
}

void ZstdResponseCompressor::disable() {
  m_enabled = false;
}

ZstdCompressor* ZstdResponseCompressor::getCompressor() {
  if (!m_compressor) {
    finalizeCompressionOnOff(m_enabled, s_zstdC);
    if (!isEnabled()) {
      return nullptr;
    }
    Variant quality;
    IniSetting::Get(s_zstdCL, quality);
    auto compression_level = quality.asInt64Val();

    Variant checksumRate;
    IniSetting::Get(s_zstdCR, checksumRate);
    auto checksum_rate = checksumRate.asInt64Val();

    Variant windowLog;
    IniSetting::Get(s_zstdWL, windowLog);
    auto window_log = windowLog.asInt64Val();

    Variant targetBlockSize;
    IniSetting::Get(s_zstdBS, targetBlockSize);
    auto target_block_size = targetBlockSize.asInt64Val();

    m_compressor = std::make_unique<ZstdCompressor>(
        compression_level, folly::Random::oneIn(checksum_rate), window_log, target_block_size);
  }

  return m_compressor.get();
}

StringHolder ZstdResponseCompressor::compressResponse(
    const char *data, int len, bool last) {
  auto compressor = getCompressor();
  if (!compressor) {
    // We just decided not to use this compressor. This doesn't necessarily
    // imply an error: zstd could have simply been disabled by the request
    // userland. Return null but don't log an error.
    return StringHolder{};
  }
  size_t size = len;
  auto compressedData = compressor->compress(data, size, last);
  if (!compressedData) {
    m_compressor.reset();
    Logger::Error("Unable to compress response to zstd: len=%d", len);
  }
  return compressedData;
}

/**************
 * Dispatcher *
 **************/

std::unique_ptr<std::vector<std::pair<ResponseCompressorManager::Factory, int>>>
    ResponseCompressorManager::s_implFactories;

void ResponseCompressorManager::addImpl(Factory implFactory, int priority) {
  if (!s_implFactories) {
    s_implFactories = std::make_unique<std::vector<std::pair<Factory, int>>>();
  }
  s_implFactories->emplace_back(implFactory, priority);
  // prefer repeatedly sorting at static init time rather than at use time
  std::sort(
      s_implFactories->begin(),
      s_implFactories->end(),
      [](const std::pair<Factory, int>& a, const std::pair<Factory, int>& b) {
        return a.second > b.second;
      });
}

std::vector<std::unique_ptr<ResponseCompressor>>
ResponseCompressorManager::makeImpls(ITransportHeaders *headers) {
  std::vector<std::unique_ptr<ResponseCompressor>> impls;
  if (s_implFactories) {
    impls.reserve(s_implFactories->size());
    for (const auto& pair : *s_implFactories) {
      auto factory = pair.first;
      impls.push_back(factory(headers));
    }
  }
  return impls;
}


ResponseCompressorManager::ResponseCompressorManager(
    ITransportHeaders *headers)
  : ResponseCompressorManager(headers, makeImpls(headers)) {}

ResponseCompressorManager::ResponseCompressorManager(
    ITransportHeaders *headers,
    std::vector<std::unique_ptr<ResponseCompressor>> impls)
  : m_headers(headers),
    m_compressionDecision(CompressionDecision::NotDecidedYet),
    m_impls(std::move(impls)),
    m_selectedImpl(nullptr),
    m_shadowImpls(),
    m_chunkedEncoding(false) {
  enable();
}

void ResponseCompressorManager::enable() {
  assertx(m_compressionDecision == CompressionDecision::NotDecidedYet);
  for (auto &impl : m_impls) {
    impl->enable();
  }
}

void ResponseCompressorManager::disable() {
  assertx(m_compressionDecision == CompressionDecision::NotDecidedYet);
  for (auto &impl : m_impls) {
    impl->disable();
  }
}

bool ResponseCompressorManager::isEnabled() const {
  for (const auto &impl : m_impls) {
    if (impl->isEnabled()) {
      return true;
    }
  }
  return false;
}

bool ResponseCompressorManager::isAccepted() {
  for (const auto &impl : m_impls) {
    if (impl->isEnabled() && impl->isAccepted()) {
      return true;
    }
  }
  return false;
}

bool ResponseCompressorManager::isCompressed() const {
  assertx(m_compressionDecision != CompressionDecision::NotDecidedYet);
  return m_selectedImpl ? m_selectedImpl->isCompressed() : false;
}

const char* ResponseCompressorManager::encodingName() const {
  assertx(m_compressionDecision != CompressionDecision::NotDecidedYet);
  return m_selectedImpl ? m_selectedImpl->encodingName() : nullptr;
}

StringHolder ResponseCompressorManager::compressResponse(
    const char *data, int len, bool last) {
  auto first = m_compressionDecision == CompressionDecision::NotDecidedYet;
  m_chunkedEncoding |= !last;
  StringHolder response{};
  if (first) {
    m_compressionDecision = CompressionDecision::Decided;
    // Select first impl that:
    // - is enabled
    // - is accepted by the client
    // - successfully compresses
    if (!m_chunkedEncoding && len < 75) {
      // probably not worth compressing. skip.
    } else {
      for (const auto &impl : m_impls) {
        if (impl->isEnabled() && impl->isAccepted()) {
          response = impl->compressResponse(data, len, last);
          if (response.data() == nullptr) {
            // doesn't necessarily indicate a failure, impl is allowed to do
            // last-minute decision making about whether it can be used, e.g.,
            // by inspecting ini settings
          } else {
            m_selectedImpl = impl.get();
            break;
          }
        }
      }
      // also build the list of shadow impls
      for (const auto &impl : m_impls) {
        if (impl->shouldShadow() && !(impl->isEnabled() && impl->isAccepted()) && impl.get() != m_selectedImpl) {
          // ensure we don't *also* shadow traffic to the selected impl, which
          // would pollute its history.
          m_shadowImpls.push_back(impl.get());
        }
      }
    }
  } else {
    if (m_selectedImpl) {
      response = m_selectedImpl->compressResponse(data, len, last);
      if (response.data() == nullptr) {
        m_selectedImpl = nullptr;
        raise_error(
            "Failed to compress a chunk. "
            "This is unrecoverable because previous chunks have already "
            "committed us to using this algorithm. :("
        );
      }
    }
  }

  // shadow the response
  for (auto *shadowImpl : m_shadowImpls) {
    try {
      shadowImpl->compressResponse(data, len, last);
    } catch (const std::runtime_error& ex) {
      // Don't let failures in shadowed code block execution.
      Logger::Error("Shadowed ResponseCompressor threw error: %s", ex.what());
    }
  }

  if (!m_chunkedEncoding &&
      response.size() + 50 /* estimated http header cost */ > len) {
    // If we still have freedom to decide, not compressing will produce a
    // better outcome. Do that.
    m_selectedImpl = nullptr;
    response = StringHolder{};
  }

  if (first) {
    setResponseHeaders();
  }

  return response;
}

void ResponseCompressorManager::setResponseHeaders() {
  if (isCompressed()) {
    auto encoding = encodingName();
    assertx(encoding && *encoding);
    m_headers->addHeader("Content-Encoding", encoding);
  }
  if (Cfg::Server::AddVaryEncoding) {
    /*
     * Our response may vary depending on the Accept-Encoding header if
     *  - we compressed it or
     *  - we didn't compress it because this client does not accept compression
     * (The alternative there being that we could have compressed it but made
     * our own choice not to, in which case it doesn't matter what the client
     * tells us it accepts.)
     */
    if (isCompressed() || (isEnabled() && !isAccepted())) {
      m_headers->addHeader("Vary", "Accept-Encoding");
    }
  }
  if (m_selectedImpl) {
    m_selectedImpl->setResponseHeaders();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
