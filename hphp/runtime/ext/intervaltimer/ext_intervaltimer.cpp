/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/intervaltimer/ext_intervaltimer.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct TimerPool final : RequestEventHandler {
  void requestInit() override {
    timers = std::unordered_set<IntervalTimer*>();
  }

  void requestShutdown() override {
    do {
      for (auto it = timers.begin(); it != timers.end(); ) {
        auto timer = *it;
        it = timers.erase(it);
        timer->~IntervalTimer();
      }
    } while (!timers.empty());
  }

  std::unordered_set<IntervalTimer*> timers;
};

IMPLEMENT_STATIC_REQUEST_LOCAL(TimerPool, s_timer_pool);

///////////////////////////////////////////////////////////////////////////////

namespace {

const StaticString
  s_IOWait("IOWait"),
  s_ResumeAwait("ResumeAwait"),
  s_Enter("Enter"),
  s_Exit("Exit");

static StaticString sample_type_string(IntervalTimer::SampleType type) {
  switch (type) {
    case IntervalTimer::IOWaitSample: return s_IOWait;
    case IntervalTimer::ResumeAwaitSample: return s_ResumeAwait;
    case IntervalTimer::EnterSample: return s_Enter;
    case IntervalTimer::ExitSample: return s_Exit;
  }
  not_reached();
}

}

void IntervalTimer::RunCallbacks(IntervalTimer::SampleType type) {
  clearSurpriseFlag(IntervalTimerFlag);

  auto const timers = s_timer_pool->timers;
  for (auto timer : timers) {
    if (!s_timer_pool->timers.count(timer)) {
      // This timer has been removed from the pool by one of the callbacks.
      continue;
    }
    int count = 0;
    {
      std::lock_guard<std::mutex> lock(timer->m_signalMutex);
      count = timer->m_count;
      timer->m_count = 0;
      if (count == 0) {
        continue;
      }
    }
    try {
      Array args = make_packed_array(sample_type_string(type), count);
      vm_call_user_func(timer->m_callback, args);
    } catch (Object& ex) {
      raise_error("Uncaught exception escaping IntervalTimer: %s",
                  ex.toString().data());
    }
  }
}

IntervalTimer::~IntervalTimer() {
  stop();
}

void IntervalTimer::init(double interval,
                         double initial,
                         const Variant& callback,
                         RequestInjectionData* data) {
  m_interval = interval;
  m_initial = initial;
  m_callback = callback;
  m_data = data;
}

void IntervalTimer::start() {
  if (!m_data) return;
  if (m_thread.joinable()) return;
  m_done = false;
  m_thread = std::thread([](IntervalTimer* t) { t->run(); }, this);
  s_timer_pool->timers.insert(this);
}

void IntervalTimer::stop() {
  if (!m_data) return;
  if (!m_thread.joinable()) return;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_done = true;
  }
  m_cv.notify_one();
  m_thread.join();
  s_timer_pool->timers.erase(this);
}

void IntervalTimer::run() {
  auto waitTime = m_initial;
  do {
    std::unique_lock<std::mutex> lock(m_mutex);
    auto status = m_cv.wait_for(lock,
                                std::chrono::duration<double>(waitTime),
                                [this]{ return m_done; });
    if (status) break;
    {
      std::lock_guard<std::mutex> l(m_signalMutex);
      m_data->setFlag(IntervalTimerFlag);
      ++m_count;
    }
    waitTime = m_interval;
  } while (waitTime != 0.0);
}

///////////////////////////////////////////////////////////////////////////////

Class* IntervalTimer::c_Class = nullptr;
const StaticString IntervalTimer::c_ClassName("IntervalTimer");

void HHVM_METHOD(IntervalTimer, __construct,
                 double interval,
                 double initial,
                 const Variant& callback) {
  auto data = Native::data<IntervalTimer>(this_);
  data->init(interval, initial, callback,
             &ThreadInfo::s_threadInfo->m_reqInjectionData);
}

void HHVM_METHOD(IntervalTimer, start) {
  auto data = Native::data<IntervalTimer>(this_);
  data->start();
}

void HHVM_METHOD(IntervalTimer, stop) {
  auto data = Native::data<IntervalTimer>(this_);
  data->stop();
}

///////////////////////////////////////////////////////////////////////////////

static struct IntervalTimerExtension final : Extension {
  IntervalTimerExtension() : Extension("intervaltimer") {}

  void moduleInit() override {
    HHVM_ME(IntervalTimer, __construct);
    HHVM_ME(IntervalTimer, start);
    HHVM_ME(IntervalTimer, stop);
    Native::registerNativeDataInfo<IntervalTimer>(
      IntervalTimer::c_ClassName.get(),
      Native::NDIFlags::NO_SWEEP);

    loadSystemlib("intervaltimer");
  }
} s_intervaltimer_extension;

///////////////////////////////////////////////////////////////////////////////
}
