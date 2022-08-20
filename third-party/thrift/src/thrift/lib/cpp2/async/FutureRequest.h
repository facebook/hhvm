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

#include <folly/CancellationToken.h>
#include <folly/futures/Future.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace apache {
namespace thrift {

template <typename Result>
class FutureCallbackBase : public RequestCallback {
 public:
  explicit FutureCallbackBase(
      folly::Promise<Result>&& promise,
      std::shared_ptr<apache::thrift::RequestChannel> channel = nullptr)
      : promise_(std::move(promise)), channel_(std::move(channel)) {}

  void requestSent() override {}

  void requestError(ClientReceiveState&& state) override {
    CHECK(state.isException());
    promise_.setException(std::move(state.exception()));
  }

 protected:
  folly::Promise<Result> promise_;
  std::shared_ptr<apache::thrift::RequestChannel> channel_;
};

template <typename Result>
class FutureCallback : public FutureCallbackBase<Result> {
 private:
  typedef folly::exception_wrapper (*Processor)(Result&, ClientReceiveState&);

 public:
  FutureCallback(
      folly::Promise<Result>&& promise,
      Processor processor,
      std::shared_ptr<apache::thrift::RequestChannel> channel = nullptr)
      : FutureCallbackBase<Result>(std::move(promise), std::move(channel)),
        processor_(processor) {}

  void replyReceived(ClientReceiveState&& state) override {
    CHECK(!state.isException());
    CHECK(state.hasResponseBuffer());

    Result result;
    auto ew = processor_(result, state);

    if (ew) {
      this->promise_.setException(ew);
    } else {
      this->promise_.setValue(std::move(result));
    }
  }

 private:
  Processor processor_;
};

template <typename Result>
class HeaderFutureCallback
    : public FutureCallbackBase<std::pair<
          Result,
          std::unique_ptr<apache::thrift::transport::THeader>>> {
 private:
  using HeaderResult =
      std::pair<Result, std::unique_ptr<apache::thrift::transport::THeader>>;
  typedef folly::exception_wrapper (*Processor)(Result&, ClientReceiveState&);
  Processor processor_;

 public:
  HeaderFutureCallback(
      folly::Promise<HeaderResult>&& promise,
      Processor processor,
      std::shared_ptr<apache::thrift::RequestChannel> channel = nullptr)
      : FutureCallbackBase<HeaderResult>(
            std::move(promise), std::move(channel)),
        processor_(processor) {}

  void replyReceived(ClientReceiveState&& state) override {
    CHECK(!state.isException());
    CHECK(state.hasResponseBuffer());

    Result result;
    auto ew = processor_(result, state);

    if (ew) {
      this->promise_.setException(ew);
    } else {
      this->promise_.setValue(
          std::make_pair(std::move(result), state.extractHeader()));
    }
  }
};

template <>
class HeaderFutureCallback<folly::Unit>
    : public FutureCallbackBase<std::pair<
          folly::Unit,
          std::unique_ptr<apache::thrift::transport::THeader>>> {
 private:
  using HeaderResult = std::
      pair<folly::Unit, std::unique_ptr<apache::thrift::transport::THeader>>;
  typedef folly::exception_wrapper (*Processor)(ClientReceiveState&);
  Processor processor_;

 public:
  HeaderFutureCallback(
      folly::Promise<HeaderResult>&& promise,
      Processor processor,
      std::shared_ptr<apache::thrift::RequestChannel> channel = nullptr)
      : FutureCallbackBase<HeaderResult>(
            std::move(promise), std::move(channel)),
        processor_(processor) {}

  void replyReceived(ClientReceiveState&& state) override {
    CHECK(!state.isException());
    CHECK(state.hasResponseBuffer());

    auto ew = processor_(state);

    if (ew) {
      promise_.setException(ew);
    } else {
      promise_.setValue(std::make_pair(folly::Unit(), state.extractHeader()));
    }
  }
};

class OneWayFutureCallback : public FutureCallbackBase<folly::Unit> {
 public:
  explicit OneWayFutureCallback(
      folly::Promise<folly::Unit>&& promise,
      std::shared_ptr<apache::thrift::RequestChannel> channel = nullptr)
      : FutureCallbackBase<folly::Unit>(
            std::move(promise), std::move(channel)) {}

