/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include "proxygen/lib/http/coro/HTTPSourceFilter.h"
#include <folly/coro/Collect.h>
#include <folly/coro/Promise.h>
#include <folly/logging/xlog.h>
#include <proxygen/lib/sampling/Sampled.h>

namespace proxygen::coro {

class Logger {
 public:
  class SampledLogger {
   public:
    virtual ~SampledLogger() = default;
    /**
     * @param error   Whether there was an ingress or egress error.
     * @result        Weight of sample (will invoke ::log() if > 0). The
     *                returned weight value will be subsequently passed into
     *                ::log()
     */
    virtual uint32_t getLoggingWeight(bool error) {
      // default implementation is to log on error or 1/10,000
      if (error) {
        return 1;
      }
      proxygen::Sampling sampling(0.0001);
      return sampling.isLucky() ? sampling.getWeight() : 0;
    }

    virtual void log(const Logger& logger, uint32_t /*weight*/) {
      logger.logWithVlog(2);
    }
  };
  using SampledLoggerPtr = std::shared_ptr<SampledLogger>;

  explicit Logger(HTTPSessionContextPtr sessionCtx,
                  std::shared_ptr<SampledLogger> logImpl = nullptr,
                  bool logOnDestroy = true) {
    XCHECK(sessionCtx);
    sessionCtx_ = std::move(sessionCtx);
    localAddr = sessionCtx_->getLocalAddress();
    peerAddr = sessionCtx_->getPeerAddress();
    protocol = sessionCtx_->getCodecProtocol();
    sessionID = sessionCtx_->getSessionID();
    transportInfo = sessionCtx_->getSetupTransportInfo();
    startTime = std::chrono::steady_clock::now();
    if (logImpl) {
      logImpl_ = std::move(logImpl);
    } else {
      logImpl_ = std::make_shared<SampledLogger>();
    }
    logOnDestroy_ = logOnDestroy;
  }

  ~Logger() {
    if (logOnDestroy_) {
      log();
    }
  }

  static folly::coro::Task<void> logWhenDone(Logger& logger) {
    co_await folly::coro::collectAll(std::move(logger.reqFilter.done.second),
                                     std::move(logger.respFilter.done.second));
    logger.log();
  }

  void log() {
    logOnDestroy_ = false;
    const uint32_t weight =
        logImpl_->getLoggingWeight(reqFilter.error || respFilter.error);
    if (weight > 0) {
      if (sessionCtx_) {
        sessionCtx_->getCurrentTransportInfo(&transportInfo, false);
      }
      logImpl_->log(*this, weight);
    }
    sessionCtx_.reset();
  }

  void logWithVlog(int level) const;

  HTTPSourceFilter* getRequestFilter(HTTPSourceHolder source) {
    reqFilter.setSource(source.release());
    return &reqFilter;
  }

  HTTPSourceFilter* getResponseFilter(HTTPSourceHolder source) {
    respFilter.setSource(source.release());
    return &respFilter;
  }

  HTTPCodec::StreamID getStreamID() const {
    if (reqFilter.streamID) {
      return *reqFilter.streamID;
    } else if (respFilter.streamID) {
      return *respFilter.streamID;
    } else {
      return HTTPCodec::MaxStreamID;
    }
  }

  std::chrono::milliseconds timeToFirstHeaderByte() const {
    auto endTime = respFilter.finalHeaderTime ? *respFilter.finalHeaderTime
                                              : respFilter.endTime;
    return std::chrono::duration_cast<std::chrono::milliseconds>(endTime -
                                                                 startTime);
  }

  std::chrono::milliseconds timeToFirstByte() const {
    auto endTime = respFilter.firstByteTime ? *respFilter.firstByteTime
                                            : respFilter.endTime;
    return std::chrono::duration_cast<std::chrono::milliseconds>(endTime -
                                                                 startTime);
  }

  std::chrono::milliseconds timeToLastByte() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        respFilter.endTime - startTime);
  }

  std::string getSecurityType() const {
    return transportInfo.secure ? transportInfo.securityType
                                : std::string("plaintext");
  }

  std::string getAuthority() const {
    if (!reqFilter.host.empty()) {
      return reqFilter.host;
    }
    if (!reqFilter.url.empty()) {
      auto parseUrl = ParseURL::parseURL(reqFilter.url);
      if (parseUrl && parseUrl->hasHost()) {
        return std::string(parseUrl->host());
      }
    }
    return {};
  }

  std::string getPath() const {
    if (!reqFilter.url.empty()) {
      auto parseUrl = ParseURL::parseURL(reqFilter.url);
      if (parseUrl) {
        return std::string(parseUrl->path());
      }
    }
    return {};
  }

  class Filter : public HTTPSourceFilter {
   public:
    enum class Direction { REQUEST, RESPONSE };
    explicit Filter(Direction dir) : direction_(dir) {
      done = folly::coro::makePromiseContract<void>();
    }
    folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override;
    folly::coro::Task<HTTPBodyEvent> readBodyEvent(uint32_t max) override;
    void stopReading(folly::Optional<const HTTPErrorCode> error) override;

    void describe(std::ostream& os) const;

    folly::Optional<HTTPCodec::StreamID> streamID;
    folly::Optional<std::chrono::steady_clock::time_point> finalHeaderTime;
    folly::Optional<std::chrono::steady_clock::time_point> firstByteTime;
    std::chrono::steady_clock::time_point endTime;
    folly::Optional<HTTPError> error;
    folly::Optional<HTTPHeaderSize> headerSize;
    std::pair<uint8_t, uint8_t> httpVersion;
    HTTPPriority priority;
    std::string method;
    std::string host;
    std::string url;
    folly::Optional<uint16_t> statusCode;
    size_t bodyBytes{0};
    std::pair<folly::coro::Promise<void>, folly::coro::Future<void>> done;
    bool valid{false};
    bool ingress{false};

   private:
    Direction direction_;
  };

  Filter reqFilter{Filter::Direction::REQUEST};
  Filter respFilter{Filter::Direction::RESPONSE};

  // Fields from session
  folly::SocketAddress localAddr;
  folly::SocketAddress peerAddr;
  CodecProtocol protocol;
  uint64_t sessionID{0};
  std::chrono::steady_clock::time_point startTime;
  wangle::TransportInfo transportInfo;

 private:
  HTTPSessionContextPtr sessionCtx_;
  std::shared_ptr<SampledLogger> logImpl_;
  bool logOnDestroy_{true};
};

} // namespace proxygen::coro
