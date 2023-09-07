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

#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataPlugins.h>

namespace apache {
namespace thrift {

void RequestChannel::sendRequestResponse(
    const RpcOptions& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr clientCallback) {
  sendRequestResponse(
      folly::copy(rpcOptions),
      std::move(metadata),
      std::move(request),
      std::move(header),
      std::move(clientCallback));
}

void RequestChannel::sendRequestNoResponse(
    const RpcOptions& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr clientCallback) {
  sendRequestNoResponse(
      folly::copy(rpcOptions),
      std::move(metadata),
      std::move(request),
      std::move(header),
      std::move(clientCallback));
}

void RequestChannel::sendRequestStream(
    const RpcOptions& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    StreamClientCallback* clientCallback) {
  sendRequestStream(
      folly::copy(rpcOptions),
      std::move(metadata),
      std::move(request),
      std::move(header),
      clientCallback);
}

void RequestChannel::sendRequestSink(
    const RpcOptions& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    SinkClientCallback* clientCallback) {
  sendRequestSink(
      folly::copy(rpcOptions),
      std::move(metadata),
      std::move(request),
      std::move(header),
      clientCallback);
}

void RequestChannel::sendRequestResponse(
    RpcOptions&& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr clientCallback) {
  sendRequestResponse(
      rpcOptions,
      std::move(metadata),
      std::move(request),
      std::move(header),
      std::move(clientCallback));
}

void RequestChannel::sendRequestNoResponse(
    RpcOptions&& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr clientCallback) {
  sendRequestNoResponse(
      rpcOptions,
      std::move(metadata),
      std::move(request),
      std::move(header),
      std::move(clientCallback));
}

void RequestChannel::sendRequestStream(
    RpcOptions&& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    StreamClientCallback* clientCallback) {
  sendRequestStream(
      rpcOptions,
      std::move(metadata),
      std::move(request),
      std::move(header),
      clientCallback);
}

void RequestChannel::sendRequestSink(
    RpcOptions&& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    SinkClientCallback* clientCallback) {
  sendRequestSink(
      rpcOptions,
      std::move(metadata),
      std::move(request),
      std::move(header),
      clientCallback);
}

void RequestChannel::terminateInteraction(InteractionId) {
  folly::terminate_with<std::runtime_error>(
      "This channel doesn't support interactions");
}
InteractionId RequestChannel::createInteraction(ManagedStringView&& name) {
  static std::atomic<int64_t> nextId{0};
  int64_t id = 1 + nextId.fetch_add(1, std::memory_order_relaxed);
  return registerInteraction(std::move(name), id);
}
InteractionId RequestChannel::registerInteraction(
    ManagedStringView&&, int64_t) {
  folly::terminate_with<std::runtime_error>(
      "This channel doesn't support interactions");
}
InteractionId RequestChannel::createInteractionId(int64_t id) {
  return InteractionId(id);
}
void RequestChannel::releaseInteractionId(InteractionId&& id) {
  id.release();
}

uint64_t RequestChannel::getChecksumSamplingRate() const {
  return checksumSamplingRate_;
}
void RequestChannel::setChecksumSamplingRate(uint64_t samplingRate) {
  checksumSamplingRate_ = samplingRate;
}

template <typename ClientBridgePtr>
class RequestClientCallbackWrapper
    : public FirstResponseClientCallback<ClientBridgePtr> {
 public:
  explicit RequestClientCallbackWrapper(
      RequestClientCallback::Ptr requestCallback)
      : requestCallback_(std::move(requestCallback)) {}
  RequestClientCallbackWrapper(
      RequestClientCallback::Ptr requestCallback,
      const BufferOptions& bufferOptions)
      : requestCallback_(std::move(requestCallback)),
        bufferOptions_(bufferOptions) {}

  void onFirstResponse(
      FirstResponsePayload&& firstResponse,
      ClientBridgePtr clientBridge) override {
    auto tHeader = std::make_unique<transport::THeader>();
    tHeader->setClientType(THRIFT_ROCKET_CLIENT_TYPE);
    if (kTempKillswitch__EnableFdPassing) {
      tHeader->fds = std::move(firstResponse.fds.dcheckReceivedOrEmpty());
    }
    if (auto tfmr = firstResponse.metadata.frameworkMetadata_ref()) {
      detail::ingestFrameworkMetadataFromResponse(std::move(*tfmr));
    }
    detail::fillTHeaderFromResponseRpcMetadata(
        firstResponse.metadata, *tHeader);
    requestCallback_.release()->onResponse(ClientReceiveState::create(
        std::move(firstResponse.payload),
        std::move(tHeader),
        std::move(clientBridge),
        bufferOptions_));
    delete this;
  }

  void onFirstResponseError(folly::exception_wrapper ew) override {
    requestCallback_.release()->onResponseError(std::move(ew));
    delete this;
  }

 private:
  RequestClientCallback::Ptr requestCallback_;
  BufferOptions bufferOptions_;
};

StreamClientCallback* createStreamClientCallback(
    RequestClientCallback::Ptr requestCallback,
    const BufferOptions& bufferOptions) {
  DCHECK(requestCallback->isInlineSafe())
      << "Streaming methods do not support the callback client method flavor. "
         "Use co_, sync_, or semifuture_ instead.";

  return apache::thrift::detail::ClientStreamBridge::create(
      new RequestClientCallbackWrapper<
          apache::thrift::detail::ClientStreamBridge::ClientPtr>(
          std::move(requestCallback), bufferOptions));
}

SinkClientCallback* createSinkClientCallback(
    RequestClientCallback::Ptr requestCallback) {
  DCHECK(requestCallback->isInlineSafe())
      << "Sink methods do not support the callback client method flavor. "
         "Use co_, sync_, or semifuture_ instead.";

  return apache::thrift::detail::ClientSinkBridge::create(
      new RequestClientCallbackWrapper<
          apache::thrift::detail::ClientSinkBridge::ClientPtr>(
          std::move(requestCallback)));
}

template class ClientBatonCallback<true, true>;
template class ClientBatonCallback<true, false>;
template class ClientBatonCallback<false, true>;
template class ClientBatonCallback<false, false>;

} // namespace thrift
} // namespace apache
