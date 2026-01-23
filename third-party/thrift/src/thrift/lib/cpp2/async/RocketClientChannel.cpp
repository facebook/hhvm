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

#include <thrift/lib/cpp2/async/RocketClientChannel.h>

#include <memory>
#include <utility>

#include <fmt/core.h>
#include <folly/ExceptionString.h>
#include <folly/Memory.h>
#include <folly/Range.h>
#include <folly/Try.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/protocol/TBase64Utils.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataPlugins.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>
#include <thrift/lib/cpp2/transport/core/ThriftClientCallback.h>
#include <thrift/lib/cpp2/transport/core/TryUtil.h>
#include <thrift/lib/cpp2/transport/rocket/FdSocket.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_constants.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#if __has_include(<thrift/lib/thrift/gen-cpp2/any_rep_types.h>)
#include <thrift/lib/thrift/gen-cpp2/any_rep_types.h>
#define THRIFT_ANY_AVAILABLE
#endif

namespace {
const int64_t kRocketClientMaxVersion = 10;
const int64_t kRocketClientMinVersion = 8;
} // namespace

THRIFT_FLAG_DEFINE_int64(rocket_client_max_version, kRocketClientMaxVersion);
THRIFT_FLAG_DEFINE_bool(rocket_client_enable_bidirectional_propagation, false);

using namespace apache::thrift::transport;

namespace apache::thrift {

namespace {
struct LegacyResponseSerializationHandler {
  LegacyResponseSerializationHandler(uint16_t protoId, folly::StringPiece mname)
      : protocolId(protoId), methodName(mname) {}

  std::unique_ptr<folly::IOBuf> handleException(
      const TApplicationException& ex) {
    return LegacySerializedResponse(protocolId, methodName, ex).buffer;
  }

  std::unique_ptr<folly::IOBuf> handleReply(
      std::unique_ptr<folly::IOBuf> buffer) {
    return LegacySerializedResponse(
               protocolId, methodName, SerializedResponse(std::move(buffer)))
        .buffer;
  }

  uint16_t protocolId;
  folly::StringPiece methodName;
};

struct ResponseSerializationHandler {
  explicit ResponseSerializationHandler(uint16_t protoId)
      : protocolId(protoId) {}

  std::unique_ptr<folly::IOBuf> handleException(
      const TApplicationException& ex) {
    mtype = MessageType::T_EXCEPTION;
    return serializeErrorStruct(
        static_cast<protocol::PROTOCOL_TYPES>(protocolId), ex);
  }

  std::unique_ptr<folly::IOBuf> handleReply(
      std::unique_ptr<folly::IOBuf> buffer) {
    return buffer;
  }

