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

#include "hphp/runtime/server/libevent-server.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class LibEventServerFactory : public ServerFactory {
public:
  LibEventServerFactory() {}

  virtual ServerPtr createServer(const ServerOptions& options);
};

ServerPtr LibEventServerFactory::createServer(const ServerOptions& options) {
  return std::make_shared<LibEventServer>(options);
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
  auto factory = std::make_shared<HPHP::LibEventServerFactory>();
  registry->registerFactory("libevent", factory);
}

}
