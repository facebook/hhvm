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

#include "hphp/runtime/server/admin_request_handler.h"
#include "hphp/runtime/base/file_repository.h"
#include "hphp/runtime/server/http_server.h"
#include "hphp/runtime/server/pagelet_server.h"
#include "hphp/runtime/base/http_client.h"
#include "hphp/runtime/server/server_stats.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/util/process.h"
#include "hphp/util/logger.h"
#include "hphp/util/util.h"
#include "hphp/util/mutex.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/memory_manager.h"
#include "hphp/runtime/base/program_functions.h"
#include "hphp/runtime/base/shared_store_base.h"
#include "hphp/runtime/base/leak_detectable.h"
#include "hphp/runtime/ext/mysql_stats.h"
#include "hphp/runtime/base/shared_store_stats.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/util/alloc.h"
#include "hphp/util/timer.h"
#include "hphp/util/repo_schema.h"
#include "hphp/runtime/ext/ext_fb.h"
#include "hphp/runtime/ext/ext_apc.h"

#include <sstream>
#include <iomanip>

#ifdef GOOGLE_CPU_PROFILER
#include "google/profiler.h"
#endif
#ifdef GOOGLE_HEAP_PROFILER
#include "google/heap-profiler.h"
#endif

using std::endl;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL(AccessLog::ThreadData,
                       AdminRequestHandler::s_accessLogThreadData);

AccessLog AdminRequestHandler::s_accessLog(
  &(AdminRequestHandler::getAccessLogThreadData));

