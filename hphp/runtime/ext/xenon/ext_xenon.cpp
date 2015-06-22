/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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


#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/request-injection-data.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/vm/vm-regs.h"

#include <signal.h>
#include <time.h>

#include <iostream>

namespace HPHP {

TRACE_SET_MOD(xenon);

void *s_waitThread(void *arg) {
  TRACE(1, "s_waitThread Starting\n");
  sem_t* sem = static_cast<sem_t*>(arg);
  while (sem_wait(sem) == 0) {
    TRACE(1, "s_waitThread Fired\n");
    if (Xenon::getInstance().m_stopping) {
      TRACE(1, "s_waitThread is exiting\n");
      return nullptr;
    }
    Xenon::getInstance().surpriseAll();
  }
  TRACE(1, "s_waitThread Ending\n");
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

// Data that is kept per request and is only valid per request.
// This structure gathers a php and async stack trace when log is called.
// These logged stacks can be then gathered via php a call, xenon_get_data.
// It needs to allocate and free its Array per request, because Array lifetime
// is per-request.  So the flow for these objects are:
// allocated when a web request begins (if Xenon is enabled)
// grab snapshots of the php and async stack when log is called
// detach itself from its snapshots when the request is ending.
namespace {
struct XenonRequestLocalData final : RequestEventHandler  {
  XenonRequestLocalData();
  virtual ~XenonRequestLocalData();
  void log(Xenon::SampleType t);
  Array createResponse();

  // implement RequestEventHandler
  void requestInit() override;
  void requestShutdown() override;

  // an array of php stacks
  Array m_stackSnapshots;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(XenonRequestLocalData, s_xenonData);
}

///////////////////////////////////////////////////////////////////////////////
// statics used by the Xenon classes

const StaticString
  s_class("class"),
  s_function("function"),
  s_file("file"),
  s_line("line"),
  s_metadata("metadata"),
  s_time("time"),
  s_isWait("ioWaitSample"),
  s_phpStack("phpStack");

namespace {

Array parsePhpStack(const Array& bt) {
  PackedArrayInit stack(bt->size());
  for (ArrayIter it(bt); it; ++it) {
    const auto& frame = it.second().toArray();
    if (frame.exists(s_function)) {
      bool fileline = frame.exists(s_file) && frame.exists(s_line);
      bool metadata = frame.exists(s_metadata);

      ArrayInit element(
        1 + (fileline ? 2 : 0) + (metadata ? 1 : 0),
        ArrayInit::Map{}
      );

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
// handler for SIGVTALRM.

Xenon& Xenon::getInstance() noexcept {
  static Xenon instance;
  return instance;
}

Xenon::Xenon() noexcept : m_stopping(false) {
#ifndef __APPLE__
  m_timerid = 0;
#endif
}

// XenonForceAlwaysOn is active - it doesn't need a timer, it is always on.
// Xenon needs to be started once per process.
// The number of milliseconds has to be greater than zero.
// We need to create a semaphore and a thread.
// If all of those happen, then we need a timer attached to a signal handler.
void Xenon::start(uint64_t msec) {
#ifndef __APPLE__
  TRACE(1, "XenonForceAlwaysOn %d\n", RuntimeOption::XenonForceAlwaysOn);
  if (!RuntimeOption::XenonForceAlwaysOn
      && m_timerid == 0
      && msec > 0
      && sem_init(&m_timerTriggered, 0, 0) == 0
      && pthread_create(&m_triggerThread, nullptr, s_waitThread,
          static_cast<void*>(&m_timerTriggered)) == 0) {

    time_t sec = msec / 1000;
    long nsec = (msec % 1000) * 1000000;
    TRACE(1, "Xenon::start periodic %ld seconds, %ld nanoseconds\n", sec, nsec);

    // for the initial timer, we want to stagger time for large installations
    unsigned int seed = time(nullptr);
    uint64_t msecInit = msec * (1.0 + rand_r(&seed) / (double)RAND_MAX);
    time_t fSec = msecInit / 1000;
    long fNsec = (msecInit % 1000) * 1000000;
    TRACE(1, "Xenon::start initial %ld seconds, %ld nanoseconds\n",
       fSec, fNsec);

    sigevent sev={};
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGVTALRM;
    sev.sigev_value.sival_ptr = nullptr; // null for Xenon signals
    timer_create(CLOCK_REALTIME, &sev, &m_timerid);

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
#ifndef __APPLE__
  if (m_timerid) {
    m_stopping = true;
    sem_post(&m_timerTriggered);
    pthread_join(m_triggerThread, nullptr);
    TRACE(1, "Xenon::stop has stopped the waiting thread\n");
    timer_delete(m_timerid);
    sem_destroy(&m_timerTriggered);
  }
#endif
}

// Xenon data is gathered for logging per request, "if we should"
// meaning that if Xenon's Surprise flag has been turned on by someone, we
// should log the stacks.  If we are in XenonForceAlwaysOn, do not clear
// the Surprise flag.  The data is gathered in thread local storage.
// If the sample is Enter, then do not record this function name because it
// hasn't done anything.  The sample belongs to the previous function.
void Xenon::log(SampleType t) const {
  if (getSurpriseFlag(XenonSignalFlag)) {
    if (!RuntimeOption::XenonForceAlwaysOn) {
      clearSurpriseFlag(XenonSignalFlag);
    }
    TRACE(1, "Xenon::log %s\n", (t == IOWaitSample) ? "IOWait" : "Normal");
    s_xenonData->log(t);
  }
}

// Called from timer handler, Lets non-signal code know the timer was fired.
void Xenon::onTimer() {
  sem_post(&m_timerTriggered);
}

// Turns on the Xenon Surprise flag for every thread via a lambda function
// passed to ExecutePerThread.
void Xenon::surpriseAll() {
  TRACE(1, "Xenon::surpriseAll\n");
  ThreadInfo::ExecutePerThread(
    [] (ThreadInfo* t) { t->m_reqInjectionData.setFlag(XenonSignalFlag); }
  );
}

///////////////////////////////////////////////////////////////////////////////
// There is one XenonRequestLocalData per thread, stored in thread local area

namespace {

XenonRequestLocalData::XenonRequestLocalData() {
  TRACE(1, "XenonRequestLocalData\n");
}

XenonRequestLocalData::~XenonRequestLocalData() {
  TRACE(1, "~XenonRequestLocalData\n");
}


// Creates an array to respond to the Xenon PHP extension;
// builds the data into the format neeeded.
Array XenonRequestLocalData::createResponse() {
  PackedArrayInit stacks(m_stackSnapshots.size());
  for (ArrayIter it(m_stackSnapshots); it; ++it) {
    const auto& frame = it.second().toArray();
    stacks.append(make_map_array(
      s_time, frame[s_time],
      s_phpStack, parsePhpStack(frame[s_phpStack].toArray()),
      s_isWait, frame[s_isWait]
    ));
  }
  return stacks.toArray();
}

void XenonRequestLocalData::log(Xenon::SampleType t) {
  TRACE(1, "XenonRequestLocalData::log\n");
  time_t now = time(nullptr);
  auto bt = createBacktrace(BacktraceArgs()
                             .skipTop(t == Xenon::EnterSample)
                             .withSelf()
                             .withMetadata()
                             .ignoreArgs());
  m_stackSnapshots.append(make_map_array(
    s_time, now,
    s_phpStack, bt,
    s_isWait, (t == Xenon::IOWaitSample)
  ));
}

void XenonRequestLocalData::requestInit() {
  TRACE(1, "XenonRequestLocalData::requestInit\n");
  m_stackSnapshots = Array::Create();
  if (RuntimeOption::XenonForceAlwaysOn) {
    setSurpriseFlag(XenonSignalFlag);
  } else {
    // Clear any Xenon flags that might still be on in this thread so that we do
    // not have a bias towards the first function.
    clearSurpriseFlag(XenonSignalFlag);
  }
}

void XenonRequestLocalData::requestShutdown() {
  TRACE(1, "XenonRequestLocalData::requestShutdown\n");
  clearSurpriseFlag(XenonSignalFlag);
  m_stackSnapshots.detach();
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
  return empty_array();
}

} // namespace

class xenonExtension final : public Extension {
 public:
  xenonExtension() : Extension("xenon", "1.0") { }

  void moduleInit() override {
    HHVM_FALIAS(HH\\xenon_get_data, xenon_get_data);
    loadSystemlib();
  }
} s_xenon_extension;

///////////////////////////////////////////////////////////////////////////////
}
