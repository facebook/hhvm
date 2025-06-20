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

#include <type_traits>
#include <utility>

#include <folly/CancellationToken.h>
#include <folly/Expected.h>
#include <folly/futures/Future.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace apache::thrift {

namespace detail {

template <class Result>
class FutureCallbackHelper {
 public:
  using PromiseResult = folly::Expected<
      std::pair<Result, ClientReceiveState>,
      std::pair<folly::exception_wrapper, ClientReceiveState>>;

  static Result extractResult(PromiseResult&& result) {
    if (result.hasValue()) {
      return std::move(std::move(result).value().first);
    }
    std::move(result).error().first.throw_exception();
  }

  static ClientReceiveState&& extractClientReceiveState(
      PromiseResult& result) noexcept {
    if (result.hasValue()) {
      return std::move(std::move(result).value().second);
    }
    return std::move(result).error().second;
  }

  using CallbackProcessorType = std::conditional_t<
      std::is_same_v<Result, folly::Unit>,
      folly::exception_wrapper(ClientReceiveState&),
      folly::exception_wrapper(Result&, ClientReceiveState&)>;

  static void invokeCallbackProcessor(
      CallbackProcessorType& processor,
      folly::exception_wrapper& ew,
      Result& result,
      ClientReceiveState& state) {
    if constexpr (std::is_same_v<Result, folly::Unit>) {
      ew = processor(state);
    } else {
      ew = processor(result, state);
    }
  }

  static PromiseResult makeResult(Result&& result, ClientReceiveState&& state) {
    return PromiseResult(std::pair{std::move(result), std::move(state)});
  }

  static folly::Unexpected<folly::ExpectedErrorType<PromiseResult>> makeError(
      folly::exception_wrapper&& ew, ClientReceiveState&& state) {
    return folly::makeUnexpected(std::pair{std::move(ew), std::move(state)});
  }

  static folly::Try<Result> processClientInterceptorsAndExtractResult(
      PromiseResult&& result) noexcept {
    apache::thrift::ClientReceiveState clientReceiveState =
        extractClientReceiveState(result);
    auto* contextStack = clientReceiveState.ctx();
    auto* header = clientReceiveState.header();
    if (contextStack != nullptr) {
      if (auto exTry =
              contextStack->processClientInterceptorsOnResponse(header);
          exTry.hasException()) {
        return folly::Try<Result>(std::move(exTry).exception());
      }
    }
    return folly::makeTryWith([&] { return extractResult(std::move(result)); });
  }
};

inline void runClientInterceptorsInHeaderSemiFutureCallback(
    const ClientReceiveState& state) {
  auto* contextStack = state.ctx();
  auto* header = state.header();
  if (contextStack != nullptr) {
    contextStack->processClientInterceptorsOnResponse(header)
        .throwUnlessValue();
  }
}

} // namespace detail

template <typename Result>
class FutureCallbackBase : public RequestCallback {
 public:
  using Helper = detail::FutureCallbackHelper<Result>;

  explicit FutureCallbackBase(
      folly::Promise<typename Helper::PromiseResult>&& promise,
      std::shared_ptr<apache::thrift::RequestChannel> channel = nullptr)
      : promise_(std::move(promise)), channel_(std::move(channel)) {}

  void requestSent() override {}

  void requestError(ClientReceiveState&& state) override {
    CHECK(state.isException());
    folly::exception_wrapper& ew = state.exception();
    promise_.setValue(Helper::makeError(std::move(ew), std::move(state)));
  }

 protected:
  folly::Promise<typename Helper::PromiseResult> promise_;
  std::shared_ptr<apache::thrift::RequestChannel> channel_;
};

template <typename Result>
class FutureCallback : public FutureCallbackBase<Result> {
 private:
  using Helper = typename FutureCallbackBase<Result>::Helper;
  using Processor = typename Helper::CallbackProcessorType;

 public:
  FutureCallback(
      folly::Promise<typename Helper::PromiseResult>&& promise,
      Processor& processor,
      std::shared_ptr<apache::thrift::RequestChannel> channel = nullptr)
      : FutureCallbackBase<Result>(std::move(promise), std::move(channel)),
        processor_(processor) {}

  void replyReceived(ClientReceiveState&& state) override {
    CHECK(!state.isException());
    CHECK(state.hasResponseBuffer());

    Result result;
    folly::exception_wrapper ew;
    Helper::invokeCallbackProcessor(processor_, ew, result, state);

    if (ew) {
      this->promise_.setValue(
          Helper::makeError(std::move(ew), std::move(state)));
    } else {
      this->promise_.setValue(
          Helper::makeResult(std::move(result), std::move(state)));
    }
  }

 private:
  Processor& processor_;
};

template <typename Result>
class HeaderFutureCallback
    : public FutureCallbackBase<std::pair<
          Result,
          std::unique_ptr<apache::thrift::transport::THeader>>> {
 private:
  using HeaderResult =
      std::pair<Result, std::unique_ptr<apache::thrift::transport::THeader>>;
  using Helper = detail::FutureCallbackHelper<HeaderResult>;
  using InnerHelper = detail::FutureCallbackHelper<Result>;
  using Processor = typename InnerHelper::CallbackProcessorType;
  Processor& processor_;

 public:
  HeaderFutureCallback(
      folly::Promise<typename Helper::PromiseResult>&& promise,
      Processor& processor,
      std::shared_ptr<apache::thrift::RequestChannel> channel = nullptr)
      : FutureCallbackBase<HeaderResult>(
            std::move(promise), std::move(channel)),
        processor_(processor) {}

  void replyReceived(ClientReceiveState&& state) override {
    CHECK(!state.isException());
    CHECK(state.hasResponseBuffer());

    Result result;
    folly::exception_wrapper ew;
    InnerHelper::invokeCallbackProcessor(processor_, ew, result, state);

    if (ew) {
      this->promise_.setValue(
          Helper::makeError(std::move(ew), std::move(state)));
    } else {
      auto header = state.extractHeader();
      this->promise_.setValue(Helper::makeResult(
          std::pair{std::move(result), std::move(header)}, std::move(state)));
    }
  }
};

