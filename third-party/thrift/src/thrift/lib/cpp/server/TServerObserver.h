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

#include <thrift/lib/cpp2/server/Overload.h>

namespace apache::thrift::server {

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
      if (writeBegin != clock::time_point() &&
          processEnd != clock::time_point()) {
        return to_microseconds(writeBegin - processEnd);
      }
      return {};
    }

    folly::Optional<uint64_t> writeLatencyUsec() const {
      if (writeBegin != clock::time_point() &&
          writeEnd != clock::time_point()) {
        return to_microseconds(writeEnd - writeBegin);
      }
      return {};
    }
  };

  // This class is used to pass information regarding a connection to
  // TServerObserver callbacks.
  //  - `connectionId` uniquely identifies a connection; connections with the
  //  same ID are considered the same connection.
  //  - `securityProtocol` indicates the security protocol being used; possible
  //  values include TLS12, Fizz, stopTLS, and kTLS.
  class ConnectionInfo {
   public:
    ConnectionInfo(uint64_t connectionId, const std::string& securityProtocol)
        : connectionId_(connectionId), securityProtocol_(securityProtocol) {}
    uint64_t getConnectionId() const { return connectionId_; }
    const std::string& getSecurityProtocol() const { return securityProtocol_; }

   private:
    uint64_t connectionId_;
    std::string securityProtocol_;
  };

  virtual void connAccepted(
      const wangle::TransportInfo&, const ConnectionInfo&) {}

  virtual void connDropped() {}

  virtual void connRejected() {}

  virtual void connClosed(const ConnectionInfo&) {}

  virtual void activeConnections(int32_t /*numConnections*/) {}

  virtual void tlsError() {}

  virtual void tlsComplete() {}

  virtual void tlsFallback() {}

  virtual void tlsResumption() {}

  virtual void taskKilled() {}

  virtual void taskTimeout() {}

  virtual void serverOverloaded(apache::thrift::LoadShedder /*loadShedder*/) {}

  virtual void receivedRequest(const std::string* /*method*/) {}

  virtual void admittedRequest(const std::string* /*method*/) {}

  virtual void queuedRequests(int32_t /*numRequests*/) {}

  virtual void queueTimeout() {}

  virtual void sentReply() {}

  virtual void activeRequests(int32_t /*numRequests*/) {}

  virtual void callCompleted(const CallTimestamps& /*runtimes*/) {}

  virtual void protocolError() {}

  virtual void tlsWithClientCert() {}

  virtual void declaredException() {}

  virtual void undeclaredException() {}

  virtual void resourcePoolsEnabled(const std::string& /*explanation*/) {}

  virtual void resourcePoolsDisabled(const std::string& /*explanation*/) {}

  virtual void resourcePoolsInitialized(
      const std::vector<std::string>& /*resourcePoolsDescriptions*/) {}

  virtual void pendingConnections(int32_t /*numPendingConnections*/) {}

  virtual void quotaExceeded() {}

  // The observer has to specify a sample rate for callCompleted notifications
  inline uint32_t getSampleRate() const { return sampleRate_; }

  virtual std::string getName() const final {
    return folly::demangle(typeid(*this)).toStdString();
  }

 protected:
  uint32_t sampleRate_;
};

class TServerObserverFactory {
 public:
  virtual std::shared_ptr<TServerObserver> getObserver() = 0;
  virtual ~TServerObserverFactory() {}
};

extern std::shared_ptr<server::TServerObserverFactory> observerFactory_;

} // namespace apache::thrift::server

#endif
