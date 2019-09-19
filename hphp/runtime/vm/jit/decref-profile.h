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

#ifndef incl_HPHP_JIT_DECREF_PROFILE_H_
#define incl_HPHP_JIT_DECREF_PROFILE_H_

#include <folly/dynamic.h>
#include <folly/Format.h>
#include <folly/Optional.h>

#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/target-profile.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Profile the frequency of the 4 possible different behaviors of a DecRef
 * instruction.  Each execution of a DecRef must fall into exactly one of these
 * categories:
 *
 *  1) Uncounted:
 *     the type was uncounted
 *  2) Persistent:
 *     the type was refcounted and the value was persistent (so no dec happens)
 *  3) Destroyed:
 *     the type was refcounted and the value was destroyed (ie. the count was 1)
 *  4) Survived:
 *     the type was refcounted, non-persistent, but wasn't destroyed (count > 1)
 *
 */
struct DecRefProfile {

  uint16_t uncounted() const {
    return total - refcounted;
  }

  uint16_t persistent() const {
    return refcounted - released - decremented;
  }

  uint16_t destroyed() const {
    return released;
  }

  uint16_t survived() const {
    return decremented;
  }

  float percent(uint16_t value) const {
    return total ? 100.0 * value / total : 0.0;
  }

  std::string toString() const {
    return folly::sformat(
      "total: {:4}\n uncounted: {:4} ({:.1f}%),\n persistent: {:4} ({:.1f}%),\n"
      " destroyed: {:4} ({:.1f}%),\n survived: {:4} ({:.1f}%)",
      total,
      uncounted(),  percent(uncounted()),
      persistent(), percent(persistent()),
      destroyed(),  percent(destroyed()),
      survived(),   percent(survived())
    );
  }

  folly::dynamic toDynamic() const {
    return folly::dynamic::object("total", total)
                                 ("uncounted", uncounted())
                                 ("percentUncounted", percent(uncounted()))
                                 ("persistent", persistent())
                                 ("percentPersistent", percent(persistent()))
                                 ("destroyed", destroyed())
                                 ("percentDestroyed", percent(destroyed()))
                                 ("survived", survived())
                                 ("percentSurvived", percent(survived()))
                                 ("profileType", "DecRefProfile");
  }

  // overflow handling isn't statistically correct; but its better
  // than overflowing, and we're expecting threads to all have similar
  // distributions.
  static void reduce(DecRefProfile& a, const DecRefProfile& b) {
    auto const total = static_cast<uint32_t>(a.total + b.total);
    auto constexpr limit = std::numeric_limits<decltype(a.total)>::max();
    if (total > limit) {
      auto scale = [&] (uint16_t& x, uint64_t y) {
        x = ((x + y) * limit + total - 1) / total;
      };
      a.total = limit;
      scale(a.refcounted, b.refcounted);
      scale(a.released, b.released);
      scale(a.decremented, b.decremented);
    } else {
      a.total       = total;
      a.refcounted  += b.refcounted;
      a.released    += b.released;
      a.decremented += b.decremented;
    }
  }

  /*
   * The total number of times this DecRef was executed.
   */
  uint16_t total;
  /*
   * The number of times this DecRef made it at least as far as the static
   * check (meaning it was given a refcounted DataType).
   */
  uint16_t refcounted;
  /*
   * The number of times this DecRef went to zero and called the release method.
   */
  uint16_t released;
  /*
   * The number of times this DecRef actually decremented the count (meaning it
   * got a non-persistent, refcounted value with count > 1).
   */
  uint16_t decremented;
};

inline const StringData* decRefProfileKey(const IRInstruction* inst) {
  return makeStaticString(folly::to<std::string>(
                            "DecRefProfile-",
                            inst->extra<DecRefData>()->locId));
}

inline TargetProfile<DecRefProfile> decRefProfile(const IRUnit& unit,
                                                  const IRInstruction* inst) {
  auto const profileKey = decRefProfileKey(inst);
  return TargetProfile<DecRefProfile>(unit, inst->marker(), profileKey);
}

///////////////////////////////////////////////////////////////////////////////

} }

#endif
