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

#include <thrift/lib/cpp2/transport/rocket/server/RocketThriftRequests.h>

#include <memory>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/Function.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include <thrift/lib/cpp/protocol/TBase64Utils.h>
#include <thrift/lib/cpp2/SerializationSwitch.h>
#include <thrift/lib/cpp2/async/ServerSinkBridge.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>
#include <thrift/lib/cpp2/transport/core/SendCallbacks.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CompressionManager.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>
#include <thrift/lib/cpp2/transport/rocket/server/IRocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketSinkClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketStreamClientCallback.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#if __has_include(<thrift/lib/thrift/gen-cpp2/any_rep_types.h>)
#include <thrift/lib/thrift/gen-cpp2/any_rep_types.h>
#define THRIFT_ANY_AVAILABLE
#endif

namespace apache::thrift::rocket {

namespace {
ResponseRpcError makeResponseRpcError(
    ResponseRpcErrorCode errorCode,
    folly::StringPiece message,
    const ResponseRpcMetadata& metadata) {
  ResponseRpcError responseRpcError;
  responseRpcError.name_utf8() =
      apache::thrift::TEnumTraits<ResponseRpcErrorCode>::findName(errorCode);
  responseRpcError.what_utf8() = message.str();
  responseRpcError.code() = errorCode;
  auto category = [&] {
    switch (errorCode) {
      case ResponseRpcErrorCode::REQUEST_PARSING_FAILURE:
      case ResponseRpcErrorCode::WRONG_RPC_KIND:
      case ResponseRpcErrorCode::UNKNOWN_METHOD:
      case ResponseRpcErrorCode::CHECKSUM_MISMATCH:
      case ResponseRpcErrorCode::UNKNOWN_INTERACTION_ID:
      case ResponseRpcErrorCode::UNIMPLEMENTED_METHOD:
        return ResponseRpcErrorCategory::INVALID_REQUEST;
      case ResponseRpcErrorCode::OVERLOAD:
      case ResponseRpcErrorCode::QUEUE_OVERLOADED:
      case ResponseRpcErrorCode::QUEUE_TIMEOUT:
      case ResponseRpcErrorCode::APP_OVERLOAD:
      case ResponseRpcErrorCode::INTERACTION_LOADSHEDDED:
      case ResponseRpcErrorCode::INTERACTION_LOADSHEDDED_OVERLOAD:
      case ResponseRpcErrorCode::INTERACTION_LOADSHEDDED_APP_OVERLOAD:
      case ResponseRpcErrorCode::INTERACTION_LOADSHEDDED_QUEUE_TIMEOUT:
        return ResponseRpcErrorCategory::LOADSHEDDING;
      case ResponseRpcErrorCode::SHUTDOWN:
        return ResponseRpcErrorCategory::SHUTDOWN;
      default:
        return ResponseRpcErrorCategory::INTERNAL_ERROR;
    }
  }();
  responseRpcError.category() = category;

  if (auto loadRef = metadata.load()) {
    responseRpcError.load() = *loadRef;
  }

  return responseRpcError;
}

RocketException makeRocketException(
    const ResponseRpcError& responseRpcError,
    PayloadSerializer& payloadSerializer) {
  auto rocketCategory = [&] {
    auto category = ResponseRpcErrorCategory::INTERNAL_ERROR;
    if (auto errorCategory = responseRpcError.category()) {
      category = *errorCategory;
    }
    switch (category) {
      case ResponseRpcErrorCategory::INVALID_REQUEST:
        return rocket::ErrorCode::INVALID;
      case ResponseRpcErrorCategory::LOADSHEDDING:
      case ResponseRpcErrorCategory::SHUTDOWN:
        return rocket::ErrorCode::REJECTED;
      default:
        return rocket::ErrorCode::CANCELED;
    }
  }();

  return RocketException(
      rocketCategory, payloadSerializer.packCompact(responseRpcError));
}

template <typename Serializer>
FOLLY_NODISCARD std::optional<ResponseRpcError> processFirstResponseHelper(
    ResponseRpcMetadata& metadata,
    std::unique_ptr<folly::IOBuf>& payload,
    int32_t version) noexcept {
  DCHECK_GE(version, 8);
  try {
    std::string methodNameIgnore;
    MessageType mtype;
    int32_t seqIdIgnore;
    typename Serializer::ProtocolReader reader;
    reader.setInput(payload.get());
    reader.readMessageBegin(methodNameIgnore, mtype, seqIdIgnore);

    switch (mtype) {
      case MessageType::T_REPLY: {
        auto prefixSize = reader.getCursorPosition();

        protocol::TType ftype;
        int16_t fid;
        reader.readStructBegin(methodNameIgnore);
        reader.readFieldBegin(methodNameIgnore, ftype, fid);

        while (payload->length() < prefixSize) {
          prefixSize -= payload->length();
          payload = payload->pop();
        }
        payload->trimStart(prefixSize);

        PayloadMetadata payloadMetadata;
        if (fid == 0) {
          payloadMetadata.responseMetadata() = PayloadResponseMetadata();
        } else {
          PayloadExceptionMetadataBase exceptionMetadataBase;
          PayloadDeclaredExceptionMetadata declaredExceptionMetadata;
          if (auto otherMetadataRef = metadata.otherMetadata()) {
            // defined in sync with
            // thrift/lib/cpp2/transport/core/RpcMetadataUtil.h

            // Setting user exception name and content
            static const auto uex =
                std::string(apache::thrift::detail::kHeaderUex);
            if (auto uexPtr = folly::get_ptr(*otherMetadataRef, uex)) {
              exceptionMetadataBase.name_utf8() = *uexPtr;
              otherMetadataRef->erase(uex);
            }
            static const auto uexw =
                std::string(apache::thrift::detail::kHeaderUexw);
            if (auto uexwPtr = folly::get_ptr(*otherMetadataRef, uexw)) {
              exceptionMetadataBase.what_utf8() = *uexwPtr;
              otherMetadataRef->erase(uexw);
            }

            // Setting user declared exception classification
            static const auto exMeta =
                std::string(apache::thrift::detail::kHeaderExMeta);
            if (auto metaPtr = folly::get_ptr(*otherMetadataRef, exMeta)) {
              ErrorClassification errorClassification =
                  apache::thrift::detail::deserializeErrorClassification(
                      *metaPtr);
              declaredExceptionMetadata.errorClassification() =
                  std::move(errorClassification);
            }
          }
          PayloadExceptionMetadata exceptionMetadata;
          exceptionMetadata.declaredException() =
              std::move(declaredExceptionMetadata);
          exceptionMetadataBase.metadata() = std::move(exceptionMetadata);
          payloadMetadata.exceptionMetadata() =
              std::move(exceptionMetadataBase);
        }
        metadata.payloadMetadata() = std::move(payloadMetadata);
        break;
      }
      case MessageType::T_EXCEPTION: {
        TApplicationException ex;
        ::apache::thrift::detail::deserializeExceptionBody(&reader, &ex);

        PayloadExceptionMetadataBase exceptionMetadataBase;
        exceptionMetadataBase.what_utf8() = ex.getMessage();

        auto otherMetadataRef = metadata.otherMetadata();
        DCHECK(
            !otherMetadataRef ||
            !folly::get_ptr(
                *otherMetadataRef,
                apache::thrift::detail::kHeaderProxiedAnyex) ||
            !folly::get_ptr(
                *otherMetadataRef,
                apache::thrift::detail::kHeaderProxiedAnyexType));

        auto anyexPtr = otherMetadataRef
            ? folly::get_ptr(
                  *otherMetadataRef, apache::thrift::detail::kHeaderAnyex)
            : nullptr;
        auto anyexTypePtr = otherMetadataRef
            ? folly::get_ptr(
                  *otherMetadataRef, apache::thrift::detail::kHeaderAnyexType)
            : nullptr;

        if (anyexPtr && anyexTypePtr) {
          if (version < 10) {
            exceptionMetadataBase.name_utf8() = *anyexTypePtr;
            PayloadExceptionMetadata exceptionMetadata;
            exceptionMetadata.DEPRECATED_proxyException() =
                PayloadProxyExceptionMetadata();
            exceptionMetadataBase.metadata() = std::move(exceptionMetadata);

            payload = protocol::base64Decode(*anyexPtr);
          } else {
#ifdef THRIFT_ANY_AVAILABLE
            exceptionMetadataBase.name_utf8() = *anyexTypePtr;
            exceptionMetadataBase.metadata().ensure().anyException().ensure();

            type::SemiAnyStruct anyException;
            anyException.type() =
                type::Type(type::exception_c{}, *anyexTypePtr);
            if (!std::is_same_v<Serializer, CompactSerializer>) {
              anyException.protocol() = type::StandardProtocol::Compact;
            }
            anyException.data() = std::move(*protocol::base64Decode(*anyexPtr));
            folly::IOBufQueue payloadQueue;
            Serializer::serialize(anyException, &payloadQueue);
            payload = payloadQueue.move();
#endif
          }

          otherMetadataRef->erase(apache::thrift::detail::kHeaderAnyexType);
          otherMetadataRef->erase(apache::thrift::detail::kHeaderAnyex);
          otherMetadataRef->erase(apache::thrift::detail::kHeaderEx);
        } else {
          auto exPtr = otherMetadataRef
              ? folly::get_ptr(
                    *otherMetadataRef, apache::thrift::detail::kHeaderEx)
              : nullptr;
          auto uexPtr = otherMetadataRef
              ? folly::get_ptr(
                    *otherMetadataRef, apache::thrift::detail::kHeaderUex)
              : nullptr;
          if (auto errorCode = [&]() -> folly::Optional<ResponseRpcErrorCode> {
                if (exPtr) {
                  if (*exPtr == kQueueOverloadedErrorCode &&
                      ex.getType() == TApplicationException::LOADSHEDDING) {
                    return ResponseRpcErrorCode::SHUTDOWN;
                  }

                  static const auto& errorCodeMap = *new std::unordered_map<
                      std::string,
                      ResponseRpcErrorCode>(
                      {{kUnknownErrorCode, ResponseRpcErrorCode::UNKNOWN},
                       {kOverloadedErrorCode, ResponseRpcErrorCode::OVERLOAD},
                       {kAppOverloadedErrorCode,
                        ResponseRpcErrorCode::APP_OVERLOAD},
                       {kTaskExpiredErrorCode,
                        ResponseRpcErrorCode::TASK_EXPIRED},
                       {kQueueOverloadedErrorCode,
                        ResponseRpcErrorCode::QUEUE_OVERLOADED},
                       {kInjectedFailureErrorCode,
                        ResponseRpcErrorCode::INJECTED_FAILURE},
                       {kServerQueueTimeoutErrorCode,
                        ResponseRpcErrorCode::QUEUE_TIMEOUT},
                       {kResponseTooBigErrorCode,
                        ResponseRpcErrorCode::RESPONSE_TOO_BIG},
                       {kMethodUnknownErrorCode,
                        ResponseRpcErrorCode::UNKNOWN_METHOD},
                       {kRequestTypeDoesntMatchServiceFunctionType,
                        ResponseRpcErrorCode::WRONG_RPC_KIND},
                       {kInteractionIdUnknownErrorCode,
                        ResponseRpcErrorCode::UNKNOWN_INTERACTION_ID},
                       {kInteractionConstructorErrorErrorCode,
                        ResponseRpcErrorCode::INTERACTION_CONSTRUCTOR_ERROR},
                       {kRequestParsingErrorCode,
                        ResponseRpcErrorCode::REQUEST_PARSING_FAILURE},
                       {kChecksumMismatchErrorCode,
                        ResponseRpcErrorCode::CHECKSUM_MISMATCH},
                       {kUnimplementedMethodErrorCode,
                        ResponseRpcErrorCode::UNIMPLEMENTED_METHOD},
                       {kTenantQuotaExceededErrorCode,
                        ResponseRpcErrorCode::TENANT_QUOTA_EXCEEDED},
                       {kInteractionLoadsheddedErrorCode,
                        ResponseRpcErrorCode::INTERACTION_LOADSHEDDED},
                       {kInteractionLoadsheddedOverloadErrorCode,
                        ResponseRpcErrorCode::INTERACTION_LOADSHEDDED_OVERLOAD},
                       {kInteractionLoadsheddedAppOverloadErrorCode,
                        ResponseRpcErrorCode::
                            INTERACTION_LOADSHEDDED_APP_OVERLOAD},
                       {kInteractionLoadsheddedQueueTimeoutErrorCode,
                        ResponseRpcErrorCode::
                            INTERACTION_LOADSHEDDED_QUEUE_TIMEOUT}});
                  if (auto foundErrorCode =
                          folly::get_ptr(errorCodeMap, *exPtr)) {
                    return *foundErrorCode;
                  }
                }

                return folly::none;
              }()) {
            return makeResponseRpcError(*errorCode, ex.getMessage(), metadata);
          }
          if (uexPtr) {
            exceptionMetadataBase.name_utf8() = *uexPtr;
            otherMetadataRef->erase(apache::thrift::detail::kHeaderUex);
          }

          const auto isClientError = exPtr && *exPtr == kAppClientErrorCode;
          exceptionMetadataBase.metadata()
              .ensure()
              .appUnknownException()
              .ensure()
              .errorClassification()
              .ensure()
              .blame() =
              isClientError ? ErrorBlame::CLIENT : ErrorBlame::SERVER;

          payload->clear();

          if (otherMetadataRef) {
            otherMetadataRef->erase(apache::thrift::detail::kHeaderEx);
            otherMetadataRef->erase(apache::thrift::detail::kHeaderUexw);
          }
        }

        PayloadMetadata payloadMetadata;
        payloadMetadata.exceptionMetadata() = std::move(exceptionMetadataBase);
        metadata.payloadMetadata() = std::move(payloadMetadata);

        break;
      }
      default:
        return makeResponseRpcError(
            ResponseRpcErrorCode::UNKNOWN, "Invalid message type", metadata);
    }
  } catch (...) {
    return makeResponseRpcError(
        ResponseRpcErrorCode::UNKNOWN,
        fmt::format(
            "Invalid response payload envelope: {}",
            folly::exceptionStr(folly::current_exception()).toStdString()),
        metadata);
  }
  return {};
}

FOLLY_NODISCARD std::optional<ResponseRpcError> processFirstResponse(
    ResponseRpcMetadata& metadata,
    std::unique_ptr<folly::IOBuf>& payload,
    apache::thrift::protocol::PROTOCOL_TYPES protType,
    int32_t version,
    const folly::Optional<CompressionConfig>& compressionConfig) noexcept {
  if (!payload) {
    return makeResponseRpcError(
        ResponseRpcErrorCode::UNKNOWN,
        "serialization failed for response",
        metadata);
  }

  THRIFT_APPLICATION_EVENT(server_write_headers).log([&] {
    auto size = metadata.otherMetadata() ? metadata.otherMetadata()->size() : 0;
    std::vector<folly::dynamic> keys;
    if (size) {
      keys.reserve(size);
      for (auto& [k, v] : *metadata.otherMetadata()) {
        keys.emplace_back(k);
      }
    }
    return folly::dynamic::object("size", size) //
        ("keys", folly::dynamic::array(std::move(keys)));
  });

  // apply compression if client has specified compression codec
  if (compressionConfig.has_value()) {
    CompressionManager().setCompressionCodec(
        *compressionConfig, metadata, payload->computeChainDataLength());
  }

  switch (protType) {
    case protocol::T_BINARY_PROTOCOL:
      return processFirstResponseHelper<BinarySerializer>(
          metadata, payload, version);
    case protocol::T_COMPACT_PROTOCOL:
      return processFirstResponseHelper<CompactSerializer>(
          metadata, payload, version);
    default: {
      return makeResponseRpcError(
          ResponseRpcErrorCode::UNKNOWN,
          "Invalid response payload protocol id",
          metadata);
    }
  }
}

template <typename Callback>
void handleStreamError(RocketException ex, Callback& callback) {
  std::exchange(callback, nullptr)->onFirstResponseError(std::move(ex));
}

} // namespace

namespace detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(
    void,
    onRocketThriftRequestReceived,
    const IRocketServerConnection&,
    StreamId,
    RpcKind,
    const transport::THeader::StringToStringMap&) {}
} // namespace detail

