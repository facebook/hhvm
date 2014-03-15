/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_RUNTIME_VM_JIT_TIMER_H_
#define incl_HPHP_RUNTIME_VM_JIT_TIMER_H_

#include <string>
#include <unordered_map>
#include <vector>

#define JIT_TIMERS                              \
  TIMER_NAME(analyze)                           \
  TIMER_NAME(codeGen)                           \
  TIMER_NAME(optimize)                          \
  TIMER_NAME(optimize_dce)                      \
  TIMER_NAME(optimize_jumpOpts)                 \
  TIMER_NAME(optimize_predictionOpts)           \
  TIMER_NAME(optimize_realxGuards)              \
  TIMER_NAME(optimize_refcountOpts)             \
  TIMER_NAME(optimize_relaxGuards)              \
  TIMER_NAME(optimize_reoptimize)               \
  TIMER_NAME(regalloc)                          \
  TIMER_NAME(regionizeFunc)                     \
  TIMER_NAME(selectTracelet)                    \
  TIMER_NAME(selectTracelet_relaxGuards)        \
  TIMER_NAME(translate)                         \
  TIMER_NAME(translateRegion)                   \
  TIMER_NAME(translateRegion_irGeneration)      \
  TIMER_NAME(translateTracelet)                 \
  TIMER_NAME(translateTracelet_irGeneration)    \

namespace HPHP { namespace JIT {

/*
 * Timer is used to track how much CPU time we spend in the different stages of
 * the jit. Typical usage starts and stops timing with construction/destruction
 * of the object, respectively. The end() function may be called to stop timing
 * early, in case it's not reasonable to add a new scope just for timing.
 *
 * The name given to the constructor may be any string, though by convention
 * any components are separated with underscores. For example, we use Timers
 * for optimize, optimize_dce, optimize_jumpOpts, and others.
 *
 * The counters are thread-local and are automatically cleared at the end of
 * each request. Timer::Counters() may be used to access the raw data, or
 * Show() may be used to get a text representation of the current counters.
 */
struct Timer {
  enum Name : uint8_t {
#   define TIMER_NAME(name) name,
    JIT_TIMERS
#   undef TIMER_NAME
  };

# define TIMER_NAME(name) + 1
  static size_t constexpr kNumTimers = JIT_TIMERS;
# undef TIMER_NAME

  struct Counter {
    int64_t total; // total CPU time, in nanoseconds
    int64_t count; // number of entries for this counter

    int64_t mean() const {
      return total / count;
    }
  };

  explicit Timer(Name name);
  ~Timer();

  void end();

  typedef std::vector<std::pair<const char*, Counter>> CounterVec;
  static CounterVec Counters();
  static void RequestInit();
  static void RequestExit();
  static void Dump();
  static std::string Show();

 private:
  Name m_name;
  int64_t m_start;
  bool m_finished;
};

} }

#endif
