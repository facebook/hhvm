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
#include <runtime/base/server/access_log.h>
#include <runtime/base/time/datetime.h>
#include <runtime/base/time/timestamp.h>
#include <time.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/server_note.h>
#include <runtime/base/server/request_uri.h>
#include <util/process.h>

namespace HPHP {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

AccessLog::~AccessLog() {
  for (uint i = 0; i < m_output.size(); ++i) {
    if (m_output[i]) {
      if (m_files[i].first[0] == '|') {
        pclose(m_output[i]);
      } else {
        fclose(m_output[i]);
      }
    }
  }
}

bool AccessLog::init(const string &defaultFormat,
                     vector<pair<string, string> > &files) {
  Lock l(m_initLock);
  if (m_initialized) return false;
  m_initialized = true;
  m_defaultFormat = defaultFormat;
  m_files = files;
  return openFiles();
}

bool AccessLog::init(const string &format,
                     const string &file) {
  Lock l(m_initLock);
  if (m_initialized) return false;
  m_initialized = true;
  m_defaultFormat = format;
  if (!file.empty() && !format.empty()) {
    m_files.push_back(pair<string, string>(file, format));
  }
  return openFiles();
}

bool AccessLog::openFiles() {
  ASSERT(m_output.empty());
  if (m_files.empty()) return false;
  for (vector<pair<string, string> >::const_iterator it = m_files.begin();
       it != m_files.end(); ++it) {
    const string &file = it->first;
    ASSERT(!file.empty());
    FILE *fp = NULL;
    if (file[0] == '|') {
      string plog = file.substr(1);
      fp = popen(plog.c_str(), "w");
    } else {
      fp = fopen(file.c_str(), "a");
    }
    if (!fp) {
      Logger::Error("Could not open access log file %s", file.c_str());
    }
    m_output.push_back(fp);
  }
  return !m_output.empty();
}

void AccessLog::log(Transport *transport) {
  ASSERT(transport);
  if (!m_initialized) return;

  FILE *threadLog = m_threadData->log;
  if (threadLog) {
    writeLog(transport, threadLog,
             m_defaultFormat.c_str());
  }
  for (uint i = 0; i < m_output.size(); ++i) {
    FILE *outFile = m_output[i];
    if (!outFile) continue;
    const char *format = m_files[i].second.c_str();
    writeLog(transport, outFile, format);
  }
}

void AccessLog::writeLog(Transport *transport, FILE *outFile,
                         const char *format) {
   char c;
   ostringstream out;
   while (c = *format++) {
     if (c != '%') {
       out << c;
       continue;
     }

     if (parseConditions(format, transport->getResponseCode())) {
       string arg = parseArgument(format);
       if (!genField(out, format, transport, arg)) {
         out << "-";
       }
     } else {
       skipField(format);
       out << "-";
     }
   }
   out << endl;
   string output = out.str();
   fprintf(outFile, "%s", output.c_str());
   fflush(outFile);
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
                         Transport *transport, const string &arg) {
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
      out << header;
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
    out << TimeStamp::Current() - m_threadData->startTime;
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
      const char *url = transport->getUrl();
      string httpVersion = transport->getHTTPVersion();
      out << method << " " << url << " HTTP/" << httpVersion;
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
      const string &sname = VirtualHost::GetCurrent()->serverName();
      if (sname.empty() || RuntimeOption::ForceServerNameToHeader) {
        out << transport->getHeader("Host");
      } else {
        out << sname;
      }
    }
    break;
  default:
    return false;
  }
  return true;
}

void AccessLog::onNewRequest() {
  if (!m_initialized) return;
  ThreadData *threadData = m_threadData.get();
  threadData->startTime = TimeStamp::Current();
}

bool AccessLog::setThreadLog(const char *file) {
  return (m_threadData->log = fopen(file, "a")) != NULL;
}
void AccessLog::clearThreadLog() {
  FILE* &threadLog = m_threadData->log;
  if (threadLog) {
    fclose(threadLog);
  }
  threadLog = NULL;
}

///////////////////////////////////////////////////////////////////////////////
}
