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

#include <runtime/base/server/http_server.h>
#include <runtime/base/server/libevent_server.h>
#include <runtime/base/server/libevent_server_with_takeover.h>
#include <runtime/base/server/http_request_handler.h>
#include <runtime/base/server/admin_request_handler.h>
#include <runtime/base/server/server_stats.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/static_content_cache.h>
#include <runtime/base/class_info.h>
#include <runtime/base/source_info.h>
#include <runtime/base/rtti_info.h>
#include <runtime/base/memory/memory_manager.h>
#include <util/logger.h>
#include <runtime/base/externals.h>
#include <runtime/base/util/http_client.h>
#include <runtime/base/server/replay_transport.h>
#include <runtime/base/program_functions.h>
#include <util/db_conn.h>
#include <util/log_aggregator.h>
#include <runtime/ext/ext_apc.h>
#include <sys/types.h>
#include <signal.h>

using namespace boost;
using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// statics

HttpServerPtr HttpServer::Server;
time_t HttpServer::StartTime;

///////////////////////////////////////////////////////////////////////////////

HttpServer::HttpServer()
  : m_stopped(false),
    m_loggerThread(this, &HttpServer::flushLog),
    m_watchDog(this, &HttpServer::watchDog) {

  // enabling mutex profiling, but it's not turned on
  LockProfiler::s_pfunc_profile = server_stats_log_mutex;

  if (RuntimeOption::TakeoverFilename.empty()) {
    m_pageServer = ServerPtr
      (new TypedServer<LibEventServer, HttpRequestHandler>
       (RuntimeOption::ServerIP, RuntimeOption::ServerPort,
        RuntimeOption::ServerThreadCount,
        RuntimeOption::RequestTimeoutSeconds));
  } else {
    LibEventServerWithTakeover* server =
      (new TypedServer<LibEventServerWithTakeover, HttpRequestHandler>
       (RuntimeOption::ServerIP, RuntimeOption::ServerPort,
        RuntimeOption::ServerThreadCount,
        RuntimeOption::RequestTimeoutSeconds));
    server->setTransferFilename(RuntimeOption::TakeoverFilename);
    server->addTakeoverListener(this);
    m_pageServer = ServerPtr(server);
  }

  m_adminServer = ServerPtr
    (new TypedServer<LibEventServer, AdminRequestHandler>
     (RuntimeOption::ServerIP, RuntimeOption::AdminServerPort,
      RuntimeOption::AdminThreadCount,
      RuntimeOption::RequestTimeoutSeconds));

  for (unsigned int i = 0; i < RuntimeOption::SatelliteServerInfos.size();
       i++) {
    SatelliteServerInfoPtr info = RuntimeOption::SatelliteServerInfos[i];
    SatelliteServerPtr satellite = SatelliteServer::Create(info);
    if (satellite) {
      if (info->getType() == SatelliteServer::KindOfDanglingPageServer) {
        m_danglings.push_back(satellite);
      } else {
        m_satellites.push_back(satellite);
      }
    }
  }

  if (RuntimeOption::EnableStaticContentCache) {
    StaticContentCache::TheCache.load();
  }
  ClassInfo::Load();
  SourceInfo::TheSourceInfo.load();
  RTTIInfo::TheRTTIInfo.init(true);

  hphp_process_init();
  apc_load(RuntimeOption::ApcLoadThread);

  Server::InstallStopSignalHandlers(m_pageServer);
  Server::InstallStopSignalHandlers(m_adminServer);

  if (!RuntimeOption::StartupDocument.empty()) {
    Hdf hdf;
    hdf["get"] = 1;
    hdf["url"] = RuntimeOption::StartupDocument;
    hdf["remote_host"] = RuntimeOption::ServerIP;

    ReplayTransport rt;
    rt.replayInput(hdf);
    HttpRequestHandler handler;
    handler.handleRequest(&rt);
    int code = rt.getResponseCode();
    if (code == 200) {
      Logger::Info("StartupDocument %s returned 200 OK: %s",
                   RuntimeOption::StartupDocument.c_str(),
                   rt.getResponse().c_str());
    } else {
      Logger::Error("StartupDocument %s failed %d: %s",
                    RuntimeOption::StartupDocument.c_str(),
                    code, rt.getResponse().data());
      return;
    }
  }

  for (unsigned int i = 0; i < RuntimeOption::ThreadDocuments.size(); i++) {
    ServiceThreadPtr thread
      (new ServiceThread(RuntimeOption::ThreadDocuments[i]));
    m_serviceThreads.push_back(thread);
  }
}

