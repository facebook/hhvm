/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Memory.h>
#include <folly/compression/Compression.h>

#include <proxygen/httpserver/Filters.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>
#include <proxygen/lib/http/RFC2616.h>
#include <proxygen/lib/utils/StreamCompressor.h>
#include <proxygen/lib/utils/ZlibStreamCompressor.h>
#include <proxygen/lib/utils/ZstdStreamCompressor.h>

namespace proxygen {

/**
 * A Server filter to perform compression. If there are any errors it will
 * fall back to sending uncompressed responses.
 */
class CompressionFilter : public Filter {
 public:
  using StreamCompressorFactory =
      std::function<std::unique_ptr<StreamCompressor>()>;

  CompressionFilter(
      RequestHandler* downstream,
      uint32_t minimumCompressionSize,
      StreamCompressorFactory factory,
      std::string headerEncoding,
      const std::shared_ptr<std::set<std::string>> compressibleContentTypes)
      : Filter(downstream),
        minimumCompressionSize_(minimumCompressionSize),
        compressorFactory_(std::move(factory)),
        headerEncoding_(std::move(headerEncoding)),
        compressibleContentTypes_(compressibleContentTypes) {
  }

  virtual ~CompressionFilter() override {
  }

  void sendHeaders(HTTPMessage& msg) noexcept override {
    DCHECK(compressor_ == nullptr);
    DCHECK(header_ == false);

    chunked_ = msg.getIsChunked();

    // Skip if it is already compressed
    auto alreadyCompressed =
        !msg.getHeaders()
             .getSingleOrEmpty(HTTP_HEADER_CONTENT_ENCODING)
             .empty();

    // Make final determination of whether to compress
    compress_ = !alreadyCompressed && isCompressibleContentType(msg) &&
                (chunked_ || isMinimumCompressibleSize(msg));

    // Add the header
    if (compress_) {
      auto& headers = msg.getHeaders();
      headers.set(HTTP_HEADER_CONTENT_ENCODING, headerEncoding_);
    }

    // Initialize compressor
    compressor_ = compressorFactory_();
    if (!compressor_ || compressor_->hasError()) {
      return fail();
    }

    // If it's chunked or not being compressed then the headers can be sent
    // if it's compressed and one body, then need to calculate content length.
    if (chunked_ || !compress_) {
      Filter::sendHeaders(msg);
      header_ = true;
    } else {
      responseMessage_ = std::make_unique<HTTPMessage>(msg);
    }
  }

  void sendChunkHeader(size_t len) noexcept override {
    // The headers should have always been sent since the message is chunked
    DCHECK_EQ(header_, true) << "Headers should have already been sent.";

    // If not compressing, pass downstream, otherwise "swallow" it
    // to send after compressing the body.
    if (!compress_) {
      Filter::sendChunkHeader(len);
    }

    // Return without sending the chunk header.
    return;
  }

  // Compress the body, if chunked may be called multiple times
  void sendBody(std::unique_ptr<folly::IOBuf> body) noexcept override {
    // If not compressing, pass the body through
    if (!compress_) {
      DCHECK(header_ == true);
      Filter::sendBody(std::move(body));
      return;
    }

    CHECK(compressor_ && !compressor_->hasError());

    // If it's chunked, never write the trailer, it will be written on EOM
    auto compressed = compressor_->compress(body.get(), !chunked_);
    if (compressor_->hasError()) {
      return fail();
    }

    auto compressedBodyLength = compressed->computeChainDataLength();

    if (chunked_) {
      // Send on the swallowed chunk header.
      Filter::sendChunkHeader(compressedBodyLength);
    } else {
      // Send the content length on compressed, non-chunked messages
      DCHECK(header_ == false);
      DCHECK(compress_ == true);
      auto& headers = responseMessage_->getHeaders();
      headers.set(HTTP_HEADER_CONTENT_LENGTH,
                  folly::to<std::string>(compressedBodyLength));

      Filter::sendHeaders(*responseMessage_);
      header_ = true;
    }

    Filter::sendBody(std::move(compressed));
  }

  void sendEOM() noexcept override {

    // Need to send the trailer for compressed chunked messages
    if (compress_ && chunked_) {

      auto emptyBuffer = folly::IOBuf::copyBuffer("");
      CHECK(compressor_ && !compressor_->hasError());
      auto compressed = compressor_->compress(emptyBuffer.get(), true);

      if (compressor_->hasError()) {
        return fail();
      }

      // "Inject" a chunk with the trailer.
      Filter::sendChunkHeader(compressed->computeChainDataLength());
      Filter::sendBody(std::move(compressed));
      Filter::sendChunkTerminator();
    }

    Filter::sendEOM();
  }

 protected:
  void fail() {
    Filter::sendAbort();
  }

