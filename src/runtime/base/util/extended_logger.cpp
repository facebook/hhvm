
#include <runtime/base/util/extended_logger.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/frame_injection.h>
#include <runtime/base/array/array_iterator.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void ExtendedLogger::log(const char *type, const Exception &e,
                         const char *file /* = NULL */, int line /* = 0 */) {
  if (!UseLogAggregator && !UseLogFile) return;
  Logger::log(type, e, file, line);

  if (RuntimeOption::InjectedStackTrace) {
    const ExtendedException *ee = dynamic_cast<const ExtendedException *>(&e);
    if (ee) {
      Log(*ee->getBackTrace());
    }
  }
}

void ExtendedLogger::log(const std::string &msg, const StackTrace *stackTrace,
                         bool escape /* = true */) {
  Logger::log(msg, stackTrace, escape);
  if (RuntimeOption::InjectedStackTrace) {
    Log(FrameInjection::GetBacktrace());
  }
}

void ExtendedLogger::Log(CArrRef stackTrace) {
  ThreadData *threadData = s_threadData.get();
  if (++threadData->message > MaxMessagesPerRequest &&
      MaxMessagesPerRequest >= 0) {
    return;
  }

  if (stackTrace.isNull()) return;

  // TODO Should we also send the stacktrace to LogAggregator?
  if (UseLogFile) {
    FILE *f = Output ? Output : stdout;
    PrintStackTrace(f, stackTrace);

    FILE *tf = threadData->log;
    if (tf) {
      PrintStackTrace(tf, stackTrace);
    }
  }
}

void ExtendedLogger::PrintStackTrace(FILE *f, CArrRef stackTrace) {
  int i = 0;
  for (ArrayIter it(stackTrace); it; ++it, ++i) {
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
    fprintf(f, "at [%s:%lld]\n", frame["file"].toString().c_str(),
            frame["line"].toInt64());
  }
  fflush(f);
}

///////////////////////////////////////////////////////////////////////////////
}
