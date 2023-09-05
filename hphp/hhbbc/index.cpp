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
#include "hphp/hhbbc/index.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_unordered_map.h>

#include <folly/Format.h>
#include <folly/Hash.h>
#include <folly/Lazy.h>
#include <folly/MapUtil.h>
#include <folly/Memory.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <folly/concurrency/ConcurrentHashMap.h>

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"

#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/trait-method-import-data.h"
#include "hphp/runtime/vm/unit-util.h"

#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/context.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/options.h"
#include "hphp/hhbbc/options-util.h"
#include "hphp/hhbbc/parallel.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-builtins.h"
#include "hphp/hhbbc/type-structure.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/wide-func.h"

#include "hphp/util/assertions.h"
#include "hphp/util/check-size.h"
#include "hphp/util/hash-set.h"
#include "hphp/util/lock-free-lazy.h"
#include "hphp/util/match.h"

#include "hphp/zend/zend-string.h"

namespace HPHP {
namespace HHBBC {

TRACE_SET_MOD(hhbbc_index);

//////////////////////////////////////////////////////////////////////

using namespace extern_worker;

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_construct("__construct");
const StaticString s_toBoolean("__toBoolean");
const StaticString s_invoke("__invoke");
const StaticString s_Closure("Closure");
const StaticString s_AsyncGenerator("HH\\AsyncGenerator");
const StaticString s_Generator("Generator");
const StaticString s_Awaitable("HH\\Awaitable");

//////////////////////////////////////////////////////////////////////

// HHBBC consumes a LOT of memory, so we keep representation types small.
static_assert(CheckSize<php::Block, 24>(), "");
static_assert(CheckSize<php::Local, use_lowptr ? 12 : 16>(), "");
static_assert(CheckSize<php::Param, use_lowptr ? 64 : 96>(), "");
static_assert(CheckSize<php::Func, use_lowptr ? 184 : 232>(), "");

// Likewise, we also keep the bytecode and immediate types small.
static_assert(CheckSize<Bytecode, use_lowptr ? 32 : 40>(), "");
static_assert(CheckSize<MKey, 16>(), "");
static_assert(CheckSize<IterArgs, 16>(), "");
static_assert(CheckSize<FCallArgs, 8>(), "");
static_assert(CheckSize<RepoAuthType, 8>(), "");

//////////////////////////////////////////////////////////////////////

template<typename T> using UniquePtrRef = Ref<std::unique_ptr<T>>;

//////////////////////////////////////////////////////////////////////

Dep operator|(Dep a, Dep b) {
  return static_cast<Dep>(
    static_cast<uintptr_t>(a) | static_cast<uintptr_t>(b)
  );
}

bool has_dep(Dep m, Dep t) {
  return static_cast<uintptr_t>(m) & static_cast<uintptr_t>(t);
}

/*
 * Maps functions to contexts that depend on information about that
 * function, with information about the type of dependency.
 */
using DepMap =
  tbb::concurrent_hash_map<
    DependencyContext,
    hphp_fast_map<
      DependencyContext,
      Dep,
      DependencyContextHash,
      DependencyContextEquals
    >,
    DependencyContextHashCompare
  >;

//////////////////////////////////////////////////////////////////////

/*
 * Each ClassInfo has a table of public static properties with these entries.
 */
struct PublicSPropEntry {
  Type inferredType{TInitCell};
  uint32_t refinements{0};
  bool everModified{true};
};

//////////////////////////////////////////////////////////////////////

/*
 * Represents a method, without requiring an explicit pointer to a
 * php::Func (so can be used across remote workers).
 */
struct MethRef {
  MethRef() = default;
  explicit MethRef(const php::Func& f)
    : cls{f.cls->name}, idx{f.clsIdx} {}
  MethRef(SString cls, uint32_t idx)
    : cls{cls}, idx{idx} {}

  SString cls{nullptr};
  // Index in the class' methods table.
  uint32_t idx{std::numeric_limits<uint32_t>::max()};

  bool operator==(const MethRef& o) const {
    return cls->isame(o.cls) && idx == o.idx;
  }
  bool operator!=(const MethRef& o) const {
    return !(*this == o);
  }
  bool operator<(const MethRef& o) const {
    // The ordering for MethRef is arbitrary. Compare by idx and then
    // by the class name's hash to avoid having to do the more
    // expensive string comparison.
    if (idx != o.idx) return idx < o.idx;
    auto const hash1 = cls->hash();
    auto const hash2 = o.cls->hash();
    if (hash1 != hash2) return hash1 < hash2;
    return string_data_lti{}(cls, o.cls);
  }

  struct Hash {
    size_t operator()(const MethRef& m) const {
      return folly::hash::hash_combine(m.cls->hash(), m.idx);
    }
  };

  template <typename SerDe> void serde(SerDe& sd) {
    sd(cls)(idx);
  }
};

using MethRefSet = hphp_fast_set<MethRef, MethRef::Hash>;

/*
 * Entries in the ClassInfo method table need to track some additional
 * information.
 *
 * The reason for this is that we need to record attributes of the
 * class hierarchy.
 *
 * We store a lot of these, so we go to some effort to keep it as
 * small as possible.
 */
struct MethTabEntry {
  MethTabEntry()
    : MethTabEntry{MethRef{}, Attr{}} {}
  explicit MethTabEntry(const php::Func& f)
    : MethTabEntry{MethRef{f}, f.attrs} {}
  MethTabEntry(const php::Func& f, Attr a)
    : MethTabEntry{MethRef{f}, a} {}
  MethTabEntry(MethRef meth, Attr a)
    : cls{TopLevel, meth.cls}
    , clsIdx{meth.idx}
    , attrs{a} {}

  MethRef meth() const { return MethRef{cls.ptr(), clsIdx}; }
  void setMeth(MethRef m) { cls.set(cls.tag(), m.cls); clsIdx = m.idx; }

  // There's a private method further up the class hierarchy with the
  // same name.
  bool hasPrivateAncestor() const { return cls.tag() & HasPrivateAncestor; }
  // This method came from the ClassInfo that owns the MethTabEntry,
  // or one of its used traits.
  bool topLevel() const { return cls.tag() & TopLevel; }
  // This method isn't overridden by methods in any regular classes.
  bool noOverrideRegular() const { return cls.tag() & NoOverrideRegular; }
  // First appearance of a method with this name in the hierarchy.
  bool firstName() const { return cls.tag() & FirstName; }

  void setHasPrivateAncestor() {
    cls.set(Bits(cls.tag() | HasPrivateAncestor), cls.ptr());
  }
  void setTopLevel() {
    cls.set(Bits(cls.tag() | TopLevel), cls.ptr());
  }
  void setNoOverrideRegular() {
    cls.set(Bits(cls.tag() | NoOverrideRegular), cls.ptr());
  }
  void setFirstName() {
    cls.set(Bits(cls.tag() | FirstName), cls.ptr());
  }

  void clearHasPrivateAncestor() {
    cls.set(Bits(cls.tag() & ~HasPrivateAncestor), cls.ptr());
  }
  void clearTopLevel() {
    cls.set(Bits(cls.tag() & ~TopLevel), cls.ptr());
  }
  void clearNoOverrideRegular() {
    cls.set(Bits(cls.tag() & ~NoOverrideRegular), cls.ptr());
  }
  void clearFirstName() {
    cls.set(Bits(cls.tag() & ~FirstName), cls.ptr());
  }

private:
  // Logically, a MethTabEntry stores a MethRef. However doing so
  // makes the MethTabEntry larger, due to alignment. So instead we
  // store the MethRef fields (which lets the clsIdx and attrs share
  // the same 64-bits). Moreover, we can store the special bits in the
  // cls name pointer.
  enum Bits : uint8_t {
    HasPrivateAncestor = (1u << 0),
    TopLevel = (1u << 1),
    NoOverrideRegular = (1u << 2),
    FirstName = (1u << 3)
  };
  CompactTaggedPtr<const StringData, Bits> cls;
  uint32_t clsIdx;

public:
  // A method could be imported from a trait, and its attributes
  // changed.
  Attr attrs;

  template <typename SerDe> void serde(SerDe& sd) {
    if constexpr (SerDe::deserializing) {
      SString clsname;
      Bits bits;
      sd(clsname)(clsIdx)(attrs)(bits);
      cls.set(bits, clsname);
    } else {
      sd(cls.ptr())(clsIdx)(attrs)(cls.tag());
    }
  }
};

// Don't indeliberably make this larger
static_assert(CheckSize<MethTabEntry, 16>(), "");

//////////////////////////////////////////////////////////////////////

using ContextRetTyMap = tbb::concurrent_hash_map<
  CallContext,
  Index::ReturnType,
  CallContextHashCompare
>;

//////////////////////////////////////////////////////////////////////

template<class Filter>
PropState make_unknown_propstate(const Index& index,
                                 const php::Class* cls,
                                 Filter filter) {
  auto ret = PropState{};
  for (auto& prop : cls->properties) {
    if (filter(prop)) {
      auto& elem = ret[prop.name];
      elem.ty = adjust_type_for_prop(
        index,
        *cls,
        &prop.typeConstraint,
        TCell
      );
      if (prop.attrs & AttrSystemInitialValue) {
        auto initial = loosen_all(from_cell(prop.val));
        if (!initial.subtypeOf(BUninit)) elem.ty |= initial;
      }
      elem.tc = &prop.typeConstraint;
      elem.attrs = prop.attrs;
      elem.everModified = true;
    }
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

bool func_supports_AER(const php::Func* func) {
  // Async functions always support async eager return, and no other
  // functions support it yet.
  return func->isAsync && !func->isGenerator;
}

uint32_t func_num_inout(const php::Func* func) {
  if (!func->hasInOutArgs) return 0;
  uint32_t count = 0;
  for (auto& p : func->params) count += p.inout;
  return count;
}

PrepKind func_param_prep(const php::Func* f, uint32_t paramId) {
  auto const sz = f->params.size();
  if (paramId >= sz) return PrepKind{TriBool::No, TriBool::No};
  PrepKind kind;
  kind.inOut = yesOrNo(f->params[paramId].inout);
  kind.readonly = yesOrNo(f->params[paramId].readonly);
  return kind;
}

uint32_t numNVArgs(const php::Func& f) {
  uint32_t cnt = f.params.size();
  return cnt && f.params[cnt - 1].isVariadic ? cnt - 1 : cnt;
}

//////////////////////////////////////////////////////////////////////

}

/*
 * Currently inferred information about a PHP function.
 *
 * Nothing in this structure can ever be untrue.  The way the
 * algorithm works, whatever is in here must be factual (even if it is
 * not complete information), because we may deduce other facts based
 * on it.
 */
struct res::Func::FuncInfo {
  const php::Func* func = nullptr;
  /*
   * The best-known return type of the function, if we have any
   * information.  May be TBottom if the function is known to never
   * return (e.g. always throws).
   */
  Type returnTy = TInitCell;

  /*
   * If the function always returns the same parameter, this will be
   * set to its id; otherwise it will be NoLocalId.
   */
  LocalId retParam{NoLocalId};

  /*
   * The number of times we've refined returnTy.
   */
  uint32_t returnRefinements{0};

  /*
   * Whether the function is effectFree.
   */
  bool effectFree{false};

  /*
   * Bitset representing which parameters definitely don't affect the
   * result of the function, assuming it produces one. Note that
   * the parameter type verification does not count as a use in this context.
   */
  std::bitset<64> unusedParams;

  /*
   * List of all func families this function belongs to.
   */
  CompactVector<FuncFamily*> families;
};

/*
 * Currently inferred information about a Hack function.
 *
 * Nothing in this structure can ever be untrue. The way the algorithm
 * works, whatever is in here must be factual (even if it is not
 * complete information), because we may deduce other facts based on
 * it.
 *
 * This class mirrors the FuncInfo struct, but is produced and used by
 * remote workers. As needed, this struct will gain more and more of
 * FuncInfo's fields (but stored in a more remote worker friendly
 * way). Local calculations will continue to use FuncInfo. Once
 * everything is converted to use remote workers, this struct will
 * subsume FuncInfo entirely (and be renamed).
 */
struct FuncInfo2 {
  /*
   * Name of this function. If this is a top level function, this will
   * be an unique identifier. For methods, it will just be the method
   * name.
   */
  SString name;

  /*
   * The php::Func representing this function. This field is not
   * serialized. If you wish to make use of it, it must be fixed up
   * manually after deserialization.
   */
  const php::Func* func = nullptr;

  /*
   * The best-known return type of the function, if we have any
   * information. May be TBottom if the function is known to never
   * return (e.g. always throws).
   */
  Type returnTy = TInitCell;

  template <typename SerDe> void serde(SerDe& sd) {
    sd(name)(returnTy);
  }
};

namespace {

//////////////////////////////////////////////////////////////////////

/*
 * Known information about a particular constant:
 *  - if system is true, it's a system constant and other definitions
 *    will be ignored.
 *  - for non-system constants, if func is non-null it's the unique
 *    pseudomain defining the constant; otherwise there was more than
 *    one definition, or a non-pseudomain definition, and the type will
 *    be TInitCell
 *  - readonly is true if we've only seen uses of the constant, and no
 *    definitions (this could change during the first pass, but not after
 *    that).
 */

struct ConstInfo {
  const php::Func* func;
  Type                          type;
  bool                          system;
  bool                          readonly;
};

using FuncFamily       = res::Func::FuncFamily;
using FuncInfo         = res::Func::FuncInfo;

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

/*
 * Sometimes function resolution can't determine which function
 * something will call, but can restrict it to a family of functions.
 *
 * For example, if you want to call an abstract function on a base
 * class with all unique derived classes, we will resolve the function
 * to a FuncFamily that contains references to all the possible
 * overriding-functions.
 *
 * In general, a FuncFamily can contain functions which are used by a
 * regular class or not. In some contexts, we only care about the
 * subset which are used by a regular class, and in some contexts we
 * care about them all. To save memory, we use a single FuncFamily for
 * both cases. The users of the FuncFamily must skip over which funcs
 * it does not care about.
 *
 * Since we cache information related to the func list, if the "all"
 * case and the "regular-only" case are potentially different, we
 * allocated space for both possibilities. If we determine they'll
 * always be the same, we do not. For example, if the possible func
 * list only contains methods on regular classes, the distinction is
 * irrelevant.
 */
struct res::Func::FuncFamily {
  // A PossibleFunc is a php::Func* with an additional bit that
  // indicates whether that func is present on a regular class or
  // not. This lets us skip over that func if we only care about the
  // regular subset of the list.
  struct PossibleFunc {
    PossibleFunc(const php::Func* f, bool r) : m_func{r, f} {}
    const php::Func* ptr() const { return m_func.ptr(); }
    bool inRegular() const { return (bool)m_func.tag(); }
    bool operator==(const PossibleFunc& o) const { return m_func == o.m_func; }
  private:
    CompactTaggedPtr<const php::Func, uint8_t> m_func;
  };
  using PFuncVec = CompactVector<PossibleFunc>;

  // We have a lot of FuncFamilies, and most of them have the same
  // "static" information (doesn't change as a result of
  // analysis). So, we store unique groups of static info separately
  // and FuncFamilies point to the same ones.
  struct StaticInfo {
    Optional<uint32_t> m_numInOut;
    Optional<RuntimeCoeffects> m_requiredCoeffects;
    Optional<CompactVector<CoeffectRule>> m_coeffectRules;
    PrepKindVec m_paramPreps;
    uint32_t m_minNonVariadicParams;
    uint32_t m_maxNonVariadicParams;
    TriBool m_isReadonlyReturn;
    TriBool m_isReadonlyThis;
    TriBool m_supportsAER;
    bool m_maybeReified : 1;
    bool m_maybeCaresAboutDynCalls : 1;
    bool m_maybeBuiltin : 1;

    bool operator==(const StaticInfo& o) const;
    size_t hash() const;
  };

  // State in the FuncFamily which might vary depending on whether
  // we're considering the regular subset or not.
  struct Info {
    LockFreeLazy<Index::ReturnType> m_returnTy;
    const StaticInfo* m_static{nullptr};
  };

  FuncFamily(PFuncVec&& v, bool add) : m_v{std::move(v)}
  { if (add) m_regular = std::make_unique<Info>(); }

  FuncFamily(FuncFamily&&) = delete;
  FuncFamily(const FuncFamily&) = delete;
  FuncFamily& operator=(FuncFamily&&) = delete;
  FuncFamily& operator=(const FuncFamily&) = delete;

  const PFuncVec& possibleFuncs() const {
    return m_v;
  };

  Info& infoFor(bool regularOnly) {
    if (regularOnly && m_regular) return *m_regular;
    return m_all;
  }
  const Info& infoFor(bool regularOnly) const {
    if (regularOnly && m_regular) return *m_regular;
    return m_all;
  }

  Info m_all;
  // Only allocated if we determined the distinction is relevant. If
  // this is nullptr, m_all can be used for both cases.
  std::unique_ptr<Info> m_regular;
  PFuncVec m_v;
};

bool FuncFamily::StaticInfo::operator==(const FuncFamily::StaticInfo& o) const {
  return
    std::tie(m_numInOut, m_requiredCoeffects, m_coeffectRules,
             m_paramPreps, m_minNonVariadicParams,
             m_maxNonVariadicParams,
             m_isReadonlyReturn, m_isReadonlyThis, m_supportsAER,
             m_maybeReified, m_maybeCaresAboutDynCalls,
             m_maybeBuiltin) ==
    std::tie(o.m_numInOut, o.m_requiredCoeffects, o.m_coeffectRules,
             o.m_paramPreps, o.m_minNonVariadicParams,
             o.m_maxNonVariadicParams,
             o.m_isReadonlyReturn, o.m_isReadonlyThis, o.m_supportsAER,
             o.m_maybeReified, o.m_maybeCaresAboutDynCalls,
             o.m_maybeBuiltin);
}

size_t FuncFamily::StaticInfo::hash() const {
  auto hash = folly::hash::hash_combine(
    m_numInOut,
    m_requiredCoeffects,
    m_minNonVariadicParams,
    m_maxNonVariadicParams,
    m_isReadonlyReturn,
    m_isReadonlyThis,
    m_supportsAER,
    m_maybeReified,
    m_maybeCaresAboutDynCalls,
    m_maybeBuiltin
  );
  hash = folly::hash::hash_range(
    m_paramPreps.begin(),
    m_paramPreps.end(),
    hash
  );
  if (m_coeffectRules) {
    hash = folly::hash::hash_range(
      m_coeffectRules->begin(),
      m_coeffectRules->end(),
      hash
    );
  }
  return hash;
}

//////////////////////////////////////////////////////////////////////

namespace {

struct PFuncVecHasher {
  size_t operator()(const FuncFamily::PFuncVec& v) const {
    return folly::hash::hash_range(
      v.begin(),
      v.end(),
      0,
      [] (FuncFamily::PossibleFunc pf) {
        return hash_int64_pair(
          pointer_hash<const php::Func>{}(pf.ptr()),
          pf.inRegular()
        );
      }
    );
  }
};
struct FuncFamilyPtrHasher {
  using is_transparent = void;
  size_t operator()(const std::unique_ptr<FuncFamily>& ff) const {
    return PFuncVecHasher{}(ff->possibleFuncs());
  }
  size_t operator()(const FuncFamily::PFuncVec& pf) const {
    return PFuncVecHasher{}(pf);
  }
};
struct FuncFamilyPtrEquals {
  using is_transparent = void;
  bool operator()(const std::unique_ptr<FuncFamily>& a,
                  const std::unique_ptr<FuncFamily>& b) const {
    return a->possibleFuncs() == b->possibleFuncs();
  }
  bool operator()(const FuncFamily::PFuncVec& pf,
                  const std::unique_ptr<FuncFamily>& ff) const {
    return pf == ff->possibleFuncs();
  }
};

struct FFStaticInfoPtrHasher {
  using is_transparent = void;
  size_t operator()(const std::unique_ptr<FuncFamily::StaticInfo>& i) const {
    return i->hash();
  }
  size_t operator()(const FuncFamily::StaticInfo& i) const {
    return i.hash();
  }
};
struct FFStaticInfoPtrEquals {
  using is_transparent = void;
  bool operator()(const std::unique_ptr<FuncFamily::StaticInfo>& a,
                  const std::unique_ptr<FuncFamily::StaticInfo>& b) const {
    return *a == *b;
  }
  bool operator()(const FuncFamily::StaticInfo& a,
                  const std::unique_ptr<FuncFamily::StaticInfo>& b) const {
    return a == *b;
  }
};

//////////////////////////////////////////////////////////////////////

}

/*
 * Sometimes function resolution can't determine which exact function
 * something will call, but can restrict it to a family of functions.
 *
 * For example, if you want to call a function on a base class, we
 * will resolve the function to a func family that contains references
 * to all the possible overriding-functions.
 *
 * In general, a func family can contain functions which are used by a
 * regular class or not. In some contexts, we only care about the
 * subset which are used by a regular class, and in some contexts we
 * care about them all. To save memory, we use a single func family
 * for both cases. The users of the func family should only consult
 * the subset they care about.
 *
 * Besides the possible functions themselves, information in common
 * about the functions is cached. For example, return type. This
 * avoids having to iterate over potentially very large sets of
 * functions.
 *
 * This class mirrors the FuncFamily struct but is produced and used
 * by remote workers. Once everything is converted to use remote
 * workers, this struct will replace FuncFamily entirely (and be
 * renamed).
 */
struct FuncFamily2 {
  // Func families don't have any inherent name, but it's convenient
  // to have an unique id to refer to each one. We produce a SHA1 hash
  // of all of the methods in the func family.
  using Id = SHA1;

  Id m_id;
  // All methods in a func family should have the same name. However,
  // multiple func families may have the same name (so this is not an
  // unique identifier).
  SString m_name;
  // Methods used by a regular classes
  std::vector<MethRef> m_regular;
  // Methods used exclusively by non-regular classes, but as a private
  // method. In some situations, these are treated as if it was on
  // m_regular.
  std::vector<MethRef> m_nonRegularPrivate;
  // Methods used exclusively by non-regular classes
  std::vector<MethRef> m_nonRegular;

  // Information about the group of methods relevant to analysis which
  // doesn't change (hence "static").
  struct StaticInfo {
    Optional<uint32_t> m_numInOut;
    Optional<RuntimeCoeffects> m_requiredCoeffects;
    Optional<CompactVector<CoeffectRule>> m_coeffectRules;
    PrepKindVec m_paramPreps;
    uint32_t m_minNonVariadicParams;
    uint32_t m_maxNonVariadicParams;
    TriBool m_isReadonlyReturn;
    TriBool m_isReadonlyThis;
    TriBool m_supportsAER;
    bool m_maybeReified;
    bool m_maybeCaresAboutDynCalls;
    bool m_maybeBuiltin;

    StaticInfo& operator|=(const StaticInfo& o) {
      if (m_numInOut != o.m_numInOut) {
        m_numInOut.reset();
      }
      if (m_requiredCoeffects != o.m_requiredCoeffects) {
        m_requiredCoeffects.reset();
      }
      if (m_coeffectRules != o.m_coeffectRules) {
        m_coeffectRules.reset();
      }

      if (o.m_paramPreps.size() > m_paramPreps.size()) {
        m_paramPreps.resize(
          o.m_paramPreps.size(),
          PrepKind{TriBool::No, TriBool::No}
        );
      }
      for (size_t i = 0; i < o.m_paramPreps.size(); ++i) {
        m_paramPreps[i].inOut |= o.m_paramPreps[i].inOut;
        m_paramPreps[i].readonly |= o.m_paramPreps[i].readonly;
      }
      for (size_t i = o.m_paramPreps.size(); i < m_paramPreps.size(); ++i) {
        m_paramPreps[i].inOut |= TriBool::No;
        m_paramPreps[i].readonly |= TriBool::No;
      }

      m_minNonVariadicParams =
        std::min(m_minNonVariadicParams, o.m_minNonVariadicParams);
      m_maxNonVariadicParams =
        std::max(m_maxNonVariadicParams, o.m_maxNonVariadicParams);
      m_isReadonlyReturn |= o.m_isReadonlyReturn;
      m_isReadonlyThis |= o.m_isReadonlyThis;
      m_supportsAER |= o.m_supportsAER;
      m_maybeReified |= o.m_maybeReified;
      m_maybeCaresAboutDynCalls |= o.m_maybeCaresAboutDynCalls;
      m_maybeBuiltin |= o.m_maybeBuiltin;

      return *this;
    }

    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_numInOut)
        (m_requiredCoeffects)
        (m_coeffectRules)
        (m_paramPreps)
        (m_minNonVariadicParams)
        (m_maxNonVariadicParams)
        (m_isReadonlyReturn)
        (m_isReadonlyThis)
        (m_supportsAER)
        (m_maybeReified)
        (m_maybeCaresAboutDynCalls)
        (m_maybeBuiltin)
        ;
    }
  };
  Optional<StaticInfo> m_allStatic;
  Optional<StaticInfo> m_regularStatic;

  const StaticInfo& infoFor(bool regular) const {
    if (regular) {
      assertx(m_regularStatic.has_value());
      return *m_regularStatic;
    }
    return *m_allStatic;
  }

  template <typename SerDe> void serde(SerDe& sd) {
    sd(m_id)
      (m_name)
      (m_regular)
      (m_nonRegularPrivate)
      (m_nonRegular)
      (m_allStatic)
      (m_regularStatic)
      ;
  }
};

namespace {

// Func families are (usually) very small, but we have a lot of
// them. To reduce remote worker overhead, we bundle func families
// together into one blob.
struct FuncFamilyGroup {
  std::vector<std::unique_ptr<FuncFamily2>> m_ffs;
  template <typename SerDe> void serde(SerDe& sd) {
    // Multiple func families may reuse the same class name, so we
    // want to de-dup strings.
    ScopedStringDataIndexer _;
    sd(m_ffs);
  }
};

//////////////////////////////////////////////////////////////////////

/*
 * A method family table entry in a compact format. Can represent a
 * FuncFamily, a single php::Func, or emptiness. This represents the
 * possible resolutions of a call to a method with same name. It also
 * stores whether the entry is "complete" or "incomplete". An
 * incomplete entry means the possible resolutions includes the
 * possibility of the method not existing. A complete entry guarantees
 * it has to be one of the methods. This is (right now) irrelevant for
 * FuncFamily, but matters for php::Func, as it determines whether you
 * can fold away the call (if it's incomplete, the call might fatal).
 *
 * We create a lot of these, so we use some trickery to keep it
 * pointer sized.
 */
struct FuncFamilyOrSingle {
  FuncFamilyOrSingle() : m_ptr{Type::Empty, nullptr} {}
  explicit FuncFamilyOrSingle(FuncFamily* ff, bool incomplete)
    : m_ptr{incomplete ? Type::FuncFamilyIncomplete : Type::FuncFamily, ff} {}
  FuncFamilyOrSingle(const php::Func* f, bool incomplete)
    : m_ptr{incomplete ? Type::SingleIncomplete : Type::Single, (void*)f} {}

  // If this represents a FuncFamily, return it (or nullptr
  // otherwise).
  FuncFamily* funcFamily() const {
    return
      (m_ptr.tag() == Type::FuncFamily ||
       m_ptr.tag() == Type::FuncFamilyIncomplete)
        ? (FuncFamily*)m_ptr.ptr()
        : nullptr;
  }

  // If this represents a single php::Func, return it (or nullptr
  // otherwise).
  const php::Func* func() const {
    return
      (m_ptr.tag() == Type::Single || m_ptr.tag() == Type::SingleIncomplete)
        ? (const php::Func*)m_ptr.ptr()
        : nullptr;
  }

  // Return true if this entry represents nothing at all (for example,
  // if the method is guaranteed to not exist).
  bool isEmpty() const { return m_ptr.tag() == Type::Empty; }

  // NB: empty entries are neither incomplete nor complete. Check
  // isEmpty() first if that matters.

  // Return true if this resolution includes the possibility of no
  // method.
  bool isIncomplete() const {
    return
      m_ptr.tag() == Type::FuncFamilyIncomplete ||
      m_ptr.tag() == Type::SingleIncomplete;
  }
  // Return true if the method would resolve to exactly one of the
  // possibilities.
  bool isComplete() const {
    return
      m_ptr.tag() == Type::FuncFamily ||
      m_ptr.tag() == Type::Single;
  }

private:
  enum class Type : uint8_t {
    Empty,
    FuncFamily,
    FuncFamilyIncomplete,
    Single,
    SingleIncomplete
  };
  CompactTaggedPtr<void, Type> m_ptr;
};

std::string show(const FuncFamilyOrSingle& fam) {
  if (auto const ff = fam.funcFamily()) {
    auto const f = ff->possibleFuncs().front().ptr();
    return folly::sformat(
      "func-family {}::{}{}",
      f->cls->name, f->name,
      fam.isIncomplete() ? " (incomplete)" : ""
    );
  }
  if (auto const f = fam.func()) {
    return folly::sformat(
      "func {}::{}{}",
      f->cls->name, f->name,
      fam.isIncomplete() ? " (incomplete)" : ""
    );
  }
  return "empty";
}

/*
 * A method family table entry. Each entry encodes the possible
 * resolutions of a method call on a particular class. The reason why
 * this isn't just a func family is because we don't want to create a
 * func family when there's only one possible method involved (this is
 * common and if we did we'd create way more func
 * families). Furthermore, we really want information for two
 * different resolutions. One resolution is when we're only
 * considering regular classes, and the other is when considering all
 * classes. One of these resolutions can correspond to a func family
 * and the other may not. This struct encodes all the possible cases
 * that can occur.
 */
struct FuncFamilyEntry {
  // The equivalent of FuncFamily::StaticInfo, but only relevant for a
  // single method (so doesn't have a FuncFamily where the StaticInfo
  // can live). This can be derived from the method directly, but by
  // storing it here, we don't need to send the actual methods to the
  // workers.
  struct MethMetadata {
    MethMetadata() : m_requiredCoeffects{RuntimeCoeffects::none()} {}

    PrepKindVec m_prepKinds;
    CompactVector<CoeffectRule> m_coeffectRules;
    uint32_t m_numInOut;
    uint32_t m_nonVariadicParams;
    RuntimeCoeffects m_requiredCoeffects;
    bool m_isReadonlyReturn : 1;
    bool m_isReadonlyThis : 1;
    bool m_supportsAER : 1;
    bool m_isReified : 1;
    bool m_caresAboutDyncalls : 1;
    bool m_builtin : 1;

    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_prepKinds)
        (m_coeffectRules)
        (m_numInOut)
        (m_nonVariadicParams)
        (m_requiredCoeffects)
        ;
      SERDE_BITFIELD(m_isReadonlyReturn, sd);
      SERDE_BITFIELD(m_isReadonlyThis, sd);
      SERDE_BITFIELD(m_supportsAER, sd);
      SERDE_BITFIELD(m_isReified, sd);
      SERDE_BITFIELD(m_caresAboutDyncalls, sd);
      SERDE_BITFIELD(m_builtin, sd);
    }
  };

  // Both "regular" and "all" resolutions map to a func family. This
  // must always be the same func family because the func family
  // stores the information necessary for both cases.
  struct BothFF {
    FuncFamily2::Id m_ff;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_ff);
    }
  };
  // The "all" resolution maps to a func family but the "regular"
  // resolution maps to a single method.
  struct FFAndSingle {
    FuncFamily2::Id m_ff;
    MethRef m_regular;
    // If true, m_regular is actually non-regular, but a private
    // method (which is sometimes treated as regular).
    bool m_nonRegularPrivate;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_ff)(m_regular)(m_nonRegularPrivate);
    }
  };
  // The "all" resolution maps to a func family but the "regular"
  // resolution maps to nothing (for example, there's no regular
  // classes with that method).
  struct FFAndNone {
    FuncFamily2::Id m_ff;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_ff);
    }
  };
  // Both the "all" and "regular" resolutions map to (the same) single
  // method.
  struct BothSingle {
    MethRef m_all;
    MethMetadata m_meta;
    // If true, m_all is actually non-regular, but a private method
    // (which is sometimes treated as regular).
    bool m_nonRegularPrivate;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_all)(m_meta)(m_nonRegularPrivate);
    }
  };
  // The "all" resolution maps to a single method but the "regular"
  // resolution maps to nothing.
  struct SingleAndNone {
    MethRef m_all;
    MethMetadata m_meta;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_all)(m_meta);
    }
  };
  // No resolutions at all.
  struct None {
    template <typename SerDe> void serde(SerDe&) {}
  };

  boost::variant<
    BothFF, FFAndSingle, FFAndNone, BothSingle, SingleAndNone, None
  > m_meths{None{}};
  // A resolution is "incomplete" if there's a subclass which does not
  // contain any method with that name (not even inheriting it). If a
  // resolution is incomplete, it means besides the set of resolved
  // methods, the call might also error due to missing method. This
  // distinction is only important in a few limited circumstances.
  bool m_allIncomplete{true};
  bool m_regularIncomplete{true};
  // Whether any method in the resolution overrides a private
  // method. This is only of interest when building func families.
  bool m_privateAncestor{false};

  template <typename SerDe> void serde(SerDe& sd) {
    if constexpr (SerDe::deserializing) {
      m_meths = [&] () -> decltype(m_meths) {
        uint8_t tag;
        sd(tag);
        switch (tag) {
          case 0: return sd.template make<BothFF>();
          case 1: return sd.template make<FFAndSingle>();
          case 2: return sd.template make<FFAndNone>();
          case 3: return sd.template make<BothSingle>();
          case 4: return sd.template make<SingleAndNone>();
          case 5: return sd.template make<None>();
          default: always_assert(false);
        }
      }();
    } else {
      match<void>(
        m_meths,
        [&] (const BothFF& e)        { sd(uint8_t(0))(e); },
        [&] (const FFAndSingle& e)   { sd(uint8_t(1))(e); },
        [&] (const FFAndNone& e)     { sd(uint8_t(2))(e); },
        [&] (const BothSingle& e)    { sd(uint8_t(3))(e); },
        [&] (const SingleAndNone& e) { sd(uint8_t(4))(e); },
        [&] (const None& e)          { sd(uint8_t(5))(e); }
      );
    }

    sd(m_allIncomplete)
       (m_regularIncomplete)
       (m_privateAncestor)
      ;
  }
};

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

/*
 * Known information about an instantiatiable class.
 */
struct ClassInfo {
  /*
   * A pointer to the underlying php::Class that we're storing
   * information about.
   */
  const php::Class* cls = nullptr;

  /*
   * The info for the parent of this Class.
   */
  ClassInfo* parent = nullptr;

  /*
   * A vector of the declared interfaces class info structures.  This is in
   * declaration order mirroring the php::Class interfaceNames vector, and does
   * not include inherited interfaces.
   */
  CompactVector<const ClassInfo*> declInterfaces;

  /*
   * A (case-insensitive) map from interface names supported by this class to
   * their ClassInfo structures, flattened across the hierarchy.
   */
  ISStringToOneT<const ClassInfo*> implInterfaces;

  /*
   * A vector of the included enums, in class order, mirroring the
   * php::Class includedEnums vector.
   */
  CompactVector<const ClassInfo*> includedEnums;

  struct ConstIndex {
    const php::Const& operator*() const {
      return cls->constants[idx];
    }
    const php::Const* operator->() const {
      return get();
    }
    const php::Const* get() const {
      return &cls->constants[idx];
    }
    const php::Class* cls;
    uint32_t idx;
  };

  /*
   * A (case-sensitive) map from class constant name to the php::Class* and
   * index into the constants vector that it came from. This map is flattened
   * across the inheritance hierarchy. Use a vector_map for stable iteration.
   */
  hphp_vector_map<SString, ConstIndex> clsConstants;

  /*
   * Inferred class constant types for the constants declared on this
   * class. The order mirrors the order in the php::Class constants
   * vector. If the vector is smaller than a constant's index, that
   * constant's type is implicitly TInitCell.
   */
  CompactVector<ClsConstInfo> clsConstTypes;

  /*
   * A vector of the used traits, in class order, mirroring the
   * php::Class usedTraitNames vector.
   */
  CompactVector<const ClassInfo*> usedTraits;

  /*
   * A list of extra properties supplied by this class's used traits.
   */
  CompactVector<php::Prop> traitProps;

  /*
   * A (case-sensitive) map from class method names to the php::Func
   * associated with it. This map is flattened across the inheritance
   * hierarchy. There's a lot of these, so we use a sorted_vector_map
   * to minimize wasted space.
   */
  folly::sorted_vector_map<SString, MethTabEntry> methods;

  /*
   * A (case-sensitive) map from class method names to associated
   * FuncFamilyOrSingle objects that represent the set of
   * possibly-overriding methods.
   *
   * In addition to the set of methods, a bit is also set indicating
   * whether the set of "complete" or not. A complete set means the
   * ultimate method will definitely be one in the set. An incomplete
   * set means that the ultimate method will either be one in the set,
   * or won't resolve to anything (a missing function).
   *
   * We do not encode "special" methods in these, as their semantics
   * are special and it's not useful.
   *
   * For every method present in this ClassInfo's method table, there
   * will be an entry in methodFamilies. For regular classes, this
   * suffices for looking up information for both all subclasses and
   * the regular subset. For non-regular classes, the results for
   * "all" and the regular subset may differ. In that case, there is a
   * separate "aux" table containing the results for the regular
   * subset. If there is no associated entry in the aux table, the
   * result is the same as the entry in the normal table (this is a
   * common case and saves on memory). For regular classes the aux
   * table is always empty.
   *
   * If a method is marked as AttrNoOverride, it will not have an
   * entry in these maps. If a method is marked as noOverrideRegular,
   * it will not have an entry in the aux map (if it would have
   * otherwise). In either case, the resolved method is assumed to be
   * the same method in this ClassInfo's method table.
   *
   * The above is true for all class types. For abstract classes and
   * interfaces, however, there may be more entries here than present
   * in the methods table. These correspond to methods implemented by
   * *all* regular subclasses of the abstract class/interface. For
   * that reason, they will only be present in the regular variant of
   * the map. This is needed to preserve monotonicity (see comment in
   * BuildSubclassListJob::process_roots).
   */
  folly::sorted_vector_map<SString, FuncFamilyOrSingle> methodFamilies;
  folly::sorted_vector_map<SString, FuncFamilyOrSingle> methodFamiliesAux;

  /*
   * For classes (abstract and non-abstract), this is the subclasses
   * of this class, including the class itself.
   *
   * For interfaces, this is the list of classes that implement this
   * interface, including the interface itself.
   *
   * For traits, this is the list of classes that use the trait where
   * the trait wasn't flattened into the class (including the trait
   * itself). Note that unlike the other cases, a class being on a
   * trait's subclass list does *not* imply a "is-a" relationship at
   * runtime. You usually want to avoid iterating the subclass list of
   * a trait.
   *
   * As an optimization, Closure has an empty subclass list (this is
   * the only situation where the subclass list can be completely
   * empty). Closure's actual subclass list can grow pathologically
   * large on a big code base and it's rarely useful. We force Closure
   * to be unresolved whenever its inside a res::Class, so we never
   * need the list.
   *
   * The elements in this vector are sorted by their pointer value.
   */
  CompactVector<ClassInfo*> subclassList;

  /*
   * A vector of ClassInfo that encodes the inheritance hierarchy,
   * unless if this ClassInfo represents an interface.
   *
   * This is the list of base classes for this class in inheritance
   * order.
   */
  CompactVector<ClassInfo*> baseList;

  /*
   * Information about interfaces' relationship to each other. Used to
   * speed up subtypeOf and couldBe operations involving
   * interfaces. Since interfaces are a minority, we heap allocate the
   * information (so only need a pointer when we don't need
   * it). Furthermore, we lazily calculate the information (we may not
   * need the information ever for a given interface).
   */
  struct InterfaceInfo {
    // The set of interfaces or base classes which have some non-empty
    // intersection with this interface. If an interface is in this
    // set, one of it's implementations also implements this
    // ClassInfo. If a non-interface is in this set, it or one of it's
    // base classes implements this ClassInfo. This set can
    // potentially be large and is needed rarely, so it is lazily
    // calculated (on top of InterfaceInfo being lazily
    // calculated). We keep two variants. The first only includes
    // information from the interface's implementations which are
    // regular classes. The second uses all of the interface's
    // implementations (and the interface itself).
    using CouldBeSet = hphp_fast_set<const ClassInfo*>;
    mutable LockFreeLazy<CouldBeSet> lazyCouldBe;
    mutable LockFreeLazy<CouldBeSet> lazyCouldBeNonRegular;

    // The set of interfaces which this interface is a subtype
    // of. That is, every implementation of this interface also
    // implements those interfaces. Every interface is a subtypeOf
    // itself, but itself is not stored here (space optimization).
    hphp_fast_set<const ClassInfo*> subtypeOf;

    // Non-nullptr if there's a single class which is a super class of
    // all implementations of this interface, nullptr otherwise.
    const ClassInfo* commonBase;
  };
  // Don't access this directly, use interfaceInfo().
  LockFreeLazyPtr<InterfaceInfo> lazyInterfaceInfo;
  LockFreeLazyPtrNoDelete<ClassInfo> lazyEquivalent;

  // Obtain the InterfaceInfo or CouldBeSet for this interface
  // (calculating it if necessary). This class must be an interface.
  const InterfaceInfo& interfaceInfo();
  const InterfaceInfo::CouldBeSet& couldBe();
  const InterfaceInfo::CouldBeSet& couldBeNonRegular();

  /*
   * Obtain an equivalent ClassInfo for an interface or abstract class
   * when ignoring all non-regular subclasses. This is used for
   * canonicalizing types. The class must have at least one
   * non-regular subclass (so check before calling).
   */
  const ClassInfo* withoutNonRegularEquivalent();

  /*
   * Property types for public static properties, declared on this exact class
   * (i.e. not flattened in the hierarchy).
   */
  SStringToOneT<PublicSPropEntry> publicStaticProps;

  /*
   * Flags to track if this class is mocked, or if any of its derived classes
   * are mocked.
   */
  bool isMocked{false};
  bool isSubMocked{false};

  /*
   * Track if this class has a property which might redeclare a property in a
   * parent class with an inequivalent type-hint.
   */
  bool hasBadRedeclareProp{true};

  /*
   * Track if this class has any properties with initial values that might
   * violate their type-hints.
   */
  bool hasBadInitialPropValues{true};

  /*
   * Track if this class has any const props (including inherited ones).
   */
  bool hasConstProp{false};

  /*
   * Track if any derived classes (including this one) have any const props.
   */
  bool subHasConstProp{false};

  /*
   * Track if this class has a reified parent.
   */
  bool hasReifiedParent{false};

  /*
   * True if there's at least one regular/non-regular class on
   * subclassList (not including this class).
   */
  bool hasRegularSubclass{false};
  bool hasNonRegularSubclass{false};

  /*
   * Return true if this is derived from o.
   */
  bool derivedFrom(const ClassInfo& o) const {
    if (this == &o) return true;
    // If o is an interface, see if this declared it.
    if (o.cls->attrs & AttrInterface) return implInterfaces.count(o.cls->name);
    // Nothing derives from traits, and we already known they're not
    // the same.
    if (o.cls->attrs & AttrTrait) return false;
    // Otherwise check for direct inheritance.
    if (baseList.size() >= o.baseList.size()) {
      return baseList[o.baseList.size() - 1] == &o;
    }
    return false;
  }

  /*
   * Given two ClassInfos, return the most specific ancestor they have
   * in common, or nullptr if they have no common ancestor.
   */
  static const ClassInfo* commonAncestor(const ClassInfo* c1,
                                         const ClassInfo* c2) {
    if (c1 == c2) return c1;
    const ClassInfo* ancestor = nullptr;
    auto it1 = c1->baseList.begin();
    auto it2 = c2->baseList.begin();
    while (it1 != c1->baseList.end() && it2 != c2->baseList.end()) {
      if (*it1 != *it2) break;
      ancestor = *it1;
      ++it1;
      ++it2;
    }
    return ancestor;
  }
};

/*
 * Known information about an instantiable class.
 *
 * This class mirrors the ClassInfo struct, but is produced and used
 * by remote workers. As needed, this struct will gain more and more
 * of ClassInfo's fields (but stored in a more remote worker friendly
 * way). Local calculations will continue to use ClassInfo. Once
 * everything is converted to use remote workers, this struct will
 * subsume ClassInfo entirely (and be renamed).
 */
struct ClassInfo2 {
  /*
   * The name of the underlying php::Class that this ClassInfo
   * represents.
   */
  LSString name{nullptr};

  /*
   * The name of the parent of this class (or nullptr if none).
   */
  LSString parent{nullptr};

  /*
   * A vector of the declared interfaces names. This does not include
   * inherited interfaces. Any traits flattened into this class will
   * have it's declInterfaces added to this. The interfaces are in no
   * particular order.
   */
  CompactVector<SString> declInterfaces;

  /*
   * A (case-insensitive) set of interface names implemented by this
   * class, flattened across the hierarchy.
   */
  ISStringSet implInterfaces;

  /*
   * A (case-insensitive) set of interface names implemented by this
   * class, or any of its subclasses. If this is folly::none, then
   * subImplInterfaces is the same as implInterfaces.
   */
  Optional<ISStringSet> subImplInterfaces;

  /*
   * A vector of the included enums names, in class order, mirroring
   * the php::Class includedEnums vector.
   */
  CompactVector<SString> includedEnums;

  /*
   * Represents a class constant, pointing to where the constant was
   * originally declared (the class name and it's position in the
   * class' constant table).
   */
  struct ConstIndex {
    SString cls;
    uint32_t idx;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(cls)(idx);
    }
  };

  /*
   * A (case-sensitive) map from class constant name to the ConstIndex
   * representing the constant. This map is flattened across the
   * inheritance hierarchy.
   */
  SStringToOneT<ConstIndex> clsConstants;

  /*
   * A vector of the used traits name, in class order, mirroring the
   * php::Class usedTraitNames vector.
   */
  CompactVector<SString> usedTraits;

  /*
   * A list of extra properties supplied by this class's used traits.
   */
  CompactVector<php::Prop> traitProps;

  /*
   * A (case-sensitive) map from class method names to the
   * MethTabEntry associated with it. This map is flattened across the
   * inheritance hierarchy. MethTabEntry represents the php::Func,
   * along with some metadata specific to the method on this specific
   * class.
   */
  SStringToOneT<MethTabEntry> methods;

  /*
   * In most situations, methods are inherited from a parent class, so
   * if a class declares a method, all of its subclasses are
   * guaranteed to possess it. An exception to this is with
   * interfaces, whose methods are not inherited.
   *
   * However, when building func-families and other method data, its
   * useful to know all of the methods declared by all parents of a
   * class. Most of those methods will be on the method table for this
   * ClassInfo, but if any aren't, they'll be added to this set.
   */
  SStringSet missingMethods;

  /*
   * For classes (abstract and non-abstract), these are the names of
   * the subclasses of this class, including the class itself.
   *
   * For interfaces, this is the list of names of classes that
   * implement this interface, including the interface itself.
   *
   * For traits, this is the list of names of classes that use the
   * trait where the trait wasn't flattened into the class (including
   * the trait itself). Note that unlike the other cases, a class
   * being on a trait's subclass list does *not* imply a "is-a"
   * relationship at runtime. You usually want to avoid iterating the
   * subclass list of a trait.
   *
   * The elements in this vector are sorted by name.
   */
  CompactVector<SString> subclassList;

  /*
   * A vector of names that encodes the inheritance hierarchy, unless
   * if this class is an interface.
   *
   * This is the list of base classes for this class in inheritance
   * order.
   */
  CompactVector<SString> baseList;

  /*
   * Set of "extra" methods for this class. These are methods which
   * aren't formally part of this class, but must be analyzed along
   * with the class' methods. Examples are unflattened trait methods,
   * and the invoke methods of their associated closures.
   */
  MethRefSet extraMethods;

  /*
   * A vector of names of all the closures associated with this class.
   */
  CompactVector<SString> closures;

  /*
   * A (case-sensitive) map from method names to associated
   * FuncFamilyEntry objects that represent the set of
   * possibly-overriding methods.
   *
   * We do not encode "special" methods in these, as their semantics
   * are special and it's not useful.
   *
   * For every method present in this ClassInfo's method table, there
   * will be an entry in methodFamilies (unless if AttrNoOverride, see
   * below). The FuncFamilyEntry will provide the set of
   * possibly-overriding methods, for both the regular class subset
   * and all classes.
   *
   * If a method is marked as AttrNoOverride, it will not have an
   * entry in this map. The resolved method is assumed to be the same
   * method in this ClassInfo's method table. If a method is marked as
   * noOverrideRegular, it will have an entry in this map, but can be
   * treated as AttrNoOverride if you're only considering regular
   * classes.
   *
   * There may be more entries in methodFamilies than in the
   * ClassInfo's method table. For classes which aren't abstract and
   * aren't interfaces, this is an artifact of how the table is
   * created and can be ignored. For abstract classes and interfaces,
   * however, these extra entries correspond to methods implemented by
   * *all* regular subclasses of the abstract class/interface. In this
   * situation, only the data in the FuncFamilyEntry corresponding to
   * the regular subset is meaningful. This is needed to preserve
   * monotonicity (see comment in
   * BuildSubclassListJob::process_roots).
   */
  SStringToOneT<FuncFamilyEntry> methodFamilies;

  /*
   * FuncInfo2s for the methods declared on this class (not
   * flattened). This is in the same order as the methods vector on
   * the associated php::Class.
   */
  CompactVector<std::unique_ptr<FuncInfo2>> funcInfos;

  /*
   * Track if this class has a property which might redeclare a property in a
   * parent class with an inequivalent type-hint.
   */
  bool hasBadRedeclareProp{true};

  /*
   * Track if this class has any properties with initial values that might
   * violate their type-hints.
   */
  bool hasBadInitialPropValues{true};

  /*
   * Track if this class has any const props (including inherited ones).
   */
  bool hasConstProp{false};

  /*
   * Track if any derived classes (including this one) have any const props.
   */
  bool subHasConstProp{false};

  /*
   * Track if this class has a reified parent.
   */
  bool hasReifiedParent{false};

  /*
   * Whether this class (or any derived classes) has a __Reified
   * attribute.
   */
  bool hasReifiedGeneric{false};
  bool subHasReifiedGeneric{false};

  /*
   * Initial AttrNoReifiedInit setting of attrs (which might be
   * modified).
   */
  bool initialNoReifiedInit{false};

  /*
   * Whether is_mock_class() is true for this class.
   */
  bool isMockClass{false};

  /*
   * Whether this class is mocked (has a direct subclass for which
   * is_mock_class() is true).
   */
  bool isMocked{false};
  /*
   * Whether isMocked is true for this class, or any of its
   * subclasses.
   */
  bool isSubMocked{false};

  /*
   * Whether is_regular_class() is true for this class.
   */
  bool isRegularClass{false};

  /*
   * True if there's at least one regular/non-regular class on
   * subclassList (not including this class). If both are false, that
   * implies this class doesn't have any subclasses.
   */
  bool hasRegularSubclass{false};
  bool hasNonRegularSubclass{false};

  template <typename SerDe> void serde(SerDe& sd) {
    ScopedStringDataIndexer _;
    sd(name)
      (parent)
      (declInterfaces)
      (implInterfaces, string_data_lti{})
      (subImplInterfaces, string_data_lti{})
      (clsConstants, string_data_lt{})
      (includedEnums)
      (usedTraits)
      (traitProps)
      (methods, string_data_lt{})
      (missingMethods, string_data_lt{})
      (subclassList)
      (baseList)
      (extraMethods, std::less<MethRef>{})
      (closures)
      (methodFamilies, string_data_lt{})
      (funcInfos)
      (hasBadRedeclareProp)
      (hasBadInitialPropValues)
      (hasConstProp)
      (subHasConstProp)
      (hasReifiedParent)
      (hasReifiedGeneric)
      (subHasReifiedGeneric)
      (initialNoReifiedInit)
      (isMockClass)
      (isMocked)
      (isSubMocked)
      (isRegularClass)
      (hasRegularSubclass)
      (hasNonRegularSubclass)
      ;
  }
};

namespace {

ClassInfo::InterfaceInfo::CouldBeSet couldBeSetBuilder(const ClassInfo* cinfo,
                                                       bool nonRegular) {
  assertx(cinfo->cls->attrs & AttrInterface);
  // For every implementation of this interface, add all of the other
  // interfaces this implementation implements, and also all of its
  // parent classes.
  ClassInfo::InterfaceInfo::CouldBeSet couldBe;
  assertx(!cinfo->subclassList.empty());
  for (auto const sub : cinfo->subclassList) {
    if (!nonRegular && !is_regular_class(*sub->cls)) continue;
    for (auto const& [_, impl] : sub->implInterfaces) couldBe.emplace(impl);

    auto c = sub;
    do {
      // If we already added it, all subsequent parents are also
      // added, so we can stop.
      if (!couldBe.emplace(c).second) break;
      c = c->parent;
    } while (c);
  }
  return couldBe;
}

}

const ClassInfo::InterfaceInfo::CouldBeSet& ClassInfo::couldBe() {
  return interfaceInfo().lazyCouldBe.get(
    [this] { return couldBeSetBuilder(this, false); }
  );
}

const ClassInfo::InterfaceInfo::CouldBeSet& ClassInfo::couldBeNonRegular() {
  return interfaceInfo().lazyCouldBeNonRegular.get(
    [this] { return couldBeSetBuilder(this, true); }
  );
}

const ClassInfo::InterfaceInfo& ClassInfo::interfaceInfo() {
  assertx(cls->attrs & AttrInterface);
  return lazyInterfaceInfo.get(
    [this] {
      auto info = std::make_unique<ClassInfo::InterfaceInfo>();

      // Start out with the info from the first implementation.
      assertx(!subclassList.empty());
      size_t idx = 0;
      while (idx < subclassList.size()) {
        auto const sub = subclassList[idx++];
        if (!is_regular_class(*sub->cls)) continue;
        info->commonBase = sub;
        for (auto const& [_, impl] : sub->implInterfaces) {
          info->subtypeOf.emplace(impl);
        }
        break;
      }

      // Update the common base and subtypeOf list for every
      // implementation. We're only a subtype of an interface if
      // *every* implementation of us implements that interface, so
      // the set can only shrink.
      while (idx < subclassList.size()) {
        auto const sub = subclassList[idx++];
        if (!is_regular_class(*sub->cls)) continue;
        if (info->commonBase) {
          info->commonBase = commonAncestor(info->commonBase, sub);
        }
        folly::erase_if(
          info->subtypeOf,
          [&] (const ClassInfo* i) {
            return !sub->implInterfaces.count(i->cls->name);
          }
        );
        if (!info->commonBase && info->subtypeOf.empty()) break;
      }

      return info.release();
    }
  );
}

const ClassInfo* ClassInfo::withoutNonRegularEquivalent() {
  assertx(cls->attrs & (AttrInterface | AttrAbstract));
  assertx(hasRegularSubclass);
  return &lazyEquivalent.get(
    [this] {
      // Remove the non-regular classes, which will automatically
      // canonicalize this class to its "best" equivalent (which may
      // be itself).
      auto const without = res::Class::removeNonRegular(
        std::array<res::Class, 1>{res::Class { this } }
      );
      // We shouldn't get anything other than one result back. It
      // shouldn't be zero because we already checked it has at least
      // one regular subclass, and we shouldn't get intersections
      // because the common base class should be a subtype of any
      // interfaces.
      always_assert(without.size() == 1);
      auto const w = without.front();
      assertx(w.val.right());
      return w.val.right();
    }
  );
}

//////////////////////////////////////////////////////////////////////

namespace res {

bool Class::same(const Class& o) const {
  return val == o.val;
}

bool Class::exactSubtypeOfExact(const Class& o,
                                bool nonRegularL,
                                bool nonRegularR) const {
  // Two exact classes are only subtypes of another if they're the
  // same. One additional complication is if the class isn't regular
  // and we're not considering non-regular classes. In that case, the
  // class is actually Bottom, and we need to apply the rules of
  // subtyping to Bottom (Bottom is a subtype of everything, but
  // nothing is a subtype of it).
  if (auto const lname = val.left()) {
    // For unresolved classes, we can't actually check if
    // is_regular_class(), so we need to be pessimistic.
    if (auto const rname = o.val.left()) {
      // Two exact unresolved classes are only guarantee to subtypes
      // of another if they're the same name and the lhs might not
      // become Bottom, but the rhs could (we're assuming the class
      // might not be regular). NB: if the class doesn't actually
      // exist, this is correct as both sides will be Bottom.
      return (!nonRegularL || nonRegularR) && lname->isame(rname);
    }
    // If the rhs is resolved, it's similar to the unresolved case,
    // except we can know whether the class is regular or not. If an
    // unresolved and resolved refer to the same class, the resolved
    // one is considered a subtype of the unresolved one.
    auto const c2 = o.val.right();
    return
      !nonRegularL &&
      lname->isame(c2->cls->name) &&
      !is_regular_class(*c2->cls);
  } else if (auto const rname = o.val.left()) {
    auto const c1 = val.right();
    return
      rname->isame(c1->cls->name) &&
      (!nonRegularL || nonRegularR || is_regular_class(*c1->cls));
  } else {
    // Otherwise both sides are resolved and we can do a precise
    // check.
    auto const c1 = val.right();
    auto const c2 = o.val.right();
    auto const bottomL = !nonRegularL && !is_regular_class(*c1->cls);
    auto const bottomR = !nonRegularR && !is_regular_class(*c2->cls);
    return bottomL || (!bottomR && c1 == c2);
  }
}

bool Class::exactSubtypeOf(const Class& o,
                           bool nonRegularL,
                           bool nonRegularR) const {
  if (auto const lname = val.left()) {
    // For unresolved classes, we can't actually check if
    // is_regular_class(), so we need to be pessimistic and assume
    // either side could go to Bottom if nonRegular is false.
    if (auto const rname = o.val.left()) {
      return (!nonRegularL || nonRegularR) && lname->isame(rname);
    }
    auto const c2 = o.val.right();
    if (lname->isame(c2->cls->name)) {
      // If they represent the same class, the lhs isn't a subtype of
      // the rhs because the lhs is unresolved. The only exception is
      // if the lhs is actually Bottom because it's not regular and we
      // don't want those.
      return !nonRegularL && !is_regular_class(*c2->cls);
    }
    if (c2->cls->attrs & AttrTrait) return false;
    // The lhs is unresolved, but the rhs is resolved. Check to see if
    // lhs exists on rhs subclass-list and is not filtered out from
    // being a non-regular class.
    for (auto const sub : c2->subclassList) {
      if (!sub->cls->name->isame(lname)) continue;
      return !nonRegularL || nonRegularR || is_regular_class(*sub->cls);
    }
    return false;
  }

  auto const c1 = val.right();
  // If we want to exclude non-regular classes on either side, and the
  // lhs is not regular, there's no subtype relation. If nonRegularL
  // is false, then lhs is just a bottom (and bottom is a subtype of
  // everything), and if nonRegularR is false, then the rhs might not
  // contain any non-regular classes, so lhs is not guaranteed to be
  // part of it.
  if ((!nonRegularL || !nonRegularR) && !is_regular_class(*c1->cls)) {
    return !nonRegularL;
  }

  if (auto const rname = o.val.left()) {
    // The lhs is resolved, but the rhs is not. The lhs is a subtype
    // of rhs if any of its bases have the same name or if it
    // implements rname.
    for (auto const base : c1->baseList) {
      if (base->cls->name->isame(rname)) return true;
    }
    return c1->implInterfaces.count(rname);
  }

  // Otherwise just do an inheritance check.
  return c1->derivedFrom(*o.val.right());
}

bool Class::subSubtypeOf(const Class& o,
                         bool nonRegularL,
                         bool nonRegularR) const {
  // An unresolved class is only a subtype of another if they're both
  // unresolved, have the same name, and the lhs doesn't contain
  // non-regular classes if the rhs doesn't.
  if (auto const lname = val.left()) {
    if (auto const rname = o.val.left()) {
      // If both classes are unresolved, we can't really say for sure
      // if they're subclasses or not. However, as a special case, if
      // their names are the same (and if the lhs doesn't contain
      // non-regular classes if the rhs doesn't).
      return (!nonRegularL || nonRegularR) && lname->isame(rname);
    }
    // The lhs is unresolved, but the rhs is resolved. We can use the
    // rhs subclass list to see if the lhs is on it.
    auto const c2 = o.val.right();
    if (lname->isame(c2->cls->name)) {
      // If they represent the same class, the lhs isn't a subtype of
      // the rhs because the lhs is unresolved. The only exception is
      // if the lhs is actually Bottom because it's not regular (and
      // none of it's children are) and we don't want those.
      return
        !nonRegularL &&
        !is_regular_class(*c2->cls) &&
        !c2->hasRegularSubclass;
    }
    if (c2->cls->attrs & AttrTrait) return false;
    // The lhs is unresolved, but the rhs is resolved. Check to see if
    // lhs exists on rhs subclass-list and is not filtered out from
    // being a non-regular class.
    for (auto const sub : c2->subclassList) {
      if (!sub->cls->name->isame(lname)) continue;
      return !nonRegularL || nonRegularR || is_regular_class(*sub->cls);
    }
    return false;
  } else if (auto const rname = o.val.left()) {
    auto const c1 = val.right();
    // The lhs is resolved, but the rhs is not. The lhs is a subtype
    // of rhs if any of its bases have the same name or if it
    // implements rname.

    if (!is_regular_class(*c1->cls)) {
      // If the lhs could contains non-regular types, but the rhs can
      // not, then the lhs cannot ever be a subtype of rhs (because
      // lhs is non-regular itself).
      if (nonRegularL) {
        if (!nonRegularR) return false;
      } else if (!c1->hasRegularSubclass) {
        // If lhs is excluding non-regular classes, and lhs does not
        // have any regular subclasses (and lhs itself is
        // non-regular), then lhs is a Bottom, so always a subtype.
        return true;
      }
    }
    for (auto const base : c1->baseList) {
      if (base->cls->name->isame(rname)) return true;
    }
    if (c1->implInterfaces.count(rname)) return true;
    // The above checks are sufficient if the lhs is regular or if
    // we're considering non-regular classes. Otherwise, we need to
    // check all of the lhs regular children if they implement rname.
    if (is_regular_class(*c1->cls) || nonRegularL) return false;
    return std::all_of(
      begin(c1->subclassList),
      end(c1->subclassList),
      [&] (const ClassInfo* sub) {
        return
          !is_regular_class(*sub->cls) ||
          sub->implInterfaces.count(rname);
      }
    );
  }

  auto const c1 = val.right();
  auto const c2 = o.val.right();

  // If the lhs might contain non-regular types, we'll just do a
  // normal derivedFrom check (there's no distinguishing regular and
  // non-regular classes here). However, if the rhs does not contain
  // non-regular types (or if the lhs doesn't actually contain any),
  // then lhs can't be a subtype of rhs (by definition the lhs has at
  // least one class which can't be in the rhs).
  if (nonRegularL) {
    if (nonRegularR ||
        (is_regular_class(*c1->cls) && !c1->hasNonRegularSubclass)) {
      return c1->derivedFrom(*c2);
    }
    return false;
  }

  if (c1->cls->attrs & AttrInterface) {
    // lhs is an interface. Since this is the "sub" variant, it means
    // any implementation of the interface (not the interface class
    // itself).

    // Ooops, the interface has no regular implementations. This means
    // lhs is a bottom, and a bottom is a subtype of everything.
    if (!c1->hasRegularSubclass) return true;

    // An interface can never be a subtype of a trait.
    if (c2->cls->attrs & AttrTrait) return false;

    auto const& info = c1->interfaceInfo();
    if (c2->cls->attrs & AttrInterface) {
      // If both are interfaces, we can use the InterfaceInfo to see
      // if lhs is a subtype of the rhs.
      return info.subtypeOf.count(c2);
    }

    // lhs is an interface, but rhs is not. The interface can only be
    // a subtype of the non-interface if it has a common base which is
    // a subtype of the rhs.
    return info.commonBase && info.commonBase->derivedFrom(*c2);
  }
  // Since this is the "sub" variant, and we're only considering
  // regular classes, a Trait as the lhs is a bottom (since Traits
  // never have subclasses).
  if (c1->cls->attrs & AttrTrait) return true;

  if (c1->cls->attrs & AttrAbstract) {
    // No regular subclasses of the abstract class. This is a bottom.
    if (!c1->hasRegularSubclass) return true;
    // Do an inheritance check first. If it passes, we're gone. If
    // not, we need to do a more expensive check.
    if (c1->derivedFrom(*c2)) return true;
    // For abstract classes, the inheritance check isn't absolute. To
    // be precise we need to check every (regular) subclass of the
    // abstract class.
    assertx(!c1->subclassList.empty());
    for (auto const sub : c1->subclassList) {
      if (!is_regular_class(*sub->cls)) continue;
      if (!sub->derivedFrom(*c2)) return false;
    }
    return true;
  }

  // If lhs is a regular non-abstract class, we can just use the
  // standard inheritance checks.
  return c1->derivedFrom(*c2);
}

bool Class::exactCouldBeExact(const Class& o,
                              bool nonRegularL,
                              bool nonRegularR) const {
  if (auto const lname = val.left()) {
    if (auto const rname = o.val.left()) return lname->isame(rname);
    // An unresolved lhs and a resolved rhs can only be each other if
    // their name is the same and neither side can become a Bottom.
    auto const c2 = o.val.right();
    return lname->isame(c2->cls->name) &&
      ((nonRegularL && nonRegularR) ||
       is_regular_class(*c2->cls) ||
       c2->hasRegularSubclass);
  } else if (auto const rname = o.val.left()) {
    auto const c1 = val.right();
    return rname->isame(c1->cls->name) &&
      ((nonRegularL && nonRegularR) ||
       is_regular_class(*c1->cls) ||
       c1->hasRegularSubclass);
  } else {
    // Two resolved exact classes can only be each other if they're the
    // same class. The only complication is if the class isn't regular
    // and we're not considering non-regular classes. In that case, the
    // class is actually Bottom, a Bottom can never could-be anything
    // (not even itself).
    auto const c1 = val.right();
    auto const c2 = o.val.right();
    if (c1 != c2) return false;
    auto const bottomL = !nonRegularL && !is_regular_class(*c1->cls);
    auto const bottomR = !nonRegularR && !is_regular_class(*c2->cls);
    return !bottomL && !bottomR;
  }
}

bool Class::exactCouldBe(const Class& o,
                         bool nonRegularL,
                         bool nonRegularR) const {
  if (auto const lname = val.left()) {
    // Two unresolved classes can always potentially be each other.
    if (o.val.left()) return true;
    // The lhs is unresolved but the rhs is resolved. The lhs can only
    // be the rhs if it's on the rhs's subclass list.
    auto const c2 = o.val.right();
    if (lname->isame(c2->cls->name)) {
      // If they're the same name, we still need to check if either
      // side goes to Bottom.
      return (nonRegularL && nonRegularR) || is_regular_class(*c2->cls);
    }
    // Traits don't have subclasses in the sense we care about here.
    if (c2->cls->attrs & AttrTrait) return false;
    for (auto const sub : c2->subclassList) {
      if (!sub->cls->name->isame(lname)) continue;
      // We found the class. If it's not regular and either side wants
      // only regular classes, no match is possible.
      return (nonRegularL && nonRegularR) || is_regular_class(*sub->cls);
    }
    return false;
  }

  // Otherwise the check is very similar to exactSubtypeOf (except for
  // the handling of bottoms).
  auto const c1 = val.right();
  if ((!nonRegularL || !nonRegularR) && !is_regular_class(*c1->cls)) {
    return false;
  }

  if (auto const rname = o.val.left()) {
    // The lhs is resolved, but the rhs is not. The lhs is a subtype
    // of rhs if any of its bases have the same name (and therefore
    // could be).
    for (auto const base : c1->baseList) {
      if (base->cls->name->isame(rname)) return true;
    }
    return c1->implInterfaces.count(rname);
  }

  return c1->derivedFrom(*o.val.right());
}

bool Class::subCouldBe(const Class& o,
                       bool nonRegularL,
                       bool nonRegularR) const {
  // If we only want to consider regular classes on either side. If
  // true, this means that any possible intersection between the
  // classes can only include regular classes. If either side doesn't
  // have any regular classes, then no intersection is possible.
  auto const eitherRegOnly = !nonRegularL || !nonRegularR;

  if (auto const lname = val.left()) {
    // Two unresolved classes can always potentially be each other.
    if (o.val.left()) return true;

    // The lhs is unresolved, and the rhs is resolved. The lhs could
    // be the rhs if the rhs has a subclass which has a parent or
    // implemented interface with the same name as the lhs.
    auto const c2 = o.val.right();
    if (c2->cls->attrs & AttrTrait) {
      if (eitherRegOnly) return false;
      for (auto const base : c2->baseList) {
        if (base->cls->name->isame(lname)) return true;
      }
      return c2->implInterfaces.count(lname);
    }

    for (auto const sub : c2->subclassList) {
      if (eitherRegOnly && !is_regular_class(*sub->cls)) continue;
      for (auto const base : sub->baseList) {
        if (eitherRegOnly && !is_regular_class(*base->cls)) continue;
        if (base->cls->name->isame(lname)) return true;
      }
      if (!eitherRegOnly && sub->implInterfaces.count(lname)) {
        return true;
      }
    }
    return false;
  } else if (auto const rname = o.val.left()) {
    // This is the same as above, but with the lhs and rhs flipped
    // (for the necessary symmetry of couldBe).
    auto const c1 = val.right();
    if (c1->cls->attrs & AttrTrait) {
      if (eitherRegOnly) return false;
      for (auto const base : c1->baseList) {
        if (base->cls->name->isame(rname)) return true;
      }
      return c1->implInterfaces.count(rname);
    }

    for (auto const sub : c1->subclassList) {
      if (eitherRegOnly && !is_regular_class(*sub->cls)) continue;
      for (auto const base : sub->baseList) {
        if (eitherRegOnly && !is_regular_class(*base->cls)) continue;
        if (base->cls->name->isame(rname)) return true;
      }
      if (!eitherRegOnly && sub->implInterfaces.count(rname)) {
        return true;
      }
    }
    return false;
  }

  auto const c1 = val.right();
  auto const c2 = o.val.right();
  if (c1->cls->attrs & AttrInterface) {
    // Check if interface has any regular implementations if that's
    // all we care about.
    if (eitherRegOnly && !c1->hasRegularSubclass) return false;

    if (c2->cls->attrs & AttrInterface) {
      // Do similar implementation check for other side.
      if (eitherRegOnly && !c2->hasRegularSubclass) return false;

      // Both classes are interfaces. The appropriate could-be sets
      // for the interfaces determine if there's any intersection
      // between them. Since couldBe() is symmetric, we can use either
      // interface's set. We arbitrarily use the interface with the
      // smaller subclass list. By forcing any ordering like this, we
      // should reduce the number of could-be sets we need to create.
      auto const smaller = c1->subclassList.size() <= c2->subclassList.size()
        ? c1 : c2;
      auto const larger = c1->subclassList.size() <= c2->subclassList.size()
        ? c2 : c1;

      // First do the check *only* considering regular classes,
      // regardless of what was requested. If this passes, it's always
      // true, so we can skip creating the set which includes
      // non-regular classes.
      if (smaller->couldBe().count(larger)) return true;
      // It didn't pass. If we're only considering regular classes,
      // then there's nothing to check further.
      if (eitherRegOnly) return false;
      // Otherwise there could be non-regular classes in the
      // intersection (but not any regular classes since we ruled that
      // out already). If the interface has no non-regular
      // implementations, the only possible candidate is the interface
      // itself, so do an implements check.
      if (!smaller->hasNonRegularSubclass) {
        return smaller->implInterfaces.count(larger->cls->name);
      }
      // The smaller interface has non-regular implementations. Do the
      // check against it's could-be set which includes non-regular
      // classes.
      return smaller->couldBeNonRegular().count(larger);
    }
    if (c2->cls->attrs & AttrTrait) {
      // An interface and a trait can only intersect if the trait
      // implements the interface, and we're including non-regular
      // classes (since a trait is always a single non-regular class).
      return !eitherRegOnly && c2->implInterfaces.count(c1->cls->name);
    }

    // c2 is either a normal class or an abstract class:

    if (eitherRegOnly && !c2->hasRegularSubclass) {
      // c2 doesn't have any regular subclasses and only regular class
      // intersections have been requested. If c2 is abstract, no
      // intersection is possible. Otherwise the only intersection is
      // c2 itself, so do an implements check against that.
      if (c2->cls->attrs & AttrAbstract) return false;
      return c2->implInterfaces.count(c1->cls->name);
    }
    // First do the check *only* considering regular classes,
    // regardless of what was requested. If this passes, it's always
    // true, so we don't need to do any further checking.
    if (c1->couldBe().count(c2)) return true;
    // No intersection considering just regular classes. If the
    // intersection can only contain regular classes, or if the
    // interface has no regular implementations, we know there's no
    // intersection at all.
    if (eitherRegOnly || !c1->hasNonRegularSubclass) return false;
    // Otherwise check against the interface's could-be set which
    // includes non-regular classes.
    return c1->couldBeNonRegular().count(c2);
  }

  if (c2->cls->attrs & AttrInterface) {
    // Check if interface contains at least one regular subclass if
    // that's all we care about.
    if (eitherRegOnly && !c2->hasRegularSubclass) return false;

    // c1 cannot be an interface because we already checked that
    // above.

    if (c1->cls->attrs & AttrTrait) {
      // A trait only intersects an interface if the trait implements
      // the interface. Traits are non-regular and have no subclasses,
      // so if we only want regular classes in the intersection, there
      // is no intersection.
      return !eitherRegOnly && c1->implInterfaces.count(c2->cls->name);
    }

    // c1 is either a normal class or an abstract class:

    if (!nonRegularL && !c1->hasRegularSubclass) {
      // c1 doesn't have any regular subclasses and only regular class
      // intersections have been requested. If c1 is abstract, no
      // intersection is possible. Otherwise the only intersection is
      // c1 itself, so do an implements check against that.
      if (c1->cls->attrs & AttrAbstract) return false;
      return c1->implInterfaces.count(c2->cls->name);
    }
    // First do the check *only* considering regular classes,
    // regardless of what was requested. If this passes, it's always
    // true, so we don't need to do any further checking.
    if (c2->couldBe().count(c1)) return true;
    // No intersection considering just regular classes. If the
    // intersection can only contain regular classes, or if the
    // interface has no regular implementations, we know there's no
    // intersection at all.
    if (eitherRegOnly || !c2->hasNonRegularSubclass) return false;
    // Otherwise check against the interface's could-be set which
    // includes non-regular classes.
    return c2->couldBeNonRegular().count(c1);
  }

  // A trait can only intersect with itself, and only if we're
  // including non-regular classes in the intersection.
  if (c1->cls->attrs & AttrTrait) return !eitherRegOnly && c1 == c2;
  if (c2->cls->attrs & AttrTrait) return false;

  // Check if either class only contains non-regular subclasses and
  // we're only looking for regular intersections.
  if (eitherRegOnly) {
    if ((c1->cls->attrs & AttrAbstract) && !c1->hasRegularSubclass) {
      return false;
    }
    if ((c2->cls->attrs & AttrAbstract) && !c2->hasRegularSubclass) {
      return false;
    }
  }

  // Both types are non-interfaces so they "could be" if they are in
  // an inheritance relationship.
  if (c1->baseList.size() >= c2->baseList.size()) {
    return c1->baseList[c2->baseList.size() - 1] == c2;
  } else {
    return c2->baseList[c1->baseList.size() - 1] == c1;
  }
}

SString Class::name() const {
  return val.match(
    [] (SString s) { return s; },
    [] (ClassInfo* ci) { return ci->cls->name.get(); }
  );
}

Optional<res::Class> Class::withoutNonRegular() const {
  return val.match(
    [&] (SString) -> Optional<res::Class> { return *this; },
    [&] (ClassInfo* cinfo) -> Optional<res::Class> {
      // Regular classes are always unchanged
      if (is_regular_class(*cinfo->cls)) return *this;
      // Non-regular class with no regular subclasses just becomes Bottom
      if (!cinfo->hasRegularSubclass) return std::nullopt;
      // Traits can have things on their subclass list (any
      // unflattened users), but still becomes Bottom.
      if (cinfo->cls->attrs & AttrTrait) return std::nullopt;
      if (!(cinfo->cls->attrs & (AttrInterface | AttrAbstract))) return *this;
      // Interfaces or abstract classes need to be canonicalized to
      // their equivalent (which may be themself).
      return Class {
        const_cast<ClassInfo*>(cinfo->withoutNonRegularEquivalent())
      };
    }
  );
}

bool Class::mightBeRegular() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) { return is_regular_class(*cinfo->cls); }
  );
}

bool Class::mightBeNonRegular() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) { return !is_regular_class(*cinfo->cls); }
  );
}

bool Class::couldBeOverridden() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return !(cinfo->cls->attrs & (AttrTrait|AttrNoOverride));
    }
  );
}

bool Class::couldBeOverriddenByRegular() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return !(cinfo->cls->attrs & (AttrTrait|AttrNoOverrideRegular));
    }
  );
}

bool Class::mightContainNonRegular() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return
        !is_regular_class(*cinfo->cls) || cinfo->hasNonRegularSubclass;
    }
  );
}

bool Class::couldHaveMagicBool() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      if (cinfo->cls->attrs & AttrInterface) {
        assertx(!cinfo->subclassList.empty());
        for (auto const sub : cinfo->subclassList) {
          if (has_magic_bool_conversion(sub->baseList[0]->cls->name)) {
            return true;
          }
        }
        return false;
      }
      return has_magic_bool_conversion(cinfo->baseList[0]->cls->name);
    }
  );
}

bool Class::couldHaveMockedSubClass() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return cinfo->isSubMocked;
    }
  );
}

bool Class::couldBeMocked() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return cinfo->isMocked;
    }
  );
}

bool Class::couldHaveReifiedGenerics() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return cinfo->cls->hasReifiedGenerics;
    }
  );
}

bool Class::mustHaveReifiedGenerics() const {
  return val.match(
    [] (SString) { return false; },
    [] (ClassInfo* cinfo) {
      return cinfo->cls->hasReifiedGenerics;
    }
  );
}

bool Class::couldHaveReifiedParent() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return cinfo->hasReifiedParent;
    }
  );
}

bool Class::mustHaveReifiedParent() const {
  return val.match(
    [] (SString) { return false; },
    [] (ClassInfo* cinfo) {
      return cinfo->hasReifiedParent;
    }
  );
}

bool Class::mightCareAboutDynConstructs() const {
  if (RuntimeOption::EvalForbidDynamicConstructs > 0) {
    return val.match(
      [] (SString) { return true; },
      [] (ClassInfo* cinfo) {
        return !(cinfo->cls->attrs & AttrDynamicallyConstructible);
      }
    );
  }
  return false;
}

bool Class::couldHaveConstProp() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) { return cinfo->hasConstProp; }
  );
}

bool Class::subCouldHaveConstProp() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) { return cinfo->subHasConstProp; }
  );
}
Optional<res::Class> Class::parent() const {
  if (!val.right()) return std::nullopt;
  auto parent = val.right()->parent;
  if (!parent) return std::nullopt;
  if (is_closure_base(*parent->cls)) {
    return res::Class { parent->cls->name.get() };
  }
  return res::Class { parent };
}

const php::Class* Class::cls() const {
  return val.right() ? val.right()->cls : nullptr;
}

void
Class::forEachSubclass(const std::function<void(const php::Class*)>& f) const {
  auto const cinfo = val.right();
  assertx(cinfo);
  assertx(!cinfo->subclassList.empty());
  for (auto const& s : cinfo->subclassList) f(s->cls);
}

std::string show(const Class& c) {
  return c.val.match(
    [] (SString s) {
      return folly::sformat("\"{}\"", s);
    },
    [] (ClassInfo* cinfo) {
      return cinfo->cls->name->toCppString();
    }
  );
}

ClassInfo* Class::commonAncestor(ClassInfo* a, ClassInfo* b) {
  if (a == b) return a;
  if (!a || !b) return nullptr;
  ClassInfo* ancestor = nullptr;
  // Walk the arrays of base classes until they match. For common
  // ancestors to exist they must be on both sides of the baseList
  // at the same positions
  auto it1 = a->baseList.begin();
  auto it2 = b->baseList.begin();
  while (it1 != a->baseList.end() && it2 != b->baseList.end()) {
    if (*it1 != *it2) break;
    ancestor = *it1;
    ++it1; ++it2;
  }
  return ancestor;
}

// Call the given callable for every class which is a subclass of
// *all* the classes in the range. If the range includes nothing but
// unresolved classes, they will be passed, as-is, to the callable. If
// the range includes a mix of resolved and unresolved classes, the
// unresolved classes will be used to narrow the classes passed to the
// callable, but the unresolved classes themself will not be passed to
// the callable. If the callable returns false, iteration is
// stopped. If includeNonRegular is true, non-regular subclasses are
// visited (normally they are skipped). The callable also receives a
// bool which indicates whether the class passed to the callable
// represents an exact type or a sub type. The only case where the
// type can be a sub type is if all classes are unresolved.
template <typename F>
void Class::visitEverySub(folly::Range<const Class*> classes,
                          bool includeNonRegular,
                          const F& f) {
  assertx(!classes.empty());

  // Simple case: if there's only one class, just iterate over the
  // subclass list.
  if (classes.size() == 1) {
    auto const cinfo = classes.front().val.right();
    if (!cinfo) {
      f(classes.front(), false);
    } else if (cinfo->cls->attrs & AttrTrait) {
      if (includeNonRegular) f(Class { cinfo }, true);
    } else {
      assertx(!cinfo->subclassList.empty());
      for (auto const sub : cinfo->subclassList) {
        if (!includeNonRegular && !is_regular_class(*sub->cls)) continue;
        if (!f(Class { sub }, true)) break;
      }
    }
    return;
  }

  // Otherwise we need to find all of the classes in common:
  CompactVector<ClassInfo*> common;

  // Find the first resolved class, and use that to initialize the
  // list of subclasses.
  auto const numClasses = classes.size();
  size_t resolvedIdx = 0;
  while (resolvedIdx < numClasses) {
    auto const cinfo = classes[resolvedIdx].val.right();
    if (!cinfo) {
      ++resolvedIdx;
      continue;
    }

    if (cinfo->cls->attrs & AttrTrait) {
      if (includeNonRegular) common.emplace_back(cinfo);
    } else if (includeNonRegular ||
               (!cinfo->hasNonRegularSubclass &&
                is_regular_class(*cinfo->cls))) {
      common = cinfo->subclassList;
    } else {
      for (auto const sub : cinfo->subclassList) {
        if (!is_regular_class(*sub->cls)) continue;
        common.emplace_back(sub);
      }
    }
    break;
  }

  // We didn't find any resolved classes. This list is nothing but
  // unresolved classes, so just provide them to the callable and then
  // we're done.
  if (resolvedIdx == numClasses) {
    assertx(common.empty());
    for (auto const c : classes) {
      assertx(c.val.left());
      if (!f(c, false)) break;
    }
    return;
  }

  // Otherwise we found a resolved class. Now process the rest of the
  // resolved classes, removing any subclasses from the list which
  // aren't a subclass of all of the classes.
  CompactVector<ClassInfo*> newCommon;
  // We start again from 0 to process any unresolved classes we might
  // have skipped over above.
  for (size_t idx = 0; idx < numClasses; ++idx) {
    assertx(!common.empty());
    // Don't process the class we selected above twice.
    if (idx == resolvedIdx) continue;
    newCommon.clear();

    // NB: We don't need to check includeNonRegular here. If it's
    // false, we won't have any non-regular classes in common
    // initially, so none will be part of any intersection.
    if (auto const cinfo = classes[idx].val.right()) {
      // If this class is resolved, intersect the subclass list with
      // the common set of classes.
      assertx(idx > resolvedIdx);
      std::set_intersection(
        begin(common),
        end(common),
        begin(cinfo->subclassList),
        end(cinfo->subclassList),
        std::back_inserter(newCommon)
      );
    } else {
      // If this class is unresolved, we can remove any classes from
      // the common set which couldn't be the unresolved class.
      for (auto const c : common) {
        Class resolved{ c };
        if (resolved.exactCouldBe(classes[idx],
                                  includeNonRegular,
                                  includeNonRegular)) {
          newCommon.emplace_back(c);
        }
      }
    }
    std::swap(common, newCommon);
    if (common.empty()) return;
  }
  assertx(!common.empty());

  // We have the final list. Iterate over these and report them to the
  // callable.
  for (auto const c : common) {
    assertx(IMPLIES(!includeNonRegular, is_regular_class(*c->cls)));
    if (!f(Class { c }, true)) return;
  }
}

// Given a list of classes, put them in canonical form for a
// DCls::IsectSet. It is assumed that couldBe is true between all of
// the classes in the list, but nothing is assumed otherwise.
TinyVector<Class, 2> Class::canonicalizeIsects(const TinyVector<Class, 8>& in,
                                               bool nonRegular) {
  auto const size = in.size();
  if (size == 0) return {};
  if (size < 2) return { in.front() };

  // Canonical ordering:
  auto const compare = [] (Class a, Class b) {
    auto const c1 = a.val.right();
    auto const c2 = b.val.right();
    // Resolved classes always come before unresolved classes.
    if (!c1) {
      if (c2) return 1;
      // Two unresolved classes are just compared by name.
      return a.val.left()->compare(b.val.left());
    } else if (!c2) {
      return -1;
    }

    // "Smaller" classes (those with less subclasses) should come
    // first.
    auto const s1 = c1->subclassList.size();
    auto const s2 = c2->subclassList.size();
    if (s1 < s2) return -1;
    if (s1 > s2) return 1;

    // Regular classes come first, followed by abstract classes,
    // interfaces, then traits.
    auto const weight = [] (const ClassInfo* c) {
      if (c->cls->attrs & AttrAbstract) return 1;
      if (c->cls->attrs & AttrInterface) return 2;
      if (c->cls->attrs & AttrTrait) return 3;
      return 0;
    };
    auto const w1 = weight(c1);
    auto const w2 = weight(c2);
    if (w1 < w2) return -1;
    if (w1 > w2) return 1;

    // All else being equal, compare the name.
    return c1->cls->name->compare(c2->cls->name);
  };

  // Remove any class which is a superclass of another. Such classes
  // are redundant because there's a "smaller" class which already
  // implies it. This also gets rid of duplicates. This is a naive
  // O(N^2) algorithm but it's fine because the lists do not get very
  // large at all.
  TinyVector<Class, 2> out;
  for (int i = 0; i < size; ++i) {
    // For every pair of classes:
    auto const c1 = in[i];
    auto const subtypeOf = [&] {
      for (int j = 0; j < size; ++j) {
        auto const c2 = in[j];
        if (i == j || !c2.subSubtypeOf(c1, nonRegular, nonRegular)) continue;
        // c2 is a subtype of c1. If c1 is not a subtype of c2, then
        // c2 is preferred and we return true to drop c1.
        if (!c1.subSubtypeOf(c2, nonRegular, nonRegular)) return true;
        // They're both subtypes of each other, so they're actually
        // equivalent. We only want to keep one, so use the sorting
        // order and keep the "lesser" one.
        auto const cmp = compare(c1, c2);
        if (cmp > 0 || (cmp == 0 && i > j)) return true;
      }
      return false;
    }();
    if (!subtypeOf) out.emplace_back(c1);
  }

  // Finally sort the list
  std::sort(
    out.begin(),
    out.end(),
    [&] (Class a, Class b) { return compare(a, b) < 0; }
  );
  return out;
}

TinyVector<Class, 2> Class::combine(folly::Range<const Class*> classes1,
                                    folly::Range<const Class*> classes2,
                                    bool isSub1,
                                    bool isSub2,
                                    bool nonRegular1,
                                    bool nonRegular2) {
  TinyVector<Class, 8> common;
  Optional<ClassInfo*> commonBase;
  Optional<hphp_fast_set<ClassInfo*>> commonInterfaces;

  // The algorithm for unioning together two intersection lists is
  // simple. For every class which is "in" either the first or second
  // list, track the interfaces which are implemented by all of the
  // classes, and the common base class amongst all of them. Build a
  // list of these classes and normalize them.

  auto const processNormal = [&] (ClassInfo* cinfo) {
    // NB: isSub and nonRegular is irrelevant here... Everything that
    // we look at in the base class here must also be true for all of
    // its children.

    // Set commonBase, or update it depending on whether this is the
    // first class processed.
    if (!commonBase) {
      commonBase = cinfo;
    } else {
      commonBase = commonAncestor(*commonBase, cinfo);
    }

    // Likewise, initialize the set of common interfaces, or remove
    // any which aren't present.
    if (!commonInterfaces) {
      commonInterfaces.emplace();
      for (auto const i : cinfo->implInterfaces) {
        commonInterfaces->emplace(const_cast<ClassInfo*>(i.second));
      }
    } else {
      folly::erase_if(
        *commonInterfaces,
        [&] (ClassInfo* i) {
          return !cinfo->implInterfaces.count(i->cls->name);
        }
      );
    }
  };

  // Process an interface's implementations (but not the interface
  // itself).
  auto const processIface = [&] (ClassInfo* cinfo) {
    assertx(cinfo->cls->attrs & AttrInterface);
    auto& info = cinfo->interfaceInfo();

    // We assume !nonRegular here since if it was true, we'd be
    // processing the interface in processNormal(). info.subtypeOf and
    // info.commonBase are calculated ignoring non-regular
    // implementations.

    // The logic for processing an interface is similar to processSub,
    // except we use the interface's common base (if any).
    auto const ifaceCommon = const_cast<ClassInfo*>(info.commonBase);
    if (!commonBase) {
      commonBase = ifaceCommon;
    } else {
      commonBase = commonAncestor(*commonBase, ifaceCommon);
    }

    // Instead of implInterfaces, we use the set of interfaces it is a
    // subtype of.
    if (!commonInterfaces) {
      commonInterfaces.emplace();
      for (auto const i : info.subtypeOf) {
        commonInterfaces->emplace(const_cast<ClassInfo*>(i));
      }
      commonInterfaces->emplace(cinfo);
    } else {
      folly::erase_if(
        *commonInterfaces,
        [&] (ClassInfo* i) { return i != cinfo && !info.subtypeOf.count(i); }
      );
    }
  };

  auto const processList = [&] (folly::Range<const Class*> classes,
                                bool isSub,
                                bool nonRegular) {
    if (classes.size() == 1) {
      // If the list is just a single class, we can process things
      // more efficiently.
      auto const cinfo = classes[0].val.right();
      // We dealt with lists of all unresolved classes specially
      // below, so shouldn't get here.
      assertx(cinfo);
      if (cinfo->cls->attrs & (AttrAbstract|AttrInterface)) {
        // Are we including non-regular classes? If we are, we can
        // process this like any other.
        if (nonRegular) {
          processNormal(cinfo);
          return;
        }
        // We're non-regular. Do we care about sub-classes? If not,
        // there's nothing more to do. We're not processing this
        // class, nor its sub-classes.
        if (!isSub) return;
        // Otherwise we're not processing the base class, but we are
        // its sub-classes. For interfaces we can deal with this
        // specially.
        if (cinfo->cls->attrs & AttrInterface) {
          processIface(cinfo);
          return;
        }
        // For abstract classes, however, we'll fall through and use
        // visitEverySub.
      } else if (cinfo->cls->attrs & AttrTrait) {
        // Traits have no subclasses, so isSub doesn't matter. Process
        // it if we're including non-regular classes.
        if (nonRegular) processNormal(cinfo);
        return;
      } else {
        // A regular class. Always process it.
        processNormal(cinfo);
        return;
      }
    }

    // The list has multiple classes or we have an abstract class and
    // we only care about its subclasses. This is more expensive, we
    // need to visit every subclass in the intersection of the classes
    // on the list.
    visitEverySub(
      classes,
      nonRegular,
      [&] (res::Class c, bool isExact) {
        // visitEverySub will only report an unresolved class if the
        // entire list is unresolved, and we deal with that case
        // specially below and shouldn't get here.
        assertx(c.val.right());
        assertx(isExact);
        // We'll only "visit" exact sub-classes, so only use
        // processNormal here.
        processNormal(c.val.right());
        // No point in continuing if there's nothing in common left.
        return *commonBase || !commonInterfaces->empty();
      }
    );
  };

  assertx(!classes1.empty());
  assertx(!classes2.empty());
  assertx(IMPLIES(!isSub1, classes1.size() == 1));
  assertx(IMPLIES(!isSub2, classes2.size() == 1));

  // If either side is composed of nothing but unresolved classes, we
  // need to deal with that specially (because we cannot know their
  // subclasses, the above logic doesn't work). If either side has
  // *some* (but not all) unresolved classes, that is fine, because
  // visitEverySub will handle that for us.
  auto const allUnresolved1 = std::all_of(
    classes1.begin(), classes1.end(),
    [] (res::Class c) { return (bool)c.val.left(); }
  );
  auto const allUnresolved2 = std::all_of(
    classes2.begin(), classes2.end(),
    [] (res::Class c) { return (bool)c.val.left(); }
  );

  if (!allUnresolved1 && !allUnresolved2) {
    // There's resolved classes on both sides. We can use the normal
    // process logic.
    processList(classes1, isSub1, nonRegular1);
    processList(classes2, isSub2, nonRegular2);
    // Combine the common classes
    if (commonBase && *commonBase) {
      // Don't process resolved classes for Closure.
      if (!is_closure_base(*(*commonBase)->cls)) {
        common.emplace_back(Class { *commonBase });
      } else {
        assertx(!commonInterfaces || commonInterfaces->empty());
        common.emplace_back(Class { (*commonBase)->cls->name.get() });
      }
    }
    if (commonInterfaces) {
      for (auto const i : *commonInterfaces) {
        common.emplace_back(Class { i });
      }
    }
  } else {
    // Either side (maybe both) is made up of unresolved
    // classes. Instead of the above subclass based logic, only keep
    // the classes (on either side) which are a subtype of a class on
    // the opposite side.
    auto const either = nonRegular1 || nonRegular2;
    for (auto const c1 : classes1) {
      auto const subtypeOf = std::any_of(
        classes2.begin(), classes2.end(),
        [&] (res::Class c2) {
          return isSub2
            ? c2.subSubtypeOf(c1, either, either)
            : c2.exactSubtypeOf(c1, either, either);
        }
      );
      if (subtypeOf) common.emplace_back(c1);
    }
    for (auto const c2 : classes2) {
      auto const subtypeOf = std::any_of(
        classes1.begin(), classes1.end(),
        [&] (res::Class c1) {
          return isSub1
            ? c1.subSubtypeOf(c2, either, either)
            : c1.exactSubtypeOf(c2, either, either);
        }
      );
      if (subtypeOf) common.emplace_back(c2);
    }
  }

  // Finally canonicalize the set
  return canonicalizeIsects(common, nonRegular1 || nonRegular2);
}

TinyVector<Class, 2>
Class::removeNonRegular(folly::Range<const Class*> classes) {
  TinyVector<Class, 8> common;
  Optional<ClassInfo*> commonBase;
  Optional<hphp_fast_set<ClassInfo*>> commonInterfaces;

  // Iterate over every exact member of the class list, filtering out
  // non-regular classes, and rebuild the common base and common
  // interface.
  visitEverySub(
    classes,
    false,
    [&] (res::Class c, bool isExact) {
      // Unresolved classes are always "regular" (we can't tell
      // otherwise), so they remain as is.
      if (c.val.left()) {
        common.emplace_back(c);
        return true;
      }

      // Must have a cinfo because we checked above all the classes
      // were resolved.
      auto const cinfo = c.val.right();
      assertx(cinfo);
      assertx(isExact);
      assertx(is_regular_class(*cinfo->cls));

      if (!commonBase) {
        commonBase = cinfo;
      } else {
        commonBase = commonAncestor(*commonBase, cinfo);
      }

      if (!commonInterfaces) {
        commonInterfaces.emplace();
        for (auto const i : cinfo->implInterfaces) {
          commonInterfaces->emplace(const_cast<ClassInfo*>(i.second));
        }
      } else {
        folly::erase_if(
          *commonInterfaces,
          [&] (ClassInfo* i) {
            return !cinfo->implInterfaces.count(i->cls->name);
          }
        );
      }

      // Stop iterating if there's no longer anything in common.
      return *commonBase || !commonInterfaces->empty();
    }
  );

  if (commonBase && *commonBase) {
    common.emplace_back(Class { *commonBase });
  }
  if (commonInterfaces) {
    for (auto const i : *commonInterfaces) {
      common.emplace_back(Class { i });
    }
  }

  // Canonicalize the common base classes/interfaces.
  return canonicalizeIsects(common, false);
}

TinyVector<Class, 2> Class::intersect(folly::Range<const Class*> classes1,
                                      folly::Range<const Class*> classes2,
                                      bool nonRegular1,
                                      bool nonRegular2,
                                      bool& nonRegularOut) {
  TinyVector<Class, 8> common;

  // The algorithm for intersecting two intersection lists is similar
  // to unioning, except we only need to consider the classes which
  // are subclasses of all the classes in *both* lists.

  assertx(!classes1.empty());
  assertx(!classes2.empty());

  auto const bothNonRegular = nonRegular1 && nonRegular2;
  nonRegularOut = bothNonRegular;

  // Even if both the lhs and rhs contain non-regular classes, the
  // intersection may not. We check if the intersection contains any
  // non-regular classes so we can inform the caller to set up the
  // type appropriately.
  auto isectContainsNonRegular = false;

  // Estimate the sizes of each side by summing their subclass list
  // lengths. We want to iterate over the "smaller" set of classes.
  size_t size1 = 0;
  size_t size2 = 0;
  for (auto const c : classes1) {
    if (auto const cinfo = c.val.right()) {
      assertx(!cinfo->subclassList.empty());
      size1 += cinfo->subclassList.size();
    }
  }
  for (auto const c : classes2) {
    if (auto const cinfo = c.val.right()) {
      assertx(!cinfo->subclassList.empty());
      size2 += cinfo->subclassList.size();
    }
  }

  auto const process = [&] (folly::Range<const Class*> lhs,
                            folly::Range<const Class*> rhs) {
    Optional<ClassInfo*> commonBase;
    Optional<hphp_fast_set<ClassInfo*>> commonInterfaces;

    // Since we're calculating the intersection, we only have to visit
    // one list, and check against the other.
    visitEverySub(
      lhs,
      bothNonRegular,
      [&] (res::Class c, bool isExact) {
        auto const cinfo = c.val.right();
        // We shouldn't use visitEverySub if the class list is nothing
        // but unresolved classes, so we should never get an
        // unresolved class in the callback.
        assertx(cinfo);
        assertx(isExact);
        assertx(IMPLIES(!bothNonRegular, is_regular_class(*cinfo->cls)));

        // Could this class be a class in the other list? If not, ignore
        // it (it's not part of the intersection result).
        for (auto const other : rhs) {
          if (!c.exactCouldBe(other, bothNonRegular, bothNonRegular)) {
            return true;
          }
        }

        // Otherwise it is part of the intersection, and we need to
        // update the common base and interfaces likewise.

        if (!commonBase) {
          commonBase = cinfo;
        } else {
          commonBase = commonAncestor(*commonBase, cinfo);
        }

        if (!commonInterfaces) {
          commonInterfaces.emplace();
          for (auto const i : cinfo->implInterfaces) {
            commonInterfaces->emplace(const_cast<ClassInfo*>(i.second));
          }
        } else {
          folly::erase_if(
            *commonInterfaces,
            [&] (ClassInfo* i) {
              return !cinfo->implInterfaces.count(i->cls->name);
            }
          );
        }

        if (bothNonRegular &&
            !isectContainsNonRegular &&
            !is_regular_class(*cinfo->cls)) {
          isectContainsNonRegular = true;
        }

        // Stop iterating if there's no longer anything in common.
        return *commonBase || !commonInterfaces->empty();
      }
    );

    if (commonBase && *commonBase) {
      common.emplace_back(Class { *commonBase });
    }
    if (commonInterfaces) {
      for (auto const i : *commonInterfaces) {
        common.emplace_back(Class { i });
      }
    }

    // If the common set is empty at this point, the intersection is
    // empty anyways, so we don't need to worry about any unresolved
    // classes. Otherwise add unresolved classes on both sides to the
    // common set. Canonicalization will remove them if they're
    // redundant.
    if (!common.empty()) {
      for (auto const c : classes1) {
        if (c.val.left()) common.emplace_back(c);
      }
      for (auto const c : classes2) {
        if (c.val.left()) common.emplace_back(c);
      }
    }
  };

  // The first parameter is the class range we'll call visitEverySub
  // on, so use the smaller of the two ranges. Don't use a range with
  // a "size" of 0, which means it's nothing but unresolved classes.
  if (size1 == 0) {
    if (size2 > 0) {
      process(classes2, classes1);
    } else {
      // If both ranges are nothing but unresolved classes, we don't
      // need to process them at all. The intersection is just the
      // combined list of classes (canonicalization will remove any
      // redundancies).
      for (auto const c : classes1) {
        if (c.val.left()) common.emplace_back(c);
      }
      for (auto const c : classes2) {
        if (c.val.left()) common.emplace_back(c);
      }
      isectContainsNonRegular = bothNonRegular;
    }
  } else if (size2 == 0 || size1 <= size2) {
    process(classes1, classes2);
  } else {
    process(classes2, classes1);
  }

  // Canonicalize the common base classes/interfaces.
  assertx(IMPLIES(!bothNonRegular, !isectContainsNonRegular));
  nonRegularOut = isectContainsNonRegular;
  return canonicalizeIsects(common, isectContainsNonRegular);
}

bool Class::couldBeIsect(folly::Range<const Class*> classes1,
                         folly::Range<const Class*> classes2,
                         bool nonRegular1,
                         bool nonRegular2) {
  assertx(!classes1.empty());
  assertx(!classes2.empty());

  auto const bothNonReg = nonRegular1 && nonRegular2;

  // Decompose the first class list into each of it's exact
  // subclasses, and do a could-be check against every class on the
  // second list. This is precise since the lhs is always exact.
  auto couldBe = false;
  visitEverySub(
    classes1,
    bothNonReg,
    [&] (res::Class c, bool isExact) {
      couldBe = std::all_of(
        classes2.begin(),
        classes2.end(),
        [&] (Class c2) {
          return isExact
            ? c.exactCouldBe(c2, bothNonReg, bothNonReg)
            : c.subCouldBe(c2, bothNonReg, bothNonReg);
        }
      );
      return !couldBe;
    }
  );
  return couldBe;
}

Class Class::unresolvedWaitHandle() {
  return Class { s_Awaitable.get() };
}

//////////////////////////////////////////////////////////////////////

Func::Func(Rep val)
  : val(val)
{}

SString Func::name() const {
  return match<SString>(
    val,
    [&] (FuncName s)   { return s.name; },
    [&] (MethodName s) { return s.name; },
    [&] (Fun f)        { return f.finfo->func->name; },
    [&] (Fun2 f)       { return f.finfo->name; },
    [&] (Method m)     { return m.finfo->func->name; },
    [&] (Method2 m)    { return m.finfo->name; },
    [&] (MethodFamily fam) {
      return fam.family->possibleFuncs().front().ptr()->name;
    },
    [&] (MethodFamily2 fam) { return fam.family->m_name; },
    [&] (MethodOrMissing m) { return m.finfo->func->name; },
    [&] (MethodOrMissing2 m) { return m.finfo->name; },
    [&] (Missing m) { return m.name; },
    [&] (const Isect& i) {
      assertx(i.families.size() > 1);
      return i.families[0]->possibleFuncs().front().ptr()->name;
    },
    [&] (const Isect2& i) {
      assertx(i.families.size() > 1);
      return i.families[0]->m_name;
    }
  );
}

const php::Func* Func::exactFunc() const {
  using Ret = const php::Func*;
  return match<Ret>(
    val,
    [&](FuncName)                    { return Ret{}; },
    [&](MethodName)                  { return Ret{}; },
    [&](Fun f)                       { return f.finfo->func; },
    [&](Fun2 f)                      { return f.finfo->func; },
    [&](Method m)                    { return m.finfo->func; },
    [&](Method2 m)                   { return m.finfo->func; },
    [&](MethodFamily)                { return Ret{}; },
    [&](MethodFamily2)               { return Ret{}; },
    [&](MethodOrMissing)             { return Ret{}; },
    [&](MethodOrMissing2)            { return Ret{}; },
    [&](Missing)                     { return Ret{}; },
    [&](const Isect&)                { return Ret{}; },
    [&](const Isect2&)               { return Ret{}; }
  );
}

TriBool Func::exists() const {
  return match<TriBool>(
    val,
    [&](FuncName)                    { return TriBool::Maybe; },
    [&](MethodName)                  { return TriBool::Maybe; },
    [&](Fun)                         { return TriBool::Yes; },
    [&](Fun2)                        { return TriBool::Yes; },
    [&](Method)                      { return TriBool::Yes; },
    [&](Method2)                     { return TriBool::Yes; },
    [&](MethodFamily)                { return TriBool::Maybe; },
    [&](MethodFamily2)               { return TriBool::Maybe; },
    [&](MethodOrMissing)             { return TriBool::Maybe; },
    [&](MethodOrMissing2)            { return TriBool::Maybe; },
    [&](Missing)                     { return TriBool::No; },
    [&](const Isect&)                { return TriBool::Maybe; },
    [&](const Isect2&)               { return TriBool::Maybe; }
  );
}

bool Func::isFoldable() const {
  return match<bool>(
    val,
    [&](FuncName)   { return false; },
    [&](MethodName) { return false; },
    [&](Fun f) {
      return f.finfo->func->attrs & AttrIsFoldable;
    },
    [&](Fun2 f) {
      return f.finfo->func->attrs & AttrIsFoldable;
    },
    [&](Method m)  { return m.finfo->func->attrs & AttrIsFoldable; },
    [&](Method2 m) { return m.finfo->func->attrs & AttrIsFoldable; },
    [&](MethodFamily)    { return false; },
    [&](MethodFamily2)   { return false; },
    [&](MethodOrMissing) { return false; },
    [&](MethodOrMissing2){ return false; },
    [&](Missing)         { return false; },
    [&](const Isect&)    { return false; },
    [&](const Isect2&)   { return false; }
  );
}

bool Func::couldHaveReifiedGenerics() const {
  return match<bool>(
    val,
    [&](FuncName s) { return true; },
    [&](MethodName) { return true; },
    [&](Fun f) { return f.finfo->func->isReified; },
    [&](Fun2 f) { return f.finfo->func->isReified; },
    [&](Method m) { return m.finfo->func->isReified; },
    [&](Method2 m) { return m.finfo->func->isReified; },
    [&](MethodFamily fa) {
      return fa.family->infoFor(fa.regularOnly).m_static->m_maybeReified;
    },
    [&](MethodFamily2 fa) {
      return fa.family->infoFor(fa.regularOnly).m_maybeReified;
    },
    [&](MethodOrMissing m) { return m.finfo->func->isReified; },
    [&](MethodOrMissing2 m) { return m.finfo->func->isReified; },
    [&](Missing) { return false; },
    [&](const Isect& i) {
      for (auto const ff : i.families) {
        if (!ff->infoFor(i.regularOnly).m_static->m_maybeReified) return false;
      }
      return true;
    },
    [&](const Isect2& i) {
      for (auto const ff : i.families) {
        if (!ff->infoFor(i.regularOnly).m_maybeReified) return false;
      }
      return true;
    }
  );
}

bool Func::mightCareAboutDynCalls() const {
  if (RuntimeOption::EvalNoticeOnBuiltinDynamicCalls && mightBeBuiltin()) {
    return true;
  }
  auto const mightCareAboutFuncs =
    RuntimeOption::EvalForbidDynamicCallsToFunc > 0;
  auto const mightCareAboutInstMeth =
    RuntimeOption::EvalForbidDynamicCallsToInstMeth > 0;
  auto const mightCareAboutClsMeth =
    RuntimeOption::EvalForbidDynamicCallsToClsMeth > 0;

  return match<bool>(
    val,
    [&](FuncName) { return mightCareAboutFuncs; },
    [&](MethodName) {
      return mightCareAboutClsMeth || mightCareAboutInstMeth;
    },
    [&](Fun f) {
      return dyn_call_error_level(f.finfo->func) > 0;
    },
    [&](Fun2 f) {
      return dyn_call_error_level(f.finfo->func) > 0;
    },
    [&](Method m)  { return dyn_call_error_level(m.finfo->func) > 0; },
    [&](Method2 m) { return dyn_call_error_level(m.finfo->func) > 0; },
    [&](MethodFamily fa) {
      return
        fa.family->infoFor(fa.regularOnly).m_static->m_maybeCaresAboutDynCalls;
    },
    [&](MethodFamily2 fa) {
      return
        fa.family->infoFor(fa.regularOnly).m_maybeCaresAboutDynCalls;
    },
    [&](MethodOrMissing m)  { return dyn_call_error_level(m.finfo->func) > 0; },
    [&](MethodOrMissing2 m) { return dyn_call_error_level(m.finfo->func) > 0; },
    [&](Missing m) { return false; },
    [&](const Isect& i) {
      for (auto const ff : i.families) {
        if (!ff->infoFor(i.regularOnly).m_static->m_maybeCaresAboutDynCalls) {
          return false;
        }
      }
      return true;
    },
    [&](const Isect2& i) {
      for (auto const ff : i.families) {
        if (!ff->infoFor(i.regularOnly).m_maybeCaresAboutDynCalls) {
          return false;
        }
      }
      return true;
    }
  );
}

bool Func::mightBeBuiltin() const {
  return match<bool>(
    val,
    [&](FuncName s) { return true; },
    [&](MethodName) { return true; },
    [&](Fun f) { return f.finfo->func->attrs & AttrBuiltin; },
    [&](Fun2 f) { return f.finfo->func->attrs & AttrBuiltin; },
    [&](Method m) { return m.finfo->func->attrs & AttrBuiltin; },
    [&](Method2 m) { return m.finfo->func->attrs & AttrBuiltin; },
    [&](MethodFamily fa) {
      return fa.family->infoFor(fa.regularOnly).m_static->m_maybeBuiltin;
    },
    [&](MethodFamily2 fa) {
      return fa.family->infoFor(fa.regularOnly).m_maybeBuiltin;
    },
    [&](MethodOrMissing m) { return m.finfo->func->attrs & AttrBuiltin; },
    [&](MethodOrMissing2 m) { return m.finfo->func->attrs & AttrBuiltin; },
    [&](Missing m) { return false; },
    [&](const Isect& i) {
      for (auto const ff : i.families) {
        if (!ff->infoFor(i.regularOnly).m_static->m_maybeBuiltin) return false;
      }
      return true;
    },
    [&](const Isect2& i) {
      for (auto const ff : i.families) {
        if (!ff->infoFor(i.regularOnly).m_maybeBuiltin) return false;
      }
      return true;
    }
  );
}

uint32_t Func::minNonVariadicParams() const {
  return match<uint32_t>(
    val,
    [&] (FuncName) { return 0; },
    [&] (MethodName) { return 0; },
    [&] (Fun f) { return numNVArgs(*f.finfo->func); },
    [&] (Fun2 f) { return numNVArgs(*f.finfo->func); },
    [&] (Method m) { return numNVArgs(*m.finfo->func); },
    [&] (Method2 m) { return numNVArgs(*m.finfo->func); },
    [&] (MethodFamily fa) {
      return
        fa.family->infoFor(fa.regularOnly).m_static->m_minNonVariadicParams;
    },
    [&] (MethodFamily2 fa) {
      return fa.family->infoFor(fa.regularOnly).m_minNonVariadicParams;
    },
    [&] (MethodOrMissing m) { return numNVArgs(*m.finfo->func); },
    [&] (MethodOrMissing2 m) { return numNVArgs(*m.finfo->func); },
    [&] (Missing) { return 0; },
    [&] (const Isect& i) {
      uint32_t nv = 0;
      for (auto const ff : i.families) {
        nv = std::max(
          nv,
          ff->infoFor(i.regularOnly).m_static->m_minNonVariadicParams
        );
      }
      return nv;
    },
    [&] (const Isect2& i) {
      uint32_t nv = 0;
      for (auto const ff : i.families) {
        nv = std::max(
          nv,
          ff->infoFor(i.regularOnly).m_minNonVariadicParams
        );
      }
      return nv;
    }
  );
}

uint32_t Func::maxNonVariadicParams() const {
  return match<uint32_t>(
    val,
    [&] (FuncName) { return std::numeric_limits<uint32_t>::max(); },
    [&] (MethodName) { return std::numeric_limits<uint32_t>::max(); },
    [&] (Fun f) { return numNVArgs(*f.finfo->func); },
    [&] (Fun2 f) { return numNVArgs(*f.finfo->func); },
    [&] (Method m) { return numNVArgs(*m.finfo->func); },
    [&] (Method2 m) { return numNVArgs(*m.finfo->func); },
    [&] (MethodFamily fa) {
      return
        fa.family->infoFor(fa.regularOnly).m_static->m_maxNonVariadicParams;
    },
    [&] (MethodFamily2 fa) {
      return fa.family->infoFor(fa.regularOnly).m_maxNonVariadicParams;
    },
    [&] (MethodOrMissing m) { return numNVArgs(*m.finfo->func); },
    [&] (MethodOrMissing2 m) { return numNVArgs(*m.finfo->func); },
    [&] (Missing) { return 0; },
    [&] (const Isect& i) {
      auto nv = std::numeric_limits<uint32_t>::max();
      for (auto const ff : i.families) {
        nv = std::min(
          nv,
          ff->infoFor(i.regularOnly).m_static->m_maxNonVariadicParams
        );
      }
      return nv;
    },
    [&] (const Isect2& i) {
      auto nv = std::numeric_limits<uint32_t>::max();
      for (auto const ff : i.families) {
        nv = std::min(
          nv,
          ff->infoFor(i.regularOnly).m_maxNonVariadicParams
        );
      }
      return nv;
    }
  );
}

const RuntimeCoeffects* Func::requiredCoeffects() const {
  return match<const RuntimeCoeffects*>(
    val,
    [&] (FuncName) { return nullptr; },
    [&] (MethodName) { return nullptr; },
    [&] (Fun f) { return &f.finfo->func->requiredCoeffects; },
    [&] (Fun2 f) { return &f.finfo->func->requiredCoeffects; },
    [&] (Method m) { return &m.finfo->func->requiredCoeffects; },
    [&] (Method2 m) { return &m.finfo->func->requiredCoeffects; },
    [&] (MethodFamily fa) {
      return fa.family->infoFor(fa.regularOnly)
        .m_static->m_requiredCoeffects.get_pointer();
    },
    [&] (MethodFamily2 fa) {
      return
        fa.family->infoFor(fa.regularOnly).m_requiredCoeffects.get_pointer();
    },
    [&] (MethodOrMissing m) { return &m.finfo->func->requiredCoeffects; },
    [&] (MethodOrMissing2 m) { return &m.finfo->func->requiredCoeffects; },
    [&] (Missing) { return nullptr; },
    [&] (const Isect& i) {
      const RuntimeCoeffects* coeffects = nullptr;
      for (auto const ff : i.families) {
        auto const& info = *ff->infoFor(i.regularOnly).m_static;
        if (!info.m_requiredCoeffects) continue;
        assertx(IMPLIES(coeffects, *coeffects == *info.m_requiredCoeffects));
        if (!coeffects) coeffects = info.m_requiredCoeffects.get_pointer();
      }
      return coeffects;
    },
    [&] (const Isect2& i) {
      const RuntimeCoeffects* coeffects = nullptr;
      for (auto const ff : i.families) {
        auto const& info = ff->infoFor(i.regularOnly);
        if (!info.m_requiredCoeffects) continue;
        assertx(IMPLIES(coeffects, *coeffects == *info.m_requiredCoeffects));
        if (!coeffects) coeffects = info.m_requiredCoeffects.get_pointer();
      }
      return coeffects;
    }
  );
}

const CompactVector<CoeffectRule>* Func::coeffectRules() const {
  return match<const CompactVector<CoeffectRule>*>(
    val,
    [&] (FuncName) { return nullptr; },
    [&] (MethodName) { return nullptr; },
    [&] (Fun f) { return &f.finfo->func->coeffectRules; },
    [&] (Fun2 f) { return &f.finfo->func->coeffectRules; },
    [&] (Method m) { return &m.finfo->func->coeffectRules; },
    [&] (Method2 m) { return &m.finfo->func->coeffectRules; },
    [&] (MethodFamily fa) {
      return fa.family->infoFor(fa.regularOnly)
        .m_static->m_coeffectRules.get_pointer();
    },
    [&] (MethodFamily2 fa) {
      return fa.family->infoFor(fa.regularOnly).m_coeffectRules.get_pointer();
    },
    [&] (MethodOrMissing m) { return &m.finfo->func->coeffectRules; },
    [&] (MethodOrMissing2 m) { return &m.finfo->func->coeffectRules; },
    [&] (Missing) { return nullptr; },
    [&] (const Isect& i) {
      const CompactVector<CoeffectRule>* coeffects = nullptr;
      for (auto const ff : i.families) {
        auto const& info = *ff->infoFor(i.regularOnly).m_static;
        if (!info.m_coeffectRules) continue;
        assertx(
          IMPLIES(
            coeffects,
            std::is_permutation(
              begin(*coeffects),
              end(*coeffects),
              begin(*info.m_coeffectRules),
              end(*info.m_coeffectRules)
            )
          )
        );
        if (!coeffects) coeffects = info.m_coeffectRules.get_pointer();
      }
      return coeffects;
    },
    [&] (const Isect2& i) {
      const CompactVector<CoeffectRule>* coeffects = nullptr;
      for (auto const ff : i.families) {
        auto const& info = ff->infoFor(i.regularOnly);
        if (!info.m_coeffectRules) continue;
        assertx(
          IMPLIES(
            coeffects,
            std::is_permutation(
              begin(*coeffects),
              end(*coeffects),
              begin(*info.m_coeffectRules),
              end(*info.m_coeffectRules)
            )
          )
        );
        if (!coeffects) coeffects = info.m_coeffectRules.get_pointer();
      }
      return coeffects;
    }
  );
}

TriBool Func::supportsAsyncEagerReturn() const {
  return match<TriBool>(
    val,
    [&] (FuncName)   { return TriBool::Maybe; },
    [&] (MethodName) { return TriBool::Maybe; },
    [&] (Fun f)      { return yesOrNo(func_supports_AER(f.finfo->func)); },
    [&] (Fun2 f)     { return yesOrNo(func_supports_AER(f.finfo->func)); },
    [&] (Method m)   { return yesOrNo(func_supports_AER(m.finfo->func)); },
    [&] (Method2 m)  { return yesOrNo(func_supports_AER(m.finfo->func)); },
    [&] (MethodFamily fam) {
      return fam.family->infoFor(fam.regularOnly).m_static->m_supportsAER;
    },
    [&] (MethodFamily2 fam) {
      return fam.family->infoFor(fam.regularOnly).m_supportsAER;
    },
    [&] (MethodOrMissing m)  {
      return yesOrNo(func_supports_AER(m.finfo->func));
    },
    [&] (MethodOrMissing2 m) {
      return yesOrNo(func_supports_AER(m.finfo->func));
    },
    [&] (Missing) { return TriBool::No; },
    [&] (const Isect& i) {
      auto aer = TriBool::Maybe;
      for (auto const ff : i.families) {
        auto const& info = *ff->infoFor(i.regularOnly).m_static;
        if (info.m_supportsAER == TriBool::Maybe) continue;
        assertx(IMPLIES(aer != TriBool::Maybe, aer == info.m_supportsAER));
        if (aer == TriBool::Maybe) aer = info.m_supportsAER;
      }
      return aer;
    },
    [&] (const Isect2& i) {
      auto aer = TriBool::Maybe;
      for (auto const ff : i.families) {
        auto const& info = ff->infoFor(i.regularOnly);
        if (info.m_supportsAER == TriBool::Maybe) continue;
        assertx(IMPLIES(aer != TriBool::Maybe, aer == info.m_supportsAER));
        if (aer == TriBool::Maybe) aer = info.m_supportsAER;
      }
      return aer;
    }
  );
}

Optional<uint32_t> Func::lookupNumInoutParams() const {
  return match<Optional<uint32_t>>(
    val,
    [&] (FuncName s)   -> Optional<uint32_t> { return std::nullopt; },
    [&] (MethodName s) -> Optional<uint32_t> { return std::nullopt; },
    [&] (Fun f)     { return func_num_inout(f.finfo->func); },
    [&] (Fun2 f)    { return func_num_inout(f.finfo->func); },
    [&] (Method m)  { return func_num_inout(m.finfo->func); },
    [&] (Method2 m) { return func_num_inout(m.finfo->func); },
    [&] (MethodFamily fam) {
      return fam.family->infoFor(fam.regularOnly).m_static->m_numInOut;
    },
    [&] (MethodFamily2 fam) {
      return fam.family->infoFor(fam.regularOnly).m_numInOut;
    },
    [&] (MethodOrMissing m)  { return func_num_inout(m.finfo->func); },
    [&] (MethodOrMissing2 m) { return func_num_inout(m.finfo->func); },
    [&] (Missing)            { return 0; },
    [&] (const Isect& i) {
      Optional<uint32_t> numInOut;
      for (auto const ff : i.families) {
        auto const& info = *ff->infoFor(i.regularOnly).m_static;
        if (!info.m_numInOut) continue;
        assertx(IMPLIES(numInOut, *numInOut == *info.m_numInOut));
        if (!numInOut) numInOut = info.m_numInOut;
      }
      return numInOut;
    },
    [&] (const Isect2& i) {
      Optional<uint32_t> numInOut;
      for (auto const ff : i.families) {
        auto const& info = ff->infoFor(i.regularOnly);
        if (!info.m_numInOut) continue;
        assertx(IMPLIES(numInOut, *numInOut == *info.m_numInOut));
        if (!numInOut) numInOut = info.m_numInOut;
      }
      return numInOut;
    }
  );
}

PrepKind Func::lookupParamPrep(uint32_t paramId) const {
  auto const fromFuncFamily = [&] (FuncFamily* ff, bool regularOnly) {
    auto const& info = *ff->infoFor(regularOnly).m_static;
    if (paramId >= info.m_paramPreps.size()) {
      return PrepKind{TriBool::No, TriBool::No};
    }
    return info.m_paramPreps[paramId];
  };
  auto const fromFuncFamily2 = [&] (const FuncFamily2* ff, bool regularOnly) {
    auto const& info = ff->infoFor(regularOnly);
    if (paramId >= info.m_paramPreps.size()) {
      return PrepKind{TriBool::No, TriBool::No};
    }
    return info.m_paramPreps[paramId];
  };

  return match<PrepKind>(
    val,
    [&] (FuncName s)         { return PrepKind{TriBool::Maybe, TriBool::Maybe}; },
    [&] (MethodName s)       { return PrepKind{TriBool::Maybe, TriBool::Maybe}; },
    [&] (Fun f)              { return func_param_prep(f.finfo->func, paramId); },
    [&] (Fun2 f)             { return func_param_prep(f.finfo->func, paramId); },
    [&] (Method m)           { return func_param_prep(m.finfo->func, paramId); },
    [&] (Method2 m)          { return func_param_prep(m.finfo->func, paramId); },
    [&] (MethodFamily f)     { return fromFuncFamily(f.family, f.regularOnly); },
    [&] (MethodFamily2 f)    { return fromFuncFamily2(f.family, f.regularOnly); },
    [&] (MethodOrMissing m)  { return func_param_prep(m.finfo->func, paramId); },
    [&] (MethodOrMissing2 m) { return func_param_prep(m.finfo->func, paramId); },
    [&] (Missing)            { return PrepKind{TriBool::No, TriBool::Yes}; },
    [&] (const Isect& i) {
      auto inOut = TriBool::Maybe;
      auto readonly = TriBool::Maybe;

      for (auto const ff : i.families) {
        auto const prepKind = fromFuncFamily(ff, i.regularOnly);
        if (prepKind.inOut != TriBool::Maybe) {
          assertx(IMPLIES(inOut != TriBool::Maybe, inOut == prepKind.inOut));
          if (inOut == TriBool::Maybe) inOut = prepKind.inOut;
        }

        if (prepKind.readonly != TriBool::Maybe) {
          assertx(
            IMPLIES(readonly != TriBool::Maybe, readonly == prepKind.readonly)
          );
          if (readonly == TriBool::Maybe) readonly = prepKind.readonly;
        }
      }

      return PrepKind{inOut, readonly};
    },
    [&] (const Isect2& i) {
      auto inOut = TriBool::Maybe;
      auto readonly = TriBool::Maybe;

      for (auto const ff : i.families) {
        auto const prepKind = fromFuncFamily2(ff, i.regularOnly);
        if (prepKind.inOut != TriBool::Maybe) {
          assertx(IMPLIES(inOut != TriBool::Maybe, inOut == prepKind.inOut));
          if (inOut == TriBool::Maybe) inOut = prepKind.inOut;
        }

        if (prepKind.readonly != TriBool::Maybe) {
          assertx(
            IMPLIES(readonly != TriBool::Maybe, readonly == prepKind.readonly)
          );
          if (readonly == TriBool::Maybe) readonly = prepKind.readonly;
        }
      }

      return PrepKind{inOut, readonly};
    }
  );
}

TriBool Func::lookupReturnReadonly() const {
  return match<TriBool>(
    val,
    [&] (FuncName)     { return TriBool::Maybe; },
    [&] (MethodName)   { return TriBool::Maybe; },
    [&] (Fun f)        { return yesOrNo(f.finfo->func->isReadonlyReturn); },
    [&] (Fun2 f)       { return yesOrNo(f.finfo->func->isReadonlyReturn); },
    [&] (Method m)     { return yesOrNo(m.finfo->func->isReadonlyReturn); },
    [&] (Method2 m)    { return yesOrNo(m.finfo->func->isReadonlyReturn); },
    [&] (MethodFamily fam) {
      return fam.family->infoFor(fam.regularOnly).m_static->m_isReadonlyReturn;
    },
    [&] (MethodFamily2 fam) {
      return fam.family->infoFor(fam.regularOnly).m_isReadonlyReturn;
    },
    [&] (MethodOrMissing m)  { return yesOrNo(m.finfo->func->isReadonlyReturn); },
    [&] (MethodOrMissing2 m) { return yesOrNo(m.finfo->func->isReadonlyReturn); },
    [&] (Missing)            { return TriBool::No; },
    [&] (const Isect& i) {
      auto readOnly = TriBool::Maybe;
      for (auto const ff : i.families) {
        auto const& info = *ff->infoFor(i.regularOnly).m_static;
        if (info.m_isReadonlyReturn == TriBool::Maybe) continue;
        assertx(IMPLIES(readOnly != TriBool::Maybe,
                        readOnly == info.m_isReadonlyReturn));
        if (readOnly == TriBool::Maybe) readOnly = info.m_isReadonlyReturn;
      }
      return readOnly;
    },
    [&] (const Isect2& i) {
      auto readOnly = TriBool::Maybe;
      for (auto const ff : i.families) {
        auto const& info = ff->infoFor(i.regularOnly);
        if (info.m_isReadonlyReturn == TriBool::Maybe) continue;
        assertx(IMPLIES(readOnly != TriBool::Maybe,
                        readOnly == info.m_isReadonlyReturn));
        if (readOnly == TriBool::Maybe) readOnly = info.m_isReadonlyReturn;
      }
      return readOnly;
    }
  );
}

TriBool Func::lookupReadonlyThis() const {
  return match<TriBool>(
    val,
    [&] (FuncName s)   { return TriBool::Maybe; },
    [&] (MethodName s) { return TriBool::Maybe; },
    [&] (Fun f)        { return yesOrNo(f.finfo->func->isReadonlyThis); },
    [&] (Fun2 f)       { return yesOrNo(f.finfo->func->isReadonlyThis); },
    [&] (Method m)     { return yesOrNo(m.finfo->func->isReadonlyThis); },
    [&] (Method2 m)    { return yesOrNo(m.finfo->func->isReadonlyThis); },
    [&] (MethodFamily fam) {
      return fam.family->infoFor(fam.regularOnly).m_static->m_isReadonlyThis;
    },
    [&] (MethodFamily2 fam) {
      return fam.family->infoFor(fam.regularOnly).m_isReadonlyThis;
    },
    [&] (MethodOrMissing m)  { return yesOrNo(m.finfo->func->isReadonlyThis); },
    [&] (MethodOrMissing2 m) { return yesOrNo(m.finfo->func->isReadonlyThis); },
    [&] (Missing)            { return TriBool::No; },
    [&] (const Isect& i) {
      auto readOnly = TriBool::Maybe;
      for (auto const ff : i.families) {
        auto const& info = *ff->infoFor(i.regularOnly).m_static;
        if (info.m_isReadonlyThis == TriBool::Maybe) continue;
        assertx(IMPLIES(readOnly != TriBool::Maybe,
                        readOnly == info.m_isReadonlyThis));
        if (readOnly == TriBool::Maybe) readOnly = info.m_isReadonlyThis;
      }
      return readOnly;
    },
    [&] (const Isect2& i) {
      auto readOnly = TriBool::Maybe;
      for (auto const ff : i.families) {
        auto const& info = ff->infoFor(i.regularOnly);
        if (info.m_isReadonlyThis == TriBool::Maybe) continue;
        assertx(IMPLIES(readOnly != TriBool::Maybe,
                        readOnly == info.m_isReadonlyThis));
        if (readOnly == TriBool::Maybe) readOnly = info.m_isReadonlyThis;
      }
      return readOnly;
    }
  );
}

std::string show(const Func& f) {
  auto ret = f.name()->toCppString();
  match<void>(
    f.val,
    [&](Func::FuncName s)        {},
    [&](Func::MethodName)        {},
    [&](Func::Fun)               { ret += "*"; },
    [&](Func::Fun2)              { ret += "*"; },
    [&](Func::Method)            { ret += "*"; },
    [&](Func::Method2)           { ret += "*"; },
    [&](Func::MethodFamily)      { ret += "+"; },
    [&](Func::MethodFamily2)     { ret += "+"; },
    [&](Func::MethodOrMissing)   { ret += "-"; },
    [&](Func::MethodOrMissing2)  { ret += "-"; },
    [&](Func::Missing)           { ret += "!"; },
    [&](const Func::Isect&)      { ret += "&"; },
    [&](const Func::Isect2&)     { ret += "&"; }
  );
  return ret;
}

}

//////////////////////////////////////////////////////////////////////

struct Index::IndexData {
  explicit IndexData(Index* index) : m_index{index} {}
  IndexData(const IndexData&) = delete;
  IndexData& operator=(const IndexData&) = delete;

  Index* m_index;

  bool frozen{false};
  bool ever_frozen{false};

  // If non-nullptr, log information about each pass into it.
  StructuredLogEntry* sample;

  // Async state:
  std::unique_ptr<coro::TicketExecutor> executor;
  std::unique_ptr<Client> client;
  DisposeCallback disposeClient;

  // Global configeration, stored in extern-worker.
  std::unique_ptr<coro::AsyncValue<Ref<Config>>> configRef;

  // Maps unit/class/func name to the extern-worker ref representing
  // php::Program data for that. Any associated bytecode is stored
  // separately.
  SStringToOneT<UniquePtrRef<php::Unit>>   unitRefs;
  ISStringToOneT<UniquePtrRef<php::Class>> classRefs;
  ISStringToOneT<UniquePtrRef<php::Func>>  funcRefs;

  // Maps class name to the extern-worker ref representing the class's
  // associated ClassInfo2. Only has entries for instantiable classes.
  ISStringToOneT<UniquePtrRef<ClassInfo2>> classInfoRefs;

  // Maps func name (global functions, not methods) to the
  // extern-worked ref representing the func's associated
  // FuncInfo2. The FuncInfo2 for methods are stored in their parent
  // ClassInfo2.
  ISStringToOneT<UniquePtrRef<FuncInfo2>> funcInfoRefs;

  // Maps class/func names to the extern-worker ref representing the
  // bytecode for that class or (global) function. The bytecode of all
  // of a class' methods are stored together.
  ISStringToOneT<UniquePtrRef<php::ClassBytecode>> classBytecodeRefs;
  ISStringToOneT<UniquePtrRef<php::FuncBytecode>> funcBytecodeRefs;

  // Func family entries representing all methods with a particular
  // name.
  SStringToOneT<FuncFamilyEntry> nameOnlyMethodFamilies;

  // Maps func-family ids to the func family group which contains the
  // func family with that id.
  hphp_fast_map<FuncFamily2::Id, Ref<FuncFamilyGroup>> funcFamilyRefs;

  std::unique_ptr<php::Program> program;

  ISStringToOneT<php::Class*>      classes;
  ISStringToOneT<php::Func*>       funcs;
  ISStringToOneT<php::TypeAlias*>  typeAliases;
  ISStringToOneT<php::Class*>      enums;
  SStringToOneT<php::Constant*>    constants;
  SStringToOneT<php::Module*>      modules;
  SStringToOneT<php::Unit*>        units;

  /*
   * Func families representing methods with a particular name (across
   * all classes).
   */
  struct MethodFamilyEntry {
    FuncFamilyOrSingle m_all;
    FuncFamilyOrSingle m_regular;
  };
  SStringToOneT<MethodFamilyEntry> methodFamilies;

  // Map from each class to all the closures that are allocated in
  // functions of that class.
  hphp_fast_map<
    const php::Class*,
    CompactVector<const php::Class*>
  > classClosureMap;

  hphp_fast_map<
    const php::Class*,
    hphp_fast_set<const php::Func*>
  > classExtraMethodMap;

  /*
   * Map from each class name to ClassInfo objects if one exists.
   *
   * It may not exists if we would fatal when defining the class. That could
   * happen for if the inheritance is bad or __Sealed or other things.
   */
  ISStringToOneT<ClassInfo*> classInfo;

  /*
   * All the ClassInfos, stored in no particular order.
   */
  std::vector<std::unique_ptr<ClassInfo>> allClassInfos;

  std::vector<FuncInfo> funcInfo;
  std::atomic<uint32_t> nextFuncId{};

  // Private instance and static property types are stored separately
  // from ClassInfo, because you don't need to resolve a class to get
  // at them.
  hphp_hash_map<
    const php::Class*,
    PropState
  > privatePropInfo;
  hphp_hash_map<
    const php::Class*,
    PropState
  > privateStaticPropInfo;

  /*
   * Public static property information:
   */

  // If this is true, we've seen mutations to public static
  // properties. Once this is true, it's no longer legal to report a
  // pessimistic static property set (unknown class and
  // property). Doing so is a monotonicity violation.
  bool seenPublicSPropMutations{false};

  // The set of gathered public static property mutations for each function. The
  // inferred types for the public static properties is the union of all these
  // mutations. If a function is not analyzed in a particular analysis round,
  // its mutations are left unchanged from the previous round.
  folly_concurrent_hash_map_simd<
    const php::Func*,
    PublicSPropMutations,
    pointer_hash<const php::Func>> publicSPropMutations;

  // All FuncFamilies. These are stored globally so we can avoid
  // generating duplicates.
  folly_concurrent_hash_map_simd<
    std::unique_ptr<FuncFamily>,
    bool,
    FuncFamilyPtrHasher,
    FuncFamilyPtrEquals
  > funcFamilies;

  folly_concurrent_hash_map_simd<
    std::unique_ptr<FuncFamily::StaticInfo>,
    bool,
    FFStaticInfoPtrHasher,
    FFStaticInfoPtrEquals
  > funcFamilyStaticInfos;

  /*
   * Map from interfaces to their assigned vtable slots, computed in
   * compute_iface_vtables().
   */
  ISStringToOneT<Slot> ifaceSlotMap;

  hphp_hash_map<
    const php::Class*,
    CompactVector<Type>
  > closureUseVars;

  struct ClsConstTypesHasher {
    bool operator()(const std::pair<const php::Class*, SString>& k) const {
      return folly::hash::hash_combine(
        pointer_hash<php::Class>{}(k.first),
        pointer_hash<StringData>{}(k.second)
      );
    }
  };
  struct ClsConstTypesEquals {
    bool operator()(const std::pair<const php::Class*, SString>& a,
                    const std::pair<const php::Class*, SString>& b) const {
      return a.first == b.first && a.second == b.second;
    }
  };

  // Cache for lookup_class_constant
  folly_concurrent_hash_map_simd<
    std::pair<const php::Class*, SString>,
    ClsConstLookupResult,
    ClsConstTypesHasher,
    ClsConstTypesEquals
  > clsConstLookupCache;

  bool useClassDependencies{};
  DepMap dependencyMap;

  /*
   * If a function is effect-free when called with a particular set of
   * literal arguments, and produces a literal result, there will be
   * an entry here representing the type.
   *
   * The map isn't just an optimization; we can't call
   * analyze_func_inline during the optimization phase, because the
   * bytecode could be modified while we do so.
   */
  ContextRetTyMap foldableReturnTypeMap;

  /*
   * Call-context sensitive return types are cached here.  This is not
   * an optimization.
   *
   * The reason we need to retain this information about the
   * calling-context-sensitive return types is that once the Index is
   * frozen (during the final optimization pass), calls to
   * lookup_return_type with a CallContext can't look at the bytecode
   * bodies of functions other than the calling function.  So we need
   * to know what we determined the last time we were alloewd to do
   * that so we can return it again.
   */
  ContextRetTyMap contextualReturnTypes{};

  /*
   * Lazily calculate the class that should be used for wait-handles.
   */
  LockFreeLazy<res::Class> lazyWaitHandleCls;
};

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

using IndexData = Index::IndexData;

std::mutex closure_use_vars_mutex;
std::mutex private_propstate_mutex;

DependencyContext make_dep(const php::Func* func) {
  return DependencyContext{DependencyContextType::Func, func};
}
DependencyContext make_dep(const php::Class* cls) {
  return DependencyContext{DependencyContextType::Class, cls};
}
DependencyContext make_dep(const php::Prop* prop) {
  return DependencyContext{DependencyContextType::Prop, prop};
}
DependencyContext make_dep(const FuncFamily* family) {
  return DependencyContext{DependencyContextType::FuncFamily, family};
}

DependencyContext dep_context(IndexData& data, const Context& baseCtx) {
  auto const& ctx = baseCtx.forDep();
  if (!ctx.cls || !data.useClassDependencies) return make_dep(ctx.func);
  auto const cls = ctx.cls->closureContextCls
    ? data.classes.at(ctx.cls->closureContextCls)
    : ctx.cls;
  if (is_used_trait(*cls)) return make_dep(ctx.func);
  return make_dep(cls);
}

template <typename T>
void add_dependency(IndexData& data,
                    T src,
                    const Context& dst,
                    Dep newMask) {
  if (data.frozen) return;

  auto d = dep_context(data, dst);
  DepMap::accessor acc;
  data.dependencyMap.insert(acc, make_dep(src));
  auto& current = acc->second[d];
  current = current | newMask;

  // We should only have a return type dependency on func families.
  assertx(
    IMPLIES(
      acc->first.tag() == DependencyContextType::FuncFamily,
      newMask == Dep::ReturnTy
    )
  );
}

template <typename T>
void find_deps(IndexData& data,
               T src,
               Dep mask,
               DependencyContextSet& deps) {
  auto const srcDep = make_dep(src);

  // We should only ever have return type dependencies on func family.
  assertx(
    IMPLIES(
      srcDep.tag() == DependencyContextType::FuncFamily,
      mask == Dep::ReturnTy
    )
  );

  DepMap::const_accessor acc;
  if (data.dependencyMap.find(acc, srcDep)) {
    for (auto const& kv : acc->second) {
      if (has_dep(kv.second, mask)) deps.insert(kv.first);
    }
  }
}

//////////////////////////////////////////////////////////////////////

FuncInfo* func_info(IndexData& data, const php::Func* f) {
  assertx(f->idx < data.funcInfo.size());
  auto const fi = &data.funcInfo[f->idx];
  assertx(fi->func == f);
  return fi;
}

//////////////////////////////////////////////////////////////////////

// Obtain the php::Func* represented by a MethRef. Doesn't work inside
// remote worker jobs.
const php::Func* func_from_meth_ref(const IndexData& index,
                                    const MethRef& meth) {
  auto const cls = index.classes.at(meth.cls);
  assertx(meth.idx < cls->methods.size());
  return cls->methods[meth.idx].get();
}

//////////////////////////////////////////////////////////////////////

struct TraitMethod {
  using class_type = std::pair<const ClassInfo2*, const php::Class*>;
  using method_type = const php::Func*;
  using origin_type = SString;

  TraitMethod(class_type trait_, method_type method_, Attr modifiers_)
      : trait(trait_)
      , method(method_)
      , modifiers(modifiers_)
    {}

  class_type trait;
  method_type method;
  Attr modifiers;
};

struct TMIOps {
  using string_type = LSString;
  using class_type = TraitMethod::class_type;
  using method_type = TraitMethod::method_type;
  using origin_type = TraitMethod::origin_type;

  struct TMIException : std::exception {
    explicit TMIException(std::string msg) : msg(msg) {}
    const char* what() const noexcept override { return msg.c_str(); }
  private:
    std::string msg;
  };

  // Return the name for the trait class.
  static const string_type clsName(class_type traitCls) {
    return traitCls.first->name;
  }

  // Return the name of the trait where the method was originally defined
  static origin_type originalClass(method_type meth) {
    return meth->originalClass ? meth->originalClass : meth->cls->name;
  }

  // Is-a methods.
  static bool isAbstract(Attr modifiers) {
    return modifiers & AttrAbstract;
  }

  // Whether to exclude methods with name `methName' when adding.
  static bool exclude(string_type methName) {
    return Func::isSpecial(methName);
  }

  // Errors.
  static void errorDuplicateMethod(class_type cls,
                                   string_type methName,
                                   const std::vector<const StringData*>&) {
    auto const& m = cls.second->methods;
    if (std::find_if(m.begin(), m.end(),
                     [&] (auto const& f) {
                       return f->name->same(methName);
                     }) != m.end()) {
      // the duplicate methods will be overridden by the class method.
      return;
    }
    throw TMIException(folly::sformat("DuplicateMethod: {}", methName));
  }
};

using TMIData = TraitMethodImportData<TraitMethod, TMIOps>;

//////////////////////////////////////////////////////////////////////

template <typename T, typename R>
void add_symbol_to_index(R& map, T t, const char* type) {
  auto const name = t->name;
  auto const ret = map.emplace(name, std::move(t));
  always_assert_flog(
    ret.second,
    "More than one {} with the name {} "
    "(should have been caught by parser)",
    type,
    name
  );
}

template <typename T, typename R, typename E>
void add_symbol_to_index(R& map, T t, const char* type, const E& other) {
  auto const it = other.find(t->name);
  always_assert_flog(
    it == other.end(),
    "More than one symbol with the name {} "
    "(should have been caught by parser)",
    t->name
  );
  add_symbol_to_index(map, std::move(t), type);
}

// We want const qualifiers on various index data structures for php
// object pointers, but during index creation time we need to
// manipulate some of their attributes (changing the representation).
// This little wrapper keeps the const_casting out of the main line of
// code below.
void attribute_setter(const Attr& attrs, bool set, Attr attr) {
  attrSetter(const_cast<Attr&>(attrs), set, attr);
}

void add_system_constants_to_index(IndexData& index) {
  for (auto cnsPair : Native::getConstants()) {
    assertx(cnsPair.second.m_type != KindOfUninit);
    auto pc = new php::Constant {
      cnsPair.first,
      cnsPair.second,
      AttrPersistent
    };
    add_symbol_to_index(index.constants, pc, "constant");
  }
}

void add_unit_to_index(IndexData& index, php::Unit& unit) {
  always_assert_flog(
    index.units.emplace(unit.filename, &unit).second,
    "More than one unit with the same name {} "
    "(should have been caught by parser)",
    unit.filename
  );

  for (auto& ta : unit.typeAliases) {
    add_symbol_to_index(
      index.typeAliases,
      ta.get(),
      "type alias",
      index.classes
    );
  }

  for (auto& c : unit.constants) {
    add_symbol_to_index(index.constants, c.get(), "constant");
  }

  for (auto& m : unit.modules) {
    add_symbol_to_index(index.modules, m.get(), "module");
  }
}

void add_class_to_index(IndexData& index, php::Class& c) {
  if (c.attrs & AttrEnum) {
    add_symbol_to_index(index.enums, &c, "enum");
  }

  add_symbol_to_index(index.classes, &c, "class", index.typeAliases);

  for (auto& m : c.methods) {
    attribute_setter(m->attrs, false, AttrNoOverride);
    m->idx = index.nextFuncId++;
  }
}

void add_func_to_index(IndexData& index, php::Func& func) {
  add_symbol_to_index(index.funcs, &func, "function");
  func.idx = index.nextFuncId++;
}

void add_program_to_index(IndexData& index) {
  trace_time timer{"add program to index", index.sample};
  timer.ignore_client_stats();

  auto& program = *index.program;
  for (auto const& u : program.units) {
    add_unit_to_index(index, *u);
  }
  for (auto const& c : program.classes) {
    add_class_to_index(index, *c);
  }
  for (auto const& f : program.funcs) {
    add_func_to_index(index, *f);
  }

  for (auto const& c : program.classes) {
    if (!c->closureContextCls) continue;
    auto& s = index.classClosureMap[index.classes.at(c->closureContextCls)];
    s.emplace_back(c.get());
  }

  // All funcs have been assigned indices above. Now for each func we
  // initialize a default FuncInfo in the funcInfo vec at the
  // appropriate index.

  trace_time timer2{"create func-infos"};
  timer2.ignore_client_stats();

  index.funcInfo.resize(index.nextFuncId);

  auto const create = [&] (const php::Func& f) {
    assertx(f.idx < index.funcInfo.size());
    auto& fi = index.funcInfo[f.idx];
    assertx(!fi.func);
    fi.func = &f;
  };

  parallel::for_each(
    program.classes,
    [&] (const std::unique_ptr<php::Class>& cls) {
      for (auto const& m : cls->methods) create(*m);
    }
  );

  parallel::for_each(
    program.funcs,
    [&] (const std::unique_ptr<php::Func>& func) { create(*func); }
  );
}

//////////////////////////////////////////////////////////////////////

/*
 * Lists of interfaces that conflict with each other due to being
 * implemented by the same class.
 */

struct InterfaceConflicts {
  SString name;
  // The number of classes which implements this interface (used to
  // prioritize lower slots for more heavily used interfaces).
  size_t usage;
  std::vector<SString> conflicts;
  template <typename SerDe> void serde(SerDe& sd) {
    sd(name)(usage)(conflicts);
  }
};

void compute_iface_vtables(IndexData& index,
                           std::vector<InterfaceConflicts> conflicts) {
  trace_time tracer{"compute interface vtables"};
  tracer.ignore_client_stats();

  if (conflicts.empty()) return;

  // Sort interfaces by usage frequencies. We assign slots greedily,
  // so sort the interface list so the most frequently implemented
  // ones come first.
  std::sort(
    begin(conflicts),
    end(conflicts),
    [&] (const InterfaceConflicts& a, const InterfaceConflicts& b) {
      if (a.usage != b.usage) return a.usage > b.usage;
      return string_data_lti{}(a.name, b.name);
    }
  );

  // Assign slots, keeping track of the largest assigned slot and the
  // total number of uses for each slot.

  Slot maxSlot = 0;
  hphp_fast_map<Slot, int> slotUses;
  boost::dynamic_bitset<> used;

  for (auto const& iface : conflicts) {
    used.reset();

    // Find the lowest Slot that doesn't conflict with anything in the
    // conflict set for iface.
    auto const slot = [&] () -> Slot {
      // No conflicts. This is the only interface implemented by the
      // classes that implement it.
      if (iface.conflicts.empty()) return 0;

      for (auto const conflict : iface.conflicts) {
        auto const it = index.ifaceSlotMap.find(conflict);
        if (it == end(index.ifaceSlotMap)) continue;
        auto const s = it->second;
        if (used.size() <= s) used.resize(s + 1);
        used.set(s);
      }

      used.flip();
      return used.any() ? used.find_first() : used.size();
    }();

    always_assert(
      index.ifaceSlotMap.emplace(iface.name, slot).second
    );
    maxSlot = std::max(maxSlot, slot);
    slotUses[slot] += iface.usage;
  }

  if (debug) {
    // Make sure we have an initialized entry for each slot for the sort below.
    for (Slot slot = 0; slot < maxSlot; ++slot) {
      always_assert(slotUses.count(slot));
    }
  }

  // Finally, sort and reassign slots so the most frequently used
  // slots come first. This slightly reduces the number of wasted
  // vtable vector entries at runtime.

  auto const slots = [&] {
    std::vector<std::pair<Slot, int>> flattened{
      begin(slotUses), end(slotUses)
    };
    std::sort(
      begin(flattened),
      end(flattened),
      [&] (auto const& a, auto const& b) {
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
      }
    );
    std::vector<Slot> out;
    out.reserve(flattened.size());
    for (auto const& [slot, _] : flattened) out.emplace_back(slot);
    return out;
  }();

  std::vector<Slot> slotsPermute(maxSlot + 1, 0);
  for (size_t i = 0; i <= maxSlot; ++i) slotsPermute[slots[i]] = i;

  // re-map interfaces to permuted slots
  for (auto& [cls, slot] : index.ifaceSlotMap) {
    slot = slotsPermute[slot];
  }
}

//////////////////////////////////////////////////////////////////////

struct CheckClassInfoInvariantsJob {
  static std::string name() { return "hhbbc-check-cinfo-invariants"; }
  static void init(const Config& config) {
    process_init(config.o, config.gd, false);
  }
  static void fini() {}

  static bool run(std::unique_ptr<ClassInfo2> cinfo,
                  std::unique_ptr<php::Class> cls) {
    // AttrNoOverride is a superset of AttrNoOverrideRegular
    always_assert(
      IMPLIES(!(cls->attrs & AttrNoOverrideRegular),
              !(cls->attrs & AttrNoOverride))
    );

    // Override attrs and what we know about the subclasses should be in
    // agreement.
    if (cls->attrs & AttrNoOverride) {
      always_assert(!cinfo->hasRegularSubclass);
      always_assert(!cinfo->hasNonRegularSubclass);
    } else if (cls->attrs & AttrNoOverrideRegular) {
      always_assert(!cinfo->hasRegularSubclass);
    }

    if (cls->attrs & AttrNoMock) {
      always_assert(!cinfo->isMocked);
      always_assert(!cinfo->isSubMocked);
    }

    for (auto const& [name, mte] : cinfo->methods) {
      // Interface method tables should only contain its own methods.
      if (cls->attrs & AttrInterface) {
        always_assert(mte.meth().cls->isame(cinfo->name));
      }

      // AttrNoOverride implies noOverrideRegular
      always_assert(IMPLIES(mte.attrs & AttrNoOverride, mte.noOverrideRegular()));

      if (!is_special_method_name(name)) {
        // If the class isn't overridden, none of it's methods can be
        // either.
        always_assert(IMPLIES(cls->attrs & AttrNoOverride,
                              mte.attrs & AttrNoOverride));
      } else {
        always_assert(!(mte.attrs & AttrNoOverride));
        always_assert(!mte.noOverrideRegular());
      }

      if (cinfo->name->isame(s_Closure.get()) || is_closure_name(cinfo->name)) {
        always_assert(mte.attrs & AttrNoOverride);
      }

      // Don't store method families for special methods.
      auto const famIt = cinfo->methodFamilies.find(name);
      if (is_special_method_name(name)) {
        always_assert(famIt == end(cinfo->methodFamilies));
        continue;
      }
      if (famIt == end(cinfo->methodFamilies)) {
        always_assert(is_closure_name(cinfo->name));
        continue;
      }
      auto const& entry = famIt->second;

      // No override methods should always have a single entry.
      if (mte.attrs & AttrNoOverride) {
        always_assert(
          boost::get<FuncFamilyEntry::BothSingle>(&entry.m_meths) ||
          boost::get<FuncFamilyEntry::SingleAndNone>(&entry.m_meths)
        );
        continue;
      }

      if (cinfo->isRegularClass) {
        // "all" should only be a func family. It can't be empty,
        // because we know there's at least one method in it (the one in
        // cinfo->methods). It can't be a single func, because one of
        // the methods must be the cinfo->methods method, and we know it
        // isn't AttrNoOverride, so there *must* be another method. So,
        // it must be a func family.
        always_assert(
          boost::get<FuncFamilyEntry::BothFF>(&entry.m_meths) ||
          boost::get<FuncFamilyEntry::FFAndSingle>(&entry.m_meths)
        );
        // This is a regular class, so we cannot have an incomplete
        // entry (can only happen with interfaces).
        always_assert(!entry.m_regularIncomplete);
      }
    }

    // The subclassList is non-empty, contains this ClassInfo, and
    // contains only unique elements.
    if (!cinfo->name->isame(s_Closure.get())) {
      always_assert(!cinfo->subclassList.empty());
      always_assert(
        std::find_if(
          begin(cinfo->subclassList),
          end(cinfo->subclassList),
          [&] (SString s) { return s->isame(cinfo->name); }
        ) != end(cinfo->subclassList)
      );
      auto cpy = cinfo->subclassList;
      std::sort(begin(cpy), end(cpy), string_data_lti{});
      cpy.erase(
        std::unique(begin(cpy), end(cpy), string_data_isame{}),
        end(cpy)
      );
      always_assert(cpy.size() == cinfo->subclassList.size());
    } else if (is_closure_name(cinfo->name)) {
      always_assert(cinfo->subclassList.size() == 2);
      always_assert(cinfo->subclassList[0]->isame(s_Closure.get()));
      always_assert(cinfo->subclassList[1]->isame(cinfo->name));
    } else {
      always_assert(cinfo->subclassList.empty());
    }

    // The baseList is non-empty, and the last element is this class.
    always_assert(!cinfo->baseList.empty());
    always_assert(cinfo->baseList.back()->isame(cinfo->name));

    return true;
  }
};

struct CheckFuncFamilyInvariantsJob {
  static std::string name() { return "hhbbc-check-ff-invariants"; }
  static void init(const Config& config) {
    process_init(config.o, config.gd, false);
  }
  static void fini() {}

  static bool run(FuncFamilyGroup group) {
    for (auto const& ff : group.m_ffs) {
      // FuncFamily should always have more than one func on it.
      always_assert(
        ff->m_regular.size() +
        ff->m_nonRegularPrivate.size() +
        ff->m_nonRegular.size()
        > 1
      );

      // Every method should be sorted in its respective list. We
      // should never see a method for the same Class more than once.
      ISStringSet classes;
      Optional<MethRef> lastReg;
      Optional<MethRef> lastPrivate;
      Optional<MethRef> lastNonReg;
      for (auto const& meth : ff->m_regular) {
        if (lastReg) always_assert(*lastReg < meth);
        lastReg = meth;
        always_assert(classes.emplace(meth.cls).second);
      }
      for (auto const& meth : ff->m_nonRegularPrivate) {
        if (lastPrivate) always_assert(*lastPrivate < meth);
        lastPrivate = meth;
        always_assert(classes.emplace(meth.cls).second);
      }
      for (auto const& meth : ff->m_nonRegular) {
        if (lastNonReg) always_assert(*lastNonReg < meth);
        lastNonReg = meth;
        always_assert(classes.emplace(meth.cls).second);
      }

      always_assert(ff->m_allStatic.has_value());
      always_assert(
        ff->m_regularStatic.has_value() ==
        (!ff->m_regular.empty() || !ff->m_nonRegularPrivate.empty())
      );
    }
    return true;
  }
};

Job<CheckClassInfoInvariantsJob> s_checkCInfoInvariantsJob;
Job<CheckFuncFamilyInvariantsJob> s_checkFuncFamilyInvariantsJob;

void check_invariants(const IndexData& index) {
  if (!debug) return;

  trace_time trace{"check-invariants", index.sample};

  constexpr size_t kCInfoBucketSize = 3000;
  constexpr size_t kFFBucketSize = 500;

  auto cinfoBuckets = consistently_bucketize(
    [&] {
      std::vector<SString> roots;
      roots.reserve(index.classInfoRefs.size());
      for (auto const& [name, _] : index.classInfoRefs) {
        roots.emplace_back(name);
      }
      return roots;
    }(),
    kCInfoBucketSize
  );

  SStringToOneT<Ref<FuncFamilyGroup>> nameToFuncFamilyGroup;

  auto ffBuckets = consistently_bucketize(
    [&] {
      std::vector<SString> roots;
      roots.reserve(index.funcFamilyRefs.size());
      for (auto const& [_, ref] : index.funcFamilyRefs) {
        auto const name = makeStaticString(ref.id().toString());
        if (nameToFuncFamilyGroup.emplace(name, ref).second) {
          roots.emplace_back(name);
        }
      }
      return roots;
    }(),
    kFFBucketSize
  );

  using namespace folly::gen;

  auto const runCInfo = [&] (std::vector<SString> work) -> coro::Task<void> {
    HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;

    if (work.empty()) HPHP_CORO_RETURN_VOID;

    auto inputs = from(work)
      | map([&] (SString name) {
          return std::make_tuple(
            index.classInfoRefs.at(name),
            index.classRefs.at(name)
          );
        })
      | as<std::vector>();

    Client::ExecMetadata metadata{
      .job_key = folly::sformat("check cinfo invariants {}", work[0])
    };
    auto config = HPHP_CORO_AWAIT(index.configRef->getCopy());
    auto outputs = HPHP_CORO_AWAIT(index.client->exec(
      s_checkCInfoInvariantsJob,
      std::move(config),
      std::move(inputs),
      std::move(metadata)
    ));
    assertx(outputs.size() == work.size());

    HPHP_CORO_RETURN_VOID;
  };

  auto const runFF = [&] (std::vector<SString> work) -> coro::Task<void> {
    HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;

    if (work.empty()) HPHP_CORO_RETURN_VOID;

    auto inputs = from(work)
      | map([&] (SString name) {
          return std::make_tuple(nameToFuncFamilyGroup.at(name));
        })
      | as<std::vector>();

    Client::ExecMetadata metadata{
      .job_key = folly::sformat("check func-family invariants {}", work[0])
    };
    auto config = HPHP_CORO_AWAIT(index.configRef->getCopy());
    auto outputs = HPHP_CORO_AWAIT(index.client->exec(
      s_checkFuncFamilyInvariantsJob,
      std::move(config),
      std::move(inputs),
      std::move(metadata)
    ));
    assertx(outputs.size() == work.size());

    HPHP_CORO_RETURN_VOID;
  };

  std::vector<coro::TaskWithExecutor<void>> tasks;
  for (auto& work : cinfoBuckets) {
    tasks.emplace_back(
      runCInfo(std::move(work)).scheduleOn(index.executor->sticky())
    );
  }
  for (auto& work : ffBuckets) {
    tasks.emplace_back(
      runFF(std::move(work)).scheduleOn(index.executor->sticky())
    );
  }
  coro::wait(coro::collectRange(std::move(tasks)));
}

void check_local_invariants(const IndexData& index, const ClassInfo* cinfo) {
  // AttrNoOverride is a superset of AttrNoOverrideRegular
  always_assert(
    IMPLIES(!(cinfo->cls->attrs & AttrNoOverrideRegular),
            !(cinfo->cls->attrs & AttrNoOverride))
  );

  // Override attrs and what we know about the subclasses should be in
  // agreement.
  if (cinfo->cls->attrs & AttrNoOverride) {
    always_assert(!cinfo->hasRegularSubclass);
    always_assert(!cinfo->hasNonRegularSubclass);
  } else if (cinfo->cls->attrs & AttrNoOverrideRegular) {
    always_assert(!cinfo->hasRegularSubclass);
  }

  if (cinfo->cls->attrs & AttrNoMock) {
    always_assert(!cinfo->isMocked);
    always_assert(!cinfo->isSubMocked);
  }

  for (size_t idx = 0; idx < cinfo->cls->methods.size(); ++idx) {
    // Each method in a class has an entry in its ClassInfo method
    // table.
    auto const& m = cinfo->cls->methods[idx];
    auto const it = cinfo->methods.find(m->name);
    always_assert(it != cinfo->methods.end());
    always_assert(it->second.meth().cls->isame(cinfo->cls->name));
    always_assert(it->second.meth().idx == idx);

    // Every method (except for constructors and special methods
    // should be in the global name-only tables.
    auto const nameIt = index.methodFamilies.find(m->name);
    if (!has_name_only_func_family(m->name)) {
      always_assert(nameIt == end(index.methodFamilies));
      continue;
    }
    always_assert(nameIt != end(index.methodFamilies));

    auto const& entry = nameIt->second;
    // The global name-only tables are never complete.
    always_assert(entry.m_all.isIncomplete());
    always_assert(entry.m_regular.isEmpty() || entry.m_regular.isIncomplete());

    // "all" should always be non-empty and contain this method.
    always_assert(!entry.m_all.isEmpty());
    if (auto const ff = entry.m_all.funcFamily()) {
      always_assert(ff->possibleFuncs().size() > 1);
      // The FuncFamily shouldn't have a section for regular results
      // if "regular" isn't using it.
      if (entry.m_regular.func() || entry.m_regular.isEmpty()) {
        always_assert(!ff->m_regular);
      } else {
        // "all" and "regular" always share the same func family.
        always_assert(entry.m_regular.funcFamily() == ff);
      }
    } else {
      auto const func = entry.m_all.func();
      always_assert(func);
      always_assert(func == m.get());
      // "regular" is always a subset of "all", so it can either be a
      // single func (the same as "all"), or empty.
      always_assert(entry.m_regular.func() || entry.m_regular.isEmpty());
      if (auto const func2 = entry.m_regular.func()) {
        always_assert(func == func2);
      }
    }

    // If this is a regular class, "regular" should be non-empty and
    // contain this method.
    if (auto const ff = entry.m_regular.funcFamily()) {
      always_assert(ff->possibleFuncs().size() > 1);
    } else if (auto const func = entry.m_regular.func()) {
      if (is_regular_class(*cinfo->cls)) {
        always_assert(func == m.get());
      }
    } else {
      always_assert(!is_regular_class(*cinfo->cls));
    }
  }

  // Interface ClassInfo method table should only contain methods from
  // the interface itself.
  if (cinfo->cls->attrs & AttrInterface) {
    always_assert(cinfo->cls->methods.size() == cinfo->methods.size());
  }

  // If a class isn't overridden, it shouldn't have any func families
  // (because the method table is sufficient).
  if (cinfo->cls->attrs & AttrNoOverride) {
    always_assert(cinfo->methodFamilies.empty());
    always_assert(cinfo->methodFamiliesAux.empty());
  }

  // The auxiliary method families map is only used by non-regular
  // classes.
  if (is_regular_class(*cinfo->cls)) {
    always_assert(cinfo->methodFamiliesAux.empty());
  }

  for (auto const& [name, mte] : cinfo->methods) {
    // Interface method tables should only contain its own methods.
    if (cinfo->cls->attrs & AttrInterface) {
      always_assert(mte.meth().cls->isame(cinfo->cls->name));
    } else {
      // Non-interface method tables should not contain any methods
      // defined by an interface.
      auto const func = func_from_meth_ref(index, mte.meth());
      always_assert(!(func->cls->attrs & AttrInterface));
    }

    // AttrNoOverride implies noOverrideRegular
    always_assert(IMPLIES(mte.attrs & AttrNoOverride, mte.noOverrideRegular()));

    if (!is_special_method_name(name)) {
      // If the class isn't overridden, none of it's methods can be
      // either.
      always_assert(IMPLIES(cinfo->cls->attrs & AttrNoOverride,
                            mte.attrs & AttrNoOverride));
    } else {
      always_assert(!(mte.attrs & AttrNoOverride));
      always_assert(!mte.noOverrideRegular());
    }

    if (is_closure_base(*cinfo->cls) || is_closure(*cinfo->cls)) {
      always_assert(mte.attrs & AttrNoOverride);
    }

    auto const famIt = cinfo->methodFamilies.find(name);
    // Don't store method families for special methods, or if there's
    // no override.
    if (is_special_method_name(name) || (mte.attrs & AttrNoOverride)) {
      always_assert(famIt == end(cinfo->methodFamilies));
      always_assert(!cinfo->methodFamiliesAux.count(name));
      continue;
    } else {
      always_assert(famIt != end(cinfo->methodFamilies));
    }
    auto const& entry = famIt->second;

    if (is_regular_class(*cinfo->cls)) {
      // "all" should only be a func family. It can't be empty,
      // because we know there's at least one method in it (the one in
      // cinfo->methods). It can't be a single func, because one of
      // the methods must be the cinfo->methods method, and we know it
      // isn't AttrNoOverride, so there *must* be another method. So,
      // it must be a func family.
      always_assert(entry.funcFamily());
      // This is a regular class, so we cannot have an incomplete
      // entry (can only happen with interfaces).
      always_assert(entry.isComplete());
    } else {
      // This class isn't AttrNoOverride, and since the method is on
      // this class, it should at least contain that.
      always_assert(!entry.isEmpty());
      // Only interfaces can have incomplete entries.
      always_assert(
        IMPLIES(entry.isIncomplete(), cinfo->cls->attrs & AttrInterface)
      );
      // If we got a single func, it should be the func on this
      // class. Since this isn't AttrNoOverride, it implies the entry
      // should be incomplete.
      always_assert(IMPLIES(entry.func(), entry.isIncomplete()));
      always_assert(
        IMPLIES(entry.func(),
                entry.func() == func_from_meth_ref(index, mte.meth()))
      );

      // The "aux" entry is optional. If it isn't present, it's the
      // same as the normal table.
      auto const auxIt = cinfo->methodFamiliesAux.find(name);
      if (auxIt != end(cinfo->methodFamiliesAux)) {
        auto const& aux = auxIt->second;

        // We shouldn't store in the aux table if the entry is the
        // same or if there's no override.
        always_assert(!mte.noOverrideRegular());
        always_assert(
          aux.isIncomplete() ||
          aux.func() != entry.func() ||
          aux.funcFamily() != entry.funcFamily()
        );

        // Normally the aux should be non-empty and complete. However
        // if this class is an interface, they could be.
        always_assert(
          IMPLIES(aux.isEmpty(), cinfo->cls->attrs & AttrInterface)
        );
        always_assert(
          IMPLIES(aux.isIncomplete(), cinfo->cls->attrs & AttrInterface)
        );

        // Since we know this was overridden (it wouldn't be in the
        // aux table otherwise), it must either be incomplete, or if
        // it has a single func, it cannot be the same func as this
        // class.
        always_assert(
          aux.isIncomplete() ||
          ((mte.attrs & AttrPrivate) && mte.topLevel()) ||
          aux.func() != func_from_meth_ref(index, mte.meth())
        );

        // Aux entry is a subset of the normal entry. If they both
        // have a func family or func, they must be the same. If the
        // normal entry has a func family, but aux doesn't, that func
        // family shouldn't have extra space allocated.
        always_assert(IMPLIES(entry.func(), !aux.funcFamily()));
        always_assert(IMPLIES(entry.funcFamily() && aux.funcFamily(),
                              entry.funcFamily() == aux.funcFamily()));
        always_assert(IMPLIES(entry.func() && aux.func(),
                              entry.func() == aux.func()));
        always_assert(IMPLIES(entry.funcFamily() && !aux.funcFamily(),
                              !entry.funcFamily()->m_regular));
      }
    }
  }

  // "Aux" entries should only exist for methods on this class, and
  // with a corresponding methodFamilies entry.
  for (auto const& [name, _] : cinfo->methodFamiliesAux) {
    always_assert(cinfo->methods.count(name));
    always_assert(cinfo->methodFamilies.count(name));
  }

  // We should only have func families for methods declared on this
  // class (except for interfaces and abstract classes).
  for (auto const& [name, entry] : cinfo->methodFamilies) {
    if (cinfo->methods.count(name)) continue;
    // Interfaces and abstract classes can have func families for
    // methods not defined on this class.
    always_assert(cinfo->cls->attrs & (AttrInterface|AttrAbstract));
    // We don't expand func families for these.
    always_assert(name != s_construct.get() && !is_special_method_name(name));

    // We only expand entries for interfaces and abstract classes if
    // it appears in every regular subclass. Therefore it cannot be
    // empty and is complete.
    always_assert(!entry.isEmpty());
    always_assert(entry.isComplete());
    if (auto const ff = entry.funcFamily()) {
      always_assert(!ff->m_regular);
    } else if (auto const func = entry.func()) {
      always_assert(func->cls != cinfo->cls);
    }
  }

  // The subclassList is non-empty, contains this ClassInfo, and
  // contains only unique elements.
  if (!is_closure_base(*cinfo->cls)) {
    always_assert(!cinfo->subclassList.empty());
    always_assert(
      std::find(
        begin(cinfo->subclassList),
        end(cinfo->subclassList),
        cinfo
      ) != end(cinfo->subclassList)
    );
    auto cpy = cinfo->subclassList;
    std::sort(begin(cpy), end(cpy));
    cpy.erase(std::unique(begin(cpy), end(cpy)), end(cpy));
    always_assert(cpy.size() == cinfo->subclassList.size());
  } else if (is_closure(*cinfo->cls)) {
    always_assert(cinfo->subclassList.size() == 2);
    always_assert(is_closure_base(*cinfo->subclassList[0]->cls));
    always_assert(cinfo->subclassList[1] == cinfo);
  } else {
    always_assert(cinfo->subclassList.empty());
  }

  // The baseList is non-empty, and the last element is this class.
  always_assert(!cinfo->baseList.empty());
  always_assert(cinfo->baseList.back() == cinfo);
}

void check_local_invariants(const IndexData& data, const FuncFamily& ff) {
  // FuncFamily should always have more than one func on it.
  always_assert(ff.possibleFuncs().size() > 1);

  SString name{nullptr};
  FuncFamily::PossibleFunc last{nullptr, false};
  for (auto const pf : ff.possibleFuncs()) {
    // Should only contain methods
    always_assert(pf.ptr()->cls);

    // Every method on the list should have the same name.
    if (!name) {
      name = pf.ptr()->name;
    } else {
      always_assert(name == pf.ptr()->name);
    }

    // Verify the list is sorted and doesn't contain any duplicates.
    hphp_fast_set<const php::Func*> seen;
    if (last.ptr()) {
      always_assert(
        [&] {
          if (last.inRegular() && !pf.inRegular()) return true;
          if (!last.inRegular() && pf.inRegular()) return false;
          return string_data_lti{}(last.ptr()->cls->name, pf.ptr()->cls->name);
        }()
      );
    }
    always_assert(seen.emplace(pf.ptr()).second);
    last = pf;
  }

  if (!ff.possibleFuncs().front().inRegular() ||
      ff.possibleFuncs().back().inRegular()) {
    // If there's no funcs on a regular class, or if all functions are
    // on a regular class, we don't need to keep separate information
    // for the regular subset (it either doesn't exist, or it's equal to
    // the entire list).
    always_assert(!ff.m_regular);
  }
}

void check_local_invariants(const IndexData& data) {
  if (!debug) return;

  trace_time timer{"check-local-invariants"};

  parallel::for_each(
    data.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      check_local_invariants(data, cinfo.get());
    }
  );

  std::vector<const FuncFamily*> funcFamilies;
  funcFamilies.reserve(data.funcFamilies.size());
  for (auto const& [ff, _] : data.funcFamilies) {
    funcFamilies.emplace_back(ff.get());
  }
  parallel::for_each(
    funcFamilies,
    [&] (const FuncFamily* ff) { check_local_invariants(data, *ff); }
  );
}

//////////////////////////////////////////////////////////////////////

Type adjust_closure_context(const Index& index, const CallContext& ctx) {
  if (ctx.callee->cls && ctx.callee->cls->closureContextCls) {
    auto const withClosureContext = Context {
      ctx.callee->unit,
      ctx.callee,
      index.lookup_closure_context(*ctx.callee->cls)
    };
    if (auto const s = selfCls(index, withClosureContext)) {
      return setctx(toobj(*s));
    }
    return TObj;
  }
  return ctx.context;
}

Index::ReturnType context_sensitive_return_type(IndexData& data,
                                                const Context& ctx,
                                                CallContext callCtx,
                                                Index::ReturnType returnType) {
  constexpr auto max_interp_nexting_level = 2;
  static __thread uint32_t interp_nesting_level;
  auto const finfo = func_info(data, callCtx.callee);

  auto const adjustedCtx = adjust_closure_context(*data.m_index, callCtx);
  returnType.t = return_with_context(std::move(returnType.t), adjustedCtx);

  auto const checkParam = [&] (int i) {
    auto const& constraint = finfo->func->params[i].typeConstraint;
    if (constraint.hasConstraint() &&
        !constraint.isTypeVar() &&
        !constraint.isTypeConstant()) {
      auto const ctx = Context {
        finfo->func->unit,
        finfo->func,
        finfo->func->cls
      };
      return callCtx.args[i].strictlyMoreRefined(
        lookup_constraint(*data.m_index, ctx, constraint).upper
      );
    }
    return callCtx.args[i].strictSubtypeOf(TInitCell);
  };

  // TODO(#3788877): more heuristics here would be useful.
  auto const tryContextSensitive = [&] {
    if (finfo->func->noContextSensitiveAnalysis ||
        finfo->func->params.empty() ||
        interp_nesting_level + 1 >= max_interp_nexting_level ||
        returnType.t.is(BBottom)) {
      return false;
    }

    if (finfo->retParam != NoLocalId &&
        callCtx.args.size() > finfo->retParam &&
        checkParam(finfo->retParam)) {
      return true;
    }

    if (!options.ContextSensitiveInterp) return false;

    if (callCtx.args.size() < finfo->func->params.size()) return true;
    for (auto i = 0; i < finfo->func->params.size(); i++) {
      if (checkParam(i)) return true;
    }
    return false;
  }();

  if (!tryContextSensitive) return returnType;

  {
    ContextRetTyMap::const_accessor acc;
    if (data.contextualReturnTypes.find(acc, callCtx)) {
      if (data.frozen ||
          acc->second.t.is(BBottom) ||
          is_scalar(acc->second.t)) {
        return acc->second;
      }
    }
  }

  if (data.frozen) return returnType;

  auto contextType = [&] {
    ++interp_nesting_level;
    SCOPE_EXIT { --interp_nesting_level; };

    auto const func = finfo->func;
    auto const wf = php::WideFunc::cns(func);
    auto const calleeCtx = AnalysisContext {
      func->unit,
      wf,
      func->cls,
      &ctx.forDep()
    };
    auto fa = analyze_func_inline(
      *data.m_index,
      calleeCtx,
      adjustedCtx,
      callCtx.args
    );
    return Index::ReturnType{
      return_with_context(std::move(fa.inferredReturn), adjustedCtx),
      fa.effectFree
    };
  }();

  if (!interp_nesting_level) {
    FTRACE(3,
           "Context sensitive type: {}\n"
           "Context insensitive type: {}\n",
           show(contextType.t), show(returnType.t));
  }

  if (!returnType.t.subtypeOf(BUnc)) {
    // If the context insensitive return type could be non-static, staticness
    // could be a result of temporary context sensitive bytecode optimizations.
    contextType.t = loosen_staticness(std::move(contextType.t));
  }

  auto ret = Index::ReturnType{
    intersection_of(std::move(returnType.t), std::move(contextType.t)),
    returnType.effectFree && contextType.effectFree
  };

  if (!interp_nesting_level) {
    FTRACE(3, "Context sensitive result: {}\n", show(ret.t));
  }

  ContextRetTyMap::accessor acc;
  if (data.contextualReturnTypes.insert(acc, callCtx) ||
      ret.t.strictSubtypeOf(acc->second.t) ||
      (ret.effectFree && !acc->second.effectFree)) {
    acc->second = ret;
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

template<typename F> auto
visit_parent_cinfo(const ClassInfo* cinfo, F fun) -> decltype(fun(cinfo)) {
  for (auto ci = cinfo; ci != nullptr; ci = ci->parent) {
    if (auto const ret = fun(ci)) return ret;
    if (ci->cls->attrs & AttrNoExpandTrait) continue;
    for (auto ct : ci->usedTraits) {
      if (auto const ret = visit_parent_cinfo(ct, fun)) {
        return ret;
      }
    }
  }
  return {};
}

//////////////////////////////////////////////////////////////////////

// The type of a public static property, considering only it's initial
// value.
Type initial_type_for_public_sprop(const Index& index,
                                   const php::Class& cls,
                                   const php::Prop& prop) {
  /*
   * If the initializer type is TUninit, it means an 86sinit provides
   * the actual initialization type or it is AttrLateInit. So we don't
   * want to include the Uninit (which isn't really a user-visible
   * type for the property) or by the time we union things in we'll
   * have inferred nothing much.
   */
  auto const ty = from_cell(prop.val);
  if (ty.subtypeOf(BUninit)) return TBottom;
  if (prop.attrs & AttrSystemInitialValue) return ty;
  return adjust_type_for_prop(index, cls, &prop.typeConstraint, ty);
}

Type lookup_public_prop_impl(
  const IndexData& data,
  const ClassInfo* cinfo,
  SString propName
) {
  // Find a property declared in this class (or a parent) with the same name.
  const php::Class* knownCls = nullptr;
  auto const prop = visit_parent_cinfo(
    cinfo,
    [&] (const ClassInfo* ci) -> const php::Prop* {
      for (auto const& prop : ci->cls->properties) {
        if (prop.name == propName) {
          knownCls = ci->cls;
          return &prop;
        }
      }
      return nullptr;
    }
  );

  if (!prop) return TCell;
  // Make sure its non-static and public. Otherwise its another function's
  // problem.
  if (prop->attrs & (AttrStatic | AttrPrivate)) return TCell;

  // Get a type corresponding to its declared type-hint (if any).
  auto ty = adjust_type_for_prop(
    *data.m_index, *knownCls, &prop->typeConstraint, TCell
  );
  // We might have to include the initial value which might be outside of the
  // type-hint.
  auto initialTy = loosen_all(from_cell(prop->val));
  if (!initialTy.subtypeOf(TUninit) && (prop->attrs & AttrSystemInitialValue)) {
    ty |= initialTy;
  }
  return ty;
}

// Test if the given property (declared in `cls') is accessible in the
// given context (null if we're not in a class).
bool static_is_accessible(const ClassInfo* clsCtx,
                          const ClassInfo* cls,
                          const php::Prop& prop) {
  assertx(prop.attrs & AttrStatic);
  switch (prop.attrs & (AttrPublic|AttrProtected|AttrPrivate)) {
    case AttrPublic:
      // Public is accessible everywhere
      return true;
    case AttrProtected:
      // Protected is accessible from both derived classes and parent
      // classes
      return clsCtx && (clsCtx->derivedFrom(*cls) || cls->derivedFrom(*clsCtx));
    case AttrPrivate:
      // Private is only accessible from within the declared class
      return clsCtx == cls;
  }
  always_assert(false);
}

// Return true if the given class can possibly throw when its
// initialized. Initialization can happen when an object of that class
// is instantiated, or (more importantly) when static properties are
// accessed.
bool class_init_might_raise(IndexData& data,
                            Context ctx,
                            const ClassInfo* cinfo) {
  // Check this class and all of its parents for possible inequivalent
  // redeclarations or bad initial values.
  do {
    // Be conservative for now if we have unflattened traits.
    if (!cinfo->traitProps.empty()) return true;
    if (cinfo->hasBadRedeclareProp) return true;
    if (cinfo->hasBadInitialPropValues) {
      add_dependency(data, cinfo->cls, ctx, Dep::PropBadInitialValues);
      return true;
    }
    cinfo = cinfo->parent;
  } while (cinfo);
  return false;
}

/*
 * Calculate the effects of applying the given type against the
 * type-constraints for the given prop. This includes the subtype
 * which will succeed (if any), and if the type-constraint check might
 * throw.
 */
PropMergeResult prop_tc_effects(const Index& index,
                                const ClassInfo* ci,
                                const php::Prop& prop,
                                const Type& val,
                                bool checkUB) {
  assertx(prop.typeConstraint.validForProp());

  using R = PropMergeResult;

  // If we're not actually checking property type-hints, everything
  // goes
  if (RuntimeOption::EvalCheckPropTypeHints <= 0) return R{ val, TriBool::No };

  auto const ctx = Context { nullptr, nullptr, ci->cls };

  auto const check = [&] (const TypeConstraint& tc, const Type& t) {
    // If the type as is satisfies the constraint, we won't throw and
    // the type is unchanged.
      if (t.moreRefined(lookup_constraint(index, ctx, tc, t).lower)) {
      return R{ t, TriBool:: No };
    }
    // Otherwise adjust the type. If we get a Bottom we'll definitely
    // throw. We already know the type doesn't completely satisfy the
    // constraint, so we'll at least maybe throw.
    auto adjusted = adjust_type_for_prop(index, *ctx.cls, &tc, t);
    auto const throws = yesOrMaybe(adjusted.subtypeOf(BBottom));
    return R{ std::move(adjusted), throws };
  };

  // First check the main type-constraint.
  auto result = check(prop.typeConstraint, val);
  // If we're not checking generics upper-bounds, or if we already
  // know we'll fail, we're done.
  if (!checkUB ||
      RuntimeOption::EvalEnforceGenericsUB <= 0 ||
      result.throws == TriBool::Yes) {
    return result;
  }

  // Otherwise check every generic upper-bound. We'll feed the
  // narrowed type into each successive round. If we reach the point
  // where we'll know we'll definitely fail, just stop.
  for (auto const& ub : prop.ubs.m_constraints) {
    auto r = check(ub, result.adjusted);
    result.throws &= r.throws;
    result.adjusted = std::move(r.adjusted);
    if (result.throws == TriBool::Yes) break;
  }

  return result;
}

/*
 * Lookup data for the static property named `propName', starting from
 * the specified class `start'. If `propName' is nullptr, then any
 * accessible static property in the class hierarchy is considered. If
 * `startOnly' is specified, if the property isn't found in `start',
 * it is treated as a lookup failure. Otherwise the lookup continues
 * in all parent classes of `start', until a property is found, or
 * until all parent classes have been exhausted (`startOnly' is used
 * to avoid redundant class hierarchy walks). `clsCtx' is the current
 * context, converted to a ClassInfo* (or nullptr if not in a class).
*/
PropLookupResult lookup_static_impl(IndexData& data,
                                    Context ctx,
                                    const ClassInfo* clsCtx,
                                    const PropertiesInfo& privateProps,
                                    const ClassInfo* start,
                                    SString propName,
                                    bool startOnly) {
  ITRACE(
    6, "lookup_static_impl: {} {} {}\n",
    clsCtx ? clsCtx->cls->name->toCppString() : std::string{"-"},
    start->cls->name,
    propName ? propName->toCppString() : std::string{"*"}
  );
  Trace::Indent _;

  auto const type = [&] (const php::Prop& prop,
                         const ClassInfo* ci) {
    switch (prop.attrs & (AttrPublic|AttrProtected|AttrPrivate)) {
      case AttrPublic:
      case AttrProtected: {
        if (ctx.unit) add_dependency(data, &prop, ctx, Dep::PublicSProp);
        if (!data.seenPublicSPropMutations) {
          // If we haven't recorded any mutations yet, we need to be
          // conservative and consider only the type-hint and initial
          // value.
          return union_of(
            adjust_type_for_prop(
              *data.m_index,
              *ci->cls,
              &prop.typeConstraint,
              TInitCell
            ),
            initial_type_for_public_sprop(*data.m_index, *ci->cls, prop)
          );
        }
        auto const it = ci->publicStaticProps.find(propName);
        if (it == end(ci->publicStaticProps)) {
          // We've recorded mutations, but have information for this
          // property. That means there's no mutations so only
          // consider the initial value.
          return initial_type_for_public_sprop(*data.m_index, *ci->cls, prop);
        }
        return it->second.inferredType;
      }
      case AttrPrivate: {
        assertx(clsCtx == ci);
        auto const elem = privateProps.readPrivateStatic(prop.name);
        if (!elem) return TInitCell;
        return remove_uninit(elem->ty);
      }
    }
    always_assert(false);
  };

  auto const initMightRaise = class_init_might_raise(data, ctx, start);

  auto const fromProp = [&] (const php::Prop& prop,
                             const ClassInfo* ci) {
    // The property was definitely found. Compute its attributes
    // from the prop metadata.
    return PropLookupResult{
      type(prop, ci),
      propName,
      TriBool::Yes,
      yesOrNo(prop.attrs & AttrIsConst),
      yesOrNo(prop.attrs & AttrIsReadonly),
      yesOrNo(prop.attrs & AttrLateInit),
      yesOrNo(prop.attrs & AttrInternal),
      initMightRaise
    };
  };

  auto const notFound = [&] {
    // The property definitely wasn't found.
    return PropLookupResult{
      TBottom,
      propName,
      TriBool::No,
      TriBool::No,
      TriBool::No,
      TriBool::No,
      TriBool::No,
      false
    };
  };

  if (!propName) {
    // We don't statically know the prop name. Walk up the hierarchy
    // and union the data for any accessible static property.
    ITRACE(4, "no prop name, considering all accessible\n");
    auto result = notFound();
    visit_parent_cinfo(
      start,
      [&] (const ClassInfo* ci) {
        for (auto const& prop : ci->cls->properties) {
          if (!(prop.attrs & AttrStatic) ||
              !static_is_accessible(clsCtx, ci, prop)) {
            ITRACE(
              6, "skipping inaccessible {}::${}\n",
              ci->cls->name, prop.name
            );
            continue;
          }
          auto const r = fromProp(prop, ci);
          ITRACE(6, "including {}:${} {}\n", ci->cls->name, prop.name, show(r));
          result |= r;
        }
        // If we're only interested in the starting class, don't walk
        // up to the parents.
        return startOnly;
      }
    );
    return result;
  }

  // We statically know the prop name. Walk up the hierarchy and stop
  // at the first matching property and use that data.
  assertx(!startOnly);
  auto const result = visit_parent_cinfo(
    start,
    [&] (const ClassInfo* ci) -> Optional<PropLookupResult> {
      for (auto const& prop : ci->cls->properties) {
        if (prop.name != propName) continue;
        // We have a matching prop. If its not static or not
        // accessible, the access will not succeed.
        if (!(prop.attrs & AttrStatic) ||
            !static_is_accessible(clsCtx, ci, prop)) {
          ITRACE(
            6, "{}::${} found but inaccessible, stopping\n",
            ci->cls->name, propName
          );
          return notFound();
        }
        // Otherwise its a match
        auto const r = fromProp(prop, ci);
        ITRACE(6, "found {}:${} {}\n", ci->cls->name, propName, show(r));
        return r;
      }
      return std::nullopt;
    }
  );
  if (!result) {
    // We walked up to all of the base classes and didn't find a
    // property with a matching name. The access will fail.
    ITRACE(6, "nothing found\n");
    return notFound();
  }
  return *result;
}

/*
 * Lookup the static property named `propName', starting from the
 * specified class `start'. If an accessible property is found, then
 * merge the given type `val' into the already known type for that
 * property. If `propName' is nullptr, then any accessible static
 * property in the class hierarchy is considered. If `startOnly' is
 * specified, if the property isn't found in `start', then the nothing
 * is done. Otherwise the lookup continues in all parent classes of
 * `start', until a property is found, or until all parent classes
 * have been exhausted (`startOnly' is to avoid redundant class
 * hierarchy walks). `clsCtx' is the current context, converted to a
 * ClassInfo* (or nullptr if not in a class). If `ignoreConst' is
 * false, then AttrConst properties will not have their type
 * modified. `mergePublic' is a lambda with the logic to merge a type
 * for a public property (this is needed to avoid cyclic
 * dependencies).
 */
template <typename F>
PropMergeResult merge_static_type_impl(IndexData& data,
                                       Context ctx,
                                       F mergePublic,
                                       PropertiesInfo& privateProps,
                                       const ClassInfo* clsCtx,
                                       const ClassInfo* start,
                                       SString propName,
                                       const Type& val,
                                       bool checkUB,
                                       bool ignoreConst,
                                       bool mustBeReadOnly,
                                       bool startOnly) {
  ITRACE(
    6, "merge_static_type_impl: {} {} {} {}\n",
    clsCtx ? clsCtx->cls->name->toCppString() : std::string{"-"},
    start->cls->name,
    propName ? propName->toCppString() : std::string{"*"},
    show(val)
  );
  Trace::Indent _;

  assertx(!val.subtypeOf(BBottom));

  // Perform the actual merge for a given property, returning the
  // effects of that merge.
  auto const merge = [&] (const php::Prop& prop, const ClassInfo* ci) {
    // First calculate the effects of the type-constraint.
    auto const effects = prop_tc_effects(*data.m_index, ci, prop, val, checkUB);
    // No point in merging if the type-constraint will always fail.
    if (effects.throws == TriBool::Yes) {
      ITRACE(
        6, "tc would throw on {}::${} with {}, skipping\n",
        ci->cls->name, prop.name, show(val)
      );
      return effects;
    }
    assertx(!effects.adjusted.subtypeOf(BBottom));

    ITRACE(
      6, "merging {} into {}::${}\n",
      show(effects), ci->cls->name, prop.name
    );

    switch (prop.attrs & (AttrPublic|AttrProtected|AttrPrivate)) {
      case AttrPublic:
      case AttrProtected:
        mergePublic(ci, prop, unctx(effects.adjusted));
         // If the property is internal, accessing it may throw
         // TODO(T131951529): we can do better by checking modules here
        if ((prop.attrs & AttrInternal) && effects.throws == TriBool::No) {
          ITRACE(6, "{}::${} is internal, "
                 "being pessimistic with regards to throwing\n",
                 ci->cls->name, prop.name);
          return PropMergeResult{
            effects.adjusted,
            TriBool::Maybe
          };
        }
        return effects;
      case AttrPrivate: {
        assertx(clsCtx == ci);
        privateProps.mergeInPrivateStaticPreAdjusted(
          prop.name,
          unctx(effects.adjusted)
        );
        return effects;
      }
    }
    always_assert(false);
  };

  // If we don't find a property, then the mutation will definitely
  // fail.
  auto const notFound = [&] {
    return PropMergeResult{
      TBottom,
      TriBool::Yes
    };
  };

  if (!propName) {
    // We don't statically know the prop name. Walk up the hierarchy
    // and merge the type for any accessible static property.
    ITRACE(6, "no prop name, considering all accessible\n");
    auto result = notFound();
    visit_parent_cinfo(
      start,
      [&] (const ClassInfo* ci) {
        for (auto const& prop : ci->cls->properties) {
          if (!(prop.attrs & AttrStatic) ||
              !static_is_accessible(clsCtx, ci, prop)) {
            ITRACE(
              6, "skipping inaccessible {}::${}\n",
              ci->cls->name, prop.name
            );
            continue;
          }
          if (!ignoreConst && (prop.attrs & AttrIsConst)) {
            ITRACE(6, "skipping const {}::${}\n", ci->cls->name, prop.name);
            continue;
          }
          if (mustBeReadOnly && !(prop.attrs & AttrIsReadonly)) {
            ITRACE(6, "skipping mutable property that must be readonly {}::${}\n",
              ci->cls->name, prop.name);
            continue;
          }
          result |= merge(prop, ci);
        }
        return startOnly;
      }
    );
    return result;
  }

  // We statically know the prop name. Walk up the hierarchy and stop
  // at the first matching property and merge the type there.
  assertx(!startOnly);
  auto result = visit_parent_cinfo(
    start,
    [&] (const ClassInfo* ci) -> Optional<PropMergeResult> {
      for (auto const& prop : ci->cls->properties) {
        if (prop.name != propName) continue;
        // We found a property with the right name, but its
        // inaccessible from this context (or not even static). This
        // mutation will fail, so we don't need to modify the type.
        if (!(prop.attrs & AttrStatic) ||
            !static_is_accessible(clsCtx, ci, prop)) {
          ITRACE(
            6, "{}::${} found but inaccessible, stopping\n",
            ci->cls->name, propName
          );
          return notFound();
        }
        // Mutations to AttrConst properties will fail as well, unless
        // it we want to override that behavior.
        if (!ignoreConst && (prop.attrs & AttrIsConst)) {
          ITRACE(
            6, "{}:${} found but const, stopping\n",
            ci->cls->name, propName
          );
          return notFound();
        }
        if (mustBeReadOnly && !(prop.attrs & AttrIsReadonly)) {
          ITRACE(
            6, "{}:${} found but is mutable and must be readonly, stopping\n",
            ci->cls->name, propName
          );
          return notFound();
        }
        return merge(prop, ci);
      }
      return std::nullopt;
    }
  );
  if (!result) {
    ITRACE(6, "nothing found\n");
    return notFound();
  }

  // If the mutation won't throw, we still need to check if the class
  // initialization can throw. If we might already throw (or
  // definitely will throw), this doesn't matter.
  if (result->throws == TriBool::No) {
    return PropMergeResult{
      std::move(result->adjusted),
      maybeOrNo(class_init_might_raise(data, ctx, start))
    };
  }
  return *result;
}

//////////////////////////////////////////////////////////////////////

/*
 * Split a group of buckets so that no bucket is larger (including its
 * dependencies) than the given max size. The given callable is used
 * to obtain the dependencies of bucket item.
 *
 * Note: if a single item has dependencies larger than maxSize, you'll
 * get a bucket with just that and its dependencies (which will be
 * larger than maxSize). This is the only situation where a returned
 * bucket will be larger than maxSize.
 */
template <typename GetDeps>
std::vector<std::vector<SString>>
split_buckets(const std::vector<std::vector<SString>>& items,
              size_t maxSize,
              const GetDeps& getDeps) {
  // Split all of the buckets in parallel
  auto rebuckets = parallel::map(
    items,
    [&] (const std::vector<SString>& bucket) {
      // If there's only one thing in a bucket, there's no point in
      // splitting it.
      if (bucket.size() <= 1) return singleton_vec(bucket);

      // The splitting algorithm is simple. Iterate over each element
      // in the bucket. As long as all the dependencies are less than
      // the maximum size, we put it into a new bucket. If we exceed
      // the max size, create a new bucket.
      std::vector<std::vector<SString>> out;
      out.emplace_back();
      out.back().emplace_back(bucket[0]);

      auto allDeps = getDeps(bucket[0]);
      for (size_t i = 1, size = bucket.size(); i < size; ++i) {
        auto const& d = getDeps(bucket[i]);
        allDeps.insert(begin(d), end(d));
        auto const newSize = allDeps.size() + out.back().size() + 1;
        if (newSize > maxSize) {
          allDeps = d;
          out.emplace_back();
        }
        out.back().emplace_back(bucket[i]);
      }
      return out;
    }
  );

  // Flatten all of the new buckets into a single list of buckets.
  std::vector<std::vector<SString>> flattened;
  flattened.reserve(items.size());
  for (auto& r : rebuckets) {
    for (auto& b : r) flattened.emplace_back(std::move(b));
  }
  return flattened;
}

//////////////////////////////////////////////////////////////////////

/*
 * For efficiency reasons, we often want to process classes in as few
 * passes as possible. However, this is tricky because the algorithms
 * are usually naturally iterative. You start at the root classes
 * (which may be the top classes in the hierarchy, or leaf classes),
 * and flow data down to each of their parents or children. This
 * requires N passes, where N is the maximum depth of the class
 * hierarchy. N can get large.
 *
 * Instead when we process a class, we ensure that all of it's
 * dependencies (all the way up to the roots) are also present in the
 * job. Since we're doing this in one pass, none of the dependencies
 * will have any calculated information, and the job will have to do
 * this first.
 *
 * It is not, in general, possible to ensure that each dependency is
 * present in exactly one job (because the dependency may be shared by
 * lots of classes which are not bucketed together). So, any given
 * dependency may end up on multiple jobs and have the same
 * information calculated for it. This is fine, as it just results in
 * some wasted work.
 *
 * We perform flattening using the following approach:
 *
 * - First we Bucketize the root classes (using the standard
 *   consistent hashing algorithm) into N buckets.
 *
 * - We split any buckets which are larger than the specified maximum
 *   size. This prevents buckets from becoming pathologically large if
 *   there's many dependencies.
 *
 * - For each bucket, find all of the (transitive) dependencies of the
 *   leaves and add them to that bucket (as dependencies). As stated
 *   above, the same class may end up in multiple buckets as
 *   dependencies.
 *
 * - So far for each bucket (each bucket will map to one job), we have
 *   a set of input classes (the roots), and all of the dependencies
 *   for each roots.
 *
 * - We want results for every class, not just the roots, so the
 *   dependencies need to become inputs of the first kind in at least
 *   one bucket. So, for each dependency, in one of the buckets
 *   they're already present in, we "promote" it to a full input (and
 *   will receive output for it). This is done by hashing the bucket
 *   index and class name and picking the bucket that results in the
 *   lowest hash. In some situations we don't want a dependency to
 *   ever be promoted, so those will be skipped.
*/

// Single output bucket for assign_hierarchical_work. Each bucket
// contains classes which will be processed and returned as output,
// and a set of dependency classes which will just be used as inputs.
struct HierarchicalWorkBucket {
  std::vector<SString> classes;
  std::vector<SString> deps;
};

/*
 * Assign work for a set of root classes (using the above
 * algorithm). The function is named because it's meant for situations
 * where we're processing classes in a "hierarchical" manner (either
 * from parent class to children, or from leaf class to parents).
 *
 * The dependencies for each class is provided by the getDeps
 * callable. For the purposes of promoting a class to a full output
 * (see above algorithm description), each class must be assigned an
 * index. The (optional) index for a class is provided by the getIdx
 * callable. If getIdx returns std::nullopt, then that class won't be
 * considered for promotion. The given "numClasses" parameter is an
 * upper bound on the possible returned indices.
 */
template <typename GetDeps, typename GetIdx>
std::vector<HierarchicalWorkBucket>
assign_hierarchical_work(std::vector<SString> roots,
                        size_t numClasses,
                        size_t bucketSize,
                        size_t maxSize,
                        const GetDeps& getDeps,
                        const GetIdx& getIdx) {
  // First turn roots into buckets, and split if any exceed the
  // maximum size.
  auto buckets = split_buckets(
    consistently_bucketize(roots, bucketSize),
    maxSize,
    [&] (SString cls) -> const ISStringSet& {
      auto const [d, _] = getDeps(cls);
      return *d;
    }
  );

  struct DepHashState {
    std::mutex lock;
    size_t lowestHash{std::numeric_limits<size_t>::max()};
    size_t lowestBucket{std::numeric_limits<size_t>::max()};
  };
  std::vector<DepHashState> depHashState{numClasses};

  // For each bucket (which right now just contains the root classes),
  // find all the transitive dependencies those root classes need. A
  // dependency might end up in multiple buckets (because multiple
  // roots in different buckets depend on it). We only want to
  // actually perform the flattening for those dependencies in one of
  // the buckets. So, we need a tie-breaker. We hash the name of the
  // dependency along with the bucket number. The bucket that the
  // dependency is present in with the lowest hash is what "wins".
  auto const bucketDeps = parallel::gen(
    buckets.size(),
    [&] (size_t bucketIdx) {
      assertx(bucketIdx < buckets.size());
      auto& bucket = buckets[bucketIdx];

      // Gather up all dependencies for this bucket, and remove
      // non-instantiable root classes.
      ISStringSet deps;
      bucket.erase(
        std::remove_if(
          begin(bucket),
          end(bucket),
          [&] (SString cls) {
            auto const [d, instantiable] = getDeps(cls);
            deps.insert(begin(*d), end(*d));
            return !instantiable;
          }
        ),
        end(bucket)
      );

      // Make sure dependencies and roots are disjoint.
      for (auto const c : bucket) deps.erase(c);

      // For each dependency, store the bucket with the lowest hash.
      for (auto const d : deps) {
        auto const idx = getIdx(d);
        if (!idx.has_value()) continue;
        assertx(*idx < depHashState.size());
        auto& s = depHashState[*idx];
        auto const hash = hash_int64_pair(
          d->hashStatic(),
          bucketIdx
        );
        std::lock_guard<std::mutex> _{s.lock};
        if (hash < s.lowestHash) {
          s.lowestHash = hash;
          s.lowestBucket = bucketIdx;
        } else if (hash == s.lowestHash) {
          s.lowestBucket = std::min(s.lowestBucket, bucketIdx);
        }
      }

      return deps;
    }
  );

  // Now for each bucket, "promote" dependencies into a full input
  // class. The dependency is promoted in the bucket with the lowest
  // hash, which we've already calculated.
  assertx(buckets.size() == bucketDeps.size());
  return parallel::gen(
    buckets.size(),
    [&] (size_t bucketIdx) {
      auto& bucket = buckets[bucketIdx];
      auto const& deps = bucketDeps[bucketIdx];

      std::vector<SString> depOut;
      depOut.reserve(deps.size());

      for (auto const d : deps) {
        // Calculate the hash for the dependency for this bucket. If
        // the hash equals the already calculated lowest hash, promote
        // this dependency.
        auto const idx = getIdx(d);
        if (!idx.has_value()) {
          depOut.emplace_back(d);
          continue;
        }
        assertx(*idx < depHashState.size());
        auto const& s = depHashState[*idx];
        auto const hash = hash_int64_pair(
          d->hashStatic(),
          bucketIdx
        );
        if (hash == s.lowestHash && bucketIdx == s.lowestBucket) {
          bucket.emplace_back(d);
        } else {
          // Otherwise keep it as a dependency.
          depOut.emplace_back(d);
        }
      }

      // Keep deterministic ordering. Make sure there's no duplicates.
      std::sort(bucket.begin(), bucket.end(), string_data_lti{});
      std::sort(depOut.begin(), depOut.end(), string_data_lti{});
      assertx(std::adjacent_find(bucket.begin(), bucket.end()) == bucket.end());
      assertx(std::adjacent_find(depOut.begin(), depOut.end()) == depOut.end());
      return HierarchicalWorkBucket{
        std::move(bucket),
        std::move(depOut)
      };
    }
  );
}

//////////////////////////////////////////////////////////////////////
// Class flattening:

const StaticString
  s___Sealed("__Sealed"),
  s___EnableMethodTraitDiamond("__EnableMethodTraitDiamond"),
  s___ModuleLevelTrait("__ModuleLevelTrait");

/*
 * Extern-worker job to build ClassInfo2s (which involves flattening
 * data across the hierarchy) and flattening traits.
 */
struct FlattenJob {
  static std::string name() { return "hhbbc-flatten"; }
  static void init(const Config& config) {
    process_init(config.o, config.gd, false);
  }
  static void fini() {}

  /*
   * Metadata representing results of flattening. This is information
   * that the local coordinator (as opposed to later remote jobs) will
   * need to consume.
   */
  struct OutputMeta {
    // Classes which have been determined to be uninstantiable
    // (therefore have no result output data).
    ISStringSet uninstantiable;
    // New closures produced from trait flattening, grouped by the
    // unit they belong to. Such new closures will require "fixups" in
    // the php::Program data.
    struct NewClosures {
      SString unit;
      std::vector<SString> names;
      template <typename SerDe> void serde(SerDe& sd) {
        sd(unit)(names);
      }
    };
    std::vector<NewClosures> newClosures;
    // Report parents of each class. A class is a parent of another if
    // it would appear on a subclass list. The parents of a closure
    // are not reported because that's implicit.
    struct Parents {
      std::vector<SString> names;
      template <typename SerDe> void serde(SerDe& sd) { sd(names); }
    };
    std::vector<Parents> parents;
    // Classes which are interfaces.
    ISStringSet interfaces;
    // The types used by the type-constraints of input classes and
    // functions.
    std::vector<ISStringSet> classTypeUses;
    std::vector<ISStringSet> funcTypeUses;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(uninstantiable, string_data_lti{})
        (newClosures)
        (parents)
        (interfaces, string_data_lti{})
        (classTypeUses, string_data_lti{})
        (funcTypeUses, string_data_lti{})
        ;
    }
  };

  /*
   * Job returns a list of (potentially modified) php::Class, a list
   * of new ClassInfo2, a list of (potentially modified) php::Func,
   * and metadata for the entire job. The order of the lists reflects
   * the order of the input classes and functions(skipping over
   * classes marked as uninstantiable in the metadata).
   */
  using Output = Multi<
    Variadic<std::unique_ptr<php::Class>>,
    Variadic<std::unique_ptr<php::ClassBytecode>>,
    Variadic<std::unique_ptr<ClassInfo2>>,
    Variadic<std::unique_ptr<php::Func>>,
    Variadic<std::unique_ptr<FuncInfo2>>,
    OutputMeta
  >;

  /*
   * Job takes a list of classes which are to be flattened. In
   * addition to this, it also takes a list of classes which are
   * dependencies of the classes to be flattened. (A class might be
   * one of the inputs *and* a dependency, in which case it should
   * just be on the input list). It is expected that *all*
   * dependencies are provided. All instantiable classes will have
   * their type-constraints resolved to their ultimate type, or left
   * as unresolved if it refers to a missing/invalid type. The
   * provided functions only have their type-constraints updated. The
   * provided type-mappings and list of missing types is used for
   * type-constraint resolution (if a type isn't in a type mapping and
   * isn't a missing type, it is assumed to be a object type).
   *
   * Bytecode needs to be provided for every provided class. The
   * bytecode must be provided in the same order as the bytecode's
   * associated class.
   */
  static Output run(Variadic<std::unique_ptr<php::Class>> classes,
                    Variadic<std::unique_ptr<php::Class>> deps,
                    Variadic<std::unique_ptr<php::ClassBytecode>> classBytecode,
                    Variadic<std::unique_ptr<php::Func>> funcs,
                    std::vector<TypeMapping> typeMappings,
                    std::vector<SString> missingTypes) {
    LocalIndex index;

    for (auto& tc : typeMappings) {
      auto const name = tc.name;
      always_assert(index.m_typeMappings.emplace(name, std::move(tc)).second);
    }
    for (auto const m : missingTypes) {
      always_assert(index.m_missingTypes.emplace(m).second);
    }
    typeMappings.clear();
    missingTypes.clear();

    // Bytecode should have been provided for every class provided.
    always_assert(
      classBytecode.vals.size() == (classes.vals.size() + deps.vals.size())
    );
    // Store the provided bytecode in the matching php::Class.
    for (size_t i = 0,
         size = classBytecode.vals.size(),
         classesSize = classes.vals.size();
         i < size; ++i) {
      auto& cls = i < classesSize
        ? classes.vals[i]
        : deps.vals[i - classesSize];
      auto& bytecode = classBytecode.vals[i];

      always_assert(bytecode->methodBCs.size() == cls->methods.size());
      for (size_t j = 0, bcSize = bytecode->methodBCs.size(); j < bcSize; ++j) {
        cls->methods[j]->rawBlocks = std::move(bytecode->methodBCs[j].bc);
      }
    }
    classBytecode.vals.clear();

    // Some classes might be dependencies of another. Moreover, some
    // classes might share dependencies. Topologically sort all of the
    // classes and process them in that order. Information will flow
    // from parent classes to their children.
    auto const worklist = prepare(
      index,
      [&] {
        ISStringToOneT<php::Class*> out;
        out.reserve(classes.vals.size() + deps.vals.size());
        for (auto const& c : classes.vals) {
          always_assert(out.emplace(c->name, c.get()).second);
        }
        for (auto const& c : deps.vals) {
          always_assert(out.emplace(c->name, c.get()).second);
        }
        return out;
      }()
    );

    std::vector<std::unique_ptr<php::Class>> newClosures;
    newClosures.reserve(worklist.size());

    for (auto const cls : worklist) {
      Trace::Bump bumper{
        Trace::hhbbc_index, kSystemLibBump, is_systemlib_part(cls->unit)
      };

      ITRACE(2, "flatten class: {}\n", cls->name);
      Trace::Indent indent;

      index.m_ctx = cls;
      SCOPE_EXIT { index.m_ctx = nullptr; };

      auto state = std::make_unique<State>();
      // Attempt to make the ClassInfo2 for this class. If we can't,
      // it means the class is not instantiable.
      auto cinfo = make_info(index, *cls, *state);
      if (!cinfo) {
        ITRACE(4, "{} is not instantiable\n", cls->name);
        always_assert(index.m_uninstantiable.emplace(cls->name).second);
        continue;
      }

      ITRACE(5, "adding state for class '{}' to local index\n", cls->name);
      assertx(cinfo->name->isame(cls->name));

      // We might look up this class when flattening itself, so add it
      // to the local index before we start.
      always_assert(index.m_classes.emplace(cls->name, cls).second);

      auto const [cinfoIt, cinfoSuccess] =
        index.m_classInfos.emplace(cls->name, std::move(cinfo));
      always_assert(cinfoSuccess);

      auto const [stateIt, stateSuccess] =
        index.m_states.emplace(cls->name, std::move(state));
      always_assert(stateSuccess);

      auto closures =
        flatten_traits(index, *cls, *cinfoIt->second, *stateIt->second);

      assertx(IMPLIES(is_closure(*cls), cinfoIt->second->closures.empty()));
      std::sort(
        begin(cinfoIt->second->closures),
        end(cinfoIt->second->closures),
        string_data_lti{}
      );

      // Trait flattening may produce new closures, so those need to
      // be added to the local index as well.
      for (auto& [c, i] : closures) {
        ITRACE(5, "adding state for closure '{}' to local index\n", c->name);
        assertx(c->name->isame(i->name));
        assertx(is_closure(*c));
        assertx(i->closures.empty());
        always_assert(index.m_classes.emplace(c->name, c.get()).second);
        always_assert(index.m_classInfos.emplace(c->name, std::move(i)).second);
        newClosures.emplace_back(std::move(c));
      }
    }

    // Format the output data and put it in a deterministic order.
    Variadic<std::unique_ptr<php::Class>> outClasses;
    Variadic<std::unique_ptr<ClassInfo2>> outInfos;
    OutputMeta outMeta;
    ISStringSet outNames;

    outClasses.vals.reserve(classes.vals.size() + newClosures.size());
    outInfos.vals.reserve(classes.vals.size() + newClosures.size());
    outMeta.newClosures.reserve(classes.vals.size());
    outNames.reserve(classes.vals.size());
    outMeta.parents.reserve(classes.vals.size() + newClosures.size());

    // Do the processing which relies on a fully accessible
    // LocalIndex
    for (auto& cls : classes.vals) {
      auto const cinfoIt = index.m_classInfos.find(cls->name);
      if (cinfoIt == end(index.m_classInfos)) {
        always_assert(index.uninstantiable(cls->name));
        outMeta.uninstantiable.emplace(cls->name);
        continue;
      }
      auto& cinfo = cinfoIt->second;

      index.m_ctx = cls.get();
      SCOPE_EXIT { index.m_ctx = nullptr; };

      outMeta.classTypeUses.emplace_back();
      update_type_constraints(index, *cls, &outMeta.classTypeUses.back());
      optimize_properties(index, *cls, *cinfo);
      for (auto const& func : cls->methods) {
        cinfo->funcInfos.emplace_back(make_func_info(index, *func));
      }

      outNames.emplace(cls->name);
    }

    std::sort(
      begin(newClosures),
      end(newClosures),
      [] (const std::unique_ptr<php::Class>& a,
          const std::unique_ptr<php::Class>& b) {
        if (a->unit != b->unit) return string_data_lt{}(a->unit, b->unit);
        return string_data_lti{}(a->name, b->name);
      }
    );

    for (auto& clo : newClosures) {
      assertx(is_closure(*clo));
      assertx(clo->closureContextCls);
      assertx(clo->unit);

      // Only return the closures for classes we were actually
      // requested to flatten.
      if (!outNames.count(clo->closureContextCls)) continue;

      auto& cinfo = index.m_classInfos.at(clo->name);

      index.m_ctx = clo.get();
      SCOPE_EXIT { index.m_ctx = nullptr; };

      outMeta.classTypeUses.emplace_back();
      update_type_constraints(index, *clo, &outMeta.classTypeUses.back());
      optimize_properties(index, *clo, *cinfo);

      for (auto const& func : clo->methods) {
        cinfo->funcInfos.emplace_back(make_func_info(index, *func));
      }
    }

    // Now move the classes out of LocalIndex and into the output. At
    // this point, it's not safe to access the LocalIndex unless
    // you're sure something hasn't been moved yet.
    for (auto& cls : classes.vals) {
      auto const name = cls->name;
      auto const cinfoIt = index.m_classInfos.find(name);
      if (cinfoIt == end(index.m_classInfos)) {
        assertx(outMeta.uninstantiable.count(name));
        continue;
      }
      auto& cinfo = cinfoIt->second;

      index.m_ctx = cls.get();
      SCOPE_EXIT { index.m_ctx = nullptr; };

      // For building FuncFamily::StaticInfo, we need to ensure that
      // every method has an entry in methodFamilies. Make all of the
      // initial entries here (they'll be created assuming this method
      // is AttrNoOverride).
      for (auto const& [methname, mte] : cinfo->methods) {
        if (is_special_method_name(methname)) continue;
        auto entry = make_initial_func_family_entry(*cls, index.meth(mte), mte);
        always_assert(
          cinfo->methodFamilies.emplace(methname, std::move(entry)).second
        );
      }

      if (!is_closure(*cls)) {
        auto const& state = index.m_states.at(name);
        outMeta.parents.emplace_back();
        auto& parents = outMeta.parents.back().names;
        parents.reserve(state->m_parents.size());
        for (auto const p : state->m_parents) {
          parents.emplace_back(p->name);
        }
      }

      if (cls->attrs & AttrInterface) {
        outMeta.interfaces.emplace(name);
      }

      outClasses.vals.emplace_back(std::move(cls));
      outInfos.vals.emplace_back(std::move(cinfo));
    }

    for (auto& clo : newClosures) {
      // Only return the closures for classes we were actually
      // requested to flatten.
      if (!outNames.count(clo->closureContextCls)) continue;

      index.m_ctx = clo.get();
      SCOPE_EXIT { index.m_ctx = nullptr; };

      auto& outNewClosures = outMeta.newClosures;
      if (outNewClosures.empty() || outNewClosures.back().unit != clo->unit) {
        outNewClosures.emplace_back();
        outNewClosures.back().unit = clo->unit;
      }
      outNewClosures.back().names.emplace_back(clo->name);

      auto& cinfo = index.m_classInfos.at(clo->name);
      outClasses.vals.emplace_back(std::move(clo));
      outInfos.vals.emplace_back(std::move(cinfo));
    }

    Variadic<std::unique_ptr<FuncInfo2>> funcInfos;
    funcInfos.vals.reserve(funcs.vals.size());
    for (auto& func : funcs.vals) {
      outMeta.funcTypeUses.emplace_back();
      update_type_constraints(index, *func, &outMeta.funcTypeUses.back());
      funcInfos.vals.emplace_back(make_func_info(index, *func));
    }

    // Provide any updated bytecode back to the caller.
    Variadic<std::unique_ptr<php::ClassBytecode>> outBytecode;
    outBytecode.vals.reserve(outClasses.vals.size());
    for (auto& cls : outClasses.vals) {
      auto bytecode = std::make_unique<php::ClassBytecode>();
      bytecode->methodBCs.reserve(cls->methods.size());
      for (auto& method : cls->methods) {
        bytecode->methodBCs.emplace_back(std::move(method->rawBlocks));
      }
      outBytecode.vals.emplace_back(std::move(bytecode));
    }

    return std::make_tuple(
      std::move(outClasses),
      std::move(outBytecode),
      std::move(outInfos),
      std::move(funcs),
      std::move(funcInfos),
      std::move(outMeta)
    );
  }

private:
  /*
   * State which needs to be propagated from a dependency to a child
   * class during flattening, but not required after flattening (so
   * doesn't belong in ClassInfo2).
   */
  struct State {
    struct PropTuple {
      SString name;
      SString src;
      php::Prop prop;
    };
    // Maintain order of properties as we inherit them.
    CompactVector<PropTuple> m_props;
    SStringToOneT<size_t> m_propIndices;
    CompactVector<php::Const> m_traitCns;
    SStringSet m_cnsFromTrait;
    SStringToOneT<size_t> m_methodIndices;
    CompactVector<const php::Class*> m_parents;

    size_t& methodIdx(SString context, SString cls, SString name) {
      auto const it = m_methodIndices.find(name);
      always_assert_flog(
        it != m_methodIndices.end(),
        "While processing '{}', "
        "tried to access missing method index for '{}::{}'",
        context, cls, name
      );
      return it->second;
    }

    size_t methodIdx(SString context, SString cls, SString name) const {
      return const_cast<State*>(this)->methodIdx(context, cls, name);
    }
  };

  /*
   * LocalIndex is similar to Index, but for this job. It maps names
   * to class information needed during flattening. It also verifies
   * we don't try to access information about a class until it's
   * actually available (which shouldn't happen if our dataflow is
   * correct).
   */
  struct LocalIndex {
    const php::Class* m_ctx{nullptr};

    ISStringToOneT<const php::Class*> m_classes;
    ISStringToOneT<std::unique_ptr<ClassInfo2>> m_classInfos;
    ISStringToOneT<std::unique_ptr<State>> m_states;

    hphp_fast_map<
      const php::Class*,
      hphp_fast_set<const php::Class*>
    > m_classClosures;

    ISStringSet m_uninstantiable;

    ISStringToOneT<TypeMapping> m_typeMappings;
    ISStringSet m_missingTypes;

    const php::Class& cls(SString name) const {
      if (m_ctx->name->isame(name)) return *m_ctx;
      auto const it = m_classes.find(name);
      always_assert_flog(
        it != m_classes.end(),
        "While processing '{}', tried to access missing class '{}' from index",
        m_ctx->name,
        name
      );
      assertx(it->second);
      return *it->second;
    }

    const ClassInfo2& classInfo(SString name) const {
      auto const it = m_classInfos.find(name);
      always_assert_flog(
        it != m_classInfos.end(),
        "While processing '{}', tried to access missing class-info for '{}' "
        "from index",
        m_ctx->name,
        name
      );
      assertx(it->second.get());
      return *it->second;
    }

    const State& state(SString name) const {
      auto const it = m_states.find(name);
      always_assert_flog(
        it != m_states.end(),
        "While processing '{}', tried to access missing flatten state for '{}' "
        "from index",
        m_ctx->name,
        name
      );
      assertx(it->second.get());
      return *it->second;
    }

    const hphp_fast_set<const php::Class*>* classClosures(
      const php::Class& cls
    ) const {
      auto const it = m_classClosures.find(&cls);
      if (it == m_classClosures.end()) return nullptr;
      return &it->second;
    }

    bool uninstantiable(SString name) const {
      return m_uninstantiable.count(name);
    }

    const TypeMapping* typeMapping(SString name) const {
      return folly::get_ptr(m_typeMappings, name);
    }

    bool missingType(SString name) const {
      return m_missingTypes.count(name);
    }

    const php::Func& meth(const MethRef& r) const {
      auto const& mcls = cls(r.cls);
      assertx(r.idx < mcls.methods.size());
      return *mcls.methods[r.idx];
    }
    const php::Func& meth(const MethTabEntry& mte) const {
      return meth(mte.meth());
    }

    const php::Const& cns(const ClassInfo2::ConstIndex& idx) const {
      auto const& c = cls(idx.cls);
      assertx(idx.idx < c.constants.size());
      return c.constants[idx.idx];
    }

    size_t methodIdx(SString cls, SString name) const {
      return state(cls).methodIdx(m_ctx->name, cls, name);
    }
  };

  /*
   * Calculate the order in which the classes should be flattened,
   * taking into account dependencies.
   */
  static std::vector<php::Class*> prepare(
    LocalIndex& index,
    const ISStringToOneT<php::Class*>& classes
  ) {
    // We might not have any classes if we're just processing funcs.
    if (classes.empty()) return {};

    auto const get = [&] (SString name) -> php::Class& {
      auto const it = classes.find(name);
      always_assert_flog(
        it != classes.end(),
        "Tried to access missing class '{}' while calculating flattening order",
        name
      );
      return *it->second;
    };

    for (auto const [_, cls] : classes) {
      if (!cls->closureContextCls) continue;
      assertx(is_closure(*cls));
      auto const it = classes.find(cls->closureContextCls);
      // Closure's context may not necessarily be present.
      if (it == classes.end()) continue;
      auto& ctx = *it->second;
      index.m_classClosures[&ctx].emplace(cls);
    }

    auto const forEachDep = [&] (php::Class& c, auto const& f) {
      if (c.parentName) f(get(c.parentName));
      for (auto const i : c.interfaceNames)    f(get(i));
      for (auto const e : c.includedEnumNames) f(get(e));
      for (auto const t : c.usedTraitNames)    f(get(t));
      if (auto const closures = index.classClosures(c)) {
        for (auto const clo : *closures) {
          f(const_cast<php::Class&>(*clo));
        }
      }
    };

    /*
     * Perform a standard topological sort:
     *
     * - For each class, calculate the number of classes which depend on it.
     *
     * - Any class which has a use count of zero is not depended on by
     *   anyone and goes onto the intitial worklist.
     *
     * - For every class on the worklist, push it onto the output
     *   list, and decrement the use count of all of it's
     *   dependencies.
     *
     * - For any class which now has a use count of zero, push it onto
     *   the worklist and repeat above step until all classes are
     *   pushed onto the output list.
     *
     * - Reverse the list.
     *
     * - This does not handle cycles, but we should not encounter any
     *   here, as such cycles should be detected earlier and not be
     *   scheduled in a job.
     */
    hphp_fast_map<const php::Class*, size_t> uses;
    uses.reserve(classes.size());
    for (auto const& [_, cls] : classes) {
      forEachDep(*cls, [&] (const php::Class& d) { ++uses[&d]; });
    }

    std::vector<php::Class*> worklist;
    for (auto const [_, cls] : classes) {
      if (!uses[cls]) worklist.emplace_back(cls);
    }
    always_assert(!worklist.empty());
    std::sort(
      worklist.begin(),
      worklist.end(),
      [] (const php::Class* c1, const php::Class* c2) {
        return string_data_lti{}(c1->name, c2->name);
      }
    );

    std::vector<php::Class*> ordered;
    ordered.reserve(classes.size());
    do {
      auto const cls = worklist.back();
      assertx(!uses[cls]);
      worklist.pop_back();
      forEachDep(
        *cls,
        [&] (php::Class& d) {
          if (!--uses.at(&d)) worklist.emplace_back(&d);
        }
      );
      ordered.emplace_back(cls);
    } while (!worklist.empty());

    for (auto const& [_, cls] : classes) always_assert(!uses.at(cls));
    std::reverse(ordered.begin(), ordered.end());
    return ordered;
  }

  /*
   * Create a FuncFamilyEntry for the give method. This
   * FuncFamilyEntry assumes that the method is AttrNoOverride, and
   * hence reflects just this method. The method isn't necessarily
   * actually AttrNoOverride, but if not, it will be updated in
   * BuildSubclassListJob (that job needs the initial entries).
   */
  static FuncFamilyEntry make_initial_func_family_entry(
    const php::Class& cls,
    const php::Func& meth,
    const MethTabEntry& mte
  ) {
    FuncFamilyEntry entry;
    entry.m_allIncomplete = false;
    entry.m_regularIncomplete = false;
    entry.m_privateAncestor = is_regular_class(cls) && mte.hasPrivateAncestor();

    FuncFamilyEntry::MethMetadata meta;

    for (size_t i = 0; i < meth.params.size(); ++i) {
      meta.m_prepKinds.emplace_back(func_param_prep(&meth, i));
    }
    // Any param beyond the size of m_paramPreps is implicitly
    // TriBool::No, so we can drop trailing entries which are
    // TriBool::No.
    while (!meta.m_prepKinds.empty()) {
      auto& back = meta.m_prepKinds.back();
      if (back.inOut != TriBool::No || back.readonly != TriBool::No) break;
      meta.m_prepKinds.pop_back();
    }
    meta.m_numInOut = func_num_inout(&meth);
    meta.m_nonVariadicParams = numNVArgs(meth);
    meta.m_coeffectRules = meth.coeffectRules;
    meta.m_requiredCoeffects = meth.requiredCoeffects;
    meta.m_isReadonlyReturn = meth.isReadonlyReturn;
    meta.m_isReadonlyThis = meth.isReadonlyThis;
    meta.m_supportsAER = func_supports_AER(&meth);
    meta.m_isReified = meth.isReified;
    meta.m_caresAboutDyncalls = (dyn_call_error_level(&meth) > 0);
    meta.m_builtin = meth.attrs & AttrBuiltin;

    if (is_regular_class(cls)) {
      entry.m_meths =
        FuncFamilyEntry::BothSingle{mte.meth(), std::move(meta), false};
    } else if (bool(mte.attrs & AttrPrivate) && mte.topLevel()) {
      entry.m_meths =
        FuncFamilyEntry::BothSingle{mte.meth(), std::move(meta), true};
    } else {
      entry.m_meths =
        FuncFamilyEntry::SingleAndNone{mte.meth(), std::move(meta)};
    }

    return entry;
  }

  static std::unique_ptr<ClassInfo2> make_info(const LocalIndex& index,
                                               php::Class& cls,
                                               State& state) {
    if (debug && (is_closure(cls) || is_closure_base(cls))) {
      if (is_closure(cls)) {
        always_assert(cls.parentName->isame(s_Closure.get()));
      } else {
        always_assert(!cls.parentName);
      }
      always_assert(cls.interfaceNames.empty());
      always_assert(cls.includedEnumNames.empty());
      always_assert(cls.usedTraitNames.empty());
      always_assert(cls.requirements.empty());
      always_assert(cls.constants.empty());
      always_assert(cls.userAttributes.empty());
      always_assert(!(cls.attrs & (AttrTrait | AttrInterface | AttrAbstract)));
    }

    // Set up some initial values for ClassInfo properties. If this
    // class is a leaf (we can't actually determine that yet), these
    // will be valid and remain as-is. If not, they'll be updated
    // properly when calculating subclass information in another pass.
    auto cinfo = std::make_unique<ClassInfo2>();
    cinfo->name = cls.name;
    cinfo->hasConstProp = cls.hasConstProp;
    cinfo->hasReifiedParent = cls.hasReifiedGenerics;
    cinfo->hasReifiedGeneric = cls.userAttributes.count(s___Reified.get());
    cinfo->subHasReifiedGeneric = cinfo->hasReifiedGeneric;
    cinfo->initialNoReifiedInit = cls.attrs & AttrNoReifiedInit;
    cinfo->isMockClass = is_mock_class(&cls);
    cinfo->isRegularClass = is_regular_class(cls);

    if (!is_closure_base(cls)) {
      attribute_setter(cls.attrs, true, AttrNoOverride);
      attribute_setter(cls.attrs, true, AttrNoOverrideRegular);
    } else {
      cinfo->hasRegularSubclass = true;
      cinfo->hasNonRegularSubclass = false;
      attribute_setter(cls.attrs, false, AttrNoOverride);
      attribute_setter(cls.attrs, false, AttrNoOverrideRegular);
    }

    // Assume this. If not a leaf, will be updated in
    // BuildSubclassList job.
    attribute_setter(cls.attrs, true, AttrNoMock);

    if (cls.parentName) {
      assertx(!is_closure_base(cls));
      assertx(is_closure(cls) == cls.parentName->isame(s_Closure.get()));

      if (index.uninstantiable(cls.parentName)) {
        ITRACE(2,
               "Making class-info failed for `{}' because "
               "its parent `{}' is uninstantiable\n",
               cls.name, cls.parentName);
        return nullptr;
      }
      auto const& parent = index.cls(cls.parentName);
      auto const& parentInfo = index.classInfo(cls.parentName);

      assertx(!is_closure(parent));
      if (parent.attrs & (AttrInterface | AttrTrait)) {
        ITRACE(2,
               "Making class-info failed for `{}' because "
               "its parent `{}' is not a class\n",
               cls.name, cls.parentName);
        return nullptr;
      }
      if (!enforce_sealing(*cinfo, parent)) return nullptr;

      cinfo->parent = cls.parentName;
      cinfo->baseList = parentInfo.baseList;
      cinfo->implInterfaces = parentInfo.implInterfaces;
      cinfo->hasConstProp |= parentInfo.hasConstProp;
      cinfo->hasReifiedParent |= parentInfo.hasReifiedParent;

      state.m_parents.emplace_back(&parent);
    } else if (!cinfo->hasReifiedGeneric) {
      attribute_setter(cls.attrs, true, AttrNoReifiedInit);
    }
    cinfo->baseList.emplace_back(cls.name);

    for (auto const iname : cls.interfaceNames) {
      assertx(!is_closure(cls));
      assertx(!is_closure_base(cls));
      if (index.uninstantiable(iname)) {
        ITRACE(2,
               "Making class-info failed for `{}' because "
               "{} is uninstantiable\n",
               cls.name, iname);
        return nullptr;
      }
      auto const& iface = index.cls(iname);
      auto const& ifaceInfo = index.classInfo(iname);

      assertx(!is_closure(iface));
      if (!(iface.attrs & AttrInterface)) {
        ITRACE(2,
               "Making class-info failed for `{}' because `{}' "
               "is not an interface\n",
               cls.name, iname);
        return nullptr;
      }
      if (!enforce_sealing(*cinfo, iface)) return nullptr;

      cinfo->declInterfaces.emplace_back(iname);
      cinfo->implInterfaces.insert(
        ifaceInfo.implInterfaces.begin(),
        ifaceInfo.implInterfaces.end()
      );
      cinfo->hasReifiedParent |= ifaceInfo.hasReifiedParent;

      state.m_parents.emplace_back(&iface);
    }

    for (auto const ename : cls.includedEnumNames) {
      assertx(!is_closure(cls));
      assertx(!is_closure_base(cls));
      if (index.uninstantiable(ename)) {
        ITRACE(2,
               "Making class-info failed for `{}' because "
               "{} is uninstantiable\n",
               cls.name, ename);
        return nullptr;
      }
      auto const& e = index.cls(ename);
      auto const& einfo = index.classInfo(ename);

      assertx(!is_closure(e));
      auto const wantAttr = cls.attrs & (AttrEnum | AttrEnumClass);
      if (!(e.attrs & wantAttr)) {
        ITRACE(2,
               "Making class-info failed for `{}' because `{}' "
               "is not an enum{}\n",
               cls.name, ename,
               wantAttr & AttrEnumClass ? " class" : "");
        return nullptr;
      }
      if (!enforce_sealing(*cinfo, e)) return nullptr;

      cinfo->includedEnums.emplace_back(ename);
      cinfo->implInterfaces.insert(
        einfo.implInterfaces.begin(),
        einfo.implInterfaces.end()
      );
    }

    auto clsHasModuleLevelTrait = cls.userAttributes.count(s___ModuleLevelTrait.get());
    if (clsHasModuleLevelTrait && (!(cls.attrs & AttrTrait) || (cls.attrs & AttrInternal))) {
      ITRACE(2,
             "Making class-info failed for `{}' because "
             "attribute <<__ModuleLevelTrait>> can only be specified on public traits\n",
             cls.name);
      return nullptr;
    }

    for (auto const tname : cls.usedTraitNames) {
      assertx(!is_closure(cls));
      assertx(!is_closure_base(cls));
      if (index.uninstantiable(tname)) {
        ITRACE(2,
               "Making class-info failed for `{}' because "
               "{} is uninstantiable\n",
               cls.name, tname);
        return nullptr;
      }
      auto const& trait = index.cls(tname);
      auto const& traitInfo = index.classInfo(tname);

      if (clsHasModuleLevelTrait && !(trait.userAttributes.count(s___ModuleLevelTrait.get()))) {
        ITRACE(2,
               "Making class-info failed for `{}' because "
               "it has attribute <<__ModuleLevelTrait>> but uses trait {} which doesn't\n",
               cls.name, trait.name);
        return nullptr;
      }

      assertx(!is_closure(trait));
      if (!(trait.attrs & AttrTrait)) {
        ITRACE(2,
               "Making class-info failed for `{}' because `{}' "
               "is not a trait\n",
               cls.name, tname);
        return nullptr;
      }
      if (!enforce_sealing(*cinfo, trait)) return nullptr;

      cinfo->usedTraits.emplace_back(tname);
      cinfo->implInterfaces.insert(
        traitInfo.implInterfaces.begin(),
        traitInfo.implInterfaces.end()
      );
      cinfo->hasConstProp |= traitInfo.hasConstProp;
      cinfo->hasReifiedParent |= traitInfo.hasReifiedParent;

      state.m_parents.emplace_back(&trait);
    }

    if (cls.attrs & AttrEnum) {
      auto const baseType = [&] {
        auto const& base = cls.enumBaseTy;
        if (!base.isUnresolved()) return base.type();
        auto const tm = index.typeMapping(base.typeName());
        if (!tm) return AnnotType::Unresolved;
        // enums cannot use case types
        assertx(tm->typeAndValueUnion.size() == 1);
        return tm->typeAndValueUnion[0].type;
      }();
      if (!enumSupportsAnnot(baseType)) {
        ITRACE(2,
               "Making class-info failed for `{}' because {} "
               "is not a valid enum base type\n",
               cls.name, annotName(baseType));
        return nullptr;
      }
    }

    if (cls.attrs & AttrInterface) cinfo->implInterfaces.emplace(cls.name);

    if (cls.attrs & (AttrInterface | AttrTrait | AttrEnum | AttrEnumClass)) {
      cinfo->subImplInterfaces.emplace();
    }

    // One can specify implementing the same interface multiple
    // times. It's easier if we ensure the list is unique.
    std::sort(
      begin(cinfo->declInterfaces),
      end(cinfo->declInterfaces),
      string_data_lti{}
    );
    cinfo->declInterfaces.erase(
      std::unique(
        begin(cinfo->declInterfaces), end(cinfo->declInterfaces),
        [&] (SString a, SString b) { return a->isame(b); }
      ),
      end(cinfo->declInterfaces)
    );

    if (!build_methods(index, cls, *cinfo, state))    return nullptr;
    if (!build_properties(index, cls, *cinfo, state)) return nullptr;
    if (!build_constants(index, cls, *cinfo, state))  return nullptr;

    if (auto const closures = index.classClosures(cls)) {
      for (auto const clo : *closures) {
        assertx(is_closure(*clo));
        assertx(
          clo->closureContextCls && clo->closureContextCls->isame(cls.name)
        );
        // Ensure closures have been processed already
        if (debug) index.cls(clo->name);
        cinfo->closures.emplace_back(clo->name);
      }
    }

    std::sort(
      begin(state.m_parents),
      end(state.m_parents),
      [] (const php::Class* a, const php::Class* b) {
        return string_data_lti{}(a->name, b->name);
      }
    );
    state.m_parents.erase(
      std::unique(begin(state.m_parents), end(state.m_parents)),
      end(state.m_parents)
    );
    assertx(
      std::none_of(
        begin(state.m_parents), end(state.m_parents),
        [] (const php::Class* c) { return is_closure(*c); }
      )
    );

    if (!is_closure_base(cls)) {
      cinfo->subclassList.emplace_back(cls.name);
    }
    cinfo->subHasConstProp = cinfo->hasConstProp;

    // All methods are originally not overridden (we'll update this as
    // necessary later), except for special methods, which are always
    // considered to be overridden.
    for (auto& [name, mte] : cinfo->methods) {
      assertx(!cinfo->missingMethods.count(name));
      if (is_special_method_name(name)) {
        attribute_setter(mte.attrs, false, AttrNoOverride);
        mte.clearNoOverrideRegular();
      } else {
        attribute_setter(mte.attrs, true, AttrNoOverride);
        mte.setNoOverrideRegular();
      }
    }

    // We don't calculate subclass information for closures, so make
    // sure their initial values are all what they should be.
    if (debug && (is_closure(cls) || is_closure_base(cls))) {
      if (is_closure(cls)) {
        always_assert(is_closure_name(cls.name));
        always_assert(state.m_parents.size() == 1);
        always_assert(state.m_parents[0]->name->isame(s_Closure.get()));
        always_assert(!(cls.attrs & AttrNoReifiedInit));
      } else {
        always_assert(state.m_parents.empty());
        always_assert(cls.attrs & AttrNoReifiedInit);
      }
      always_assert(cinfo->closures.empty());
      always_assert(cinfo->missingMethods.empty());
      always_assert(!cinfo->hasConstProp);
      always_assert(!cinfo->subHasConstProp);
      always_assert(!cinfo->hasReifiedParent);
      always_assert(!cinfo->hasReifiedGeneric);
      always_assert(!cinfo->subHasReifiedGeneric);
      always_assert(!cinfo->initialNoReifiedInit);
      always_assert(!cinfo->isMockClass);
      always_assert(cinfo->isRegularClass);
    }

    ITRACE(2, "new class-info: {}\n", cls.name);
    if (Trace::moduleEnabled(Trace::hhbbc_index, 3)) {
      if (cinfo->parent) {
        ITRACE(3, "           parent: {}\n", cinfo->parent);
      }
      for (auto const DEBUG_ONLY base : cinfo->baseList) {
        ITRACE(3, "             base: {}\n", base);
      }
      for (auto const DEBUG_ONLY iface : cinfo->declInterfaces) {
        ITRACE(3, "  decl implements: {}\n", iface);
      }
      for (auto const DEBUG_ONLY iface : cinfo->implInterfaces) {
        ITRACE(3, "       implements: {}\n", iface);
      }
      for (auto const DEBUG_ONLY e : cinfo->includedEnums) {
        ITRACE(3, "             enum: {}\n", e);
      }
      for (auto const DEBUG_ONLY trait : cinfo->usedTraits) {
        ITRACE(3, "             uses: {}\n", trait);
      }
      for (auto const DEBUG_ONLY closure : cinfo->closures) {
        ITRACE(3, "          closure: {}\n", closure);
      }
    }

    return cinfo;
  }

  static bool enforce_sealing(const ClassInfo2& cinfo,
                              const php::Class& parent) {
    if (cinfo.isMockClass) return true;
    if (!(parent.attrs & AttrSealed)) return true;
    auto const it = parent.userAttributes.find(s___Sealed.get());
    assertx(it != parent.userAttributes.end());
    assertx(tvIsArrayLike(it->second));
    auto allowed = false;
    IterateV(
      it->second.m_data.parr,
      [&] (TypedValue v) {
        assertx(tvIsStringLike(v));
        if (tvAssertStringLike(v)->isame(cinfo.name)) {
          allowed = true;
          return true;
        }
        return false;
      }
    );
    if (!allowed) {
      ITRACE(
        2,
        "Making class-info failed for `{}' because "
        "`{}' is sealed\n",
        cinfo.name, parent.name
      );
    }
    return allowed;
  }

  static bool build_properties(const LocalIndex& index,
                               const php::Class& cls,
                               ClassInfo2& cinfo,
                               State& state) {
    if (cls.parentName) {
      auto const& parentState = index.state(cls.parentName);
      state.m_props = parentState.m_props;
      state.m_propIndices = parentState.m_propIndices;
    }

    for (auto const iface : cinfo.declInterfaces) {
      if (!merge_properties(cinfo, state, index.state(iface))) {
        return false;
      }
    }
    for (auto const trait : cinfo.usedTraits) {
      if (!merge_properties(cinfo, state, index.state(trait))) {
        return false;
      }
    }
    for (auto const e : cinfo.includedEnums) {
      if (!merge_properties(cinfo, state, index.state(e))) {
        return false;
      }
    }

    if (cls.attrs & AttrInterface) return true;

    auto clsHasModuleLevelTrait = cls.userAttributes.count(s___ModuleLevelTrait.get());
    for (auto const& p : cls.properties) {
      if (!add_property(cinfo, state, p.name, p, cinfo.name, false, clsHasModuleLevelTrait)) {
        return false;
      }
    }

    // There's no need to do this work if traits have been flattened
    // already, or if the top level class has no traits.  In those
    // cases, we might be able to rule out some instantiations, but it
    // doesn't seem worth it.
    if (cls.attrs & AttrNoExpandTrait) return true;

    for (auto const traitName : cinfo.usedTraits) {
      auto const& trait = index.cls(traitName);
      auto const& traitInfo = index.classInfo(traitName);
      for (auto const& p : trait.properties) {
        if (!add_property(cinfo, state, p.name, p, cinfo.name, true, false)) {
          return false;
        }
      }
      for (auto const& p : traitInfo.traitProps) {
        if (!add_property(cinfo, state, p.name, p, cinfo.name, true, false)) {
          return false;
        }
      }
    }

    return true;
  }

  static bool add_property(ClassInfo2& cinfo,
                           State& state,
                           SString name,
                           const php::Prop& prop,
                           SString src,
                           bool trait,
                           bool moduleLevelTrait) {
    if (moduleLevelTrait && (prop.attrs & AttrInternal)) {
      ITRACE(2,
             "Adding property failed for `{}' because "
             "property `{}' is internal and public traits cannot define internal properties\n",
             cinfo.name, prop.name);
      return false;
    }
    auto const [it, emplaced] =
      state.m_propIndices.emplace(name, state.m_props.size());
    if (emplaced) {
      state.m_props.emplace_back(State::PropTuple{name, src, prop});
      if (trait) cinfo.traitProps.emplace_back(prop);
      return true;
    }
    assertx(it->second < state.m_props.size());
    auto& prevTuple = state.m_props[it->second];
    auto const& prev = prevTuple.prop;
    auto const prevSrc = prevTuple.src;

    if (cinfo.name->isame(prevSrc)) {
      if ((prev.attrs ^ prop.attrs) &
          (AttrStatic | AttrPublic | AttrProtected | AttrPrivate) ||
          (!(prop.attrs & AttrSystemInitialValue) &&
           !(prev.attrs & AttrSystemInitialValue) &&
           !Class::compatibleTraitPropInit(prev.val, prop.val))) {
        ITRACE(2,
               "Adding property failed for `{}' because "
               "two declarations of `{}' at the same level had "
               "different attributes\n",
               cinfo.name, prop.name);
        return false;
      }
      return true;
    }

    if (!(prev.attrs & AttrPrivate)) {
      if ((prev.attrs ^ prop.attrs) & AttrStatic) {
        ITRACE(2,
               "Adding property failed for `{}' because "
               "`{}' was defined both static and non-static\n",
               cinfo.name, prop.name);
        return false;
      }
      if (prop.attrs & AttrPrivate) {
        ITRACE(2,
               "Adding property failed for `{}' because "
               "`{}' was re-declared private\n",
               cinfo.name, prop.name);
        return false;
      }
      if (prop.attrs & AttrProtected && !(prev.attrs & AttrProtected)) {
        ITRACE(2,
               "Adding property failed for `{}' because "
               "`{}' was redeclared protected from public\n",
               cinfo.name, prop.name);
        return false;
      }
    }

    if (trait) cinfo.traitProps.emplace_back(prop);
    prevTuple = State::PropTuple{name, src, prop};
    return true;
  }

  static bool merge_properties(ClassInfo2& cinfo,
                               State& dst,
                               const State& src) {
    for (auto const& [name, src, prop] : src.m_props) {
      if (!add_property(cinfo, dst, name, prop, src, false, false)) {
        return false;
      }
    }
    return true;
  }

  static bool build_constants(const LocalIndex& index,
                              php::Class& cls,
                              ClassInfo2& cinfo,
                              State& state) {
    if (cls.parentName) {
      cinfo.clsConstants = index.classInfo(cls.parentName).clsConstants;
      state.m_cnsFromTrait = index.state(cls.parentName).m_cnsFromTrait;
    }

    for (auto const iname : cinfo.declInterfaces) {
      auto const& iface = index.classInfo(iname);
      auto const& ifaceState = index.state(iname);
      for (auto const& [cnsName, cnsIdx] : iface.clsConstants) {
        auto const added = add_constant(
          index, cinfo, state, cnsName,
          cnsIdx, ifaceState.m_cnsFromTrait.count(cnsName)
        );
        if (!added) return false;
      }
    }

    auto const addShallowConstants = [&] {
      auto const numConstants = cls.constants.size();
      for (uint32_t idx = 0; idx < numConstants; ++idx) {
        auto const added = add_constant(
          index, cinfo, state,
          cls.constants[idx].name,
          ClassInfo2::ConstIndex { cls.name, idx },
          false
        );
        if (!added) return false;
      }
      return true;
    };

    auto const addTraitConstants = [&] {
      for (auto const tname : cinfo.usedTraits) {
        auto const& trait = index.classInfo(tname);
        for (auto const& [cnsName, cnsIdx] : trait.clsConstants) {
          auto const added = add_constant(
            index, cinfo, state, cnsName,
            cnsIdx, true
          );
          if (!added) return false;
        }
      }
      return true;
    };

    if (RO::EvalTraitConstantInterfaceBehavior) {
      // trait constants must be inserted before constants shallowly
      // declared on the class to match the interface semantics
      if (!addTraitConstants()) return false;
      if (!addShallowConstants()) return false;
    } else {
      if (!addShallowConstants()) return false;
      if (!addTraitConstants()) return false;
    }

    for (auto const ename : cinfo.includedEnums) {
      auto const& e = index.classInfo(ename);
      for (auto const& [cnsName, cnsIdx] : e.clsConstants) {
        auto const added = add_constant(
          index, cinfo, state, cnsName,
          cnsIdx, false
        );
        if (!added) return false;
      }
    }

    auto const addTraitConst = [&] (const php::Const& c) {
     /*
      * Only copy in constants that win. Otherwise, in the runtime, if
      * we have a constant from an interface implemented by a trait
      * that wins over this fromTrait constant, we won't know which
      * trait it came from, and therefore won't know which constant
      * should win. Dropping losing constants here works because if
      * they fatal with constants in declared interfaces, we catch that
      * above.
      */
      auto const& existing = cinfo.clsConstants.find(c.name);
      if (existing->second.cls->isame(c.cls)) {
        state.m_traitCns.emplace_back(c);
        state.m_traitCns.back().isFromTrait = true;
      }
    };
    for (auto const tname : cinfo.usedTraits) {
      auto const& trait      = index.cls(tname);
      auto const& traitState = index.state(tname);
      for (auto const& c : trait.constants)       addTraitConst(c);
      for (auto const& c : traitState.m_traitCns) addTraitConst(c);
    }

    if (cls.attrs & (AttrAbstract | AttrInterface | AttrTrait)) return true;

    std::vector<SString> sortedClsConstants;
    sortedClsConstants.reserve(cinfo.clsConstants.size());
    for (auto const& [name, _] : cinfo.clsConstants) {
      sortedClsConstants.emplace_back(name);
    }
    std::sort(
      sortedClsConstants.begin(),
      sortedClsConstants.end(),
      string_data_lt{}
    );

    for (auto const name : sortedClsConstants) {
      auto& cnsIdx = cinfo.clsConstants.find(name)->second;
      if (cnsIdx.cls->isame(cls.name)) continue;

      auto const& cns = index.cns(cnsIdx);
      if (!cns.isAbstract || !cns.val) continue;

      if (cns.val->m_type == KindOfUninit) {
        auto const& cnsCls = index.cls(cnsIdx.cls);
        assertx(!cnsCls.methods.empty());
        assertx(cnsCls.methods.back()->name == s_86cinit.get());
        auto const& cnsCInit = *cnsCls.methods.back();

        if (cls.methods.empty() ||
            cls.methods.back()->name != s_86cinit.get()) {
          ClonedClosures clonedClosures;
          auto cloned = clone(
            index,
            cnsCInit,
            cnsCInit.name,
            cnsCInit.attrs,
            cls,
            clonedClosures,
            true
          );
          assertx(cloned);
          assertx(clonedClosures.empty());
          assertx(cloned->cls == &cls);
          cloned->clsIdx = cls.methods.size();
          auto const DEBUG_ONLY emplaced =
            cinfo.methods.emplace(cloned->name, MethTabEntry { *cloned });
          assertx(emplaced.second);
          cls.methods.emplace_back(std::move(cloned));
        } else {
          auto const DEBUG_ONLY succeeded =
            append_86cinit(cls.methods.back().get(), cnsCInit);
          assertx(succeeded);
        }
      }

      // This is similar to trait constant flattening
      auto copy = cns;
      copy.cls = cls.name;
      copy.isAbstract = false;
      state.m_cnsFromTrait.erase(copy.name);

      cnsIdx.cls = cls.name;
      cnsIdx.idx = cls.constants.size();
      cls.constants.emplace_back(std::move(copy));
    }

    return true;
  }

  static bool add_constant(const LocalIndex& index,
                           ClassInfo2& cinfo,
                           State& state,
                           SString name,
                           const ClassInfo2::ConstIndex& cnsIdx,
                           bool fromTrait) {
    auto [it, emplaced] = cinfo.clsConstants.emplace(name, cnsIdx);
    if (emplaced) {
      if (fromTrait) {
        always_assert(state.m_cnsFromTrait.emplace(name).second);
      } else {
        always_assert(!state.m_cnsFromTrait.count(name));
      }
      return true;
    }
    auto& existingIdx = it->second;

    // Same constant (from an interface via two different paths) is ok
    if (existingIdx.cls->isame(cnsIdx.cls)) return true;

    auto const& existingCnsCls = index.cls(existingIdx.cls);
    auto const& existing = index.cns(existingIdx);
    auto const& cns = index.cns(cnsIdx);

    if (existing.kind != cns.kind) {
      ITRACE(
        2,
        "Adding constant failed for `{}' because `{}' was defined by "
        "`{}' as a {} and by `{}' as a {}\n",
        cinfo.name,
        name,
        cnsIdx.cls,
        ConstModifiers::show(cns.kind),
        existingIdx.cls,
        ConstModifiers::show(existing.kind)
      );
      return false;
    }

    // Ignore abstract constants
    if (cns.isAbstract && !cns.val) return true;
    // If the existing constant in the map is concrete, then don't
    // overwrite it with an incoming abstract constant's default
    if (!existing.isAbstract && cns.isAbstract) return true;

    if (existing.val) {
      /*
       * A constant from a declared interface collides with a constant
       * (Excluding constants from interfaces a trait implements).
       *
       * Need this check otherwise constants from traits that conflict
       * with declared interfaces will silently lose and not conflict
       * in the runtime.
       *
       * Type and Context constants can be overridden.
       */
      auto const& cnsCls = index.cls(cnsIdx.cls);
      if (cns.kind == ConstModifiers::Kind::Value &&
          !existing.isAbstract &&
          (existingCnsCls.attrs & AttrInterface) &&
          !((cnsCls.attrs & AttrInterface) && fromTrait)) {
        for (auto const iface : cinfo.declInterfaces) {
          if (existingIdx.cls->isame(iface)) {
            ITRACE(
              2,
              "Adding constant failed for `{}' because "
              "`{}' was defined by both `{}' and `{}'\n",
              cinfo.name,
              name,
              cnsIdx.cls,
              existingIdx.cls
            );
            return false;
          }
        }
      }

      // Constants from traits silently lose
      if (!RO::EvalTraitConstantInterfaceBehavior && fromTrait) return true;

      if ((cnsCls.attrs & AttrInterface ||
           (RO::EvalTraitConstantInterfaceBehavior &&
            (cnsCls.attrs & AttrTrait))) &&
          (existing.isAbstract ||
           cns.kind == ConstModifiers::Kind::Type)) {
        // Because existing has val, this covers the case where it is
        // abstract with default allow incoming to win.  Also, type
        // constants from interfaces may be overridden even if they're
        // not abstract.
      } else {
        // A constant from an interface or from an included enum
        // collides with an existing constant.
        if (cnsCls.attrs & (AttrInterface | AttrEnum | AttrEnumClass) ||
            (RO::EvalTraitConstantInterfaceBehavior &&
             (cnsCls.attrs & AttrTrait))) {
          ITRACE(
            2,
            "Adding constant failed for `{}' because "
            "`{}' was defined by both `{}' and `{}'\n",
            cinfo.name,
            name,
            cnsIdx.cls,
            existingIdx.cls
          );
          return false;
        }
      }
    }

    if (fromTrait) {
      state.m_cnsFromTrait.emplace(name);
    } else {
      state.m_cnsFromTrait.erase(name);
    }
    existingIdx = cnsIdx;
    return true;
  }

  /*
   * Make a flattened table of the methods on this class.
   *
   * Duplicate method names override parent methods, unless the parent
   * method is final and the class is not a __MockClass, in which case
   * this class definitely would fatal if ever defined.
   *
   * Note: we're leaving non-overridden privates in their subclass
   * method table, here. This isn't currently "wrong", because calling
   * it would be a fatal, but note that resolve_method needs to be
   * pretty careful about privates and overriding in general.
   */
  static bool build_methods(const LocalIndex& index,
                            const php::Class& cls,
                            ClassInfo2& cinfo,
                            State& state) {
    // Since interface methods are not inherited, any methods in
    // interfaces this class implements are automatically missing.
    assertx(cinfo.methods.empty());
    for (auto const iname : cinfo.declInterfaces) {
      auto const& iface = index.classInfo(iname);
      for (auto const& [name, _] : iface.methods) {
        if (is_special_method_name(name)) continue;
        cinfo.missingMethods.emplace(name);
      }
      for (auto const name : iface.missingMethods) {
        assertx(!is_special_method_name(name));
        cinfo.missingMethods.emplace(name);
      }
    }

    // Interface methods are just stubs which return null. They don't
    // get inherited by their implementations.
    if (cls.attrs & AttrInterface) {
      assertx(!cls.parentName);
      assertx(cinfo.usedTraits.empty());
      uint32_t idx = cinfo.methods.size();
      assertx(!idx);
      for (auto const& m : cls.methods) {
        auto const res = cinfo.methods.emplace(m->name, MethTabEntry { *m });
        always_assert(res.second);
        always_assert(state.m_methodIndices.emplace(m->name, idx++).second);
        if (cinfo.missingMethods.count(m->name)) {
          assertx(!res.first->second.firstName());
          cinfo.missingMethods.erase(m->name);
        } else {
          res.first->second.setFirstName();
        }
        ITRACE(4, "  {}: adding method {}::{}\n",
               cls.name, cls.name, m->name);
      }
      return true;
    }

    auto const overridden = [&] (MethTabEntry& existing,
                                 MethRef meth,
                                 Attr attrs) {
      auto const& existingMeth = index.meth(existing);
      if (existingMeth.attrs & AttrFinal) {
        if (!is_mock_class(&cls)) {
          ITRACE(
            2,
            "Adding methods failed for `{}' because "
            "it tried to override final method `{}::{}'\n",
            cls.name,
            existing.meth().cls,
            existingMeth.name
          );
          return false;
        }
      }
      ITRACE(
        4,
        "{}: overriding method {}::{} with {}::{}\n",
        cls.name,
        existing.meth().cls,
        existingMeth.name,
        meth.cls,
        existingMeth.name
      );
      if (existingMeth.attrs & AttrPrivate) {
        existing.setHasPrivateAncestor();
      }
      existing.setMeth(meth);
      existing.attrs = attrs;
      existing.setTopLevel();
      return true;
    };

    // If there's a parent, start by copying its methods
    if (cls.parentName) {
      auto const& parentInfo = index.classInfo(cls.parentName);

      assertx(cinfo.methods.empty());
      cinfo.missingMethods.insert(
        begin(parentInfo.missingMethods),
        end(parentInfo.missingMethods)
      );

      for (auto const& mte : parentInfo.methods) {
        // Don't inherit the 86* methods
        if (HPHP::Func::isSpecial(mte.first)) continue;

        auto const emplaced = cinfo.methods.emplace(mte);
        always_assert(emplaced.second);
        emplaced.first->second.clearTopLevel();
        emplaced.first->second.clearFirstName();

        always_assert(
          state.m_methodIndices.emplace(
            mte.first,
            index.methodIdx(cls.parentName, mte.first)
          ).second
        );

        cinfo.missingMethods.erase(mte.first);

        ITRACE(
          4,
          "{}: inheriting method {}::{}\n",
          cls.name,
          cls.parentName,
          mte.first
        );
      }
    }

    auto idx = cinfo.methods.size();
    auto clsHasModuleLevelTrait = cls.userAttributes.count(s___ModuleLevelTrait.get());

    // Now add our methods.
    for (auto const& m : cls.methods) {
      if (clsHasModuleLevelTrait && (m->attrs & AttrInternal)) {
        ITRACE(2,
            "Adding methods failed for `{}' because "
            "method `{}' is internal and public traits cannot define internal methods\n",
            cls.name, m->name);
        return false;
      }
      auto const emplaced = cinfo.methods.emplace(m->name, MethTabEntry { *m });
      if (emplaced.second) {
        ITRACE(
          4,
          "{}: adding method {}::{}\n",
          cls.name,
          cls.name,
          m->name
        );
        always_assert(state.m_methodIndices.emplace(m->name, idx++).second);
        if (cinfo.missingMethods.count(m->name)) {
          assertx(!emplaced.first->second.firstName());
          cinfo.missingMethods.erase(m->name);
        } else {
          emplaced.first->second.setFirstName();
        }
        continue;
      }

      // If the method is already in our table, it shouldn't be
      // missing.
      assertx(!cinfo.missingMethods.count(m->name));

      assertx(!emplaced.first->second.firstName());

      if ((m->attrs & AttrTrait) && (m->attrs & AttrAbstract)) {
        // Abstract methods from traits never override anything.
        continue;
      }
      if (!overridden(emplaced.first->second, MethRef { *m }, m->attrs)) {
        return false;
      }
    }

    // If our traits were previously flattened, we're done.
    if (cls.attrs & AttrNoExpandTrait) return true;

    try {
      TMIData tmid;
      for (auto const tname : cinfo.usedTraits) {
        auto const& tcls = index.cls(tname);
        auto const& t = index.classInfo(tname);
        std::vector<std::pair<SString, const MethTabEntry*>>
          methods(t.methods.size());
        for (auto const& [name, mte] : t.methods) {
          if (HPHP::Func::isSpecial(name)) continue;
          auto const idx = index.methodIdx(tname, name);
          assertx(!methods[idx].first);
          methods[idx] = std::make_pair(name, &mte);
          if (auto it = cinfo.methods.find(name);
              it != end(cinfo.methods)) {
            it->second.clearFirstName();
          }
        }

        for (auto const name : t.missingMethods) {
          assertx(!is_special_method_name(name));
          if (cinfo.methods.count(name)) continue;
          cinfo.missingMethods.emplace(name);
        }

        for (auto const& [name, mte] : methods) {
          if (!name) continue;
          auto const& meth = index.meth(*mte);
          tmid.add(
            TraitMethod { std::make_pair(&t, &tcls), &meth, mte->attrs },
            name
          );
        }
        for (auto const cloname : t.closures) {
          auto const& clo = index.cls(cloname);
          auto const invoke = find_method(&clo, s_invoke.get());
          assertx(invoke);
          cinfo.extraMethods.emplace(MethRef { *invoke });
        }
      }

      auto const traitMethods = tmid.finish(
        std::make_pair(&cinfo, &cls),
        cls.userAttributes.count(s___EnableMethodTraitDiamond.get())
      );

      // Import the methods.
      for (auto const& mdata : traitMethods) {
        auto const method = mdata.tm.method;
        auto attrs = mdata.tm.modifiers;

        if (attrs == AttrNone) {
          attrs = method->attrs;
        } else {
          auto const attrMask =
            (Attr)(AttrPublic | AttrProtected | AttrPrivate |
                   AttrAbstract | AttrFinal);
          attrs = (Attr)((attrs        &  attrMask) |
                         (method->attrs & ~attrMask));
        }

        auto const emplaced = cinfo.methods.emplace(
          mdata.name,
          MethTabEntry { *method, attrs }
        );
        if (emplaced.second) {
          ITRACE(
            4,
            "{}: adding trait method {}::{} as {}\n",
            cls.name,
            method->cls->name, method->name, mdata.name
          );
          always_assert(
            state.m_methodIndices.emplace(mdata.name, idx++).second
          );
          cinfo.missingMethods.erase(mdata.name);
        } else {
          assertx(!cinfo.missingMethods.count(mdata.name));
          if (attrs & AttrAbstract) continue;
          if (emplaced.first->second.meth().cls->isame(cls.name)) continue;
          if (!overridden(emplaced.first->second, MethRef { *method }, attrs)) {
            return false;
          }
          state.methodIdx(index.m_ctx->name, cinfo.name, mdata.name) = idx++;
        }
        cinfo.extraMethods.emplace(MethRef { *method });
      }
    } catch (const TMIOps::TMIException& exn) {
      ITRACE(
        2,
        "Adding methods failed for `{}' importing traits: {}\n",
        cls.name, exn.what()
      );
      return false;
    }

    return true;
  }

  using ClonedClosures =
    hphp_fast_map<const php::Class*, std::unique_ptr<php::Class>>;

  static SString rename_closure(const php::Class& closure,
                                const php::Class& newContext) {
    auto n = closure.name->slice();
    auto const p = n.find(';');
    if (p != std::string::npos) n = n.subpiece(0, p);
    return makeStaticString(folly::sformat("{};{}", n, newContext.name));
  }

  static std::unique_ptr<php::Class>
  clone_closure(const LocalIndex& index,
                const php::Class& closure,
                const php::Class& newContext,
                ClonedClosures& clonedClosures) {
    auto clone = std::make_unique<php::Class>(closure);
    assertx(clone->closureContextCls);

    clone->name = rename_closure(closure, newContext);
    clone->closureContextCls = newContext.name;
    clone->unit = newContext.unit;

    ITRACE(4, "- cloning closure {} as {} (with context {})\n",
           closure.name, clone->name, newContext.name);

    for (size_t i = 0, numMeths = clone->methods.size(); i < numMeths; ++i) {
      auto meth = std::move(clone->methods[i]);
      meth->idx = 0; // Set later
      meth->cls = clone.get();
      assertx(meth->clsIdx == i);
      if (!meth->originalFilename) meth->originalFilename = meth->unit;
      if (!meth->originalUnit)     meth->originalUnit = meth->unit;
      if (!meth->originalClass)    meth->originalClass = closure.name;
      meth->unit = newContext.unit;

      clone->methods[i] =
        clone_closures(index, std::move(meth), clonedClosures);
      if (!clone->methods[i]) return nullptr;
    }

    return clone;
  }

  static std::unique_ptr<php::Func>
  clone_closures(const LocalIndex& index,
                 std::unique_ptr<php::Func> cloned,
                 ClonedClosures& clonedClosures) {
    if (!cloned->hasCreateCl) return cloned;

    auto const onClosure = [&] (LSString& closureName) {
      auto const& cls = index.cls(closureName);
      assertx(is_closure(cls));

      // CreateCls are allowed to refer to the same closure within the
      // same func. If this is a duplicate, use the already cloned
      // closure name.
      if (auto const it = clonedClosures.find(&cls);
          it != clonedClosures.end()) {
        closureName = it->second->name;
        return true;
      }

      // Otherwise clone the closure (which gives it a new name), and
      // update the name in the CreateCl to match.
      auto closure = clone_closure(
        index,
        cls,
        cloned->cls->closureContextCls
          ? index.cls(cloned->cls->closureContextCls)
          : *cloned->cls,
        clonedClosures
      );
      if (!closure) return false;
      closureName = closure->name;
      always_assert(clonedClosures.emplace(&cls, std::move(closure)).second);
      return true;
    };

    auto mf = php::WideFunc::mut(cloned.get());
    assertx(!mf.blocks().empty());
    for (size_t bid = 0; bid < mf.blocks().size(); bid++) {
      auto const b = mf.blocks()[bid].mutate();
      for (size_t ix = 0; ix < b->hhbcs.size(); ix++) {
        auto& bc = b->hhbcs[ix];
        switch (bc.op) {
          case Op::CreateCl: {
            if (!onClosure(bc.CreateCl.str2)) return nullptr;
            break;
          }
          default:
            break;
        }
      }
    }

    return cloned;
  }

  static std::unique_ptr<php::Func> clone(const LocalIndex& index,
                                          const php::Func& orig,
                                          SString name,
                                          Attr attrs,
                                          const php::Class& dstCls,
                                          ClonedClosures& clonedClosures,
                                          bool internal = false) {
    auto cloned = std::make_unique<php::Func>(orig);
    cloned->name = name;
    cloned->attrs = attrs;
    if (!internal) cloned->attrs |= AttrTrait;
    cloned->idx = 0; // Set later
    cloned->cls = const_cast<php::Class*>(&dstCls);
    cloned->unit = dstCls.unit;

    if (!cloned->originalFilename) cloned->originalFilename = orig.unit;
    if (!cloned->originalUnit)     cloned->originalUnit = orig.unit;
    cloned->originalClass = orig.originalClass
      ? orig.originalClass
      : orig.cls->name;
    cloned->originalModuleName = orig.originalModuleName;

    // If the "module level traits" semantics is enabled, whenever HHBBC
    // inlines a method from a trait defined in module A into a trait/class
    // defined in module B, it sets the requiresFromOriginalModule flag of the
    // method to true. This flag causes the originalModuleName field to be
    // copied in the HHVM extendedSharedData section of the method, so that
    // HHVM is able to resolve correctly the original module of the method.
    if (RO::EvalModuleLevelTraits && orig.fromModuleLevelTrait &&
        !orig.requiresFromOriginalModule &&
        orig.originalModuleName != dstCls.moduleName) {
      cloned->requiresFromOriginalModule = true;
    } else {
      cloned->requiresFromOriginalModule = orig.requiresFromOriginalModule;
    }

    // cloned method isn't in any method table yet, so trash its
    // index.
    cloned->clsIdx = std::numeric_limits<uint32_t>::max();
    return clone_closures(index, std::move(cloned), clonedClosures);
  }

  static bool merge_inits(const LocalIndex& index,
                          const php::Class& cls,
                          const ClassInfo2& cinfo,
                          SString name,
                          std::vector<std::unique_ptr<php::Func>>& clones) {
    auto const existing = [&] () -> const php::Func* {
      for (auto const& m : cls.methods) {
        if (m->name == name) return m.get();
      }
      return nullptr;
    }();

    std::unique_ptr<php::Func> cloned;

    auto const merge = [&] (const php::Func& f) {
      if (!cloned) {
        ClonedClosures clonedClosures;
        if (existing) {
          cloned = clone(
            index,
            *existing,
            existing->name,
            existing->attrs,
            cls,
            clonedClosures,
            true
          );
          assertx(clonedClosures.empty());
          if (!cloned) return false;
        } else {
          ITRACE(4, "- cloning {}::{} as {}::{}\n",
                 f.cls->name, f.name, cls.name, name);
          cloned = clone(index, f, f.name, f.attrs, cls, clonedClosures, true);
          assertx(clonedClosures.empty());
          return (bool)cloned;
        }
      }

      ITRACE(4, "- appending {}::{} into {}::{}\n",
             f.cls->name, f.name, cls.name, name);
      if (name == s_86cinit.get()) return append_86cinit(cloned.get(), f);
      return append_func(cloned.get(), f);
    };

    for (auto const tname : cinfo.usedTraits) {
      auto const& trait = index.classInfo(tname);
      auto const it = trait.methods.find(name);
      if (it == trait.methods.end()) continue;
      auto const& meth = index.meth(it->second);
      if (!merge(meth)) {
        ITRACE(4, "merge_inits: failed to merge {}::{}\n",
               meth.cls->name, name);
        return false;
      }
    }

    if (cloned) {
      ITRACE(4, "merge_inits: adding {}::{} to method table\n",
             cloned->cls->name, cloned->name);
      clones.emplace_back(std::move(cloned));
    }

    return true;
  }

  static bool merge_xinits(const LocalIndex& index,
                           const php::Class& cls,
                           const ClassInfo2& cinfo,
                           const State& state,
                           std::vector<std::unique_ptr<php::Func>>& clones) {
    auto const merge_one = [&] (SString name, Attr attr) {
      auto const unnecessary = std::all_of(
        cinfo.traitProps.begin(),
        cinfo.traitProps.end(),
        [&] (const php::Prop& p) {
         if ((p.attrs & (AttrStatic | AttrLSB)) != attr) return true;
         if (p.val.m_type != KindOfUninit) return true;
         if (p.attrs & AttrLateInit) return true;
         return false;
       }
      );
      if (unnecessary) return true;
      return merge_inits(index, cls, cinfo, name, clones);
    };

    if (!merge_one(s_86pinit.get(), AttrNone))             return false;
    if (!merge_one(s_86sinit.get(), AttrStatic))           return false;
    if (!merge_one(s_86linit.get(), AttrStatic | AttrLSB)) return false;

    auto const unnecessary = std::all_of(
      state.m_traitCns.begin(),
      state.m_traitCns.end(),
      [&] (const php::Const& c) {
        return !c.val || c.val->m_type != KindOfUninit;
      }
    );
    if (unnecessary) return true;
    return merge_inits(index, cls, cinfo, s_86cinit.get(), clones);
  }

  struct NewClosure {
    std::unique_ptr<php::Class> cls;
    std::unique_ptr<ClassInfo2> cinfo;
  };

  static std::vector<NewClosure> flatten_traits(const LocalIndex& index,
                                                php::Class& cls,
                                                ClassInfo2& cinfo,
                                                State& state) {
    if (cls.attrs & AttrNoExpandTrait) return {};
    if (cls.usedTraitNames.empty()) {
      assertx(cinfo.usedTraits.empty());
      return {};
    }

    ITRACE(4, "flatten traits: {}\n", cls.name);
    Trace::Indent indent;

    assertx(!is_closure(cls));

    auto traitHasConstProp = cls.hasConstProp;
    for (auto const tname : cinfo.usedTraits) {
      auto const& trait = index.cls(tname);
      auto const& tinfo = index.classInfo(tname);
      if (tinfo.usedTraits.size() && !(trait.attrs & AttrNoExpandTrait)) {
        ITRACE(4, "Not flattening {} because of {}\n", cls.name, trait.name);
        return {};
      }
      if (is_noflatten_trait(&trait)) {
        ITRACE(
          4, "Not flattening {} because {} is annotated with __NoFlatten\n",
          cls.name, trait.name
        );
        return {};
      }
      if (tinfo.hasConstProp) traitHasConstProp = true;
    }

    std::vector<std::pair<SString, MethTabEntry*>> toAdd;
    for (auto& [name, mte] : cinfo.methods) {
      if (!mte.topLevel()) continue;
      if (mte.meth().cls->isame(cls.name)) continue;
      assertx(index.cls(mte.meth().cls).attrs & AttrTrait);
      toAdd.emplace_back(name, &mte);
    }

    if (!toAdd.empty()) {
      assertx(!cinfo.extraMethods.empty());
      std::sort(
        toAdd.begin(), toAdd.end(),
        [&] (auto const& a, auto const& b) {
          return
            state.methodIdx(index.m_ctx->name, cinfo.name, a.first) <
            state.methodIdx(index.m_ctx->name, cinfo.name, b.first);
        }
      );
    } else if (debug) {
      // When building the ClassInfos, we proactively added all
      // closures from usedTraits to the extraMethods map; but now
      // we're going to start from the used methods, and deduce which
      // closures actually get pulled in. Its possible *none* of the
      // methods got used, in which case, we won't need their closures
      // either. To be safe, verify that the only things in the map
      // are closures.
      for (auto const& mte : cinfo.extraMethods) {
        auto const& meth = index.meth(mte);
        always_assert(meth.isClosureBody);
      }
    }

    std::vector<std::unique_ptr<php::Func>> clones;
    ClonedClosures clonedClosures;

    for (auto const& [name, mte] : toAdd) {
      auto const& meth = index.meth(*mte);
      auto cloned = clone(
        index,
        meth,
        name,
        mte->attrs,
        cls,
        clonedClosures
      );
      if (!cloned) {
        ITRACE(4, "Not flattening {} because {}::{} could not be cloned\n",
               cls.name, mte->meth().cls, name);
        return {};
      }
      assertx(cloned->attrs & AttrTrait);
      clones.emplace_back(std::move(cloned));
    }

    if (!merge_xinits(index, cls, cinfo, state, clones)) {
      ITRACE(4, "Not flattening {} because we couldn't merge the 86xinits\n",
             cls.name);
      return {};
    }

    // We're now committed to flattening.
    ITRACE(3, "Flattening {}\n", cls.name);

    if (traitHasConstProp) {
      assertx(cinfo.hasConstProp);
      cls.hasConstProp = true;
    }
    cinfo.extraMethods.clear();

    for (auto [_, mte] : toAdd) mte->attrs |= AttrTrait;

    for (auto& p : cinfo.traitProps) {
      ITRACE(4, "- prop {}\n", p.name);
      cls.properties.emplace_back(std::move(p));
      cls.properties.back().attrs |= AttrTrait;
    }
    cinfo.traitProps.clear();

    for (auto& c : state.m_traitCns) {
      ITRACE(4, "- const {}\n", c.name);

      auto it = cinfo.clsConstants.find(c.name);
      assertx(it != cinfo.clsConstants.end());
      auto& cnsIdx = it->second;

      c.cls = cls.name;
      state.m_cnsFromTrait.erase(c.name);
      cnsIdx.cls = cls.name;
      cnsIdx.idx = cls.constants.size();
      cls.constants.emplace_back(std::move(c));
    }
    state.m_traitCns.clear();

    // A class should inherit any declared interfaces of any traits
    // that are flattened into it.
    for (auto const tname : cinfo.usedTraits) {
      auto const& tinfo = index.classInfo(tname);
      cinfo.declInterfaces.insert(
        end(cinfo.declInterfaces),
        begin(tinfo.declInterfaces),
        end(tinfo.declInterfaces)
      );
    }
    std::sort(
      begin(cinfo.declInterfaces),
      end(cinfo.declInterfaces),
      string_data_lti{}
    );
    cinfo.declInterfaces.erase(
      std::unique(
        begin(cinfo.declInterfaces), end(cinfo.declInterfaces),
        [&] (SString a, SString b) { return a->isame(b); }
      ),
      end(cinfo.declInterfaces)
    );

    // If we flatten the traits into us, they're no longer actual
    // parents.
    state.m_parents.erase(
      std::remove_if(
        begin(state.m_parents),
        end(state.m_parents),
        [] (const php::Class* c) { return bool(c->attrs & AttrTrait); }
      ),
      end(state.m_parents)
    );

    for (auto const tname : cinfo.usedTraits) {
      auto const& traitState = index.state(tname);
      state.m_parents.insert(
        end(state.m_parents),
        begin(traitState.m_parents),
        end(traitState.m_parents)
      );
    }
    std::sort(
      begin(state.m_parents),
      end(state.m_parents),
      [] (const php::Class* a, const php::Class* b) {
        return string_data_lti{}(a->name, b->name);
      }
    );
    state.m_parents.erase(
      std::unique(begin(state.m_parents), end(state.m_parents)),
      end(state.m_parents)
    );

    std::vector<NewClosure> newClosures;
    if (!clones.empty()) {
      auto const add = [&] (std::unique_ptr<php::Func> clone) {
        assertx(clone->cls == &cls);
        clone->clsIdx = cls.methods.size();

        if (!is_special_method_name(clone->name)) {
          auto it = cinfo.methods.find(clone->name);
          assertx(it != cinfo.methods.end());
          assertx(!it->second.meth().cls->isame(cls.name));
          it->second.setMeth(MethRef { cls.name, clone->clsIdx });
        } else {
          auto const [existing, emplaced] =
            cinfo.methods.emplace(clone->name, MethTabEntry { *clone });
          if (!emplaced) {
            assertx(existing->second.meth().cls->isame(cls.name));
            if (clone->name != s_86cinit.get()) {
              auto const idx = existing->second.meth().idx;
              clone->clsIdx = idx;
              cls.methods[idx] = std::move(clone);
              return;
            } else {
              existing->second.setMeth(MethRef { cls.name, clone->clsIdx });
            }
          }
        }

        cls.methods.emplace_back(std::move(clone));
      };

      auto cinit = [&] () -> std::unique_ptr<php::Func> {
        if (cls.methods.empty()) return nullptr;
        if (cls.methods.back()->name != s_86cinit.get()) return nullptr;
        auto init = std::move(cls.methods.back());
        cls.methods.pop_back();
        return init;
      }();

      for (auto& clone : clones) {
        ITRACE(4, "- meth {}\n", clone->name);
        if (clone->name == s_86cinit.get()) {
          cinit = std::move(clone);
          continue;
        }
        add(std::move(clone));
      }
      if (cinit) add(std::move(cinit));

      for (auto& [orig, clo] : clonedClosures) {
        ITRACE(4, "- closure {} as {}\n", orig->name, clo->name);
        assertx(is_closure(*orig));
        assertx(is_closure(*clo));
        assertx(clo->closureContextCls->isame(cls.name));
        assertx(clo->unit == cls.unit);

        assertx(clo->usedTraitNames.empty());
        State cloState;
        auto cloinfo = make_info(index, *clo, cloState);
        assertx(cloinfo);
        assertx(cloState.m_traitCns.empty());
        assertx(cloState.m_cnsFromTrait.empty());
        assertx(cloState.m_parents.size() == 1);
        assertx(cloState.m_parents[0]->name->isame(s_Closure.get()));

        cinfo.closures.emplace_back(clo->name);
        newClosures.emplace_back(
          NewClosure { std::move(clo), std::move(cloinfo) }
        );
      }

      std::sort(
        newClosures.begin(),
        newClosures.end(),
        [] (auto const& p1, auto const& p2) {
          return string_data_lti{}(p1.cls->name, p2.cls->name);
        }
      );
    }

    // Flattening methods into traits can turn methods from not "first
    // name" to "first name", so recalculate that here.
    for (auto& [name, mte] : cinfo.methods) {
      if (mte.firstName()) continue;
      auto const firstName = [&, name=name] {
        if (cls.parentName) {
          auto const& parentInfo = index.classInfo(cls.parentName);
          if (parentInfo.methods.count(name)) return false;
          if (parentInfo.missingMethods.count(name)) return false;
        }
        for (auto const iname : cinfo.declInterfaces) {
          auto const& iface = index.classInfo(iname);
          if (iface.methods.count(name)) return false;
          if (iface.missingMethods.count(name)) return false;
        }
        return true;
      }();
      if (firstName) mte.setFirstName();
    }

    struct EqHash {
      bool operator()(const PreClass::ClassRequirement& a,
                      const PreClass::ClassRequirement& b) const {
        return a.is_same(&b);
      }
      size_t operator()(const PreClass::ClassRequirement& a) const {
        return a.hash();
      }
    };
    hphp_fast_set<PreClass::ClassRequirement, EqHash, EqHash> reqs{
      cls.requirements.begin(),
      cls.requirements.end()
    };

    for (auto const tname : cinfo.usedTraits) {
      auto const& trait = index.cls(tname);
      for (auto const& req : trait.requirements) {
        if (reqs.emplace(req).second) cls.requirements.emplace_back(req);
      }
    }

    cls.attrs |= AttrNoExpandTrait;
    return newClosures;
  }

  // Update a type constraint to it's ultimate type, or leave it as
  // unresolved if it resolves to nothing valid. Record the new type
  // in case it needs to be fixed up later.
  static void update_type_constraint(const LocalIndex& index,
                                     TypeConstraint& tc,
                                     bool isProp,
                                     ISStringSet* uses) {
    always_assert(IMPLIES(isProp, tc.validForProp()));

    if (!tc.isUnresolved()) {
      // Any TC already resolved is assumed to be correct.
      if (uses && tc.clsName()) uses->emplace(tc.clsName());
      return;
    }
    auto const name = tc.typeName();

    // Is this name a type-alias or enum?
    if (auto const tm = index.typeMapping(name)) {
      if (tm->typeAndValueUnion.size() > 1) {
        // TODO(T151885113): Support multiple constraints created by case types
        // for param, return, upper bound and property type hints
        always_assert(RO::EvalTreatCaseTypesAsMixed);
      }
      assertx(tm->typeAndValueUnion.size() == 1);
      auto const tm_type = tm->typeAndValueUnion[0].type;
      // Whatever it's an alias of isn't valid, so leave unresolved.
      if (tm_type == AnnotType::Unresolved) return;
      if (isProp && !propSupportsAnnot(tm_type)) return;
      auto const value = [&] () -> SString {
        // Store the first enum encountered during resolution. This
        // lets us fixup the type later if needed.
        if (tm->firstEnum) return tm->firstEnum;
        if (tm_type == AnnotType::Object) {
          auto const tm_value = tm->typeAndValueUnion[0].value;
          assertx(tm_value);
          return tm_value;
        }
        return nullptr;
      }();
      tc.resolveType(
        tm_type,
        tm->nullable,
        value
      );
      assertx(IMPLIES(isProp, tc.validForProp()));
      if (uses && value) uses->emplace(value);
      return;
    }

    // Not a type-alias or enum. If it's explicitly marked as missing,
    // leave it unresolved. Otherwise assume it's an object with that
    // name.
    if (index.missingType(name)) return;
    tc.resolveType(AnnotType::Object, tc.isNullable(), name);
    if (uses) uses->emplace(name);
  }

  static void update_type_constraints(const LocalIndex& index,
                                      php::Func& func,
                                      ISStringSet* uses) {
    for (auto& p : func.params) {
      update_type_constraint(index, p.typeConstraint, false, uses);
      for (auto& ub : p.upperBounds.m_constraints) update_type_constraint(index, ub, false, uses);
    }
    update_type_constraint(index, func.retTypeConstraint, false, uses);
    for (auto& ub : func.returnUBs.m_constraints) update_type_constraint(index, ub, false, uses);
  }

  static void update_type_constraints(const LocalIndex& index,
                                      php::Class& cls,
                                      ISStringSet* uses) {
    if (cls.attrs & AttrEnum) {
      update_type_constraint(index, cls.enumBaseTy, false, uses);
    }
    for (auto& meth : cls.methods) update_type_constraints(index, *meth, uses);
    for (auto& prop : cls.properties) {
      update_type_constraint(index, prop.typeConstraint, true, uses);
      for (auto& ub : prop.ubs.m_constraints) update_type_constraint(index, ub, true, uses);
    }
  }

  static std::unique_ptr<FuncInfo2> make_func_info(const LocalIndex& index,
                                                   const php::Func& f) {
    auto finfo = std::make_unique<FuncInfo2>();
    finfo->name = f.name;
    finfo->returnTy = initial_return_type(index, f);
    return finfo;
  }

  static Type initial_return_type(const LocalIndex& index,
                                  const php::Func& f) {
    auto const ty = [&] {
      // Return type of native functions is calculated differently.
      if (f.isNative) return native_function_return_type(&f);

      if ((f.attrs & AttrBuiltin) || f.isMemoizeWrapper) return TInitCell;

      if (f.isGenerator) {
        if (f.isAsync) {
          // Async generators always return AsyncGenerator object.
          return objExact(res::Class::makeUnresolved(s_AsyncGenerator.get()));
        }
        // Non-async generators always return Generator object.
        return objExact(res::Class::makeUnresolved(s_Generator.get()));
      }

      auto const make_type = [&] (const TypeConstraint& tc) {
        auto lookup = type_from_constraint(
          tc,
          TInitCell,
          [] (SString name) { return res::Class::makeUnresolved(name); },
          [&] () -> Optional<Type> {
            if (!f.cls) return std::nullopt;
            auto const& cls = f.cls->closureContextCls
              ? index.cls(f.cls->closureContextCls)
              : *f.cls;
            if (cls.attrs & AttrTrait) return std::nullopt;
            return subCls(res::Class::makeUnresolved(cls.name));
          }
        );
        if (lookup.coerceClassToString == TriBool::Yes) {
          lookup.upper = promote_classish(std::move(lookup.upper));
        } else if (lookup.coerceClassToString == TriBool::Maybe) {
          lookup.upper |= TSStr;
        }
        return unctx(std::move(lookup.upper));
      };

      auto const process = [&] (const TypeConstraint& tc,
                                const TypeIntersectionConstraint& ubs) {
        auto ret = TInitCell;
        ret = intersection_of(std::move(ret), make_type(tc));
        for (auto const& ub : ubs.m_constraints) {
          ret = intersection_of(std::move(ret), make_type(ub));
        }
        return ret;
      };

      auto ret = process(f.retTypeConstraint, f.returnUBs);
      if (f.hasInOutArgs && !ret.is(BBottom)) {
        std::vector<Type> types;
        types.reserve(f.params.size() + 1);
        types.emplace_back(std::move(ret));
        for (auto const& p : f.params) {
          if (!p.inout) continue;
          auto t = process(p.typeConstraint, p.upperBounds);
          if (t.is(BBottom)) return TBottom;
          types.emplace_back(std::move(t));
        }
        std::reverse(begin(types)+1, end(types));
        ret = vec(std::move(types));
      }

      if (f.isAsync) {
        // Async functions always return WaitH<T>, where T is the type
        // returned internally.
        return wait_handle_unresolved(std::move(ret));
      }
      return ret;
    }();

    FTRACE(3, "Initial return type for {}: {}\n",
           func_fullname(f), show(ty));
    return ty;
  }

  /*
   * Mark any properties in cls that definitely do not redeclare a
   * property in the parent with an inequivalent type-hint.
   *
   * Rewrite the initial values for any AttrSystemInitialValue
   * properties. If the properties' type-hint does not admit null
   * values, change the initial value to one that is not null
   * (if possible). This is only safe to do so if the property is not
   * redeclared in a derived class or if the redeclaration does not
   * have a null system provided default value. Otherwise, a property
   * can have a null value (even if its type-hint doesn't allow it)
   * without the JIT realizing that its possible.
   *
   * Note that this ignores any unflattened traits. This is okay
   * because properties pulled in from traits which match an already
   * existing property can't change the initial value. The runtime
   * will clear AttrNoImplicitNullable on any property pulled from the
   * trait if it doesn't match an existing property.
   */
  static void optimize_properties(const LocalIndex& index,
                                  php::Class& cls,
                                  ClassInfo2& cinfo) {
    assertx(cinfo.hasBadRedeclareProp);
    assertx(cinfo.hasBadInitialPropValues);

    auto const isClosure = is_closure(cls);

    cinfo.hasBadRedeclareProp = false;
    cinfo.hasBadInitialPropValues = false;
    for (auto& prop : cls.properties) {
      assertx(!(prop.attrs & AttrNoBadRedeclare));
      assertx(!(prop.attrs & AttrNoImplicitNullable));
      assertx(!(prop.attrs & AttrInitialSatisfiesTC));

      // Check whether the property's initial value satisfies it's
      // type-hint.
      auto const initialSatisfies = [&] {
        if (isClosure) return true;
        if (is_used_trait(cls)) return false;

        // Any property with an unresolved type-constraint here might
        // fatal when we initialize the class.
        if (prop.typeConstraint.isUnresolved()) return false;
        for (auto const& ub : prop.ubs.m_constraints) {
          if (ub.isUnresolved()) return false;
        }

        if (prop.attrs & (AttrSystemInitialValue | AttrLateInit)) return true;

        auto const initial = from_cell(prop.val);
        if (initial.subtypeOf(BUninit)) return false;

        auto const make_type = [&] (const TypeConstraint& tc) {
          auto lookup = type_from_constraint(
            tc,
            TInitCell,
            [] (SString name) { return res::Class::makeUnresolved(name); },
            [&] () -> Optional<Type> {
              auto const& ctx = cls.closureContextCls
                ? index.cls(cls.closureContextCls)
                : cls;
              if (ctx.attrs & AttrTrait) return std::nullopt;
              return subCls(res::Class::makeUnresolved(ctx.name));
            }
          );
          return unctx(std::move(lookup.lower));
        };

        if (!initial.subtypeOf(make_type(prop.typeConstraint))) return false;
        for (auto const& ub : prop.ubs.m_constraints) {
          if (!initial.subtypeOf(make_type(ub))) return false;
        }
        return true;
      }();

      if (initialSatisfies) {
        attribute_setter(prop.attrs, true, AttrInitialSatisfiesTC);
      } else {
        cinfo.hasBadInitialPropValues = true;
      }

      auto const noBadRedeclare = [&] {
        // Closures should never have redeclared properties.
        if (isClosure) return true;
        // Static and private properties never redeclare anything so
        // need not be considered.
        if (prop.attrs & (AttrStatic | AttrPrivate)) return true;

        for (auto const base : cinfo.baseList) {
          if (base->isame(cls.name)) continue;

          auto& baseCInfo = index.classInfo(base);
          auto& baseCls = index.cls(base);

          auto const parentProp = [&] () -> php::Prop* {
            for (auto& p : baseCls.properties) {
              if (p.name == prop.name) return const_cast<php::Prop*>(&p);
            }
            for (auto& p : baseCInfo.traitProps) {
              if (p.name == prop.name) return const_cast<php::Prop*>(&p);
            }
            return nullptr;
          }();
          if (!parentProp) continue;
          if (parentProp->attrs & (AttrStatic | AttrPrivate)) continue;

          // This property's type-constraint might not have been
          // resolved (if the parent is not on the output list for
          // this job), so do so here.
          update_type_constraint(index, parentProp->typeConstraint, true, nullptr);

          // This check is safe, but conservative. It might miss a few
          // rare cases, but it's sufficient and doesn't require class
          // hierarchies.
          if (prop.typeConstraint.maybeInequivalentForProp(
                parentProp->typeConstraint
              )) {
            return false;
          }

          for (auto const& ub : prop.ubs.m_constraints) {
            for (auto& pub : parentProp->ubs.m_constraints) {
              update_type_constraint(index, pub, true, nullptr);
              if (ub.maybeInequivalentForProp(pub)) return false;
            }
          }
        }

        return true;
      }();

      if (noBadRedeclare) {
        attribute_setter(prop.attrs, true, AttrNoBadRedeclare);
      } else {
        cinfo.hasBadRedeclareProp = true;
      }

      auto const nullable = [&] {
        if (isClosure) return true;
        if (!(prop.attrs & AttrSystemInitialValue)) return false;
        return prop.typeConstraint.defaultValue().m_type == KindOfNull;
      }();

      attribute_setter(prop.attrs, !nullable, AttrNoImplicitNullable);
      if (!(prop.attrs & AttrSystemInitialValue)) continue;
      if (prop.val.m_type == KindOfUninit) {
        assertx(isClosure || bool(prop.attrs & AttrLateInit));
        continue;
      }

      prop.val = [&] {
        if (nullable) return make_tv<KindOfNull>();
        // Give the 86reified_prop a special default value to avoid
        // pessimizing the inferred type (we want it to always be a
        // vec of a specific size).
        if (prop.name == s_86reified_prop.get()) {
          return get_default_value_of_reified_list(cls.userAttributes);
        }
        return prop.typeConstraint.defaultValue();
      }();
    }
  }
};

/*
 * "Fixups" a php::Unit by removing specified funcs from it, and
 * adding specified classes. This is needed to add closures created
 * from trait flattening into their associated units. While we're
 * doing this, we also remove redundant meth caller funcs here
 * (because it's convenient).
 */
struct UnitFixupJob {
  static std::string name() { return "hhbbc-flatten-fixup"; }
  static void init(const Config& config) {
    process_init(config.o, config.gd, false);
  }
  static void fini() {}

  // For a given unit, the classes to add and the funcs to remove.
  struct Fixup {
    std::vector<SString> addClass;
    std::vector<SString> removeFunc;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(addClass)(removeFunc);
    }
  };

  static std::unique_ptr<php::Unit> run(std::unique_ptr<php::Unit> unit,
                                        const Fixup& fixup) {
    if (!fixup.removeFunc.empty()) {
      // If we want to remove a func, it should be in this unit.
      auto DEBUG_ONLY erased = false;
      unit->funcs.erase(
        std::remove_if(
          begin(unit->funcs),
          end(unit->funcs),
          [&] (SString func) {
            // This is a kinda dumb O(N^2) algorithm, but these lists
            // are typicaly size 1.
            auto const erase = std::any_of(
              begin(fixup.removeFunc),
              end(fixup.removeFunc),
              [&] (SString remove) { return remove == func; }
            );
            if (erase) erased = true;
            return erase;
          }
        ),
        end(unit->funcs)
      );
      assertx(erased);
    }

    unit->classes.insert(
      end(unit->classes),
      begin(fixup.addClass),
      end(fixup.addClass)
    );
    return unit;
  }
};

/*
 * We might have resolved a type-constraint during FlattenJob and then
 * later on learned that one of the types are invalid. For example, we
 * resolved a type-constraint to a particular object type, and then
 * realized the associated class isn't instantiable. We have the hard
 * invariant that a resolved type-constraint always points at a valid
 * type (which means for objects they have a ClassInfo). So, if we
 * determined if a type is invalid during FlattenJob, we then "fixup"
 * the type-constraint here by making it unresolved again. This is
 * rare, as such invalid types don't occur in well-typed code, so the
 * extra overhead of another job is fine.
 */
struct MissingClassFixupJob {
static std::string name() { return "hhbbc-flatten-missing"; }
  static void init(const Config& config) {
    process_init(config.o, config.gd, false);
  }
  static void fini() {}

  using Output = Multi<
    Variadic<std::unique_ptr<php::Class>>,
    Variadic<std::unique_ptr<ClassInfo2>>,
    Variadic<std::unique_ptr<php::Func>>,
    Variadic<std::unique_ptr<FuncInfo2>>
  >;
  static Output run(Variadic<std::unique_ptr<php::Class>> classes,
                    Variadic<std::unique_ptr<ClassInfo2>> cinfos,
                    Variadic<std::unique_ptr<php::Func>> funcs,
                    Variadic<std::unique_ptr<FuncInfo2>> finfos,
                    std::vector<SString> missingIn) {
    assertx(classes.vals.size() == cinfos.vals.size());
    assertx(funcs.vals.size() == finfos.vals.size());
    ISStringSet missing{ begin(missingIn), end(missingIn) };
    for (size_t i = 0, size = classes.vals.size(); i < size; ++i) {
      update(*classes.vals[i], *cinfos.vals[i], missing);
    }
    for (size_t i = 0, size = funcs.vals.size(); i < size; ++i) {
      update(*funcs.vals[i], *finfos.vals[i], missing);
    }
    return std::make_tuple(
      std::move(classes),
      std::move(cinfos),
      std::move(funcs),
      std::move(finfos)
    );
  }
private:
  static bool update(TypeConstraint& tc, const ISStringSet& missing) {
    auto const name = tc.clsName();
    if (!name || !missing.count(name)) return false;
    tc.unresolve();
    return true;
  }

  static void update(php::Func& func,
                     FuncInfo2& finfo,
                     const ISStringSet& missing) {
    for (auto& p : func.params) {
      update(p.typeConstraint, missing);
      for (auto& ub : p.upperBounds.m_constraints) update(ub, missing);
    }
    auto updated = update(func.retTypeConstraint, missing);
    for (auto& ub : func.returnUBs.m_constraints) updated |= update(ub, missing);
    if (updated) {
      // One of the return constraints corresponds to a missing
      // type. This means the function can never return and must have
      // a Bottom return type.
      FTRACE(2, "Fixing up return type of {} to Bottom\n", func_fullname(func));
      finfo.returnTy = TBottom;
    }
  }

  static void update(php::Class& cls,
                     ClassInfo2& cinfo,
                     const ISStringSet& missing) {
    assertx(cls.methods.size() == cinfo.funcInfos.size());
    if (cls.attrs & AttrEnum) update(cls.enumBaseTy, missing);
    for (size_t i = 0, size = cls.methods.size(); i < size; ++i) {
      update(*cls.methods[i], *cinfo.funcInfos[i], missing);
    }
    for (auto& prop : cls.properties) {
      update(prop.typeConstraint, missing);
      for (auto& ub : prop.ubs.m_constraints) update(ub, missing);
    }
  }
};

Job<FlattenJob> s_flattenJob;
Job<UnitFixupJob> s_unitFixupJob;
Job<MissingClassFixupJob> s_missingClassFixupJob;

/*
 * For efficiency reasons, we want to do class flattening all in one
 * pass. So, we use assign_hierarchical_work (described above) to
 * calculate work buckets to allow us to do this.
 *
 * - The "root" classes are the leaf classes in the hierarchy. These are
 * the buckets which are not dependencies of anything.
 *
 * - The dependencies of a class are all of the (transitive) parent
 *   classes of that class (up to the top classes with no parents).
 *
 * - Each job takes two kinds of input. The first is the set of
 *   classes which are actually to be flattened. These will have the
 *   flattening results returned as output from the job. The second is
 *   the set of dependencies that are required to perform flattening
 *   on the first set of inputs. These will have the same flattening
 *   algorithm applied to them, but only to obtain intermediate state
 *   to calculate the output for the first set of inputs. Their
 *   results will be thrown away.
 *
 * - When we run the jobs, we'll take the outputs and turn that into a set of
 *   updates, which we then apply to the Index data structures. Some
 *   of these updates require changes to the php::Unit, which we do a
 *   in separate set of "fixup" jobs at the end.
 */

// Input class metadata to be turned into work buckets.
struct IndexFlattenMetadata {
  struct ClassMeta {
    ISStringSet deps;
    std::vector<SString> closures;
    // All types mentioned in type-constraints in this class.
    std::vector<SString> unresolvedTypes;
    size_t idx; // Index into allCls vector
    bool isClosure{false};
    bool uninstantiable{false};
  };
  ISStringToOneT<ClassMeta> cls;
  // All classes to be flattened
  std::vector<SString> allCls;
  // Mapping of units to classes which should be deleted from that
  // unit. This is typically from duplicate meth callers. This is
  // performed as part of "fixing up" the unit after flattening
  // because it's convenient to do so there.
  SStringToOneT<std::vector<SString>> unitDeletions;
  struct FuncMeta {
    // All types mentioned in type-constraints in this func.
    std::vector<SString> unresolvedTypes;
  };
  ISStringToOneT<FuncMeta> func;
  std::vector<SString> allFuncs;
  ISStringToOneT<TypeMapping> typeMappings;
};

//////////////////////////////////////////////////////////////////////

constexpr size_t kNumTypeMappingRounds = 20;

/*
 * Update the type-mappings in the program so they all point to their
 * ultimate type. After this step, every type-mapping that still has
 * an unresolved type points to an invalid type.
 */
void flatten_type_mappings(IndexData& index,
                           IndexFlattenMetadata& meta) {
  trace_time tracer{"flatten type mappings"};
  tracer.ignore_client_stats();

  std::vector<const TypeMapping*> work;
  work.reserve(meta.typeMappings.size());
  for (auto const& [_, tm] : meta.typeMappings) work.emplace_back(&tm);

  auto resolved = parallel::map(
    work,
    [&] (const TypeMapping* typeMapping) {
      Optional<ISStringSet> seen;
      auto nullable = typeMapping->nullable;
      auto firstEnum = typeMapping->firstEnum;

      auto enumMeta = folly::get_ptr(meta.cls, typeMapping->name);

      php::TypeAlias::TypeAndValueUnion tvu;

      for (auto const& [type, value] : typeMapping->typeAndValueUnion) {
        auto name = value;

        if (type != AnnotType::Unresolved) {
          // If the type-mapping is already resolved, we mainly take it
          // as is. The exception is if it's an enum, in which case we
          // validate the underlying base type.
          assertx(type != AnnotType::Object);
          if (!enumMeta) {
            tvu.emplace_back(php::TypeAndValue{type, value});
            continue;
          }
          if (!enumSupportsAnnot(type)) {
            FTRACE(
              2, "Type-mapping '{}' is invalid because it resolves to "
              "invalid enum type {}\n",
              typeMapping->name,
              annotName(type)
            );
            tvu.emplace_back(php::TypeAndValue{AnnotType::Unresolved, value});
            continue;
          }
          tvu.emplace_back(php::TypeAndValue{type, value});
          continue;
        }

        std::queue<LSString> queue;
        queue.push(name);

        for (size_t rounds = 0;; ++rounds) {
          if (queue.empty()) break;
          name = normalizeNS(queue.back());
          queue.pop();

          if (auto const next = folly::get_ptr(meta.typeMappings, name)) {
            nullable |= next->nullable;
            if (!firstEnum) firstEnum = next->firstEnum;

            if (enumMeta && next->firstEnum) {
              enumMeta->deps.emplace(next->firstEnum);
            }

            for (auto const& [next_type, next_value] : next->typeAndValueUnion) {
              if (next_type == AnnotType::Unresolved) {
                queue.push(next_value);
                continue;
              }
              assertx(next_type != AnnotType::Object);
              if (firstEnum && !enumSupportsAnnot(next_type)) {
                FTRACE(
                  2, "Type-mapping '{}' is invalid because it resolves to "
                  "invalid enum type {}{}\n",
                  typeMapping->name,
                  annotName(next_type),
                  firstEnum->isame(typeMapping->name)
                    ? "" : folly::sformat(" (via {})", firstEnum)
                );
                tvu.emplace_back(php::TypeAndValue{AnnotType::Unresolved, name});
                continue;
              }
              tvu.emplace_back(php::TypeAndValue{next_type, next_value});
            }
          } else if (index.classRefs.count(name)) {
            if (firstEnum) {
              FTRACE(
                2, "Type-mapping '{}' is invalid because it resolves to "
                "invalid object '{}' for enum type (via {})\n",
                typeMapping->name,
                name,
                firstEnum
              );
            }

            tvu.emplace_back(php::TypeAndValue {
              firstEnum ? AnnotType::Unresolved : AnnotType::Object,
              name
            });
            break;
          } else {
            FTRACE(
              2, "Type-mapping '{}' is invalid because it involves "
              "non-existent type '{}'{}\n",
              typeMapping->name,
              name,
              (firstEnum && !firstEnum->isame(typeMapping->name))
                ? folly::sformat(" (via {})", firstEnum) : ""
            );
            tvu.emplace_back(php::TypeAndValue{AnnotType::Unresolved, name});
            break;
          }

          // Deal with cycles. Since we don't expect to encounter them, just
          // use a counter until we hit a chain length of kNumTypeMappingRounds,
          // then start tracking the names we resolve.
          if (rounds == kNumTypeMappingRounds) {
            seen.emplace();
            seen->insert(name);
          } else if (rounds > kNumTypeMappingRounds) {
            if (!seen->insert(name).second) {
              FTRACE(
                2, "Type-mapping '{}' is invalid because it's definition "
                "is circular with '{}'\n",
                typeMapping->name,
                name
              );
              return TypeMapping {
                typeMapping->name,
                firstEnum,
                {php::TypeAndValue{AnnotType::Unresolved, name}},
                nullable
              };
            }
          }
        }
      }
      assertx(!tvu.empty());
      return TypeMapping {
        typeMapping->name,
        firstEnum,
        std::move(tvu),
        nullable
      };
    }
  );

  for (auto& after : resolved) {
    assertx(!after.typeAndValueUnion.empty());
    auto const name = after.name;
    bool unresolved = std::any_of(after.typeAndValueUnion.begin(),
                                  after.typeAndValueUnion.end(),
                                  [](const php::TypeAndValue& tv) {
                                    return tv.type == AnnotType::Unresolved;
                                  });
    using namespace folly::gen;
    FTRACE(
      4, "Type-mapping '{}' flattened to {}({}){}\n",
      name,
      from(after.typeAndValueUnion)
        | map([&] (const php::TypeAndValue& tv) { return annotName(tv.type); })
        | unsplit<std::string>("|"),
      from(after.typeAndValueUnion)
        | map([&] (const php::TypeAndValue& tv) {
            return tv.value && !tv.value->empty() ? tv.value->toCppString() : "";
          })
        | unsplit<std::string>("|"),
      (after.firstEnum && !after.firstEnum->isame(name))
        ? folly::sformat(" (via {})", after.firstEnum) : ""
    );
    if (unresolved && meta.cls.count(name)) {
      FTRACE(4, "  Marking enum '{}' as uninstantiable\n", name);
      meta.cls.at(name).uninstantiable = true;
    }
    meta.typeMappings.at(name) = std::move(after);
  }
}

//////////////////////////////////////////////////////////////////////

struct FlattenClassesWork {
  std::vector<SString> classes;
  std::vector<SString> deps;
  std::vector<SString> funcs;
};

std::vector<FlattenClassesWork>
flatten_classes_assign(IndexFlattenMetadata& meta) {
  trace_time trace{"flatten classes assign"};
  trace.ignore_client_stats();

  // First calculate the classes which *aren't* leafs. A class is a
  // leaf if it is not depended on by another class. The sense is
  // inverted because we want to default construct the atomics.
  std::vector<std::atomic<bool>> isNotLeaf(meta.allCls.size());
  parallel::for_each(
    meta.allCls,
    [&] (SString cls) {
      auto const onDep = [&] (SString d) {
        auto const it = meta.cls.find(d);
        if (it == meta.cls.end()) return;
        assertx(it->second.idx < isNotLeaf.size());
        isNotLeaf[it->second.idx] = true;
      };
      auto const& clsMeta = meta.cls.at(cls);
      for (auto const d : clsMeta.deps)       onDep(d);
      for (auto const clo : clsMeta.closures) onDep(clo);
    }
  );

  // Store all of the (transitive) dependencies for every class,
  // calculated lazily. LockFreeLazy ensures that multiple classes can
  // access this concurrently and safely calculate it on demand.
  struct DepLookup {
    ISStringSet deps;
    // Whether this class is instantiable
    bool instantiable{false};
  };
  std::vector<LockFreeLazy<DepLookup>> allDeps{meta.allCls.size()};

  // Look up all of the transitive dependencies for the given class.
  auto const findAllDeps = [&] (SString cls,
                                ISStringSet& visited,
                                auto const& self) -> const DepLookup& {
    static const DepLookup empty;

    auto const it = meta.cls.find(cls);
    if (it == meta.cls.end()) {
      FTRACE(
        4, "{} is not instantiable because it is missing\n",
        cls
      );
      return empty;
    }

    // The class exists, so look up it's dependency information.
    auto const idx = it->second.idx;
    auto const& deps = it->second.deps;
    auto const& closures = it->second.closures;

    // Check for cycles. A class involved in cyclic inheritance is not
    // instantiable (and has no dependencies). This needs to be done
    // before accessing the LockFreeLazy below, because if we are in a
    // cycle, we'll deadlock when we do so.
    auto const emplaced = visited.emplace(cls).second;
    if (!emplaced) {
      FTRACE(
        4, "{} is not instantiable because it forms a dependency "
        "cycle with itself\n", cls
      );
      it->second.uninstantiable = true;
      return empty;
    }
    SCOPE_EXIT { visited.erase(cls); };

    assertx(idx < allDeps.size());
    return allDeps[idx].get(
      [&] {
        // Otherwise get all of the transitive dependencies of it's
        // dependencies and combine them.
        DepLookup out;
        out.instantiable = !it->second.uninstantiable;
        auto const onDep = [&] (SString d) {
          auto const& lookup = self(d, visited, self);
          out.deps.insert(begin(lookup.deps), end(lookup.deps));
          if (!lookup.instantiable) {
            // If the dependency is not instantiable, this isn't
            // either. Note, however, we still need to preserve the
            // already gathered dependencies, since they'll have to be
            // placed in some bucket.
            if (out.instantiable) {
              FTRACE(
                4, "{} is not instantiable because it depends on {}, "
                "which is not instantiable\n",
                cls, d
              );
              it->second.uninstantiable = true;
            }
            out.instantiable = false;
          } else {
            // Only add this if instantiable.
            out.deps.emplace(d);
          }
        };

        for (auto const d : deps)       onDep(d);
        for (auto const clo : closures) onDep(clo);
        return out;
      }
    );
  };

  constexpr size_t kBucketSize = 2000;
  constexpr size_t kMaxBucketSize = 30000;

  auto assignments = assign_hierarchical_work(
    [&] {
      std::vector<SString> l;
      auto const size = meta.allCls.size();
      assertx(size == isNotLeaf.size());
      l.reserve(size);
      for (size_t i = 0; i < size; ++i) {
        if (!isNotLeaf[i]) l.emplace_back(meta.allCls[i]);
      }
      return l;
    }(),
    meta.allCls.size(),
    kBucketSize,
    kMaxBucketSize,
    [&] (SString c) {
      ISStringSet visited;
      auto const& lookup = findAllDeps(c, visited, findAllDeps);
      return std::make_pair(&lookup.deps, lookup.instantiable);
    },
    [&] (SString c) -> Optional<size_t> { return meta.cls.at(c).idx; }
  );

  // Bucketize functions separately

  constexpr size_t kFuncBucketSize = 5000;

  auto funcBuckets = consistently_bucketize(meta.allFuncs, kFuncBucketSize);

  std::vector<FlattenClassesWork> work;
  // If both the class and func assignments map to a single bucket,
  // combine them both together. This is an optimization for things
  // like unit tests, where the total amount of work is low and we
  // want to run it all in a single job if possible.
  if (assignments.size() == 1 && funcBuckets.size() == 1) {
    work.emplace_back(
      FlattenClassesWork{
        std::move(assignments[0].classes),
        std::move(assignments[0].deps),
        std::move(funcBuckets[0])
      }
    );
  } else {
    // Otherwise split the classes and func work.
    work.reserve(assignments.size() + funcBuckets.size());
    for (auto& assignment : assignments) {
      work.emplace_back(
        FlattenClassesWork{
          std::move(assignment.classes),
          std::move(assignment.deps),
          {}
        }
      );
    }
    for (auto& bucket : funcBuckets) {
      work.emplace_back(
        FlattenClassesWork{ {}, {}, std::move(bucket) }
      );
    }
  }

  if (Trace::moduleEnabled(Trace::hhbbc_index, 5)) {
    for (size_t i = 0; i < work.size(); ++i) {
      auto const& [classes, deps, funcs] = work[i];
      FTRACE(5, "flatten work item #{}:\n", i);
      FTRACE(5, "  classes ({}):\n", classes.size());
      for (auto const DEBUG_ONLY c : classes) FTRACE(5, "    {}\n", c);
      FTRACE(5, "  deps ({}):\n", deps.size());
      for (auto const DEBUG_ONLY d : deps) FTRACE(5, "    {}\n", d);
      FTRACE(5, "  funcs ({}):\n", funcs.size());
      for (auto const DEBUG_ONLY f : funcs) FTRACE(5, "    {}\n", f);
    }
  }

  return work;
}

// Run MissingClassFixupJob to fixup any type-constraints in classes
// and funcs which were resolved to types which are now invalid.
void flatten_classes_fixup_missing(IndexData& index,
                                   ISStringToOneT<ISStringSet> typeUsesByClass,
                                   ISStringToOneT<ISStringSet> typeUsesByFunc,
                                   ISStringSet uninstantiable) {
  // If FlattenJob didn't find anything uninstantiable, we don't need
  // to do this (this is the common case).
  if (uninstantiable.empty()) return;
  trace_time timer{"flatten classes fixup missing", index.sample};

  // Build a map of classes and funcs that reference the
  // uninstantiable types.
  ISStringToOneT<ISStringSet> classes;
  ISStringToOneT<ISStringSet> funcs;
  for (auto const u : uninstantiable) {
    if (auto const uses = folly::get_ptr(typeUsesByClass, u)) {
      for (auto const use : *uses) classes[use].emplace(u);
    }
    if (auto const uses = folly::get_ptr(typeUsesByFunc, u)) {
      for (auto const use : *uses) funcs[use].emplace(u);
    }
  }
  // We found uninstantiable types, but they're not actually used, so
  // we don't have to fix anything up.
  if (classes.empty() && funcs.empty()) return;

  constexpr size_t kBucketSize = 3000;

  auto buckets = consistently_bucketize(
    [&] {
      // NB: classes and funcs are in different namespaces and can
      // have the same name. If that happens, they'll be de-duped
      // here. This is harmless, as it just means the class and func
      // will be assigned to the same bucket.
      std::vector<SString> sorted;
      sorted.reserve(classes.size() + funcs.size());
      for (auto const& [c, _] : classes) sorted.emplace_back(c);
      for (auto const& [f, _] : funcs) sorted.emplace_back(f);
      std::sort(begin(sorted), end(sorted), string_data_lti{});
      sorted.erase(
        std::unique(
          begin(sorted), end(sorted),
          [&] (SString a, SString b) { return a->isame(b); }
        ),
        end(sorted)
      );
      return sorted;
    }(),
    kBucketSize
  );

  using namespace folly::gen;

  auto const run = [&] (std::vector<SString> work) -> coro::Task<void> {
    HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;

    if (work.empty()) HPHP_CORO_RETURN_VOID;

    Client::ExecMetadata metadata{
      .job_key = folly::sformat("fixup missing classes {}", work[0])
    };

    std::vector<UniquePtrRef<php::Class>> clsRefs;
    std::vector<UniquePtrRef<php::Func>> funcRefs;
    std::vector<UniquePtrRef<ClassInfo2>> cinfoRefs;
    std::vector<UniquePtrRef<FuncInfo2>> finfoRefs;
    std::vector<SString> clsNames;
    std::vector<SString> funcNames;
    ISStringSet missingSet;

    for (auto const w : work) {
      if (auto const used = folly::get_ptr(classes, w)) {
        missingSet.insert(begin(*used), end(*used));
        clsRefs.emplace_back(index.classRefs.at(w));
        cinfoRefs.emplace_back(index.classInfoRefs.at(w));
        clsNames.emplace_back(w);
      }
      // Not else if because of potentially collision between class
      // and func names.
      if (auto const used = folly::get_ptr(funcs, w)) {
        missingSet.insert(begin(*used), end(*used));
        funcRefs.emplace_back(index.funcRefs.at(w));
        finfoRefs.emplace_back(index.funcInfoRefs.at(w));
        funcNames.emplace_back(w);
      }
    }
    assertx(!missingSet.empty());

    std::vector<SString> missing{begin(missingSet), end(missingSet)};
    std::sort(begin(missing), end(missing), string_data_lti{});

    auto [missingRef, config] = HPHP_CORO_AWAIT(
      coro::collect(
        index.client->store(std::move(missing)),
        index.configRef->getCopy()
      )
    );

    auto results = HPHP_CORO_AWAIT(
      index.client->exec(
        s_missingClassFixupJob,
        std::move(config),
        singleton_vec(
          std::make_tuple(
            std::move(clsRefs),
            std::move(cinfoRefs),
            std::move(funcRefs),
            std::move(finfoRefs),
            std::move(missingRef)
          )
        ),
        std::move(metadata)
      )
    );
    assertx(results.size() == 1);
    auto& [outClsRefs, outCInfoRefs, outFuncRefs, outFInfoRefs] = results[0];
    assertx(outClsRefs.size() == clsNames.size());
    assertx(outFuncRefs.size() == funcNames.size());
    assertx(outClsRefs.size() == outCInfoRefs.size());
    assertx(outFuncRefs.size() == outFInfoRefs.size());

    for (size_t i = 0, size = clsNames.size(); i < size; ++i) {
      index.classRefs.at(clsNames[i]) = outClsRefs[i];
      index.classInfoRefs.at(clsNames[i]) = outCInfoRefs[i];
    }
    for (size_t i = 0, size = funcNames.size(); i < size; ++i) {
      index.funcRefs.at(funcNames[i]) = outFuncRefs[i];
      index.funcInfoRefs.at(funcNames[i]) = outFInfoRefs[i];
    }

    HPHP_CORO_RETURN_VOID;
  };

  coro::wait(coro::collectRange(
    from(buckets)
      | move
      | map([&] (std::vector<SString> work) {
          return run(std::move(work)).scheduleOn(index.executor->sticky());
        })
      | as<std::vector>()
  ));
}

// Run FixupUnitJob on all of the given fixups and store the new
// php::Unit refs in the Index.
void flatten_classes_fixup_units(IndexData& index,
                                 SStringToOneT<UnitFixupJob::Fixup> allFixups) {
  trace_time trace("flatten classes fixup units", index.sample);

  using namespace folly::gen;

  auto const run = [&] (std::vector<SString> units) -> coro::Task<void> {
    HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;

    if (units.empty()) HPHP_CORO_RETURN_VOID;

    std::vector<UnitFixupJob::Fixup> fixups;

    // Gather up the fixups and ensure a deterministic ordering.
    fixups.reserve(units.size());
    for (auto const unit : units) {
      auto f = std::move(allFixups.at(unit));
      assertx(!f.addClass.empty() || !f.removeFunc.empty());
      std::sort(f.addClass.begin(), f.addClass.end(), string_data_lti{});
      std::sort(f.removeFunc.begin(), f.removeFunc.end(), string_data_lt{});
      fixups.emplace_back(std::move(f));
    }
    auto fixupRefs =
      HPHP_CORO_AWAIT(index.client->storeMulti(std::move(fixups)));
    assertx(fixupRefs.size() == units.size());

    std::vector<std::tuple<UniquePtrRef<php::Unit>, Ref<UnitFixupJob::Fixup>>>
      inputs;
    inputs.reserve(units.size());

    for (size_t i = 0, size = units.size(); i < size; ++i) {
      inputs.emplace_back(
        index.unitRefs.at(units[i]),
        std::move(fixupRefs[i])
      );
    }

    Client::ExecMetadata metadata{
      .job_key = folly::sformat("fixup units {}", units[0])
    };

    auto config = HPHP_CORO_AWAIT(index.configRef->getCopy());
    auto outputs = HPHP_CORO_AWAIT(index.client->exec(
      s_unitFixupJob,
      std::move(config),
      std::move(inputs),
      std::move(metadata)
    ));
    assertx(outputs.size() == units.size());

    // Every unit is already in the Index table, so we can overwrite
    // them without locking.
    for (size_t i = 0, size = units.size(); i < size; ++i) {
      index.unitRefs.at(units[i]) = std::move(outputs[i]);
    }

    HPHP_CORO_RETURN_VOID;
  };

  constexpr size_t kBucketSize = 3000;

  // Bucketize by unit
  auto buckets = consistently_bucketize(
    [&] {
      std::vector<SString> sorted;
      sorted.reserve(allFixups.size());
      for (auto& [unit, _] : allFixups) sorted.emplace_back(unit);
      std::sort(sorted.begin(), sorted.end(), string_data_lt{});
      return sorted;
    }(),
    kBucketSize
  );

  coro::wait(coro::collectRange(
    from(buckets)
      | move
      | map([&] (std::vector<SString> units) {
          return run(std::move(units)).scheduleOn(index.executor->sticky());
        })
      | as<std::vector>()
  ));
}

// Metadata used to assign work buckets for building subclasses. This
// is produced from flattening classes. We don't put closures (or
// Closure base class) into here. There's a lot of them, but we can
// predict their results without running build subclass pass on them.
struct SubclassMetadata {
  // Immediate children and parents of class (not transitive!).
  struct Meta {
    std::vector<SString> children;
    std::vector<SString> parents;
    size_t idx; // Index into all classes vector.
  };
  ISStringToOneT<Meta> meta;
  // All classes to be processed
  std::vector<SString> all;
  // Classes which are interfaces
  ISStringSet interfaces;
};

SubclassMetadata flatten_classes(IndexData& index, IndexFlattenMetadata meta) {
  trace_time trace("flatten classes", index.sample);

  using namespace folly::gen;

  struct ClassUpdate {
    SString name;
    UniquePtrRef<php::Class> cls;
    UniquePtrRef<php::ClassBytecode> bytecode;
    UniquePtrRef<ClassInfo2> cinfo;
    SString unitToAddTo;
    ISStringSet typeUses;
    bool isInterface{false};
    CompactVector<SString> parents;
  };
  struct FuncUpdate {
    SString name;
    UniquePtrRef<php::Func> func;
    UniquePtrRef<FuncInfo2> finfo;
    ISStringSet typeUses;
  };
  using Update = boost::variant<ClassUpdate, FuncUpdate>;
  using UpdateVec = std::vector<Update>;

  ISStringSet uninstantiable;
  std::mutex uninstantiableLock;

  auto const run = [&] (FlattenClassesWork work) -> coro::Task<UpdateVec> {
    HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;

    if (work.classes.empty() && work.funcs.empty()) {
      assertx(work.deps.empty());
      HPHP_CORO_RETURN(UpdateVec{});
    }

    Client::ExecMetadata metadata{
      .job_key = folly::sformat(
        "flatten classes {}",
        work.classes.empty() ? work.funcs[0] : work.classes[0]
      )
    };
    auto classes = from(work.classes)
      | map([&] (SString c) { return index.classRefs.at(c); })
      | as<std::vector>();
    auto deps = from(work.deps)
      | map([&] (SString c) { return index.classRefs.at(c); })
      | as<std::vector>();
    auto bytecode = (from(work.classes) + from(work.deps))
      | map([&] (SString c) { return index.classBytecodeRefs.at(c); })
      | as<std::vector>();
    auto funcs = from(work.funcs)
      | map([&] (SString f) { return index.funcRefs.at(f); })
      | as<std::vector>();

    // Gather any type-mappings or missing types referenced by these
    // classes or funcs.
    std::vector<TypeMapping> typeMappings;
    std::vector<SString> missingTypes;
    {
      ISStringSet seen;

      auto const addUnresolved = [&] (SString u) {
        if (!seen.emplace(u).second) return;
        if (auto const m = folly::get_ptr(meta.typeMappings, u)) {
          // If the type-mapping maps an enum, and that enum is
          // uninstantiable, just treat it as a missing type.
          if (m->firstEnum && meta.cls.at(m->firstEnum).uninstantiable) {
            missingTypes.emplace_back(u);
          } else {
            typeMappings.emplace_back(*m);
          }
        } else if (!index.classRefs.count(u) ||
                   meta.cls.at(u).uninstantiable) {
          missingTypes.emplace_back(u);
        }
      };

      auto const addClass = [&] (SString c) {
        for (auto const u : meta.cls.at(c).unresolvedTypes) addUnresolved(u);
      };
      auto const addFunc = [&] (SString f) {
        for (auto const u : meta.func.at(f).unresolvedTypes) addUnresolved(u);
      };

      for (auto const c : work.classes) addClass(c);
      for (auto const d : work.deps)    addClass(d);
      for (auto const f : work.funcs)   addFunc(f);

      std::sort(
        begin(typeMappings), end(typeMappings),
        [] (const TypeMapping& a, const TypeMapping& b) {
          return string_data_lti{}(a.name, b.name);
        }
      );
      std::sort(begin(missingTypes), end(missingTypes), string_data_lti{});
    }

    auto [typeMappingsRef, missingTypesRef, config] = HPHP_CORO_AWAIT(
      coro::collect(
        index.client->store(std::move(typeMappings)),
        index.client->store(std::move(missingTypes)),
        index.configRef->getCopy()
      )
    );

    auto results = HPHP_CORO_AWAIT(
      index.client->exec(
        s_flattenJob,
        std::move(config),
        singleton_vec(
          std::make_tuple(
            std::move(classes),
            std::move(deps),
            std::move(bytecode),
            std::move(funcs),
            std::move(typeMappingsRef),
            std::move(missingTypesRef)
          )
        ),
        std::move(metadata)
      )
    );
    // Every flattening job is a single work-unit, so we should only
    // ever get one result for each one.
    assertx(results.size() == 1);
    auto& [clsRefs, bytecodeRefs, cinfoRefs, funcRefs, finfoRefs, classMetaRef]
      = results[0];
    assertx(clsRefs.size() == cinfoRefs.size());
    assertx(clsRefs.size() == bytecodeRefs.size());
    assertx(funcRefs.size() == work.funcs.size());
    assertx(funcRefs.size() == finfoRefs.size());

    // We need the output metadata, but everything else stays
    // uploaded.
    auto clsMeta = HPHP_CORO_AWAIT(index.client->load(std::move(classMetaRef)));

    // Create the updates by combining the job output (but skipping
    // over uninstantiable classes).
    UpdateVec updates;
    updates.reserve(work.classes.size() * 3);

    size_t outputIdx = 0;
    size_t parentIdx = 0;
    for (auto const name : work.classes) {
      if (clsMeta.uninstantiable.count(name)) {
        // Uninstantiable classes are very rare, so taking a lock here
        // is fine.
        std::scoped_lock<std::mutex> _{uninstantiableLock};
        uninstantiable.emplace(name);
        continue;
      }

      assertx(outputIdx < clsRefs.size());
      assertx(outputIdx < clsMeta.classTypeUses.size());
      updates.emplace_back(
        ClassUpdate{
          name,
          std::move(clsRefs[outputIdx]),
          std::move(bytecodeRefs[outputIdx]),
          std::move(cinfoRefs[outputIdx]),
          nullptr,
          std::move(clsMeta.classTypeUses[outputIdx]),
          (bool)clsMeta.interfaces.count(name)
        }
      );

      // Ignore closures. We don't run the build subclass pass for
      // closures, so we don't need information for them.
      if (!meta.cls.at(name).isClosure) {
        assertx(parentIdx < clsMeta.parents.size());
        auto const& parents = clsMeta.parents[parentIdx].names;
        auto& update = boost::get<ClassUpdate>(updates.back());
        update.parents.insert(
          end(update.parents), begin(parents), end(parents)
        );
        ++parentIdx;
      }

      ++outputIdx;
    }

    for (auto const& [unit, names] : clsMeta.newClosures) {
      for (auto const name : names) {
        assertx(outputIdx < clsRefs.size());
        assertx(outputIdx < clsMeta.classTypeUses.size());
        updates.emplace_back(
          ClassUpdate{
            name,
            std::move(clsRefs[outputIdx]),
            std::move(bytecodeRefs[outputIdx]),
            std::move(cinfoRefs[outputIdx]),
            unit,
            std::move(clsMeta.classTypeUses[outputIdx])
          }
        );
        ++outputIdx;
      }
    }
    assertx(outputIdx == clsRefs.size());
    assertx(outputIdx == clsMeta.classTypeUses.size());

    assertx(work.funcs.size() == clsMeta.funcTypeUses.size());
    for (size_t i = 0, size = work.funcs.size(); i < size; ++i) {
      updates.emplace_back(
        FuncUpdate{
          work.funcs[i],
          std::move(funcRefs[i]),
          std::move(finfoRefs[i]),
          std::move(clsMeta.funcTypeUses[i])
        }
      );
    }

    HPHP_CORO_MOVE_RETURN(updates);
  };

  // Calculate the grouping of classes into work units for flattening,
  // perform the flattening, and gather all updates from the jobs.
  auto allUpdates = [&] {
    auto assignments = flatten_classes_assign(meta);

    trace_time trace2("flatten classes work", index.sample);
    return coro::wait(coro::collectRange(
      from(assignments)
        | move
        | map([&] (FlattenClassesWork w) {
            return run(std::move(w)).scheduleOn(index.executor->sticky());
          })
        | as<std::vector>()
    ));
  }();

  // Now take the updates and apply them to the Index tables. This
  // needs to be done in a single threaded context (per data
  // structure). This also gathers up all the fixups needed.

  SStringToOneT<UnitFixupJob::Fixup> unitFixups;
  SubclassMetadata subclassMeta;

  ISStringToOneT<ISStringSet> typeUsesByClass;
  ISStringToOneT<ISStringSet> typeUsesByFunc;

  {
    trace_time trace2("flatten classes update");
    trace2.ignore_client_stats();

    parallel::parallel(
      [&] {
        for (auto& updates : allUpdates) {
          for (auto& update : updates) {
            auto u = boost::get<ClassUpdate>(&update);
            if (!u) continue;
            index.classRefs.insert_or_assign(
              u->name,
              std::move(u->cls)
            );
          }
        }
      },
      [&] {
        for (auto& updates : allUpdates) {
          for (auto& update : updates) {
            auto u = boost::get<ClassUpdate>(&update);
            if (!u) continue;
            always_assert(
              index.classInfoRefs.emplace(
                u->name,
                std::move(u->cinfo)
              ).second
            );
          }
        }
      },
      [&] {
        for (auto& updates : allUpdates) {
          for (auto& update : updates) {
            auto u = boost::get<ClassUpdate>(&update);
            if (!u) continue;
            index.classBytecodeRefs.insert_or_assign(
              u->name,
              std::move(u->bytecode)
            );
          }
        }
      },
      [&] {
        for (auto& updates : allUpdates) {
          for (auto& update : updates) {
            auto u = boost::get<FuncUpdate>(&update);
            if (!u) continue;
            index.funcRefs.at(u->name) = std::move(u->func);
          }
        }
      },
      [&] {
        for (auto& updates : allUpdates) {
          for (auto& update : updates) {
            auto u = boost::get<FuncUpdate>(&update);
            if (!u) continue;
            always_assert(
              index.funcInfoRefs.emplace(
                u->name,
                std::move(u->finfo)
              ).second
            );
          }
        }
      },
      [&] {
        for (auto& updates : allUpdates) {
          for (auto& update : updates) {
            auto u = boost::get<ClassUpdate>(&update);
            if (!u || !u->unitToAddTo) continue;
            unitFixups[u->unitToAddTo].addClass.emplace_back(u->name);
          }
        }
        for (auto& [unit, deletions] : meta.unitDeletions) {
          unitFixups[unit].removeFunc = std::move(deletions);
        }
      },
      [&] {
        for (auto& updates : allUpdates) {
          for (auto& update : updates) {
            if (auto const u = boost::get<ClassUpdate>(&update)) {
              for (auto const t : u->typeUses) {
                typeUsesByClass[t].emplace(u->name);
              }
            } else if (auto const f = boost::get<FuncUpdate>(&update)) {
              for (auto const t : f->typeUses) {
                typeUsesByFunc[t].emplace(f->name);
              }
            }
          }
        }
      },
      [&] {
        // Build metadata for the next build subclass pass.
        auto& all = subclassMeta.all;
        auto& meta = subclassMeta.meta;
        for (auto& updates : allUpdates) {
          for (auto& update : updates) {
            auto u = boost::get<ClassUpdate>(&update);
            if (!u) continue;

            // We shouldn't have parents for closures because we
            // special case those explicitly.
            if (is_closure_name(u->name) ||
                u->name->isame(s_Closure.get())) {
              assertx(u->parents.empty());
              continue;
            }
            // Otherwise build the children lists from the parents.
            all.emplace_back(u->name);
            for (auto const p : u->parents) {
              meta[p].children.emplace_back(u->name);
            }
            auto& parents = meta[u->name].parents;
            assertx(parents.empty());
            parents.insert(
              end(parents),
              begin(u->parents), end(u->parents)
            );
          }
        }

        std::sort(begin(all), end(all), string_data_lti{});
        // Make sure there's no duplicates:
        assertx(std::adjacent_find(begin(all), end(all)) == end(all));

        for (size_t i = 0; i < all.size(); ++i) meta[all[i]].idx = i;
      },
      [&] {
        for (auto& updates : allUpdates) {
          for (auto& update : updates) {
            auto u = boost::get<ClassUpdate>(&update);
            if (!u || !u->isInterface) continue;
            subclassMeta.interfaces.emplace(u->name);
          }
        }
      }
    );
  }

  flatten_classes_fixup_missing(
    index,
    std::move(typeUsesByClass),
    std::move(typeUsesByFunc),
    std::move(uninstantiable)
  );

  // Apply the fixups
  flatten_classes_fixup_units(index, std::move(unitFixups));

  return subclassMeta;
}

//////////////////////////////////////////////////////////////////////
// Subclass list

/*
 * Subclass lists are built in a similar manner as flattening classes,
 * except the order is reversed.
 *
 * However, there is one complication: the transitive children of each
 * class can be huge. In fact, for large hierarchies, they can easily
 * be too large to (efficiently) handle in a single job.
 *
 * Rather than (always) processing everything in a single pass, we
 * might need to use multiple passes to keep the fan-in down. When
 * calculating the work buckets, we keep the size of each bucket into
 * account and don't allow any bucket to grow too large. If it does,
 * we'll just process that bucket, and process any dependencies in the
 * next pass.
 *
 * This isn't sufficient. A single class have (far) more direct
 * children than we want in a single bucket. Multiple passes don't
 * help here because there's no intermediate classes to use as an
 * output. To fix this, we insert "splits", which serve to "summarize"
 * some subset of a class' direct children.
 *
 * For example, suppose a class has 10k direct children, and our
 * maximum bucket size is 1k. On the first pass we'll process all of
 * the children in ~10 different jobs, each one processing 1k of the
 * children, and producing a single split node. The second pass will
 * actually process the class and take all of the splits as inputs
 * (not the actual children). The inputs to the job has been reduced
 * from 10k to 10. This is a simplification. In reality a job can
 * produce multiple splits, and inputs can be a mix of splits and
 * actual classes. In extreme cases, you might need multiple rounds of
 * splits before processing the class.
 *
 * There is one other difference between this and the flatten classes
 * pass. Unlike in flatten classes, every class (except leafs) are
 * "roots" here. We do not promote any dependencies. This causes more
 * work overall, but it lets us process more classes in parallel.
 */

/*
 * Extern-worker job to build ClassInfo2 subclass lists, and calculate
 * various properties on the ClassInfo2 from it.
 */
struct BuildSubclassListJob {
  static std::string name() { return "hhbbc-build-subclass"; }
  static void init(const Config& config) {
    process_init(config.o, config.gd, false);
  }
  static void fini() {}

  // Aggregated data for some group of classes. The data can either
  // come from a split node, or inferred from a group of classes.
  struct Data {
    // All of the classes comprising this data.
    ISStringSet subclasses;

    // All of the interfaces implemented by the classes comprising
    // this data.
    ISStringSet implInterfaces;

    // Information about all of the methods with a particular name
    // between all of the classes in this Data.
    struct MethInfo {
      // Methods which are present on at least one regular class.
      MethRefSet regularMeths;
      // Methods which are only present on non-regular classes, but is
      // private on at least one class. These are sometimes treated
      // like a regular class.
      MethRefSet nonRegularPrivateMeths;
      // Methods which are only present on non-regular classes (and
      // never private). These three sets are always disjoint.
      MethRefSet nonRegularMeths;

      Optional<FuncFamily2::StaticInfo> allStatic;
      Optional<FuncFamily2::StaticInfo> regularStatic;

      // Whether all classes in this Data have a method with this
      // name.
      bool complete{true};
      // Whether all regular classes in this Data have a method with
      // this name.
      bool regularComplete{true};
      // Whether any of the methods has a private ancestor.
      bool privateAncestor{false};

      template <typename SerDe> void serde(SerDe& sd) {
        sd(regularMeths, std::less<MethRef>{})
          (nonRegularPrivateMeths, std::less<MethRef>{})
          (nonRegularMeths, std::less<MethRef>{})
          (allStatic)
          (regularStatic)
          (complete)
          (regularComplete)
          (privateAncestor)
          ;
      }
    };
    SStringToOneT<MethInfo> methods;

    // The name of properties which might have null values even if the
    // type-constraint doesn't allow it (due to system provided
    // initial values).
    SStringSet propsWithImplicitNullable;

    // The classes for whom isMocked would be true due to one of the
    // classes making up this Data. The classes in this set may not
    // necessarily be also part of this Data.
    ISStringSet mockedClasses;

    bool hasConstProp{false};
    bool hasReifiedGeneric{false};

    bool isSubMocked{false};

    bool hasRegularClass{false};
    bool hasNonRegularClass{false};
    bool hasRegularSubclass{false};
    bool hasNonRegularSubclass{false};

    template <typename SerDe> void serde(SerDe& sd) {
      sd(subclasses, string_data_lti{})
        (implInterfaces, string_data_lti{})
        (methods, string_data_lt{})
        (propsWithImplicitNullable, string_data_lt{})
        (mockedClasses, string_data_lti{})
        (hasConstProp)
        (hasReifiedGeneric)
        (isSubMocked)
        (hasRegularClass)
        (hasNonRegularClass)
        (hasRegularSubclass)
        (hasNonRegularSubclass)
        ;
    }
  };

  // Split node. Used to wrap a Data when summarizing some subset of a
  // class' children.
  struct Split {
    Split() = default;
    explicit Split(SString name) : name{name} {}

    SString name;
    CompactVector<SString> children;
    Data data;

    template <typename SerDe> void serde(SerDe& sd) {
      sd(name)
        (children)
        (data)
        ;
    }
  };

  // Mark a dependency on a class to a split node. Since the splits
  // are not actually part of the hierarchy, the relationship between
  // classes and splits cannot be inferred otherwise.
  struct EdgeToSplit {
    SString cls;
    SString split;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(cls)
        (split)
        ;
    }
  };

  // Job output meant to be downloaded and drive the next round.
  struct OutputMeta {
    // For every input ClassInfo, the set of func families present in
    // that ClassInfo's method family table. If the ClassInfo is used
    // as a dep later, these func families need to be provided as
    // well.
    std::vector<hphp_fast_set<FuncFamily2::Id>> funcFamilyDeps;
    // The ids of new (not provided as an input) func families
    // produced. The ids are grouped together to become
    // FuncFamilyGroups.
    std::vector<std::vector<FuncFamily2::Id>> newFuncFamilyIds;
    // Func family entries corresponding to all methods with a
    // particular name encountered in this job. Multiple jobs will
    // generally produce func family entries for the same name, so
    // they must be aggregated together afterwards.
    std::vector<std::pair<SString, FuncFamilyEntry>> nameOnly;
    std::vector<InterfaceConflicts> interfaces;

    template <typename SerDe> void serde(SerDe& sd) {
      sd(funcFamilyDeps, std::less<FuncFamily2::Id>{})
        (newFuncFamilyIds)
        (nameOnly)
        (interfaces)
        ;
    }
  };
  using Output = Multi<
    Variadic<std::unique_ptr<ClassInfo2>>,
    Variadic<std::unique_ptr<Split>>,
    Variadic<std::unique_ptr<php::Class>>,
    Variadic<FuncFamilyGroup>,
    OutputMeta
  >;

  // Each job takes the list of classes and splits which should be
  // produced, dependency classes and splits (which are not updated),
  // edges between classes and splits, and func families (needed by
  // dependency classes). Leafs are like deps, except they'll be
  // considered as part of calculating the name-only func families
  // (normal deps are not).
  static Output
  run(Variadic<std::unique_ptr<ClassInfo2>> classes,
      Variadic<std::unique_ptr<ClassInfo2>> deps,
      Variadic<std::unique_ptr<ClassInfo2>> leafs,
      Variadic<std::unique_ptr<Split>> splits,
      Variadic<std::unique_ptr<Split>> splitDeps,
      Variadic<std::unique_ptr<php::Class>> phpClasses,
      Variadic<EdgeToSplit> edges,
      Variadic<FuncFamilyGroup> funcFamilies) {
    // Store mappings of names to classes and edges.
    LocalIndex index;
    for (auto& cinfo : classes.vals) {
      always_assert(
        index.classInfos.emplace(cinfo->name, cinfo.get()).second
      );
      index.top.emplace(cinfo->name);
    }
    for (auto& cinfo : deps.vals) {
      always_assert(
        index.classInfos.emplace(cinfo->name, cinfo.get()).second
      );
    }
    for (auto& cinfo : leafs.vals) {
      always_assert(
        index.classInfos.emplace(cinfo->name, cinfo.get()).second
      );
    }
    for (auto& split : splits.vals) {
      always_assert(
        index.splits.emplace(split->name, split.get()).second
      );
      index.top.emplace(split->name);
    }
    for (auto& split : splitDeps.vals) {
      always_assert(
        index.splits.emplace(split->name, split.get()).second
      );
    }
    for (auto& cls : phpClasses.vals) {
      always_assert(
        index.classes.emplace(cls->name, cls.get()).second
      );
    }
    for (auto& group : funcFamilies.vals) {
      for (auto& ff : group.m_ffs) {
        auto const id = ff->m_id;
        // We could have multiple groups which contain the same
        // FuncFamily, so don't assert uniqueness here. We'll just
        // take the first one we see (they should all be equivalent).
        index.funcFamilies.emplace(id, std::move(ff));
      }
    }

    OutputMeta meta;

    // If there's no classes or splits, this job is doing nothing but
    // calculating name only func family entries (so should have at
    // least one leaf).
    if (!index.top.empty()) {
      build_children(index, edges.vals);
      process_roots(index, classes.vals, splits.vals);

      for (auto const& cinfo : classes.vals) {
        auto const& cls = index.cls(cinfo->name);
        if (!(cls.attrs & AttrInterface)) continue;
        assertx(cinfo->subImplInterfaces);

        meta.interfaces.emplace_back(InterfaceConflicts{cinfo->name});

        auto& back = meta.interfaces.back();
        back.usage = cinfo->subclassList.size();

        for (auto const iface : *cinfo->subImplInterfaces) {
          if (iface->isame(cinfo->name)) continue;
          back.conflicts.emplace_back(iface);
        }
        std::sort(
          begin(back.conflicts),
          end(back.conflicts),
          string_data_lti{}
        );
      }
    } else {
      assertx(classes.vals.empty());
      assertx(splits.vals.empty());
      assertx(index.splits.empty());
      assertx(index.funcFamilies.empty());
      assertx(!leafs.vals.empty());
      assertx(!index.classInfos.empty());
    }

    meta.nameOnly =
      make_name_only_method_entries(index, classes.vals, leafs.vals);

    // Record dependencies for each input class. A func family is a
    // dependency of the class if it appears in the method families
    // table.
    meta.funcFamilyDeps.reserve(classes.vals.size());
    for (auto const& cinfo : classes.vals) {
      meta.funcFamilyDeps.emplace_back();
      auto& deps = meta.funcFamilyDeps.back();
      for (auto const& [_, entry] : cinfo->methodFamilies) {
        match<void>(
          entry.m_meths,
          [&] (const FuncFamilyEntry::BothFF& e)      { deps.emplace(e.m_ff); },
          [&] (const FuncFamilyEntry::FFAndSingle& e) { deps.emplace(e.m_ff); },
          [&] (const FuncFamilyEntry::FFAndNone& e)   { deps.emplace(e.m_ff); },
          [&] (const FuncFamilyEntry::BothSingle&)    {},
          [&] (const FuncFamilyEntry::SingleAndNone&) {},
          [&] (const FuncFamilyEntry::None&)          {}
        );
      }
    }

    Variadic<FuncFamilyGroup> funcFamilyGroups;
    group_func_families(index, funcFamilyGroups.vals, meta.newFuncFamilyIds);

    // We only need to provide php::Class which correspond to a class
    // which wasn't a dep.
    phpClasses.vals.erase(
      std::remove_if(
        begin(phpClasses.vals),
        end(phpClasses.vals),
        [&] (const std::unique_ptr<php::Class>& c) {
          return !index.top.count(c->name);
        }
      ),
      end(phpClasses.vals)
    );

    return std::make_tuple(
      std::move(classes),
      std::move(splits),
      std::move(phpClasses),
      std::move(funcFamilyGroups),
      std::move(meta)
    );
  }

protected:

  /*
   * MethInfo caching
   *
   * When building a func family, MethRefSets must be sorted, then
   * hashed in order to generate the unique id. Once we do so, we can
   * then check if that func family already exists. For large func
   * families, this can very expensive and we might have to do this
   * (wasted) work multiple times.
   *
   * To avoid this, we add a cache before the sorting/hashing
   * step. Instead of using a func family id (which is the expensive
   * thing to generate), the cache is keyed by the set of methods
   * directly. A commutative hash is used so that we don't actually
   * have to sort the MethRefSets, and equality is just equality of
   * the MethRefSets. Moreover, we make use of hetereogenous lookup to
   * avoid having to copy any MethRefSets (again they can be large)
   * when doing the lookup.
   */

  // What is actually stored in the cache. Keeps a copy of the
  // MethRefSets.
  struct MethInfoTuple {
    MethRefSet regular;
    MethRefSet nonRegularPrivate;
    MethRefSet nonRegular;
  };
  // Used for lookups. Just has pointers to the MethRefSets, so we
  // don't have to do any copying for the lookup.
  struct MethInfoTupleProxy {
    const MethRefSet* regular;
    const MethRefSet* nonRegularPrivate;
    const MethRefSet* nonRegular;
  };

  struct MethInfoTupleHasher {
    using is_transparent = void;

    size_t operator()(const MethInfoTuple& t) const {
      auto const h1 = folly::hash::commutative_hash_combine_range_generic(
        0, MethRef::Hash{}, begin(t.regular), end(t.regular)
      );
      auto const h2 = folly::hash::commutative_hash_combine_range_generic(
        0, MethRef::Hash{}, begin(t.nonRegularPrivate), end(t.nonRegularPrivate)
      );
      auto const h3 = folly::hash::commutative_hash_combine_range_generic(
        0, MethRef::Hash{}, begin(t.nonRegular), end(t.nonRegular)
      );
      return folly::hash::hash_combine(h1, h2, h3);
    }
    size_t operator()(const MethInfoTupleProxy& t) const {
      auto const h1 = folly::hash::commutative_hash_combine_range_generic(
        0, MethRef::Hash{}, begin(*t.regular), end(*t.regular)
      );
      auto const h2 = folly::hash::commutative_hash_combine_range_generic(
        0, MethRef::Hash{}, begin(*t.nonRegularPrivate), end(*t.nonRegularPrivate)
      );
      auto const h3 = folly::hash::commutative_hash_combine_range_generic(
        0, MethRef::Hash{}, begin(*t.nonRegular), end(*t.nonRegular)
      );
      return folly::hash::hash_combine(h1, h2, h3);
    }
  };
  struct MethInfoTupleEquals {
    using is_transparent = void;

    bool operator()(const MethInfoTuple& t1, const MethInfoTuple& t2) const {
      return
        t1.regular == t2.regular &&
        t1.nonRegularPrivate == t2.nonRegularPrivate &&
        t1.nonRegular == t2.nonRegular;
    }
    bool operator()(const MethInfoTupleProxy& t1,
                    const MethInfoTuple& t2) const {
      return
        *t1.regular == t2.regular &&
        *t1.nonRegularPrivate == t2.nonRegularPrivate &&
        *t1.nonRegular == t2.nonRegular;
    }
  };

  struct LocalIndex {
    // All ClassInfos, whether inputs or dependencies.
    ISStringToOneT<ClassInfo2*> classInfos;
    // All splits, whether inputs or dependencies.
    ISStringToOneT<Split*> splits;
    // All php::Class, whether inputs or dependencies.
    ISStringToOneT<php::Class*> classes;

    // ClassInfos and splits which are inputs (IE, we want to
    // calculate data for).
    ISStringSet top;

    // Mapping of input ClassInfos/splits to all of their subclasses
    // present in this Job. Some of the children may be splits, which
    // means some subset of the children were processed in another
    // Job.
    ISStringToOneT<ISStringSet> children;

    // All func families available in this Job, either from inputs, or
    // created during processing.
    hphp_fast_map<FuncFamily2::Id, std::unique_ptr<FuncFamily2>> funcFamilies;

    // Above mentioned func family cache. If an entry is present here,
    // we know the func family already exists and don't need to do
    // expensive sorting/hashing.
    hphp_fast_map<
      MethInfoTuple,
      FuncFamily2::Id,
      MethInfoTupleHasher,
      MethInfoTupleEquals
    > funcFamilyCache;

    // funcFamilies contains all func families. If a func family is
    // created during processing, it will be inserted here (used to
    // determine outputs).
    std::vector<FuncFamily2::Id> newFuncFamilies;

    php::Class& cls(SString name) {
      auto const it = classes.find(name);
      always_assert(it != end(classes));
      return *it->second;
    }
  };

  // Take all of the func families produced by this job and group them
  // together into FuncFamilyGroups. We produce both the
  // FuncFamilyGroups themselves, but also the associated ids in each
  // group (which will be output as metadata).
  static void group_func_families(
    LocalIndex& index,
    std::vector<FuncFamilyGroup>& groups,
    std::vector<std::vector<FuncFamily2::Id>>& ids
  ) {
    constexpr size_t kGroupSize = 5000;

    // The grouping algorithm is very simple. First we sort all of the
    // func families by size. We then just group adjacent families
    // until their total size exceeds some threshold. Once it does, we
    // start a new group.
    std::sort(
      begin(index.newFuncFamilies),
      end(index.newFuncFamilies),
      [&] (const FuncFamily2::Id& id1, const FuncFamily2::Id& id2) {
        auto const& ff1 = index.funcFamilies.at(id1);
        auto const& ff2 = index.funcFamilies.at(id2);
        auto const size1 =
          ff1->m_regular.size() +
          ff1->m_nonRegularPrivate.size() +
          ff1->m_nonRegular.size();
        auto const size2 =
          ff2->m_regular.size() +
          ff2->m_nonRegularPrivate.size() +
          ff2->m_nonRegular.size();
        if (size1 != size2) return size1 < size2;
        return id1 < id2;
      }
    );

    size_t current = 0;
    for (auto const& id : index.newFuncFamilies) {
      auto& ff = index.funcFamilies.at(id);
      auto const size =
        ff->m_regular.size() +
        ff->m_nonRegularPrivate.size() +
        ff->m_nonRegular.size();
      if (groups.empty() || current > kGroupSize) {
        groups.emplace_back();
        ids.emplace_back();
        current = 0;
      }
      groups.back().m_ffs.emplace_back(std::move(ff));
      ids.back().emplace_back(id);
      current += size;
    }
  }

  // Produce a set of name-only func families from the given set of
  // roots and leafs. It is assumed that any roots have already been
  // processed by process_roots, so that they'll have any appropriate
  // method families on them. Only entries which are "first name" are
  // processed.
  static std::vector<std::pair<SString, FuncFamilyEntry>>
  make_name_only_method_entries(
    LocalIndex& index,
    const std::vector<std::unique_ptr<ClassInfo2>>& roots,
    const std::vector<std::unique_ptr<ClassInfo2>>& leafs
  ) {
    SStringToOneT<Data::MethInfo> infos;

    // Use the already calculated method family and merge
    // it's contents into what we already have.
    auto const process = [&] (const ClassInfo2* cinfo,
                              SString name) {
      auto const it = cinfo->methodFamilies.find(name);
      always_assert(it != end(cinfo->methodFamilies));
      auto entryInfo = meth_info_from_func_family_entry(index, it->second);

      auto& info = infos[name];
      info.complete = false;
      info.regularComplete = false;

      for (auto const& meth : entryInfo.regularMeths) {
        if (info.regularMeths.count(meth)) continue;
        info.regularMeths.emplace(meth);
        info.nonRegularPrivateMeths.erase(meth);
        info.nonRegularMeths.erase(meth);
      }
      for (auto const& meth : entryInfo.nonRegularPrivateMeths) {
        if (info.regularMeths.count(meth) ||
            info.nonRegularPrivateMeths.count(meth)) {
          continue;
        }
        info.nonRegularPrivateMeths.emplace(meth);
        info.nonRegularMeths.erase(meth);
      }
      for (auto const& meth : entryInfo.nonRegularMeths) {
        if (info.regularMeths.count(meth) ||
            info.nonRegularPrivateMeths.count(meth) ||
            info.nonRegularMeths.count(meth)) {
          continue;
        }
        info.nonRegularMeths.emplace(meth);
      }

      // Merge any StaticInfo entries we have for this method.
      if (entryInfo.allStatic) {
        if (!info.allStatic) {
          info.allStatic = std::move(*entryInfo.allStatic);
        } else {
          *info.allStatic |= *entryInfo.allStatic;
        }
      }
      if (entryInfo.regularStatic) {
        if (!info.regularStatic) {
          info.regularStatic = std::move(*entryInfo.regularStatic);
        } else {
          *info.regularStatic |= *entryInfo.regularStatic;
        }
      }
    };

    // First process the roots. These methods might be overridden or
    // not.
    for (auto const& cinfo : roots) {
      for (auto const& [name, mte] : cinfo->methods) {
        if (!mte.firstName()) continue;
        if (!has_name_only_func_family(name)) continue;
        process(cinfo.get(), name);
      }
    }

    // Leafs are by definition always AttrNoOverride.
    for (auto const& cinfo : leafs) {
      for (auto const& [name, mte] : cinfo->methods) {
        if (!mte.firstName()) continue;
        if (!has_name_only_func_family(name)) continue;
        process(cinfo.get(), name);
      }
    }

    // Make the MethInfo order deterministic
    std::vector<SString> sorted;
    sorted.reserve(infos.size());
    for (auto const& [name, _] : infos) sorted.emplace_back(name);
    std::sort(begin(sorted), end(sorted), string_data_lt{});

    std::vector<std::pair<SString, FuncFamilyEntry>> entries;
    entries.reserve(infos.size());

    // Turn the MethInfos into FuncFamilyEntries
    for (auto const name : sorted) {
      auto& info = infos.at(name);
      entries.emplace_back(
        name,
        make_method_family_entry(index, name, std::move(info))
      );
    }
    return entries;
  }

  // From the information present in the inputs, calculate a mapping
  // of classes and splits to their children (which can be other
  // classes or split nodes). This is not just direct children, but
  // all transitive subclasses.
  static void build_children(LocalIndex& index,
                             const std::vector<EdgeToSplit>& edges) {
    // First record direct children. This can be inferred from the
    // parents of all present ClassInfos:

    auto const onParent = [&] (SString parent, const ClassInfo2* child) {
      // Due to how work is divided, a class might have parents not
      // present in this job. Ignore those.
      if (!index.classInfos.count(parent)) return;
      index.children[parent].emplace(child->name);
    };

    // For the purposes of this algorithm, the only parents we care
    // about are the parent class, the declared interfaces (not
    // implemented interfaces), and if traits aren't flattened, the
    // used traits.
    for (auto const [name, cinfo] : index.classInfos) {
      if (cinfo->parent) onParent(cinfo->parent, cinfo);
      for (auto const iface : cinfo->declInterfaces) {
        onParent(iface, cinfo);
      }
      auto const& cls = index.cls(cinfo->name);
      if (!(cls.attrs & AttrNoExpandTrait)) {
        for (auto const trait : cinfo->usedTraits) {
          onParent(trait, cinfo);
        }
      }
    }

    // Use the edges provided to the Job to know the mapping from
    // ClassInfo to split (it cannot be inferred otherwise).
    for (auto const& edge : edges) {
      SCOPE_ASSERT_DETAIL("Edge not present in job") {
        return folly::sformat("{} -> {}", edge.cls, edge.split);
      };
      assertx(index.classInfos.count(edge.cls));
      assertx(index.splits.count(edge.split));
      index.children[edge.cls].emplace(edge.split);
    }

    // Every "top" ClassInfo also has itself as a subclass (this
    // matches the semantics of the subclass list and simplifies the
    // processing).
    for (auto const name : index.top) {
      if (auto const split = folly::get_default(index.splits, name, nullptr)) {
        // Copy the children list out of the split and add it to the
        // map.
        auto& children = index.children[name];
        for (auto const child : split->children) {
          assertx(index.classInfos.count(child) ||
                  index.splits.count(child));
          children.emplace(child);
        }
      } else {
        index.children[name].emplace(name);
      }
    }

    // For every ClassInfo and split, we now know the direct
    // children. Iterate and find all transitive children.
    for (auto& [_, children] : index.children) {
      auto newChildren1 = children;
      ISStringSet newChildren2;

      while (!newChildren1.empty()) {
        newChildren2.clear();
        for (auto const child : newChildren1) {
          auto const it = index.children.find(child);
          // May not exist in index.children if processed in earlier round.
          if (it == end(index.children)) continue;
          for (auto const c : it->second) {
            if (children.count(c)) continue;
            newChildren2.emplace(c);
          }
        }

        std::swap(newChildren1, newChildren2);
        for (auto const child : newChildren1) {
          children.emplace(child);
        }
      }
    }
  }

  static FuncFamily2::StaticInfo static_info_from_meth_meta(
    const FuncFamilyEntry::MethMetadata& meta
  ) {
    FuncFamily2::StaticInfo info;
    info.m_numInOut = meta.m_numInOut;
    info.m_requiredCoeffects = meta.m_requiredCoeffects;
    info.m_coeffectRules = meta.m_coeffectRules;
    info.m_paramPreps = meta.m_prepKinds;
    info.m_minNonVariadicParams = info.m_maxNonVariadicParams =
      meta.m_nonVariadicParams;
    info.m_isReadonlyReturn = yesOrNo(meta.m_isReadonlyReturn);
    info.m_isReadonlyThis = yesOrNo(meta.m_isReadonlyThis);
    info.m_supportsAER = yesOrNo(meta.m_supportsAER);
    info.m_maybeReified = meta.m_isReified;
    info.m_maybeCaresAboutDynCalls = meta.m_caresAboutDyncalls;
    info.m_maybeBuiltin = meta.m_builtin;
    return info;
  }

  // Turn a FuncFamilyEntry into an equivalent Data::MethInfo.
  static Data::MethInfo
  meth_info_from_func_family_entry(LocalIndex& index,
                                   const FuncFamilyEntry& entry) {
    Data::MethInfo info;
    info.complete = !entry.m_allIncomplete;
    info.regularComplete = !entry.m_regularIncomplete;
    info.privateAncestor = entry.m_privateAncestor;

    auto const getFF = [&] (const FuncFamily2::Id& id)
      -> const FuncFamily2& {
      auto const it = index.funcFamilies.find(id);
      always_assert_flog(
        it != end(index.funcFamilies),
        "Tried to access non-existent func-family '{}'",
        id.toString()
      );
      return *it->second;
    };

    match<void>(
      entry.m_meths,
      [&] (const FuncFamilyEntry::BothFF& e) {
        auto const& ff = getFF(e.m_ff);
        info.regularMeths.insert(
          begin(ff.m_regular),
          end(ff.m_regular)
        );
        info.nonRegularPrivateMeths.insert(
          begin(ff.m_nonRegularPrivate),
          end(ff.m_nonRegularPrivate)
        );
        info.nonRegularMeths.insert(
          begin(ff.m_nonRegular),
          end(ff.m_nonRegular)
        );
        assertx(ff.m_allStatic);
        assertx(ff.m_regularStatic);
        info.allStatic = ff.m_allStatic;
        info.regularStatic = ff.m_regularStatic;
      },
      [&] (const FuncFamilyEntry::FFAndSingle& e) {
        auto const& ff = getFF(e.m_ff);
        info.nonRegularMeths.insert(
          begin(ff.m_nonRegular),
          end(ff.m_nonRegular)
        );
        if (e.m_nonRegularPrivate) {
          assertx(ff.m_nonRegularPrivate.size() == 1);
          assertx(ff.m_nonRegularPrivate[0] == e.m_regular);
          info.nonRegularPrivateMeths.emplace(e.m_regular);
        } else {
          assertx(ff.m_regular.size() == 1);
          assertx(ff.m_regular[0] == e.m_regular);
          info.regularMeths.emplace(e.m_regular);
        }
        assertx(ff.m_allStatic);
        assertx(ff.m_regularStatic);
        info.allStatic = ff.m_allStatic;
        info.regularStatic = ff.m_regularStatic;
      },
      [&] (const FuncFamilyEntry::FFAndNone& e) {
        auto const& ff = getFF(e.m_ff);
        assertx(ff.m_regular.empty());
        info.nonRegularMeths.insert(
          begin(ff.m_nonRegular),
          end(ff.m_nonRegular)
        );
        assertx(ff.m_allStatic);
        assertx(!ff.m_regularStatic);
        info.allStatic = ff.m_allStatic;
      },
      [&] (const FuncFamilyEntry::BothSingle& e) {
        if (e.m_nonRegularPrivate) {
          info.nonRegularPrivateMeths.emplace(e.m_all);
        } else {
          info.regularMeths.emplace(e.m_all);
        }
        info.allStatic = info.regularStatic =
          static_info_from_meth_meta(e.m_meta);
      },
      [&] (const FuncFamilyEntry::SingleAndNone& e) {
        info.nonRegularMeths.emplace(e.m_all);
        info.allStatic = static_info_from_meth_meta(e.m_meta);
      },
      [&] (const FuncFamilyEntry::None&) {
        assertx(!info.complete);
      }
    );

    return info;
  }

  // Create a Data representing the single ClassInfo or split with the
  // name "clsname".
  static Data build_data(LocalIndex& index, SString clsname, SString top) {
    // Does this name represent a class?
    if (auto const cinfo =
        folly::get_default(index.classInfos, clsname, nullptr)) {
      // It's a class. We need to build a Data from what's in the
      // ClassInfo. If the ClassInfo hasn't been processed already
      // (it's a leaf or its the first round), the data will reflect
      // just that class. However if the ClassInfo has been processed
      // (it's a dependencies and it's past the first round), it will
      // reflect any subclasses of that ClassInfo as well.
      Data data;

      assertx(!cinfo->subclassList.empty());
      data.subclasses.insert(
        begin(cinfo->subclassList),
        end(cinfo->subclassList)
      );

      if (cinfo->subImplInterfaces) {
        data.implInterfaces.insert(
          begin(*cinfo->subImplInterfaces),
          end(*cinfo->subImplInterfaces)
        );
      } else {
        data.implInterfaces.insert(
          begin(cinfo->implInterfaces),
          end(cinfo->implInterfaces)
        );
      }

      // Use the method family table to build initial MethInfos (if
      // the ClassInfo hasn't been processed this will be empty).
      for (auto const& [name, entry] : cinfo->methodFamilies) {
        data.methods.emplace(
          name,
          meth_info_from_func_family_entry(index, entry)
        );
      }

      auto const& cls = index.cls(cinfo->name);

      if (debug) {
        if (cls.attrs &
            (AttrInterface | AttrTrait | AttrEnum | AttrEnumClass)) {
          always_assert(cinfo->subImplInterfaces.has_value());
        }

        for (auto const& [name, mte] : cinfo->methods) {
          if (is_special_method_name(name)) continue;

          // Every method should have a methodFamilies entry. If this
          // method is AttrNoOverride, it shouldn't have a FuncFamily
          // associated with it.
          auto const it = cinfo->methodFamilies.find(name);
          always_assert(it != end(cinfo->methodFamilies));

          if (mte.attrs & AttrNoOverride) {
            always_assert(
              boost::get<FuncFamilyEntry::BothSingle>(&it->second.m_meths) ||
              boost::get<FuncFamilyEntry::SingleAndNone>(&it->second.m_meths)
            );
          }
        }
      }

      // Create a MethInfo for any missing methods as well.
      for (auto const name : cinfo->missingMethods) {
        assertx(!cinfo->methods.count(name));
        if (data.methods.count(name)) continue;
        // The MethInfo will be empty, and be marked as incomplete.
        auto& info = data.methods[name];
        info.complete = false;
        if (cinfo->isRegularClass) info.regularComplete = false;
      }

      data.hasConstProp = cinfo->subHasConstProp;
      data.hasReifiedGeneric = cinfo->subHasReifiedGeneric;

      // If this is a mock class, any direct parent of this class
      // should be marked as mocked.
      if (cinfo->isMockClass) {
        if (cinfo->parent) data.mockedClasses.emplace(cinfo->parent);
        for (auto const iface : cinfo->declInterfaces) {
          data.mockedClasses.emplace(iface);
        }
        if (!(cls.attrs & AttrNoExpandTrait)) {
          for (auto const trait : cinfo->usedTraits) {
            data.mockedClasses.emplace(trait);
          }
        }
      }
      data.isSubMocked = cinfo->isMocked || cinfo->isSubMocked;

      data.hasRegularClass =
        cinfo->hasRegularSubclass || cinfo->isRegularClass;
      data.hasNonRegularClass =
        cinfo->hasNonRegularSubclass || !cinfo->isRegularClass;

      // If this class is the one we're ultimately calculating results
      // for, ignore it for the purposes of these booleans. They only
      // reflect subclasses and not the class itself.
      if (clsname->isame(top)) {
        data.hasRegularSubclass = cinfo->hasRegularSubclass;
        data.hasNonRegularSubclass = cinfo->hasNonRegularSubclass;
      } else {
        data.hasRegularSubclass = data.hasRegularClass;
        data.hasNonRegularSubclass = data.hasNonRegularClass;
      }

      for (auto const& prop : cls.properties) {
        if (!(prop.attrs & (AttrStatic|AttrPrivate|AttrNoImplicitNullable))) {
          data.propsWithImplicitNullable.emplace(prop.name);
        }
      }

      return data;
    }

    // It doesn't represent a class. It should represent a
    // split.

    // A split cannot be both a root and a dependency due to how we
    // set up the buckets.
    assertx(!index.top.count(clsname));
    auto const split = index.splits.at(clsname);
    assertx(!split->data.subclasses.empty());
    assertx(split->data.hasRegularClass || split->data.hasNonRegularClass);
    // Split already contains the Data, so nothing to do but return
    // it.
    return split->data;
  }

  // Obtain a Data for the given class/split named "top".
  static Data aggregate_data(LocalIndex& index, SString top) {
    Data data;

    auto const& children = [&] () -> const ISStringSet& {
      auto const it = index.children.find(top);
      always_assert(it != end(index.children));
      return it->second;
    }();
    assertx(!children.empty());

    // For each child of the class/split (for classes this includes
    // the top class itself), we create a Data, then union it together
    // with the rest.
    auto first = true;
    for (auto const child : children) {
      auto childData = build_data(index, child, top);

      // The first Data has nothing to union with, so just use it as
      // is.
      if (first) {
        data = std::move(childData);
        first = false;
        continue;
      }

      data.subclasses.insert(
        begin(childData.subclasses),
        end(childData.subclasses)
      );

      data.implInterfaces.insert(
        begin(childData.implInterfaces),
        end(childData.implInterfaces)
      );

      // Combine MethInfos for each method name:
      folly::erase_if(
        data.methods,
        [&] (std::pair<const SString, Data::MethInfo>& p) {
          auto const name = p.first;
          auto& info = p.second;

          if (auto const childInfo =
              folly::get_ptr(childData.methods, name)) {
            // There's a MethInfo with that name in the
            // child. "Promote" the MethRefs if they're in a superior
            // status in the child.
            for (auto const& meth : childInfo->regularMeths) {
              if (info.regularMeths.count(meth)) continue;
              info.regularMeths.emplace(meth);
              info.nonRegularPrivateMeths.erase(meth);
              info.nonRegularMeths.erase(meth);
            }
            for (auto const& meth : childInfo->nonRegularPrivateMeths) {
              if (info.regularMeths.count(meth) ||
                  info.nonRegularPrivateMeths.count(meth)) {
                continue;
              }
              info.nonRegularPrivateMeths.emplace(meth);
              info.nonRegularMeths.erase(meth);
            }
            for (auto const& meth : childInfo->nonRegularMeths) {
              if (info.regularMeths.count(meth) ||
                  info.nonRegularPrivateMeths.count(meth) ||
                  info.nonRegularMeths.count(meth)) {
                continue;
              }
              info.nonRegularMeths.emplace(meth);
            }
            info.complete &= childInfo->complete;
            if (childData.hasRegularClass) {
              info.regularComplete &= childInfo->regularComplete;
              info.privateAncestor |= childInfo->privateAncestor;
            } else {
              assertx(childInfo->regularComplete);
              assertx(!childInfo->privateAncestor);
            }

            if (childInfo->allStatic) {
              if (!info.allStatic) {
                info.allStatic = std::move(*childInfo->allStatic);
              } else {
                *info.allStatic |= *childInfo->allStatic;
              }
            }
            if (childInfo->regularStatic) {
              if (!info.regularStatic) {
                info.regularStatic = std::move(*childInfo->regularStatic);
              } else {
                *info.regularStatic |= *childInfo->regularStatic;
              }
            }

            return false;
          }

          // There's no MethInfo with that name in the child. We might
          // still want to keep the MethInfo because it will be needed
          // for expanding abstract class/interface method
          // families. If the child has a regular class, we can remove
          // it (it won't be part of the expansion).
          return
            childData.hasRegularClass ||
            !info.regularComplete ||
            info.privateAncestor ||
            is_special_method_name(name) ||
            name == s_construct.get();
        }
      );

      // Since we drop non-matching method names only if the class has
      // a regular class, it introduces an ordering dependency. If the
      // first class we encounter has a regular class, everything
      // works fine. However, if the first class we encounter does not
      // have a regular class, the Data will have its methods. If we
      // eventually process a class which does have a regular class,
      // we'll never process it's non-matching methods (because we
      // iterate over data.methods). They won't end up in data.methods
      // whereas they would if a class with regular class was
      // processed first. Detect this condition and manually add such
      // methods to data.methods.
      if (!data.hasRegularClass && childData.hasRegularClass) {
        for (auto& [name, info] : childData.methods) {
          if (!info.regularComplete || info.privateAncestor) continue;
          if (is_special_method_name(name)) continue;
          if (name == s_construct.get()) continue;
          if (data.methods.count(name)) continue;
          auto& newInfo = data.methods[name];
          newInfo.regularMeths = std::move(info.regularMeths);
          newInfo.nonRegularPrivateMeths =
            std::move(info.nonRegularPrivateMeths);
          newInfo.nonRegularMeths = std::move(info.nonRegularMeths);
          newInfo.allStatic = std::move(info.allStatic);
          newInfo.regularStatic = std::move(info.regularStatic);
          newInfo.complete = false;
          newInfo.regularComplete = true;
          newInfo.privateAncestor = false;
        }
      }

      data.propsWithImplicitNullable.insert(
        begin(childData.propsWithImplicitNullable),
        end(childData.propsWithImplicitNullable)
      );

      data.mockedClasses.insert(
        begin(childData.mockedClasses),
        end(childData.mockedClasses)
      );

      // The rest are booleans which can just be unioned together.
      data.hasConstProp |= childData.hasConstProp;
      data.hasReifiedGeneric |= childData.hasReifiedGeneric;
      data.isSubMocked |= childData.isSubMocked;
      data.hasRegularClass |= childData.hasRegularClass;
      data.hasNonRegularClass |= childData.hasNonRegularClass;
      data.hasRegularSubclass |= childData.hasRegularSubclass;
      data.hasNonRegularSubclass |= childData.hasNonRegularSubclass;
    }

    return data;
  }

  // Create (or re-use an existing) FuncFamily for the given MethInfo.
  static FuncFamily2::Id make_func_family(
    LocalIndex& index,
    SString name,
    Data::MethInfo info
  ) {
    // We should have more than one method because otherwise we
    // shouldn't be trying to create a FuncFamily for it.
    assertx(
      info.regularMeths.size() +
      info.nonRegularPrivateMeths.size() +
      info.nonRegularMeths.size() > 1
    );

    // Before doing the expensive sorting and hashing, see if this
    // FuncFamily already exists. If so, just return the id.
    if (auto const id = folly::get_ptr(
        index.funcFamilyCache,
        MethInfoTupleProxy{
          &info.regularMeths,
          &info.nonRegularPrivateMeths,
          &info.nonRegularMeths
        }
      )) {
      return *id;
    }

    // Nothing in the cache. We need to do the expensive step of
    // actually creating the FuncFamily.

    // First sort the methods so they're in deterministic order.
    std::vector<MethRef> regular{
      begin(info.regularMeths), end(info.regularMeths)
    };
    std::vector<MethRef> nonRegularPrivate{
      begin(info.nonRegularPrivateMeths), end(info.nonRegularPrivateMeths)
    };
    std::vector<MethRef> nonRegular{
      begin(info.nonRegularMeths), end(info.nonRegularMeths)
    };
    std::sort(begin(regular), end(regular));
    std::sort(begin(nonRegularPrivate), end(nonRegularPrivate));
    std::sort(begin(nonRegular), end(nonRegular));

    // Create the id by hashing the methods:
    SHA1Hasher hasher;
    {
      auto const size1 = regular.size();
      auto const size2 = nonRegularPrivate.size();
      auto const size3 = nonRegular.size();
      hasher.update((const char*)&size1, sizeof(size1));
      hasher.update((const char*)&size2, sizeof(size2));
      hasher.update((const char*)&size3, sizeof(size3));
    }
    for (auto const& m : regular) {
      hasher.update(m.cls->data(), m.cls->size());
      hasher.update((const char*)&m.idx, sizeof(m.idx));
    }
    for (auto const& m : nonRegularPrivate) {
      hasher.update(m.cls->data(), m.cls->size());
      hasher.update((const char*)&m.idx, sizeof(m.idx));
    }
    for (auto const& m : nonRegular) {
      hasher.update(m.cls->data(), m.cls->size());
      hasher.update((const char*)&m.idx, sizeof(m.idx));
    }
    auto const id = hasher.finish();

    // See if this id exists already. If so, record it in the cache
    // and we're done.
    if (index.funcFamilies.count(id)) {
      index.funcFamilyCache.emplace(
        MethInfoTuple{
          std::move(info.regularMeths),
          std::move(info.nonRegularPrivateMeths),
          std::move(info.nonRegularMeths)
        },
        id
      );
      return id;
    }

    // It's a new id. Create the actual FuncFamily:

    regular.shrink_to_fit();
    nonRegularPrivate.shrink_to_fit();
    nonRegular.shrink_to_fit();

    auto ff = std::make_unique<FuncFamily2>();
    ff->m_id = id;
    ff->m_name = name;
    ff->m_regular = std::move(regular);
    ff->m_nonRegularPrivate = std::move(nonRegularPrivate);
    ff->m_nonRegular = std::move(nonRegular);
    ff->m_allStatic = std::move(info.allStatic);
    ff->m_regularStatic = std::move(info.regularStatic);

    always_assert(
      index.funcFamilies.emplace(id, std::move(ff)).second
    );
    index.newFuncFamilies.emplace_back(id);
    index.funcFamilyCache.emplace(
      MethInfoTuple{
        std::move(info.regularMeths),
        std::move(info.nonRegularPrivateMeths),
        std::move(info.nonRegularMeths)
      },
      id
    );

    return id;
  }

  // Turn a FuncFamily::StaticInfo into an equivalent
  // FuncFamilyEntry::MethMetadata. The StaticInfo must be valid for a
  // single method.
  static FuncFamilyEntry::MethMetadata single_meth_meta_from_static_info(
    const FuncFamily2::StaticInfo& info
  ) {
    assertx(info.m_numInOut);
    assertx(info.m_requiredCoeffects);
    assertx(info.m_coeffectRules);
    assertx(info.m_minNonVariadicParams == info.m_maxNonVariadicParams);
    assertx(info.m_isReadonlyReturn != TriBool::Maybe);
    assertx(info.m_isReadonlyThis != TriBool::Maybe);
    assertx(info.m_supportsAER != TriBool::Maybe);

    FuncFamilyEntry::MethMetadata meta;
    meta.m_prepKinds = info.m_paramPreps;
    meta.m_coeffectRules = *info.m_coeffectRules;
    meta.m_numInOut = *info.m_numInOut;
    meta.m_requiredCoeffects = *info.m_requiredCoeffects;
    meta.m_nonVariadicParams = info.m_minNonVariadicParams;
    meta.m_isReadonlyReturn = info.m_isReadonlyReturn == TriBool::Yes;
    meta.m_isReadonlyThis = info.m_isReadonlyThis == TriBool::Yes;
    meta.m_supportsAER = info.m_supportsAER == TriBool::Yes;
    meta.m_isReified = info.m_maybeReified;
    meta.m_caresAboutDyncalls = info.m_maybeCaresAboutDynCalls;
    meta.m_builtin = info.m_maybeBuiltin;
    return meta;
  }

  // Translate a MethInfo into the appropriate FuncFamilyEntry
  static FuncFamilyEntry make_method_family_entry(
    LocalIndex& index,
    SString name,
    Data::MethInfo info
  ) {
    FuncFamilyEntry entry;
    entry.m_allIncomplete = !info.complete;
    entry.m_regularIncomplete = !info.regularComplete;
    entry.m_privateAncestor = info.privateAncestor;

    if (info.regularMeths.size() + info.nonRegularPrivateMeths.size() > 1) {
      // There's either multiple regularMeths, multiple
      // nonRegularPrivateMeths, or one of each (remember they are
      // disjoint). In either case, there's more than one method, so
      // we need a func family.
      assertx(info.allStatic);
      assertx(info.regularStatic);
      auto const ff = make_func_family(index, name, std::move(info));
      entry.m_meths = FuncFamilyEntry::BothFF{ff};
    } else if (!info.regularMeths.empty() ||
               !info.nonRegularPrivateMeths.empty()) {
      // We know their sum isn't greater than one, so only one of them
      // can be non-empty (and the one that is has only a single
      // method).
      assertx(info.allStatic);
      assertx(info.regularStatic);
      auto const r = !info.regularMeths.empty()
        ? *begin(info.regularMeths)
        : *begin(info.nonRegularPrivateMeths);
      if (info.nonRegularMeths.empty()) {
        // There's only one method and it covers both variants.
        entry.m_meths = FuncFamilyEntry::BothSingle{
          r,
          single_meth_meta_from_static_info(*info.allStatic),
          info.regularMeths.empty()
        };
      } else {
        // nonRegularMeths is non-empty. Since the MethRefSets are
        // disjoint, overall there's more than one method so need a
        // func family.
        auto const nonRegularPrivate = info.regularMeths.empty();
        auto const ff = make_func_family(index, name, std::move(info));
        entry.m_meths = FuncFamilyEntry::FFAndSingle{ff, r, nonRegularPrivate};
      }
    } else if (info.nonRegularMeths.size() > 1) {
      // Both regularMeths and nonRegularPrivateMeths is empty. If
      // there's multiple nonRegularMeths, we need a func family for
      // the non-regular variant, but the regular variant is empty.
      assertx(info.allStatic);
      assertx(!info.regularStatic);
      auto const ff = make_func_family(index, name, std::move(info));
      entry.m_meths = FuncFamilyEntry::FFAndNone{ff};
    } else if (!info.nonRegularMeths.empty()) {
      // There's exactly one nonRegularMeths method (and nothing for
      // the regular variant).
      assertx(info.allStatic);
      assertx(!info.regularStatic);
      entry.m_meths = FuncFamilyEntry::SingleAndNone{
        *begin(info.nonRegularMeths),
        single_meth_meta_from_static_info(*info.allStatic)
      };
    } else {
      // No methods at all
      assertx(!info.complete);
      assertx(!info.allStatic);
      assertx(!info.regularStatic);
      entry.m_meths = FuncFamilyEntry::None{};
    }

    return entry;
  }

  // Calculate the data for each root (those which will we'll provide
  // outputs for) and update the ClassInfo or Split as appropriate.
  static void process_roots(
    LocalIndex& index,
    const std::vector<std::unique_ptr<ClassInfo2>>& roots,
    const std::vector<std::unique_ptr<Split>>& splits
  ) {
    for (auto const& cinfo : roots) {
      assertx(index.top.count(cinfo->name));
      // Process the children of this class and build a unified Data
      // for it.
      auto data = aggregate_data(index, cinfo->name);

      auto& cls = index.cls(cinfo->name);

      // These are just copied directly from Data.
      cinfo->subHasConstProp = data.hasConstProp;
      cinfo->subHasReifiedGeneric = data.hasReifiedGeneric;
      cinfo->hasRegularSubclass = data.hasRegularSubclass;
      cinfo->hasNonRegularSubclass = data.hasNonRegularSubclass;

      // This class is mocked if its on the mocked classes list.
      cinfo->isMocked = data.mockedClasses.count(cinfo->name) > 0;
      cinfo->isSubMocked = data.isSubMocked || cinfo->isMocked;
      attribute_setter(cls.attrs, !cinfo->isSubMocked, AttrNoMock);

      // We can use whether we saw regular/non-regular subclasses to
      // infer if this class is overridden.
      if (cinfo->hasRegularSubclass) {
        attribute_setter(cls.attrs, false, AttrNoOverrideRegular);
        attribute_setter(cls.attrs, false, AttrNoOverride);
      } else if (cinfo->hasNonRegularSubclass) {
        attribute_setter(cls.attrs, true, AttrNoOverrideRegular);
        attribute_setter(cls.attrs, false, AttrNoOverride);
      } else {
        attribute_setter(cls.attrs, true, AttrNoOverrideRegular);
        attribute_setter(cls.attrs, true, AttrNoOverride);
      }

      assertx(
        IMPLIES(
          cinfo->initialNoReifiedInit,
          cls.attrs & AttrNoReifiedInit
        )
      );

      attribute_setter(
        cls.attrs,
        cinfo->initialNoReifiedInit ||
        (!cinfo->parent &&
         (!data.hasReifiedGeneric || (cls.attrs & AttrInterface))),
        AttrNoReifiedInit
      );

      for (auto& [name, mte] : cinfo->methods) {
        if (is_special_method_name(name)) continue;

        // Since this is the first time we're processing this class,
        // all of the methods should be marked as AttrNoOverride.
        assertx(mte.attrs & AttrNoOverride);
        assertx(mte.noOverrideRegular());

        auto& info = [&, name=name] () -> Data::MethInfo& {
          auto it = data.methods.find(name);
          always_assert(it != end(data.methods));
          return it->second;
        }();

        auto const meth = mte.meth();

        // Is this method overridden?
        auto const noOverride = [&] {
          // An incomplete method family is always overridden because
          // the call could fail.
          if (!info.complete) return false;
          // If more than one method then no.
          if (info.regularMeths.size() +
              info.nonRegularPrivateMeths.size() +
              info.nonRegularMeths.size() > 1) {
            return false;
          }
          // NB: All of the below checks all return true. The
          // different conditions are just for checking the right
          // invariants.
          if (info.regularMeths.empty()) {
            // The (single) method isn't on a regular class. This
            // class shouldn't have any regular classes (the set is
            // complete so if we did, the method would have been on
            // it). The (single) method must be on nonRegularMeths or
            // nonRegularPrivateMeths.
            assertx(!cinfo->isRegularClass);
            if (info.nonRegularPrivateMeths.empty()) {
              assertx(info.nonRegularMeths.count(meth));
              return true;
            }
            assertx(info.nonRegularMeths.empty());
            assertx(info.nonRegularPrivateMeths.count(meth));
            return true;
          }
          assertx(info.nonRegularPrivateMeths.empty());
          assertx(info.nonRegularMeths.empty());
          assertx(info.regularMeths.count(meth));
          return true;
        };

        // Is this method overridden in a regular class? (weaker
        // condition)
        auto const noOverrideRegular = [&] {
          // An incomplete method family is always overridden because
          // the call could fail.
          if (!info.regularComplete) return false;
          // If more than one method then no. For the purposes of this
          // check, non-regular but private methods are included.
          if (info.regularMeths.size() +
              info.nonRegularPrivateMeths.size() > 1) {
            return false;
          }
          if (info.regularMeths.empty()) {
            // The method isn't on a regular class. Like in
            // noOverride(), the class shouldn't have any regular
            // classes. If nonRegularPrivateMethos is empty, this
            // means any possible override is non-regular, so we're
            // good.
            assertx(!cinfo->isRegularClass);
            if (info.nonRegularPrivateMeths.empty()) return true;
            return (bool)info.nonRegularPrivateMeths.count(meth);
          }
          if (cinfo->isRegularClass) {
            // If this class is regular, the method on this class
            // should be marked as regular.
            assertx(info.regularMeths.count(meth));
            return true;
          }
          // We know regularMeths is non-empty, and the size is at
          // most one. If this method is the (only) one in
          // regularMeths, it's not overridden by anything.
          return (bool)info.regularMeths.count(meth);
        };

        if (!noOverrideRegular()) {
          mte.clearNoOverrideRegular();
          attribute_setter(mte.attrs, false, AttrNoOverride);
        } else if (!noOverride()) {
          attribute_setter(mte.attrs, false, AttrNoOverride);
        }

        auto& entry = cinfo->methodFamilies.at(name);
        assertx(
          boost::get<FuncFamilyEntry::BothSingle>(&entry.m_meths) ||
          boost::get<FuncFamilyEntry::SingleAndNone>(&entry.m_meths)
        );

        if (debug) {
          if (mte.attrs & AttrNoOverride) {
            always_assert(info.complete);
            always_assert(info.regularComplete);

            if (cinfo->isRegularClass || cinfo->hasRegularSubclass) {
              always_assert(info.regularMeths.size() == 1);
              always_assert(info.regularMeths.count(meth));
              always_assert(info.nonRegularPrivateMeths.empty());
              always_assert(info.nonRegularMeths.empty());
            } else {
              // If this class isn't regular, it could still have a
              // regular method which it inherited from a (regular)
              // parent. There should only be one method across all the
              // sets though.
              always_assert(
                info.regularMeths.size() +
                info.nonRegularPrivateMeths.size() +
                info.nonRegularMeths.size() == 1
              );
              always_assert(
                info.regularMeths.count(meth) ||
                info.nonRegularPrivateMeths.count(meth) ||
                info.nonRegularMeths.count(meth)
              );
            }

            if (mte.hasPrivateAncestor() &&
                (cinfo->isRegularClass || cinfo->hasRegularSubclass)) {
              always_assert(info.privateAncestor);
            } else {
              always_assert(!info.privateAncestor);
            }
          } else {
            always_assert(!(cls.attrs & AttrNoOverride));
          }
        }

        // NB: Even if the method is AttrNoOverride, we might need to
        // change the FuncFamilyEntry. This class could be non-regular
        // and a child class could be regular. Even if the child class
        // doesn't override the method, it changes it from non-regular
        // to regular.
        entry = make_method_family_entry(index, name, std::move(info));

        if (mte.attrs & AttrNoOverride) {
          // However, even if the entry changes with AttrNoOverride,
          // it can only be these two cases.
          always_assert(
            boost::get<FuncFamilyEntry::BothSingle>(&entry.m_meths) ||
            boost::get<FuncFamilyEntry::SingleAndNone>(&entry.m_meths)
          );
        }
      }

      /*
       * Interfaces can cause monotonicity violations. Suppose we have two
       * interfaces: I2 and I2. I1 declares a method named Foo. Every
       * class which implements I2 also implements I1 (therefore I2
       * implies I1). During analysis, a type is initially Obj<=I1 and we
       * resolve a call to Foo using I1's func families. After further
       * optimization, we narrow the type to Obj<=I2. Now when we go to
       * resolve a call to Foo using I2's func families, we find
       * nothing. Foo is declared in I1, not in I2, and interface methods
       * are not inherited. We use the fall back name-only tables, which
       * might give us a worse type than before. This is a monotonicity
       * violation because refining the object type gave us worse
       * analysis.
       *
       * To avoid this, we expand an interface's (and abstract class'
       * which has similar issues) func families to include all methods
       * defined by *all* of it's (regular) implementations. So, in the
       * example above, we'd expand I2's func families to include Foo,
       * since all of I2's implements should define a Foo method (since
       * they also all implement I1).
       *
       * Any MethInfos which are part of the abstract class/interface
       * method table has already been processed above. Any ones which
       * haven't are candidates for the above expansion and must also
       * be placed in the method families table. Note: we do not just
       * restrict this to just abstract classes or interfaces. This
       * class may be a child of an abstract class or interfaces and
       * we need to propagate these "expanded" methods so they're
       * available in the dependency when we actually process the
       * abstract class/interface in a later round.
       */
      for (auto& [name, info] : data.methods) {
        if (cinfo->methods.count(name)) continue;
        assertx(!is_special_method_name(name));
        auto entry = make_method_family_entry(index, name, std::move(info));
        always_assert(
          cinfo->methodFamilies.emplace(name, std::move(entry)).second
        );
      }

      // Flatten classes sets up the subclass list to just contain this
      // class (as if it was a leaf). Since we're updating this class
      // (and we're the only ones who should be doing so), we're
      // expecting the subclass list to reflect that.
      assertx(cinfo->subclassList.size() == 1);
      assertx(cinfo->subclassList[0]->isame(cinfo->name));

      // Update the list with what we computed. We need to sort the
      // list to maintain a deterministic order.
      cinfo->subclassList.clear();
      cinfo->subclassList.reserve(data.subclasses.size());
      for (auto const s : data.subclasses) {
        cinfo->subclassList.emplace_back(s);
      }
      std::sort(
        begin(cinfo->subclassList),
        end(cinfo->subclassList),
        string_data_lti{}
      );
      assertx(!cinfo->subclassList.empty());

      assertx(
        IMPLIES(cinfo->subImplInterfaces, cinfo->subImplInterfaces->empty())
      );
      if (cinfo->subImplInterfaces ||
          cinfo->implInterfaces != data.implInterfaces) {
        cinfo->subImplInterfaces = std::move(data.implInterfaces);
      }

      for (auto& prop : cls.properties) {
        if (bool(prop.attrs & AttrNoImplicitNullable) &&
            !(prop.attrs & (AttrStatic | AttrPrivate))) {
          attribute_setter(
            prop.attrs,
            !data.propsWithImplicitNullable.count(prop.name),
            AttrNoImplicitNullable
          );
        }

        if (!(prop.attrs & AttrSystemInitialValue)) continue;
        if (prop.val.m_type == KindOfUninit) {
          assertx(prop.attrs & AttrLateInit);
          continue;
        }

        prop.val = [&] {
          if (!(prop.attrs & AttrNoImplicitNullable)) {
            return make_tv<KindOfNull>();
          }
          // Give the 86reified_prop a special default value to
          // avoid pessimizing the inferred type (we want it to
          // always be a vec of a specific size).
          if (prop.name == s_86reified_prop.get()) {
            return get_default_value_of_reified_list(cls.userAttributes);
          }
          return prop.typeConstraint.defaultValue();
        }();
      }
    }

    // Splits just store the data directly. Since this split hasn't
    // been processed yet (and no other job should process it), all of
    // the fields should be their default settings.
    for (auto& split : splits) {
      assertx(index.top.count(split->name));
      assertx(!split->children.empty());
      split->data = aggregate_data(index, split->name);
    }
  }
};

/*
 * BuildSubclassListJob produces name-only func family entries. This
 * job merges entries for the same name into one.
 */
struct AggregateNameOnlyJob: public BuildSubclassListJob {
  static std::string name() { return "hhbbc-aggregate-name-only"; }

  struct OutputMeta {
    std::vector<std::vector<FuncFamily2::Id>> newFuncFamilyIds;
    std::vector<FuncFamilyEntry> nameOnly;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(newFuncFamilyIds)
        (nameOnly)
        ;
    }
  };
  using Output = Multi<
    Variadic<FuncFamilyGroup>,
    OutputMeta
  >;

  static Output
  run(std::vector<std::pair<SString, std::vector<FuncFamilyEntry>>> allEntries,
      Variadic<FuncFamilyGroup> funcFamilies) {
    LocalIndex index;

    for (auto& group : funcFamilies.vals) {
      for (auto& ff : group.m_ffs) {
        auto const id = ff->m_id;
        // We could have multiple groups which contain the same
        // FuncFamily, so don't assert uniqueness here. We'll just
        // take the first one we see (they should all be equivalent).
        index.funcFamilies.emplace(id, std::move(ff));
      }
    }

    OutputMeta meta;

    for (auto const& [name, entries] : allEntries) {
      Data::MethInfo info;
      info.complete = false;
      info.regularComplete = false;

      for (auto const& entry : entries) {
        auto entryInfo = meth_info_from_func_family_entry(index, entry);
        for (auto const& meth : entryInfo.regularMeths) {
          if (info.regularMeths.count(meth)) continue;
          info.regularMeths.emplace(meth);
          info.nonRegularPrivateMeths.erase(meth);
          info.nonRegularMeths.erase(meth);
        }
        for (auto const& meth : entryInfo.nonRegularPrivateMeths) {
          if (info.regularMeths.count(meth) ||
              info.nonRegularPrivateMeths.count(meth)) {
            continue;
          }
          info.nonRegularPrivateMeths.emplace(meth);
          info.nonRegularMeths.erase(meth);
        }
        for (auto const& meth : entryInfo.nonRegularMeths) {
          if (info.regularMeths.count(meth) ||
              info.nonRegularPrivateMeths.count(meth) ||
              info.nonRegularMeths.count(meth)) {
            continue;
          }
          info.nonRegularMeths.emplace(meth);
        }

        if (entryInfo.allStatic) {
          if (!info.allStatic) {
            info.allStatic = std::move(*entryInfo.allStatic);
          } else {
            *info.allStatic |= *entryInfo.allStatic;
          }
        }
        if (entryInfo.regularStatic) {
          if (!info.regularStatic) {
            info.regularStatic = std::move(*entryInfo.regularStatic);
          } else {
            *info.regularStatic |= *entryInfo.regularStatic;
          }
        }
      }

      meta.nameOnly.emplace_back(
        make_method_family_entry(index, name, std::move(info))
      );
    }

    Variadic<FuncFamilyGroup> funcFamilyGroups;
    group_func_families(index, funcFamilyGroups.vals, meta.newFuncFamilyIds);

    return std::make_tuple(
      std::move(funcFamilyGroups),
      std::move(meta)
    );
  }
};

Job<BuildSubclassListJob> s_buildSubclassJob;
Job<AggregateNameOnlyJob> s_aggregateNameOnlyJob;

struct SubclassWork {
  ISStringToOneT<std::unique_ptr<BuildSubclassListJob::Split>> allSplits;
  struct Bucket {
    std::vector<SString> classes;
    std::vector<SString> deps;
    std::vector<SString> splits;
    std::vector<SString> splitDeps;
    std::vector<SString> leafs;
    std::vector<BuildSubclassListJob::EdgeToSplit> edges;
  };
  std::vector<std::vector<Bucket>> buckets;
  ISStringSet leafInterfaces;
};

/*
 * Algorithm for assigning work for building subclass lists:
 *
 * - Keep track of which classes have been processed and which ones
 *   have not yet been.
 *
 * - Keep looping until all classes have been processed. Each round of
 *   the algorithm becomes a round of output.
 *
 * - Iterate over all classes which haven't been
 *   processed. Distinguish classes which are eligible for processing
 *   or not. A class is eligible for processing if its transitive
 *   dependencies are below the maximum size.
 *
 * - Non-eligible classes are ignored and will be processed again next
 *   round. However, if the class has more eligible direct children
 *   than the bucket size, the class' children will be turned into
 *   split nodes.
 *
 * - Create split nodes. For each class (who we're splitting), use the
 *   typical consistent hashing algorithm to assign each child to a
 *   split node. Change the class' child list to contain the split
 *   nodes instead of the children (this should shrink it
 *   considerably). Each new split becomes a root.
 *
 * - Assign each eligible class to a bucket. Use
 *   assign_hierachial_work to map each eligible class to a bucket.
 *
 * - Update the processed set. Any class which hasn't been processed
 *   that round should have their dependency set shrunken. Processing
 *   a class makes its dependency set be empty. So if a class wasn't
 *   eligible, it should have a dependency which was. Therefore the
 *   class' transitive dependencies should shrink. It should continue
 *   to shrink until its eventually becomes eligible. The same happens
 *   if the class' children are turned into split nodes. Each N
 *   children is replaced with a single split (with no other
 *   dependencies), so the class' dependencies should shrink. Thus,
 *   the algorithm eventually terminates.
*/

SubclassWork build_subclass_lists_assign(SubclassMetadata subclassMeta) {
  trace_time trace{"build subclass lists assign"};
  trace.ignore_client_stats();

  constexpr size_t kBucketSize = 2000;
  constexpr size_t kMaxBucketSize = 20000;

  SubclassWork out;

  auto const maxClassIdx = subclassMeta.all.size();

  // A processed class/split is considered processed once it's
  // assigned to a bucket in a round. Once considered processed, it
  // will have no dependencies.
  ISStringSet processed;

  // Dependency information for a class or split node.
  struct DepData {
    // Transitive dependencies (children) for this class.
    ISStringSet deps;
    // Any split nodes which are dependencies of this class.
    ISStringSet edges;
    // The number of direct children of this class which will be
    // processed this round.
    size_t processChildren{0};
  };
  ISStringToOneT<std::unique_ptr<DepData>> splitDeps;
  ISStringToOneT<std::unique_ptr<BuildSubclassListJob::Split>> splitPtrs;

  // Keep creating rounds until all of the classes are assigned to a
  // bucket in a round.
  auto toProcess = std::move(subclassMeta.all);
  for (size_t round = 0; !toProcess.empty(); ++round) {
    // If we have this many rounds, something has gone wrong, because
    // it should require an astronomical amount of classes.
    always_assert(round < 10);

    // The dependency information for every class, for just this
    // round. The information is calculated lazily and recursively by
    // findDeps.
    std::vector<LockFreeLazy<DepData>> deps{maxClassIdx};

    auto const findDeps = [&] (SString cls,
                               auto const& self) -> const DepData& {
      // If it's processed, there's implicitly no dependencies
      static DepData empty;
      if (processed.count(cls)) return empty;

      // Look up the metadata for this class. If we don't find any,
      // assume that it's for a split.
      auto const it = subclassMeta.meta.find(cls);
      if (it == end(subclassMeta.meta)) {
        auto const it2 = splitDeps.find(cls);
        always_assert(it2 != end(splitDeps));
        return *it2->second;
      }
      auto const& meta = it->second;
      auto const idx = meta.idx;
      assertx(idx < deps.size());

      // Now that we have the index into the dependency vector, look
      // it up, calculating it if it hasn't been already.
      return deps[idx].get(
        [&] {
          DepData out;
          for (auto const c : meta.children) {
            // At a minimum, we need the immediate deps in order to
            // construct the subclass lists for the parent.
            out.deps.emplace(c);
            if (splitDeps.count(c)) out.edges.emplace(c);
            auto const& childDeps = self(c, self);
            if (childDeps.deps.size() <= kMaxBucketSize) ++out.processChildren;
            out.deps.insert(begin(childDeps.deps), end(childDeps.deps));
          }
          return out;
        }
      );
    };

    auto const depsSize = [&] (SString cls) {
      return findDeps(cls, findDeps).deps.size();
    };
    // If this class' children needs to be split into split nodes this
    // round. This happens if the number of direct children of this
    // class which are eligible for processing exceeds the bucket
    // size.
    auto const willSplitChildren = [&] (SString cls) {
      return findDeps(cls, findDeps).processChildren > kBucketSize;
    };
    // If this class will be processed this round. A class will be
    // processed if it's dependencies are less than the maximum bucket
    // size.
    auto const willProcess = [&] (SString cls) {
      return depsSize(cls) <= kMaxBucketSize;
    };

    // Process every remaining class in parallel and assign an action
    // to each:

    // Do nothing with this class this round
    struct Defer { SString cls; };
    // This class is a leaf. Leafs never need to be processed (since
    // their method families are already correct), but they may be
    // dependencies, and need to be examined to create name-only func
    // families.
    struct Leaf { SString cls; };
    // This class will be processed this round and is a root.
    struct Root { SString cls; };
    // This class' children should be split. The class' child list
    // will be replaced with the new child list and splits created.
    struct Split {
      SString cls;
      std::vector<SString> children;
      struct Data {
        SString name;
        std::unique_ptr<DepData> deps;
        std::unique_ptr<BuildSubclassListJob::Split> ptr;
      };
      std::vector<Data> splits;
    };
    using Action = boost::variant<Defer, Leaf, Root, Split>;

    auto const actions = parallel::map(
      toProcess,
      [&] (SString cls) {
        auto const& meta = subclassMeta.meta.at(cls);
        // If the class has no children, it's a leaf. It never will be
        // a root because it's information is already correct and
        // doesn't need to be updated.
        if (meta.children.empty()) return Action{ Leaf{cls} };

        if (willProcess(cls)) return Action{ Root{cls} };

        // The class isn't eligible and we won't split the
        // children. Nothing to do here but defer it to another
        // round. It's dependency size should shrink and make it
        // eligible.
        if (!willSplitChildren(cls)) return Action{ Defer{cls} };

        // Otherwise we're going to split some/all of this class'
        // children. Once we process those in this round, this class'
        // dependencies should be smaller and be able to be processed.
        Split split;
        split.cls = cls;
        split.splits = [&] {
          // Group all of the eligible children into buckets, and
          // split the buckets to ensure they remain below the maximum
          // size.
          auto const buckets = split_buckets(
            [&] {
              auto const numChildren = findDeps(cls, findDeps).processChildren;
              auto const numBuckets =
                (numChildren + kMaxBucketSize - 1) / kMaxBucketSize;
              assertx(numBuckets > 0);

              std::vector<std::vector<SString>> buckets;
              buckets.resize(numBuckets);
              for (auto const child : meta.children) {
                if (!willProcess(child)) continue;
                auto const idx =
                  consistent_hash(child->hashStatic(), numBuckets);
                assertx(idx < numBuckets);
                buckets[idx].emplace_back(child);
              }

              buckets.erase(
                std::remove_if(
                  begin(buckets),
                  end(buckets),
                  [] (const std::vector<SString>& b) { return b.empty(); }
                ),
                end(buckets)
              );

              return buckets;
            }(),
            kMaxBucketSize,
            [&] (SString child) -> const ISStringSet& {
              return findDeps(child, findDeps).deps;
            }
          );
          // Each bucket corresponds to a new split node, which will
          // contain the results for the children in that bucket.
          auto const numSplits = buckets.size();

          // Actually make the splits and fill their children list.
          std::vector<Split::Data> splits;
          splits.reserve(numSplits);
          for (size_t i = 0; i < numSplits; ++i) {
            // The names of a split node are arbitrary, but most be
            // unique and not collide with any actual classes.
            auto const name = makeStaticString(
              folly::sformat("{}_{}_split;{}", round, i, cls)
            );

            auto deps = std::make_unique<DepData>();
            auto split = std::make_unique<BuildSubclassListJob::Split>(name);

            for (auto const child : buckets[i]) {
              split->children.emplace_back(child);
              auto const& childDeps = findDeps(child, findDeps).deps;
              deps->deps.insert(begin(childDeps), end(childDeps));
              deps->deps.emplace(child);
            }

            std::sort(
              begin(split->children),
              end(split->children),
              string_data_lti{}
            );

            splits.emplace_back(
              Split::Data{name, std::move(deps), std::move(split)}
            );
          }
          return splits;
        }();

        // Create the new children list for this class. The new
        // children list are any children which won't be processed,
        // and the new splits.
        for (auto const child : meta.children) {
          if (willProcess(child)) continue;
          split.children.emplace_back(child);
        }
        for (auto const& [name, _, _2] : split.splits) {
          split.children.emplace_back(name);
        }

        return Action{ std::move(split) };
      }
    );

    assertx(actions.size() == toProcess.size());
    std::vector<SString> roots;
    std::vector<SString> markProcessed;
    ISStringSet leafs;
    roots.reserve(actions.size());
    markProcessed.reserve(actions.size());
    // Clear the list of classes to process. It will be repopulated
    // with any classes marked as Defer.
    toProcess.clear();

    for (auto const& action : actions) {
      match<void>(
        action,
        [&] (Defer d) { toProcess.emplace_back(d.cls); },
        [&] (Leaf l) {
          // Leafs have some special handling. Generally they're just
          // deps, but they need to be examined for name-only
          // entries. We only want this done for a leaf in one
          // specific job (as a dep they can appear in multiple
          // jobs). We'll use the dep promotion machinery to
          // selectively promote these to roots in a certain job.
          toProcess.emplace_back(l.cls);
          leafs.emplace(l.cls);
          auto const& meta = subclassMeta.meta.at(l.cls);
          // If the leaf has no parent either, it will never be a dep,
          // so cannot promote it. Just treat it as a root.
          if (meta.parents.empty()) roots.emplace_back(l.cls);
          if (subclassMeta.interfaces.count(l.cls)) {
            out.leafInterfaces.emplace(l.cls);
          }
        },
        [&] (Root r) {
          roots.emplace_back(r.cls);
          markProcessed.emplace_back(r.cls);
        },
        [&] (const Split& s) {
          auto& meta = subclassMeta.meta.at(s.cls);
          meta.children = s.children;
          toProcess.emplace_back(s.cls);
          auto& splits = const_cast<std::vector<Split::Data>&>(s.splits);
          for (auto& [name, deps, ptr] : splits) {
            roots.emplace_back(name);
            markProcessed.emplace_back(name);
            splitDeps.emplace(name, std::move(deps));
            splitPtrs.emplace(name, std::move(ptr));
          }
        }
      );
    }

    // We have a root set now. So use assign_hierarchical_work to turn
    // it into a set of buckets:

    auto const bucketSize = [&] {
      // Anything other than the first round tends to have a very
      // small amount of items (but are relatively expensive). In that
      // case, it's faster to try to assign everything to its own
      // bucket.
      if (out.buckets.empty() || roots.size() > kBucketSize) {
        return kBucketSize;
      }
      return 1ul;
    }();

    auto const work = assign_hierarchical_work(
      roots,
      maxClassIdx,
      bucketSize,
      kMaxBucketSize,
      [&] (SString c) {
        auto const& deps = findDeps(c, findDeps).deps;
        // Everything at this point is always instantiable.
        return std::make_pair(&deps, true);
      },
      // Called to get the class index for promotion from a dependency
      // to an output.
      [&] (SString c) -> Optional<size_t> {
        // We only promote leafs
        if (!leafs.count(c)) return std::nullopt;
        return subclassMeta.meta.at(c).idx;
      }
    );

    // The output of assign_hierarchical_work is just buckets with the
    // names. We need to map those to classes or edge nodes and put
    // them in the correct data structure in the output. If there's a
    // class dependency on a split node, we also need to record an
    // edge between them.
    auto const add = [&] (SString cls, auto& clsList,
                          auto& splitList, auto& edgeList) {
      auto const it = splitPtrs.find(cls);
      if (it == end(splitPtrs)) {
        clsList.emplace_back(cls);
        for (auto const s : findDeps(cls, findDeps).edges) {
          edgeList.emplace_back(BuildSubclassListJob::EdgeToSplit{cls, s});
        }
      } else {
        splitList.emplace_back(it->second->name);
      }
    };

    out.buckets.emplace_back();
    for (auto const& w : work) {
      out.buckets.back().emplace_back();
      auto& bucket = out.buckets.back().back();
      // Separate out any of the "roots" which are actually leafs.
      for (auto const cls : w.classes) {
        if (leafs.count(cls)) {
          bucket.leafs.emplace_back(cls);
          markProcessed.emplace_back(cls);
        } else {
          add(cls, bucket.classes, bucket.splits, bucket.edges);
        }
      }
      for (auto const cls : w.deps) {
        add(cls, bucket.deps, bucket.splitDeps, bucket.edges);
      }

      std::sort(
        begin(bucket.edges), end(bucket.edges),
        [] (const BuildSubclassListJob::EdgeToSplit& a,
            const BuildSubclassListJob::EdgeToSplit& b) {
          if (string_data_lti{}(a.cls, b.cls)) return true;
          if (string_data_lti{}(b.cls, a.cls)) return false;
          return string_data_lti{}(a.split, b.split);
        }
      );
      std::sort(begin(bucket.leafs), end(bucket.leafs), string_data_lti{});
    }

    // Update the processed set. We have to defer that until here
    // because we'd check it when building the buckets.
    processed.insert(begin(markProcessed), end(markProcessed));

    toProcess.erase(
      std::remove_if(
        begin(toProcess), end(toProcess),
        [&] (SString c) { return processed.count(c); }
      ),
      end(toProcess)
    );
  }

  // Keep all split nodes created in the output
  for (auto& [name, p] : splitPtrs) out.allSplits.emplace(name, std::move(p));

  if (Trace::moduleEnabled(Trace::hhbbc_index, 5)) {
    for (size_t round = 0; round < out.buckets.size(); ++round) {
      auto const& r = out.buckets[round];
      for (size_t i = 0; i < r.size(); ++i) {
        auto const& bucket = r[i];
        FTRACE(5, "build subclass lists round #{} work item #{}:\n", round, i);
        FTRACE(5, "  classes ({}):\n", bucket.classes.size());
        for (auto const DEBUG_ONLY c : bucket.classes) FTRACE(6, "    {}\n", c);
        FTRACE(5, "  splits ({}):\n", bucket.splits.size());
        for (auto const DEBUG_ONLY s : bucket.splits) FTRACE(6, "    {}\n", s);
        FTRACE(5, "  deps ({}):\n", bucket.deps.size());
        for (auto const DEBUG_ONLY d : bucket.deps) FTRACE(6, "    {}\n", d);
        FTRACE(5, "  split deps ({}):\n", bucket.splitDeps.size());
        for (auto const DEBUG_ONLY s : bucket.splitDeps) {
          FTRACE(6, "    {}\n", s);
        }
        FTRACE(5, "  leafs ({}):\n", bucket.leafs.size());
        for (auto const DEBUG_ONLY c : bucket.leafs) {
          FTRACE(6, "    {}\n", c);
        }
        FTRACE(5, "  edges ({}):\n", bucket.edges.size());
        for (DEBUG_ONLY auto const& e : bucket.edges) {
          FTRACE(6, "    {} -> {}\n", e.cls, e.split);
        }
      }
    }
  }

  return out;
}

/*
 * Aggregate together func family entries corresponding to the same
 * name. Insert the aggregate entries into the index.
 */
void aggregate_name_only_entries(
  IndexData& index,
  SStringToOneT<std::vector<FuncFamilyEntry>> work
) {
  trace_time tracer{"aggregate name-only entries", index.sample};

  using namespace folly::gen;

  constexpr size_t kBucketSize = 3000;

  auto buckets = consistently_bucketize(
    [&] {
      std::vector<SString> sorted;
      sorted.reserve(work.size());
      for (auto const& [name, entries] : work) {
        if (entries.size() <= 1) {
          // If there's only one entry for a name, there's nothing to
          // aggregate, and can be inserted directly as the final
          // result.
          always_assert(
            index.nameOnlyMethodFamilies.emplace(name, entries[0]).second
          );
          continue;
        }
        // Otherwise insert a dummy entry. This will let us update the
        // entry later from multiple threads without having to mutate
        // the map.
        always_assert(
          index.nameOnlyMethodFamilies.emplace(name, FuncFamilyEntry{}).second
        );
        sorted.emplace_back(name);
      }
      std::sort(begin(sorted), end(sorted), string_data_lt{});
      return sorted;
    }(),
    kBucketSize
  );

  if (buckets.empty()) return;

  struct Updates {
    std::vector<
      std::pair<FuncFamily2::Id, Ref<FuncFamilyGroup>>
    > funcFamilies;
  };

  auto const run = [&] (std::vector<SString> names) -> coro::Task<Updates> {
    HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;

    if (names.empty()) HPHP_CORO_RETURN(Updates{});

    std::vector<std::pair<SString, std::vector<FuncFamilyEntry>>> entries;
    std::vector<Ref<FuncFamilyGroup>> funcFamilies;

    entries.reserve(names.size());
    // Extract out any func families the entries refer to, so they can
    // be provided to the job.
    for (auto const n : names) {
      auto& e = work.at(n);
      entries.emplace_back(n, std::move(e));
      for (auto const& entry : entries.back().second) {
        match<void>(
          entry.m_meths,
          [&] (const FuncFamilyEntry::BothFF& e) {
            funcFamilies.emplace_back(index.funcFamilyRefs.at(e.m_ff));
          },
          [&] (const FuncFamilyEntry::FFAndSingle& e) {
            funcFamilies.emplace_back(index.funcFamilyRefs.at(e.m_ff));
          },
          [&] (const FuncFamilyEntry::FFAndNone& e) {
            funcFamilies.emplace_back(index.funcFamilyRefs.at(e.m_ff));
          },
          [&] (const FuncFamilyEntry::BothSingle&)    {},
          [&] (const FuncFamilyEntry::SingleAndNone&) {},
          [&] (const FuncFamilyEntry::None&)          {}
        );
      }
    }

    std::sort(begin(funcFamilies), end(funcFamilies));
    funcFamilies.erase(
      std::unique(begin(funcFamilies), end(funcFamilies)),
      end(funcFamilies)
    );

    auto [entriesRef, config] = HPHP_CORO_AWAIT(coro::collect(
      index.client->store(std::move(entries)),
      index.configRef->getCopy()
    ));

    Client::ExecMetadata metadata{
      .job_key = folly::sformat(
        "aggregate name-only {}",
        names[0]
      )
    };

    auto results = HPHP_CORO_AWAIT(
      index.client->exec(
        s_aggregateNameOnlyJob,
        std::move(config),
        singleton_vec(
          std::make_tuple(std::move(entriesRef), std::move(funcFamilies))
        ),
        std::move(metadata)
      )
    );
    assertx(results.size() == 1);
    auto& [ffRefs, outMetaRef] = results[0];

    auto outMeta = HPHP_CORO_AWAIT(index.client->load(std::move(outMetaRef)));
    assertx(outMeta.newFuncFamilyIds.size() == ffRefs.size());
    assertx(outMeta.nameOnly.size() == names.size());

    // Update the dummy entries with the actual result.
    for (size_t i = 0, size = names.size(); i < size; ++i) {
      auto& old = index.nameOnlyMethodFamilies.at(names[i]);
      assertx(boost::get<FuncFamilyEntry::None>(&old.m_meths));
      old = std::move(outMeta.nameOnly[i]);
    }

    Updates updates;
    updates.funcFamilies.reserve(outMeta.newFuncFamilyIds.size());
    for (size_t i = 0, size = outMeta.newFuncFamilyIds.size(); i < size; ++i) {
      auto const ref = ffRefs[i];
      for (auto const& id : outMeta.newFuncFamilyIds[i]) {
        updates.funcFamilies.emplace_back(id, ref);
      }
    }

    HPHP_CORO_RETURN(updates);
  };

  auto const updates = coro::wait(coro::collectRange(
    from(buckets)
      | move
      | map([&] (std::vector<SString>&& n) {
          return run(std::move(n)).scheduleOn(index.executor->sticky());
        })
      | as<std::vector>()
  ));

  for (auto const& u : updates) {
    for (auto const& [id, ref] : u.funcFamilies) {
      // The same FuncFamily can be grouped into multiple
      // different groups. Prefer the group that's smaller and
      // if they're the same size, use the one with the lowest
      // id to keep determinism.
      auto const& [existing, inserted] =
        index.funcFamilyRefs.emplace(id, ref);
      if (inserted) continue;
      if (existing->second.id().m_size < ref.id().m_size) continue;
      if (ref.id().m_size < existing->second.id().m_size) {
        existing->second = ref;
        continue;
      }
      if (existing->second.id() <= ref.id()) continue;
      existing->second = ref;
    }
  }
}

std::vector<InterfaceConflicts> build_subclass_lists(IndexData& index,
                                                     SubclassMetadata meta) {
  trace_time tracer{"build subclass lists", index.sample};

  using namespace folly::gen;

  // Mapping of splits to their Ref. We only upload a split when we're
  // going to run a job which it is part of the output.
  ISStringToOneT<UniquePtrRef<BuildSubclassListJob::Split>> splitsToRefs;

  ISStringToOneT<hphp_fast_set<FuncFamily2::Id>> funcFamilyDeps;

  SStringToOneT<std::vector<FuncFamilyEntry>> nameOnlyEntries;

  std::vector<InterfaceConflicts> ifaceConflicts;

  // Use the metadata to assign to rounds and buckets.
  auto work = build_subclass_lists_assign(std::move(meta));

  // We need to defer updates to data structures until after all the
  // jobs in a round have completed. Otherwise we could update a ref
  // to a class at the same time another thread is reading it.
  struct Updates {
    std::vector<
      std::tuple<SString, UniquePtrRef<ClassInfo2>, UniquePtrRef<php::Class>>
    > classes;
    std::vector<
      std::pair<SString, UniquePtrRef<BuildSubclassListJob::Split>>
    > splits;
    std::vector<
      std::pair<FuncFamily2::Id, Ref<FuncFamilyGroup>>
    > funcFamilies;
    std::vector<
      std::pair<SString, hphp_fast_set<FuncFamily2::Id>>
    > funcFamilyDeps;
    std::vector<std::pair<SString, FuncFamilyEntry>> nameOnly;
    std::vector<InterfaceConflicts> ifaceConflicts;
  };

  auto const run = [&] (SubclassWork::Bucket bucket, size_t round)
    -> coro::Task<Updates> {
    HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;

    if (bucket.classes.empty() &&
        bucket.splits.empty() &&
        bucket.leafs.empty()) {
      assertx(bucket.splitDeps.empty());
      HPHP_CORO_RETURN(Updates{});
    }

    // We shouldn't get closures or Closure in any of this.
    if (debug) {
      for (auto const c : bucket.classes) {
        always_assert(!c->isame(s_Closure.get()));
        always_assert(!is_closure_name(c));
      }
      for (auto const c : bucket.deps) {
        always_assert(!c->isame(s_Closure.get()));
        always_assert(!is_closure_name(c));
      }
    }

    auto classes = from(bucket.classes)
      | map([&] (SString c) { return index.classInfoRefs.at(c); })
      | as<std::vector>();
    auto deps = from(bucket.deps)
      | map([&] (SString c) { return index.classInfoRefs.at(c); })
      | as<std::vector>();
    auto leafs = from(bucket.leafs)
      | map([&] (SString c) { return index.classInfoRefs.at(c); })
      | as<std::vector>();
    auto splits = from(bucket.splits)
      | map([&] (SString s) {
          std::unique_ptr<BuildSubclassListJob::Split> split =
            std::move(work.allSplits.at(s));
          assertx(split);
          return split;
        })
      | as<std::vector>();
    auto splitDeps = from(bucket.splitDeps)
      | map([&] (SString s) { return splitsToRefs.at(s); })
      | as<std::vector>();
    auto phpClasses =
      (from(bucket.classes) + from(bucket.deps) + from(bucket.leafs))
      | map([&] (SString c) { return index.classRefs.at(c); })
      | as<std::vector>();

    std::vector<Ref<FuncFamilyGroup>> funcFamilies;
    if (round > 0) {
      // Provide the func families associated with any dependency
      // classes going into this job. We only need to do this after
      // the first round because in the first round all dependencies
      // are leafs and won't have any func families.
      for (auto const c : bucket.deps) {
        if (auto const deps = folly::get_ptr(funcFamilyDeps, c)) {
          for (auto const& d : *deps) {
            funcFamilies.emplace_back(index.funcFamilyRefs.at(d));
          }
        }
      }
      // Keep the func families in deterministic order and avoid
      // duplicates.
      std::sort(begin(funcFamilies), end(funcFamilies));
      funcFamilies.erase(
        std::unique(begin(funcFamilies), end(funcFamilies)),
        end(funcFamilies)
      );
    } else {
      assertx(funcFamilyDeps.empty());
      assertx(index.funcFamilyRefs.empty());
    }

    // ClassInfos and any dependency splits should already be
    // stored. Any splits as output of the job, or edges need to be
    // uploaded, however.
    auto [splitRefs, edges, config] = HPHP_CORO_AWAIT(coro::collect(
      index.client->storeMulti(std::move(splits)),
      index.client->storeMulti(std::move(bucket.edges)),
      index.configRef->getCopy()
    ));

    Client::ExecMetadata metadata{
      .job_key = folly::sformat(
        "build subclass list {}",
        [&] {
          if (!bucket.classes.empty()) return bucket.classes[0];
          if (!bucket.splits.empty()) return bucket.splits[0];
          assertx(!bucket.leafs.empty());
          return bucket.leafs[0];
        }()
      )
    };

    auto results = HPHP_CORO_AWAIT(
      index.client->exec(
        s_buildSubclassJob,
        std::move(config),
        singleton_vec(
          std::make_tuple(
            std::move(classes),
            std::move(deps),
            std::move(leafs),
            std::move(splitRefs),
            std::move(splitDeps),
            std::move(phpClasses),
            std::move(edges),
            std::move(funcFamilies)
          )
        ),
        std::move(metadata)
      )
    );
    // Every job is a single work-unit, so we should only ever get one
    // result for each one.
    assertx(results.size() == 1);
    auto& [cinfoRefs, outSplitRefs, clsRefs, ffRefs, outMetaRef] = results[0];
    assertx(cinfoRefs.size() == bucket.classes.size());
    assertx(outSplitRefs.size() == bucket.splits.size());
    assertx(clsRefs.size() == bucket.classes.size());

    auto outMeta = HPHP_CORO_AWAIT(index.client->load(std::move(outMetaRef)));
    assertx(outMeta.newFuncFamilyIds.size() == ffRefs.size());
    assertx(outMeta.funcFamilyDeps.size() == cinfoRefs.size());

    Updates updates;
    updates.classes.reserve(bucket.classes.size());
    updates.splits.reserve(bucket.splits.size());
    updates.funcFamilies.reserve(outMeta.newFuncFamilyIds.size());
    updates.funcFamilyDeps.reserve(outMeta.funcFamilyDeps.size());
    updates.nameOnly.reserve(outMeta.nameOnly.size());

    for (size_t i = 0, size = bucket.classes.size(); i < size; ++i) {
      updates.classes.emplace_back(bucket.classes[i], cinfoRefs[i], clsRefs[i]);
    }
    for (size_t i = 0, size = bucket.splits.size(); i < size; ++i) {
      updates.splits.emplace_back(bucket.splits[i], outSplitRefs[i]);
    }
    for (size_t i = 0, size = outMeta.newFuncFamilyIds.size(); i < size; ++i) {
      auto const ref = ffRefs[i];
      for (auto const& id : outMeta.newFuncFamilyIds[i]) {
        updates.funcFamilies.emplace_back(id, ref);
      }
    }
    for (size_t i = 0, size = outMeta.funcFamilyDeps.size(); i < size; ++i) {
      updates.funcFamilyDeps.emplace_back(
        bucket.classes[i],
        std::move(outMeta.funcFamilyDeps[i])
      );
    }
    updates.nameOnly = std::move(outMeta.nameOnly);
    updates.ifaceConflicts = std::move(outMeta.interfaces);

    HPHP_CORO_MOVE_RETURN(updates);
  };

  {
    trace_time tracer2{"build subclass lists work", index.sample};

    for (size_t roundNum = 0; roundNum < work.buckets.size(); ++roundNum) {
      auto& round = work.buckets[roundNum];
      // In each round, run all of the work for each bucket
      // simultaneously, gathering up updates from each job.
      auto const updates = coro::wait(coro::collectRange(
        from(round)
          | move
          | map([&] (SubclassWork::Bucket&& b) {
              return run(std::move(b), roundNum)
                .scheduleOn(index.executor->sticky());
            })
          | as<std::vector>()
      ));

      // Apply the updates to ClassInfo refs. We can do this
      // concurrently because every ClassInfo is already in the map, so
      // we can update in place (without mutating the map).
      parallel::for_each(
        updates,
        [&] (const Updates& u) {
          for (auto const& [name, cinfo, cls] : u.classes) {
            index.classInfoRefs.at(name) = cinfo;
            index.classRefs.at(name) = cls;
          }
        }
      );

      // However updating splitsToRefs cannot be, because we're mutating
      // the map by inserting into it. However there's a relatively
      // small number of splits, so this should be fine.
      parallel::parallel(
        [&] {
          for (auto const& u : updates) {
            for (auto const& [name, ref] : u.splits) {
              always_assert(splitsToRefs.emplace(name, ref).second);
            }
          }
        },
        [&] {
          for (auto const& u : updates) {
            for (auto const& [id, ref] : u.funcFamilies) {
              // The same FuncFamily can be grouped into multiple
              // different groups. Prefer the group that's smaller and
              // if they're the same size, use the one with the lowest
              // id to keep determinism.
              auto const& [existing, inserted] =
                index.funcFamilyRefs.emplace(id, ref);
              if (inserted) continue;
              if (existing->second.id().m_size < ref.id().m_size) continue;
              if (ref.id().m_size < existing->second.id().m_size) {
                existing->second = ref;
                continue;
              }
              if (existing->second.id() <= ref.id()) continue;
              existing->second = ref;
            }
          }
        },
        [&] {
          for (auto& u : updates) {
            for (auto& [name, ids] : u.funcFamilyDeps) {
              always_assert(funcFamilyDeps.emplace(name, std::move(ids)).second);
            }
          }
        },
        [&] {
          for (auto& u : updates) {
            for (auto& [name, entry] : u.nameOnly) {
              nameOnlyEntries[name].emplace_back(std::move(entry));
            }
          }
        },
        [&] {
          for (auto& u : updates) {
            ifaceConflicts.insert(
              end(ifaceConflicts),
              begin(u.ifaceConflicts),
              end(u.ifaceConflicts)
            );
          }
        }
      );
    }
  }

  splitsToRefs.clear();
  funcFamilyDeps.clear();
  work.buckets.clear();
  work.allSplits.clear();

  // Leaf interfaces won't be processed by a Job, but their
  // InterfaceConflicts are trivial.
  for (auto const iface : work.leafInterfaces) {
    ifaceConflicts.emplace_back(InterfaceConflicts{iface, 1});
  }
  work.leafInterfaces.clear();

  aggregate_name_only_entries(index, std::move(nameOnlyEntries));

  return ifaceConflicts;
}

//////////////////////////////////////////////////////////////////////

Index::Input::UnitMeta make_native_unit_meta(IndexData& index) {
  auto unit = make_native_unit();
  auto const name = unit->filename;
  auto unitRef = coro::wait(index.client->store(std::move(unit)));
  return Index::Input::UnitMeta { std::move(unitRef), name };
}

// Set up the async state, populate the (initial) table of
// extern-worker refs in the Index, and build some metadata needed for
// class flattening.
IndexFlattenMetadata make_remote(IndexData& index,
                                 Config config,
                                 Index::Input input,
                                 std::unique_ptr<coro::TicketExecutor> executor,
                                 std::unique_ptr<Client> client,
                                 DisposeCallback dispose) {
  trace_time tracer("make remote");
  tracer.ignore_client_stats();

  assertx(input.classes.size() == input.classBC.size());
  assertx(input.funcs.size() == input.funcBC.size());

  index.executor = std::move(executor);
  index.client = std::move(client);
  index.disposeClient = std::move(dispose);

  // Kick off the storage of the global config. We'll start early so
  // it will (hopefully) be done before we need it.
  index.configRef = std::make_unique<coro::AsyncValue<Ref<Config>>>(
    [&index, config = std::move(config)] () mutable {
      return index.client->store(std::move(config));
    },
    index.executor->sticky()
  );

  // Create a fake unit to store native constants and add it as an
  // input.
  input.units.emplace_back(make_native_unit_meta(index));

  IndexFlattenMetadata flattenMeta;
  SStringToOneT<SString> methCallerUnits;

  flattenMeta.cls.reserve(input.classes.size());
  flattenMeta.allCls.reserve(input.classes.size());
  flattenMeta.allFuncs.reserve(input.funcs.size());

  // Add unit and class information to their appropriate tables. This
  // is also where we'll detect duplicate funcs and class names (which
  // should be caught earlier during parsing).
  for (auto& unit : input.units) {
    FTRACE(5, "unit {} -> {}\n", unit.name, unit.unit.id().toString());

    for (auto& typeMapping : unit.typeMappings) {
      auto const name = typeMapping.name;
      always_assert_flog(
        flattenMeta.typeMappings.emplace(name, std::move(typeMapping)).second,
        "Duplicate type-mapping: {}",
        name
      );
    }

    always_assert_flog(
      index.unitRefs.emplace(unit.name, std::move(unit.unit)).second,
      "Duplicate unit: {}",
      unit.name
    );
  }
  for (auto& cls : input.classes) {
    FTRACE(5, "class {} -> {}\n", cls.name, cls.cls.id().toString());
    always_assert_flog(
      index.classRefs.emplace(cls.name, std::move(cls.cls)).second,
      "Duplicate class: {}",
      cls.name
    );

    auto& meta = flattenMeta.cls[cls.name];
    if (cls.closureContext) {
      flattenMeta.cls[cls.closureContext].closures.emplace_back(cls.name);
    }
    meta.isClosure = cls.isClosure;
    meta.deps.insert(begin(cls.dependencies), end(cls.dependencies));
    meta.unresolvedTypes = std::move(cls.unresolvedTypes);
    meta.idx = flattenMeta.allCls.size();
    flattenMeta.allCls.emplace_back(cls.name);

    if (cls.typeMapping) {
      auto const name = cls.typeMapping->name;
      always_assert_flog(
        flattenMeta.typeMappings.emplace(
          name, std::move(*cls.typeMapping)
        ).second,
        "Duplicate type-mapping: {}",
        name
      );
    }
  }
  // Funcs have an additional wrinkle, however. A func might be a meth
  // caller. Meth callers are special in that they might be present
  // (with the same name) in multiple units. However only one "wins"
  // and is actually emitted in the repo. We detect that here and
  // select a winner. The "losing" meth callers will be actually
  // removed from their unit after class flattening.
  for (auto& func : input.funcs) {
    FTRACE(5, "func {} -> {}\n", func.name, func.func.id().toString());

    if (func.methCallerUnit) {
      // If this meth caller a duplicate of one we've already seen?
      auto const [existing, emplaced] =
        methCallerUnits.emplace(func.name, func.methCallerUnit);
      if (!emplaced) {
        // It is. The duplicate shouldn't be in the same unit,
        // however.
        always_assert_flog(
          existing->second != func.methCallerUnit,
          "Duplicate meth-caller {} in same unit {}",
          func.name,
          func.methCallerUnit
        );
        // The winner is the one with the unit with the "lesser"
        // name. This is completely arbitrary.
        if (string_data_lt{}(func.methCallerUnit, existing->second)) {
          // This one wins. Schedule the older entry for deletion and
          // take over it's position in the map.
          FTRACE(
            4, "  meth caller {} from unit {} taking priority over unit {}",
            func.name, func.methCallerUnit, existing->second
          );
          flattenMeta.unitDeletions[existing->second].emplace_back(func.name);
          existing->second = func.methCallerUnit;
          index.funcRefs.at(func.name) = std::move(func.func);
        } else {
          // This one loses. Schedule it for deletion.
          flattenMeta.unitDeletions[func.methCallerUnit].emplace_back(
            func.name
          );
        }
        continue;
      }
      // It's not. Treat it like anything else.
    }

    // If not a meth caller, treat it like anything else.
    always_assert_flog(
      index.funcRefs.emplace(func.name, std::move(func.func)).second,
      "Duplicate func: {}",
      func.name
    );

    auto& meta = flattenMeta.func[func.name];
    meta.unresolvedTypes = std::move(func.unresolvedTypes);

    flattenMeta.allFuncs.emplace_back(func.name);
  }

  for (auto& bc : input.classBC) {
    FTRACE(5, "class bytecode {} -> {}\n", bc.name, bc.bc.id().toString());

    always_assert_flog(
      index.classRefs.count(bc.name),
      "Class bytecode for non-existent class {}",
      bc.name
    );
    always_assert_flog(
      index.classBytecodeRefs.emplace(bc.name, std::move(bc.bc)).second,
      "Duplicate class bytecode: {}",
      bc.name
    );
  }

  for (auto& bc : input.funcBC) {
    FTRACE(5, "func bytecode {} -> {}\n", bc.name, bc.bc.id().toString());

    always_assert_flog(
      index.funcRefs.count(bc.name),
      "Func bytecode for non-existent func {}",
      bc.name
    );

    if (bc.methCallerUnit) {
      // Only record this bytecode if it's associated meth-caller was
      // kept.
      auto const it = methCallerUnits.find(bc.name);
      always_assert_flog(
        it != end(methCallerUnits),
        "Bytecode for func {} is marked as meth-caller, "
        "but func is not a meth-caller",
        bc.name
      );
      auto const unit = it->second;
      if (bc.methCallerUnit != unit) {
        FTRACE(
          4,
          "Bytecode for meth-caller func {} in unit {} "
          "skipped because the meth-caller was dropped\n",
          bc.name, bc.methCallerUnit
        );
        continue;
      }
    } else {
      always_assert_flog(
        !methCallerUnits.count(bc.name),
        "Bytecode for func {} is not marked as meth-caller, "
        "but func is a meth-caller",
        bc.name
      );
    }

    always_assert_flog(
      index.funcBytecodeRefs.emplace(bc.name, std::move(bc.bc)).second,
      "Duplicate func bytecode: {}",
      bc.name
    );
  }

  return flattenMeta;
}

//////////////////////////////////////////////////////////////////////

/*
 * Combines multiple classes/class-infos/funcs/units into a single
 * blob. Makes make_local() more efficient, as you can download a
 * smaller number of large blobs rather than many small blobs.
 */
struct AggregateJob {
  static std::string name() { return "hhbbc-aggregate"; }
  static void init(const Config& config) {
    process_init(config.o, config.gd, false);
  }
  static void fini() {}

  struct Bundle {
    std::vector<std::unique_ptr<php::Class>> classes;
    std::vector<std::unique_ptr<ClassInfo2>> classInfos;
    std::vector<std::unique_ptr<php::ClassBytecode>> classBytecode;
    std::vector<std::unique_ptr<php::Func>> funcs;
    std::vector<std::unique_ptr<FuncInfo2>> funcInfos;
    std::vector<std::unique_ptr<php::FuncBytecode>> funcBytecode;
    std::vector<std::unique_ptr<php::Unit>> units;
    std::vector<FuncFamilyGroup> funcFamilies;

    template <typename SerDe> void serde(SerDe& sd) {
      ScopedStringDataIndexer _;
      sd(classes)
        (classInfos)
        (classBytecode)
        (funcs)
        (funcInfos)
        (funcBytecode)
        (units)
        (funcFamilies)
        ;
    }
  };

  static Bundle run(Variadic<std::unique_ptr<php::Class>> classes,
                    Variadic<std::unique_ptr<ClassInfo2>> classInfos,
                    Variadic<std::unique_ptr<php::ClassBytecode>> classBytecode,
                    Variadic<std::unique_ptr<php::Func>> funcs,
                    Variadic<std::unique_ptr<FuncInfo2>> funcInfos,
                    Variadic<std::unique_ptr<php::FuncBytecode>> funcBytecode,
                    Variadic<std::unique_ptr<php::Unit>> units,
                    Variadic<FuncFamilyGroup> funcFamilies) {
    Bundle bundle;
    bundle.classes.reserve(classes.vals.size());
    bundle.classInfos.reserve(classInfos.vals.size());
    bundle.classBytecode.reserve(classBytecode.vals.size());
    bundle.funcs.reserve(funcs.vals.size());
    bundle.funcInfos.reserve(funcInfos.vals.size());
    bundle.funcBytecode.reserve(funcBytecode.vals.size());
    bundle.units.reserve(units.vals.size());
    bundle.funcFamilies.reserve(funcFamilies.vals.size());
    for (auto& c : classes.vals) {
      bundle.classes.emplace_back(std::move(c));
    }
    for (auto& c : classInfos.vals) {
      bundle.classInfos.emplace_back(std::move(c));
    }
    for (auto& b : classBytecode.vals) {
      bundle.classBytecode.emplace_back(std::move(b));
    }
    for (auto& f : funcs.vals) {
      bundle.funcs.emplace_back(std::move(f));
    }
    for (auto& f : funcInfos.vals) {
      bundle.funcInfos.emplace_back(std::move(f));
    }
    for (auto& b : funcBytecode.vals) {
      bundle.funcBytecode.emplace_back(std::move(b));
    }
    for (auto& u : units.vals) {
      bundle.units.emplace_back(std::move(u));
    }
    for (auto& group : funcFamilies.vals) {
      bundle.funcFamilies.emplace_back(std::move(group));
    }
    return bundle;
  }
};

Job<AggregateJob> s_aggregateJob;

// Convert the FuncInfo2s we loaded from extern-worker into their
// equivalent FuncInfos.
void make_func_infos_local(IndexData& index,
                           std::vector<std::unique_ptr<FuncInfo2>> remote) {
  trace_time tracer{"make func-infos local"};
  tracer.ignore_client_stats();

  parallel::for_each(
    remote,
    [&] (const std::unique_ptr<FuncInfo2>& rfinfo) {
      auto const it = index.funcs.find(rfinfo->name);
      always_assert_flog(
        it != end(index.funcs),
        "Func-info for {} has no associated php::Func in index",
        rfinfo->name
      );
      auto const func = it->second;
      assertx(func->name == rfinfo->name);
      auto finfo = func_info(index, func);
      assertx(finfo->returnTy.is(BInitCell));
      finfo->returnTy = std::move(rfinfo->returnTy);
    }
  );
}

// Convert the ClassInfo2s we loaded from extern-worker into their
// equivalent ClassInfos (and store it in the Index).
void make_class_infos_local(
  IndexData& index,
  std::vector<std::unique_ptr<ClassInfo2>> remote,
  std::vector<std::unique_ptr<FuncFamily2>> funcFamilies
) {
  trace_time tracer{"make class-infos local"};
  tracer.ignore_client_stats();

  assertx(index.allClassInfos.empty());
  assertx(index.classInfo.empty());

  // First create a ClassInfo for each ClassInfo2. Since a ClassInfo
  // can refer to other ClassInfos, we can't do much more at this
  // stage.
  index.allClassInfos = parallel::map(
    remote,
    [&] (const std::unique_ptr<ClassInfo2>& cinfo) {
      auto out = std::make_unique<ClassInfo>();

      auto const it = index.classes.find(cinfo->name);
      always_assert_flog(
        it != end(index.classes),
        "Class-info for {} has no associated php::Class in index",
        cinfo->name
      );
      out->cls = it->second;
      return out;
    }
  );

  // Build table mapping name to ClassInfo.
  for (auto& cinfo : index.allClassInfos) {
    always_assert(
      index.classInfo.emplace(cinfo->cls->name, cinfo.get()).second
    );
  }
  index.allClassInfos.shrink_to_fit();

  // Set AttrNoOverride to true for all methods. If we determine it's
  // actually overridden below, we'll clear it.
  parallel::for_each(
    index.program->classes,
    [&] (std::unique_ptr<php::Class>& cls) {
      for (auto& m : cls->methods) {
        assertx(!(m->attrs & AttrNoOverride));
        if (is_special_method_name(m->name)) continue;
        attribute_setter(m->attrs, true, AttrNoOverride);
      }
    }
  );

  auto const get = [&] (SString name) {
    auto const it = index.classInfo.find(name);
    always_assert_flog(
      it != end(index.classInfo),
      "Class-info for {} not found in index",
      name
    );
    return it->second;
  };

  auto const vec = [&] (auto const& src, auto& dst) {
    dst.reserve(src.size());
    for (auto const s : src) dst.emplace_back(get(s));
    dst.shrink_to_fit();
  };

  struct FFState {
    explicit FFState(std::unique_ptr<FuncFamily2> ff) : m_ff{std::move(ff)} {}
    std::unique_ptr<FuncFamily2> m_ff;
    LockFreeLazyPtrNoDelete<FuncFamily> m_notExpanded;
    LockFreeLazyPtrNoDelete<FuncFamily> m_expanded;

    FuncFamily* notExpanded(IndexData& index) {
      return const_cast<FuncFamily*>(
        &m_notExpanded.get([&] { return make(index, false); })
      );
    }
    FuncFamily* expanded(IndexData& index) {
      return const_cast<FuncFamily*>(
        &m_expanded.get([&] { return make(index, true); })
      );
    }

    FuncFamily* make(IndexData& index, bool expanded) const {
      FuncFamily::PFuncVec funcs;
      funcs.reserve(
        m_ff->m_regular.size() +
        (expanded
         ? 0
         : (m_ff->m_nonRegularPrivate.size() + m_ff->m_nonRegular.size())
        )
      );

      for (auto const& m : m_ff->m_regular) {
        funcs.emplace_back(func_from_meth_ref(index, m), true);
      }
      for (auto const& m : m_ff->m_nonRegularPrivate) {
        funcs.emplace_back(func_from_meth_ref(index, m), true);
      }
      if (!expanded) {
        for (auto const& m : m_ff->m_nonRegular) {
          funcs.emplace_back(func_from_meth_ref(index, m), false);
        }
      }

      auto const extra = !expanded && !m_ff->m_nonRegular.empty() &&
        (m_ff->m_regular.size() + m_ff->m_nonRegularPrivate.size()) > 1;

      std::sort(
        begin(funcs), end(funcs),
        [] (FuncFamily::PossibleFunc a, const FuncFamily::PossibleFunc b) {
          if (a.inRegular() && !b.inRegular()) return true;
          if (!a.inRegular() && b.inRegular()) return false;
          return string_data_lti{}(a.ptr()->cls->name, b.ptr()->cls->name);
        }
      );
      funcs.shrink_to_fit();

      assertx(funcs.size() > 1);

      auto const convert = [&] (const FuncFamily2::StaticInfo& in) {
        FuncFamily::StaticInfo out;
        out.m_numInOut = in.m_numInOut;
        out.m_requiredCoeffects = in.m_requiredCoeffects;
        out.m_coeffectRules = in.m_coeffectRules;
        out.m_paramPreps = in.m_paramPreps;
        out.m_minNonVariadicParams = in.m_minNonVariadicParams;
        out.m_maxNonVariadicParams = in.m_maxNonVariadicParams;
        out.m_isReadonlyReturn = in.m_isReadonlyReturn;
        out.m_isReadonlyThis = in.m_isReadonlyThis;
        out.m_supportsAER = in.m_supportsAER;
        out.m_maybeReified = in.m_maybeReified;
        out.m_maybeCaresAboutDynCalls = in.m_maybeCaresAboutDynCalls;
        out.m_maybeBuiltin = in.m_maybeBuiltin;

        auto const it = index.funcFamilyStaticInfos.find(out);
        if (it != end(index.funcFamilyStaticInfos)) return it->first.get();
        return index.funcFamilyStaticInfos.insert(
          std::make_unique<FuncFamily::StaticInfo>(std::move(out)),
          false
        ).first->first.get();
      };

      auto newFuncFamily =
        std::make_unique<FuncFamily>(std::move(funcs), extra);

      always_assert(m_ff->m_allStatic);
      if (m_ff->m_regularStatic) {
        const FuncFamily::StaticInfo* reg = nullptr;
        if (expanded || extra) reg = convert(*m_ff->m_regularStatic);
        newFuncFamily->m_all.m_static =
          expanded ? reg : convert(*m_ff->m_allStatic);
        if (extra) {
          newFuncFamily->m_regular = std::make_unique<FuncFamily::Info>();
          newFuncFamily->m_regular->m_static = reg;
        }
      } else {
        newFuncFamily->m_all.m_static = convert(*m_ff->m_allStatic);
      }

      return index.funcFamilies.insert(
        std::move(newFuncFamily),
        false
      ).first->first.get();
    }
  };

  hphp_fast_map<FuncFamily2::Id, std::unique_ptr<FFState>> ffState;
  for (auto& ff : funcFamilies) {
    auto const id = ff->m_id;
    always_assert(
      ffState.emplace(
        id,
        std::make_unique<FFState>(std::move(ff))
      ).second
    );
  }
  funcFamilies.clear();

  std::mutex extraMethodLock;

  // Now that we can map name to ClassInfo, we can populate the rest
  // of the fields in each ClassInfo.
  parallel::for_each(
    remote,
    [&] (std::unique_ptr<ClassInfo2>& rcinfo) {
      auto const cinfo = get(rcinfo->name);
      if (rcinfo->parent) {
        cinfo->parent = get(rcinfo->parent);
      }

      vec(rcinfo->declInterfaces, cinfo->declInterfaces);
      vec(rcinfo->includedEnums, cinfo->includedEnums);
      vec(rcinfo->usedTraits, cinfo->usedTraits);
      vec(rcinfo->baseList, cinfo->baseList);

      cinfo->traitProps = std::move(rcinfo->traitProps);

      cinfo->implInterfaces.reserve(rcinfo->implInterfaces.size());
      for (auto const iface : rcinfo->implInterfaces) {
        cinfo->implInterfaces.emplace(iface, get(iface));
      }

      cinfo->clsConstants.reserve(rcinfo->clsConstants.size());
      for (auto const& [name, cns] : rcinfo->clsConstants) {
        auto const it = index.classes.find(cns.cls);
        always_assert_flog(
          it != end(index.classes),
          "php::Class for {} not found in index",
          name
        );
        cinfo->clsConstants.emplace(
          name,
          ClassInfo::ConstIndex { it->second, cns.idx }
        );
      }

      {
        std::vector<std::pair<SString, MethTabEntry>> methods;
        methods.reserve(cinfo->methods.size());
        for (auto const& [name, mte] : rcinfo->methods) {
          if (!(mte.attrs & AttrNoOverride)) {
            attribute_setter(
              func_from_meth_ref(index, mte.meth())->attrs,
              false,
              AttrNoOverride
            );
          }
          methods.emplace_back(name, mte);
        }
        std::sort(
          begin(methods), end(methods),
          [] (auto const& p1, auto const& p2) { return p1.first < p2.first; }
        );
        cinfo->methods.insert(
          folly::sorted_unique, begin(methods), end(methods)
        );
        cinfo->methods.shrink_to_fit();
      }

      for (auto const sub : rcinfo->subclassList) {
        cinfo->subclassList.emplace_back(get(sub));
      }
      std::sort(begin(cinfo->subclassList), end(cinfo->subclassList));

      cinfo->hasBadRedeclareProp = rcinfo->hasBadRedeclareProp;
      cinfo->hasBadInitialPropValues = rcinfo->hasBadInitialPropValues;
      cinfo->hasConstProp = rcinfo->hasConstProp;
      cinfo->hasReifiedParent = rcinfo->hasReifiedParent;
      cinfo->subHasConstProp = rcinfo->subHasConstProp;
      cinfo->isMocked = rcinfo->isMocked;
      cinfo->isSubMocked = rcinfo->isSubMocked;
      cinfo->hasRegularSubclass = rcinfo->hasRegularSubclass;
      cinfo->hasNonRegularSubclass = rcinfo->hasNonRegularSubclass;

      auto const noOverride = [&] (SString name) {
        if (auto const mte = folly::get_ptr(cinfo->methods, name)) {
          return bool(mte->attrs & AttrNoOverride);
        }
        return true;
      };

      auto const noOverrideRegular = [&] (SString name) {
        if (auto const mte = folly::get_ptr(cinfo->methods, name)) {
          return mte->noOverrideRegular();
        }
        return true;
      };

      std::vector<std::pair<SString, FuncFamilyOrSingle>> entries;
      std::vector<std::pair<SString, FuncFamilyOrSingle>> aux;
      for (auto const& [name, entry] : rcinfo->methodFamilies) {
        assertx(!is_special_method_name(name));

        auto expanded = false;
        if (!cinfo->methods.count(name)) {
          if (!(cinfo->cls->attrs & (AttrAbstract|AttrInterface))) continue;
          if (!rcinfo->hasRegularSubclass) continue;
          if (entry.m_regularIncomplete || entry.m_privateAncestor) continue;
          if (name == s_construct.get()) continue;
          expanded = true;
        } else if (noOverride(name)) {
          continue;
        }

        match<void>(
          entry.m_meths,
          [&, name=name, &entry=entry] (const FuncFamilyEntry::BothFF& e) {
            auto const it = ffState.find(e.m_ff);
            assertx(it != end(ffState));
            auto const& state = it->second;

            if (expanded) {
              if (state->m_ff->m_regular.empty()) return;
              if (state->m_ff->m_regular.size() == 1) {
                entries.emplace_back(
                  name,
                  FuncFamilyOrSingle{
                    func_from_meth_ref(index, state->m_ff->m_regular[0]),
                    false
                  }
                );
                return;
              }
              entries.emplace_back(
                name,
                FuncFamilyOrSingle{
                  state->expanded(index),
                  false
                }
              );
              return;
            }

            entries.emplace_back(
              name,
              FuncFamilyOrSingle{
                state->notExpanded(index),
                entry.m_allIncomplete
              }
            );
          },
          [&, name=name, &entry=entry] (const FuncFamilyEntry::FFAndSingle& e) {
            if (expanded) {
              if (e.m_nonRegularPrivate) return;
              entries.emplace_back(
                name,
                FuncFamilyOrSingle{
                  func_from_meth_ref(index, e.m_regular),
                  false
                }
              );
              return;
            }

            auto const it = ffState.find(e.m_ff);
            assertx(it != end(ffState));

            entries.emplace_back(
              name,
              FuncFamilyOrSingle{
                it->second->notExpanded(index),
                entry.m_allIncomplete
              }
            );
            if (noOverrideRegular(name)) return;
            aux.emplace_back(
              name,
              FuncFamilyOrSingle{
                func_from_meth_ref(index, e.m_regular),
                false
              }
            );
          },
          [&, name=name, &entry=entry] (const FuncFamilyEntry::FFAndNone& e) {
            if (expanded) return;
            auto const it = ffState.find(e.m_ff);
            assertx(it != end(ffState));

            entries.emplace_back(
              name,
              FuncFamilyOrSingle{
                it->second->notExpanded(index),
                entry.m_allIncomplete
              }
            );
            if (!noOverrideRegular(name)) {
              aux.emplace_back(name, FuncFamilyOrSingle{});
            }
          },
          [&, name=name, &entry=entry] (const FuncFamilyEntry::BothSingle& e) {
            if (expanded && e.m_nonRegularPrivate) return;
            entries.emplace_back(
              name,
              FuncFamilyOrSingle{
                func_from_meth_ref(index, e.m_all),
                !expanded && entry.m_allIncomplete
              }
            );
          },
          [&, name=name, &entry=entry] (const FuncFamilyEntry::SingleAndNone& e) {
            if (expanded) return;
            entries.emplace_back(
              name,
              FuncFamilyOrSingle{
                func_from_meth_ref(index, e.m_all),
                entry.m_allIncomplete
              }
            );
            if (!noOverrideRegular(name)) {
              aux.emplace_back(name, FuncFamilyOrSingle{});
            }
          },
          [&, &entry=entry] (const FuncFamilyEntry::None&) {
            assertx(entry.m_allIncomplete);
          }
        );
      }

      // Sort the lists of new entries, so we can insert them into the
      // method family maps (which are sorted_vector_maps) in bulk.
      std::sort(
        begin(entries), end(entries),
        [] (auto const& p1, auto const& p2) { return p1.first < p2.first; }
      );
      std::sort(
        begin(aux), end(aux),
        [] (auto const& p1, auto const& p2) { return p1.first < p2.first; }
      );
      if (!entries.empty()) {
        cinfo->methodFamilies.insert(
          folly::sorted_unique, begin(entries), end(entries)
        );
      }
      if (!aux.empty()) {
        cinfo->methodFamiliesAux.insert(
          folly::sorted_unique, begin(aux), end(aux)
        );
      }
      cinfo->methodFamilies.shrink_to_fit();
      cinfo->methodFamiliesAux.shrink_to_fit();

      if (!rcinfo->extraMethods.empty()) {
        // This is rare. Only happens with unflattened traits, so we
        // taking a lock here is fine.
        std::lock_guard<std::mutex> _{extraMethodLock};
        auto& extra = index.classExtraMethodMap[cinfo->cls];
        for (auto const& meth : rcinfo->extraMethods) {
          extra.emplace(func_from_meth_ref(index, meth));
        }
      }

      // Build the FuncInfo for every method on this class. The
      // FuncInfos already have default types, so update them with the
      // type from the FuncInfo2. Any class types here will be
      // unresolved (will be resolved later).
      assertx(cinfo->cls->methods.size() == rcinfo->funcInfos.size());
      for (size_t i = 0, size = cinfo->cls->methods.size(); i < size; ++i) {
        auto& func = cinfo->cls->methods[i];
        auto& rfi = rcinfo->funcInfos[i];
        assertx(func->name == rfi->name);
        auto fi = func_info(index, func.get());
        assertx(fi->returnTy.is(BInitCell));
        fi->returnTy = std::move(rfi->returnTy);
      }

      // Free memory as we go.
      rcinfo.reset();
    }
  );

  remote.clear();

  for (auto const& [name, entry] : index.nameOnlyMethodFamilies) {
    match<void>(
      entry.m_meths,
      [&, name=name] (const FuncFamilyEntry::BothFF& e) {
        auto const it = ffState.find(e.m_ff);
        assertx(it != end(ffState));

        FuncFamilyOrSingle f{ it->second->notExpanded(index), true };
        index.methodFamilies.emplace(
          name,
          IndexData::MethodFamilyEntry { f, f }
        );
      },
      [&, name=name] (const FuncFamilyEntry::FFAndSingle& e) {
        auto const it = ffState.find(e.m_ff);
        assertx(it != end(ffState));

        index.methodFamilies.emplace(
          name,
          IndexData::MethodFamilyEntry {
            FuncFamilyOrSingle {
              it->second->notExpanded(index),
              true
            },
            FuncFamilyOrSingle {
              func_from_meth_ref(index, e.m_regular),
              true
            }
          }
        );
      },
      [&, name=name] (const FuncFamilyEntry::FFAndNone& e) {
        auto const it = ffState.find(e.m_ff);
        assertx(it != end(ffState));

        index.methodFamilies.emplace(
          name,
          IndexData::MethodFamilyEntry {
            FuncFamilyOrSingle {
              it->second->notExpanded(index),
              true
            },
            FuncFamilyOrSingle {}
          }
        );
      },
      [&, name=name] (const FuncFamilyEntry::BothSingle& e) {
        FuncFamilyOrSingle f{ func_from_meth_ref(index, e.m_all), true };
        index.methodFamilies.emplace(
          name,
          IndexData::MethodFamilyEntry { f, f }
        );
      },
      [&, name=name] (const FuncFamilyEntry::SingleAndNone& e) {
        index.methodFamilies.emplace(
          name,
          IndexData::MethodFamilyEntry {
            FuncFamilyOrSingle {
              func_from_meth_ref(index, e.m_all),
              true
            },
            FuncFamilyOrSingle {}
          }
        );
      },
      [&] (const FuncFamilyEntry::None&) { always_assert(false); }
    );
  }

  ffState.clear();
  decltype(index.nameOnlyMethodFamilies){}.swap(index.nameOnlyMethodFamilies);

  // Now that all of the FuncFamilies have been created, generate the
  // back links from FuncInfo to their FuncFamilies.
  std::vector<FuncFamily*> work;
  work.reserve(index.funcFamilies.size());
  for (auto const& kv : index.funcFamilies) work.emplace_back(kv.first.get());

  // First calculate the needed capacity for each FuncInfo's family
  // list. We use this to presize the family list. This is superior
  // just pushing back and then shrinking the vectors, as that can
  // excessively fragment the heap.
  std::vector<std::atomic<size_t>> capacities(index.nextFuncId);

  parallel::for_each(
    work,
    [&] (FuncFamily* ff) {
      for (auto const pf : ff->possibleFuncs()) {
        ++capacities[pf.ptr()->idx];
      }
    }
  );

  parallel::for_each(
    index.funcInfo,
    [&] (FuncInfo& fi) {
      if (!fi.func) return;
      fi.families.reserve(capacities[fi.func->idx]);
    }
  );
  capacities.clear();
  capacities.shrink_to_fit();

  // Different threads can touch the same FuncInfo when adding to the
  // func family list, so use sharded locking scheme.
  std::array<std::mutex, 256> locks;
  parallel::for_each(
    work,
    [&] (FuncFamily* ff) {
      for (auto const pf : ff->possibleFuncs()) {
        auto finfo = func_info(index, pf.ptr());
        auto& lock = locks[pointer_hash<FuncInfo>{}(finfo) % locks.size()];
        std::lock_guard<std::mutex> _{lock};
        finfo->families.emplace_back(ff);
      }
    }
  );
}

// Switch to "local" mode, in which all calculations are expected to
// be done locally (not using extern-worker). This involves
// downloading everything out of extern-worker and converting it. To
// improve efficiency, we first aggregate many small(er) items into
// larger aggregate blobs in external workers, then download the
// larger blobs.
void make_local(IndexData& index) {
  trace_time tracer("make local", index.sample);

  using namespace folly::gen;

  // Unlike other cases, we want to bound each bucket to roughly the
  // same total byte size (since ultimately we're going to download
  // everything).
  auto const usingSubprocess = index.client->usingSubprocess();
  // We can be more aggressive in subprocess mode because there's no
  // actual aggregation.
  auto const sizePerBucket = usingSubprocess
    ? 256*1024*1024
    : 32*1024*1024;

  /*
   * We'll use the names of the various items as the items to
   * bucketize. This is somewhat problematic because names between
   * units/funcs/classes/class-infos can collide (indeed classes and
   * class-infos will always collide). Adding to the complication is
   * that some of these are case sensitive and some aren't.
   *
   * We'll store a case sensitive version of each name exactly *once*,
   * using a seen set. Since the hash for a static string (which
   * consistently_bucketize() uses) is case insensitive, all case
   * sensitive versions of the same name will always hash to the same
   * bucket.
   *
   * When we obtain the Refs for a corresponding bucket, we'll load
   * all items with that given name, using a set to ensure each RefId
   * is only used once.
   */

  SStringToOneT<Ref<FuncFamilyGroup>> nameToFuncFamilyGroup;
  auto const& [items, totalSize] = [&] {
    SStringSet seen;
    std::vector<SString> items;
    size_t totalSize = 0;
    items.reserve(
      index.unitRefs.size() + index.classRefs.size() +
      index.classInfoRefs.size() + index.funcRefs.size() +
      index.funcInfoRefs.size() + index.funcFamilyRefs.size()
    );
    for (auto const& [name, ref] : index.unitRefs) {
      totalSize += ref.id().m_size;
      if (!seen.emplace(name).second) continue;
      items.emplace_back(name);
    }
    for (auto const& [name, ref] : index.classRefs) {
      totalSize += ref.id().m_size;
      if (!seen.emplace(name).second) continue;
      items.emplace_back(name);
    }
    for (auto const& [name, ref] : index.classInfoRefs) {
      totalSize += ref.id().m_size;
      if (!seen.emplace(name).second) continue;
      items.emplace_back(name);
    }
    for (auto const& [name, ref] : index.classBytecodeRefs) {
      totalSize += ref.id().m_size;
      if (!seen.emplace(name).second) continue;
      items.emplace_back(name);
    }
    for (auto const& [name, ref] : index.funcRefs) {
      totalSize += ref.id().m_size;
      if (!seen.emplace(name).second) continue;
      items.emplace_back(name);
    }
    for (auto const& [name, ref] : index.funcInfoRefs) {
      totalSize += ref.id().m_size;
      if (!seen.emplace(name).second) continue;
      items.emplace_back(name);
    }
    for (auto const& [name, ref] : index.funcBytecodeRefs) {
      totalSize += ref.id().m_size;
      if (!seen.emplace(name).second) continue;
      items.emplace_back(name);
    }

    for (auto const& [_, ref] : index.funcFamilyRefs) {
      auto const name = makeStaticString(ref.id().toString());
      nameToFuncFamilyGroup.emplace(name, ref);
    }
    for (auto const& [name, ref] : nameToFuncFamilyGroup) {
      totalSize += ref.id().m_size;
      if (!seen.emplace(name).second) continue;
      items.emplace_back(name);
    }
    std::sort(begin(items), end(items), string_data_lt{});
    return std::make_pair(items, totalSize);
  }();

  // Back out the number of buckets we want from the total size of
  // everything and the target size of a bucket.
  auto const numBuckets = (totalSize + sizePerBucket - 1) / sizePerBucket;
  auto buckets = consistently_bucketize(
    items,
    (items.size() + numBuckets - 1) / numBuckets
  );

  // We're going to be downloading all bytecode, so to avoid wasted
  // memory, try to re-use identical bytecode.
  Optional<php::FuncBytecode::Reuser> reuser;
  reuser.emplace();
  php::FuncBytecode::s_reuser = reuser.get_pointer();
  SCOPE_EXIT { php::FuncBytecode::s_reuser = nullptr; };

  std::mutex lock;
  auto program = std::make_unique<php::Program>();

  // Index stores ClassInfos, not ClassInfo2s, so we need a place to
  // store them until we convert it.
  std::vector<std::unique_ptr<ClassInfo2>> remoteClassInfos;
  remoteClassInfos.reserve(index.classInfoRefs.size());

  std::vector<std::unique_ptr<FuncInfo2>> remoteFuncInfos;
  remoteFuncInfos.reserve(index.funcInfoRefs.size());

  hphp_fast_set<FuncFamily2::Id> remoteFuncFamilyIds;
  std::vector<std::unique_ptr<FuncFamily2>> remoteFuncFamilies;
  remoteFuncFamilies.reserve(index.funcFamilyRefs.size());

  auto const run = [&] (std::vector<SString> chunks) -> coro::Task<void> {
    HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR;

    if (chunks.empty()) HPHP_CORO_RETURN_VOID;

    std::vector<UniquePtrRef<php::Class>> classes;
    std::vector<UniquePtrRef<ClassInfo2>> classInfos;
    std::vector<UniquePtrRef<php::ClassBytecode>> classBytecode;
    std::vector<UniquePtrRef<php::Func>> funcs;
    std::vector<UniquePtrRef<FuncInfo2>> funcInfos;
    std::vector<UniquePtrRef<php::FuncBytecode>> funcBytecode;
    std::vector<UniquePtrRef<php::Unit>> units;
    std::vector<Ref<FuncFamilyGroup>> funcFamilies;

    // Since a name can map to multiple items, and different case
    // sensitive version of the same name can appear in the same
    // bucket, use a set to make sure any given RefId only included
    // once. A set of case insensitive identical names will always
    // appear in the same bucket, so there's no need to track
    // duplicates globally.
    hphp_fast_set<RefId, RefId::Hasher> ids;

    for (auto const name : chunks) {
      if (auto const r = folly::get_optional(index.unitRefs, name)) {
        if (ids.emplace(r->id()).second) {
          units.emplace_back(*r);
        }
      }
      if (auto const r = folly::get_optional(index.classRefs, name)) {
        auto const bytecode = index.classBytecodeRefs.at(name);
        if (ids.emplace(r->id()).second) {
          classes.emplace_back(*r);
          classBytecode.emplace_back(bytecode);
        }
      }
      if (auto const r = folly::get_optional(index.classInfoRefs, name)) {
        if (ids.emplace(r->id()).second) {
          classInfos.emplace_back(*r);
        }
      }
      if (auto const r = folly::get_optional(index.funcRefs, name)) {
        auto const bytecode = index.funcBytecodeRefs.at(name);
        if (ids.emplace(r->id()).second) {
          funcs.emplace_back(*r);
          funcBytecode.emplace_back(bytecode);
        }
      }
      if (auto const r = folly::get_optional(index.funcInfoRefs, name)) {
        if (ids.emplace(r->id()).second) {
          funcInfos.emplace_back(*r);
        }
      }
      if (auto const r = folly::get_optional(nameToFuncFamilyGroup, name)) {
        if (ids.emplace(r->id()).second) {
          funcFamilies.emplace_back(*r);
        }
      }
    }

    AggregateJob::Bundle chunk;
    if (!usingSubprocess) {
      Client::ExecMetadata metadata{
        .job_key = folly::sformat("aggregate {}", chunks[0])
      };

      // Aggregate the data, which makes downloading it more efficient.
      auto config = HPHP_CORO_AWAIT(index.configRef->getCopy());
      auto outputs = HPHP_CORO_AWAIT(index.client->exec(
        s_aggregateJob,
        std::move(config),
        singleton_vec(
          std::make_tuple(
            std::move(classes),
            std::move(classInfos),
            std::move(classBytecode),
            std::move(funcs),
            std::move(funcInfos),
            std::move(funcBytecode),
            std::move(units),
            std::move(funcFamilies)
          )
        ),
        std::move(metadata)
      ));
      assertx(outputs.size() == 1);

      // Download the aggregate chunks.
      chunk = HPHP_CORO_AWAIT(index.client->load(std::move(outputs[0])));
    } else {
      // If we're using subprocess mode, we don't need to aggregate
      // and we can just download the items directly.
      auto [c, cinfo, cbc, f, finfo, fbc, u, ff] =
        HPHP_CORO_AWAIT(coro::collect(
          index.client->load(std::move(classes)),
          index.client->load(std::move(classInfos)),
          index.client->load(std::move(classBytecode)),
          index.client->load(std::move(funcs)),
          index.client->load(std::move(funcInfos)),
          index.client->load(std::move(funcBytecode)),
          index.client->load(std::move(units)),
          index.client->load(std::move(funcFamilies))
        )
      );
      chunk.classes.insert(
        end(chunk.classes),
        std::make_move_iterator(begin(c)),
        std::make_move_iterator(end(c))
      );
      chunk.classInfos.insert(
        end(chunk.classInfos),
        std::make_move_iterator(begin(cinfo)),
        std::make_move_iterator(end(cinfo))
      );
      chunk.classBytecode.insert(
        end(chunk.classBytecode),
        std::make_move_iterator(begin(cbc)),
        std::make_move_iterator(end(cbc))
      );
      chunk.funcs.insert(
        end(chunk.funcs),
        std::make_move_iterator(begin(f)),
        std::make_move_iterator(end(f))
      );
      chunk.funcInfos.insert(
        end(chunk.funcInfos),
        std::make_move_iterator(begin(finfo)),
        std::make_move_iterator(end(finfo))
      );
      chunk.funcBytecode.insert(
        end(chunk.funcBytecode),
        std::make_move_iterator(begin(fbc)),
        std::make_move_iterator(end(fbc))
      );
      chunk.units.insert(
        end(chunk.units),
        std::make_move_iterator(begin(u)),
        std::make_move_iterator(end(u))
      );
      chunk.funcFamilies.insert(
        end(chunk.funcFamilies),
        std::make_move_iterator(begin(ff)),
        std::make_move_iterator(end(ff))
      );
    }

    always_assert(chunk.classBytecode.size() == chunk.classes.size());
    for (size_t i = 0, size = chunk.classes.size(); i < size; ++i) {
      auto& bytecode = chunk.classBytecode[i];
      auto& cls = chunk.classes[i];
      always_assert(cls->methods.size() == bytecode->methodBCs.size());
      for (size_t j = 0, methSize = cls->methods.size(); j < methSize; ++j) {
        always_assert(!cls->methods[j]->rawBlocks);
        cls->methods[j]->rawBlocks = std::move(bytecode->methodBCs[j].bc);
      }
    }
    chunk.classBytecode.clear();

    always_assert(chunk.funcBytecode.size() == chunk.funcs.size());
    for (size_t i = 0, size = chunk.funcs.size(); i < size; ++i) {
      auto& bytecode = chunk.funcBytecode[i];
      chunk.funcs[i]->rawBlocks = std::move(bytecode->bc);
    }
    chunk.funcBytecode.clear();

    // And add it to our php::Program.
    {
      std::scoped_lock<std::mutex> _{lock};
      for (auto& unit : chunk.units) {
        // Local execution doesn't need the native unit, so strip it
        // out.
        if (is_native_unit(*unit)) continue;
        program->units.emplace_back(std::move(unit));
      }
      for (auto& cls : chunk.classes) {
        program->classes.emplace_back(std::move(cls));
      }
      for (auto& func : chunk.funcs) {
        program->funcs.emplace_back(std::move(func));
      }
      remoteClassInfos.insert(
        end(remoteClassInfos),
        std::make_move_iterator(begin(chunk.classInfos)),
        std::make_move_iterator(end(chunk.classInfos))
      );
      remoteFuncInfos.insert(
        end(remoteFuncInfos),
        std::make_move_iterator(begin(chunk.funcInfos)),
        std::make_move_iterator(end(chunk.funcInfos))
      );
      for (auto& group : chunk.funcFamilies) {
        for (auto& ff : group.m_ffs) {
          if (remoteFuncFamilyIds.emplace(ff->m_id).second) {
            remoteFuncFamilies.emplace_back(std::move(ff));
          }
        }
      }
    }

    HPHP_CORO_RETURN_VOID;
  };

  coro::wait(coro::collectRange(
    from(buckets)
      | move
      | map([&] (std::vector<SString> chunks) {
          return run(std::move(chunks)).scheduleOn(index.executor->sticky());
        })
      | as<std::vector>()
  ));

  // We've used any refs we need. Free them now to save memory.
  decltype(index.unitRefs){}.swap(index.unitRefs);
  decltype(index.classRefs){}.swap(index.classRefs);
  decltype(index.funcRefs){}.swap(index.funcRefs);
  decltype(index.classInfoRefs){}.swap(index.classInfoRefs);
  decltype(index.funcInfoRefs){}.swap(index.funcInfoRefs);
  decltype(index.funcFamilyRefs){}.swap(index.funcFamilyRefs);
  decltype(index.classBytecodeRefs){}.swap(index.classBytecodeRefs);
  decltype(index.funcBytecodeRefs){}.swap(index.funcBytecodeRefs);

  // Done with any extern-worker stuff at this point:
  index.configRef.reset();

  Logger::FInfo(
    "{}",
    index.client->getStats().toString(
      "hhbbc",
      folly::sformat(
        "{:,} units, {:,} classes, {:,} class-infos, {:,} funcs",
        program->units.size(),
        program->classes.size(),
        remoteClassInfos.size(),
        program->funcs.size()
      )
    )
  );

  if (index.sample) {
    index.client->getStats().logSample("hhbbc", *index.sample);
    index.sample->setStr(
      "hhbbc_fellback",
      index.client->fellback() ? "true" : "false"
    );
  }

  index.disposeClient(
    std::move(index.executor),
    std::move(index.client)
  );
  index.disposeClient = decltype(index.disposeClient){};

  php::FuncBytecode::s_reuser = nullptr;
  reuser.reset();

  buckets.clear();
  nameToFuncFamilyGroup.clear();
  remoteFuncFamilyIds.clear();

  program->units.shrink_to_fit();
  program->classes.shrink_to_fit();
  program->funcs.shrink_to_fit();
  index.program = std::move(program);

  // For now we don't require system constants in any extern-worker
  // stuff we do. So we can just add it to the Index now.
  add_system_constants_to_index(index);

  // Buid Index data structures from the php::Program.
  add_program_to_index(index);

  // Convert the FuncInfo2s into FuncInfos.
  make_func_infos_local(
    index,
    std::move(remoteFuncInfos)
  );

  // Convert the ClassInfo2s into ClassInfos.
  make_class_infos_local(
    index,
    std::move(remoteClassInfos),
    std::move(remoteFuncFamilies)
  );

  // Any classes in the FuncInfo return types are unresolved. Now that
  // the index is completely built, we can resolve them to resolved
  // classes instead.
  trace_time tracer2{"resolve classes"};
  tracer2.ignore_client_stats();

  parallel::for_each(
    index.funcInfo,
    [&] (FuncInfo& fi) {
      if (!fi.func) return;
      fi.returnTy = resolve_classes(*index.m_index, std::move(fi.returnTy));
    }
  );
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

std::vector<SString> Index::Input::makeDeps(const php::Class& cls) {
  std::vector<SString> deps;
  if (cls.parentName) deps.emplace_back(cls.parentName);
  deps.insert(deps.end(), cls.interfaceNames.begin(), cls.interfaceNames.end());
  deps.insert(deps.end(), cls.usedTraitNames.begin(), cls.usedTraitNames.end());
  deps.insert(
    deps.end(),
    cls.includedEnumNames.begin(),
    cls.includedEnumNames.end()
  );
  return deps;
}

//////////////////////////////////////////////////////////////////////

Index::Index(Input input,
             Config config,
             std::unique_ptr<coro::TicketExecutor> executor,
             std::unique_ptr<Client> client,
             DisposeCallback dispose,
             StructuredLogEntry* sample)
  : m_data{std::make_unique<IndexData>(this)}
{
  trace_time tracer("create index", sample);
  m_data->sample = sample;

  auto flattenMeta = make_remote(
    *m_data,
    std::move(config),
    std::move(input),
    std::move(executor),
    std::move(client),
    std::move(dispose)
  );

  flatten_type_mappings(*m_data, flattenMeta);
  auto subclassMeta = flatten_classes(*m_data, std::move(flattenMeta));
  auto ifaceConflicts = build_subclass_lists(*m_data, std::move(subclassMeta));
  compute_iface_vtables(*m_data, std::move(ifaceConflicts));
  check_invariants(*m_data);
  make_local(*m_data);
  check_local_invariants(*m_data);
}

// Defined here so IndexData is a complete type for the unique_ptr
// destructor.
Index::~Index() {}

//////////////////////////////////////////////////////////////////////

const php::Program& Index::program() const {
  return *m_data->program;
}

StructuredLogEntry* Index::sample() const {
  return m_data->sample;
}

//////////////////////////////////////////////////////////////////////

const php::Class* Index::lookup_closure_context(const php::Class& cls) const {
  if (!cls.closureContextCls) return &cls;
  return m_data->classes.at(cls.closureContextCls);
}

const php::Unit* Index::lookup_func_unit(const php::Func& func) const {
  return m_data->units.at(func.unit);
}

const php::Unit* Index::lookup_func_original_unit(const php::Func& func) const {
  auto const unit = func.originalUnit ? func.originalUnit : func.unit;
  return m_data->units.at(unit);
}

const php::Unit* Index::lookup_class_unit(const php::Class& cls) const {
  return m_data->units.at(cls.unit);
}

const php::Class* Index::lookup_const_class(const php::Const& cns) const {
  return m_data->classes.at(cns.cls);
}

//////////////////////////////////////////////////////////////////////

void Index::for_each_unit_func(const php::Unit& unit,
                               std::function<void(const php::Func&)> f) const {
  for (auto const func : unit.funcs) {
    f(*m_data->funcs.at(func));
  }
}

void Index::for_each_unit_func_mutable(php::Unit& unit,
                                       std::function<void(php::Func&)> f) {
  for (auto const func : unit.funcs) {
    f(*m_data->funcs.at(func));
  }
}

void Index::for_each_unit_class(
    const php::Unit& unit,
    std::function<void(const php::Class&)> f) const {
  for (auto const cls : unit.classes) {
    f(*m_data->classes.at(cls));
  }
}

void Index::for_each_unit_class_mutable(php::Unit& unit,
                                        std::function<void(php::Class&)> f) {
  for (auto const cls : unit.classes) {
    f(*m_data->classes.at(cls));
  }
}

//////////////////////////////////////////////////////////////////////

void Index::preresolve_type_structures() {
  trace_time tracer("pre-resolve type-structures", m_data->sample);

  // First resolve and update type-aliases. We do this first because
  // the resolutions may help us resolve the type-constants below
  // faster.
  struct TAUpdate {
    php::TypeAlias* typeAlias;
    SArray ts;
  };

  auto const taUpdates = parallel::map(
    m_data->program->units,
    [&] (const std::unique_ptr<php::Unit>& unit) {
      CompactVector<TAUpdate> updates;
      for (auto const& typeAlias : unit->typeAliases) {
        assertx(typeAlias->resolvedTypeStructure.isNull());
        if (auto const ts =
            resolve_type_structure(*this, nullptr, *typeAlias).sarray()) {
          updates.emplace_back(TAUpdate{ typeAlias.get(), ts });
        }
      }
      return updates;
    }
  );

  parallel::for_each(
    taUpdates,
    [&] (const CompactVector<TAUpdate>& updates) {
      for (auto const& u : updates) {
        assertx(u.ts->isStatic());
        assertx(u.ts->isDictType());
        assertx(!u.ts->empty());
        u.typeAlias->resolvedTypeStructure =
          Array::attach(const_cast<ArrayData*>(u.ts));
      }
    }
  );

  // Then do the type-constants. Here we not only resolve the
  // type-structures, we make a copy of each type-constant for each
  // class. The reason is that a type-structure may be resolved
  // differently for each class in the inheritance hierarchy (due to
  // this::). By making a separate copy for each class, we can resolve
  // the type-structure specifically for that class.
  struct CnsUpdate {
    ClassInfo* to;
    ClassInfo::ConstIndex from;
    php::Const cns;
  };

  auto const cnsUpdates = parallel::map(
    m_data->allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      CompactVector<CnsUpdate> updates;
      for (auto const& kv : cinfo->clsConstants) {
        auto const& cns = *kv.second;
        assertx(!cns.resolvedTypeStructure);
        if (!cns.val.has_value()) continue;
        if (cns.kind != ConstModifiers::Kind::Type) continue;
        assertx(tvIsDict(*cns.val));

        // If we can resolve it, schedule an update
        if (auto const resolved =
            resolve_type_structure(*this, cns, *cinfo->cls).sarray()) {
          auto newCns = cns;
          newCns.resolvedTypeStructure = resolved;
          updates.emplace_back(CnsUpdate{ cinfo.get(), kv.second, newCns });
        } else if (cinfo->cls != kv.second.cls) {
          // Even if we can't, we need to copy it anyways (unless it's
          // already in it's original location).
          updates.emplace_back(CnsUpdate{ cinfo.get(), kv.second, cns });
        }
      }
      return updates;
    }
  );

  parallel::for_each(
    cnsUpdates,
    [&] (const CompactVector<CnsUpdate>& updates) {
      for (auto const& u : updates) {
        assertx(u.cns.val.has_value());
        assertx(u.cns.kind == ConstModifiers::Kind::Type);

        if (u.to->cls == u.from.cls) {
          assertx(u.from.idx < u.to->cls->constants.size());
          const_cast<php::Class*>(u.to->cls)->constants[u.from.idx] = u.cns;
        } else {
          auto const idx = u.to->cls->constants.size();
          const_cast<php::Class*>(u.to->cls)->constants.emplace_back(u.cns);
          u.to->clsConstants.insert_or_assign(
            u.cns.name,
            ClassInfo::ConstIndex{ u.to->cls, (uint32_t)idx }
          );
        }
      }
    }
  );

  // Now that everything has been updated, calculate the invariance
  // for each resolved type-structure. For each class constant,
  // examine all subclasses and see how the resolved type-structure
  // changes.
  parallel::for_each(
    m_data->allClassInfos,
    [&] (std::unique_ptr<ClassInfo>& cinfo) {
      for (auto& cns : const_cast<php::Class*>(cinfo->cls)->constants) {
        assertx(cns.invariance == php::Const::Invariance::None);
        if (cns.kind != ConstModifiers::Kind::Type) continue;
        if (!cns.val.has_value()) continue;
        if (!cns.resolvedTypeStructure) continue;

        auto const checkClassname =
          tvIsString(cns.resolvedTypeStructure->get(s_classname));

        // Assume it doesn't change
        auto invariance = php::Const::Invariance::Same;
        for (auto const& s : cinfo->subclassList) {
          assertx(invariance != php::Const::Invariance::None);
          assertx(
            IMPLIES(!checkClassname,
                    invariance != php::Const::Invariance::ClassnamePresent)
          );
          if (s == cinfo.get()) continue;

          auto const it = s->clsConstants.find(cns.name);
          assertx(it != s->clsConstants.end());
          if (it->second.cls != s->cls) continue;
          auto const& scns = *it->second;

          // Overridden in some strange way. Be pessimistic.
          if (!scns.val.has_value() ||
              scns.kind != ConstModifiers::Kind::Type) {
            invariance = php::Const::Invariance::None;
            break;
          }

          // The resolved type structure in this subclass is not the
          // same.
          if (scns.resolvedTypeStructure != cns.resolvedTypeStructure) {
            if (!scns.resolvedTypeStructure) {
              // It's not even resolved here, so we can't assume
              // anything.
              invariance = php::Const::Invariance::None;
              break;
            }
            // We might still be able to assert that a classname is
            // always present, or a resolved type structure at least
            // exists.
            if (invariance == php::Const::Invariance::Same ||
                invariance == php::Const::Invariance::ClassnamePresent) {
              invariance =
                (checkClassname &&
                 tvIsString(scns.resolvedTypeStructure->get(s_classname)))
                ? php::Const::Invariance::ClassnamePresent
                : php::Const::Invariance::Present;
            }
          }
        }

        if (invariance != php::Const::Invariance::None) {
          cns.invariance = invariance;
        }
      }
    }
  );
}

//////////////////////////////////////////////////////////////////////

const CompactVector<const php::Class*>*
Index::lookup_closures(const php::Class* cls) const {
  auto const it = m_data->classClosureMap.find(cls);
  if (it != end(m_data->classClosureMap)) {
    return &it->second;
  }
  return nullptr;
}

const hphp_fast_set<const php::Func*>*
Index::lookup_extra_methods(const php::Class* cls) const {
  if (cls->attrs & AttrNoExpandTrait) return nullptr;
  auto const it = m_data->classExtraMethodMap.find(cls);
  if (it != end(m_data->classExtraMethodMap)) {
    return &it->second;
  }
  return nullptr;
}

//////////////////////////////////////////////////////////////////////

Optional<res::Class> Index::resolve_class(SString clsName) const {
  clsName = normalizeNS(clsName);

  auto const it = m_data->classInfo.find(clsName);
  if (it == end(m_data->classInfo)) return std::nullopt;
  auto const cinfo = it->second;

  if (is_closure_base(*cinfo->cls)) {
    // Force Closure to be unresolved
    return res::Class { clsName };
  }
  return res::Class { cinfo };
}

const php::TypeAlias* Index::lookup_type_alias(SString name) const {
  auto const it = m_data->typeAliases.find(name);
  if (it == m_data->typeAliases.end()) return nullptr;
  return it->second;
}

res::Class Index::wait_handle_class() const {
  return m_data->lazyWaitHandleCls.get(
    [&] {
      auto const awaitable = builtin_class(*this, s_Awaitable.get());
      auto const without = awaitable.withoutNonRegular();
      assertx(without.has_value());
      return *without;
    }
  );
}

// Given a DCls, return the most specific res::Func for that DCls. For
// intersections, this will call process/general on every component of
// the intersection and combine the results. For non-intersections, it
// will call process/general on the sole member of the DCls. process
// is called to obtain a res::Func from a ClassInfo. If a ClassInfo
// isn't available, general will be called instead.
template <typename P, typename G>
res::Func Index::rfunc_from_dcls(const DCls& dcls,
                                 SString name,
                                 const P& process,
                                 const G& general) const {
  if (!dcls.isIsect()) {
    // If this isn't an intersection, there's only one cinfo to
    // process and we're done.
    auto const cinfo = dcls.cls().val.right();
    if (!cinfo) return general(dcls.containsNonRegular());
    return process(cinfo, dcls.isExact(), dcls.containsNonRegular());
  }

  /*
   * Otherwise get a res::Func for all members of the intersection and
   * combine them together. Since the DCls represents a class which is
   * a subtype of every ClassInfo in the list, every res::Func we get
   * is true.
   *
   * The relevant res::Func types in order from most general to more
   * specific are:
   *
   * MethodName -> FuncFamily -> MethodOrMissing -> Method -> Missing
   *
   * Since every res::Func in the intersection is true, we take the
   * res::Func which is most specific. Two different res::Funcs cannot
   * be contradict. For example, we shouldn't get a Method and a
   * Missing since one implies there's no func and the other implies
   * one specific func. Or two different res::Funcs shouldn't resolve
   * to two different methods.
   */

  using Func = res::Func;

  auto missing = TriBool::Maybe;
  Func::Isect isect;
  const php::Func* singleMethod = nullptr;

  for (auto const i : dcls.isect()) {
    auto const cinfo = i.val.right();
    if (!cinfo) continue;

    auto const func = process(cinfo, false, dcls.containsNonRegular());
    match<void>(
      func.val,
      [&] (Func::MethodName) {},
      [&] (Func::Method m) {
        assertx(IMPLIES(singleMethod, singleMethod == m.finfo->func));
        assertx(IMPLIES(singleMethod, isect.families.empty()));
        assertx(missing != TriBool::Yes);
        if (!singleMethod) {
          singleMethod = m.finfo->func;
          isect.families.clear();
        }
        missing = TriBool::No;
      },
      [&] (Func::MethodFamily fam) {
        if (missing == TriBool::Yes) {
          assertx(!singleMethod);
          assertx(isect.families.empty());
          return;
        }
        if (singleMethod) {
          assertx(missing != TriBool::Yes);
          assertx(isect.families.empty());
          return;
        }
        assertx(missing == TriBool::Maybe);
        isect.families.emplace_back(fam.family);
        isect.regularOnly |= fam.regularOnly;
      },
      [&] (Func::MethodOrMissing m) {
        assertx(IMPLIES(singleMethod, singleMethod == m.finfo->func));
        assertx(IMPLIES(singleMethod, isect.families.empty()));
        if (missing == TriBool::Yes) {
          assertx(!singleMethod);
          assertx(isect.families.empty());
          return;
        }
        if (!singleMethod) {
          singleMethod = m.finfo->func;
          isect.families.clear();
        }
      },
      [&] (Func::Missing) {
        assertx(missing != TriBool::No);
        singleMethod = nullptr;
        isect.families.clear();
        missing = TriBool::Yes;
      },
      [&] (Func::FuncName)         { always_assert(false); },
      [&] (Func::Fun)              { always_assert(false); },
      [&] (Func::Fun2)             { always_assert(false); },
      [&] (Func::Method2)          { always_assert(false); },
      [&] (Func::MethodFamily2)    { always_assert(false); },
      [&] (Func::MethodOrMissing2) { always_assert(false); },
      [&] (const Func::Isect&)     { always_assert(false); },
      [&] (const Func::Isect2&)    { always_assert(false); }
    );
  }

  // If we got a method, that always wins. Again, every res::Func is
  // true, and method is more specific than a FuncFamily, so it is
  // preferred.
  if (singleMethod) {
    assertx(missing != TriBool::Yes);
    // If missing is Maybe, then *every* resolution was to a
    // MethodName or MethodOrMissing, so include that fact here by
    // using MethodOrMissing.
    if (missing == TriBool::Maybe) {
      return Func {
        Func::MethodOrMissing { func_info(*m_data, singleMethod) }
      };
    }
    return Func { Func::Method { func_info(*m_data, singleMethod) } };
  }
  // We only got unresolved classes. If missing is TriBool::Yes, the
  // function doesn't exist. Otherwise be pessimistic.
  if (isect.families.empty()) {
    if (missing == TriBool::Yes) return Func { Func::Missing { name } };
    assertx(missing == TriBool::Maybe);
    return general(dcls.containsNonRegular());
  }
  // Isect case. Isects always might contain missing funcs.
  assertx(missing == TriBool::Maybe);

  // We could add a FuncFamily multiple times, so remove duplicates.
  std::sort(begin(isect.families), end(isect.families));
  isect.families.erase(
    std::unique(begin(isect.families), end(isect.families)),
    end(isect.families)
  );
  // If everything simplifies down to a single FuncFamily, just use
  // that.
  if (isect.families.size() == 1) {
    return Func { Func::MethodFamily { isect.families[0], isect.regularOnly } };
  }
  return Func { std::move(isect) };
}

res::Func Index::resolve_method(Context ctx,
                                const Type& thisType,
                                SString name) const {
  assertx(thisType.subtypeOf(BCls) || thisType.subtypeOf(BObj));

  using Func = res::Func;

  /*
   * Without using the class type, try to infer a set of methods
   * using just the method name. This will, naturally, not produce
   * as precise a set as when using the class type, but it's better
   * than nothing. For all of these results, we need to include the
   * possibility of the method not existing (we cannot rule that out
   * for this situation).
   */
  auto const general = [&] (bool includeNonRegular) {
    assertx(name != s_construct.get());

    // We don't produce name-only global func families for special
    // methods, so be conservative. We don't call special methods in a
    // context where we'd expect to not know the class, so it's not
    // worthwhile. The same goes for __invoke and __debuginfo, which
    // is corresponds to every closure, and gets too large without
    // much value.
    if (!has_name_only_func_family(name)) {
      return Func { Func::MethodName { name } };
    }

    // Lookup up the name-only global func families for this name. If
    // we don't have one, the method cannot exist because it contains
    // every method with that name in the program.
    auto const famIt = m_data->methodFamilies.find(name);
    if (famIt == end(m_data->methodFamilies)) {
      return Func { Func::Missing { name } };
    }

    // The entry exists. Consult the correct data in it, depending on
    // whether we're including non-regular classes or not.
    auto const& entry = includeNonRegular
      ? famIt->second.m_all
      : famIt->second.m_regular;
    assertx(entry.isEmpty() || entry.isIncomplete());

    if (auto const ff = entry.funcFamily()) {
      return Func { Func::MethodFamily { ff, !includeNonRegular } };
    } else if (auto const f = entry.func()) {
      return Func { Func::MethodOrMissing { func_info(*m_data, f) } };
    } else {
      return Func { Func::Missing { name } };
    }
  };

  auto const process = [&] (ClassInfo* cinfo,
                            bool isExact,
                            bool includeNonRegular) {
    assertx(name != s_construct.get());

    auto const methIt = cinfo->methods.find(name);
    if (methIt == end(cinfo->methods)) {
      // We don't store metadata for special methods, so be
      // pessimistic (the lack of a method entry does not mean the
      // call might fail at runtme).
      if (is_special_method_name(name)) {
        return Func { Func::MethodName { name } };
      }
      // We're only considering this class, not it's subclasses. Since
      // it doesn't exist here, the resolution will always fail.
      if (isExact) return Func { Func::Missing { name } };
      // The method isn't present on this class, but it might be in
      // the subclasses. In most cases try a general lookup to get a
      // slightly better type than nothing.
      if (includeNonRegular ||
          !(cinfo->cls->attrs & (AttrInterface|AttrAbstract))) {
        return general(includeNonRegular);
      }

      // A special case is if we're only considering regular classes,
      // and this is an interface or abstract class. For those, we
      // "expand" the method families table to include any methods
      // defined in *all* regular subclasses. This is needed to
      // preserve monotonicity. Check this now.
      auto const famIt = cinfo->methodFamilies.find(name);
      // If no entry, treat it pessimistically like the rest of the
      // cases.
      if (famIt == end(cinfo->methodFamilies)) return general(false);

      // We found an entry. This cannot be empty (remember the method
      // is guaranteed to exist on *all* regular subclasses), and must
      // be complete (for the same reason). Use it.
      auto const& entry = famIt->second;
      assertx(!entry.isEmpty());
      assertx(entry.isComplete());
      if (auto const ff = entry.funcFamily()) {
        return Func { Func::MethodFamily { ff, true } };
      } else if (auto const func = entry.func()) {
        return Func { Func::Method { func_info(*m_data, func) } };
      } else {
        always_assert(false);
      }
    }
    // The method on this class.
    auto const& meth = methIt->second;
    auto const ftarget = func_from_meth_ref(*m_data, meth.meth());

    // We don't store method family information about special methods
    // and they have special inheritance semantics.
    if (is_special_method_name(name)) {
      // If we know the class exactly, we can use ftarget.
      if (isExact) return Func { Func::Method { func_info(*m_data, ftarget) } };
      // The method isn't overwritten, but they don't inherit, so it
      // could be missing.
      if (meth.attrs & AttrNoOverride) {
        return Func { Func::MethodOrMissing { func_info(*m_data, ftarget) } };
      }
      // Otherwise be pessimistic.
      return Func { Func::MethodName { name } };
    }

    // Private method handling: Private methods have special lookup
    // rules. If we're in the context of a particular class, and that
    // class defines a private method, an instance of the class will
    // always call that private method (even if overridden) in that
    // context.
    assertx(cinfo->cls);
    if (ctx.cls == cinfo->cls) {
      // The context matches the current class. If we've looked up a
      // private method (defined on this class), then that's what
      // we'll call.
      if ((meth.attrs & AttrPrivate) && meth.topLevel()) {
        return Func { Func::Method { func_info(*m_data, ftarget) } };
      }
    } else if ((meth.attrs & AttrPrivate) || meth.hasPrivateAncestor()) {
      // Otherwise the context doesn't match the current class. If the
      // looked up method is private, or has a private ancestor,
      // there's a chance we'll call that method (or
      // ancestor). Otherwise there's no private method in the
      // inheritance tree we'll call.
      auto const ancestor = [&] () -> const php::Func* {
        if (!ctx.cls) return nullptr;
        // Look up the ClassInfo corresponding to the context:
        auto const it = m_data->classInfo.find(ctx.cls->name);
        if (it == end(m_data->classInfo)) return nullptr;
        auto const ctxCInfo = it->second;
        // Is this context a parent of our class?
        if (!cinfo->derivedFrom(*ctxCInfo)) return nullptr;
        // It is. See if it defines a private method.
        auto const it2 = ctxCInfo->methods.find(name);
        if (it2 == end(ctxCInfo->methods)) return nullptr;
        auto const& mte = it2->second;
        // If it defines a private method, use it.
        if ((mte.attrs & AttrPrivate) && mte.topLevel()) {
          return func_from_meth_ref(*m_data, mte.meth());
        }
        // Otherwise do normal lookup.
        return nullptr;
      }();
      if (ancestor) {
        return Func { Func::Method { func_info(*m_data, ancestor) } };
      }
    }
    // If none of the above cases trigger, we still might call a
    // private method (in a child class), but the func-family logic
    // below will handle that.

    // If we're only including regular subclasses, and this class
    // itself isn't regular, the result may not necessarily include
    // ftarget.
    if (!includeNonRegular && !is_regular_class(*cinfo->cls)) {
      // We're not including this base class. If we're exactly this
      // class, there's no method at all. It will always be missing.
      if (isExact) return Func { Func::Missing { name } };
      if (meth.noOverrideRegular()) {
        // The method isn't overridden in a subclass, but we can't
        // use the base class either. This leaves two cases. Either
        // the method isn't overridden because there are no regular
        // subclasses (in which case there's no resolution at all), or
        // because there's regular subclasses, but they use the same
        // method (in which case the result is just ftarget).
        if (!cinfo->hasRegularSubclass) {
          return Func { Func::Missing { name } };
        }
        return Func { Func::Method { func_info(*m_data, ftarget) } };
      }
      // We can't use the base class (because it's non-regular), but
      // the method is overridden by a regular subclass.

      // Since this is a non-regular class and we want the result for
      // the regular subset, we need to consult the aux table first.
      auto const auxIt = cinfo->methodFamiliesAux.find(name);
      if (auxIt != end(cinfo->methodFamiliesAux)) {
        // Found an entry in the aux table. Use whatever it provides.
        auto const& aux = auxIt->second;
        if (auto const ff = aux.funcFamily()) {
          return Func { Func::MethodFamily { ff, true } };
        } else if (auto const f = aux.func()) {
          return aux.isComplete()
            ? Func { Func::Method { func_info(*m_data, f) } }
            : Func { Func::MethodOrMissing { func_info(*m_data, f) } };
        } else {
          return Func { Func::Missing { name } };
        }
      }
      // No entry in the aux table. The result is the same as the
      // normal table, so fall through and use that.
    } else if (isExact ||
               meth.attrs & AttrNoOverride ||
               (!includeNonRegular && meth.noOverrideRegular())) {
      // Either we want all classes, or the base class is regular. If
      // the method isn't overridden we know it must be just ftarget
      // (the override bits include it being missing in a subclass, so
      // we know it cannot be missing either).
      return Func { Func::Method { func_info(*m_data, ftarget) } };
    }

    // Look up the entry in the normal method family table and use
    // whatever is there.
    auto const famIt = cinfo->methodFamilies.find(name);
    assertx(famIt != end(cinfo->methodFamilies));
    auto const& fam = famIt->second;
    assertx(!fam.isEmpty());

    if (auto const ff = fam.funcFamily()) {
      return Func { Func::MethodFamily { ff, !includeNonRegular } };
    } else if (auto const f = fam.func()) {
      return (!includeNonRegular || fam.isComplete())
        ? Func { Func::Method { func_info(*m_data, f) } }
        : Func { Func::MethodOrMissing { func_info(*m_data, f) } };
    } else {
      always_assert(false);
    }
  };

  auto const isClass = thisType.subtypeOf(BCls);
  if (name == s_construct.get()) {
    if (isClass) return Func { Func::MethodName { s_construct.get() } };
    return resolve_ctor(thisType);
  }

  if (isClass) {
    if (!is_specialized_cls(thisType)) return general(true);
  } else if (!is_specialized_obj(thisType)) {
    return general(false);
  }
  return rfunc_from_dcls(
    isClass ? dcls_of(thisType) : dobj_of(thisType),
    name,
    process,
    general
  );
}

res::Func Index::resolve_ctor(const Type& obj) const {
  assertx(obj.subtypeOf(BObj));

  using Func = res::Func;

  // Can't say anything useful if we don't know the object type.
  if (!is_specialized_obj(obj)) {
    return Func { Func::MethodName { s_construct.get() } };
  }

  return rfunc_from_dcls(
    dobj_of(obj),
    s_construct.get(),
    [&] (ClassInfo* cinfo, bool isExact, bool includeNonRegular) {
      // We're dealing with an object here, which never uses
      // non-regular classes.
      assertx(!includeNonRegular);

      // See if this class has a ctor.
      auto const methIt = cinfo->methods.find(s_construct.get());
      if (methIt == end(cinfo->methods)) {
        // There's no ctor on this class. This doesn't mean the ctor
        // won't exist at runtime, it might get the default ctor, so
        // we have to be conservative.
        return Func { Func::MethodName { s_construct.get() } };
      }

      // We have a ctor, but it might be overridden in a subclass.
      auto const& mte = methIt->second;
      assertx(!(mte.attrs & AttrStatic));
      auto const ftarget = func_from_meth_ref(*m_data, mte.meth());
      assertx(!(ftarget->attrs & AttrStatic));

      // If this class is known exactly, or we know nothing overrides
      // this ctor, we know this ctor is precisely it.
      if (isExact || mte.noOverrideRegular()) {
        // If this class isn't regular, and doesn't have any regular
        // subclasses (or if it's exact), this resolution will always
        // fail.
        if (!is_regular_class(*cinfo->cls) &&
            (isExact || !cinfo->hasRegularSubclass)) {
          return Func { Func::Missing { s_construct.get() } };
        }
        return Func { Func::Method { func_info(*m_data, ftarget) } };
      }

      // If this isn't a regular class, we need to check the "aux"
      // entry first (which always has priority when only looking at
      // the regular subset).
      if (!is_regular_class(*cinfo->cls)) {
        auto const auxIt = cinfo->methodFamiliesAux.find(s_construct.get());
        if (auxIt != end(cinfo->methodFamiliesAux)) {
          auto const& aux = auxIt->second;
          if (auto const ff = aux.funcFamily()) {
            return Func { Func::MethodFamily { ff, true } };
          } else if (auto const f = aux.func()) {
            return aux.isComplete()
              ? Func { Func::Method { func_info(*m_data, f) } }
              : Func { Func::MethodOrMissing { func_info(*m_data, f) } };
          } else {
            // Ctor doesn't exist in any regular subclasses. This can
            // happen with interfaces. The ctor might get the default
            // ctor at runtime, so be conservative.
            return Func { Func::MethodName { s_construct.get() } };
          }
        }
      }
      // Otherwise this class is regular (in which case there's just
      // method families, or there's no entry in aux, which means the
      // regular subset entry is the same as the full entry.

      auto const famIt = cinfo->methodFamilies.find(s_construct.get());
      assertx(famIt != cinfo->methodFamilies.end());
      auto const& fam = famIt->second;
      assertx(!fam.isEmpty());

      if (auto const ff = fam.funcFamily()) {
        return Func { Func::MethodFamily { ff, true } };
      } else if (auto const f = fam.func()) {
        // Since we're looking at the regular subset, we can assume
        // the set is complete, regardless of the flag on fam.
        return Func { Func::Method { func_info(*m_data, f) } };
      } else {
        always_assert(false);
      }
    },
    [&] (bool includeNonRegular) {
      assertx(!includeNonRegular);
      return Func { Func::MethodName { s_construct.get() } };
    }
  );
}

res::Func Index::resolve_func(SString name) const {
  name = normalizeNS(name);
  auto const it = m_data->funcs.find(name);
  if (it == end(m_data->funcs)) return res::Func { res::Func::Missing { name } };
  auto const func = it->second;
  assertx(func->attrs & AttrPersistent);
  return res::Func { res::Func::Fun { func_info(*m_data, func) } };
}

res::Func Index::resolve_func_or_method(const php::Func& f) const {
  if (!f.cls) return res::Func { res::Func::Fun { func_info(*m_data, &f) } };
  return res::Func { res::Func::Method { func_info(*m_data, &f) } };
}

bool Index::func_depends_on_arg(const php::Func* func, size_t arg) const {
  auto const& finfo = *func_info(*m_data, func);
  return arg >= finfo.unusedParams.size() || !finfo.unusedParams.test(arg);
}

// Helper function: Given a DCls, visit every subclass it represents,
// passing it to the given callable. If the callable returns false,
// stop iteration. Return false if any of the classes is unresolved,
// true otherwise. This is used to simplify the below functions which
// need to iterate over all possible subclasses and union the results.
template <typename F>
bool Index::visit_every_dcls_cls(const DCls& dcls, const F& f) const {
  if (dcls.isExact()) {
    auto const cinfo = dcls.cls().val.right();
    if (!cinfo) return false;
    if (dcls.containsNonRegular() || is_regular_class(*cinfo->cls)) {
      f(cinfo);
    }
    return true;
  } else if (dcls.isSub()) {
    auto const cinfo = dcls.cls().val.right();
    if (!cinfo) return false;
    if (cinfo->cls->attrs & AttrTrait) {
      if (dcls.containsNonRegular()) f(cinfo);
    } else {
      assertx(!cinfo->subclassList.empty());
      for (auto const sub : cinfo->subclassList) {
        if (!dcls.containsNonRegular() && !is_regular_class(*sub->cls)) {
          continue;
        }
        if (!f(sub)) break;
      }
    }
    return true;
  }
  auto const& isect = dcls.isect();
  assertx(isect.size() > 1);

  auto unresolved = false;
  res::Class::visitEverySub(
    isect,
    dcls.containsNonRegular(),
    [&] (res::Class c, bool isExact) {
      if (auto const cinfo = c.val.right()) {
        assertx(isExact);
        return f(cinfo);
      }
      unresolved = true;
      return false;
    }
  );
  return !unresolved;
}

ClsConstLookupResult Index::lookup_class_constant(Context ctx,
                                                  const Type& cls,
                                                  const Type& name) const {
  ITRACE(4, "lookup_class_constant: ({}) {}::{}\n",
         show(ctx), show(cls), show(name));
  Trace::Indent _;

  using R = ClsConstLookupResult;

  auto const conservative = [] {
    ITRACE(4, "conservative\n");
    return R{ TInitCell, TriBool::Maybe, true };
  };

  auto const notFound = [] {
    ITRACE(4, "not found\n");
    return R{ TBottom, TriBool::No, false };
  };

  if (!is_specialized_cls(cls)) return conservative();

  // We could easily support the case where we don't know the constant
  // name, but know the class (like we do for properties), by unioning
  // together all possible constants. However it very rarely happens,
  // but when it does, the set of constants to union together can be
  // huge and it becomes very expensive.
  if (!is_specialized_string(name)) return conservative();
  auto const sname = sval_of(name);

  // If this lookup is safe to cache. Some classes can have a huge
  // number of subclasses and unioning together all possible constants
  // can become very expensive. We can aleviate some of this expense
  // by caching results. We cannot cache a result when we use 86cinit
  // analysis since that can change.
  auto cachable = true;

  auto const process = [&] (const ClassInfo* ci) {
    ITRACE(4, "{}:\n", ci->cls->name);
    Trace::Indent _;

    // Does the constant exist on this class?
    auto const it = ci->clsConstants.find(sname);
    if (it == ci->clsConstants.end()) return notFound();

    // Is it a value and is it non-abstract (we only deal with
    // concrete constants).
    auto const& cns = *it->second.get();
    if (cns.kind != ConstModifiers::Kind::Value) return notFound();
    if (!cns.val.has_value()) return notFound();

    auto const cnsIdx = it->second.idx;

    // Determine the constant's value and return it
    auto const r = [&] {
      if (cns.val->m_type == KindOfUninit) {
        // Constant is defined by a 86cinit. Use the result from
        // analysis and add a dependency. We cannot cache in this
        // case.
        cachable = false;
        auto const cnsCls = m_data->classes.at(cns.cls);
        if (ctx.func) {
          auto const cinit = cnsCls->methods.back().get();
          assertx(cinit->name == s_86cinit.get());
          add_dependency(*m_data, cinit, ctx, Dep::ClsConst);
        }

        ITRACE(4, "(dynamic)\n");
        auto const type = [&] {
          auto const cnsClsCi = folly::get_default(m_data->classInfo, cnsCls->name);
          if (!cnsClsCi || cnsIdx >= cnsClsCi->clsConstTypes.size()) {
            return TInitCell;
          }
          return cnsClsCi->clsConstTypes[cnsIdx].type;
        }();
        return R{ type, TriBool::Yes, true };
      }

      // Fully resolved constant with a known value
      auto const mightThrow = bool(ci->cls->attrs & AttrInternal);
      return R{ from_cell(*cns.val), TriBool::Yes, mightThrow };
    }();
    ITRACE(4, "-> {}\n", show(r));
    return r;
  };

  auto const& dcls = dcls_of(cls);
  if (dcls.isSub()) {
    // Before anything, look up this entry in the cache. We don't
    // bother with the cache for the exact case because it's quick and
    // there's little point.
    auto const cinfo = dcls.cls().val.right();
    if (!cinfo) return conservative();
    if (auto const it =
        m_data->clsConstLookupCache.find(std::make_pair(cinfo->cls, sname));
        it != m_data->clsConstLookupCache.end()) {
      ITRACE(4, "cache hit: {}\n", show(it->second));
      return it->second;
    }
  }

  Optional<R> result;
  auto const resolved = visit_every_dcls_cls(
    dcls,
    [&] (const ClassInfo* cinfo) {
      if (result) ITRACE(5, "-> {}\n", show(*result));
      auto r = process(cinfo);
      if (!result) {
        result.emplace(std::move(r));
      } else {
        *result |= r;
      }
      return true;
    }
  );
  if (!resolved) return conservative();
  assertx(result.has_value());

  // Save this for future lookups if we can
  if (dcls.isSub() && cachable) {
    auto const cinfo = dcls.cls().val.right();
    assertx(cinfo);
    m_data->clsConstLookupCache.emplace(
      std::make_pair(cinfo->cls, sname),
      *result
    );
  }

  ITRACE(4, "-> {}\n", show(*result));
  return *result;
}

std::vector<std::pair<SString, ClsConstInfo>>
Index::lookup_class_constants(const php::Class& cls) const {
  std::vector<std::pair<SString, ClsConstInfo>> out;
  out.reserve(cls.constants.size());

  auto const cinfo = folly::get_default(m_data->classInfo, cls.name);
  for (size_t i = 0, size = cls.constants.size(); i < size; ++i) {
    auto const& cns = cls.constants[i];
    if (cns.kind != ConstModifiers::Kind::Value) continue;
    if (!cns.val) continue;
    if (cns.val->m_type != KindOfUninit) {
      out.emplace_back(cns.name, ClsConstInfo{ from_cell(*cns.val), 0 });
    } else if (cinfo && i < cinfo->clsConstTypes.size()) {
      out.emplace_back(cns.name, cinfo->clsConstTypes[i]);
    } else {
      out.emplace_back(cns.name, ClsConstInfo{ TInitCell, 0 });
    }
  }
  return out;
}

ClsTypeConstLookupResult
Index::lookup_class_type_constant(
    const Type& cls,
    const Type& name,
    const ClsTypeConstLookupResolver& resolver) const {
  ITRACE(4, "lookup_class_type_constant: {}::{}\n", show(cls), show(name));
  Trace::Indent _;

  using R = ClsTypeConstLookupResult;

  auto const conservative = [] {
    ITRACE(4, "conservative\n");
    return R{
      TypeStructureResolution { TSDictN, true },
      TriBool::Maybe,
      TriBool::Maybe
    };
  };

  auto const notFound = [] {
    ITRACE(4, "not found\n");
    return R {
      TypeStructureResolution { TBottom, false },
      TriBool::No,
      TriBool::No
    };
  };

  // Unlike lookup_class_constant, we distinguish abstract from
  // not-found, as the runtime sometimes treats them differently.
  auto const abstract = [] {
    ITRACE(4, "abstract\n");
    return R {
      TypeStructureResolution { TBottom, false },
      TriBool::No,
      TriBool::Yes
    };
  };

  if (!is_specialized_cls(cls)) return conservative();

  // As in lookup_class_constant, we could handle this, but it's not
  // worth it.
  if (!is_specialized_string(name)) return conservative();
  auto const sname = sval_of(name);

  auto const process = [&] (const ClassInfo* ci) {
    ITRACE(4, "{}:\n", ci->cls->name);
    Trace::Indent _;

    // Does the type constant exist on this class?
    auto const it = ci->clsConstants.find(sname);
    if (it == ci->clsConstants.end()) return notFound();

    // Is it an actual non-abstract type-constant?
    auto const& cns = *it->second;
    if (cns.kind != ConstModifiers::Kind::Type) return notFound();
    if (!cns.val.has_value()) return abstract();

    assertx(tvIsDict(*cns.val));
    ITRACE(4, "({}) {}\n", cns.cls, show(dict_val(val(*cns.val).parr)));

    // If we've been given a resolver, use it. Otherwise resolve it in
    // the normal way.
    auto resolved = resolver
      ? resolver(cns, *ci->cls)
      : resolve_type_structure(*this, cns, *ci->cls);

    // The result of resolve_type_structure isn't, in general,
    // static. However a type-constant will always be, so force that
    // here.
    assertx(resolved.type.is(BBottom) || resolved.type.couldBe(BUnc));
    resolved.type &= TUnc;
    auto const r = R{
      std::move(resolved),
      TriBool::Yes,
      TriBool::No
    };
    ITRACE(4, "-> {}\n", show(r));
    return r;
  };

  auto const& dcls = dcls_of(cls);

  Optional<R> result;
  auto const resolved = visit_every_dcls_cls(
    dcls,
    [&] (const ClassInfo* cinfo) {
      if (result) {
        ITRACE(5, "-> {}\n", show(*result));
      }
      auto r = process(cinfo);
      if (!result) {
        result.emplace(std::move(r));
      } else {
        *result |= r;
      }
      return true;
    }
  );
  if (!resolved) return conservative();
  assertx(result.has_value());

  ITRACE(4, "-> {}\n", show(*result));
  return *result;
}

Type Index::lookup_constant(Context ctx, SString cnsName) const {
  auto iter = m_data->constants.find(cnsName);
  if (iter == end(m_data->constants)) return TBottom;

  auto constant = iter->second;
  if (type(constant->val) != KindOfUninit) {
    return from_cell(constant->val);
  }

  // Assume a runtime call to Constant::get(), which will invoke
  // 86cinit_<cnsName>(). Look up it's return type.

  auto const func_name = Constant::funcNameFromName(cnsName);
  assertx(func_name && "func_name will never be nullptr");

  auto rfunc = resolve_func(func_name);
  assertx(rfunc.exactFunc());

  return lookup_return_type(ctx, nullptr, rfunc, Dep::ConstVal).t;
}

Index::ReturnType
Index::lookup_foldable_return_type(Context ctx,
                                   const CallContext& calleeCtx) const {
  auto const func = calleeCtx.callee;
  constexpr auto max_interp_nexting_level = 2;
  static __thread uint32_t interp_nesting_level;

  using R = ReturnType;

  auto const ctxType = adjust_closure_context(*this, calleeCtx);

  // Don't fold functions when staticness mismatches
  if (!func->isClosureBody) {
    if ((func->attrs & AttrStatic) && ctxType.couldBe(TObj)) {
      return R{ TInitCell, false };
    }
    if (!(func->attrs & AttrStatic) && ctxType.couldBe(TCls)) {
      return R{ TInitCell, false };
    }
  }

  auto const& finfo = *func_info(*m_data, func);
  if (finfo.effectFree && is_scalar(finfo.returnTy)) {
    return R{ finfo.returnTy, true };
  }

  auto showArgs DEBUG_ONLY = [] (const CompactVector<Type>& a) {
    std::string ret, sep;
    for (auto& arg : a) {
      folly::format(&ret, "{}{}", sep, show(arg));
      sep = ",";
    };
    return ret;
  };

  {
    ContextRetTyMap::const_accessor acc;
    if (m_data->foldableReturnTypeMap.find(acc, calleeCtx)) {
      FTRACE_MOD(
        Trace::hhbbc, 4,
        "Found foldableReturnType for {}{}{} with args {} (hash: {})\n",
        func->cls ? func->cls->name : staticEmptyString(),
        func->cls ? "::" : "",
        func->name,
        showArgs(calleeCtx.args),
        CallContextHashCompare{}.hash(calleeCtx));

      assertx(is_scalar(acc->second.t));
      assertx(acc->second.effectFree);
      return acc->second;
    }
  }

  if (frozen()) {
    FTRACE_MOD(
      Trace::hhbbc, 4,
      "MISSING: foldableReturnType for {}{}{} with args {} (hash: {})\n",
      func->cls ? func->cls->name : staticEmptyString(),
      func->cls ? "::" : "",
      func->name,
      showArgs(calleeCtx.args),
      CallContextHashCompare{}.hash(calleeCtx));
    return R{ TInitCell, false };
  }

  if (interp_nesting_level > max_interp_nexting_level) {
    add_dependency(*m_data, func, ctx, Dep::InlineDepthLimit);
    return R{ TInitCell, false };
  }

  auto const contextType = [&] {
    ++interp_nesting_level;
    SCOPE_EXIT { --interp_nesting_level; };

    auto const wf = php::WideFunc::cns(func);
    auto const fa = analyze_func_inline(
      *this,
      AnalysisContext { func->unit, wf, func->cls, &ctx.forDep() },
      ctxType,
      calleeCtx.args,
      nullptr,
      CollectionOpts::EffectFreeOnly
    );
    return R{
      fa.effectFree ? fa.inferredReturn : TInitCell,
      fa.effectFree
    };
  }();

  if (!is_scalar(contextType.t)) return R{ TInitCell, false };

  ContextRetTyMap::accessor acc;
  if (m_data->foldableReturnTypeMap.insert(acc, calleeCtx)) {
    acc->second = contextType;
  } else {
    // someone beat us to it
    assertx(acc->second.t == contextType.t);
  }
  return contextType;
}

Index::ReturnType Index::lookup_return_type(Context ctx,
                                            MethodsInfo* methods,
                                            res::Func rfunc,
                                            Dep dep) const {
  using R = ReturnType;

  auto const funcFamily = [&] (FuncFamily* fam, bool regularOnly) {
    add_dependency(*m_data, fam, ctx, dep);
    return fam->infoFor(regularOnly).m_returnTy.get(
      [&] {
        auto ret = TBottom;
        auto effectFree = true;
        for (auto const pf : fam->possibleFuncs()) {
          if (regularOnly && !pf.inRegular()) continue;
          auto const finfo = func_info(*m_data, pf.ptr());
          if (!finfo->func) return R{ TInitCell, false };
          ret |= unctx(finfo->returnTy);
          effectFree &= finfo->effectFree;
          if (!ret.strictSubtypeOf(BInitCell) && !effectFree) break;
        }
        return R{ std::move(ret), effectFree };
      }
    );
  };
  auto const meth = [&] (const php::Func* func) {
    if (methods) {
      if (auto ret = methods->lookupReturnType(*func)) {
        return R{ unctx(std::move(ret->t)), ret->effectFree };
      }
    }
    add_dependency(*m_data, func, ctx, dep);
    auto const finfo = func_info(*m_data, func);
    if (!finfo->func) return R{ TInitCell, false };
    return R{ unctx(finfo->returnTy), finfo->effectFree };
  };

  return match<R>(
    rfunc.val,
    [&] (res::Func::FuncName)   { return R{ TInitCell, false }; },
    [&] (res::Func::MethodName) { return R{ TInitCell, false }; },
    [&] (res::Func::Fun f) {
      add_dependency(*m_data, f.finfo->func, ctx, dep);
      return R{ unctx(f.finfo->returnTy), f.finfo->effectFree };
    },
    [&] (res::Func::Method m)          { return meth(m.finfo->func); },
    [&] (res::Func::MethodFamily fam)  {
      return funcFamily(fam.family, fam.regularOnly);
    },
    [&] (res::Func::MethodOrMissing m) { return meth(m.finfo->func); },
    [&] (res::Func::Missing)           { return R{ TBottom, false }; },
    [&] (const res::Func::Isect& i) {
      auto ty = TInitCell;
      auto anyEffectFree = false;
      for (auto const ff : i.families) {
        auto const [t, e] = funcFamily(ff, i.regularOnly);
        ty &= t;
        if (e) anyEffectFree = true;
      }
      return R{ std::move(ty), anyEffectFree };
    },
    [&] (res::Func::Fun2)             -> R { always_assert(false); },
    [&] (res::Func::Method2)          -> R { always_assert(false); },
    [&] (res::Func::MethodFamily2)    -> R { always_assert(false); },
    [&] (res::Func::MethodOrMissing2) -> R { always_assert(false); },
    [&] (res::Func::Isect2&)          -> R { always_assert(false); }
  );
}

Index::ReturnType Index::lookup_return_type(Context caller,
                                            MethodsInfo* methods,
                                            const CompactVector<Type>& args,
                                            const Type& context,
                                            res::Func rfunc,
                                            Dep dep) const {
  using R = ReturnType;

  auto const funcFamily = [&] (FuncFamily* fam, bool regularOnly) {
    add_dependency(*m_data, fam, caller, dep);
    auto ret = fam->infoFor(regularOnly).m_returnTy.get(
      [&] {
        auto ty = TBottom;
        auto effectFree = true;
        for (auto const pf : fam->possibleFuncs()) {
          if (regularOnly && !pf.inRegular()) continue;
          auto const finfo = func_info(*m_data, pf.ptr());
          if (!finfo->func) return R{ TInitCell, false };
          ty |= finfo->returnTy;
          effectFree &= finfo->effectFree;
          if (!ty.strictSubtypeOf(BInitCell) && !effectFree) break;
        }
        return R{ std::move(ty), effectFree };
      }
    );
    return R{
      return_with_context(std::move(ret.t), context),
      ret.effectFree
    };
  };
  auto const meth = [&] (const php::Func* func) {
    auto const finfo = func_info(*m_data, func);
    if (!finfo->func) return R{ TInitCell, false };

    auto returnType = [&] {
      if (methods) {
        if (auto ret = methods->lookupReturnType(*func)) {
          return *ret;
        }
      }
      add_dependency(*m_data, func, caller, dep);
      return R{ finfo->returnTy, finfo->effectFree };
    }();

    return context_sensitive_return_type(
      *m_data,
      caller,
      { finfo->func, args, context },
      std::move(returnType)
    );
  };

  return match<R>(
    rfunc.val,
    [&] (res::Func::FuncName) {
      return lookup_return_type(caller, methods, rfunc, dep);
    },
    [&] (res::Func::MethodName) {
      return lookup_return_type(caller, methods, rfunc, dep);
    },
    [&] (res::Func::Fun f) {
      add_dependency(*m_data, f.finfo->func, caller, dep);
      return context_sensitive_return_type(
        *m_data,
        caller,
        { f.finfo->func, args, context },
        R{ f.finfo->returnTy, f.finfo->effectFree }
      );
    },
    [&] (res::Func::Method m)          { return meth(m.finfo->func); },
    [&] (res::Func::MethodFamily fam)  {
      return funcFamily(fam.family, fam.regularOnly);
    },
    [&] (res::Func::MethodOrMissing m) { return meth(m.finfo->func); },
    [&] (res::Func::Missing)           { return R{ TBottom, false }; },
    [&] (const res::Func::Isect& i) {
      auto ty = TInitCell;
      auto anyEffectFree = false;
      for (auto const ff : i.families) {
        auto const [t, e] = funcFamily(ff, i.regularOnly);
        ty &= t;
        if (e) anyEffectFree = true;
      }
      return R{ std::move(ty), anyEffectFree };
    },
    [&] (res::Func::Fun2)             -> R { always_assert(false); },
    [&] (res::Func::Method2)          -> R { always_assert(false); },
    [&] (res::Func::MethodFamily2)    -> R { always_assert(false); },
    [&] (res::Func::MethodOrMissing2) -> R { always_assert(false); },
    [&] (res::Func::Isect2&)          -> R { always_assert(false); }
  );
}

std::pair<Index::ReturnType, size_t>
Index::lookup_return_type_raw(const php::Func* f) const {
  auto it = func_info(*m_data, f);
  if (it->func) {
    assertx(it->func == f);
    return {
      ReturnType{ it->returnTy, it->effectFree },
      it->returnRefinements
    };
  }
  return { ReturnType{ TInitCell, false }, 0 };
}

CompactVector<Type>
Index::lookup_closure_use_vars(const php::Func* func,
                               bool move) const {
  assertx(func->isClosureBody);

  auto const numUseVars = closure_num_use_vars(func);
  if (!numUseVars) return {};
  auto const it = m_data->closureUseVars.find(func->cls);
  if (it == end(m_data->closureUseVars)) {
    return CompactVector<Type>(numUseVars, TCell);
  }
  if (move) return std::move(it->second);
  return it->second;
}

PropState
Index::lookup_private_props(const php::Class* cls,
                            bool move) const {
  auto it = m_data->privatePropInfo.find(cls);
  if (it != end(m_data->privatePropInfo)) {
    if (move) return std::move(it->second);
    return it->second;
  }
  return make_unknown_propstate(
    *this,
    cls,
    [&] (const php::Prop& prop) {
      return (prop.attrs & AttrPrivate) && !(prop.attrs & AttrStatic);
    }
  );
}

PropState
Index::lookup_private_statics(const php::Class* cls,
                              bool move) const {
  auto it = m_data->privateStaticPropInfo.find(cls);
  if (it != end(m_data->privateStaticPropInfo)) {
    if (move) return std::move(it->second);
    return it->second;
  }
  return make_unknown_propstate(
    *this,
    cls,
    [&] (const php::Prop& prop) {
      return (prop.attrs & AttrPrivate) && (prop.attrs & AttrStatic);
    }
  );
}

PropState Index::lookup_public_statics(const php::Class* cls) const {
  auto const cinfo = [&] () -> const ClassInfo* {
    auto const it = m_data->classInfo.find(cls->name);
    if (it == end(m_data->classInfo)) return nullptr;
    return it->second;
  }();

  PropState state;
  for (auto const& prop : cls->properties) {
    if (!(prop.attrs & (AttrPublic|AttrProtected)) ||
        !(prop.attrs & AttrStatic)) {
      continue;
    }

    auto [ty, everModified] = [&] {
      if (!cinfo) return std::make_pair(TInitCell, true);

      if (!m_data->seenPublicSPropMutations) {
        return std::make_pair(
          union_of(
            adjust_type_for_prop(
              *this,
              *cls,
              &prop.typeConstraint,
              TInitCell
            ),
            initial_type_for_public_sprop(*this, *cls, prop)
          ),
          true
        );
      }

      auto const it = cinfo->publicStaticProps.find(prop.name);
      if (it == end(cinfo->publicStaticProps)) {
        return std::make_pair(
          initial_type_for_public_sprop(*this, *cls, prop),
          false
        );
      }
      return std::make_pair(
        it->second.inferredType,
        it->second.everModified
      );
    }();
    state.emplace(
      prop.name,
      PropStateElem{
        std::move(ty),
        &prop.typeConstraint,
        prop.attrs,
        everModified
      }
    );
  }
  return state;
}

/*
 * Entry point for static property lookups from the Index. Return
 * metadata about a `cls'::`name' static property access in the given
 * context.
 */
PropLookupResult Index::lookup_static(Context ctx,
                                      const PropertiesInfo& privateProps,
                                      const Type& cls,
                                      const Type& name) const {
  ITRACE(4, "lookup_static: {} {}::${}\n", show(ctx), show(cls), show(name));
  Trace::Indent _;

  using R = PropLookupResult;

  // First try to obtain the property name as a static string
  auto const sname = [&] () -> SString {
    // Treat non-string names conservatively, but the caller should be
    // checking this.
    if (!is_specialized_string(name)) return nullptr;
    return sval_of(name);
  }();

  // Conservative result when we can't do any better. The type can be
  // anything, and anything might throw.
  auto const conservative = [&] {
    ITRACE(4, "conservative\n");
    return R{
      TInitCell,
      sname,
      TriBool::Maybe,
      TriBool::Maybe,
      TriBool::Maybe,
      TriBool::Maybe,
      TriBool::Maybe,
      true
    };
  };

  // If we don't know what `cls' is, there's not much we can do.
  if (!is_specialized_cls(cls)) return conservative();

  // Turn the context class into a ClassInfo* for convenience.
  const ClassInfo* ctxCls = nullptr;
  if (ctx.cls) {
    // I don't think this can ever fail (we should always be able to
    // resolve the class since we're currently processing it). If it
    // does, be conservative.
    auto const rCtx = resolve_class(ctx.cls->name);
    if (!rCtx || rCtx->val.left()) return conservative();
    ctxCls = rCtx->val.right();
  }

  auto const& dcls = dcls_of(cls);
  auto const start = dcls.cls();

  Optional<R> result;
  auto const resolved = visit_every_dcls_cls(
    dcls,
    [&] (const ClassInfo* cinfo) {
      auto r = lookup_static_impl(
        *m_data,
        ctx,
        ctxCls,
        privateProps,
        cinfo,
        sname,
        dcls.isSub() && !sname && cinfo != start.val.right()
      );
      ITRACE(4, "{} -> {}\n", cinfo->cls->name, show(r));
      if (!result) {
        result.emplace(std::move(r));
      } else {
        *result |= r;
      }
      return true;
    }
  );
  if (!resolved) return conservative();
  assertx(result.has_value());

  ITRACE(4, "union -> {}\n", show(*result));
  return *result;
}

Type Index::lookup_public_prop(const Type& obj, const Type& name) const {
  if (!is_specialized_obj(obj)) return TCell;

  if (!is_specialized_string(name)) return TCell;
  auto const sname = sval_of(name);

  auto ty = TBottom;
  auto const resolved = visit_every_dcls_cls(
    dobj_of(obj),
    [&] (const ClassInfo* cinfo) {
      ty |= lookup_public_prop_impl(
        *m_data,
        cinfo,
        sname
      );
      return ty.strictSubtypeOf(TCell);
    }
  );
  if (!resolved) return TCell;
  return ty;
}

Type Index::lookup_public_prop(const php::Class* cls, SString name) const {
  auto const it = m_data->classInfo.find(cls->name);
  if (it == end(m_data->classInfo)) {
    return TCell;
  }
  return lookup_public_prop_impl(*m_data, it->second, name);
}

bool Index::lookup_class_init_might_raise(Context ctx, res::Class cls) const {
  return cls.val.match(
    []  (SString) { return true; },
    [&] (ClassInfo* cinfo) {
      return class_init_might_raise(*m_data, ctx, cinfo);
    }
  );
}

Slot
Index::lookup_iface_vtable_slot(const php::Class* cls) const {
  return folly::get_default(m_data->ifaceSlotMap, cls->name, kInvalidSlot);
}

//////////////////////////////////////////////////////////////////////

/*
 * Entry point for static property type mutation from the Index. Merge
 * `val' into the known type for any accessible `cls'::`name' static
 * property. The mutation will be recovered into either
 * `publicMutations' or `privateProps' depending on the properties
 * found. Mutations to AttrConst properties are ignored, unless
 * `ignoreConst' is true.
 */
PropMergeResult Index::merge_static_type(
    Context ctx,
    PublicSPropMutations& publicMutations,
    PropertiesInfo& privateProps,
    const Type& cls,
    const Type& name,
    const Type& val,
    bool checkUB,
    bool ignoreConst,
    bool mustBeReadOnly) const {
  ITRACE(
    4, "merge_static_type: {} {}::${} {}\n",
    show(ctx), show(cls), show(name), show(val)
  );
  Trace::Indent _;

  assertx(val.subtypeOf(BInitCell));

  using R = PropMergeResult;

  // In some cases we might try to merge Bottom if we're in
  // unreachable code. This won't affect anything, so just skip out
  // early.
  if (val.subtypeOf(BBottom)) return R{ TBottom, TriBool::No };

  // Try to turn the given property name into a static string
  auto const sname = [&] () -> SString {
    // Non-string names are treated conservatively here. The caller
    // should be checking for these and doing the right thing.
    if (!is_specialized_string(name)) return nullptr;
    return sval_of(name);
  }();

  // The case where we don't know `cls':
  auto const unknownCls = [&] {
    if (!sname) {
      // Very bad case. We don't know `cls' or the property name. This
      // mutation can be affecting anything, so merge it into all
      // properties (this drops type information for public
      // properties).
      ITRACE(4, "unknown class and prop. merging everything\n");
      publicMutations.mergeUnknown(ctx);
      privateProps.mergeInAllPrivateStatics(
        *this, unctx(val), ignoreConst, mustBeReadOnly
      );
    } else {
      // Otherwise we don't know `cls', but do know the property
      // name. We'll store this mutation separately and union it in to
      // any lookup with the same name.
      ITRACE(4, "unknown class. merging all props with name {}\n", sname);

      publicMutations.mergeUnknownClass(sname, unctx(val));

      // Assume that it could possibly affect any private property with
      // the same name.
      privateProps.mergeInPrivateStatic(
        *this, sname, unctx(val), ignoreConst, mustBeReadOnly
      );
    }

    // To be conservative, say we might throw and be conservative about
    // conversions.
    return PropMergeResult{
      loosen_likeness(val),
      TriBool::Maybe
    };
  };

  // check if we can determine the class.
  if (!is_specialized_cls(cls)) return unknownCls();

  const ClassInfo* ctxCls = nullptr;
  if (ctx.cls) {
    auto const rCtx = resolve_class(ctx.cls->name);
    // We should only be not able to resolve our own context if the
    // class is not instantiable. In that case, the merge can't
    // happen.
    if (!rCtx) return R{ TBottom, TriBool::No };
    if (rCtx->val.left()) return unknownCls();
    ctxCls = rCtx->val.right();
  }

  auto const mergePublic = [&] (const ClassInfo* ci,
                                const php::Prop& prop,
                                const Type& val) {
    publicMutations.mergeKnown(ci, prop, val);
  };

  auto const& dcls = dcls_of(cls);
  Optional<res::Class> start;
  if (!dcls.isIsect()) start = dcls.cls();

  Optional<R> result;
  auto const resolved = visit_every_dcls_cls(
    dcls,
    [&] (const ClassInfo* cinfo) {
      auto r = merge_static_type_impl(
        *m_data,
        ctx,
        mergePublic,
        privateProps,
        ctxCls,
        cinfo,
        sname,
        val,
        checkUB,
        ignoreConst,
        mustBeReadOnly,
        dcls.isSub() && !sname && cinfo != start->val.right()
      );
      ITRACE(4, "{} -> {}\n", cinfo->cls->name, show(r));
      if (!result) {
        result.emplace(std::move(r));
      } else {
        *result |= r;
      }
      return true;
    }
  );
  if (!resolved) return unknownCls();
  assertx(result.has_value());
  ITRACE(4, "union -> {}\n", show(*result));
  return *result;
}

//////////////////////////////////////////////////////////////////////

DependencyContext Index::dependency_context(const Context& ctx) const {
  return dep_context(*m_data, ctx);
}

bool Index::using_class_dependencies() const {
  return m_data->useClassDependencies;
}

void Index::use_class_dependencies(bool f) {
  if (f != m_data->useClassDependencies) {
    m_data->dependencyMap.clear();
    m_data->useClassDependencies = f;
  }
}

void Index::refine_class_constants(const Context& ctx,
                                   const ResolvedConstants& resolved,
                                   DependencyContextSet& deps) {
  if (resolved.empty()) return;

  auto changed = false;
  auto const cls = ctx.func->cls;
  assertx(cls);
  auto& constants = cls->constants;

  for (auto& c : resolved) {
    assertx(c.first < constants.size());
    auto& cnst = constants[c.first];
    assertx(cnst.kind == ConstModifiers::Kind::Value);

    always_assert(cnst.val && type(*cnst.val) == KindOfUninit);
    if (auto const val = tv(c.second.type)) {
      assertx(val->m_type != KindOfUninit);
      cnst.val = *val;
      // Deleting from the types map is too expensive, so just leave
      // any entry. We won't look at it if val is set.
      changed = true;
    } else if (auto const cinfo =
               folly::get_default(m_data->classInfo, cls->name)) {
      auto& old = [&] () -> ClsConstInfo& {
        if (c.first >= cinfo->clsConstTypes.size()) {
          auto const newSize = std::max(c.first+1, resolved.back().first+1);
          cinfo->clsConstTypes.resize(newSize, ClsConstInfo { TInitCell, 0 });
        }
        return cinfo->clsConstTypes[c.first];
      }();

      if (c.second.type.strictlyMoreRefined(old.type)) {
        always_assert(c.second.refinements > old.refinements);
        old = std::move(c.second);
        changed = true;
      } else {
        always_assert_flog(
          c.second.type.moreRefined(old.type),
          "Class constant type invariant violated for {}::{}\n"
          "    {} is not at least as refined as {}\n",
          ctx.func->cls->name,
          cnst.name,
          show(c.second.type),
          show(old.type)
        );
      }
    }
  }

  if (changed) {
    find_deps(*m_data, ctx.func, Dep::ClsConst, deps);
  }
}

void Index::refine_constants(const FuncAnalysisResult& fa,
                             DependencyContextSet& deps) {
  auto const& func = fa.ctx.func;
  if (func->cls) return;

  auto const cns_name = Constant::nameFromFuncName(func->name);
  if (!cns_name) return;

  auto const cns = m_data->constants.at(cns_name);
  auto const val = tv(fa.inferredReturn);
  if (!val) {
    always_assert_flog(
      type(cns->val) == KindOfUninit,
      "Constant value invariant violated in {}.\n"
      "    Value went from {} to {}",
      cns_name,
      show(from_cell(cns->val)),
      show(fa.inferredReturn)
    );
    return;
  }

  if (type(cns->val) != KindOfUninit) {
    always_assert_flog(
      from_cell(cns->val) == fa.inferredReturn,
      "Constant value invariant violated in {}.\n"
      "    Value went from {} to {}",
      cns_name,
      show(from_cell(cns->val)),
      show(fa.inferredReturn)
    );
  } else {
    cns->val = *val;
  }

  find_deps(*m_data, func, Dep::ConstVal, deps);
}

void Index::refine_return_info(const FuncAnalysisResult& fa,
                               DependencyContextSet& deps) {
  auto const& func = fa.ctx.func;
  auto const finfo = func_info(*m_data, func);

  auto const error_loc = [&] {
    return folly::sformat(
      "{} {}{}",
      func->unit,
      func->cls
        ? folly::to<std::string>(func->cls->name->data(), "::")
        : std::string{},
      func->name
    );
  };

  auto dep = Dep{};
  if (finfo->retParam == NoLocalId && fa.retParam != NoLocalId) {
    // This is just a heuristic; it doesn't mean that the value passed
    // in was returned, but that the value of the parameter at the
    // point of the RetC was returned. We use it to make (heuristic)
    // decisions about whether to do inline interps, so we only allow
    // it to change once (otherwise later passes might not do the
    // inline interp, and get worse results, which could trigger other
    // assertions in Index::refine_*).
    dep = Dep::ReturnTy;
    finfo->retParam = fa.retParam;
  }

  auto unusedParams = ~fa.usedParams;
  if (finfo->unusedParams != unusedParams) {
    dep = Dep::ReturnTy;
    always_assert_flog(
        (finfo->unusedParams | unusedParams) == unusedParams,
        "Index unusedParams decreased in {}.\n",
        error_loc()
    );
    finfo->unusedParams = unusedParams;
  }

  auto resetFuncFamilies = false;
  if (fa.inferredReturn.strictlyMoreRefined(finfo->returnTy)) {
    if (finfo->returnRefinements < options.returnTypeRefineLimit) {
      finfo->returnTy = fa.inferredReturn;
      // We've modifed the return type, so reset any cached FuncFamily
      // return types.
      resetFuncFamilies = true;
      dep = is_scalar(fa.inferredReturn)
        ? Dep::ReturnTy | Dep::InlineDepthLimit : Dep::ReturnTy;
      finfo->returnRefinements += fa.localReturnRefinements + 1;
      if (finfo->returnRefinements > options.returnTypeRefineLimit) {
        FTRACE(1, "maxed out return type refinements at {}\n", error_loc());
      }
    } else {
      FTRACE(1, "maxed out return type refinements at {}\n", error_loc());
    }
  } else {
    always_assert_flog(
      fa.inferredReturn.moreRefined(finfo->returnTy),
      "Index return type invariant violated in {}.\n"
      "   {} is not at least as refined as {}\n",
      error_loc(),
      show(fa.inferredReturn),
      show(finfo->returnTy)
    );
  }

  always_assert_flog(
    !finfo->effectFree || fa.effectFree,
    "Index effectFree changed from true to false in {} {}.\n",
    func->unit,
    func_fullname(*func)
  );

  if (finfo->effectFree != fa.effectFree) {
    finfo->effectFree = fa.effectFree;
    dep = Dep::InlineDepthLimit | Dep::ReturnTy;
  }

  if (dep != Dep{}) {
    find_deps(*m_data, func, dep, deps);
    if (resetFuncFamilies) {
      assertx(has_dep(dep, Dep::ReturnTy));
      for (auto const ff : finfo->families) {
        // Reset the cached return type information for all the
        // FuncFamilies this function is a part of. Always reset the
        // "all" information, and if there's regular subset
        // information, reset that too.
        if (!ff->m_all.m_returnTy.reset() &&
            (!ff->m_regular || !ff->m_regular->m_returnTy.reset())) {
          continue;
        }
        // Only load the deps for this func family if we're the ones
        // who successfully reset. Only one thread needs to do it.
        find_deps(*m_data, ff, Dep::ReturnTy, deps);
      }
    }
  }
}

bool Index::refine_closure_use_vars(const php::Class* cls,
                                    const CompactVector<Type>& vars) {
  assertx(is_closure(*cls));

  for (auto i = uint32_t{0}; i < vars.size(); ++i) {
    always_assert_flog(
      vars[i].equivalentlyRefined(unctx(vars[i])),
      "Closure cannot have a used var with a context dependent type"
    );
  }

  auto& current = [&] () -> CompactVector<Type>& {
    std::lock_guard<std::mutex> _{closure_use_vars_mutex};
    return m_data->closureUseVars[cls];
  }();

  always_assert(current.empty() || current.size() == vars.size());
  if (current.empty()) {
    current = vars;
    return true;
  }

  auto changed = false;
  for (auto i = uint32_t{0}; i < vars.size(); ++i) {
    if (vars[i].strictSubtypeOf(current[i])) {
      changed = true;
      current[i] = vars[i];
    } else {
      always_assert_flog(
        vars[i].moreRefined(current[i]),
        "Index closure_use_var invariant violated in {}.\n"
        "   {} is not at least as refined as {}\n",
        cls->name,
        show(vars[i]),
        show(current[i])
      );
    }
  }

  return changed;
}

template<class Container>
void refine_private_propstate(Container& cont,
                              const php::Class* cls,
                              const PropState& state) {
  assertx(!is_used_trait(*cls));
  auto* elm = [&] () -> typename Container::value_type* {
    std::lock_guard<std::mutex> _{private_propstate_mutex};
    auto it = cont.find(cls);
    if (it == end(cont)) {
      if (!state.empty()) cont[cls] = state;
      return nullptr;
    }
    return &*it;
  }();

  if (!elm) return;

  for (auto& kv : state) {
    auto& target = elm->second[kv.first];
    assertx(target.tc == kv.second.tc);
    always_assert_flog(
      kv.second.ty.moreRefined(target.ty),
      "PropState refinement failed on {}::${} -- {} was not a subtype of {}\n",
      cls->name->data(),
      kv.first->data(),
      show(kv.second.ty),
      show(target.ty)
    );
    target.ty = kv.second.ty;

    if (kv.second.everModified) {
      always_assert_flog(
        target.everModified,
        "PropState refinement failed on {}::${} -- "
        "everModified flag went from false to true\n",
        cls->name->data(),
        kv.first->data()
      );
    } else {
      target.everModified = false;
    }
  }
}

void Index::refine_private_props(const php::Class* cls,
                                 const PropState& state) {
  refine_private_propstate(m_data->privatePropInfo, cls, state);
}

void Index::refine_private_statics(const php::Class* cls,
                                   const PropState& state) {
  // We can't store context dependent types in private statics since they
  // could be accessed using different contexts.
  auto cleanedState = PropState{};
  for (auto const& prop : state) {
    auto& elem = cleanedState[prop.first];
    elem.ty = unctx(prop.second.ty);
    elem.tc = prop.second.tc;
    elem.attrs = prop.second.attrs;
    elem.everModified = prop.second.everModified;
  }

  refine_private_propstate(m_data->privateStaticPropInfo, cls, cleanedState);
}

void Index::record_public_static_mutations(const php::Func& func,
                                           PublicSPropMutations mutations) {
  if (!mutations.m_data) {
    m_data->publicSPropMutations.erase(&func);
    return;
  }
  m_data->publicSPropMutations.insert_or_assign(&func, std::move(mutations));
}

void Index::update_prop_initial_values(const Context& ctx,
                                       const ResolvedPropInits& resolved,
                                       DependencyContextSet& deps) {
  auto& props = const_cast<php::Class*>(ctx.cls)->properties;

  auto changed = false;
  for (auto const& [idx, info] : resolved) {
    assertx(idx < props.size());
    auto& prop = props[idx];

    auto const allResolved = [&] {
      if (prop.typeConstraint.isUnresolved()) return false;
      for (auto const& ub : prop.ubs.m_constraints) {
        if (ub.isUnresolved()) return false;
      }
      return true;
    };

    if (info.satisfies) {
      if (!(prop.attrs & AttrInitialSatisfiesTC) && allResolved()) {
        attribute_setter(prop.attrs, true, AttrInitialSatisfiesTC);
        changed = true;
      }
    } else {
      always_assert_flog(
        !(prop.attrs & AttrInitialSatisfiesTC),
        "AttrInitialSatisfiesTC invariant violated for {}::{}\n"
        "  Went from true to false",
        ctx.cls->name, prop.name
      );
    }

    always_assert_flog(
      IMPLIES(!(prop.attrs & AttrDeepInit), !info.deepInit),
      "AttrDeepInit invariant violated for {}::{}\n"
      "  Went from false to true",
      ctx.cls->name, prop.name
    );
    attribute_setter(prop.attrs, info.deepInit, AttrDeepInit);

    if (type(info.val) != KindOfUninit) {
      always_assert_flog(
        type(prop.val) == KindOfUninit ||
        from_cell(prop.val) == from_cell(info.val),
        "Property initial value invariant violated for {}::{}\n"
        "  Value went from {} to {}",
        ctx.cls->name, prop.name,
        show(from_cell(prop.val)), show(from_cell(info.val))
      );
      prop.val = info.val;
    } else {
      always_assert_flog(
        type(prop.val) == KindOfUninit,
        "Property initial value invariant violated for {}::{}\n"
        " Value went from {} to not set",
        ctx.cls->name, prop.name,
        show(from_cell(prop.val))
      );
    }
  }
  if (!changed) return;

  auto const it = m_data->classInfo.find(ctx.cls->name);
  if (it == end(m_data->classInfo)) return;
  auto const cinfo = it->second;

  assertx(cinfo->hasBadInitialPropValues);
  auto const noBad = std::all_of(
    begin(props), end(props),
    [] (const php::Prop& prop) {
      return bool(prop.attrs & AttrInitialSatisfiesTC);
    }
  );

  if (noBad) {
    cinfo->hasBadInitialPropValues = false;
    find_deps(*m_data, ctx.cls, Dep::PropBadInitialValues, deps);
  }
}

void Index::refine_public_statics(DependencyContextSet& deps) {
  trace_time update("update public statics");

  // Union together the mutations for each function, including the functions
  // which weren't analyzed this round.
  auto nothing_known = false;
  PublicSPropMutations::UnknownMap unknown;
  PublicSPropMutations::KnownMap known;
  for (auto const& mutations : m_data->publicSPropMutations) {
    if (!mutations.second.m_data) continue;
    if (mutations.second.m_data->m_nothing_known) {
      nothing_known = true;
      break;
    }

    for (auto const& kv : mutations.second.m_data->m_unknown) {
      auto const ret = unknown.insert(kv);
      if (!ret.second) ret.first->second |= kv.second;
    }
    for (auto const& kv : mutations.second.m_data->m_known) {
      auto const ret = known.insert(kv);
      if (!ret.second) ret.first->second |= kv.second;
    }
  }

  if (nothing_known) {
    // We cannot go from knowing the types to not knowing the types (this is
    // equivalent to widening the types).
    always_assert(!m_data->seenPublicSPropMutations);
    return;
  }
  m_data->seenPublicSPropMutations = true;

  // Refine known class state
  parallel::for_each(
    m_data->allClassInfos,
    [&] (std::unique_ptr<ClassInfo>& cinfo) {
      for (auto const& prop : cinfo->cls->properties) {
        if (!(prop.attrs & (AttrPublic|AttrProtected)) ||
            !(prop.attrs & AttrStatic)) {
          continue;
        }

        auto knownClsType = [&] {
          auto const it = known.find(
            PublicSPropMutations::KnownKey { cinfo.get(), prop.name }
          );
          // If we didn't see a mutation, the type is TBottom.
          return it == end(known) ? TBottom : it->second;
        }();

        auto unknownClsType = [&] {
          auto const it = unknown.find(prop.name);
          // If we didn't see a mutation, the type is TBottom.
          return it == end(unknown) ? TBottom : it->second;
        }();

        // We can't keep context dependent types in public properties.
        auto newType = adjust_type_for_prop(
          *this,
          *cinfo->cls,
          &prop.typeConstraint,
          unctx(union_of(std::move(knownClsType), std::move(unknownClsType)))
        );

        auto& entry = cinfo->publicStaticProps[prop.name];

        if (!newType.is(BBottom)) {
          always_assert_flog(
            entry.everModified,
            "Static property index invariant violated on {}::{}:\n"
            " everModified flag went from false to true",
            cinfo->cls->name,
            prop.name
          );
        } else {
          entry.everModified = false;
        }

        // The type from the mutations doesn't contain the in-class
        // initializer types. Add that here.
        auto effectiveType = union_of(
          std::move(newType),
          initial_type_for_public_sprop(*this, *cinfo->cls, prop)
        );

        /*
         * We may only shrink the types we recorded for each property. (If a
         * property type ever grows, the interpreter could infer something
         * incorrect at some step.)
         */
        always_assert_flog(
          effectiveType.subtypeOf(entry.inferredType),
          "Static property index invariant violated on {}::{}:\n"
          "  {} is not a subtype of {}",
          cinfo->cls->name,
          prop.name,
          show(effectiveType),
          show(entry.inferredType)
        );

        // Put a limit on the refinements to ensure termination. Since
        // we only ever refine types, we can stop at any point and still
        // maintain correctness.
        if (effectiveType.strictSubtypeOf(entry.inferredType)) {
          if (entry.refinements + 1 < options.publicSPropRefineLimit) {
            find_deps(*m_data, &prop, Dep::PublicSProp, deps);
            entry.inferredType = std::move(effectiveType);
            ++entry.refinements;
          } else {
            FTRACE(
              1, "maxed out public static property refinements for {}:{}\n",
              cinfo->cls->name,
              prop.name
            );
          }
        }
      }
    }
  );
}

bool Index::frozen() const {
  return m_data->frozen;
}

void Index::freeze() {
  m_data->frozen = true;
  m_data->ever_frozen = true;
}

/*
 * Note that these functions run in separate threads, and
 * intentionally don't bump Trace::hhbbc_time. If you want to see
 * these times, set TRACE=hhbbc_time:1
 */
#define CLEAR(x)                                \
  {                                             \
    trace_time _{"clearing " #x};               \
    _.ignore_client_stats();                    \
    (x).clear();                                \
  }

void Index::cleanup_for_final() {
  trace_time _{"cleanup for final", m_data->sample};
  CLEAR(m_data->dependencyMap);
}

void Index::cleanup_post_emit() {
  trace_time _{"cleanup post emit", m_data->sample};

  std::vector<std::function<void()>> clearers;
  #define CLEAR_PARALLEL(x) clearers.push_back([&] CLEAR(x));
  CLEAR_PARALLEL(m_data->classes);
  CLEAR_PARALLEL(m_data->funcs);
  CLEAR_PARALLEL(m_data->typeAliases);
  CLEAR_PARALLEL(m_data->enums);
  CLEAR_PARALLEL(m_data->constants);
  CLEAR_PARALLEL(m_data->modules);
  CLEAR_PARALLEL(m_data->units);

  CLEAR_PARALLEL(m_data->classClosureMap);
  CLEAR_PARALLEL(m_data->classExtraMethodMap);

  CLEAR_PARALLEL(m_data->classInfo);

  CLEAR_PARALLEL(m_data->privatePropInfo);
  CLEAR_PARALLEL(m_data->privateStaticPropInfo);
  CLEAR_PARALLEL(m_data->publicSPropMutations);
  CLEAR_PARALLEL(m_data->ifaceSlotMap);
  CLEAR_PARALLEL(m_data->closureUseVars);

  CLEAR_PARALLEL(m_data->methodFamilies);

  CLEAR_PARALLEL(m_data->funcFamilies);
  CLEAR_PARALLEL(m_data->funcFamilyStaticInfos);

  CLEAR_PARALLEL(m_data->clsConstLookupCache);

  CLEAR_PARALLEL(m_data->foldableReturnTypeMap);
  CLEAR_PARALLEL(m_data->contextualReturnTypes);

  parallel::for_each(clearers, [] (const std::function<void()>& f) { f(); });

  {
    trace_time t{"reset funcInfo"};
    t.ignore_client_stats();
    parallel::for_each(
      m_data->funcInfo,
      [] (auto& u) {
        u.returnTy = TBottom;
        u.families.clear();
      }
    );
    m_data->funcInfo.clear();
  }

  // Class-infos and program need to be freed after all Type instances
  // are destroyed, as Type::checkInvariants may try to access them.

  {
    trace_time t{"reset allClassInfos"};
    t.ignore_client_stats();
    parallel::for_each(m_data->allClassInfos, [] (auto& u) { u.reset(); });
    m_data->allClassInfos.clear();
  }

  {
    trace_time t{"reset program"};
    t.ignore_client_stats();
    parallel::for_each(m_data->program->units, [] (auto& u) { u.reset(); });
    parallel::for_each(m_data->program->classes, [] (auto& u) { u.reset(); });
    parallel::for_each(m_data->program->funcs, [] (auto& f) { f.reset(); });
    m_data->program.reset();
  }
}

void Index::thaw() {
  m_data->frozen = false;
}

//////////////////////////////////////////////////////////////////////

PublicSPropMutations::PublicSPropMutations(bool enabled) : m_enabled{enabled} {}

PublicSPropMutations::Data& PublicSPropMutations::get() {
  if (!m_data) m_data = std::make_unique<Data>();
  return *m_data;
}

void PublicSPropMutations::mergeKnown(const ClassInfo* ci,
                                      const php::Prop& prop,
                                      const Type& val) {
  if (!m_enabled) return;
  ITRACE(4, "PublicSPropMutations::mergeKnown: {} {} {}\n",
         ci->cls->name->data(), prop.name, show(val));

  auto const res = get().m_known.emplace(
    KnownKey { const_cast<ClassInfo*>(ci), prop.name }, val
  );
  if (!res.second) res.first->second |= val;
}

void PublicSPropMutations::mergeUnknownClass(SString prop, const Type& val) {
  if (!m_enabled) return;
  ITRACE(4, "PublicSPropMutations::mergeUnknownClass: {} {}\n",
         prop, show(val));

  auto const res = get().m_unknown.emplace(prop, val);
  if (!res.second) res.first->second |= val;
}

void PublicSPropMutations::mergeUnknown(Context ctx) {
  if (!m_enabled) return;
  ITRACE(4, "PublicSPropMutations::mergeUnknown\n");

  /*
   * We have a case here where we know neither the class nor the static
   * property name.  This means we have to pessimize public static property
   * types for the entire program.
   *
   * We could limit it to pessimizing them by merging the `val' type, but
   * instead we just throw everything away---this optimization is not
   * expected to be particularly useful on programs that contain any
   * instances of this situation.
   */
  std::fprintf(
    stderr,
    "NOTE: had to mark everything unknown for public static "
    "property types due to dynamic code.  -fanalyze-public-statics "
    "will not help for this program.\n"
    "NOTE: The offending code occured in this context: %s\n",
    show(ctx).c_str()
  );
  get().m_nothing_known = true;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::ClassInfo2);
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::FuncInfo2);
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::FuncFamily2);
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::BuildSubclassListJob::Split);

//////////////////////////////////////////////////////////////////////

}
