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

#pragma once

#include <atomic>
#include <memory>
#include <string_view>
#include <utility>

#include <folly/Function.h>
#include <folly/dynamic.h>
#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache {
namespace thrift {

#define THRIFT_LOGGING_EVENT(KEY, FETCH_FUNC)                      \
  ([]() -> auto& {                                                 \
    static auto& handler =                                         \
        apache::thrift::getLoggingEventRegistry().FETCH_FUNC(KEY); \
    return handler;                                                \
  }())

#define THRIFT_SERVER_EVENT(NAME) \
  THRIFT_LOGGING_EVENT(#NAME, getServerEventHandler)

#define THRIFT_CONNECTION_EVENT(NAME) \
  THRIFT_LOGGING_EVENT(#NAME, getConnectionEventHandler)

#define THRIFT_APPLICATION_EVENT(NAME) \
  THRIFT_LOGGING_EVENT(#NAME, getApplicationEventHandler)

class ThriftServer;
class Cpp2Worker;
class Cpp2ConnContext;

namespace instrumentation {
class ServerTracker;
} // namespace instrumentation

class LoggingSampler {
 public:
  using SamplingRate = int64_t;
  explicit LoggingSampler(SamplingRate samplingRate)
      : samplingRate_{samplingRate}, isSampled_{shouldSample(samplingRate)} {}

  SamplingRate getSamplingRate() const { return samplingRate_; }

  bool isSampled() const { return isSampled_; }
  explicit operator bool() const { return isSampled(); }

  static bool shouldSample(SamplingRate);

 private:
  SamplingRate samplingRate_;
  bool isSampled_;
};

class LoggingEventHandler {
 public:
  using DynamicFieldsCallback = folly::FunctionRef<folly::dynamic()>;
  using LoggingSampler = apache::thrift::LoggingSampler;
  using SamplingRate = LoggingSampler::SamplingRate;
  virtual ~LoggingEventHandler() {}
};

class ServerEventHandler : public LoggingEventHandler {
 public:
  virtual void log(const ThriftServer&, DynamicFieldsCallback = {}) {}
  virtual ~ServerEventHandler() override {}
};

using ConnectionLoggingContext = Cpp2ConnContext;
class ConnectionEventHandler : public LoggingEventHandler {
 public:
  virtual ~ConnectionEventHandler() override {}

  virtual void log(
      const ConnectionLoggingContext&, DynamicFieldsCallback = {}) {}

  virtual void logSampled(
      const ConnectionLoggingContext&,
      SamplingRate /* presampledRate */,
      DynamicFieldsCallback = {}) {}
  void logSampled(
      const ConnectionLoggingContext& context,
      const LoggingSampler& loggingSampler,
      DynamicFieldsCallback callback = {}) {
    DCHECK(loggingSampler.isSampled())
        << "logSampled should not be called if sampling did not pass";
    logSampled(context, loggingSampler.getSamplingRate(), std::move(callback));
  }
};

class ApplicationEventHandler : public LoggingEventHandler {
 public:
  virtual void log(DynamicFieldsCallback = {}) {}
  virtual ~ApplicationEventHandler() override {}
};

class ServerTrackerHandler {
 public:
  virtual void log(const instrumentation::ServerTracker&) {}
  virtual ~ServerTrackerHandler() {}
};

struct RequestLoggingContext {
  server::TServerObserver::CallTimestamps timestamps;
  // for application exception (a.k.a user exception), e.g. user defined thrift
  // exception
  std::optional<PayloadExceptionMetadataBase> exceptionMetaData;
  // for thrift internal exception, e.g. OVERLOAD, QUEUE_TIMEOUT,
  // UNKNOWN_METHOD.
  std::optional<ResponseRpcError> responseRpcError;
  std::string methodName;
  std::string clientId;
  // request id passed from the client
  std::string requestId;
  bool requestStartedProcessing;
};

class RequestEventHandler : public LoggingEventHandler {
 public:
  virtual void log(const RequestLoggingContext&) {}
  virtual void logSampled(SamplingRate, const RequestLoggingContext&) {}
  virtual ~RequestEventHandler() override {}
};

class LoggingEventRegistry {
 public:
  virtual ServerEventHandler& getServerEventHandler(
      std::string_view eventKey) const = 0;
  virtual ConnectionEventHandler& getConnectionEventHandler(
      std::string_view eventKey) const = 0;
  virtual ApplicationEventHandler& getApplicationEventHandler(
      std::string_view eventKey) const = 0;
  virtual ServerTrackerHandler& getServerTrackerHandler(
      std::string_view trackerKey) const = 0;
  virtual RequestEventHandler& getRequestEventHandler(
      std::string_view eventKey) const = 0;

  virtual ~LoggingEventRegistry() {}
};

namespace detail {
THRIFT_PLUGGABLE_FUNC_DECLARE(
    std::unique_ptr<apache::thrift::LoggingEventRegistry>,
    makeLoggingEventRegistry);

THRIFT_PLUGGABLE_FUNC_DECLARE(
    bool,
    isCertIPMismatch,
    const ConnectionLoggingContext& ctx,
    const folly::AsyncTransportCertificate* cert);
} // namespace detail

const LoggingEventRegistry& getLoggingEventRegistry();

void useMockLoggingEventRegistry();

} // namespace thrift
} // namespace apache
