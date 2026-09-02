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

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/Executor.h>
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

/**
 * Sole-ownership handle for a FastHandlerCallback.
 *
 * Move-only on purpose. A request has exactly one owner at any instant and
 * ownership is handed along a chain — dispatcher, then CPU pool task, then
 * handler, then possibly a continuation — with each hand-off carrying the
 * happens-before edge of whatever moved it (the executor queue, a future's
 * fulfilment, an EventBase notification queue). Two threads never hold the
 * callback at once, so nothing here needs to be atomic.
 *
 * Making the handle uncopyable is what enforces that: a second simultaneous
 * owner would reintroduce the "who destroys it?" question that a reference
 * count exists to answer, and there is deliberately no reference count.
 */
template <typename T>
class CallbackPtr {
 public:
  CallbackPtr() = default;
  explicit CallbackPtr(T* ptr) noexcept : ptr_(ptr) {}

  CallbackPtr(const CallbackPtr&) = delete;
  CallbackPtr& operator=(const CallbackPtr&) = delete;

  CallbackPtr(CallbackPtr&& other) noexcept
      : ptr_(std::exchange(other.ptr_, nullptr)) {}
  CallbackPtr& operator=(CallbackPtr&& other) noexcept {
    if (this != &other) {
      reset();
      ptr_ = std::exchange(other.ptr_, nullptr);
    }
    return *this;
  }

  ~CallbackPtr() { reset(); }

  void reset() noexcept {
    if (auto* ptr = std::exchange(ptr_, nullptr)) {
      ptr->destroyOnEventBase();
    }
  }

  // Yields the pointer without destroying it, for a hand-off that cannot carry
  // the handle itself. The caller takes on the destroy-on-EventBase
  // obligation, so ownership is still held by exactly one party.
  [[nodiscard]] T* release() noexcept { return std::exchange(ptr_, nullptr); }

  T* get() const noexcept { return ptr_; }
  T* operator->() const noexcept { return ptr_; }
  T& operator*() const noexcept { return *ptr_; }
  explicit operator bool() const noexcept { return ptr_ != nullptr; }

  friend bool operator==(const CallbackPtr& ptr, std::nullptr_t) noexcept {
    return ptr.ptr_ == nullptr;
  }

 private:
  T* ptr_{nullptr};
};

// what() materializes a std::string, so it can throw under memory pressure.
// The callers are noexcept, so an allocation failure degrades to an empty
// reason rather than terminating the process.
inline std::string exceptionMessage(
    const folly::exception_wrapper& ew) noexcept {
  try {
    return ew.what().toStdString();
  } catch (...) {
    return {};
  }
}

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
    folly::DelayedDestruction::DestructorGuard&& adapterGuard,
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
            sid, presult, ew, classification),
        std::move(adapterGuard));
  } else {
    a->writeResponse(
        makeUnknownExceptionMessage(
            sid, ew, apache::thrift::ErrorBlame::SERVER),
        std::move(adapterGuard));
  }
}

} // namespace detail

/**
 * Per-request completion handle handed to a FastServiceHandler method.
 *
 * Lifetime is sole ownership, not shared: exactly one owner at any instant,
 * passed hand to hand — dispatcher, then CPU pool task, then handler, then
 * possibly a continuation. Each hand-off travels through something that
 * already synchronizes (an executor queue, a future, an EventBase queue), so
 * the new owner sees everything the previous one wrote and no two threads
 * ever touch the callback at once. That is why there is no reference count
 * here, atomic or otherwise, and no vtable.
 *
 * The one placement requirement is that *destruction* lands on the adapter's
 * EventBase, because it releases two counts that are deliberately non-atomic
 * — the adapter's DelayedDestruction guard and, via the request context, a
 * ThriftConnContext reference. destroyOnEventBase enforces that, independent
 * of which thread happened to own the callback last.
 *
 * Construction must happen on the EventBase for the mirror-image reason: the
 * constructor acquires the adapter guard.
 */
template <typename T>
class FastHandlerCallback {
 public:
  using ResultFn = void (*)(
      ThriftServerAppAdapter*,
      uint32_t,
      std::unique_ptr<ThriftRequestContext>,
      folly::DelayedDestruction::DestructorGuard&&,
      T);
  using ExceptionFn = void (*)(
      ThriftServerAppAdapter*,
      uint32_t,
      folly::DelayedDestruction::DestructorGuard&&,
      folly::exception_wrapper);

