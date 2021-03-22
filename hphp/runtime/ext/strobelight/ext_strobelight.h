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

#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/xenon/ext_xenon.h"

/*
                        Strobelight

  What is it?

  A mode of profiling hhvm based on signals fired from an external
  process.  Specifically, signals will be sent by BPF programs
  triggered off of perf counters in CPU hardware.  This should let us
  do profiling based on arbitrary hardware events with a high degree
  of accuracy.

  WARNINGS ==============================================

  - This is designed to stay out of the way of Xenon by actively
    disabling itself if recent xenon activity is detected., and if you
    are using Xenon you may run tinto issues with collecting
    strobelight data.

  - Signals take time to be handled, so the stacks collected from hhvm
    may not be correlated perfectly with stacks captured from BPF

  - This project is very much in under development, and we are
    investigating how to deal with the above issues.

  API ==================================================

  The api for any user consists of 2 pieces

  1) POSIX signals for triggering signal handlers, and requesting snapshots
  2) a USDT in event-hooks fired when hhvm is changing it's stack state

  1) The signals

    a) kSignumAll (42)

      In tracing_types.h we define HPHP::tracing::kSignumAll which is
      a signal id in the 'realtime' range. Sending this signal will
      cause our a signal handler to be run via sync_signal, which is
      slower than directly handling the signal. The handler will walk
      all active requests and set the surprise flag on each in turn.

      This signal has poorer time resolution than kSignumCurrent, but
      is an easy way to request all active HHVM stacks

    b) kSignumCurrent (43)

      In tracing_types.h we define HPHP::tracing::kSignumCurrent which
      is a signal id in the 'realtime' range. The handler for this
      signal will only set the surprise flag for the current
      thread. This signal is meant be be sent ONLY by
      bpf_send_signal_thread. This ensures that the signal handler
      runs on the same thread that trigged the BPF program to run.

      This should have very good time resolution and can be used to
      correlate perf events with HHVM stack traces closely.

  2) The stack USDT (hhvm/hhvm_stack)

  This USDT provides one argument: a pointer to a fixed-width
  representation of the hhvm stack at the time when the USDT was encountered.

  see HPHP::tracing::backtrace_t

  USAGE ================================================

  Typical usage will have a caller attach a BPF program to the
  hhvm_stack USDT for reading stacks, and separately sending signals
  to signal kSignumCurrent via bpf_send_signal_thread in some other
  function. Any BPF mechanism can be used to drive the signal such as
  perf events (CPU, memory utilization) or kprobes (any kernel
  function being called).

  See the examples directory for more

*/

namespace HPHP {

struct c_WaitableWaitHandle;

struct Strobelight final {
  static Strobelight& getInstance(void) noexcept;
  static bool active();
  static bool isXenonActive();

  ~Strobelight() noexcept {};
  Strobelight(const Strobelight&) = delete;
  Strobelight(Strobelight&&) = delete;
  void operator=(const Strobelight&) = delete;
  void operator=(const Strobelight&&) = delete;

  void init();
  void log(
      Xenon::SampleType t,
      c_WaitableWaitHandle* wh = nullptr) const;
  void surpriseAll();
  static void shutdown();

 private:
  Strobelight() noexcept {};
};

}
