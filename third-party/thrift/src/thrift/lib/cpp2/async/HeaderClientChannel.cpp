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

#include <folly/io/async/DelayedDestruction.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>

#include <chrono>
#include <utility>

#include <folly/io/Cursor.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/gen/client_cpp.h>
#include <thrift/lib/thrift/gen-cpp2/RocketUpgradeAsyncClient.h>

THRIFT_FLAG_DEFINE_int64(raw_client_rocket_upgrade_timeout_ms, 2000);
THRIFT_FLAG_DEFINE_bool(client_header_coerce_framed_to_header, true);
THRIFT_FLAG_DEFINE_bool(client_header_coerce_unframed_to_header, true);

using std::make_unique;
using std::unique_ptr;
using namespace apache::thrift::transport;
using folly::EventBase;

namespace apache::thrift {
namespace {
class ReleasableDestructor : public folly::DelayedDestruction::Destructor {
 public:
  void operator()(folly::DelayedDestruction* dd) const {
    if (!released_) {
      dd->destroy();
    }
  }

  /**
   * Release the object managed by smart pointers. This is used when the
   * object ownership is transferred to another smart pointer or manually
   * managed by the caller. The original object must be properly deleted at
   * the end of its life cycle to avoid resource leaks.
   */
  void release() { released_ = true; }

