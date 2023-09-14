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
    std::set<std::string> compressibleContentTypes = {};
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
    std::set<std::string> compressibleContentTypes;
  };

  static folly::Optional<FilterParams> getFilterParams(
      const HTTPMessage& msg, const FactoryOptions& options) {
    switch (
        determineCompressionType(msg, options.enableZstd, options.enableGzip)) {
      case CodecType::ZLIB:
        return FilterParams{options.minimumCompressionSize,
                            [level = options.zlibCompressionLevel]()
                                -> std::unique_ptr<StreamCompressor> {
                              return std::make_unique<ZlibStreamCompressor>(
                                  proxygen::CompressionType::GZIP, level);
                            },
                            "gzip",
                            options.compressibleContentTypes};
      case CodecType::ZSTD:
        return FilterParams{options.minimumCompressionSize,
                            [level = options.zstdCompressionLevel,
                             independent = options.independentChunks]()
                                -> std::unique_ptr<StreamCompressor> {
                              return std::make_unique<ZstdStreamCompressor>(
                                  level, independent);
                            },
                            "zstd",
                            options.compressibleContentTypes};
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

    auto idx = params.compressibleContentTypes.find(responseContentType);

    if (idx != params.compressibleContentTypes.end()) {
      return true;
    }

    return false;
  }

 private:
  enum class CodecType : uint8_t {
    NO_COMPRESSION = 0,
    ZLIB = 1,
    ZSTD = 2,
  };
  static CodecType determineCompressionType(const HTTPMessage& msg,
                                            bool enableZstd,
                                            bool enableGzip) noexcept {

    RFC2616::TokenPairVec output;

    // Accept encoding header could have qvalues (gzip; q=5.0)
    auto acceptEncodingHeader =
        msg.getHeaders().getSingleOrEmpty(HTTP_HEADER_ACCEPT_ENCODING);

    if (!RFC2616::parseQvalues(acceptEncodingHeader, output)) {
      return CodecType::NO_COMPRESSION;
    }

    auto it = std::find_if(
        output.begin(),
        output.end(),
        [enableZstd, enableGzip](RFC2616::TokenQPair elem) {
          return (enableGzip &&
                  elem.first.compare(folly::StringPiece("gzip")) == 0) ||
                 (enableZstd &&
                  elem.first.compare(folly::StringPiece("zstd")) == 0);
        });

    if (it == output.end()) {
      return CodecType::NO_COMPRESSION;
    }
    if (it->first == "zstd") {
      return CodecType::ZSTD;
    } else if (it->first == "gzip") {
      return CodecType::ZLIB;
    } else {
      DCHECK(false) << "found unexpected content-coding selection";
      return CodecType::NO_COMPRESSION;
    }
  }
};
} // namespace proxygen