AdminRequestHandler::AdminRequestHandler() {
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
#endif

void AdminRequestHandler::handleRequest(Transport *transport) {
  GetAccessLog().onNewRequest();
  transport->addHeader("Content-Type", "text/plain");
  string cmd = transport->getCommand();

  do {
    if (cmd == "" || cmd == "help") {
      string usage =
        "/stop:            stop the web server\n"
        "/translate:       translate hex encoded stacktrace in 'stack' param\n"
        "    stack         required, stack trace to translate\n"
        "    build-id      optional, if specified, build ID has to match\n"
        "    bare          optional, whether to display frame ordinates\n"
        "/build-id:        returns build id that's passed in from command line"
        "\n"
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

        "/status.xml:      show server status in XML\n"
        "/status.json:     show server status in JSON\n"
        "/status.html:     show server status in HTML\n"

        "/stats-on:        main switch: enable server stats\n"
        "/stats-off:       main switch: disable server stats\n"
        "/stats-clear:     clear all server stats\n"

        "/stats-web:       turn on/off server page stats (CPU and gen time)\n"
        "/stats-mem:       turn on/off memory statistics\n"
        "/stats-apc:       turn on/off APC statistics\n"
        "/stats-apc-key:   turn on/off APC key statistics\n"
        "/stats-mcc:       turn on/off memcache statistics\n"
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

        "/apc-ss:          get apc size stats\n"
        "/apc-ss-flat:     get apc size stats in flat format\n"
        "/apc-ss-keys:     get apc size break-down on keys\n"
        "/apc-ss-dump:     dump the size info on each key to /tmp/APC_details\n"
        "                  only valid when EnableAPCSizeDetail is true\n"
        "    keysample     optional, only dump keys that belongs to the same\n"
        "                  group as <keysample>\n"
        "/const-ss:        get const_map_size\n"
        "/static-strings:  get number of static strings\n"
        "/dump-apc:        dump all current value in APC to /tmp/apc_dump\n"
        "/dump-const:      dump all constant value in constant map to\n"
        "                  /tmp/const_map_dump\n"
        "/dump-file-repo:  dump file repository to /tmp/file_repo_dump\n"

        "/pcre-cache-size: get pcre cache map size\n"

#ifdef GOOGLE_CPU_PROFILER
        "/prof-cpu-on:     turn on CPU profiler\n"
        "/prof-cpu-off:    turn off CPU profiler\n"
#endif
#ifdef GOOGLE_HEAP_PROFILER
        "/prof-heap-on:    turn on heap profiler\n"
        "/prof-heap-dump:  take one snapshot of the heap\n"
        "/prof-heap-off:   turn off heap profiler\n"
        "/stats-malloc:    turn on/off malloc statistics\n"
        "/leak-on:         start leak detection\n"
        "    sampling      required, frequency\n"
        "/leak-off:        end leak detection and report leaking\n"
        "    cutoff        optional, default 20 seconds, ignore newer allocs\n"
#endif
#ifdef EXECUTION_PROFILER
        "/prof-exe:        returns sampled execution profile\n"
#endif
        "/vm-tcspace:      show space used by translator caches\n"
        "/vm-dump-tc:      dump translation cache to /tmp/tc_dump_a and\n"
        "                  /tmp/tc_dump_astub\n"
        "/vm-tcreset:      throw away translations and start over\n"
        "/vm-namedentities:show size of the NamedEntityTable\n"
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
              );
        }
#endif

      transport->sendString(usage);
      break;
    }

    if (!RuntimeOption::AdminPasswords.empty()) {
      std::set<std::string>::const_iterator iter =
        RuntimeOption::AdminPasswords.find(transport->getParam("auth"));
      if (iter == RuntimeOption::AdminPasswords.end()) {
        transport->sendString("Unauthorized", 401);
        break;
      }
    } else {
      if (!RuntimeOption::AdminPassword.empty() &&
          RuntimeOption::AdminPassword != transport->getParam("auth")) {
        transport->sendString("Unauthorized", 401);
        break;
      }
    }

    if (cmd == "stop") {
      transport->sendString("OK\n");
      Logger::Info("Got admin port stop request from %s",
                   transport->getRemoteHost());
      HttpServer::Server->stop();
      break;
    }
    if (cmd == "build-id") {
      transport->sendString(RuntimeOption::BuildId, 200);
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
    if (strncmp(cmd.c_str(), "stats", 5) == 0 &&
        handleStatsRequest(cmd, transport)) {
      break;
    }
    if (strncmp(cmd.c_str(), "prof", 4) == 0 &&
        handleProfileRequest(cmd, transport)) {
      break;
    }
    if (strncmp(cmd.c_str(), "leak", 4) == 0 &&
        handleLeakRequest(cmd, transport)) {
      break;
    }
    if (strncmp(cmd.c_str(), "apc-ss", 6) == 0 &&
        handleAPCSizeRequest(cmd, transport)) {
      break;
    }
    if (strncmp(cmd.c_str(), "dump", 4) == 0 &&
        handleDumpCacheRequest(cmd, transport)) {
      break;
    }
    if (strncmp(cmd.c_str(), "const-ss", 8) == 0 &&
        handleConstSizeRequest(cmd, transport)) {
      break;
    }
    if (strcmp(cmd.c_str(), "static-strings") == 0 &&
        handleStaticStringsRequest(cmd, transport)) {
      break;
    }
    if (strncmp(cmd.c_str(), "vm-", 3) == 0 &&
        handleVMRequest(cmd, transport)) {
      break;
    }

    if (cmd == "pcre-cache-size") {
      std::ostringstream size;
      size << preg_pcre_cache_size() << endl;
      transport->sendString(size.str());
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
        mallctl("epoch", nullptr, nullptr, &epoch, sizeof(epoch));

        size_t allocated = 0; // Initialize in case stats aren't enabled.
        size_t sz = sizeof(size_t);
        mallctl("stats.allocated", &allocated, &sz, nullptr, 0);

        size_t active = 0;
        mallctl("stats.active", &active, &sz, nullptr, 0);

        size_t mapped = 0;
        mallctl("stats.mapped", &mapped, &sz, nullptr, 0);

        std::ostringstream stats;
        stats << "<jemalloc-stats>" << endl;
        stats << "  <allocated>" << allocated << "</allocated>" << endl;
        stats << "  <active>" << active << "</active>" << endl;
        stats << "  <mapped>" << mapped << "</mapped>" << endl;
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
        bool active = true;
        int err = mallctl("prof.active", nullptr, nullptr, &active, sizeof(bool));
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
        bool active = false;
        int err = mallctl("prof.active", nullptr, nullptr, &active, sizeof(bool));
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
        if (f != "") {
          const char *s = f.c_str();
          int err = mallctl("prof.dump", nullptr, nullptr, (void *)&s,
              sizeof(char *));
          if (err) {
            std::ostringstream estr;
            estr << "Error " << err << " in mallctl(\"prof.dump\", ..., \"" << f
              << "\", ...)" << endl;
            transport->sendString(estr.str());
            break;
          }
        } else {
          int err = mallctl("prof.dump", nullptr, nullptr, nullptr, 0);
          if (err) {
            std::ostringstream estr;
            estr << "Error " << err << " in mallctl(\"prof.dump\", ...)"
              << endl;
            transport->sendString(estr.str());
            break;
          }
        }
        transport->sendString("OK\n");
        break;
      }
    }
#endif

    transport->sendString("Unknown command: " + cmd + "\n", 404);
  } while (0);
  GetAccessLog().log(transport, nullptr);
}

