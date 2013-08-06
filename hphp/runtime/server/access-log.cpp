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
#include "hphp/runtime/server/access-log.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/timestamp.h"
#include <time.h>
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/server-note.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/server/request-uri.h"
#include "hphp/util/process.h"
#include "hphp/util/atomic.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/timer.h"
#include "hphp/util/util.h"
#include "hphp/runtime/base/hardware-counter.h"

using std::endl;

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

AccessLog::~AccessLog() {
  signal(SIGCHLD, SIG_DFL);
  for (uint i = 0; i < m_output.size(); ++i) {
    if (m_output[i].log) {
      if (m_files[i].file[0] == '|') {
        pclose(m_output[i].log);
      } else {
        fclose(m_output[i].log);
      }
    }
  }
}

void AccessLog::init(const string &defaultFormat,
                     vector<AccessLogFileData> &files,
                     const string &username) {
  Lock l(m_lock);
  if (m_initialized) return;
  m_initialized = true;
  m_defaultFormat = defaultFormat;
  m_files = files;
  openFiles(username);
}

void AccessLog::init(const string &format,
                     const string &symLink,
                     const string &file,
                     const string &username) {
  Lock l(m_lock);
  if (m_initialized) return;
  m_initialized = true;
  m_defaultFormat = format;
  if (!file.empty() && !format.empty()) {
    m_files.push_back(AccessLogFileData(file, symLink, format));
  }
  openFiles(username);
}

void AccessLog::openFiles(const string &username) {
  assert(m_output.empty() && m_cronOutput.empty());
  if (m_files.empty()) return;
  for (vector<AccessLogFileData>::const_iterator it = m_files.begin();
       it != m_files.end(); ++it) {
    const string &file = it->file;
    const string &symLink = it->symLink;
    assert(!file.empty());
    FILE *fp = nullptr;
    if (Logger::UseCronolog) {
      CronologPtr cl(new Cronolog);
      if (strchr(file.c_str(), '%')) {
        cl->m_template = file;
        cl->setPeriodicity();
        cl->m_linkName = symLink;
        Cronolog::changeOwner(username, symLink);
      } else {
        cl->m_file = fopen(file.c_str(), "a");
      }
      m_cronOutput.push_back(cl);
    } else {
      if (file[0] == '|') {
        string plog = file.substr(1);
        fp = popen(plog.c_str(), "w");
      } else {
        fp = fopen(file.c_str(), "a");
      }
      if (!fp) {
        Logger::Error("Could not open access log file %s", file.c_str());
      }
      m_output.emplace_back(fp);
    }
  }
}

void AccessLog::log(Transport *transport, const VirtualHost *vhost) {
  assert(transport);
  if (!m_initialized) return;

  AccessLog::ThreadData *threadData = m_fGetThreadData();
  FILE *threadLog = threadData->log;
  if (threadLog) {
    int bytes = writeLog(transport, vhost, threadLog, m_defaultFormat.c_str());
    threadData->flusher.recordWriteAndMaybeDropCaches(threadLog, bytes);
  }
  if (Logger::UseCronolog) {
    for (uint i = 0; i < m_cronOutput.size(); ++i) {
      Cronolog &cronOutput = *m_cronOutput[i];
      FILE *outFile = cronOutput.getOutputFile();
      if (!outFile) continue;
      const char *format = m_files[i].format.c_str();
      int bytes = writeLog(transport, vhost, outFile, format);
      cronOutput.flusher.recordWriteAndMaybeDropCaches(outFile, bytes);
    }
  } else {
    for (uint i = 0; i < m_output.size(); ++i) {
      LogFileData& output = m_output[i];
      FILE *outFile = output.log;
      if (!outFile) continue;
      const char *format = m_files[i].format.c_str();
      int bytes = writeLog(transport, vhost, outFile, format);
      if (m_files[i].file[0] != '|') {
        output.flusher.recordWriteAndMaybeDropCaches(outFile, bytes);
      }
    }
  }
}

int AccessLog::writeLog(Transport *transport, const VirtualHost *vhost,
                        FILE *outFile, const char *format) {
   char c;
   std::ostringstream out;
   while ((c = *format++)) {
     if (c != '%') {
       out << c;
       continue;
     }

     if (parseConditions(format, transport->getResponseCode())) {
       string arg = parseArgument(format);
       if (!genField(out, format, transport, vhost, arg)) {
         out << "-";
       }
     } else {
       skipField(format);
       out << "-";
     }
   }
   out << endl;
   string output = out.str();
   int nbytes = fprintf(outFile, "%s", output.c_str());
   fflush(outFile);
   return nbytes;
}

bool AccessLog::parseConditions(const char* &format, int code) {
  bool wantMatch = true;
  if (*format == '!') {
    wantMatch = false;
    format++;
  } else if (!isdigit(*format)) {
    // No conditions
    return true;
  }
  char buf[4];
  buf[3] = '\0';

  bool matched = false;
  while(isdigit(*format)) {
    buf[0] = format[0];
    buf[1] = format[1];
    buf[2] = format[2];
    int c = atoi(buf);
    if (c == code) {
      matched = true;
      break;
    }
    format+=4;
  }
  while (!(*format == '{' || isalpha(*format))) {
    format++;
  }
  return wantMatch == matched;
}

string AccessLog::parseArgument(const char* &format) {
  if (*format != '{') return string();
  format++;
  const char *start = format;
  while (*format != '}') { format++; }
  string res(start, format - start);
  format++;
  return res;
}