  void requestSent() override { promise_.setValue(); }

  void replyReceived(ClientReceiveState&& /*state*/) override { CHECK(false); }
};

template <>
class FutureCallback<folly::Unit> : public FutureCallbackBase<folly::Unit> {
 private:
  typedef folly::exception_wrapper (*Processor)(ClientReceiveState&);

 public:
  FutureCallback(
      folly::Promise<folly::Unit>&& promise,
      Processor processor,
      std::shared_ptr<apache::thrift::RequestChannel> channel = nullptr)
      : FutureCallbackBase<folly::Unit>(std::move(promise), std::move(channel)),
        processor_(processor) {}

  void replyReceived(ClientReceiveState&& state) override {
    CHECK(!state.isException());
    CHECK(state.hasResponseBuffer());

    auto ew = processor_(state);

    if (ew) {
      promise_.setException(ew);
    } else {
      promise_.setValue();
    }
  }

 private:
  Processor processor_;
};

class SemiFutureCallback : public RequestCallback {
 public:
  template <typename Result>
  using Processor = folly::exception_wrapper (*)(Result&, ClientReceiveState&);
  using ProcessorVoid = folly::exception_wrapper (*)(ClientReceiveState&);

  explicit SemiFutureCallback(
      folly::Promise<ClientReceiveState>&& promise,
      std::shared_ptr<apache::thrift::RequestChannel> channel)
      : promise_(std::move(promise)), channel_(std::move(channel)) {}

  void requestSent() override {}

  void replyReceived(ClientReceiveState&& state) override {
    promise_.setValue(std::move(state));
  }

  void requestError(ClientReceiveState&& state) override {
    promise_.setException(std::move(state.exception()));
  }

  bool isInlineSafe() const override { return true; }

 protected:
  folly::Promise<ClientReceiveState> promise_;
  std::shared_ptr<apache::thrift::RequestChannel> channel_;
};

class OneWaySemiFutureCallback : public RequestCallback {
 public:
  OneWaySemiFutureCallback(
      folly::Promise<folly::Unit>&& promise,
      std::shared_ptr<apache::thrift::RequestChannel> channel)
      : promise_(std::move(promise)), channel_(std::move(channel)) {}

  void requestSent() override { promise_.setValue(); }

  void replyReceived(ClientReceiveState&&) override { CHECK(false); }

  void requestError(ClientReceiveState&& state) override {
    promise_.setException(std::move(state.exception()));
  }

  bool isInlineSafe() const override { return true; }

