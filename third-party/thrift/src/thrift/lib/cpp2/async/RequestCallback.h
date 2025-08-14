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

#include <chrono>
#include <map>
#include <optional>
#include <string>

#include <folly/Try.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp/ContextStack.h>
#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/ClientSinkBridge.h>
#include <thrift/lib/cpp2/async/ClientStreamBridge.h>
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>

namespace apache::thrift {

struct RpcTransportStats {
  RpcTransportStats() = default;

  uint32_t requestSerializedSizeBytes{
      0}; // size of serialized payload without meta data (uncompressed)
  uint32_t requestWireSizeBytes{0}; // size of data (possibly compressed)
  uint32_t requestMetadataAndPayloadSizeBytes{
      0}; // size of meta data (uncompressed) and data (possibly compressed)
  uint32_t responseSerializedSizeBytes{0};
  uint32_t responseWireSizeBytes{0};
  uint32_t responseMetadataAndPayloadSizeBytes{0};

  // I/O latencies
  std::chrono::nanoseconds requestWriteLatency{0};
  std::chrono::nanoseconds responseRoundTripLatency{0};
};

class ClientReceiveState {
 public:
  ClientReceiveState() : protocolId_(-1) {}

  ClientReceiveState(
      uint16_t _protocolId,
      std::unique_ptr<folly::IOBuf> _buf,
      std::unique_ptr<apache::thrift::transport::THeader> _header,
      apache::thrift::ContextStack::UniquePtr _ctx)
      : protocolId_(_protocolId),
        ctx_(std::move(_ctx)),
        header_(std::move(_header)) {
    initFromLegacyFormat(std::move(_buf));
  }
  ClientReceiveState(
      uint16_t _protocolId,
      std::unique_ptr<folly::IOBuf> _buf,
      std::unique_ptr<apache::thrift::transport::THeader> _header,
      apache::thrift::ContextStack::UniquePtr _ctx,
      const RpcTransportStats& _rpcTransportStats)
      : protocolId_(_protocolId),
        ctx_(std::move(_ctx)),
        header_(std::move(_header)),
        rpcTransportStats_(_rpcTransportStats) {
    initFromLegacyFormat(std::move(_buf));
  }
  ClientReceiveState(
      uint16_t _protocolId,
      std::unique_ptr<folly::IOBuf> buf,
      std::unique_ptr<apache::thrift::transport::THeader> _header,
      apache::thrift::ContextStack::UniquePtr _ctx,
      apache::thrift::detail::ClientStreamBridge::ClientPtr streamBridge,
      const BufferOptions& bufferOptions)
      : protocolId_(_protocolId),
        ctx_(std::move(_ctx)),
        header_(std::move(_header)),
        streamBridge_(std::move(streamBridge)),
        bufferOptions_(bufferOptions) {
    initFromLegacyFormat(std::move(buf));
  }
  ClientReceiveState(
      folly::exception_wrapper _excw,
      apache::thrift::ContextStack::UniquePtr _ctx)
      : protocolId_(-1),
        ctx_(std::move(_ctx)),
        header_(std::make_unique<apache::thrift::transport::THeader>()),
        excw_(std::move(_excw)) {}
  ClientReceiveState(
      uint16_t _protocolId,
      std::unique_ptr<folly::IOBuf> _buf,
      apache::thrift::detail::ClientSinkBridge::ClientPtr sink,
      std::unique_ptr<apache::thrift::transport::THeader> _header,
      apache::thrift::ContextStack::UniquePtr _ctx)
      : protocolId_(_protocolId),
        ctx_(std::move(_ctx)),
        header_(std::move(_header)),
        sink_(std::move(sink)) {
    initFromLegacyFormat(std::move(_buf));
  }
  ClientReceiveState(
      uint16_t _protocolId,
      MessageType mtype,
      SerializedResponse response,
      std::unique_ptr<apache::thrift::transport::THeader> _header,
      apache::thrift::ContextStack::UniquePtr _ctx,
      const RpcTransportStats& _rpcTransportStats)
      : protocolId_(_protocolId),
        messageType_(mtype),
        ctx_(std::move(_ctx)),
        response_(std::move(response)),
        header_(std::move(_header)),
        rpcTransportStats_(_rpcTransportStats) {}

  bool isException() const { return excw_ ? true : false; }

  const folly::exception_wrapper& exception() const { return excw_; }

  folly::exception_wrapper& exception() { return excw_; }

  uint16_t protocolId() const { return protocolId_; }

  bool hasResponseBuffer() const { return (bool)response_.buffer; }

  MessageType messageType() const { return messageType_; }

  SerializedResponse& serializedResponse() { return response_; }
  const SerializedResponse& serializedResponse() const { return response_; }

