/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/network/ThriftTransport.h"

#include <folly/fibers/FiberManager.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>

#ifndef LIBMC_FBTRACE_DISABLE
#include <contextprop/cpp/serde/SerDeHelper.h>
#endif

#include "mcrouter/lib/fbi/cpp/LogFailure.h"
#include "mcrouter/lib/network/AsyncTlsToPlaintextSocket.h"
#include "mcrouter/lib/network/ConnectionOptions.h"
#include "mcrouter/lib/network/McFizzClient.h"
#include "mcrouter/lib/network/SecurityOptions.h"
#include "mcrouter/lib/network/SocketUtil.h"
#include "mcrouter/lib/network/ThreadLocalSSLContextProvider.h"

using folly::AsyncSocket;

namespace facebook {
namespace memcache {

ThriftTransportBase::ThriftTransportBase(
    folly::EventBase& eventBase,
    ConnectionOptions options)
    : eventBase_(eventBase), connectionOptions_(std::move(options)) {}

void ThriftTransportBase::closeNow() {
  resetClient();
}

void ThriftTransportBase::setConnectionStatusCallbacks(
    ConnectionStatusCallbacks callbacks) {
  connectionCallbacks_ = std::move(callbacks);

  if (connectionState_ == ConnectionState::Up && connectionCallbacks_.onUp) {
    // Connection retries not currently supported in thrift transport so pass 0
    connectionCallbacks_.onUp(*channel_->getTransport(), 0);
  }
}

void ThriftTransportBase::setRequestStatusCallbacks(
    RequestStatusCallbacks callbacks) {
  requestCallbacks_ = std::move(callbacks);
}

void ThriftTransportBase::setAuthorizationCallbacks(
    AuthorizationCallbacks callbacks) {
  authorizationCallbacks_ = std::move(callbacks);
}

void ThriftTransportBase::setThrottle(size_t maxInflight, size_t maxPending) {
  maxInflight_ = maxInflight;
  maxPending_ = maxPending;
}

Transport::RequestQueueStats ThriftTransportBase::getRequestQueueStats() const {
  return RequestQueueStats{0, 0};
}

void ThriftTransportBase::updateTimeoutsIfShorter(
    std::chrono::milliseconds /* connectTimeout */,
    std::chrono::milliseconds /* writeTimeout */) {}

const folly::AsyncTransportWrapper* ThriftTransportBase::getTransport() const {
  return nullptr;
}

double ThriftTransportBase::getRetransmitsPerKb() {
  return 0.0;
}

folly::AsyncTransportWrapper::UniquePtr
ThriftTransportBase::getConnectingSocket() {
  return folly::fibers::runInMainContext(
      [this]() -> folly::AsyncTransportWrapper::UniquePtr {
        auto expectedSocket = createAsyncSocket(eventBase_, connectionOptions_);
        if (expectedSocket.hasError()) {
          LOG_FAILURE(
              "ThriftTransport",
              failure::Category::kBadEnvironment,
              "{}",
              expectedSocket.error().what());
          return {};
        }
        auto socket = std::move(expectedSocket).value();

        auto sockAddressExpected = getSocketAddress(connectionOptions_);
        if (sockAddressExpected.hasError()) {
          const auto& ex = sockAddressExpected.error();
          LOG_FAILURE(
              "ThriftTransport",
              failure::Category::kBadEnvironment,
              "{}",
              ex.what());
          return {};
        }
        folly::SocketAddress address = std::move(sockAddressExpected).value();
        auto socketOptions = createSocketOptions(address, connectionOptions_);
        connectionState_ = ConnectionState::Connecting;

        const auto securityMech =
            connectionOptions_.accessPoint->getSecurityMech();
        if (securityMech == SecurityMech::TLS_TO_PLAINTEXT) {
          socket->setSendTimeout(connectionOptions_.writeTimeout.count());
          socket->getUnderlyingTransport<AsyncTlsToPlaintextSocket>()->connect(
              this,
              address,
              connectionOptions_.connectTimeout,
              std::move(socketOptions));
        } else if (securityMech == SecurityMech::TLS) {
          socket->setSendTimeout(connectionOptions_.writeTimeout.count());
          socket->getUnderlyingTransport<folly::AsyncSSLSocket>()->connect(
              this,
              address,
              connectionOptions_.connectTimeout.count(),
              socketOptions);
        } else if (securityMech == SecurityMech::TLS13_FIZZ) {
          auto fizzClient = socket->getUnderlyingTransport<McFizzClient>();
          fizzClient->setSendTimeout(connectionOptions_.writeTimeout.count());
          fizzClient->connect(
              this,
              address,
              connectionOptions_.connectTimeout.count(),
              socketOptions);
        } else {
          DCHECK(securityMech == SecurityMech::NONE);
          socket->setSendTimeout(connectionOptions_.writeTimeout.count());
          socket->getUnderlyingTransport<folly::AsyncSocket>()->connect(
              this,
              address,
              connectionOptions_.connectTimeout.count(),
              socketOptions);
        }
        return socket;
      });
}

apache::thrift::RocketClientChannel::Ptr ThriftTransportBase::createChannel() {
  // HHVM supports Debian 8 (EOL 2020-06-30), which includes OpenSSL 1.0.1;
  // Rocket/RSocket require ALPN, which requiers 1.0.2.
  //
  // For these platforms, build MCRouter client without a functional
  // Thrift transport, but continue to permit use as an async Memcache client
  // library for Hack
#ifndef MCROUTER_NOOP_THRIFT_CLIENT
  auto socket = getConnectingSocket();
  if (!socket) {
    return nullptr;
  }
  auto channel =
      apache::thrift::RocketClientChannel::newChannel(std::move(socket));
  channel->setCloseCallback(this);
  if (connectionOptions_.thriftCompression) {
    apache::thrift::CodecConfig codec;
    codec.zstdConfig_ref() = apache::thrift::ZstdCompressionCodecConfig();
    apache::thrift::CompressionConfig compressionConfig;
    if (connectionOptions_.thriftCompressionThreshold > 0) {
      compressionConfig.compressionSizeLimit() =
          connectionOptions_.thriftCompressionThreshold;
    }
    compressionConfig.codecConfig() = std::move(codec);
    channel->setDesiredCompressionConfig(std::move(compressionConfig));
  }

  return channel;
#else
  return nullptr;
#endif
}

void ThriftTransportBase::connectSuccess() noexcept {
  auto transport = channel_->getTransport();
  assert(
      transport != nullptr && connectionState_ == ConnectionState::Connecting);
  connectionState_ = ConnectionState::Up;
  if (isAsyncSSLSocketMech(connectionOptions_.accessPoint->getSecurityMech())) {
    if (authorizationCallbacks_.onAuthorize &&
        !authorizationCallbacks_.onAuthorize(
            *transport->getUnderlyingTransport<AsyncSocket>(),
            connectionOptions_)) {
      if (connectionOptions_.securityOpts.sslAuthorizationEnforce) {
        // Enforcement is enabled, close the connection.
        closeNow();
        return;
      }
    }
  }

  if (connectionCallbacks_.onUp) {
    // Connection retries not currently supported in thrift transport so pass 0
    connectionCallbacks_.onUp(*channel_->getTransport(), 0);
  }
  VLOG(5) << "[ThriftTransport] Connection successfully established!";
}

void ThriftTransportBase::connectErr(
    const folly::AsyncSocketException& ex) noexcept {
  assert(connectionState_ == ConnectionState::Connecting);

  connectionState_ = ConnectionState::Error;
  connectionTimedOut_ =
      (ex.getType() == folly::AsyncSocketException::TIMED_OUT);
  if (connectionCallbacks_.onDown) {
    ConnectionDownReason reason = ConnectionDownReason::CONNECT_ERROR;
    if (connectionTimedOut_) {
      reason = ConnectionDownReason::CONNECT_ERROR;
    }
    connectionCallbacks_.onDown(reason, 0);
  }
  VLOG(2) << "[ThriftTransport] Error connecting: " << ex.what();
}

void ThriftTransportBase::channelClosed() {
  VLOG(3) << "[ThriftTransport] Channel closed.";
  // If callbacks configured and connection up, defer reset
  // to the callback
  if (connectionCallbacks_.onDown && connectionState_ == ConnectionState::Up) {
    connectionState_ = ConnectionState::Down;
    connectionCallbacks_.onDown(ConnectionDownReason::ABORTED, 0);
  }
  resetClient();
}

#ifndef LIBMC_FBTRACE_DISABLE
void ThriftTransportUtil::traceRequest(
    const carbon::MessageCommon& request,
    apache::thrift::RpcOptions& rpcOptions) {
  if (FOLLY_UNLIKELY(!request.traceContext().empty())) {
    folly::fibers::runInMainContext(
        [&]() { traceRequestImpl(request, rpcOptions); });
  }
}

void ThriftTransportUtil::traceRequestImpl(
    const carbon::MessageCommon& request,
    apache::thrift::RpcOptions& rpcOptions) {
  auto artilleryTraceIDs =
      facebook::contextprop::SerDeHelper::getArtilleryTraceIDsFromLegacyHeader(
          request.traceContext());
  rpcOptions.setWriteHeader(
      facebook::contextprop::ContextpropConstants_constants::
          artillery_trace_ids_header_,
      facebook::contextprop::SerDeHelper::encodeAndSerialize<
          facebook::contextprop::ArtilleryTraceIDs>(artilleryTraceIDs));
}

void ThriftTransportUtil::traceResponseImpl(
    carbon::MessageCommon& response,
    const apache::thrift::transport::THeader::StringToStringMap&
        responseHeaders) {
  auto artilleryTraceIDs = contextprop::SerDeHelper::decodeAndDeserialize<
      facebook::contextprop::ArtilleryTraceIDs>(
      responseHeaders,
      facebook::contextprop::ContextpropConstants_constants::
          artillery_trace_ids_header_);
  if (artilleryTraceIDs) {
    response.setTraceContext(
        facebook::contextprop::SerDeHelper::convertToLegacyTraceContext(
            *artilleryTraceIDs));
  }
}
#endif

apache::thrift::RpcOptions ThriftTransportUtil::getRpcOptions(
    std::chrono::milliseconds timeout) {
  apache::thrift::RpcOptions rpcOptions;
  rpcOptions.setTimeout(timeout);
  rpcOptions.setClientOnlyTimeouts(true);
  return rpcOptions;
}

} // namespace memcache
} // namespace facebook
