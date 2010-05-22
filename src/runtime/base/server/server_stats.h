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

#ifndef __HPHP_SERVER_STATS_H__
#define __HPHP_SERVER_STATS_H__

#include <util/lock.h>
#include <util/thread_local.h>
#include <runtime/base/shared/shared_string.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ServerStats {
public:
  enum Format {
    XML,
    JSON,
    KVP,
    HTML,
  };

  enum ThreadMode {
    Idling,
    Processing,
    Writing,
    PostProcessing
  };

public:
  static void Log(const std::string &name, int64 value);
  static int64 Get(const std::string &name);
  static void LogPage(const std::string &url, int code);
  static void Clear();
  static void GetKeys(std::string &out, int64 from, int64 to);
  static void Report(std::string &out, Format format, int64 from, int64 to,
                     const std::string &agg, const std::string &keys,
                     const std::string &url, int code,
                     const std::string &prefix);

  // thread status functions
  static void LogBytes(int64 bytes);
  static void StartRequest(const char *url, const char *clientIP,
                           const char *vhost);
  static void SetThreadMode(ThreadMode mode);
  static void SetThreadIOStatus(const char *status);
  static void ReportStatus(std::string &out, Format format);

public:
  ServerStats();
  ~ServerStats();

private:
  enum UDF {
    UDF_NONE = 1, // count
    UDF_HIT  = 2, // count per hit
    UDF_SEC  = 4, // count per second

    PRECISION = 1000,
  };

  static Mutex s_lock;
  static std::vector<ServerStats*> s_loggers;
  static DECLARE_THREAD_LOCAL(ServerStats, s_logger);

  typedef hphp_shared_string_map<int64> CounterMap;

  struct PageStats {
    std::string m_url; // which page
    int m_code;        // response code
    int m_hit;         // page hits
    CounterMap m_values; // name value pairs
  };
  typedef hphp_shared_string_map<PageStats> PageStatsMap;
  struct TimeSlot {
    int64 m_time;
    PageStatsMap m_pages;
  };

  static void Merge(CounterMap &dest,
                    const CounterMap &src);
  static void Merge(PageStatsMap &dest, const PageStatsMap &src);
  static void Merge(std::list<TimeSlot*> &dest,
                    const std::list<TimeSlot*> &src);
  static void Filter(std::list<TimeSlot*> &slots, const std::string &keys,
                     const std::string &url, int code,
                     std::map<std::string, int> &wantedKeys);
  static void Aggregate(std::list<TimeSlot*> &slots, const std::string &agg,
                        std::map<std::string, int> &wantedKeys);

  static void CollectSlots(std::list<TimeSlot*> &slots, int64 from, int64 to);
  static void FreeSlots(std::list<TimeSlot*> &slots);

  static void GetAllKeys(std::set<std::string> &allKeys,
                         const std::list<TimeSlot*> &slots);
  static void Report(std::string &out, Format format,
                     const std::list<TimeSlot*> &slots,
                     const std::string &prefix);

  Mutex m_lock;
  std::vector<TimeSlot> m_slots;
  int64 m_last; // previous timepoint
  int64 m_min;  // earliest timepoint
  int64 m_max;  // latest timepoint
  CounterMap m_values;  // current page's name value pairs

  void log(const std::string &name, int64 value);
  int64 get(const std::string &name);
  void logPage(const std::string &url, int code);
  void clear();
  void collect(std::list<TimeSlot*> &slots, int64 from, int64 to);

  /**
   * Live status, instead of historical statistics.
   */
  void logBytes(int64 bytes);
  void startRequest(const char *url, const char *clientIP, const char *vhost);
  void setThreadMode(ThreadMode mode);
  void setThreadIOStatus(const char *status);

  class ThreadStatus {
  public:
    ThreadStatus();

    pthread_t m_threadId;

    // total traffic
    int64 m_requestCount;
    int64 m_writeBytes;

    // current request
    time_t m_start;
    time_t m_done;
    ThreadMode m_mode;
    time_t m_iostart;
    char m_iostatus[1024];
    char m_url[1024];
    char m_clientIP[256];
    char m_vhost[256];
  };
  ThreadStatus m_threadStatus;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Taking server stats at different time point of execution.
 */
class ServerStatsHelper {
public:
  ServerStatsHelper(const char *section, bool trackMem = false);
  ~ServerStatsHelper();

private:
  const char *m_section;
  timespec m_wallStart;
  timespec m_cpuStart;
  bool m_trackMemory;

  void logTime(const std::string &prefix, const timespec &start,
               const timespec &end);
};

/**
 * Recording I/O status in a scoped manner.
 */
class IOStatusHelper {
public:
  IOStatusHelper(const char *name, const char *address, int port = 0);
  ~IOStatusHelper();
};

/**
 * For profiling mutexes.
 */
void server_stats_log_mutex(const std::string &stack, int64 elapsed_us);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_SERVER_STATS_H__
