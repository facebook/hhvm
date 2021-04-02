/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/ext/xenon/ext_xenon.h"

#include "hphp/system/systemlib.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/request-injection-data.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/util/thread-local.h"
#include "hphp/util/rds-local.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/sync-signal.h"

#include <signal.h>
#include <time.h>

#include <folly/Random.h>

namespace HPHP {

TRACE_SET_MOD(xenon);

// Data that is kept per request and is only valid per request.
// This structure gathers a php and async stack trace when log is called.
// These logged stacks can be then gathered via a php call, xenon_get_data.
// It needs to allocate and free its Array per request, because Array lifetime
// is per-request.  So the flow for these objects are:
// allocated when a web request begins (if Xenon is enabled)
// grab snapshots of the php and async stack when log is called
// detach itself from its snapshots when the request is ending.
struct XenonRequestLocalData final {
  XenonRequestLocalData();
  ~XenonRequestLocalData();
  void log(
    Xenon::SampleType t,
    EventHook::Source sourceType,
    c_WaitableWaitHandle* wh = nullptr,
    int64_t triggerTime = 0
  );
  static StaticString show(EventHook::Source sourceType);

  Array createResponse();

  void requestInit();
  void requestShutdown();
  bool getIsProfiledRequest();

  // an array of php stacks
  Array m_stackSnapshots;

