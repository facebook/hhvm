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

#pragma once

#include <memory>

#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/HeaderServerChannel.h>

namespace apache {
namespace thrift {

class DuplexChannel {
 public:
  class Who {
   public:
    enum WhoEnum { UNKNOWN, CLIENT, SERVER };
    explicit Who(WhoEnum who = UNKNOWN) : who_(who) {}
    void set(WhoEnum who) { who_ = who; }
    WhoEnum get() const {
      DCHECK(who_ != UNKNOWN);
      return who_;
    }
    WhoEnum getOther() const {
      DCHECK(who_ != UNKNOWN);
      return who_ == CLIENT ? SERVER : CLIENT;
    }

   private:
    WhoEnum who_;
  };

  explicit DuplexChannel(
      Who::WhoEnum mainChannel,
      const std::shared_ptr<folly::AsyncTransport>& transport,
      HeaderClientChannel::Options options = HeaderClientChannel::Options());

  ~DuplexChannel() {
    clientChannel_->duplex_ = nullptr;
    serverChannel_->duplex_ = nullptr;
  }

  std::shared_ptr<HeaderClientChannel> getClientChannel() {
    return clientChannel_;
  }

  std::shared_ptr<HeaderServerChannel> getServerChannel() {
    return serverChannel_;
  }

  std::shared_ptr<Cpp2Channel> getCpp2Channel() { return cpp2Channel_; }

 private:
  class DuplexClientChannel : public HeaderClientChannel {
   public:
    DuplexClientChannel(
        DuplexChannel& duplex,
        const std::shared_ptr<Cpp2Channel>& cpp2Channel,
        HeaderClientChannel::Options options = HeaderClientChannel::Options())
        : HeaderClientChannel(cpp2Channel, std::move(options)),
          duplex_(&duplex) {}
    void sendMessage(
        Cpp2Channel::SendCallback* callback,
        std::unique_ptr<folly::IOBuf> buf,
        apache::thrift::transport::THeader* header) override {
      if (duplex_) {
        duplex_->lastSender_.set(Who::CLIENT);
      }
      HeaderClientChannel::sendMessage(callback, std::move(buf), header);
    }
    void messageChannelEOF() override {
      HeaderClientChannel::messageChannelEOF();
      if (duplex_ && duplex_->serverChannel_) {
        duplex_->serverChannel_->HeaderServerChannel::messageChannelEOF();
      }
    }

   private:
    friend class DuplexChannel;
    DuplexChannel* duplex_;
  };

  class DuplexServerChannel : public HeaderServerChannel {
   public:
    DuplexServerChannel(
        DuplexChannel& duplex, const std::shared_ptr<Cpp2Channel>& cpp2Channel)
        : HeaderServerChannel(cpp2Channel), duplex_(&duplex) {}
    void sendMessage(
        Cpp2Channel::SendCallback* callback,
        std::unique_ptr<folly::IOBuf> buf,
        apache::thrift::transport::THeader* header) override {
      if (duplex_) {
        duplex_->lastSender_.set(Who::SERVER);
      }
      HeaderServerChannel::sendMessage(callback, std::move(buf), header);
    }
    void messageChannelEOF() override {
      if (duplex_ && duplex_->clientChannel_) {
        duplex_->clientChannel_->HeaderClientChannel::messageChannelEOF();
      }
      HeaderServerChannel::messageChannelEOF();
    }

   private:
    friend class DuplexChannel;
    DuplexChannel* duplex_;
  };

  class DuplexCpp2Channel : public Cpp2Channel {
   public:
    DuplexCpp2Channel(
        Who::WhoEnum duplex_main_channel,
        const std::shared_ptr<folly::AsyncTransport>& transport,
        std::unique_ptr<FramingHandler> framingHandler)
        : Cpp2Channel(transport, std::move(framingHandler)),
          duplexMainChannel_(duplex_main_channel),
          client_(nullptr),
          server_(nullptr) {}

    void setReceiveCallback(RecvCallback*) override {
      // the magic happens in primeCallbacks and useCallback
    }

    void primeCallbacks(RecvCallback* client, RecvCallback* server) {
      DCHECK(client != nullptr);
      DCHECK(server != nullptr);
      DCHECK(client_ == nullptr);
      DCHECK(server_ == nullptr);
      client_ = client;
      server_ = server;
      Cpp2Channel::setReceiveCallback(
          duplexMainChannel_.get() == Who::CLIENT ? client_ : server_);
    }

    void useCallback(Who::WhoEnum who) {
      switch (who) {
        case Who::CLIENT:
          Cpp2Channel::setReceiveCallback(client_);
          break;
        case Who::SERVER:
          Cpp2Channel::setReceiveCallback(server_);
          break;
        default:
          DCHECK(false);
      }
    }

   private:
    const Who duplexMainChannel_;
    RecvCallback* client_;
    RecvCallback* server_;
  };

  const std::shared_ptr<DuplexCpp2Channel> cpp2Channel_;

  const std::shared_ptr<DuplexClientChannel> clientChannel_;
  HeaderClientChannel::ClientFramingHandler clientFramingHandler_;

  const std::shared_ptr<DuplexServerChannel> serverChannel_;
  HeaderServerChannel::ServerFramingHandler serverFramingHandler_;

  const Who mainChannel_;
  Who lastSender_;

  class DuplexFramingHandler : public FramingHandler {
   public:
    explicit DuplexFramingHandler(DuplexChannel& duplex) : duplex_(duplex) {}

    std::tuple<
        std::unique_ptr<folly::IOBuf>,
        size_t,
        std::unique_ptr<apache::thrift::transport::THeader>>
    removeFrame(folly::IOBufQueue* q) override;

    std::unique_ptr<folly::IOBuf> addFrame(
        std::unique_ptr<folly::IOBuf> buf,
        apache::thrift::transport::THeader* header) override;

   private:
    DuplexChannel& duplex_;

    FramingHandler& getHandler(DuplexChannel::Who::WhoEnum who);
  };
};

} // namespace thrift
} // namespace apache
