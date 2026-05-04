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

#include <exception>

#include <folly/ExceptionWrapper.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/SerializationSwitch.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseError.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseSerializer.h>
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
    auto buf = serializeResponse<ProtocolWriter>(
        [&](ProtocolWriter& w) { presult.write(&w); },
        [&](ProtocolWriter& w) -> uint32_t {
          return presult.serializedSizeZC(&w);
        });
    (void)a->writeResponse(
        sid,
        std::move(buf),
        makeDeclaredExceptionMetadata(
            ew.class_name().toStdString(),
            ew.what().toStdString(),
            std::move(classification)),
        /*complete=*/true);
  } else {
    (void)a->writeResponse(
        sid,
        /*data=*/nullptr,
        makeAppErrorResponseMetadata(
            ew.class_name().toStdString(),
            ew.what().toStdString(),
            apache::thrift::ErrorBlame::SERVER),
        /*complete=*/true);
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

  // ---- Codegen-targeted static helpers ----
  // Codegen instantiates these with the per-method Presult / ProtocolWriter
  // and passes the function pointers to the ctor above. Keeping the
  // response-building logic here (instead of in the .tcc) means generated
  // code only needs to name the types — not understand how a fast_thrift
  // success / exception frame is assembled.

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
    auto buf = serializeResponse<ProtocolWriter>(
        [&](ProtocolWriter& w) { presult.write(&w); },
        [&](ProtocolWriter& w) -> uint32_t {
          return presult.serializedSizeZC(&w);
        });
    (void)a->writeResponse(
        sid,
        std::move(buf),
        getDefaultSuccessMetadata(),
        /*complete=*/true);
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

  // ---- Codegen-targeted static helpers (void return) ----

  template <typename Presult, typename ProtocolWriter>
  static void writeDone(ThriftServerAppAdapter* a, uint32_t sid) noexcept {
    Presult presult;
    auto buf = serializeResponse<ProtocolWriter>(
        [&](ProtocolWriter& w) { presult.write(&w); },
        [&](ProtocolWriter& w) -> uint32_t {
          return presult.serializedSizeZC(&w);
        });
    (void)a->writeResponse(
        sid,
        std::move(buf),
        getDefaultSuccessMetadata(),
        /*complete=*/true);
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

// Outer cascade: deserialize a request's args (`pargs`) from `data` using
// `ProtocolReader`, sending the appropriate error frame if deserialization
// fails. Returns std::nullopt on success — caller continues dispatch. On
// failure returns the channel_pipeline::Result of the write, which the
// caller should propagate.
//
// Failure modes:
//   TProtocolException → ERROR frame, REQUEST_PARSING_FAILURE
//   std::exception     → PAYLOAD frame, appUnknownException metadata
//   ...                → PAYLOAD frame, appUnknownException metadata
//
// The actual deser delegates to
// apache::thrift::detail::deserializeRequestBodySimple — same primitive legacy
// thrift uses — so behavior matches exactly.
template <typename ProtocolReader, typename Pargs>
[[nodiscard]] inline std::optional<channel_pipeline::Result>
parseArgsOrSendError(
    ThriftServerAppAdapter* adapter,
    uint32_t streamId,
    folly::IOBuf& data,
    Pargs& pargs) noexcept {
  try {
    ProtocolReader reader;
    folly::io::Cursor cursor(&data);
    reader.setInput(cursor);
    apache::thrift::detail::deserializeRequestBodySimple(&reader, &pargs);
    return std::nullopt;
  } catch (const apache::thrift::protocol::TProtocolException& ex) {
    auto err = serializeResponseRpcError(
        apache::thrift::ResponseRpcErrorCode::REQUEST_PARSING_FAILURE,
        ex.what());
    return adapter->writeError(streamId, std::move(err.data), err.errorCode);
  } catch (const std::exception& ex) {
    return adapter->writeResponse(
        streamId,
        /*data=*/nullptr,
        makeAppErrorResponseMetadata(
            "TApplicationException",
            ex.what(),
            apache::thrift::ErrorBlame::SERVER),
        /*complete=*/true);
  } catch (...) {
    return adapter->writeResponse(
        streamId,
        /*data=*/nullptr,
        makeAppErrorResponseMetadata(
            "TApplicationException",
            "Unknown exception during args deserialization",
            apache::thrift::ErrorBlame::SERVER),
        /*complete=*/true);
  }
}

} // namespace apache::thrift::fast_thrift::thrift
