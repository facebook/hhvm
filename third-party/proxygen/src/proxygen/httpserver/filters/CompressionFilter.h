/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/httpserver/Filters.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>
#include <proxygen/lib/utils/CompressionFilterUtils.h>

namespace proxygen {

/**
 * A Server filter to perform compression. If there are any errors it will
 * fall back to sending uncompressed responses.
 */
class CompressionFilter : public Filter {
 public:
  CompressionFilter(RequestHandler* downstream,
                    CompressionFilterUtils::FilterParams params)
      : Filter(downstream), params_(std::move(params)) {
  }

  virtual ~CompressionFilter() override {
  }

  void sendHeaders(HTTPMessage& msg) noexcept override {
    DCHECK(compressor_ == nullptr);
    DCHECK(header_ == false);

    chunked_ = msg.getIsChunked();
    compress_ = CompressionFilterUtils::shouldCompress(msg, params_);

    // Add the header
    if (compress_) {
      auto& headers = msg.getHeaders();
      headers.set(HTTP_HEADER_CONTENT_ENCODING, params_.headerEncoding);
    }

    // Initialize compressor
    compressor_ = params_.compressorFactory();
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
      folly::IOBuf emptyBuf{};
      CHECK(compressor_ && !compressor_->hasError());
      auto compressed = compressor_->compress(&emptyBuf, true);

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

  std::unique_ptr<HTTPMessage> responseMessage_;
  std::unique_ptr<StreamCompressor> compressor_{nullptr};
  CompressionFilterUtils::FilterParams params_;
  bool header_{false};
  bool chunked_{false};
  bool compress_{false};
};

class CompressionFilterFactory : public RequestHandlerFactory {
 public:
  using Options = CompressionFilterUtils::FactoryOptions;

  CompressionFilterFactory(const Options& opts) : options_(opts) {
  }

  virtual ~CompressionFilterFactory() {
  }

  void onServerStart(folly::EventBase* /*evb*/) noexcept override {
  }

  void onServerStop() noexcept override {
  }

  RequestHandler* onRequest(RequestHandler* h,
                            HTTPMessage* msg) noexcept override {
    auto filterParams = CompressionFilterUtils::getFilterParams(*msg, options_);
    if (!filterParams) {
      return h;
    }
    return new CompressionFilter(h, std::move(*filterParams));
  }

 private:
  const Options options_;
};
} // namespace proxygen
