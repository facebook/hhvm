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

#include "hphp/runtime/server/server.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/server/satellite-server.h"
#include "hphp/runtime/base/preg.h"
#include <signal.h>

///////////////////////////////////////////////////////////////////////////////
// statics

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool Server::StackTraceOnError = true;
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
    if (name == "libevent") {
      throw ServerException(
        "HHVM no longer supports the built-in webserver as of 3.0.0. Please "
        "use your own webserver (nginx or apache) talking to HHVM over "
        "fastcgi. https://github.com/facebook/hhvm/wiki/FastCGI");
    }
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
