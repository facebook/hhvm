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

#include "hphp/runtime/server/admin-request-handler.h"

#include "hphp/runtime/base/apc-file-storage.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/hhprof.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/request-tracing.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/timestamp.h"
#include "hphp/runtime/base/unit-cache.h"

#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/tc-record.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-profile.h"

#include "hphp/runtime/ext/apc/ext_apc.h"
#include "hphp/runtime/ext/json/ext_json.h"
#include "hphp/runtime/ext/xenon/ext_xenon.h"

#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/server/memory-stats.h"
#include "hphp/runtime/server/pagelet-server.h"
#include "hphp/runtime/server/rpc-request-handler.h"
#include "hphp/runtime/server/server-stats.h"

#include "hphp/util/alloc.h"
#include "hphp/util/build-info.h"
#include "hphp/util/hphp-config.h"
#include "hphp/util/hugetlb.h"
#include "hphp/util/logger.h"
#include "hphp/util/managed-arena.h"
#include "hphp/util/mutex.h"
#include "hphp/util/numa.h"
#include "hphp/util/process.h"
#include "hphp/util/service-data.h"
#include "hphp/util/stacktrace-profiler.h"
#include "hphp/util/timer.h"

#ifdef ENABLE_EXTENSION_MYSQL
#include "hphp/runtime/ext/mysql/mysql_stats.h"
#endif

#include <folly/Conv.h>
#include <folly/Random.h>
#include <folly/portability/Unistd.h>

#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

#if defined(FACEBOOK) || defined(HAVE_LIBSODIUM)
#include <sodium.h>
#ifdef crypto_pwhash_STRBYTES
#define HAVE_CRYPTO_PWHASH_STR
#endif
#endif

#ifdef GOOGLE_CPU_PROFILER
#include <gperftools/profiler.h>
#include "hphp/runtime/base/file-util.h"
#endif

