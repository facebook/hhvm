/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <chrono>
#include <string>

#include <folly/Range.h>

namespace facebook {
namespace memcache {

class ClientSocket {
 public:
  static constexpr size_t kMaxReplySize = 1000;

  /**
   * @throws std::runtime_error  if failed to create a socket and connect
   */
  explicit ClientSocket(uint16_t port);
  ClientSocket(const std::string& hostName, uint16_t port);
  ~ClientSocket();

  /**
   * Write data to socket.
   *
   * @param data  data to write
   * @param timeout  max time to wait for write
   *
   * @throws std::runtime_error  if failed to write data
   */
  void write(
      folly::StringPiece data,
      std::chrono::milliseconds timeout = std::chrono::seconds(1));

  /**
   * Send a request and receive a reply of `replySize`.
   *
   * @param request  string to send
   * @param timeout  max time to wait for send/receive
   *
   * @throws std::runtime_error  if failed to send/receive data
   */
  std::string sendRequest(
      folly::StringPiece request,
      size_t replySize,
      std::chrono::milliseconds timeout = std::chrono::seconds(1));
  std::string sendRequest(
      folly::StringPiece request,
      std::chrono::milliseconds timeout = std::chrono::seconds(1));

  // movable, but not copyable
  ClientSocket(ClientSocket&& other) noexcept;
  ClientSocket& operator=(ClientSocket&& other) noexcept;
  ClientSocket(const ClientSocket&) = delete;
  ClientSocket& operator=(const ClientSocket&) = delete;

 private:
  void setupSocket(struct addrinfo*, uint16_t port);
  int socketFd_{-1};
};
} // namespace memcache
} // namespace facebook