  // Must be constructed on `evb`: adapterGuard_ bumps the adapter's
  // non-atomic guardCount_.
  FastHandlerCallback(
      ResultFn resultFn,
      ExceptionFn exceptionFn,
      ThriftServerAppAdapter* handler,
      uint32_t streamId,
      folly::EventBase* evb,
      folly::Executor* executor,
      std::unique_ptr<ThriftRequestContext> requestContext)
      : resultFn_(resultFn),
        exceptionFn_(exceptionFn),
        handler_(handler),
        adapterGuard_(handler),
        streamId_(streamId),
        evb_(folly::getKeepAliveToken(evb)),
        executor_(executor),
        requestContext_(std::move(requestContext)) {}

  FastHandlerCallback(const FastHandlerCallback&) = delete;
  FastHandlerCallback& operator=(const FastHandlerCallback&) = delete;
  FastHandlerCallback(FastHandlerCallback&&) = delete;
  FastHandlerCallback& operator=(FastHandlerCallback&&) = delete;

  // Callable from any thread *when the request was offloaded to a CPU pool*;
  // the adapter's writeResponse handles the EventBase hop. Without a pool the
  // request's reference count is non-atomic, so completion must stay on the
  // EventBase.
  //
  // Safe even after the connection has been force-closed: the donated guard
  // keeps the adapter alive, and writeResponse drops the write because
  // pipelineActive_ is false.
  void result(T value) {
    completed_ = true;
    // Hand the per-request context to the thunk so it rides onto the response
    // message; write-side handlers (e.g. checksum) read request-derived state
    // off the message. The callback itself stays request/response-agnostic.
    //
    // The adapter guard is donated, not copied — constructing one here could
    // be off the EventBase. Moving it is refcount-free, and it lands on the
    // EventBase inside writeResponse.
    resultFn_(
        handler_,
        streamId_,
        std::move(requestContext_),
        std::move(adapterGuard_),
        std::move(value));
  }

  void exception(folly::exception_wrapper ew) {
    completed_ = true;
    exceptionFn_(handler_, streamId_, std::move(adapterGuard_), std::move(ew));
  }

  // Writes a TApplicationException frame directly rather than through the
  // declared/undeclared cascade, for requests that never decoded far enough
  // for the cascade to mean anything. Marks the callback complete so the
  // destructor does not add a second response.
  void sendAppError(const folly::exception_wrapper& ew) noexcept {
    completed_ = true;
    handler_->writeResponse(
        makeAppErrorMessage(
            streamId_,
            "TApplicationException",
            detail::exceptionMessage(ew),
            apache::thrift::ErrorBlame::SERVER),
        std::move(adapterGuard_));
  }

  // True once result()/exception()/sendAppError() has been invoked. Used by
  // generated dispatchers to avoid double-completing a callback when a user
  // handler both completes the callback and then throws synchronously.
  bool isCompleted() const noexcept { return completed_; }

  uint32_t streamId() const noexcept { return streamId_; }

  folly::EventBase* getEventBase() const { return evb_.get(); }

  // Where a generated dispatcher should run a coroutine handler body. Null
  // when the server has no CPU pool, in which case the caller falls back to
  // the EventBase. Raw rather than a KeepAlive: the executor outlives the
  // adapter, which this callback already keeps alive through adapterGuard_,
  // so a per-request refcount would buy nothing.
  folly::Executor* getHandlerExecutor() const noexcept { return executor_; }

  ThriftRequestContext* requestContext() const noexcept {
    return requestContext_.get();
  }

  // Called by CallbackPtr when the sole owner lets go. Destruction has to land
  // on the adapter's EventBase: it releases adapterGuard_ and, through
  // requestContext_, a ThriftConnContext reference, and neither of those
  // counts is atomic.
  //
  // isInEventBaseThread answers "true" for a stopped loop, which is fine here:
  // we hold a KeepAlive on the EventBase, so its loop cannot have finished
  // while this object is alive. The stopped-loop case is only reachable from
  // tests driving an EventBase by hand, where deleting inline is what's
  // wanted.
  void destroyOnEventBase() noexcept {
    if (!evb_ || evb_->isInEventBaseThread()) {
      delete this;
      return;
    }
    evb_->runInEventBaseThread([this] { delete this; });
  }

  // ---- Codegen-targeted static helpers ----
  // Codegen instantiates these with the per-method Presult / ProtocolWriter
  // and passes the function pointers to the ctor above. Each thunk builds
  // the response message via util/ResponsePayloads.h and hands it to the
  // adapter's single writeResponse entry point.