namespace HPHP {

using std::endl;
using std::string;

///////////////////////////////////////////////////////////////////////////////

AdminCommandExt* AdminCommandExt::s_head{nullptr};

THREAD_LOCAL(AccessLog::ThreadData, AdminRequestHandler::s_accessLogThreadData);

AccessLog AdminRequestHandler::s_accessLog(
  &(AdminRequestHandler::getAccessLogThreadData));

AdminRequestHandler::AdminRequestHandler(int timeout) :
    RequestHandler(timeout) {
}

// Helper machinery for jemalloc-stats-print command.
#ifdef USE_JEMALLOC
struct malloc_write {
  char *s;
  size_t slen;
  size_t smax;
  bool oom;
};

void malloc_write_init(malloc_write *mw) {
  mw->s = nullptr;
  mw->slen = 0;
  mw->smax = 0;
  mw->oom = false;
}

void malloc_write_fini(malloc_write *mw) {
  if (mw->s != nullptr) {
    free(mw->s);
    malloc_write_init(mw);
  }
}

static void malloc_write_cb(void *cbopaque, const char *s) {
  malloc_write* mw = (malloc_write*)cbopaque;
  size_t slen = strlen(s);

  if (mw->oom || slen == 0) {
    return;
  }

  if (mw->slen + slen+1 >= mw->smax) {
    assertx(mw->slen + slen > 0);
    char* ts = (char*)realloc(mw->s, (mw->slen + slen) << 1);
    if (ts == nullptr) {
      mw->oom = true;
      return;
    }
    mw->s = ts;
    mw->smax = (mw->slen + slen) << 1;
  }
  memcpy(&mw->s[mw->slen], s, slen+1);
  mw->slen += slen;
}
#endif

void WarnIfNotOK(Transport* transport) {
  auto code = static_cast<Transport::StatusCode>(transport->getResponseCode());
  if (code != Transport::StatusCode::OK) {
    Logger::Warning("Non-OK response from admin server: %d %s",
                    static_cast<int>(code),
                    transport->getResponseInfo().c_str());
  }
}

#ifdef HPHP_TRACE
namespace {
/*
 * Task to trace a total of 'count' requests whose URL contain 'url', using the
 * module:level,... specificaion 'spec' (see HPHP::Trace).
 *
 * To ensure an unbroken trace output stream, the task is held locally by the
 * thread currently tracing its request. If filtering on URL, threads may pass
 * the task around to find matches faster. Concurrent tasks are not supported.
 */
struct TraceTask {
  const std::string spec;
  const std::string url;
  int64_t count;
};
std::atomic<TraceTask*> s_traceTask{nullptr}; // Task up for grabs.
__thread TraceTask* tl_traceTask{nullptr};

InitFiniNode s_traceRequestStart([]() {
  if (!tl_traceTask && !s_traceTask.load(std::memory_order_acquire)) return;
  if (!tl_traceTask) {
    // Try grab the task.
    tl_traceTask = s_traceTask.exchange(nullptr, std::memory_order_acq_rel);
  }
  if (!tl_traceTask) return; // We lost the race; nothing to do.
  if (tl_traceTask->count == 0) {
    // Task already complete.
    Trace::trace("Trace complete at %d\n", (int)time(nullptr));
    Trace::setTraceThread("");
    delete tl_traceTask;
    tl_traceTask = nullptr;
    return;
  }
  const string url = g_context->getRequestUrl();
  if (url.find(tl_traceTask->url) == string::npos) {
    // URL mismatch; hand task back (and discard any unlikely colliding task).
    delete s_traceTask.exchange(tl_traceTask, std::memory_order_acq_rel);
    tl_traceTask = nullptr;
    Trace::setTraceThread("");
  } else {
    // Work on task.
    --tl_traceTask->count;
    const auto spec = tl_traceTask->spec;
    Trace::setTraceThread(spec);
    Trace::trace("Trace for %s at %d using spec %s\n",
                 url.c_str(), (int)time(nullptr), spec.c_str());
  }
}, InitFiniNode::When::RequestStart, "trace");

} // namespace
#endif // HPHP_TRACE

void AdminRequestHandler::logToAccessLog(Transport* transport) {
  GetAccessLog().onNewRequest();
  GetAccessLog().log(transport, nullptr);
  WarnIfNotOK(transport);
}

void AdminRequestHandler::setupRequest(Transport* transport) {
  auto const cmd = transport->getCommand();

  if (strncmp(cmd.c_str(), "dump-apc", 8) == 0) {
    hphp_session_init(Treadmill::SessionKind::AdminPort);
  } else {
    g_context.getCheck();
  }
  GetAccessLog().onNewRequest();
}

void AdminRequestHandler::teardownRequest(Transport* transport) noexcept {
  SCOPE_EXIT {
    auto const cmd = transport->getCommand();

    if (strncmp(cmd.c_str(), "dump-apc", 8) == 0) {
      hphp_context_exit();
      hphp_session_exit();
    } else {
      hphp_memory_cleanup();
    }
  };
  GetAccessLog().log(transport, nullptr);
  WarnIfNotOK(transport);
}

void AdminRequestHandler::handleRequest(Transport *transport) {
  transport->addHeader("Content-Type", "text/plain");
  std::string cmd = transport->getCommand();

  do {
    if (cmd == "" || cmd == "help") {
      string usage =
        "/stop:            stop the web server\n"
        "    instance-id   optional, if specified, instance ID has to match\n"
        "/oom-kill:        abort all requests whose memory usage exceed\n"
        "                  Server.RequestMemoryOOMKillBytes\n"
        "/free-mem:        ask allocator to release unused memory to system\n"
        "/prepare-to-stop: ask the server to prepare for stopping\n"
        "/flush-profile:   flush profiling counters (in -fprofile-gen builds)\n"
        "/flush-logs:      trigger batching log-writers to flush all content\n"
        "/translate:       translate hex encoded stacktrace in 'stack' param\n"
        "    stack         required, stack trace to translate\n"
        "    build-id      optional, if specified, build ID has to match\n"
        "    bare          optional, whether to display frame ordinates\n"
        "/build-id:        returns build id that's passed in from command line"
        "\n"
        "/instance-id:     instance id that's passed in from command line\n"
        "/compiler-id:     returns the compiler id that built this app\n"
        "/config-id:       returns the config id passed in from command line\n"
        "/repo-schema:     return the repo schema id used by this app\n"
        "/ini-get-all:     dump all settings as JSON\n"
        "/check-load:      how many threads are actively handling requests\n"
        "/check-queued:    how many http requests are queued waiting to be\n"
        "                  handled\n"
        "/check-health:    return json containing basic load/usage stats\n"
        "/check-ev:        how many http requests are active by libevent\n"
        "/check-pl-load:   how many pagelet threads are actively handling\n"
        "                  requests\n"
        "/check-pl-queued: how many pagelet requests are queued waiting to\n"
        "                  be handled\n"
        "/check-sql:       report SQL table statistics\n"
        "/check-sat        how many satellite threads are actively handling\n"
        "                  requests and queued waiting to be handled\n"
        "/status.xml:      show server status in XML\n"
        "/status.json:     show server status in JSON\n"
        "/status.html:     show server status in HTML\n"

        "/rqtrace-stats:   show aggregate request trace stats in JSON\n"

        "/memory.xml:      show memory status in XML\n"
        "/memory.json:     show memory status in JSON\n"
        "/memory.html:     show memory status in HTML\n"

        "/statcache-clear: clear the stat cache entries\n"

        "/stats-on:        main switch: enable server stats\n"
        "/stats-off:       main switch: disable server stats\n"
        "/stats-clear:     clear all server stats\n"

        "/stats-web:       turn on/off server page stats (CPU and gen time)\n"
        "/stats-mem:       turn on/off memory statistics\n"
        "/stats-sql:       turn on/off SQL statistics\n"
        "/stats-mutex:     turn on/off mutex statistics\n"
        "    sampling      optional, default 1000\n"

        "/stats.keys:      list all available keys\n"
        "    from          optional, <timestamp>, or <-n> second ago\n"
        "    to            optional, <timestamp>, or <-n> second ago\n"
        "/stats.kvp:       show server stats in key-value pairs\n"
        "    from          optional, <timestamp>, or <-n> second ago\n"
        "    to            optional, <timestamp>, or <-n> second ago\n"
        "    agg           optional, aggragation: *, url, code\n"
        "    keys          optional, <key>,<key/hit>,<key/sec>,<:regex:>\n"
        "    url           optional, only stats of this page or URL\n"
        "    code          optional, only stats of pages returning this code\n"

        "/xenon-snap:      generate a Xenon snapshot, which is logged later\n"
        "/hugepage:        show stats about hugepage usage\n"
        "/jit-des-info:    show information about deserialized profile data\n"

        "/const-ss:        get const_map_size\n"
        "/static-strings:  get number of static strings\n"
        "/static-strings-rds: ... that correspond to defined constants\n"
        "/dump-static-strings: dump static strings to /tmp/static_strings\n"
        "/random-static-strings: return randomly selected static strings\n"
        "    count         number of strings to return, default 1\n"
        "/dump-apc:        dump all current value in APC to /tmp/apc_dump\n"
        "/dump-apc-prefix: dump a key prefix contents from APC to\n"
        "                  /tmp/apc_dump_prefix\n"
        "    prefix        required, the prefix to dump\n"
        "    count         optional, the number of keys to dump, default 1\n"
        "/dump-apc-info:   show basic APC stats\n"
        "/dump-apc-meta:   dump meta information for all objects in APC to\n"
        "                  /tmp/apc_dump_meta\n"
        "/advise-out-apc:  forcibly madvise out APC prime data\n"
        "/random-apc:      dump the key and size of a random APC entry\n"
        "    count         number of entries to return\n"
        "/treadmill:       dump treadmill information\n"

        "/pcre-cache-size: get pcre cache map size\n"
        "/dump-pcre-cache: dump cached pcre's to /tmp/pcre_cache\n"
        "/dump-array-info: dump array tracer info to /tmp/array_tracer_dump\n"

        "/invalidate-units: remove specified files from the unit cache\n"
        "    path           absolute path of files to invalidate\n"

        "/start-stacktrace-profiler: set enable_stacktrace_profiler to true\n"
        "/relocate:        relocate translations\n"
        "    random        optional, default false, relocate random subset\n"
        "       all        optional, default false, relocate all translations\n"
        "      time        optional, default 20 (seconds)\n"
#ifdef HPHP_TRACE
        "/trace-request:   write trace for next request(s) to "
        + RuntimeOption::getTraceOutputFile() + "\n"
        "    spec          module:level,... spec; see hphp/util/trace.h\n"
        "    count         optional, total requests to trace (default: 1)\n"
        "    url           optional, trace only if URL contains \'url\'\n"
#endif
#ifdef GOOGLE_CPU_PROFILER
        "/prof-cpu-on:     turn on CPU profiler\n"
        "/prof-cpu-off:    turn off CPU profiler\n"
#endif
#ifdef EXECUTION_PROFILER
        "/prof-exe:        returns sampled execution profile\n"
#endif
        "/vm-tcspace:      show space used by translator caches\n"
        "/vm-tcaddr:       show addresses of translation cache sections\n"
        "/vm-dump-tc:      dump translation cache to /tmp/tc_dump_a and\n"
        "                  /tmp/tc_dump_astub\n"
        "/vm-namedentities:show size of the NamedEntityTable\n"
        "/proxy:           set up request proxy\n"
        "    origin        URL to proxy requests to\n"
        "    percentage    percentage of requests to proxy\n"
        "/load-factor:     get or set load factor\n"
        "    set           optional, set new load factor (default 1.0,\n"
        "                  valid range [-1.0, 10.0])\n"
        "/queue-discount:  get/set how much we discount the queue-length \n"
        "    set           optional, set discount value (default 0,\n"
        "                  valid range [0, 10000])\n"
        "/warmup-status:   Describes state of JIT warmup.\n"
        "                  Returns empty string if warmed up.\n"
        ;
#ifdef USE_TCMALLOC
        if (MallocExtensionInstance) {
          usage.append(
              "/tcmalloc-stats:  get internal tcmalloc stats\n"
              "/tcmalloc-set-tc: set max mem tcmalloc thread-cache can use\n"
          );
        }
#endif

#ifdef USE_JEMALLOC
        if (mallctl) {
          usage.append(
              "/jemalloc-stats:  get internal jemalloc stats\n"
              "/jemalloc-stats-print:\n"
              "                  get comprehensive jemalloc stats in\n"
              "                  human-readable form\n"
              "/jemalloc-prof-activate:\n"
              "                  activate heap profiling\n"
              "/jemalloc-prof-deactivate:\n"
              "                  deactivate heap profiling\n"
              "/jemalloc-prof-dump:\n"
              "                  dump heap profile\n"
              "    file          optional, filesystem path\n"
              "/jemalloc-prof-request:\n"
              "                  dump thread-local heap profile in\n"
              "                  the next request that runs\n"
              "    file          optional, filesystem path\n"
          );
#ifdef ENABLE_HHPROF
          usage.append(
              "/hhprof/start:    start profiling\n"
              "    requestType   \"all\" or \"next\"*\n"
              "    url           profile next matching url for \"next\"\n"
              "    lgSample      lg sample rate\n"
              "    profileType   \"current\"* or \"cumulative\"\n"
              "/hhprof/status:   configuration and current dump status\n"
              "/hhprof/stop:     stop profiling\n"
              "/pprof/cmdline:   program command line\n"
              "/pprof/heap:      heap dump\n"
              "/pprof/symbol:    symbol lookup\n"
          );
#endif // ENABLE_HHPROF
        }
#endif // USE_JEMALLOC

      AdminCommandExt::iterate([&](AdminCommandExt* ace) {
        usage.append(ace->usage());
        return false;
      });

      transport->sendString(usage);
      break;
    }

    bool needs_password = (cmd != "build-id") && (cmd != "compiler-id") &&
                          (cmd != "instance-id") && (cmd != "flush-logs") &&
                          (cmd != "warmup-status") && (cmd != "config-id")
#if defined(ENABLE_HHPROF) && defined(USE_JEMALLOC)
                          && (mallctl == nullptr || (
                                 (cmd != "hhprof/start")
                              && (cmd != "hhprof/status")
                              && (cmd != "hhprof/stop")
                              && (cmd != "pprof/cmdline")
                              && (cmd != "pprof/heap")
                              && (cmd != "pprof/symbol")))
#endif
                          ;
    // When configured, we allow read-only stats to be read without a password.
    if (needs_password && !RuntimeOption::AdminServerStatsNeedPassword) {
      if ((strncmp(cmd.c_str(), "memory.", 7) == 0) ||
          (strncmp(cmd.c_str(), "stats.", 6) == 0) ||
          (strncmp(cmd.c_str(), "check-", 6) == 0) ||
          (strncmp(cmd.c_str(), "static-strings", 14) == 0) ||
          cmd == "hugepage" || cmd == "pcre-cache-size" ||
          cmd == "vm-tcspace" || cmd == "vm-tcaddr" ||
          cmd == "vm-namedentities" || cmd == "jemalloc-stats") {
        needs_password = false;
      }
    }

    if (needs_password && !RuntimeOption::HashedAdminPasswords.empty()) {
      bool matched = false;
#ifdef HAVE_CRYPTO_PWHASH_STR
      const auto password = transport->getParam("auth");
      for (const std::string& hash : RuntimeOption::HashedAdminPasswords) {
        if (crypto_pwhash_str_verify(hash.data(),
                                     password.data(),
                                     password.size()) == 0) {
          matched = true;
          break;
        }
      }
#endif
      if (!matched) {
        transport->sendString("Unauthorized", 401);
        break;
      }
    } else if (needs_password && !RuntimeOption::AdminPasswords.empty()) {
      std::set<std::string>::const_iterator iter =
        RuntimeOption::AdminPasswords.find(transport->getParam("auth"));
      if (iter == RuntimeOption::AdminPasswords.end()) {
        transport->sendString("Unauthorized", 401);
        break;
      }
    } else {
      if (needs_password && !RuntimeOption::AdminPassword.empty() &&
          RuntimeOption::AdminPassword != transport->getParam("auth")) {
        transport->sendString("Unauthorized", 401);
        break;
      }
    }

    if (cmd == "stop") {
      string instanceId = transport->getParam("instance-id");
      if (!instanceId.empty() && instanceId != RuntimeOption::InstanceId) {
        transport->sendString("Instance ID doesn't match.", 500);
        break;
      }

      transport->sendString("OK\n");
      Logger::Info("Got admin port stop request from %s",
                   transport->getRemoteHost());
      HttpServer::Server->stop();
      break;
    }
    if (cmd == "oom-kill") {
      Logger::Info("Invoking OOM killer upon admin port request from %s",
                   transport->getRemoteHost());
      RequestInfo::InvokeOOMKiller();
      transport->sendString("OOM killer invoked");
      break;
    }
    if (cmd == "free-mem") {
      const auto before = Process::GetMemUsageMb();
      std::string errStr;
      if (purge_all(&errStr)) {
        const auto after = Process::GetMemUsageMb();
        transport->sendString(
          folly::sformat("Purged {} -> {} MB RSS", before, after).c_str());
      } else {
        transport->sendString(errStr.c_str(), 500);
      }
      break;
    }
    if (cmd == "prepare-to-stop") {
      Logger::Info("Got admin port prepare-to-stop request from %s",
                   transport->getRemoteHost());
      MemInfo info, newInfo;
      Process::GetMemoryInfo(info);
      HttpServer::PrepareToStop();

      // We may consider purge_all() here, too.  But since requests
      // are still coming in, it may not be very useful, and has some
      // performance penalties.

      // TODO: evaluate the effect of sync() and uncomment if
      // desirable.  It is blocking and can take some time, so do it
      // in a separate thread.
      // std::thread t{sync};
      // t.detach();

      transport->sendString("OK\n");
      Process::GetMemoryInfo(newInfo);
      Logger::FInfo("free/cached/buffer {}/{}/{} -> {}/{}/{}",
                    info.freeMb, info.cachedMb, info.buffersMb,
                    newInfo.freeMb, newInfo.cachedMb, newInfo.buffersMb);
      break;
    }
    if (cmd == "flush-profile") {
      HttpServer::ProfileFlush();
      // send the response *after* flushing, so the caller knows the
      // data has been updated.
      transport->sendString("OK\n");
      break;
    }
    if (cmd == "flush-logs") {
      transport->sendString("OK\n");
      Logger::FlushAll();
      HttpRequestHandler::GetAccessLog().flushAllWriters();
      AdminRequestHandler::GetAccessLog().flushAllWriters();
      RPCRequestHandler::GetAccessLog().flushAllWriters();
      break;
    }
    if (cmd == "set-log-level") {
      string result("OK\n");
      string level = transport->getParam("level");
      if (level == "None") {
        Logger::LogLevel = Logger::LogNone;
      } else if (level == "Error") {
        Logger::LogLevel = Logger::LogError;
      } else if (level == "Warning") {
        Logger::LogLevel = Logger::LogWarning;
      } else if (level == "Info") {
        Logger::LogLevel = Logger::LogInfo;
      } else if (level == "Verbose") {
        Logger::LogLevel = Logger::LogVerbose;
      } else {
        result = "Failed to set log level\n";
      }

      transport->sendString(result);
      break;
    }
#ifdef HPHP_TRACE
    if (cmd == "trace-request") {
      Trace::ensureInit(RuntimeOption::getTraceOutputFile());
      // Just discard any existing task.
      delete s_traceTask.exchange(
        new TraceTask{transport->getParam("spec"),
                      transport->getParam("url"),
                      std::max(transport->getInt64Param("count"), 1ll)},
        std::memory_order_acq_rel);
      transport->sendString("OK\n");
      break;
    }
#endif
    if (cmd == "build-id") {
      transport->sendString(RuntimeOption::BuildId, 200);
      break;
    }
    if (cmd == "instance-id") {
      transport->sendString(RuntimeOption::InstanceId, 200);
      break;
    }
    if (cmd == "compiler-id") {
      transport->sendString(compilerId().begin(), 200);
      break;
    }
    if (cmd == "config-id") {
      transport->sendString(std::to_string(RuntimeOption::ConfigId), 200);
      break;
    }
    if (cmd == "repo-schema") {
      transport->sendString(repoSchemaId().begin(), 200);
      break;
    }
    if (cmd == "translate") {
      string buildId = transport->getParam("build-id");
      if (!buildId.empty() && buildId != RuntimeOption::BuildId) {
        transport->sendString("Build ID doesn't match.", 500);
        break;
      }

      string translated = translate_stack(transport->getParam("stack").c_str(),
                                          transport->getParam("bare").empty());
      transport->sendString(translated);
      break;
    }
    if (strncmp(cmd.c_str(), "check", 5) == 0 &&
        handleCheckRequest(cmd, transport)) {
      break;
    }
    if (strncmp(cmd.c_str(), "status", 6) == 0 &&
        handleStatusRequest(cmd, transport)) {
      break;
    }
    if (strncmp(cmd.c_str(),"memory", 6) == 0 &&
        handleMemoryRequest(cmd, transport)) {
      break;
    }
    if (strncmp(cmd.c_str(), "stats", 5) == 0 &&
        handleStatsRequest(cmd, transport)) {
      break;
    }
    if (strncmp(cmd.c_str(), "prof", 4) == 0 &&
        handleProfileRequest(cmd, transport)) {
      break;
    }
    if (strncmp(cmd.c_str(), "dump-apc", 8) == 0 &&
        handleDumpCacheRequest(cmd, transport)) {
      break;
    }
    if (strncmp(cmd.c_str(), "invalidate-units", 16) == 0 &&
        handleInvalidateUnitRequest(cmd, transport)) {
      break;
    }
    if (strncmp(cmd.c_str(), "xenon-snap", 10) == 0) {
      static int64_t s_lastSampleTime = 0;
      auto const current = TimeStamp::Current();
      if (current > s_lastSampleTime) {
        s_lastSampleTime = current;
        Xenon::getInstance().surpriseAll();
      }
      transport->sendString("a Xenon sample will be collected\n", 200);
      break;
    }
    if (strncmp(cmd.c_str(), "hugepage", 9) == 0) {
#if USE_JEMALLOC_EXTENT_HOOKS
      std::string msg =
        folly::sformat("{} 1G huge pages active\n", num_1g_pages());
      if (auto a = alloc::lowArena()) {
        msg += a->reportStats();
      }
      if (auto a = alloc::highArena()) {
        msg += a->reportStats();
      }
      transport->sendString(msg, 200);
#else
      transport->sendString("", 200);
#endif
      break;
    }
    if (strncmp(cmd.c_str(), "jit-des-info", 13) == 0) {
      if (!jit::ProfData::wasDeserialized()) {
        transport->sendString("", 200);
        break;
      }
      auto msg = folly::sformat("{}:{}",
                                jit::ProfData::buildHost()->slice(),
                                jit::ProfData::buildTime());
      transport->sendString(msg, 200);
    }

    if (strncmp(cmd.c_str(), "const-ss", 8) == 0 &&
        handleConstSizeRequest(cmd, transport)) {
      break;
    }
    if (strncmp(cmd.c_str(), "static-strings", 14) == 0 &&
        handleStaticStringsRequest(cmd, transport)) {
      break;
    }
    if (strncmp(cmd.c_str(), "dump-static-strings", 19) == 0) {
      auto filename = transport->getParam("file");
      if (filename == "") filename = "/tmp/static_strings";
      handleDumpStaticStringsRequest(cmd, filename);
      transport->sendString("OK\n");
      break;
    }
    if (strncmp(cmd.c_str(), "random-static-strings", 21) == 0) {
      handleRandomStaticStringsRequest(cmd, transport);
      break;
    }
    if (strncmp(cmd.c_str(), "vm-", 3) == 0 &&
        handleVMRequest(cmd, transport)) {
      break;
    }
    if (cmd == "proxy") {
      handleProxyRequest(cmd, transport);
      break;
    }

    if (cmd == "statcache-clear") {
      StatCache::clearCache();
      break;
    }

    if (cmd == "pcre-cache-size") {
      std::ostringstream size;
      size << preg_pcre_cache_size() << endl;
      transport->sendString(size.str());
      break;
    }

    if (cmd == "dump-pcre-cache") {
      auto filename = transport->getParam("file");
      if (filename == "") filename = "/tmp/pcre_cache";
      pcre_dump_cache(filename);
      transport->sendString("OK\n");
      break;
    }

    if (cmd == "start-stacktrace-profiler") {
      enable_stacktrace_profiler = true;
      transport->sendString("OK\n");
      break;
    }

    if (cmd == "relocate") {
      auto randomParam = transport->getParam("random");
      auto allParam = transport->getParam("all");
      auto time = transport->getIntParam("time");
      bool random = randomParam == "true" || randomParam == "1";
      if (allParam == "true" || allParam == "1") {
        jit::tc::liveRelocate(-2);
      } else if (random || time == 0) {
        jit::tc::liveRelocate(random);
      } else {
        jit::tc::liveRelocate(time);
      }
      transport->sendString("OK\n");
      break;
    }

    if (cmd == "warmup-status") {
      transport->sendString(jit::tc::warmupStatusString());
      break;
    }

    if (cmd == "advise-out-apc") {
      if (!apcExtension::Enable) {
        transport->sendString("No APC\n");
        break;
      }
      apc_advise_out();
      transport->sendString("Done\n");
      break;
    }

    if (strncmp(cmd.c_str(), "random-apc", 10) == 0 &&
        handleRandomApcRequest(cmd, transport)) {
      break;
    }
    if (cmd == "treadmill") {
      transport->sendString(Treadmill::dumpTreadmillInfo());
      break;
    }

    if (cmd == "load-factor") {
      auto const factorStr = transport->getParam("set");
      if (factorStr.empty()) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3)
            << HttpServer::LoadFactor.load();
        transport->sendString(oss.str());
        break;
      }
      double factor = 1.0;
      if (sscanf(factorStr.c_str(), "%lf", &factor) < 1 ||
          factor > 10 || factor < -1) {
        transport->sendString("Invalid load factor spec: " + factorStr, 400);
        break;
      }
      HttpServer::LoadFactor.store(factor, std::memory_order_relaxed);
      transport->sendString(folly::sformat("Load factor updated to {}\n",
                                           factor));
      Logger::Info("Load factor updated to %lf", factor);
      break;
    }
    if (cmd == "queue-discount") {
      auto const discountStr = transport->getParam("set");
      if (discountStr.empty()) {
        transport->sendString(folly::to<string>(
          HttpServer::QueueDiscount.load(std::memory_order_relaxed)));
        break;
      }
      int queue_discount = 0;
      if (sscanf(discountStr.c_str(), "%d", &queue_discount) < 1 ||
          queue_discount > 10000 || queue_discount < 0) {
        transport->sendString("Invalid queue discount spec: " +
          discountStr, 400);
        break;
      }
      HttpServer::QueueDiscount.store(queue_discount,
        std::memory_order_relaxed);
      transport->sendString(folly::sformat("Queue Discount updated to {}\n",
        queue_discount));
      Logger::Info("Queue Discount updated to %d", queue_discount);
      break;
    }
    if (cmd == "ini-get-all") {
      auto out = IniSetting::GetAllAsJSON();
      transport->sendString(out.c_str());
      break;
    }