  SerializedResponse extractSerializedResponse() {
    return std::move(response_);
  }
  apache::thrift::ClientInterceptorOnResponseResult& extractResult() {
    return result_;
  }

  void setResult(apache::thrift::ClientInterceptorOnResponseResult&& result) {
    result_ = std::move(result);
  }
  apache::thrift::detail::ClientStreamBridge::ClientPtr extractStreamBridge() {
    return std::move(streamBridge_);
  }

  apache::thrift::detail::ClientSinkBridge::ClientPtr extractSink() {
    return std::move(sink_);
  }

  const BufferOptions& bufferOptions() const { return bufferOptions_; }

  apache::thrift::transport::THeader* header() const { return header_.get(); }

  std::unique_ptr<apache::thrift::transport::THeader> extractHeader() {
    return std::move(header_);
  }

  void resetHeader(std::unique_ptr<apache::thrift::transport::THeader> h) {
    header_ = std::move(h);
  }

  apache::thrift::ContextStack* ctx() const { return ctx_.get(); }

  void resetProtocolId(uint16_t protocolId) { protocolId_ = protocolId; }

  void resetCtx(apache::thrift::ContextStack::UniquePtr _ctx) {
    ctx_ = std::move(_ctx);
  }

  const RpcTransportStats& getRpcTransportStats() const {
    return rpcTransportStats_;
  }

  static ClientReceiveState create(
      std::unique_ptr<folly::IOBuf> buf,
      std::unique_ptr<apache::thrift::transport::THeader> tHeader,
      apache::thrift::detail::ClientStreamBridge::ClientPtr clientStreamBridge,
      BufferOptions bufferOptions = BufferOptions(),
      uint16_t protocolId = static_cast<uint16_t>(-1)) {
    return ClientReceiveState(
        protocolId,
        std::move(buf),
        std::move(tHeader),
        nullptr,
        std::move(clientStreamBridge),
        bufferOptions);
  }

  static ClientReceiveState create(
      std::unique_ptr<folly::IOBuf> buf,
      std::unique_ptr<apache::thrift::transport::THeader> tHeader,
      apache::thrift::detail::ClientSinkBridge::ClientPtr clientSinkBridge,
      BufferOptions,
      uint16_t protocolId = static_cast<uint16_t>(-1)) {
    return ClientReceiveState(
        protocolId,
        std::move(buf),
        std::move(clientSinkBridge),
        std::move(tHeader),
        nullptr);
  }

 private:
  uint16_t protocolId_;
  MessageType messageType_{MessageType::T_CALL};
  apache::thrift::ContextStack::UniquePtr ctx_;
  SerializedResponse response_{nullptr};
  std::unique_ptr<apache::thrift::transport::THeader> header_;
  folly::exception_wrapper excw_;
  apache::thrift::detail::ClientStreamBridge::ClientPtr streamBridge_;
  apache::thrift::detail::ClientSinkBridge::ClientPtr sink_;
  BufferOptions bufferOptions_;
  RpcTransportStats rpcTransportStats_;
  apache::thrift::ClientInterceptorOnResponseResult result_;
  void initFromLegacyFormat(std::unique_ptr<folly::IOBuf> buffer);
};

class RequestClientCallback {
 public:
  struct RequestClientCallbackDeleter {
    void operator()(RequestClientCallback* callback) const {
      callback->onResponseError(folly::exception_wrapper(
          std::logic_error("Request callback detached")));
    }
  };
  using Ptr =
      std::unique_ptr<RequestClientCallback, RequestClientCallbackDeleter>;

  virtual ~RequestClientCallback() {}
  virtual void onResponse(ClientReceiveState&&) noexcept = 0;
  virtual void onResponseError(folly::exception_wrapper) noexcept = 0;

  // If true, the transport can block current thread/fiber until the request is
  // complete.
  virtual bool isSync() const { return false; }

  // If true, the transport can safely run this callback on its internal thread.
  // Should only be used for Thrift internal callbacks.
  virtual bool isInlineSafe() const { return false; }

  // If available, returns an executor of the calling code. If provided, this
  // executor can be used by the channel.
  virtual folly::Executor::KeepAlive<> getExecutor() const { return {}; }
};

class RequestCallback : public RequestClientCallback {
 public:
  virtual void requestSent() = 0;
  virtual void replyReceived(ClientReceiveState&&) = 0;
  virtual void requestError(ClientReceiveState&&) = 0;

