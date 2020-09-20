/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <string>
#include <vector>

#define JIT_TIMERS                              \
  TIMER_NAME(analyze)                           \
  TIMER_NAME(collectPostConditions)             \
  TIMER_NAME(hhir_lower)                        \
  TIMER_NAME(optimize)                          \
  TIMER_NAME(optimize_dce)                      \
  TIMER_NAME(optimize_cleancfg)                 \
  TIMER_NAME(optimize_predictionOpts)           \
  TIMER_NAME(optimize_realxGuards)              \
  TIMER_NAME(optimize_refcountOpts)             \
  TIMER_NAME(optimize_reoptimize)               \
  TIMER_NAME(optimize_loads)                    \
  TIMER_NAME(optimize_stores)                   \
  TIMER_NAME(optimize_gvn)                      \
  TIMER_NAME(optimize_phis)                     \
  TIMER_NAME(optimize_licm)                     \
  TIMER_NAME(hoist_type_checks)                 \
  TIMER_NAME(regionizeFunc)                     \
  TIMER_NAME(selectTracelet)                    \
  TIMER_NAME(mcg_translate)                     \
  TIMER_NAME(mcg_finishTranslation)             \
  TIMER_NAME(mcg_finishTranslation_metadata)    \
  TIMER_NAME(irGenRegion)                       \
  TIMER_NAME(irGenRegionAttempt)                \
  TIMER_NAME(translateTracelet)                 \
  TIMER_NAME(translateTracelet_irGeneration)    \
  TIMER_NAME(vasm_layout)                       \
  TIMER_NAME(vasm_reg_alloc)                    \
  TIMER_NAME(vasm_reg_alloc_spill)              \
  TIMER_NAME(vasm_jumps)                        \
  TIMER_NAME(vasm_bind_ptrs)                    \
  TIMER_NAME(vasm_emit)                         \
  TIMER_NAME(vasm_lower)                        \
  TIMER_NAME(vasm_copy)                         \
  TIMER_NAME(vasm_optimize)                     \
  TIMER_NAME(vasm_dce)                          \
  TIMER_NAME(vasm_sf_peepholes)                 \

namespace HPHP {
struct StructuredLogEntry;
namespace jit {

/*
 * Timer is used to track how much CPU time we spend in the different stages of
 * the jit. Typical usage starts and stops timing with construction/destruction
 * of the object, respectively. The stop() function may be called to stop
 * timing early, in case it's not reasonable to add a new scope just for
 * timing.
 *
 * There are no rules about the values in the Name enum, though by convention
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
    int64_t max;   // longest CPU time, in nanoseconds
    int64_t wall_time_elapsed;   // Wall clock elapsed, in micros

    int64_t mean() const {
      return total / count;
    }
  };

  explicit Timer(Name name, StructuredLogEntry* = nullptr);
  ~Timer();

  /*
   * Stop the timer, and return the elapsed time in nanoseconds.
   */
  int64_t stop();

  typedef std::vector<std::pair<const char*, Counter>> CounterVec;
  static CounterVec Counters();
  static Counter CounterValue(Name name);
  static void RequestInit();
  static void RequestExit();
  static void Dump();
  static std::string Show();

 private:
  Name m_name;
  bool m_finished;
  int64_t m_start;
  int64_t m_start_wall;
  StructuredLogEntry* m_log_entry;
};

} }

