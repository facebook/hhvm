/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/ext/apc/ext_apc.h"
#include "hphp/runtime/server/admin-request-handler.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/server/replay-transport.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/server/warmup-request-handler.h"
#include "hphp/runtime/server/xbox-server.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/util/alloc.h"
#include "hphp/util/boot-stats.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/ssl-init.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/struct-log.h"

#include <folly/Conv.h>
#include <folly/Format.h>
#include <folly/portability/Unistd.h>

#include <sys/types.h>
#include <signal.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// statics

std::shared_ptr<HttpServer> HttpServer::Server;
time_t HttpServer::StartTime;
std::atomic<double> HttpServer::LoadFactor{1.0};
std::atomic_int_fast64_t HttpServer::PrepareToStopTime{0};
time_t HttpServer::OldServerStopTime;
std::vector<ShutdownStat> HttpServer::ShutdownStats;
folly::MicroSpinLock HttpServer::StatsLock;

const int kNumProcessors = sysconf(_SC_NPROCESSORS_ONLN);

static void on_kill(int sig) {
  signal(sig, SIG_DFL);
  // There is a small race condition here with HttpServer::reset in
  // program-functions.cpp, but it can only happen if we get a signal while
  // shutting down.  The fix is to add a lock to HttpServer::Server but it seems
  // like overkill.
  if (HttpServer::Server) {
    HttpServer::Server->stopOnSignal(sig);
  } else {
    raise(sig);
  }
}

///////////////////////////////////////////////////////////////////////////////