void HttpServer::onServerShutdown() {
  // When a new instance of HPHP has taken over our page server socket,
  // stop our admin server and satellites so it can acquire those ports.
  for (unsigned int i = 0; i < m_satellites.size(); i++) {
    string name = m_satellites[i]->getName();
    m_satellites[i]->stop();
    Logger::Info("satellite server %s stopped", name.c_str());
  }
  if (RuntimeOption::AdminServerPort) {
    m_adminServer->stop();
    Logger::Info("admin server stopped");
  }

  // start dangling servers, so they can serve old version of pages
  for (unsigned int i = 0; i < m_danglings.size(); i++) {
    string name = m_danglings[i]->getName();
    try {
      m_danglings[i]->start();
      Logger::Info("dangling server %s started", name.c_str());
    } catch (Exception &e) {
      Logger::Error("Unable to start danglings server %s: %s",
                    name.c_str(), e.getMessage().c_str());
      // it's okay not able to start them
    }
  }
}

void HttpServer::takeoverShutdown(LibEventServerWithTakeover* server) {
  ASSERT(server == m_pageServer.get());
  // We want to synchronously shut down our satellite servers to free up ports,
  // then asynchronously shut down everything else.
  onServerShutdown();
  stop();
}

HttpServer::~HttpServer() {
  stop();
}

void HttpServer::run() {
  StartTime = time(0);

  m_loggerThread.start();
  m_watchDog.start();

  for (unsigned int i = 0; i < m_serviceThreads.size(); i++) {
    m_serviceThreads[i]->start();
  }
  for (unsigned int i = 0; i < m_serviceThreads.size(); i++) {
    m_serviceThreads[i]->waitForStarted();
  }

  if (RuntimeOption::ServerPort) {
    if (!startServer(true)) {
      Logger::Error("Unable to start page server");
      return;
    }
    Logger::Info("page server started");
  }

  if (RuntimeOption::AdminServerPort) {
    if (!startServer(false)) {
      Logger::Error("Unable to start admin server");
      abortServers();
      return;
    }
    Logger::Info("admin server started");
  }

  for (unsigned int i = 0; i < m_satellites.size(); i++) {
    string name = m_satellites[i]->getName();
    try {
      m_satellites[i]->start();
      Logger::Info("satellite server %s started", name.c_str());
    } catch (Exception &e) {
      Logger::Error("Unable to start satellite server %s: %s",
                    name.c_str(), e.getMessage().c_str());
      abortServers();
      return;
    }
  }

  {
    Logger::Info("all servers started");
    createPid();
    Lock lock(this);
    // continously running until /stop is received on admin server
    while (!m_stopped) {
      wait();
    }
    removePid();
    Logger::Info("page server stopped");
  }

  onServerShutdown(); // dangling server already started here
  time_t t0 = time(0);
  if (RuntimeOption::ServerPort) {
    m_pageServer->stop();
  }
  time_t t1 = time(0);
  if (!m_danglings.empty() && RuntimeOption::ServerDanglingWait > 0) {
    int elapsed = t1 - t0;
    if (RuntimeOption::ServerDanglingWait > elapsed) {
      sleep(RuntimeOption::ServerDanglingWait - elapsed);
    }
  }

  for (unsigned int i = 0; i < m_danglings.size(); i++) {
    m_danglings[i]->stop();
    Logger::Info("dangling server %s stopped",
                 m_danglings[i]->getName().c_str());
  }

  m_watchDog.waitForEnd();
  m_loggerThread.waitForEnd();
  Logger::Info("all servers stopped");
}

static void exit_on_timeout(int sig) {
  signal(sig, SIG_DFL);
  kill(getpid(), SIGKILL);
  exit(0);
}

void HttpServer::stop() {
  if (RuntimeOption::ServerGracefulShutdownWait) {
    signal(SIGALRM, exit_on_timeout);
    alarm(RuntimeOption::ServerGracefulShutdownWait);
  }

  Lock lock(this);
  m_stopped = true;
  notify();
}

void HttpServer::abortServers() {
  for (unsigned int i = 0; i < m_satellites.size(); i++) {
    m_satellites[i]->stop();
  }
  if (RuntimeOption::AdminServerPort) {
    m_adminServer->stop();
  }
  if (RuntimeOption::ServerPort) {
    m_pageServer->stop();
  }
}

void HttpServer::createPid() {
  if (!RuntimeOption::PidFile.empty()) {
    FILE * f = fopen(RuntimeOption::PidFile.c_str(), "w");
    if (f) {
      pid_t pid = Process::GetProcessId();
      char buf[64];
      snprintf(buf, sizeof(buf), "%lld", (int64)pid);
      fwrite(buf, strlen(buf), 1, f);
      fclose(f);
    } else {
      Logger::Error("Unable to open pid file %s for write",
                    RuntimeOption::PidFile.c_str());
    }
  }
}

