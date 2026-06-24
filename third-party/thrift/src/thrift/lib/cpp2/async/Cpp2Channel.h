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

#ifndef THRIFT_ASYNC_CPP2CHANNEL_H_
#define THRIFT_ASYNC_CPP2CHANNEL_H_ 1

#include <deque>
#include <memory>
#include <vector>

#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/FramingHandler.h>
#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/async/TAsyncTransportHandler.h>
#include <wangle/channel/Handler.h>
#include <wangle/channel/OutputBufferingHandler.h>
#include <wangle/channel/StaticPipeline.h>

namespace apache::thrift {

using apache::thrift::transport::THeader;

class Cpp2Channel
    : public MessageChannel,
      public wangle::Handler<
          std::pair<std::unique_ptr<folly::IOBuf>, std::unique_ptr<THeader>>,
          int, // last inbound handler so this doesn't matter
          // Does nothing when writing
          std::pair<std::unique_ptr<folly::IOBuf>, THeader*>,
          std::pair<std::unique_ptr<folly::IOBuf>, THeader*>> {
 public:
  explicit Cpp2Channel(
      const std::shared_ptr<folly::AsyncTransport>& transport,
      std::unique_ptr<FramingHandler> framingHandler);

  // TODO(jsedgwick) This should be protected, but wangle::StaticPipeline
  // will encase this in a folly::Optional, which requires a public destructor.
  // Need to add a static_assert to Optional to make that prereq clearer
  ~Cpp2Channel() override {}

  static std::unique_ptr<Cpp2Channel, folly::DelayedDestruction::Destructor>
  newChannel(
      const std::shared_ptr<folly::AsyncTransport>& transport,
      std::unique_ptr<FramingHandler> framingHandler) {
    return std::unique_ptr<Cpp2Channel, folly::DelayedDestruction::Destructor>(
        new Cpp2Channel(transport, std::move(framingHandler)));
  }
  void closeNow();

  void setTransport(const std::shared_ptr<folly::AsyncTransport>& transport) {
    transport_ = transport;
    transportHandler_->setTransport(transport);
    // swapped transports must be attached to same EventBase
    DCHECK(!transport_ || evb_ == transport->getEventBase());
  }
  folly::AsyncTransport* getTransport() { return transport_.get(); }

  // Return a shared_ptr of the AsyncTransport
  std::shared_ptr<folly::AsyncTransport> getTransportShared() {
    return transport_;
  }

  // DelayedDestruction methods
  void destroy() override;

  // BytesToBytesHandler methods
  void read(
      Context* ctx,
      std::pair<std::unique_ptr<folly::IOBuf>, std::unique_ptr<THeader>>
          bufAndHeader) override;
  void readEOF(Context* ctx) override;
  void readException(Context* ctx, folly::exception_wrapper e) override;
  folly::Future<folly::Unit> close(Context* ctx) override;

  folly::Future<folly::Unit> write(
      Context* ctx,
      std::pair<std::unique_ptr<folly::IOBuf>, THeader*> bufAndHeader)
      override {
    return ctx->fireWrite(std::move(bufAndHeader));
  }

  void writeSuccess() noexcept;
  void writeError(
      size_t bytesWritten,
      const apache::thrift::transport::TTransportException& ex) noexcept;

  void processReadEOF() noexcept;

  // Interface from MessageChannel
  void sendMessage(
      SendCallback* callback,
      std::unique_ptr<folly::IOBuf>&& buf,
      apache::thrift::transport::THeader* header) override;
  void setReceiveCallback(RecvCallback* callback) override;

  // event base methods
  virtual void attachEventBase(folly::EventBase*);
  virtual void detachEventBase();
  folly::EventBase* getEventBase();

  /**
   * Set read buffer size.
   *
   * @param readBufferSize   The read buffer size to set
   * @param strict           True means given size will always be used; false
   *                         means given size may not be used if it is too small
   */
  void setReadBufferSize(uint32_t readBufferSize, bool strict = false) {
    framingHandler_->setReadBufferSize(readBufferSize, strict);
  }

 private:
  std::shared_ptr<folly::AsyncTransport> transport_;
  folly::EventBase* evb_;
  std::deque<SendCallback*> sendCallbacks_;

  RecvCallback* recvCallback_;
  bool eofInvoked_;

  std::shared_ptr<wangle::OutputBufferingHandler> outputBufferingHandler_;
  std::shared_ptr<FramingHandler> framingHandler_;

  using Pipeline = wangle::StaticPipeline<
      folly::IOBufQueue&,
      std::pair<
          std::unique_ptr<folly::IOBuf>,
          apache::thrift::transport::THeader*>,
      TAsyncTransportHandler,
      wangle::OutputBufferingHandler,
      FramingHandler,
      Cpp2Channel>;
  std::shared_ptr<Pipeline> pipeline_;
  TAsyncTransportHandler* transportHandler_;
};

} // namespace apache::thrift

#endif // THRIFT_ASYNC_CPP2CHANNEL_H_
