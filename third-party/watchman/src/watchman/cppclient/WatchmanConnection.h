/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <atomic>
#include <deque>
#include <memory>
#include <optional>

#include <folly/ExceptionWrapper.h>
#include <folly/futures/Future.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/json/dynamic.h>

namespace watchman {

// General watchman error
class WatchmanError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

// Encapsulates an error reported by the protocol
// what() returns the error message, getResponse() returns
// the complete response packet
class WatchmanResponseError : public WatchmanError {
 public:
  explicit WatchmanResponseError(const folly::dynamic& response);
  const folly::dynamic& getResponse() const;

 private:
  folly::dynamic response_;
};

// Represents a raw connection to the watchman service
class WatchmanConnection
    : folly::AsyncSocket::ConnectCallback,
      folly::AsyncReader::ReadCallback,
      folly::AsyncWriter::WriteCallback,
      public std::enable_shared_from_this<WatchmanConnection> {
 public:
  using Callback = std::function<void(folly::Try<folly::dynamic>)>;

  explicit WatchmanConnection(
      folly::EventBase* eventBase,
      std::optional<std::string>&& sockPath = {},
      std::optional<Callback>&& callback = {},
      // You really should provide an executor that runs in a different
      // thread to avoid blocking your event base for large responses
      folly::Executor* cpuExecutor = nullptr);
  ~WatchmanConnection() override;

  // Initiate a connection.  Yields the version information for the
  // service at a later time.  You need to call connect once before
  // you can use the run() method.
  // versionArgs, if specified, must be an object value.  It will
  // be passed as part of the extended version command and should
  // be used to list required capabilities for the session
  folly::Future<folly::dynamic> connect(
      folly::dynamic versionArgs = folly::dynamic::object(
          "required",
          folly::dynamic::array("relative_root")));

  // Issue a watchman command, yielding the results at a later time.
  // If the connection was terminated, will throw immediately
  folly::Future<folly::dynamic> run(const folly::dynamic& command) noexcept;

  // Close the connection.  All queued commands will be cancelled
  void close();

  // Returns true if the connection has been closed or is in a broken state
  bool isDead() {
    return closing_ || broken_;
  }

  // This is intended for test only.
  void forceEOF() {
    readEOF();
  }

 private:
  // Represents a command queued up by the run() function
  struct QueuedCommand {
    folly::dynamic cmd;
    folly::Promise<folly::dynamic> promise;

    explicit QueuedCommand(const folly::dynamic& command);
  };

  folly::Future<std::string> getSockPath();
  void failQueuedCommands(folly::exception_wrapper&& ex);
  void sendCommand(bool pop = false);
  void popAndSendCommand();
  void decodeNextResponse();
  folly::Try<folly::dynamic> watchmanResponseToTry(folly::dynamic&& value);
  std::unique_ptr<folly::IOBuf> splitNextPdu();

  // ConnectCallback
  void connectSuccess() noexcept override;
  void connectErr(const folly::AsyncSocketException& ex) noexcept override;

  // WriteCallback
  void writeSuccess() noexcept override;
  void writeErr(
      size_t bytesWritten,
      const folly::AsyncSocketException& ex) noexcept override;

  // ReadCallback
  void getReadBuffer(void** bufReturn, size_t* lenReturn) override;
  void readDataAvailable(size_t len) noexcept override;
  void readEOF() noexcept override;
  void readErr(const folly::AsyncSocketException& ex) noexcept override;

  folly::Executor::KeepAlive<folly::EventBase> eventBase_;
  std::optional<std::string> sockPath_;
  std::optional<Callback> callback_;
  folly::Executor::KeepAlive<folly::Executor> cpuExecutor_;
  folly::Promise<folly::dynamic> connectPromise_;
  folly::dynamic versionCmd_;
  std::shared_ptr<folly::AsyncSocket> sock_;
  std::mutex mutex_;
  std::deque<std::shared_ptr<QueuedCommand>> commandQ_;
  folly::IOBufQueue bufQ_{folly::IOBufQueue::cacheChainLength()};
  bool broken_{false};
  bool closing_{false};
  std::atomic<bool> decoding_{false};
};
} // namespace watchman