 protected:
  folly::Promise<folly::Unit> promise_;
  std::shared_ptr<apache::thrift::RequestChannel> channel_;
};

template <typename Result>
std::pair<std::unique_ptr<SemiFutureCallback>, folly::SemiFuture<Result>>
makeSemiFutureCallback(
    SemiFutureCallback::Processor<Result> processor,
    std::shared_ptr<apache::thrift::RequestChannel> channel) {
  folly::Promise<ClientReceiveState> promise;
  auto future = promise.getSemiFuture();

  return {
      std::make_unique<SemiFutureCallback>(
          std::move(promise), std::move(channel)),
      std::move(future).deferValue([processor](ClientReceiveState&& state) {
        CHECK(!state.isException());
        CHECK(state.hasResponseBuffer());

        Result result;
        auto ew = processor(result, state);

        if (ew) {
          ew.throw_exception();
        }
        return result;
      })};
}

inline std::
    pair<std::unique_ptr<SemiFutureCallback>, folly::SemiFuture<folly::Unit>>
    makeSemiFutureCallback(
        SemiFutureCallback::ProcessorVoid processor,
        std::shared_ptr<apache::thrift::RequestChannel> channel) {
  folly::Promise<ClientReceiveState> promise;
  auto future = promise.getSemiFuture();

  return {
      std::make_unique<SemiFutureCallback>(
          std::move(promise), std::move(channel)),
      std::move(future).deferValue([processor](ClientReceiveState&& state) {
        CHECK(!state.isException());
        CHECK(state.hasResponseBuffer());

        auto ew = processor(state);

        if (ew) {
          ew.throw_exception();
        }
      })};
}

template <typename Result>
std::pair<
    std::unique_ptr<SemiFutureCallback>,
    folly::SemiFuture<
        std::pair<Result, std::unique_ptr<apache::thrift::transport::THeader>>>>
makeHeaderSemiFutureCallback(
    SemiFutureCallback::Processor<Result> processor,
    std::shared_ptr<apache::thrift::RequestChannel> channel) {
  folly::Promise<ClientReceiveState> promise;
  auto future = promise.getSemiFuture();

  return {
      std::make_unique<SemiFutureCallback>(
          std::move(promise), std::move(channel)),
      std::move(future).deferValue([processor](ClientReceiveState&& state) {
        CHECK(!state.isException());
        CHECK(state.hasResponseBuffer());

        Result result;
        auto ew = processor(result, state);

        if (ew) {
          ew.throw_exception();
        }
        return std::make_pair(std::move(result), state.extractHeader());
      })};
}

inline std::pair<
    std::unique_ptr<SemiFutureCallback>,
    folly::SemiFuture<std::pair<
        folly::Unit,
        std::unique_ptr<apache::thrift::transport::THeader>>>>
makeHeaderSemiFutureCallback(
    SemiFutureCallback::ProcessorVoid processor,
    std::shared_ptr<apache::thrift::RequestChannel> channel) {
  folly::Promise<ClientReceiveState> promise;
  auto future = promise.getSemiFuture();

  return {
      std::make_unique<SemiFutureCallback>(
          std::move(promise), std::move(channel)),
      std::move(future).deferValue([processor](ClientReceiveState&& state) {
        CHECK(!state.isException());
        CHECK(state.hasResponseBuffer());

        auto ew = processor(state);

        if (ew) {
          ew.throw_exception();
        }

        return std::make_pair(folly::unit, state.extractHeader());
      })};
}

inline std::pair<
    std::unique_ptr<OneWaySemiFutureCallback>,
    folly::SemiFuture<folly::Unit>>
makeOneWaySemiFutureCallback(
    std::shared_ptr<apache::thrift::RequestChannel> channel) {
  folly::Promise<folly::Unit> promise;
  auto future = promise.getSemiFuture();
  return {
      std::make_unique<OneWaySemiFutureCallback>(
          std::move(promise), std::move(channel)),
      std::move(future)};
}

template <bool oneWay>
class CancellableRequestClientCallback : public RequestClientCallback {
  CancellableRequestClientCallback(
      RequestClientCallback* wrapped, std::shared_ptr<RequestChannel> channel)
      : callback_(wrapped), channel_(std::move(channel)) {
    DCHECK(wrapped->isInlineSafe());
  }

 public:
  static std::unique_ptr<CancellableRequestClientCallback> create(
      RequestClientCallback* wrapped, std::shared_ptr<RequestChannel> channel) {
    return std::unique_ptr<CancellableRequestClientCallback>(
        new CancellableRequestClientCallback(wrapped, std::move(channel)));
  }

  static void cancel(std::unique_ptr<CancellableRequestClientCallback> cb) {
    cb.release()->onResponseError(
        folly::make_exception_wrapper<folly::OperationCancelled>());
  }

  void onResponse(ClientReceiveState&& state) noexcept override {
    if (auto callback =
            callback_.exchange(nullptr, std::memory_order_acq_rel)) {
      callback->onResponse(std::move(state));
    } else {
      delete this;
    }
  }
  void onResponseError(folly::exception_wrapper ew) noexcept override {
    if (auto callback =
            callback_.exchange(nullptr, std::memory_order_acq_rel)) {
      callback->onResponseError(std::move(ew));
    } else {
      delete this;
    }
  }

  bool isInlineSafe() const override { return true; }

 private:
  std::atomic<RequestClientCallback*> callback_;
  std::shared_ptr<RequestChannel> channel_;
};
} // namespace thrift
} // namespace apache
