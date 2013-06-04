// Copyright 2004-present Facebook. All Rights Reserved.
#include "hphp/runtime/base/server/libevent_server.h"
#include "hphp/runtime/base/server/libevent_server_with_fd.h"
#include "hphp/runtime/base/server/libevent_server_with_takeover.h"

#include <boost/make_shared.hpp>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class LibEventServerFactory : public ServerFactory {
public:
  LibEventServerFactory() {}

  virtual ServerPtr createServer(const ServerOptions& options);
};

ServerPtr LibEventServerFactory::createServer(const ServerOptions& options) {
  if (options.m_serverFD != -1 || options.m_sslFD != -1) {
    auto const server = boost::make_shared<LibEventServerWithFd>
      (options.m_address, options.m_port, options.m_numThreads,
       options.m_timeout.count());
    server->setServerSocketFd(options.m_serverFD);
    server->setSSLSocketFd(options.m_sslFD);
    return server;
  }

  if (!options.m_takeoverFilename.empty()) {
    auto const server = boost::make_shared<LibEventServerWithTakeover>
      (options.m_address, options.m_port, options.m_numThreads,
       options.m_timeout.count());
    server->setTransferFilename(options.m_takeoverFilename);
    return server;
  }

  return boost::make_shared<LibEventServer>(options.m_address, options.m_port,
                                            options.m_numThreads,
                                            options.m_timeout.count());
}

///////////////////////////////////////////////////////////////////////////////
}

extern "C" {

/*
 * Automatically register LibEventServerFactory on program start
 */
void register_libevent_server() __attribute__((constructor));
void register_libevent_server() {
  auto registry = HPHP::ServerFactoryRegistry::getInstance();
  auto factory = boost::make_shared<HPHP::LibEventServerFactory>();
  registry->registerFactory("libevent", factory);
}

}