RocketThriftRequest::RocketThriftRequest(
    server::ServerConfigs& serverConfigs,
    RequestRpcMetadata&& metadata,
    Cpp2ConnContext& connContext,
    RequestsRegistry::DecoratorAndInterceptorStorage
        decoratorAndInterceptorsStorage,
    folly::EventBase& evb,
    RocketServerFrameContext&& context)
    : ThriftRequestCore(
          serverConfigs,
          std::move(metadata),
          connContext,
          std::move(decoratorAndInterceptorsStorage)),
      evb_(evb),
      context_(std::move(context)) {
  detail::onRocketThriftRequestReceived(
      context_.connection(),
      context_.streamId(),
      kind(),
      getTHeader().getHeaders());
}

ThriftServerRequestResponse::ThriftServerRequestResponse(
    ColocatedConstructionParams colocationParams,
    folly::EventBase& evb,
    server::ServerConfigs& serverConfigs,
    RequestRpcMetadata&& metadata,
    Cpp2ConnContext& connContext,
    std::shared_ptr<folly::RequestContext> rctx,
    RequestsRegistry& reqRegistry,
    rocket::Payload&& debugPayload,
    RocketServerFrameContext&& context,
    int32_t version,
    std::chrono::milliseconds maxResponseWriteTime)
    : RocketThriftRequest(
          serverConfigs,
          std::move(metadata),
          connContext,
          std::move(colocationParams.data),
          evb,
          std::move(context)),
      version_(version),
      maxResponseWriteTime_(maxResponseWriteTime) {
  new (colocationParams.debugStubToInit) RequestsRegistry::DebugStub(
      reqRegistry,
      *this,
      *getRequestContext(),
      std::move(rctx),
      getProtoId(),
      std::move(debugPayload),
      stateMachine_);
  scheduleTimeouts();
}