HttpServer::HttpServer()
  : m_stopped(false), m_killed(false), m_stopReason(nullptr),
    m_watchDog(this, &HttpServer::watchDog) {
  LoadFactor = RuntimeOption::EvalInitialLoadFactor;

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
  ServerOptions options(address, RuntimeOption::ServerPort,
    RuntimeOption::ServerThreadCount, startingThreadCount);
  options.m_useFileSocket = !RuntimeOption::ServerFileSocket.empty();
  options.m_serverFD = RuntimeOption::ServerPortFd;
  options.m_sslFD = RuntimeOption::SSLPortFd;
  options.m_takeoverFilename = RuntimeOption::TakeoverFilename;
  m_pageServer = serverFactory->createServer(options);
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

  if (RuntimeOption::EnableSSLWithPlainText) {
    assert(SSLInit::IsInited());
    m_pageServer->enableSSLWithPlainText();
  }


  ServerOptions admin_options(RuntimeOption::AdminServerIP,
                              RuntimeOption::AdminServerPort,
                              RuntimeOption::AdminThreadCount);
  admin_options.m_queueToWorkerRatio =
    RuntimeOption::AdminServerQueueToWorkerRatio;
  m_adminServer = serverFactory->createServer(admin_options);
  m_adminServer->setRequestHandlerFactory<AdminRequestHandler>(
    RuntimeOption::RequestTimeoutSeconds);

  for (unsigned int i = 0; i < RuntimeOption::SatelliteServerInfos.size();
       i++) {
    auto info = RuntimeOption::SatelliteServerInfos[i];
    auto satellite(SatelliteServer::Create(info));
    if (satellite) {
      m_satellites.push_back(std::move(satellite));
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

  m_counterCallback.init(
    [this](std::map<std::string, int64_t>& counters) {
      counters["ev_connections"] = m_pageServer->getLibEventConnectionCount();
      counters["inflight_requests"] = m_pageServer->getActiveWorker();
      counters["queued_requests"] = m_pageServer->getQueuedJobs();

      auto const sat_requests = getSatelliteRequestCount();
      counters["satellite_inflight_requests"] = sat_requests.first;
      counters["satellite_queued_requests"] = sat_requests.second;
    }
  );

  signal(SIGTERM, on_kill);
  signal(SIGUSR1, on_kill);
  signal(SIGHUP, on_kill);
}

// Synchronously stop satellites
void HttpServer::onServerShutdown() {
  InitFiniNode::ServerFini();

  Eval::Debugger::Stop();
  if (RuntimeOption::EnableDebuggerServer) {
    Logger::Info("debugger server stopped");
  }

  // When a new instance of HPHP has taken over our page server socket,
  // stop our admin server and satellites so it can acquire those
  // ports.
  if (RuntimeOption::AdminServerPort) {
    m_adminServer->stop();
  }
  for (unsigned int i = 0; i < m_satellites.size(); i++) {
    std::string name = m_satellites[i]->getName();
    m_satellites[i]->stop();
    Logger::Info("satellite server %s stopped", name.c_str());
  }
  XboxServer::Stop();
  if (RuntimeOption::AdminServerPort) {
    m_adminServer->waitForEnd();
    Logger::Info("admin server stopped");
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

  LogShutdownStats();
}

void HttpServer::playShutdownRequest(const std::string& fileName) {
  if (fileName.empty()) return;
  Logger::Info("playing request upon shutdown %s", fileName.c_str());
  try {
    ReplayTransport rt;
    rt.replayInput(fileName.c_str());
    HttpRequestHandler handler(0);
    handler.run(&rt);
    if (rt.getResponseCode() == 200) {
      Logger::Info("successfully finished request: %s", rt.getUrl());
    } else {
      Logger::Error("request unsuccessful: %s", rt.getUrl());
    }
  } catch (...) {
    Logger::Error("got exception when playing request: %s",
                  fileName.c_str());
  }
}

HttpServer::~HttpServer() {
  m_counterCallback.deinit();

  // XXX: why should we have to call stop here?  If we haven't already
  // stopped (and joined all the threads), watchDog could still be
  // running and leaving this destructor without a wait would be
  // wrong...
  stop();
}

static StaticString s_file{"file"}, s_line{"line"};

void HttpServer::runOrExitProcess() {
  if (StaticContentCache::TheFileCache &&
      StructuredLog::enabled() &&
      StructuredLog::coinflip(RuntimeOption::EvalStaticContentsLogRate)) {
    CacheManager::setLogger([](bool existsCheck, const std::string& name) {
        auto record = StructuredLogEntry{};
        record.setInt("existsCheck", existsCheck);
        record.setStr("file", name);
        bool needsCppStack = true;
        if (!g_context.isNull()) {
          VMRegAnchor _;
          if (vmfp()) {
            auto const bt =
              createBacktrace(BacktraceArgs().withArgValues(false));
            std::vector<std::string> frameStrings;
            std::vector<folly::StringPiece> frames;
            for (int i = 0; i < bt.size(); i++) {
              auto f = bt.rvalAt(i).toArray();
              if (f.exists(s_file)) {
                std::string s = f.rvalAt(s_file).toString().toCppString();
                if (f.exists(s_line)) {
                  s += folly::sformat(":{}", f.rvalAt(s_line).toInt64());
                }
                frameStrings.emplace_back(std::move(s));
                frames.push_back(frameStrings.back());
              }
            }
            record.setVec("stack", frames);
            needsCppStack = false;
          }
        }
        if (needsCppStack) {
          record.setStackTrace("stack", StackTrace{StackTrace::Force{}});
        }
        StructuredLog::log("hhvm_file_cache", record);
      });
  }
  auto startupFailure = [] (const std::string& msg) {
    Logger::Error(msg);
    Logger::Error("Shutting down due to failure(s) to bind in "
                  "HttpServer::runAndExitProcess");
    // Logger flushes itself---we don't need to run any atexit handlers
    // (historically we've mostly just SEGV'd while trying) ...
    _Exit(1);
  };

  if (!RuntimeOption::InstanceId.empty()) {
    std::string msg = "Starting instance " + RuntimeOption::InstanceId;
    if (!RuntimeOption::DeploymentId.empty()) {
      msg += " from deployment " + RuntimeOption::DeploymentId;
    }
    Logger::Info(msg);
  }

  m_watchDog.start();

  if (RuntimeOption::ServerPort) {
    if (!startServer(true)) {
      startupFailure("Unable to start page server");
      not_reached();
    }
    Logger::Info("page server started");
  }

  StartTime = time(nullptr);

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

  if (!Eval::Debugger::StartServer()) {
    startupFailure("Unable to start debugger server");
    not_reached();
  } else if (RuntimeOption::EnableDebuggerServer) {
    Logger::Info("debugger server started");
  }

  try {
    InitFiniNode::ServerInit();
  } catch (std::exception &e) {
    startupFailure(
      folly::sformat("Exception in InitFiniNode::ServerInit(): {}",
                     e.what()));
  }

  {
    BootStats::mark("servers started");
    Logger::Info("all servers started");
    createPid();
    Lock lock(this);
    BootStats::done();
    // continously running until /stop is received on admin server, or
    // takeover is requested.
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
  }

  if (RuntimeOption::ServerPort) {
    Logger::Info("stopping page server");
    m_pageServer->stop();
  }
  onServerShutdown();

  EvictFileCache();

  waitForServers();
  m_watchDog.waitForEnd();
  playShutdownRequest(RuntimeOption::ServerCleanupRequest);
}

void HttpServer::waitForServers() {
  if (RuntimeOption::ServerPort) {
    m_pageServer->waitForEnd();
  }
  if (RuntimeOption::AdminServerPort) {
    m_adminServer->waitForEnd();
  }
  // all other servers invoke waitForEnd on stop
}

static void exit_on_timeout(int sig) {
  signal(sig, SIG_DFL);
#ifdef _WIN32
  TerminateProcess(GetCurrentProcess(), (UINT)-1);
#else
  kill(getpid(), SIGKILL);
#endif
  // we really shouldn't get here, but who knows.
  // abort so we catch it as a crash.
  abort();
}

// Tell OOM killer to kill this process if it has to.  This is used during
// server shutdown.  If we are dying anyway, let's try to protect others.
static void oom_sacrifice() {
#ifdef __linux__
  // Use open() instead of fopen() here to avoid additional buffering and
  // allocation, which could go wrong if memory is really limited.
  int fd = open("/proc/self/oom_score_adj", O_WRONLY, 0);
  if (fd >= 0) {
    write(fd, "800", 3);
    close(fd);
  }
#endif
}

void HttpServer::stop(const char* stopReason) {
  if (m_stopped) return;
  // we're shutting down flush http logs
  Logger::FlushAll();
  HttpRequestHandler::GetAccessLog().flushAllWriters();
  MarkShutdownStat(ShutdownEvent::SHUTDOWN_INITIATED);
  oom_sacrifice();

  if (RuntimeOption::ServerKillOnTimeout) {
    int totalWait =
      RuntimeOption::ServerPreShutdownWait +
      RuntimeOption::ServerShutdownListenWait +
      RuntimeOption::ServerGracefulShutdownWait;

    if (totalWait > 0) {
      // Use a killer thread to _Exit() after totalWait seconds.  If
      // the main thread returns before that, the killer thread will
      // exit.  So don't do join() on this thread.  Since we create a
      // thread here, `HttpServer::stop()` cannot be called in signal
      // handlers, use `stopOnSignal()` (which uses SIGALRM) in that
      // case.
      auto killer = std::thread([totalWait] {
#ifdef __linux__
          sched_param param;
          param.sched_priority = 5;
          pthread_setschedparam(pthread_self(), SCHED_RR, &param);
          // It is OK if we fail to increase thread priority.
#endif
          /* sleep override */
          std::this_thread::sleep_for(std::chrono::seconds{totalWait});
          _Exit(1);
        });
      killer.detach();
    }
  }

  Lock lock(this);
  m_stopped = true;
  m_stopReason = stopReason;
  notify();
}

void HttpServer::stopOnSignal(int sig) {
  if (m_stopped) return;
  // we're shutting down flush http logs
  Logger::FlushAll();
  HttpRequestHandler::GetAccessLog().flushAllWriters();
  MarkShutdownStat(ShutdownEvent::SHUTDOWN_INITIATED);
  oom_sacrifice();

  // Signal to the main server thread to exit immediately if
  // we want to die on SIGTERM
  if (RuntimeOption::ServerKillOnSIGTERM && sig == SIGTERM) {
    {
      Lock lock(this);
      m_stopped = true;
      m_killed = true;
      m_stopReason = "SIGTERM received";
      notify();
    }
    raise(sig);
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

void HttpServer::EvictFileCache() {
  // In theory, the kernel should do just as well even if we don't
  // explicitly advise files out.  But we can do it anyway when we
  // need more free memory, e.g., when a new instance of the server is
  // about to start.
  advise_out(RuntimeOption::RepoLocalPath);
  advise_out(RuntimeOption::FileCache);
  apc_advise_out();
}

void HttpServer::PrepareToStop() {
  MarkShutdownStat(ShutdownEvent::SHUTDOWN_PREPARE);
  PrepareToStopTime.store(time(nullptr), std::memory_order_release);
  EvictFileCache();
}

void HttpServer::createPid() {
  if (!RuntimeOption::PidFile.empty()) {
    FILE * f = fopen(RuntimeOption::PidFile.c_str(), "w");
    if (f) {
      auto const pid = getpid();
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

static bool sendAdminCommand(const char* cmd) {
  if (RuntimeOption::AdminServerPort <= 0) return false;
  std::string host = RuntimeOption::AdminServerIP;
  if (host.empty()) host = "localhost";
  auto passwords = RuntimeOption::AdminPasswords;
  if (passwords.empty() && !RuntimeOption::AdminPassword.empty()) {
    passwords.insert(RuntimeOption::AdminPassword);
  }
  auto passwordIter = passwords.begin();
  do {
    std::string url;
    if (passwordIter != passwords.end()) {
      url = folly::sformat("http://{}:{}/{}?auth={}", host,
                           RuntimeOption::AdminServerPort,
                           cmd, *passwordIter);
      ++passwordIter;
    } else {
      url = folly::sformat("http://{}:{}/{}", host,
                           RuntimeOption::AdminServerPort, cmd);
    }
    if (CURL* curl = curl_easy_init()) {
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
      curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
      curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
      auto code = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
      if (code == CURLE_OK) {
        Logger::Info("sent %s via admin port", cmd);
        return true;
      }
    }
  } while (passwordIter != passwords.end());
  return false;
}

bool HttpServer::ReduceOldServerLoad() {
  if (!RuntimeOption::StopOldServer) return false;
  if (OldServerStopTime > 0) return true;
  Logger::Info("trying to reduce load of the old HPHP server");
  return sendAdminCommand("prepare-to-stop");
}

bool HttpServer::StopOldServer() {
  if (OldServerStopTime > 0) return true;
  SCOPE_EXIT { OldServerStopTime = time(nullptr); };
  Logger::Info("trying to shut down old HPHP server by /stop command");
  return sendAdminCommand("stop");
}

// Return the estimated amount of memory that can be safely taken into
// the current process RSS.
static inline int64_t availableMemory(const MemInfo& mem, int64_t rss,
                                      int factor) {
  // Estimation of page cache that are readily evictable without
  // causing big memory pressure.  We consider it safe to take
  // `pageFreeFactor` percent of cached pages (excluding those used by
  // the current process, estimated to be equal to the current RSS)
  auto const otherCacheMb = std::max((int64_t)0,  mem.cachedMb - rss);
  auto const availableMb = mem.freeMb + otherCacheMb * factor / 100;
  return availableMb;
}

bool HttpServer::CanContinue(const MemInfo& mem, int64_t rssMb,
                             int64_t rssNeeded, int cacheFreeFactor) {
  if (!mem.valid()) return false;
  if (mem.freeMb < RuntimeOption::ServerCriticalFreeMb) return false;
  auto const availableMb = availableMemory(mem, rssMb, cacheFreeFactor);
  auto const result = (rssMb + availableMb >= rssNeeded);
  if (result) assert(CanStep(mem, rssMb, rssNeeded, cacheFreeFactor));
  return result;
}

bool HttpServer::CanStep(const MemInfo& mem, int64_t rssMb,
                         int64_t rssNeeded, int cacheFreeFactor) {
  if (!mem.valid()) return false;
  if (mem.freeMb < RuntimeOption::ServerCriticalFreeMb) return false;
  auto const availableMb = availableMemory(mem, rssMb, cacheFreeFactor);
  // Estimation of the memory needed till the next check point.  Since
  // the current check point is not the last one, we try to be more
  // optimistic, by assuming that memory requirement won't grow
  // drastically between successive check points, and that it won't
  // grow over our estimate.
  auto const neededToStep = std::min(rssNeeded / 4, rssNeeded - rssMb);
  return (availableMb >= neededToStep);
}

void HttpServer::CheckMemAndWait(bool final) {
  if (!RuntimeOption::StopOldServer) return;
  if (RuntimeOption::OldServerWait <= 0) return;

  auto const pid = getpid();
  auto const rssNeeded = RuntimeOption::ServerRSSNeededMb;
  auto const factor = RuntimeOption::CacheFreeFactor;
  do {
    // Don't wait too long
    if (OldServerStopTime > 0 &&
        time(nullptr) - OldServerStopTime >= RuntimeOption::OldServerWait) {
      return;
    }

    auto const rssMb = Process::GetProcessRSS(pid);
    MemInfo memInfo;
    if (!Process::GetMemoryInfo(memInfo)) {
      Logger::Error("Failed to obtain memory information");
      HttpServer::StopOldServer();
      return;
    }
    Logger::FInfo("Memory pressure check: free/cached/buffers {}/{}/{} "
                  "currentRss {}Mb, required {}Mb.",
                  memInfo.freeMb, memInfo.cachedMb, memInfo.buffersMb,
                  rssMb, rssNeeded);

    if (!final) {
      if (CanStep(memInfo, rssMb, rssNeeded, factor)) return;
    } else {
      if (CanContinue(memInfo, rssMb, rssNeeded, factor)) return;
    }

    // OK, we don't have enough memory, let's do something.
    HttpServer::StopOldServer();  // nop if already called before
    /* sleep override */ sleep(1);
  } while (true); // Guaranteed to return in the loop, at least upon timeout.
  not_reached();
}

void HttpServer::MarkShutdownStat(ShutdownEvent event) {
  if (!RuntimeOption::EvalLogServerRestartStats) return;
  std::lock_guard<folly::MicroSpinLock> lock(StatsLock);
  MemInfo mem;
  Process::GetMemoryInfo(mem);
  auto const rss = Process::GetProcessRSS(getpid());
  auto const requests = requestCount();
  if (event == ShutdownEvent::SHUTDOWN_PREPARE) {
    ShutdownStats.clear();
    ShutdownStats.reserve(ShutdownEvent::kNumEvents);
  }
#ifndef NDEBUG
  if (!ShutdownStats.empty()) {
    assert(ShutdownStats.back().event <= event);
  }
#endif
  ShutdownStats.push_back({event, time(nullptr), mem, rss, requests});
}

void HttpServer::LogShutdownStats() {
  if (!RuntimeOption::EvalLogServerRestartStats) return;
  StructuredLogEntry entry;
  std::lock_guard<folly::MicroSpinLock> lock(StatsLock);
  if (ShutdownStats.empty()) return;
  for (size_t i = 0; i < ShutdownStats.size(); ++i) {
    const auto& stat = ShutdownStats[i];
    auto const eventName = stat.eventName();
    entry.setInt(folly::sformat("{}.rss", eventName), stat.rss);
    entry.setInt(folly::sformat("{}.free", eventName),
                 stat.memUsage.freeMb);
    entry.setInt(folly::sformat("{}.cached", eventName),
                 stat.memUsage.cachedMb);
    entry.setInt(folly::sformat("{}.buffers", eventName),
                 stat.memUsage.buffersMb);
    // Log the difference since last event, if available
    if (i > 0) {
      const auto& last = ShutdownStats[i - 1];
      auto const lastEvent = last.eventName();
      entry.setInt(folly::sformat("{}.duration", lastEvent),
                   stat.time - last.time);
      entry.setInt(folly::sformat("{}.requests", lastEvent),
                   stat.requestsServed - last.requestsServed);
      entry.setInt(folly::sformat("{}.rss.delta", lastEvent),
                   stat.rss - last.rss);
    }
  }
  StructuredLog::log("webserver_shutdown_timing", entry);
  ShutdownStats.clear();
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
  int64_t used = Process::GetProcessRSS(getpid()) * 1024 * 1024;
  if (RuntimeOption::MaxRSS > 0 && used > RuntimeOption::MaxRSS) {
    Logger::Error(
      "ResourceLimit.MaxRSS %" PRId64 " reached %" PRId64 " used, exiting",
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

std::pair<int, int> HttpServer::getSatelliteRequestCount() const {
  int inflight = 0;
  int queued = 0;
  for (const auto& i : m_satellites) {
    inflight += i->getActiveWorker();
    queued += i->getQueuedJobs();
  }
  return std::make_pair(inflight, queued);
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

      StopOldServer();

      if (errno == EACCES) {
        if (pageServer && !RuntimeOption::ServerFileSocket.empty()) {
          Logger::Error("Permission denied opening socket at %s",
                        RuntimeOption::ServerFileSocket.c_str());
        } else {
          Logger::Error("Permission denied listening on port %d", port);
        }
        return false;
      }

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