 private:
  bool released_{false};
};

std::unique_ptr<folly::AsyncTransport, ReleasableDestructor> toReleasable(
    folly::AsyncTransport::UniquePtr transport) {
  return std::unique_ptr<folly::AsyncTransport, ReleasableDestructor>(
      transport.release());
}

HeaderClientChannel::Options& processOptions(
    HeaderClientChannel::Options& options) {
  if (THRIFT_FLAG(client_header_coerce_framed_to_header) &&
      (options.clientType == THRIFT_FRAMED_DEPRECATED ||
       options.clientType == THRIFT_FRAMED_COMPACT)) {
    options.protocolId = options.clientType == THRIFT_FRAMED_COMPACT
        ? T_COMPACT_PROTOCOL
        : T_BINARY_PROTOCOL;
    options.clientType = THRIFT_HEADER_CLIENT_TYPE;
  }
  if (THRIFT_FLAG(client_header_coerce_unframed_to_header) &&
      (options.clientType == THRIFT_UNFRAMED_DEPRECATED ||
       options.clientType == THRIFT_UNFRAMED_COMPACT_DEPRECATED)) {
    options.protocolId =
        options.clientType == THRIFT_UNFRAMED_COMPACT_DEPRECATED
        ? T_COMPACT_PROTOCOL
        : T_BINARY_PROTOCOL;
    options.clientType = THRIFT_HEADER_CLIENT_TYPE;
  }
  return options;
}
} // namespace

template class ChannelCallbacks::TwowayCallback<HeaderClientChannel>;

HeaderClientChannel::HeaderClientChannel(
    folly::AsyncTransport::UniquePtr transport, Options options)
    : HeaderClientChannel(
          std::shared_ptr<Cpp2Channel>(Cpp2Channel::newChannel(
              toReleasable(std::move(transport)),
              make_unique<ClientFramingHandler>(*this))),
          std::move(options)) {}

HeaderClientChannel::HeaderClientChannel(
    std::shared_ptr<Cpp2Channel> cpp2Channel, Options options)
    : clientType_(processOptions(options).clientType),
      sendSeqId_(0),
      closeCallback_(nullptr),
      timeout_(0),
      cpp2Channel_(cpp2Channel),
      protocolId_(options.protocolId),
      agentName_(options.agentName) {
  checkSupportedClient(clientType_);
  apache::thrift::detail::hookForClientTransport(getTransport());
  if (options.httpClientOptions) {
    updateHttpClientConfig(
        options.httpClientOptions->host, options.httpClientOptions->uri);
  }
}

HeaderClientChannel::Ptr HeaderClientChannel::newChannel(
    folly::AsyncTransport::UniquePtr transport, Options options) {
  if (options.clientType == THRIFT_HTTP_CLIENT_TYPE) {
    return newChannel(
        WithoutRocketUpgrade(), std::move(transport), std::move(options));
  }
  auto rocketUpgradeSetupMetadata =
      std::move(options.rocketUpgradeSetupMetadata);
  auto headerChannel = newChannel(
      WithoutRocketUpgrade(), std::move(transport), std::move(options));
  return Ptr(new RocketUpgradeChannel(
      std::move(headerChannel), std::move(rocketUpgradeSetupMetadata)));
}

HeaderClientChannel::Ptr HeaderClientChannel::newChannel(
    WithRocketUpgrade,
    folly::AsyncTransport::UniquePtr transport,
    Options options) {
  DCHECK(options.clientType != THRIFT_HTTP_CLIENT_TYPE);
  auto rocketUpgradeSetupMetadata =
      std::move(options.rocketUpgradeSetupMetadata);
  auto headerChannel = newChannel(
      WithoutRocketUpgrade(), std::move(transport), std::move(options));
  return Ptr(new RocketUpgradeChannel(
      std::move(headerChannel), std::move(rocketUpgradeSetupMetadata)));
}

void HeaderClientChannel::updateHttpClientConfig(
    const std::string& host, const std::string& uri) {
  DCHECK(clientType_ == THRIFT_HTTP_CLIENT_TYPE);
  httpClientParser_ = std::make_shared<util::THttpClientParser>(host, uri);
}

void HeaderClientChannel::setTimeout(uint32_t ms) {
  getTransport()->setSendTimeout(ms);
  timeout_ = ms;
}

void HeaderClientChannel::closeNow() {
  cpp2Channel_->closeNow();
}

void HeaderClientChannel::destroy() {
  closeNow();
  folly::DelayedDestruction::destroy();
}

bool HeaderClientChannel::good() {
  auto transport = getTransport();
  return transport && transport->good();
}

void HeaderClientChannel::attachEventBase(EventBase* eventBase) {
  cpp2Channel_->attachEventBase(eventBase);
}

void HeaderClientChannel::detachEventBase() {
  cpp2Channel_->detachEventBase();
}

bool HeaderClientChannel::isDetachable() {
  return getTransport()->isDetachable() && recvCallbacks_.empty();
}

bool HeaderClientChannel::clientSupportHeader() {
  return getClientType() == THRIFT_HEADER_CLIENT_TYPE ||
      getClientType() == THRIFT_HTTP_CLIENT_TYPE;
}

// Client Interface
void HeaderClientChannel::sendRequestNoResponse(
    const RpcOptions& rpcOptions,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& serializedRequest,
    std::shared_ptr<THeader> header,
    RequestClientCallback::Ptr cb,
    std::unique_ptr<folly::IOBuf> /*unsupported - header has no RpcMetadata*/) {
  preprocessHeader(header.get());

  auto buf = LegacySerializedRequest(
                 header->getProtocolId(),
                 methodMetadata.name_view(),
                 std::move(serializedRequest))
                 .buffer;

  setRequestHeaderOptions(header.get(), buf->computeChainDataLength());
  addRpcOptionHeaders(header.get(), rpcOptions);
  attachMetadataOnce(header.get());

  DestructorGuard dg(this);

  // Both cb and buf are allowed to be null.
  uint32_t oldSeqId = sendSeqId_;
  sendSeqId_ = ResponseChannel::ONEWAY_REQUEST_ID;

  if (cb) {
    sendMessage(
        new OnewayCallback(std::move(cb)), std::move(buf), header.get());
  } else {
    sendMessage(nullptr, std::move(buf), header.get());
  }
  sendSeqId_ = oldSeqId;
}

void HeaderClientChannel::setCloseCallback(CloseCallback* cb) {
  closeCallback_ = cb;
  setBaseReceivedCallback();
}

void HeaderClientChannel::setRequestHeaderOptions(
    THeader* header, ssize_t payloadSize) {
  header->setFlags(HEADER_FLAG_SUPPORT_OUT_OF_ORDER);
  header->setClientType(getClientType());
  header->forceClientType(true);
  if (auto compressionConfig = header->getDesiredCompressionConfig()) {
    if (auto codecRef = compressionConfig->codecConfig()) {
      if (payloadSize > compressionConfig->compressionSizeLimit().value_or(0)) {
        switch (codecRef->getType()) {
          case CodecConfig::Type::zlibConfig:
            header->setTransform(THeader::ZLIB_TRANSFORM);
            break;
          case CodecConfig::Type::zstdConfig:
            header->setTransform(THeader::ZSTD_TRANSFORM);
            break;
          default:
            break;
        }
      }
    }
  }
  if (getClientType() == THRIFT_HTTP_CLIENT_TYPE) {
    header->setHttpClientParser(httpClientParser_);
  }
}

void HeaderClientChannel::attachMetadataOnce(THeader* header) {
  if (std::exchange(firstRequest_, false)) {
    ClientMetadata md;
    if (const auto& hostMetadata = ClientChannel::getHostMetadata()) {
      md.hostname().from_optional(hostMetadata->hostname);
      md.otherMetadata().from_optional(hostMetadata->otherMetadata);
    }
    if (!agentName_.empty()) {
      md.agent() = std::move(agentName_);
    }
    header->setClientMetadata(md);
  }
}

uint16_t HeaderClientChannel::getProtocolId() {
  if (getClientType() == THRIFT_HEADER_CLIENT_TYPE ||
      getClientType() == THRIFT_HTTP_CLIENT_TYPE) {
    return protocolId_;
  } else if (getClientType() == THRIFT_FRAMED_COMPACT) {
    return T_COMPACT_PROTOCOL;
  } else {
    return T_BINARY_PROTOCOL; // Assume other transports use TBinary
  }
}

void HeaderClientChannel::sendRequestResponse(
    const RpcOptions& rpcOptions,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& serializedRequest,
    std::shared_ptr<THeader> header,
    RequestClientCallback::Ptr cb,
    std::unique_ptr<folly::IOBuf> /*unsupported - header has no RpcMetadata*/) {
  preprocessHeader(header.get());

  auto buf = LegacySerializedRequest(
                 header->getProtocolId(),
                 methodMetadata.name_view(),
                 std::move(serializedRequest))
                 .buffer;

  // cb is not allowed to be null.
  DCHECK(cb);

  DestructorGuard dg(this);

  // Oneway requests use a special sequence id.
  // Make sure this non-oneway request doesn't use
  // the oneway request ID.
  if (++sendSeqId_ == ResponseChannel::ONEWAY_REQUEST_ID) {
    ++sendSeqId_;
  }

  std::chrono::milliseconds timeout(timeout_);
  if (rpcOptions.getTimeout() > std::chrono::milliseconds(0)) {
    timeout = rpcOptions.getTimeout();
  }

  auto twcb = new TwowayCallback<HeaderClientChannel>(
      this, sendSeqId_, std::move(cb), &getEventBase()->timer(), timeout);

  setRequestHeaderOptions(header.get(), buf->computeChainDataLength());
  addRpcOptionHeaders(header.get(), rpcOptions);
  attachMetadataOnce(header.get());

  if (getClientType() != THRIFT_HEADER_CLIENT_TYPE) {
    recvCallbackOrder_.push_back(sendSeqId_);
  }
  recvCallbacks_[sendSeqId_] = twcb;
  try {
    setBaseReceivedCallback(); // Cpp2Channel->setReceiveCallback can throw
  } catch (const TTransportException&) {
    twcb->messageSendError(
        folly::exception_wrapper(folly::current_exception()));
    return;
  }

  sendMessage(twcb, std::move(buf), header.get());
}

// Header framing
std::unique_ptr<folly::IOBuf>
HeaderClientChannel::ClientFramingHandler::addFrame(
    unique_ptr<IOBuf> buf, THeader* header) {
  header->setSequenceNumber(channel_.sendSeqId_);
  return header->addHeader(std::move(buf));
}

std::tuple<std::unique_ptr<IOBuf>, size_t, std::unique_ptr<THeader>>
HeaderClientChannel::ClientFramingHandler::removeFrame(IOBufQueue* q) {
  std::unique_ptr<THeader> header(new THeader(THeader::ALLOW_BIG_FRAMES));
  if (!q || !q->front() || q->front()->empty()) {
    return make_tuple(std::unique_ptr<IOBuf>(), 0, nullptr);
  }

  size_t remaining = 0;
  std::unique_ptr<folly::IOBuf> buf =
      header->removeHeader(q, remaining, channel_.persistentReadHeaders_);
  if (!buf) {
    return make_tuple(std::unique_ptr<folly::IOBuf>(), remaining, nullptr);
  }
  HeaderChannelTrait::checkSupportedClient(header->getClientType());
  return make_tuple(std::move(buf), 0, std::move(header));
}

// Interface from MessageChannel::RecvCallback
void HeaderClientChannel::messageReceived(
    unique_ptr<IOBuf>&& buf, unique_ptr<THeader>&& header) {
  DestructorGuard dg(this);

  if (!buf) {
    return;
  }

  uint32_t recvSeqId;

  if (header->getClientType() != THRIFT_HEADER_CLIENT_TYPE) {
    if (header->getClientType() == THRIFT_HTTP_CLIENT_TYPE &&
        buf->computeChainDataLength() == 0) {
      // HTTP/1.x Servers must send a response, even for oneway requests.
      // Ignore these responses.
      return;
    }
    // Non-header clients will always be in order.
    // Note that for non-header clients, getSequenceNumber()
    // will return garbage.
    recvSeqId = recvCallbackOrder_.front();
    recvCallbackOrder_.pop_front();
  } else {
    // The header contains the seq-id.  May be out of order.
    recvSeqId = header->getSequenceNumber();
  }

  auto cb = recvCallbacks_.find(recvSeqId);

  // TODO: On some errors, some servers will return 0 for seqid.
  // Could possibly try and deserialize the buf and throw a
  // TApplicationException.
  // BUT, we don't even know for sure what protocol to deserialize with.
  if (cb == recvCallbacks_.end()) {
    VLOG(5) << "Could not find message id in recvCallbacks "
            << "(timed out, possibly server is just now sending response?)";
    return;
  }

  auto f(cb->second);

  recvCallbacks_.erase(recvSeqId);
  // we are the last callback?
  setBaseReceivedCallback();
  f->replyReceived(std::move(buf), std::move(header));
}

void HeaderClientChannel::messageChannelEOF() {
  DestructorGuard dg(this);
  messageReceiveErrorWrapped(
      folly::make_exception_wrapper<TTransportException>(
          TTransportException::TTransportExceptionType::END_OF_FILE,
          "Channel got EOF. Check for server hitting connection limit, "
          "connection age timeout, server connection idle timeout, and server crashes."));
  if (closeCallback_) {
    closeCallback_->channelClosed();
    closeCallback_ = nullptr;
  }
  setBaseReceivedCallback();
}

void HeaderClientChannel::messageReceiveErrorWrapped(
    folly::exception_wrapper&& ex) {
  DestructorGuard dg(this);

  while (!recvCallbacks_.empty()) {
    auto cb = recvCallbacks_.begin()->second;
    recvCallbacks_.erase(recvCallbacks_.begin());
    DestructorGuard dgcb(cb);
    cb->requestError(ex);
  }

  setBaseReceivedCallback();
}

void HeaderClientChannel::eraseCallback(
    uint32_t seqId, TwowayCallback<HeaderClientChannel>* cb) {
  CHECK(getEventBase()->isInEventBaseThread());
  auto it = recvCallbacks_.find(seqId);
  CHECK(it != recvCallbacks_.end());
  CHECK(it->second == cb);
  recvCallbacks_.erase(it);

  setBaseReceivedCallback(); // was this the last callback?
}

void HeaderClientChannel::setBaseReceivedCallback() {
  if (recvCallbacks_.size() != 0 || closeCallback_) {
    cpp2Channel_->setReceiveCallback(this);
  } else {
    cpp2Channel_->setReceiveCallback(nullptr);
  }
}

folly::AsyncTransport::UniquePtr HeaderClientChannel::stealTransport() {
  auto transportShared = cpp2Channel_->getTransportShared();
  cpp2Channel_->setTransport(nullptr);
  cpp2Channel_->closeNow();
  assert(transportShared.use_count() == 1);
  auto deleter = std::get_deleter<ReleasableDestructor>(transportShared);
  deleter->release();
  return folly::AsyncTransport::UniquePtr(transportShared.get());
}

class HeaderClientChannel::RocketUpgradeChannel::RocketUpgradeCallback
    : public apache::thrift::RequestCallback {
 public:
  explicit RocketUpgradeCallback(RocketUpgradeChannel* upgradeChannel)
      : upgradeChannel_(upgradeChannel) {}

  void requestSent() override {}

  void replyReceived(apache::thrift::ClientReceiveState&& state) override {
    if (auto ew =
            RocketUpgradeAsyncClient::recv_wrapped_upgradeToRocket(state)) {
      upgradeChannel_->upgradeComplete(std::move(ew));
      return;
    }
    upgradeChannel_->getEventBase()->runInEventBaseThread(
        [dg = std::move(upgradeChannelDestructorGuard_),
         upgradeChannel = upgradeChannel_]() mutable {
          upgradeChannel->upgradeComplete({});
        });
  }

  void requestError(apache::thrift::ClientReceiveState&& state) override {
    upgradeChannel_->upgradeComplete(std::move(state.exception()));
  }

  bool isInlineSafe() const override { return true; }

 private:
  RocketUpgradeChannel* upgradeChannel_;
  folly::DelayedDestruction::DestructorGuard upgradeChannelDestructorGuard_{
      upgradeChannel_};
};

HeaderClientChannel::RocketUpgradeChannel::RocketUpgradeChannel(
    HeaderClientChannel::LegacyPtr headerChannel,
    std::unique_ptr<RequestSetupMetadata> rocketUpgradeSetupMetadata)
    : headerChannel_(std::move(headerChannel)),
      rocketUpgradeSetupMetadata_(std::move(rocketUpgradeSetupMetadata)),
      state_(State::INIT) {}

HeaderClientChannel::RocketUpgradeChannel::~RocketUpgradeChannel() {
  if (rocketChannel_) {
    rocketChannel_->unsetOnDetachable();
  }
}

void HeaderClientChannel::RocketUpgradeChannel::sendRequestResponse(
    const RpcOptions& rpcOptions,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& serializedRequest,
    std::shared_ptr<apache::thrift::transport::THeader> header,
    RequestClientCallback::Ptr cb,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  preprocessHeader(header.get());

  initUpgradeIfNeeded(rpcOptions);
  if (state_ == State::UPGRADE_IN_PROGRESS) {
    bufferedRequests_.emplace(
        rpcOptions,
        std::move(methodMetadata),
        std::move(serializedRequest),
        std::move(header),
        std::move(cb),
        std::move(frameworkMetadata),
        false /* oneWay */);
    return;
  }

  DCHECK(state_ == State::DONE);

  getImpl().sendRequestResponse(
      rpcOptions,
      std::move(methodMetadata),
      std::move(serializedRequest),
      std::move(header),
      std::move(cb),
      std::move(frameworkMetadata));
}

void HeaderClientChannel::RocketUpgradeChannel::sendRequestNoResponse(
    const RpcOptions& rpcOptions,
    apache::thrift::MethodMetadata&& methodMetadata,
    SerializedRequest&& serializedRequest,
    std::shared_ptr<apache::thrift::transport::THeader> header,
    RequestClientCallback::Ptr cb,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  preprocessHeader(header.get());

  initUpgradeIfNeeded(rpcOptions);
  if (state_ == State::UPGRADE_IN_PROGRESS) {
    bufferedRequests_.emplace(
        rpcOptions,
        std::move(methodMetadata),
        std::move(serializedRequest),
        std::move(header),
        std::move(cb),
        std::move(frameworkMetadata),
        true /* oneWay */);
    return;
  }

  DCHECK(state_ == State::DONE);

  getImpl().sendRequestNoResponse(
      rpcOptions,
      std::move(methodMetadata),
      std::move(serializedRequest),
      std::move(header),
      std::move(cb),
      std::move(frameworkMetadata));
}

void HeaderClientChannel::RocketUpgradeChannel::setCloseCallback(
    CloseCallback* closeCallback) {
  getImpl().setCloseCallback(closeCallback);
}

folly::EventBase* HeaderClientChannel::RocketUpgradeChannel::getEventBase()
    const {
  return evb_;
}

uint16_t HeaderClientChannel::RocketUpgradeChannel::getProtocolId() {
  return protocolId_;
}

folly::AsyncTransport*
HeaderClientChannel::RocketUpgradeChannel::getTransport() {
  return getImpl().getTransport();
}

bool HeaderClientChannel::RocketUpgradeChannel::good() {
  return getImpl().good();
}

ClientChannel::SaturationStatus
HeaderClientChannel::RocketUpgradeChannel::getSaturationStatus() {
  return getImpl().getSaturationStatus();
}

void HeaderClientChannel::RocketUpgradeChannel::attachEventBase(
    folly::EventBase* evb) {
  getImpl().attachEventBase(evb);
  evb_ = evb;
}

void HeaderClientChannel::RocketUpgradeChannel::detachEventBase() {
  getImpl().detachEventBase();
  evb_ = nullptr;
}
bool HeaderClientChannel::RocketUpgradeChannel::isDetachable() {
  return state_ != State::UPGRADE_IN_PROGRESS && getImpl().isDetachable();
}

uint32_t HeaderClientChannel::RocketUpgradeChannel::getTimeout() {
  return getImpl().getTimeout();
}

void HeaderClientChannel::RocketUpgradeChannel::setTimeout(uint32_t ms) {
  getImpl().setTimeout(ms);
}

void HeaderClientChannel::RocketUpgradeChannel::closeNow() {
  if (state_ == State::UPGRADE_IN_PROGRESS) {
    auto ex = TTransportException("Channel closed");
    for (; !bufferedRequests_.empty(); bufferedRequests_.pop()) {
      std::move(bufferedRequests_.front()).fail(ex);
    }
  }
  state_ = State::DONE;
  getImpl().closeNow();
}

CLIENT_TYPE HeaderClientChannel::RocketUpgradeChannel::getClientType() {
  return getImpl().getClientType();
}

void HeaderClientChannel::RocketUpgradeChannel::initUpgradeIfNeeded(
    const RpcOptions& firstRequestRpcOptions) {
  if (state_ != State::INIT) {
    return;
  }

  state_ = State::UPGRADE_IN_PROGRESS;

  apache::thrift::RpcOptions rpcOptions;
  rpcOptions.setTimeout(
      folly::constexpr_max(
          std::chrono::milliseconds(
              THRIFT_FLAG(raw_client_rocket_upgrade_timeout_ms)),
          firstRequestRpcOptions.getTimeout(),
          std::chrono::milliseconds(headerChannel_->timeout_)));

  auto callback = std::make_unique<RocketUpgradeCallback>(this);
  auto client = std::make_unique<apache::thrift::RocketUpgradeAsyncClient>(
      std::shared_ptr<HeaderClientChannel>(
          headerChannel_.get(), [](HeaderClientChannel*) {}));

  client->clearEventHandlers();

  client->upgradeToRocket(rpcOptions, std::move(callback));
}

void HeaderClientChannel::RocketUpgradeChannel::upgradeComplete(
    folly::exception_wrapper ew) {
  if (state_ == State::DONE) {
    return;
  }

  DCHECK(state_ == State::UPGRADE_IN_PROGRESS);

  if (ew) {
    VLOG(4) << "Unable to upgrade transport from header to rocket! "
            << "Exception : " << folly::exceptionStr(ew);
    ew.with_exception<TTransportException>([&](const auto& tex) {
      // In case we hit a transport error (e.g. a timeout), we don't know if the
      // server is using header or rocket, so we have to close the connection.
      for (; !bufferedRequests_.empty(); bufferedRequests_.pop()) {
        std::move(bufferedRequests_.front()).fail(tex);
      }
      headerChannel_->closeNow();
    });
  } else {
    auto transport = headerChannel_->stealTransport();
    rocketChannel_ = rocketUpgradeSetupMetadata_
        ? RocketClientChannel::newChannelWithMetadata(
              std::move(transport), std::move(*rocketUpgradeSetupMetadata_))
        : RocketClientChannel::newChannel(std::move(transport));

    // Copy configuration to rocket channel
    if (headerChannel_->closeCallback_) {
      rocketChannel_->setCloseCallback(headerChannel_->closeCallback_);
    }
    rocketChannel_->setProtocolId(headerChannel_->getProtocolId());
    auto transportSendTimeout =
        rocketChannel_->getTransport()->getSendTimeout();
    rocketChannel_->setTimeout(headerChannel_->timeout_);
    rocketChannel_->getTransport()->setSendTimeout(transportSendTimeout);
    rocketChannel_->setOnDetachable([&] {
      if (isDetachable()) {
        notifyDetachable();
      }
    });

    headerChannel_.reset();
  }

  for (; !bufferedRequests_.empty(); bufferedRequests_.pop()) {
    std::move(bufferedRequests_.front()).send(getImpl());
  }

  state_ = State::DONE;
}

ClientChannel& HeaderClientChannel::RocketUpgradeChannel::getImpl() const {
  if (rocketChannel_) {
    return *rocketChannel_;
  }
  return *headerChannel_;
}

void HeaderClientChannel::RocketUpgradeChannel::BufferedRequest::send(
    ClientChannel& channel) && {
  if (oneWay_) {
    channel.sendRequestNoResponse(
        rpcOptions_,
        std::move(methodMetadata_),
        std::move(serializedRequest_),
        std::move(header_),
        std::move(callback_),
        std::move(frameworkMetadata_));
  } else {
    channel.sendRequestResponse(
        rpcOptions_,
        std::move(methodMetadata_),
        std::move(serializedRequest_),
        std::move(header_),
        std::move(callback_),
        std::move(frameworkMetadata_));
  }
}

void HeaderClientChannel::RocketUpgradeChannel::BufferedRequest::fail(
    folly::exception_wrapper ew) && {
  callback_.release()->onResponseError(std::move(ew));
}

} // namespace apache::thrift