///////////////////////////////////////////////////////////////////////////////
// stats commands

static bool toggle_switch(Transport *transport, bool &setting) {
  setting = !setting;
  transport->sendString(setting ? "On\n" : "Off\n");
  return true;
}

static bool send_report(Transport *transport, ServerStats::Format format,
                        const char *mime) {
  int64_t  from   = transport->getInt64Param ("from");
  int64_t  to     = transport->getInt64Param ("to");
  string agg    = transport->getParam      ("agg");
  string keys   = transport->getParam      ("keys");
  string url    = transport->getParam      ("url");
  int    code   = transport->getIntParam   ("code");
  string prefix = transport->getParam      ("prefix");

  string out;
  ServerStats::Report(out, format, from, to, agg, keys, url, code, prefix);

  transport->addHeader("Content-Type", mime);
  transport->sendString(out);
  return true;
}

static bool send_status(Transport *transport, ServerStats::Format format,
                        const char *mime) {
  string out;
  ServerStats::ReportStatus(out, format);

  transport->addHeader("Content-Type", mime);
  transport->sendString(out);
  return true;
}

  extern size_t hhbc_arena_capacity();

bool AdminRequestHandler::handleCheckRequest(const std::string &cmd,
                                             Transport *transport) {
  if (cmd == "check-load") {
    int count = HttpServer::Server->getPageServer()->getActiveWorker();
    transport->sendString(lexical_cast<string>(count));
    return true;
  }
  if (cmd == "check-ev") {
    int count =
      HttpServer::Server->getPageServer()->getLibEventConnectionCount();
    transport->sendString(lexical_cast<string>(count));
    return true;
  }
  if (cmd == "check-queued") {
    int count = HttpServer::Server->getPageServer()->getQueuedJobs();
    transport->sendString(lexical_cast<string>(count));
    return true;
  }
  if (cmd == "check-health") {
    std::stringstream out;
    bool first = true;
    out << "{" << endl;
    auto appendStat = [&](const char* name, int64_t value) {
       out << (!first ? "," : "") << "  \"" << name << "\":" << value << endl;
       first = false;
    };
    ServerPtr server = HttpServer::Server->getPageServer();
    appendStat("load", server->getActiveWorker());
    appendStat("queued", server->getQueuedJobs());
    Transl::Translator* tx = Transl::Translator::Get();
    appendStat("hhbc-roarena-capac", hhbc_arena_capacity());
    appendStat("tc-size", tx->getCodeSize());
    appendStat("tc-stubsize", tx->getStubSize());
    appendStat("targetcache", tx->getTargetCacheSize());
    appendStat("units", Eval::FileRepository::getLoadedFiles());
    appendStat("Funcs", Func::nextFuncId());
    out << "}" << endl;
    transport->sendString(out.str());
    return true;
  }
  if (cmd == "check-pl-load") {
    int count = PageletServer::GetActiveWorker();
    transport->sendString(lexical_cast<string>(count));
    return true;
  }
  if (cmd == "check-pl-queued") {
    int count = PageletServer::GetQueuedJobs();
    transport->sendString(lexical_cast<string>(count));
    return true;
  }
  if (cmd == "check-mem") {
    return toggle_switch(transport, RuntimeOption::CheckMemory);
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
    return send_status(transport, ServerStats::Format::XML, "application/xml");
  }
  if (cmd == "status.json") {
    return send_status(transport, ServerStats::Format::JSON,
                       "application/json");
  }
  if (cmd == "status.html" || cmd == "status.htm") {
    return send_status(transport, ServerStats::Format::HTML, "text/html");
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
  if (cmd == "stats-malloc") {
    toggle_switch(transport, RuntimeOption::EnableMallocStats);
    LeakDetectable::EnableMallocStats(RuntimeOption::EnableMallocStats);
    return true;
  }
  if (cmd == "stats-apc") {
    return toggle_switch(transport, RuntimeOption::EnableAPCStats);
  }
  if (cmd == "stats-apc-key") {
    return toggle_switch(transport, RuntimeOption::EnableAPCKeyStats);
  }
  if (cmd == "stats-mcc") {
    return toggle_switch(transport, RuntimeOption::EnableMemcacheStats);
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
    return send_report(transport, ServerStats::Format::XML, "application/xml");
  }
  if (cmd == "stats.json") {
    return send_report(transport, ServerStats::Format::JSON,
                       "application/json");
  }
  if (cmd == "stats.kvp") {
    return send_report(transport, ServerStats::Format::KVP, "text/plain");
  }
  if (cmd == "stats.html" || cmd == "stats.htm") {
    return send_report(transport, ServerStats::Format::HTML, "text/html");
  }

  if (cmd == "stats.xsl") {
    string xsl;
    if (!RuntimeOption::StatsXSLProxy.empty()) {
      StringBuffer response;
      if (HttpClient().get(RuntimeOption::StatsXSLProxy.c_str(), response) ==
          200) {
        xsl = response.data();
        if (!xsl.empty()) {
          transport->addHeader("Content-Type", "application/xml");
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
      res += lexical_cast<string>(iter->first) + ", " +
        lexical_cast<string>(iter->second) + ", ";
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
#ifdef GOOGLE_HEAP_PROFILER
  if (handleHeapProfilerRequest(cmd, transport)) {
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
    if (Util::mkdir(file)) {
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

#ifdef GOOGLE_HEAP_PROFILER
bool AdminRequestHandler::handleHeapProfilerRequest(const std::string &cmd,
                                                    Transport *transport) {
  string root = RuntimeOption::ProfilerOutputDir + "/" +
    Process::HostName;
  string file = root + "/hphp.prof";

  if (cmd == "prof-heap-on") {
    if (IsHeapProfilerRunning()) {
      transport->sendString("HeapProfiler is already running.\n");
    } else {
      if (Util::mkdir(file)) {
        // clean up leftovers
        string cmd = "/bin/rm -f ";
        cmd += file + ".*.heap";
        Util::ssystem(cmd.c_str());

        HeapProfilerStart(file.c_str());
        HeapProfilerDump("start");
        transport->sendString("OK\n");
      } else {
        transport->sendString("Unable to mkdir for profile data.\n");
      }
    }
    return true;
  }
  if (cmd == "prof-heap-dump") {
    if (!IsHeapProfilerRunning()) {
      transport->sendString("HeapProfiler is not running.\n");
    } else {
      HeapProfilerDump("end");
      transport->sendString("OK\n");
    }
    return true;
  }
  if (cmd == "prof-heap-off") {
    if (!IsHeapProfilerRunning()) {
      transport->sendString("HeapProfiler is not running.\n");
    } else {
      HeapProfilerDump("end");
      HeapProfilerStop();
      transport->sendString("OK\n");

      const char *argv[] = {"", root.c_str(), "-name", "*.heap", nullptr};
      string files;
      Process::Exec("find", argv, nullptr, files);
      vector<string> out;
      Util::split('\n', files.c_str(), out, true);
      if (out.size() > 1) {
        string base = "--base=";
        base += out[0];
      } else {
        Logger::Error("Unable to find heap profiler output");
      }
    }
    return true;
  }
  return false;
}
#endif

///////////////////////////////////////////////////////////////////////////////
// leak detection

bool AdminRequestHandler::handleLeakRequest(const std::string &cmd,
                                            Transport *transport) {
  if (cmd == "leak-on") {
    LeakDetectable::BeginLeakChecking();   // class-based detection starts
    LeakDetectable::MallocSampling = transport->getIntParam("sampling");
    LeakDetectable::BeginMallocSampling(); // malloc-based detection starts
    transport->sendString("OK\n");
    return true;
  }
  if (cmd == "leak-off") {
    std::string dumps;

    // class-based detection ends
    int count = LeakDetectable::EndLeakChecking(dumps, 1);
    dumps += "\n";
    dumps += boost::lexical_cast<string>(count) + " leaked LeakDetectables\n";

    // malloc-based detection
    int cutoff = transport->getIntParam("cutoff");
    LeakDetectable::EndMallocSampling(dumps, cutoff ? cutoff : 20);
    LeakDetectable::MallocSampling = 0;

    transport->sendString(dumps);
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// APC size profiling

bool AdminRequestHandler::handleAPCSizeRequest (const std::string &cmd,
                                                Transport *transport) {
  if (!RuntimeOption::EnableAPCSizeStats &&
      (cmd == "apc-ss" || cmd == "apc-ss-keys" || cmd == "apc-ss-dump" ||
       cmd == "apc-ss-flat")) {
    transport->sendString("Not Enabled\n");
    return true;
  }
  if (cmd == "apc-ss") {
    std::string result = SharedStoreStats::report_basic();
    transport->sendString(result);
    return true;
  }
  if (cmd == "apc-ss-flat") {
    std::string result = SharedStoreStats::report_basic_flat();
    transport->sendString(result);
    return true;
  }
  if (cmd == "apc-ss-keys") {
    if (!RuntimeOption::EnableAPCSizeGroup) {
      transport->sendString("Not Enabled\n");
      return true;
    }
    std::string result = SharedStoreStats::report_keys();
    transport->sendString(result);
    return true;
  }
  if (cmd == "apc-ss-dump") {
    if (!RuntimeOption::EnableAPCSizeDetail) {
      transport->sendString("Not Enabled\n");
      return true;
    }
    string key_sample = transport->getParam("keysample");
    if (SharedStoreStats::snapshot("/tmp/APC_details", key_sample)) {
      transport->sendString("Done\n");
    } else {
      transport->sendString("Failed\n");
    }
    return true;
  }
  return false;
}

bool AdminRequestHandler::handleConstSizeRequest (const std::string &cmd,
                                                  Transport *transport) {
  if (!RuntimeOption::EnableConstLoad && cmd == "const-ss") {
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
  result << StringData::GetStaticStringCount();
  transport->sendString(result.str());
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
    transport->sendString(Transl::Translator::Get()->getUsage());
    return true;
  }
  if (cmd == "vm-namedentities") {
    std::ostringstream result;
    result << Unit::GetNamedEntityTableSize();
    transport->sendString(result.str());
    return true;
  }
  if (cmd == "vm-dump-tc") {
    if (HPHP::Transl::tc_dump()) {
      transport->sendString("Done");
    } else {
      transport->sendString("Error dumping the translation cache");
    }
    return true;
  }
  if (cmd == "vm-tcreset") {
    int64_t start = Timer::GetCurrentTimeMicros();
    if (Transl::Translator::Get()->replace()) {
      string msg;
      Util::string_printf(msg, "Done %" PRId64 " ms",
                          (Timer::GetCurrentTimeMicros() - start) / 1000);
      transport->sendString(msg);
    } else {
      transport->sendString("Failed");
    }
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Dump cache content

extern bool const_dump(const char *filename);
bool (*file_dump)(const char *filename) = nullptr;

bool AdminRequestHandler::handleDumpCacheRequest(const std::string &cmd,
                                                 Transport *transport) {
  if (cmd == "dump-const") {
    if (!RuntimeOption::EnableApc ||
        !RuntimeOption::EnableConstLoad ||
        RuntimeOption::ApcPrimeLibrary.empty()) {
      transport->sendString("No Constant Cache\n");
      return true;
    }
    const_dump("/tmp/const_map_dump");
    transport->sendString("Done");
    return true;
  }
  if (cmd == "dump-apc") {
    if (!RuntimeOption::EnableApc) {
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
    apc_dump("/tmp/apc_dump", keyOnly, waitSeconds);
    transport->sendString("Done");
    return true;
  }
  if (cmd == "dump-file-repo") {
    if (file_dump) {
      (*file_dump)("/tmp/file_repo_dump");
    }
    transport->sendString("Done");
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
