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

#include "hphp/runtime/server/server.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/server/satellite-server.h"
#include "hphp/runtime/base/preg.h"
#include <signal.h>

///////////////////////////////////////////////////////////////////////////////
// statics

static HPHP::ServerPtrVec AllServers;
static void on_kill(int sig) {
  signal(sig, SIG_DFL);
  for (unsigned int i = 0; i < AllServers.size(); i++) {
    AllServers[i]->stop();
  }
  raise(sig);
}

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool Server::StackTraceOnError = true;

void Server::InstallStopSignalHandlers(ServerPtr server) {
  if (AllServers.empty()) {
    signal(SIGTERM, on_kill);
    signal(SIGUSR1, on_kill);
  }

  AllServers.push_back(server);
}

///////////////////////////////////////////////////////////////////////////////

Server::Server(const std::string &address, int port, int threadCount)
  : m_address(address), m_port(port), m_threadCount(threadCount),
    m_urlChecker(SatelliteServerInfo::checkMainURL),
    m_status(RunStatus::NOT_YET_STARTED) {
}

///////////////////////////////////////////////////////////////////////////////

ServerPtr ServerFactory::createServer(const std::string &address,
                                      uint16_t port,
                                      int numThreads) {
  ServerOptions options(address, port, numThreads);
  return createServer(options);
}

ServerFactoryRegistry::ServerFactoryRegistry()
  : m_lock(false) {
  }

ServerFactoryRegistry *ServerFactoryRegistry::getInstance() {
  static ServerFactoryRegistry singleton;
  return &singleton;
}

ServerPtr ServerFactoryRegistry::createServer(const std::string &type,
                                              const std::string &address,
                                              uint16_t port,
                                              int numThreads) {
  auto factory = getInstance()->getFactory(type);
  ServerOptions options(address, port, numThreads);
  return factory->createServer(options);
}

void ServerFactoryRegistry::registerFactory(const std::string &name,
                                            const ServerFactoryPtr &factory) {
  Lock lock(m_lock);
  auto ret = m_factories.insert(std::make_pair(name, factory));
  if (!ret.second) {
    throw ServerException("a factory already exists for server type \"%s\"",
                          name.c_str());
  }
}

ServerFactoryPtr ServerFactoryRegistry::getFactory(const std::string &name) {
  Lock lock(m_lock);
  auto it = m_factories.find(name);
  if (it == m_factories.end()) {
    throw ServerException("no factory for server type \"%s\"", name.c_str());
  }
  return it->second;
}

///////////////////////////////////////////////////////////////////////////////

ServerException::ServerException(const char *fmt, ...) {
  va_list ap; va_start(ap, fmt); format(fmt, ap); va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////
}
