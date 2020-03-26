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

#ifndef incl_HPHP_JIT_TARGET_PROFILE_H_
#define incl_HPHP_JIT_TARGET_PROFILE_H_

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/types.h"

#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/print.h"
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

template<typename T>
auto call_reduce(T& out, const T& in, uint32_t size) ->
  decltype(T::reduce(out, in, size)) {
  return T::reduce(out, in, size);
}

template<typename T>
void call_reduce(T& out, const T& in, uint64_t size) {
  return T::reduce(out, in);
}

template<typename T>
auto call_tostring(const T& t, uint32_t size) ->
  decltype(t.toString(size)) {
  return t.toString(size);
}

template<typename T>
auto call_tostring(const T& t, uint64_t size) -> decltype(auto) {
  return t.toString();
}

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
 * MyType::toString([uint32_t]) should return a std::string with a
 * single human-readable line representing the state of the
 * profile. The optional parameter is the actual size allocated in
 * rds (only needed for variable size types).
 *
 * MyType::reduce(T& out, const T& in[, uint32_t]) should accumulate
 * the in into out, and should assume that another thread might be
 * writing to in while its reading it. The third, optional, parameter
 * is the actual size allocated in rds (see SwitchProfile for an
 * example of a variably sized profiler).
 *
 * MyType also needs to be added to the RDS_PROFILE_SYMBOLS macro.
 *
 * If the MyType contains pointers, or other data that needs updating
 * when serializing/deserializing, it should also define
 *
 *  void MyType::serialize(ProfDataSerializer& ser, const T& t) const;
 *  void MyType::deserialize(ProfDataDeserializer& ser, T& t);
 *
 * Note that custom serialization/deserialization is currently not
 * supported for variable sized profilers.
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
    , m_key{(T*)nullptr, profTransID, bcOff, name}
  {}

  TargetProfile(const IRUnit& unit,
                BCMarker marker,
                const StringData* name,
                size_t extraSize = 0)
    : TargetProfile(unit.context().kind == TransKind::Profile ?
                      unit.context().transID :
                      marker.profTransID(),
                    unit.context().kind,
                    marker.bcOff(),
                    name,
                    extraSize)
  {
    if (dumpIREnabled(unit.context().kind)) {
      auto const profile = rds::Profile(
        (T*)nullptr,
        m_key.transId,
        marker.bcOff(),
        name);
      unit.annotationData->profileKeys.push_back(profile);
    }
  }

  /*
   * Calls T::reduce to fold the data from each local RDS slot.
   *
   * Its the caller's responsibility to ensure that size is correct,
   * and that refers to a block of at least size bytes, which is
   * initially zeroed.
   */
  static void reduce(T& out, rds::Handle hand, uint32_t size) {
    for (auto& base : rds::allTLBases()) {
      detail::call_reduce(
        out, rds::handleToRef<T, rds::Mode::Local>(base, hand), size);
    }
  }

  /*
   * Access the data we collected during profiling.
   *
   * It calls reduce to populate out (see description above).
   *
   * Most callers want the second overload, for simplicity. This
   * version is just for variable-sized T (and has the same
   * assumptions about out and size as reduce).
   *
   * Pre: optimizing()
   */
  void data(T& out, uint32_t size) const {
    assertx(optimizing());
    reduce(out, handle(), size);
    if (RuntimeOption::EvalDumpTargetProfiles) {
      detail::addTargetProfileInfo(m_key, detail::call_tostring(out, size));
    }
  }

  T data(uint32_t size = sizeof(T)) const {
    auto accum = T{};
    data(accum, size);
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
  T& value() const { return *m_link; }

private:
  static rds::Link<T, rds::Mode::Local>
  createLink(TransID profTransID,
             TransKind kind,
             Offset bcOff,
             const StringData* name,
             size_t extraSize) {
    auto const rdsKey = rds::Profile((T*)nullptr, profTransID, bcOff, name);

    switch (kind) {
    case TransKind::Profile:
      return rds::bind<T, rds::Mode::Local>(rdsKey, extraSize);

    case TransKind::Optimize:
      if (isValidTransID(profTransID)) {
        return rds::attach<T, rds::Mode::Local>(rdsKey);
      }

      // fallthrough
    case TransKind::Anchor:
    case TransKind::Interp:
    case TransKind::Live:
    case TransKind::LivePrologue:
    case TransKind::ProfPrologue:
    case TransKind::OptPrologue:
    case TransKind::Invalid:
      return rds::Link<T, rds::Mode::Local>{};
    }
    not_reached();
  }

private:
  rds::Link<T, rds::Mode::Local> const m_link;
  TransKind const m_kind;
  rds::Profile const m_key;
};

///////////////////////////////////////////////////////////////////////////////

}}

#endif
