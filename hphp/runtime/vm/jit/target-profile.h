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

#ifndef incl_HPHP_JIT_TARGET_PROFILE_H_
#define incl_HPHP_JIT_TARGET_PROFILE_H_

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/types.h"

#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/assertions.h"

#include <utility>

namespace HPHP {

struct Func;
struct Class;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace detail {
  void addTargetProfileInfo(const rds::Profile& key,
                            const std::string& dbgInfo);
}

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
 * The type must have a toString(...) method returning a std::string with a
 * single human-readable line representing the state of the profile, taking
 * the same set of extra arguments as the reduce function passed to 'data'.
 */
template<class T>
struct TargetProfile {
  TargetProfile(TransID profTransID,
                TransKind kind,
                Offset bcOff,
                const StringData* name,
                size_t extraSize = 0)
    : m_link(createLink(profTransID, kind, bcOff, name, extraSize))
    , m_kind(kind)
    , m_key{profTransID, bcOff, name}
  {}

  TargetProfile(const TransContext& context,
                BCMarker marker,
                const StringData* name,
                size_t extraSize = 0)
    : TargetProfile(context.kind == TransKind::Profile ? context.transID
                                                       : marker.profTransID(),
                    context.kind,
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
    if (RuntimeOption::EvalDumpTargetProfiles) {
      detail::addTargetProfileInfo(
        m_key,
        out.toString(std::forward<Args>(extraArgs)...)
      );
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
    return m_kind == TransKind::Profile;
  }
  bool optimizing() const {
    return m_kind == TransKind::Optimize && m_link.bound();
  }

  /*
   * Access the handle to the link.  You generally should only need to do this
   * if profiling().
   */
  rds::Handle handle() const { return m_link.handle(); }

private:
  static rds::Link<T> createLink(TransID profTransID,
                                 TransKind kind,
                                 Offset bcOff,
                                 const StringData* name,
                                 size_t extraSize) {
    auto const rdsKey = rds::Profile{profTransID, bcOff, name};

    switch (kind) {
    case TransKind::Profile:
      return rds::bind<T>(rdsKey, rds::Mode::Local, extraSize);

    case TransKind::Optimize:
      if (isValidTransID(profTransID)) return rds::attach<T>(rdsKey);

      // fallthrough
    case TransKind::Anchor:
    case TransKind::Interp:
    case TransKind::Live:
    case TransKind::LivePrologue:
    case TransKind::ProfPrologue:
    case TransKind::OptPrologue:
    case TransKind::Invalid:
      return rds::Link<T>(rds::kInvalidHandle);
    }
    not_reached();
  }

  static void addDebugInfo(const rds::Profile& key,
                           const std::string& dbgInfo);

private:
  rds::Link<T> const m_link;
  TransKind const m_kind;
  rds::Profile const m_key;
};

///////////////////////////////////////////////////////////////////////////////

}}

#endif