    if (cmd == "numa-info") {
      std::ostringstream out;
#ifdef HAVE_NUMA
      out << "use_numa: " << use_numa << endl;
      out << "numa_num_nodes: " << numa_num_nodes << endl;
      out << "numa_node_mask: " << numa_node_mask << endl;
      out << "numa_node_set: " << numa_node_set << endl;
#else
      out << "HAVE_NUMA not defined" << endl;
#endif
      transport->sendString(out.str());
      break;
    }

#ifdef USE_JEMALLOC
    assertx(mallctlnametomib && mallctlbymib);
    if (cmd == "jemalloc-stats") {
      // jemalloc stats update is periodically triggered in the
      // host-health-monitor thread.
      uint32_t error = 0;

      auto call_mallctl = [&](const char* statName) {
        size_t value = 0;
        if (mallctlRead<size_t, true>(statName, &value) != 0) {
          error = 1;
        }
        return value;
      };
      size_t allocated = call_mallctl("stats.allocated");
      size_t active = call_mallctl("stats.active");
      size_t mapped = call_mallctl("stats.mapped");

#if USE_JEMALLOC_EXTENT_HOOKS
      size_t low_mapped = 0;
      // The low range [1G, 4G) is divided into two ranges, and shared by 3
      // arenas.
      low_mapped += alloc::getRange(alloc::AddrRangeClass::VeryLow).used();
      low_mapped += alloc::getRange(alloc::AddrRangeClass::Low).used();
#else
      size_t low_mapped = call_mallctl(
          folly::sformat("stats.arenas.{}.mapped",
                         low_arena).c_str());
#endif
      std::ostringstream stats;
      stats << "<jemalloc-stats>" << endl;
      stats << "  <allocated>" << allocated << "</allocated>" << endl;
      stats << "  <active>" << active << "</active>" << endl;
      stats << "  <mapped>" << mapped << "</mapped>" << endl;
      stats << "  <low_mapped>" << low_mapped << "</low_mapped>" << endl;
      stats << "</jemalloc-stats>" << endl;
      transport->sendString(stats.str());
      break;
    }
    if (cmd == "jemalloc-stats-print") {
      malloc_write mwo;

      malloc_write_init(&mwo);
      malloc_stats_print(malloc_write_cb, (void *)&mwo, "");
      if (mwo.oom) {
        malloc_write_fini(&mwo);
        transport->sendString("OOM\n");
        break;
      }

      transport->sendString(mwo.s == nullptr ? "" : mwo.s);
      malloc_write_fini(&mwo);
      break;
    }
    if (cmd == "jemalloc-prof-activate") {
      if (jemalloc_pprof_enable()) {
        transport->sendString("Error in mallctl(\"prof.active\", true)\n");
      } else {
        transport->sendString("OK\n");
      }
      break;
    }
    if (cmd == "jemalloc-prof-deactivate") {
      if (jemalloc_pprof_disable()) {
        transport->sendString("Error in mallctl(\"prof.active\", false)\n");
      } else {
        transport->sendString("OK\n");
      }
      break;
    }
    if (cmd == "jemalloc-prof-dump") {
      string f = transport->getParam("file");
      if (jemalloc_pprof_dump(f, true)) {
        transport->sendString("Error in mallctl(\"prof.dump\", " + f + ")\n");
      } else {
        transport->sendString("OK\n");
      }
      break;
    }
    if (cmd == "jemalloc-prof-request") {
      auto f = transport->getParam("file");
      bool success = MemoryManager::triggerProfiling(f);

      if (success) {
        transport->sendString("OK\n");
      } else {
        transport->sendString("Request profiling already triggered\n");
      }
      break;
    }
#ifdef ENABLE_HHPROF
    if (cmd == "hhprof/start") {
      HHProf::HandleHHProfStart(transport);
      break;
    }
    if (cmd == "hhprof/status") {
      HHProf::HandleHHProfStatus(transport);
      break;
    }
    if (cmd == "hhprof/stop") {
      HHProf::HandleHHProfStop(transport);
      break;
    }
    if (cmd == "pprof/cmdline") {
      HHProf::HandlePProfCmdline(transport);
      break;
    }
    if (cmd == "pprof/heap") {
      HHProf::HandlePProfHeap(transport);
      break;
    }
    if (cmd == "pprof/symbol") {
      HHProf::HandlePProfSymbol(transport);
      break;
    }
#endif // ENABLE_HHPROF
#endif // USE_JEMALLOC

