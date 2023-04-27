/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Expected.h>
#include <folly/Range.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>

namespace folly {
class AsyncSocketException;
class EventBase;
} // namespace folly

namespace facebook {
namespace memcache {

struct ConnectionOptions;

enum class ConnectionState {
  Up, // Connection is open and we can write into it.
  Down, // Connection is not open (or close), we need to reconnect.
  Connecting, // Currently connecting.
  Error // Currently processing error.
};

/**
 * Creates a socket based on the given options. The socket will be attached to
 * the given Eventbase.
 */
folly::Expected<
    folly::AsyncTransportWrapper::UniquePtr,
    folly::AsyncSocketException>
createSocket(
    folly::EventBase& eventBase,
    const ConnectionOptions& connectionOptions);

/**
 * Like createSocket(), but instead creates a AsyncSocket-based socket that
 * plays well with Thrift clients and channels; all AsyncSocketExceptions are
 * automatically converted into TAsyncTransportExceptions.
 */
folly::Expected<
    folly::AsyncTransportWrapper::UniquePtr,
    folly::AsyncSocketException>
createAsyncSocket(
    folly::EventBase& eventBase,
    const ConnectionOptions& connectionOptions);
/**
 * Get the socket address based on the given options.
 */
folly::Expected<folly::SocketAddress, folly::AsyncSocketException>
getSocketAddress(const ConnectionOptions& connectionOptions);

/**
 * Create the socket options map based on the given ConnectionOptions and
 * SocketAddress.
 */
folly::SocketOptionMap createSocketOptions(
    const folly::SocketAddress& address,
    const ConnectionOptions& connectionOptions);

/**
 * Checks whether of not QoS was successfully applied to the given socket.
 * If QoS wasn't applied successfully (or if the getsockopt syscall fails),
 * this function will log failure.
 */
void checkWhetherQoSIsApplied(
    int socketFd,
    const folly::SocketAddress& address,
    const ConnectionOptions& connectionOptions,
    folly::StringPiece transportName);

} // namespace memcache
} // namespace facebook