void ThriftServerRequestResponse::sendThriftResponse(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> data,
    apache::thrift::MessageChannel::SendCallbackPtr cb) noexcept {
  auto responseRpcError = processFirstResponse(
      metadata, data, getProtoId(), version_, getCompressionConfig());
  // When creating request logging callback, we need to access payload metadata
  // which is populated in the processFirstResponse, so
  // createRequestLoggingCallback must happen after processFirstResponse.
  cb = createRequestLoggingCallback(std::move(cb), metadata, responseRpcError);

  auto payloadSerializerPtr = context_.connection().getPayloadSerializer();
  if (responseRpcError) {
    auto ex = makeRocketException(*responseRpcError, *payloadSerializerPtr);
    context_.sendError(std::move(ex), std::move(cb));
    return;
  }

  // Prevents potential complexity (e.g. using custom compression to
  // encode custom compression failures, or to that matter, any failure at all),
  // we use ZSTD to compress exception payloads.
  // custom compression implies ZSTD, so it is safe to fallback to it.
  if (metadata.compression().value_or(CompressionAlgorithm::NONE) ==
      CompressionAlgorithm::CUSTOM) {
    auto payloadIsException = false;
    if (!metadata.payloadMetadata()) {
      // unexpected, handle it conservatively
      payloadIsException = true;
    } else if (metadata.payloadMetadata()->exceptionMetadata()) {
      payloadIsException = true;
    }

    if (payloadIsException) {
      metadata.compression() = CompressionAlgorithm::ZSTD;
    }
  }

  rocket::Payload payload;
  try {
    payload = payloadSerializerPtr->packWithFds(
        &metadata,
        std::move(data),
        std::move(getRequestContext()->getHeader()->fds),
        context_.connection().isDecodingMetadataUsingBinaryProtocol(),
        context_.connection().getRawSocket());
  } catch (std::exception const& ex) {
    auto error = makeResponseRpcError(
        ResponseRpcErrorCode::UNKNOWN,
        fmt::format("Failed to pack or send payload due to: {}", ex.what()),
        metadata);
    auto rex = makeRocketException(error, *payloadSerializerPtr);
    context_.sendError(std::move(rex), std::move(cb));
    return;
  }

  if (maxResponseWriteTime_ > std::chrono::milliseconds{0}) {
    cb = apache::thrift::MessageChannel::SendCallbackPtr(
        new ResponseWriteTimeoutSendCallback(
            context_.connection(), maxResponseWriteTime_, cb.release()));
  }
  context_.sendPayload(
      std::move(payload), Flags().next(true).complete(true), std::move(cb));
}

