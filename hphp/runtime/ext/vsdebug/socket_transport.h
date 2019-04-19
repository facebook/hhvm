/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_VSDEBUG_SOCKET_TRANSPORT_H_
#define incl_HPHP_VSDEBUG_SOCKET_TRANSPORT_H_

#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/transport.h"

namespace HPHP {
namespace VSDEBUG {

struct Debugger;

struct SocketTransportOptions {
  std::string domainSocketPath;
  int tcpListenPort;
};

// SocketTransport transport speaks to the debugger client via a TCP socket
// listening on a predetermined port.
struct SocketTransport : public DebugTransport {
  SocketTransport(Debugger* debugger, const SocketTransportOptions& options);
  virtual ~SocketTransport();

  void onClientDisconnected() override;
  bool clientConnected() const override;
  void cleanupFd(int fd) override;

private:

  void createAbortPipe();
  void listenForClientConnection();
  bool setSocketPermissions(const char* path);

  bool bindAndListenTCP(
    struct addrinfo* address,
    std::vector<int>& socketFds
  );

  bool bindAndListenDomain(std::vector<int>& socketFds);
  bool useDomainSocket() const;

  static bool validatePeerCreds(int fd, ClientInfo& info);

  void waitForConnection(
    std::vector<int>& socketFds,
    int abortFd
  );

  void stopConnectionThread();

  enum RejectReason {
    None,
    ClientAlreadyAttached,
    AuthenticationFailed
  };

  static void rejectClientWithMsg(
    int newFd,
    int abortFd,
    RejectReason reason,
    ClientInfo& existingClientInfo
  );

  static void shutdownSocket(int sockFd, int abortFd);

  mutable Mutex m_lock;
  bool m_terminating;
  bool m_clientConnected;
  int m_listenPort;
  std::string m_domainSocketPath;
  int m_abortPipeFd[2] {-1, -1};
  AsyncFunc<SocketTransport> m_connectThread;

  // Information about the connected client ID, only available
  // when using a UNIX domain socket connection.
  ClientInfo m_clientInfo;
};

}
}

#endif // incl_HPHP_VSDEBUG_SOCKET_TRANSPORT_H_
