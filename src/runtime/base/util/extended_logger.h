#ifndef __RUNTIME_BASE_EXTENDED_LOGGER_H__
#define __RUNTIME_BASE_EXTENDED_LOGGER_H__

#include <util/logger.h>
#include <util/exception.h>
#include <util/stack_trace.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ExtendedLogger : public Logger {
protected:
  virtual void log(const char *type, const Exception &e,
                   const char *file = NULL, int line = 0);
  virtual void log(const std::string &msg, const StackTrace *stackTrace,
                   bool escape = true);
private:
  // Log additional injected stacktrace.
  void Log(CArrRef stackTrace);
  void PrintStackTrace(FILE *f, CArrRef stackTrace);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __RUNTIME_BASE_EXTENDED_LOGGER_H__