void ThriftServerRequestResponse::sendThriftException(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> data,
    apache::thrift::MessageChannel::SendCallbackPtr cb) noexcept {
  sendThriftResponse(std::move(metadata), std::move(data), std::move(cb));
}

void ThriftServerRequestResponse::sendSerializedError(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> exbuf) noexcept {
  sendThriftResponse(std::move(metadata), std::move(exbuf), nullptr);
}

void ThriftServerRequestResponse::closeConnection(
    folly::exception_wrapper ew) noexcept {
  context_.connection().close(std::move(ew));
}

ThriftServerRequestFnf::ThriftServerRequestFnf(
    ColocatedConstructionParams colocationParams,
    folly::EventBase& evb,
    server::ServerConfigs& serverConfigs,
    RequestRpcMetadata&& metadata,
    Cpp2ConnContext& connContext,
    std::shared_ptr<folly::RequestContext> rctx,
    RequestsRegistry& reqRegistry,
    rocket::Payload&& debugPayload,
    RocketServerFrameContext&& context,
    folly::Function<void()> onComplete)
    : RocketThriftRequest(
          serverConfigs,
          std::move(metadata),
          connContext,
          std::move(colocationParams.data),
          evb,
          std::move(context)),
      onComplete_(std::move(onComplete)) {
  new (colocationParams.debugStubToInit) RequestsRegistry::DebugStub(
      reqRegistry,
      *this,
      *getRequestContext(),
      std::move(rctx),
      getProtoId(),
      std::move(debugPayload),
      stateMachine_);
  scheduleTimeouts();
}

