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

#include <folly/Singleton.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/futures/Future.h>
#include <folly/futures/Promise.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/NotificationQueue.h>

#ifdef SPLICE_F_NONBLOCK
namespace wangle {

class FileRegion {
 public:
  FileRegion(int fd, loff_t offset, size_t count)
      : fd_(fd), offset_(offset), count_(count) {}

  folly::Future<folly::Unit> transferTo(
      std::shared_ptr<folly::AsyncTransport> transport) {
    auto socket = std::dynamic_pointer_cast<folly::AsyncSocket>(transport);
    CHECK(socket);
    auto cb = new WriteCallback();
    auto f = cb->promise_.getFuture();
    auto req = new FileWriteRequest(socket.get(), cb, fd_, offset_, count_);
    socket->writeRequest(req);
    return f;
  }

 private:
  class WriteCallback : private folly::AsyncSocket::WriteCallback {
    void writeSuccess() noexcept override {
      promise_.setValue();
      delete this;
    }

    void writeErr(
        size_t /* bytesWritten */,
        const folly::AsyncSocketException& ex) noexcept override {
      promise_.setException(ex);
      delete this;
    }

    friend class FileRegion;
    folly::Promise<folly::Unit> promise_;
  };

  const int fd_;
  const loff_t offset_;
  const size_t count_;

  class FileWriteRequest : public folly::AsyncSocket::WriteRequest,
                           public folly::NotificationQueue<size_t>::Consumer {
   public:
    FileWriteRequest(
        folly::AsyncSocket* socket,
        WriteCallback* callback,
        int fd,
        loff_t offset,
        size_t count);

    void destroy() override;

    folly::AsyncSocket::WriteResult performWrite() override;

    void consume() override;

    bool isComplete() override;

    void messageAvailable(size_t&& count) noexcept override;

    void start() override;

    class FileReadHandler : public folly::EventHandler {
     public:
      FileReadHandler(FileWriteRequest* req, int pipe_in, size_t bytesToRead);

      ~FileReadHandler() override;

      void handlerReady(uint16_t events) noexcept override;

     private:
      FileWriteRequest* req_;
      int pipe_in_;
      size_t bytesToRead_;
    };

   private:
    ~FileWriteRequest() override;

    void fail(const char* fn, const folly::AsyncSocketException& ex);

    const int readFd_;
    loff_t offset_;
    const size_t count_;
    bool started_{false};
    int pipe_out_{-1};

    size_t bytesInPipe_{0};
    folly::EventBase* readBase_;
    folly::NotificationQueue<size_t> queue_;
    std::unique_ptr<FileReadHandler> readHandler_;
  };
};

} // namespace wangle
#endif
