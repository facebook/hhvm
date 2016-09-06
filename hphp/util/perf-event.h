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

#ifndef incl_HPHP_UTIL_PERF_EVENT_H_
#define incl_HPHP_UTIL_PERF_EVENT_H_

#include <cstdint>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

enum class PerfEvent { Load, Store };

/*
 * Raw data from a sampled perf event.
 */
struct perf_event_sample {
  uintptr_t ip;     // address of the sampled instruction
  uint32_t pid;     // process in which the event occurred
  uint32_t tid;     // thread in which the event occurred
  uintptr_t addr;   // memory address corresponding to the event, if applicable
  uint64_t nr;      // number of addresses in the callchain
  uintptr_t ips[];  // instruction pointers in the callchain for the event
};

using perf_event_signal_fn_t = void (*)(PerfEvent);
using perf_event_consume_fn_t = void (*)(PerfEvent, const perf_event_sample*);

/*
 * Enable sampling of load and store instructions on this thread.
 *
 * Perf events will be sampled roughly `sample_freq' times per second (with
 * loads and stores sampled separately).  On each sampled event, `signal_fn'
 * will be invoked in the context of the calling thread, via signal handler.
 *
 * Behavior is undefined if the SIGIO handler is reset, or if SIGIO is masked,
 * after calling this function.
 *
 * Returns true if sampling was successfully enabled, else false (both if
 * sampling has already been enabled, or if an error occurs).
 */
bool perf_event_enable(uint64_t sample_freq, perf_event_signal_fn_t signal_fn);

/*
 * Disable perf memory event sampling on this thread.
 *
 * All unconsumed samples will be lost, even if sampling is later reenabled.
 */
void perf_event_disable();

/*
 * Process and clear sampled perf events.
 *
 * This function can be safely called as long as sampling has been enabled for
 * the calling thread.  Each invocation of `signal_fn' indicates that that
 * there are events to be consumed---though consuming them from within
 * `signal_fn' is discouraged.
 *
 * Each sampled event is passed to `consume' exactly once.
 */
void perf_event_consume(perf_event_consume_fn_t consume);

/*
 * Pause or resume sampling for an event.
 *
 * Can be used instead of perf_event_{enable,disable}() for an event that has
 * already been opened, but which should be briefly turned off.
 *
 * This should likely be called in whatever routine is used to consume events,
 * to avoid reentrant sampling.
 */
void perf_event_pause();
void perf_event_resume();

///////////////////////////////////////////////////////////////////////////////

}

#endif
