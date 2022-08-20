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
#include <thrift/lib/cpp2/async/FutureRequest.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace thrift {
namespace py3 {
template <typename Result>
class FutureCallback : public apache::thrift::FutureCallbackBase<Result> {
 private:
  typedef folly::exception_wrapper (*Processor)(
      Result&, apache::thrift::ClientReceiveState&);
  Processor processor_;
  apache::thrift::RpcOptions& options_;

 public:
  FutureCallback(
      folly::Promise<Result>&& promise,
      apache::thrift::RpcOptions& options,
      Processor processor,
      std::shared_ptr<apache::thrift::RequestChannel> channel = nullptr)
      : apache::thrift::FutureCallbackBase<Result>(
            std::move(promise), std::move(channel)),
        processor_(processor),
        options_(options) {}

  void replyReceived(apache::thrift::ClientReceiveState&& state) override {
    CHECK(!state.isException());
    CHECK(state.hasResponseBuffer());

    Result result;
    auto ew = processor_(result, state);

    if (state.header() && !state.header()->getHeaders().empty()) {
      options_.setReadHeaders(state.header()->releaseHeaders());
    }

    if (ew) {
      this->promise_.setException(ew);
    } else {
      this->promise_.setValue(std::move(result));
    }
  }

  bool isInlineSafe() const override { return true; }
};

template <>
class FutureCallback<folly::Unit>
    : public apache::thrift::FutureCallbackBase<folly::Unit> {
 private:
  typedef folly::exception_wrapper (*Processor)(
      apache::thrift::ClientReceiveState&);
  Processor processor_;
  apache::thrift::RpcOptions& options_;

 public:
  FutureCallback(
      folly::Promise<folly::Unit>&& promise,
      apache::thrift::RpcOptions& options,
      Processor processor,
      std::shared_ptr<apache::thrift::RequestChannel> channel = nullptr)
      : apache::thrift::FutureCallbackBase<folly::Unit>(
            std::move(promise), std::move(channel)),
        processor_(processor),
        options_(options) {}

  void replyReceived(apache::thrift::ClientReceiveState&& state) override {
    CHECK(!state.isException());
    CHECK(state.hasResponseBuffer());

    auto ew = processor_(state);

    if (state.header() && !state.header()->getHeaders().empty()) {
      options_.setReadHeaders(state.header()->releaseHeaders());
    }

    if (ew) {
      promise_.setException(ew);
    } else {
      promise_.setValue();
    }
  }

  bool isInlineSafe() const override { return true; }
};

} // namespace py3
} // namespace thrift