class OneWayFutureCallback : public FutureCallbackBase<folly::Unit> {
 private:
  using Helper = detail::FutureCallbackHelper<folly::Unit>;

 public:
  explicit OneWayFutureCallback(
      folly::Promise<Helper::PromiseResult>&& promise,
      std::shared_ptr<apache::thrift::RequestChannel> channel = nullptr)
      : FutureCallbackBase<folly::Unit>(
            std::move(promise), std::move(channel)) {}

  void requestSent() override {
    promise_.setValue(Helper::makeResult(folly::Unit(), ClientReceiveState()));
  }

  void replyReceived(ClientReceiveState&& /*state*/) override { CHECK(false); }
};

template <typename Result>
class SemiFutureCallback : public FutureCallback<Result> {
 public:
  using FutureCallback<Result>::FutureCallback;

  bool isInlineSafe() const override { return true; }
};

class OneWaySemiFutureCallback : public OneWayFutureCallback {
 public:
  using OneWayFutureCallback::OneWayFutureCallback;

  bool isInlineSafe() const override { return true; }
};

template <typename Result>
class HeaderSemiFutureCallback : public HeaderFutureCallback<Result> {
 public:
  using HeaderFutureCallback<Result>::HeaderFutureCallback;

  bool isInlineSafe() const override { return true; }
};

class LegacySemiFutureCallback : public RequestCallback {
 public:
  template <typename Result>
  using Processor = folly::exception_wrapper (*)(Result&, ClientReceiveState&);
  using ProcessorVoid = folly::exception_wrapper (*)(ClientReceiveState&);

  explicit LegacySemiFutureCallback(
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

class LegacyOneWaySemiFutureCallback : public RequestCallback {
 public:
  LegacyOneWaySemiFutureCallback(
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
std::pair<std::unique_ptr<LegacySemiFutureCallback>, folly::SemiFuture<Result>>
makeSemiFutureCallback(
    LegacySemiFutureCallback::Processor<Result> processor,
    std::shared_ptr<apache::thrift::RequestChannel> channel) {
  folly::Promise<ClientReceiveState> promise;
  auto future = promise.getSemiFuture();

  return {
      std::make_unique<LegacySemiFutureCallback>(
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

inline std::pair<
    std::unique_ptr<LegacySemiFutureCallback>,
    folly::SemiFuture<folly::Unit>>
makeSemiFutureCallback(
    LegacySemiFutureCallback::ProcessorVoid processor,
    std::shared_ptr<apache::thrift::RequestChannel> channel) {
  folly::Promise<ClientReceiveState> promise;
  auto future = promise.getSemiFuture();

  return {
      std::make_unique<LegacySemiFutureCallback>(
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
    std::unique_ptr<LegacySemiFutureCallback>,
    folly::SemiFuture<
        std::pair<Result, std::unique_ptr<apache::thrift::transport::THeader>>>>
makeHeaderSemiFutureCallback(
    LegacySemiFutureCallback::Processor<Result> processor,
    std::shared_ptr<apache::thrift::RequestChannel> channel) {
  folly::Promise<ClientReceiveState> promise;
  auto future = promise.getSemiFuture();

  return {
      std::make_unique<LegacySemiFutureCallback>(
          std::move(promise), std::move(channel)),
      std::move(future).deferValue([processor](ClientReceiveState&& state) {
        CHECK(!state.isException());
        CHECK(state.hasResponseBuffer());

        Result result;
        auto ew = processor(result, state);

        if (ew) {
          ew.throw_exception();
        }

        detail::runClientInterceptorsInHeaderSemiFutureCallback(state);

        return std::make_pair(std::move(result), state.extractHeader());
      })};
}

inline std::pair<
    std::unique_ptr<LegacySemiFutureCallback>,
    folly::SemiFuture<std::pair<
        folly::Unit,
        std::unique_ptr<apache::thrift::transport::THeader>>>>
makeHeaderSemiFutureCallback(
    LegacySemiFutureCallback::ProcessorVoid processor,
    std::shared_ptr<apache::thrift::RequestChannel> channel) {
  folly::Promise<ClientReceiveState> promise;
  auto future = promise.getSemiFuture();

  return {
      std::make_unique<LegacySemiFutureCallback>(
          std::move(promise), std::move(channel)),
      std::move(future).deferValue([processor](ClientReceiveState&& state) {
        CHECK(!state.isException());
        CHECK(state.hasResponseBuffer());

        auto ew = processor(state);

        if (ew) {
          ew.throw_exception();
        }

        detail::runClientInterceptorsInHeaderSemiFutureCallback(state);

        return std::make_pair(folly::unit, state.extractHeader());
      })};
}

inline std::pair<
    std::unique_ptr<LegacyOneWaySemiFutureCallback>,
    folly::SemiFuture<folly::Unit>>
makeOneWaySemiFutureCallback(
    std::shared_ptr<apache::thrift::RequestChannel> channel) {
  folly::Promise<folly::Unit> promise;
  auto future = promise.getSemiFuture();
  return {
      std::make_unique<LegacyOneWaySemiFutureCallback>(
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

} // namespace apache::thrift
