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

#ifndef incl_HPHP_EXT_XENON_H_
#define incl_HPHP_EXT_XENON_H_

#include "hphp/runtime/ext/extension.h"
#include <semaphore.h>

/*
                        Project Xenon
  What is it?
  The name Xenon is from the type of bulb used in strobelights.  The idea is
  that a timer goes off and the code takes a snapshot of the combined stack
  stack at that time (ie it flashes the code).

  How does it work?
  There are two ways for Xenon to work: 1) always on, 2) via timer.
  For the timer mode:  Xenon appends a timer to the already existing SIGVTALRM
  handler.  When that timer fires it sets a semaphore so that others may
  know.  We'd like to be able to record the status of every stack for every
  thread at this point, but during a timer handler is not a reasonable place
  to do this.
  Instead, Xenon has a pthread waiting for the semaphore.  When the semaphore
  is set in the handler, it wakes, sets the Xenon Surprise flag for every
  thread - this is the flash.
  There is a mechanism that hooks the enter/exit of every PHP function,
  using the Surprise flags leverages that mechanism and calls logging.
  At this point, if the flag is set for this thread, the PHP and async stack
  will be logged for that thread.

  The for the always on mode:  when execute_command_line_begin is called,
  all of the threads are Surprised.  When the Xenon methods are invoked in
  this mode, the Xenon Surprise flags are not cleared, so functions will always
  be Surprised on enter and exit.  In this mode, no threads or semaphores or
  timers are created, since they are not needed.

  How do I use it?
  Enabling it via Xenon Period runtime option will start Xenon snapping and
  gathering data.  There is a PHP extension, xenon_get_data, that returns
  all of the data gather for that request/thread.  Period is a double that
  represents seconds (ie 0.1 is a ten of a second).
  Once can alternatively set Xenon ForceAlways=true, which will then
  gather combined stacks at every function enter/exit.  This will
  gather potentially large amounts of data - mostly used for small
  script debugging.
*/

namespace HPHP {

struct c_WaitableWaitHandle;

struct Xenon final {

    enum SampleType {
      // Sample was taken during I/O wait and thus does not represent CPU time.
      IOWaitSample,

      // Sample was taken to trace loading of new units.
      UnitLoadEvent,

      // Sample was taken before an async function was resumed at await opcode.
      // The CPU time is attributed to the resumed async function, because the
      // CPU time was spent by the scheduler on the behalf of the resumed
      // function (preparing for reentry, unserializing result of external
      // thread event, etc.).
      ResumeAwaitSample,

      // Sample was taken before a function was called or a generator was
      // resumed at yield opcode.
      // The CPU time is attributed to the caller of the entered function.
      // Inlined frames are skipped as they did not trigger EnterSample events
      // to properly attribute their parent's cost.
      EnterSample,

      // Sample was taken before a function returned, suspended or failed
      // with an exception.
      // The CPU time is attributed to the exited function.
      ExitSample,
    };

    static bool isCPUTime(SampleType t) {
      switch (t) {
        case IOWaitSample:
        case UnitLoadEvent:
          return false;
        case ResumeAwaitSample:
        case EnterSample:
        case ExitSample:
          return true;
      }
      always_assert(false);
    }

    static const char* show(SampleType t) {
      switch (t) {
        case IOWaitSample: return "IOWait";
        case UnitLoadEvent: return "UnitLoad";
        case ResumeAwaitSample: return "ResumeAwait";
        case EnterSample: return "Enter";
        case ExitSample: return "Exit";
      }
      always_assert(false);
    }

    static Xenon& getInstance(void) noexcept;

    Xenon() noexcept;
    ~Xenon() noexcept {};
    Xenon(const Xenon&) = delete;
    void operator=(const Xenon&) = delete;

    void start(uint64_t msec);
    void stop();
    void incrementMissedSampleCount(ssize_t val);
    int64_t getAndClearMissedSampleCount();
    // Log a sample if XenonSignalFlag is set. Also clear it, unless
    // in always-on mode.
    void log(SampleType t, c_WaitableWaitHandle* wh = nullptr) const;
    // Like `log' but does not check or reset XenonSignalFlag. `info' may be
    // brief description of why sample was logged, or nullptr.
    void logNoSurprise(SampleType t,
                       const char* info,
                       c_WaitableWaitHandle* wh = nullptr) const;
    void surpriseAll();
    void onTimer();

    bool      m_stopping;
  private:
    std::atomic<int64_t> m_missedSampleCount;
    sem_t     m_timerTriggered;
#if !defined(__APPLE__) && !defined(_MSC_VER)
    pthread_t m_triggerThread;
    timer_t   m_timerid;
#endif
};
}

#endif // incl_HPHP_EXT_XENON_H_
