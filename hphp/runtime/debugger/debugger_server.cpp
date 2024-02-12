/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <exception>

#include <folly/portability/Sockets.h>

#include "hphp/runtime/debugger/debugger_client.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/base/configs/debugger.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/util/network.h"
#include "hphp/util/logger.h"

#define POLLING_SECONDS 1

namespace HPHP::Eval {
///////////////////////////////////////////////////////////////////////////////
TRACE_SET_MOD(debugger);

DebuggerServer DebuggerServer::s_debugger_server;

bool DebuggerServer::Start() {
  TRACE(2, "DebuggerServer::Start\n");
  if (Cfg::Debugger::EnableServer) {
    if (Cfg::Debugger::EnableColor) {
      Debugger::SetTextColors();

      // Some server commands pre-formatted texts with color for clients.
      // Loading a set of default colors for better display.
      IniSetting::Map ini = IniSetting::Map::object;
      Hdf hdf;
      DebuggerClient::LoadColors(ini, hdf);
    }

    return s_debugger_server.start();
  }
  return true;
}

void DebuggerServer::Stop() {
  TRACE(2, "DebuggerServer::Stop\n");
  if (Cfg::Debugger::EnableServer) {
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
  int port = Cfg::Debugger::ServerPort;
  int backlog = 128;

  struct addrinfo hint;
  struct addrinfo *ai;
  memset(&hint, 0, sizeof(hint));
  hint.ai_family = AF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;
  hint.ai_flags = AI_PASSIVE;
  if (Cfg::Debugger::DisableIPv6) {
    hint.ai_family = AF_INET;
  }

  const auto nodename = Cfg::Debugger::ServerIP.empty()
    ? "localhost"
    : Cfg::Debugger::ServerIP.c_str();
  if (getaddrinfo(nodename, std::to_string(port).c_str(), &hint, &ai)) {
    Logger::Error("unable to get address information");
    return false;
  }

  SCOPE_EXIT {
    freeaddrinfo(ai);
  };

  /* use a cur pointer so we still have ai to be able to free the struct */
  struct addrinfo *cur;
  for (cur = ai; cur; cur = cur->ai_next) {
    int s_fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
    if (s_fd < 0 && errno == EAFNOSUPPORT) {
      continue;
    }
    auto m_sock = req::make<StreamSocket>(
      s_fd, cur->ai_family, cur->ai_addr->sa_data, port);

    int yes = 1;
    setsockopt(m_sock->fd(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (!m_sock->valid()) {
      Logger::Error("unable to create debugger server socket");
      return false;
    }

    if (cur->ai_family == AF_INET6) {
      int on = 1;
      setsockopt(m_sock->fd(), IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
    }

    if (bind(m_sock->fd(), cur->ai_addr, cur->ai_addrlen) < 0) {
      Logger::Error("unable to bind to port %d for debugger server", port);
      return false;
    }
    if (listen(m_sock->fd(), backlog) < 0) {
      Logger::Error("unable to listen on port %d for debugger server", port);
      return false;
    }

    m_socks.push_back(m_sock->getData());
  }

  if (m_socks.size() == 0) {
    Logger::Error("Did not bind to any sockets on port %d", port);
    return false;
  }

  m_serverThread.start();
  return true;
}

void DebuggerServer::stop() {
  TRACE(2, "DebuggerServer::stop\n");
  m_stopped = true;
  m_serverThread.waitForEnd();
  m_socks.clear();
}

void DebuggerServer::accept() {
  g_context.getCheck();
  TRACE(2, "DebuggerServer::accept\n");
  // Setup server-side usage logging before accepting any connections.
  Debugger::InitUsageLogging();
  // server loop
  unsigned int count = m_socks.size();
  struct pollfd* fds = (struct pollfd*)alloca(sizeof(struct pollfd) * count);

  for (unsigned int i = 0; i < count; i++) {
    fds[i].fd = nthSocket(i)->fd();
    fds[i].events = POLLIN|POLLERR|POLLHUP;
  }

  while (!m_stopped) {
    int ret = poll(fds, count, POLLING_SECONDS * 1000);
    for (unsigned int i = 0; ret > 0 && i < count; i++) {
      bool in = (fds[i].revents & POLLIN);
      if (in) {
        struct sockaddr sa;
        socklen_t salen = sizeof(sa);
        try {
          auto sock = nthSocket(i);
          auto new_sock = req::make<StreamSocket>(
            ::accept(sock->fd(), &sa, &salen), sock->getType());
          if (new_sock->valid()) {
            Debugger::CreateProxy(new_sock, false);
          } else {
            Logger::Error("unable to accept incoming debugger request");
          }
        } catch (Exception& e) {
          Logger::Error(e.getMessage());
        } catch (std::exception& e) {
          Logger::Error(std::string{e.what()});
        } catch (...) {
          Logger::Error("(unknown exception was thrown)");
        }
      }

      fds[i].revents = 0; // reset the POLLIN flag
    } // else timed out, then we have a chance to check m_stopped bit

    // A chance for some housekeeping...
    Debugger::CleanupRetiredProxies();
  }

  for(auto &m_sock : m_socks) {
    m_sock.reset();
  }
  hphp_memory_cleanup();
}

///////////////////////////////////////////////////////////////////////////////
}
