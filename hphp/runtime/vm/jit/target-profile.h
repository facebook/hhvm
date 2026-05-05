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

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/types.h"

#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
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
  TargetProfile(const TransIDSet& profTransIDs,
                TransKind kind,
                SrcKey sk,
                const StringData* name,
                size_t extraSize = 0)
    : TargetProfile(profTransIDs, kind, bcOffForProfileKey(sk), name, extraSize)
  {}

  TargetProfile(const TransContext& context,
                const BCMarker& marker,
                const StringData* name,
                size_t extraSize = 0)
    : TargetProfile(isProfiling(context.kind)
                      ? context.transIDs : marker.profTransIDs(),
                    context.kind,
                    marker.sk(),
                    name,
                    extraSize)
  {}

  static TargetProfile deserialize(const TransIDSet& profTransIDs,
                                   TransKind kind,
                                   Offset bcOff,
                                   const StringData* name,
                                   size_t extraSize) {
    return TargetProfile{profTransIDs, kind, bcOff, name, extraSize};
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
    auto s = profDataTargetProfile();
    if (s) {
      for (auto const& key : m_keys) {
        if (auto const v = s->get<T>(key)) {
          detail::call_reduce(out, *v, size);
        }
      }
    } else {
      for (auto const& link : getLinks()) {
        if (link.bound()) {
          reduce(out, link.handle(), size);
        }
      }
    }
    if (Cfg::Eval::DumpTargetProfiles) {
      for (auto const& key : m_keys) {
        detail::addTargetProfileInfo(key, detail::call_tostring(out, size));
      }
    }
  }

  T data() const {
    auto accum = T{};
    data(accum, sizeof(T));
    return accum;
  }

  /*
   * Query whether this is set up to profile or optimize.  It's possible
   * neither is true (e.g. if we're producing a TransKind::Live translation or
   * we're producing a TransKind::Optimize translation and the link couldn't be
   * attached for some reason.).
   */
  bool profiling() const {
    return isProfiling(m_kind);
  }

  bool optimizing() const {
    if (!isOptimized(m_kind)) return false;
    auto s = profDataTargetProfile();
    if (s) {
      for (auto const& key : m_keys) {
        if (s->get<T>(key)) return true;
      }
    } else {
      for (auto const& link : getLinks()) {
        if (link.bound()) return true;
      }
    }
    return false;
  }

  /*
   * Access the handle to the link.  You can only do this if profiling().
   */
  rds::Handle handle() const {
    assertx(profiling());
    assertx(getLinks().size() == 1);
    return getLinks().front().handle();
  }
  T& value() const {
    assertx(profiling());
    assertx(getLinks().size() == 1);
    return *getLinks().front();
  }

private:
  TargetProfile(const TransIDSet& profTransIDs,
                TransKind kind,
                Offset bcOff,
                const StringData* name,
                size_t extraSize)
    : m_kind(kind)
    , m_keys(createKeys(profTransIDs, bcOff, name))
    , m_extraSize(extraSize)
  {}

  static constexpr Offset kPrologueOffset = -1;
  static constexpr Offset kFuncEntryOffset = -2;
  static constexpr Offset kNamedParamsFuncEntryOffset = -3;

  static Offset bcOffForProfileKey(SrcKey sk) {
    // Use a placeholder value for prologues and function entries, as they
    // are not part of a bytecode at any offset. Profiling of prologues for
    // different number of arguments is still differentiated by profTransID.
    if (sk.prologue()) return kPrologueOffset;
    if (sk.namedParamsFuncEntry()) return kNamedParamsFuncEntryOffset;
    if (sk.funcEntry()) return kFuncEntryOffset;
    return sk.offset();
  }

  static rds::Link<T, rds::Mode::Local>
  createLink(TransID profTransID,
             TransKind kind,
             Offset bcOff,
             const StringData* name,
             size_t extraSize) {
    auto const rdsKey = rds::Profile((T*)nullptr, profTransID, bcOff, name);

    switch (kind) {
    case TransKind::Profile:
    case TransKind::ProfPrologue:
      return rds::bind<T, rds::Mode::Local>(rdsKey, extraSize);

    case TransKind::Optimize:
    case TransKind::OptPrologue:
      if (isValidTransID(profTransID)) {
        return rds::attach<T, rds::Mode::Local>(rdsKey);
      }
      [[fallthrough]];

    case TransKind::Anchor:
    case TransKind::Interp:
    case TransKind::Live:
    case TransKind::LivePrologue:
    case TransKind::Invalid:
      return rds::Link<T, rds::Mode::Local>{};
    }
    not_reached();
  }

  static jit::vector<rds::Link<T, rds::Mode::Local>>
  createLinks(const jit::vector<rds::Profile>& keys,
              TransKind kind,
              size_t extraSize) {
    assertx(!keys.empty());
    jit::vector<rds::Link<T, rds::Mode::Local>> links;
    links.reserve(keys.size());
    for (auto key : keys) {
      links.push_back(createLink(key.transId, kind, key.bcOff, key.name, extraSize));
    }
    return links;
  }

  static jit::vector<rds::Profile>
  createKeys(const TransIDSet& profTransIDs,
             Offset bcOff,
             const StringData* name) {
    auto const size = profTransIDs.size();
    // NB: size can be zero during tracelet formation. In this case, create a
    // dummy key corresponding to kInvalidTransID.
    if (size == 0) {
      jit::vector<rds::Profile> keys;
      keys.push_back({(T*)nullptr, kInvalidTransID, bcOff, name});
      return keys;
    }
    jit::vector<rds::Profile> keys;
    keys.reserve(size);
    for (auto tid : profTransIDs) {
      keys.push_back({(T*)nullptr, tid, bcOff, name});
    }
    return keys;
  }

  jit::vector<rds::Link<T, rds::Mode::Local>> getLinks() const {
    if (!m_links.empty()) {
      return m_links;
    }

    m_links = createLinks(m_keys, m_kind, m_extraSize);
    return m_links;
  }

private:
  const TransKind m_kind;
  mutable jit::vector<rds::Link<T, rds::Mode::Local>> m_links = {};
  const jit::vector<rds::Profile> m_keys;
  const uint32_t m_extraSize;
};

///////////////////////////////////////////////////////////////////////////////

}}