    if (cmd == "rqtrace-stats") {
      std::stringstream out;
      bool first = true;
      out << "{" << endl;
      auto appendStat =
        [&](folly::StringPiece name, folly::StringPiece kind, int64_t value) {
          out << folly::format(
            "{} \"{}_{}\":{}\n", first ? "" : ",", name, kind, value);
          first = false;
        };
      rqtrace::visit_process_stats(
        [&] (const StringData* name, rqtrace::EventStats stats) {
          appendStat(name->data(), "duration", stats.total_duration);
          appendStat(name->data(), "count", stats.total_count);
        }
      );
      out << "}" << endl;
      transport->sendString(out.str());
      break;
    }

    if (AdminCommandExt::iterate([&](AdminCommandExt* ace) {
          return ace->handleRequest(transport);
       })) {
      break;
    }

    transport->sendString("Unknown command: " + cmd + "\n", 404);
  } while (0);
  transport->onSendEnd();
}

void AdminRequestHandler::abortRequest(Transport *transport) {
  g_context.getCheck();
  transport->sendString("Service Unavailable", 503);
  transport->onSendEnd();
}

///////////////////////////////////////////////////////////////////////////////
// stats commands

static bool toggle_switch(Transport *transport, bool &setting) {
  setting = !setting;
  transport->sendString(setting ? "On\n" : "Off\n");
  return true;
}

