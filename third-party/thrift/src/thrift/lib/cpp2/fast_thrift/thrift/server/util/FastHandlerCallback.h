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
#include <optional>
#include <type_traits>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftRequestContext.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseError.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponsePayloads.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

namespace detail {

template <typename U>
struct IsUniquePtr : std::false_type {};
template <typename U>
struct IsUniquePtr<std::unique_ptr<U>> : std::true_type {};

// Shared exception cascade — declared exception via insert_exn (success
// frame with declaredException metadata) vs undeclared (success frame with
// appUnknownException metadata). Templated on Presult/ProtocolWriter so the
// per-method types only need to flow in from the codegen instantiation
// site; the kHasReturnType bit is also a template arg because insert_exn
// itself is templated on it.
template <typename Presult, typename ProtocolWriter, bool HasReturnType>
inline void writeExceptionCascade(
    ThriftServerAppAdapter* a,
    uint32_t sid,
    folly::exception_wrapper ew) noexcept {
  Presult presult;
  std::optional<apache::thrift::ErrorClassification> classification;
  bool handled = ::apache::thrift::detail::ap::insert_exn<HasReturnType>(
      presult, ew, [&]<typename Ex>(Ex&) {
        classification = getDeclaredExceptionClassification<Ex>(ew);
      });
  if (handled) {
    a->writeResponse(
        makeDeclaredExceptionMessage<ProtocolWriter>(
            sid, presult, ew, classification));
  } else {
    a->writeResponse(makeUnknownExceptionMessage(
        sid, ew, apache::thrift::ErrorBlame::SERVER));
  }
}

} // namespace detail

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
      folly::EventBase* evb,
      std::unique_ptr<ThriftRequestContext> requestContext)
      : resultFn_(resultFn),
        exceptionFn_(exceptionFn),
        handler_(handler),
        adapterGuard_(handler),
        streamId_(streamId),
        evb_(evb),
        requestContext_(std::move(requestContext)) {}

  FastHandlerCallback(const FastHandlerCallback&) = delete;
  FastHandlerCallback& operator=(const FastHandlerCallback&) = delete;
  FastHandlerCallback(FastHandlerCallback&&) = delete;
  FastHandlerCallback& operator=(FastHandlerCallback&&) = delete;

  // Safe to call from any thread; the adapter's writeResponse handles the
  // EVB hop. Safe even after the connection has been force-closed: the
  // DG keeps the adapter alive, writeResponse drops on pipelineActive_.
  void result(T value) {
    completed_ = true;
    resultFn_(handler_, streamId_, std::move(value));
  }

  void exception(folly::exception_wrapper ew) {
    completed_ = true;
    exceptionFn_(handler_, streamId_, std::move(ew));
  }

  // True once result()/exception() has been invoked. Used by generated
  // dispatchers to avoid double-completing a callback when a user handler
  // both completes the callback and then throws synchronously.
  bool isCompleted() const noexcept { return completed_; }

  folly::EventBase* getEventBase() const { return evb_; }

  ThriftRequestContext* requestContext() const noexcept {
    return requestContext_.get();
  }

  // ---- Codegen-targeted static helpers ----
  // Codegen instantiates these with the per-method Presult / ProtocolWriter
  // and passes the function pointers to the ctor above. Each thunk builds
  // the response message via util/ResponsePayloads.h and hands it to the
  // adapter's single writeResponse entry point.

  template <typename Presult, typename ProtocolWriter>
  static void writeSuccess(
      ThriftServerAppAdapter* a, uint32_t sid, T value) noexcept {
    Presult presult;
    if constexpr (detail::IsUniquePtr<T>::value) {
      presult.template get<0>().value = value.get();
    } else {
      presult.template get<0>().value = &value;
    }
    presult.setIsSet(0, true);
    a->writeResponse(makeSuccessResponseMessage<ProtocolWriter>(sid, presult));
  }

  template <typename Presult, typename ProtocolWriter>
  static void writeException(
      ThriftServerAppAdapter* a,
      uint32_t sid,
      folly::exception_wrapper ew) noexcept {
    detail::
        writeExceptionCascade<Presult, ProtocolWriter, /*HasReturnType=*/true>(
            a, sid, std::move(ew));
  }

 protected:
  // Synthesize INTERNAL_ERROR if the user handler dropped the callback
  // without completing. exceptionFn_ → adapter->writeResponse hops to EVB
  // internally if we're off-thread.
  ~FastHandlerCallback() override {
    if (completed_) {
      return;
    }
    exceptionFn_(
        handler_,
        streamId_,
        folly::make_exception_wrapper<TApplicationException>(
            TApplicationException::INTERNAL_ERROR,
            "FastHandlerCallback not completed"));
  }

 private:
  ResultFn resultFn_;
  ExceptionFn exceptionFn_;
  ThriftServerAppAdapter* handler_;
  // Keeps the adapter alive as long as this callback exists, so a
  // straggler completing after force-close still has a valid adapter
  // to dispatch through. The adapter drops the write internally
  // because pipelineActive_ is false.
  folly::DelayedDestruction::DestructorGuard adapterGuard_;
  uint32_t streamId_;
  folly::EventBase* evb_;
  std::unique_ptr<ThriftRequestContext> requestContext_;
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
      folly::EventBase* evb,
      std::unique_ptr<ThriftRequestContext> requestContext)
      : doneFn_(doneFn),
        exceptionFn_(exceptionFn),
        handler_(handler),
        adapterGuard_(handler),
        streamId_(streamId),
        evb_(evb),
        requestContext_(std::move(requestContext)) {}

  FastHandlerCallback(const FastHandlerCallback&) = delete;
  FastHandlerCallback& operator=(const FastHandlerCallback&) = delete;
  FastHandlerCallback(FastHandlerCallback&&) = delete;
  FastHandlerCallback& operator=(FastHandlerCallback&&) = delete;

  // Safe to call from any thread; the adapter's writeResponse handles the
  // EVB hop.
  void done() {
    completed_ = true;
    doneFn_(handler_, streamId_);
  }

  void exception(folly::exception_wrapper ew) {
    completed_ = true;
    exceptionFn_(handler_, streamId_, std::move(ew));
  }

  // True once done()/exception() has been invoked. Used by generated
  // dispatchers to avoid double-completing a callback when a user handler
  // both completes the callback and then throws synchronously.
  bool isCompleted() const noexcept { return completed_; }

  folly::EventBase* getEventBase() const { return evb_; }

  ThriftRequestContext* requestContext() const noexcept {
    return requestContext_.get();
  }

  // ---- Codegen-targeted static helpers (void return) ----

  template <typename Presult, typename ProtocolWriter>
  static void writeDone(ThriftServerAppAdapter* a, uint32_t sid) noexcept {
    Presult presult;
    a->writeResponse(makeSuccessResponseMessage<ProtocolWriter>(sid, presult));
  }

  template <typename Presult, typename ProtocolWriter>
  static void writeException(
      ThriftServerAppAdapter* a,
      uint32_t sid,
      folly::exception_wrapper ew) noexcept {
    detail::
        writeExceptionCascade<Presult, ProtocolWriter, /*HasReturnType=*/false>(
            a, sid, std::move(ew));
  }

 protected:
  // See FastHandlerCallback<T>::~dtor.
  ~FastHandlerCallback() override {
    if (completed_) {
      return;
    }
    exceptionFn_(
        handler_,
        streamId_,
        folly::make_exception_wrapper<TApplicationException>(
            TApplicationException::INTERNAL_ERROR,
            "FastHandlerCallback not completed"));
  }

 private:
  DoneFn doneFn_;
  ExceptionFn exceptionFn_;
  ThriftServerAppAdapter* handler_;
  // See FastHandlerCallback<T>::adapterGuard_.
  folly::DelayedDestruction::DestructorGuard adapterGuard_;
  uint32_t streamId_;
  folly::EventBase* evb_;
  std::unique_ptr<ThriftRequestContext> requestContext_;
  bool completed_{false};
};

template <typename T>
using FastHandlerCallbackPtr =
    folly::DelayedDestructionUniquePtr<FastHandlerCallback<T>>;

} // namespace apache::thrift::fast_thrift::thrift