  uint16_t protocolId;
  MessageType mtype{MessageType::T_REPLY};
};

template <class Handler>
folly::Try<FirstResponsePayload> decodeResponseError(
    rocket::RocketException&& ex,
    Handler&& handler,
    rocket::PayloadSerializer& payloadSerializer) noexcept {
  switch (ex.getErrorCode()) {
    case rocket::ErrorCode::CANCELED:
    case rocket::ErrorCode::INVALID:
    case rocket::ErrorCode::REJECTED:
      break;
    default:
      return folly::Try<FirstResponsePayload>(
          folly::make_exception_wrapper<TApplicationException>(fmt::format(
              "Unexpected error frame type: {}",
              static_cast<uint32_t>(ex.getErrorCode()))));
  }

  ResponseRpcError responseError;
  try {
    payloadSerializer.unpack(responseError, ex.moveErrorData().get(), false);
  } catch (...) {
    return folly::Try<FirstResponsePayload>(
        folly::make_exception_wrapper<TApplicationException>(fmt::format(
            "Error parsing error frame: {}",
            folly::exceptionStr(folly::current_exception()).toStdString())));
  }

  folly::Optional<std::string> exCode;
  TApplicationException::TApplicationExceptionType exType{
      TApplicationException::UNKNOWN};
  switch (responseError.code().value_or(ResponseRpcErrorCode::UNKNOWN)) {
    case ResponseRpcErrorCode::OVERLOAD:
      exCode = kOverloadedErrorCode;
      exType = TApplicationException::LOADSHEDDING;
      break;
    case ResponseRpcErrorCode::TASK_EXPIRED:
      exCode = kTaskExpiredErrorCode;
      exType = TApplicationException::TIMEOUT;
      break;
    case ResponseRpcErrorCode::QUEUE_OVERLOADED:
    case ResponseRpcErrorCode::SHUTDOWN:
      exCode = kQueueOverloadedErrorCode;
      exType = TApplicationException::LOADSHEDDING;
      break;
    case ResponseRpcErrorCode::INJECTED_FAILURE:
      exCode = kInjectedFailureErrorCode;
      exType = TApplicationException::INJECTED_FAILURE;
      break;
    case ResponseRpcErrorCode::REQUEST_PARSING_FAILURE:
      exCode = kRequestParsingErrorCode;
      exType = TApplicationException::UNSUPPORTED_CLIENT_TYPE;
      break;
    case ResponseRpcErrorCode::QUEUE_TIMEOUT:
      exCode = kServerQueueTimeoutErrorCode;
      exType = TApplicationException::TIMEOUT;
      break;
    case ResponseRpcErrorCode::RESPONSE_TOO_BIG:
      exCode = kResponseTooBigErrorCode;
      exType = TApplicationException::INTERNAL_ERROR;
      break;
    case ResponseRpcErrorCode::WRONG_RPC_KIND:
      exCode = kRequestTypeDoesntMatchServiceFunctionType;
      exType = TApplicationException::UNKNOWN_METHOD;
      break;
    case ResponseRpcErrorCode::UNKNOWN_METHOD:
      exCode = kMethodUnknownErrorCode;
      exType = TApplicationException::UNKNOWN_METHOD;
      break;
    case ResponseRpcErrorCode::CHECKSUM_MISMATCH:
      exCode = kUnknownErrorCode;
      exType = TApplicationException::CHECKSUM_MISMATCH;
      break;
    case ResponseRpcErrorCode::INTERRUPTION:
      exType = TApplicationException::INTERRUPTION;
      break;
    case ResponseRpcErrorCode::APP_OVERLOAD:
      exCode = kAppOverloadedErrorCode;
      exType = TApplicationException::LOADSHEDDING;
      break;
    case ResponseRpcErrorCode::UNKNOWN_INTERACTION_ID:
      exCode = kInteractionIdUnknownErrorCode;
      break;
    case ResponseRpcErrorCode::INTERACTION_CONSTRUCTOR_ERROR:
      exCode = kInteractionConstructorErrorErrorCode;
      break;
    case ResponseRpcErrorCode::UNIMPLEMENTED_METHOD:
      exCode = kUnimplementedMethodErrorCode;
      exType = TApplicationException::UNKNOWN_METHOD;
      break;
    case ResponseRpcErrorCode::TENANT_QUOTA_EXCEEDED:
      exCode = kTenantQuotaExceededErrorCode;
      exType = TApplicationException::LOADSHEDDING;
      break;
    case ResponseRpcErrorCode::INTERACTION_LOADSHEDDED:
      exCode = kInteractionLoadsheddedErrorCode;
      exType = TApplicationException::LOADSHEDDING;
      break;
    case ResponseRpcErrorCode::INTERACTION_LOADSHEDDED_OVERLOAD:
      exCode = kInteractionLoadsheddedOverloadErrorCode;
      exType = TApplicationException::LOADSHEDDING;
      break;
    case ResponseRpcErrorCode::INTERACTION_LOADSHEDDED_APP_OVERLOAD:
      exCode = kInteractionLoadsheddedAppOverloadErrorCode;
      exType = TApplicationException::LOADSHEDDING;
      break;
    case ResponseRpcErrorCode::INTERACTION_LOADSHEDDED_QUEUE_TIMEOUT:
      exCode = kInteractionLoadsheddedQueueTimeoutErrorCode;
      exType = TApplicationException::LOADSHEDDING;
      break;
    default:
      exCode = kUnknownErrorCode;
      break;
  }

  ResponseRpcMetadata metadata;
  if (exCode) {
    metadata.otherMetadata().emplace();
    (*metadata.otherMetadata())[detail::kHeaderEx] = *exCode;
  }
  if (auto loadRef = responseError.load()) {
    metadata.load() = *loadRef;
  }
  return folly::Try<FirstResponsePayload>(FirstResponsePayload(
      handler.handleException(TApplicationException(
          exType, responseError.what_utf8().value_or(""))),
      std::move(metadata)));
}

template <class Handler>
[[nodiscard]] folly::exception_wrapper processFirstResponse(
    uint16_t protocolId,
    ResponseRpcMetadata& metadata,
    std::unique_ptr<folly::IOBuf>& payload,
    Handler& handler) {
  (void)protocolId;
  if (auto payloadMetadataRef = metadata.payloadMetadata()) {
    const auto isProxiedResponse =
        metadata.proxiedPayloadMetadata().has_value();

    switch (payloadMetadataRef->getType()) {
      case PayloadMetadata::Type::responseMetadata:
        payload = handler.handleReply(std::move(payload));
        break;
      case PayloadMetadata::Type::exceptionMetadata: {
        auto& exceptionMetadataBase =
            payloadMetadataRef->get_exceptionMetadata();
        auto otherMetadataRef = metadata.otherMetadata();
        if (!otherMetadataRef) {
          otherMetadataRef.emplace();
        }
        auto exceptionNameRef = exceptionMetadataBase.name_utf8();
        auto exceptionWhatRef = exceptionMetadataBase.what_utf8();
        if (auto exceptionMetadataRef = exceptionMetadataBase.metadata()) {
          auto metaType = exceptionMetadataRef->getType();
          switch (metaType) {
            case PayloadExceptionMetadata::Type::declaredException:
              if (exceptionNameRef) {
                (*otherMetadataRef)
                    [isProxiedResponse ? detail::kHeaderProxiedUex
                                       : detail::kHeaderUex] =
                        *exceptionNameRef;
              }
              if (exceptionWhatRef) {
                (*otherMetadataRef)
                    [isProxiedResponse ? detail::kHeaderProxiedUexw
                                       : detail::kHeaderUexw] =
                        *exceptionWhatRef;
              }
              if (auto dExClass = exceptionMetadataRef->declaredException()
                                      ->errorClassification()) {
                auto metaStr =
                    apache::thrift::detail::serializeErrorClassification(
                        *dExClass);
                (*otherMetadataRef)[std::string(detail::kHeaderExMeta)] =
                    std::move(metaStr);
              }
              payload = handler.handleReply(std::move(payload));
              break;
            case PayloadExceptionMetadata::Type::DEPRECATED_proxyException:
              (*otherMetadataRef)
                  [isProxiedResponse ? detail::kHeaderProxiedAnyex
                                     : detail::kHeaderAnyex] =
                      protocol::base64Encode(payload->coalesce());
              payload = handler.handleException(
                  TApplicationException(exceptionWhatRef.value_or("")));
              break;
#ifdef THRIFT_ANY_AVAILABLE
            case PayloadExceptionMetadata::Type::anyException: {
              type::SemiAnyStruct anyException;
              try {
                if (protocolId == protocol::T_COMPACT_PROTOCOL) {
                  CompactSerializer::deserialize(payload.get(), anyException);
                } else {
                  BinarySerializer::deserialize(payload.get(), anyException);
                }
              } catch (...) {
                return TApplicationException(
                    "anyException deserialization failure: " +
                    folly::exceptionStr(folly::current_exception())
                        .toStdString());
              }
              if (*anyException.protocol() == type::kNoProtocol) {
                anyException.protocol() =
                    protocolId == protocol::T_COMPACT_PROTOCOL
                    ? type::Protocol::get<type::StandardProtocol::Compact>()
                    : type::Protocol::get<type::StandardProtocol::Binary>();
              }
              auto exceptionTypeRef =
                  anyException.type()->toThrift().name()->exceptionType();
              if (exceptionTypeRef && exceptionTypeRef->uri() &&
                  anyException.protocol() ==
                      type::Protocol::get<type::StandardProtocol::Compact>()) {
                (*otherMetadataRef)
                    [isProxiedResponse ? detail::kHeaderProxiedAnyexType
                                       : detail::kHeaderAnyexType] =
                        *exceptionTypeRef->uri();
                (*otherMetadataRef)
                    [isProxiedResponse ? detail::kHeaderProxiedAnyex
                                       : detail::kHeaderAnyex] =
                        protocol::base64Encode(anyException.data()->coalesce());
              }
              payload = handler.handleException(
                  TApplicationException(exceptionWhatRef.value_or("")));
              break;
            }
#endif
            default:
              switch (metaType) {
                case PayloadExceptionMetadata::Type::appUnknownException:
                  if (auto ec = exceptionMetadataRef->appUnknownException()
                                    ->errorClassification()) {
                    if (ec->blame() && *ec->blame() == ErrorBlame::CLIENT) {
                      (*otherMetadataRef)
                          [isProxiedResponse ? detail::kHeaderProxiedEx
                                             : detail::kHeaderEx] =
                              kAppClientErrorCode;
                    }
                  }
                  break;
                default:;
              }

              if (exceptionNameRef) {
                (*otherMetadataRef)
                    [isProxiedResponse ? detail::kHeaderProxiedUex
                                       : detail::kHeaderUex] =
                        *exceptionNameRef;
              }
              if (exceptionWhatRef) {
                (*otherMetadataRef)
                    [isProxiedResponse ? detail::kHeaderProxiedUexw
                                       : detail::kHeaderUexw] =
                        *exceptionWhatRef;
              }
              payload = handler.handleException(
                  TApplicationException(exceptionWhatRef.value_or("")));
          }
        } else {
          return TApplicationException("Missing payload exception metadata");
        }
        break;
      }
      default:
        return TApplicationException("Invalid payload metadata type");
    }
  }
  return {};
}

class FirstRequestProcessorStream : public StreamClientCallback,
                                    private StreamServerCallback {
 public:
  FirstRequestProcessorStream(
      uint16_t protocolId,
      apache::thrift::ManagedStringView&& methodName,
      StreamClientCallback* clientCallback,
      folly::EventBase* evb,
      rocket::RocketClient::DestructionGuardedClient guardedClient)
      : protocolId_(protocolId),
        methodName_(std::move(methodName)),
        clientCallback_(clientCallback),
        evb_(evb),
        guardedClient_(std::move(guardedClient)) {}

  [[nodiscard]] bool onFirstResponse(
      FirstResponsePayload&& firstResponse,
      folly::EventBase* evb,
      StreamServerCallback* serverCallback) override {
    SCOPE_EXIT {
      delete this;
    };
    DCHECK_EQ(evb, evb_);
    LegacyResponseSerializationHandler handler(protocolId_, methodName_.view());
    if (auto error = processFirstResponse(
            protocolId_,
            firstResponse.metadata,
            firstResponse.payload,
            handler)) {
      serverCallback->onStreamCancel();
      clientCallback_->onFirstResponseError(std::move(error));
      return false;
    }
    serverCallback->resetClientCallback(*clientCallback_);
    return clientCallback_->onFirstResponse(
        std::move(firstResponse), evb, serverCallback);
  }
  void onFirstResponseError(folly::exception_wrapper ew) override {
    SCOPE_EXIT {
      delete this;
    };
    ew.handle(
        [&](rocket::RocketException& ex) {
          LegacyResponseSerializationHandler handler(
              protocolId_, methodName_.view());
          auto response = decodeResponseError(
              std::move(ex),
              handler,
              *guardedClient_.client->getPayloadSerializer());
          if (response.hasException()) {
            clientCallback_->onFirstResponseError(
                std::move(response).exception());
            return;
          }

          if (clientCallback_->onFirstResponse(
                  std::move(*response), evb_, this)) {
            DCHECK(clientCallback_);
            clientCallback_->onStreamComplete();
          }
        },
        [&](...) { clientCallback_->onFirstResponseError(std::move(ew)); });
  }

  bool onStreamNext(StreamPayload&&) override { std::terminate(); }
  void onStreamError(folly::exception_wrapper) override { std::terminate(); }
  void onStreamComplete() override { std::terminate(); }
  void resetServerCallback(StreamServerCallback&) override { std::terminate(); }

  void onStreamCancel() override { clientCallback_ = nullptr; }
  bool onStreamRequestN(int32_t) override { return true; }
  void resetClientCallback(StreamClientCallback& clientCallback) override {
    clientCallback_ = &clientCallback;
  }

 private:
  const uint16_t protocolId_;
  const ManagedStringView methodName_;
  StreamClientCallback* clientCallback_;
  folly::EventBase* evb_;
  rocket::RocketClient::DestructionGuardedClient guardedClient_;
};

class FirstRequestProcessorSink : public SinkClientCallback,
                                  private SinkServerCallback {
 public:
  FirstRequestProcessorSink(
      uint16_t protocolId,
      apache::thrift::ManagedStringView&& methodName,
      SinkClientCallback* clientCallback,
      folly::EventBase* evb,
      rocket::RocketClient::DestructionGuardedClient guardedClient)
      : protocolId_(protocolId),
        methodName_(std::move(methodName)),
        clientCallback_(clientCallback),
        evb_(evb),
        guardedClient_(std::move(guardedClient)) {}

  [[nodiscard]] bool onFirstResponse(
      FirstResponsePayload&& firstResponse,
      folly::EventBase* evb,
      SinkServerCallback* serverCallback) override {
    SCOPE_EXIT {
      delete this;
    };
    LegacyResponseSerializationHandler handler(protocolId_, methodName_.view());
    if (auto error = processFirstResponse(
            protocolId_,
            firstResponse.metadata,
            firstResponse.payload,
            handler)) {
      serverCallback->onSinkError(
          folly::make_exception_wrapper<TApplicationException>(
              TApplicationException::INTERRUPTION,
              "process first response error"));
      clientCallback_->onFirstResponseError(std::move(error));
      return false;
    }
    serverCallback->resetClientCallback(*clientCallback_);
    return clientCallback_->onFirstResponse(
        std::move(firstResponse), evb, serverCallback);
  }
  void onFirstResponseError(folly::exception_wrapper ew) override {
    SCOPE_EXIT {
      delete this;
    };
    ew.handle(
        [&](rocket::RocketException& ex) {
          LegacyResponseSerializationHandler handler(
              protocolId_, methodName_.view());
          auto response = decodeResponseError(
              std::move(ex),
              handler,
              *guardedClient_.client->getPayloadSerializer());
          if (response.hasException()) {
            clientCallback_->onFirstResponseError(
                std::move(response).exception());
            return;
          }

          if (clientCallback_->onFirstResponse(
                  std::move(*response), evb_, this)) {
            DCHECK(clientCallback_);
            // This exception will be ignored, but we have to send it to follow
            // the contract.
            clientCallback_->onFinalResponseError(
                folly::make_exception_wrapper<TApplicationException>(
                    TApplicationException::INTERRUPTION,
                    "Initial response error"));
          }
        },
        [&](...) {
          clientCallback_->onFirstResponseError(std::move(ew));
          return false;
        });
  }

  void onFinalResponse(StreamPayload&&) override { std::terminate(); }
  void onFinalResponseError(folly::exception_wrapper) override {
    std::terminate();
  }
  [[nodiscard]] bool onSinkRequestN(int32_t) override { std::terminate(); }
  void resetServerCallback(SinkServerCallback&) override { std::terminate(); }

  bool onSinkNext(StreamPayload&&) override { return true; }
  void onSinkError(folly::exception_wrapper) override {
    clientCallback_ = nullptr;
  }
  bool onSinkComplete() override { return true; }
  void resetClientCallback(SinkClientCallback& clientCallback) override {
    clientCallback_ = &clientCallback;
  }

 private:
  const uint16_t protocolId_;
  const ManagedStringView methodName_;
  SinkClientCallback* clientCallback_;
  folly::EventBase* evb_;
  rocket::RocketClient::DestructionGuardedClient guardedClient_;
};

class FirstRequestProcessorBiDi : public BiDiClientCallback,
                                  private BiDiServerCallback {
 public:
  FirstRequestProcessorBiDi(
      uint16_t protocolId,
      apache::thrift::ManagedStringView&& methodName,
      BiDiClientCallback* clientCallback,
      folly::EventBase* evb,
      rocket::RocketClient::DestructionGuardedClient guardedClient)
      : protocolId_(protocolId),
        methodName_(std::move(methodName)),
        clientCallback_(clientCallback),
        evb_(evb),
        guardedClient_(std::move(guardedClient)) {}

  [[nodiscard]] bool onFirstResponse(
      FirstResponsePayload&& firstResponse,
      folly::EventBase* evb,
      BiDiServerCallback* serverCallback) override {
    SCOPE_EXIT {
      delete this;
    };
    LegacyResponseSerializationHandler handler(protocolId_, methodName_.view());
    if (auto error = processFirstResponse(
            protocolId_,
            firstResponse.metadata,
            firstResponse.payload,
            handler)) {
      clientCallback_->onFirstResponseError(std::move(error));
      return serverCallback->onStreamCancel() &&
          serverCallback->onSinkError(
              folly::make_exception_wrapper<TApplicationException>(
                  TApplicationException::INTERRUPTION,
                  "process first response error"));
    }
    serverCallback->resetClientCallback(*clientCallback_);
    return clientCallback_->onFirstResponse(
        std::move(firstResponse), evb, serverCallback);
  }
  void onFirstResponseError(folly::exception_wrapper ew) override {
    SCOPE_EXIT {
      delete this;
    };
    ew.handle(
        [&](rocket::RocketException& ex) {
          LegacyResponseSerializationHandler handler(
              protocolId_, methodName_.view());
          auto response = decodeResponseError(
              std::move(ex),
              handler,
              *guardedClient_.client->getPayloadSerializer());
          if (response.hasException()) {
            clientCallback_->onFirstResponseError(
                std::move(response).exception());
            return;
          }

          if (clientCallback_->onFirstResponse(
                  std::move(*response), evb_, this)) {
            DCHECK(clientCallback_);
            if (clientCallback_->onSinkCancel()) {
              std::ignore = clientCallback_->onStreamComplete();
            }
          }
        },
        [&](...) { clientCallback_->onFirstResponseError(std::move(ew)); });
  }

  bool onSinkRequestN(int32_t) override { std::terminate(); }
  void resetServerCallback(BiDiServerCallback&) override { std::terminate(); }

  bool onSinkNext(StreamPayload&&) override { std::terminate(); }
  bool onSinkError(folly::exception_wrapper) override { std::terminate(); }
  bool onSinkComplete() override { std::terminate(); }
  bool onSinkCancel() override { std::terminate(); }
  void resetClientCallback(BiDiClientCallback& clientCallback) override {
    clientCallback_ = &clientCallback;
  }

  bool onStreamNext(StreamPayload&&) override { std::terminate(); }
  bool onStreamError(folly::exception_wrapper) override { std::terminate(); }
  bool onStreamComplete() override { std::terminate(); }

  bool onStreamCancel() override { std::terminate(); }
  bool onStreamRequestN(int32_t) override { std::terminate(); }

 private:
  const uint16_t protocolId_;
  const ManagedStringView methodName_;
  BiDiClientCallback* clientCallback_;
  folly::EventBase* evb_;
  rocket::RocketClient::DestructionGuardedClient guardedClient_;
};

int32_t getMetaKeepAliveTimeoutMs(RequestSetupMetadata& meta) {
  return meta.keepAliveTimeoutMs().value_or(0);
}

} // namespace

class RocketClientChannel::SingleRequestSingleResponseCallback final
    : public rocket::RocketClient::RequestResponseCallback {
  using clock = std::chrono::steady_clock;

 public:
  SingleRequestSingleResponseCallback(
      RequestClientCallback::Ptr cb,
      uint16_t protocolId,
      ManagedStringView&& methodName,
      size_t requestSerializedSize,
      size_t requestWireSize,
      size_t requestMetadataAndPayloadSize,
      bool encodeMetadataUsingBinary,
      rocket::RocketClient::DestructionGuardedClient guardedClient)
      : cb_(std::move(cb)),
        protocolId_(protocolId),
        methodName_(std::move(methodName)),
        requestSerializedSize_(requestSerializedSize),
        requestWireSize_(requestWireSize),
        requestMetadataAndPayloadSize_(requestMetadataAndPayloadSize),
        timeBeginSend_(clock::now()),
        encodeMetadataUsingBinary_(encodeMetadataUsingBinary),
        guardedClient_(std::move(guardedClient)) {}

  void onWriteSuccess() noexcept override { timeEndSend_ = clock::now(); }

  void onResponsePayload(
      folly::AsyncTransport* transport,
      folly::Try<rocket::Payload>&& payload) noexcept override {
    folly::Try<FirstResponsePayload> response;
    folly::Try<folly::SocketFds> tryFds;
    RpcTransportStats stats;
    stats.requestSerializedSizeBytes = requestSerializedSize_;
    stats.requestWireSizeBytes = requestWireSize_;
    stats.requestMetadataAndPayloadSizeBytes = requestMetadataAndPayloadSize_;
    stats.requestWriteLatency = timeEndSend_ - timeBeginSend_;
    stats.responseRoundTripLatency = clock::now() - timeEndSend_;
    ResponseSerializationHandler handler(protocolId_);
    if (payload.hasException()) {
      if (!payload.exception().with_exception<rocket::RocketException>(
              [&](auto& ex) {
                response = decodeResponseError(
                    std::move(ex),
                    handler,
                    *guardedClient_.client->getPayloadSerializer());
              })) {
        cb_.release()->onResponseError(std::move(payload.exception()));
        return;
      }
      if (response.hasException()) {
        cb_.release()->onResponseError(std::move(response.exception()));
        return;
      }
      const auto& metadata = response->metadata;
      if (metadata.fdMetadata().has_value()) {
        const auto& fdMetadata = *metadata.fdMetadata();
        tryFds = rocket::popReceivedFdsFromSocket(
            transport,
            fdMetadata.numFds().value_or(0),
            fdMetadata.fdSeqNum().value_or(folly::SocketFds::kNoSeqNum));
      }
    } else {
      stats.responseWireSizeBytes =
          payload->metadataAndDataSize() - payload->metadataSize();
      stats.responseMetadataAndPayloadSizeBytes =
          payload->metadataAndDataSize();

      response = guardedClient_.client->getPayloadSerializer()
                     ->unpack<FirstResponsePayload>(
                         std::move(*payload), encodeMetadataUsingBinary_);
      if (response.hasException()) {
        cb_.release()->onResponseError(std::move(response.exception()));
        return;
      }

      // Precedes `processFirstResponse` to promptly close the FDs on error.
      const auto& metadata = response->metadata;
      if (metadata.fdMetadata().has_value()) {
        const auto& fdMetadata = *metadata.fdMetadata();
        tryFds = rocket::popReceivedFdsFromSocket(
            transport,
            fdMetadata.numFds().value_or(0),
            fdMetadata.fdSeqNum().value_or(folly::SocketFds::kNoSeqNum));
      }

      if (auto error = processFirstResponse(
              protocolId_, response->metadata, response->payload, handler)) {
        cb_.release()->onResponseError(std::move(error));
        return;
      }
    }

    // Both `if` and `else` set this -- share the error handling.
    if (tryFds.hasException()) {
      cb_.release()->onResponseError(std::move(tryFds.exception()));
      return;
    }

    stats.responseSerializedSizeBytes =
        response->payload->computeChainDataLength();

    auto tHeader = std::make_unique<transport::THeader>();
    tHeader->setClientType(THRIFT_ROCKET_CLIENT_TYPE);
    if (tryFds.hasValue()) {
      tHeader->fds = std::move(tryFds->dcheckReceivedOrEmpty());
    }
    if (THRIFT_FLAG(rocket_client_enable_bidirectional_propagation)) {
      auto otherMetadata = response->metadata.otherMetadata();
      if (!otherMetadata ||
          !detail::ingestFrameworkMetadataOnResponseHeader(*otherMetadata)) {
        if (auto tfmr = response->metadata.frameworkMetadata()) {
          detail::ingestFrameworkMetadataOnResponse(std::move(*tfmr));
        }
      }
    }
    apache::thrift::detail::fillTHeaderFromResponseRpcMetadata(
        response->metadata, *tHeader);
    cb_.release()->onResponse(ClientReceiveState(
        protocolId_,
        handler.mtype,
        SerializedResponse(std::move(response->payload)),
        std::move(tHeader),
        nullptr, /* ctx */
        stats));
  }

 private:
  RequestClientCallback::Ptr cb_;
  const uint16_t protocolId_;
  ManagedStringView methodName_;
  const size_t requestSerializedSize_;
  const size_t requestWireSize_;
  const size_t requestMetadataAndPayloadSize_;
  const std::chrono::time_point<clock> timeBeginSend_;
  std::chrono::time_point<clock> timeEndSend_;
  const bool encodeMetadataUsingBinary_;
  rocket::RocketClient::DestructionGuardedClient guardedClient_;
};

class RocketClientChannel::SingleRequestNoResponseCallback final
    : public rocket::RocketClient::RequestFnfCallback {
 public:
  explicit SingleRequestNoResponseCallback(RequestClientCallback::Ptr cb)
      : cb_(std::move(cb)) {}

  void onWrite(folly::Try<void> writeResult) noexcept override {
    auto* cbPtr = cb_.release();
    if (writeResult.hasException()) {
      cbPtr->onResponseError(std::move(writeResult).exception());
    } else {
      cbPtr->onResponse({});
    }
  }

 private:
  RequestClientCallback::Ptr cb_;
};

RequestSetupMetadata RocketClientChannel::populateSetupMetadata(
    RequestSetupMetadata&& meta) {
  meta.maxVersion() =
      std::min(kRocketClientMaxVersion, THRIFT_FLAG(rocket_client_max_version));
  meta.minVersion() = kRocketClientMinVersion;
  auto& clientMetadata = meta.clientMetadata().ensure();

  if (const auto& hostMetadata = ClientChannel::getHostMetadata()) {
    // TODO: verify if we can avoid overriding hostname blindly
    // here.
    clientMetadata.hostname().from_optional(hostMetadata->hostname);
    // no otherMetadata provided in populateSetupMetadata override, copy
    // hostMetadata.otherMetadata directly instead of doing inserts
    if (!apache::thrift::get_pointer(clientMetadata.otherMetadata())) {
      clientMetadata.otherMetadata().from_optional(hostMetadata->otherMetadata);
    } else if (hostMetadata->otherMetadata) {
      DCHECK(clientMetadata.otherMetadata());
      // append values from hostMetadata.otherMetadata to
      // clientMetadata.otherMetadata
      clientMetadata.otherMetadata()->insert(
          hostMetadata->otherMetadata->begin(),
          hostMetadata->otherMetadata->end());
    }
  }

  if (!clientMetadata.agent()) {
    clientMetadata.agent() = "RocketClientChannel.cpp";
  }

  return meta;
}

RocketClientChannel::RocketClientChannel(
    folly::EventBase* eventBase,
    folly::AsyncTransport::UniquePtr socket,
    RequestSetupMetadata meta,
    int32_t keepAliveTimeoutMs,
    std::shared_ptr<rocket::ParserAllocatorType> allocatorPtr)
    : rocket::RocketClient(
          *eventBase,
          std::move(socket),
          populateSetupMetadata(std::move(meta)),
          keepAliveTimeoutMs,
          allocatorPtr),
      evb_(eventBase) {
  apache::thrift::detail::hookForClientTransport(getTransport());
}

RocketClientChannel::~RocketClientChannel() {
  if (evb_) {
    evb_->dcheckIsInEventBaseThread();
  }
  unsetOnDetachable();
  closeNow();
}

void RocketClientChannel::setFlushList(FlushList* flushList) {
  rocket::RocketClient::setFlushList(flushList);
}

RocketClientChannel::Ptr RocketClientChannel::newChannel(
    folly::AsyncTransport::UniquePtr socket) {
  auto evb = socket->getEventBase();
  return RocketClientChannel::Ptr(
      new RocketClientChannel(evb, std::move(socket), RequestSetupMetadata()));
}

RocketClientChannel::Ptr RocketClientChannel::newChannelWithMetadata(
    folly::AsyncTransport::UniquePtr socket, RequestSetupMetadata meta) {
  auto evb = socket->getEventBase();
  auto keepAliveTimeoutMs = getMetaKeepAliveTimeoutMs(meta);
  return RocketClientChannel::Ptr(new RocketClientChannel(
      evb, std::move(socket), std::move(meta), keepAliveTimeoutMs));
}

void RocketClientChannel::sendRequestResponse(
    const RpcOptions& rpcOptions,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr cb,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendThriftRequest(
      rpcOptions,
      RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      std::move(cb),
      std::move(frameworkMetadata));
}

void RocketClientChannel::sendRequestNoResponse(
    const RpcOptions& rpcOptions,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr cb,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendThriftRequest(
      rpcOptions,
      RpcKind::SINGLE_REQUEST_NO_RESPONSE,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      std::move(cb),
      std::move(frameworkMetadata));
}

void RocketClientChannel::sendRequestStream(
    const RpcOptions& rpcOptions,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<THeader> header,
    StreamClientCallback* clientCallback,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendThriftRequest(
      rpcOptions,
      RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      clientCallback,
      std::move(frameworkMetadata));
}

void RocketClientChannel::sendRequestSink(
    const RpcOptions& rpcOptions,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    SinkClientCallback* clientCallback,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendThriftRequest(
      rpcOptions,
      RpcKind::SINK,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      clientCallback,
      std::move(frameworkMetadata));
}

void RocketClientChannel::sendRequestBiDi(
    RpcOptions&& rpcOptions,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    BiDiClientCallback* clientCallback,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendThriftRequest(
      rpcOptions,
      RpcKind::BIDIRECTIONAL_STREAM,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      clientCallback,
      std::move(frameworkMetadata));
}

template <typename Callback>
void RocketClientChannel::sendThriftRequest(
    const RpcOptions& rpcOptions,
    RpcKind kind,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    Callback clientCallback,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  DestructorGuard dg(this);
  if (!canHandleRequest(clientCallback)) {
    return;
  }
  preprocessHeader(header.get());
  auto timeout = getClientTimeout(rpcOptions);
  auto buf = std::move(request.buffer);
  auto metadata = apache::thrift::detail::makeRequestRpcMetadata(
      rpcOptions,
      kind,
      std::move(methodMetadata),
      timeout,
      getInteractionHandle(rpcOptions),
      getServerZstdSupported(),
      buf->computeChainDataLength(),
      *header,
      std::move(frameworkMetadata),
      customCompressor_ != nullptr);

  size_t requestSerializedSize;
  // Avoid unnecessary computation for streaming methods.
  if constexpr (std::is_same_v<Callback, RequestClientCallback::Ptr>) {
    requestSerializedSize = buf->computeChainDataLength();
  }
  rocket::Payload requestPayload;
  try {
    requestPayload = getPayloadSerializer()->packWithFds(
        &metadata,
        std::move(buf),
        rpcOptions.copySocketFdsToSend(),
        encodeMetadataUsingBinary(),
        getTransportWrapper());
    requestPayload.setDataFirstFieldAlignment(
        rpcOptions.getFrameRelativeDataAlignment());
    if (metadata.protocol()) {
      requestPayload.setDataSerializationProtocol(
          static_cast<ProtocolType>(*metadata.protocol()));
    }
  } catch (std::exception const& ex) {
    auto err = folly::make_exception_wrapper<transport::TTransportException>(
        transport::TTransportException::TTransportExceptionType::UNKNOWN,
        fmt::format("Failed to pack request payload: {}", ex.what()));
    if constexpr (std::is_same_v<Callback, RequestClientCallback::Ptr>) {
      clientCallback.release()->onResponseError(std::move(err));
    } else {
      clientCallback->onFirstResponseError(std::move(err));
    }
    return;
  }

  if constexpr (std::is_same_v<Callback, RequestClientCallback::Ptr>) {
    header.reset(); // avoid extending lifetime past fiber suspension in send*
    switch (kind) {
      case RpcKind::SINGLE_REQUEST_NO_RESPONSE:
        sendSingleRequestNoResponse(
            rpcOptions,
            std::move(metadata),
            std::move(requestPayload),
            std::move(clientCallback));
        break;

      case RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE:
        sendSingleRequestSingleResponse(
            rpcOptions,
            std::move(metadata),
            timeout.value_or(std::chrono::milliseconds(0)),
            requestSerializedSize,
            std::move(requestPayload),
            std::move(clientCallback));
        break;

      default:
        folly::assume_unreachable();
    }
  } else if constexpr (std::is_same_v<Callback, StreamClientCallback*>) {
    DCHECK(kind == RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE);
    sendStreamRequest(
        rpcOptions,
        std::move(metadata),
        timeout.value_or(std::chrono::milliseconds(0)),
        std::move(requestPayload),
        header->getProtocolId(),
        clientCallback);
  } else if constexpr (std::is_same_v<Callback, SinkClientCallback*>) {
    DCHECK(kind == RpcKind::SINK);
    sendSinkRequest(
        rpcOptions,
        std::move(metadata),
        timeout.value_or(std::chrono::milliseconds(0)),
        std::move(requestPayload),
        header->getProtocolId(),
        header->getDesiredCompressionConfig(),
        clientCallback);
  } else if constexpr (std::is_same_v<Callback, BiDiClientCallback*>) {
    DCHECK(kind == RpcKind::BIDIRECTIONAL_STREAM);
    sendBiDiRequest(
        rpcOptions,
        std::move(metadata),
        timeout.value_or(std::chrono::milliseconds(0)),
        std::move(requestPayload),
        header->getProtocolId(),
        header->getDesiredCompressionConfig(),
        clientCallback);
  } else {
    static_assert(folly::always_false<Callback>, "Unsupported callback type");
  }
}

void RocketClientChannel::sendSingleRequestNoResponse(
    const RpcOptions& rpcOptions,
    RequestRpcMetadata&&,
    rocket::Payload requestPayload,
    RequestClientCallback::Ptr cb) {
  const bool isSync = cb->isSync() || rpcOptions.getForceSyncOnFiber();
  SingleRequestNoResponseCallback callback(std::move(cb));

  if (isSync && folly::fibers::onFiber()) {
    callback.onWrite(
        rocket::RocketClient::sendRequestFnfSync(std::move(requestPayload)));
  } else {
    rocket::RocketClient::sendRequestFnf(
        std::move(requestPayload),
        folly::copy_to_unique_ptr(std::move(callback)));
  }
}

void RocketClientChannel::sendSingleRequestSingleResponse(
    const RpcOptions& rpcOptions,
    RequestRpcMetadata&& metadata,
    std::chrono::milliseconds timeout,
    size_t requestSerializedSize,
    rocket::Payload requestPayload,
    RequestClientCallback::Ptr cb) {
  const auto requestWireSize = requestPayload.dataSize();
  const auto requestMetadataAndPayloadSize =
      requestPayload.metadataAndDataSize();
  const bool isSync = cb->isSync() || rpcOptions.getForceSyncOnFiber();
  SingleRequestSingleResponseCallback callback(
      std::move(cb),
      static_cast<uint16_t>(*metadata.protocol()),
      std::move(*metadata.name()),
      requestSerializedSize,
      requestWireSize,
      requestMetadataAndPayloadSize,
      encodeMetadataUsingBinary(),
      DestructionGuardedClient(this));

  if (isSync && folly::fibers::onFiber()) {
    callback.onResponsePayload(
        getTransportWrapper(),
        rocket::RocketClient::sendRequestResponseSync(
            std::move(requestPayload), timeout, &callback));
  } else {
    rocket::RocketClient::sendRequestResponse(
        std::move(requestPayload),
        timeout,
        folly::copy_to_unique_ptr(std::move(callback)));
  }
}

void RocketClientChannel::sendStreamRequest(
    const RpcOptions& rpcOptions,
    RequestRpcMetadata&& metadata,
    std::chrono::milliseconds firstResponseTimeout,
    rocket::Payload requestPayload,
    uint16_t protocolId,
    StreamClientCallback* clientCallback) {
  assert(metadata.name());
  rocket::RocketClient::sendRequestStream(
      std::move(requestPayload),
      firstResponseTimeout,
      rpcOptions.getChunkTimeout(),
      rpcOptions.getChunkBufferSize(),
      new FirstRequestProcessorStream(
          protocolId,
          std::move(*metadata.name()),
          clientCallback,
          evb_,
          DestructionGuardedClient(this)));
}

void RocketClientChannel::sendSinkRequest(
    const RpcOptions&,
    RequestRpcMetadata&& metadata,
    std::chrono::milliseconds firstResponseTimeout,
    rocket::Payload requestPayload,
    uint16_t protocolId,
    folly::Optional<CompressionConfig> compressionConfig,
    SinkClientCallback* clientCallback) {
  assert(metadata.name());
  rocket::RocketClient::sendRequestSink(
      std::move(requestPayload),
      firstResponseTimeout,
      new FirstRequestProcessorSink(
          protocolId,
          std::move(*metadata.name()),
          clientCallback,
          evb_,
          DestructionGuardedClient(this)),
      std::move(compressionConfig));
}

void RocketClientChannel::sendBiDiRequest(
    const RpcOptions& rpcOptions,
    RequestRpcMetadata&& metadata,
    std::chrono::milliseconds firstResponseTimeout,
    rocket::Payload requestPayload,
    uint16_t protocolId,
    folly::Optional<CompressionConfig> compressionConfig,
    BiDiClientCallback* clientCallback) {
  assert(metadata.name());
  rocket::RocketClient::sendRequestBiDi(
      std::move(requestPayload),
      firstResponseTimeout,
      rpcOptions.getChunkBufferSize(),
      new FirstRequestProcessorBiDi(
          protocolId,
          std::move(*metadata.name()),
          clientCallback,
          evb_,
          DestructionGuardedClient(this)),
      std::move(compressionConfig));
}

void onResponseError(
    RequestClientCallback::Ptr& cb, folly::exception_wrapper ew) {
  cb.release()->onResponseError(std::move(ew));
}

void onResponseError(StreamClientCallback* cb, folly::exception_wrapper ew) {
  cb->onFirstResponseError(std::move(ew));
}

void onResponseError(SinkClientCallback* cb, folly::exception_wrapper ew) {
  cb->onFirstResponseError(std::move(ew));
}

void onResponseError(BiDiClientCallback* cb, folly::exception_wrapper ew) {
  cb->onFirstResponseError(std::move(ew));
}

template <typename CallbackPtr>
bool RocketClientChannel::canHandleRequest(CallbackPtr& cb) {
  if (!isAlive()) {
    // Channel is not in connected state due to some pre-existing transport
    // exception, pass it back for some breadcrumbs.
    onResponseError(
        cb,
        folly::make_exception_wrapper<TTransportException>(
            TTransportException::NOT_OPEN,
            folly::sformat(
                "Connection not open: {}",
                rocket::RocketClient::getLastError().what())));
    return false;
  }

  if (inflightRequestsAndStreams() >= maxInflightRequestsAndStreams_) {
    TTransportException ex(
        TTransportException::NETWORK_ERROR,
        "Too many active requests on connection");
    // Might be able to create another transaction soon
    ex.setOptions(TTransportException::CHANNEL_IS_VALID);
    onResponseError(cb, std::move(ex));
    return false;
  }

  return true;
}

std::optional<std::chrono::milliseconds> RocketClientChannel::getClientTimeout(
    const RpcOptions& rpcOptions) const {
  if (rpcOptions.getTimeout() > std::chrono::milliseconds::zero()) {
    return rpcOptions.getTimeout();
  } else if (timeout_ > std::chrono::milliseconds::zero()) {
    return timeout_;
  }
  return std::nullopt;
}

std::variant<InteractionCreate, int64_t, std::monostate>
RocketClientChannel::getInteractionHandle(const RpcOptions& rpcOptions) {
  if (auto interactionId = rpcOptions.getInteractionId()) {
    evb_->dcheckIsInEventBaseThread();
    if (auto* name = folly::get_ptr(pendingInteractions_, interactionId)) {
      InteractionCreate create;
      create.interactionId() = interactionId;
      create.interactionName() = std::move(*name).str();
      pendingInteractions_.erase(interactionId);
      return create;
    } else {
      return interactionId;
    }
  }

  return std::monostate{};
}

ClientChannel::SaturationStatus RocketClientChannel::getSaturationStatus() {
  DCHECK(evb_);
  evb_->dcheckIsInEventBaseThread();
  return ClientChannel::SaturationStatus(
      inflightRequestsAndStreams(), maxInflightRequestsAndStreams_);
}

void RocketClientChannel::closeNow() {
  if (evb_) {
    evb_->dcheckIsInEventBaseThread();
  }
  rocket::RocketClient::closeNow(
      transport::TTransportException(
          transport::TTransportException::INTERRUPTED, "Client shutdown."));
}

void RocketClientChannel::setCloseCallback(CloseCallback* closeCallback) {
  rocket::RocketClient::setCloseCallback([closeCallback] {
    if (closeCallback) {
      closeCallback->channelClosed();
    }
  });
}

folly::AsyncTransport* FOLLY_NULLABLE RocketClientChannel::getTransport() {
  auto* transportWrapper = rocket::RocketClient::getTransportWrapper();
  return transportWrapper
      ? transportWrapper->getUnderlyingTransport<folly::AsyncTransport>()
      : nullptr;
}

bool RocketClientChannel::good() {
  if (evb_) {
    evb_->dcheckIsInEventBaseThread();
  }
  return isAlive();
}

size_t RocketClientChannel::inflightRequestsAndStreams() const {
  return streams() + requests();
}

void RocketClientChannel::setTimeout(uint32_t timeoutMs) {
  if (evb_) {
    evb_->dcheckIsInEventBaseThread();
  }
  if (auto* transport = getTransport()) {
    transport->setSendTimeout(timeoutMs);
  }
  timeout_ = std::chrono::milliseconds(timeoutMs);
}

void RocketClientChannel::attachEventBase(folly::EventBase* evb) {
  evb->dcheckIsInEventBaseThread();
  if (getTransportWrapper()) {
    rocket::RocketClient::attachEventBase(*evb);
  }
  evb_ = evb;
}

void RocketClientChannel::detachEventBase() {
  DCHECK(isDetachable());
  DCHECK(getDestructorGuardCount() == 0);

  if (getTransportWrapper()) {
    rocket::RocketClient::detachEventBase();
  }
  evb_ = nullptr;
}

bool RocketClientChannel::isDetachable() {
  if (evb_) {
    evb_->dcheckIsInEventBaseThread();
  }
  auto* transport = getTransport();
  return !evb_ || !transport ||
      (rocket::RocketClient::isDetachable() && pendingInteractions_.empty());
}

void RocketClientChannel::setOnDetachable(
    folly::Function<void()> onDetachable) {
  DCHECK(getTransportWrapper());
  ClientChannel::setOnDetachable(std::move(onDetachable));
  rocket::RocketClient::setOnDetachable([this] {
    if (isDetachable()) {
      notifyDetachable();
    }
  });
}

void RocketClientChannel::unsetOnDetachable() {
  ClientChannel::unsetOnDetachable();
  rocket::RocketClient::setOnDetachable(nullptr);
}

void RocketClientChannel::terminateInteraction(InteractionId id) {
  evb_->dcheckIsInEventBaseThread();
  auto pending = pendingInteractions_.find(id);
  if (pending != pendingInteractions_.end()) {
    pendingInteractions_.erase(pending);
    releaseInteractionId(std::move(id));
    return;
  }
  rocket::RocketClient::terminateInteraction(id);
  releaseInteractionId(std::move(id));
}

InteractionId RocketClientChannel::registerInteraction(
    ManagedStringView&& name, int64_t id) {
  CHECK(!name.view().empty());
  CHECK_GT(id, 0);
  evb_->dcheckIsInEventBaseThread();

  auto res = pendingInteractions_.insert({id, std::move(name)});
  DCHECK(res.second);

  rocket::RocketClient::addInteraction();

  return createInteractionId(id);
}

int32_t RocketClientChannel::getServerVersion() const {
  return rocket::RocketClient::getServerVersion();
}
} // namespace apache::thrift
