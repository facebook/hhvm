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
#include <folly/GLog.h>
#include <folly/Memory.h>
#include <folly/Range.h>
#include <folly/Try.h>
#include <folly/compression/Compression.h>
#include <folly/fibers/FiberManager.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/Request.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/protocol/TBase64Utils.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/HeaderChannel.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/transport/core/EnvelopeUtil.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>
#include <thrift/lib/cpp2/transport/core/ThriftClientCallback.h>
#include <thrift/lib/cpp2/transport/core/TryUtil.h>
#include <thrift/lib/cpp2/transport/rocket/FdSocket.h>
#include <thrift/lib/cpp2/transport/rocket/PayloadUtils.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>
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

THRIFT_FLAG_DEFINE_bool(rocket_client_new_protocol_key, true);
THRIFT_FLAG_DEFINE_int64(rocket_client_max_version, kRocketClientMaxVersion);
THRIFT_FLAG_DEFINE_bool(rocket_client_rocket_skip_protocol_key, false);

using namespace apache::thrift::transport;

namespace apache {
namespace thrift {

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
    rocket::RocketException&& ex, Handler&& handler) noexcept {
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
    rocket::unpackCompact(responseError, ex.moveErrorData().get());
  } catch (...) {
    return folly::Try<FirstResponsePayload>(
        folly::make_exception_wrapper<TApplicationException>(fmt::format(
            "Error parsing error frame: {}",
            folly::exceptionStr(std::current_exception()).toStdString())));
  }

  folly::Optional<std::string> exCode;
  TApplicationException::TApplicationExceptionType exType{
      TApplicationException::UNKNOWN};
  switch (responseError.code_ref().value_or(ResponseRpcErrorCode::UNKNOWN)) {
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
    default:
      exCode = kUnknownErrorCode;
      break;
  }

  ResponseRpcMetadata metadata;
  if (exCode) {
    metadata.otherMetadata_ref().emplace();
    (*metadata.otherMetadata_ref())[detail::kHeaderEx] = *exCode;
  }
  if (auto loadRef = responseError.load_ref()) {
    metadata.load_ref() = *loadRef;
  }
  return folly::Try<FirstResponsePayload>(FirstResponsePayload(
      handler.handleException(TApplicationException(
          exType, responseError.what_utf8_ref().value_or(""))),
      std::move(metadata)));
}

