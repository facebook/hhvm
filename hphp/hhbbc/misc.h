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

#include <chrono>
#include <cassert>

#include <boost/variant.hpp>
#include <memory>

#include "hphp/util/compact-vector.h"
#include "hphp/util/low-ptr.h"
#include "hphp/util/match.h"
#include "hphp/util/trace.h"

namespace HPHP {
struct StringData;
struct ArrayData;
}

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * String that must be a static string, and Array that must be a
 * static array.
 */
using SString  = const StringData*;
using LSString = LowPtr<const StringData>;
using SArray   = const ArrayData*;

struct Bytecode;
using BytecodeVec = CompactVector<Bytecode>;

/*
 * HHBC evaluation stack flavors.
 */
enum class Flavor { C, U, CU };

/*
 * Types of parameter preparation (or unknown).
 */
enum class PrepKind { InOut, Val, Unknown };

using LocalId = uint32_t;
constexpr const LocalId NoLocalId = -1;
/*
 * Special value used by StackElem::equivLoc to indicate that
 * this element is a dup of the one below.
 */
constexpr const LocalId StackDupId = -2;
constexpr const LocalId StackThisId = -3;
constexpr const LocalId MaxLocalId = StackThisId - 1;

using ClosureId = uint32_t;
using IterId = uint32_t;
using BlockId = uint32_t;
constexpr const BlockId NoBlockId = -1;
using ExnNodeId = uint32_t;
constexpr const ExnNodeId NoExnNodeId = -1;

//////////////////////////////////////////////////////////////////////

/*
 * Many places in the code want to bump tracing levels by some amount
 * for systemlib-related processing.  This is the amount they all bump
 * by.
 */
constexpr int kSystemLibBump = 10;

/*
 * Functions listed in the --trace functions list get trace level bumped by
 * this amount.
 */
constexpr int kTraceFuncBump = -10;

/*
 * We may run the interpreter collecting stats and when trace is on
 * the amount of noise is unbearable. This is to keep tracing out
 * of stats collection.
 */
constexpr int kStatsBump = 50;

//////////////////////////////////////////////////////////////////////

void profile_memory(const char* what, const char* when, const std::string&);
void summarize_memory();

struct trace_time {
  using clock      = std::chrono::system_clock;
  using time_point = clock::time_point;

  explicit trace_time(const char* what,
                      const std::string& extra = std::string{})
    : what(what)
    , start(clock::now())
    , extra(extra)
  {
    profile_memory(what, "start", extra);
    if (Trace::moduleEnabledRelease(Trace::hhbbc_time, 1)) {
      Trace::traceRelease(
        "%s",
        folly::sformat(
          "{}: {}: start{}\n",
          ts(start),
          what,
          !extra.empty() ? folly::format(" ({})", extra).str() : extra
        ).c_str()
      );
    }
  }

  ~trace_time() {
    namespace C = std::chrono;
    auto const end = clock::now();
    auto const elapsed = C::duration_cast<C::milliseconds>(
      end - start
    );
    profile_memory(what, "end", extra);
    if (Trace::moduleEnabledRelease(Trace::hhbbc_time, 1)) {
      Trace::traceRelease(
        "%s",
        folly::sformat(
          "{}: {}: {}ms elapsed\n",
          ts(end), what, elapsed.count())
          .c_str()
      );
    }
  }

  trace_time(const trace_time&) = delete;
  trace_time& operator=(const trace_time&) = delete;

private:
  std::string ts(time_point t) {
    char snow[64];
    auto tm = std::chrono::system_clock::to_time_t(t);
    ctime_r(&tm, snow);
    // Eliminate trailing newline from ctime_r.
    snow[24] = '\0';
    return snow;
  }
  const char* what;
  time_point start;
  std::string extra;
};

//////////////////////////////////////////////////////////////////////

}}

