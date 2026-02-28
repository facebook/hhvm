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

#ifndef THRIFT_ASYNC_THEADERSERVERCHANNEL_H_
#define THRIFT_ASYNC_THEADERSERVERCHANNEL_H_ 1

#include <memory>
#include <unordered_map>

#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/async/Cpp2Channel.h>
#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/async/ServerChannel.h>

namespace apache::thrift {

/**
 * HeaderServerChannel
 *
 * This is a server channel implementation that
 * manages requests / responses via seqId.
 */
class HeaderServerChannel : public ServerChannel,
                            public MessageChannel::RecvCallback,
                            virtual public folly::DelayedDestruction {
 protected:
  ~HeaderServerChannel() override {}

 public:
  explicit HeaderServerChannel(
      const std::shared_ptr<folly::AsyncTransport>& transport);

  explicit HeaderServerChannel(const std::shared_ptr<Cpp2Channel>& cpp2Channel);

  static std::unique_ptr< //
      HeaderServerChannel,
      folly::DelayedDestruction::Destructor>
  newChannel(const std::shared_ptr<folly::AsyncTransport>& transport) {
    return std::
        unique_ptr<HeaderServerChannel, folly::DelayedDestruction::Destructor>(
            new HeaderServerChannel(transport));
  }

  // DelayedDestruction methods
  void destroy() override;

  folly::AsyncTransport* getTransport() { return cpp2Channel_->getTransport(); }

  void setTransport(std::shared_ptr<folly::AsyncTransport> transport) {
    cpp2Channel_->setTransport(transport);
  }

  // Server interface from ResponseChannel
  void setCallback(ResponseChannel::Callback* callback) override;

  virtual void sendMessage(
      Cpp2Channel::SendCallback* callback,
      std::unique_ptr<folly::IOBuf> buf,
      apache::thrift::transport::THeader* header) {
    cpp2Channel_->sendMessage(callback, std::move(buf), header);
  }

  // Interface from MessageChannel::RecvCallback
  void messageReceived(
      std::unique_ptr<folly::IOBuf>&&,
      std::unique_ptr<apache::thrift::transport::THeader>&&) override;
  void messageChannelEOF() override;
  void messageReceiveErrorWrapped(folly::exception_wrapper&&) override;

  folly::EventBase* getEventBase() { return cpp2Channel_->getEventBase(); }

  void sendCatchupRequests(
      std::unique_ptr<folly::IOBuf> next_req,
      MessageChannel::SendCallback* cb,
      apache::thrift::transport::THeader* header);

  class HeaderRequest final : public ResponseChannelRequest {
   public:
    HeaderRequest(
        HeaderServerChannel* channel,
        std::unique_ptr<folly::IOBuf>&& buf,
        std::unique_ptr<apache::thrift::transport::THeader>&& header,
        const server::TServerObserver::SamplingStatus& samplingStatus);

    bool isActive() const override {
      DCHECK(false);
      return true;
    }

    bool isOneway() const override {
      return header_->getSequenceNumber() == ONEWAY_REQUEST_ID;
    }

    bool includeEnvelope() const override { return true; }

    void setInOrderRecvSequenceId(uint32_t seqId) { InOrderRecvSeqId_ = seqId; }

    apache::thrift::transport::THeader* getHeader() { return header_.get(); }

    void sendReply(
        ResponsePayload&&,
        MessageChannel::SendCallback* cb = nullptr,
        folly::Optional<uint32_t> crc32 = folly::none) override;

    void sendException(
        ResponsePayload&& response,
        MessageChannel::SendCallback* cb = nullptr) override {
      sendReply(std::move(response), cb);
    }

    void serializeAndSendError(
        apache::thrift::transport::THeader& header,
        TApplicationException& tae,
        const std::string& methodName,
        int32_t protoSeqId,
        MessageChannel::SendCallback* cb);

    void sendErrorWrapped(
        folly::exception_wrapper ex, std::string exCode) override;

    void sendErrorWrapped(
        folly::exception_wrapper ex,
        std::string exCode,
        const std::string& methodName,
        int32_t protoSeqId,
        MessageChannel::SendCallback* cb = nullptr);

    /* We differentiate between two types of timeouts:
       1) Task timeouts refer to timeouts that fire while the request is
       currently being proceesed
       2) Queue timeouts refer to timeouts that fire before processing
       of the request has begun
    */
    enum TimeoutResponseType { TASK, QUEUE };

    void sendTimeoutResponse(
        const std::string& methodName,
        int32_t protoSeqId,
        MessageChannel::SendCallback* cb,
        const transport::THeader::StringToStringMap& headers,
        TimeoutResponseType responseType);

    const SamplingStatus& getSamplingStatus() const { return samplingStatus_; }

    folly::IOBuf* getBuf() { return buf_.get(); }
    std::unique_ptr<folly::IOBuf> extractBuf() { return std::move(buf_); }

   protected:
    bool tryStartProcessing() override {
      DCHECK(false);
      return true;
    }

   private:
    std::unique_ptr<folly::IOBuf> buf_;
    HeaderServerChannel* channel_;
    std::unique_ptr<apache::thrift::transport::THeader> header_;
    std::unique_ptr<apache::thrift::transport::THeader> timeoutHeader_;
    uint32_t InOrderRecvSeqId_{0}; // Used internally for in-order requests
    SamplingStatus samplingStatus_;
  };

  class Callback : public ResponseChannel::Callback {
   public:
    virtual void requestReceived(std::unique_ptr<HeaderRequest>&&) = 0;
  };

  void setSampleRate(uint32_t sampleRate) { sampleRate_ = sampleRate; }

  void closeNow() { cpp2Channel_->closeNow(); }

  class ServerFramingHandler : public FramingHandler {
   public:
    explicit ServerFramingHandler(HeaderServerChannel& channel)
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
    HeaderServerChannel& channel_;
  };

 private:
  static std::string getTHeaderPayloadString(folly::IOBuf* buf);
  static std::string getTransportDebugString(folly::AsyncTransport* transport);

  server::TServerObserver::SamplingStatus shouldSample(
      const apache::thrift::transport::THeader* header) const;

  HeaderServerChannel::Callback* callback_;

  // For backwards-compatible in-order responses support
  std::unordered_map<
      uint32_t,
      std::tuple<
          MessageChannel::SendCallback*,
          std::unique_ptr<folly::IOBuf>,
          std::unique_ptr<apache::thrift::transport::THeader>>>
      inOrderRequests_;

  uint32_t arrivalSeqId_;
  uint32_t lastWrittenSeqId_;

  folly::Optional<bool> outOfOrder_;

  static const int MAX_REQUEST_SIZE = 2000;
  static std::atomic<uint32_t> sample_;
  uint32_t sampleRate_;

  transport::THeader::StringToStringMap persistentReadHeaders_;

  std::shared_ptr<Cpp2Channel> cpp2Channel_;
};

} // namespace apache::thrift

#endif // THRIFT_ASYNC_THEADERSERVERCHANNEL_H_
