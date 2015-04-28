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
  explicit TargetProfile(const TransContext& context,
                         BCMarker marker,
                         const StringData* name)
    : m_link(createLink(context, marker, name))
  {}

  /*
   * Access the data we collected during profiling.
   *
   * ReduceFn is used to fold the data from each local RDS slot.  It must have
   * the signature void(T&, const T&), and should assume the second argument
   * might be concurrently written to by other threads running in the
   * translation cache.
   *
   * Pre: optimizing()
   */
  template<class ReduceFn>
  T data(ReduceFn reduce) const {
    assertx(optimizing());
    auto const hand = handle();
    auto accum = T{};
    for (auto& base : rds::allTLBases()) {
      reduce(accum, rds::handleToRef<T>(base, hand));
    }
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
  rds::Link<T> link() {
    if (!m_link) m_link = createLink();
    return *m_link;
  }

  static rds::Link<T> createLink(const TransContext& context,
                                 BCMarker marker,
                                 const StringData* name) {
    switch (mcg->tx().mode()) {
    case TransKind::Profile:
      return rds::bind<T>(
        rds::Profile {
          context.transID,
          marker.bcOff(),
          name
        },
        rds::Mode::Local
      );

    case TransKind::Optimize:
      if (isValidTransID(marker.profTransID())) {
        return rds::attach<T>(
          rds::Profile {
            marker.profTransID(), // transID from profiling translation
            marker.bcOff(),
            name
          }
        );
      }
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

//////////////////////////////////////////////////////////////////////

/*
 * DecRefProfile is used to track which DecRef instructions are likely to go to
 * zero. During an optimized translation, the release path will be put in
 * acold if it rarely went to zero during profiling.
 */
struct DecRefProfile {
  uint16_t decrement;
  uint16_t destroy;

  int hitRate() const {
    return decrement ? destroy * 100 / decrement : 0;
  }

  std::string toString() const {
    return folly::format("decl: {:3}, destroy: {:3} ({:3}%)",
                         decrement, destroy, hitRate()).str();
  }

  static void reduce(DecRefProfile& a, const DecRefProfile& b) {
    // This is slightly racy but missing a few either way isn't a
    // disaster. It's already racy at profiling time because the two values
    // aren't updated atomically.
    a.decrement += b.decrement;
    a.destroy   += b.destroy;
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

}}

#endif
