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

#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>

#include <chrono>
#include <limits>
#include <string>
#include <utility>

#include <fmt/core.h>
#include <folly/Conv.h>
#include <folly/ExceptionWrapper.h>
#include <folly/GLog.h>
#include <folly/Likely.h>
#include <folly/ScopeGuard.h>
#include <folly/Try.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/lang/Exception.h>
#include <folly/portability/GFlags.h>

#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/transport/core/TryUtil.h>
#include <thrift/lib/cpp2/transport/rocket/FdSocket.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/client/RequestContext.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CustomCompressorRegistry.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Util.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_constants.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

THRIFT_FLAG_DEFINE_bool(rocket_client_new_protocol_key, true);
THRIFT_FLAG_DEFINE_bool(rocket_client_binary_rpc_metadata_encoding, false);
THRIFT_FLAG_DEFINE_bool(rocket_client_rocket_skip_protocol_key, false);
THRIFT_FLAG_DEFINE_bool(rocket_client_set_eor_flag, false);

namespace apache::thrift {

namespace detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(
    std::optional<TransportMetadataPush>,
    getTransportMetadataPush,
    folly::AsyncTransport*) {
  return {};
}
} // namespace detail

namespace rocket {
namespace {
folly::exception_wrapper err(folly::Try<void> t) {
  return t.hasException() ? std::move(t.exception())
                          : folly::exception_wrapper();
}

transport::TTransportException toTransportException(
    folly::exception_wrapper ew) {
  transport::TTransportException result;
  if (ew.with_exception<transport::TTransportException>(
          [&](transport::TTransportException ex) { result = std::move(ex); })) {
    return result;
  }

  return transport::TTransportException(folly::exceptionStr(ew).toStdString());
}

folly::exception_wrapper makeContractViolation(std::string msg) {
  return folly::make_exception_wrapper<transport::TTransportException>(
      transport::TTransportException::TTransportExceptionType::
          STREAMING_CONTRACT_VIOLATION,
      std::move(msg));
}

std::unique_ptr<rocket::SetupFrame> makeSetupFrame(
    RequestSetupMetadata const& meta) {
  uint32_t serialized_size;
  folly::IOBufQueue paramQueue;

  bool encodeMetadataUsingBinary =
      THRIFT_FLAG(rocket_client_binary_rpc_metadata_encoding);

  if (meta.encodeMetadataUsingBinary().has_value()) {
    encodeMetadataUsingBinary = meta.encodeMetadataUsingBinary().value();
  }

  // TODO: migrate this to
  if (encodeMetadataUsingBinary) {
    BinaryProtocolWriter binaryProtocolWriter;
    binaryProtocolWriter.setOutput(&paramQueue);
    meta.write(&binaryProtocolWriter);
    serialized_size = meta.serializedSize(&binaryProtocolWriter);
  } else {
    CompactProtocolWriter compactProtocolWriter;
    compactProtocolWriter.setOutput(&paramQueue);
    meta.write(&compactProtocolWriter);
    serialized_size = meta.serializedSize(&compactProtocolWriter);
  }

  // Serialize RocketClient's major/minor version (which is separate from the
  // rsocket protocol major/minor version) into setup metadata.
  auto buf = folly::IOBuf::createCombined(sizeof(int32_t) + serialized_size);
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

  return std::make_unique<SetupFrame>(
      rocket::Payload::makeFromMetadataAndData(queue.move(), {}),
      encodeMetadataUsingBinary);
}

} // namespace

RocketClient::RocketClient(
    folly::EventBase& evb,
    folly::AsyncTransport::UniquePtr socket,
    RequestSetupMetadata&& setupMetadata,
    int32_t keepAliveTimeoutMs,
    std::shared_ptr<rocket::ParserAllocatorType> allocatorPtr)
    : evb_(&evb),
      writeLoopCallback_(*this),
      socket_(std::move(socket)),
      parser_(*this, THRIFT_FLAG(rocket_frame_parser), allocatorPtr),
      detachableLoopCallback_(*this),
      closeLoopCallback_(*this),
      eventBaseDestructionCallback_(*this),
      setupFrame_(makeSetupFrame(setupMetadata)),
      encodeMetadataUsingBinary_(setupFrame_->encodeMetadataUsingBinary()) {
  DCHECK(socket_ != nullptr);
  socket_->setReadCB(&parser_);
  if (auto socket_2 = dynamic_cast<folly::AsyncSocket*>(socket_.get())) {
    socket_2->setCloseOnFailedWrite(false);
  }

  if (keepAliveTimeoutMs > 0) {
    keepAliveWatcher_ = std::unique_ptr<
        KeepAliveWatcher,
        folly::DelayedDestruction::Destructor>(new KeepAliveWatcher(
        evb_, socket_.get(), std::chrono::milliseconds(keepAliveTimeoutMs)));
    auto setupFrameMoveOut = moveOutSetupFrame();
    keepAliveWatcher_->start(setupFrameMoveOut.get());
  }

  evb_->runOnDestruction(eventBaseDestructionCallback_);
  // Get or create flush manager from EventBaseLocal
  flushManager_ = &FlushManager::getInstance(*evb_);

  // keep a copy to be used in later
  if (setupMetadata.compressionSetupRequest()) {
    if (auto ref = setupMetadata.compressionSetupRequest()->custom()) {
      customCompressionSetupRequest_.emplace(*ref);
    }
  }
}

RocketClient::~RocketClient() {
  if (keepAliveWatcher_) {
    keepAliveWatcher_->stop();
  }
  closeNow(transport::TTransportException("Destroying RocketClient"));
  eventBaseDestructionCallback_.cancel();
  detachableLoopCallback_.cancelLoopCallback();

  // All outstanding request contexts should have been cleaned up in closeNow()
  DCHECK(streams_.empty());
}

RocketClient::Ptr RocketClient::create(
    folly::EventBase& evb,
    folly::AsyncTransport::UniquePtr socket,
    RequestSetupMetadata&& setupMetadata,
    int32_t keepAliveTimeoutMs,
    std::shared_ptr<rocket::ParserAllocatorType> allocatorPtr) {
  return Ptr(new RocketClient(
      evb,
      std::move(socket),
      std::move(setupMetadata),
      keepAliveTimeoutMs,
      std::move(allocatorPtr)));
}

void RocketClient::handleFrame(std::unique_ptr<folly::IOBuf> frame) {
  DestructorGuard dg(this);

  folly::io::Cursor cursor(frame.get());

  const auto streamId = readStreamId(cursor);
  FrameType frameType;
  std::tie(frameType, std::ignore) = readFrameTypeAndFlags(cursor);

  if (UNLIKELY(frameType == FrameType::ERROR && streamId == StreamId{0})) {
    ErrorFrame errorFrame(std::move(frame));
    handleError(RocketException(
        errorFrame.errorCode(), std::move(errorFrame.payload()).data()));
    return;
  }
  if (frameType == FrameType::KEEPALIVE && streamId == StreamId{0} &&
      keepAliveWatcher_) {
    keepAliveWatcher_->handleKeepaliveFrame(std::move(frame));
    return;
  }

  if (frameType == FrameType::METADATA_PUSH && streamId == StreamId{0}) {
    MetadataPushFrame mdPushFrame(std::move(frame));
    if (!mdPushFrame.metadata()) {
      return;
    }
    ServerPushMetadata serverMeta;
    try {
      getPayloadSerializer()->unpack(
          serverMeta, std::move(mdPushFrame.metadata()), false /* useBinary */);
    } catch (...) {
      close(
          transport::TTransportException(
              transport::TTransportException::CORRUPTED_DATA,
              "Failed to deserialize metadata push frame"));
      return;
    }
    switch (serverMeta.getType()) {
      case ServerPushMetadata::Type::setupResponse: {
        handleSetupResponse(serverMeta);
        break;
      }
      case ServerPushMetadata::Type::streamHeadersPush: {
        StreamId sid(serverMeta.streamHeadersPush()->streamId().value_or(0));
        auto it = streams_.find(sid);
        if (it != streams_.end()) {
          it->match([&](auto* serverCallbackPtr) {
            // Sink currently ignores this frame
            if constexpr (std::is_convertible_v<
                              decltype(serverCallbackPtr),
                              RocketStreamServerCallback*>) {
              serverCallbackPtr->onStreamHeaders(
                  HeadersPayload(serverMeta.streamHeadersPush_ref()
                                     ->headersPayloadContent()
                                     .value_or({})));
            }
          });
        }
        return;
      }
      case ServerPushMetadata::Type::drainCompletePush: {
        auto drainCode = serverMeta.drainCompletePush()->drainCompleteCode();
        if (drainCode &&
            *drainCode == DrainCompleteCode::EXCEEDED_INGRESS_MEM_LIMIT) {
          ResponseRpcError responseRpcError;
          responseRpcError.name_utf8() =
              apache::thrift::TEnumTraits<ResponseRpcErrorCode>::findName(
                  ResponseRpcErrorCode::OVERLOAD);
          responseRpcError.what_utf8() = "Exceeded ingress memory limit";
          responseRpcError.code() = ResponseRpcErrorCode::OVERLOAD;
          responseRpcError.category() = ResponseRpcErrorCategory::LOADSHEDDING;

          close(RocketException(
              ErrorCode::REJECTED,
              getPayloadSerializer()->packCompact(responseRpcError)));
        }
        return;
      }
      default:
        break;
    }
    return;
  }

  if (auto* ctx = queue_.getRequestResponseContext(streamId)) {
    DCHECK(ctx->isRequestResponse());
    return handleRequestResponseFrame(*ctx, frameType, std::move(frame));
  }

  handleStreamChannelFrame(streamId, frameType, std::move(frame));
}

void RocketClient::handleSetupResponse(const ServerPushMetadata& serverMeta) {
  sendTransportMetadataPush();
  setServerVersion(
      std::min(
          serverMeta.setupResponse()->version().value_or(0),
          (int32_t)std::numeric_limits<int16_t>::max()));
  serverZstdSupported_ =
      serverMeta.setupResponse()->zstdSupported().value_or(false);

  if (auto ref = serverMeta.setupResponse()->compressionSetupResponse()) {
    auto customCompressionRes = handleSetupResponseCustomCompression(*ref);
    if (customCompressionRes.hasError()) {
      close(customCompressionRes.error());
      return;
    }
  }
}

folly::Expected<folly::Unit, transport::TTransportException>
RocketClient::handleSetupResponseCustomCompression(
    CompressionSetupResponse const& setupResponse) {
  if (!setupResponse.custom()) {
    return folly::makeUnexpected(
        transport::TTransportException(
            transport::TTransportException::NOT_SUPPORTED,
            "Only 'custom' compressor setup response is supported on client."));
  }
  const auto& customSetupResponse = setupResponse.custom().value();

  auto factory =
      CustomCompressorRegistry::get(*customSetupResponse.compressorName());
  if (!factory) {
    return folly::makeUnexpected(
        transport::TTransportException(
            transport::TTransportException::NOT_SUPPORTED,
            fmt::format(
                "Custom compressor {} is not supported on client.",
                *customSetupResponse.compressorName())));
  }

  if (!customCompressionSetupRequest_) {
    return folly::makeUnexpected(
        transport::TTransportException(
            transport::TTransportException::INTERNAL_ERROR,
            "Trying to setup custom compression on client without a valid request. "
            "Maybe it has already been moved out? Did we somehow invoke "
            "handleSetupResponseCustomCompression more than once?"));
  }

  std::shared_ptr<CustomCompressor> compressor;
  auto customSetupRequest = std::move(*customCompressionSetupRequest_);
  try {
    compressor = factory->make(
        customSetupRequest,
        customSetupResponse,
        CustomCompressorFactory::CompressorLocation::CLIENT);
  } catch (const std::exception& ex) {
    return folly::makeUnexpected(
        transport::TTransportException(
            transport::TTransportException::INVALID_SETUP,
            fmt::format(
                "Failed to make custom compressor on client due to: {}",
                ex.what())));
  }

  if (!compressor) {
    return folly::makeUnexpected(
        transport::TTransportException(
            transport::TTransportException::INVALID_SETUP,
            "Failed to make custom compressor on client."));
  }

  if (!payloadSerializerHolder_) {
    payloadSerializerHolder_.emplace();
  }

  CustomCompressionPayloadSerializerStrategyOptions options;
  options.compressor = compressor;
  CustomCompressionPayloadSerializerStrategy<DefaultPayloadSerializerStrategy>
      strategy{options};
  payloadSerializerHolder_->initialize(std::move(strategy));
  customCompressor_ = compressor;

  return folly::unit;
}

void RocketClient::handleRequestResponseFrame(
    RequestContext& ctx,
    FrameType frameType,
    std::unique_ptr<folly::IOBuf> frame) {
  switch (frameType) {
    case FrameType::PAYLOAD: {
      PayloadFrame payloadFrame(std::move(frame));
      if (!payloadFrame.hasNext() && !payloadFrame.hasComplete()) {
        return close(makeContractViolation(
            "Client received single response payload without next or "
            "complete flag"));
      }
      // We're handling a response to a request.  When sending the request,
      // `RocketClientChannel::sendSingleRequestSingleResponse` had wired up
      // the `queue_.baton_.post()` triggered by `onPayloadFrame` to call
      // into `SingleRequestSingleResponseCallback::onResponsePayload`,
      // which will parse the response and extract the FDs.
      //
      // Per @andrii, there are three possible scenarios here:
      //  - Socket used without fibers. `onResponsePayload` runs immediately.
      //  - Socket used only from a fiber manager.  There may be a delay in
      //    calling `popReceivedFdsFromSocket`, but there should be no
      //    reordering of events.
      //  - EDGE CASE: Socket used both from fiber manager and not.  In this
      //    case, reordering is possible.  This kind of usage is not
      //    supported with FD passing, and it will exhibit (potentially
      //    nondeterministic) runtime failures due to sequence number
      //    checks.
      return ctx.onPayloadFrame(std::move(payloadFrame));
    }
    case FrameType::ERROR: {
      // Protocol specifies that partial PAYLOAD cannot have arrived before an
      // ERROR frame.
      if (ctx.hasPartialPayload()) {
        return close(makeContractViolation(
            "Partial payload has arrived before an error frame."));
      }
      return ctx.onErrorFrame(ErrorFrame(std::move(frame)));
    }
    default:
      close(
          transport::TTransportException(
              transport::TTransportException::TTransportExceptionType::
                  NETWORK_ERROR,
              folly::to<std::string>(
                  "Client attempting to handle unhandleable frame type: ",
                  static_cast<uint8_t>(frameType))));
  }
}

void RocketClient::handleStreamChannelFrame(
    StreamId streamId,
    FrameType frameType,
    std::unique_ptr<folly::IOBuf> frame) {
  auto it = streams_.find(streamId);
  if (it == streams_.end()) {
    notifyIfDetachable();
    return;
  }
  it->match([&](auto* serverCallbackPtr) {
    auto& serverCallback = *serverCallbackPtr;
    switch (frameType) {
      case FrameType::PAYLOAD:
        return this->handlePayloadFrame(serverCallback, std::move(frame));
      case FrameType::ERROR:
        return this->handleErrorFrame(serverCallback, std::move(frame));
      case FrameType::REQUEST_N:
        return this->handleRequestNFrame(serverCallback, std::move(frame));
      case FrameType::CANCEL:
        return this->handleCancelFrame(serverCallback, std::move(frame));
      case FrameType::EXT:
        return this->handleExtFrame(serverCallback, std::move(frame));
      default:
        this->close(
            transport::TTransportException(
                transport::TTransportException::TTransportExceptionType::
                    NETWORK_ERROR,
                folly::to<std::string>(
                    "Client attempting to handle unhandleable frame type: ",
                    static_cast<uint8_t>(frameType))));
    }
  });
}

template <typename CallbackType>
void RocketClient::handlePayloadFrame(
    CallbackType& serverCallback, std::unique_ptr<folly::IOBuf> frame) {
  PayloadFrame payloadFrame{std::move(frame)};
  const auto streamId = payloadFrame.streamId();
  // Note that if the payload frame arrives in fragments, we rely on the
  // last fragment having the right next and/or complete flags set.
  const bool next = payloadFrame.hasNext();
  const bool complete = payloadFrame.hasComplete();
  if (auto fullPayload = bufferOrGetFullPayload(std::move(payloadFrame))) {
    if (isFirstResponse(streamId)) {
      handleFirstResponse(
          serverCallback, std::move(*fullPayload), next, complete);
    } else if constexpr (std::is_same_v<
                             CallbackType,
                             RocketSinkServerCallback>) {
      handleSinkResponse(
          streamId, serverCallback, std::move(*fullPayload), next, complete);
    } else if constexpr (std::is_same_v<
                             CallbackType,
                             RocketBiDiServerCallback>) {
      handleBiDiResponse(
          streamId, serverCallback, std::move(*fullPayload), next, complete);
    } else {
      handleStreamResponse(
          streamId, serverCallback, std::move(*fullPayload), next, complete);
    }
  }
}

template <typename CallbackType>
void RocketClient::handleFirstResponse(
    CallbackType& serverCallback,
    Payload&& fullPayload,
    bool next,
    bool complete) {
  auto streamId = serverCallback.streamId();
  acknowledgeFirstResponse(streamId);
  if (!next) {
    constexpr auto kErrorMsg =
        "Received payload frame but missing initial response";
    serverCallback.onInitialError(makeContractViolation(kErrorMsg));
    freeStream(streamId);
    contractViolation(kErrorMsg);
    return;
  }
  auto firstResponse = getPayloadSerializer()->unpack<FirstResponsePayload>(
      std::move(fullPayload), encodeMetadataUsingBinary_);
  if (firstResponse.hasException()) {
    serverCallback.onInitialError(std::move(firstResponse.exception()));
    freeStream(streamId);
    return;
  }

  // Populate `firstResponse->fds`.  Then,
  // `RequestClientCallbackWrapper::onFirstResponse` moves the FDs into
  // `THeader`.  The user can access `THeader::fds` via the
  // `header_semifuture_*` generated methods.
  //
  // The rough dataflow is:
  //   (RocketSinkServerCallback and friends)::onInitialPayload
  //   -> ClientStreamBridge::onFirstResponse
  //      created via createStreamClientCallback in codegen code
  //      which wraps makeHeaderSemiFutureCallback in codegen code
  //   -> RequestClientCallbackWrapper::onFirstResponse
  //      populates THeader
  const auto& metadata = firstResponse->metadata;
  if (metadata.fdMetadata().has_value()) {
    const auto& fdMetadata = *metadata.fdMetadata();
    auto tryFds = popReceivedFdsFromSocket(
        getTransportWrapper(),
        fdMetadata.numFds().value_or(0),
        fdMetadata.fdSeqNum().value_or(folly::SocketFds::kNoSeqNum));
    if (tryFds.hasException()) {
      auto errMsg = tryFds.exception().what().toStdString();
      serverCallback.onInitialError(std::move(tryFds.exception()));
      freeStream(streamId);
      contractViolation(errMsg);
      return;
    }
    firstResponse->fds = std::move(tryFds->dcheckReceivedOrEmpty());
  }

  if (!serverCallback.onInitialPayload(std::move(*firstResponse), evb_)) {
    return;
  }
  if (complete) {
    if constexpr (std::is_same_v<CallbackType, RocketSinkServerCallback>) {
      serverCallback.onFinalResponse(StreamPayload({}, {}));
    } else if constexpr (std::is_same_v<
                             CallbackType,
                             RocketBiDiServerCallback>) {
      bool needsFree = !serverCallback.state().isSinkOpen();
      std::ignore = serverCallback.onStreamComplete();
      if (needsFree) {
        freeStream(streamId);
      }
      return;
    } else {
      serverCallback.onStreamComplete();
    }
    freeStream(streamId);
  }
}

template <typename CallbackType>
void RocketClient::handleStreamResponse(
    StreamId streamId,
    CallbackType& serverCallback,
    Payload&& fullPayload,
    bool next,
    bool complete) {
  if (next) {
    auto streamPayload = getPayloadSerializer()->unpack<StreamPayload>(
        std::move(fullPayload), encodeMetadataUsingBinary_);
    if (streamPayload.hasException()) {
      serverCallback.onStreamError(std::move(streamPayload.exception()));
      freeStream(streamId);
      return;
    }

    // Populates `streamPayload->fds`.  Then, if the user calls
    // `toAsyncGeneratorWithHeader`, the FDs get moved into
    // ``RichPayloadReceived::fds`, where the stream consumer can access
    // them together with the payload.
    //
    // The rough dataflow is:
    //   -> RocketStreamServerCallback::{onStreamPayload,onStreamFinalPayload}
    //   -> ClientStreamBridge::onStreamNext
    //      created via createStreamClientCallback in codegen code
    //      which wraps makeHeaderSemiFutureCallback in codegen code
    //   -> TwoWayBridge:serverPush
    //   -> feeds into `toAsyncGeneratorImpl` via the inline `ReadyCallback`
    //      if the user calls `toAsyncGeneratorWithHeader`
    const auto metadata = streamPayload->metadata;
    if (metadata.fdMetadata().has_value()) {
      const auto& fdMetadata = *metadata.fdMetadata();
      auto tryFds = popReceivedFdsFromSocket(
          getTransportWrapper(),
          fdMetadata.numFds().value_or(0),
          fdMetadata.fdSeqNum().value_or(folly::SocketFds::kNoSeqNum));
      if (tryFds.hasException()) {
        auto errMsg = tryFds.exception().what().toStdString();
        serverCallback.onStreamError(std::move(tryFds.exception()));
        freeStream(streamId);
        contractViolation(errMsg);
        return;
      }
      streamPayload->fds = std::move(tryFds->dcheckReceivedOrEmpty());
    }

    streamPayload->isOrderedHeader = true;
    auto payloadMetadataRef = streamPayload->metadata.payloadMetadata();
    if (payloadMetadataRef &&
        payloadMetadataRef->getType() ==
            PayloadMetadata::Type::exceptionMetadata) {
      serverCallback.onStreamError(
          apache::thrift::detail::EncodedStreamError(
              std::move(streamPayload.value())));
      freeStream(streamId);
      return;
    }
    if (complete) {
      serverCallback.onStreamFinalPayload(std::move(*streamPayload));
      freeStream(streamId);
      return;
    }
    std::ignore = serverCallback.onStreamPayload(std::move(*streamPayload));
    return;
  }
  if (complete) {
    serverCallback.onStreamComplete();
    freeStream(streamId);
    return;
  }
  constexpr auto kErrorMsg =
      "Received stream payload frame with both next and complete flags not set";
  serverCallback.onStreamError(makeContractViolation(kErrorMsg));
  freeStream(streamId);
  contractViolation(kErrorMsg);
}

void RocketClient::handleSinkResponse(
    StreamId streamId,
    RocketSinkServerCallback& serverCallback,
    Payload&& fullPayload,
    bool next,
    bool complete) {
  auto sendContractViolation = [&](std::string msg) {
    serverCallback.onFinalResponseError(makeContractViolation(msg));
    freeStream(streamId);
    contractViolation(msg);
  };
  if (next) {
    auto streamPayload = getPayloadSerializer()->unpack<StreamPayload>(
        std::move(fullPayload), encodeMetadataUsingBinary_);
    if (streamPayload.hasException()) {
      serverCallback.onFinalResponseError(std::move(streamPayload.exception()));
      freeStream(streamId);
      return;
    }

    // FIXME: Don't bother populating `streamPayload.fds`.  Unfortunately,
    // FDs currently have nowhere to go in the bidi codegen'd code, since
    // they don't seem to have the analog of `header_semifuture_METHOD`.
    DCHECK(
        !streamPayload->metadata.fdMetadata().has_value() ||
        streamPayload->metadata.fdMetadata()->numFds().value_or(0) == 0)
        << "FD passing is not implemented for bidirectional streams";

    auto payloadMetadataRef = streamPayload->metadata.payloadMetadata();
    if (payloadMetadataRef &&
        payloadMetadataRef->getType() ==
            PayloadMetadata::Type::exceptionMetadata) {
      serverCallback.onFinalResponseError(
          apache::thrift::detail::EncodedStreamError(
              std::move(streamPayload.value())));
      freeStream(streamId);
      return;
    }
    if (complete) {
      serverCallback.onFinalResponse(std::move(*streamPayload));
      freeStream(streamId);
      return;
    }
    return sendContractViolation(
        "Received sink payload frame with next flag set but complete flag not set");
  }
  if (complete) {
    return sendContractViolation(
        "Received sink payload frame with complete flag set but next flag not set");
  }
  return sendContractViolation(
      "Received sink payload frame with both next and complete flags not set");
}

void RocketClient::handleBiDiResponse(
    StreamId streamId,
    RocketBiDiServerCallback& serverCallback,
    Payload&& fullPayload,
    bool next,
    bool complete) {
  if (next) {
    auto streamPayload = getPayloadSerializer()->unpack<StreamPayload>(
        std::move(fullPayload), encodeMetadataUsingBinary_);
    if (streamPayload.hasException()) {
      bool needsFree = !serverCallback.state().isSinkOpen();
      serverCallback.onStreamError(std::move(streamPayload.exception()));
      if (needsFree) {
        freeStream(streamId);
      }
      return;
    }

    // FIXME: Don't bother populating `streamPayload.fds`.  Unfortunately,
    // FDs currently have nowhere to go in the bidi codegen'd code, since
    // they don't seem to have the analog of `header_semifuture_METHOD`.
    DCHECK(
        !streamPayload->metadata.fdMetadata().has_value() ||
        streamPayload->metadata.fdMetadata()->numFds().value_or(0) == 0)
        << "FD passing is not implemented for bidirectional streams";
    auto payloadMetadataRef = streamPayload->metadata.payloadMetadata();
    if (payloadMetadataRef &&
        payloadMetadataRef->getType() ==
            PayloadMetadata::Type::exceptionMetadata) {
      bool needsFree = !serverCallback.state().isSinkOpen();
      serverCallback.onStreamError(
          apache::thrift::detail::EncodedStreamError(
              std::move(streamPayload.value())));
      if (needsFree) {
        freeStream(streamId);
      }
      return;
    }
    if (complete) {
      bool needsFree = !serverCallback.state().isSinkOpen();
      std::ignore =
          serverCallback.onStreamFinalPayload(std::move(*streamPayload));
      if (needsFree) {
        freeStream(streamId);
      }
      return;
    }
    std::ignore = serverCallback.onStreamPayload(std::move(*streamPayload));
    return;
  }

  if (complete) {
    bool needsFree = !serverCallback.state().isSinkOpen();
    std::ignore = serverCallback.onStreamComplete();
    if (needsFree) {
      freeStream(streamId);
    }
    return;
  }
  constexpr auto kErrorMsg =
      "Received stream payload frame with both next and complete flags not set";
  serverCallback.onStreamError(makeContractViolation(kErrorMsg));
  freeStream(streamId);
  contractViolation(kErrorMsg);
}

template <typename CallbackType>
void RocketClient::handleErrorFrame(
    CallbackType& serverCallback, std::unique_ptr<folly::IOBuf> frame) {
  ErrorFrame errorFrame{std::move(frame)};
  const auto streamId = errorFrame.streamId();
  auto ew = folly::make_exception_wrapper<RocketException>(
      errorFrame.errorCode(), std::move(errorFrame.payload()).data());
  if (isFirstResponse(streamId)) {
    acknowledgeFirstResponse(streamId);
    serverCallback.onInitialError(std::move(ew));
  } else if constexpr (std::is_same_v<CallbackType, RocketSinkServerCallback>) {
    serverCallback.onFinalResponseError(std::move(ew));
  } else if constexpr (std::is_same_v<CallbackType, RocketBiDiServerCallback>) {
    // TODO(T235448311): this should probably close the sink too
    bool needsFree = !serverCallback.state().isSinkOpen();
    serverCallback.onStreamError(std::move(ew));
    if (needsFree) {
      freeStream(streamId);
    }
    return;
  } else {
    serverCallback.onStreamError(std::move(ew));
  }
  freeStream(streamId);
}

template <typename CallbackType>
void RocketClient::handleRequestNFrame(
    CallbackType& serverCallback, std::unique_ptr<folly::IOBuf> frame) {
  RequestNFrame requestNFrame{std::move(frame)};
  auto streamId = requestNFrame.streamId();
  if (isFirstResponse(streamId)) {
    constexpr auto kErrorMsg =
        "Received RequestN frame but missing initial response";
    serverCallback.onInitialError(makeContractViolation(kErrorMsg));
    freeStream(streamId);
    contractViolation(kErrorMsg);
  } else if constexpr (
      std::is_same_v<CallbackType, RocketSinkServerCallback> ||
      std::is_same_v<CallbackType, RocketBiDiServerCallback>) {
    serverCallback.onSinkRequestN(std::move(requestNFrame).requestN());
  } else {
    constexpr auto kErrorMsg = "Received RequestN frame for stream";
    serverCallback.onStreamError(makeContractViolation(kErrorMsg));
    freeStream(streamId);
    contractViolation(kErrorMsg);
  }
}

template <typename CallbackType>
void RocketClient::handleCancelFrame(
    CallbackType& serverCallback, std::unique_ptr<folly::IOBuf> frame) {
  CancelFrame cancelFrame{std::move(frame)};
  auto streamId = cancelFrame.streamId();
  if (isFirstResponse(streamId)) {
    constexpr auto kErrorMsg =
        "Received Cancel frame but missing initial response";
    serverCallback.onInitialError(makeContractViolation(kErrorMsg));
    freeStream(streamId);
    contractViolation(kErrorMsg);
    return;
  }

  if constexpr (std::is_same_v<CallbackType, RocketBiDiServerCallback>) {
    if (!serverCallback.state().isSinkOpen()) {
      return;
    }
    bool needsFree = !serverCallback.state().isStreamOpen();
    serverCallback.onSinkCancel();
    if (needsFree) {
      freeStream(streamId);
    }
    return;
  }

  constexpr auto kErrorMsg = "Received Cancel frame";
  if constexpr (std::is_same_v<CallbackType, RocketSinkServerCallback>) {
    serverCallback.onFinalResponseError(makeContractViolation(kErrorMsg));
  } else {
    serverCallback.onStreamError(makeContractViolation(kErrorMsg));
  }
  freeStream(streamId);
  contractViolation(kErrorMsg);
}

template <typename CallbackType>
void RocketClient::handleExtFrame(
    CallbackType& serverCallback, std::unique_ptr<folly::IOBuf> frame) {
  ExtFrame extFrame{std::move(frame)};
  auto streamId = extFrame.streamId();
  if (isFirstResponse(streamId)) {
    constexpr auto kErrorMsg =
        "Received Ext frame but missing initial response";
    serverCallback.onInitialError(makeContractViolation(kErrorMsg));
    freeStream(streamId);
    contractViolation(kErrorMsg);
    return;
  }

  if (extFrame.hasIgnore()) {
    return;
  }

  auto errorMsg = fmt::format(
      "Received unhandleable ext frame type ({}) without ignore flag",
      static_cast<uint32_t>(extFrame.extFrameType()));
  close(
      transport::TTransportException(
          transport::TTransportException::TTransportExceptionType::
              NOT_SUPPORTED,
          errorMsg));
}

void RocketClient::handleError(RocketException&& rex) {
  auto enrichMsg = [](const char* msg, RocketException& rex) {
    if (auto errorData = rex.moveErrorData()) {
      return fmt::format(
          "{}: {} [{}]",
          msg,
          folly::StringPiece(errorData->coalesce()),
          toString(rex.getErrorCode()));
    }
    return fmt::format("{} [{}]", msg, toString(rex.getErrorCode()));
  };
  folly::exception_wrapper ew;
  switch (rex.getErrorCode()) {
    case ErrorCode::CONNECTION_CLOSE: {
      if (clientState_.connState != ConnectionState::CONNECTED) {
        return;
      }

      clientState_.connState = ConnectionState::CLOSING;

      writeLoopCallback_.cancelLoopCallback();
      queue_.failAllScheduledWrites(
          transport::TTransportException(
              transport::TTransportException::NOT_OPEN,
              "Connection closed by server"));

      notifyIfDetachable();

      return;
    }
    case ErrorCode::INVALID_SETUP: {
      ew = transport::TTransportException(
          transport::TTransportException::INVALID_SETUP,
          enrichMsg("Connection setup failed", rex));
      break;
    }
    default: {
      close(
          transport::TTransportException(
              transport::TTransportException::END_OF_FILE,
              enrichMsg("Unhandled error frame on control stream", rex)));
      return;
    }
  }
  if (clientState_.connState == ConnectionState::ERROR) {
    error_ = std::move(ew);
  } else {
    close(std::move(ew));
  }
}

folly::Try<Payload> RocketClient::sendRequestResponseSync(
    Payload&& request,
    std::chrono::milliseconds timeout,
    WriteSuccessCallback* writeSuccessCallback) {
  DestructorGuard dg(this);
  auto g = makeRequestCountGuard(RequestType::CLIENT);
  auto setupFrame = moveOutSetupFrame();
  RequestContext ctx(
      RequestResponseFrame(makeStreamId(), std::move(request)),
      queue_,
      setupFrame.get(),
      writeSuccessCallback,
      getIOBufFactory());
  if (auto ew = err(scheduleWrite(ctx))) {
    return folly::Try<Payload>(std::move(ew));
  }
  return ctx.waitForResponse(timeout);
}

void RocketClient::sendRequestResponse(
    Payload&& request,
    std::chrono::milliseconds timeout,
    std::unique_ptr<RequestResponseCallback> callback) {
  auto setupFrame = moveOutSetupFrame();
  auto rctx = std::make_unique<RequestContext>(
      RequestResponseFrame(makeStreamId(), std::move(request)),
      queue_,
      setupFrame.get(),
      callback.get(),
      getIOBufFactory());
  auto callbackWithGuard = [this,
                            dg = DestructorGuard(this),
                            g = makeRequestCountGuard(RequestType::CLIENT),
                            callback =
                                std::move(callback)](auto&& response) mutable {
    callback->onResponsePayload(getTransportWrapper(), std::move(response));
  };

  using CallbackWithGuard = decltype(callbackWithGuard);
  class Context : public folly::fibers::Baton::Waiter,
                  public folly::HHWheelTimer::Callback {
   public:
    Context(std::unique_ptr<RequestContext> rctx, CallbackWithGuard&& callback)
        : callback_(std::move(callback)), rctx_(std::move(rctx)) {}

    void send(RocketClient& client, std::chrono::milliseconds timeout) && {
      if (auto ew = err(client.scheduleWrite(*rctx_))) {
        SCOPE_EXIT {
          delete this;
        };
        return callback_(folly::Try<rocket::Payload>(std::move(ew)));
      }
      rctx_->setTimeoutInfo(client.evb_->timer(), *this, timeout);
      rctx_->waitForWriteToCompleteSchedule(this);
    }

   private:
    // DestructorGuard held by callback_ is needed to avoid freeing RocketClient
    // within the invocation of callback_.
    CallbackWithGuard callback_;
    std::unique_ptr<RequestContext> rctx_;

    void post() override {
      // On the timeout path, post() will be called once more in
      // abortSentRequest() within getResponse(). Avoid recursive misbehavior.
      if (auto requestCtx = std::move(rctx_)) {
        SCOPE_EXIT {
          delete this;
        };
        cancelTimeout();
        callback_(std::move(*requestCtx).getResponse());
      }
    }

    void timeoutExpired() noexcept override { post(); }
  };

  auto* context = new Context(std::move(rctx), std::move(callbackWithGuard));
  std::move(*context).send(*this, timeout);
}

folly::Try<void> RocketClient::sendRequestFnfSync(Payload&& request) {
  CHECK(folly::fibers::onFiber());
  DestructorGuard dg(this);
  auto g = makeRequestCountGuard(RequestType::CLIENT);
  auto setupFrame = moveOutSetupFrame();
  RequestContext ctx(
      RequestFnfFrame(makeStreamId(), std::move(request)),
      queue_,
      setupFrame.get(),
      nullptr,
      getIOBufFactory());
  if (auto ew = err(scheduleWrite(ctx))) {
    return folly::Try<void>(std::move(ew));
  }
  return ctx.waitForWriteToComplete();
}

void RocketClient::sendRequestFnf(
    Payload&& request, std::unique_ptr<RequestFnfCallback> callback) {
  auto setupFrame = moveOutSetupFrame();
  auto rctx = std::make_unique<RequestContext>(
      RequestFnfFrame(makeStreamId(), std::move(request)),
      queue_,
      setupFrame.get(),
      nullptr,
      getIOBufFactory());
  auto callbackWithGuard =
      [dg = DestructorGuard(this),
       g = makeRequestCountGuard(RequestType::CLIENT),
       callback = std::move(callback)](auto&& writeResult) mutable {
        callback->onWrite(std::move(writeResult));
      };

  using CallbackWithGuard = decltype(callbackWithGuard);
  class Context : public folly::fibers::Baton::Waiter {
   public:
    Context(std::unique_ptr<RequestContext> rctx, CallbackWithGuard&& callback)
        : callback_(std::move(callback)), rctx_(std::move(rctx)) {}

    void send(RocketClient& client) && {
      if (auto ew = err(client.scheduleWrite(*rctx_))) {
        SCOPE_EXIT {
          delete this;
        };
        return callback_(folly::Try<void>(std::move(ew)));
      }
      rctx_->waitForWriteToCompleteSchedule(this);
    }

   private:
    // DestructorGuard held by callback_ is needed to avoid freeing
    // RocketClient within the invocation of callback_.
    CallbackWithGuard callback_;
    std::unique_ptr<RequestContext> rctx_;

    void post() override {
      SCOPE_EXIT {
        delete this;
      };
      callback_(rctx_->waitForWriteToCompleteResult());
    }
  };

  auto* context = new Context(std::move(rctx), std::move(callbackWithGuard));
  std::move(*context).send(*this);
}

void RocketClient::sendRequestStream(
    Payload&& request,
    std::chrono::milliseconds firstResponseTimeout,
    std::chrono::milliseconds chunkTimeout,
    int32_t initialRequestN,
    StreamClientCallback* clientCallback) {
  const auto streamId = makeStreamId();
  if (chunkTimeout != std::chrono::milliseconds::zero()) {
    auto serverCallback =
        std::make_unique<RocketStreamServerCallbackWithChunkTimeout>(
            streamId, *this, *clientCallback, chunkTimeout, initialRequestN);
    sendRequestStreamChannel(
        streamId,
        std::move(request),
        firstResponseTimeout,
        initialRequestN,
        std::move(serverCallback));
  } else {
    auto serverCallback = std::make_unique<RocketStreamServerCallback>(
        streamId, *this, *clientCallback);
    sendRequestStreamChannel(
        streamId,
        std::move(request),
        firstResponseTimeout,
        initialRequestN,
        std::move(serverCallback));
  }
}

void RocketClient::sendRequestSink(
    Payload&& request,
    std::chrono::milliseconds firstResponseTimeout,
    SinkClientCallback* clientCallback,
    folly::Optional<CompressionConfig> compressionConfig) {
  const auto streamId = makeStreamId();

  std::unique_ptr<CompressionConfig> compressionConfigP;
  if (compressionConfig.has_value()) {
    compressionConfigP =
        std::make_unique<CompressionConfig>(*compressionConfig);
  }
  auto serverCallback = std::make_unique<RocketSinkServerCallback>(
      streamId, *this, *clientCallback, std::move(compressionConfigP));
  sendRequestStreamChannel(
      streamId,
      std::move(request),
      firstResponseTimeout,
      1,
      std::move(serverCallback));
}

void RocketClient::sendRequestBiDi(
    Payload&& request,
    std::chrono::milliseconds firstResponseTimeout,
    int32_t initialRequestN,
    BiDiClientCallback* clientCallback,
    folly::Optional<CompressionConfig> compressionConfig) {
  const auto streamId = makeStreamId();

  std::unique_ptr<CompressionConfig> compressionConfigP;
  if (compressionConfig.has_value()) {
    compressionConfigP =
        std::make_unique<CompressionConfig>(*compressionConfig);
  }
  auto serverCallback = std::make_unique<RocketBiDiServerCallback>(
      streamId, *this, *clientCallback, std::move(compressionConfigP));
  sendRequestStreamChannel(
      streamId,
      std::move(request),
      firstResponseTimeout,
      initialRequestN,
      std::move(serverCallback));
}

template <typename ServerCallback>
void RocketClient::sendRequestStreamChannel(
    const StreamId& streamId,
    Payload&& request,
    std::chrono::milliseconds firstResponseTimeout,
    int32_t initialRequestN,
    std::unique_ptr<ServerCallback> serverCallback) {
  using Frame = std::conditional_t<
      std::is_same_v<RocketStreamServerCallback, ServerCallback> ||
          std::is_same_v<
              RocketStreamServerCallbackWithChunkTimeout,
              ServerCallback>,
      RequestStreamFrame,
      RequestChannelFrame>;

  // One extra credit for initial response.
  if (initialRequestN < std::numeric_limits<int32_t>::max()) {
    initialRequestN += 1;
  }

  class Context : public folly::fibers::Baton::Waiter {
   public:
    Context(
        RocketClient& client,
        StreamId streamId,
        std::unique_ptr<SetupFrame> setupFrame,
        Frame&& frame,
        RequestContextQueue& queue,
        ServerCallback& serverCallback)
        : client_(client),
          streamId_(streamId),
          setupFrame_(std::move(setupFrame)),
          ctx_(
              std::move(frame),
              queue,
              setupFrame_.get(),
              nullptr /* writeCallback */),
          serverCallback_(serverCallback) {}

    ~Context() override {
      if (!complete_) {
        client_.freeStream(streamId_);
      }
    }

    static void run(
        std::unique_ptr<Context> self,
        std::chrono::milliseconds firstResponseTimeout) {
      if (auto ew = err(self->client_.scheduleWrite(self->ctx_))) {
        return self->serverCallback_.onInitialError(std::move(ew));
      }

      self->client_.maybeScheduleFirstResponseTimeout(
          self->streamId_, firstResponseTimeout);
      auto& ctx = self->ctx_;
      ctx.waitForWriteToCompleteSchedule(self.release());
    }

   private:
    void post() override {
      SCOPE_EXIT {
        delete this;
      };

      auto writeCompleted = ctx_.waitForWriteToCompleteResult();
      // If the write failed, check that the stream has not already received the
      // first response. The writeErr() callback for a batch of requests may
      // fire after the initial payload/error has already been processed.
      if (writeCompleted.hasException() && client_.isFirstResponse(streamId_)) {
        return serverCallback_.onInitialError(
            std::move(writeCompleted.exception()));
      }
      complete_ = true;
    }

    RocketClient& client_;
    StreamId streamId_;
    std::unique_ptr<SetupFrame> setupFrame_;
    RequestContext ctx_;
    ServerCallback& serverCallback_;
    bool complete_{false};
  };

  auto serverCallbackPtr = serverCallback.get();
  streams_.emplace(ServerCallbackUniquePtr(std::move(serverCallback)));

  Context::run(
      std::make_unique<Context>(
          *this,
          streamId,
          moveOutSetupFrame(),
          Frame(streamId, std::move(request), initialRequestN),
          queue_,
          *serverCallbackPtr),
      firstResponseTimeout);
}

template <class OnError>
class RocketClient::SendFrameContext : public folly::fibers::Baton::Waiter {
 public:
  template <class Frame>
  SendFrameContext(RocketClient& client, Frame&& frame, OnError&& onError)
      : client_(client),
        ctx_(
            std::forward<Frame>(frame),
            client_.queue_,
            nullptr,
            nullptr,
            client_.getIOBufFactory()),
        onError_(std::forward<OnError>(onError)) {}

