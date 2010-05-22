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

#include <runtime/base/server/admin_request_handler.h>
#include <runtime/base/server/http_server.h>
#include <runtime/base/util/http_client.h>
#include <runtime/base/server/server_stats.h>
#include <runtime/base/runtime_option.h>
#include <util/process.h>
#include <util/logger.h>
#include <util/util.h>
#include <util/mutex.h>
#include <runtime/base/time/datetime.h>
#include <runtime/base/memory/memory_manager.h>
#include <runtime/base/program_functions.h>
#include <runtime/base/shared/shared_store.h>
#include <runtime/base/memory/leak_detectable.h>

#ifdef GOOGLE_CPU_PROFILER
#include <google/profiler.h>
#endif
#ifdef GOOGLE_HEAP_PROFILER
#include <google/heap-profiler.h>
#endif
#ifdef GOOGLE_TCMALLOC
#include <google/malloc_extension.h>
#endif
#ifdef USE_JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

using namespace std;
using namespace boost;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL(AccessLog::ThreadData,
                       AdminRequestHandler::s_accessLog_tl);
AccessLog AdminRequestHandler::s_accessLog(AdminRequestHandler::s_accessLog_tl);

AdminRequestHandler::AdminRequestHandler() {
}

#ifdef USE_JEMALLOC
#define MALLOC_WRITE_CB_BUFLEN 1024*1024
static void
malloc_write_cb(void *cbopaque, const char *s) {
  char *buf = (char *)cbopaque;
  strncat(buf, s, MALLOC_WRITE_CB_BUFLEN - 1);
}
#endif

void AdminRequestHandler::handleRequest(Transport *transport) {
  GetAccessLog().onNewRequest();
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
#ifdef COMPILER_ID
        "/compiler-id:     returns the compiler id that built this app\n"
#endif

        "/check-load:      how many threads are actively handling requests\n"
        "/check-mem:       report memory quick statistics in log file\n"
        "/check-apc:       report APC quick statistics\n"

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
#ifdef GOOGLE_TCMALLOC
        "/free-mem:        ask tcmalloc to release memory to system\n"
        "/tcmalloc-stats:  get internal tcmalloc stats\n"
#endif
#ifdef USE_JEMALLOC
        "/jemalloc-stats:  get internal jemalloc stats\n"
        "/jemalloc-stats-print:\n"
        "                  get comprehensive jemalloc stats in human-readable form\n"
        "/jemalloc-prof-activate:\n"
        "                  activate heap profiling\n"
        "/jemalloc-prof-deactivate:\n"
        "                  deactivate heap profiling\n"
        "/jemalloc-prof-dump:\n"
        "                  dump heap profile\n"
        "    file          optional, filesystem path\n"
#endif
        ;
      transport->sendString(usage);
      break;
    }

    if (!RuntimeOption::AdminPassword.empty() &&
        RuntimeOption::AdminPassword != transport->getParam("auth")) {
      transport->sendString("Unauthorized", 401);
      break;
    }

    if (cmd == "stop") {
      transport->sendString("OK\n");
      HttpServer::Server->stop();
      break;
    }
    if (cmd == "build-id") {
      transport->sendString(RuntimeOption::BuildId, 200);
      break;
    }
#ifdef COMPILER_ID
    if (cmd == "compiler-id") {
      transport->sendString(COMPILER_ID, 200);
      break;
    }
#endif
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
#ifdef GOOGLE_TCMALLOC
    if (cmd == "free-mem") {
      MallocExtension::instance()->ReleaseFreeMemory();
      transport->sendString("OK\n");
      break;
    }
    if (cmd == "tcmalloc-stats") {
      ostringstream stats;
      size_t user_allocated, heap_size, slack_bytes;
      MallocExtension::instance()->
        GetNumericProperty("generic.current_allocated_bytes", &user_allocated);
      MallocExtension::instance()->
        GetNumericProperty("generic.heap_size", &heap_size);
      MallocExtension::instance()->
        GetNumericProperty("tcmalloc.slack_bytes", &slack_bytes);
      stats << "<tcmalloc-stats>" << endl;
      stats << "  <user_allocated>" << user_allocated << "</user_allocated>" <<
        endl;
      stats << "  <heap_size>" << heap_size << "</heap_size>" << endl;
      stats << "  <slack_bytes>" << slack_bytes << "</slack_bytes>" << endl;
      stats << "</tcmalloc-stats>" << endl;
      transport->sendString(stats.str());
      break;
    }