  template <typename Presult, typename ProtocolWriter>
  static void writeSuccess(
      ThriftServerAppAdapter* a,
      uint32_t sid,
      std::unique_ptr<ThriftRequestContext> requestContext,
      folly::DelayedDestruction::DestructorGuard&& adapterGuard,
      T value) noexcept {
    Presult presult;
    if constexpr (detail::IsUniquePtr<T>::value) {
      presult.template get<0>().value = value.get();
    } else {
      presult.template get<0>().value = &value;
    }
    presult.setIsSet(0, true);
    auto message = makeSuccessResponseMessage<ProtocolWriter>(sid, presult);
    message.requestContext = std::move(requestContext);
    a->writeResponse(std::move(message), std::move(adapterGuard));
  }

  template <typename Presult, typename ProtocolWriter>
  static void writeException(
      ThriftServerAppAdapter* a,
      uint32_t sid,
      folly::DelayedDestruction::DestructorGuard&& adapterGuard,
      folly::exception_wrapper ew) noexcept {
    detail::
        writeExceptionCascade<Presult, ProtocolWriter, /*HasReturnType=*/true>(
            a, sid, std::move(adapterGuard), std::move(ew));
  }

 private:
  // Non-virtual and private: the only caller is destroyOnEventBase, so there
  // is no vtable on this type. Synthesizes INTERNAL_ERROR if the user handler
  // dropped the callback without completing. Always runs on the EventBase, so
  // adapterGuard_ — still held, since no completion donated it away — and the
  // request context both release on the right thread.
  ~FastHandlerCallback() {
    if (completed_) {
      return;
    }
    exceptionFn_(
        handler_,
        streamId_,
        std::move(adapterGuard_),
        folly::make_exception_wrapper<TApplicationException>(
            TApplicationException::INTERNAL_ERROR,
            "FastHandlerCallback not completed"));
  }

  ResultFn resultFn_;
  ExceptionFn exceptionFn_;
  ThriftServerAppAdapter* handler_;
  // Keeps the adapter alive while this request is outstanding. Acquired on
  // the EventBase in the constructor, and either donated to the write hop on
  // completion or released here on the EventBase — never acquired or released
  // from a CPU thread, because the adapter's count is not atomic.
  folly::DelayedDestruction::DestructorGuard adapterGuard_;
  uint32_t streamId_;
  // Keepalive rather than a raw pointer: destroyOnEventBase may need to hop
  // onto this EventBase from a CPU thread, so it has to outlive us.
  folly::Executor::KeepAlive<folly::EventBase> evb_;
  // Non-owning; see getHandlerExecutor().
  folly::Executor* executor_{nullptr};
  std::unique_ptr<ThriftRequestContext> requestContext_;
  bool completed_{false};
};

template <>
class FastHandlerCallback<void> {
 public:
  using DoneFn = void (*)(
      ThriftServerAppAdapter*,
      uint32_t,
      std::unique_ptr<ThriftRequestContext>,
      folly::DelayedDestruction::DestructorGuard&&);
  using ExceptionFn = void (*)(
      ThriftServerAppAdapter*,
      uint32_t,
      folly::DelayedDestruction::DestructorGuard&&,
      folly::exception_wrapper);

  // See FastHandlerCallback<T>'s constructor.
  FastHandlerCallback(
      DoneFn doneFn,
      ExceptionFn exceptionFn,
      ThriftServerAppAdapter* handler,
      uint32_t streamId,
      folly::EventBase* evb,
      folly::Executor* executor,
      std::unique_ptr<ThriftRequestContext> requestContext)
      : doneFn_(doneFn),
        exceptionFn_(exceptionFn),
        handler_(handler),
        adapterGuard_(handler),
        streamId_(streamId),
        evb_(folly::getKeepAliveToken(evb)),
        executor_(executor),
        requestContext_(std::move(requestContext)) {}

  FastHandlerCallback(const FastHandlerCallback&) = delete;
  FastHandlerCallback& operator=(const FastHandlerCallback&) = delete;
  FastHandlerCallback(FastHandlerCallback&&) = delete;
  FastHandlerCallback& operator=(FastHandlerCallback&&) = delete;

  // Safe to call from any thread; see FastHandlerCallback<T>::result.
  void done() {
    completed_ = true;
    doneFn_(
        handler_,
        streamId_,
        std::move(requestContext_),
        std::move(adapterGuard_));
  }

  void exception(folly::exception_wrapper ew) {
    completed_ = true;
    exceptionFn_(handler_, streamId_, std::move(adapterGuard_), std::move(ew));
  }

  // See FastHandlerCallback<T>::sendAppError.
  void sendAppError(const folly::exception_wrapper& ew) noexcept {
    completed_ = true;
    handler_->writeResponse(
        makeAppErrorMessage(
            streamId_,
            "TApplicationException",
            detail::exceptionMessage(ew),
            apache::thrift::ErrorBlame::SERVER),
        std::move(adapterGuard_));
  }

