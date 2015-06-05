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
#ifndef incl_HPHP_TARGET_PROFILE_H_
#define incl_HPHP_TARGET_PROFILE_H_

#include <folly/Optional.h>

#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"

namespace HPHP {
struct Func;
struct Class;
}

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

/*
 * This is a utility for creating or querying a 'target profiling'
 * counter during JIT compilation.  The idea is similar to target
 * cache, except instead of caching the information, these collect
 * information that can be used to generate a smarter
 * TransKind::Optimize translation.
 *
 * To use one of these, define a type for the data you are going to
 * collect, and then create and query it in a JIT translation doing
 * something like this (example assumes you're in hhbc-translator):
 *
 *    TargetProfile<MyType> prof(m_context,
 *                               m_irb->state().marker(),
 *                               s_something);
 *
 *    if (prof.optimizing()) {
 *      auto const data = prof.data(MyType::reduce);
 *      // You can read here "data" to decide whether to generate
 *      // different IR based on the profiling information.
 *      return;
 *    }
 *
 *    // Normal translation
 *
 *    if (prof.profiling()) {
 *      gen(ProfMyTarget, RDSHandleData { prof.handle() }, ...);
 *    }
 *
 */
template<class T>
struct TargetProfile {
  TargetProfile(TransID profTransID,
                Offset bcOff,
                const StringData* name,
                size_t extraSize = 0)
    : m_link(createLink(profTransID, bcOff, name, extraSize))
  {}

  TargetProfile(const TransContext& context,
                BCMarker marker,
                const StringData* name,
                size_t extraSize = 0)
    : TargetProfile(profiling() ? context.transID : marker.profTransID(),
                    marker.bcOff(),
                    name,
                    extraSize)
  {}

  /*
   * Access the data we collected during profiling.
   *
   * ReduceFn is used to fold the data from each local RDS slot.  It must have
   * the signature void(T&, const T&, Args...), and should assume the second
   * argument might be concurrently written to by other threads running in the
   * translation cache. Any arguments passed to data() after reduce will be
   * forwarded to the reduce function.
   *
   * Most callers probably want the second overload, for simplicity. The
   * two-argument version is for variable-sized T, and the caller must ensure
   * that out is zero-initialized before calling data().
   *
   * Pre: optimizing()
   */
  template<class ReduceFn, class... Args>
  void data(T& out, ReduceFn reduce, Args&&... extraArgs) const {
    assertx(optimizing());
    auto const hand = handle();
    for (auto& base : rds::allTLBases()) {
      reduce(out, rds::handleToRef<T>(base, hand),
             std::forward<Args>(extraArgs)...);
    }
  }

  template<class ReduceFn, class... Args>
  T data(ReduceFn reduce, Args&&... extraArgs) const {
    auto accum = T{};
    data(accum, reduce, std::forward<Args>(extraArgs)...);
    return accum;
  }

  /*
   * Query whether this is set up to profile or optimize.  It's possible
   * neither is true (e.g. if we're producing a TransKind::Live translation or
   * we're producing a TransKind::Optimize translation and the link couldn't be
   * attached for some reason.).
   */
  bool profiling() const {
    return mcg->tx().mode() == TransKind::Profile;
  }
  bool optimizing() const {
    return mcg->tx().mode() == TransKind::Optimize && m_link.bound();
  }

  /*
   * Access the handle to the link.  You generally should only need to do this
   * if profiling().
   */
  rds::Handle handle() const { return m_link.handle(); }

private:
  static rds::Link<T> createLink(TransID profTransID,
                                 Offset bcOff,
                                 const StringData* name,
                                 size_t extraSize) {
    auto const rdsKey = rds::Profile{profTransID, bcOff, name};

    switch (mcg->tx().mode()) {
    case TransKind::Profile:
      return rds::bind<T>(rdsKey, rds::Mode::Local, extraSize);

    case TransKind::Optimize:
      if (isValidTransID(profTransID)) return rds::attach<T>(rdsKey);

      // fallthrough
    case TransKind::Anchor:
    case TransKind::Prologue:
    case TransKind::Interp:
    case TransKind::Live:
    case TransKind::Proflogue:
    case TransKind::Invalid:
      return rds::Link<T>(rds::kInvalidHandle);
    }
    not_reached();
  }

private:
  rds::Link<T> const m_link;
};

struct ClassProfile {
  static const size_t kClassProfileSampleSize = 4;

  const Class* sampledClasses[kClassProfileSampleSize];

  bool isMonomorphic() const {
    return size() == 1;
  }

  const Class* getClass(size_t i) const {
    if (i >= kClassProfileSampleSize) return nullptr;
    return sampledClasses[i];
  }

  size_t size() const {
    for (auto i = 0; i < kClassProfileSampleSize; ++i) {
      auto const cls = sampledClasses[i];
      if (!cls) return i;
    }
    return kClassProfileSampleSize;
  }