ThriftServerRequestFnf::~ThriftServerRequestFnf() {
  if (auto f = std::move(onComplete_)) {
    f();
  }
}

void ThriftServerRequestFnf::sendThriftResponse(
    ResponseRpcMetadata&&,
    std::unique_ptr<folly::IOBuf>,
    apache::thrift::MessageChannel::SendCallbackPtr) noexcept {
  LOG(FATAL) << "One-way requests cannot send responses";
}

void ThriftServerRequestFnf::sendThriftException(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> data,
    apache::thrift::MessageChannel::SendCallbackPtr cb) noexcept {
  sendThriftResponse(std::move(metadata), std::move(data), std::move(cb));
}

void ThriftServerRequestFnf::sendSerializedError(
    ResponseRpcMetadata&&, std::unique_ptr<folly::IOBuf>) noexcept {}

void ThriftServerRequestFnf::closeConnection(
    folly::exception_wrapper ew) noexcept {
  context_.connection().close(std::move(ew));
}

ThriftServerRequestStream::ThriftServerRequestStream(
    ColocatedConstructionParams colocationParams,
    folly::EventBase& evb,
    server::ServerConfigs& serverConfigs,
    RequestRpcMetadata&& metadata,
    Cpp2ConnContext& connContext,
    std::shared_ptr<folly::RequestContext> rctx,
    RequestsRegistry& reqRegistry,
    rocket::Payload&& debugPayload,
    RocketServerFrameContext&& context,
    int32_t version,
    RocketStreamClientCallback* clientCallback,
    std::shared_ptr<AsyncProcessor> cpp2Processor)
    : RocketThriftRequest(
          serverConfigs,
          std::move(metadata),
          connContext,
          std::move(colocationParams.data),
          evb,
          std::move(context)),
      version_(version),
      clientCallback_(clientCallback),
      cpp2Processor_(std::move(cpp2Processor)) {
  new (colocationParams.debugStubToInit) RequestsRegistry::DebugStub(
      reqRegistry,
      *this,
      *getRequestContext(),
      std::move(rctx),
      getProtoId(),
      std::move(debugPayload),
      stateMachine_);
  if (auto compressionConfig = getCompressionConfig()) {
    clientCallback_->setCompressionConfig(*compressionConfig);
  }
  clientCallback_->setRpcMethodName(getMethodName());
  scheduleTimeouts();
}