  template <class InitFunc>
  SendFrameContext(
      RocketClient& client,
      InitFunc&& initFunc,
      StreamId streamId,
      OnError&& onError)
      : client_(client),
        ctx_(
            std::forward<InitFunc>(initFunc),
            client_.serverVersion_,
            streamId,
            client_.queue_),
        onError_(std::forward<OnError>(onError)) {
    // if server version is not yet available, start buffering requests
    if (UNLIKELY(ctx_.state() == RequestContext::State::DEFERRED_INIT)) {
      client_.onServerVersionRequired();
    }
  }

  [[nodiscard]] static bool run(std::unique_ptr<SendFrameContext> self) {
    if (auto ew = err(self->client_.scheduleWrite(self->ctx_))) {
      self->onError_(toTransportException(std::move(ew)));
      return false;
    }
    auto& ctx = self->ctx_;
    ctx.waitForWriteToCompleteSchedule(self.release());
    return true;
  }

 private:
  void post() override {
    std::unique_ptr<SendFrameContext> self(this);
    auto writeCompleted = ctx_.waitForWriteToCompleteResult();
    if (writeCompleted.hasException()) {
      self->onError_(
          toTransportException(std::move(writeCompleted.exception())));
    }
  }

  RocketClient& client_;
  RequestContext ctx_;
  std::decay_t<OnError> onError_;
};

template <typename Frame, typename OnError>
bool RocketClient::sendFrame(Frame&& frame, OnError&& onError) {
  using Context = SendFrameContext<OnError>;
  auto ctx = std::make_unique<Context>(
      *this, std::forward<Frame>(frame), std::forward<OnError>(onError));
  return Context::run(std::move(ctx));
}

template <typename DeferredInitFunc, typename OnError>
bool RocketClient::sendVersionDependentFrame(
    DeferredInitFunc&& deferredInit, StreamId streamId, OnError&& onError) {
  using Context = SendFrameContext<OnError>;
  auto ctx = std::make_unique<Context>(
      *this,
      std::forward<DeferredInitFunc>(deferredInit),
      streamId,
      std::forward<OnError>(onError));
  return Context::run(std::move(ctx));
}

bool RocketClient::sendRequestN(StreamId streamId, int32_t n) {
  auto g = makeRequestCountGuard(RequestType::INTERNAL);
  if (UNLIKELY(n <= 0)) {
    return true;
  }

  DCHECK(streamExists(streamId));

  return sendFrame(
      RequestNFrame(streamId, n),
      [dg = DestructorGuard(this), this, g = std::move(g)](
          transport::TTransportException ex) {
        VLOG(1) << "sendRequestN failed, closing now: " << ex.what();
        close(std::move(ex));
      });
}

void RocketClient::cancelStream(StreamId streamId, bool freeChannel) {
  auto g = makeRequestCountGuard(RequestType::INTERNAL);
  if (freeChannel) {
    freeStream(streamId);
  }
  std::ignore = sendFrame(
      CancelFrame(streamId),
      [dg = DestructorGuard(this), this, g = std::move(g)](
          transport::TTransportException ex) {
        VLOG(1) << "cancelStream failed, closing now: " << ex.what();
        close(std::move(ex));
      });
}

bool RocketClient::sendPayload(
    StreamId streamId, StreamPayload&& payload, Flags flags) {
  return sendFrame(
      PayloadFrame(
          streamId,
          getPayloadSerializer()->pack(
              std::move(payload),
              encodeMetadataUsingBinary(),
              getTransportWrapper()),
          flags),
      [this,
       dg = DestructorGuard(this),
       g = makeRequestCountGuard(RequestType::INTERNAL)](
          transport::TTransportException ex) {
        VLOG(1) << "sendPayload failed, closing now: " << ex.what();
        close(std::move(ex));
      });
}

bool RocketClient::sendError(
    StreamId streamId, RocketException&& rex, bool freeChannel) {
  if (freeChannel) {
    freeStream(streamId);
  }
  return sendFrame(
      ErrorFrame(streamId, std::move(rex)),
      [this,
       dg = DestructorGuard(this),
       g = makeRequestCountGuard(RequestType::INTERNAL)](
          transport::TTransportException ex) {
        VLOG(1) << "sendError failed, closing now: " << ex.what();
        close(std::move(ex));
      });
}

bool RocketClient::sendComplete(StreamId streamId, bool freeChannel) {
  auto g = makeRequestCountGuard(RequestType::INTERNAL);
  if (freeChannel) {
    freeStream(streamId);
  }
  return sendPayload(
      streamId,
      StreamPayload(std::unique_ptr<folly::IOBuf>{}, {}),
      rocket::Flags().complete(true));
}

bool RocketClient::sendHeadersPush(
    StreamId streamId, HeadersPayload&& payload) {
  auto g = makeRequestCountGuard(RequestType::INTERNAL);
  auto onError = [dg = DestructorGuard(this), this, g = std::move(g)](
                     transport::TTransportException ex) {
    VLOG(1) << "sendHeadersPush failed, closing now: " << ex.what();
    close(std::move(ex));
  };
  ClientPushMetadata clientMeta;
  clientMeta.streamHeadersPush().ensure().streamId() =
      static_cast<uint32_t>(streamId);
  clientMeta.streamHeadersPush()->headersPayloadContent() =
      std::move(payload.payload);
  return sendFrame(
      MetadataPushFrame::makeFromMetadata(
          getPayloadSerializer()->packCompact(clientMeta)),
      std::move(onError));
}

bool RocketClient::sendSinkError(
    StreamId streamId, StreamPayload&& payload, bool freeChannel) {
  if (freeChannel) {
    freeStream(streamId);
  }
  return sendPayload(
      streamId, std::move(payload), Flags().next(true).complete(true));
}

folly::Try<void> RocketClient::scheduleWrite(RequestContext& ctx) {
  if (!evb_) {
    return folly::Try<
        void>(folly::make_exception_wrapper<transport::TTransportException>(
        transport::TTransportException::TTransportExceptionType::INVALID_STATE,
        "Cannot send requests on a detached client"));
  }

  if (clientState_.connState != ConnectionState::CONNECTED) {
    return folly::Try<void>(
        folly::make_exception_wrapper<transport::TTransportException>(
            transport::TTransportException::TTransportExceptionType::NOT_OPEN,
            "Write not scheduled on disconnected client"));
  }

  queue_.enqueueScheduledWrite(ctx);
  scheduleWriteLoopCallback();
  return {};
}

StreamId RocketClient::makeStreamId() {
  StreamId id;
  do {
    id = nextStreamId_;
    nextStreamId_ += 2;
  } while (clientState_.hitMaxStreamId && streams_.contains(id));

  if (UNLIKELY(id == StreamId::maxClientStreamId())) {
    nextStreamId_ = StreamId(1);
    clientState_.hitMaxStreamId = true;
  }
  return id;
}

void RocketClient::WriteLoopCallback::runLoopCallback() noexcept {
  client_.writeScheduledRequestsToSocket();
}

void RocketClient::scheduleWriteLoopCallback() {
  if (!writeLoopCallback_.isLoopCallbackScheduled()) {
    if (flushList_) {
      flushList_->push_back(writeLoopCallback_);
    } else {
      // use FlushManager to handle scheduling
      flushManager_->enqueueFlush(this->writeLoopCallback_);
    }
  }
}

void RocketClient::writeScheduledRequestsToSocket() noexcept {
  DestructorGuard dg(this);

  folly::WriteFlags wflags = THRIFT_FLAG(rocket_client_set_eor_flag)
      ? folly::WriteFlags::EOR
      : folly::WriteFlags::NONE;
  if (clientState_.connState == ConnectionState::CONNECTED) {
    size_t endOffset = 0;
    std::unique_ptr<folly::IOBuf> buf;
    queue_.prepareNextScheduledWritesBatch([&](RequestContext& req) {
      auto reqBuf = req.releaseSerializedChain();
      endOffset += reqBuf->computeChainDataLength(); // for `writeErr`
      req.setEndOffsetInBatch(endOffset);
      if (!buf) {
        buf = std::move(reqBuf);
      } else {
        buf->prependChain(std::move(reqBuf));
      }
      if (LIKELY(req.fds.empty())) {
        return; // Fast path: no FDs, no batch splitting.
      }

      // The current request has FDs, send them together with any
      // accumulated data.
      //
      // First, split up the batch -- each `writeChain*` gets a separate
      // `writeSuccess` callback (which might be called synchronously from
      // `writeChainWithFds` below).
      req.markLastInWriteBatch();
      // KEEP THIS INVARIANT: For the receiver to correctly match FDs to a
      // message, the FDs must be sent with the IOBuf ending with the
      // FINAL fragment of that message.  Today, message fragments are not
      // interleaved, so there is no explicit logic around this, but this
      // invariant must be preserved going forward.
      prepareWriteEvent();
      writeChainWithFds(
          socket_.get(), this, std::move(buf), std::move(req.fds), wflags);
    });
    // This batch didn't have any FDs attached.
    if (buf) {
      prepareWriteEvent();
      socket_->writeChain(this, std::move(buf), wflags);
    }
  }

  notifyIfDetachable();
}

void RocketClient::writeSuccess() noexcept {
  DestructorGuard dg(this);
  DCHECK(clientState_.connState != ConnectionState::CLOSED);

  finishWriteEvent();

  queue_.markNextSendingBatchAsSent([&](auto& req) {
    req.onWriteSuccess();
    if (!req.isRequestResponse()) {
      queue_.markAsResponded(req);
    }
  });
}

void RocketClient::writeErr(
    size_t bytesWritten, const folly::AsyncSocketException& ex) noexcept {
  DestructorGuard dg(this);
  DCHECK(clientState_.connState != ConnectionState::CLOSED);

  finishWriteEvent();

  queue_.markNextSendingBatchAsSent([&](auto& req) {
    if (bytesWritten < req.endOffsetInBatch()) {
      queue_.abortSentRequest(
          req,
          transport::TTransportException(
              transport::TTransportException::NOT_OPEN,
              fmt::format(
                  "Failed to write to remote endpoint. Wrote {} bytes."
                  " AsyncSocketException: {}",
                  bytesWritten,
                  ex.what())));
    }
  });

  close(
      transport::TTransportException(
          transport::TTransportException::UNKNOWN,
          fmt::format(
              "Failed to write to remote endpoint. Wrote {} bytes."
              " AsyncSocketException: {}",
              bytesWritten,
              ex.what())));
}

void RocketClient::closeNow(transport::TTransportException ex) noexcept {
  DestructorGuard dg(this);

  if (clientState_.connState == ConnectionState::CLOSED) {
    return;
  }

  close(ex);
  closeLoopCallback_.cancelLoopCallback();
  closeNowImpl();
}

void RocketClient::close(folly::exception_wrapper ew) noexcept {
  DestructorGuard dg(this);

  if (clientState_.connState != ConnectionState::CONNECTED &&
      clientState_.connState != ConnectionState::CLOSING) {
    return;
  }

  error_ = std::move(ew);
  clientState_.connState = ConnectionState::ERROR;

  writeLoopCallback_.cancelLoopCallback();
  queue_.failAllScheduledWrites(error_);

  if (evb_) {
    evb_->runInLoop(&closeLoopCallback_);
  }
}

void RocketClient::closeNowImpl() noexcept {
  DestructorGuard dg(this);
  DCHECK(clientState_.connState == ConnectionState::ERROR);
  if (keepAliveWatcher_) {
    keepAliveWatcher_->stop();
    if (keepAliveWatcher_->closeConnection()) {
      error_ = transport::TTransportException(
          transport::TTransportException::TTransportExceptionType::END_OF_FILE,
          "Connection was closed due to KeepAliveTimeout.");
    }
  }
  // Notice that AsyncSocket::closeNow() is a no-op if the socket is already in
  // the ERROR state -- such as if we are currently handling a writeErr() event.
  DCHECK(socket_);
  socket_->closeNow();
  // AsyncSocket::closeNow() may not unset the read callback. AsyncSocket
  // destruction may also be delayed past RocketClient destruction. Unset the
  // read callback to ensure that AsyncSocket does not subsequently use a
  // destroyed RocketClient.
  socket_->setReadCB(nullptr);
  socket_.reset();

  clientState_.connState = ConnectionState::CLOSED;
  if (auto closeCallback = std::move(closeCallback_)) {
    closeCallback();
  }
  queue_.failAllSentWrites(error_);

  // Move streams_ into a local copy before iterating and erasing. Note that
  // flowable->onError() may itself attempt to erase an element of streams_,
  // invalidating any outstanding iterators. Also, since the client is
  // shutting down now, we don't bother with notifyIfDetachable().
  auto streams = std::move(streams_);
  for (const auto& callback : streams) {
    callback.match([&](auto* serverCallback) {
      if (isFirstResponse(serverCallback->streamId())) {
        serverCallback->onInitialError(error_);
      } else {
        if constexpr (std::is_same_v<
                          decltype(serverCallback),
                          RocketSinkServerCallback*>) {
          serverCallback->onFinalResponseError(error_);
        } else if constexpr (
            std::is_same_v<
                decltype(serverCallback),
                RocketStreamServerCallback*> ||
            std::is_same_v<
                decltype(serverCallback),
                RocketStreamServerCallbackWithChunkTimeout*>) {
          serverCallback->onStreamError(error_);
        } else if constexpr (std::is_same_v<
                                 decltype(serverCallback),
                                 RocketBiDiServerCallback*>) {
          serverCallback->onConnectionClosed(error_);
        } else {
          static_assert(
              folly::always_false<decltype(serverCallback)>,
              "Impossible server callback type for stream");
        }
      }
    });
  }
  firstResponseTimeouts_.clear();
  bufferedFragments_.clear();
}

bool RocketClient::streamExists(StreamId streamId) const {
  return streams_.find(streamId) != streams_.end();
}

void RocketClient::freeStream(StreamId streamId) {
  streams_.erase(streamId);
  bufferedFragments_.erase(streamId);
  firstResponseTimeouts_.erase(streamId);
  notifyIfDetachable();
}

void RocketClient::contractViolation(std::string_view msg) {
  close(makeContractViolation(
      fmt::format(
          "Streaming contract violation: {}. Closing the connection.", msg)));
}

folly::Optional<Payload> RocketClient::bufferOrGetFullPayload(
    PayloadFrame&& payloadFrame) {
  folly::Optional<Payload> fullPayload;

  const auto streamId = payloadFrame.streamId();
  const bool hasFollows = payloadFrame.hasFollows();
  const auto it = bufferedFragments_.find(streamId);

  if (hasFollows) {
    if (it != bufferedFragments_.end()) {
      auto& firstFragments = it->second;
      firstFragments.append(std::move(payloadFrame.payload()));
    } else {
      bufferedFragments_.emplace(streamId, std::move(payloadFrame.payload()));
    }
  } else {
    if (it != bufferedFragments_.end()) {
      auto firstFragments = std::move(it->second);
      bufferedFragments_.erase(it);
      firstFragments.append(std::move(payloadFrame.payload()));
      fullPayload = std::move(firstFragments);
    } else {
      fullPayload = std::move(payloadFrame.payload());
    }
  }

  return fullPayload;
}

void RocketClient::maybeScheduleFirstResponseTimeout(
    StreamId streamId, std::chrono::milliseconds timeout) {
  DCHECK(evb_);
  DCHECK(firstResponseTimeouts_.find(streamId) == firstResponseTimeouts_.end());

  if (timeout == std::chrono::milliseconds::zero()) {
    firstResponseTimeouts_.emplace(streamId, nullptr);
    return;
  }

  auto firstResponseTimeout =
      std::make_unique<FirstResponseTimeout>(*this, streamId);
  evb_->timer().scheduleTimeout(firstResponseTimeout.get(), timeout);
  firstResponseTimeouts_.emplace(streamId, std::move(firstResponseTimeout));
}

bool RocketClient::isFirstResponse(StreamId streamId) const {
  return firstResponseTimeouts_.contains(streamId);
}

void RocketClient::acknowledgeFirstResponse(StreamId streamId) {
  firstResponseTimeouts_.erase(streamId);
}

void RocketClient::FirstResponseTimeout::timeoutExpired() noexcept {
  // remove ourselves from the timeout set to avoid being freed prematurely by
  // the callback below
  auto ptr = std::move(client_.firstResponseTimeouts_.at(streamId_));

  const auto streamIt = client_.streams_.find(streamId_);
  CHECK(streamIt != client_.streams_.end());

  streamIt->match([&](auto* serverCallback) {
    serverCallback->onInitialError(
        folly::make_exception_wrapper<transport::TTransportException>(
            transport::TTransportException::TIMED_OUT));
  });
}

void RocketClient::attachEventBase(folly::EventBase& evb) {
  if (evb_ == &evb) {
    return;
  }

  DCHECK(!evb_);
  evb.dcheckIsInEventBaseThread();

  evb_ = &evb;
  socket_->attachEventBase(evb_);
  if (keepAliveWatcher_) {
    keepAliveWatcher_->attachEventBase(evb_);
  }
  evb_->runOnDestruction(eventBaseDestructionCallback_);
  flushManager_ = &FlushManager::getInstance(*evb_);
}

void RocketClient::detachEventBase() {
  DCHECK(getDestructorGuardCount() == 0);
  DCHECK(evb_);
  evb_->dcheckIsInEventBaseThread();
  DCHECK(!writeLoopCallback_.isLoopCallbackScheduled());

  eventBaseDestructionCallback_.cancel();
  detachableLoopCallback_.cancelLoopCallback();
  if (keepAliveWatcher_) {
    keepAliveWatcher_->detachEventBase();
  }
  socket_->detachEventBase();
  flushManager_ = nullptr;
  evb_ = nullptr;
  flushList_ = nullptr;
}

void RocketClient::DetachableLoopCallback::runLoopCallback() noexcept {
  if (client_.onDetachable_ && client_.isDetachable()) {
    client_.onDetachable_();
  }
}

void RocketClient::CloseLoopCallback::runLoopCallback() noexcept {
  client_.closeNowImpl();
}

void RocketClient::OnEventBaseDestructionCallback::
    onEventBaseDestruction() noexcept {
  // Make sure we never run RocketClient destructor inline from
  // OnEventBaseDestructionCallback, since it will try to deregister itself from
  // the EventBase and deadlock.
  client_.evb_->runInLoop([dg = DestructorGuard(&client_)] {});
  client_.closeNow(transport::TTransportException("Destroying EventBase"));
}

void RocketClient::terminateInteraction(int64_t id) {
  auto guard = folly::makeGuard([this] {
    if (!--interactions_) {
      notifyIfDetachable();
    }
  });

  if (setupFrame_) {
    // we haven't sent any requests so don't need to send the termination
    return;
  }

  auto onError =
      [dg = DestructorGuard(this),
       this,
       guard = std::move(guard),
       ka = folly::getKeepAliveToken(evb_)](transport::TTransportException ex) {
        close(std::move(ex));
      };

  ClientPushMetadata clientMeta;
  clientMeta.interactionTerminate().ensure().interactionId() = id;
  std::ignore = sendFrame(
      MetadataPushFrame::makeFromMetadata(
          getPayloadSerializer()->packCompact(clientMeta)),
      std::move(onError));
}

void RocketClient::onServerVersionRequired() {
  // if a version dependent frame is scheduled but server setup response has
  // not been received, buffer all other requests until server version is
  // known
  DCHECK(serverVersion_ == -1);
  if (!queue_.startBufferingRequests()) {
    return; // already buffering
  }
  // include a timeout to close the connection if server does not respond
  serverVersionTimeout_.reset(new ServerVersionTimeout(*this));
  evb_->timer().scheduleTimeout(
      serverVersionTimeout_.get(), std::chrono::milliseconds(500));
}

void RocketClient::setServerVersion(int32_t serverVersion) {
  // set server version, cancel timeout and resolve any buffered requests
  serverVersion_ = serverVersion;
  serverVersionTimeout_.reset();
  if (queue_.resolveWriteBuffer(serverVersion)) {
    scheduleWriteLoopCallback();
  }
}

void RocketClient::sendTransportMetadataPush() {
  auto transport = getTransportWrapper();
  auto onError = [dg = DestructorGuard(this),
                  g = makeRequestCountGuard(RequestType::INTERNAL)](
                     transport::TTransportException ex) {
    FB_LOG_EVERY_MS(ERROR, 1000)
        << "sendTransportMetadataPush failed: " << ex.what();
  };
  if (auto transportMetadataPush =
          apache::thrift::detail::getTransportMetadataPush(transport)) {
    ClientPushMetadata clientMeta;
    clientMeta.transportMetadataPush() = std::move(*transportMetadataPush);
    std::ignore = sendFrame(
        MetadataPushFrame::makeFromMetadata(
            getPayloadSerializer()->packCompact(clientMeta)),
        std::move(onError));
  }
}

std::unique_ptr<SetupFrame> RocketClient::moveOutSetupFrame() {
  if (UNLIKELY(clientState_.hasPendingSetupFrame)) {
    clientState_.hasPendingSetupFrame = false;
    return std::move(setupFrame_);
  }
  return nullptr;
}

} // namespace rocket
} // namespace apache::thrift
