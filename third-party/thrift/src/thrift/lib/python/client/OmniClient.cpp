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

#include <thrift/lib/python/client/OmniClient.h>

#include <fmt/format.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/futures/Promise.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp/ContextStack.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/TProcessorEventHandler.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/FutureRequest.h>
#include <thrift/lib/cpp2/async/RequestCallback.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/util/MethodMetadata.h>

namespace thrift {
namespace python {
namespace client {

namespace {

using namespace apache::thrift;

std::pair<
    apache::thrift::ContextStack::UniquePtr,
    std::shared_ptr<apache::thrift::transport::THeader>>
makeOmniClientRequestContext(
    uint16_t protocolId,
    apache::thrift::transport::THeader::StringToStringMap headers,
    std::shared_ptr<
        std::vector<std::shared_ptr<apache::thrift::TProcessorEventHandler>>>
        handlers,
    const std::string& serviceName,
    const std::string& functionName) {
  auto header = std::make_shared<apache::thrift::transport::THeader>(
      apache::thrift::transport::THeader::ALLOW_BIG_FRAMES);
  header->setProtocolId(protocolId);
  header->setHeaders(std::move(headers));
  auto ctx = apache::thrift::ContextStack::createWithClientContextCopyNames(
      handlers, serviceName, functionName, *header);

  return {std::move(ctx), std::move(header)};
}

template <class Protocol>
TApplicationException deserializeApplicationException(
    std::unique_ptr<folly::IOBuf> buf) {
  CHECK(buf);
  Protocol in;
  in.setInput(buf.get());
  TApplicationException ex;
  ex.read(&in);
  return ex;
}

TApplicationException deserializeApplicationException(
    uint16_t protocolId, std::unique_ptr<folly::IOBuf> buf) {
  switch (protocolId) {
    case protocol::T_BINARY_PROTOCOL:
      return deserializeApplicationException<BinaryProtocolReader>(
          std::move(buf));
    case protocol::T_COMPACT_PROTOCOL:
      return deserializeApplicationException<CompactProtocolReader>(
          std::move(buf));
    default:
      return TApplicationException(
          TApplicationException::TApplicationExceptionType::INVALID_PROTOCOL,
          fmt::format("Invalid protocol {}", protocolId));
  }
}

folly::Try<folly::IOBuf> decode_stream_exception(folly::exception_wrapper ew) {
  using IOBufTry = folly::Try<folly::IOBuf>;
  IOBufTry ret;
  ew.handle(
      [&ret](apache::thrift::detail::EncodedError& err) {
        ret = IOBufTry(std::move(*err.encoded));
      },
      [&ret](apache::thrift::detail::EncodedStreamError& err) {
        auto& payload = err.encoded;
        DCHECK_EQ(payload.metadata.payloadMetadata().has_value(), true);
        DCHECK_EQ(
            payload.metadata.payloadMetadata()->getType(),
            PayloadMetadata::Type::exceptionMetadata);
        auto& exceptionMetadataBase =
            payload.metadata.payloadMetadata()->get_exceptionMetadata();
        if (auto exceptionMetadataRef = exceptionMetadataBase.metadata()) {
          if (exceptionMetadataRef->getType() ==
              PayloadExceptionMetadata::Type::declaredException) {
            ret = IOBufTry(std::move(*payload.payload));
          } else {
            ret = IOBufTry(TApplicationException(
                exceptionMetadataBase.what_utf8().value_or("")));
          }
        } else {
          ret = IOBufTry(
              TApplicationException("Missing payload exception metadata"));
        }
      },
      [&ret](apache::thrift::detail::EncodedStreamRpcError& err) {
        StreamRpcError streamRpcError;
        CompactProtocolReader reader;
        reader.setInput(err.encoded.get());
        streamRpcError.read(&reader);
        TApplicationException::TApplicationExceptionType exType{
            TApplicationException::UNKNOWN};
        auto code = streamRpcError.code();
        if (code &&
            (code.value() == StreamRpcErrorCode::CREDIT_TIMEOUT ||
             code.value() == StreamRpcErrorCode::CHUNK_TIMEOUT)) {
          exType = TApplicationException::TIMEOUT;
        }
        ret = IOBufTry(TApplicationException(
            exType, streamRpcError.what_utf8().value_or("")));
      },
      [](...) {});

  return ret;
}

folly::Try<folly::IOBuf> decode_stream_element(
    folly::Try<apache::thrift::StreamPayload>&& payload) {
  if (payload.hasValue()) {
    return folly::Try<folly::IOBuf>(std::move(*payload->payload));
  } else if (payload.hasException()) {
    return decode_stream_exception(std::move(payload).exception());
  } else {
    return {};
  }
}

std::unique_ptr<IOBufClientBufferedStream> extractClientStream(
    apache::thrift::ClientReceiveState& state) {
  return std::make_unique<IOBufClientBufferedStream>(
      state.extractStreamBridge(),
      decode_stream_element,
      state.bufferOptions());
}

} // namespace

OmniClient::OmniClient(RequestChannelUnique channel)
    : channel_(std::move(channel)) {}

OmniClient::OmniClient(RequestChannelShared channel)
    : channel_(std::move(channel)) {}

OmniClient::OmniClient(OmniClient&& other) noexcept
    : channel_(std::move(other.channel_)),
      factoryClient_(other.factoryClient_.load()) {}

OmniClient::~OmniClient() {
  if (!channel_) {
    return;
  }

  auto* eb = channel_->getEventBase();
  if (eb != nullptr) {
    eb->runInEventBaseThread([channel = std::move(channel_)] {});
  }
}

OmniClientResponseWithHeaders OmniClient::sync_send(
    const std::string& serviceName,
    const std::string& functionName,
    std::unique_ptr<folly::IOBuf> args,
    apache::thrift::MethodMetadata::Data&& metadata,
    const std::unordered_map<std::string, std::string>& headers,
    apache::thrift::RpcOptions&& rpcOptions) {
  return folly::coro::blockingWait(semifuture_send(
      serviceName,
      functionName,
      std::move(args),
      std::move(metadata),
      headers,
      std::move(rpcOptions)));
}

OmniClientResponseWithHeaders OmniClient::sync_send(
    const std::string& serviceName,
    const std::string& functionName,
    const std::string& args,
    apache::thrift::MethodMetadata::Data&& metadata,
    const std::unordered_map<std::string, std::string>& headers,
    apache::thrift::RpcOptions&& rpcOptions) {
  return sync_send(
      serviceName,
      functionName,
      folly::IOBuf::copyBuffer(args),
      std::move(metadata),
      headers,
      std::move(rpcOptions));
}

void OmniClient::oneway_send(
    const std::string& serviceName,
    const std::string& functionName,
    std::unique_ptr<folly::IOBuf> args,
    apache::thrift::MethodMetadata::Data&& metadata,
    const std::unordered_map<std::string, std::string>& headers,
    apache::thrift::RpcOptions&& rpcOptions) {
  for (const auto& entry : headers) {
    rpcOptions.setWriteHeader(entry.first, entry.second);
  }

  auto callbackAndFuture = makeOneWaySemiFutureCallback(channel_);
  auto callback = std::move(callbackAndFuture.first);
  sendImpl(
      std::move(rpcOptions),
      std::move(args),
      serviceName,
      functionName,
      std::move(callback),
      RpcKind::SINGLE_REQUEST_NO_RESPONSE,
      std::move(metadata));
}

void OmniClient::oneway_send(
    const std::string& serviceName,
    const std::string& functionName,
    const std::string& args,
    apache::thrift::MethodMetadata::Data&& metadata,
    const std::unordered_map<std::string, std::string>& headers,
    apache::thrift::RpcOptions&& rpcOptions) {
  oneway_send(
      serviceName,
      functionName,
      folly::IOBuf::copyBuffer(args),
      std::move(metadata),
      headers,
      std::move(rpcOptions));
}

folly::SemiFuture<OmniClientResponseWithHeaders> OmniClient::semifuture_send(
    const std::string& serviceName,
    const std::string& functionName,
    std::unique_ptr<folly::IOBuf> args,
    apache::thrift::MethodMetadata::Data&& metadata,
    const std::unordered_map<std::string, std::string>& headers,
    apache::thrift::RpcOptions&& rpcOptions,
    const apache::thrift::RpcKind rpcKind) {
  for (const auto& entry : headers) {
    rpcOptions.setWriteHeader(entry.first, entry.second);
  }

  folly::Promise<ClientReceiveState> promise;
  auto future = promise.getSemiFuture();
  try {
    sendImpl(
        std::move(rpcOptions),
        std::move(args),
        serviceName,
        functionName,
        std::make_unique<SemiFutureCallback>(std::move(promise), channel_),
        rpcKind,
        std::move(metadata));
  } catch (...) {
    return folly::makeSemiFutureWith(
        [ew = folly::exception_wrapper(std::current_exception())]() mutable {
          OmniClientResponseWithHeaders resp;
          resp.buf = folly::makeUnexpected(std::move(ew));
          return resp;
        });
  }
  return std::move(future)
      .deferValue([protocol = getChannelProtocolId(),
                   rpcKind](ClientReceiveState&& state) {
        if (state.isException()) {
          state.exception().throw_exception();
        }
        ContextStack* ctx = state.ctx();
        if (ctx) {
          ctx->preRead();
        }
        OmniClientResponseWithHeaders resp;
        if (state.messageType() == MessageType::T_REPLY) {
          SerializedMessage smsg;
          smsg.protocolType =
              apache::thrift::protocol::PROTOCOL_TYPES(protocol);
          smsg.buffer = state.serializedResponse().buffer.get();
          if (ctx) {
            ctx->onReadData(smsg);
            ctx->postRead(
                state.header(),
                state.serializedResponse()
                    .buffer
                    ->computeChainDataLength()); // TODO move this call
                                                 // to inside the python code
          }
          resp.buf = std::move(state.serializedResponse().buffer);
          if (rpcKind == RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE) {
            resp.stream = extractClientStream(state);
          }

        } else if (state.messageType() == MessageType::T_EXCEPTION) {
          resp.buf = folly::makeUnexpected(deserializeApplicationException(
              state.protocolId(),
              std::move(state.serializedResponse().buffer)));
        } else {
          resp.buf = folly::makeUnexpected(
              folly::make_exception_wrapper<TApplicationException>(
                  TApplicationException::TApplicationExceptionType::
                      INVALID_MESSAGE_TYPE,
                  fmt::format(
                      "Invalid message type: {}",
                      folly::to_underlying(state.messageType()))));
        }
        resp.headers = state.header()->releaseHeaders();
        state.resetCtx(nullptr);

        return resp;
      })
      .deferError([=](folly::exception_wrapper&& e) {
        OmniClientResponseWithHeaders resp;
        resp.buf = folly::makeUnexpected(e);

        return resp;
      });
}

folly::SemiFuture<OmniClientResponseWithHeaders> OmniClient::semifuture_send(
    const std::string& serviceName,
    const std::string& functionName,
    const std::string& args,
    apache::thrift::MethodMetadata::Data&& metadata,
    const std::unordered_map<std::string, std::string>& headers,
    apache::thrift::RpcOptions&& rpcOptions,
    const apache::thrift::RpcKind rpcKind) {
  return semifuture_send(
      serviceName,
      functionName,
      folly::IOBuf::copyBuffer(args),
      std::move(metadata),
      headers,
      std::move(rpcOptions),
      rpcKind);
}

void OmniClient::sendImpl(
    apache::thrift::RpcOptions&& rpcOptions,
    std::unique_ptr<folly::IOBuf> args,
    const std::string& serviceName,
    const std::string& functionName,
    std::unique_ptr<RequestCallback> callback,
    const apache::thrift::RpcKind rpcKind,
    apache::thrift::MethodMetadata::Data&& metadata) {
  // Create the request context.
  auto [ctx, header] = makeOmniClientRequestContext(
      channel_->getProtocolId(),
      rpcOptions.releaseWriteHeaders(),
      handlers_,
      serviceName,
      functionName);
  RequestCallback::Context callbackContext;
  callbackContext.protocolId = channel_->getProtocolId();
  callbackContext.ctx = std::move(ctx);

  if (callbackContext.ctx) {
    callbackContext.ctx->preWrite();
  }

  SerializedMessage smsg;
  smsg.protocolType =
      apache::thrift::protocol::PROTOCOL_TYPES(getChannelProtocolId());
  smsg.buffer = args.get();
  smsg.methodName = functionName;
  if (callbackContext.ctx) {
    callbackContext.ctx->onWriteData(smsg);
    callbackContext.ctx->postWrite(
        args->computeChainDataLength()); // TODO get data length from python
                                         // serialize call
  }

  SerializedRequest serializedRequest(std::move(args));

  setInteraction(rpcOptions);
  factoryClient_ = nullptr;

  // Send the request!
  switch (rpcKind) {
    case RpcKind::SINK:
      channel_->sendRequestAsync<RpcKind::SINK>(
          std::move(rpcOptions),
          apache::thrift::MethodMetadata(std::move(metadata)),
          std::move(serializedRequest),
          std::move(header),
          createSinkClientCallback(toRequestClientCallbackPtr(
              std::move(callback), std::move(callbackContext))));
      break;
    case RpcKind::SINGLE_REQUEST_NO_RESPONSE:
      callbackContext.oneWay = true;
      channel_->sendRequestAsync<RpcKind::SINGLE_REQUEST_NO_RESPONSE>(
          std::move(rpcOptions),
          apache::thrift::MethodMetadata(std::move(metadata)),
          std::move(serializedRequest),
          std::move(header),
          toRequestClientCallbackPtr(
              std::move(callback), std::move(callbackContext)));
      break;
    case RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE:
      channel_->sendRequestAsync<RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE>(
          std::move(rpcOptions),
          apache::thrift::MethodMetadata(std::move(metadata)),
          std::move(serializedRequest),
          std::move(header),
          toRequestClientCallbackPtr(
              std::move(callback), std::move(callbackContext)));
      break;
    case RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE:
      BufferOptions bufferOptions = rpcOptions.getBufferOptions();
      channel_->sendRequestAsync<RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE>(
          std::move(rpcOptions),
          apache::thrift::MethodMetadata(std::move(metadata)),
          std::move(serializedRequest),
          std::move(header),
          createStreamClientCallback(
              toRequestClientCallbackPtr(
                  std::move(callback), std::move(callbackContext)),
              bufferOptions));
      break;
  }
}

void OmniClient::setInteraction(apache::thrift::RpcOptions& rpcOptions) {
  auto* client = factoryClient_.load();
  if (client != nullptr) {
    client->setInteraction(rpcOptions);
  }
}

OmniInteractionClient::OmniInteractionClient(
    std::shared_ptr<apache::thrift::RequestChannel> channel,
    const std::string& methodName)
    : OmniClient(channel), methodName_(methodName) {
  DCHECK(
      !channel_->getEventBase() ||
      channel_->getEventBase()->isInEventBaseThread());
  this->interactionId_ = channel_->createInteraction(methodName_);
}

OmniInteractionClient::~OmniInteractionClient() {
  terminate();
}

OmniInteractionClient& OmniInteractionClient::operator=(
    OmniInteractionClient&& other) {
  if (this != &other) {
    terminate();
  }
  channel_ = std::move(other.channel_);
  interactionId_ = std::move(other.interactionId_);
  return *this;
}

void OmniInteractionClient::terminate() {
  if (!channel_ || !interactionId_) {
    return;
  }
  auto* eb = channel_->getEventBase();
  if (eb != nullptr) {
    folly::getKeepAliveToken(eb).add(
        [channel = channel_, id = std::move(interactionId_)](auto&&) mutable {
          channel->terminateInteraction(std::move(id));
        });
  } else {
    channel_->terminateInteraction(std::move(interactionId_));
  }
}

void OmniInteractionClient::setInteraction(
    apache::thrift::RpcOptions& rpcOptions) {
  DCHECK(interactionId_);
  DCHECK(rpcOptions.getInteractionId() == 0);
  rpcOptions.setInteractionId(interactionId_);
}

folly::Future<std::unique_ptr<OmniInteractionClient>>
createOmniInteractionClient(
    std::shared_ptr<apache::thrift::RequestChannel> channel,
    const std::string& methodName) {
  auto* eventBase = channel->getEventBase();
  return folly::via(
      eventBase,
      [copiedMethod = methodName, channel = std::move(channel)]()
          -> std::unique_ptr<OmniInteractionClient> {
        return std::make_unique<OmniInteractionClient>(channel, copiedMethod);
      });
}

} // namespace client
} // namespace python
} // namespace thrift