void ThriftServerRequestStream::sendThriftResponse(
    ResponseRpcMetadata&&,
    std::unique_ptr<folly::IOBuf> buf,
    apache::thrift::MessageChannel::SendCallbackPtr) noexcept {
  LOG(ERROR) << "Serialized Payload: " << buf->moveToFbString().toStdString();
  LOG(FATAL) << "Stream requests must respond via sendStreamThriftResponse";
}

void ThriftServerRequestStream::sendThriftException(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> data,
    apache::thrift::MessageChannel::SendCallbackPtr) noexcept {
  sendSerializedError(std::move(metadata), std::move(data));
}

bool ThriftServerRequestStream::sendStreamThriftResponse(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> data,
    StreamServerCallbackPtr stream) noexcept {
  if (!stream) {
    sendSerializedError(std::move(metadata), std::move(data));
    return false;
  }
  if (auto responseRpcError = processFirstResponse(
          metadata, data, getProtoId(), version_, getCompressionConfig())) {
    auto ex = makeRocketException(
        *responseRpcError, *context_.connection().getPayloadSerializer());
    handleStreamError(std::move(ex), clientCallback_);
    return false;
  }
  context_.unsetMarkRequestComplete();
  stream->resetClientCallback(*clientCallback_);
  clientCallback_->setProtoId(getProtoId());
  auto payload = FirstResponsePayload{std::move(data), std::move(metadata)};
  payload.fds =
      std::move(getRequestContext()->getHeader()->fds.dcheckToSendOrEmpty());
  return clientCallback_->onFirstResponse(
      std::move(payload), nullptr /* evb */, stream.release());
}

void ThriftServerRequestStream::sendStreamThriftResponse(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> data,
    apache::thrift::detail::ServerStreamFactory&& stream) noexcept {
  if (!stream) {
    sendSerializedError(std::move(metadata), std::move(data));
    return;
  }
  if (auto responseRpcError = processFirstResponse(
          metadata, data, getProtoId(), version_, getCompressionConfig())) {
    auto ex = makeRocketException(
        *responseRpcError, *context_.connection().getPayloadSerializer());
    handleStreamError(std::move(ex), clientCallback_);
    return;
  }
  context_.unsetMarkRequestComplete();
  clientCallback_->setProtoId(getProtoId());
  clientCallback_->setContextStack(stream.getContextStack());
  auto payload = apache::thrift::FirstResponsePayload{
      std::move(data), std::move(metadata)};
  payload.fds =
      std::move(getRequestContext()->getHeader()->fds.dcheckToSendOrEmpty());
  stream(std::move(payload), clientCallback_, &evb_);
}

