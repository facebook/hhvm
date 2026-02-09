/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/RFC2616.h>
#include <proxygen/lib/utils/StreamCompressor.h>
#include <proxygen/lib/utils/ZlibStreamCompressor.h>
#include <proxygen/lib/utils/ZstdStreamCompressor.h>

namespace proxygen {

class CompressionFilterUtils {
 public:
  struct FactoryOptions {
    FactoryOptions() = default;
    uint32_t minimumCompressionSize = 1000;
    std::shared_ptr<const std::set<std::string>> compressibleContentTypes;
    int32_t zlibCompressionLevel = 4;
    int32_t zstdCompressionLevel = 8;
    bool enableZstd = false;
    bool independentChunks = false;
    bool enableGzip = true;
  };

  using StreamCompressorFactory =
      std::function<std::unique_ptr<StreamCompressor>()>;

  struct FilterParams {
    uint32_t minimumCompressionSize;
    StreamCompressorFactory compressorFactory;
    std::string headerEncoding;
    std::shared_ptr<const std::set<std::string>> compressibleContentTypes;
  };

  static folly::Optional<FilterParams> getFilterParams(
      const HTTPMessage& msg, const FactoryOptions& options) {
    auto info =
        determineCompressionInfo(msg, options.enableZstd, options.enableGzip);
    switch (info.codecType) {
      case CodecType::ZLIB:
        return FilterParams{
            .minimumCompressionSize = options.minimumCompressionSize,
            .compressorFactory = [level = options.zlibCompressionLevel]()
                -> std::unique_ptr<StreamCompressor> {
              return std::make_unique<ZlibStreamCompressor>(
                  proxygen::CompressionType::GZIP, level);
            },
            .headerEncoding = "gzip",
            .compressibleContentTypes = options.compressibleContentTypes};
      case CodecType::ZSTD: {
        int32_t level = info.level.value_or(options.zstdCompressionLevel);
        return FilterParams{
            .minimumCompressionSize = options.minimumCompressionSize,
            .compressorFactory = [level,
                                  independent = options.independentChunks]()
                -> std::unique_ptr<StreamCompressor> {
              return std::make_unique<ZstdStreamCompressor>(level, independent);
            },
            .headerEncoding = "zstd",
            .compressibleContentTypes = options.compressibleContentTypes};
      }
      case CodecType::NO_COMPRESSION:
        return folly::none;
    }
    folly::assume_unreachable();
  }

  // Filter helpers

  static bool shouldCompress(const HTTPMessage& msg,
                             const FilterParams& params) {
    // Skip if it is already compressed
    auto alreadyCompressed =
        !msg.getHeaders()
             .getSingleOrEmpty(HTTP_HEADER_CONTENT_ENCODING)
             .empty();

    // Make final determination of whether to compress
    return !alreadyCompressed && isCompressibleContentType(msg, params) &&
           (msg.getIsChunked() || isMinimumCompressibleSize(msg, params));
  }

  // Verify the response is large enough to compress
  static bool isMinimumCompressibleSize(const HTTPMessage& msg,
                                        const FilterParams& params) {
    auto contentLengthHeader =
        msg.getHeaders().getSingleOrEmpty(HTTP_HEADER_CONTENT_LENGTH);

    uint32_t contentLength = 0;
    if (!contentLengthHeader.empty()) {
      contentLength = folly::to<uint32_t>(contentLengthHeader);
    }

    return contentLength >= params.minimumCompressionSize;
  }

  // Check the response's content type against a list of compressible types
  static bool isCompressibleContentType(const HTTPMessage& msg,
                                        const FilterParams& params) {

    auto responseContentType =
        msg.getHeaders().getSingleOrEmpty(HTTP_HEADER_CONTENT_TYPE);
    folly::toLowerAscii(responseContentType);

    // Handle  text/html; encoding=utf-8 case
    auto parameter_idx = responseContentType.find(';');
    if (parameter_idx != std::string::npos) {
      responseContentType = responseContentType.substr(0, parameter_idx);
    }

    if (params.compressibleContentTypes == nullptr) {
      return false;
    }
    auto it = params.compressibleContentTypes->find(responseContentType);
    return it != params.compressibleContentTypes->end();
  }

  enum class CodecType : uint8_t {
    NO_COMPRESSION = 0,
    ZLIB = 1,
    ZSTD = 2,
  };

  struct CompressionInfo {
    CodecType codecType{CodecType::NO_COMPRESSION};
    folly::Optional<int32_t> level;
  };

  static CodecType determineCompressionType(
      folly::StringPiece acceptEncodingHeader,
      bool enableZstd,
      bool enableGzip) noexcept {
    return determineCompressionInfo(
               acceptEncodingHeader, enableZstd, enableGzip)
        .codecType;
  }
  // Accept encoding header could have qvalues (gzip; q=5.0)
  // For zstd, compression level is encoded in the format zstd-N (e.g., zstd-8)
  static CompressionInfo determineCompressionInfo(
      folly::StringPiece acceptEncodingHeader,
      bool enableZstd,
      bool enableGzip) noexcept {
    CompressionInfo info;

    auto encodings = RFC2616::parseEncoding(acceptEncodingHeader);
    if (encodings.hasException()) {
      return info;
    }

    auto it = std::find_if(encodings.value().begin(),
                           encodings.value().end(),
                           [enableZstd, enableGzip](const auto& elem) {
                             const auto& encoding = elem.first;
                             return (enableGzip && encoding == "gzip") ||
                                    (enableZstd && encoding.startsWith("zstd"));
                           });

    if (it == encodings.value().end()) {
      return info;
    }

    const auto& encoding = it->first;

    if (encoding.startsWith("zstd")) {
      info.codecType = CodecType::ZSTD;
      info.level = parseLevelFromEncoding(encoding);
    } else if (encoding == "gzip") {
      info.codecType = CodecType::ZLIB;
    } else {
      DCHECK(false) << "found unexpected content-coding selection";
    }

    return info;
  }

 private:
  // Parse level from encoding name like "zstd-8"
  static folly::Optional<int32_t> parseLevelFromEncoding(
      folly::StringPiece encoding) noexcept {
    constexpr std::string_view kZstdPrefix = "zstd-";
    if (!encoding.startsWith(kZstdPrefix)) {
      return folly::none;
    }
    auto levelStr = encoding.subpiece(kZstdPrefix.size()); // Skip "zstd-"
    int32_t level = folly::tryTo<int32_t>(levelStr).value_or(0);
    // zstd valid levels are -5 to 22, excluding 0
    bool ok = level >= -5 && level <= 22 && level != 0;
    return ok ? folly::Optional<int32_t>(level) : folly::none;
  }

  static CompressionInfo determineCompressionInfo(const HTTPMessage& msg,
                                                  bool enableZstd,
                                                  bool enableGzip) noexcept {
    auto acceptEncodingHeader =
        msg.getHeaders().getSingleOrEmpty(HTTP_HEADER_ACCEPT_ENCODING);
    return determineCompressionInfo(
        acceptEncodingHeader, enableZstd, enableGzip);
  }
};
} // namespace proxygen
