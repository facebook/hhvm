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
#include "hphp/runtime/server/access-log.h"

#include <sstream>
#include <string>
#include <cctype>
#include <cstdio>
#include <cstdlib>

#include <signal.h>
#include <time.h>
#include <stdio.h>

#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/timestamp.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/log-writer.h"
#include "hphp/runtime/server/server-note.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/server/request-uri.h"
#include "hphp/util/process.h"
#include "hphp/util/atomic.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/hardware-counter.h"
#include "hphp/util/timer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
Mutex AccessLogFileData::m_lock;
std::unordered_map<std::string, AccessLogFileData::factory_t>
AccessLogFileData::m_factories;

AccessLogFileData::AccessLogFileData(const std::string& fil,
                                     const std::string& lnk,
                                     const std::string& fmt)
  : file(fil)
  , symLink(lnk)
  , format(fmt)
{
  /*
   * a LogWriter with it's format can be selected between colons like:
   * Format = :thrift: [["%{%s}t", "out-name", "STRING"], ...]
   */
  m_logOutputType = ClassicWriter::handle;
  auto fmt_ = folly::StringPiece(fmt);
  while (!fmt_.empty() && std::isspace(fmt_.front())) fmt_.pop_front();
  if (fmt_.removePrefix(':')) {
    size_t close = fmt_.find(':');
    if (close != fmt_.npos) {
      m_logOutputType = fmt_.subpiece(0, close).str();
      fmt_.advance(close + 1);
      format = folly::trimWhitespace(fmt_).str();
    }
  }
}

std::unique_ptr<LogWriter> AccessLogFileData::Writer(LogChannel chan) const {
  Lock l(m_lock);
  auto ifactory = m_factories.find(m_logOutputType);
  if (ifactory != m_factories.end()) {
    return ifactory->second(*this, chan);
  }
  throw std::runtime_error(
      ("LogWriter not registered: " + m_logOutputType).c_str());
}

void AccessLogFileData::registerWriter(const std::string& handle,
                                       factory_t factory) {
  Lock l(m_lock);
  m_factories[handle] = factory;
}

AccessLog::~AccessLog() {
  signal(SIGCHLD, SIG_DFL);
}

void AccessLog::init(const std::string &defaultFormat,
                     std::vector<AccessLogFileData> &files,
                     const std::string &username) {
  Lock l(m_lock);
  if (m_initialized) return;
  m_initialized = true;
  m_defaultWriter =
    AccessLogFileData("", "", defaultFormat).Writer(LogChannel::THREADLOCAL);
  m_defaultWriter->init(username, m_fGetThreadData);
  for (auto const& file : files) {
    auto ch = Logger::UseCronolog ? LogChannel::CRONOLOG : LogChannel::REGULAR;
    auto writer = std::shared_ptr<LogWriter>(file.Writer(ch));
    writer->init(username, m_fGetThreadData);
    m_files.push_back(writer);
  }
}

void AccessLog::init(const std::string &defaultFormat,
                     std::map<std::string, AccessLogFileData> &files,
                     const std::string &username) {
  Lock l(m_lock);
  if (m_initialized) return;
  m_initialized = true;
  m_defaultWriter =
    AccessLogFileData("", "", defaultFormat).Writer(LogChannel::THREADLOCAL);
  m_defaultWriter->init(username, m_fGetThreadData);
  for (auto const& file : files) {
    auto ch = Logger::UseCronolog ? LogChannel::CRONOLOG : LogChannel::REGULAR;
    auto writer = std::shared_ptr<LogWriter>(file.second.Writer(ch));
    writer->init(username, m_fGetThreadData);
    m_files.push_back(writer);
  }
}

void AccessLog::init(const std::string &format,
                     const std::string &symLink,
                     const std::string &file,
                     const std::string &username) {
  Lock l(m_lock);
  if (m_initialized) return;
  m_initialized = true;
  m_defaultWriter =
    AccessLogFileData("", "", format).Writer(LogChannel::THREADLOCAL);
  m_defaultWriter->init(username, m_fGetThreadData);
  if (!file.empty() && !format.empty()) {
    auto ch = Logger::UseCronolog ? LogChannel::CRONOLOG : LogChannel::REGULAR;
    auto writer = std::shared_ptr<LogWriter>(
      AccessLogFileData(file, symLink, format).Writer(ch));
    writer->init(username, m_fGetThreadData);
    m_files.push_back(writer);
  }
}

void AccessLog::log(Transport *transport, const VirtualHost *vhost) {
  assert(transport);
  if (!m_initialized) return;
  m_defaultWriter->write(transport, vhost);
  for (auto& file : m_files) file->write(transport, vhost);
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

FILE* LogWriter::getOutputFile() const {
  FILE* outfile = nullptr;
  switch (m_channel) {
    case LogChannel::THREADLOCAL:
      {
        auto tData = (m_threadDataFn ? m_threadDataFn() : nullptr);
        outfile = (tData ? tData->log : nullptr);
      }
      break;
    case LogChannel::CRONOLOG:
      outfile = (m_cronolog.get() ? m_cronolog->getOutputFile() : nullptr);
      break;
    case LogChannel::REGULAR:
      outfile = m_filelog;
      break;
  }
  return outfile;
}

void LogWriter::recordWriteAndMaybeDropCaches(FILE* out, int bytes) {
  switch (m_channel) {
    case LogChannel::THREADLOCAL:
      {
        auto tData = (m_threadDataFn ? m_threadDataFn() : nullptr);
        if (tData) tData->flusher.recordWriteAndMaybeDropCaches(out, bytes);
      }
      break;
    case LogChannel::CRONOLOG:
    case LogChannel::REGULAR:
      m_flusher.recordWriteAndMaybeDropCaches(out, bytes);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
