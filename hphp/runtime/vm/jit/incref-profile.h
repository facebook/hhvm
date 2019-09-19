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

#ifndef incl_HPHP_JIT_INCREF_PROFILE_H_
#define incl_HPHP_JIT_INCREF_PROFILE_H_

#include <folly/dynamic.h>
#include <folly/Format.h>
#include <folly/Optional.h>

#include "hphp/runtime/vm/jit/target-profile.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Profile the frequency of the 3 possible different behaviors for an IncRef
 * instruction.  Each execution must fall into exactly one of these categories:
 *
 *  1) Uncounted:
 *     the type was uncounted
 *  2) Persistent:
 *     the type was refcounted and the value was persistent (so no inc happened)
 *  3) Incremented:
 *     the count for the value was incremented.
 *
 */
struct IncRefProfile {
  uint16_t uncounted() const {
    return total - refcounted;
  }

  uint16_t persistent() const {
    return refcounted - incremented;
  }

  float percent(uint16_t value) const {
    return total ? 100.0 * value / total : 0.0;
  }

  std::string toString() const {
    return folly::sformat(
      "total: {:4}\n uncounted: {:4} ({:.1f}%),\n persistent: {:4} ({:.1f}%),\n"
      " incremented: {:4} ({:.1f}%)",
      total,
      uncounted(),  percent(uncounted()),
      persistent(), percent(persistent()),
      incremented,  percent(incremented)
    );
  }

  folly::dynamic toDynamic() const {
    return folly::dynamic::object("total", total)
                                 ("uncounted", uncounted())
                                 ("percentUncounted", percent(uncounted()))
                                 ("persistent", persistent())
                                 ("percentPersistent", percent(persistent()))
                                 ("incremented", incremented)
                                 ("percentIncremented", percent(incremented))
                                 ("profileType", "IncRefProfile");
  }

  // overflow handling isn't statistically correct; but its better
  // than overflowing, and we're expecting threads to all have similar
  // distributions.
  static void reduce(IncRefProfile& a, const IncRefProfile& b) {
    auto const total = static_cast<uint32_t>(a.total + b.total);
    auto constexpr limit = std::numeric_limits<decltype(a.total)>::max();
    if (total > limit) {
      auto scale = [&] (uint16_t& x, uint64_t y) {
        x = ((x + y) * limit + total - 1) / total;
      };
      a.total = limit;
      scale(a.refcounted, b.refcounted);
      scale(a.incremented, b.incremented);
    } else {
      a.total       += b.total;
      a.refcounted  += b.refcounted;
      a.incremented += b.incremented;
    }
  }

  /*
   * The total number of times this IncRef was executed.
   */
  uint16_t total;
  /*
   * The number of times this IncRef made it past the refcounted check.
   */
  uint16_t refcounted;
  /*
   * The number of times this IncRef actually incremented the refcount.
   */
  uint16_t incremented;
};

///////////////////////////////////////////////////////////////////////////////

} }

#endif
