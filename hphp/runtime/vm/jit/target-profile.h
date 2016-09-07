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

#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/rds.h"

#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/util/type-scan.h"

#include <folly/Optional.h>

namespace HPHP {
struct Func;
struct Class;
}

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

void addTargetProfileInfo(const rds::Profile& key, const std::string& dbgInfo);

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
      addTargetProfileInfo(m_key,
                           out.toString(std::forward<Args>(extraArgs)...));
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

private:
  rds::Link<T> const m_link;
  TransKind const m_kind;
  rds::Profile const m_key;
};

//////////////////////////////////////////////////////////////////////

struct MethProfile {
  using RawType = LowPtr<Class>::storage_type;

  enum class Tag {
    UniqueClass = 0,
    UniqueMeth = 1,
    BaseMeth = 2,
    InterfaceMeth = 3,
    Invalid = 4
  };

  MethProfile() : m_curMeth(nullptr), m_curClass(nullptr) {}
  MethProfile(const MethProfile& other) :
      m_curMeth(other.m_curMeth),
      m_curClass(other.m_curClass) {}

  std::string toString() const;

  const Class* uniqueClass() const {
    return curTag() == Tag::UniqueClass ? rawClass() : nullptr;
  }

  const Func* uniqueMeth() const {
    return curTag() == Tag::UniqueMeth || curTag() == Tag::UniqueClass ?
      rawMeth() : nullptr;
  }

  const Func* baseMeth() const {
    return curTag() == Tag::BaseMeth ? rawMeth() : nullptr;
  }

  const Func* interfaceMeth() const {
    return curTag() == Tag::InterfaceMeth ? rawMeth() : nullptr;
  }

  void reportMeth(const ActRec* ar, const Class* cls) {
    auto const meth = ar->func();
    if (!cls && meth->cls()) {
      cls = ar->hasThis() ?
        ar->getThis()->getVMClass() : ar->getClass();
    }
    reportMethHelper(cls, meth);
  }

  static void reduce(MethProfile& a, const MethProfile& b);
 private:
  void reportMethHelper(const Class* cls, const Func* meth);

  static Tag toTag(uintptr_t val) {
    return static_cast<Tag>(val & 7);
  }

  static const Func* fromValue(uintptr_t value) {
    return (Func*)(value & uintptr_t(-8));
  }

  Tag curTag() const { return toTag(methValue()); }

  const Class* rawClass() const {
    return m_curClass;
  }

  const Func* rawMeth() const {
    return fromValue(methValue());
  }

  const uintptr_t methValue() const {
    return uintptr_t(m_curMeth.get());
  }

  void setMeth(const Func* meth, Tag tag) {
    auto encoded_meth = (Func*)(uintptr_t(meth) | static_cast<uintptr_t>(tag));
    m_curMeth = encoded_meth;
  }

  AtomicLowPtr<const Func,
               std::memory_order_acquire, std::memory_order_release> m_curMeth;
  AtomicLowPtr<const Class,
               std::memory_order_acquire, std::memory_order_release> m_curClass;
};

//////////////////////////////////////////////////////////////////////

/*
 * ArrayKindProfile profiles the distribution of the array kinds
 * observed for a given value.  The array kinds currently tracked are
 * Empty, Packed, and Mixed.
 */
struct ArrayKindProfile {

  static const uint32_t kNumProfiledArrayKinds = 4;

  std::string toString() const {
    std::ostringstream out;
    for (auto c : count) out << folly::format("{},", c);
    return out.str();
  }

  static void reduce(ArrayKindProfile& a, const ArrayKindProfile& b) {
    for (uint32_t i = 0; i < kNumProfiledArrayKinds; i++) {
      a.count[i] += b.count[i];
    }
  }

  void report(ArrayData::ArrayKind kind);

  /*
   * Returns what fraction of the total profiled arrays had the given `kind'.
   */
  double fraction(ArrayData::ArrayKind kind) const;

  /*
   * Returns the total number of samples profiled so far.
   */
  uint32_t total() const {
    uint32_t sum = 0;
    for (uint32_t i = 0; i < kNumProfiledArrayKinds; i++) {
      sum += count[i];
    }
    return sum;
  }

  uint32_t count[kNumProfiledArrayKinds];
};

//////////////////////////////////////////////////////////////////////

/*
 * TypeProfile keeps the union of all the types observed during profiling.
 */
struct TypeProfile {
  Type type; // this gets initialized with 0, which is TBottom
  static_assert(Type::Bits::kBottom == 0, "Assuming TBottom is 0");

  std::string toString() const { return type.toString(); }

  void report(TypedValue tv) {
    type |= typeFromTV(&tv, nullptr);
  }

  static void reduce(TypeProfile& a, const TypeProfile& b) {
    a.type |= b.type;
  }

  // In RDS but can't contain pointers to request-allocated data
  TYPE_SCAN_IGNORE_ALL;
};

//////////////////////////////////////////////////////////////////////

struct SwitchProfile {
  SwitchProfile(const SwitchProfile&) = delete;
  SwitchProfile& operator=(const SwitchProfile&) = delete;

  std::string toString(int nCases) const {
    std::ostringstream out;
    for (int i = 0; i < nCases; ++i) out << folly::format("{},", cases[i]);
    return out.str();
  }

  uint32_t cases[0]; // dynamically sized

  static void reduce(SwitchProfile& a, const SwitchProfile& b, int nCases) {
    for (uint32_t i = 0; i < nCases; ++i) {
      a.cases[i] += b.cases[i];
    }
  }

  // In RDS but can't contain pointers to request-allocated data
  TYPE_SCAN_IGNORE_ALL;
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

//////////////////////////////////////////////////////////////////////

}}

#endif