static bool send_report(Transport *transport) {
  std::string keys = transport->getParam("keys");
  std::string prefix = transport->getParam("prefix");

  std::string out = ServerStats::Report(keys, prefix);

  transport->replaceHeader("Content-Type", "text/plain");
  transport->sendString(out);
  return true;
}

static bool send_status(Transport *transport, Writer::Format format,
                        const char *mime) {
  transport->replaceHeader("Content-Type", mime);
  transport->sendString(ServerStats::ReportStatus(format));
  return true;
}

bool AdminRequestHandler::handleCheckRequest(const std::string &cmd,
                                             Transport *transport) {
  if (cmd == "check-load") {
    int count = HttpServer::Server->getPageServer()->getActiveWorker();
    transport->sendString(folly::to<std::string>(count));
    return true;
  }
  if (cmd == "check-ev") {
    int count =
      HttpServer::Server->getPageServer()->getLibEventConnectionCount();
    transport->sendString(folly::to<string>(count));
    return true;
  }
  if (cmd == "check-queued") {
    int count = HttpServer::Server->getPageServer()->getQueuedJobs();
    transport->sendString(folly::to<string>(count));
    return true;
  }
  if (cmd == "check-health") {
    std::stringstream out;
    bool first = true;
    out << "{" << endl;
    auto const appendStat = [&](folly::StringPiece name, int64_t value) {
       out << folly::format("{} \"{}\":{}\n", first ? "" : ",", name, value);
       first = false;
    };
    appendStat("hhbc-roarena-capac", hhbc_arena_capacity());
    appendStat("hhbc-size", g_hhbc_size->getSum());
    appendStat("rds", rds::usedBytes());
    appendStat("rds-local", rds::usedLocalBytes());
    appendStat("rds-persistent", rds::usedPersistentBytes());
    appendStat("catch-traces", jit::numCatchTraces());
    appendStat("fixups", jit::FixupMap::size());
    appendStat("units", numLoadedUnits());
    appendStat("funcs", Func::nextFuncId());
    appendStat("named-entities", NamedEntity::tableSize());
    for (auto& pair : NamedEntity::tableStats()) {
      appendStat(folly::sformat("named-entities-{}", pair.first), pair.second);
    }
    appendStat("static-strings", makeStaticStringCount());
    appendStat("request-count", requestCount());
    appendStat("jit-des", jit::ProfData::triedDeserialization());
    appendStat("jit-des-succ", jit::ProfData::wasDeserialized());

    /*
     * We're only using globalProfData() here because admin requests don't call
     * requestInitProfData(). Normal request threads should always use
     * profData() which provides much better guarantees about the lifetime of
     * the returned object.
     */
    if (auto profData = jit::globalProfData()) {
      appendStat("prof-funcs", profData->profilingFuncs());
      appendStat("prof-bc", profData->profilingBCSize());
      appendStat("opt-funcs", profData->optimizedFuncs());
    }

    if (RuntimeOption::EvalEnableReusableTC) {
      auto const memInfos = jit::tc::getTCMemoryUsage();
      for (auto const info : memInfos) {
        appendStat(folly::format("tc-{}-allocs", info.name).str(), info.allocs);
        appendStat(folly::format("tc-{}-frees", info.name).str(), info.frees);
        appendStat(folly::format("tc-{}-free-size", info.name).str(),
                   info.free_size);
        appendStat(folly::format("tc-{}-free-blocks", info.name).str(),
                   info.free_blocks);
      }
      appendStat("tc-recorded-funcs", jit::tc::recordedFuncs());
      appendStat("tc-smashed-calls", jit::tc::smashedCalls());
      appendStat("tc-smashed-branches", jit::tc::smashedBranches());
    }

    out << "}" << endl;
    transport->sendString(out.str());
    return true;
  }
  if (cmd == "check-pl-load") {
    int count = PageletServer::GetActiveWorker();
    transport->sendString(folly::to<string>(count));
    return true;
  }
  if (cmd == "check-pl-queued") {
    int count = PageletServer::GetQueuedJobs();
    transport->sendString(folly::to<string>(count));
    return true;
  }
  if (cmd == "check-sat") {
    std::vector<std::pair<std::string, int>> stats;
    HttpServer::Server->getSatelliteStats(&stats);
    std::stringstream out;
    bool first = true;
    out << "{" << endl;
    auto appendStat = [&](const std::string &name, int value) {
       out << (!first ? "," : "") << "  \"" << name << "\":" << value << endl;
       first = false;
    };
    for (auto i : stats) {
      appendStat(i.first, i.second);
    }
    out << "}" << endl;
    transport->sendString(out.str());
    return true;
  }
  if (cmd == "check-sql") {
    string stats = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    stats += "<SQL>\n";
#ifdef ENABLE_EXTENSION_MYSQL
    stats += MySqlStats::ReportStats();
#endif
    stats += "</SQL>\n";
    transport->sendString(stats);
    return true;
  }
  return false;
}

