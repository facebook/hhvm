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
#include <string>
#include <unordered_map>

#include <folly/Executor.h>
#include <folly/io/async/EventBase.h>
#include <proxygen/lib/http/session/test/HTTPTransactionMocks.h>

namespace apache::thrift {

/**
 * A simple response handler that collects the response from the channel
 * and makes it available to the test code.
 */
class FakeResponseHandler {
 public:
  explicit FakeResponseHandler(folly::EventBase* evb);

  ~FakeResponseHandler() = default;

  proxygen::MockHTTPTransaction* getTransaction() { return &txn_; }

  std::unordered_map<std::string, std::string>* getHeaders();

  folly::IOBuf* getBodyBuf();

  std::string getBody();

  bool eomReceived();

 private:
  folly::Executor::KeepAlive<folly::EventBase> evb_;

  proxygen::HTTP2PriorityQueue dummyEgressQueue_;
  proxygen::MockHTTPTransaction txn_;
  std::unordered_map<std::string, std::string> headers_;
  std::unique_ptr<folly::IOBuf> body_;
  bool eomReceived_{false};
};

} // namespace apache::thrift