  // Verify the response is large enough to compress
  bool isMinimumCompressibleSize(const HTTPMessage& msg) const noexcept {
    auto contentLengthHeader =
        msg.getHeaders().getSingleOrEmpty(HTTP_HEADER_CONTENT_LENGTH);

    uint32_t contentLength = 0;
    if (!contentLengthHeader.empty()) {
      contentLength = folly::to<uint32_t>(contentLengthHeader);
    }

    return contentLength >= minimumCompressionSize_;
  }

  // Check the response's content type against a list of compressible types
  bool isCompressibleContentType(const HTTPMessage& msg) const noexcept {

    auto responseContentType =
        msg.getHeaders().getSingleOrEmpty(HTTP_HEADER_CONTENT_TYPE);
    folly::toLowerAscii(responseContentType);

    // Handle  text/html; encoding=utf-8 case
    auto parameter_idx = responseContentType.find(';');
    if (parameter_idx != std::string::npos) {
      responseContentType = responseContentType.substr(0, parameter_idx);
    }

    auto idx = compressibleContentTypes_->find(responseContentType);

    if (idx != compressibleContentTypes_->end()) {
      return true;
    }

    return false;
  }

  std::unique_ptr<HTTPMessage> responseMessage_;
  std::unique_ptr<StreamCompressor> compressor_{nullptr};
  const uint32_t minimumCompressionSize_{1000};
  StreamCompressorFactory compressorFactory_{};
  const std::string headerEncoding_{};
  const std::shared_ptr<std::set<std::string>> compressibleContentTypes_;
  bool header_{false};
  bool chunked_{false};
  bool compress_{false};
};

class CompressionFilterFactory : public RequestHandlerFactory {
 private:
  enum class CodecType : uint8_t {
    NO_COMPRESSION = 0,
    ZLIB = 1,
    ZSTD = 2,
  };

 public:
  struct Options {
    Options() = default;
    uint32_t minimumCompressionSize = 1000;
    std::set<std::string> compressibleContentTypes = {};
    int32_t zlibCompressionLevel = 4;
    int32_t zstdCompressionLevel = 8;
    bool enableZstd = false;
    bool independentChunks = false;
    bool enableGzip = true;
  };

  CompressionFilterFactory(const Options& opts)
      : minimumCompressionSize_(opts.minimumCompressionSize),
        zlibCompressionLevel_(opts.zlibCompressionLevel),
        zstdCompressionLevel_(opts.zstdCompressionLevel),
        compressibleContentTypes_(std::make_shared<std::set<std::string>>(
            opts.compressibleContentTypes)),
        enableZstd_(opts.enableZstd),
        independentChunks_(opts.independentChunks),
        enableGzip_(opts.enableGzip) {
  }

  virtual ~CompressionFilterFactory() {
  }

  void onServerStart(folly::EventBase* /*evb*/) noexcept override {
  }

  void onServerStop() noexcept override {
  }

  RequestHandler* onRequest(RequestHandler* h,
                            HTTPMessage* msg) noexcept override {
    switch (determineCompressionType(msg)) {
      case CodecType::ZLIB:
        return new CompressionFilter{
            h,
            minimumCompressionSize_,
            [level =
                 zlibCompressionLevel_]() -> std::unique_ptr<StreamCompressor> {
              return std::make_unique<ZlibStreamCompressor>(
                  proxygen::CompressionType::GZIP, level);
            },
            "gzip",
            compressibleContentTypes_};
      case CodecType::ZSTD:
        return new CompressionFilter{
            h,
            minimumCompressionSize_,
            [level = zstdCompressionLevel_, independent = independentChunks_]()
                -> std::unique_ptr<StreamCompressor> {
              return std::make_unique<ZstdStreamCompressor>(level, independent);
            },
            "zstd",
            compressibleContentTypes_};
      case CodecType::NO_COMPRESSION:
        return h;
    };
    return h;
  }

 private:
  // Check whether the client supports a compression type we support
  CodecType determineCompressionType(HTTPMessage* msg) noexcept {

    RFC2616::TokenPairVec output;

    // Accept encoding header could have qvalues (gzip; q=5.0)
    auto acceptEncodingHeader =
        msg->getHeaders().getSingleOrEmpty(HTTP_HEADER_ACCEPT_ENCODING);

    if (!RFC2616::parseQvalues(acceptEncodingHeader, output)) {
      return CodecType::NO_COMPRESSION;
    }

    auto it = std::find_if(
        output.begin(),
        output.end(),
        [enableZstd = enableZstd_,
         enableGzip = enableGzip_](RFC2616::TokenQPair elem) {
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

  const uint32_t minimumCompressionSize_;
  const int32_t zlibCompressionLevel_;
  const int32_t zstdCompressionLevel_;
  const std::shared_ptr<std::set<std::string>> compressibleContentTypes_;
  const bool enableZstd_;
  const bool independentChunks_;
  const bool enableGzip_;
};
} // namespace proxygen
