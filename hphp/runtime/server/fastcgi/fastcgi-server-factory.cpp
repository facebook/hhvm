/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/server/fastcgi/fastcgi-server.h"

namespace HPHP {

struct FastCGIServerFactory : ServerFactory {
  FastCGIServerFactory() {}

  virtual ServerPtr createServer(const ServerOptions& options) override {
    return folly::make_unique<FastCGIServer>(options.m_address,
                                             options.m_port,
                                             options.m_numThreads,
                                             options.m_useFileSocket);
  }
};

}

namespace {
/*
* Automatically register FastCGIServerFactory on program start
*/
struct RegisterFastCGIServer {
public:
  RegisterFastCGIServer() {
    auto registry = HPHP::ServerFactoryRegistry::getInstance();
    auto factory = std::make_shared<HPHP::FastCGIServerFactory>();
    registry->registerFactory("fastcgi", factory);
  }
} s_RegisterFastCGIServer;
}
