/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ListenSocket.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <glog/logging.h>

#include <folly/Conv.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>
#include <folly/portability/Sockets.h>

#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {

std::pair<int, uint16_t> createAndBind(
    uint16_t port,
    bool zeroCopyEnable = false) {
  struct addrinfo hints;
  struct addrinfo* res;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  auto portStr = folly::to<std::string>(port);
  if (::getaddrinfo(nullptr, portStr.data(), &hints, &res)) {
    hints.ai_family = AF_INET;
    auto ret = ::getaddrinfo(nullptr, portStr.data(), &hints, &res);
    checkRuntime(!ret, "Failed to find a local IP: {}", ::gai_strerror(ret));
  }
  SCOPE_EXIT {
    ::freeaddrinfo(res);
  };

  auto socketFd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (socketFd < 0) {
    throwRuntime(
        "Failed to create a socket for port {}: {}",
        port,
        folly::errnoStr(errno));
  }
  if (::bind(socketFd, res->ai_addr, res->ai_addrlen) != 0) {
    auto errStr = folly::errnoStr(errno);
    ::close(socketFd);
    throwRuntime("Failed to bind a socket for port {}: {}", port, errStr);
  }

  int val = 1;
  if (zeroCopyEnable &&
      setsockopt(socketFd, SOL_SOCKET, SO_ZEROCOPY, &val, sizeof(val))) {
    auto errStr = folly::errnoStr(errno);
    ::close(socketFd);
    throwRuntime("Failed enable zero copy: {}", errStr);
  }

  struct sockaddr_in addr;
  socklen_t len = sizeof(struct sockaddr_in);
  if (::getsockname(socketFd, (struct sockaddr*)&addr, &len) != 0) {
    auto errStr = folly::errnoStr(errno);
    ::close(socketFd);
    throwRuntime("Failed to get socket name for port {}: {}", port, errStr);
  }

  return std::make_pair(socketFd, ntohs(addr.sin_port));
}

ListenSocket::ListenSocket(bool zeroCopyEnabled) {
  auto sockPort = createAndBind(0, zeroCopyEnabled);
  socketFd_ = sockPort.first;
  port_ = sockPort.second;
  if (::listen(socketFd_, SOMAXCONN) != 0) {
    throwRuntime(
        "Failed to listen on a socket for port {}: {}",
        port_,
        folly::errnoStr(errno));
  }

  VLOG(1) << "Listening on " << socketFd_ << ", port " << port_;
}

void ListenSocket::setCloseOnExec(bool value) {
  if (socketFd_ < 0) {
    return;
  }

  // Read the current flags
  int old_flags = ::fcntl(socketFd_, F_GETFD, 0);
  if (old_flags < 0) {
    return;
  }

  // Set just the flag we want to set
  int new_flags;
  if (value) {
    new_flags = old_flags | FD_CLOEXEC;
  } else {
    new_flags = old_flags & ~FD_CLOEXEC;
  }

  ::fcntl(socketFd_, F_SETFD, new_flags);
}

ListenSocket::ListenSocket(ListenSocket&& other) noexcept
    : socketFd_(other.socketFd_), port_(other.port_) {
  other.socketFd_ = -1;
  other.port_ = 0;
}

ListenSocket& ListenSocket::operator=(ListenSocket&& other) noexcept {
  if (this != &other) {
    if (socketFd_ > 0) {
      ::close(socketFd_);
    }
    socketFd_ = other.socketFd_;
    port_ = other.port_;
    other.socketFd_ = -1;
    other.port_ = 0;
  }
  return *this;
}

ListenSocket::~ListenSocket() {
  if (socketFd_ >= 0) {
    ::close(socketFd_);
  }
}

/**
 * @return true  if port is open, false otherwise
 */
bool isPortOpen(uint16_t port) {
  try {
    auto sockPort = createAndBind(port);
    ::close(sockPort.first);
    return false;
  } catch (const std::runtime_error&) {
    return true;
  }
}
} // namespace memcache
} // namespace facebook
