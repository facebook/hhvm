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

#include <runtime/base/util/extended_logger.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/frame_injection.h>
#include <runtime/base/array/array_iterator.h>

///////////////////////////////////////////////////////////////////////////////

#define IMPLEMENT_LOGLEVEL(LOGLEVEL, err)                              \
  void ExtendedLogger::LOGLEVEL(const char *fmt, ...) {                \
    if (LogLevel < Log ## LOGLEVEL) return;                            \
    if (RuntimeOption::InjectedStackTrace &&                           \
        !ExtendedLogger::EnabledByDefault) {                           \
      Array bt = FrameInjection::GetBacktrace();                       \
      if (!bt.empty()) {                                               \
        va_list ap; va_start(ap, fmt);                                 \
        Logger::LogEscapeMore(err, fmt, ap);                           \
        va_end(ap);                                                    \
        Log(err, bt);                                                  \
        return;                                                        \
      }                                                                \
    }                                                                  \
    va_list ap; va_start(ap, fmt);                                     \
    Logger::Log(err, fmt, ap);                                         \
    va_end(ap);                                                        \
  }                                                                    \
  void ExtendedLogger::LOGLEVEL(const std::string &msg) {              \
    if (LogLevel < Log ## LOGLEVEL) return;                            \
    if (RuntimeOption::InjectedStackTrace &&                           \
        !ExtendedLogger::EnabledByDefault) {                           \
      Array bt = FrameInjection::GetBacktrace();                       \
      if (!bt.empty()) {                                               \
        Logger::Log(err, msg, NULL, true, true);                       \
        Log(err, bt);                                                  \
        return;                                                        \
      }                                                                \
    }                                                                  \
    Logger::Log(err, msg, NULL, true);                                 \
  }                                                                    \
  void ExtendedLogger::Raw ## LOGLEVEL(const std::string &msg) {       \
    if (LogLevel < Log ## LOGLEVEL) return;                            \
    Logger::Log(err, msg, NULL, false);                                \
    if (RuntimeOption::InjectedStackTrace &&                           \
        !ExtendedLogger::EnabledByDefault) {                           \
      Log(err, FrameInjection::GetBacktrace());                        \
    }                                                                  \
  }                                                                    \

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

bool ExtendedLogger::EnabledByDefault = false;

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_LOGLEVEL(Error,   true);
IMPLEMENT_LOGLEVEL(Warning, true);
IMPLEMENT_LOGLEVEL(Info,    false);
IMPLEMENT_LOGLEVEL(Verbose, false);

///////////////////////////////////////////////////////////////////////////////

void ExtendedLogger::log(bool err, const char *type, const Exception &e,
                         const char *file /* = NULL */, int line /* = 0 */) {
  if (!UseLogAggregator && !UseLogFile) return;
  Logger::log(err, type, e, file, line);

  if (RuntimeOption::InjectedStackTrace) {
    const ExtendedException *ee = dynamic_cast<const ExtendedException *>(&e);
    if (ee) {
      Log(err, *ee->getBackTrace());
    }
  }
}

void ExtendedLogger::log(bool err, const std::string &msg,
                         const StackTrace *stackTrace,
                         bool escape /* = true */,
                         bool escapeMore /* = false */) {
  if (RuntimeOption::InjectedStackTrace) {
    Array bt = FrameInjection::GetBacktrace();
    if (!bt.empty()) {
      Logger::log(err, msg, stackTrace, escape, escape);
      Log(err, bt, escape, escapeMore);
      return;
    }
  }
  Logger::log(err, msg, stackTrace, escape, escapeMore);
}

void ExtendedLogger::Log(bool err, CArrRef stackTrace,
                         bool escape /* = true */,
                         bool escapeMore /* = false */) {
  ASSERT(!escapeMore || escape);
  ThreadData *threadData = s_threadData.get();
  if (++threadData->message > MaxMessagesPerRequest &&
      MaxMessagesPerRequest >= 0) {
    return;
  }

  if (stackTrace.isNull()) return;

  // TODO Should we also send the stacktrace to LogAggregator?
  if (UseLogFile) {
    FILE *f = Output ? Output : (err ? stderr : stdout);
    PrintStackTrace(f, stackTrace, escape, escapeMore);

    FILE *tf = threadData->log;
    if (tf) {
      PrintStackTrace(tf, stackTrace, escape, escapeMore);
    }
  }
}

void ExtendedLogger::PrintStackTrace(FILE *f, CArrRef stackTrace,
                                     bool escape /* = false */,
                                     bool escapeMore /* = false */) {
  int i = 0;
  for (ArrayIter it(stackTrace); it; ++it, ++i) {
    if (i > 0) {
      fprintf(f, "%s", escape ? "\\n" : "\n");
    }
    Array frame = it.second().toArray();
    fprintf(f, "    #%d ", i);
    if (frame.exists("function")) {
      if (frame.exists("class")) {
        fprintf(f, "%s%s%s(), called ", frame["class"].toString().c_str(),
                frame["type"].toString().c_str(),
                frame["function"].toString().c_str());
      } else {
        fprintf(f, "%s(), called ", frame["function"].toString().c_str());
      }
    }
    fprintf(f, "at [%s:%lld]", frame["file"].toString().c_str(),
            frame["line"].toInt64());
  }
  fprintf(f, escapeMore ? "\\n" : "\n");
  fflush(f);
}

std::string ExtendedLogger::StringOfStackTrace(CArrRef stackTrace) {
  std::ostringstream res;
  int i = 0;
  for (ArrayIter it(stackTrace); it; ++it, ++i) {
    Array frame = it.second().toArray();
    res << "    #" << i << " ";
    if (frame.exists("function")) {
      if (frame.exists("class")) {
        res << frame["class"].toString().c_str()
            << frame["type"].toString().c_str()
            << frame["function"].toString().c_str()
            << "(), called ";
      } else {
        res << frame["function"].toString().c_str()
            << "(), called ";
      }
    }
    res << "at [" << frame["file"].toString().c_str()
        << ":" << frame["line"].toInt64() << "]\n";
  }
  std::string buff(res.str());
  return buff;
}
///////////////////////////////////////////////////////////////////////////////
}
