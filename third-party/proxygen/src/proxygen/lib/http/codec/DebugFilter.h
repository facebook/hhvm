/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/container/EvictingCacheMap.h>
#include <proxygen/lib/http/codec/HTTPCodecFilter.h>
#include <proxygen/lib/utils/Logging.h>

namespace proxygen {
class DebugFilter : public PassThroughHTTPCodecFilter {
 public:
  explicit DebugFilter(
      std::string traceHeaderName,
      size_t maxBufSize = 10000,
      std::function<void(std::unique_ptr<folly::IOBuf>)> dumpFn = nullptr)
      : traceHeaderName_(std::move(traceHeaderName)),
        maxBufSize_(maxBufSize),
        dumpFn_(std::move(dumpFn)) {
  }

 private:
  size_t onIngress(const folly::IOBuf& buf) override {
    ingressBuffer_.append(buf.clone());
    if (ingressBuffer_.chainLength() > maxBufSize_) {
      ingressBuffer_.trimStart(ingressBuffer_.chainLength() - maxBufSize_);
    }
    return call_->onIngress(buf);
  }

  void onHeadersComplete(HTTPCodec::StreamID streamID,
                         std::unique_ptr<HTTPMessage> msg) override {
    if (shouldTrackStream(*msg)) {
      trackingStreams_.insert(streamID, getCurrentTime());
    }
    callback_->onHeadersComplete(streamID, std::move(msg));
  }

  size_t generateRstStream(folly::IOBufQueue& writeBuf,
                           StreamID streamID,
                           ErrorCode code) override {
    if (shouldDumpStream(streamID)) {
      VLOG(2) << "generateRstStream, streamID=" << streamID
              << " error=" << getErrorCodeString(code);
      dump();
    }
    return call_->generateRstStream(writeBuf, streamID, code);
  }

  size_t generateGoaway(
      folly::IOBufQueue& writeBuf,
      StreamID lastStream = MaxStreamID,
      ErrorCode code = ErrorCode::NO_ERROR,
      std::unique_ptr<folly::IOBuf> debugData = nullptr) override {
    if (code != ErrorCode::NO_ERROR) {
      VLOG(2) << "generateGoaway, lastStream=" << lastStream
              << " error=" << getErrorCodeString(code);
      dump();
    }
    return call_->generateGoaway(
        writeBuf, lastStream, code, std::move(debugData));
  }

  bool shouldDumpStream(HTTPCodec::StreamID streamID) {
    return (streamID == 0 ||
            trackingStreams_.find(streamID) != trackingStreams_.end());
  }

  bool shouldTrackStream(const HTTPMessage& msg) {
    return msg.getHeaders().exists(traceHeaderName_);
  }

  void onAbort(StreamID streamID, ErrorCode code) override {
    if (shouldDumpStream(streamID)) {
      VLOG(2) << "onAbort, streamID=" << streamID
              << " error=" << getErrorCodeString(code);
      dump();
    }
    callback_->onAbort(streamID, code);
  }

  void onError(HTTPCodec::StreamID streamID,
               const HTTPException& error,
               bool newTxn) override {
    // newTxn
    if (shouldDumpStream(streamID) ||
        (error.getPartialMsg() && shouldTrackStream(*error.getPartialMsg()))) {
      VLOG(2) << "onError, streamID=" << streamID << " error=" << error.what();
      dump();
    }
    callback_->onError(streamID, error, newTxn);
  }

  void onGoaway(uint64_t lastStream,
                ErrorCode code,
                std::unique_ptr<folly::IOBuf> debugData) override {
    if (code != ErrorCode::NO_ERROR) {
      VLOG(2) << "onGoaway, lastStream=" << lastStream
              << " error=" << getErrorCodeString(code)
              << " debugData=" << IOBufPrinter::printHexFolly(debugData.get());
      dump();
    }
    callback_->onGoaway(lastStream, code, std::move(debugData));
  }

  void dump() {
    auto ingress = ingressBuffer_.move();
    if (ingress) {
      if (dumpFn_) {
        dumpFn_(std::move(ingress));
      } else {
        VLOG(2) << IOBufPrinter::printHexFolly(ingress.get());
      }
    }
  }

  constexpr static size_t kMaxTrackingStreams = 100;

  std::string traceHeaderName_;
  size_t maxBufSize_{10000};
  folly::EvictingCacheMap<HTTPCodec::StreamID,
                          std::chrono::steady_clock::time_point>
      trackingStreams_{kMaxTrackingStreams};
  folly::IOBufQueue ingressBuffer_{folly::IOBufQueue::cacheChainLength()};
  std::function<void(std::unique_ptr<folly::IOBuf>)> dumpFn_;
};

} // namespace proxygen
