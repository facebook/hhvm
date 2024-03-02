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

#include <string>

#include <folly/experimental/observer/SimpleObservable.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/server/AdaptiveConcurrency.h>
#include <thrift/lib/cpp2/server/CPUConcurrencyController.h>
#include <thrift/lib/cpp2/server/ServerConfigs.h>
#include <thrift/lib/cpp2/server/ThriftServerConfig.h>
#include <thrift/lib/cpp2/transport/core/testutil/FakeServerObserver.h>

namespace apache {
namespace thrift {
namespace server {

// Use instance of this class, instead of ThriftServer, in the unit tests of
// ThriftProcessor.
class ServerConfigsMock : public ServerConfigs {
 public:
  uint64_t getMaxResponseSize() const override {
    return thriftServerConfig_.getMaxResponseSize().get();
  }

  /**
   * @see BaseThriftServer::getTaskExpireTimeForRequest function.
   */
  bool getTaskExpireTimeForRequest(
      std::chrono::milliseconds,
      std::chrono::milliseconds,
      std::chrono::milliseconds& queueTimeout,
      std::chrono::milliseconds& taskTimeout) const override {
    queueTimeout = queueTimeout_;
    taskTimeout = taskTimeout_;
    return queueTimeout == taskTimeout;
  }

  server::TServerObserver* getObserver() const override {
    return observer_.get();
  }

  size_t getNumIOWorkerThreads() const override { return numIOWorkerThreads_; }

  std::chrono::milliseconds getStreamExpireTime() const override {
    return streamExpireTime_;
  }

  int64_t getLoad(const std::string& /* counter */, bool /* check_custom */)
      const override {
    return 123;
  }

  folly::Optional<ServerConfigs::ErrorCodeAndMessage> checkOverload(
      const transport::THeader::StringToStringMap*,
      const std::string*) override {
    return {};
  }

  PreprocessResult preprocess(const server::PreprocessParams&) const override {
    return {};
  }

  bool getTosReflect() const override { return false; }

  AdaptiveConcurrencyController& getAdaptiveConcurrencyController() override {
    return adaptiveConcurrencyController_;
  }
  const AdaptiveConcurrencyController& getAdaptiveConcurrencyController()
      const override {
    return adaptiveConcurrencyController_;
  }

  apache::thrift::CPUConcurrencyController& getCPUConcurrencyController()
      override {
    return cpuConcurrencyController_;
  }

  const apache::thrift::CPUConcurrencyController& getCPUConcurrencyController()
      const override {
    return cpuConcurrencyController_;
  }

  uint32_t getMaxRequests() const override {
    return thriftServerConfig_.getMaxRequests().get();
  }

  void setMaxRequests(uint32_t maxRequests) override {
    thriftServerConfig_.setMaxRequests(
        folly::observer::makeStaticObserver(std::optional{maxRequests}),
        AttributeSource::OVERRIDE);
  }

  uint32_t getMaxQps() const override {
    return thriftServerConfig_.getMaxQps().get();
  }

  void setMaxQps(uint32_t maxQps) override {
    thriftServerConfig_.setMaxQps(
        folly::observer::makeStaticObserver(std::optional{maxQps}),
        AttributeSource::OVERRIDE);
  }

  uint32_t getListenerTos() const override { return 0; }

  std::shared_ptr<concurrency::ThreadManager> getThreadManager_deprecated()
      const override {
    return {};
  }

  std::shared_ptr<folly::Executor> getThreadManager() const override {
    return {};
  }

  std::shared_ptr<folly::Executor> getHandlerExecutor_deprecated()
      const override {
    return {};
  }

  folly::Executor::KeepAlive<> getHandlerExecutorKeepAlive() const override {
    return {};
  }

  std::chrono::milliseconds getQueueTimeout() const override {
    return queueTimeout_;
  }

  uint32_t getQueueTimeoutPct() const override {
    return thriftServerConfig_.getQueueTimeoutPct().get();
  }

  bool getUseClientTimeout() const override {
    return thriftServerConfig_.getUseClientTimeout().get();
  }

  std::chrono::milliseconds getTaskExpireTime() const override {
    return taskTimeout_;
  }

 public:
  uint64_t maxResponseSize_{0};
  std::chrono::milliseconds queueTimeout_{std::chrono::milliseconds(500)};
  std::chrono::milliseconds taskTimeout_{std::chrono::milliseconds(500)};
  std::shared_ptr<server::TServerObserver> observer_{
      std::make_shared<FakeServerObserver>()};
  size_t numIOWorkerThreads_{10};
  std::chrono::milliseconds streamExpireTime_{std::chrono::minutes(1)};

  ThriftServerConfig thriftServerConfig_{};
  folly::observer::SimpleObservable<AdaptiveConcurrencyController::Config>
      oConfig_{AdaptiveConcurrencyController::Config{}};
  AdaptiveConcurrencyController adaptiveConcurrencyController_{
      oConfig_.getObserver(),
      thriftServerConfig_.getMaxRequests().getObserver(),
      thriftServerConfig_};

  folly::observer::SimpleObservable<
      apache::thrift::CPUConcurrencyController::Config>
      cConfig_{apache::thrift::CPUConcurrencyController::Config{}};
  apache::thrift::CPUConcurrencyController cpuConcurrencyController_{
      cConfig_.getObserver(), *this, thriftServerConfig_};
};

} // namespace server
} // namespace thrift
} // namespace apache
