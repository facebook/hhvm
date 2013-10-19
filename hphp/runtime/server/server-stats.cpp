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

#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/util/json.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/process.h"
#include "hphp/util/timer.h"
#include "hphp/runtime/base/hardware-counter.h"

using std::list;
using std::set;
using std::map;
using std::ostream;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// helpers

void ServerStats::GetLogger() {
  s_logger.getCheck();
}

void ServerStats::Merge(CounterMap &dest, const CounterMap &src) {
  for (CounterMap::const_iterator iter = src.begin();
       iter != src.end(); ++iter) {
    dest[iter->first] += iter->second;
  }
}

void ServerStats::Merge(PageStatsMap &dest, const PageStatsMap &src) {
  for (PageStatsMap::const_iterator iter = src.begin();
       iter != src.end(); ++iter) {
    const SharedString &key = iter->first;
    const PageStats &s = iter->second;

    PageStatsMap::iterator diter = dest.find(key);
    if (diter == dest.end()) {
      dest[key] = s;
    } else {
      PageStats &d = diter->second;
      assert(d.m_url == s.m_url);
      assert(d.m_code == s.m_code);
      d.m_hit += s.m_hit;
      Merge(d.m_values, s.m_values);
    }
  }
}

void ServerStats::Merge(list<TimeSlot*> &dest, const list<TimeSlot*> &src) {
  list<TimeSlot*>::iterator diter = dest.begin();
  for (list<TimeSlot*>::const_iterator iter = src.begin();
       iter != src.end(); ++iter) {
    TimeSlot *s = *iter;

    for (; diter != dest.end(); ++diter) {
      TimeSlot *d = *diter;
      if (d->m_time > s->m_time) {
        TimeSlot *c = new TimeSlot();
        *c = *s;
        dest.insert(diter, c);
        break;
      }
      if (d->m_time == s->m_time) {
        Merge(d->m_pages, s->m_pages);
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

void ServerStats::GetAllKeys(set<string> &allKeys,
                             const list<TimeSlot*> &slots) {
  for (list<TimeSlot*>::const_iterator iter = slots.begin();
       iter != slots.end(); ++iter) {
    TimeSlot *s = *iter;
    for (PageStatsMap::const_iterator piter = s->m_pages.begin();
         piter != s->m_pages.end(); ++piter) {
      const PageStats &ps = piter->second;
      for (CounterMap::const_iterator viter =
             ps.m_values.begin(); viter != ps.m_values.end(); ++viter) {
        allKeys.insert(viter->first->getString());
      }
    }
  }

  // special keys
  allKeys.insert("hit");
  allKeys.insert("load");
  allKeys.insert("idle");
  allKeys.insert("queued");
}

void ServerStats::Filter(list<TimeSlot*> &slots, const std::string &keys,
                         const std::string &url, int code,
                         map<string, int> &wantedKeys) {
  if (!keys.empty()) {
    vector<string> rules0;
    Util::split(',', keys.c_str(), rules0, true);
    if (!rules0.empty()) {

      // prepare rules
      map<string, int> rules;
      for (unsigned int i = 0; i < rules0.size(); i++) {
        string &rule = rules0[i];
        assert(!rule.empty());
        int len = rule.length();
        string suffix;
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
      set<string> allKeys;
      GetAllKeys(allKeys, slots);

      // prepare wantedKeys
      for (set<string>::const_iterator iter = allKeys.begin();
           iter != allKeys.end(); ++iter) {
        const string &key = *iter;
        for (map<string, int>::const_iterator riter = rules.begin();
             riter != rules.end(); ++riter) {
          const string &rule = riter->first;
          if (rule[0] == ':') {
            Variant ret = preg_match(String(rule.c_str(), rule.size(),
                  CopyString),
                String(key.c_str(), key.size(), CopyString));
            if (!same(ret, false) && more(ret, 0)) {
              wantedKeys[key] |= riter->second;
            }
          } else if (rule == key) {
            wantedKeys[key] |= riter->second;
          }
        }
      }
    }
  }

  bool urlEmpty = url.empty();
  bool keysEmpty = keys.empty();
  for (list<TimeSlot*>::const_iterator iter = slots.begin();
       iter != slots.end(); ++iter) {
    TimeSlot *s = *iter;
    for (PageStatsMap::iterator piter = s->m_pages.begin();
         piter != s->m_pages.end();) {
      PageStats &ps = piter->second;
      if ((code && ps.m_code != code) || (!urlEmpty && ps.m_url != url)) {
        PageStatsMap::iterator piterTemp = piter;
        ++piter;
        s->m_pages.erase(piterTemp);
        continue;
      }

      if (!keysEmpty) {
        CounterMap &values = ps.m_values;
        for (CounterMap::iterator viter =
               values.begin(); viter != values.end();) {
          if (wantedKeys.find(viter->first->getString()) == wantedKeys.end()) {
            CounterMap::iterator iterTemp = viter;
            ++viter;
            values.erase(iterTemp);
          } else {
            ++viter;
          }
        }
      }
      ++piter;
    }
  }
}

void ServerStats::Aggregate(list<TimeSlot*> &slots, const std::string &agg,
                            std::map<std::string, int> &wantedKeys) {
  int slotCount = slots.size();

  if (!agg.empty()) {
    TimeSlot *ts = new TimeSlot();
    ts->m_time = 0;
    for (list<TimeSlot*>::const_iterator iter = slots.begin();
         iter != slots.end(); ++iter) {
      TimeSlot *s = *iter;
      for (PageStatsMap::const_iterator piter = s->m_pages.begin();
           piter != s->m_pages.end(); ++piter) {
        const PageStats &ps = piter->second;
        string url = ps.m_url;
        int code = ps.m_code;
        if (agg != "url") {
          url.clear();
        }
        if (agg != "code") {
          code = 0;
        }
        PageStats &psDest = ts->m_pages[url + lexical_cast<string>(code)];
        psDest.m_hit += ps.m_hit;
        psDest.m_url = url;
        psDest.m_code = code;
        Merge(psDest.m_values, ps.m_values);
      }
    }
    FreeSlots(slots);
    slots.push_back(ts);
  }

  std::map<std::string, int> udfKeys;
  for (std::map<std::string, int>::const_iterator iter = wantedKeys.begin();
       iter != wantedKeys.end(); ++iter) {
    if (iter->second != UDF_NONE) {
      udfKeys[iter->first] = iter->second;
    }
  }

  // Hack: These two are not really page specific.
  int load = HttpServer::Server->getPageServer()->getActiveWorker();
  int idle = RuntimeOption::ServerThreadCount - load;
  int queued = HttpServer::Server->getPageServer()->getQueuedJobs();

  for (list<TimeSlot*>::const_iterator iter = slots.begin();
       iter != slots.end(); ++iter) {
    TimeSlot *s = *iter;
    int sec = (s->m_time == 0 ? slotCount : 1) *
      RuntimeOption::StatsSlotDuration;
    for (PageStatsMap::iterator piter = s->m_pages.begin();
         piter != s->m_pages.end(); ++piter) {
      PageStats &ps = piter->second;
      CounterMap &values = ps.m_values;

      // special keys
      if (wantedKeys.find("hit") != wantedKeys.end()) {
        values["hit"] = ps.m_hit;
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

      for (map<string, int>::const_iterator iter = udfKeys.begin();
           iter != udfKeys.end(); ++iter) {
        const string &key = iter->first;
        int udf = iter->second;
        CounterMap::iterator viter = values.find(key);
        if (viter != values.end()) {
          if ((udf & UDF_HIT) && ps.m_hit) {
            values[key + "/hit"] = viter->second * PRECISION / ps.m_hit;
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
}

void ServerStats::FreeSlots(list<TimeSlot*> &slots) {
  for (list<TimeSlot*>::const_iterator iter = slots.begin();
       iter != slots.end(); ++iter) {
    delete *iter;
  }
  slots.clear();
}

///////////////////////////////////////////////////////////////////////////////
// writers

class Writer {
public:
  explicit Writer(ostream &out) : m_out(out), m_indent(0) {}
  virtual ~Writer() {}

  virtual void writeFileHeader() = 0;
  virtual void writeFileFooter() = 0;

  // Begins writing an object which is different than a list in JSON.
  virtual void beginObject(const char *name) = 0;

  // Begins writing a list (an ordered set of potentially unnamed children)
  // Defaults to begining an object.
  virtual void beginList(const char *name) {
    beginObject(name);
  }

  // Writes a string value with a given name.
  virtual void writeEntry(const char *name, const std::string &value) = 0;

  // Writes a string value with a given name.
  virtual void writeEntry(const char *name, int64_t value) = 0;

  // Ends the writing of an object.
  virtual void endObject(const char *name) = 0;

  // Ends the writing of a list. Defaults to simply ending an Object.
  virtual void endList(const char *name) {
    endObject(name);
  }


protected:
  ostream &m_out;
  int m_indent;

  virtual void writeIndent() {
    for (int i = 0; i < m_indent; i++) {
      m_out << "  ";
    }
  }
};

class XMLWriter : public Writer {
public:
  explicit XMLWriter(ostream &out) : Writer(out) {}


  virtual void writeFileHeader() {
    m_out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    if (!RuntimeOption::StatsXSL.empty()) {
      m_out << "<?xml-stylesheet type=\"text/xsl\" href=\""
            << RuntimeOption::StatsXSL << "\"?>\n";
    } else if (!RuntimeOption::StatsXSLProxy.empty()) {
      m_out << "<?xml-stylesheet type=\"text/xsl\" href=\"stats.xsl\"?>\n";
    }
  }

  virtual void writeFileFooter() {}


  /**
   * In XML/HTML there is no distinction between creating a list, and creating
   * an object with keyed attributes. (unlike the JSON format).
   */
  virtual void beginObject(const char *name) {
    writeIndent();
    m_out << '<' << name << ">\n";
    ++m_indent;
  }

  virtual void endObject(const char *name) {
    --m_indent;
    writeIndent();
    m_out << "</" << name << ">\n";
  }

  virtual void writeEntry(const char *name, const string &value) {
    writeIndent();
    m_out << "<entry><key>" << Escape(name) << "</key>";
    m_out << "<value>" << Escape(value.c_str()) << "</value></entry>\n";
  }

  virtual void writeEntry(const char *name, int64_t value) {
    writeIndent();
    m_out << "<entry><key>" << Escape(name) << "</key>";
    m_out << "<value>" << value << "</value></entry>\n";
  }

private:
  static std::string Escape(const char *s) {
    string ret;
    for (const char *p = s; *p; p++) {
      switch (*p) {
      case '<':  ret += "&lt;";  break;
      case '&':  ret += "&amp;"; break;
      default:   ret += *p;      break;
      }
    }
    return ret;
  }
};

class JSONWriter : public Writer {

protected:
  // Whether or not we have skiped a comma for this current indent level. Valid
  // json may not have trailing commas such as {"a":4, "b":5,} Since we are
  // writing to a stream, we output *valid* json that has commas preceding all
  // elements except the first, which is equivalent to outputing commas after
  // each element except the last. Skip the preceding comma when m_justIndented.
  bool m_justIndented;

  // Stack that determines whether or not at a given object depth level, we are
  // to be listing child objects with keyed entries. For example, in Json,
  // inside an array, entries are not keyed. Also at the json root node, we
  // begin at a nameless context.
  std::stack<bool> m_namelessContextStack;

  virtual void increaseIndent() {
    ++m_indent;
    m_justIndented = true;
  }

  /**
   * It is *important* to set m_justIndented to false here in the event that we
   * write objects with *no* members, we need to set it to false.
   */
  virtual void decreaseIndent() {
    --m_indent;
    m_justIndented = false;

    // We should never pop off more than we pushed, but just in case someone
    // called too many endObject's etc, we don't want a segfault.
    if (m_namelessContextStack.size() != 0) {
      m_namelessContextStack.pop();
    }
  }

  /**
   * Begins writing a containing entity (such as a list or object).
   * See the 'isList' parameter.
   */
  virtual void beginContainer(const char *name, bool isList) {
    char opener = isList ? '[' : '{';
    beginEntity(name);
    m_out << opener << '\n';

    // Push on whether or not we're entering a nameless context
    m_namelessContextStack.push(isList);
    increaseIndent();
  }

  virtual void endContainer(bool isList) {
    char closer = isList ? ']' : '}';
    decreaseIndent();
    writeIndent();
    m_out << closer << '\n';
  }

  /**
   * Writes any needed leading commas, and keyed name if appropriate.
   * Reduces redundancy. Used whenever an entity is a child of another
   * entity - does all the work of determining if the object should be
   * written with/without a name and if we need a comma.
   */
  virtual void beginEntity(const char *name) {
    writeIndent();
    if (!m_justIndented) {
      m_out << ", ";
    }
    if (m_namelessContextStack.size() != 0 && !m_namelessContextStack.top()) {
      m_out << '"' << JSON::Escape(name) << "\": ";
    }
    m_justIndented = false;
  }

public:

  explicit JSONWriter(ostream &out) : Writer(out),
      m_justIndented(true) {

    // A valid json object begins in the nameless context. See
    // json.org for JSON state machine.
    m_namelessContextStack.push(true);
  }

  virtual void writeFileHeader() {
    beginContainer("root", false);
  }

  virtual void writeFileFooter() {
    endContainer(false);
  }


  virtual void beginObject(const char *name) {
    beginContainer(name, false);
  }

  virtual void beginList(const char *name) {
    beginContainer(name, true);
  }

  void endObject(const char *name) {
    endContainer(false);
  }

  void endList(const char *name) {
    endContainer(true);
  }

  virtual void writeEntry(const char *name, const string &value) {
    beginEntity(name);

    // Now write the actual value
    m_out << "\"" << JSON::Escape(value.c_str()) << "\"\n";
  }

  virtual void writeEntry(const char *name, int64_t value) {
    beginEntity(name);

    // Now write the actual value
    m_out << value << '\n';
  }
};

class HTMLWriter : public Writer {

public:
  explicit HTMLWriter(ostream &out) : Writer(out) {}

  virtual void writeFileHeader() {
    m_out << "<!doctype html>\n<html>\n<head>\n"
             "<meta http-equiv=\"content-type\" "
             "content=\"text/html; charset=UTF-8\">\n"
             "<style type=\"text/css\"> table {margin-left:20px} "
             "th {text-align:left}</style>\n"
             "<title>HPHP Stats</title>\n"
             "</head>\n<body>\n<table>\n<tbody>\n";
  }

  virtual void writeFileFooter() {
    m_out << "</tbody>\n</table>\n</body>\n</html>\n";
  }


  /**
   * In XML/HTML there is no distinction between creating a list, and creating
   * an object with keyed attributes. (unlike the JSON format).
   */
  virtual void beginObject(const char *name) {
    writeIndent();
    m_out << "<tr id='" << name << "'><td colspan=2>"
          << "<table><tbody><tr><th colspan=2>" << name << "</th></tr>\n";
    ++m_indent;
  }

  virtual void endObject(const char *name) {
    --m_indent;
    writeIndent();
    m_out << "</tbody></table></td></tr>\n";
  }

  virtual void writeEntry(const char *name, const string &value) {
    writeIndent();
    m_out << "<tr><td>" << Escape(name) << "</td>";
    m_out << "<td>" << Escape(value.c_str()) << "</td></tr>\n";
  }

  virtual void writeEntry(const char *name, int64_t value) {
    writeIndent();
    m_out << "<tr><td>" << Escape(name) << "</td>";
    m_out << "<td>" << value << "</td></tr>\n";
  }

private:
  static std::string Escape(const char *s) {
    string ret;
    for (const char *p = s; *p; p++) {
      switch (*p) {
      case '<':  ret += "&lt;";  break;
      case '&':  ret += "&amp;"; break;
      default:   ret += *p;      break;
      }
    }
    return ret;
  }
};

///////////////////////////////////////////////////////////////////////////////
// static

Mutex ServerStats::s_lock;
vector<ServerStats*> ServerStats::s_loggers;
bool ServerStats::s_profile_network = false;
IMPLEMENT_THREAD_LOCAL_NO_CHECK(ServerStats, ServerStats::s_logger);

void ServerStats::LogPage(const string &url, int code) {
  if (RuntimeOption::EnableStats && RuntimeOption::EnableWebStats) {
    ServerStats::s_logger->logPage(url, code);
  }
}

void ServerStats::Log(const string &name, int64_t value) {
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

void ServerStats::SetThreadMode(ThreadMode mode) {
  ServerStats::s_logger->setThreadMode(mode);
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

int64_t ServerStats::Get(const string &name) {
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

void ServerStats::CollectSlots(list<TimeSlot*> &slots, int64_t from, int64_t to) {
  if (from < 0 || to <= 0) {
    time_t now = time(nullptr);
    if (from < 0) from = now + from;
    if (to <= 0) to = now + to;
  }

  int tp1 = from / RuntimeOption::StatsSlotDuration;
  int tp2 = to / RuntimeOption::StatsSlotDuration;

  Lock lock(s_lock, false);
  for (unsigned int i = 0; i < s_loggers.size(); i++) {
    s_loggers[i]->collect(slots, tp1, tp2);
  }
}

void ServerStats::GetKeys(string &out, int64_t from, int64_t to) {
  list<TimeSlot*> slots;
  CollectSlots(slots, from, to);
  set<string> allKeys;
  GetAllKeys(allKeys, slots);
  for (set<string>::const_iterator iter = allKeys.begin();
       iter != allKeys.end(); ++iter) {
    out += *iter;
    out += "\n";
  }
}

void ServerStats::Report(string &out, Format format, int64_t from, int64_t to,
                         const std::string &agg, const std::string &keys,
                         const std::string &url, int code,
                         const std::string &prefix) {
  list<TimeSlot*> slots;
  CollectSlots(slots, from, to);
  map<string, int> wantedKeys;
  Filter(slots, keys, url, code, wantedKeys);
  Aggregate(slots, agg, wantedKeys);
  Report(out, format, slots, prefix);
  FreeSlots(slots);
}

void ServerStats::Report(string &output, Format format,
                         const list<TimeSlot*> &slots,
                         const std::string &prefix) {
  std::ostringstream out;
  if (format == Format::KVP) {
    bool first = true;
    for (list<TimeSlot*>::const_iterator iter = slots.begin();
         iter != slots.end(); ++iter) {
      if (first) {
        first = false;
      } else {
        out << ",\n";
      }
      TimeSlot *s = *iter;
      if (s->m_time) {
        out << s->m_time << ": ";
      }
      out << "{";
      for (PageStatsMap::const_iterator piter = s->m_pages.begin();
           piter != s->m_pages.end(); ++piter) {
        const PageStats &ps = piter->second;
        string key = prefix;
        if (!ps.m_url.empty()) {
          key += ps.m_url;
        }
        if (ps.m_code) {
          key += "$";
          key += lexical_cast<string>(ps.m_code);
        }
        if (!key.empty()) {
          key += ".";
        }
        bool firstKey = true;
        for (CounterMap::const_iterator viter =
               ps.m_values.begin(); viter != ps.m_values.end(); ++viter) {
          if (firstKey) {
            firstKey = false;
          } else {
            out << ", ";
          }
          out << '"' << JSON::Escape((key + viter->first->getString()).c_str())
              << "\": " << viter->second;
        }
      }
      out << "}\n";
    }

  } else {
    Writer *w;
    if (format == Format::XML) {
      w = new XMLWriter(out);
    } else if (format == Format::HTML) {
      w = new HTMLWriter(out);
    } else {
      assert(format == Format::JSON);
      w = new JSONWriter(out);
    }

    w->writeFileHeader();
    w->beginObject("stats");
    for (list<TimeSlot*>::const_iterator iter = slots.begin();
         iter != slots.end(); ++iter) {
      TimeSlot *s = *iter;
      if (s->m_time) {
        w->beginObject("slot");
        w->writeEntry("time", s->m_time * RuntimeOption::StatsSlotDuration);
      }
      w->beginList("pages");
      for (PageStatsMap::const_iterator piter = s->m_pages.begin();
           piter != s->m_pages.end(); ++piter) {
        const PageStats &ps = piter->second;
        w->beginObject("page");
        w->writeEntry("url", ps.m_url);
        w->writeEntry("code", ps.m_code);
        w->writeEntry("hit", ps.m_hit);

        w->beginObject("details");
        for (CounterMap::const_iterator viter =
               ps.m_values.begin(); viter != ps.m_values.end(); ++viter) {
          w->writeEntry(viter->first->getString().c_str(), viter->second);
        }
        w->endObject("details");

        w->endObject("page");
      }
      w->endList("pages");
      if (s->m_time) {
        w->endObject("slot");
      }
    }
    w->endObject("stats");
    w->writeFileFooter();

    delete w;
  }

  output = out.str();
}

static std::string format_duration(timeval &duration) {
  string ret;
  if (duration.tv_sec > 0 || duration.tv_usec > 0) {
    int milliseconds = duration.tv_usec / 1000;
    double seconds = duration.tv_sec % 60 + milliseconds * .001;
    int minutes = duration.tv_sec / 60;
    int hours = minutes / 60;
    minutes = minutes % 60;
    if (hours) {
      ret += lexical_cast<string>(hours) + " hour";
      ret += (hours == 1) ? " " : "s ";
    }
    if (minutes || (hours && seconds)) {
      ret += lexical_cast<string>(minutes) + " minute";
      ret += (minutes == 1) ? " " : "s ";
    }
    if (seconds || minutes || hours) {
      char buf[7];
      snprintf(buf, sizeof(buf), "%.3f", seconds);
      buf[sizeof(buf) - 1] = '\0';
      ret += lexical_cast<string>(buf) + " second";
      ret += (seconds == 1) ? "" : "s";
    }
  } else {
   ret = "0 seconds";
  }
  return ret;
}

void ServerStats::ReportStatus(std::string &output, Format format) {
  std::ostringstream out;
  Writer *w;
  if (format == Format::XML) {
    w = new XMLWriter(out);
  } else if (format == Format::HTML) {
    w = new HTMLWriter(out);
  } else {
    assert(format == Format::JSON);
    w = new JSONWriter(out);
  }

  time_t now = time(0);

  w->writeFileHeader();
  w->beginObject("status");

  w->beginObject("process");
  w->writeEntry("id", (int64_t)Process::GetProcessId());
  w->writeEntry("build", RuntimeOption::BuildId);

  w->writeEntry("compiler", kCompilerId);

#ifdef DEBUG
  w->writeEntry("debug", "yes");
#else
  w->writeEntry("debug", "no");
#endif

#ifdef HOTPROFILER
  w->writeEntry("hotprofiler", "yes");
#else
  w->writeEntry("hotprofiler", "no");
#endif

  timeval up;
  up.tv_sec = now - HttpServer::StartTime;
  up.tv_usec = 0;
  w->writeEntry("now", DateTime(now).
                toString(DateTime::DateFormatCookie).data());
  w->writeEntry("start", DateTime(HttpServer::StartTime).
                toString(DateTime::DateFormatCookie).data());
  w->writeEntry("up", format_duration(up));
  w->endObject("process");

  w->beginList("threads");
  Lock lock(s_lock, false);
  for (unsigned int i = 0; i < s_loggers.size(); i++) {
    ThreadStatus &ts = s_loggers[i]->m_threadStatus;

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
    w->writeEntry("req", ts.m_requestCount);
    w->writeEntry("bytes", ts.m_writeBytes);
    w->writeEntry("start", DateTime(ts.m_start.tv_sec).
                  toString(DateTime::DateFormatCookie).data());
    w->writeEntry("duration", format_duration(duration));
    if (ts.m_requestCount > 0) {
      auto const stats = ts.m_mm->getStatsCopy();
      w->beginObject("memory");
      w->writeEntry("current usage", stats.usage);
      w->writeEntry("current alloc", stats.alloc);
      w->writeEntry("peak usage", stats.peakUsage);
      w->writeEntry("peak alloc", stats.peakAlloc);
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
    Lock lock(ss->m_lock, false);
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
    Lock lock(ss->m_lock, false);

    IOStatusMap &status = ss->m_ioProfiles;
    for (IOStatusMap::const_iterator iter = status.begin();
         iter != status.end(); ++iter) {
      ret.set(String(iter->first),
              make_map_array(s_ct, iter->second.count,
                          s_wt, iter->second.wall_time));
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

void ServerStats::log(const string &name, int64_t value) {
  m_values[name] += value;
}

int64_t ServerStats::get(const std::string &name) {
  CounterMap::const_iterator iter = m_values.find(name);
  if (iter != m_values.end()) {
    return iter->second;
  }
  return 0;
}

void ServerStats::logPage(const string &url, int code) {
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
    TimeSlot &ts = m_slots[slot];
    if (ts.m_time != now) {
      if (ts.m_time && m_min <= ts.m_time) {
        m_min = ts.m_time + 1;
      }
      ts.m_time = now;
      ts.m_pages.clear();
    }
    PageStats &ps = ts.m_pages[url + lexical_cast<string>(code)];
    ps.m_url = url;
    ps.m_code = code;
    ps.m_hit++;
    Merge(ps.m_values, m_values);
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

void ServerStats::collect(std::list<TimeSlot*> &slots, int64_t from, int64_t to) {
  if (from > to) {
    int64_t tmp = from;
    from = to;
    to = tmp;
  }
  if (from < m_min) from = m_min;
  if (to > m_max) to = m_max;

  Lock lock(m_lock, false);
  list<TimeSlot*> collected;
  for (int64_t t = from; t <= to; t++) {
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
  strncpy(dest, src, len > max ? max : len);
}

void ServerStats::startRequest(const char *url, const char *clientIP,
                               const char *vhost) {
  ++m_threadStatus.m_requestCount;

  m_threadStatus.m_mm = ThreadInfo::s_threadInfo->m_mm;
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

      const char *name = m_threadStatus.m_ioName;
      const char *addr = m_threadStatus.m_ioLogicalName;
      if (!*addr) addr = m_threadStatus.m_ioAddr;

      if (RuntimeOption::EnableNetworkIOStatus) {
        string key = name;
        if (*addr) {
          key += ' '; key += addr;
        }
        IOStatus &io = m_threadStatus.m_ioStatuses[key];
        ++io.count;
        io.wall_time += wt;
      }

      if (s_profile_network) {
        const char *key0 = "main()";
        const char *key1 = m_threadStatus.m_url;
        string key2 = m_threadStatus.m_url; key2 += "==>"; key2 += name;
        const char *key3 = name;
        string key4 = name;
        if (*addr) {
          key4 += "==>"; key4 += addr;
        }

        Lock lock(m_lock, false);
        { IOStatus &io = m_ioProfiles[key0]; ++io.count; io.wall_time += wt;}
        { IOStatus &io = m_ioProfiles[key1]; ++io.count; io.wall_time += wt;}
        { IOStatus &io = m_ioProfiles[key2]; ++io.count; io.wall_time += wt;}
        { IOStatus &io = m_ioProfiles[key3]; ++io.count; io.wall_time += wt;}
        if (*addr) {
          IOStatus &io = m_ioProfiles[key4]; ++io.count; io.wall_time += wt;
        }
      }

      *m_threadStatus.m_ioLogicalName = 0;
    }
  }
}

Array ServerStats::getThreadIOStatuses() {
  Array ret;
  IOStatusMap &status = m_threadStatus.m_ioStatuses;
  for (IOStatusMap::const_iterator iter = status.begin();
       iter != status.end(); ++iter) {
    ret.set(String(iter->first),
            make_map_array(s_ct, iter->second.count,
                        s_wt, iter->second.wall_time));
  }
  status.clear();
  return ret;
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
      int64_t mem = MM().getStats().peakUsage;
      ServerStats::Log(string("mem.") + m_section, mem);
    }

    if (m_track & TRACK_HWINST) {
      int64_t instEnd = HardwareCounter::GetInstructionCount();
      logTime("page.inst.", m_instStart, instEnd);
    }
  }
}

void ServerStatsHelper::logTime(const std::string &prefix,
                                const timespec &start, const timespec &end) {
  ServerStats::Log(prefix + m_section, gettime_diff_us(start, end));
}

void ServerStatsHelper::logTime(const std::string &prefix,
                                const int64_t &start, const int64_t &end) {
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
      msg += boost::lexical_cast<std::string>(port);
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

void server_stats_log_mutex(const std::string &stack, int64_t elapsed_us) {
  char buf[128];
  snprintf(buf, sizeof(buf), "mutex.%s.", stack.c_str());
  ServerStats::Log(string(buf) + "hit", 1);
  ServerStats::Log(string(buf) + "time", elapsed_us);
}

///////////////////////////////////////////////////////////////////////////////
}
