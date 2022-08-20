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

#ifndef THRIFT_ASYNC_TASYNCCHANNEL_H_
#define THRIFT_ASYNC_TASYNCCHANNEL_H_ 1

#include <functional>

#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp/transport/TBufferTransports.h>
#include <thrift/lib/cpp/transport/TTransportException.h>

namespace apache {
namespace thrift {
namespace async {

/**
 * TAsyncChannel defines an asynchronous API for message-based I/O.
 */
class TAsyncChannel {
 public:
  typedef std::function<void()> VoidCallback;

  virtual ~TAsyncChannel() {}

  // is the channel readable (possibly closed by the remote site)?
  virtual bool readable() const = 0;
  // is the channel in a good state?
  virtual bool good() const = 0;
  virtual bool error() const = 0;
  virtual bool timedOut() const = 0;

  // Get TTransportException error type if error() returns true
  virtual transport::TTransportException::TTransportExceptionType errorType()
      const {
    return transport::TTransportException::TTransportExceptionType::UNKNOWN;
  }

  /**
   * Send a message over the channel.
   *
   * @return  call "cob" on success, "errorCob" on fail.  Caller must be ready
   *          for either cob to be called before return.  Only one cob will be
   *          called and it will be called exactly once per invocation.
   */
  virtual void sendMessage(
      const VoidCallback& cob,
      const VoidCallback& errorCob,
      apache::thrift::transport::TMemoryBuffer* message) = 0;

  virtual void sendOnewayMessage(
      const VoidCallback& cob,
      const VoidCallback& errorCob,
      apache::thrift::transport::TMemoryBuffer* message) {
    sendMessage(cob, errorCob, message);
  }

  /**
   * Receive a message from the channel.
   *
   * @return  call "cob" on success, "errorCob" on fail.  Caller must be ready
   *          for either cob to be called before return.  Only one cob will be
   *          called and it will be called exactly once per invocation.
   */
  virtual void recvMessage(
      const VoidCallback& cob,
      const VoidCallback& errorCob,
      apache::thrift::transport::TMemoryBuffer* message) = 0;

  /**
   * Send a message over the channel and receive a response.
   *
   * @return  call "cob" on success, "errorCob" on fail.  Caller must be ready
   *          for either cob to be called before return.  Only one cob will be
   *          called and it will be called exactly once per invocation.
   */
  virtual void sendAndRecvMessage(
      const VoidCallback& cob,
      const VoidCallback& errorCob,
      apache::thrift::transport::TMemoryBuffer* sendBuf,
      apache::thrift::transport::TMemoryBuffer* recvBuf) = 0;

  /**
   * Send a message over the channel, single cob version.  (See above.)
   *
   * @return  call "cob" on success or fail; channel status must be queried
   *          by the cob.
   */
  void sendMessage(
      const VoidCallback& cob,
      apache::thrift::transport::TMemoryBuffer* message) {
    return sendMessage(cob, cob, message);
  }

  void sendOnewayMessage(
      const VoidCallback& cob,
      apache::thrift::transport::TMemoryBuffer* message) {
    return sendOnewayMessage(cob, cob, message);
  }

  /**
   * Receive a message from the channel, single cob version.  (See above.)
   *
   * @return  call "cob" on success or fail; channel status must be queried
   *          by the cob.
   */
  void recvMessage(
      const VoidCallback& cob,
      apache::thrift::transport::TMemoryBuffer* message) {
    return recvMessage(cob, cob, message);
  }

  /**
   * Send a message over the channel and receive response, single cob version.
   * (See above.)
   *
   * @return  call "cob" on success or fail; channel status must be queried
   *          by the cob.
   */
  void sendAndRecvMessage(
      const VoidCallback& cob,
      apache::thrift::transport::TMemoryBuffer* sendBuf,
      apache::thrift::transport::TMemoryBuffer* recvBuf) {
    return sendAndRecvMessage(cob, cob, sendBuf, recvBuf);
  }

  /**
   * Cancel pending callbacks. Use this when the channel is closing because the
   * server had been shut down.
   */
  virtual void cancelCallbacks() = 0;

  // TODO(dreiss): Make this nonvirtual when TFramedSocketAsyncChannel gets
  // renamed to TFramedAsyncChannel.
  virtual std::shared_ptr<folly::AsyncTransport> getTransport() = 0;
};

} // namespace async
} // namespace thrift
} // namespace apache

#endif // #ifndef THRIFT_ASYNC_TASYNCCHANNEL_H_
