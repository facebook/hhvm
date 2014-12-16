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

#ifndef incl_HPHP_HTTP_SERVER_H_
#define incl_HPHP_HTTP_SERVER_H_

#include "hphp/runtime/server/server.h"
#include "hphp/runtime/server/satellite-server.h"
#include "hphp/util/async-func.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class HttpServer : public Synchronizable, public TakeoverListener,
                   public Server::ServerEventListener {
public:
  static std::shared_ptr<HttpServer> Server;
  static time_t StartTime;

public:
  explicit HttpServer();
  ~HttpServer();

  /*
   * Try to run the various servers that this class controls.
   *
   * If any of them can't bind their appropriate port (or otherwise
   * fail their initialization steps), shut down the entire process
   * without running any atexit handlers.
   */
  void runOrExitProcess();

  // Stop may be called from a signal handler.
  void stop(const char* reason = nullptr);

  bool isStopped() const { return m_stopped;}

  void flushLog();
  void watchDog();

  void takeoverShutdown() override;

  void serverStopped(HPHP::Server* server) override;

  HPHP::Server *getPageServer() { return m_pageServer.get(); }
  void getSatelliteStats(std::vector<std::pair<std::string, int>> *stats);

  void stopOnSignal(int sig);

private:
  bool m_stopped;
  bool m_killed;
  const char* m_stopReason;

  ServerPtr m_pageServer;
  ServerPtr m_adminServer;
  std::vector<std::unique_ptr<SatelliteServer>> m_satellites;
  std::vector<std::unique_ptr<SatelliteServer>> m_danglings;
  AsyncFunc<HttpServer> m_watchDog;

  bool startServer(bool pageServer);
  void onServerShutdown();
  void waitForServers();

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

#endif // incl_HPHP_HTTP_SERVER_H_
