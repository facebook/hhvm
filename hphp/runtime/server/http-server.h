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

#ifndef incl_HPHP_HTTP_SERVER_H_
#define incl_HPHP_HTTP_SERVER_H_

#include "hphp/runtime/server/server.h"
#include "hphp/runtime/server/satellite-server.h"
#include "hphp/util/async-func.h"

namespace HPHP {
struct MemInfo;

///////////////////////////////////////////////////////////////////////////////

struct HttpServer : Synchronizable, TakeoverListener,
                    Server::ServerEventListener {
  static std::shared_ptr<HttpServer> Server;
  static time_t StartTime;

private:
  static time_t OldServerStopTime;
  static unsigned LoadFactor;

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
  // Get total ongoing/queued request count for all satellite servers.
  std::pair<int, int> getSatelliteRequestCount() const;

  void stopOnSignal(int sig);

  static unsigned GetLoadFactor() { return LoadFactor; }
  static void ResetLoadFactor() { LoadFactor = 100; }

  /*
   * Try to stop the previous server instance.  Return true if the
   * old server acknowledges.  This function doesn't wait until the
   * previous server dies.  Nothing bad happens if the old server
   * isn't there, or is already in the process of stopping.
   *
   * Currently it is implemented through the admin port command.  So
   * it will not work if admin server is not present, or if the new
   * server and old server disagree on port and password for admin
   * server.
   */
  static bool StopOldServer();
  /*
   * When running with RuntimeOption::StopOldServer, given a target
   * memory needed (RuntimeOption::ServerRSSNeededMb), check memory
   * status, stop the old server when necessary, and wait for at most
   * RuntimeOption::OldServerWait seconds after trying to stop the old
   * server, before proceeding regardless of available memory. `final`
   * indicates whether this is the final wait (and thus must wait till
   * all the required memory is available).
   */
  static void CheckMemAndWait(bool final = false);

  // Helpers to decide whether it is safe to proceed till next check
  // point, or continue indefinitely.
  static bool CanStep(const MemInfo& mem, int64_t rss,
                      int64_t rssNeeded, int cacheFreeFactor);
  static bool CanContinue(const MemInfo& mem, int64_t rss,
                          int64_t rssNeeded, int cacheFreeFactor);

  static void EvictFileCache();
  static void PrepareToStop();

private:
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

  // Allow cleanups (e.g., flush cached values into a database) using
  // PHP code when server stops.
  void playShutdownRequest(const std::string& fileName);

private:
  bool m_stopped;
  bool m_killed;
  const char* m_stopReason;

  ServerPtr m_pageServer;
  ServerPtr m_adminServer;
  std::vector<std::unique_ptr<SatelliteServer>> m_satellites;
  AsyncFunc<HttpServer> m_watchDog;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_SERVER_H_