bool AdminRequestHandler::handleStatusRequest(const std::string &cmd,
                                              Transport *transport) {
  if (cmd == "status.xml") {
    return send_status(transport, Writer::Format::XML, "application/xml");
  }
  if (cmd == "status.json") {
    return send_status(transport, Writer::Format::JSON,
                       "application/json");
  }
  if (cmd == "status.html" || cmd == "status.htm") {
    return send_status(transport, Writer::Format::HTML, "text/html");
  }
  return false;
}

bool AdminRequestHandler::handleInvalidateUnitRequest(const std::string &cmd,
                                                      Transport *transport) {
  if (RuntimeOption::RepoAuthoritative) {
    transport->sendString("Cannot invalidate units in repo authoritative mode\n", 400);
    return true;
  }

  std::string invalidated;
  std::vector<std::string> paths;
  // Support GET for convenience, and POST for large requests.
  transport->getArrayParam("path", paths, Transport::Method::AUTO);

  bool first = true;
  for (auto const& path : paths) {
    String translatedPath = File::TranslatePathKeepRelative(path);
    invalidateUnit(translatedPath.get());

    if (first) {
      first = false;
    } else {
      invalidated += " ";
    }
    invalidated += translatedPath.c_str();
  }

  auto msg = folly::sformat("Invalidated {} path(s): {}\n",
                            paths.size(), invalidated);
  transport->sendString(msg);
  return true;
}

