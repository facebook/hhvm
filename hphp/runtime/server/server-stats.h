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

#pragma once

#include "hphp/runtime/base/execution-profiler.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/server/writer.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/lock.h"
#include "hphp/util/thread-local.h"

#include <curl/curl.h>
#include <map>
#include <time.h>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Intermediate types for reporting
 */
struct ServerStatusProcessReport {
  int64_t id;
  std::string build;
  std::string compiler;
  bool debug;
  int64_t now;
  int64_t start;
  int64_t up;
};

struct ServerStatusRequestMemoryReport {
  int64_t currentUsage;
  int64_t currentAlloc;
  int64_t peakUsage;
  int64_t peakAlloc;
  int64_t limit;
  int64_t currentMmUsage;
};

struct ServerStatusRequestIOReport {
  std::string status;
  int64_t duration;
};

struct ServerStatusRequestReport {
  std::string vhost;
  std::string url;
  std::string endpoint;
  std::string client;
  int64_t start;
  int64_t duration_ms;
  ServerStatusRequestMemoryReport memory;
  HPHP::Optional<ServerStatusRequestIOReport> io;
};

struct ServerStatusThreadReport {
  int64_t id;
  int64_t tid;
  int64_t req;
  int64_t bytes;
  std::string mode;
  HPHP::Optional<ServerStatusRequestReport> request;
};

struct ServerStatusReport {
  ServerStatusProcessReport process;
  std::vector<ServerStatusThreadReport> threads;
};

struct ServerStats {

  enum class ThreadMode {
    Idling = 0,
    Processing,
    Writing,
    PostProcessing
  };

public:
  static void Log(const std::string& name, int64_t value);
  static int64_t Get(const std::string& name);
  static void LogPage(const std::string& url, int code);
  static void Reset();
  static void Clear();
  static std::string GetKeys();
  static std::string ReportString(const std::string& keys,
                                  const std::string& prefix);
  static hphp_fast_string_map<int64_t> Report(
    const std::vector<std::string>& keys, const std::string& prefix);

  // thread status functions
  static void LogBytes(int64_t bytes);
  static void StartRequest(const char* url, const char* clientIP,
                           const char* vhost);
  static void SetEndpoint(const char* endpoint);
  static void SetThreadMode(ThreadMode mode);
  static ThreadMode GetThreadMode();
  static const char* ThreadModeString(ThreadMode mode);
  static ServerStatusReport ReportStatus();
  static std::string ReportStatus(Writer::Format format);

  // io status functions
  static void SetThreadIOStatus(const char* name, const char* addr,
                                int64_t usWallTime = -1);
  static Array GetThreadIOStatuses();
  static void StartNetworkProfile();
  static Array EndNetworkProfile();

  static bool s_profile_network;

public:
  ServerStats();
  ~ServerStats();

  static void GetLogger();
private:
  enum UDF {
    UDF_NONE = 1, // count
    UDF_HIT  = 2, // count per hit
    UDF_SEC  = 4, // count per second

    PRECISION = 1000
  };
  using KeyMap = hphp_fast_string_map<int>;
  using CounterMap = hphp_fast_string_map<int64_t>;

  static Mutex s_lock;
  static std::vector<ServerStats*> s_loggers;
  static THREAD_LOCAL_NO_CHECK(ServerStats, s_logger);

  struct TimeSlot {
    uint32_t m_time{0};
    uint32_t m_hits{0};
    CounterMap m_values;
    void clear() {
      m_time = 0;
      m_hits = 0;
      m_values.clear();
    }
    bool empty() const {
      return m_hits == 0;
    }
  };

  static uint32_t curr() {
    auto const now = static_cast<uint64_t>(time(nullptr));
    return now / RuntimeOption::StatsSlotDuration;
  }

  static void Merge(CounterMap& dest, const CounterMap& src,
                    const KeyMap& wanted);
  static void Merge(TimeSlot& dest, const TimeSlot& src,
                    const KeyMap& wanted);
  static KeyMap CompileKeys(const std::vector<std::string>& rules);
  static CounterMap ReportImpl(const KeyMap& keys);

  template<typename F> static void VisitAllSlots(F fun) {
    Lock lock(s_lock, false);
    for (auto& l : s_loggers) {
      Lock _(l->m_lock, false);
      for (auto& slot : l->m_slots) {
        fun(slot);
      }
    }
  }

  Mutex m_lock;
  std::vector<TimeSlot> m_slots;
  CounterMap m_values;                  // current page's name value pairs

  void log(const std::string& name, int64_t value);
  int64_t get(const std::string& name);
  void logPage(const std::string& url, int code);
  void reset();

  /**
   * Live status, instead of historical statistics.
   */
  void logBytes(int64_t bytes);
  void startRequest(const char* url, const char* clientIP, const char* vhost);
  void setEndpoint(const char* endpoint);
  void setThreadMode(ThreadMode mode) { m_threadStatus.m_mode = mode; }

  void setThreadIOStatus(const char* name, const char* addr,
                         int64_t usWallTime = -1);
  Array getThreadIOStatuses();

  struct IOStatus {
    int64_t count{0};
    int64_t wall_time{0}; // micro-seconds
  };
  // keys: "url==>name" and "name==>address"
  using IOStatusMap = hphp_fast_string_map<IOStatus>;

  struct ThreadStatus {
    ThreadStatus();

    pthread_t m_threadId;
    pid_t m_threadPid;
    MemoryManager* m_mm;

    // total traffic
    int64_t m_requestCount;
    int64_t m_writeBytes;

    // current request
    timeval m_start;
    timeval m_done;
    ThreadMode m_mode;

    // Whether or not an io is in process.
    bool m_ioInProcess;

    // If an io is in process, the time that it started.
    timespec m_ioStart;

    char m_ioName[64];
    char m_ioAddr[64];
    char m_clientIP[64];
    char m_vhost[64];
    char m_url[512];
    char m_endpoint[512];

    IOStatusMap m_ioStatuses;
  };
  ThreadStatus m_threadStatus;
  IOStatusMap m_ioProfiles;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Taking server stats at different time point of execution.
 */
struct ServerStatsHelper {
  enum {
    TRACK_MEMORY = 0x00000001,
    TRACK_HWINST = 0x00000002,
  };
  explicit ServerStatsHelper(const char* section, uint32_t track = 0);
  ~ServerStatsHelper();

private:
  const char* m_section;
  timespec m_wallStart;
  timespec m_cpuStart;
  int64_t m_instStart;
  uint32_t m_track;

  void logTime(const std::string& prefix, const timespec& start,
               const timespec& end);
  void logTime(const std::string& prefix, const int64_t& start,
               const int64_t& end);
};

/**
 * Recording I/O status in a scoped manner.
 */
struct IOStatusHelper {
  explicit IOStatusHelper(const char* name, const char* address = nullptr,
                          int port = 0);
  ~IOStatusHelper();

private:
  ExecutionProfiler m_exeProfiler;
};

/**
 * For profiling CURL calls.
 */
void set_curl_statuses(CURL *cp, const char* url);

/**
 * For profiling mutexes.
 */
void server_stats_log_mutex(const std::string& stack, int64_t elapsed_us);

///////////////////////////////////////////////////////////////////////////////
}
