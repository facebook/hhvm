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

#include <thrift/lib/cpp2/fast_thrift/rocket/server/connection/ConnectionHandler.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <string>
#include <utility>

#if defined(__linux__)
#include <linux/filter.h>
#include <sys/socket.h>
#endif

#include <folly/String.h>
#include <folly/logging/xlog.h>
#include <folly/net/NetworkSocket.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzHandshakeHelper.h>

namespace apache::thrift::fast_thrift::rocket::server::connection {

ConnectionHandler::ConnectionHandler(
    folly::EventBase& evb,
    ConnectionFactory connectionFactory,
    std::shared_ptr<const fizz::server::FizzServerContext> fizzContext,
    std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams,
    std::chrono::milliseconds tlsHandshakeTimeout)
    : evb_(folly::getKeepAliveToken(&evb)),
      socket_(new folly::AsyncServerSocket(evb_.get())),
      connectionFactory_(std::move(connectionFactory)),
      fizzContext_(std::move(fizzContext)),
      thriftParams_(std::move(thriftParams)),
      tlsHandshakeTimeout_(tlsHandshakeTimeout) {}

ConnectionHandler::~ConnectionHandler() {
  if (FOLLY_LIKELY(socket_)) {
    evb_->runImmediatelyOrRunInEventBaseThreadAndWait(
        [socket = std::move(socket_)]() mutable { socket.reset(); });
  }
}

void ConnectionHandler::connectionAccepted(
    folly::NetworkSocket fd,
    const folly::SocketAddress& clientAddr,
    AcceptInfo) noexcept {
  if (state_ != State::ACCEPTING) {
    folly::netops::close(fd);
    return;
  }

  XLOG(DBG3) << "Connection accepted from " << clientAddr.describe();

  auto socket = folly::AsyncSocket::newSocket(evb_.get(), fd);

  if (!fizzContext_) {
    installConnection(folly::AsyncTransport::UniquePtr(socket.release()));
    return;
  }

  security::PendingHandshakes::HelperPtr helper(
      new security::FizzHandshakeHelper(
          std::move(socket),
          fizzContext_,
          thriftParams_,
          tlsHandshakeTimeout_,
          pendingHandshakes_,
          [this](
              folly::AsyncTransport::UniquePtr negotiated,
              const folly::exception_wrapper& ex) noexcept {
            if (ex || !negotiated) {
              XLOG(DBG3) << "TLS handshake failed: "
                         << (ex ? ex.what().toStdString()
                                : std::string("null"));
              return;
            }
            if (state_ != State::ACCEPTING) {
              return;
            }
            installConnection(std::move(negotiated));
          }));
  auto* rawHelper = helper.get();
  pendingHandshakes_.add(std::move(helper));
  rawHelper->start();
}

void ConnectionHandler::acceptError(folly::exception_wrapper ew) noexcept {
  XLOG(ERR) << "Accept error: " << ew.what();
}

void ConnectionHandler::acceptStopped() noexcept {
  DestructorGuard dg(this);
  XLOG(DBG3) << "Accept stopped";
  state_ = State::STOPPED;
  pendingHandshakes_.cancelAll();
  closeAllConnections();

  // Release the guard - this may trigger deferred destruction
  stoppingDestroyGuard_.reset();
}

void ConnectionHandler::startAccepting(const folly::SocketAddress& address) {
  DestructorGuard dg(this);

  DCHECK(state_ == State::NONE);
  DCHECK(socket_);
  socket_->setReusePortEnabled(true);
  socket_->bind(address);
  socket_->listen(1024);
  if (enableReusePortBpfSpread_) {
    attachReusePortBpfSpread();
  }
  socket_->addAcceptCallback(this, evb_.get());
  socket_->startAccepting();
  state_ = State::ACCEPTING;
}

void ConnectionHandler::stopAccepting() {
  if (state_ != State::ACCEPTING) {
    return;
  }

  DCHECK(socket_);
  state_ = State::STOPPING;

  // Hold a guard to keep the object alive until acceptStopped() is called
  stoppingDestroyGuard_.emplace(this);

  socket_->removeAcceptCallback(this, evb_.get());
}

void ConnectionHandler::closeAllConnections() {
  for (auto& conn : connections_) {
    conn.close(folly::exception_wrapper{});
  }
  connections_.clear();
}

size_t ConnectionHandler::connectionCount() {
  size_t count = 0;
  evb_->runImmediatelyOrRunInEventBaseThreadAndWait(
      [&count, this] { count = connections_.size(); });
  return count;
}

void ConnectionHandler::getAddress(folly::SocketAddress* address) const {
  socket_->getAddress(address);
}

void ConnectionHandler::destroy() {
  if (state_ == State::STOPPED || state_ == State::NONE) {
    onDelayedDestroy(false);
  } else {
    destroyPending_ = true;
    if (state_ == State::ACCEPTING) {
      // Schedule destruction after acceptStopped callback completes
      evb_->runInEventBaseThread([this] { stopAccepting(); });
      return;
    }
    if (state_ == State::STOPPING) {
      // Still waiting for acceptStopped callback
      return;
    }
  }
}

void ConnectionHandler::onDelayedDestroy(bool delayed) {
  if (delayed && !destroyPending_) {
    return;
  }
  // State is STOPPED or NONE - safe to delete
  delete this;
}

void ConnectionHandler::installConnection(
    folly::AsyncTransport::UniquePtr socket) {
  auto conn = connectionFactory_(std::move(socket));

  auto* rawTransportHandler = conn.transportHandler.get();
  conn.transportHandler->setCloseCallback(
      [this, rawTransportHandler]() { removeConnection(rawTransportHandler); });

  connections_.emplace_back(std::move(conn));

  rawTransportHandler->onConnect();
}

void ConnectionHandler::removeConnection(
    transport::TransportHandler* transportHandler) {
  DestructorGuard dg(this);

  auto it = std::find_if(
      connections_.begin(),
      connections_.end(),
      [transportHandler](const RocketServerConnection& conn) {
        return conn.transportHandler.get() == transportHandler;
      });
  if (it != connections_.end()) {
    it->close(folly::exception_wrapper{});
    connections_.erase(it);
  }
}

void ConnectionHandler::attachReusePortBpfSpread() {
  // Replace the kernel's default 4-tuple-hash REUSEPORT selection with a
  // 2-instruction cBPF program returning a random u32 — kernel mods by
  // group size internally, so the program is group-size oblivious. Only
  // one socket in the REUSEPORT group needs the program attached, but
  // every worker installs the same identical program on its own fd;
  // last-write-wins is harmless. Errors are logged but non-fatal: a
  // failed attach leaves the kernel's default selector in place, which
  // is exactly the pre-change behavior.
#if defined(__linux__)
  auto code = std::to_array<sock_filter>({
      BPF_STMT(
          BPF_LD | BPF_W | BPF_ABS,
          static_cast<uint32_t>(SKF_AD_OFF + SKF_AD_RANDOM)),
      BPF_STMT(BPF_RET | BPF_A, 0),
  });
  struct sock_fprog prog = {
      .len = static_cast<unsigned short>(code.size()),
      .filter = code.data(),
  };
  for (auto fd : socket_->getNetworkSockets()) {
    if (::setsockopt(
            fd.toFd(),
            SOL_SOCKET,
            SO_ATTACH_REUSEPORT_CBPF,
            &prog,
            sizeof(prog)) != 0) {
      const int savedErrno = errno;
      XLOGF_EVERY_MS(
          ERR,
          1000,
          "SO_ATTACH_REUSEPORT_CBPF failed on fd {}: errno={} ({})",
          fd.toFd(),
          savedErrno,
          folly::errnoStr(savedErrno));
    }
  }
#endif
}

} // namespace apache::thrift::fast_thrift::rocket::server::connection