  void onResponse(ClientReceiveState&& state) noexcept override {
    CHECK(thriftContext_);
    {
      state.resetProtocolId(thriftContext_->protocolId);
      state.resetCtx(std::move(thriftContext_->ctx));
      folly::RequestContextScopeGuard rctx(std::move(context_));
      requestSentHelper();
      if (!thriftContext_->oneWay) {
        try {
          replyReceived(std::move(state));
        } catch (...) {
          LOG(DFATAL)
              << "Exception thrown while executing replyReceived() callback. "
              << "Exception: "
              << folly::exceptionStr(folly::current_exception());
        }
      }
    }
    if (unmanaged_) {
      delete this;
    }
  }

  void onResponseError(folly::exception_wrapper ex) noexcept override {
    CHECK(thriftContext_);
    {
      folly::RequestContextScopeGuard rctx(std::move(context_));
      bool sendError = false;
      ex.with_exception([&](const transport::TTransportException& tex) {
        sendError = tex.getType() ==
            apache::thrift::transport::TTransportException::NOT_OPEN;
      });
      if (!sendError && !thriftContext_->oneWay) {
        requestSentHelper();
      }
      try {
        requestError(
            ClientReceiveState(std::move(ex), std::move(thriftContext_->ctx)));
      } catch (...) {
        LOG(DFATAL)
            << "Exception thrown while executing requestError() callback. "
            << "Exception: " << folly::exceptionStr(folly::current_exception());
      }
    }
    if (unmanaged_) {
      delete this;
    }
  }

  std::shared_ptr<folly::RequestContext> context_;

  struct Context {
    bool oneWay{false};
    uint16_t protocolId;
    apache::thrift::ContextStack::UniquePtr ctx;
  };

 private:
  void requestSentHelper() noexcept {
    try {
      requestSent();
    } catch (...) {
      LOG(DFATAL) << "Exception thrown while executing requestSent() callback. "
                  << "Exception: "
                  << folly::exceptionStr(folly::current_exception());
    }
  }

  friend RequestClientCallback::Ptr toRequestClientCallbackPtr(
      std::unique_ptr<RequestCallback>, RequestCallback::Context);

  void setContext(Context context) {
    context_ = folly::RequestContext::saveContext();
    thriftContext_ = std::move(context);
  }

  void setUnmanaged() { unmanaged_ = true; }

  bool unmanaged_{false};
  folly::Optional<Context> thriftContext_;
};

inline RequestClientCallback::Ptr toRequestClientCallbackPtr(
    std::unique_ptr<RequestCallback> cb, RequestCallback::Context context) {
  if (!cb) {
    return RequestClientCallback::Ptr();
  }
  cb->setContext(std::move(context));
  cb->setUnmanaged();
  return RequestClientCallback::Ptr(cb.release());
}

/* FunctionReplyCallback is meant to make RequestCallback easy to use
 * with folly::Function objects.  It is slower than implementing
 * RequestCallback directly.  It also throws the specific error
 * away, since there is no place to save it in a backwards
 * compatible way to thrift1.  It is still logged, though.
 *
 * Recommend upgrading to RequestCallback if possible
 */
class FunctionReplyCallback : public RequestCallback {
 public:
  explicit FunctionReplyCallback(
      folly::Function<void(ClientReceiveState&&)> callback)
      : callback_(std::move(callback)) {}
  void replyReceived(ClientReceiveState&& state) override {
    callback_(std::move(state));
  }
  void requestError(ClientReceiveState&& state) override {
    VLOG(1) << "Got an exception in FunctionReplyCallback replyReceiveError: "
            << state.exception();
    callback_(std::move(state));
  }
  void requestSent() override {}

 private:
  folly::Function<void(ClientReceiveState&&)> callback_;
};

/* Useful for oneway methods. */
class FunctionSendCallback : public RequestCallback {
 public:
  explicit FunctionSendCallback(
      folly::Function<void(ClientReceiveState&&)>&& callback)
      : callback_(std::move(callback)) {}
  void requestSent() override {
    auto cb = std::move(callback_);
    cb(ClientReceiveState(folly::exception_wrapper(), nullptr));
  }
  void requestError(ClientReceiveState&& state) override {
    auto cb = std::move(callback_);
    cb(std::move(state));
  }
  void replyReceived(ClientReceiveState&& /*state*/) override {}

 private:
  folly::Function<void(ClientReceiveState&&)> callback_;
};

class CloseCallback {
 public:
  /**
   * When the channel is closed, replyError() will be invoked on all of the
   * outstanding replies, then channelClosed() on the CloseCallback.
   */
  virtual void channelClosed() = 0;

  virtual ~CloseCallback() {}
};

struct RpcResponseContext {
  transport::THeader::StringToStringMap headers;
  RpcTransportStats rpcTransportStats;
  std::optional<int64_t> serverLoad;
};

template <class T>
struct RpcResponseComplete {
  using response_type = T;

  folly::Try<T> response;
  RpcResponseContext responseContext;
};

} // namespace apache::thrift
