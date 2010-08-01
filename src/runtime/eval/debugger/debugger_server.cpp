/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/eval/debugger/debugger_server.h>
#include <runtime/eval/debugger/debugger.h>
#include <runtime/base/runtime_option.h>
#include <util/network.h>
#include <util/logger.h>

#define POLLING_SECONDS 1

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DebuggerServer DebuggerServer::s_debugger_server;

void DebuggerServer::Start() {
  if (RuntimeOption::EnableDebuggerServer) {
    Debugger::SetTextColors();
    s_debugger_server.start();
  }
}

void DebuggerServer::Stop() {
  if (RuntimeOption::EnableDebuggerServer) {
    s_debugger_server.stop();
  }
}

///////////////////////////////////////////////////////////////////////////////

DebuggerServer::DebuggerServer()
    : m_serverThread(this, &DebuggerServer::accept), m_stopped(false) {
}

void DebuggerServer::start() {
  m_serverThread.start();
}

void DebuggerServer::stop() {
  m_stopped = true;
  m_serverThread.waitForEnd();
}

void DebuggerServer::accept() {
  int port = RuntimeOption::DebuggerServerPort;
  int backlog = 128;

  Util::HostEnt result;
  if (!Util::safe_gethostbyname("0.0.0.0", result)) {
    return;
  }
  struct sockaddr_in la;
  memcpy((char*)&la.sin_addr, result.hostbuf.h_addr,
         result.hostbuf.h_length);
  la.sin_family = result.hostbuf.h_addrtype;
  la.sin_port = htons((unsigned short)port);

  Socket *sock = new Socket(socket(PF_INET, SOCK_STREAM, 0), PF_INET,
                            "0.0.0.0", port);
  Object deleter(sock);
  if (!sock->valid()) {
    Logger::Error("unable to create debugger server socket");
    return;
  }
  if (bind(sock->fd(), (struct sockaddr *)&la, sizeof(la)) < 0) {
    Logger::Error("unable to bind to port %d for debugger server", port);
    return;
  }
  if (listen(sock->fd(), backlog) < 0) {
    Logger::Error("unable to listen on port %d for debugger server", port);
    return;
  }

  // server loop
  while (!m_stopped) {
    struct pollfd fds[1];
    fds[0].fd = sock->fd();
    fds[0].events = POLLIN|POLLERR|POLLHUP;
    if (poll(fds, 1, POLLING_SECONDS * 1000) == 1 &&
        (fds[0].revents & POLLIN)) {
      struct sockaddr sa;
      socklen_t salen = sizeof(sa);
      Socket *new_sock = new Socket(::accept(sock->fd(), &sa, &salen),
                                    sock->getType());
      SmartPtr<Socket> ret(new_sock);
      if (!new_sock->valid()) {
        Logger::Error("unable to accept incoming debugger request");
        break;
      }
      Debugger::RegisterProxy(ret, false);
    } // else timed out, then we have a chance to check m_stopped bit
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
