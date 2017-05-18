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

#include "hphp/runtime/server/server-stats.h"

#include <set>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <iostream>

#include <folly/Conv.h>
#include <folly/FBVector.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <folly/json.h>
#include <folly/portability/Unistd.h>

#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/util/build-info.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/hardware-counter.h"
#include "hphp/util/process.h"
#include "hphp/util/timer.h"
#include "hphp/util/text-util.h"
#include "hphp/runtime/server/writer.h"

namespace HPHP {
//////////////////////////////////////////////////////////////////////

using std::list;
using std::set;
using std::map;
using std::ostream;
using std::string;

///////////////////////////////////////////////////////////////////////////////
// helpers

void ServerStats::GetLogger() {
  s_logger.getCheck();
}

void ServerStats::Merge(CounterMap& dest, const CounterMap& src) {
  for (auto const& iter : src) {
    dest[iter.first] += iter.second;
  }
}

void ServerStats::Merge(list<TimeSlot*>& dest, const list<TimeSlot*>& src) {
  auto diter = dest.begin();
  for (auto const& s : src) {
    for (; diter != dest.end(); ++diter) {
      auto d = *diter;
      if (d->m_time > s->m_time) {
        TimeSlot *c = new TimeSlot();
        *c = *s;
        dest.insert(diter, c);
        break;
      }
      if (d->m_time == s->m_time) {
        d->m_hits += s->m_hits;
        Merge(d->m_values, s->m_values);
        break;
      }
    }

    if (diter == dest.end()) {
      TimeSlot *c = new TimeSlot();
      *c = *s;
      dest.insert(diter, c);
      diter = dest.end();
    }
  }
}

void ServerStats::GetAllKeys(std::set<std::string>& allKeys,
                             const std::list<TimeSlot*>& slots) {
  for (auto& slot : slots) {
    for (auto const& kvpair : slot->m_values) {
      allKeys.insert(kvpair.first);
    }
  }

  // special keys
  allKeys.insert("hit");
  allKeys.insert("load");
  allKeys.insert("idle");
  allKeys.insert("queued");
  allKeys.insert("health_level");
}

void ServerStats::Filter(list<TimeSlot*>& slots, const std::string& keys,
                         std::map<std::string, int>& wantedKeys) {
  // no keys to filter
  if (keys.empty()) {
    return;
  }

  folly::fbvector<std::string> rules0;
  split(',', keys.c_str(), rules0, true);
  if (!rules0.empty()) {

    // prepare rules
    std::map<std::string, int> rules;
    for (unsigned int i = 0; i < rules0.size(); i++) {
      auto const& rule = rules0[i];
      assert(!rule.empty());
      int len = rule.length();
      std::string suffix;
      if (len > 4) {
        len -= 4;
        suffix = rule.substr(len);
      }
      if (suffix == "/hit") {
        rules[rule.substr(0, len)] |= UDF_HIT;
      } else if (suffix == "/sec") {
        rules[rule.substr(0, len)] |= UDF_SEC;
      } else {
        rules[rule] |= UDF_NONE;
      }
    }

    // prepare all keys
    std::set<std::string> allKeys;
    GetAllKeys(allKeys, slots);

    // prepare wantedKeys
    for (auto const& key : allKeys) {
      for (auto const& riter : rules) {
        const string& rule = riter.first;
        if (rule[0] == ':') {
          Variant ret = preg_match(String(rule.c_str(), rule.size(),
                CopyString),
              String(key.c_str(), key.size(), CopyString));
          if (!same(ret, false) && more(ret, 0)) {
            wantedKeys[key] |= riter.second;
          }
        } else if (rule == key) {
          wantedKeys[key] |= riter.second;
        }
      }
    }
  }

  for (auto const& s : slots) {
    auto& values = s->m_values;
    for (auto viter = values.begin(); viter != values.end();) {
      if (wantedKeys.find(viter->first) == wantedKeys.end()) {
        auto iterTemp = viter;
        ++viter;
        values.erase(iterTemp);
      } else {
        ++viter;
      }
    }
  }
}

void ServerStats::Aggregate(list<TimeSlot*>& slots,
                            std::map<std::string, int>& wantedKeys) {
  int slotCount = slots.size();

  auto const ts = new TimeSlot();
  ts->m_time = 0;
  for (auto const& s : slots) {
    ts->m_hits += s->m_hits;
    Merge(ts->m_values, s->m_values);
  }
  FreeSlots(slots);
  slots.push_back(ts);

  std::map<std::string, int> udfKeys;
  for (auto const& iter : wantedKeys) {
    if (iter.second != UDF_NONE) {
      udfKeys[iter.first] = iter.second;
    }
  }

  // Hack: These two are not really page specific.
  int load = HttpServer::Server->getPageServer()->getActiveWorker();
  int idle = RuntimeOption::ServerThreadCount - load;
  int queued = HttpServer::Server->getPageServer()->getQueuedJobs();
  int health_level = (int)ServerStats::m_ServerHealthLevel;

  for (auto const& s : slots) {
    int sec = (s->m_time == 0 ? slotCount : 1) *
      RuntimeOption::StatsSlotDuration;
    auto& values = s->m_values;

    // special keys
    if (wantedKeys.find("hit") != wantedKeys.end()) {
      values["hit"] = s->m_hits;
    }
    if (wantedKeys.find("load") != wantedKeys.end()) {
      values["load"] = load;
    }
    if (wantedKeys.find("idle") != wantedKeys.end()) {
      values["idle"] = idle;
    }
    if (wantedKeys.find("queued") != wantedKeys.end()) {
      values["queued"] = queued;
    }

    if (wantedKeys.find("health_level") != wantedKeys.end()) {
      values["health_level"] = health_level;
    }

    for (auto const& iter : udfKeys) {
      const string& key = iter.first;
      int udf = iter.second;
      auto viter = values.find(key);
      if (viter != values.end()) {
        if ((udf & UDF_HIT) && s->m_hits) {
          values[key + "/hit"] = viter->second * PRECISION / s->m_hits;
        }
        if ((udf & UDF_SEC) && sec) {
          values[key + "/sec"] = viter->second * PRECISION / sec;
        }
        if ((wantedKeys[key] & UDF_NONE) == 0) {
          values.erase(viter);
        }
      }
    }
  }
}

void ServerStats::FreeSlots(list<TimeSlot*>& slots) {
  for (auto const& slot : slots) {
    delete slot;
  }
  slots.clear();
}

///////////////////////////////////////////////////////////////////////////////
// static

Mutex ServerStats::s_lock;
std::vector<ServerStats*> ServerStats::s_loggers;
bool ServerStats::s_profile_network = false;
HealthLevel ServerStats::m_ServerHealthLevel = HealthLevel::Bold;
IMPLEMENT_THREAD_LOCAL_NO_CHECK(ServerStats, ServerStats::s_logger);

void ServerStats::LogPage(const string& url, int code) {
  if (RuntimeOption::EnableStats && RuntimeOption::EnableWebStats) {
    ServerStats::s_logger->logPage(url, code);
  }
}

void ServerStats::Log(const string& name, int64_t value) {
  if (RuntimeOption::EnableStats && RuntimeOption::EnableWebStats) {
    ServerStats::s_logger->log(name, value);
  }
}

void ServerStats::LogBytes(int64_t bytes) {
  if (RuntimeOption::EnableStats && RuntimeOption::EnableWebStats) {
    ServerStats::s_logger->logBytes(bytes);
  }
}

void ServerStats::StartRequest(const char *url, const char *clientIP,
                               const char *vhost) {
  if (RuntimeOption::EnableStats && RuntimeOption::EnableWebStats) {
    ServerStats::s_logger->startRequest(url, clientIP, vhost);
  }
}

void ServerStats::SetServerHealthLevel(HealthLevel new_health_level) {
  ServerStats::m_ServerHealthLevel = new_health_level;
}

void ServerStats::SetThreadMode(ThreadMode mode) {
  ServerStats::s_logger->setThreadMode(mode);
}

ServerStats::ThreadMode ServerStats::GetThreadMode() {
  return ServerStats::s_logger->m_threadStatus.m_mode;
}

const char* ServerStats::ThreadModeString(ThreadMode mode) {
  switch (mode) {
    case ThreadMode::Idling:
      return "Idling";
    case ThreadMode::Processing:
      return "Processing";
    case ThreadMode::Writing:
      return "Writing";
    case ThreadMode::PostProcessing:
      return "PostProcessing";
    default:
      return "Unknown";
  }
}

void ServerStats::SetThreadIOStatusAddress(const char *name) {
  ServerStats::s_logger->setThreadIOStatusAddress(name);
}

void ServerStats::SetThreadIOStatus(const char *name, const char *addr,
                                    int64_t usWallTime /* = -1 */) {
  ServerStats::s_logger->setThreadIOStatus(name, addr, usWallTime);
}

Array ServerStats::GetThreadIOStatuses() {
  return ServerStats::s_logger->getThreadIOStatuses();
}

int64_t ServerStats::Get(const string& name) {
  return ServerStats::s_logger->get(name);
}

void ServerStats::Reset() {
  ServerStats::s_logger->reset();
}

void ServerStats::Clear() {
  Lock lock(s_lock, false);
  for (unsigned int i = 0; i < s_loggers.size(); i++) {
    s_loggers[i]->clear();
  }
}

void ServerStats::CollectSlots(list<TimeSlot*>& slots) {
  Lock lock(s_lock, false);
  for (unsigned int i = 0; i < s_loggers.size(); i++) {
    s_loggers[i]->collect(slots);
  }
}

void ServerStats::GetKeys(string& out) {
  list<TimeSlot*> slots;
  CollectSlots(slots);
  set<string> allKeys;
  GetAllKeys(allKeys, slots);
  for (auto const& iter : allKeys) {
    out += iter;
    out += "\n";
  }
}

void ServerStats::Report(string& out,
                         const std::string& keys,
                         const std::string& prefix) {
  list<TimeSlot*> slots;
  CollectSlots(slots);
  map<string, int> wantedKeys;
  Filter(slots, keys, wantedKeys);
  Aggregate(slots, wantedKeys);
  Report(out, slots, prefix);
  FreeSlots(slots);
}

void ServerStats::Report(string& output,
                         const list<TimeSlot*>& slots,
                         const std::string& prefix) {
  std::ostringstream out;
  bool first = true;
  for (auto const& s : slots) {
    if (first) {
      first = false;
    } else {
      out << ",\n";
    }
    out << "{";
    string key = prefix;
    if (!key.empty()) {
      key += ".";
    }
    bool firstKey = true;
    for (auto const& kvpair : s->m_values) {
      if (firstKey) {
        firstKey = false;
      } else {
        out << ", ";
      }
      out << Writer::escape_for_json(
              (key + kvpair.first).c_str())
          << ": " << kvpair.second;
    }
    out << "}\n";
  }

  output = out.str();
}

static std::string format_duration(timeval& duration) {
  string ret;
  if (duration.tv_sec > 0 || duration.tv_usec > 0) {
    int milliseconds = duration.tv_usec / 1000;
    double seconds = duration.tv_sec % 60 + milliseconds * .001;
    int minutes = duration.tv_sec / 60;
    int hours = minutes / 60;
    minutes = minutes % 60;
    if (hours) {
      ret += folly::to<string>(hours) + " hour";
      ret += (hours == 1) ? " " : "s ";
    }
    if (minutes || (hours && seconds)) {
      ret += folly::to<string>(minutes) + " minute";
      ret += (minutes == 1) ? " " : "s ";
    }
    if (seconds || minutes || hours) {
      ret += folly::stringPrintf("%.3f", seconds);
      ret += (seconds == 1) ? "" : "s";
    }
  } else {
   ret = "0 seconds";
  }
  return ret;
}

void ServerStats::ReportStatus(std::string& output, Writer::Format format) {
  std::ostringstream out;
  Writer *w;
  if (format == Writer::Format::XML) {
    w = new XMLWriter(out);
  } else if (format == Writer::Format::HTML) {
    w = new HTMLWriter(out);
  } else {
    assert(format == Writer::Format::JSON);
    w = new JSONWriter(out);
  }

  time_t now = time(0);

  w->writeFileHeader();
  w->beginObject("status");

  w->beginObject("process");
  w->writeEntry("id", (int64_t)getpid());
  w->writeEntry("build", RuntimeOption::BuildId);

  w->writeEntry("compiler", compilerId().begin());

#ifdef DEBUG
  w->writeEntry("debug", "yes");
#else
  w->writeEntry("debug", "no");
#endif

  w->writeEntry("hotprofiler", "yes");

  timeval up;
  up.tv_sec = now - HttpServer::StartTime;
  up.tv_usec = 0;
  w->writeEntry("now", req::make<DateTime>(now)->
                       toString(DateTime::DateFormatCookie).data());
  w->writeEntry("start", req::make<DateTime>(HttpServer::StartTime)->
                         toString(DateTime::DateFormatCookie).data());
  w->writeEntry("up", format_duration(up));
  w->endObject("process");

  w->beginList("threads");
  Lock lock(s_lock, false);
  for (unsigned int i = 0; i < s_loggers.size(); i++) {
    ThreadStatus& ts = s_loggers[i]->m_threadStatus;

    timeval duration;
    if (ts.m_start.tv_sec > 0 && ts.m_done.tv_sec > 0) {
      timersub(&ts.m_done, &ts.m_start, &duration);
    } else if (ts.m_start.tv_sec > 0) {
      timeval current;
      gettimeofday(&current, 0);
      timersub(&current, &ts.m_start, &duration);
    } else {
      memset(&duration, 0, sizeof(duration));
    }

    const char *mode = "(unknown)";
    switch (ts.m_mode) {
    case ThreadMode::Idling:         mode = "idle";    break;
    case ThreadMode::Processing:     mode = "process"; break;
    case ThreadMode::Writing:        mode = "writing"; break;
    case ThreadMode::PostProcessing: mode = "psp";     break;
    default: assert(false);
    }

    w->beginObject("thread");
    w->writeEntry("id", (int64_t)ts.m_threadId);
    w->writeEntry("tid", (int64_t)ts.m_threadPid);
    w->writeEntry("req", ts.m_requestCount);
    w->writeEntry("bytes", ts.m_writeBytes);
    w->writeEntry("start", req::make<DateTime>(ts.m_start.tv_sec)->
                           toString(DateTime::DateFormatCookie).data());
    w->writeEntry("duration", format_duration(duration));
    if (ts.m_requestCount > 0) {
      auto const stats = ts.m_mm->getStatsCopy();
      w->beginObject("memory");
      w->writeEntry("current usage", stats.usage());
      w->writeEntry("current alloc", stats.capacity);
      w->writeEntry("peak usage", stats.peakUsage);
      w->writeEntry("peak alloc", stats.peakCap);
      w->endObject("memory");
    }
    w->writeEntry("io", ts.m_ioInProcess);

    // Only in the event that we are currently in the process of an io, will
    // we output the iostatus, and ioInProcessDuationMicros
    if (ts.m_ioInProcess) {
      timespec now;
      Timer::GetMonotonicTime(now);
      w->writeEntry("iostatus", string(ts.m_ioName) + " " + ts.m_ioAddr);
      w->writeEntry("ioduration", gettime_diff_us(ts.m_ioStart, now));
    }
    w->writeEntry("mode", mode);
    w->writeEntry("url", ts.m_url);
    w->writeEntry("client", ts.m_clientIP);
    w->writeEntry("vhost", ts.m_vhost);
    w->endObject("thread");
  }
  w->endList("threads");
  w->endObject("status");
  w->writeFileFooter();

  delete w;
  output = out.str();
}

void ServerStats::StartNetworkProfile() {
  s_profile_network = true;

  // It is necessary to clear leftovers, as EndNetworkProfile() can race with
  // threads writing their status.
  Lock lock(s_lock, false);
  for (unsigned int i = 0; i < s_loggers.size(); i++) {
    ServerStats *ss = s_loggers[i];
    Lock loggerLock(ss->m_lock, false);
    ss->m_ioProfiles.clear();
  }
}

const StaticString
  s_ct("ct"),
  s_wt("wt");

Array ServerStats::EndNetworkProfile() {
  s_profile_network = false;
  Lock lock(s_lock, false);

  Array ret;
  for (unsigned int i = 0; i < s_loggers.size(); i++) {
    ServerStats *ss = s_loggers[i];
    Lock loggerLock(ss->m_lock, false);

    IOStatusMap& status = ss->m_ioProfiles;
    for (auto const& iter : status) {
      ret.set(String(iter.first),
              make_map_array(s_ct, iter.second.count,
                             s_wt, iter.second.wall_time));
    }
    status.clear();
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

ServerStats::ThreadStatus::ThreadStatus()
    : m_requestCount(0), m_writeBytes(0), m_mode(ThreadMode::Idling),
      m_ioInProcess(false) {
  m_threadId = Process::GetThreadId();
  m_threadPid = Process::GetThreadPid();
  memset(&m_start, 0, sizeof(m_start));
  memset(&m_done, 0, sizeof(m_done));
  memset(m_ioName, 0, sizeof(m_ioName));
  memset(m_ioLogicalName, 0, sizeof(m_ioLogicalName));
  memset(m_ioAddr, 0, sizeof(m_ioAddr));
  memset(m_url, 0, sizeof(m_url));
  memset(m_clientIP, 0, sizeof(m_clientIP));
  memset(m_vhost, 0, sizeof(m_vhost));
}

ServerStats::ServerStats() : m_last(0), m_min(0), m_max(0) {
  m_slots.resize(RuntimeOption::StatsMaxSlot);
  clear();

  Lock lock(s_lock, false);
  s_loggers.push_back(this);
}

ServerStats::~ServerStats() {
  clear();

  // Remove this from the s_loggers vector
  Lock lock(s_lock, false);
  int pos = -1;
  // Scan the vector looking for this instance of ServerStats. Scanning
  // the vector is not terribly efficient, but this doesn't happen often
  // when the server is in a steady state
  for (unsigned i = 0; i < s_loggers.size(); ++i) {
    if (s_loggers[i] == this) {
      pos = i;
      break;
    }
  }
  if (pos >= 0) {
    s_loggers[pos] = s_loggers.back();
    s_loggers.pop_back();
  }
}

void ServerStats::log(const string& name, int64_t value) {
  m_values[name] += value;
}

int64_t ServerStats::get(const std::string& name) {
  CounterMap::const_iterator iter = m_values.find(name);
  if (iter != m_values.end()) {
    return iter->second;
  }
  return 0;
}

void ServerStats::logPage(const string& url, int code) {
  int64_t now = time(nullptr) / RuntimeOption::StatsSlotDuration;
  int slot = now % RuntimeOption::StatsMaxSlot;

  {
    Lock lock(m_lock, false);
    int count = 0;
    for (int64_t t = m_last + 1; t < now; t++) {
      m_slots[t % RuntimeOption::StatsMaxSlot].m_time = 0;
      if (++count > RuntimeOption::StatsMaxSlot) {
        break; // we have cleared all slots, good enough
      }
    }
    auto& ts = m_slots[slot];
    if (ts.m_time != now) {
      if (ts.m_time && m_min <= ts.m_time) {
        m_min = ts.m_time + 1;
      }
      ts.m_time = now;
      ts.m_hits = 0;
      ts.m_values.clear();
    }
    ts.m_hits++;
    Merge(ts.m_values, m_values);
  }

  m_last = now;
  if (m_min == 0) {
    m_min = now;
  }
  if (m_max < now) {
    m_max = now;
  }

  m_threadStatus.m_mode = ThreadMode::Idling;
  gettimeofday(&m_threadStatus.m_done, 0);
}

void ServerStats::reset() {
  m_values.clear();
}

void ServerStats::clear() {
  Lock lock(m_lock, false);
  for (unsigned int i = 0; i < m_slots.size(); i++) {
    m_slots[i].m_time = 0;
  }
}

void ServerStats::collect(std::list<TimeSlot*>& slots) {
  Lock lock(m_lock, false);
  list<TimeSlot*> collected;
  for (int64_t t = m_min; t <= m_max; t++) {
    int slot = t % RuntimeOption::StatsMaxSlot;
    if (m_slots[slot].m_time == t) {
      collected.push_back(&m_slots[slot]);
    }
  }
  Merge(slots, collected);
}

void ServerStats::logBytes(int64_t bytes) {
  m_threadStatus.m_writeBytes += bytes;
}

static void safe_copy(char *dest, const char *src, int max) {
  int len = strlen(src) + 1;
  dest[--max] = '\0';
  memcpy(dest, src, len > max ? max : len);
}

void ServerStats::startRequest(const char *url, const char *clientIP,
                               const char *vhost) {
  ++m_threadStatus.m_requestCount;

  m_threadStatus.m_mm = &MM();
  gettimeofday(&m_threadStatus.m_start, 0);
  memset(&m_threadStatus.m_done, 0, sizeof(m_threadStatus.m_done));
  m_threadStatus.m_mode = ThreadMode::Processing;
  m_threadStatus.m_ioStatuses.clear();

  *m_threadStatus.m_ioLogicalName = 0;
  safe_copy(m_threadStatus.m_url, url, sizeof(m_threadStatus.m_url));
  safe_copy(m_threadStatus.m_clientIP, clientIP,
            sizeof(m_threadStatus.m_clientIP));
  safe_copy(m_threadStatus.m_vhost, vhost, sizeof(m_threadStatus.m_vhost));
}

void ServerStats::setThreadMode(ThreadMode mode) {
  m_threadStatus.m_mode = mode;
}

void ServerStats::setThreadIOStatusAddress(const char *name) {
  if (name) {
    safe_copy(m_threadStatus.m_ioLogicalName, name,
              sizeof(m_threadStatus.m_ioLogicalName));
  }
}

void ServerStats::setThreadIOStatus(const char *name, const char *addr,
                                    int64_t usWallTime /* = -1 */) {
  bool starting = ((name && *name) || (addr && *addr));

  if (starting) {
    if (name) {
      safe_copy(m_threadStatus.m_ioName, name,
                sizeof(m_threadStatus.m_ioName));
    }
    if (addr) {
      safe_copy(m_threadStatus.m_ioAddr, addr,
                sizeof(m_threadStatus.m_ioAddr));
    }

    // Mark the current thread as being in the process of completing
    // an io, and record the time that the io started.
    m_threadStatus.m_ioInProcess = true;
    if (usWallTime < 0) {
      Timer::GetMonotonicTime(m_threadStatus.m_ioStart);
    }
  }

  if (!starting || usWallTime >= 0) {
    m_threadStatus.m_ioInProcess = false;

    if (RuntimeOption::EnableNetworkIOStatus || s_profile_network) {
      int64_t wt = usWallTime;
      if (wt < 0) {
        timespec now;
        Timer::GetMonotonicTime(now);
        wt = gettime_diff_us(m_threadStatus.m_ioStart, now);
      }

      const char *ioName = m_threadStatus.m_ioName;
      const char *ioAddr = m_threadStatus.m_ioLogicalName;
      if (!*ioAddr) ioAddr = m_threadStatus.m_ioAddr;

      if (RuntimeOption::EnableNetworkIOStatus) {
        string key = ioName;
        if (*ioAddr) {
          key += ' '; key += ioAddr;
        }
        IOStatus& io = m_threadStatus.m_ioStatuses[key];
        ++io.count;
        io.wall_time += wt;
      }

      if (s_profile_network) {
        const char *key0 = "main()";
        const char *key1 = m_threadStatus.m_url;
        string key2 = m_threadStatus.m_url; key2 += "==>"; key2 += ioName;
        const char *key3 = ioName;
        string key4 = ioName;
        if (*ioAddr) {
          key4 += "==>"; key4 += ioAddr;
        }

        Lock lock(m_lock, false);
        { IOStatus& io = m_ioProfiles[key0]; ++io.count; io.wall_time += wt;}
        { IOStatus& io = m_ioProfiles[key1]; ++io.count; io.wall_time += wt;}
        { IOStatus& io = m_ioProfiles[key2]; ++io.count; io.wall_time += wt;}
        { IOStatus& io = m_ioProfiles[key3]; ++io.count; io.wall_time += wt;}
        if (*ioAddr) {
          IOStatus& io = m_ioProfiles[key4]; ++io.count; io.wall_time += wt;
        }
      }

      *m_threadStatus.m_ioLogicalName = 0;
    }
  }
}

Array ServerStats::getThreadIOStatuses() {
  IOStatusMap& status = m_threadStatus.m_ioStatuses;
  ArrayInit ret(status.size(), ArrayInit::Map{});
  for (auto const& iter : status) {
    ret.set(String(iter.first),
            make_map_array(s_ct, iter.second.count,
                           s_wt, iter.second.wall_time));
  }
  status.clear();
  return ret.toArray();
}

///////////////////////////////////////////////////////////////////////////////

ServerStatsHelper::ServerStatsHelper(const char *section,
                                     uint32_t track /* = false */)
  : m_section(section), m_instStart(0), m_track(track) {
  if (RuntimeOption::EnableStats && RuntimeOption::EnableWebStats) {
    Timer::GetMonotonicTime(m_wallStart);
#ifdef CLOCK_THREAD_CPUTIME_ID
    gettime(CLOCK_THREAD_CPUTIME_ID, &m_cpuStart);
#endif
    if (m_track & TRACK_HWINST) {
      m_instStart = HardwareCounter::GetInstructionCount();
    }
  }
}

ServerStatsHelper::~ServerStatsHelper() {
  if (RuntimeOption::EnableStats && RuntimeOption::EnableWebStats) {
    timespec wallEnd;
    Timer::GetMonotonicTime(wallEnd);
#ifdef CLOCK_THREAD_CPUTIME_ID
    timespec cpuEnd;
    gettime(CLOCK_THREAD_CPUTIME_ID, &cpuEnd);
#endif

    logTime("page.wall.", m_wallStart, wallEnd);
#ifdef CLOCK_THREAD_CPUTIME_ID
    logTime("page.cpu.", m_cpuStart, cpuEnd);
#endif

    if (m_track & TRACK_MEMORY) {
      auto const stats = MM().getStats();
      ServerStats::Log(string("mem.") + m_section, stats.peakUsage);
      ServerStats::Log(string("mem.allocated.") + m_section,
                       stats.peakCap);
      ServerStats::Log(string("mem.cumulative.") + m_section,
                       stats.totalAlloc);
    }

    if (m_track & TRACK_HWINST) {
      int64_t instEnd = HardwareCounter::GetInstructionCount();
      logTime("page.inst.", m_instStart, instEnd);
    }
  }
}

void ServerStatsHelper::logTime(const std::string& prefix,
                                const timespec& start, const timespec& end) {
  ServerStats::Log(prefix + m_section, gettime_diff_us(start, end));
}

void ServerStatsHelper::logTime(const std::string& prefix,
                                const int64_t& start, const int64_t& end) {
  ServerStats::Log(prefix + m_section, end - start);
}

///////////////////////////////////////////////////////////////////////////////

IOStatusHelper::IOStatusHelper(const char *name,
                               const char *address /* = NULL */,
                               int port /* = 0 */)
    : m_exeProfiler(ThreadInfo::NetworkIO) {
  assert(name && *name);

  if (ServerStats::s_profile_network ||
      (RuntimeOption::EnableStats && RuntimeOption::EnableWebStats)) {
    std::string msg;
    if (address) {
      msg = address;
    }
    if (port) {
      msg += ":";
      msg += folly::to<std::string>(port);
    }
    ServerStats::SetThreadIOStatus(name, msg.c_str());
  }
}

IOStatusHelper::~IOStatusHelper() {
  if (ServerStats::s_profile_network ||
      (RuntimeOption::EnableStats && RuntimeOption::EnableWebStats)) {
    ServerStats::SetThreadIOStatus(nullptr, nullptr);
  }
}

///////////////////////////////////////////////////////////////////////////////

static void set_curl_status(CURL *cp, CURLINFO info, const char *name,
                            const char *url) {
  double option;
  curl_easy_getinfo(cp, info, &option);
  if (option >= 0) {
    ServerStats::SetThreadIOStatus(name, url, option * 1000000);
  }
}

void set_curl_statuses(CURL *cp, const char *url) {
  set_curl_status(cp, CURLINFO_NAMELOOKUP_TIME,    "curl-namelookup",    url);
  set_curl_status(cp, CURLINFO_CONNECT_TIME,       "curl-connect",       url);
  set_curl_status(cp, CURLINFO_STARTTRANSFER_TIME, "curl-starttransfer", url);
  set_curl_status(cp, CURLINFO_PRETRANSFER_TIME,   "curl-pretransfer",   url);
}

///////////////////////////////////////////////////////////////////////////////

void server_stats_log_mutex(const std::string& stack, int64_t elapsed_us) {
  auto const prefix = folly::to<string>("mutex.", stack);
  ServerStats::Log(prefix + ".hit", 1);
  ServerStats::Log(prefix + ".time", elapsed_us);
}

///////////////////////////////////////////////////////////////////////////////
};
