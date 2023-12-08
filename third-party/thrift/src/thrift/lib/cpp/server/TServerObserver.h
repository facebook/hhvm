/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef THRIFT_SERVER_TSERVEROBSERVER_H_
#define THRIFT_SERVER_TSERVEROBSERVER_H_ 1

#include <stdint.h>

#include <chrono>
#include <memory>
#include <string>

#include <folly/Optional.h>
#include <wangle/acceptor/TransportInfo.h>

namespace apache {
namespace thrift {
namespace server {

class TServerObserver {
 public:
  virtual ~TServerObserver() {}

  TServerObserver() : sampleRate_(0) {}
  explicit TServerObserver(uint32_t sampleRate) : sampleRate_(sampleRate) {}

  class SamplingStatus {
   public:
    SamplingStatus() noexcept : SamplingStatus(false, 0, 0) {}
    explicit SamplingStatus(bool isServerSamplingEnabled) noexcept
        : isServerSamplingEnabled_(isServerSamplingEnabled),
          logSampleRatio_(0),
          logErrorSampleRatio_(0) {}
    SamplingStatus(
        bool isServerSamplingEnabled,
        int64_t logSampleRatio,
        int64_t logErrorSampleRatio) noexcept
        : isServerSamplingEnabled_(isServerSamplingEnabled),
          logSampleRatio_(std::max(int64_t{0}, logSampleRatio)),
          logErrorSampleRatio_(std::max(int64_t{0}, logErrorSampleRatio)) {}

    bool isEnabled() const {
      return isServerSamplingEnabled() || isRequestLoggingEnabled();
    }

    bool isServerSamplingEnabled() const { return isServerSamplingEnabled_; }
    bool isRequestLoggingEnabled() const {
      return logSampleRatio_ > 0 || logErrorSampleRatio_ > 0;
    }

    int64_t getLogSampleRatio() const { return logSampleRatio_; }
    int64_t getLogErrorSampleRatio() const { return logErrorSampleRatio_; }

   private:
    bool isServerSamplingEnabled_;
    int64_t logSampleRatio_;
    int64_t logErrorSampleRatio_;
  };

  class PreHandlerTimestamps {
   protected:
    using clock = std::chrono::steady_clock;
    using us = std::chrono::microseconds;

   public:
    static uint64_t to_microseconds(clock::duration dur) {
      return std::chrono::duration_cast<us>(dur).count();
    }
    static clock::time_point from_microseconds(uint64_t usec) {
      return clock::time_point() + us(usec);
    }

    clock::time_point readEnd;
    clock::time_point processBegin;

    folly::Optional<uint64_t> processDelayLatencyUsec() const {
      if (processBegin != clock::time_point()) {
        return to_microseconds(processBegin - readEnd);
      }
      return {};
    }

    void setStatus(const SamplingStatus& status) { status_ = status; }

    const SamplingStatus& getSamplingStatus() const { return status_; }

   private:
    SamplingStatus status_;
  };

  class CallTimestamps : public PreHandlerTimestamps {
   public:
    std::chrono::steady_clock::time_point processEnd;
    std::chrono::steady_clock::time_point writeBegin;
    std::chrono::steady_clock::time_point writeEnd;

    folly::Optional<uint64_t> processLatencyUsec() const {
      if (processBegin != clock::time_point() &&
          processEnd != clock::time_point()) {
        return to_microseconds(processEnd - processBegin);
      }
      return {};
    }

    folly::Optional<uint64_t> writeDelayLatencyUsec() const {
      if (writeBegin != clock::time_point()) {
        return to_microseconds(writeBegin - processEnd);
      }
      return {};
    }

    folly::Optional<uint64_t> writeLatencyUsec() const {
      if (writeBegin != clock::time_point()) {
        return to_microseconds(writeEnd - writeBegin);
      }
      return {};
    }
  };

  virtual void connAccepted(const wangle::TransportInfo&) {}

  virtual void connDropped() {}

  virtual void connRejected() {}

  virtual void connClosed() {}

  virtual void activeConnections(int32_t /*numConnections*/) {}

  virtual void tlsError() {}

  virtual void tlsComplete() {}

  virtual void tlsFallback() {}

  virtual void tlsResumption() {}

  virtual void taskKilled() {}

  virtual void taskTimeout() {}

  virtual void serverOverloaded() {}

  virtual void receivedRequest(const std::string* /*method*/) {}

  virtual void admittedRequest(const std::string* /*method*/) {}

  virtual void queuedRequests(int32_t /*numRequests*/) {}

  virtual void queueTimeout() {}

  virtual void shadowQueueTimeout() {}

  virtual void sentReply() {}

  virtual void activeRequests(int32_t /*numRequests*/) {}

  virtual void callCompleted(const CallTimestamps& /*runtimes*/) {}

  virtual void protocolError() {}

  virtual void tlsWithClientCert() {}

  virtual void declaredException() {}

  virtual void undeclaredException() {}

  virtual void resourcePoolsEnabled(std::string /*explanation*/) {}

  virtual void resourcePoolsDisabled(std::string /*explanation*/) {}

  virtual void resourcePoolsInitialized(
      std::vector<std::string> /*resourcePoolsDescriptions*/) {}

  // The observer has to specify a sample rate for callCompleted notifications
  inline uint32_t getSampleRate() const { return sampleRate_; }

 protected:
  uint32_t sampleRate_;
};

class TServerObserverFactory {
 public:
  virtual std::shared_ptr<TServerObserver> getObserver() = 0;
  virtual ~TServerObserverFactory() {}
};

extern std::shared_ptr<server::TServerObserverFactory> observerFactory_;

} // namespace server
} // namespace thrift
} // namespace apache

#endif