bool AdminRequestHandler::handleMemoryRequest(const std::string &cmd,
                                              Transport *transport){
  std::string out;
  if (cmd == "memory.xml") {
      MemoryStats::GetInstance()->ReportMemory(out, Writer::Format::XML);
      transport->replaceHeader("Content-Type","application/xml");
      transport->sendString(out);
      return true;
  }
  if (cmd == "memory.json") {
      MemoryStats::GetInstance()->ReportMemory(out, Writer::Format::JSON);
      transport->replaceHeader("Content-Type","application/json");
      transport->sendString(out);
      return true;
  }
  if (cmd == "memory.html" || cmd == "memory.htm") {
      MemoryStats::GetInstance()->ReportMemory(out, Writer::Format::HTML);
      transport->replaceHeader("Content-Type","text/html");
      transport->sendString(out);
      return true;
  }
  return false;
}

bool AdminRequestHandler::handleStatsRequest(const std::string &cmd,
                                             Transport *transport) {
  if (cmd == "stats-on") {
    RuntimeOption::EnableStats = true;
    transport->sendString("OK\n");
    return true;
  }
  if (cmd == "stats-off") {
    RuntimeOption::EnableStats = false;
    transport->sendString("OK\n");
    return true;
  }
  if (cmd == "stats-clear") {
    ServerStats::Clear();
    transport->sendString("OK\n");
    return true;
  }

  if (cmd == "stats-web") {
    return toggle_switch(transport, RuntimeOption::EnableWebStats);
  }
  if (cmd == "stats-mem") {
    toggle_switch(transport, RuntimeOption::EnableMemoryStats);
    return true;
  }
  if (cmd == "stats-sql") {
    return toggle_switch(transport, RuntimeOption::EnableSQLStats);
  }
  if (cmd == "stats-mutex") {
    int sampling = transport->getIntParam("sampling");
    if (sampling > 0) {
      LockProfiler::s_profile_sampling = sampling;
    }
    return toggle_switch(transport, LockProfiler::s_profile);
  }

  if (cmd == "stats.keys") {
    transport->sendString(ServerStats::GetKeys());
    return true;
  }
  if (cmd == "stats.kvp") {
    return send_report(transport);
  }

  if (cmd == "stats.xsl") {
    string xsl;
    if (!RuntimeOption::StatsXSLProxy.empty()) {
      StringBuffer response;
      if (HttpClient().get(RuntimeOption::StatsXSLProxy.c_str(), response) ==
          200) {
        xsl = response.data();
        if (!xsl.empty()) {
          transport->replaceHeader("Content-Type", "application/xml");
          transport->sendString(xsl);
          return true;
        }
      }
    }
    transport->sendString("Not Found\n", 404);
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// profile commands

bool AdminRequestHandler::handleProfileRequest(const std::string &cmd,
                                               Transport *transport) {
  if (cmd == "prof-exe") {
    std::map<RequestInfo::Executing, int> counts;
    RequestInfo::GetExecutionSamples(counts);

    string res = "[ ";
    for (std::map<RequestInfo::Executing, int>::const_iterator iter =
           counts.begin(); iter != counts.end(); ++iter) {
      res += folly::to<string>(iter->first) + ", " +
        folly::to<string>(iter->second) + ", ";
    }
    res += "-1 ]";
    transport->sendString(res);

    return true;
  }
#ifdef GOOGLE_CPU_PROFILER
  if (handleCPUProfilerRequest(cmd, transport)) {
    return true;
  }
#endif
  return false;
}

#ifdef GOOGLE_CPU_PROFILER
bool AdminRequestHandler::handleCPUProfilerRequest(const std::string &cmd,
                                                   Transport *transport) {
  string file = RuntimeOption::ProfilerOutputDir + "/" +
    Process::HostName + "/hphp.prof";

  if (cmd == "prof-cpu-on") {
    if (FileUtil::mkdir(file)) {
      ProfilerStart(file.c_str());
      transport->sendString("OK\n");
    } else {
      transport->sendString("Unable to mkdir for profile data.\n");
    }
    return true;
  }
  if (cmd == "prof-cpu-off") {
    ProfilerStop();
    ProfilerFlush();
    transport->sendString("OK\n");
    return true;
  }
  return false;
}
#endif

bool AdminRequestHandler::handleConstSizeRequest(const std::string &cmd,
                                                 Transport *transport) {
  if (!apcExtension::EnableConstLoad && cmd == "const-ss") {
    transport->sendString("Not Enabled\n");
    return true;
  }
  if (cmd == "const-ss") {
    std::ostringstream result;
    size_t size = get_const_map_size();
    result << "{ \"hphp.const_map.size\":" << size << "}\n";
    transport->sendString(result.str());
    return true;
  }
  return false;
}

bool AdminRequestHandler::handleStaticStringsRequest(const std::string& cmd,
                                                     Transport* transport) {
  if (cmd == "static-strings") {
    std::ostringstream result;
    result << makeStaticStringCount();
    transport->sendString(result.str());
    return true;
  } else if (cmd == "static-strings-rds") {
    std::ostringstream result;
    result << countStaticStringConstants();
    transport->sendString(result.str());
    return true;
  }
  return false;
}

std::string formatStaticString(StringData* str) {
  return folly::sformat(
      "----\n{} bytes\n{}\n", str->size(), str->toCppString());
}

bool AdminRequestHandler::handleDumpStaticStringsRequest(
  const std::string& /*cmd*/, const std::string& filename) {
  std::vector<StringData*> list = lookupDefinedStaticStrings();
  std::ofstream out(filename.c_str());
  SCOPE_EXIT { out.close(); };
  for (auto item : list) {
    out << formatStaticString(item);
    if (RuntimeOption::EvalPerfDataMap) {
      size_t len = item->size();
      if (len > 255) len = 255;
      std::string str(item->data(), len);
      // Only print the first line (up to 255 characters). Since we want '\0' in
      // the list of characters to avoid, we need to use the version of
      // `find_first_of()' with explicit length.
      auto cutOffPos = str.find_first_of("\r\n", 0, 3);
      if (cutOffPos != std::string::npos) str.erase(cutOffPos);
      Debug::DebugInfo::recordDataMap(item->mutableData(),
                                      item->mutableData() + item->size(),
                                      "Str-" + str);
    }
  }
  return true;
}

bool AdminRequestHandler::handleRandomStaticStringsRequest(
  const std::string& /*cmd*/, Transport* transport) {
  size_t count = 1;
  auto countParam = transport->getParam("count");
  if (countParam != "") {
    try {
      count = folly::to<size_t>(countParam);
    } catch (...) {
      // do the default on invalid input
    }
  }
  std::string output;
  std::vector<StringData*> list = lookupDefinedStaticStrings();
  if (count < list.size()) {
    for (size_t i = 0; i < count; i++) {
      size_t j = folly::Random::rand64(i, list.size());
      std::swap(list[i], list[j]);
    }
    list.resize(count);
  }
  for (auto item : list) {
    output += formatStaticString(item);
  }
  transport->sendString(output);
  return true;
}

bool AdminRequestHandler::handleVMRequest(const std::string &cmd,
                                          Transport *transport) {
  if (cmd == "vm-tcspace") {
    transport->sendString(jit::tc::getTCSpace());
    return true;
  }
  if (cmd == "vm-tcaddr") {
    transport->sendString(jit::tc::getTCAddrs());
    return true;
  }
  if (cmd == "vm-namedentities") {
    std::ostringstream result;
    result << NamedEntity::tableSize();
    transport->sendString(result.str());
    return true;
  }
  if (cmd == "vm-dump-tc") {
    if (jit::tc::dump()) {
      transport->sendString("Done");
    } else {
      transport->sendString("Error dumping the translation cache");
    }
    return true;
  }
  return false;
}

void AdminRequestHandler::handleProxyRequest(const std::string& /*cmd*/,
                                             Transport* transport) {
  try {
    auto const percentStr = transport->getParam("percentage");
    auto const percent    = percentStr.empty() ? 0 : folly::to<int>(percentStr);
    if (percent < 0 || percent > 100) {
      throw std::range_error("must be in [0, 100]");
    }

    setProxyOriginPercentage(transport->getParam("origin"), percent);
    transport->sendString("Origin and percentage updated");
  } catch (const std::range_error& re) {
    transport->sendString(folly::sformat("Invalid percentage: {}", re.what()));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Dump cache content

bool AdminRequestHandler::handleDumpCacheRequest(const std::string &cmd,
                                                 Transport *transport) {
  if (cmd == "dump-apc") {
    if (!apcExtension::Enable) {
      transport->sendString("No APC\n");
      return true;
    }
    string keyOnlyParam = transport->getParam("keyonly");
    bool keyOnly = false;
    if (keyOnlyParam == "true" || keyOnlyParam == "1") {
      keyOnly = true;
    }
    apc_dump("/tmp/apc_dump", keyOnly, false);
    transport->sendString("Done");
    return true;
  }
  if (cmd == "dump-apc-prefix") {
    if (!apcExtension::Enable) {
      transport->sendString("No APC\n");
      return true;
    }
    auto const prefix = transport->getParam("prefix");
    if (prefix.empty()) {
      transport->sendString("No prefix provided\n");
      return true;
    }
    auto const countStr = transport->getParam("count");

    uint32_t count = 1;
    try {
      count = countStr.empty() ? count : folly::to<uint32_t>(countStr);
    } catch (...) {
      // use default if invalid count
    }
    apc_dump_prefix("/tmp/apc_dump_prefix", prefix, count);
    transport->sendString("Done");
    return true;
  }
  if (cmd == "dump-apc-info") {
    if (!apcExtension::Enable) {
      transport->sendString("No APC\n");
      return true;
    }
    transport->sendString(APCStats::getAPCStats().getStatsInfo());
    return true;
  }
  if (cmd == "dump-apc-meta") {
    if (!apcExtension::Enable) {
      transport->sendString("No APC\n");
      return true;
    }
    apc_dump("/tmp/apc_dump_meta", false, true);
    transport->sendString("Done");
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AdminRequestHandler::handleRandomApcRequest(const std::string& /*cmd*/,
                                                 Transport* transport) {
  std::ostringstream out;
  uint32_t keyCount = 1;
  std::string count = transport->getParam("count");
  if (count != "") {
    try {
      keyCount = folly::to<int64_t>(count);
    } catch (...) {
      // set keyCount to 1 if an invalid string is passed
      keyCount = 1;
    }
  }
  apc_get_random_entries(out, keyCount);
  transport->sendString(out.str());

  return true;
}
}
