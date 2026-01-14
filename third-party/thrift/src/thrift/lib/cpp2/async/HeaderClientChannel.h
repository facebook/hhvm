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

#ifndef THRIFT_ASYNC_THEADERCLIENTCHANNEL_H_
#define THRIFT_ASYNC_THEADERCLIENTCHANNEL_H_ 1
#include <deque>
#include <limits>
#include <memory>
#include <unordered_map>

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/Request.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp/util/THttpParser.h>
#include <thrift/lib/cpp2/async/ChannelCallbacks.h>
#include <thrift/lib/cpp2/async/ClientChannel.h>
#include <thrift/lib/cpp2/async/Cpp2Channel.h>
#include <thrift/lib/cpp2/async/HeaderChannel.h>
#include <thrift/lib/cpp2/async/HeaderChannelTrait.h>
#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>

namespace apache::thrift {

/**
 * HeaderClientChannel
 *
 * This is a channel implementation that reads and writes
 * messages encoded using THeaderProtocol.
 */
class HeaderClientChannel : public ClientChannel,
                            public HeaderChannelTrait,
                            public MessageChannel::RecvCallback,
                            public ChannelCallbacks,
                            virtual public folly::DelayedDestruction {
 protected:
  ~HeaderClientChannel() override {}

 public:
  struct Options {
    Options() {}

    protocol::PROTOCOL_TYPES protocolId{protocol::T_COMPACT_PROTOCOL};
    Options& setProtocolId(protocol::PROTOCOL_TYPES prot) & {
      protocolId = prot;
      return *this;
    }
    Options&& setProtocolId(protocol::PROTOCOL_TYPES prot) && {
      setProtocolId(prot);
      return std::move(*this);
    }
    CLIENT_TYPE clientType{THRIFT_HEADER_CLIENT_TYPE};
    Options& setClientType(CLIENT_TYPE ct) & {
      clientType = ct;
      return *this;
    }
    Options&& setClientType(CLIENT_TYPE ct) && {
      setClientType(ct);
      return std::move(*this);
    }
    struct HttpClientOptions {
      std::string host;
      std::string uri;
    };
    std::unique_ptr<HttpClientOptions> httpClientOptions;
    Options& useAsHttpClient(
        const std::string& host, const std::string& uri) & {
      clientType = THRIFT_HTTP_CLIENT_TYPE;
      httpClientOptions = std::make_unique<HttpClientOptions>();
      httpClientOptions->host = host;
      httpClientOptions->uri = uri;
      return *this;
    }
    Options&& useAsHttpClient(
        const std::string& host, const std::string& uri) && {
      useAsHttpClient(host, uri);
      return std::move(*this);
    }
    std::string agentName{"HeaderClientChannel.cpp"};
    /**
     * Optional RequestSetupMetadata for transport upgrade from header
     * to rocket. If this is provided, then the upgrade mechanism will call
     * `RocketClientChannel::newChannelWithMetadata` instead of
     * `RocketClientChannel::newChannel`.
     * NOTE: This is for transport upgrade from header to rocket for non-TLS
     * services.
     */
    std::unique_ptr<RequestSetupMetadata> rocketUpgradeSetupMetadata;
  };

  struct WithRocketUpgrade {};
  struct WithoutRocketUpgrade {};

  using Ptr =
      std::unique_ptr<ClientChannel, folly::DelayedDestruction::Destructor>;
  using LegacyPtr = std::
      unique_ptr<HeaderClientChannel, folly::DelayedDestruction::Destructor>;

  static Ptr newChannel(
      folly::AsyncTransport::UniquePtr transport, Options options = Options());

  static Ptr newChannel(
      WithRocketUpgrade,
      folly::AsyncTransport::UniquePtr transport,
      Options options = Options());

  static LegacyPtr newChannel(
      WithoutRocketUpgrade,
      folly::AsyncTransport::UniquePtr transport,
      Options options = Options()) {
    return LegacyPtr(
        new HeaderClientChannel(std::move(transport), std::move(options)));
  }

  virtual void sendMessage(
      Cpp2Channel::SendCallback* callback,
      std::unique_ptr<folly::IOBuf> buf,
      apache::thrift::transport::THeader* header) {
    cpp2Channel_->sendMessage(callback, std::move(buf), header);
  }

  void closeNow() override;

  // DelayedDestruction methods
  void destroy() override;

  folly::AsyncTransport* getTransport() override {
    return cpp2Channel_->getTransport();
  }

  /**
   * Steal the transport (AsyncSocket) from this channel.
   * NOTE: This is for transport upgrade from header to rocket for non-TLS
   * services.
   */
  folly::AsyncTransport::UniquePtr stealTransport();

  // Client interface from RequestChannel
  using RequestChannel::sendRequestNoResponse;
  using RequestChannel::sendRequestResponse;
  using RequestChannel::sendRequestSink;
  using RequestChannel::sendRequestStream;

  void sendRequestResponse(
      const RpcOptions&,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<apache::thrift::transport::THeader>,
      RequestClientCallback::Ptr,
      std::unique_ptr<folly::IOBuf>) override;

  void sendRequestNoResponse(
      const RpcOptions&,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<apache::thrift::transport::THeader>,
      RequestClientCallback::Ptr,
      std::unique_ptr<folly::IOBuf>) override;

  void sendRequestStream(
      const RpcOptions&,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<transport::THeader>,
      StreamClientCallback* clientCallback,
      std::unique_ptr<folly::IOBuf>) override {
    clientCallback->onFirstResponseError(
        folly::make_exception_wrapper<transport::TTransportException>(
            "This channel doesn't support stream RPC"));
  }

  void sendRequestSink(
      const RpcOptions&,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<transport::THeader>,
      SinkClientCallback* clientCallback,
      std::unique_ptr<folly::IOBuf>) override {
    clientCallback->onFirstResponseError(
        folly::make_exception_wrapper<transport::TTransportException>(
            "This channel doesn't support sink RPC"));
  }

  void setCloseCallback(CloseCallback*) override;

  // Interface from MessageChannel::RecvCallback
  void messageReceived(
      std::unique_ptr<folly::IOBuf>&&,
      std::unique_ptr<apache::thrift::transport::THeader>&&) override;
  void messageChannelEOF() override;
  void messageReceiveErrorWrapped(folly::exception_wrapper&&) override;

  // Client timeouts for read, write.
  // Servers should use timeout methods on underlying transport.
  void setTimeout(uint32_t ms) override;
  uint32_t getTimeout() override { return getTransport()->getSendTimeout(); }

  folly::EventBase* getEventBase() const override {
    return cpp2Channel_->getEventBase();
  }

  /**
   * Updates the HTTP client config for the channel. The channel has to be
   * constructed with HTTP client type.
   */
  void updateHttpClientConfig(const std::string& host, const std::string& uri);

  bool good() override;

  SaturationStatus getSaturationStatus() override {
    return SaturationStatus(0, std::numeric_limits<uint32_t>::max());
  }

  // event base methods
  void attachEventBase(folly::EventBase*) override;
  void detachEventBase() override;
  bool isDetachable() override;

  uint16_t getProtocolId() override;

  CLIENT_TYPE getClientType() override { return clientType_; }

  class ClientFramingHandler : public FramingHandler {
   public:
    explicit ClientFramingHandler(HeaderClientChannel& channel)
        : channel_(channel) {}

    std::tuple<
        std::unique_ptr<folly::IOBuf>,
        size_t,
        std::unique_ptr<apache::thrift::transport::THeader>>
    removeFrame(folly::IOBufQueue* q) override;

    std::unique_ptr<folly::IOBuf> addFrame(
        std::unique_ptr<folly::IOBuf> buf,
        apache::thrift::transport::THeader* header) override;

   private:
    HeaderClientChannel& channel_;
  };

  // Remove a callback from the recvCallbacks_ map.
  void eraseCallback(uint32_t seqId, TwowayCallback<HeaderClientChannel>* cb);

 protected:
  explicit HeaderClientChannel(
      std::shared_ptr<Cpp2Channel> cpp2Channel, Options options);

  bool clientSupportHeader() override;

 private:
  HeaderClientChannel(
      folly::AsyncTransport::UniquePtr transport, Options options);

  void setRequestHeaderOptions(
      apache::thrift::transport::THeader* header, ssize_t payloadSize);
  void attachMetadataOnce(apache::thrift::transport::THeader* header);

  std::shared_ptr<apache::thrift::util::THttpClientParser> httpClientParser_;

  // Set the base class callback based on current state.
  void setBaseReceivedCallback();

  const CLIENT_TYPE clientType_;

  uint32_t sendSeqId_;

  std::unordered_map<uint32_t, TwowayCallback<HeaderClientChannel>*>
      recvCallbacks_;
  std::deque<uint32_t> recvCallbackOrder_;
  CloseCallback* closeCallback_;

  uint32_t timeout_;

  std::shared_ptr<Cpp2Channel> cpp2Channel_;

  const uint16_t protocolId_;

  const std::string agentName_;
  bool firstRequest_{true};

  transport::THeader::StringToStringMap persistentReadHeaders_;

  class RocketUpgradeChannel : public ClientChannel {
   public:
    RocketUpgradeChannel(
        HeaderClientChannel::LegacyPtr headerChannel,
        std::unique_ptr<RequestSetupMetadata>);

    ~RocketUpgradeChannel() override;

    using RequestChannel::sendRequestNoResponse;
    using RequestChannel::sendRequestResponse;
    using RequestChannel::sendRequestSink;
    using RequestChannel::sendRequestStream;

    void sendRequestResponse(
        const RpcOptions&,
        apache::thrift::MethodMetadata&& methodMetadata,
        SerializedRequest&&,
        std::shared_ptr<apache::thrift::transport::THeader>,
        RequestClientCallback::Ptr,
        std::unique_ptr<folly::IOBuf>) override;

    void sendRequestNoResponse(
        const RpcOptions&,
        apache::thrift::MethodMetadata&& methodMetadata,
        SerializedRequest&&,
        std::shared_ptr<apache::thrift::transport::THeader>,
        RequestClientCallback::Ptr,
        std::unique_ptr<folly::IOBuf>) override;

    void sendRequestStream(
        const RpcOptions&,
        MethodMetadata&&,
        SerializedRequest&&,
        std::shared_ptr<transport::THeader>,
        StreamClientCallback* clientCallback,
        std::unique_ptr<folly::IOBuf>) override {
      clientCallback->onFirstResponseError(
          folly::make_exception_wrapper<transport::TTransportException>(
              "This channel doesn't support stream RPC"));
    }

    void sendRequestSink(
        const RpcOptions&,
        MethodMetadata&&,
        SerializedRequest&&,
        std::shared_ptr<transport::THeader>,
        SinkClientCallback* clientCallback,
        std::unique_ptr<folly::IOBuf>) override {
      clientCallback->onFirstResponseError(
          folly::make_exception_wrapper<transport::TTransportException>(
              "This channel doesn't support sink RPC"));
    }

    void setCloseCallback(CloseCallback*) override;

    folly::EventBase* getEventBase() const override;

    uint16_t getProtocolId() override;

    folly::AsyncTransport* getTransport() override;

    bool good() override;

    SaturationStatus getSaturationStatus() override;

    void attachEventBase(folly::EventBase*) override;
    void detachEventBase() override;
    bool isDetachable() override;

    uint32_t getTimeout() override;
    void setTimeout(uint32_t ms) override;

    void closeNow() override;
    CLIENT_TYPE getClientType() override;

   private:
    void initUpgradeIfNeeded(const RpcOptions& firstRequestRpcOptions);
    void upgradeComplete(folly::exception_wrapper ew);

    ClientChannel& getImpl() const;

    // Set until the upgrade is complete
    HeaderClientChannel::LegacyPtr headerChannel_;
    // Set only when upgrade was successful
    RocketClientChannel::Ptr rocketChannel_;
    // The metadata that will be passed to the RocketClientChannel during
    // transport upgrade.
    std::unique_ptr<RequestSetupMetadata> rocketUpgradeSetupMetadata_;

    const protocol::PROTOCOL_TYPES protocolId_{
        static_cast<protocol::PROTOCOL_TYPES>(headerChannel_->getProtocolId())};
    folly::EventBase* evb_{headerChannel_->getEventBase()};

    enum class State { INIT = 0, UPGRADE_IN_PROGRESS = 1, DONE = 2 };
    State state_;

    class RocketUpgradeCallback;

    class BufferedRequest {
     public:
      BufferedRequest(
          const RpcOptions& rpcOptions,
          MethodMetadata&& methodMetadata,
          SerializedRequest&& serializedRequest,
          std::shared_ptr<apache::thrift::transport::THeader> header,
          RequestClientCallback::Ptr cb,
          std::unique_ptr<folly::IOBuf> frameworkMetadata,
          bool oneWay)
          : rpcOptions_(rpcOptions),
            methodMetadata_(std::move(methodMetadata)),
            serializedRequest_(std::move(serializedRequest)),
            header_(std::move(header)),
            callback_(std::move(cb)),
            frameworkMetadata_(std::move(frameworkMetadata)),
            oneWay_(oneWay) {}

      void send(ClientChannel& channel) &&;
      void fail(folly::exception_wrapper ew) &&;

     private:
      const RpcOptions rpcOptions_;
      MethodMetadata methodMetadata_;
      SerializedRequest serializedRequest_;
      std::shared_ptr<apache::thrift::transport::THeader> header_;
      RequestClientCallback::Ptr callback_;
      std::unique_ptr<folly::IOBuf> frameworkMetadata_;
      const bool oneWay_;
    };

    std::queue<BufferedRequest> bufferedRequests_;

    friend class TransportUpgradeTest;
    friend class TransportUpgradeTest_RawClientRocketUpgradeOneway_Test;
    friend class TransportUpgradeTest_RawClientNoUpgrade_Test;
    friend class TransportUpgradeTest_RawClientRocketUpgradeTimeout_Test;
  };

  friend class TransportUpgradeTest;
  friend class TransportUpgradeTest_RawClientRocketUpgradeOneway_Test;
  friend class TransportUpgradeTest_RawClientNoUpgrade_Test;
  friend class TransportUpgradeTest_RawClientRocketUpgradeTimeout_Test;
};

} // namespace apache::thrift

#endif // THRIFT_ASYNC_THEADERCLIENTCHANNEL_H_
