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

#include "hphp/runtime/debugger/debugger_server.h"
#include "hphp/runtime/debugger/debugger_client.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/util/network.h"
#include "hphp/util/logger.h"

#define POLLING_SECONDS 1

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////
TRACE_SET_MOD(debugger);

DebuggerServer DebuggerServer::s_debugger_server;

bool DebuggerServer::Start() {
  TRACE(2, "DebuggerServer::Start\n");
  if (RuntimeOption::EnableDebuggerServer) {
    if (RuntimeOption::EnableDebuggerColor) {
      Debugger::SetTextColors();

      // Some server commands pre-formatted texts with color for clients.
      // Loading a set of default colors for better display.
      Hdf hdf;
      DebuggerClient::LoadColors(hdf);
    }

    return s_debugger_server.start();
  }
  return true;
}

void DebuggerServer::Stop() {
  TRACE(2, "DebuggerServer::Stop\n");
  if (RuntimeOption::EnableDebuggerServer) {
    s_debugger_server.stop();
  }
}

///////////////////////////////////////////////////////////////////////////////

DebuggerServer::DebuggerServer()
    : m_serverThread(this, &DebuggerServer::accept), m_stopped(false) {
  TRACE(2, "DebuggerServer::DebuggerServer\n");
}

DebuggerServer::~DebuggerServer() {
  TRACE(2, "DebuggerServer::~DebuggerServer\n");
  m_stopped = true;
  m_serverThread.waitForEnd();
}

bool DebuggerServer::start() {
  TRACE(2, "DebuggerServer::start\n");
  int port = RuntimeOption::DebuggerServerPort;
  int backlog = 128;

  Util::HostEnt result;
  if (!Util::safe_gethostbyname("0.0.0.0", result)) {
    return false;
  }
  struct sockaddr_in la;
  memcpy((char*)&la.sin_addr, result.hostbuf.h_addr,
         result.hostbuf.h_length);
  la.sin_family = result.hostbuf.h_addrtype;
  la.sin_port = htons((unsigned short)port);

  m_sock = new Socket(socket(PF_INET, SOCK_STREAM, 0), PF_INET, "0.0.0.0",
                      port);

  int yes = 1;
  setsockopt(m_sock->fd(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  if (!m_sock->valid()) {
    Logger::Error("unable to create debugger server socket");
    return false;
  }
  if (bind(m_sock->fd(), (struct sockaddr *)&la, sizeof(la)) < 0) {
    Logger::Error("unable to bind to port %d for debugger server", port);
    return false;
  }
  if (listen(m_sock->fd(), backlog) < 0) {
    Logger::Error("unable to listen on port %d for debugger server", port);
    return false;
  }

  m_serverThread.start();
  return true;
}

void DebuggerServer::stop() {
  TRACE(2, "DebuggerServer::stop\n");
  m_stopped = true;
  m_serverThread.waitForEnd();
}

void DebuggerServer::accept() {
  TRACE(2, "DebuggerServer::accept\n");
  // Setup server-side usage logging before accepting any connections.
  Debugger::InitUsageLogging();
  // server loop
  while (!m_stopped) {
    struct pollfd fds[1];
    fds[0].fd = m_sock->fd();
    fds[0].events = POLLIN|POLLERR|POLLHUP;
    int ret = poll(fds, 1, POLLING_SECONDS * 1000);
    if (ret > 0) {
      bool in = (fds[0].revents & POLLIN);
      if (in) {
        struct sockaddr sa;
        socklen_t salen = sizeof(sa);
        try {
          Socket *new_sock = new Socket(::accept(m_sock->fd(), &sa, &salen),
                                        m_sock->getType());
          SmartPtr<Socket> ret(new_sock);
          if (new_sock->valid()) {
            Debugger::CreateProxy(ret, false);
          } else {
            Logger::Error("unable to accept incoming debugger request");
          }
        } catch (Exception &e) {
          Logger::Error("%s", e.getMessage().c_str());
        } catch (std::exception &e) {
          Logger::Error("%s", e.what());
        } catch (...) {
          Logger::Error("(unknown exception was thrown)");
        }
      }
    } // else timed out, then we have a chance to check m_stopped bit

    // A chance for some housekeeping...
    Debugger::CleanupRetiredProxies();
  }

  m_sock.reset();
}

///////////////////////////////////////////////////////////////////////////////
}}
