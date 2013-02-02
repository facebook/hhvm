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
#ifndef __HPHP_ACCESS_LOG_H__
#define __HPHP_ACCESS_LOG_H__

#include <runtime/base/base_includes.h>
#include <util/thread_local.h>
#include <util/logger.h>
#include <util/lock.h>
#include <util/cronolog.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class AccessLogFileData {
public:
  AccessLogFileData(const std::string &fil,
                    const std::string &lnk,
                    const std::string &fmt) :
    file(fil), symLink(lnk), format(fmt) {}
  std::string file;
  std::string symLink;
  std::string format;
};

class AccessLog {
public:
  class ThreadData {
  public:
    ThreadData() : log(NULL), bytesWritten(0), prevBytesWritten(0) {}
    FILE *log;
    int64 startTime;
    int bytesWritten;
    int prevBytesWritten;
  };
  typedef ThreadData* (*GetThreadDataFunc)();
  AccessLog(GetThreadDataFunc f) :
      m_initialized(false), m_fGetThreadData(f) {}
  ~AccessLog();
  void init(const std::string &defaultFormat,
            std::vector<AccessLogFileData> &files,
            const std::string &username);
  void init(const std::string &format, const std::string &symLink,
            const std::string &file, const std::string &username);
  void log(Transport *transport, const VirtualHost *vhost);
  bool setThreadLog(const char *file);
  void clearThreadLog();
  void onNewRequest();
  std::string &defaultFormat() { return m_defaultFormat; }
  std::vector<AccessLogFileData> &files() { return m_files; }
private:
  bool parseConditions(const char* &format, int code);
  std::string parseArgument(const char* &format);
  bool genField(std::ostringstream &out, const char* &format,
                Transport *transport, const VirtualHost *vhost,
                const std::string &arg);
  void skipField(const char* &format);
  int writeLog(Transport *transport, const VirtualHost *vhost,
               FILE *outFile, const char *format);

  std::vector<LogFileData> m_output;
  std::vector<CronologPtr> m_cronOutput;
  bool m_initialized;
  GetThreadDataFunc m_fGetThreadData;
  std::string m_defaultFormat;
  std::vector<AccessLogFileData> m_files;

  void openFiles(const std::string &username);
  Mutex m_lock;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ACCESS_LOG_H__
