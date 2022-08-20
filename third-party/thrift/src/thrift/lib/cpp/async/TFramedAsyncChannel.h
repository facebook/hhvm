/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#ifndef THRIFT_ASYNC_TFRAMEDASYNCCHANNEL_H_
#define THRIFT_ASYNC_TFRAMEDASYNCCHANNEL_H_ 1

#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp/async/TStreamAsyncChannel.h>

namespace apache {
namespace thrift {
namespace async {

namespace detail {

/**
 * Encapsulation of one outstanding write request on a TFramedAsyncChannel.
 */
class TFramedACWriteRequest
    : public TAsyncChannelWriteRequestBase<TFramedACWriteRequest> {
 public:
  TFramedACWriteRequest(
      const VoidCallback& callback,
      const VoidCallback& errorCallback,
      transport::TMemoryBuffer* message,
      TAsyncEventChannel* channel);

  void write(
      folly::AsyncTransport* transport,
      folly::AsyncTransport::WriteCallback* callback) noexcept;

  void writeSuccess() noexcept;
  void writeError(
      size_t bytesWritten, const transport::TTransportException& ex) noexcept;

 private:
  union {
    uint32_t frameSize_;
    char frameSizeBuf_[sizeof(uint32_t)];
  };
};

/**
 * Read state for TFramedAsyncChannel
 */
class TFramedACReadState {
 public:
  typedef std::function<void()> VoidCallback;

  TFramedACReadState();

  // Methods required by TStreamAsyncChannel

  void setCallbackBuffer(transport::TMemoryBuffer* buffer) {
    buffer_ = buffer;
    bytesRead_ = 0;
  }
  void unsetCallbackBuffer() { buffer_ = nullptr; }

  bool hasReadAheadData() {
    assert(bytesRead_ == 0);
    return false;
  }
  bool hasPartialMessage() { return (bytesRead_ > 0); }

  void getReadBuffer(void** bufReturn, size_t* lenReturn);
  bool readDataAvailable(size_t len);

  // Other methods specifict to TFramedAsyncChannel

  void setMaxFrameSize(uint32_t size) { maxFrameSize_ = size; }

  uint32_t getMaxFrameSize() const { return maxFrameSize_; }

 private:
  /// maximum frame size accepted
  uint32_t maxFrameSize_;

  union {
    uint32_t frameSize_;
    char frameSizeBuf_[sizeof(uint32_t)];
  };

  /**
   * The number of bytes read.
   *
   * This includes the bytes in the frame size.  When bytesRead_ is less than
   * sizeof(uint32_t), we are still reading the frame size.  Otherwise, we have
   * read bytesRead_ - sizeof(uint32_t) bytes of the body.
   */
  uint32_t bytesRead_;
  apache::thrift::transport::TMemoryBuffer* buffer_;
};

} // namespace detail

/**
 * TFramedAsyncChannel
 *
 * This is a TAsyncChannel implementation that reads and writes messages
 * prefixed with a 4-byte frame length.
 *
 * Its messages are compatible with TFramedTransport.
 */
class TFramedAsyncChannel
    : public TStreamAsyncChannel<
          apache::thrift::async::detail::TFramedACWriteRequest,
          apache::thrift::async::detail::TFramedACReadState> {
 private:
  typedef TStreamAsyncChannel<
      apache::thrift::async::detail::TFramedACWriteRequest,
      apache::thrift::async::detail::TFramedACReadState>
      Parent;

 public:
  explicit TFramedAsyncChannel(
      const std::shared_ptr<folly::AsyncTransport>& transport)
      : Parent(transport) {}

  /**
   * Helper function to create a shared_ptr<TFramedAsyncChannel>.
   *
   * This passes in the correct destructor object, since TFramedAsyncChannel's
   * destructor is protected and cannot be invoked directly.
   */
  static std::shared_ptr<TFramedAsyncChannel> newChannel(
      const std::shared_ptr<folly::AsyncTransport>& transport) {
    return std::shared_ptr<TFramedAsyncChannel>(
        new TFramedAsyncChannel(transport), Destructor());
  }

  /// size in bytes beyond which we'll reject a given frame size.
  void setMaxFrameSize(uint32_t size) { readState_.setMaxFrameSize(size); }

  uint32_t getMaxFrameSize() const { return readState_.getMaxFrameSize(); }

 protected:
  /**
   * Protected destructor.
   *
   * Users of TFramedAsyncChannel must never delete it directly.  Instead,
   * invoke destroy().
   */
  ~TFramedAsyncChannel() override {}
};

class TFramedAsyncChannelFactory : public TStreamAsyncChannelFactory {
 public:
  TFramedAsyncChannelFactory()
      : maxFrameSize_(0x7fffffff), recvTimeout_(0), sendTimeout_(0) {}

  void setMaxFrameSize(uint32_t bytes) { maxFrameSize_ = bytes; }

  void setRecvTimeout(uint32_t milliseconds) { recvTimeout_ = milliseconds; }

  void setSendTimeout(uint32_t milliseconds) { sendTimeout_ = milliseconds; }

  std::shared_ptr<TAsyncEventChannel> newChannel(
      const std::shared_ptr<folly::AsyncTransport>& transport) override {
    std::shared_ptr<TFramedAsyncChannel> channel(
        TFramedAsyncChannel::newChannel(transport));
    transport->setSendTimeout(sendTimeout_);
    channel->setMaxFrameSize(maxFrameSize_);
    channel->setRecvTimeout(recvTimeout_);
    return channel;
  }

 private:
  uint32_t maxFrameSize_;
  uint32_t recvTimeout_;
  uint32_t sendTimeout_;
};

} // namespace async
} // namespace thrift
} // namespace apache

#endif // THRIFT_ASYNC_TFRAMEDASYNCCHANNEL_H_
