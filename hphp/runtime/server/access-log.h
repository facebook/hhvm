/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_ACCESS_LOG_H_
#define incl_HPHP_ACCESS_LOG_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/runtime/server/virtual-host.h"
#include "hphp/util/thread-local.h"
#include "hphp/util/logger.h"
#include "hphp/util/lock.h"
#include "hphp/util/cronolog.h"
#include "hphp/util/log-file-flusher.h"


namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
class LogWriter;
enum class LogChannel {CRONOLOG, THREADLOCAL, REGULAR};

class AccessLogFileData {
public:
  AccessLogFileData() {}
  AccessLogFileData(const std::string& fil,
                    const std::string& lnk,
                    const std::string& fmt);
  std::string file;
  std::string symLink;
  std::string format;
  // Concrete LogWriter factories to be used depending on the handle
  using factory_t = std::function<
    std::unique_ptr<LogWriter>(const AccessLogFileData&, LogChannel)>;
  // Ask a registered factory for a new LogWriter instance
  std::unique_ptr<LogWriter> Writer(LogChannel chan) const;
  static void registerWriter(const std::string& handle, factory_t factory);
private:
  std::string m_logOutputType;
  static Mutex m_lock;
  static std::unordered_map<std::string, factory_t> m_factories;
};


class AccessLog {
public:
  class ThreadData {
  public:
    ThreadData() : log(nullptr) {}
    FILE *log;
    int64_t startTime;
    LogFileFlusher flusher;
  };
  typedef ThreadData* (*GetThreadDataFunc)();
  explicit AccessLog(GetThreadDataFunc f) :
    m_initialized(false), m_fGetThreadData(f) {}
  ~AccessLog();
  void init(const std::string &defaultFormat,
            std::vector<AccessLogFileData> &files,
            const std::string &username);
  void init(const std::string &defaultFormat,
            std::map<std::string, AccessLogFileData> &files,
            const std::string &username);
  void init(const std::string &format, const std::string &symLink,
            const std::string &file, const std::string &username);
  void log(Transport *transport, const VirtualHost *vhost);
  bool setThreadLog(const char *file);
  void clearThreadLog();
  void onNewRequest();
private:

  bool m_initialized;
  Mutex m_lock;
  GetThreadDataFunc m_fGetThreadData;
  std::unique_ptr<LogWriter> m_defaultWriter;
  std::vector<std::shared_ptr<LogWriter>> m_files;
};


class LogWriter {
public:
  explicit LogWriter(LogChannel chan)
    : m_channel(chan)
  {}
  virtual ~LogWriter() {};
  virtual void init(const std::string& username,
                    AccessLog::GetThreadDataFunc fn) = 0;
  virtual void write(Transport* transport, const VirtualHost* vhost) = 0;
protected:
  const LogChannel m_channel;
  FILE* m_filelog{nullptr};
  std::unique_ptr<Cronolog> m_cronolog;
  AccessLog::GetThreadDataFunc m_threadDataFn{nullptr};
  LogFileFlusher m_flusher;
protected:
  FILE* getOutputFile() const;
  void recordWriteAndMaybeDropCaches(FILE* out, int bytes);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ACCESS_LOG_H_
