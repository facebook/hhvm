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

#include <chrono>

#include <gmock/gmock.h>

#include <thrift/lib/cpp2/server/ServerConfigs.h>

namespace apache::thrift::server::test {

class MockServerConfigs : public apache::thrift::server::ServerConfigs {
 public:
  MOCK_METHOD(uint64_t, getMaxResponseSize, (), (const, override));
  MOCK_METHOD(
      bool,
      getTaskExpireTimeForRequest,
      (std::chrono::milliseconds clientQueueTimeoutMs,
       std::chrono::milliseconds clientTimeoutMs,
       std::chrono::milliseconds& queueTimeout,
       std::chrono::milliseconds& taskTimeout),
      (const, override));
  MOCK_METHOD(
      apache::thrift::server::TServerObserver*,
      getObserver,
      (),
      (const, override));
  MOCK_METHOD(
      apache::thrift::AdaptiveConcurrencyController&,
      getAdaptiveConcurrencyController,
      (),
      (override));
  MOCK_METHOD(
      const apache::thrift::AdaptiveConcurrencyController&,
      getAdaptiveConcurrencyController,
      (),
      (const, override));
  MOCK_METHOD(
      apache::thrift::CPUConcurrencyController&,
      getCPUConcurrencyController,
      (),
      (override));
  MOCK_METHOD(
      const apache::thrift::CPUConcurrencyController&,
      getCPUConcurrencyController,
      (),
      (const, override));
  MOCK_METHOD(size_t, getNumIOWorkerThreads, (), (const, override));
  MOCK_METHOD(
      std::chrono::milliseconds, getStreamExpireTime, (), (const, override));
  MOCK_METHOD(int64_t, getLoad, (const std::string&, bool), (const, override));

  // using ErrorCodeAndMessage = std::pair<std::string, std::string>;
  MOCK_METHOD(
      folly::Optional<ErrorCodeAndMessage>,
      checkOverload,
      (const apache::thrift::transport::THeader::StringToStringMap*,
       const std::string* method),
      (override));
  MOCK_METHOD(
      apache::thrift::PreprocessResult,
      preprocess,
      (const apache::thrift::server::PreprocessParams&),
      (const, override));
  MOCK_METHOD(bool, getTosReflect, (), (const, override));
  MOCK_METHOD(uint32_t, getListenerTos, (), (const, override));
  MOCK_METHOD(uint32_t, getMaxRequests, (), (const, override));
  MOCK_METHOD(void, setMaxRequests, (uint32_t), (override));
  MOCK_METHOD(uint32_t, getMaxQps, (), (const, override));
  MOCK_METHOD(void, setMaxQps, (uint32_t), (override));
  MOCK_METHOD(
      std::shared_ptr<folly::Executor>,
      getThreadManager,
      (),
      (const, override));
  MOCK_METHOD(
      std::shared_ptr<apache::thrift::concurrency::ThreadManager>,
      getThreadManager_deprecated,
      (),
      (const, override));
  MOCK_METHOD(
      std::shared_ptr<folly::Executor>,
      getHandlerExecutor_deprecated,
      (),
      (const, override));
  MOCK_METHOD(
      folly::Executor::KeepAlive<>,
      getHandlerExecutorKeepAlive,
      (),
      (const, override));
  MOCK_METHOD(
      std::chrono::milliseconds, getQueueTimeout, (), (const, override));
  MOCK_METHOD(uint32_t, getQueueTimeoutPct, (), (const, override));
  MOCK_METHOD(bool, getUseClientTimeout, (), (const, override));
  MOCK_METHOD(
      std::chrono::milliseconds, getTaskExpireTime, (), (const, override));
};

} // namespace apache::thrift::server::test