void HttpServer::removePid() {
  if (!RuntimeOption::PidFile.empty()) {
    unlink(RuntimeOption::PidFile.c_str());
  }
}

void HttpServer::killPid() {
  if (!RuntimeOption::PidFile.empty()) {
    StringBuffer sb(RuntimeOption::PidFile.c_str());
    if (sb.size()) {
      int64 pid = sb.detach().toInt64();
      if (pid) {
        kill((pid_t)pid, SIGKILL);
        return;
      }
    }
    Logger::Error("Unable to read pid file %s for any meaningful pid",
                  RuntimeOption::PidFile.c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////
// logger thread

void HttpServer::flushLog() {
  if (!Logger::UseLogAggregator) return;

  ServerDataPtr database;
  ostream *out = NULL;
  if (!RuntimeOption::LogAggregatorDatabase.empty()) {
    database = ServerData::Create(RuntimeOption::LogAggregatorDatabase);
  } else if (!RuntimeOption::LogAggregatorFile.empty()) {
    out = new ofstream(RuntimeOption::LogAggregatorFile.c_str());
  } else {
    out = &cout;
  }

  bool stopped = false;
  while (!stopped) {
    if (database) {
      LogAggregator::TheLogAggregator.flush(database);
    } else {
      LogAggregator::TheLogAggregator.flush(*out);
    }
    sleep(RuntimeOption::LogAggregatorSleepSeconds);

    Lock lock(this);
    stopped = m_stopped;
  }

  if (out != &cout) {
    delete out;
  }
}

///////////////////////////////////////////////////////////////////////////////
// watch dog thread

void HttpServer::watchDog() {
  int count = 0;
  while (!m_stopped) {
    if (RuntimeOption::DropCacheCycle > 0 &&
        (count % RuntimeOption::DropCacheCycle) == 0) { // every hour
      dropCache();
    }

    sleep(1);
    ++count;

    if (RuntimeOption::MaxRSSPollingCycle > 0 &&
        (count % RuntimeOption::MaxRSSPollingCycle) == 0) { // every minute
      checkMemory();
    }
  }
}

void HttpServer::dropCache() {
  FILE *f = fopen("/proc/sys/vm/drop_caches", "w");
  if (f) {
    // http://www.linuxinsight.com/proc_sys_vm_drop_caches.html
    const char *FREE_ALL_CACHES = "3\n";
    fwrite(FREE_ALL_CACHES, 2, 1, f);
    fclose(f);
  }
}

void HttpServer::checkMemory() {
  if (RuntimeOption::MaxRSS > 0 &&
      Process::GetProcessRSS(Process::GetProcessId()) * 1024 * 1024 >
      RuntimeOption::MaxRSS) {
    stop();
  }
}

///////////////////////////////////////////////////////////////////////////////
// page server

bool HttpServer::startServer(bool pageServer) {
  int port = pageServer ?
    RuntimeOption::ServerPort : RuntimeOption::AdminServerPort;

  // 1. try something nice
  for (unsigned int i = 0; i < 60; i++) {
    try {
      if (pageServer) {
        m_pageServer->start();
      } else {
        m_adminServer->start();
      }
      return true;
    } catch (FailedToListenException &e) {
      if (i == 0) {
        Logger::Info("shutting down old HPHP server by /stop command");
      }

      if (errno == EACCES) {
        Logger::Error("Permission denied listening on port %d", port);
        return false;
      }

      HttpClient http;
      string url = "http://";
      url += RuntimeOption::ServerIP;
      url += ":";
      url += lexical_cast<string>(RuntimeOption::AdminServerPort);
      url += "/stop";
      StringBuffer response;
      http.get(url.c_str(), response);

      sleep(1);
    }
  }

  // 2. try something harsh
  if (RuntimeOption::ServerHarshShutdown) {
    for (unsigned int i = 0; i < 5; i++) {
      try {
        if (pageServer) {
          m_pageServer->start();
        } else {
          m_adminServer->start();
        }
        return true;
      } catch (FailedToListenException &e) {
        if (i == 0) {
          Logger::Info("shutting down old HPHP server by pid file");
        }
        killPid();
        sleep(1);
      }
    }
  }

  // 3. try something evil
  if (RuntimeOption::ServerEvilShutdown) {
    for (unsigned int i = 0; i < 60; i++) {
      try {
        if (pageServer) {
          m_pageServer->start();
        } else {
          m_adminServer->start();
        }
        return true;
      } catch (FailedToListenException &e) {
        if (i == 0) {
          Logger::Info("killing anything listening on port %d", port);
        }

        string cmd = "lsof -t -i :";
        cmd += lexical_cast<string>(port);
        cmd += " | xargs kill -9";
        Util::ssystem(cmd.c_str());

        sleep(1);
      }
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
