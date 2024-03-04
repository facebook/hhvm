/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WatchmanConnection.h"

#include <cstdlib>

#include <fmt/core.h>

#include <folly/ExceptionWrapper.h>
#include <folly/SocketAddress.h>
#include <folly/executors/InlineExecutor.h>
#include <folly/experimental/bser/Bser.h>

#ifdef _WIN32
#include <eden/common/utils/SpawnedProcess.h> // @manual
#else
#include <folly/Subprocess.h> // @manual
#endif

namespace watchman {

using namespace folly::bser;
using namespace folly;
#ifdef _WIN32
using facebook::eden::SpawnedProcess;
#endif

// Ordered with the most likely kind first
static const std::vector<dynamic> kUnilateralLabels{"subscription", "log"};

static const dynamic kError("error");
static const dynamic kCapabilities("capabilities");

// We'll just dispatch bser decodes and callbacks inline unless they
// give us an alternative environment
static InlineExecutor inlineExecutor;

WatchmanConnection::WatchmanConnection(
    EventBase* eventBase,
    std::optional<std::string>&& sockPath,
    std::optional<WatchmanConnection::Callback>&& callback,
    Executor* cpuExecutor)
    : eventBase_(eventBase),
      sockPath_(std::move(sockPath)),
      callback_(std::move(callback)),
      cpuExecutor_(cpuExecutor ? cpuExecutor : &inlineExecutor),
      versionCmd_(nullptr),
      bufQ_(IOBufQueue::cacheChainLength()) {
  CHECK_NOTNULL(eventBase);
}

WatchmanConnection::~WatchmanConnection() {
  close();
}

folly::Future<std::string> WatchmanConnection::getSockPath() {
  // Take explicit configuration first
  if (sockPath_.has_value()) {
    return makeFuture(sockPath_.value());
  }

  // Else use the environmental variable used by watchman to report
  // the active socket path
  auto var = std::getenv("WATCHMAN_SOCK");
  if (var && *var) {
    return makeFuture(std::string(var));
  }

  return via(cpuExecutor_, [] {
  // Else discover it from the CLI
#ifdef _WIN32
    SpawnedProcess::Options options;
    options.pipeStdout();
    options.pipeStderr();

    SpawnedProcess proc{
        {"watchman", "--output-encoding=bser", "get-sockname"},
        std::move(options)};
#else
  folly::Subprocess proc(
      {"watchman", "--output-encoding=bser", "get-sockname"},
      folly::Subprocess::Options().pipeStdout().pipeStderr().usePath());
#endif

    auto out_pair = proc.communicate();
    auto returnCode = proc.wait();
    if (returnCode.exitStatus() != 0) {
#ifndef _WIN32
      throw WatchmanError{fmt::format(
          "`watchman get-sockname` returned error code {} when called as user {}. Error: {}",
          returnCode.exitStatus(),
          geteuid(),
          out_pair.second)};
#else
      throw WatchmanError{fmt::format(
          "`watchman get-sockname` returned error code {}. Error: {}", returnCode.exitStatus(), out_pair.second)};
#endif
    }
    auto result = parseBser(out_pair.first);

    // Recent versions of watchman include both `unix_domain` and `sockname`
    // fields, however older versions - such as v4.9.0, included in Ubuntu
    // 20.04 - only define `sockname`.
    //
    // Prefer the newer, more specific 'unix_domain', but fall back to
    // 'sockname'.
    if (result.count("unix_domain")) {
      return result["unix_domain"].asString();
    }
    return result["sockname"].asString();
  });
}

Future<dynamic> WatchmanConnection::connect(folly::dynamic versionArgs) {
  if (!versionArgs.isObject()) {
    throw WatchmanError("versionArgs must be object");
  }
  versionCmd_ = folly::dynamic::array("version", versionArgs);

  auto res = getSockPath().thenValue(
      [shared_this = shared_from_this()](std::string&& path) {
        shared_this->eventBase_->runInEventBaseThread([=] {
          folly::SocketAddress addr;
          addr.setFromPath(path);

          shared_this->sock_ =
              folly::AsyncSocket::newSocket(shared_this->eventBase_.get());
          shared_this->sock_->connect(shared_this.get(), addr);
        });

        return shared_this->connectPromise_.getFuture();
      });
  return res;
}

void WatchmanConnection::close() {
  if (closing_) {
    return;
  }
  closing_ = true;
  if (sock_) {
    eventBase_->runImmediatelyOrRunInEventBaseThreadAndWait([this] {
      // This implicitly closes the connection without flushing outstanding
      // writes. Should be fine as Watchman is mostly just providing info, so
      // an incomplete partial write isn't a problem. Doing a fully flushing
      // close here might be the cause a deadlock accessing the event base.
      sock_.reset();
    });
  }
  failQueuedCommands(make_exception_wrapper<WatchmanError>(
      "WatchmanConnection::close() was called"));
}

// The convention for Watchman responses is that they represent
// an error if they contain the "error" key.  We want to report
// those as exceptions, but it is easier to do that via a Try
Try<dynamic> WatchmanConnection::watchmanResponseToTry(dynamic&& value) {
  auto error = value.get_ptr(kError);
  if (error) {
    return Try<dynamic>(make_exception_wrapper<WatchmanResponseError>(value));
  }
  return Try<dynamic>(std::move(value));
}

void WatchmanConnection::connectSuccess() noexcept {
  try {
    sock_->setReadCB(this);
    sock_->setCloseOnExec();

    run(versionCmd_)
        .thenValue([shared_this = shared_from_this()](dynamic&& result) {
          // If there is no "capabilities" key then the version of
          // watchman is too old; treat this as an error
          if (!result.get_ptr(kCapabilities)) {
            result["error"] =
                "This watchman server has no support for capabilities, "
                "please upgrade to the current stable version of watchman";
            shared_this->connectPromise_.setTry(
                shared_this->watchmanResponseToTry(std::move(result)));
            return;
          }
          shared_this->connectPromise_.setValue(std::move(result));
        })
        .thenError(
            [shared_this = shared_from_this()](folly::exception_wrapper&& e) {
              shared_this->connectPromise_.setException(std::move(e));
            });
  } catch (...) {
    connectPromise_.setException(
        folly::exception_wrapper(std::current_exception()));
  }
}

void WatchmanConnection::connectErr(
    const folly::AsyncSocketException& ex) noexcept {
  connectPromise_.setException(ex);
}

WatchmanConnection::QueuedCommand::QueuedCommand(const dynamic& command)
    : cmd(command) {}

Future<dynamic> WatchmanConnection::run(const dynamic& command) noexcept {
  auto cmd = std::make_shared<QueuedCommand>(command);
  if (broken_) {
    cmd->promise.setException(WatchmanError("The connection was broken"));
    return cmd->promise.getFuture();
  }
  if (!sock_) {
    cmd->promise.setException(WatchmanError(
        "No socket (did you call connect() and check result for exceptions?)"));
    return cmd->promise.getFuture();
  }

  bool shouldWrite;
  {
    std::lock_guard<std::mutex> g(mutex_);
    // We only need to call sendCommand if we don't have a command in
    // progress; the completion handler will trigger it once we receive
    // the response
    shouldWrite = commandQ_.empty();
    commandQ_.push_back(cmd);
  }

  if (shouldWrite) {
    eventBase_->runInEventBaseThread(
        [shared_this = shared_from_this()] { shared_this->sendCommand(); });
  }

  return cmd->promise.getFuture();
}

// Generate a failure for all queued commands
void WatchmanConnection::failQueuedCommands(folly::exception_wrapper&& ex) {
  std::lock_guard<std::mutex> g(mutex_);
  auto q = commandQ_;
  commandQ_.clear();

  broken_ = true;
  for (auto& cmd : q) {
    if (!cmd->promise.isFulfilled()) {
      cmd->promise.setException(ex);
    }
  }

  // If the user has explicitly closed the connection no need for callback
  if (callback_ && !closing_) {
    cpuExecutor_->add([shared_this = shared_from_this(), ex = std::move(ex)] {
      // Make sure we weren't asked to close between the time we fired this
      // callback and when the callback ran.
      if (!shared_this->closing_) {
        (*(shared_this->callback_))(folly::Try<folly::dynamic>(std::move(ex)));
      }
    });
  }
}

// Sends the next eligible command to the Watchman service
void WatchmanConnection::sendCommand(bool pop) {
  std::shared_ptr<QueuedCommand> cmd;

  {
    std::lock_guard<std::mutex> g(mutex_);

    if (pop) {
      // We finished processing this one, discard it and focus
      // on the next item, if any.
      commandQ_.pop_front();
    }
    if (commandQ_.empty()) {
      return;
    }
    cmd = commandQ_.front();
  }

  sock_->writeChain(this, toBserIOBuf(cmd->cmd, serialization_opts()));
}

void WatchmanConnection::popAndSendCommand() {
  sendCommand(/* pop = */ true);
}

// Called when AsyncSocket::writeChain completes
void WatchmanConnection::writeSuccess() noexcept {
  // Don't care particularly
}

// Called when AsyncSocket::writeChain fails
void WatchmanConnection::writeErr(
    size_t,
    const folly::AsyncSocketException& ex) noexcept {
  failQueuedCommands(ex);
}

// Called when AsyncSocket wants to give us data
void WatchmanConnection::getReadBuffer(void** bufReturn, size_t* lenReturn) {
  std::lock_guard<std::mutex> g(mutex_);
  const auto ret = bufQ_.preallocate(2048, 2048);
  *bufReturn = ret.first;
  *lenReturn = ret.second;
}

// Called when AsyncSocket gave us data
void WatchmanConnection::readDataAvailable(size_t len) noexcept {
  {
    std::lock_guard<std::mutex> g(mutex_);
    bufQ_.postallocate(len);
  }
  cpuExecutor_->add([shared_this = shared_from_this()] {
    shared_this->decodeNextResponse();
  });
}

std::unique_ptr<folly::IOBuf> WatchmanConnection::splitNextPdu() {
  std::lock_guard<std::mutex> g(mutex_);
  if (!bufQ_.front()) {
    return nullptr;
  }

  // Do we have enough data to decode the next item?
  size_t pdu_len = 0;
  try {
    pdu_len = decodePduLength(bufQ_.front());
  } catch (const std::out_of_range&) {
    // Don't have enough data yet
    return nullptr;
  }

  if (pdu_len > bufQ_.chainLength()) {
    // Don't have enough data yet
    return nullptr;
  }

  // Remove the PDU blob from the front of the chain
  return bufQ_.split(pdu_len);
}

// Try to peel off one or more PDU's from our buffer queue.
// Decode each complete PDU from BSER -> dynamic and dispatch
// either the associated QueuedCommand or to the callback_ for
// unilateral responses.
// This is executed via the cpuExecutor.  We only allow one
// thread to carry out the decoding at a time so that the callbacks
// are triggered in the order that they are received.  It is possible
// for us to receive a large PDU followed by a small one and for the
// small one to finish decoding before the large one, so we must
// serialize the dispatching.
void WatchmanConnection::decodeNextResponse() {
  if (decoding_.exchange(true)) {
    return;
  }
  SCOPE_EXIT {
    decoding_.store(false);
  };

  while (true) {
    auto pdu = splitNextPdu();
    if (!pdu) {
      return;
    }

    try {
      auto decoded = parseBser(pdu.get());

      bool is_unilateral = false;
      // Check for a unilateral response
      for (const auto& k : kUnilateralLabels) {
        if (decoded.get_ptr(k)) {
          // This is a unilateral response
          if (callback_.has_value()) {
            callback_.value()(watchmanResponseToTry(std::move(decoded)));
            is_unilateral = true;
            break;
          }
          // No callback; usage error :-/
          failQueuedCommands(
              std::runtime_error("No unilateral callback has been installed"));
          return;
        }
      }
      if (is_unilateral) {
        continue;
      }

      // It's actually a command response; get the cmd so that we
      // can fulfil its promise
      std::shared_ptr<QueuedCommand> cmd;
      {
        std::lock_guard<std::mutex> g(mutex_);
        if (commandQ_.empty()) {
          failQueuedCommands(
              std::runtime_error("No commands have been queued"));
          return;
        }
        cmd = commandQ_.front();
      }

      // Dispatch outside of the lock in case it tries to send another
      // command
      cmd->promise.setTry(watchmanResponseToTry(std::move(decoded)));

      // Now we're in a position to send the next queued command.
      // We remove it after dispatching the try above in case that
      // queued up more commands; we want to be the one thing that
      // is responsible for sending the next queued command here
      popAndSendCommand();
    } catch (...) {
      failQueuedCommands(folly::exception_wrapper{std::current_exception()});
      return;
    }
  }
}

// Called when AsyncSocket hits EOF
void WatchmanConnection::readEOF() noexcept {
  failQueuedCommands(
      std::system_error(ENOTCONN, std::system_category(), "connection closed"));
}

// Called when AsyncSocket has a read error
void WatchmanConnection::readErr(
    const folly::AsyncSocketException& ex) noexcept {
  failQueuedCommands(ex);
}
} // namespace watchman