template <class Handler>
FOLLY_NODISCARD folly::exception_wrapper processFirstResponse(
    uint16_t protocolId,
    ResponseRpcMetadata& metadata,
    std::unique_ptr<folly::IOBuf>& payload,
    Handler& handler) {
  (void)protocolId;
  if (auto payloadMetadataRef = metadata.payloadMetadata_ref()) {
    const auto isProxiedResponse =
        metadata.proxiedPayloadMetadata_ref().has_value();

    switch (payloadMetadataRef->getType()) {
      case PayloadMetadata::Type::responseMetadata:
        payload = handler.handleReply(std::move(payload));
        break;
      case PayloadMetadata::Type::exceptionMetadata: {
        auto& exceptionMetadataBase =
            payloadMetadataRef->get_exceptionMetadata();
        auto otherMetadataRef = metadata.otherMetadata_ref();
        if (!otherMetadataRef) {
          otherMetadataRef.emplace();
        }
        auto exceptionNameRef = exceptionMetadataBase.name_utf8_ref();
        auto exceptionWhatRef = exceptionMetadataBase.what_utf8_ref();
        if (auto exceptionMetadataRef = exceptionMetadataBase.metadata_ref()) {
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
              if (auto dExClass = exceptionMetadataRef->declaredException_ref()
                                      ->errorClassification_ref()) {
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
                    folly::exceptionStr(std::current_exception())
                        .toStdString());
              }
              if (*anyException.protocol_ref() == type::kNoProtocol) {
                anyException.protocol_ref() =
                    protocolId == protocol::T_COMPACT_PROTOCOL
                    ? type::Protocol::get<type::StandardProtocol::Compact>()
                    : type::Protocol::get<type::StandardProtocol::Binary>();
              }
              auto exceptionTypeRef = anyException.type_ref()
                                          ->toThrift()
                                          .name_ref()
                                          ->exceptionType_ref();
              if (exceptionTypeRef && exceptionTypeRef->uri_ref() &&
                  anyException.protocol_ref() ==
                      type::Protocol::get<type::StandardProtocol::Compact>()) {
                (*otherMetadataRef)
                    [isProxiedResponse ? detail::kHeaderProxiedAnyexType
                                       : detail::kHeaderAnyexType] =
                        *exceptionTypeRef->uri_ref();
                (*otherMetadataRef)
                    [isProxiedResponse ? detail::kHeaderProxiedAnyex
                                       : detail::kHeaderAnyex] =
                        protocol::base64Encode(
                            anyException.data_ref()->coalesce());
              }
              payload = handler.handleException(
                  TApplicationException(exceptionWhatRef.value_or("")));
              break;
            }
#endif
            default:
              switch (metaType) {
                case PayloadExceptionMetadata::Type::appUnknownException:
                  if (auto ec = exceptionMetadataRef->appUnknownException_ref()
                                    ->errorClassification_ref()) {
                    if (ec->blame_ref() &&
                        *ec->blame_ref() == ErrorBlame::CLIENT) {
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
      folly::EventBase* evb)
      : protocolId_(protocolId),
        methodName_(std::move(methodName)),
        clientCallback_(clientCallback),
        evb_(evb) {}

  FOLLY_NODISCARD bool onFirstResponse(
      FirstResponsePayload&& firstResponse,
      folly::EventBase* evb,
      StreamServerCallback* serverCallback) override {
    SCOPE_EXIT { delete this; };
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
    SCOPE_EXIT { delete this; };
    ew.handle(
        [&](rocket::RocketException& ex) {
          LegacyResponseSerializationHandler handler(
              protocolId_, methodName_.view());
          auto response = decodeResponseError(std::move(ex), handler);
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
  bool onStreamRequestN(uint64_t) override { return true; }
  void resetClientCallback(StreamClientCallback& clientCallback) override {
    clientCallback_ = &clientCallback;
  }

 private:
  const uint16_t protocolId_;
  const ManagedStringView methodName_;
  StreamClientCallback* clientCallback_;
  folly::EventBase* evb_;
};

class FirstRequestProcessorSink : public SinkClientCallback,
                                  private SinkServerCallback {
 public:
  FirstRequestProcessorSink(
      uint16_t protocolId,
      apache::thrift::ManagedStringView&& methodName,
      SinkClientCallback* clientCallback,
      folly::EventBase* evb)
      : protocolId_(protocolId),
        methodName_(std::move(methodName)),
        clientCallback_(clientCallback),
        evb_(evb) {}

  FOLLY_NODISCARD bool onFirstResponse(
      FirstResponsePayload&& firstResponse,
      folly::EventBase* evb,
      SinkServerCallback* serverCallback) override {
    SCOPE_EXIT { delete this; };
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
    SCOPE_EXIT { delete this; };
    ew.handle(
        [&](rocket::RocketException& ex) {
          LegacyResponseSerializationHandler handler(
              protocolId_, methodName_.view());
          auto response = decodeResponseError(std::move(ex), handler);
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
  FOLLY_NODISCARD bool onSinkRequestN(uint64_t) override { std::terminate(); }
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
};
} // namespace

void RocketClientChannel::setCompression(
    RequestRpcMetadata& metadata, ssize_t payloadSize) {
  if (auto compressionConfig = metadata.compressionConfig_ref()) {
    if (auto codecRef = compressionConfig->codecConfig_ref()) {
      if (codecRef->getType() ==
              apache::thrift::CodecConfig::Type::zlibConfig &&
          getServerZstdSupported()) {
        codecRef->zstdConfig_ref().emplace();
      }
      if (payloadSize >
          compressionConfig->compressionSizeLimit_ref().value_or(0)) {
        switch (codecRef->getType()) {
          case CodecConfig::Type::zlibConfig:
            metadata.compression_ref() = CompressionAlgorithm::ZLIB;
            break;
          case CodecConfig::Type::zstdConfig:
            metadata.compression_ref() = CompressionAlgorithm::ZSTD;
            break;
          default:
            break;
        }
      }
    }
  }
}

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
      size_t requestMetadataAndPayloadSize)
      : cb_(std::move(cb)),
        protocolId_(protocolId),
        methodName_(std::move(methodName)),
        requestSerializedSize_(requestSerializedSize),
        requestWireSize_(requestWireSize),
        requestMetadataAndPayloadSize_(requestMetadataAndPayloadSize),
        timeBeginSend_(clock::now()) {}

  void onWriteSuccess() noexcept override { timeEndSend_ = clock::now(); }

  void onResponsePayload(
      folly::AsyncTransport* transport,
      folly::Try<rocket::Payload>&& payload) noexcept override {
    folly::Try<FirstResponsePayload> response;
    folly::Try<folly::SocketFds> tryFds;
    RpcSizeStats stats;
    stats.requestSerializedSizeBytes = requestSerializedSize_;
    stats.requestWireSizeBytes = requestWireSize_;
    stats.requestMetadataAndPayloadSizeBytes = requestMetadataAndPayloadSize_;
    stats.requestLatency = timeEndSend_ - timeBeginSend_;
    stats.responseLatency = clock::now() - timeEndSend_;
    ResponseSerializationHandler handler(protocolId_);
    if (payload.hasException()) {
      if (!payload.exception().with_exception<rocket::RocketException>(
              [&](auto& ex) {
                response = decodeResponseError(std::move(ex), handler);
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

      response = rocket::unpack<FirstResponsePayload>(std::move(*payload));
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

rocket::SetupFrame RocketClientChannel::makeSetupFrame(
    RequestSetupMetadata meta) {
  meta.maxVersion_ref() =
      std::min(kRocketClientMaxVersion, THRIFT_FLAG(rocket_client_max_version));
  meta.minVersion_ref() = kRocketClientMinVersion;
  auto& clientMetadata = meta.clientMetadata_ref().ensure();

  if (const auto& hostMetadata = ClientChannel::getHostMetadata()) {
    // TODO: verify if we can avoid overriding hostname blindly
    // here.
    clientMetadata.hostname_ref().from_optional(hostMetadata->hostname);
    // no otherMetadata provided in makeSetupFrame override, copy
    // hostMetadata.otherMetadata directly instead of doing inserts
    if (!clientMetadata.get_otherMetadata()) {
      clientMetadata.otherMetadata_ref().from_optional(
          hostMetadata->otherMetadata);
    } else if (hostMetadata->otherMetadata) {
      DCHECK(clientMetadata.otherMetadata_ref());
      // append values from hostMetadata.otherMetadata to
      // clientMetadata.otherMetadata
      clientMetadata.otherMetadata_ref()->insert(
          hostMetadata->otherMetadata->begin(),
          hostMetadata->otherMetadata->end());
    }
  }

  if (!clientMetadata.agent_ref()) {
    clientMetadata.agent_ref() = "RocketClientChannel.cpp";
  }
  CompactProtocolWriter compactProtocolWriter;
  folly::IOBufQueue paramQueue;
  compactProtocolWriter.setOutput(&paramQueue);
  meta.write(&compactProtocolWriter);

  // Serialize RocketClient's major/minor version (which is separate from the
  // rsocket protocol major/minor version) into setup metadata.
  auto buf = folly::IOBuf::createCombined(
      sizeof(int32_t) + meta.serializedSize(&compactProtocolWriter));
  folly::IOBufQueue queue;
  queue.append(std::move(buf));
  folly::io::QueueAppender appender(&queue, /* do not grow */ 0);
  if (!THRIFT_FLAG(rocket_client_rocket_skip_protocol_key)) {
    const uint32_t protocolKey = THRIFT_FLAG(rocket_client_new_protocol_key)
        ? RpcMetadata_constants::kRocketProtocolKey()
        : 1;

    appender.writeBE<uint32_t>(protocolKey); // Rocket protocol key
  }
  // Append serialized setup parameters to setup frame metadata
  appender.insert(paramQueue.move());

  return rocket::SetupFrame(
      rocket::Payload::makeFromMetadataAndData(queue.move(), {}),
      /* rocketMimeTypes = */ true);
}

RocketClientChannel::RocketClientChannel(
    folly::EventBase* eventBase,
    folly::AsyncTransport::UniquePtr socket,
    RequestSetupMetadata meta)
    : rocket::RocketClient(
          *eventBase,
          std::move(socket),
          std::make_unique<rocket::SetupFrame>(
              makeSetupFrame(std::move(meta)))),
      evb_(eventBase) {
  apache::thrift::detail::hookForClientTransport(getTransport());
}

RocketClientChannel::~RocketClientChannel() {
  DCHECK(!evb_ || evb_->isInEventBaseThread());
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
  return RocketClientChannel::Ptr(
      new RocketClientChannel(evb, std::move(socket), std::move(meta)));
}

void RocketClientChannel::sendRequestResponse(
    const RpcOptions& rpcOptions,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr cb) {
  sendThriftRequest(
      rpcOptions,
      RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      std::move(methodMetadata).name_managed(),
      std::move(request),
      std::move(header),
      std::move(cb));
}

void RocketClientChannel::sendRequestNoResponse(
    const RpcOptions& rpcOptions,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr cb) {
  sendThriftRequest(
      rpcOptions,
      RpcKind::SINGLE_REQUEST_NO_RESPONSE,
      std::move(methodMetadata).name_managed(),
      std::move(request),
      std::move(header),
      std::move(cb));
}

void RocketClientChannel::sendRequestStream(
    const RpcOptions& rpcOptions,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<THeader> header,
    StreamClientCallback* clientCallback) {
  DestructorGuard dg(this);

  preprocessHeader(header.get());

  auto metadata = apache::thrift::detail::makeRequestRpcMetadata(
      rpcOptions,
      RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE,
      static_cast<ProtocolId>(header->getProtocolId()),
      methodMetadata.name_managed(),
      timeout_,
      *header);

  std::chrono::milliseconds firstResponseTimeout;
  if (!preSendValidation(
          metadata, rpcOptions, clientCallback, firstResponseTimeout)) {
    return;
  }

  auto buf = std::move(request.buffer);
  setCompression(metadata, buf->computeChainDataLength());

  auto payload = rocket::packWithFds(
      &metadata,
      std::move(buf),
      rpcOptions.copySocketFdsToSend(),
      getTransportWrapper());
  assert(metadata.name_ref());
  return rocket::RocketClient::sendRequestStream(
      std::move(payload),
      firstResponseTimeout,
      rpcOptions.getChunkTimeout(),
      rpcOptions.getChunkBufferSize(),
      new FirstRequestProcessorStream(
          header->getProtocolId(),
          std::move(*metadata.name_ref()),
          clientCallback,
          evb_));
}

void RocketClientChannel::sendRequestSink(
    const RpcOptions& rpcOptions,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    SinkClientCallback* clientCallback) {
  DestructorGuard dg(this);

  preprocessHeader(header.get());

  auto metadata = apache::thrift::detail::makeRequestRpcMetadata(
      rpcOptions,
      RpcKind::SINK,
      static_cast<ProtocolId>(header->getProtocolId()),
      methodMetadata.name_managed(),
      timeout_,
      *header);

  std::chrono::milliseconds firstResponseTimeout;
  if (!preSendValidation(
          metadata, rpcOptions, clientCallback, firstResponseTimeout)) {
    return;
  }

  auto buf = std::move(request.buffer);
  setCompression(metadata, buf->computeChainDataLength());

  auto payload = rocket::packWithFds(
      &metadata,
      std::move(buf),
      rpcOptions.copySocketFdsToSend(),
      getTransportWrapper());
  assert(metadata.name_ref());
  return rocket::RocketClient::sendRequestSink(
      std::move(payload),
      firstResponseTimeout,
      new FirstRequestProcessorSink(
          header->getProtocolId(),
          std::move(*metadata.name_ref()),
          clientCallback,
          evb_),
      // rpcOptions.getMemAllocType(),
      header->getDesiredCompressionConfig());
}

void RocketClientChannel::sendThriftRequest(
    const RpcOptions& rpcOptions,
    RpcKind kind,
    apache::thrift::ManagedStringView&& methodName,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr cb) {
  DestructorGuard dg(this);

  preprocessHeader(header.get());

  auto metadata = apache::thrift::detail::makeRequestRpcMetadata(
      rpcOptions,
      kind,
      static_cast<ProtocolId>(header->getProtocolId()),
      std::move(methodName),
      timeout_,
      *header);
  header.reset();

  std::chrono::milliseconds timeout;
  if (!preSendValidation(metadata, rpcOptions, cb, timeout)) {
    return;
  }

  auto buf = std::move(request.buffer);
  setCompression(metadata, buf->computeChainDataLength());

  switch (kind) {
    case RpcKind::SINGLE_REQUEST_NO_RESPONSE:
      sendSingleRequestNoResponse(
          rpcOptions, std::move(metadata), std::move(buf), std::move(cb));
      break;

    case RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE:
      sendSingleRequestSingleResponse(
          rpcOptions,
          std::move(metadata),
          timeout,
          std::move(buf),
          std::move(cb));
      break;

    case RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE:
      // should no longer reach here anymore, use sendRequestStream
      DCHECK(false);
      break;

    default:
      folly::assume_unreachable();
  }
}

void RocketClientChannel::sendSingleRequestNoResponse(
    const RpcOptions& rpcOptions,
    RequestRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> buf,
    RequestClientCallback::Ptr cb) {
  auto requestPayload = rocket::packWithFds(
      &metadata,
      std::move(buf),
      rpcOptions.copySocketFdsToSend(),
      getTransportWrapper());
  const bool isSync = cb->isSync();
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
    std::unique_ptr<folly::IOBuf> buf,
    RequestClientCallback::Ptr cb) {
  const auto requestSerializedSize = buf->computeChainDataLength();
  auto requestPayload = rocket::packWithFds(
      &metadata,
      std::move(buf),
      rpcOptions.copySocketFdsToSend(),
      getTransportWrapper());
  const auto requestWireSize = requestPayload.dataSize();
  const auto requestMetadataAndPayloadSize =
      requestPayload.metadataAndDataSize();
  const bool isSync = cb->isSync();
  assert(metadata.protocol_ref());
  assert(metadata.name_ref());
  SingleRequestSingleResponseCallback callback(
      std::move(cb),
      static_cast<uint16_t>(*metadata.protocol_ref()),
      std::move(*metadata.name_ref()),
      requestSerializedSize,
      requestWireSize,
      requestMetadataAndPayloadSize);

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

template <typename CallbackPtr>
bool RocketClientChannel::preSendValidation(
    RequestRpcMetadata& metadata,
    const RpcOptions& rpcOptions,
    CallbackPtr& cb,
    std::chrono::milliseconds& firstResponseTimeout) {
  DCHECK(metadata.kind_ref().has_value());

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

  firstResponseTimeout =
      std::chrono::milliseconds(metadata.clientTimeoutMs_ref().value_or(0));
  if (rpcOptions.getClientOnlyTimeouts()) {
    metadata.clientTimeoutMs_ref().reset();
    metadata.queueTimeoutMs_ref().reset();
  }

  if (auto interactionId = rpcOptions.getInteractionId()) {
    evb_->dcheckIsInEventBaseThread();
    if (auto* name = folly::get_ptr(pendingInteractions_, interactionId)) {
      InteractionCreate create;
      create.interactionId_ref() = interactionId;
      create.interactionName_ref() = std::move(*name).str();
      metadata.interactionCreate_ref() = std::move(create);
      pendingInteractions_.erase(interactionId);
    } else {
      metadata.interactionId_ref() = interactionId;
    }
  }

  return true;
}

ClientChannel::SaturationStatus RocketClientChannel::getSaturationStatus() {
  DCHECK(evb_ && evb_->isInEventBaseThread());
  return ClientChannel::SaturationStatus(
      inflightRequestsAndStreams(), maxInflightRequestsAndStreams_);
}

void RocketClientChannel::closeNow() {
  DCHECK(!evb_ || evb_->isInEventBaseThread());
  rocket::RocketClient::closeNow(transport::TTransportException(
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
  DCHECK(!evb_ || evb_->isInEventBaseThread());
  return isAlive();
}

size_t RocketClientChannel::inflightRequestsAndStreams() const {
  return streams() + requests();
}

void RocketClientChannel::setTimeout(uint32_t timeoutMs) {
  DCHECK(!evb_ || evb_->isInEventBaseThread());
  if (auto* transport = getTransport()) {
    transport->setSendTimeout(timeoutMs);
  }
  timeout_ = std::chrono::milliseconds(timeoutMs);
}

void RocketClientChannel::attachEventBase(folly::EventBase* evb) {
  DCHECK(evb->isInEventBaseThread());
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
  DCHECK(!evb_ || evb_->isInEventBaseThread());
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

constexpr std::chrono::seconds RocketClientChannel::kDefaultRpcTimeout;
} // namespace thrift
} // namespace apache
