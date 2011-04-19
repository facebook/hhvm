/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <runtime/ext/ext_output.h>
#include <runtime/ext/ext_json.h>
#include <runtime/base/runtime_option.h>
#include <util/lock.h>
#include <util/logger.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
static ReadWriteMutex s_loggers_mutex;

typedef hphp_string_map<LogFileData> StringToLoggerMap;
struct Loggers {
  StringToLoggerMap plain;
  StringToCronologPtrMap cron;

  ~Loggers() {
    // LogFileData files must be explicitly closed
    for (StringToLoggerMap::iterator it = plain.begin(), end = plain.end();
         it != end; ++it) {
      LogFileData &data = it->second;
      if (data.log) {
        if (it->first[0] == '|') {
          pclose(data.log);
        } else {
          fclose(data.log);
        }
      }
    }
    // Cronolog files are closed automatically
  }
};

static Loggers s_loggers;

static void extractLog(LogFileData &data, CStrRef filename, FILE *&f,
                       int *&bytesWritten, int *&prevBytesWritten) {
  f = data.log;
  if (filename.charAt(0) != '|') {
    bytesWritten = &data.bytesWritten;
    prevBytesWritten = &data.prevBytesWritten;
  }
}

static void extractLog(Cronolog &data, FILE *&f,
                       int *&bytesWritten, int *&prevBytesWritten) {
  f = data.getOutputFile();
  bytesWritten = &data.m_bytesWritten;
  prevBytesWritten = &data.m_prevBytesWritten;
}
}

bool f_hphp_log(CStrRef filename, CStrRef message) {
  if (!RuntimeOption::EnableApplicationLog) {
    return false;
  }

  FILE *f = NULL;
  int *bytesWritten = NULL;
  int *prevBytesWritten = NULL;
  bool found = false;
  {
    ReadLock lock(s_loggers_mutex);
    if (Logger::UseCronolog) {
      StringToCronologPtrMap::iterator iter =
        s_loggers.cron.find(filename.data());
      if (iter != s_loggers.cron.end()) {
        extractLog(*iter->second, f, bytesWritten, prevBytesWritten);
        found = true;
      }
    } else {
      StringToLoggerMap::iterator iter = s_loggers.plain.find(filename.data());
      if (iter != s_loggers.plain.end()) {
        extractLog(iter->second, filename, f, bytesWritten, prevBytesWritten);
        found = true;
      }
    }
  }

  if (!found) {
    WriteLock lock(s_loggers_mutex);
    if (Logger::UseCronolog) {
      std::pair<StringToCronologPtrMap::iterator, bool> res =
        s_loggers.cron.insert(StringToCronologPtrMap::value_type(
          filename.data(), CronologPtr()));
      if (res.second) {
        CronologPtr newCl(new Cronolog);
        if (strchr(filename.c_str(), '%')) {
          newCl->m_template = filename;
          newCl->setPeriodicity();
        } else {
          newCl->m_file = fopen(filename.data(), "a");
        }
        res.first->second = newCl;
      }
      extractLog(*res.first->second, f, bytesWritten, prevBytesWritten);
    } else {
      std::pair<StringToLoggerMap::iterator, bool> res =
        s_loggers.plain.insert(StringToLoggerMap::value_type(
          filename.data(), LogFileData()));
      if (res.second) {
        if (filename.charAt(0) == '|') {
          f = popen(filename.data() + 1, "w");
        } else {
          f = fopen(filename.data(), "a");
        }
        res.first->second = LogFileData(f);
      }
      extractLog(res.first->second, filename,
                 f, bytesWritten, prevBytesWritten);
    }
  }

  if (!f) return false;

  bool ret = (fwrite(message.data(), message.size(), 1, f) == 1);
  if (ret) {
    fflush(f);
    if (bytesWritten) {
      atomic_add(*bytesWritten, message.size());
      Logger::checkDropCache(*bytesWritten, *prevBytesWritten, f);
    }
  }
  return ret;
}

void f_hphp_crash_log(CStrRef name, CStrRef value) {
  StackTraceNoHeap::AddExtraLogging(name.data(), value.data());
}

Array f_hphp_get_status() {
  std::string out;
  ServerStats::ReportStatus(out, ServerStats::JSON);
  return f_json_decode(String(out));
}

static double ts_float(const timespec &ts) {
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000;
}

static String ts_microtime(const timespec &ts) {
  char ret[100];
  snprintf(ret, 100, "%.8F %ld", (double)ts.tv_nsec / 1000000000, ts.tv_sec);
  return String(ret, CopyString);
}

Variant f_hphp_get_timers(bool get_as_float /* = true */) {
  Transport *transport = g_context->getTransport();
  if (transport == NULL) {
    return false;
  }

  const timespec &tsQueue = transport->getQueueTime();
  const timespec &tsWall = transport->getWallTime();
  const timespec &tsCpu = transport->getCpuTime();

  Array ret;
  if (get_as_float) {
    ret.set("queue",        ts_float(tsQueue));
    ret.set("process-wall", ts_float(tsWall));
    ret.set("process-cpu",  ts_float(tsCpu));
  } else {
    ret.set("queue",        ts_microtime(tsQueue));
    ret.set("process-wall", ts_microtime(tsWall));
    ret.set("process-cpu",  ts_microtime(tsCpu));
  }
  return ret;
}

Variant f_hphp_output_global_state(bool serialize /* = true */) {
  Array r(get_global_state());
  if (serialize) {
    return f_serialize(r);
  } else {
    return r;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