void AccessLog::skipField(const char* &format) {
  // Skip argument
  if (*format == '{') {
    while (*format != '}') { format++; }
    format++;
  }
  // Find control letter
  while (!isalpha(*format)) { format++; }
  // Skip it
  format++;
}

static void escape_data(std::ostringstream &out, const char *s, int len)
{
  static const char digits[] = "0123456789abcdef";

  for (int i = 0; i < len; i++) {
    unsigned char uc = *s++;
    switch (uc) {
      case '"':  out << "\\\""; break;
      case '\\': out << "\\\\"; break;
      case '\b': out << "\\b";  break;
      case '\f': out << "\\f";  break;
      case '\n': out << "\\n";  break;
      case '\r': out << "\\r";  break;
      case '\t': out << "\\t";  break;
      default:
        if (uc >= ' ' && (uc & 127) == uc) {
          out << (char)uc;
        } else {
          out << "\\x" << digits[(uc >> 4) & 15] << digits[(uc >> 0) & 15];
        }
        break;
    }
  }
}

bool AccessLog::genField(std::ostringstream &out, const char* &format,
                         Transport *transport, const VirtualHost *vhost,
                         const string &arg) {
  int responseSize = transport->getResponseSize();
  int code = transport->getResponseCode();

  while (!isalpha(*format)) { format++; }
  char type = *format;
  format++;

  switch (type) {
  case 'b':
    if (responseSize == 0) return false;
    // Fall through
  case 'B':
    out << responseSize;
    break;
  case 'C':
    if (arg.empty()) {
      return false;
    } else {
      string cookie = transport->getCookie(arg);
      if (cookie.empty()) return false;
      escape_data(out, cookie.c_str(), cookie.size());
    }
    break;
  case 'D':
    {
      struct timespec now;
      Timer::GetMonotonicTime(now);
      out << gettime_diff_us(transport->getWallTime(), now);
    }
    break;
  case 'd':
    {
#ifdef CLOCK_THREAD_CPUTIME_ID
      struct timespec now;
      gettime(CLOCK_THREAD_CPUTIME_ID, &now);
      out << gettime_diff_us(transport->getCpuTime(), now);
#else
      return false;
#endif
    }
    break;
  case 'h':
    out << transport->getRemoteHost();
    break;
  case 'i':
    if (arg.empty()) return false;
    {
      string header = transport->getHeader(arg.c_str());
      if (header.empty()) return false;

      if (vhost && vhost->hasLogFilter() &&
          strcasecmp(arg.c_str(), "Referer") == 0) {
        out << vhost->filterUrl(header);
      } else {
        out << header;
      }
    }
    break;
  case 'I':
    out << transport->getRequestSize();
    break;
  case 'n':
    if (arg.empty()) return false;
    {
      String note = ServerNote::Get(arg);
      if (note.isNull()) return false;
      out << note.c_str();
    }
    break;
  case 'r':
    {
      const char *method = nullptr;
      switch (transport->getMethod()) {
      case Transport::Method::GET: method = "GET"; break;
      case Transport::Method::POST: method = "POST"; break;
      case Transport::Method::HEAD: method = "HEAD"; break;
      default: break;
      }
      if (!method) return false;
      out << method << " ";

      const char *url = transport->getUrl();
      if (vhost && vhost->hasLogFilter()) {
        out << vhost->filterUrl(url);
      } else {
        out << url;
      }

      string httpVersion = transport->getHTTPVersion();
      out << " HTTP/" << httpVersion;
    }
    break;
  case 's':
    out << code;
    break;
  case 'S':
    // %S is not defined in Apache, we grab it here
    {
      const std::string &info (transport->getResponseInfo());
      if (info.empty()) return false;
      out << info;
    }
    break;
  case 't':
    {
      const char *format;
      if (arg.empty()) {
        format = "[%d/%b/%Y:%H:%M:%S %z]";
      } else {
        format = arg.c_str();
      }
      char buf[256];
      time_t rawtime;
      struct tm * timeinfo;
      time(&rawtime);
      timeinfo = localtime(&rawtime);
      strftime(buf, 256, format, timeinfo);
      out << buf;
    }
    break;
  case 'T':
    out << TimeStamp::Current() - m_fGetThreadData()->startTime;
    break;
  case 'U':
    {
      String b, q;
      RequestURI::splitURL(transport->getUrl(), b, q);
      out << b.c_str();
    }
    break;
  case 'v':
    {
      string host = transport->getHeader("Host");
      const string &sname = VirtualHost::GetCurrent()->serverName(host);
      if (sname.empty() || RuntimeOption::ForceServerNameToHeader) {
        out << host;
      } else {
        out << sname;
      }
    }
    break;
  case 'Y':
    {
      int64_t now = HardwareCounter::GetInstructionCount();
      out << now - transport->getInstructions();
    }
    break;
  case 'y':
    out << ServerStats::Get("page.inst.psp");
    break;
  case 'Z':
     out << ServerStats::Get("page.wall.psp");
     break;
  case 'z':
     out << ServerStats::Get("page.cpu.psp");
     break;
  default:
    return false;
  }
  return true;
}

void AccessLog::onNewRequest() {
  if (!m_initialized) return;
  ThreadData *threadData = m_fGetThreadData();
  threadData->startTime = TimeStamp::Current();
}

bool AccessLog::setThreadLog(const char *file) {
  return (m_fGetThreadData()->log = fopen(file, "a")) != nullptr;
}
void AccessLog::clearThreadLog() {
  FILE* &threadLog = m_fGetThreadData()->log;
  if (threadLog) {
    fclose(threadLog);
  }
  threadLog = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}
