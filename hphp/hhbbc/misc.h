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
#ifndef incl_HHBBC_MISC_H_
#define incl_HHBBC_MISC_H_

#include <chrono>
#include <cassert>

#include <boost/variant.hpp>
#include <memory>

#include "hphp/util/trace.h"
#include "hphp/util/match.h"
#include "hphp/util/low-ptr.h"

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

/*
 * HHBC evaluation stack flavors.
 */
enum class Flavor { C, V, R, U, CR, CU, CV, CVU };

/*
 * Types of parameter preparation (or unknown).
 */
enum class PrepKind { Ref, Val, Unknown };

using LocalId = uint32_t;
constexpr const LocalId NoLocalId = -1;
/*
 * Special value used by StackElem::equivLoc to indicate that
 * this element is a dup of the one below.
 */
constexpr const LocalId StackDupId = -2;
constexpr const LocalId MaxLocalId = StackDupId - 1;

using IterId = uint32_t;
using ClsRefSlotId = uint32_t;
constexpr const ClsRefSlotId NoClsRefSlotId = -1;
using BlockId = uint32_t;
constexpr const BlockId NoBlockId = -1;

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

struct trace_time {
  using clock      = std::chrono::high_resolution_clock;
  using time_point = clock::time_point;

  explicit trace_time(const char* what,
                      const std::string& extra = std::string{})
    : what(what)
    , start(clock::now())
  {
    if (Trace::moduleEnabledRelease(Trace::hhbbc_time, 1)) {
      Trace::traceRelease(
        "%s",
        folly::format("{}: start{}\n",
          what,
          !extra.empty() ? folly::format(" ({})", extra).str() : extra
        ).str().c_str()
      );
    }
  }

  ~trace_time() {
    namespace C = std::chrono;
    DEBUG_ONLY auto const elapsed = C::duration_cast<C::milliseconds>(
      clock::now() - start
    );
    if (Trace::moduleEnabledRelease(Trace::hhbbc_time, 1)) {
      Trace::traceRelease(
        "%s",
        folly::format("{}: {}ms elapsed\n", what, elapsed.count())
          .str().c_str()
      );
    }
  }

  trace_time(const trace_time&) = delete;
  trace_time& operator=(const trace_time&) = delete;

private:
  const char* what;
  time_point start;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