void ThriftServerRequestStream::sendSerializedError(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> exbuf) noexcept {
  if (auto responseRpcError = processFirstResponse(
          metadata, exbuf, getProtoId(), version_, getCompressionConfig())) {
    auto ex = makeRocketException(
        *responseRpcError, *context_.connection().getPayloadSerializer());
    handleStreamError(std::move(ex), clientCallback_);
    return;
  }
  std::exchange(clientCallback_, nullptr)
      ->onFirstResponseError(folly::make_exception_wrapper<
                             thrift::detail::EncodedFirstResponseError>(
          FirstResponsePayload(std::move(exbuf), std::move(metadata))));
}

void ThriftServerRequestStream::closeConnection(
    folly::exception_wrapper ew) noexcept {
  context_.connection().close(std::move(ew));
}

ThriftServerRequestSink::ThriftServerRequestSink(
    ColocatedConstructionParams colocationParams,
    folly::EventBase& evb,
    server::ServerConfigs& serverConfigs,
    RequestRpcMetadata&& metadata,
    Cpp2ConnContext& connContext,
    std::shared_ptr<folly::RequestContext> rctx,
    RequestsRegistry& reqRegistry,
    rocket::Payload&& debugPayload,
    RocketServerFrameContext&& context,
    int32_t version,
    RocketSinkClientCallback* clientCallback,
    std::shared_ptr<AsyncProcessor> cpp2Processor)
    : RocketThriftRequest(
          serverConfigs,
          std::move(metadata),
          connContext,
          std::move(colocationParams.data),
          evb,
          std::move(context)),
      version_(version),
      clientCallback_(clientCallback),
      cpp2Processor_(std::move(cpp2Processor)) {
  new (colocationParams.debugStubToInit) RequestsRegistry::DebugStub(
      reqRegistry,
      *this,
      *getRequestContext(),
      std::move(rctx),
      getProtoId(),
      std::move(debugPayload),
      stateMachine_);
  if (auto compressionConfig = getCompressionConfig()) {
    clientCallback_->setCompressionConfig(*compressionConfig);
  }
  scheduleTimeouts();
}

void ThriftServerRequestSink::sendThriftResponse(
    ResponseRpcMetadata&&,
    std::unique_ptr<folly::IOBuf>,
    apache::thrift::MessageChannel::SendCallbackPtr) noexcept {
  LOG(FATAL) << "Sink requests must respond via sendSinkThriftResponse";
}

void ThriftServerRequestSink::sendThriftException(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> data,
    apache::thrift::MessageChannel::SendCallbackPtr) noexcept {
  sendSerializedError(std::move(metadata), std::move(data));
}

void ThriftServerRequestSink::sendSerializedError(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> exbuf) noexcept {
  if (auto responseRpcError = processFirstResponse(
          metadata, exbuf, getProtoId(), version_, getCompressionConfig())) {
    auto ex = makeRocketException(
        *responseRpcError, *context_.connection().getPayloadSerializer());
    handleStreamError(std::move(ex), clientCallback_);
    return;
  }
  std::exchange(clientCallback_, nullptr)
      ->onFirstResponseError(folly::make_exception_wrapper<
                             thrift::detail::EncodedFirstResponseError>(
          FirstResponsePayload(std::move(exbuf), std::move(metadata))));
}

#if FOLLY_HAS_COROUTINES
void ThriftServerRequestSink::sendSinkThriftResponse(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> data,
    apache::thrift::detail::SinkConsumerImpl&& sinkConsumer) noexcept {
  if (!sinkConsumer) {
    sendSerializedError(std::move(metadata), std::move(data));
    return;
  }
  if (auto responseRpcError = processFirstResponse(
          metadata, data, getProtoId(), version_, getCompressionConfig())) {
    auto ex = makeRocketException(
        *responseRpcError, *context_.connection().getPayloadSerializer());
    handleStreamError(std::move(ex), clientCallback_);
    return;
  }
  context_.unsetMarkRequestComplete();
  auto* executor = sinkConsumer.executor.get();
  clientCallback_->setProtoId(getProtoId());
  clientCallback_->setChunkTimeout(sinkConsumer.chunkTimeout);
  auto serverCallback = apache::thrift::detail::ServerSinkBridge::create(
      std::move(sinkConsumer), *getEventBase(), clientCallback_);
  DCHECK(getRequestContext()->getHeader()->fds.empty()); // No FDs for sinks
  clientCallback_->onFirstResponse(
      FirstResponsePayload{std::move(data), std::move(metadata)},
      nullptr /* evb */,
      serverCallback.get());

  co_withExecutor(
      executor,
      folly::coro::co_invoke(
          &apache::thrift::detail::ServerSinkBridge::start,
          std::move(serverCallback)))
      .start();
}

