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

#include <thrift/lib/cpp2/server/LoggingEvent.h>

#include <folly/Indestructible.h>
#include <folly/Portability.h>
#include <folly/Random.h>
#include <folly/Synchronized.h>
#include <folly/io/async/AsyncSSLSocket.h>

namespace apache {
namespace thrift {
namespace {

class DefaultLoggingEventRegistry : public LoggingEventRegistry {
 public:
  ServerEventHandler& getServerEventHandler(std::string_view) const override {
    static auto* handler = new ServerEventHandler();
    return *handler;
  }
  ConnectionEventHandler& getConnectionEventHandler(
      std::string_view) const override {
    static auto* handler = new ConnectionEventHandler();
    return *handler;
  }
  ApplicationEventHandler& getApplicationEventHandler(
      std::string_view) const override {
    static auto* handler = new ApplicationEventHandler();
    return *handler;
  }
  ServerTrackerHandler& getServerTrackerHandler(
      std::string_view) const override {
    static auto* handler = new ServerTrackerHandler();
    return *handler;
  }
  RequestEventHandler& getRequestEventHandler(std::string_view) const override {
    static auto* handler = new RequestEventHandler();
    return *handler;
  }
};
} // namespace

namespace detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(
    std::unique_ptr<apache::thrift::LoggingEventRegistry>,
    makeLoggingEventRegistry) {
  return std::make_unique<DefaultLoggingEventRegistry>();
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    bool,
    isCertIPMismatch,
    const ConnectionLoggingContext&,
    const folly::AsyncTransportCertificate*) {
  return false;
}
} // namespace detail

namespace {
class Registry {
 public:
  Registry() : reg_(detail::makeLoggingEventRegistry()) {}

  LoggingEventRegistry& getRegistry() const { return *reg_.get(); }

 private:
  std::unique_ptr<LoggingEventRegistry> reg_;
};

} // namespace

bool LoggingSampler::shouldSample(SamplingRate samplingRate) {
  if (samplingRate <= 0) {
    return false;
  }
  return folly::Random::rand64(samplingRate) == 0;
}

const LoggingEventRegistry& getLoggingEventRegistry() {
  static folly::Indestructible<Registry> registryStorage;
  return registryStorage.get()->getRegistry();
}

} // namespace thrift
} // namespace apache