  void reportClass(const Class* cls) {
    for (auto& myCls : sampledClasses) {
      // If the current slot is empty, store the class here.
      if (!myCls) {
        myCls = cls;
        break;
      }

      // If the current slot matches the requested class, give up.
      if (cls == myCls) {
        break;
      }
    }
  }

  static void reduce(ClassProfile& a, const ClassProfile& b) {
    // Racy, but who cares?
    for (auto const cls : b.sampledClasses) {
      if (!cls) return;
      a.reportClass(cls);
    }
  }
};

//////////////////////////////////////////////////////////////////////

struct IncRefProfile {
  /* The number of times this IncRef made it at least as far as the static
   * check (meaning it was given a refcounted DataType. */
  uint16_t tryinc;

  std::string toString() const {
    return folly::sformat("tryinc: {:4}", tryinc);
  }

  static void reduce(IncRefProfile& a, const IncRefProfile& b) {
    a.tryinc += b.tryinc;
  }
};

/*
 * DecRefProfile is used to track which types go through DecRef instructions,
 * and which ones arelikely go to zero.
 */
struct DecRefProfile {
  /* The number of times this DecRef was executed. */
  uint16_t hits;

  /* The number of times this DecRef made it at least as far as the static
   * check (meaning it was given a refcounted DataType. */
  uint16_t trydec;

  /* The number of times this DecRef went to zero and called destroy(). */
  uint16_t destroy;

  float destroyRate() const {
    return hits ? float(destroy) / hits : 0.0;
  }

  std::string toString() const {
    return folly::sformat("hits: {:4} trydec: {:4}, destroy: {:4} ({:.2%}%)",
                          hits, trydec, destroy, destroyRate());
  }

  static void reduce(DecRefProfile& a, const DecRefProfile& b) {
    a.hits    += b.hits;
    a.trydec  += b.trydec;
    a.destroy += b.destroy;
  }
};
typedef folly::Optional<TargetProfile<DecRefProfile>> OptDecRefProfile;

//////////////////////////////////////////////////////////////////////

/*
 * Record profiling information about non-packed arrays. This counts the
 * number of times a non-packed array was used as the base of a CGetElem
 * operation.
 */
struct NonPackedArrayProfile {
  int32_t count;
  static void reduce(NonPackedArrayProfile& a, const NonPackedArrayProfile& b) {
    a.count += b.count;
  }
};

struct StructArrayProfile {
  int32_t nonStructCount;
  int32_t numShapesSeen;
  Shape* shape{nullptr}; // Never access this directly. Use getShape instead.

  bool isEmpty() const {
    return !numShapesSeen;
  }

  bool isMonomorphic() const {
    return numShapesSeen == 1;
  }

  bool isPolymorphic() const {
    return numShapesSeen > 1;
  }

  void makePolymorphic() {
    numShapesSeen = INT_MAX;
    shape = nullptr;
  }

  Shape* getShape() const {
    assertx(isMonomorphic());
    return shape;
  }

  static void reduce(StructArrayProfile& a, const StructArrayProfile& b) {
    a.nonStructCount += b.nonStructCount;
    if (a.isPolymorphic()) return;

    if (a.isEmpty()) {
      a.shape = b.shape;
      a.numShapesSeen = b.numShapesSeen;
      return;
    }

    assertx(a.isMonomorphic());
    if (b.isEmpty()) return;
    if (b.isMonomorphic() && a.getShape() == b.getShape()) return;
    a.makePolymorphic();
    return;
  }
};

//////////////////////////////////////////////////////////////////////

struct ReleaseVVProfile {
  uint16_t executed;
  uint16_t released;

  int percentReleased() const {
    return executed ? (100 * released / executed) : 0;
  };

  static void reduce(ReleaseVVProfile& a, const ReleaseVVProfile& b) {
    // Racy but OK -- just used for profiling to trigger optimization.
    a.executed += b.executed;
    a.released += b.released;
  }
};

//////////////////////////////////////////////////////////////////////

struct SwitchProfile {
  SwitchProfile(const SwitchProfile&) = delete;
  SwitchProfile& operator=(const SwitchProfile&) = delete;

  uint32_t cases[0]; // dynamically sized

  static void reduce(SwitchProfile& a, const SwitchProfile& b, int nCases) {
    for (uint32_t i = 0; i < nCases; ++i) {
      a.cases[i] += b.cases[i];
    }
  }
};

struct SwitchCaseCount {
  int32_t caseIdx;
  uint32_t count;

  bool operator<(const SwitchCaseCount& b) const { return count > b.count; }
};

/*
 * Collect the data for the given SwitchProfile, and return a vector of case
 * indexes and hit count, sorted in descending order of hit count.
 */
std::vector<SwitchCaseCount> sortedSwitchProfile(
  TargetProfile<SwitchProfile>& profile,
  int32_t nCases
);

}}

#endif
