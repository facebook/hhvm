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

#ifndef incl_HPHP_SERVERSTATS_H_
#define incl_HPHP_SERVERSTATS_H_

#include <set>

#include <curl/curl.h>
#include <time.h>

#include "hphp/util/lock.h"
#include "hphp/util/thread-local.h"
#include "hphp/runtime/base/shared-string.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/execution-profiler.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ServerStats {
public:
  enum class Format {
    XML,
    JSON,
    KVP,
    HTML
  };

  enum class ThreadMode {
    Idling,
    Processing,
    Writing,
    PostProcessing
  };

public:
  static void Log(const std::string &name, int64_t value);
  static int64_t Get(const std::string &name);
  static void LogPage(const std::string &url, int code);
  static void Reset();
  static void Clear();
  static void GetKeys(std::string &out, int64_t from, int64_t to);
  static void Report(std::string &out, Format format, int64_t from, int64_t to,
                     const std::string &agg, const std::string &keys,
                     const std::string &url, int code,
                     const std::string &prefix);

  // thread status functions
  static void LogBytes(int64_t bytes);
  static void StartRequest(const char *url, const char *clientIP,
                           const char *vhost);
  static void SetThreadMode(ThreadMode mode);
  static void ReportStatus(std::string &out, Format format);

  // io status functions
  static void SetThreadIOStatusAddress(const char *name);
  static void SetThreadIOStatus(const char *name, const char *addr,
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

  static Mutex s_lock;
  static std::vector<ServerStats*> s_loggers;
  static DECLARE_THREAD_LOCAL_NO_CHECK(ServerStats, s_logger);

  typedef hphp_shared_string_map<int64_t> CounterMap;

  struct PageStats {
    std::string m_url; // which page
    int m_code;        // response code
    int m_hit;         // page hits
    CounterMap m_values; // name value pairs
  };
  typedef hphp_shared_string_map<PageStats> PageStatsMap;
  struct TimeSlot {
    int64_t m_time;
    PageStatsMap m_pages;
  };

  static void Merge(CounterMap &dest, const CounterMap &src);
  static void Merge(PageStatsMap &dest, const PageStatsMap &src);
  static void Merge(std::list<TimeSlot*> &dest,
                    const std::list<TimeSlot*> &src);
  static void Filter(std::list<TimeSlot*> &slots, const std::string &keys,
                     const std::string &url, int code,
                     std::map<std::string, int> &wantedKeys);
  static void Aggregate(std::list<TimeSlot*> &slots, const std::string &agg,
                        std::map<std::string, int> &wantedKeys);

  static void CollectSlots(std::list<TimeSlot*> &slots, int64_t from, int64_t to);
  static void FreeSlots(std::list<TimeSlot*> &slots);

  static void GetAllKeys(std::set<std::string> &allKeys,
                         const std::list<TimeSlot*> &slots);
  static void Report(std::string &out, Format format,
                     const std::list<TimeSlot*> &slots,
                     const std::string &prefix);

  Mutex m_lock;
  std::vector<TimeSlot> m_slots;
  int64_t m_last; // previous timepoint
  int64_t m_min;  // earliest timepoint
  int64_t m_max;  // latest timepoint
  CounterMap m_values;  // current page's name value pairs

  void log(const std::string &name, int64_t value);
  int64_t get(const std::string &name);
  void logPage(const std::string &url, int code);
  void reset();
  void clear();
  void collect(std::list<TimeSlot*> &slots, int64_t from, int64_t to);

  /**
   * Live status, instead of historical statistics.
   */
  void logBytes(int64_t bytes);
  void startRequest(const char *url, const char *clientIP, const char *vhost);
  void setThreadMode(ThreadMode mode);

  void setThreadIOStatusAddress(const char *name);
  void setThreadIOStatus(const char *name, const char *addr,
                         int64_t usWallTime = -1);
  Array getThreadIOStatuses();

  class IOStatus {
  public:
    IOStatus() : count(0), wall_time(0) {}

    int64_t count;
    int64_t wall_time; // micro-seconds
  };
  // keys: "url==>name" and "name==>address"
  typedef hphp_string_map<IOStatus> IOStatusMap;

  class ThreadStatus {
  public:
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

    char m_ioName[512];
    char m_ioLogicalName[512];
    char m_ioAddr[512];
    char m_url[1024];
    char m_clientIP[256];
    char m_vhost[256];

    IOStatusMap m_ioStatuses;
  };
  ThreadStatus m_threadStatus;
  IOStatusMap m_ioProfiles;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Taking server stats at different time point of execution.
 */
class ServerStatsHelper {
public:
  enum {
    TRACK_MEMORY = 0x00000001,
    TRACK_HWINST = 0x00000002,
  };
  explicit ServerStatsHelper(const char *section, uint32_t track = 0);
  ~ServerStatsHelper();

private:
  const char *m_section;
  timespec m_wallStart;
  timespec m_cpuStart;
  int64_t m_instStart;
  uint32_t m_track;

  void logTime(const std::string &prefix, const timespec &start,
               const timespec &end);
  void logTime(const std::string &prefix, const int64_t &start,
               const int64_t &end);
};

/**
 * Recording I/O status in a scoped manner.
 */
class IOStatusHelper {
public:
  explicit IOStatusHelper(const char *name, const char *address = nullptr,
                          int port = 0);
  ~IOStatusHelper();

private:
  ExecutionProfiler m_exeProfiler;
};

/**
 * For profiling CURL calls.
 */
void set_curl_statuses(CURL *cp, const char *url);

/**
 * For profiling mutexes.
 */
void server_stats_log_mutex(const std::string &stack, int64_t elapsed_us);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_SERVERSTATS_H_
