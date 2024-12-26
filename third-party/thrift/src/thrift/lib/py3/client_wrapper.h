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

#include <memory>

#include <folly/futures/Future.h>
#include <folly/futures/Promise.h>
#include <thrift/lib/cpp2/async/AsyncClient.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace thrift::py3 {

class ClientWrapper {
 protected:
  std::unique_ptr<apache::thrift::GeneratedAsyncClient> async_client_;
  std::shared_ptr<apache::thrift::RequestChannel> channel_;

 public:
  explicit ClientWrapper(
      std::unique_ptr<apache::thrift::GeneratedAsyncClient> async_client,
      std::shared_ptr<apache::thrift::RequestChannel> channel)
      : async_client_(std::move(async_client)), channel_(std::move(channel)) {}
  virtual ~ClientWrapper() {
    auto eb = channel_->getEventBase();
    if (eb) {
      eb->runInEventBaseThread(
          [cha = std::move(channel_), cli = std::move(async_client_)] {});
    }
  }

  void setPersistentHeader(const std::string& key, const std::string& value) {
    auto headerChannel = async_client_->getHeaderChannel();
    if (headerChannel != nullptr) {
      headerChannel->setPersistentHeader(key, value);
    }
  }

  void addEventHandler(
      const std::shared_ptr<apache::thrift::TProcessorEventHandler>& handler) {
    async_client_->addEventHandler(handler);
  }
};

} // namespace thrift::py3
