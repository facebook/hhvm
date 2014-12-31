/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/server/fastcgi/socket-connection.h"
#include "hphp/util/assertions.h"
#include <folly/io/IOBuf.h>
#include "thrift/lib/cpp/async/TAsyncTransport.h"
#include <folly/SocketAddress.h>
#include "thrift/lib/cpp/transport/TTransportException.h"
#include <folly/wangle/acceptor/ManagedConnection.h>

namespace HPHP {

using folly::IOBuf;
using folly::EventBase;
using apache::thrift::async::TAsyncTransport;
using apache::thrift::async::TAsyncTimeout;
using folly::SocketAddress;
using apache::thrift::transport::TTransportException;

///////////////////////////////////////////////////////////////////////////////

SocketConnection::SocketConnection(
  TAsyncTransport::UniquePtr sock,
  const SocketAddress& localAddr,
  const SocketAddress& peerAddr)
  : m_localAddr(localAddr),
    m_peerAddr(peerAddr),
    m_sock(std::move(sock)) {}

SocketConnection::~SocketConnection() {
  assert(!m_sock->getReadCallback());
  close();
}

void SocketConnection::timeoutExpired() noexcept {
  close();
}

void SocketConnection::close() {
  m_sock->close();
}

void SocketConnection::describe(std::ostream& os) const {
  os << "[peerAddr: " << m_peerAddr << ", localAddr: " << m_localAddr << "]";
}

bool SocketConnection::isBusy() const {
  return false;
}

void SocketConnection::notifyPendingShutdown() {
}

void SocketConnection::closeWhenIdle() {
}

void SocketConnection::dropConnection() {
  m_sock->closeWithReset();
}

void SocketConnection::dumpConnectionState(uint8_t loglevel) {}

///////////////////////////////////////////////////////////////////////////////
}
