/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_RUNTIME_SERVER_FASTCGI_SOCKET_CONNECTION_H_
#define incl_HPHP_RUNTIME_SERVER_FASTCGI_SOCKET_CONNECTION_H_

#include "folly/io/IOBuf.h"
#include "thrift/lib/cpp/async/TAsyncTransport.h"
#include "thrift/lib/cpp/transport/TSocketAddress.h"
#include "thrift/lib/cpp/transport/TTransportException.h"
#include "ti/proxygen/lib/services/ManagedConnection.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

class SocketConnection : public facebook::proxygen::ManagedConnection {
public:
  SocketConnection(
    apache::thrift::async::TAsyncTransport::UniquePtr sock,
    const apache::thrift::transport::TSocketAddress& localAddr,
    const apache::thrift::transport::TSocketAddress& peerAddr);
  virtual ~SocketConnection();

  // ManagedConnection
  virtual void timeoutExpired() noexcept;
  virtual void describe(std::ostream& os) const;
  virtual bool isBusy() const;
  virtual void notifyPendingShutdown();
  virtual void dropConnection();
  virtual void dumpConnectionState(uint8_t loglevel);

  virtual bool shouldShutdown() { return false; }
  void shutdownTransport();

protected:
  apache::thrift::transport::TSocketAddress m_localAddr;
  apache::thrift::transport::TSocketAddress m_peerAddr;

  apache::thrift::async::TAsyncTransport::UniquePtr m_sock;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_RUNTIME_SERVER_FASTCGI_SOCKET_CONNECTION_H_