#endif
#ifdef USE_JEMALLOC
    if (cmd == "jemalloc-stats") {
      // Force jemalloc to update stats cached for use by mallctl().
      uint64_t epoch = 1;
      mallctl("epoch", NULL, NULL, &epoch, sizeof(epoch));

      size_t allocated = 0; // Initialize in case jemalloc stats aren't enabled.
      size_t sz = sizeof(size_t);
      mallctl("stats.allocated", &allocated, &sz, NULL, 0);

      size_t active = 0;
      mallctl("stats.active", &active, &sz, NULL, 0);

      size_t mapped = 0;
      mallctl("stats.mapped", &mapped, &sz, NULL, 0);

      ostringstream stats;
      stats << "<jemalloc-stats>" << endl;
      stats << "  <allocated>" << allocated << "</allocated>" << endl;
      stats << "  <active>" << active << "</active>" << endl;
      stats << "  <mapped>" << mapped << "</mapped>" << endl;
      stats << "</jemalloc-stats>" << endl;
      transport->sendString(stats.str());
      break;
    }
    if (cmd == "jemalloc-stats-print") {
      char *buf = (char *)malloc(MALLOC_WRITE_CB_BUFLEN);
      if (buf == NULL) {
        transport->sendString("OOM\n");
        break;
      }

      buf[0] = '\0';
      malloc_stats_print(malloc_write_cb, (void *)buf, "");
      transport->sendString(buf);
      free(buf);
      break;
    }
    if (cmd == "jemalloc-prof-activate") {
      bool active = true;
      int err = mallctl("prof.active", NULL, NULL, &active, sizeof(bool));
      if (err) {
        ostringstream estr;
        estr << "Error " << err << " in mallctl(\"prof.active\", ...)" << endl;
        transport->sendString(estr.str());
      } else {
        transport->sendString("OK\n");
      }
      break;
    }
    if (cmd == "jemalloc-prof-deactivate") {
      bool active = false;
      int err = mallctl("prof.active", NULL, NULL, &active, sizeof(bool));
      if (err) {
        ostringstream estr;
        estr << "Error " << err << " in mallctl(\"prof.active\", ...)" << endl;
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
        int err = mallctl("prof.dump", NULL, NULL, (void *)&s, sizeof(char *));
        if (err) {
          ostringstream estr;
          estr << "Error " << err << " in mallctl(\"prof.dump\", ..., \"" << f
               << "\", ...)" << endl;
          transport->sendString(estr.str());
          break;
        }
      } else {
        int err = mallctl("prof.dump", NULL, NULL, NULL, 0);
        if (err) {
          ostringstream estr;
          estr << "Error " << err << " in mallctl(\"prof.dump\", ...)" << endl;
          transport->sendString(estr.str());
          break;
        }
      }
      transport->sendString("OK\n");
      break;
    }
#endif

    transport->sendString("Unknown command: " + cmd + "\n", 404);
  } while (0);
  GetAccessLog().log(transport);
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
  int64  from   = transport->getInt64Param ("from");
  int64  to     = transport->getInt64Param ("to");
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

bool AdminRequestHandler::handleCheckRequest(const std::string &cmd,
                                             Transport *transport) {
  if (cmd == "check-load") {
    int count = HttpServer::Server->getPageServer()->getActiveWorker();
    transport->sendString(lexical_cast<string>(count));
    return true;
  }
  if (cmd == "check-mem") {
    return toggle_switch(transport, RuntimeOption::CheckMemory);
  }
  if (cmd == "check-apc") {
    string stats = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    stats += "<APC>\n";
    stats += SharedStores::ReportStats(1);
    stats += "</APC>\n";
    transport->sendString(stats);
    return true;
  }
  return false;
}

bool AdminRequestHandler::handleStatusRequest(const std::string &cmd,
                                              Transport *transport) {
  if (cmd == "status.xml") {
    return send_status(transport, ServerStats::XML, "application/xml");
  }
  if (cmd == "status.json") {
    return send_status(transport, ServerStats::JSON, "application/json");
  }
  if (cmd == "status.html" || cmd == "status.htm") {
    return send_status(transport, ServerStats::HTML, "text/html");
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
    int64 from = transport->getInt64Param("from");
    int64 to = transport->getInt64Param("to");
    string out;
    ServerStats::GetKeys(out, from, to);
    transport->sendString(out);
    return true;
  }
  if (cmd == "stats.xml") {
    return send_report(transport, ServerStats::XML, "application/xml");
  }
  if (cmd == "stats.json") {
    return send_report(transport, ServerStats::JSON, "application/json");
  }
  if (cmd == "stats.kvp") {
    return send_report(transport, ServerStats::KVP, "text/plain");
  }
  if (cmd == "stats.html" || cmd == "stats.htm") {
    return send_report(transport, ServerStats::HTML, "text/html");
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

#if (defined(GOOGLE_CPU_PROFILER) || defined(GOOGLE_HEAP_PROFILER))

// call pprof to generate outputs
static void pprof(const std::string &file, const char *extra = NULL) {
  string program = Process::GetAppName();
  const char *formats[] = {"--pdf", "--gif", "--callgrind"};
  const char *argv[] = {"", NULL, program.c_str(), file.c_str(), extra, NULL};
  String now = DateTime::Current()->toString("YmdHis");
  for (unsigned int i = 0; i < sizeof(formats)/sizeof(formats[0]); i++) {
    argv[1] = formats[i];
    string out, err;
    if (Process::Exec("pprof", argv, "", out, &err) && !out.empty()) {
      char filename[PATH_MAX];
      snprintf(filename, sizeof(filename), "%s.%s.%s", file.c_str(),
               now.data(), formats[i] + 2 /* skipping -- */);
      FILE *f = fopen(filename, "w");
      if (f) {
        fwrite(out.c_str(), 1, out.size(), f);
        fclose(f);
        Logger::Info("pprof generated %s", filename);
      }
    }
  }
}

#endif

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
    pprof(file);
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

      const char *argv[] = {"", root.c_str(), "-name", "*.heap", NULL};
      string files;
      Process::Exec("find", argv, NULL, files);
      vector<string> out;
      Util::split('\n', files.c_str(), out, true);
      if (out.size() > 1) {
        string base = "--base=";
        base += out[0];
        pprof(out[out.size() - 1], base.c_str());
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
}
