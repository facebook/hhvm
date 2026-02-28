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

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/ClientBiDiBridge.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataPlugins.h>

THRIFT_FLAG_DEFINE_int64(thrift_client_checksum_sampling_rate, 0);

namespace apache::thrift {

RequestChannel::RequestChannel() {
  setChecksumSamplingRate(THRIFT_FLAG(thrift_client_checksum_sampling_rate));
}

void RequestChannel::sendRequestResponse(
    const RpcOptions& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr clientCallback,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendRequestResponse(
      folly::copy(rpcOptions),
      std::move(metadata),
      std::move(request),
      std::move(header),
      std::move(clientCallback),
      std::move(frameworkMetadata));
}

void RequestChannel::sendRequestNoResponse(
    const RpcOptions& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr clientCallback,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendRequestNoResponse(
      folly::copy(rpcOptions),
      std::move(metadata),
      std::move(request),
      std::move(header),
      std::move(clientCallback),
      std::move(frameworkMetadata));
}

void RequestChannel::sendRequestStream(
    const RpcOptions& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    StreamClientCallback* clientCallback,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendRequestStream(
      folly::copy(rpcOptions),
      std::move(metadata),
      std::move(request),
      std::move(header),
      clientCallback,
      std::move(frameworkMetadata));
}

void RequestChannel::sendRequestSink(
    const RpcOptions& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    SinkClientCallback* clientCallback,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendRequestSink(
      folly::copy(rpcOptions),
      std::move(metadata),
      std::move(request),
      std::move(header),
      clientCallback,
      std::move(frameworkMetadata));
}

void RequestChannel::sendRequestBiDi(
    const RpcOptions& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    BiDiClientCallback* clientCallback,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendRequestBiDi(
      folly::copy(rpcOptions),
      std::move(metadata),
      std::move(request),
      std::move(header),
      clientCallback,
      std::move(frameworkMetadata));
}

void RequestChannel::sendRequestResponse(
    RpcOptions&& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr clientCallback,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendRequestResponse(
      rpcOptions,
      std::move(metadata),
      std::move(request),
      std::move(header),
      std::move(clientCallback),
      std::move(frameworkMetadata));
}

void RequestChannel::sendRequestNoResponse(
    RpcOptions&& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    RequestClientCallback::Ptr clientCallback,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendRequestNoResponse(
      rpcOptions,
      std::move(metadata),
      std::move(request),
      std::move(header),
      std::move(clientCallback),
      std::move(frameworkMetadata));
}

void RequestChannel::sendRequestStream(
    RpcOptions&& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    StreamClientCallback* clientCallback,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendRequestStream(
      rpcOptions,
      std::move(metadata),
      std::move(request),
      std::move(header),
      clientCallback,
      std::move(frameworkMetadata));
}

void RequestChannel::sendRequestSink(
    RpcOptions&& rpcOptions,
    MethodMetadata&& metadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    SinkClientCallback* clientCallback,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendRequestSink(
      rpcOptions,
      std::move(metadata),
      std::move(request),
      std::move(header),
      clientCallback,
      std::move(frameworkMetadata));
}

void RequestChannel::sendRequestBiDi(
    RpcOptions&&,
    MethodMetadata&&,
    SerializedRequest&&,
    std::shared_ptr<transport::THeader>,
    BiDiClientCallback*,
    std::unique_ptr<folly::IOBuf>) {
  folly::terminate_with<std::runtime_error>(
      "This channel doesn't support bidirectional streams");
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
    tHeader->fds = std::move(firstResponse.fds.dcheckReceivedOrEmpty());
    detail::fillTHeaderFromResponseRpcMetadata(
        firstResponse.metadata, *tHeader);
    requestCallback_.release()->onResponse(
        ClientReceiveState::create(
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

BiDiClientCallback* createBiDiClientCallback(
    RequestClientCallback::Ptr requestCallback) {
  DCHECK(requestCallback->isInlineSafe())
      << "Bidirectional streaming methods do not support the callback client "
         "method flavor. "
         "Use co_, sync_, or semifuture_ instead.";

  return apache::thrift::detail::ClientBiDiBridge::create(
      new RequestClientCallbackWrapper<apache::thrift::ClientBridgePtrPair>(
          std::move(requestCallback)));
}

template class ClientBatonCallback<true, true>;
template class ClientBatonCallback<true, false>;
template class ClientBatonCallback<false, true>;
template class ClientBatonCallback<false, false>;

} // namespace apache::thrift