bool ThriftServerRequestSink::sendSinkThriftResponse(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> data,
    SinkServerCallbackPtr serverCallback) noexcept {
  if (!serverCallback) {
    sendSerializedError(std::move(metadata), std::move(data));
    return false;
  }
  if (auto responseRpcError = processFirstResponse(
          metadata, data, getProtoId(), version_, getCompressionConfig())) {
    auto ex = makeRocketException(
        *responseRpcError, *context_.connection().getPayloadSerializer());
    handleStreamError(std::move(ex), clientCallback_);
    return false;
  }
  context_.unsetMarkRequestComplete();
  serverCallback->resetClientCallback(*clientCallback_);
  clientCallback_->setProtoId(getProtoId());
  DCHECK(getRequestContext()->getHeader()->fds.empty()); // No FDs for sinks
  return clientCallback_->onFirstResponse(
      FirstResponsePayload{std::move(data), std::move(metadata)},
      nullptr, /* evb */
      serverCallback.release());
}
#endif

void ThriftServerRequestBiDi::sendSerializedError(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> exbuf) noexcept {
  if (auto responseRpcError = processFirstResponse(
          metadata, exbuf, getProtoId(), version_, getCompressionConfig())) {
    auto ex = makeRocketException(
        *responseRpcError, *context_.connection().getPayloadSerializer());
    handleStreamError(std::move(ex), clientCallback_);
    return;
  }
  std::exchange(clientCallback_, nullptr)
      ->onFirstResponseError(folly::make_exception_wrapper<
                             thrift::detail::EncodedFirstResponseError>(
          FirstResponsePayload(std::move(exbuf), std::move(metadata))));
}

void ThriftServerRequestBiDi::sendBiDiThriftResponse(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> data,
    apache::thrift::detail::ServerBiDiStreamFactory&&
        bidiStreamFactory) noexcept {
  if (!bidiStreamFactory.valid()) {
    sendSerializedError(std::move(metadata), std::move(data));
    return;
  }

  if (auto responseRpcError = processFirstResponse(
          metadata, data, getProtoId(), version_, getCompressionConfig())) {
    auto ex = makeRocketException(
        *responseRpcError, *context_.connection().getPayloadSerializer());
    handleStreamError(std::move(ex), clientCallback_);
    return;
  }

  context_.unsetMarkRequestComplete();
  // Missing some configuration here
  // clientCallback_->setProtoId(getProtoId());
  // clientCallback_->setChunkTimeout(sinkConsumer.chunkTimeout);
  auto payload = apache::thrift::FirstResponsePayload{
      std::move(data), std::move(metadata)};
  payload.fds =
      std::move(getRequestContext()->getHeader()->fds.dcheckToSendOrEmpty());
  std::move(bidiStreamFactory)
      .start(std::move(payload), clientCallback_, getEventBase());
}

bool ThriftServerRequestBiDi::sendBiDiThriftResponse(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> data,
    BiDiServerCallbackPtr serverCallback) noexcept {
  if (!serverCallback) {
    sendSerializedError(std::move(metadata), std::move(data));
    return false;
  }

  if (auto responseRpcError = processFirstResponse(
          metadata, data, getProtoId(), version_, getCompressionConfig())) {
    auto ex = makeRocketException(
        *responseRpcError, *context_.connection().getPayloadSerializer());
    handleStreamError(std::move(ex), clientCallback_);
    return false;
  }

  context_.unsetMarkRequestComplete();
  serverCallback->resetClientCallback(*clientCallback_);

  DCHECK(getRequestContext()->getHeader()->fds.empty()); // No FDs for bidi

  return clientCallback_->onFirstResponse(
      FirstResponsePayload{std::move(data), std::move(metadata)},
      nullptr, /* evb */
      serverCallback.release());
}

void ThriftServerRequestSink::closeConnection(
    folly::exception_wrapper ew) noexcept {
  context_.connection().close(std::move(ew));
}

} // namespace apache::thrift::rocket