  bool isCompleted() const noexcept { return completed_; }

  uint32_t streamId() const noexcept { return streamId_; }

  folly::EventBase* getEventBase() const { return evb_.get(); }

  // Where a generated dispatcher should run a coroutine handler body. Null
  // when the server has no CPU pool, in which case the caller falls back to
  // the EventBase. Raw rather than a KeepAlive: the executor outlives the
  // adapter, which this callback already keeps alive through adapterGuard_,
  // so a per-request refcount would buy nothing.
  folly::Executor* getHandlerExecutor() const noexcept { return executor_; }

  ThriftRequestContext* requestContext() const noexcept {
    return requestContext_.get();
  }

  // See FastHandlerCallback<T>::destroyOnEventBase.
  void destroyOnEventBase() noexcept {
    if (!evb_ || evb_->isInEventBaseThread()) {
      delete this;
      return;
    }
    evb_->runInEventBaseThread([this] { delete this; });
  }

  // ---- Codegen-targeted static helpers (void return) ----

  template <typename Presult, typename ProtocolWriter>
  static void writeDone(
      ThriftServerAppAdapter* a,
      uint32_t sid,
      std::unique_ptr<ThriftRequestContext> requestContext,
      folly::DelayedDestruction::DestructorGuard&& adapterGuard) noexcept {
    Presult presult;
    auto message = makeSuccessResponseMessage<ProtocolWriter>(sid, presult);
    message.requestContext = std::move(requestContext);
    a->writeResponse(std::move(message), std::move(adapterGuard));
  }

  template <typename Presult, typename ProtocolWriter>
  static void writeException(
      ThriftServerAppAdapter* a,
      uint32_t sid,
      folly::DelayedDestruction::DestructorGuard&& adapterGuard,
      folly::exception_wrapper ew) noexcept {
    detail::
        writeExceptionCascade<Presult, ProtocolWriter, /*HasReturnType=*/false>(
            a, sid, std::move(adapterGuard), std::move(ew));
  }

 private:
  // See FastHandlerCallback<T>::~FastHandlerCallback.
  ~FastHandlerCallback() {
    if (completed_) {
      return;
    }
    exceptionFn_(
        handler_,
        streamId_,
        std::move(adapterGuard_),
        folly::make_exception_wrapper<TApplicationException>(
            TApplicationException::INTERNAL_ERROR,
            "FastHandlerCallback not completed"));
  }

  DoneFn doneFn_;
  ExceptionFn exceptionFn_;
  ThriftServerAppAdapter* handler_;
  // See FastHandlerCallback<T>::adapterGuard_.
  folly::DelayedDestruction::DestructorGuard adapterGuard_;
  uint32_t streamId_;
  // See FastHandlerCallback<T>::evb_.
  folly::Executor::KeepAlive<folly::EventBase> evb_;
  // Non-owning; see getHandlerExecutor().
  folly::Executor* executor_{nullptr};
  std::unique_ptr<ThriftRequestContext> requestContext_;
  bool completed_{false};
};

template <typename T>
using FastHandlerCallbackPtr = detail::CallbackPtr<FastHandlerCallback<T>>;

// The callback sits on the per-request path, so it carries no vtable: nothing
// about it is dispatched dynamically, and destruction is driven by the handle
// rather than a virtual destructor. Asserted rather than commented so that
// reintroducing a base class or a virtual is a build failure.
static_assert(
    !std::is_polymorphic_v<FastHandlerCallback<void>>,
    "FastHandlerCallback must not gain a vtable");
static_assert(
    !std::is_polymorphic_v<FastHandlerCallback<int>>,
    "FastHandlerCallback must not gain a vtable");

// Unique ownership is what removes the need for a reference count. A copyable
// handle would allow two simultaneous owners and quietly bring the whole
// question back.
static_assert(
    !std::is_copy_constructible_v<FastHandlerCallbackPtr<void>> &&
        !std::is_copy_assignable_v<FastHandlerCallbackPtr<void>>,
    "FastHandlerCallbackPtr must stay move-only");

// Constructs a callback and returns the owning handle. Must be called on the
// adapter's EventBase — see FastHandlerCallback's constructor.
template <typename Cb, typename... Args>
detail::CallbackPtr<Cb> makeFastHandlerCallback(Args&&... args) {
  return detail::CallbackPtr<Cb>(new Cb(static_cast<Args&&>(args)...));
}

} // namespace apache::thrift::fast_thrift::thrift
