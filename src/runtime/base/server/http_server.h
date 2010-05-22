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

#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <runtime/base/server/server.h>
#include <runtime/base/server/satellite_server.h>
#include <runtime/base/server/libevent_server_with_takeover.h>
#include <util/async_func.h>
#include <runtime/base/server/service_thread.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(HttpServer);

class HttpServer : public Synchronizable, public TakeoverListener {
public:
  static HttpServerPtr Server;
  static time_t StartTime;

public:
  HttpServer();
  ~HttpServer();

  void run();
  void stop();

  void flushLog();
  void watchDog();

  void takeoverShutdown(LibEventServerWithTakeover* server);

  ServerPtr getPageServer() { return m_pageServer;}

private:
  bool m_stopped;

  ServerPtr m_pageServer;
  ServerPtr m_adminServer;
  SatelliteServerPtrVec m_satellites;
  SatelliteServerPtrVec m_danglings;
  AsyncFunc<HttpServer> m_loggerThread;
  AsyncFunc<HttpServer> m_watchDog;
  ServiceThreadPtrVec m_serviceThreads;

  bool startServer(bool pageServer);
  void onServerShutdown();
  void abortServers();

  // pid file functions
  void createPid();
  void removePid();
  void killPid();

  // memory monitoring functions
  void dropCache();
  void checkMemory();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HTTP_SERVER_H__