  // Set to true if this request is going to handle the log calls
  bool m_isProfiledRequest{false};
};
static RDS_LOCAL(XenonRequestLocalData, s_xenonData);

///////////////////////////////////////////////////////////////////////////////
// statics used by the Xenon classes

const StaticString
  s_asio("Asio"),
  s_class("class"),
  s_function("function"),
  s_file("file"),
  s_interpreter("Interpreter"),
  s_isWait("ioWaitSample"),
  s_jit("Jit"),
  s_line("line"),
  s_metadata("metadata"),
  s_native("Native"),
  s_phpStack("phpStack"),
  s_sourceType("sourceType"),
  s_stack("stack"),
  s_time("time"),
  s_time_ns("timeNano"),
  s_lastTriggerTime("lastTriggerTimeNano"),
  s_unwinder("Unwinder");


namespace {

Array parsePhpStack(const Array& bt) {
  VArrayInit stack(bt->size());
  for (ArrayIter it(bt); it; ++it) {
    const auto& frame = it.second().toArray();
    if (frame.exists(s_function)) {
      bool fileline = frame.exists(s_file) && frame.exists(s_line);
      bool metadata = frame.exists(s_metadata);

      DArrayInit element(1 + (fileline ? 2 : 0) + (metadata ? 1 : 0));

      if (frame.exists(s_class)) {
        auto func = folly::to<std::string>(
          frame[s_class].toString().c_str(),
          "::",
          frame[s_function].toString().c_str()
        );
        element.set(s_function, func);
      } else {
        element.set(s_function, frame[s_function].toString());
      }

      if (fileline) {
        element.set(s_file, frame[s_file]);
        element.set(s_line, frame[s_line]);
      }

      if (metadata) {
        element.set(s_metadata, frame[s_metadata]);
      }

      stack.append(element.toArray());
    }
  }
  return stack.toArray();
}

} // namespace

///////////////////////////////////////////////////////////////////////////
// A singleton object that handles the two Xenon modes (always or timer).
// If in always on mode, the Xenon Surprise flags have to be on for each thread
// and are never cleared.
// For timer mode, when start is invoked, it adds a new timer to the existing
// handler for SIGPROF.

Xenon& Xenon::getInstance() noexcept {
  static Xenon instance;
  return instance;
}

Xenon::Xenon() noexcept : m_lastSurpriseTime(0), m_stopping(false), m_missedSampleCount(0) {
#if !defined(__APPLE__) && !defined(_MSC_VER)
  m_timerid = 0;
#endif
}

void Xenon::incrementMissedSampleCount(ssize_t val) {
  m_missedSampleCount += val;
}

int64_t Xenon::getAndClearMissedSampleCount() {
    return std::atomic_exchange<int64_t>(&m_missedSampleCount, 0);
}

static void onXenonTimer(int signo) {
  if (signo == SIGPROF) {
    Xenon::getInstance().onTimer();
  }
}

// XenonForceAlwaysOn is active - it doesn't need a timer, it is always on.
// Xenon needs to be started once per process.
// The number of milliseconds has to be greater than zero.
// If all of those happen, then we need a timer attached to a signal handler.
void Xenon::start(uint64_t msec) {
#if !defined(__APPLE__) && !defined(_MSC_VER)
  TRACE(1, "XenonForceAlwaysOn %d\n", RuntimeOption::XenonForceAlwaysOn);
  if (!RuntimeOption::XenonForceAlwaysOn && !m_timerid && msec > 0) {
    time_t sec = msec / 1000;
    long nsec = (msec % 1000) * 1000000;
    TRACE(1, "Xenon::start periodic %ld seconds, %ld nanoseconds\n", sec, nsec);

    // for the initial timer, we want to stagger time for large installations
    auto const msecInit = folly::Random::rand32(static_cast<uint32_t>(msec));
    auto const fSec = msecInit / 1000;
    auto const fNsec = (msecInit % 1000) * 1000000;
    TRACE(1, "Xenon::start initial %d seconds, %d nanoseconds\n",
          fSec, fNsec);

    sigevent sev={};
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGPROF;
    timer_create(CLOCK_REALTIME, &sev, &m_timerid);

    sync_signal(SIGPROF, onXenonTimer);
    signal(SIGPROF, SIG_IGN);

    itimerspec ts={};
    ts.it_value.tv_sec = fSec;
    ts.it_value.tv_nsec = fNsec;
    ts.it_interval.tv_sec = sec;
    ts.it_interval.tv_nsec = nsec;
    timer_settime(m_timerid, 0, &ts, nullptr);
  }
#endif
}

// If Xenon owns a pthread, tell it to stop, also clean up anything from start.
void Xenon::stop() {
#if !defined(__APPLE__) && !defined(_MSC_VER)
  if (m_timerid) {
    m_stopping = true;
    TRACE(1, "Xenon::stop has stopped the waiting thread\n");
    timer_delete(m_timerid);
  }
#endif
}

// Xenon data is gathered for logging per request, "if we should"
// meaning that if Xenon's Surprise flag has been turned on by someone, we
// should log the stacks.  If we are in XenonForceAlwaysOn, do not clear
// the Surprise flag.  The data is gathered in thread local storage.
// If the sample is Enter, then do not record this function name because it
// hasn't done anything.  The sample belongs to the previous function.
void Xenon::log(SampleType t,
                EventHook::Source sourceType,
                c_WaitableWaitHandle* wh) const {
  if (getSurpriseFlag(XenonSignalFlag)) {
    if (!RuntimeOption::XenonForceAlwaysOn) {
      clearSurpriseFlag(XenonSignalFlag);
    }
    TRACE(1, "Xenon::log %s\n", show(t));
    s_xenonData->log(t, sourceType, wh, m_lastSurpriseTime);
  }
}

// Called from timer handler, Lets non-signal code know the timer was fired.
void Xenon::onTimer() {
  auto& instance = getInstance();
  if (instance.m_stopping) return;
  instance.surpriseAll();
}

// Turns on the Xenon Surprise flag for every thread via a lambda function
// passed to ExecutePerThread.
void Xenon::surpriseAll() {
  TRACE(1, "Xenon::surpriseAll\n");
  m_lastSurpriseTime = gettime_ns(CLOCK_REALTIME);
  RequestInfo::ExecutePerRequest(
    [] (RequestInfo* t) { t->m_reqInjectionData.setFlag(XenonSignalFlag); }
  );
}

bool Xenon::getIsProfiledRequest() {
  return s_xenonData->getIsProfiledRequest();
}

///////////////////////////////////////////////////////////////////////////////
// There is one XenonRequestLocalData per thread, stored in thread local area

XenonRequestLocalData::XenonRequestLocalData() {
  TRACE(1, "XenonRequestLocalData\n");
}

XenonRequestLocalData::~XenonRequestLocalData() {
  TRACE(1, "~XenonRequestLocalData\n");
}


// Creates an array to respond to the Xenon PHP extension;
// builds the data into the format neeeded.
Array XenonRequestLocalData::createResponse() {
  VArrayInit stacks(m_stackSnapshots.size());
  for (ArrayIter it(m_stackSnapshots); it; ++it) {
    const auto& frame = it.second().toArray();
    stacks.append(make_darray(
      s_time, frame[s_time],
      s_time_ns, frame[s_time_ns],
      s_lastTriggerTime, frame[s_lastTriggerTime],
      s_stack, frame[s_stack].toArray(),
      s_phpStack, parsePhpStack(frame[s_stack].toArray()),
      s_isWait, frame[s_isWait],
      s_sourceType, frame[s_sourceType]
    ));
  }
  return stacks.toArray();
}

StaticString XenonRequestLocalData::show(EventHook::Source sourceType) {
  switch (sourceType) {
    case EventHook::Source::Asio: return s_asio;
    case EventHook::Source::Interpreter: return s_interpreter;
    case EventHook::Source::Jit: return s_jit;;
    case EventHook::Source::Native: return s_native;
    case EventHook::Source::Unwinder: return s_unwinder;
  }
  always_assert(false);
}

void XenonRequestLocalData::log(Xenon::SampleType t,
                                EventHook::Source sourceType,
                                c_WaitableWaitHandle* wh,
                                int64_t triggerTime
) {
  if (!m_isProfiledRequest) return;

  TRACE(1, "XenonRequestLocalData::log\n");
  time_t now = time(nullptr);
  auto now_ns = gettime_ns(CLOCK_REALTIME);
  auto bt = createBacktrace(BacktraceArgs()
                             .skipTop(t == Xenon::EnterSample)
                             .skipInlined(t == Xenon::EnterSample)
                             .fromWaitHandle(wh)
                             .withMetadata()
                             .ignoreArgs());
  m_stackSnapshots.append(make_darray(
    s_time, now,
    s_time_ns, now_ns,
    s_lastTriggerTime, triggerTime,
    s_stack, bt,
    s_sourceType, show(sourceType),
    s_isWait, !Xenon::isCPUTime(t)
  ));
}

void XenonRequestLocalData::requestInit() {
  TRACE(1, "XenonRequestLocalData::requestInit\n");

  assertx(!m_isProfiledRequest);
  assertx(m_stackSnapshots.get() == nullptr);
  if (RuntimeOption::XenonForceAlwaysOn) {
    setSurpriseFlag(XenonSignalFlag);
  } else {
    // Clear any Xenon flags that might still be on in this thread so that we do
    // not have a bias towards the first function.
    clearSurpriseFlag(XenonSignalFlag);
  }

  uint32_t freq = RuntimeOption::XenonRequestFreq;
  m_isProfiledRequest = (freq > 0 && folly::Random::rand32(freq) == 0);
}

void XenonRequestLocalData::requestShutdown() {
  TRACE(1, "XenonRequestLocalData::requestShutdown\n");

  m_isProfiledRequest = false;
  clearSurpriseFlag(XenonSignalFlag);
  Xenon::getInstance().incrementMissedSampleCount(m_stackSnapshots.size());
  m_stackSnapshots.reset();
}

bool XenonRequestLocalData::getIsProfiledRequest() {
  return m_isProfiledRequest;
}

///////////////////////////////////////////////////////////////////////////////
// Function that allows php code to access request local data that has been
// gathered via surprise flags.

Array HHVM_FUNCTION(xenon_get_data, void) {
  if (RuntimeOption::XenonForceAlwaysOn ||
      RuntimeOption::XenonPeriodSeconds > 0) {
    TRACE(1, "xenon_get_data\n");
    return s_xenonData->createResponse();
  }
  return Array::CreateVec();
}

Array HHVM_FUNCTION(xenon_get_and_clear_samples, void) {
  if (RuntimeOption::XenonForceAlwaysOn ||
      RuntimeOption::XenonPeriodSeconds > 0) {
    TRACE(1, "xenon_get_and_clear_samples\n");
    Array ret = s_xenonData->createResponse();
    s_xenonData->m_stackSnapshots.reset();
    return ret;
  }
  return Array::CreateVec();
}

int64_t HHVM_FUNCTION(xenon_get_and_clear_missed_sample_count, void) {
    TRACE(1, "xenon_get_and_clear_missed_sample_count\n");
    return Xenon::getInstance().getAndClearMissedSampleCount();
}

bool HHVM_FUNCTION(xenon_get_is_profiled_request, void) {
  return Xenon::getInstance().getIsProfiledRequest();
}

struct xenonExtension final : Extension {
  xenonExtension() : Extension("xenon", "2.0") { }

  void moduleInit() override {
    HHVM_FALIAS(HH\\xenon_get_data, xenon_get_data);
    HHVM_FALIAS(HH\\xenon_get_and_clear_samples, xenon_get_and_clear_samples);
    HHVM_FALIAS(HH\\xenon_get_and_clear_missed_sample_count,
                xenon_get_and_clear_missed_sample_count);
    HHVM_FALIAS(HH\\xenon_get_is_profiled_request,
                xenon_get_is_profiled_request);
    loadSystemlib();
  }

  void requestInit() override {
    s_xenonData->requestInit();
  }

  void requestShutdown() override {
    s_xenonData->requestShutdown();
  }
} s_xenon_extension;

///////////////////////////////////////////////////////////////////////////////
}
