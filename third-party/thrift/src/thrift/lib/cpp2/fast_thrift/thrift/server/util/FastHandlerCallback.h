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

#include <folly/ExceptionWrapper.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp/TApplicationException.h>

namespace apache::thrift::fast_thrift::thrift {

class ThriftServerAppAdapter;

template <typename T>
class FastHandlerCallback : public folly::DelayedDestruction {
 public:
  using ResultFn = void (*)(ThriftServerAppAdapter*, uint32_t, T);
  using ExceptionFn =
      void (*)(ThriftServerAppAdapter*, uint32_t, folly::exception_wrapper);

  FastHandlerCallback(
      ResultFn resultFn,
      ExceptionFn exceptionFn,
      ThriftServerAppAdapter* handler,
      uint32_t streamId,
      folly::EventBase* evb)
      : resultFn_(resultFn),
        exceptionFn_(exceptionFn),
        handler_(handler),
        streamId_(streamId),
        evb_(evb) {}

  FastHandlerCallback(const FastHandlerCallback&) = delete;
  FastHandlerCallback& operator=(const FastHandlerCallback&) = delete;
  FastHandlerCallback(FastHandlerCallback&&) = delete;
  FastHandlerCallback& operator=(FastHandlerCallback&&) = delete;

  void result(T value) {
    completed_ = true;
    resultFn_(handler_, streamId_, std::move(value));
  }

  void exception(folly::exception_wrapper ew) {
    completed_ = true;
    exceptionFn_(handler_, streamId_, std::move(ew));
  }

  folly::EventBase* getEventBase() const { return evb_; }

 protected:
  ~FastHandlerCallback() override {
    if (!completed_) {
      exceptionFn_(
          handler_,
          streamId_,
          folly::make_exception_wrapper<TApplicationException>(
              TApplicationException::INTERNAL_ERROR,
              "FastHandlerCallback not completed"));
    }
  }

 private:
  ResultFn resultFn_;
  ExceptionFn exceptionFn_;
  ThriftServerAppAdapter* handler_;
  uint32_t streamId_;
  folly::EventBase* evb_;
  bool completed_{false};
};

template <>
class FastHandlerCallback<void> : public folly::DelayedDestruction {
 public:
  using DoneFn = void (*)(ThriftServerAppAdapter*, uint32_t);
  using ExceptionFn =
      void (*)(ThriftServerAppAdapter*, uint32_t, folly::exception_wrapper);

  FastHandlerCallback(
      DoneFn doneFn,
      ExceptionFn exceptionFn,
      ThriftServerAppAdapter* handler,
      uint32_t streamId,
      folly::EventBase* evb)
      : doneFn_(doneFn),
        exceptionFn_(exceptionFn),
        handler_(handler),
        streamId_(streamId),
        evb_(evb) {}

  FastHandlerCallback(const FastHandlerCallback&) = delete;
  FastHandlerCallback& operator=(const FastHandlerCallback&) = delete;
  FastHandlerCallback(FastHandlerCallback&&) = delete;
  FastHandlerCallback& operator=(FastHandlerCallback&&) = delete;

  void done() {
    completed_ = true;
    doneFn_(handler_, streamId_);
  }

  void exception(folly::exception_wrapper ew) {
    completed_ = true;
    exceptionFn_(handler_, streamId_, std::move(ew));
  }

  folly::EventBase* getEventBase() const { return evb_; }

 protected:
  ~FastHandlerCallback() override {
    if (!completed_) {
      exceptionFn_(
          handler_,
          streamId_,
          folly::make_exception_wrapper<TApplicationException>(
              TApplicationException::INTERNAL_ERROR,
              "FastHandlerCallback not completed"));
    }
  }

 private:
  DoneFn doneFn_;
  ExceptionFn exceptionFn_;
  ThriftServerAppAdapter* handler_;
  uint32_t streamId_;
  folly::EventBase* evb_;
  bool completed_{false};
};

template <typename T>
using FastHandlerCallbackPtr =
    folly::DelayedDestructionUniquePtr<FastHandlerCallback<T>>;

} // namespace apache::thrift::fast_thrift::thrift
