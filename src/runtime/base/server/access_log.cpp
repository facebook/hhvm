/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <runtime/base/server/access_log.h>
#include <runtime/base/time/datetime.h>
#include <runtime/base/time/timestamp.h>
#include <time.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/server_note.h>
#include <runtime/base/server/request_uri.h>
#include <util/process.h>
#include <util/atomic.h>
#include <util/compatibility.h>
#include <util/util.h>

namespace HPHP {
using namespace std;
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
  ASSERT(m_output.empty() && m_cronOutput.empty());
  if (m_files.empty()) return;
  for (vector<AccessLogFileData>::const_iterator it = m_files.begin();
       it != m_files.end(); ++it) {
    const string &file = it->file;
    const string &symLink = it->symLink;
    ASSERT(!file.empty());
    FILE *fp = NULL;
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
      m_output.push_back(LogFileData(fp));
    }
  }
}

void AccessLog::log(Transport *transport, const VirtualHost *vhost) {
  ASSERT(transport);
  if (!m_initialized) return;

  AccessLog::ThreadData *threadData = m_fGetThreadData();
  FILE *threadLog = threadData->log;
  if (threadLog) {
    threadData->bytesWritten +=
      writeLog(transport, vhost, threadLog, m_defaultFormat.c_str());
    Logger::checkDropCache(threadData->bytesWritten,
                           threadData->prevBytesWritten,
                           threadLog);
  }
  if (Logger::UseCronolog) {
    for (uint i = 0; i < m_cronOutput.size(); ++i) {
      Cronolog &cronOutput = *m_cronOutput[i];
      FILE *outFile = cronOutput.getOutputFile();
      if (!outFile) continue;
      const char *format = m_files[i].format.c_str();
      int bytes = writeLog(transport, vhost, outFile, format);
      atomic_add(cronOutput.m_bytesWritten, bytes);
      Logger::checkDropCache(cronOutput.m_bytesWritten,
                             cronOutput.m_prevBytesWritten,
                             outFile);
    }
  } else {
    for (uint i = 0; i < m_output.size(); ++i) {
      LogFileData &output = m_output[i];
      FILE *outFile = output.log;
      if (!outFile) continue;
      const char *format = m_files[i].format.c_str();
      int bytes = writeLog(transport, vhost, outFile, format);
      atomic_add(output.bytesWritten, bytes);
      if (m_files[i].file[0] != '|') {
        Logger::checkDropCache(output.bytesWritten,
                               output.prevBytesWritten,
                               outFile);
      }
    }
  }
}

int AccessLog::writeLog(Transport *transport, const VirtualHost *vhost,
                        FILE *outFile, const char *format) {
   char c;
   ostringstream out;
   while (c = *format++) {
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

bool AccessLog::genField(ostringstream &out, const char* &format,
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
  case 'n':
    if (arg.empty()) return false;
    {
      String note = ServerNote::Get(arg);
      if (note.isNull()) return false;
      out << note.c_str();
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
  case 'r':
    {
      const char *method = NULL;
      switch (transport->getMethod()) {
      case Transport::GET: method = "GET"; break;
      case Transport::POST: method = "POST"; break;
      case Transport::HEAD: method = "HEAD"; break;
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
  case 'U':
    {
      String b, q;
      RequestURI::splitURL(transport->getUrl(), b, q);
      out << b;
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
  case 'D':
    {
      struct timespec now;
      gettime(CLOCK_MONOTONIC, &now);
      out << gettime_diff_us(transport->getWallTime(), now);
    }
    break;
  case 'd':
    {
      struct timespec now;
      gettime(CLOCK_THREAD_CPUTIME_ID, &now);
      out << gettime_diff_us(transport->getCpuTime(), now);
    }
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
  return (m_fGetThreadData()->log = fopen(file, "a")) != NULL;
}
void AccessLog::clearThreadLog() {
  FILE* &threadLog = m_fGetThreadData()->log;
  if (threadLog) {
    fclose(threadLog);
  }
  threadLog = NULL;
}

///////////////////////////////////////////////////////////////////////////////
}
