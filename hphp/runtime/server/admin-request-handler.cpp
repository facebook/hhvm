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

#include "hphp/runtime/server/admin-request-handler.h"

#include "hphp/runtime/base/apc-file-storage.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/thread-hooks.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/recycle-tc.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/repo.h"

#include "hphp/runtime/ext/apc/ext_apc.h"
#include "hphp/runtime/ext/json/ext_json.h"
#include "hphp/runtime/ext/mysql/mysql_stats.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/server/memory-stats.h"
#include "hphp/runtime/server/pagelet-server.h"
#include "hphp/runtime/server/server-stats.h"

#include "hphp/util/alloc.h"
#include "hphp/util/logger.h"
#include "hphp/util/mutex.h"
#include "hphp/util/process.h"
#include "hphp/util/repo-schema.h"
#include "hphp/util/stacktrace-profiler.h"
#include "hphp/util/timer.h"

#include <folly/Conv.h>

#include <boost/lexical_cast.hpp>

#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

#include <unistd.h>

#ifdef GOOGLE_CPU_PROFILER
#include <google/profiler.h>
#include "hphp/runtime/base/file-util.h"
#endif

namespace HPHP {

using std::endl;
using std::string;

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL(AccessLog::ThreadData,
                       AdminRequestHandler::s_accessLogThreadData);

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

  if (mw->oom) {
    return;
  }

