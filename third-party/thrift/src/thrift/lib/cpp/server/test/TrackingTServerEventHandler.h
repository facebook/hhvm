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

#include <vector>

#include <folly/Synchronized.h>

#include <thrift/lib/cpp/server/TServerEventHandler.h>

namespace apache::thrift::server::test {

class TrackingTServerEventHandler : public TServerEventHandler {
 private:
  folly::Synchronized<std::vector<std::string>> history_;

 public:
  std::vector<std::string> getHistory() const { return history_.copy(); }

  void preStart(const folly::SocketAddress* address) override;
  void preServe(const folly::SocketAddress* address) override;
  void handleServeError(const std::exception& exception) override;
  void newConnection(TConnectionContext* context) override;
  void connectionDestroyed(TConnectionContext* context) override;
  void postStop() override;
};

} // namespace apache::thrift::server::test
