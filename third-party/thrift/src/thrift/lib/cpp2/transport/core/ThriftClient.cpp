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

#include <thrift/lib/cpp2/transport/core/ThriftClient.h>

#include <glog/logging.h>

#include <folly/ExceptionWrapper.h>
#include <folly/io/async/Request.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataPlugins.h>
#include <thrift/lib/cpp2/transport/core/ThriftChannelIf.h>
#include <thrift/lib/cpp2/transport/core/ThriftClientCallback.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache {
namespace thrift {

using apache::thrift::transport::THeader;
using apache::thrift::transport::TTransportException;
using folly::EventBase;

const std::chrono::milliseconds ThriftClient::kDefaultRpcTimeout =
    std::chrono::milliseconds(500);

ThriftClient::ThriftClient(
    std::shared_ptr<ClientConnectionIf> connection,
    folly::EventBase* callbackEvb)
    : connection_(connection), callbackEvb_(callbackEvb) {}

ThriftClient::ThriftClient(std::shared_ptr<ClientConnectionIf> connection)
    : ThriftClient(connection, connection->getEventBase()) {}

ThriftClient::~ThriftClient() {
  connection_->setCloseCallback(this, nullptr);
}

void ThriftClient::setProtocolId(uint16_t protocolId) {
  protocolId_ = protocolId;
}

void ThriftClient::setHTTPHost(const std::string& host) {
  httpHost_ = host;
}

void ThriftClient::setHTTPUrl(const std::string& url) {
  httpUrl_ = url;
}

void ThriftClient::sendRequestResponse(
    const RpcOptions& rpcOptions,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& serializedRequest,
    std::shared_ptr<THeader> header,
    RequestClientCallback::Ptr cb) {
  auto buf = LegacySerializedRequest(
                 header->getProtocolId(),
                 methodMetadata.name_view(),
                 std::move(serializedRequest))
                 .buffer;

  return sendRequestHelper(
      rpcOptions,
      RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      std::move(buf),
      std::move(header),
      std::move(cb));
}

void ThriftClient::sendRequestNoResponse(
    const RpcOptions& rpcOptions,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& serializedRequest,
    std::shared_ptr<THeader> header,
    RequestClientCallback::Ptr cb) {
  auto buf = LegacySerializedRequest(
                 header->getProtocolId(),
                 methodMetadata.name_view(),
                 std::move(serializedRequest))
                 .buffer;

  sendRequestHelper(
      rpcOptions,
      RpcKind::SINGLE_REQUEST_NO_RESPONSE,
      std::move(buf),
      std::move(header),
      std::move(cb));
}

std::unique_ptr<ThriftChannelIf::RequestMetadata>
ThriftClient::createRequestMetadata(
    const RpcOptions& rpcOptions,
    RpcKind kind,
    apache::thrift::ProtocolId protocolId,
    THeader* header) {
  preprocessHeader(header);
  auto requestMetadata = std::make_unique<ThriftChannelIf::RequestMetadata>();
  auto& metadata = requestMetadata->requestRpcMetadata;
  metadata.protocol_ref() = protocolId;
  metadata.kind_ref() = kind;
  if (!httpHost_.empty()) {
    requestMetadata->host = httpHost_;
  }
  if (!httpUrl_.empty()) {
    requestMetadata->url = httpUrl_;
  }
  if (rpcOptions.getTimeout() > std::chrono::milliseconds(0)) {
    metadata.clientTimeoutMs_ref() = rpcOptions.getTimeout().count();
  } else {
    metadata.clientTimeoutMs_ref() = connection_->getTimeout();
  }
  if (rpcOptions.getQueueTimeout() > std::chrono::milliseconds(0)) {
    metadata.queueTimeoutMs_ref() = rpcOptions.getQueueTimeout().count();
  }
  if (rpcOptions.getPriority() < concurrency::N_PRIORITIES) {
    metadata.priority_ref() =
        static_cast<RpcPriority>(rpcOptions.getPriority());
  }
  if (header->getCrc32c().has_value()) {
    metadata.crc32c_ref() = header->getCrc32c().value();
  }
  auto otherMetadata = metadata.otherMetadata_ref();
  otherMetadata = header->releaseWriteHeaders();
  auto clientId = header->clientId();
  auto tenantId = header->tenantId();
  auto serviceTraceMeta = header->serviceTraceMeta();
  if (clientId.has_value()) {
    (*otherMetadata)[transport::THeader::kClientId] = std::move(*clientId);
  }
  if (serviceTraceMeta.has_value()) {
    (*otherMetadata)[transport::THeader::kServiceTraceMeta] =
        std::move(*serviceTraceMeta);
  }
  if (tenantId.has_value()) {
    (*otherMetadata)[transport::THeader::kTenantId] = std::move(*tenantId);
  }
  auto* eh = header->getExtraWriteHeaders();
  if (eh) {
    // Extra write headers always take precedence over write headers (see
    // THeader.cpp). We must copy here since we don't own the extra write
    // headers.
    for (const auto& entry : *eh) {
      (*otherMetadata)[entry.first] = entry.second;
    }
  }
  if (otherMetadata->empty()) {
    otherMetadata.reset();
  }

  folly::dynamic logMessages = folly::dynamic::object();
  auto frameworkMetadata =
      detail::makeFrameworkMetadata(rpcOptions, logMessages);
  if (frameworkMetadata) {
    metadata.frameworkMetadata_ref() = std::move(frameworkMetadata);
  }

  return requestMetadata;
}

void ThriftClient::sendRequestHelper(
    const RpcOptions& rpcOptions,
    RpcKind kind,
    std::unique_ptr<IOBuf> buf,
    std::shared_ptr<THeader> header,
    RequestClientCallback::Ptr cb) noexcept {
  DestructorGuard dg(this);
  auto callbackEvb =
      cb->isInlineSafe() ? connection_->getEventBase() : callbackEvb_;
  auto metadata = createRequestMetadata(
      rpcOptions,
      kind,
      static_cast<apache::thrift::ProtocolId>(protocolId_),
      header.get());
  auto callback = std::make_unique<ThriftClientCallback>(
      callbackEvb,
      kind == RpcKind::SINGLE_REQUEST_NO_RESPONSE,
      std::move(cb),
      std::chrono::milliseconds(
          metadata->requestRpcMetadata.clientTimeoutMs_ref().value_or(0)));
  auto conn = connection_;
  connection_->getEventBase()->runInEventBaseThread([conn = std::move(conn),
                                                     metadata =
                                                         std::move(metadata),
                                                     buf = std::move(buf),
                                                     callback = std::move(
                                                         callback)]() mutable {
    getChannelAndSendThriftRequest(
        conn.get(), std::move(*metadata), std::move(buf), std::move(callback));
  });
}

void ThriftClient::getChannelAndSendThriftRequest(
    ClientConnectionIf* connection,
    ThriftChannelIf::RequestMetadata&& metadata,
    std::unique_ptr<IOBuf> payload,
    std::unique_ptr<ThriftClientCallback> callback) noexcept {
  DCHECK(connection->getEventBase()->isInEventBaseThread());
  try {
    auto channel = connection->getChannel();
    channel->sendThriftRequest(
        std::move(metadata), std::move(payload), std::move(callback));
  } catch (TTransportException& te) {
    // sendThriftRequest is noexcept, so callback will never be nullptr in here
    auto callbackEvb = callback->getEventBase();
    callbackEvb->runInEventBaseThread([callback = std::move(callback),
                                       te = std::move(te)]() mutable {
      callback->onError(
          folly::make_exception_wrapper<TTransportException>(std::move(te)));
    });
  }
}

EventBase* ThriftClient::getEventBase() const {
  return connection_->getEventBase();
}

uint16_t ThriftClient::getProtocolId() {
  return protocolId_;
}

void ThriftClient::setCloseCallback(CloseCallback* cb) {
  connection_->setCloseCallback(this, cb);
}

folly::AsyncTransport* ThriftClient::getTransport() {
  return connection_->getTransport();
}

bool ThriftClient::good() {
  return connection_->good();
}

ClientChannel::SaturationStatus ThriftClient::getSaturationStatus() {
  return connection_->getSaturationStatus();
}

void ThriftClient::attachEventBase(folly::EventBase* eventBase) {
  connection_->attachEventBase(eventBase);
}

void ThriftClient::detachEventBase() {
  connection_->detachEventBase();
}

bool ThriftClient::isDetachable() {
  return connection_->isDetachable();
}

uint32_t ThriftClient::getTimeout() {
  return connection_->getTimeout();
}

void ThriftClient::setTimeout(uint32_t ms) {
  connection_->setTimeout(ms);
}

void ThriftClient::closeNow() {
  connection_->closeNow();
}

CLIENT_TYPE ThriftClient::getClientType() {
  return connection_->getClientType();
}

} // namespace thrift
} // namespace apache