  if (mw->slen + slen+1 >= mw->smax) {
    assert(mw->slen + slen > 0);
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

extern unsigned low_arena;
#endif

void AdminRequestHandler::logToAccessLog(Transport* transport) {
  GetAccessLog().onNewRequest();
  GetAccessLog().log(transport, nullptr);
}

void AdminRequestHandler::setupRequest(Transport* transport) {
  g_context.getCheck();
  GetAccessLog().onNewRequest();
}

void AdminRequestHandler::teardownRequest(Transport* transport) noexcept {
  SCOPE_EXIT { hphp_memory_cleanup(); };
  GetAccessLog().log(transport, nullptr);
}

void AdminRequestHandler::handleRequest(Transport *transport) {
  transport->addHeader("Content-Type", "text/plain");
  std::string cmd = transport->getCommand();

  do {
    if (cmd == "" || cmd == "help") {
      string usage =
        "/stop:            stop the web server\n"
        "    instance-id   optional, if specified, instance ID has to match\n"
        "/translate:       translate hex encoded stacktrace in 'stack' param\n"
        "    stack         required, stack trace to translate\n"
        "    build-id      optional, if specified, build ID has to match\n"
        "    bare          optional, whether to display frame ordinates\n"
        "/build-id:        returns build id that's passed in from command line"
        "\n"
        "/instance-id:     instance id that's passed in from command line\n"
        "/compiler-id:     returns the compiler id that built this app\n"
        "/repo-schema:     return the repo schema id used by this app\n"
        "/check-load:      how many threads are actively handling requests\n"
        "/check-queued:    how many http requests are queued waiting to be\n"
        "                  handled\n"
        "/check-health:    return json containing basic load/usage stats\n"
        "/check-ev:        how many http requests are active by libevent\n"
        "/check-pl-load:   how many pagelet threads are actively handling\n"
        "                  requests\n"
        "/check-pl-queued: how many pagelet requests are queued waiting to\n"
        "                  be handled\n"
        "/check-mem:       report memory quick statistics in log file\n"
        "/check-sql:       report SQL table statistics\n"
        "/check-sat        how many satellite threads are actively handling\n"
        "                  requests and queued waiting to be handled\n"
        "/status.xml:      show server status in XML\n"
        "/status.json:     show server status in JSON\n"
        "/status.html:     show server status in HTML\n"

        "/memory.xml:      show memory status in XML\n"
        "/memory.json:     show memory status in JSON\n"
        "/memory.html:     show memory status in HTML\n"

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
        "/stats.xml:       show server stats in XML\n"
        "    from          optional, <timestamp>, or <-n> second ago\n"
        "    to            optional, <timestamp>, or <-n> second ago\n"
        "    agg           optional, aggragation: *, url, code\n"
        "    keys          optional, <key>,<key/hit>,<key/sec>,<:regex:>\n"
        "    url           optional, only stats of this page or URL\n"
        "    code          optional, only stats of pages returning this code\n"
        "/stats.json:      show server stats in JSON\n"
        "    (same as /stats.xml)\n"
        "/stats.kvp:       show server stats in key-value pairs\n"
        "    (same as /stats.xml)\n"
        "/stats.html:      show server stats in HTML\n"
        "    (same as /stats.xml)\n"

        "/const-ss:        get const_map_size\n"
        "/static-strings:  get number of static strings\n"
        "/dump-static-strings: dump static strings to /tmp/static_strings\n"
        "/dump-apc:        dump all current value in APC to /tmp/apc_dump\n"
        "/dump-apc-info:   show basic APC stats\n"
        "/dump-apc-meta:   dump meta information for all objects in APC to\n"
        "                  /tmp/apc_dump_meta\n"
        "/advise-out-apc:  forcibly madvise out APC prime data\n"
        "/random-apc:      dump the key and size of a random APC entry\n"
        "    count         number of entries to return\n"

        "/pcre-cache-size: get pcre cache map size\n"
        "/dump-pcre-cache: dump cached pcre's to /tmp/pcre_cache\n"
        "/dump-array-info: dump array tracer info to /tmp/array_tracer_dump\n"

        "/start-stacktrace-profiler: set enable_stacktrace_profiler to true\n"
        "/relocate:        relocate translations\n"
        "    random        optional, default false, relocate random subset\n"
        "       all        optional, default false, relocate all translations\n"
        "      time        optional, default 20 (seconds)\n"

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
        "/thread-mem-usage:show memory usage per thread\n"
        "/proxy:           set up request proxy\n"
        "    origin        URL to proxy requests to\n"
        "    percentage    percentage of requests to proxy\n"
        ;
#ifdef USE_TCMALLOC
        if (MallocExtensionInstance) {
          usage.append(
              "/free-mem:        ask tcmalloc to release memory to system\n"
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
        }
#endif

      transport->sendString(usage);
      break;
    }

    bool needs_password = (cmd != "build-id") && (cmd != "compiler-id") &&
                          (cmd != "instance-id");

    if (needs_password && !RuntimeOption::AdminPasswords.empty()) {
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
    if (cmd == "build-id") {
      transport->sendString(RuntimeOption::BuildId, 200);
      break;
    }
    if (cmd == "instance-id") {
      transport->sendString(RuntimeOption::InstanceId, 200);
      break;
    }
    if (cmd == "compiler-id") {
      transport->sendString(kCompilerId, 200);
      break;
    }
    if (cmd == "repo-schema") {
      transport->sendString(kRepoSchemaId, 200);
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
      handleDumpStaticStrings(cmd, transport, filename);
      transport->sendString("OK\n");
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
    if (!strcmp(cmd.c_str(), "thread-mem")) {
      transport->sendString(get_thread_mem_usage());
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
        jit::liveRelocate(-2);
      } else if (random || time == 0) {
        jit::liveRelocate(random);
      } else {
        jit::liveRelocate(time);
      }
      transport->sendString("OK\n");
      break;
    }

    if (cmd == "advise-out-apc") {
      if (!apcExtension::Enable) {
        transport->sendString("No APC\n");
        break;
      }
      s_apc_file_storage.adviseOut();
      transport->sendString("Done\n");
      break;
    }

    if (strncmp(cmd.c_str(), "random-apc", 10) == 0 &&
        handleRandomApcRequest(cmd, transport)) {
      break;
    }

#ifdef USE_TCMALLOC
    if (MallocExtensionInstance) {
      if (cmd == "free-mem") {
        MallocExtensionInstance()->ReleaseFreeMemory();
        transport->sendString("OK\n");
        break;
      }
      if (cmd == "tcmalloc-stats") {
        std::ostringstream stats;
        size_t user_allocated, heap_size, slack_bytes;
        size_t pageheap_free, pageheap_unmapped;
        size_t tc_max, tc_allocated;

        MallocExtensionInstance()->
          GetNumericProperty("generic.current_allocated_bytes",
              &user_allocated);
        MallocExtensionInstance()->
          GetNumericProperty("generic.heap_size", &heap_size);
        MallocExtensionInstance()->
          GetNumericProperty("tcmalloc.slack_bytes", &slack_bytes);
        MallocExtensionInstance()->
          GetNumericProperty("tcmalloc.pageheap_free_bytes", &pageheap_free);
        MallocExtensionInstance()->
          GetNumericProperty("tcmalloc.pageheap_unmapped_bytes",
              &pageheap_unmapped);
        MallocExtensionInstance()->
          GetNumericProperty("tcmalloc.max_total_thread_cache_bytes",
              &tc_max);
        MallocExtensionInstance()->
          GetNumericProperty("tcmalloc.current_total_thread_cache_bytes",
              &tc_allocated);
        stats << "<tcmalloc-stats>" << endl;
        stats << "  <user_allocated>" << user_allocated << "</user_allocated>"
          << endl;
        stats << "  <heap_size>" << heap_size << "</heap_size>" << endl;
        stats << "  <slack_bytes>" << slack_bytes << "</slack_bytes>" << endl;
        stats << "  <pageheap_free>" << pageheap_free
          << "</pageheap_free>" << endl;
        stats << "  <pageheap_unmapped>" << pageheap_unmapped
          << "</pageheap_unmapped>" << endl;
        stats << "  <thread_cache_max>" << tc_max
          << "</thread_cache_max>" << endl;
        stats << "  <thread_cache_allocated>" << tc_allocated
          << "</thread_cache_allocated>" << endl;
        stats << "</tcmalloc-stats>" << endl;
        transport->sendString(stats.str());
        break;
      }
      if (cmd == "tcmalloc-set-tc") {
        size_t tc_max;

        MallocExtensionInstance()->
          GetNumericProperty("tcmalloc.max_total_thread_cache_bytes", &tc_max);

        size_t tcache = transport->getInt64Param("s");
        bool retval = MallocExtensionInstance()->
          SetNumericProperty("tcmalloc.max_total_thread_cache_bytes", tcache);
        transport->sendString(retval == true ? "OK\n" : "FAILED\n");
        break;
      }
    }
#endif

#ifdef USE_JEMALLOC
    if (mallctl) {
      if (cmd == "free-mem") {
        // Purge all dirty unused pages.
        int err = mallctl("arenas.purge", nullptr, nullptr, nullptr, 0);
        if (err) {
          std::ostringstream estr;
          estr << "Error " << err << " in mallctl(\"arenas.purge\", ...)"
            << endl;
          transport->sendString(estr.str());
        } else {
          transport->sendString("OK\n");
        }
        break;
      }
      if (cmd == "jemalloc-stats") {
        // Force jemalloc to update stats cached for use by mallctl().
        uint64_t epoch = 1;
        uint32_t error = 0;
        if (mallctl("epoch", nullptr, nullptr, &epoch, sizeof(epoch)) != 0) {
          error = 1;
        }

        auto call_mallctl = [&](const char* statName) {
          size_t value = 0;
          size_t sz = sizeof(value);
          if (mallctl(statName, &value, &sz, nullptr, 0) != 0){
            error = 1;
          }
          return value;
        };
        size_t allocated = call_mallctl("stats.allocated");
        size_t active = call_mallctl("stats.active");
        size_t mapped = call_mallctl("stats.mapped");
        size_t low_mapped = call_mallctl(
            folly::format("stats.arenas.{}.mapped",
              low_arena).str().c_str());
        size_t low_small_allocated = call_mallctl(
            folly::format("stats.arenas.{}.small.allocated",
              low_arena).str().c_str());
        size_t low_large_allocated = call_mallctl(
            folly::format("stats.arenas.{}.large.allocated",
              low_arena).str().c_str());
        size_t low_active = call_mallctl(
            folly::format("stats.arenas.{}.pactive",
              low_arena).str().c_str()) *
            sysconf(_SC_PAGESIZE);


        std::ostringstream stats;
        stats << "<jemalloc-stats>" << endl;
        stats << "  <allocated>" << allocated << "</allocated>" << endl;
        stats << "  <active>" << active << "</active>" << endl;
        stats << "  <mapped>" << mapped << "</mapped>" << endl;
        stats << "  <low_mapped>" << low_mapped << "</low_mapped>" << endl;
        stats << "  <low_allocated>"
              << (low_small_allocated + low_large_allocated)
              << "</low_allocated>" << endl;
        stats << "  <low_active>"
              << low_active
              << "</low_active>" << endl;
        stats << "  <error>" << error << "</error>" << endl;
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

        transport->sendString(mwo.s);
        malloc_write_fini(&mwo);
        break;
      }
      if (cmd == "jemalloc-prof-activate") {
        int err = jemalloc_pprof_enable();
        if (err) {
          std::ostringstream estr;
          estr << "Error " << err << " in mallctl(\"prof.active\", ...)"
            << endl;
          transport->sendString(estr.str());
        } else {
          transport->sendString("OK\n");
        }
        break;
      }
      if (cmd == "jemalloc-prof-deactivate") {
        int err = jemalloc_pprof_disable();
        if (err) {
          std::ostringstream estr;
          estr << "Error " << err << " in mallctl(\"prof.active\", ...)"
            << endl;
          transport->sendString(estr.str());
        } else {
          transport->sendString("OK\n");
        }
        break;
      }
      if (cmd == "jemalloc-prof-dump") {
        string f = transport->getParam("file");
        int err = jemalloc_pprof_dump(f, true);
        if (err) {
          std::ostringstream estr;
          estr << "Error " << err << " in mallctl(\"prof.dump\", ...";
          if (!f.empty()) {
            estr << ", \"" << f << "\", ...";
          }
          estr << ")" << endl;
          transport->sendString(estr.str());
          break;
        }
        transport->sendString("OK\n");
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
    }
#endif

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

static bool send_report(Transport *transport, Writer::Format format,
                        const char *mime) {
  int64_t  from   = transport->getInt64Param ("from");
  int64_t  to     = transport->getInt64Param ("to");
  std::string agg    = transport->getParam      ("agg");
  std::string keys   = transport->getParam      ("keys");
  std::string url    = transport->getParam      ("url");
  int    code   = transport->getIntParam   ("code");
  std::string prefix = transport->getParam      ("prefix");

  std::string out;
  ServerStats::Report(out, format, from, to, agg, keys, url, code, prefix);

  transport->replaceHeader("Content-Type", mime);
  transport->sendString(out);
  return true;
}

static bool send_status(Transport *transport, Writer::Format format,
                        const char *mime) {
  string out;
  ServerStats::ReportStatus(out, format);

  transport->replaceHeader("Content-Type", mime);
  transport->sendString(out);
  return true;
}

  extern size_t hhbc_arena_capacity();

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
    auto appendStat = [&](const std::string& name, int64_t value) {
       out << folly::format("{} \"{}\":{}\n",
                            first ? "" : ",", name, value);
       first = false;
    };
    HPHP::Server* server = HttpServer::Server->getPageServer();
    appendStat("load", server->getActiveWorker());
    appendStat("queued", server->getQueuedJobs());
    auto* mCGenerator = jit::mcg;
    appendStat("hhbc-roarena-capac", hhbc_arena_capacity());
    mCGenerator->code.forEachBlock([&](const char* name, const CodeBlock& a) {
      auto isMain = strncmp(name, "main", 4) == 0;
      appendStat(folly::format("tc-{}size",
                               isMain ? "" : name).str(),
                 a.used());
    });
    appendStat("rds", rds::usedBytes());
    appendStat("rds-local", rds::usedLocalBytes());
    appendStat("rds-persistent", rds::usedPersistentBytes());
    appendStat("units", numLoadedUnits());
    appendStat("funcs", Func::nextFuncId());

    if (RuntimeOption::EvalEnableReusableTC) {
      mCGenerator->code.forEachBlock([&](const char* name, const CodeBlock& a) {
        appendStat(folly::format("tc-{}-allocs", name).str(), a.numAllocs());
        appendStat(folly::format("tc-{}-frees", name).str(), a.numFrees());
        appendStat(folly::format("tc-{}-free-size", name).str(), a.bytesFree());
        appendStat(folly::format("tc-{}-free-blocks", name).str(),
                   a.blocksFree());
      });
      appendStat("tc-recorded-funcs", jit::recordedFuncs());
      appendStat("tc-smashed-calls", jit::smashedCalls());
      appendStat("tc-smashed-branches", jit::smashedBranches());
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
    stats += MySqlStats::ReportStats();
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
      MemoryStats::GetInstance()->ReportMemory(out, Writer::Format::XML);
      transport->replaceHeader("Content-Type","application/html");
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
    int64_t from = transport->getInt64Param("from");
    int64_t to = transport->getInt64Param("to");
    string out;
    ServerStats::GetKeys(out, from, to);
    transport->sendString(out);
    return true;
  }
  if (cmd == "stats.xml") {
    return send_report(transport, Writer::Format::XML, "application/xml");
  }
  if (cmd == "stats.json") {
    return send_report(transport, Writer::Format::JSON,
                       "application/json");
  }
  if (cmd == "stats.kvp") {
    return send_report(transport, Writer::Format::KVP, "text/plain");
  }
  if (cmd == "stats.html" || cmd == "stats.htm") {
    return send_report(transport, Writer::Format::HTML, "text/html");
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
    std::map<ThreadInfo::Executing, int> counts;
    ThreadInfo::GetExecutionSamples(counts);

    string res = "[ ";
    for (std::map<ThreadInfo::Executing, int>::const_iterator iter =
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

bool AdminRequestHandler::handleConstSizeRequest (const std::string &cmd,
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
  std::ostringstream result;
  result << makeStaticStringCount();
  transport->sendString(result.str());
  return true;
}

bool AdminRequestHandler::handleDumpStaticStrings(const std::string& cmd,
                                                  Transport* transporti,
                                                  const std::string &filename) {
  std::vector<StringData*> list = lookupDefinedStaticStrings();
  std::ofstream out(filename.c_str());
  SCOPE_EXIT { out.close(); };
  for (auto item : list) {
    out << "----\n";
    out << item->size() << " bytes\n";
    out << item->toCppString() << "\n";
  }
  return true;
}

namespace {
struct PCInfo {
  PCInfo() : count(0), unique(0) {}
  int count;
  int unique;
};
typedef std::map<int, PCInfo> InfoMap;
}

bool AdminRequestHandler::handleVMRequest(const std::string &cmd,
                                          Transport *transport) {
  if (cmd == "vm-tcspace") {
    transport->sendString(jit::mcg->getUsageString());
    return true;
  }
  if (cmd == "vm-tcaddr") {
    transport->sendString(jit::mcg->getTCAddrs());
    return true;
  }
  if (cmd == "vm-namedentities") {
    std::ostringstream result;
    result << NamedEntity::tableSize();
    transport->sendString(result.str());
    return true;
  }
  if (cmd == "vm-dump-tc") {
    if (HPHP::jit::tc_dump()) {
      transport->sendString("Done");
    } else {
      transport->sendString("Error dumping the translation cache");
    }
    return true;
  }
  return false;
}

void AdminRequestHandler::handleProxyRequest(const std::string& cmd,
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
    int waitSeconds = transport->getIntParam("waitseconds");
    if (!waitSeconds) {
      waitSeconds = RuntimeOption::RequestTimeoutSeconds > 0 ?
                    RuntimeOption::RequestTimeoutSeconds : 10;
    }
    apc_dump("/tmp/apc_dump", keyOnly, false, waitSeconds);
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
    int waitSeconds = transport->getIntParam("waitseconds");
    if (!waitSeconds) {
      waitSeconds = RuntimeOption::RequestTimeoutSeconds > 0 ?
        RuntimeOption::RequestTimeoutSeconds : 10;
    }
    apc_dump("/tmp/apc_dump_meta", false, true, waitSeconds);
    transport->sendString("Done");
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AdminRequestHandler::handleRandomApcRequest(const std::string &cmd,
                                                 Transport *transport){
  std::ostringstream out;
  uint32_t keyCount = 1;
  std::string count = transport->getParam("count");
  if (count != "") {
    try {
      keyCount = folly::to<int64_t>(count);
    } catch (const std::invalid_argument& ia) {
      // set keyCount to 1 if an invalid string is passed
      keyCount = 1;
    }
  }
  apc_get_random_entries(out, keyCount);
  transport->sendString(out.str());

  return true;
}
}
