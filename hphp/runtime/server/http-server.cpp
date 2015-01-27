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

#include "hphp/runtime/server/http-server.h"

#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/pprof-server.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/thread-init-fini.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/server/admin-request-handler.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/server/replay-transport.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/server/warmup-request-handler.h"
#include "hphp/runtime/server/xbox-server.h"

#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/ssl-init.h"

#include <folly/Conv.h>
#include <folly/Format.h>

#include <sys/types.h>
#include <signal.h>

namespace HPHP {
extern InitFiniNode *extra_server_init, *extra_server_exit;

using std::string;


///////////////////////////////////////////////////////////////////////////////
// statics

std::shared_ptr<HttpServer> HttpServer::Server;
time_t HttpServer::StartTime;

const int kNumProcessors = sysconf(_SC_NPROCESSORS_ONLN);

static void on_kill(int sig) {
  // There is a small race condition here with HttpServer::reset in
  // program-functions.cpp, but it can only happen if we get a signal while
  // shutting down.  The fix is to add a lock to HttpServer::Server but it seems
  // like overkill.
  if (HttpServer::Server) {
    HttpServer::Server->stopOnSignal(sig);
  }
  signal(sig, SIG_DFL);
  raise(sig);
}

///////////////////////////////////////////////////////////////////////////////

HttpServer::HttpServer()
  : m_stopped(false), m_killed(false), m_stopReason(nullptr),
    m_watchDog(this, &HttpServer::watchDog) {

  // enabling mutex profiling, but it's not turned on
  LockProfiler::s_pfunc_profile = server_stats_log_mutex;

  int startingThreadCount = RuntimeOption::ServerThreadCount;
  uint32_t additionalThreads = 0;
  if (RuntimeOption::ServerWarmupThrottleRequestCount > 0 &&
      RuntimeOption::ServerThreadCount > kNumProcessors) {
    startingThreadCount = kNumProcessors;
    additionalThreads = RuntimeOption::ServerThreadCount - startingThreadCount;
  }

  auto serverFactory = ServerFactoryRegistry::getInstance()->getFactory
      (RuntimeOption::ServerType);
  const std::string address = RuntimeOption::ServerFileSocket.empty()
    ? RuntimeOption::ServerIP : RuntimeOption::ServerFileSocket;
  ServerOptions options(
      address, RuntimeOption::ServerPort, startingThreadCount);
  options.m_useFileSocket = !RuntimeOption::ServerFileSocket.empty();
  options.m_serverFD = RuntimeOption::ServerPortFd;
  options.m_sslFD = RuntimeOption::SSLPortFd;
  options.m_takeoverFilename = RuntimeOption::TakeoverFilename;
  m_pageServer = std::move(serverFactory->createServer(options));
  m_pageServer->addTakeoverListener(this);
  m_pageServer->addServerEventListener(this);

  if (additionalThreads) {
    auto handlerFactory = std::make_shared<WarmupRequestHandlerFactory>(
      m_pageServer.get(), additionalThreads,
      RuntimeOption::ServerWarmupThrottleRequestCount,
      RuntimeOption::RequestTimeoutSeconds);
    m_pageServer->setRequestHandlerFactory([handlerFactory] {
      return handlerFactory->createHandler();
    });
  } else {
    m_pageServer->setRequestHandlerFactory<HttpRequestHandler>(
      RuntimeOption::RequestTimeoutSeconds);
  }

  if (RuntimeOption::EnableSSL) {
    assert(SSLInit::IsInited());
    m_pageServer->enableSSL(RuntimeOption::SSLPort);
  }

  ServerOptions admin_options
    (RuntimeOption::ServerIP, RuntimeOption::AdminServerPort,
     RuntimeOption::AdminThreadCount);
  m_adminServer = std::move(serverFactory->createServer(admin_options));
  m_adminServer->setRequestHandlerFactory<AdminRequestHandler>(
    RuntimeOption::RequestTimeoutSeconds);

  for (unsigned int i = 0; i < RuntimeOption::SatelliteServerInfos.size();
       i++) {
    auto info = RuntimeOption::SatelliteServerInfos[i];
    auto satellite(std::move(SatelliteServer::Create(info)));
    if (satellite) {
      if (info->getType() == SatelliteServer::Type::KindOfDanglingPageServer) {
        m_danglings.push_back(std::move(satellite));
      } else {
        m_satellites.push_back(std::move(satellite));
      }
    }
  }

  if (RuntimeOption::XboxServerPort != 0) {
    auto xboxInfo = std::make_shared<XboxServerInfo>();
    auto satellite = SatelliteServer::Create(xboxInfo);
    if (satellite) {
      m_satellites.push_back(std::move(satellite));
    }
  }

  StaticContentCache::TheCache.load();
  hphp_process_init();

  signal(SIGTERM, on_kill);
  signal(SIGUSR1, on_kill);
  signal(SIGHUP, on_kill);

  if (!RuntimeOption::StartupDocument.empty()) {
    Hdf hdf;
    hdf["cmd"] = static_cast<int>(Transport::Method::GET);
    hdf["url"] = RuntimeOption::StartupDocument;
    hdf["remote_host"] = RuntimeOption::ServerIP;

    ReplayTransport rt;
    rt.replayInput(hdf);
    HttpRequestHandler handler(0);
    handler.run(&rt);
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
}

// Synchronously stop satellites and start danglings
void HttpServer::onServerShutdown() {
  for (InitFiniNode *in = extra_server_exit; in; in = in->next) {
    in->func();
  }

  Eval::Debugger::Stop();
  if (RuntimeOption::EnableDebuggerServer) {
    Logger::Info("debugger server stopped");
  }

  XboxServer::Stop();

  // When a new instance of HPHP has taken over our page server socket,
  // stop our admin server and satellites so it can acquire those ports.
  for (unsigned int i = 0; i < m_satellites.size(); i++) {
    std::string name = m_satellites[i]->getName();
    m_satellites[i]->stop();
    Logger::Info("satellite server %s stopped", name.c_str());
  }
  if (RuntimeOption::AdminServerPort) {
    m_adminServer->stop();
    m_adminServer->waitForEnd();
    Logger::Info("admin server stopped");
  }

  // start dangling servers, so they can serve old version of pages
  for (unsigned int i = 0; i < m_danglings.size(); i++) {
    std::string name = m_danglings[i]->getName();
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

void HttpServer::takeoverShutdown() {
  // We want to synchronously shut down our satellite servers to free up ports,
  // then asynchronously shut down everything else.
  onServerShutdown();
  stop();
}

void HttpServer::serverStopped(HPHP::Server* server) {
  Logger::Info("Page server stopped");
  assert(server == m_pageServer.get());
  removePid();

  auto sockFile = RuntimeOption::ServerFileSocket;
  if (!sockFile.empty()) {
    unlink(sockFile.c_str());
  }
}

HttpServer::~HttpServer() {
  // XXX: why should we have to call stop here?  If we haven't already
  // stopped (and joined all the threads), watchDog could still be
  // running and leaving this destructor without a wait would be
  // wrong...
  stop();
}

void HttpServer::runOrExitProcess() {
  StartTime = time(0);

  auto startupFailure = [] (const std::string& msg) {
    Logger::Error(msg);
    Logger::Error("Shutting down due to failure(s) to bind in "
                  "HttpServer::runAndExitProcess");
    // Logger flushes itself---we don't need to run any atexit handlers
    // (historically we've mostly just SEGV'd while trying) ...
    _Exit(1);
  };

  m_watchDog.start();

  if (RuntimeOption::ServerPort) {
    if (!startServer(true)) {
      startupFailure("Unable to start page server");
      not_reached();
    }
    Logger::Info("page server started");
  }

  if (RuntimeOption::AdminServerPort) {
    if (!startServer(false)) {
      startupFailure("Unable to start admin server");
      not_reached();
    }
    Logger::Info("admin server started");
  }

  for (unsigned int i = 0; i < m_satellites.size(); i++) {
    std::string name = m_satellites[i]->getName();
    try {
      m_satellites[i]->start();
      Logger::Info("satellite server %s started", name.c_str());
    } catch (Exception &e) {
      startupFailure(
        folly::format("Unable to start satellite server {}: {}",
                      name, e.getMessage()).str()
      );
      not_reached();
    }
  }

  if (RuntimeOption::HHProfServerEnabled) {
    try {
      HeapProfileServer::Server = std::make_shared<HeapProfileServer>();
    } catch (FailedToListenException &e) {
      startupFailure("Unable to start profiling server");
      not_reached();
    }
    Logger::Info("profiling server started");
  }

  if (!Eval::Debugger::StartServer()) {
    startupFailure("Unable to start debugger server");
    not_reached();
  } else if (RuntimeOption::EnableDebuggerServer) {
    Logger::Info("debugger server started");
  }

  for (InitFiniNode *in = extra_server_init; in; in = in->next) {
    in->func();
  }

  {
    Logger::Info("all servers started");
    createPid();
    Lock lock(this);
    // continously running until /stop is received on admin server
    while (!m_stopped) {
      wait();
    }
    if (m_stopReason) {
      Logger::Warning("Server stopping with reason: %s\n", m_stopReason);
    }
    // if we were killed, bail out immediately
    if (m_killed) {
      Logger::Info("page server killed");
      return;
    }
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

  waitForServers();
  m_watchDog.waitForEnd();
  hphp_process_exit();
  Logger::Info("all servers stopped");
}

void HttpServer::waitForServers() {
  if (RuntimeOption::ServerPort) {
    m_pageServer->waitForEnd();
  }
  if (RuntimeOption::AdminServerPort) {
    m_adminServer->waitForEnd();
  }
  if (RuntimeOption::HHProfServerEnabled) {
    HeapProfileServer::Server.reset();
  }
  // all other servers invoke waitForEnd on stop
}

static void exit_on_timeout(int sig) {
  signal(sig, SIG_DFL);
  kill(getpid(), SIGKILL);
  exit(0);
}

void HttpServer::stop(const char* stopReason) {
  if (RuntimeOption::ServerGracefulShutdownWait) {
    signal(SIGALRM, exit_on_timeout);
    alarm(RuntimeOption::ServerGracefulShutdownWait);
  }

  Lock lock(this);
  m_stopped = true;
  m_stopReason = stopReason;
  notify();
}

void HttpServer::stopOnSignal(int sig) {
  // Signal to the main server thread to exit immediately if
  // we want to die on SIGTERM
  if (RuntimeOption::ServerKillOnSIGTERM && sig == SIGTERM) {
    Lock lock(this);
    m_stopped = true;
    m_killed = true;
    m_stopReason = "SIGTERM received";
    notify();
    return;
  }

  if (RuntimeOption::ServerGracefulShutdownWait) {
    signal(SIGALRM, exit_on_timeout);
    alarm(RuntimeOption::ServerGracefulShutdownWait);
  }

  // NOTE: Server->stop does a graceful stop by design.
  if (m_pageServer) {
    m_pageServer->stop();
  }
  if (m_adminServer) {
    m_adminServer->stop();
  }

  waitForServers();
}

void HttpServer::createPid() {
  if (!RuntimeOption::PidFile.empty()) {
    FILE * f = fopen(RuntimeOption::PidFile.c_str(), "w");
    if (f) {
      pid_t pid = Process::GetProcessId();
      char buf[64];
      snprintf(buf, sizeof(buf), "%" PRId64, (int64_t)pid);
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
    CstrBuffer sb(RuntimeOption::PidFile.c_str());
    if (sb.size()) {
      int64_t pid = sb.detach().toInt64();
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
// watch dog thread

void HttpServer::watchDog() {
  int count = 0;
  bool noneed = false;
  while (!m_stopped && !noneed) {
    noneed = true;

    if (RuntimeOption::DropCacheCycle > 0) {
      noneed = false;
      if ((count % RuntimeOption::DropCacheCycle) == 0) { // every hour
        dropCache();
      }
    }

    sleep(1);
    ++count;

    if (RuntimeOption::MaxRSSPollingCycle > 0) {
      noneed = false;
      if ((count % RuntimeOption::MaxRSSPollingCycle) == 0) { // every minute
        checkMemory();
      }
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
  int64_t used = Process::GetProcessRSS(Process::GetProcessId()) * 1024 * 1024;
  if (RuntimeOption::MaxRSS > 0 && used > RuntimeOption::MaxRSS) {
    Logger::Error("ResourceLimit.MaxRSS %ld reached %ld used, exiting",
                  RuntimeOption::MaxRSS, used);
    stop();
  }
}

void HttpServer::getSatelliteStats(
    std::vector<std::pair<std::string, int>> *stats) {
  for (const auto& i : m_satellites) {
    std::pair<std::string, int> active("satellite." + i->getName() + ".load",
                                       i->getActiveWorker());
    std::pair<std::string, int> queued("satellite." + i->getName() + ".queued",
                                       i->getQueuedJobs());
    stats->push_back(active);
    stats->push_back(queued);
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
      if (RuntimeOption::ServerExitOnBindFail) return false;

      if (i == 0) {
        Logger::Info("shutting down old HPHP server by /stop command");
      }

      if (errno == EACCES) {
        if (pageServer && !RuntimeOption::ServerFileSocket.empty()) {
          Logger::Error("Permission denied opening socket at %s",
                        RuntimeOption::ServerFileSocket.c_str());
        } else {
          Logger::Error("Permission denied listening on port %d", port);
        }
        return false;
      }

      HttpClient http;
      std::string url = "http://";
      std::string serverIp = (RuntimeOption::ServerIP.empty()) ? "localhost" :
        RuntimeOption::ServerIP;
      url += serverIp;
      url += ":";
      url += folly::to<std::string>(RuntimeOption::AdminServerPort);
      url += "/stop";
      std::string auth;

      auto passwords = RuntimeOption::AdminPasswords;
      if (passwords.empty() && !RuntimeOption::AdminPassword.empty()) {
        passwords.insert(RuntimeOption::AdminPassword);
      }
      auto passwordIter = passwords.begin();
      int statusCode = 401;
      do {
        std::string auth_url;
        if (passwordIter != passwords.end()) {
          auth_url = url + "?auth=";
          auth_url += *passwordIter;
          passwordIter++;
        } else {
          auth_url = url;
        }
        StringBuffer response;
        statusCode = http.get(auth_url.c_str(), response);
      } while (statusCode == 401 && passwordIter != passwords.end());

      if (pageServer && !RuntimeOption::ServerFileSocket.empty()) {
        if (i == 0) {
          Logger::Info("Unlinking unused socket at %s",
                       RuntimeOption::ServerFileSocket.c_str());
        }
        struct stat stat_buf;
        if (stat(RuntimeOption::ServerFileSocket.c_str(), &stat_buf) == 0
            && S_ISSOCK(stat_buf.st_mode)) {
          std::string cmd = "bash -c '! fuser ";
          cmd += RuntimeOption::ServerFileSocket;
          cmd += "'";
          if (FileUtil::ssystem(cmd.c_str()) == 0) {
            unlink(RuntimeOption::ServerFileSocket.c_str());
          }
        }
      }
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
        if (pageServer && !RuntimeOption::ServerFileSocket.empty()) {
          if (i == 0) {
            Logger::Info("unlinking socket at %s",
                         RuntimeOption::ServerFileSocket.c_str());
          }

          struct stat stat_buf;
          if (stat(RuntimeOption::ServerFileSocket.c_str(), &stat_buf) == 0
              && S_ISSOCK(stat_buf.st_mode)) {
            unlink(RuntimeOption::ServerFileSocket.c_str());
          }
        } else {
          if (i == 0) {
            Logger::Info("killing anything listening on port %d", port);
          }

          std::string cmd = "lsof -t -i :";
          cmd += folly::to<std::string>(port);
          cmd += " | xargs kill -9";
          FileUtil::ssystem(cmd.c_str());
        }
        sleep(1);
      }
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
