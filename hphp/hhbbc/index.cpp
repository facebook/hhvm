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
#include <folly/SharedMutex.h>
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
#include "hphp/hhbbc/optimize.h"
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
#include "hphp/util/bitset-utils.h"
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
namespace coro = folly::coro;

//////////////////////////////////////////////////////////////////////

struct ClassInfo;
struct ClassInfo2;

struct ClassGraphHasher;

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

template<typename Filter>
PropState make_unknown_propstate(const IIndex& index,
                                 const php::Class& cls,
                                 Filter filter) {
  auto ret = PropState{};
  for (auto& prop : cls.properties) {
    if (filter(prop)) {
      auto& elem = ret[prop.name];
      elem.ty = adjust_type_for_prop(
        index,
        cls,
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
   * Whether this function is effect-free.
   */
  bool effectFree{false};

  /*
   * Bitset representing which parameters definitely don't affect the
   * result of the function, assuming it produces one. Note that
   * the parameter type verification does not count as a use in this context.
   */
  std::bitset<64> unusedParams;

  /*
   * If we utilize a ClassGraph while resolving types, we store it
   * here. This ensures that that ClassGraph will always be available
   * again. This is only used for top-level functions. If this
   * function is a method, it will instead be stored in the
   * ClassInfo2.
   *
   * This is wrapped in a std::unique_ptr because needing this for a
   * function is very rare and we want to avoid making FuncInfo2 any
   * larger than necessary. It also helps solve a circular dependency
   * between the types (since std::unique_ptr does not require a
   * complete type at declaration).
   */
  std::unique_ptr<hphp_fast_set<ClassGraph, ClassGraphHasher>> auxClassGraphs;

  template <typename SerDe> void serde(SerDe&);
};

//////////////////////////////////////////////////////////////////////

/*
 * FuncInfos2 for the methods of a class which does not have a
 * ClassInfo2 (because it's uninstantiable). Even if a class doesn't
 * have a ClassInfo2, we might still need FuncInfo2 for it's
 * methods. These are stored in here.
 */
struct MethodsWithoutCInfo {
  SString cls;
  // Same order as the methods vector on the associated php::Class.
  CompactVector<std::unique_ptr<FuncInfo2>> finfos;
  template <typename SerDe> void serde(SerDe& sd) {
    sd(cls)
      (finfos)
      ;
  }
};

//////////////////////////////////////////////////////////////////////

using FuncFamily       = res::Func::FuncFamily;
using FuncInfo         = res::Func::FuncInfo;

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
 * ClassGraph is an abstraction for representing a subset of the
 * complete class hierarchy in a program. Every instance of ClassGraph
 * "points" into the hierarchy at a specific class. From an instance,
 * all (transitive) parents are known, and all (transitive) children
 * may be known.
 *
 * Using a ClassGraph, one can obtain relevant information about the
 * class hierarchy, or perform operations between two classes (union
 * or intersection, for example), without having the ClassInfo
 * available.
 *
 * One advantage of ClassGraph is that it is self
 * contained. Serializing an instance of ClassGraph will serialize all
 * information needed about the hierarchy. This means that an
 * extern-worker job does not need to retrieve additional metadata
 * before using it.
 */
struct ClassGraph {
  // Default constructed instances are not usable for anything (needed
  // for serialization). Use the below factory functions to actually
  // create them.
  ClassGraph() = default;

  // A ClassGraph is falsey if it has been default-constructed.
  explicit operator bool() const { return this_; }

  // Retrieve the name of this class.
  SString name() const;

  // Retrieve an optional ClassInfo/ClassInfo2 associated with this
  // class. A ClassGraph is not guaranteed to have a
  // ClassInfo/ClassInfo2, but if it does, this can be used to save a
  // name -> info lookup.
  ClassInfo* cinfo() const;
  ClassInfo2* cinfo2() const;

  // Return a class equivalent to this one without considering
  // non-regular classes. This might be this class, or a subclass.
  ClassGraph withoutNonRegular() const;

  // Whether the class might be a regular or non-regular class. This
  // check is precise if isMissing() is false.
  bool mightBeRegular() const;
  bool mightBeNonRegular() const;

  // Whether this class might have any regular or non-regular
  // subclasses (not including this class itself). This check is
  // precise if hasCompleteChildren() or isConservative() is true.
  bool mightHaveRegularSubclass() const;
  bool mightHaveNonRegularSubclass() const;

  // A "missing" ClassGraph is a special variant which represents a
  // class about which nothing is known. The class might not even
  // exist. The only valid thing to do with such a class is query its
  // name.
  bool isMissing() const;

  // A class might or might not have complete knowledge about its
  // children. If this returns true, you can perform any of the below
  // queries on it. If not, you can only perform queries related to
  // the parents (non-missing classes always have complete parent
  // information).
  bool hasCompleteChildren() const;

  // A conservative class is one which will never have complete
  // children. This generally means that the class has too many
  // subclasses to efficiently represent. We treat such classes
  // conservatively, except in a few cases.
  bool isConservative() const;

  // Whether this class is an interface, a trait, an enum, or an
  // abstract class. It is invalid to check these if isMissing() is
  // true.
  bool isInterface() const;
  bool isTrait() const;
  bool isEnum() const;
  bool isAbstract() const;

  // Retrieve the immediate base class of this class. If this class
  // doesn't have an immediate base, or if isMissing() is true, a
  // falsey ClassGraph is returned.
  ClassGraph base() const;

  // Retrieve the "topmost" base class of this class. This is the base
  // class which does not have a base class.
  ClassGraph topBase() const;

  // Retrieve all base classes of this class, including this
  // class. The ordering is from top-most class to this one (so this
  // one will be the last element). The returned classes may not have
  // complete child info, even if this class does.
  std::vector<ClassGraph> bases() const;

  // Retrieve all interfaces implemented by this class, either
  // directly, or by transitive parents. This includes this class if
  // it is an interface. The ordering is alphabetical. Like bases(),
  // the returned classes may not have complete child info, even if
  // this class does.
  std::vector<ClassGraph> interfaces() const;

  // Retrieve all children of this class (including this class
  // itself). This is only valid to call if hasCompleteChildren() is
  // true. NB: Being on a class' children list does not necessarily
  // mean that it has a "is-a" relationship. Namely, classes on a
  // trait's children list are not instances of the trait itself.
  std::vector<ClassGraph> children() const;

  // Retrieve the interfaces implemented by this class directly.
  std::vector<ClassGraph> declInterfaces() const {
    return directParents(FlagInterface);
  }
  // Retrieve the set of non-flattened traits used by this class
  // directly. Any traits flattened into this class will not appear.
  std::vector<ClassGraph> usedTraits() const {
    return directParents(FlagTrait);
  }

  // Retrieve the direct parents of this class (all of the classes for
  // which this class is a direct child).
  std::vector<ClassGraph> directParents() const;

  // Returns true if this class is a child of the other class.
  bool isChildOf(ClassGraph) const;

  // Retrieve the set of classes which *might* be equivalent to this
  // class when ignoring non-regular classes. This does not include
  // subclasses of this class.
  std::vector<ClassGraph> candidateRegOnlyEquivs() const;

  // Subtype checks
  bool exactSubtypeOfExact(ClassGraph, bool nonRegL, bool nonRegR) const;
  bool exactSubtypeOf(ClassGraph, bool nonRegL, bool nonRegR) const;
  bool subSubtypeOf(ClassGraph, bool nonRegL, bool nonRegR) const;

  // Could-be checks
  bool exactCouldBeExact(ClassGraph, bool nonRegL, bool nonRegR) const;
  bool exactCouldBe(ClassGraph, bool nonRegL, bool nonRegR) const;
  bool subCouldBe(ClassGraph, bool nonRegL, bool nonRegR) const;

  // "Ensures" that this ClassGraph will be present (with the
  // requested information) for subsequent analysis rounds.
  void ensure() const;
  void ensureWithChildren() const;
  void ensureCInfo() const;

  // Used when building ClassGraphs initially.
  void setClosureBase();
  void setComplete();
  void setBase(ClassGraph);
  void addParent(ClassGraph);
  void flattenTraitInto(ClassGraph);
  void setCInfo(ClassInfo&);
  void setRegOnlyEquivs() const;
  void reset();

  // ClassGraphs are ordered by their name alphabetically.
  bool operator==(ClassGraph h) const { return this_ == h.this_; }
  bool operator!=(ClassGraph h) const { return this_ != h.this_; }
  bool operator<(ClassGraph h) const;

  // Create a new ClassGraph corresponding to the given php::Class.
  // This function will assert if a ClassGraph with the php::Class'
  // name already exists.
  static ClassGraph create(const php::Class&);
  // Retrieve a ClassGraph with the given name, asserting if one
  // doesn't exist.
  static ClassGraph get(SString);
  // Retrieve a ClassGraph with the given name. If one doesn't already
  // existing, one will be created as if by calling getMissing().
  static ClassGraph getOrCreate(SString);
  // Retrieve a "missing" ClassGraph with the given name, asserting if
  // a non-missing one already exists. This is mainly used for tests.
  static ClassGraph getMissing(SString);

  // Before using ClassGraph, it must be initialized (particularly
  // before deserializing any). initConcurrent() must be used if any
  // deserialization with be performed in multiple threads
  // concurrently.
  static void init();
  static void initConcurrent();
  // If initConcurrent() was used, this must be used when
  // deserialization is done and perform querying any ClassGraphs.
  static void stopConcurrent();
  // Called to clean up memory used by ClassGraph framework.
  static void destroy();

  // Set/clear an AnalysisIndex to use for calls to ensure() (and
  // family).
  static void setAnalysisIndex(AnalysisIndex::IndexData&);
  static void clearAnalysisIndex();

  template <typename SerDe, typename T> void serde(SerDe&, T);

  // When serializing multiple ClassGraphs, this can be declared
  // before serializing any of them, to allow for the serialization to
  // share common state and take up less space.
  struct ScopedSerdeState;

  friend struct ClassGraphHasher;

private:
  struct Node;
  struct SerdeState;
  struct Table;

  using NodeSet = hphp_fast_set<Node*>;
  template <typename T> using NodeMap = hphp_fast_map<Node*, T>;
  using NodeVec = TinyVector<Node*, 4>;

  struct SmallBitset;
  struct LargeBitset;
  template <typename> struct ParentTracker;

  enum Flags : std::uint16_t {
    FlagNone         = 0,
    FlagInterface    = (1 << 0),
    FlagTrait        = (1 << 1),
    FlagAbstract     = (1 << 2),
    FlagEnum         = (1 << 3),
    FlagCInfo2       = (1 << 4),
    FlagRegSub       = (1 << 5),
    FlagNonRegSub    = (1 << 6),
    FlagWait         = (1 << 7),
    FlagChildren     = (1 << 8),
    FlagConservative = (1 << 9),
    FlagMissing      = (1 << 10)
  };

  // Iterating through parents or children can result in one of three
  // different outcomes:
  enum class Action {
    Continue, // Keep iterating into any children/parents
    Stop, // Stop iteration entirely
    Skip // Continue iteration, but skip over any children/parents of
         // this class
  };

  std::vector<ClassGraph> directParents(Flags) const;

  void storeAuxs(AnalysisIndex::IndexData&) const;

  static Table& table();

  static NodeVec combine(const NodeVec&, const NodeVec&,
                         bool, bool, bool, bool);
  static NodeVec intersect(const NodeVec&, const NodeVec&,
                           bool, bool, bool&);
  static NodeVec removeNonReg(const NodeVec&);
  static bool couldBeIsect(const NodeVec&, const NodeVec&, bool, bool);
  static NodeVec canonicalize(const NodeSet&, bool);

  template <typename F>
  static void enumerateIsectMembers(const NodeVec&, bool,
                                    const F&, bool = false);

  static std::pair<NodeSet, NodeSet> calcSubclassOfSplit(Node&);
  static NodeSet calcSubclassOf(Node&);
  static Node* calcRegOnlyEquiv(Node&, const NodeSet&);

  static bool betterNode(const Node*, const Node*);

  template <typename F>
  static Action forEachParent(Node& n, const F& f, NodeSet& v) {
    return forEachParentImpl(n, f, v, true);
  }
  template <typename F>
  static Action forEachParent(Node& n, const F& f) {
    NodeSet v;
    return forEachParentImpl(n, f, v, true);
  }
  template <typename F>
  static Action forEachParentImpl(Node&, const F&, NodeSet&, bool);

  template <typename F>
  static Action forEachChild(Node& n, const F& f, NodeSet& v) {
    return forEachChildImpl(n, f, v, true);
  }
  template <typename F>
  static Action forEachChild(Node& n, const F& f) {
    NodeSet v;
    return forEachChildImpl(n, f, v, true);
  }
  template <typename F>
  static Action forEachChildImpl(Node&, const F&, NodeSet&, bool);

  template <typename F, typename F2, typename T>
  static T foldParents(Node& n, const F& f, const F2& f2, NodeMap<T>& m) {
    return foldParentsImpl(n, f, f2, m, true);
  }
  template <typename F, typename F2, typename T>
  static T foldParentsImpl(Node&, const F&, const F2&, NodeMap<T>&, bool);

  static bool findParent(Node&, Node&, NodeSet&);
  static bool findParent(Node& n1, Node& n2) {
    NodeSet visited;
    return findParent(n1, n2, visited);
  }

  static NodeSet allParents(Node&);

  struct LockedSerdeImpl;
  struct UnlockedSerdeImpl;

  static std::vector<Node*> sort(const hphp_fast_set<Node*>&);

  template <typename SerDe, typename Impl, typename T>
  void serdeImpl(SerDe&, const Impl&, T);

  template <typename SerDe, typename Impl>
  static void deserBlock(SerDe&, const Impl&);
  template <typename SerDe> static size_t serDownward(SerDe&, Node&);
  template <typename SerDe> static bool serUpward(SerDe&, Node&);

  template <typename Impl>
  static std::pair<Flags, Optional<size_t>> setCompleteImpl(const Impl&, Node&);

  static std::unique_ptr<Table> g_table;
  static __thread SerdeState* tl_serde_state;

  friend struct res::Class;

  explicit ClassGraph(Node* n) : this_{n} {}

  Node* this_{nullptr};
};

std::unique_ptr<ClassGraph::Table> ClassGraph::g_table{nullptr};
__thread ClassGraph::SerdeState* ClassGraph::tl_serde_state{nullptr};

struct ClassGraphHasher {
  size_t operator()(ClassGraph g) const {
    return pointer_hash<ClassGraph::Node>{}(g.this_);
  }
};

// Node on the graph:
struct ClassGraph::Node {
  // The flags are stored along with any ClassInfo to save memory.
  using CIAndFlags = CompactTaggedPtr<void, Flags>;

  SString name{nullptr};
  // Atomic because they might be manipulated from multiple threads
  // during deserialization.
  std::atomic<CIAndFlags::Opaque> ci{CIAndFlags{}.getOpaque()};
  // Direct (not transitive) parents and children of this node.
  NodeSet parents;
  NodeSet children;

  // This information is lazily cached (and is not serialized).
  struct NonRegularInfo {
    NodeSet subclassOf;
    Node* regOnlyEquiv{nullptr};
  };
  LockFreeLazyPtr<NonRegularInfo> nonRegInfo;

  Flags flags() const { return CIAndFlags{ci.load()}.tag(); }
  ClassInfo* cinfo() const {
    CIAndFlags cif{ci.load()};
    if (cif.tag() & FlagCInfo2) return nullptr;
    return (ClassInfo*)cif.ptr();
  }
  ClassInfo2* cinfo2() const {
    CIAndFlags cif{ci.load()};
    if (!(cif.tag() & FlagCInfo2)) return nullptr;
    return (ClassInfo2*)cif.ptr();
  }
  void* rawPtr() const {
    CIAndFlags cif{ci.load()};
    return cif.ptr();
  }

  bool isBase() const { return !(flags() & (FlagInterface | FlagTrait)); }
  bool isRegular() const {
    return !(flags() & (FlagInterface | FlagTrait | FlagAbstract | FlagEnum));
  }
  bool isTrait() const { return flags() & FlagTrait; }
  bool isInterface() const { return flags() & FlagInterface; }
  bool isEnum() const { return flags() & FlagEnum; }
  bool isAbstract() const { return flags() & FlagAbstract; }
  bool isMissing() const { return flags() & FlagMissing; }
  bool hasCompleteChildren() const { return flags() & FlagChildren; }
  bool isConservative() const { return flags() & FlagConservative; }

  bool hasRegularSubclass() const { return flags() & FlagRegSub; }
  bool hasNonRegularSubclass() const { return flags() & FlagNonRegSub; }

  const NonRegularInfo& nonRegularInfo();

  // NB: These aren't thread-safe, so don't use them during concurrent
  // deserialization.
  void setFlags(Flags f) {
    CIAndFlags old{ci.load()};
    old.set((Flags)(old.tag() | f), old.ptr());
    ci.store(old.getOpaque());
  }
  void setCInfo(ClassInfo& c) {
    CIAndFlags old{ci.load()};
    old.set((Flags)(old.tag() & ~FlagCInfo2), &c);
    ci.store(old.getOpaque());
  }
  void setCInfo(ClassInfo2& c) {
    CIAndFlags old{ci.load()};
    old.set((Flags)(old.tag() | FlagCInfo2), &c);
    ci.store(old.getOpaque());
  }
};

struct ClassGraph::SerdeState {
  NodeSet upward;
  NodeSet downward;
};

struct ClassGraph::Table {
  // Node map to ensure pointer stability.
  TSStringToOneNodeT<Node> nodes;
  // Mapping of one node equivalent to another when only considering
  // regular subclasses. No entry if the mapping is an identity or to
  // a subclass (which is common). Stored separately to save memory
  // since it's rare.
  hphp_fast_map<Node*, Node*> regOnlyEquivs;
  AnalysisIndex::IndexData* index{nullptr};
  struct Locking {
    folly::SharedMutex table;
    std::array<std::mutex, 2048> nodes;
    folly::SharedMutex equivs;
  };
  // If present, we're doing concurrent deserialization.
  Optional<Locking> locking;
};

struct ClassGraph::ScopedSerdeState {
  ScopedSerdeState() {
    // If there's no SerdeState active, make one active, otherwise do
    // nothing.
    if (tl_serde_state) return;
    s.emplace();
    tl_serde_state = s.get_pointer();
  }

  ~ScopedSerdeState() {
    if (!s.has_value()) return;
    assertx(tl_serde_state == s.get_pointer());
    tl_serde_state = nullptr;
  }

  ScopedSerdeState(const ScopedSerdeState&) = delete;
  ScopedSerdeState(ScopedSerdeState&&) = delete;
  ScopedSerdeState& operator=(const ScopedSerdeState&) = delete;
  ScopedSerdeState& operator=(ScopedSerdeState&&) = delete;
private:
  Optional<SerdeState> s;
};

/*
 * When operating over ClassGraph nodes, we often need compact sets of
 * them. This is easy to do with bitsets, but we don't have a fixed
 * upper-size of the sets. We could always use dynamic_bitset, but
 * this can be inefficient. Instead we templatize the algorithm over
 * the bitset, and use a fixed size std::bitset for the common case
 * and dynamic_bitset for the (rare) exceptions.
 */
struct ClassGraph::SmallBitset {
  explicit SmallBitset(size_t limit) {
    assertx(limit <= bits.size());
  }
  SmallBitset(size_t limit, size_t idx) {
    assertx(limit <= bits.size());
    assertx(idx < limit);
    bits[idx] = true;
  }
  SmallBitset& operator|=(const SmallBitset& o) {
    bits |= o.bits;
    return *this;
  }
  SmallBitset& operator&=(const SmallBitset& o) {
    bits &= o.bits;
    return *this;
  }
  SmallBitset& operator-=(size_t idx) {
    assertx(idx < bits.size());
    bits[idx] = false;
    return *this;
  }
  SmallBitset& flip(size_t limit) {
    assertx(limit <= bits.size());
    bits.flip();
    auto const offset = bits.size() - limit;
    bits <<= offset;
    bits >>= offset;
    return *this;
  }
  bool test(size_t idx) const {
    assertx(idx < bits.size());
    return bits[idx];
  }
  bool any() const { return bits.any(); }
  bool none() const { return bits.none(); }
  bool all(size_t limit) const {
    auto s = *this;
    s.flip(limit);
    return s.none();
  }
  size_t first() const { return bitset_find_first(bits); }
  size_t next(size_t prev) const { return bitset_find_next(bits, prev); }
  bool operator==(const SmallBitset& o) const { return bits == o.bits; }

  static constexpr size_t kMaxSize = 64;
  using B = std::bitset<kMaxSize>;
  B bits;

  struct Hasher {
    size_t operator()(const SmallBitset& b) const {
      return std::hash<B>{}(b.bits);
    }
  };
};

struct ClassGraph::LargeBitset {
  explicit LargeBitset(size_t limit): bits{limit} {}
  LargeBitset(size_t limit, size_t idx): bits{limit} {
    assertx(idx < limit);
    bits[idx] = true;
  }
  LargeBitset& operator|=(const LargeBitset& o) {
    bits |= o.bits;
    return *this;
  }
  LargeBitset& operator&=(const LargeBitset& o) {
    bits &= o.bits;
    return *this;
  }
  LargeBitset& operator-=(size_t idx) {
    assertx(idx < bits.size());
    bits[idx] = false;
    return *this;
  }
  LargeBitset& flip(size_t limit) {
    assertx(limit == bits.size());
    bits.flip();
    return *this;
  }
  bool test(size_t idx) const {
    assertx(idx < bits.size());
    return bits[idx];
  }
  bool any() const { return bits.any(); }
  bool none() const { return bits.none(); }
  bool all(size_t) const { return bits.all(); }
  size_t first() const { return bits.find_first(); }
  size_t next(size_t prev) const { return bits.find_next(prev); }
  bool operator==(const LargeBitset& o) const { return bits == o.bits; }

  boost::dynamic_bitset<> bits;

  struct Hasher {
    size_t operator()(const LargeBitset& b) const {
      return std::hash<boost::dynamic_bitset<>>{}(b.bits);
    }
  };
};

// Helper class (parameterized over bitset type) to track Nodes and
// their parents.
template <typename Set>
struct ClassGraph::ParentTracker {
  // Heads is the universe of nodes which are tracked.
  template <typename T>
  explicit ParentTracker(const T& heads, const NodeSet* ignore = nullptr)
    : count{heads.size()}
    , valid{count}
    , ignore{ignore}
  {
    toNode.insert(heads.begin(), heads.end());
    indices.reserve(count);
    for (size_t i = 0; i < count; ++i) indices[toNode[i]] = i;
    valid.flip(count);
  }

  // Intersect this node and it's parents with the current valid ones
  // and return true if any remain.
  bool operator()(Node& n) {
    valid &= set(n);
    return valid.any();
  }
  // Return true if this node and it's parents contain all nodes being
  // tracked.
  bool all(Node& n) { return set(n).all(count); }

  // Return all nodes left in the valid set.
  NodeSet nodes() const {
    NodeSet s;
    for (auto i = valid.first(); i < count; i = valid.next(i)) {
      s.emplace(toNode[i]);
    }
    return s;
  }

private:
  struct Wrapper {
    explicit Wrapper(size_t limit): s{limit} {}
    Wrapper(size_t limit, size_t b): s{limit, b} {}
    explicit operator bool() const { return stop; }
    Wrapper& operator|=(const Wrapper& o) {
      s |= o.s;
      stop &= o.stop;
      return *this;
    }
    Set s;
    bool stop{false};
  };

  Set set(Node& n) {
    if (n.isMissing()) {
      auto const idx = folly::get_ptr(indices, &n);
      if (!idx) return Set{count};
      assertx(*idx < count);
      return Set{count, *idx};
    }

    auto wrapper = foldParents(
      n,
      [&] (Node& p) {
        if (ignore && ignore->count(&p)) {
          Wrapper w{count};
          w.stop = true;
          return w;
        }
        auto const idx = folly::get_ptr(indices, &p);
        if (!idx) return Wrapper{count};
        assertx(*idx < count);
        return Wrapper{count, *idx};
      },
      [&] { return Wrapper{count}; },
      nodeToParents
    );
    return wrapper.s;
  }

  size_t count;
  Set valid;
  NodeMap<size_t> indices;
  NodeMap<Wrapper> nodeToParents;
  NodeVec toNode;
  const NodeSet* ignore;
};

// Abstractions for whether we're deserializing concurrently or
// not. This separates out the locking logic and let's us avoid any
// runtime checks (except one) during deserializing to see if we need
// to lock.

// Non-concurrent implementation. This just modifies the fields
// without any locking.
struct ClassGraph::UnlockedSerdeImpl {
  std::pair<Node*, bool> create(SString name) const {
    auto& n = table().nodes[name];
    if (n.name) {
      assertx(n.name->tsame(name));
      return std::make_pair(&n, false);
    }
    n.name = name;
    return std::make_pair(&n, true);
  }
  Node& get(SString name) const {
    auto n = folly::get_ptr(table().nodes, name);
    always_assert_flog(
      n,
      "Attempting to retrieve missing ClassGraph node '{}'",
      name
    );
    assertx(n->name->tsame(name));
    return *n;
  }
  void setEquiv(Node& n, Node& e) const {
    auto const [it, s] = table().regOnlyEquivs.emplace(&n, &e);
    always_assert(s || it->second == &e);
  }
  template <typename F> void lock(Node&, const F& f) const { f(); }
  void signal(Node& n, Flags f) const {
    assertx(n.flags() == FlagNone);
    assertx(!(f & FlagWait));
    n.setFlags(f);
  }
  void setCInfo(Node& n, ClassInfo& cinfo) const { n.setCInfo(cinfo); }
  void setCInfo(Node& n, ClassInfo2& cinfo) const { n.setCInfo(cinfo); }
  void updateFlags(Node& n, Flags add, Flags remove = FlagNone) const {
    if (add == FlagNone && remove == FlagNone) return;
    auto const oldFlags = n.flags();
    auto const newFlags = (Flags)((oldFlags | add) & ~remove);
    if (newFlags == oldFlags) return;
    n.ci.store(Node::CIAndFlags{newFlags, n.rawPtr()}.getOpaque());
  }
};

// Concurrent implementation. The node table is guarded by a RWLock
// and the individual nodes are guarded by an array of locks. The hash
// of the node's address determines the lock to use. In addition,
// FlagWait is used to tell threads that the node is being created and
// one must wait for the flag to be reset.
struct ClassGraph::LockedSerdeImpl {
  std::pair<Node*, bool> create(SString name) const {
    // Called when we find an existing node. We cannot safely return
    // this node until FlagWait is cleared.
    auto const wait = [&] (Node& n) {
      assertx(n.name->tsame(name));
      while (true) {
        Node::CIAndFlags f{n.ci.load()};
        if (!(f.tag() & FlagWait)) return std::make_pair(&n, false);
        // Still set, wait for it to change and then check again.
        n.ci.wait(f.getOpaque());
      }
    };

    auto& t = table();

    // First access the table with a read lock and see if the node
    // already exists.
    {
      auto const n = [&] {
        folly::SharedMutex::ReadHolder _{t.locking->table};
        return folly::get_ptr(t.nodes, name);
      }();
      // It already exists, wait on FlagWait and then return.
      if (n) return wait(*n);
    }

    // It didn't, we need to create it:
    std::unique_lock _{t.locking->table};
    // We now have exlusive access to the table. Check for the node
    // again, as someone else might have created it in the meantime.
    if (auto const n = folly::get_ptr(t.nodes, name)) {
      // If someone did, wait on FlagWait and then return. Drop the
      // write lock before waiting to avoid deadlock.
      _.unlock();
      return wait(*n);
    }

    // Node still isn't present. Create one now.
    auto [it, emplaced] = t.nodes.try_emplace(name);
    always_assert(emplaced);
    auto& n = it->second;
    assertx(!n.name);
    assertx(!(n.flags() & FlagWait));
    n.name = name;
    // Set FlagWait, this will ensure that any other thread who
    // retrieves this node (after we drop the write lock) will block
    // until we're done deserializing it and it's children.
    n.setFlags(FlagWait);
    return std::make_pair(&n, true);
  }
  Node& get(SString name) const {
    auto& t = table();
    folly::SharedMutex::ReadHolder _{t.locking->table};
    auto n = folly::get_ptr(t.nodes, name);
    always_assert_flog(
      n,
      "Attempting to retrieve missing ClassGraph node '{}'",
      name
    );
    assertx(n->name->tsame(name));
    // FlagWait shouldn't be set here because we shouldn't call get()
    // until a node and all of it's dependents are created.
    assertx(!(n->flags() & FlagWait));
    return *n;
  }
  void setEquiv(Node& n, Node& e) const {
    auto& t = table();
    {
      folly::SharedMutex::ReadHolder _{t.locking->equivs};
      if (auto const old = folly::get_default(t.regOnlyEquivs, &n)) {
        assertx(old == &e);
        return;
      }
    }
    std::unique_lock _{t.locking->equivs};
    auto const [it, s] = t.regOnlyEquivs.emplace(&n, &e);
    always_assert(s || it->second == &e);
  }
  // Lock a node by using the lock it hashes to and execute f() while
  // holding the lock.
  template <typename F> void lock(Node& n, const F& f) const {
    auto& t = table();
    auto& lock = t.locking->nodes[
      pointer_hash<Node>{}(&n) % t.locking->nodes.size()
    ];
    lock.lock();
    SCOPE_EXIT { lock.unlock(); };
    f();
  }
  // Signal that a node (and all of it's dependents) is done being
  // deserialized. This clears FlagWait and wakes up any threads
  // waiting on the flag. In addition, it sets the node's flags to the
  // provided flags.
  void signal(Node& n, Flags other) const {
    assertx(!(other & FlagWait));
    while (true) {
      auto old = n.ci.load();
      Node::CIAndFlags f{old};
      assertx(f.tag() == FlagWait);
      // Use CAS to set the flag
      f.set(other, f.ptr());
      if (n.ci.compare_exchange_strong(old, f.getOpaque())) break;
    }
    // Wake up any other threads.
    n.ci.notify_all();
  }
  // Set a ClassInfo2 on this node, using CAS to detect concurrent
  // modifications.
  void setCInfo(Node& n, ClassInfo& c) const {
    while (true) {
      auto old = n.ci.load();
      Node::CIAndFlags f{old};
      f.set((Flags)(f.tag() & ~FlagCInfo2), &c);
      if (n.ci.compare_exchange_strong(old, f.getOpaque())) break;
    }
  }
  void setCInfo(Node& n, ClassInfo2& c) const {
    while (true) {
      auto old = n.ci.load();
      Node::CIAndFlags f{old};
      f.set((Flags)(f.tag() | FlagCInfo2), &c);
      if (n.ci.compare_exchange_strong(old, f.getOpaque())) break;
    }
  }
  void updateFlags(Node& n, Flags add, Flags remove = FlagNone) const {
    if (add == FlagNone && remove == FlagNone) return;
    while (true) {
      auto old = n.ci.load();
      Node::CIAndFlags f{old};
      auto const oldFlags = (Flags)f.tag();
      auto const newFlags = (Flags)((oldFlags | add) & ~remove);
      if (newFlags == oldFlags) break;
      f.set(newFlags, f.ptr());
      if (n.ci.compare_exchange_strong(old, f.getOpaque())) break;
    }
  }
};

SString ClassGraph::name() const {
  assertx(this_);
  return this_->name;
}

ClassInfo* ClassGraph::cinfo() const {
  assertx(this_);
  return this_->cinfo();
}

ClassInfo2* ClassGraph::cinfo2() const {
  assertx(this_);
  return this_->cinfo2();
}

bool ClassGraph::mightBeRegular() const {
  assertx(this_);
  return this_->isMissing() || this_->isRegular();
}

bool ClassGraph::mightBeNonRegular() const {
  assertx(this_);
  return this_->isMissing() || !this_->isRegular();
}

bool ClassGraph::mightHaveRegularSubclass() const {
  assertx(this_);
  if (this_->hasCompleteChildren() || this_->isConservative()) {
    return this_->hasRegularSubclass();
  }
  return true;
}

bool ClassGraph::mightHaveNonRegularSubclass() const {
  assertx(this_);
  if (this_->hasCompleteChildren() || this_->isConservative()) {
    return this_->hasNonRegularSubclass();
  }
  return true;
}

bool ClassGraph::isMissing() const {
  assertx(this_);
  return this_->isMissing();
}

bool ClassGraph::hasCompleteChildren() const {
  assertx(this_);
  return !this_->isMissing() && this_->hasCompleteChildren();
}

bool ClassGraph::isConservative() const {
  assertx(this_);
  return !this_->isMissing() && this_->isConservative();
}

bool ClassGraph::isInterface() const {
  assertx(this_);
  assertx(!isMissing());
  return this_->isInterface();
}

bool ClassGraph::isTrait() const {
  assertx(this_);
  assertx(!isMissing());
  return this_->isTrait();
}

bool ClassGraph::isEnum() const {
  assertx(this_);
  assertx(!isMissing());
  return this_->isEnum();
}

bool ClassGraph::isAbstract() const {
  assertx(this_);
  assertx(!isMissing());
  return this_->isAbstract();
}

void ClassGraph::setComplete() {
  assertx(!table().locking);
  assertx(this_);
  assertx(!this_->isMissing());
  assertx(!this_->hasCompleteChildren());
  assertx(!this_->isConservative());
  setCompleteImpl(UnlockedSerdeImpl{}, *this_);
}

void ClassGraph::setClosureBase() {
  assertx(!table().locking);
  assertx(this_);
  assertx(!this_->isMissing());
  assertx(!this_->hasCompleteChildren());
  assertx(!this_->isConservative());
  assertx(is_closure_base(this_->name));
  this_->children.clear();
  this_->setFlags((Flags)(FlagConservative | FlagRegSub));
}

void ClassGraph::setBase(ClassGraph b) {
  assertx(!table().locking);
  assertx(this_);
  assertx(b.this_);
  assertx(!isMissing());
  assertx(!b.isMissing());
  assertx(b.this_->isBase());
  this_->parents.emplace(b.this_);
  if (!b.this_->isConservative()) b.this_->children.emplace(this_);
}

void ClassGraph::addParent(ClassGraph p) {
  assertx(!table().locking);
  assertx(this_);
  assertx(p.this_);
  assertx(!isMissing());
  assertx(!p.isMissing());
  assertx(!p.this_->isBase());
  this_->parents.emplace(p.this_);
  if (!p.this_->isConservative()) p.this_->children.emplace(this_);
}

void ClassGraph::flattenTraitInto(ClassGraph t) {
  assertx(!table().locking);
  assertx(this_);
  assertx(t.this_);
  assertx(t.this_->isTrait());
  assertx(!isMissing());
  assertx(!t.isMissing());
  assertx(!t.isConservative());
  assertx(!t.hasCompleteChildren());

  // Remove this trait as a parent, and move all of it's parents into
  // this class.
  always_assert(this_->parents.erase(t.this_));
  always_assert(t.this_->children.erase(this_));
  for (auto const p : t.this_->parents) {
    this_->parents.emplace(p);
    p->children.emplace(this_);
  }
}

void ClassGraph::reset() {
  assertx(!table().locking);
  assertx(this_);
  assertx(this_->children.empty());
  assertx(!this_->isMissing());
  assertx(!this_->isConservative());
  assertx(!this_->hasCompleteChildren());
  for (auto const parent : this_->parents) {
    if (parent->isConservative()) continue;
    always_assert(parent->children.erase(this_));
  }
  this_->parents.clear();
  this_->ci.store(Node::CIAndFlags::Opaque{});
  this_ = nullptr;
}

void ClassGraph::setCInfo(ClassInfo& ci) {
  assertx(!table().locking);
  assertx(this_);
  assertx(!this_->isMissing());
  assertx(this_->hasCompleteChildren() || this_->isConservative());
  this_->setCInfo(ci);
}

bool ClassGraph::operator<(ClassGraph h) const {
  return string_data_lt_type{}(this_->name, h.this_->name);
}

ClassGraph ClassGraph::withoutNonRegular() const {
  assertx(!table().locking);
  assertx(this_);
  if (this_->isMissing() || this_->isRegular()) {
    ensure();
    return *this;
  }
  return ClassGraph { this_->nonRegularInfo().regOnlyEquiv };
}

ClassGraph ClassGraph::base() const {
  assertx(!table().locking);
  assertx(this_);
  ensure();
  if (this_->isMissing()) return ClassGraph { nullptr };
  for (auto const p : this_->parents) {
    if (p->isBase()) return ClassGraph { p };
  }
  return ClassGraph { nullptr };
}

ClassGraph ClassGraph::topBase() const {
  assertx(!table().locking);
  assertx(this_);
  assertx(!isMissing());

  ensure();

  auto current = this_;
  auto last = this_;
  do {
    last = this_;
    auto const& parents = current->parents;
    current = nullptr;
    // There should be at most one base on the parent list. Verify
    // this in debug builds.
    for (auto const p : parents) {
      if (p->isBase()) {
        assertx(!current);
        current = p;
        if (!debug) break;
      }
    }
  } while (current);

  return ClassGraph { last };
}

std::vector<ClassGraph> ClassGraph::bases() const {
  assertx(!table().locking);
  assertx(this_);
  assertx(!isMissing());

  ensure();

  std::vector<ClassGraph> out;
  auto current = this_;
  do {
    out.emplace_back(ClassGraph{ current });
    auto const& parents = current->parents;
    current = nullptr;
    // There should be at most one base on the parent list. Verify
    // this in debug builds.
    for (auto const p : parents) {
      if (p->isBase()) {
        assertx(!current);
        current = p;
        if (!debug) break;
      }
    }
  } while (current);

  std::reverse(begin(out), end(out));
  return out;
}

std::vector<ClassGraph> ClassGraph::interfaces() const {
  assertx(!table().locking);
  assertx(this_);
  assertx(!isMissing());

  ensure();

  std::vector<ClassGraph> out;
  forEachParent(
    *this_,
    [&] (Node& p) {
      if (p.isInterface()) out.emplace_back(ClassGraph{ &p });
      return Action::Continue;
    }
  );
  std::sort(begin(out), end(out));
  return out;
}

std::vector<ClassGraph> ClassGraph::children() const {
  assertx(!table().locking);
  assertx(this_);
  assertx(!isMissing());
  assertx(hasCompleteChildren());

  ensureWithChildren();

  std::vector<ClassGraph> out;
  // If this_ is a trait, then forEachChild won't walk the list. Use
  // forEachChildImpl with the right params to prevent this.
  NodeSet visited;
  forEachChildImpl(
    *this_,
    [&] (Node& c) {
      out.emplace_back(ClassGraph{ &c });
      return Action::Continue;
    },
    visited,
    false
  );
  std::sort(begin(out), end(out));
  return out;
}

std::vector<ClassGraph> ClassGraph::directParents(Flags flags) const {
  assertx(!table().locking);
  assertx(this_);
  assertx(!isMissing());

  ensure();

  std::vector<ClassGraph> out;
  out.reserve(this_->parents.size());
  for (auto const p : this_->parents) {
    if (!(p->flags() & flags)) continue;
    out.emplace_back(ClassGraph { p });
  }
  std::sort(begin(out), end(out));
  return out;
}

std::vector<ClassGraph> ClassGraph::directParents() const {
  assertx(!table().locking);
  assertx(this_);
  assertx(!isMissing());

  ensure();

  std::vector<ClassGraph> out;
  out.reserve(this_->parents.size());
  for (auto const p : this_->parents) out.emplace_back(ClassGraph { p });
  std::sort(begin(out), end(out));
  return out;
}

bool ClassGraph::isChildOf(ClassGraph o) const {
  assertx(!table().locking);
  assertx(this_);
  assertx(o.this_);
  assertx(!this_->isMissing());
  assertx(!o.this_->isMissing());
  if (this_ == o.this_) return true;
  ensure();
  // Nothing is a child of a trait except itself and we know they're
  // not equal.
  if (o.this_->isTrait()) return false;
  return findParent(*this_, *o.this_);
}

std::vector<ClassGraph> ClassGraph::candidateRegOnlyEquivs() const {
  assertx(!table().locking);
  assertx(this_);
  assertx(!this_->isMissing());

  if (this_->isRegular() || this_->isConservative()) return {};
  assertx(this_->hasCompleteChildren());
  auto const nonParents = calcSubclassOfSplit(*this_).first;
  if (nonParents.empty()) return {};
  if (nonParents.size() == 1) return { ClassGraph { *nonParents.begin() } };

  auto heads = nonParents;

  // Remove any nodes which are reachable from another node. Such
  // nodes are redundant.
  NodeSet visited;
  for (auto const n : nonParents) {
    if (!heads.count(n)) continue;
    forEachParent(
      *n,
      [&] (Node& p) {
        if (&p == n) return Action::Continue;
        if (!nonParents.count(&p)) return Action::Continue;
        if (!heads.count(&p)) return Action::Skip;
        heads.erase(&p);
        return Action::Continue;
      },
      visited
    );
    visited.erase(n);
  }

  // Remove any nodes which have a (regular) child which does not have
  // this_ a parent. Such a node can't be an equivalent because it
  // contains more regular nodes than this_. Note that we might not
  // have complete children information for all nodes, so this check
  // is conservative. We only remove nodes which are definitely not
  // candidates.
  folly::erase_if(
    heads,
    [&] (Node* n) {
      auto const action = forEachChild(
        *n,
        [&] (Node& c) {
          if (!c.isRegular()) return Action::Continue;
          if (!findParent(c, *this_)) return Action::Stop;
          return Action::Skip;
        }
      );
      return action == Action::Stop;
    }
  );

  std::vector<ClassGraph> out;
  out.reserve(heads.size());
  for (auto const n : heads) out.emplace_back(ClassGraph { n });
  std::sort(begin(out), end(out));
  return out;
}

void ClassGraph::setRegOnlyEquivs() const {
  assertx(!table().locking);
  assertx(this_);
  assertx(!this_->isMissing());
  if (this_->isRegular() || this_->isConservative()) return;
  assertx(this_->hasCompleteChildren());
  auto const equiv = calcRegOnlyEquiv(*this_, calcSubclassOf(*this_));
  if (!equiv || equiv == this_ || findParent(*equiv, *this_)) return;
  auto const [it, s] = table().regOnlyEquivs.emplace(this_, equiv);
  always_assert(s || it->second == equiv);
}

const ClassGraph::Node::NonRegularInfo&
ClassGraph::Node::nonRegularInfo() {
  assertx(!table().locking);
  assertx(!isMissing());
  assertx(!isRegular());
  ClassGraph{this}.ensureWithChildren();
  return nonRegInfo.get(
    [this] {
      auto info = std::make_unique<NonRegularInfo>();
      info->subclassOf = calcSubclassOf(*this);
      info->regOnlyEquiv = calcRegOnlyEquiv(*this, info->subclassOf);
      return info.release();
    }
  );
}

bool ClassGraph::exactSubtypeOfExact(ClassGraph o,
                                     bool nonRegL,
                                     bool nonRegR) const {
  assertx(!table().locking);
  assertx(this_);
  assertx(o.this_);

  ensure();
  o.ensure();

  // Two exact classes are only subtypes of another if they're the
  // same. One additional complication is if the class isn't regular
  // and we're not considering non-regular classes. In that case, the
  // class is actually Bottom, and we need to apply the rules of
  // subtyping to Bottom (Bottom is a subtype of everything, but
  // nothing is a subtype of it).
  if (this_->isMissing()) {
    // Missing classes are only definitely a subtype if it's the same
    // node and the lhs can become bottom or the rhs cannot.
    return (this_ == o.this_) && (!nonRegL || nonRegR);
  } else if (!nonRegL && !this_->isRegular()) {
    // Lhs is a bottom, so a subtype of everything.
    return true;
  } else if (o.this_->isMissing() || (!nonRegR && !o.this_->isRegular())) {
    // Lhs is not a bottom, check if the rhs is.
    return false;
  } else {
    // Neither is bottom, check for equality.
    return this_ == o.this_;
  }
}

bool ClassGraph::exactSubtypeOf(ClassGraph o,
                                bool nonRegL,
                                bool nonRegR) const {
  assertx(!table().locking);
  assertx(this_);
  assertx(o.this_);

  ensure();
  o.ensure();

  // If we want to exclude non-regular classes on either side, and the
  // lhs is not regular, there's no subtype relation. If nonRegL is
  // false, then lhs is just a bottom (and bottom is a subtype of
  // everything), and if nonRegularR is false, then the rhs does not
  // contain any non-regular classes, so lhs is guaranteed to not be
  // part of it.
  if (this_->isMissing()) {
    // If the lhs side is missing, it's identical to
    // exactSubtypeOfExact.
    return (this_ == o.this_) && (!nonRegL || nonRegR);
  } else if ((!nonRegL || !nonRegR) && !this_->isRegular()) {
    return !nonRegL;
  } else if (this_ == o.this_) {
    return true;
  } else if (o.this_->isMissing() || o.this_->isTrait()) {
    // The lhs is not missing, so cannot be a subtype of a missing
    // class. Nothing is a subtype of a trait.
    return false;
  } else {
    // Otherwise try to find the rhs node among the parents of the lhs
    // side.
    return findParent(*this_, *o.this_);
  }
}

bool ClassGraph::subSubtypeOf(ClassGraph o, bool nonRegL, bool nonRegR) const {
  assertx(!table().locking);
  assertx(this_);
  assertx(o.this_);

  ensure();
  o.ensure();

  if (nonRegL && !nonRegR) {
    if (this_->isMissing() || !this_->isRegular()) return false;
    if (this_->hasNonRegularSubclass()) return false;
  }

  // If this_ must be part of the lhs, it's equivalent to
  // exactSubtypeOf. Otherwise if exactSubtypeOf returns true for the
  // conservative case, then it must always be true, so we don't need
  // to look at children.
  if (nonRegL || this_->isMissing() || this_->isRegular()) {
    return exactSubtypeOf(o, nonRegL, nonRegR);
  }
  if (exactSubtypeOf(o, true, true)) return true;

  // this_ is not regular and will not be part of the lhs. We need to
  // look at the regular children of this_ and check whether they're
  // all subtypes of the rhs.

  // Traits have no children for the purposes of this test.
  if (this_->isTrait() || o.this_->isMissing() || o.this_->isTrait()) {
    return false;
  }
  return this_->nonRegularInfo().subclassOf.count(o.this_);
}

bool ClassGraph::exactCouldBeExact(ClassGraph o,
                                   bool nonRegL,
                                   bool nonRegR) const {
  assertx(!table().locking);
  assertx(this_);
  assertx(o.this_);

  ensure();
  o.ensure();

  // Two exact classes can only be each other if they're the same
  // class. The only complication is if the class isn't regular and
  // we're not considering non-regular classes. In that case, the
  // class is actually Bottom, a Bottom can never could-be anything
  // (not even itself).
  if (this_ != o.this_) return false;
  if (this_->isMissing() || this_->isRegular()) return true;
  return nonRegL && nonRegR;
}

bool ClassGraph::exactCouldBe(ClassGraph o, bool nonRegL, bool nonRegR) const {
  assertx(!table().locking);
  assertx(this_);
  assertx(o.this_);

  ensure();
  o.ensure();

  // exactCouldBe is almost identical to exactSubtypeOf, except the
  // case of the lhs being bottom is treated differently (bottom in
  // exactSubtypeOf implies true, but here it implies false).
  if (this_->isMissing()) {
    if (o.this_->isMissing()) return true;
    if ((!nonRegL || !nonRegR) &&
        !o.this_->isRegular() &&
        !o.this_->hasRegularSubclass()) {
      return false;
    }
    return !o.this_->hasCompleteChildren();
  } else if ((!nonRegL || !nonRegR) && !this_->isRegular()) {
    return false;
  } else if (this_ == o.this_) {
    return true;
  } else if (o.this_->isMissing() || o.this_->isTrait()) {
    return false;
  } else {
    return findParent(*this_, *o.this_);
  }
}

bool ClassGraph::subCouldBe(ClassGraph o, bool nonRegL, bool nonRegR) const {
  assertx(!table().locking);
  assertx(this_);
  assertx(o.this_);

  ensure();
  o.ensure();

  auto const regOnly = !nonRegL || !nonRegR;

  if (exactCouldBe(o, nonRegL, nonRegR) ||
      o.exactCouldBe(*this, nonRegR, nonRegL)) {
    return true;
  } else if (this_->isMissing() || o.this_->isMissing() ||
             this_->isTrait() || o.this_->isTrait()) {
    return false;
  } else if (regOnly &&
             ((!this_->isRegular() && !this_->hasRegularSubclass()) ||
              (!o.this_->isRegular() && !o.this_->hasRegularSubclass()))) {
    return false;
  }

  auto left = this_;
  auto right = o.this_;
  if (betterNode(right, left)) std::swap(left, right);

  ClassGraph{left}.ensureWithChildren();
  if (!left->hasCompleteChildren()) return true;

  NodeSet visited;
  auto const action = forEachChild(
    *left,
    [&] (Node& c) {
      if (regOnly && !c.isRegular()) return Action::Continue;
      return findParent(c, *right, visited)
        ? Action::Stop : Action::Continue;
    }
  );
  return action == Action::Stop;
}

// Calculate all the classes which this class could be a subclass
// of. This is only necessary if the class is non-regular (for regular
// classes the subtype check is trivial). This not only speeds up
// subtypeOf checks, but it is used when calculating reg-only
// equivalent classes. The results are split into the nodes which
// aren't parents of this class, and the ones which are not.
std::pair<ClassGraph::NodeSet, ClassGraph::NodeSet>
ClassGraph::calcSubclassOfSplit(Node& n) {
  assertx(!table().locking);
  assertx(!n.isMissing());
  assertx(!n.isRegular());

  // Traits cannot be a subclass of anything but itself, and if we
  // don't know all of the children, we have to be pessimistic and
  // report only the parents.
  if (n.isTrait()) return std::make_pair(NodeSet{}, NodeSet{});
  if (!n.hasCompleteChildren()) {
    return std::make_pair(NodeSet{}, allParents(n));
  }

  // Find the first regular child of this non-regular class.
  Node* first = nullptr;
  forEachChild(
    n,
    [&] (Node& c) {
      if (!c.isRegular()) return Action::Continue;
      assertx(&c != &n);
      // n has complete children, so all children must as well.
      assertx(c.hasCompleteChildren());
      first = &c;
      return Action::Stop;
    }
  );
  // No regular child
  if (!first) return std::make_pair(NodeSet{}, NodeSet{});

  auto parents = allParents(n);

  // Given the regular class we found, gather all of it's
  // children. This is the initial set of candidates.
  NodeVec candidates;
  forEachParent(
    *first,
    [&] (Node& p) {
      // Ignore parents since we already know about this.
      if (parents.count(&p)) return Action::Skip;
      candidates.emplace_back(&p);
      return Action::Continue;
    }
  );
  if (candidates.empty()) return std::make_pair(NodeSet{}, std::move(parents));

  // Then use ParentTracker to remove any nodes which are not parents
  // of the other children.
  auto const run = [&] (auto tracker) {
    forEachChild(
      n,
      [&] (Node& c) {
        if (!c.isRegular()) return Action::Continue;
        return tracker(c) ? Action::Skip : Action::Stop;
      }
    );
    auto nodes = tracker.nodes();
    return std::make_pair(std::move(nodes), std::move(parents));
  };

  if (candidates.size() <= SmallBitset::kMaxSize) {
    return run(ParentTracker<SmallBitset>{candidates, &parents});
  } else {
    return run(ParentTracker<LargeBitset>{candidates, &parents});
  }
}

ClassGraph::NodeSet ClassGraph::calcSubclassOf(Node& n) {
  assertx(!table().locking);
  assertx(!n.isMissing());
  assertx(!n.isRegular());
  auto [nonParents, parents] = calcSubclassOfSplit(n);
  nonParents.insert(begin(parents), end(parents));
  return nonParents;
}

// Calculate the "best" Node which is equivalent to this Node when
// considering only regular children. We canonicalize this Node to the
// equivalent Node where appropriate. Nullptr is returned if the
// "equivalent" Node is actually Bottom (because there are no regular
// children). subclassOf gives the set of "candidate" Nodes.
ClassGraph::Node* ClassGraph::calcRegOnlyEquiv(Node& base,
                                               const NodeSet& subclassOf) {
  assertx(!table().locking);
  assertx(!base.isMissing());
  assertx(!base.isRegular());

  if (subclassOf.empty()) return nullptr;
  if (!base.hasRegularSubclass()) return nullptr;
  if (subclassOf.size() == 1) {
    assertx(subclassOf.count(&base));
    return &base;
  }
  // Traits should be captured by one of the above checks.
  assertx(!base.isTrait());

  // If we recorded an equivalent when deserializing, just use that.
  if (auto const e = folly::get_default(table().regOnlyEquivs, &base)) {
    assertx(e->hasCompleteChildren());
    return e;
  }
  // Otherwise calculate it.

  auto heads = subclassOf;

  // Remove any nodes which are reachable from another node. Such
  // nodes are redundant.
  NodeSet visited;
  for (auto const n : subclassOf) {
    if (!heads.count(n)) continue;
    forEachParent(
      *n,
      [&] (Node& p) {
        if (&p == n) return Action::Continue;
        if (!subclassOf.count(&p)) return Action::Continue;
        if (!heads.count(&p)) return Action::Skip;
        heads.erase(&p);
        return Action::Continue;
      },
      visited
    );
    visited.erase(n);
  }
  if (heads.size() == 1) return *heads.begin();

  assertx(base.hasCompleteChildren());

  // Find the best node among the remaining candidates.
  auto const run = [&] (auto tracker) {
    // A node is only a valid candidate (and thus sufficient) if all
    // of it's children are subclasses of the "base" Node.
    auto const isSufficient = [&] (Node* n) {
      if (n == &base) return true;
      if (findParent(*n, base)) return true;
      if (!n->hasCompleteChildren()) return false;
      auto const action = forEachChild(
        *n,
        [&] (Node& c) {
          if (!c.isRegular()) return Action::Continue;
          return tracker.all(c) ? Action::Skip : Action::Stop;
        }
      );
      return action != Action::Stop;
    };

    Node* best = nullptr;
    for (auto const n : heads) {
      if (!isSufficient(n)) continue;
      if (!best || betterNode(n, best)) best = n;
    }
    assertx(best);
    return best;
  };

  if (heads.size() <= SmallBitset::kMaxSize) {
    return run(ParentTracker<SmallBitset>{heads});
  } else {
    return run(ParentTracker<LargeBitset>{heads});
  }
}

// Someone arbitrarily ranks two Nodes in a consistent manner.
bool ClassGraph::betterNode(const Node* n1, const Node* n2) {
  if (n1 == n2) return false;

  // Non-missing nodes are always better. Two missing nodes are ranked
  // according to name.
  if (n1->isMissing()) {
    if (!n2->isMissing()) return false;
    return string_data_lt_type{}(n1->name, n2->name);
  } else if (n2->isMissing()) {
    return true;
  }

  // Nodes with complete children are better than those that don't.
  if (!n1->hasCompleteChildren()) {
    if (n2->hasCompleteChildren()) return false;
    return string_data_lt_type{}(n1->name, n2->name);
  } else if (!n2->hasCompleteChildren()) {
    return true;
  }

  // Choose the one with the least (immediate) children. Calculating
  // the full subclass list would be better, but more
  // expensive. Traits are always considered to have no children.
  auto const s1 = n1->isTrait() ? 0 : n1->children.size();
  auto const s2 = n2->isTrait() ? 0 : n2->children.size();
  if (s1 != s2) return s1 < s2;

  // Otherwise rank them acccording to what flags they have and then
  // finally by name.
  auto const weight = [] (const Node* n) {
    auto const f = n->flags();
    if (f & FlagAbstract)  return 1;
    if (f & FlagInterface) return 2;
    if (f & FlagTrait)     return 3;
    if (f & FlagEnum)      return 4;
    return 0;
  };
  auto const w1 = weight(n1);
  auto const w2 = weight(n2);
  if (w1 != w2) return w1 < w2;
  return string_data_lt_type{}(n1->name, n2->name);
}

// Union together two sets of intersected classes.
ClassGraph::NodeVec ClassGraph::combine(const NodeVec& lhs,
                                        const NodeVec& rhs,
                                        bool isSubL,
                                        bool isSubR,
                                        bool nonRegL,
                                        bool nonRegR) {
  assertx(!table().locking);
  assertx(!lhs.empty());
  assertx(!rhs.empty());
  assertx(IMPLIES(!isSubL, lhs.size() == 1));
  assertx(IMPLIES(!isSubR, rhs.size() == 1));

  // Combine two sets, keeping only the nodes that are in both
  // sets. An empty set is equivalent to Top (not Bottom), thus
  // everything in the other set goes in.
  NodeSet combined;
  auto const combine = [&] (const NodeSet& s1, const NodeSet& s2) {
    if (s1.empty()) {
      always_assert(!s2.empty());
      combined.insert(begin(s2), end(s2));
    } else if (s2.empty()) {
      combined.insert(begin(s1), end(s1));
    } else if (s1.size() < s2.size()) {
      for (auto const e : s1) {
        if (s2.count(e)) combined.emplace(e);
      }
    } else {
      for (auto const e : s2) {
        if (s1.count(e)) combined.emplace(e);
      }
    }
  };

  /*
   * (A&B) | (C&D) = (A|C) & (A|D) & (B|C) & (B|D)
   *
   * (this generalizes to arbitrary size lists). So to union together
   * two intersection sets, we need to calculate the union of
   * individual classes pair-wise, then intersect them together.
   */
  auto const process = [&] (Node* l, Node* r) {
    ClassGraph{l}.ensure();
    ClassGraph{r}.ensure();

    // Order the l and r to cut down on the number of cases to deal
    // with below.
    auto const flip = [&] {
      if (l->isMissing()) return false;
      if (r->isMissing()) return true;
      if (nonRegL || l->isRegular()) return false;
      if (nonRegR || r->isRegular()) return true;
      if (isSubL) return false;
      return isSubR;
    }();
    if (flip) std::swap(l, r);
    auto const flipSubL = flip ? isSubR : isSubL;
    auto const flipSubR = flip ? isSubL : isSubR;
    auto const flipNonRegL = flip ? nonRegR : nonRegL;
    auto const flipNonRegR = flip ? nonRegL : nonRegR;

    // This logic handles the unioning of two classes. If two classes
    // don't have a common parent, their union if Top, which is
    // dropped from the intersection list. For regular classes, we can
    // just get the parent list. For non-regular classes, we need to
    // use subclassOf.
    if (l->isMissing()) {
      if (l == r) combined.emplace(l);
    } else if (flipNonRegL || l->isRegular()) {
      if (flipNonRegR || r->isRegular()) {
        combine(allParents(*l), allParents(*r));
      } else if (flipSubR) {
        combine(allParents(*l), r->nonRegularInfo().subclassOf);
      } else {
        combine(allParents(*l), {});
      }
    } else if (flipSubL) {
      if (flipSubR) {
        combine(l->nonRegularInfo().subclassOf,
                r->nonRegularInfo().subclassOf);
      } else {
        combine(l->nonRegularInfo().subclassOf, {});
      }
    } else {
      always_assert(false);
    }
  };

  for (auto const l : lhs) {
    for (auto const r : rhs) process(l, r);
  }

  // The resultant list is not in canonical form, so canonicalize it.
  return canonicalize(combined, nonRegL || nonRegR);
}

// Intersect two sets of intersected classes together. This seems like
// you would just combine the lists, but the intersection of the two
// might have more specific classes in common.
ClassGraph::NodeVec ClassGraph::intersect(const NodeVec& lhs,
                                          const NodeVec& rhs,
                                          bool nonRegL,
                                          bool nonRegR,
                                          bool& nonRegOut) {
  assertx(!table().locking);
  assertx(!lhs.empty());
  assertx(!rhs.empty());

  // Combine the two lists together.
  NodeVec combined;
  combined.reserve(lhs.size() + rhs.size());
  std::set_union(
    lhs.begin(), lhs.end(),
    rhs.begin(), rhs.end(),
    std::back_inserter(combined),
    betterNode
  );

  // Build the "heads". These are the nodes which could potentially be
  // part of the new intersection list, but don't need to be
  // calculated below. We leave them out to reduce the amount of work.
  NodeSet heads;
  for (auto const n : combined) {
    ClassGraph{n}.ensure();
    if (n->isMissing()) {
      heads.emplace(n);
    } else if ((nonRegL && nonRegR) || n->isRegular()) {
      auto const p = allParents(*n);
      heads.insert(p.begin(), p.end());
    } else if (!n->hasRegularSubclass()) {
      nonRegOut = false;
      return {};
    } else {
      auto const& s = n->nonRegularInfo().subclassOf;
      heads.insert(s.begin(), s.end());
    }
  }

  // Then enumerate over all of the classes in the combined set and
  // track which parents they *all* have in common.

  Optional<ParentTracker<SmallBitset>> small;
  Optional<ParentTracker<LargeBitset>> large;
  nonRegOut = false;

  enumerateIsectMembers(
    combined,
    nonRegL && nonRegR,
    [&] (Node& n) {
      if (n.isMissing() || !n.hasCompleteChildren()) {
        if (nonRegL && nonRegR) nonRegOut = true;
      } else if (!n.isRegular()) {
        nonRegOut = true;
      } else if (!nonRegOut && nonRegL && nonRegR) {
        if (n.hasNonRegularSubclass()) nonRegOut = true;
      }

      // If we already created a tracker, use it.
      if (small) {
        return (*small)(n);
      } else if (large) {
        return (*large)(n);
      } else {
        // Otherwise this is the first time. Find the initial set of
        // candidates (all of the parents of this node minus the
        // heads) and set up the tracker.
        auto common = n.isMissing() ? NodeSet{&n} : allParents(n);
        folly::erase_if(common, [&] (Node* c) { return heads.count(c); });
        if (common.size() <= SmallBitset::kMaxSize) {
          small.emplace(common, &heads);
        } else {
          large.emplace(common, &heads);
        }
        return !common.empty();
      }
    }
  );
  assertx(IMPLIES(!nonRegL || !nonRegR, !nonRegOut));

  // At this point the tracker only contains all of the parents which
  // are in common. These nodes, plus the heads, only need to be
  // canonicalized.
  if (small) {
    auto s = small->nodes();
    s.insert(heads.begin(), heads.end());
    return canonicalize(s, nonRegOut);
  } else if (large) {
    auto s = large->nodes();
    s.insert(heads.begin(), heads.end());
    return canonicalize(s, nonRegOut);
  } else {
    return NodeVec{};
  }
}

// Given a set of intersected nodes, return an equivalent set with all
// of the non-regular classes removed.
ClassGraph::NodeVec ClassGraph::removeNonReg(const NodeVec& v) {
  assertx(!table().locking);
  assertx(!v.empty());

  // We can just treat this as an intersection:
  NodeVec lhs;
  NodeVec rhs;
  for (auto const n : v) {
    ClassGraph{n}.ensure();
    if (n->isMissing() || n->isRegular()) {
      lhs.emplace_back(n);
    } else if (auto const e = n->nonRegularInfo().regOnlyEquiv) {
      lhs.emplace_back(e);
    } else {
      return NodeVec{};
    }
  }
  if (lhs.size() <= 1) return lhs;

  std::sort(lhs.begin(), lhs.end(), betterNode);
  rhs.emplace_back(lhs.back());
  lhs.pop_back();

  auto nonRegOut = false;
  SCOPE_EXIT { assertx(!nonRegOut); };
  return intersect(lhs, rhs, false, false, nonRegOut);
}

// Given two lists of intersected classes, return true if the
// intersection of the two lists might be non-empty.
bool ClassGraph::couldBeIsect(const NodeVec& lhs,
                              const NodeVec& rhs,
                              bool nonRegL,
                              bool nonRegR) {
  assertx(!table().locking);
  assertx(!lhs.empty());
  assertx(!rhs.empty());

  NodeVec combined;
  combined.reserve(lhs.size() + rhs.size());
  std::set_union(
    lhs.begin(), lhs.end(),
    rhs.begin(), rhs.end(),
    std::back_inserter(combined),
    betterNode
  );

  auto couldBe = false;
  enumerateIsectMembers(
    combined,
    nonRegL && nonRegR,
    [&] (Node&) {
      couldBe = true;
      return false;
    }
  );
  return couldBe;
}

// Call the provided callback for each Node in the given intersection
// list. If "all" is true, all Nodes are provided. Otherwise, only the
// "top most" children are provided (the children which meet the
// criteria and don't have any parents which do).
template <typename F>
void ClassGraph::enumerateIsectMembers(const NodeVec& nodes,
                                       bool nonReg,
                                       const F& f,
                                       bool all) {
  assertx(!table().locking);

  if (nodes.empty()) return;

  if (table().index) {
    for (auto const n : nodes) ClassGraph{n}.ensure();
  }

  // Find the "best" node to start with.
  auto head = *std::min_element(nodes.begin(), nodes.end(), betterNode);
  if (head->isMissing() || !head->hasCompleteChildren()) {
    // If the best node is missing or doesn't have complete children,
    // they all don't, so just supply the input list.
    for (auto const n : nodes) {
      assertx(n->isMissing() || !n->hasCompleteChildren());
      if (!f(*n)) break;
    }
    return;
  }

  // Otherwise report Nodes which are children of all of the Nodes.
  auto const run = [&] (auto tracker) {
    ClassGraph{head}.ensureWithChildren();
    forEachChild(
      *head,
      [&] (Node& c) {
        if (nonReg || c.isRegular()) {
          if (tracker.all(c)) {
            if (!f(c)) return Action::Stop;
            return all ? Action::Continue : Action::Skip;
          }
        }
        return Action::Continue;
      }
    );
  };

  if (nodes.size() <= SmallBitset::kMaxSize) {
    return run(ParentTracker<SmallBitset>{nodes});
  } else {
    return run(ParentTracker<LargeBitset>{nodes});
  }
}

// "Canonicalize" a set of intersected classes by removing redundant
// Nodes and putting them in a deterministic order.
ClassGraph::NodeVec ClassGraph::canonicalize(const NodeSet& nodes,
                                             bool nonReg) {
  NodeVec out;
  if (nodes.size() <= 1) {
    // Trivial cases
    out.insert(begin(nodes), end(nodes));
    return out;
  }

  // Remove redundant nodes. A node is redundant if it is reachable
  // via another node. In that case, this node is implied by the other
  // and can be removed.
  auto heads = nodes;
  NodeSet visited;
  for (auto const n : nodes) {
    ClassGraph{n}.ensure();
    if (!heads.count(n)) continue;
    if (n->isMissing()) continue;
    forEachParent(
      *n,
      [&] (Node& p) {
        if (&p == n) return Action::Continue;
        if (!nodes.count(&p)) return Action::Continue;
        if (!heads.count(&p)) return Action::Skip;
        heads.erase(&p);
        return Action::Continue;
      },
      visited
    );
    visited.erase(n);
  }

  // A node can be redundant with another even they aren't reachable
  // via each other. This can only happen if we're considering only
  // regular nodes. If a class is a super type of another, it's
  // redundant, as the other class implies this class. If they classes
  // are both super types of each other, they're equivalent, and we
  // keep the "best" one.
  if (!nonReg) {
    for (auto const n1 : heads) {
      auto const hasSuperType = [&] {
        for (auto const n2 : heads) {
          if (n1 == n2) continue;
          if (!ClassGraph{n2}.subSubtypeOf(ClassGraph{n1}, false, false)) {
            continue;
          }
          if (!ClassGraph{n1}.subSubtypeOf(ClassGraph{n2}, false, false)) {
            return true;
          }
          if (betterNode(n2, n1)) return true;
        }
        return false;
      }();
      if (!hasSuperType) out.emplace_back(n1);
    }
  } else {
    out.insert(begin(heads), end(heads));
  }

  // Finally sort them according to how good they are.
  std::sort(out.begin(), out.end(), betterNode);
  return out;
}

template <typename F>
ClassGraph::Action ClassGraph::forEachParentImpl(Node& n,
                                                 const F& f,
                                                 NodeSet& v,
                                                 bool start) {
  assertx(!n.isMissing());
  if (!v.emplace(&n).second) return Action::Skip;
  if (start || !n.isTrait()) {
    auto const action = f(n);
    if (action != Action::Continue) return action;
  }
  for (auto const parent : n.parents) {
    if (forEachParentImpl(*parent, f, v, false) == Action::Stop) {
      return Action::Stop;
    }
  }
  return Action::Continue;
}

template <typename F, typename F2, typename T>
T ClassGraph::foldParentsImpl(Node& n,
                              const F& f,
                              const F2& f2,
                              NodeMap<T>& m,
                              bool start) {
  assertx(!n.isMissing());
  if (auto const t = folly::get_ptr(m, &n)) return *t;
  T t = (start || !n.isTrait()) ? f(n) : f2();
  for (auto const parent : n.parents) {
    if (t) break;
    t |= foldParentsImpl(*parent, f, f2, m, false);
  }
  m.insert_or_assign(&n, t);
  return t;
}

bool ClassGraph::findParent(Node& start, Node& target, NodeSet& visited) {
  assertx(!start.isMissing());
  auto const action = forEachParent(
    start,
    [&] (Node& p) { return (&p == &target) ? Action::Stop : Action::Continue; },
    visited
  );
  return action == Action::Stop;
}

ClassGraph::NodeSet ClassGraph::allParents(Node& n) {
  assertx(!n.isMissing());
  NodeSet s;
  forEachParent(
    n,
    [&] (Node& p) {
      s.emplace(&p);
      return Action::Continue;
    }
  );
  return s;
}

template <typename F>
ClassGraph::Action
ClassGraph::forEachChildImpl(Node& n, const F& f, NodeSet& v, bool start) {
  assertx(!n.isMissing());
  if (!v.emplace(&n).second) return Action::Skip;
  auto const action = f(n);
  if (action != Action::Continue) return action;
  if (start && n.isTrait()) return action;
  for (auto const child : n.children) {
    if (forEachChildImpl(*child, f, v, false) == Action::Stop) {
      return Action::Stop;
    }
  }
  return Action::Continue;
}

ClassGraph ClassGraph::create(const php::Class& cls) {
  assertx(!table().locking);

  auto const& [n, emplaced] = table().nodes.try_emplace(cls.name);
  always_assert_flog(
    emplaced,
    "Attempting to create already existing ClassGraph node '{}'",
    cls.name
  );
  assertx(!n->second.name);
  assertx(n->second.flags() == FlagNone);
  assertx(!n->second.cinfo());
  assertx(!n->second.cinfo2());
  n->second.name = cls.name;

  auto f = FlagNone;
  if (cls.attrs & AttrInterface)              f = Flags(f | FlagInterface);
  if (cls.attrs & AttrTrait)                  f = Flags(f | FlagTrait);
  if (cls.attrs & AttrAbstract)               f = Flags(f | FlagAbstract);
  if (cls.attrs & (AttrEnum | AttrEnumClass)) f = Flags(f | FlagEnum);
  n->second.setFlags(f);

  return ClassGraph{ &n->second };
}

ClassGraph ClassGraph::get(SString name) {
  assertx(!table().locking);
  auto n = folly::get_ptr(table().nodes, name);
  always_assert_flog(
    n,
    "Attempting to retrieve missing ClassGraph node '{}'",
    name
  );
  assertx(n->name->tsame(name));
  return ClassGraph{ n };
}

ClassGraph ClassGraph::getOrCreate(SString name) {
  assertx(!table().locking);

  auto const& [n, emplaced] = table().nodes.try_emplace(name);
  if (emplaced) {
    assertx(!n->second.name);
    assertx(n->second.flags() == FlagNone);
    assertx(!n->second.cinfo());
    assertx(!n->second.cinfo2());
    n->second.name = name;
    n->second.setFlags(FlagMissing);
  } else {
    assertx(n->second.name->tsame(name));
  }
  return ClassGraph{ &n->second };
}

ClassGraph ClassGraph::getMissing(SString name) {
  assertx(!table().locking);

  auto const& [n, emplaced] = table().nodes.try_emplace(name);
  if (emplaced) {
    assertx(!n->second.name);
    assertx(n->second.flags() == FlagNone);
    assertx(!n->second.cinfo());
    assertx(!n->second.cinfo2());
    n->second.name = name;
    n->second.setFlags(FlagMissing);
  } else {
    assertx(n->second.name->tsame(name));
    assertx(n->second.flags() == FlagMissing);
  }
  return ClassGraph{ &n->second };
}

void ClassGraph::init() {
  always_assert(!g_table);
  g_table = std::make_unique<Table>();
}

void ClassGraph::initConcurrent() {
  always_assert(!g_table);
  g_table = std::make_unique<Table>();
  g_table->locking.emplace();
}

void ClassGraph::stopConcurrent() {
  always_assert(g_table);
  g_table->locking.reset();
}

void ClassGraph::destroy() {
  assertx(IMPLIES(g_table, !g_table->index));
  g_table.reset();
}

ClassGraph::Table& ClassGraph::table() {
  always_assert_flog(
    g_table,
    "Attempting to access ClassGraph node table when one isn't active!"
  );
  return *g_table;
}

void ClassGraph::setAnalysisIndex(AnalysisIndex::IndexData& index) {
  assertx(!table().locking);
  assertx(!table().index);
  table().index = &index;
}

void ClassGraph::clearAnalysisIndex() {
  assertx(!table().locking);
  assertx(table().index);
  table().index = nullptr;
}

// Set the FlagComplete/FlagConservative and FlagRegSub/FlagNonRegSub
// flags in this class and it's children. Return the
// FlagRegSub/FlagNonRegSub flags from the children, along with the
// count of the subclass list from that child. If std::nullopt is
// given as the count, we've exceeded the limit we want to track.
template <typename Impl>
std::pair<ClassGraph::Flags, Optional<size_t>>
ClassGraph::setCompleteImpl(const Impl& impl, Node& n) {
  assertx(!n.isMissing());

  // Conservative nodes don't have children. However, they're
  // guaranteed to have their FlagRegSub and FlagNonRegSub flags set
  // properly, so we don't need to.
  if (n.isConservative()) {
    assertx(n.children.empty());
    auto f = FlagNone;
    if (n.isRegular() || n.hasRegularSubclass()) {
      f = (Flags)(f | FlagRegSub);
    }
    if (!n.isRegular() || n.hasNonRegularSubclass()) {
      f = (Flags)(f | FlagNonRegSub);
    }
    return std::make_pair(f, std::nullopt);
  }

  // Otherwise aggregate the flags and counts from the children.
  auto flags = FlagNone;
  Optional<size_t> count;
  count.emplace(1);

  // Copy the children list for concurrency safety.
  NodeSet children;
  impl.lock(n, [&] { children = n.children; });
  for (auto const child : children) {
    auto const [f, c] = setCompleteImpl(impl, *child);
    flags = (Flags)(flags | f);
    if (count) {
      if (c) {
        *count += *c;
      } else {
        count.reset();
      }
    }
  }

  if (!count || *count > options.preciseSubclassLimit) {
    // The child is conservative, or we've exceeded the subclass list
    // limit. Mark this node as being conservative.
    assertx(!n.hasCompleteChildren());
    impl.updateFlags(n, (Flags)(flags | FlagConservative));
    impl.lock(n, [&] { n.children.clear(); });
    count.reset();
  } else if (n.hasCompleteChildren()) {
    // Otherwise if this node is already marked as having complete
    // children, verify we inferred the same thing already stored
    // here.
    assertx(n.hasRegularSubclass() == bool(flags & FlagRegSub));
    assertx(n.hasNonRegularSubclass() == bool(flags & FlagNonRegSub));
  } else {
    // Didn't have complete children, but now does. Update the flags.
    impl.updateFlags(n, (Flags)(flags | FlagChildren));
  }

  if (n.isRegular())  flags = (Flags)(flags | FlagRegSub);
  if (!n.isRegular()) flags = (Flags)(flags | FlagNonRegSub);
  return std::make_pair(flags, count);
}

template <typename SerDe, typename T>
void ClassGraph::serde(SerDe& sd, T cinfo) {
  // Serialization/deserialization entry point. If we're operating
  // concurrently, use one Impl, otherwise, use the other.
  if (SerDe::deserializing && table().locking) {
    serdeImpl(sd, LockedSerdeImpl{}, cinfo);
  } else {
    serdeImpl(sd, UnlockedSerdeImpl{}, cinfo);
  }
}

template <typename SerDe, typename Impl, typename T>
void ClassGraph::serdeImpl(SerDe& sd,
                           const Impl& impl,
                           T cinfo) {
  // Allocate SerdeState if someone else hasn't already.
  ScopedSerdeState _;

  if constexpr (SerDe::deserializing) {
    // Deserializing:

    // First ensure that all nodes reachable by this node are
    // deserialized.
    sd.readWithLazyCount([&] { deserBlock(sd, impl); });

    // Then obtain a pointer to the node that ClassGraph points
    // to.
    if (auto const name = sd.template make<const StringData*>()) {
      this_ = &impl.get(name);

      // If this node was marked as having complete children (and
      // we're not ignoring that), mark this node and all of it's
      // transitive children as also having complete children.
      bool complete;
      sd(complete);
      if (complete && !this_->hasCompleteChildren()) {
        assertx(!this_->isConservative());
        setCompleteImpl(impl, *this_);
        assertx(this_->hasCompleteChildren());
      }

      // If this node isn't regular, and we've recorded an equivalent
      // node for it, make sure that the equivalent is
      // hasCompleteChildren(), as that's an invariant.
      if (auto const equiv = sd.template make<const StringData*>()) {
        auto const equivNode = &impl.get(equiv);
        assertx(!equivNode->isRegular());
        if (!equivNode->hasCompleteChildren()) {
          assertx(!equivNode->isConservative());
          setCompleteImpl(impl, *equivNode);
          assertx(equivNode->hasCompleteChildren());
        }
        impl.setEquiv(*this_, *equivNode);
      }

      if constexpr (!std::is_null_pointer_v<T>) {
        if (cinfo) {
          assertx(!this_->isMissing());
          impl.setCInfo(*this_, *cinfo);
        }
      }
    } else {
      this_ = nullptr;
    }
  } else {
    // Serializing:

    // Serialize all of the nodes reachable by this node (parents,
    // children, and parents of children) and encode how many.
    sd.lazyCount(
      [&] () -> size_t {
        if (!this_) return 0;
        auto count = serDownward(sd, *this_);
        if (auto const e = folly::get_default(table().regOnlyEquivs, this_)) {
          assertx(!this_->isRegular());
          assertx(e->hasCompleteChildren());
          count += serDownward(sd, *e);
        }
        return count;
      }
    );
    // Encode the "entry-point" into the graph represented by this
    // ClassGraph.
    if (this_) {
      sd(this_->name);
      // Record whether this node has complete children, so we can
      // reconstruct that when deserializing.
      assertx(IMPLIES(hasCompleteChildren(), !isConservative()));
      assertx(IMPLIES(isConservative(), this_->children.empty()));
      sd(this_->hasCompleteChildren());

      // If this Node isn't regular and has an equivalent node, record
      // that here.
      if (!this_->isMissing() && !this_->isRegular()) {
        if (auto const e = folly::get_default(table().regOnlyEquivs, this_)) {
          assertx(e->hasCompleteChildren());
          sd(e->name);
        } else {
          sd((const StringData*)nullptr);
        }
      } else {
        sd((const StringData*)nullptr);
      }
    } else {
      sd((const StringData*)nullptr);
    }
  }
}

// Sort a hphp_fast_set of nodes to a vector. Used to ensure
// deterministic serialization of graph edges.
std::vector<ClassGraph::Node*> ClassGraph::sort(const hphp_fast_set<Node*>& v) {
  std::vector<Node*> sorted{begin(v), end(v)};
  std::sort(
    begin(sorted), end(sorted),
    [] (const Node* a, const Node* b) {
      assertx(a->name);
      assertx(b->name);
      return string_data_lt_type{}(a->name, b->name);
    }
  );
  return sorted;
}

// Deserialize a node, along with any other nodes it depends on.
template <typename SerDe, typename Impl>
void ClassGraph::deserBlock(SerDe& sd, const Impl& impl) {
  // First get the name for this node.
  auto const name = sd.template make<const StringData*>();
  assertx(name);

  // Try to create it:
  auto const [node, created] = impl.create(name);

  // Either it already existed and we got an existing Node, or we
  // created it. Even if it already existed, we still need to process
  // it below as if it was new, because this might have additional
  // flags to add to the Node.

  // Deserialize dependent nodes.
  sd.readWithLazyCount([&] { deserBlock(sd, impl); });

  // At this point all dependent nodes are guaranteed to exist.

  // Read the parent links. The children links are not encoded as
  // they can be inferred from the parent links.
  std::vector<SString> parents;
  {
    size_t size;
    sd(size);
    parents.reserve(size);
    for (size_t i = 0; i < size; ++i) {
      parents.emplace_back(sd.template make<const StringData*>());
    }
  }

  // For each parent, register this node as a child. Lock the
  // appropriate node if we're concurrent deserializing.
  for (auto const parent : parents) {
    // This should always succeed because all dependents should
    // exist.
    auto& parentNode = impl.get(parent);
    impl.lock(
      *node,
      [&, node=node] { node->parents.emplace(&parentNode); }
    );
    impl.lock(
      parentNode,
      [&, node=node] {
        if (!parentNode.isConservative()) parentNode.children.emplace(node);
      }
    );
  }

  Flags flags;
  sd(flags);
  // These flags are never encoded and only exist at runtime.
  assertx(
    !(flags & (FlagWait | FlagChildren | FlagCInfo2))
  );
  // If this is a "missing" node, it shouldn't have any links
  // (because we shouldn't know anything about it).
  assertx(IMPLIES(flags & FlagMissing, parents.empty()));
  assertx(IMPLIES(flags & FlagMissing, flags == FlagMissing));

  if (created) {
    // If we created this node, we need to clear FlagWait and
    // simultaneously set the node's flags to what we decoded.
    impl.signal(*node, flags);
  } else if (node->isMissing()) {
    if (!(flags & FlagMissing)) impl.updateFlags(*node, flags, FlagMissing);
  } else if (node->isConservative()) {
    assertx(!(flags & FlagConservative) ||
            ((node->flags() & ~(FlagChildren | FlagCInfo2))) == flags);
  } else if (!node->hasCompleteChildren()) {
    impl.updateFlags(*node, flags, FlagMissing);
    if (node->isConservative()) {
      impl.lock(*node, [node=node] { node->children.clear(); });
    }
  }
}

// Walk downward through a node's children until we hit a leaf. At
// that point, we call serUpward on the leaf, which will serialize it
// and all of it's parents (which should include all nodes traversed
// here). Return the number of nodes serialized.
template <typename SerDe>
size_t ClassGraph::serDownward(SerDe& sd, Node& n) {
  assertx(!table().locking);
  assertx(tl_serde_state);
  if (!tl_serde_state->downward.emplace(&n).second) return 0;

  if (n.children.empty()) return serUpward(sd, n);

  size_t count = 0;
  for (auto const child : sort(n.children)) {
    count += serDownward(sd, *child);
  }
  return count;
}

// Serialize the given node, along with all of it's parents. Return
// true if anything was serialized.
template <typename SerDe>
bool ClassGraph::serUpward(SerDe& sd, Node& n) {
  assertx(!table().locking);
  assertx(tl_serde_state);
  // If we've already serialized this node, no need to serialize it
  // again.
  if (!tl_serde_state->upward.emplace(&n).second) return false;

  assertx(n.name);
  assertx(IMPLIES(n.isMissing(), n.parents.empty()));
  assertx(IMPLIES(n.isMissing(), n.children.empty()));
  assertx(IMPLIES(n.isMissing(), n.flags() == FlagMissing));
  assertx(IMPLIES(n.isConservative(), n.children.empty()));

  sd(n.name);

  // Sort the parents into a deterministic order.
  auto const sorted = sort(n.parents);

  // Recursively serialize all parents of this node. This ensures
  // that when deserializing, the parents will be available before
  // deserializing this node.
  sd.lazyCount(
    [&] {
      size_t count = 0;
      for (auto const parent : sorted) {
        count += serUpward(sd, *parent);
      }
      return count;
    }
  );

  // Record the names of the parents, to restore the links when
  // deserializing.
  sd(sorted.size());
  for (auto const p : sorted) {
    assertx(p->name);
    sd(p->name);
  }
  // Shouldn't have any FlagWait when serializing.
  assertx(!(n.flags() & FlagWait));
  // These are only set at runtime, so shouldn't be serialized.
  sd((Flags)(n.flags() & ~(FlagChildren | FlagCInfo2)));

  return true;
}

//////////////////////////////////////////////////////////////////////

template <typename SerDe> void FuncInfo2::serde(SerDe& sd) {
  ScopedStringDataIndexer _;
  ClassGraph::ScopedSerdeState _2;
  sd(name)
    (returnTy)
    (retParam)
    (returnRefinements)
    (effectFree)
    (unusedParams)
    ;

  if constexpr (SerDe::deserializing) {
    bool present;
    sd(present);
    using T = decltype(auxClassGraphs)::element_type;
    if (present) {
      auxClassGraphs =
        std::make_unique<T>(sd.template make<T>(std::less<>{}, nullptr));
    }
  } else {
    sd((bool)auxClassGraphs);
    if (auxClassGraphs) sd(*auxClassGraphs, std::less<>{}, nullptr);
  }
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
   * The traits used by this class, which *haven't* been flattened
   * into it. If the class is AttrNoExpandTrait, this will always be
   * empty.
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

  ClassGraph classGraph;

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
   * php::Class associated with this ClassInfo. Must be set manually
   * when needed.
   */
  const php::Class* cls{nullptr};

  /*
   * A (case-sensitive) map from class constant name to the ConstIndex
   * representing the constant. This map is flattened across the
   * inheritance hierarchy.
   */
  SStringToOneT<ConstIndex> clsConstants;

  /*
   * Inferred information about a class constant declared on this
   * class (not flattened).
   */
  SStringToOneT<ClsConstInfo> clsConstantInfo;

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
   * ClassGraph representing this class. This ClassGraph is guaranteed
   * to have hasCompleteChildren() be true (except if this is the
   * Closure base class).
   */
  ClassGraph classGraph;

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
   * If we utilize a ClassGraph while resolving types, we store it
   * here. This ensures that that ClassGraph will always be available
   * again.
   */
  hphp_fast_set<ClassGraph, ClassGraphHasher> auxClassGraphs;

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

  template <typename SerDe> void serde(SerDe& sd) {
    ScopedStringDataIndexer _;
    ClassGraph::ScopedSerdeState _2;
    sd(name)
      (parent)
      (clsConstants, string_data_lt{})
      (clsConstantInfo, string_data_lt{})
      (traitProps)
      (methods, string_data_lt{})
      (missingMethods, string_data_lt{})
      (classGraph, this)
      (extraMethods, std::less<MethRef>{})
      (closures)
      (methodFamilies, string_data_lt{})
      (funcInfos)
      (auxClassGraphs, std::less<>{}, nullptr)
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
      ;
  }
};

//////////////////////////////////////////////////////////////////////

namespace res {

Class::Class(ClassGraph g): opaque{g.this_} {
  assertx(g.this_);
}

ClassGraph Class::graph() const {
  assertx(opaque.p);
  return ClassGraph{ (ClassGraph::Node*)opaque.p };
}

ClassInfo* Class::cinfo() const {
  return graph().cinfo();
}

ClassInfo2* Class::cinfo2() const {
  return graph().cinfo2();
}

bool Class::same(const Class& o) const {
  return graph() == o.graph();
}

bool Class::exactSubtypeOfExact(const Class& o,
                                bool nonRegL,
                                bool nonRegR) const {
  return graph().exactSubtypeOfExact(o.graph(), nonRegL, nonRegR);
}

bool Class::exactSubtypeOf(const Class& o, bool nonRegL, bool nonRegR) const {
  return graph().exactSubtypeOf(o.graph(), nonRegL, nonRegR);
}

bool Class::subSubtypeOf(const Class& o, bool nonRegL, bool nonRegR) const {
  return graph().subSubtypeOf(o.graph(), nonRegL, nonRegR);
}

bool Class::exactCouldBeExact(const Class& o,
                              bool nonRegL,
                              bool nonRegR) const {
  return graph().exactCouldBeExact(o.graph(), nonRegL, nonRegR);
}

bool Class::exactCouldBe(const Class& o, bool nonRegL, bool nonRegR) const {
  return graph().exactCouldBe(o.graph(), nonRegL, nonRegR);
}

bool Class::subCouldBe(const Class& o, bool nonRegL, bool nonRegR) const {
  return graph().subCouldBe(o.graph(), nonRegL, nonRegR);
}

SString Class::name() const { return graph().name(); }

Optional<res::Class> Class::withoutNonRegular() const {
  if (auto const g = graph().withoutNonRegular()) {
    return Class { g };
  } else {
    return std::nullopt;
  }
}

bool Class::mightBeRegular()    const { return graph().mightBeRegular(); }
bool Class::mightBeNonRegular() const { return graph().mightBeNonRegular(); }

bool Class::couldBeOverridden() const {
  if (!graph().isMissing() && graph().isTrait()) return false;
  return
    graph().mightHaveRegularSubclass() ||
    graph().mightHaveNonRegularSubclass();
}

bool Class::couldBeOverriddenByRegular() const {
  if (!graph().isMissing() && graph().isTrait()) return false;
  return graph().mightHaveRegularSubclass();
}

bool Class::mightContainNonRegular() const {
  return graph().mightBeNonRegular() || graph().mightHaveNonRegularSubclass();
}

bool Class::couldHaveMagicBool() const {
  auto const g = graph();
  g.ensure();
  if (g.isMissing()) return true;
  if (!g.isInterface()) return has_magic_bool_conversion(g.topBase().name());
  g.ensureWithChildren();
  if (!g.hasCompleteChildren()) return true;
  for (auto const c : g.children()) {
    if (has_magic_bool_conversion(c.topBase().name())) return true;
  }
  return false;
}

bool Class::couldHaveMockedSubClass() const {
  graph().ensureCInfo();
  if (auto const ci = cinfo()) {
    return ci->isSubMocked;
  } else if (auto const ci = cinfo2()) {
    return ci->isSubMocked;
  } else {
    return true;
  }
}

bool Class::couldBeMocked() const {
  graph().ensureCInfo();
  if (auto const ci = cinfo()) {
    return ci->isMocked;
  } else if (auto const ci = cinfo2()) {
    return ci->isMocked;
  } else {
    return true;
  }
}

bool Class::couldHaveReifiedGenerics() const {
  graph().ensureCInfo();
  if (auto const ci = cinfo()) {
    return ci->cls->hasReifiedGenerics;
  } else if (auto const ci = cinfo2()) {
    return ci->hasReifiedGeneric;
  } else {
    return true;
  }
}

bool Class::mustHaveReifiedGenerics() const {
  graph().ensureCInfo();
  if (auto const ci = cinfo()) {
    return ci->cls->hasReifiedGenerics;
  } else if (auto const ci = cinfo2()) {
    return ci->hasReifiedGeneric;
  } else {
    return false;
  }
}

bool Class::couldHaveReifiedParent() const {
  graph().ensureCInfo();
  if (auto const ci = cinfo()) {
    return ci->hasReifiedParent;
  } else if (auto const ci = cinfo2()) {
    return ci->hasReifiedParent;
  } else {
    return true;
  }
}

bool Class::mustHaveReifiedParent() const {
  graph().ensureCInfo();
  if (auto const ci = cinfo()) {
    return ci->hasReifiedParent;
  } else if (auto const ci = cinfo2()) {
    return ci->hasReifiedParent;
  } else {
    return false;
  }
}

bool Class::mightCareAboutDynConstructs() const {
  if (!RuntimeOption::EvalForbidDynamicConstructs) return false;
  graph().ensureCInfo();
  if (auto const ci = cinfo()) {
    return !(ci->cls->attrs & AttrDynamicallyConstructible);
  } else if (auto const ci = cinfo2()) {
    return !ci->cls || !(ci->cls->attrs & AttrDynamicallyConstructible);
  } else {
    return true;
  }
}

bool Class::couldHaveConstProp() const {
  graph().ensureCInfo();
  if (auto const ci = cinfo()) {
    return ci->hasConstProp;
  } else if (auto const ci = cinfo2()) {
    return ci->hasConstProp;
  } else {
    return true;
  }
}

bool Class::subCouldHaveConstProp() const {
  graph().ensureCInfo();
  if (auto const ci = cinfo()) {
    return ci->subHasConstProp;
  } else if (auto const ci = cinfo2()) {
    return ci->subHasConstProp;
  } else {
    return true;
  }
}

Optional<res::Class> Class::parent() const {
  if (auto const p = graph().base()) {
    return Class { p };
  } else {
    return std::nullopt;
  }
}

const php::Class* Class::cls() const {
  graph().ensureCInfo();
  if (auto const ci = cinfo()) {
    return ci->cls;
  } else if (auto const ci = cinfo2()) {
    return ci->cls;
  } else {
    return nullptr;
  }
}

bool Class::hasCompleteChildren() const {
  return graph().hasCompleteChildren();
}

bool Class::isComplete() const {
  return
    !graph().isMissing() &&
    (graph().hasCompleteChildren() || graph().isConservative());
}

bool
Class::forEachSubclass(const std::function<void(SString, Attr)>& f) const {
  auto const g = graph();
  g.ensureWithChildren();
  if (g.isMissing() || !g.hasCompleteChildren()) return false;
  for (auto const c : g.children()) {
    auto const attrs = [&] {
      if (c.isInterface()) return AttrInterface;
      if (c.isTrait())     return AttrTrait;
      if (c.isEnum())      return AttrEnum;
      if (c.isAbstract())  return AttrAbstract;
      return AttrNone;
    }();
    f(c.name(), attrs);
  }
  return true;
}

std::string show(const Class& c) {
  if (!c.isComplete()) {
    return folly::sformat("\"{}\"", c.graph().name());
  }
  if (!c.cinfo() && !c.cinfo2()) {
    return folly::sformat("*{}", c.graph().name());
  }
  return c.graph().name()->toCppString();
}

// Call the given callable for every class which is a subclass of
// *all* the classes in the range. If the range includes nothing but
// unresolved classes, they will be passed, as-is, to the callable. If
// the range includes a mix of resolved and unresolved classes, the
// unresolved classes will be used to narrow the classes passed to the
// callable, but the unresolved classes themself will not be passed to
// the callable. If the callable returns false, iteration is
// stopped. If includeNonRegular is true, non-regular subclasses are
// visited (normally they are skipped).
template <typename F>
void Class::visitEverySub(folly::Range<const Class*> classes,
                          bool includeNonRegular,
                          const F& f) {
  if (classes.size() == 1) {
    auto const n = classes[0].graph().this_;
    ClassGraph{n}.ensureWithChildren();
    if (n->isMissing() || !n->hasCompleteChildren()) {
      f(Class{ClassGraph{ n }});
      return;
    }
    ClassGraph::forEachChild(
      *n,
      [&] (ClassGraph::Node& c) {
        assertx(!c.isMissing());
        if (includeNonRegular || c.isRegular()) {
          if (!f(Class{ClassGraph{ &c }})) return ClassGraph::Action::Stop;
        }
        return ClassGraph::Action::Continue;
      }
    );
    return;
  }

  ClassGraph::NodeVec v;
  v.reserve(classes.size());
  for (auto const c : classes) v.emplace_back(c.graph().this_);

  ClassGraph::enumerateIsectMembers(
    v,
    includeNonRegular,
    [&] (ClassGraph::Node& c) { return f(Class{ClassGraph{ &c }}); },
    true
  );
}

Class::ClassVec Class::combine(folly::Range<const Class*> classes1,
                               folly::Range<const Class*> classes2,
                               bool isSub1,
                               bool isSub2,
                               bool nonRegular1,
                               bool nonRegular2) {
  ClassGraph::NodeVec v1;
  ClassGraph::NodeVec v2;
  v1.reserve(classes1.size());
  v2.reserve(classes2.size());
  for (auto const c : classes1) v1.emplace_back(c.graph().this_);
  for (auto const c : classes2) v2.emplace_back(c.graph().this_);
  auto const i =
    ClassGraph::combine(v1, v2, isSub1, isSub2, nonRegular1, nonRegular2);
  ClassVec out;
  out.reserve(i.size());
  for (auto const c : i) out.emplace_back(Class{ClassGraph { c }});
  return out;
}

Class::ClassVec Class::removeNonRegular(folly::Range<const Class*> classes) {
  ClassGraph::NodeVec v;
  v.reserve(classes.size());
  for (auto const c : classes) v.emplace_back(c.graph().this_);
  auto const i = ClassGraph::removeNonReg(v);
  ClassVec out;
  out.reserve(i.size());
  for (auto const c : i) out.emplace_back(Class{ClassGraph { c }});
  return out;
}

Class::ClassVec Class::intersect(folly::Range<const Class*> classes1,
                                 folly::Range<const Class*> classes2,
                                 bool nonRegular1,
                                 bool nonRegular2,
                                 bool& nonRegularOut) {
  ClassGraph::NodeVec v1;
  ClassGraph::NodeVec v2;
  v1.reserve(classes1.size());
  v2.reserve(classes2.size());
  for (auto const c : classes1) v1.emplace_back(c.graph().this_);
  for (auto const c : classes2) v2.emplace_back(c.graph().this_);
  auto const i =
    ClassGraph::intersect(v1, v2, nonRegular1, nonRegular2, nonRegularOut);
  ClassVec out;
  out.reserve(i.size());
  for (auto const c : i) out.emplace_back(Class{ClassGraph { c }});
  return out;
}

bool Class::couldBeIsect(folly::Range<const Class*> classes1,
                         folly::Range<const Class*> classes2,
                         bool nonRegular1,
                         bool nonRegular2) {
  ClassGraph::NodeVec v1;
  ClassGraph::NodeVec v2;
  v1.reserve(classes1.size());
  v2.reserve(classes2.size());
  for (auto const c : classes1) v1.emplace_back(c.graph().this_);
  for (auto const c : classes2) v2.emplace_back(c.graph().this_);
  return ClassGraph::couldBeIsect(v1, v2, nonRegular1, nonRegular2);
}

Class Class::get(SString name) {
  return Class{ ClassGraph::get(name) };
}

Class Class::get(const ClassInfo& cinfo) {
  assertx(cinfo.classGraph);
  return Class{ cinfo.classGraph };
}

Class Class::get(const ClassInfo2& cinfo) {
  assertx(cinfo.classGraph);
  return Class{ cinfo.classGraph };
}

Class Class::getOrCreate(SString name) {
  return Class{ ClassGraph::getOrCreate(name) };
}

Class Class::getUnresolved(SString name) {
  return Class{ ClassGraph::getMissing(name) };
}

void Class::serde(BlobEncoder& sd) const {
  sd(graph(), nullptr);
}

Class Class::makeForSerde(BlobDecoder& sd) {
  ClassGraph g;
  sd(g, nullptr);
  assertx(g.this_);
  return Class{ Opaque { g.this_ } };
}

//////////////////////////////////////////////////////////////////////

Func::Func(Rep val)
  : val(val)
{}

std::string Func::name() const {
  return match<std::string>(
    val,
    [] (FuncName s)   { return s.name->toCppString(); },
    [] (MethodName s) {
      if (s.cls) return folly::sformat("{}::{}", s.cls, s.name);
      return folly::sformat("???::{}", s.name);
    },
    [] (Fun f)        { return f.finfo->func->name->toCppString(); },
    [] (Fun2 f)       { return f.finfo->name->toCppString(); },
    [] (Method m)     { return func_fullname(*m.finfo->func); },
    [] (Method2 m)    { return func_fullname(*m.finfo->func); },
    [] (MethodFamily fam) {
      return folly::sformat(
        "*::{}",
        fam.family->possibleFuncs().front().ptr()->name
      );
    },
    [] (MethodFamily2 fam)  {
      return folly::sformat(
        "{}::{}",
        fam.family->m_id,
        fam.family->m_name
      );
    },
    [] (MethodOrMissing m)  { return func_fullname(*m.finfo->func); },
    [] (MethodOrMissing2 m) { return func_fullname(*m.finfo->func); },
    [] (MissingFunc m)      { return m.name->toCppString(); },
    [] (MissingMethod m)    {
      if (m.cls) return folly::sformat("{}::{}", m.cls, m.name);
      return folly::sformat("???::{}", m.name);
    },
    [] (const Isect& i) {
      assertx(i.families.size() > 1);
      return func_fullname(*i.families[0]->possibleFuncs().front().ptr());
    },
    [] (const Isect2& i) {
      assertx(i.families.size() > 1);
      using namespace folly::gen;
      return folly::sformat(
        "{}::{}",
        from(i.families)
          | map([] (const FuncFamily2* ff) { return ff->m_id.toString(); })
          | unsplit<std::string>("&"),
        i.families[0]->m_name
      );
    }
  );
}

const php::Func* Func::exactFunc() const {
  using Ret = const php::Func*;
  return match<Ret>(
    val,
    [] (FuncName)                    { return Ret{}; },
    [] (MethodName)                  { return Ret{}; },
    [] (Fun f)                       { return f.finfo->func; },
    [] (Fun2 f)                      { return f.finfo->func; },
    [] (Method m)                    { return m.finfo->func; },
    [] (Method2 m)                   { return m.finfo->func; },
    [] (MethodFamily)                { return Ret{}; },
    [] (MethodFamily2)               { return Ret{}; },
    [] (MethodOrMissing)             { return Ret{}; },
    [] (MethodOrMissing2)            { return Ret{}; },
    [] (MissingFunc)                 { return Ret{}; },
    [] (MissingMethod)               { return Ret{}; },
    [] (const Isect&)                { return Ret{}; },
    [] (const Isect2&)               { return Ret{}; }
  );
}

TriBool Func::exists() const {
  return match<TriBool>(
    val,
    [] (FuncName)                    { return TriBool::Maybe; },
    [] (MethodName)                  { return TriBool::Maybe; },
    [] (Fun)                         { return TriBool::Yes; },
    [] (Fun2)                        { return TriBool::Yes; },
    [] (Method)                      { return TriBool::Yes; },
    [] (Method2)                     { return TriBool::Yes; },
    [] (MethodFamily)                { return TriBool::Maybe; },
    [] (MethodFamily2)               { return TriBool::Maybe; },
    [] (MethodOrMissing)             { return TriBool::Maybe; },
    [] (MethodOrMissing2)            { return TriBool::Maybe; },
    [] (MissingFunc)                 { return TriBool::No; },
    [] (MissingMethod)               { return TriBool::No; },
    [] (const Isect&)                { return TriBool::Maybe; },
    [] (const Isect2&)               { return TriBool::Maybe; }
  );
}

bool Func::isFoldable() const {
  return match<bool>(
    val,
    [] (FuncName)   { return false; },
    [] (MethodName) { return false; },
    [] (Fun f) {
      return f.finfo->func->attrs & AttrIsFoldable;
    },
    [] (Fun2 f) {
      return f.finfo->func->attrs & AttrIsFoldable;
    },
    [] (Method m)  { return m.finfo->func->attrs & AttrIsFoldable; },
    [] (Method2 m) { return m.finfo->func->attrs & AttrIsFoldable; },
    [] (MethodFamily)    { return false; },
    [] (MethodFamily2)   { return false; },
    [] (MethodOrMissing) { return false; },
    [] (MethodOrMissing2){ return false; },
    [] (MissingFunc)     { return false; },
    [] (MissingMethod)   { return false; },
    [] (const Isect&)    { return false; },
    [] (const Isect2&)   { return false; }
  );
}

bool Func::couldHaveReifiedGenerics() const {
  return match<bool>(
    val,
    [] (FuncName s) { return true; },
    [] (MethodName) { return true; },
    [] (Fun f) { return f.finfo->func->isReified; },
    [] (Fun2 f) { return f.finfo->func->isReified; },
    [] (Method m) { return m.finfo->func->isReified; },
    [] (Method2 m) { return m.finfo->func->isReified; },
    [] (MethodFamily fa) {
      return fa.family->infoFor(fa.regularOnly).m_static->m_maybeReified;
    },
    [] (MethodFamily2 fa) {
      return fa.family->infoFor(fa.regularOnly).m_maybeReified;
    },
    [] (MethodOrMissing m) { return m.finfo->func->isReified; },
    [] (MethodOrMissing2 m) { return m.finfo->func->isReified; },
    [] (MissingFunc) { return false; },
    [] (MissingMethod) { return false; },
    [] (const Isect& i) {
      for (auto const ff : i.families) {
        if (!ff->infoFor(i.regularOnly).m_static->m_maybeReified) return false;
      }
      return true;
    },
    [] (const Isect2& i) {
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
    [&] (FuncName) { return mightCareAboutFuncs; },
    [&] (MethodName) {
      return mightCareAboutClsMeth || mightCareAboutInstMeth;
    },
    [&] (Fun f) {
      return dyn_call_error_level(f.finfo->func) > 0;
    },
    [&] (Fun2 f) {
      return dyn_call_error_level(f.finfo->func) > 0;
    },
    [&] (Method m)  { return dyn_call_error_level(m.finfo->func) > 0; },
    [&] (Method2 m) { return dyn_call_error_level(m.finfo->func) > 0; },
    [&] (MethodFamily fa) {
      return
        fa.family->infoFor(fa.regularOnly).m_static->m_maybeCaresAboutDynCalls;
    },
    [&] (MethodFamily2 fa) {
      return
        fa.family->infoFor(fa.regularOnly).m_maybeCaresAboutDynCalls;
    },
    [&] (MethodOrMissing m)  { return dyn_call_error_level(m.finfo->func) > 0; },
    [&] (MethodOrMissing2 m) { return dyn_call_error_level(m.finfo->func) > 0; },
    [&] (MissingFunc m) { return false; },
    [&] (MissingMethod m) { return false; },
    [&] (const Isect& i) {
      for (auto const ff : i.families) {
        if (!ff->infoFor(i.regularOnly).m_static->m_maybeCaresAboutDynCalls) {
          return false;
        }
      }
      return true;
    },
    [&] (const Isect2& i) {
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
    [] (FuncName s) { return true; },
    [] (MethodName) { return true; },
    [] (Fun f) { return f.finfo->func->attrs & AttrBuiltin; },
    [] (Fun2 f) { return f.finfo->func->attrs & AttrBuiltin; },
    [] (Method m) { return m.finfo->func->attrs & AttrBuiltin; },
    [] (Method2 m) { return m.finfo->func->attrs & AttrBuiltin; },
    [] (MethodFamily fa) {
      return fa.family->infoFor(fa.regularOnly).m_static->m_maybeBuiltin;
    },
    [] (MethodFamily2 fa) {
      return fa.family->infoFor(fa.regularOnly).m_maybeBuiltin;
    },
    [] (MethodOrMissing m) { return m.finfo->func->attrs & AttrBuiltin; },
    [] (MethodOrMissing2 m) { return m.finfo->func->attrs & AttrBuiltin; },
    [] (MissingFunc m) { return false; },
    [] (MissingMethod m) { return false; },
    [] (const Isect& i) {
      for (auto const ff : i.families) {
        if (!ff->infoFor(i.regularOnly).m_static->m_maybeBuiltin) return false;
      }
      return true;
    },
    [] (const Isect2& i) {
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
    [] (FuncName) { return 0; },
    [] (MethodName) { return 0; },
    [] (Fun f) { return numNVArgs(*f.finfo->func); },
    [] (Fun2 f) { return numNVArgs(*f.finfo->func); },
    [] (Method m) { return numNVArgs(*m.finfo->func); },
    [] (Method2 m) { return numNVArgs(*m.finfo->func); },
    [] (MethodFamily fa) {
      return
        fa.family->infoFor(fa.regularOnly).m_static->m_minNonVariadicParams;
    },
    [&] (MethodFamily2 fa) {
      return fa.family->infoFor(fa.regularOnly).m_minNonVariadicParams;
    },
    [] (MethodOrMissing m) { return numNVArgs(*m.finfo->func); },
    [] (MethodOrMissing2 m) { return numNVArgs(*m.finfo->func); },
    [] (MissingFunc)   { return 0; },
    [] (MissingMethod) { return 0; },
    [] (const Isect& i) {
      uint32_t nv = 0;
      for (auto const ff : i.families) {
        nv = std::max(
          nv,
          ff->infoFor(i.regularOnly).m_static->m_minNonVariadicParams
        );
      }
      return nv;
    },
    [] (const Isect2& i) {
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
    [] (FuncName) { return std::numeric_limits<uint32_t>::max(); },
    [] (MethodName) { return std::numeric_limits<uint32_t>::max(); },
    [] (Fun f) { return numNVArgs(*f.finfo->func); },
    [] (Fun2 f) { return numNVArgs(*f.finfo->func); },
    [] (Method m) { return numNVArgs(*m.finfo->func); },
    [] (Method2 m) { return numNVArgs(*m.finfo->func); },
    [] (MethodFamily fa) {
      return
        fa.family->infoFor(fa.regularOnly).m_static->m_maxNonVariadicParams;
    },
    [] (MethodFamily2 fa) {
      return fa.family->infoFor(fa.regularOnly).m_maxNonVariadicParams;
    },
    [] (MethodOrMissing m) { return numNVArgs(*m.finfo->func); },
    [] (MethodOrMissing2 m) { return numNVArgs(*m.finfo->func); },
    [] (MissingFunc) { return 0; },
    [] (MissingMethod) { return 0; },
    [] (const Isect& i) {
      auto nv = std::numeric_limits<uint32_t>::max();
      for (auto const ff : i.families) {
        nv = std::min(
          nv,
          ff->infoFor(i.regularOnly).m_static->m_maxNonVariadicParams
        );
      }
      return nv;
    },
    [] (const Isect2& i) {
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
    [] (FuncName) { return nullptr; },
    [] (MethodName) { return nullptr; },
    [] (Fun f) { return &f.finfo->func->requiredCoeffects; },
    [] (Fun2 f) { return &f.finfo->func->requiredCoeffects; },
    [] (Method m) { return &m.finfo->func->requiredCoeffects; },
    [] (Method2 m) { return &m.finfo->func->requiredCoeffects; },
    [] (MethodFamily fa) {
      return fa.family->infoFor(fa.regularOnly)
        .m_static->m_requiredCoeffects.get_pointer();
    },
    [] (MethodFamily2 fa) {
      return
        fa.family->infoFor(fa.regularOnly).m_requiredCoeffects.get_pointer();
    },
    [] (MethodOrMissing m) { return &m.finfo->func->requiredCoeffects; },
    [] (MethodOrMissing2 m) { return &m.finfo->func->requiredCoeffects; },
    [] (MissingFunc) { return nullptr; },
    [] (MissingMethod) { return nullptr; },
    [] (const Isect& i) {
      const RuntimeCoeffects* coeffects = nullptr;
      for (auto const ff : i.families) {
        auto const& info = *ff->infoFor(i.regularOnly).m_static;
        if (!info.m_requiredCoeffects) continue;
        assertx(IMPLIES(coeffects, *coeffects == *info.m_requiredCoeffects));
        if (!coeffects) coeffects = info.m_requiredCoeffects.get_pointer();
      }
      return coeffects;
    },
    [] (const Isect2& i) {
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
    [] (FuncName) { return nullptr; },
    [] (MethodName) { return nullptr; },
    [] (Fun f) { return &f.finfo->func->coeffectRules; },
    [] (Fun2 f) { return &f.finfo->func->coeffectRules; },
    [] (Method m) { return &m.finfo->func->coeffectRules; },
    [] (Method2 m) { return &m.finfo->func->coeffectRules; },
    [] (MethodFamily fa) {
      return fa.family->infoFor(fa.regularOnly)
        .m_static->m_coeffectRules.get_pointer();
    },
    [] (MethodFamily2 fa) {
      return fa.family->infoFor(fa.regularOnly).m_coeffectRules.get_pointer();
    },
    [] (MethodOrMissing m) { return &m.finfo->func->coeffectRules; },
    [] (MethodOrMissing2 m) { return &m.finfo->func->coeffectRules; },
    [] (MissingFunc) { return nullptr; },
    [] (MissingMethod) { return nullptr; },
    [] (const Isect& i) {
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
    [] (const Isect2& i) {
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
    [] (FuncName)   { return TriBool::Maybe; },
    [] (MethodName) { return TriBool::Maybe; },
    [] (Fun f)      { return yesOrNo(func_supports_AER(f.finfo->func)); },
    [] (Fun2 f)     { return yesOrNo(func_supports_AER(f.finfo->func)); },
    [] (Method m)   { return yesOrNo(func_supports_AER(m.finfo->func)); },
    [] (Method2 m)  { return yesOrNo(func_supports_AER(m.finfo->func)); },
    [] (MethodFamily fam) {
      return fam.family->infoFor(fam.regularOnly).m_static->m_supportsAER;
    },
    [] (MethodFamily2 fam) {
      return fam.family->infoFor(fam.regularOnly).m_supportsAER;
    },
    [] (MethodOrMissing m)  {
      return yesOrNo(func_supports_AER(m.finfo->func));
    },
    [] (MethodOrMissing2 m) {
      return yesOrNo(func_supports_AER(m.finfo->func));
    },
    [] (MissingFunc) { return TriBool::No; },
    [] (MissingMethod) { return TriBool::No; },
    [] (const Isect& i) {
      auto aer = TriBool::Maybe;
      for (auto const ff : i.families) {
        auto const& info = *ff->infoFor(i.regularOnly).m_static;
        if (info.m_supportsAER == TriBool::Maybe) continue;
        assertx(IMPLIES(aer != TriBool::Maybe, aer == info.m_supportsAER));
        if (aer == TriBool::Maybe) aer = info.m_supportsAER;
      }
      return aer;
    },
    [] (const Isect2& i) {
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
    [] (FuncName s)   -> Optional<uint32_t> { return std::nullopt; },
    [] (MethodName s) -> Optional<uint32_t> { return std::nullopt; },
    [] (Fun f)     { return func_num_inout(f.finfo->func); },
    [] (Fun2 f)    { return func_num_inout(f.finfo->func); },
    [] (Method m)  { return func_num_inout(m.finfo->func); },
    [] (Method2 m) { return func_num_inout(m.finfo->func); },
    [] (MethodFamily fam) {
      return fam.family->infoFor(fam.regularOnly).m_static->m_numInOut;
    },
    [] (MethodFamily2 fam) {
      return fam.family->infoFor(fam.regularOnly).m_numInOut;
    },
    [] (MethodOrMissing m)  { return func_num_inout(m.finfo->func); },
    [] (MethodOrMissing2 m) { return func_num_inout(m.finfo->func); },
    [] (MissingFunc)        { return 0; },
    [] (MissingMethod)      { return 0; },
    [] (const Isect& i) {
      Optional<uint32_t> numInOut;
      for (auto const ff : i.families) {
        auto const& info = *ff->infoFor(i.regularOnly).m_static;
        if (!info.m_numInOut) continue;
        assertx(IMPLIES(numInOut, *numInOut == *info.m_numInOut));
        if (!numInOut) numInOut = info.m_numInOut;
      }
      return numInOut;
    },
    [] (const Isect2& i) {
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
    [&] (MissingFunc)        { return PrepKind{TriBool::No, TriBool::Yes}; },
    [&] (MissingMethod)      { return PrepKind{TriBool::No, TriBool::Yes}; },
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
    [] (FuncName)     { return TriBool::Maybe; },
    [] (MethodName)   { return TriBool::Maybe; },
    [] (Fun f)        { return yesOrNo(f.finfo->func->isReadonlyReturn); },
    [] (Fun2 f)       { return yesOrNo(f.finfo->func->isReadonlyReturn); },
    [] (Method m)     { return yesOrNo(m.finfo->func->isReadonlyReturn); },
    [] (Method2 m)    { return yesOrNo(m.finfo->func->isReadonlyReturn); },
    [] (MethodFamily fam) {
      return fam.family->infoFor(fam.regularOnly).m_static->m_isReadonlyReturn;
    },
    [] (MethodFamily2 fam) {
      return fam.family->infoFor(fam.regularOnly).m_isReadonlyReturn;
    },
    [] (MethodOrMissing m)  { return yesOrNo(m.finfo->func->isReadonlyReturn); },
    [] (MethodOrMissing2 m) { return yesOrNo(m.finfo->func->isReadonlyReturn); },
    [] (MissingFunc)        { return TriBool::No; },
    [] (MissingMethod)      { return TriBool::No; },
    [] (const Isect& i) {
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
    [] (const Isect2& i) {
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
    [] (FuncName s)   { return TriBool::Maybe; },
    [] (MethodName s) { return TriBool::Maybe; },
    [] (Fun f)        { return yesOrNo(f.finfo->func->isReadonlyThis); },
    [] (Fun2 f)       { return yesOrNo(f.finfo->func->isReadonlyThis); },
    [] (Method m)     { return yesOrNo(m.finfo->func->isReadonlyThis); },
    [] (Method2 m)    { return yesOrNo(m.finfo->func->isReadonlyThis); },
    [] (MethodFamily fam) {
      return fam.family->infoFor(fam.regularOnly).m_static->m_isReadonlyThis;
    },
    [] (MethodFamily2 fam) {
      return fam.family->infoFor(fam.regularOnly).m_isReadonlyThis;
    },
    [] (MethodOrMissing m)  { return yesOrNo(m.finfo->func->isReadonlyThis); },
    [] (MethodOrMissing2 m) { return yesOrNo(m.finfo->func->isReadonlyThis); },
    [] (MissingFunc)        { return TriBool::No; },
    [] (MissingMethod)      { return TriBool::No; },
    [] (const Isect& i) {
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
    [] (const Isect2& i) {
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
  auto ret = f.name();
  match<void>(
    f.val,
    [&] (Func::FuncName)          {},
    [&] (Func::MethodName)        {},
    [&] (Func::Fun)               { ret += "*"; },
    [&] (Func::Fun2)              { ret += "*"; },
    [&] (Func::Method)            { ret += "*"; },
    [&] (Func::Method2)           { ret += "*"; },
    [&] (Func::MethodFamily)      { ret += "+"; },
    [&] (Func::MethodFamily2)     { ret += "+"; },
    [&] (Func::MethodOrMissing)   { ret += "-"; },
    [&] (Func::MethodOrMissing2)  { ret += "-"; },
    [&] (Func::MissingFunc)       { ret += "!"; },
    [&] (Func::MissingMethod)     { ret += "!"; },
    [&] (const Func::Isect&)      { ret += "&"; },
    [&] (const Func::Isect2&)     { ret += "&"; }
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
  std::unique_ptr<TicketExecutor> executor;
  std::unique_ptr<Client> client;
  DisposeCallback disposeClient;

  // Global configeration, stored in extern-worker.
  std::unique_ptr<CoroAsyncValue<Ref<Config>>> configRef;

  // Maps unit/class/func name to the extern-worker ref representing
  // php::Program data for that. Any associated bytecode is stored
  // separately.
  SStringToOneT<UniquePtrRef<php::Unit>>   unitRefs;
  TSStringToOneT<UniquePtrRef<php::Class>> classRefs;
  FSStringToOneT<UniquePtrRef<php::Func>>  funcRefs;

  // Maps class name to the extern-worker ref representing the class's
  // associated ClassInfo2. Only has entries for instantiable classes.
  TSStringToOneT<UniquePtrRef<ClassInfo2>> classInfoRefs;

  // Maps func name (global functions, not methods) to the
  // extern-worked ref representing the func's associated
  // FuncInfo2. The FuncInfo2 for methods are stored in their parent
  // ClassInfo2.
  FSStringToOneT<UniquePtrRef<FuncInfo2>> funcInfoRefs;

  // Maps class/func names to the extern-worker ref representing the
  // bytecode for that class or (global) function. The bytecode of all
  // of a class' methods are stored together.
  TSStringToOneT<UniquePtrRef<php::ClassBytecode>> classBytecodeRefs;
  FSStringToOneT<UniquePtrRef<php::FuncBytecode>> funcBytecodeRefs;

  // Uninstantiable classes do not have ClassInfo2s, but their methods
  // still have FuncInfo2s. Since we don't have a ClassInfo2 to store
  // them on, we do it separately.
  TSStringToOneT<UniquePtrRef<MethodsWithoutCInfo>> uninstantiableClsMethRefs;

  // Func family entries representing all methods with a particular
  // name.
  SStringToOneT<FuncFamilyEntry> nameOnlyMethodFamilies;

  // Maps func-family ids to the func family group which contains the
  // func family with that id.
  hphp_fast_map<FuncFamily2::Id, Ref<FuncFamilyGroup>> funcFamilyRefs;

  // Maps classes and functions to the names of closures defined
  // within.
  TSStringToOneT<TSStringSet> classToClosures;
  FSStringToOneT<TSStringSet> funcToClosures;

  // Maps entities to the unit they were declared in.
  TSStringToOneT<SString> classToUnit;
  FSStringToOneT<SString> funcToUnit;
  TSStringToOneT<SString> typeAliasToUnit;
  // If bool is true, then the constant is "dynamic" and has an
  // associated 86cinit function.
  SStringToOneT<std::pair<SString, bool>> constantToUnit;

  // All the classes that have a 86*init function.
  TSStringSet classesWith86Inits;
  // All the 86cinit functions for "dynamic" top-level constants.
  FSStringSet constantInitFuncs;

  std::unique_ptr<php::Program> program;

  TSStringToOneT<php::Class*>      classes;
  FSStringToOneT<php::Func*>       funcs;
  TSStringToOneT<php::TypeAlias*>  typeAliases;
  TSStringToOneT<php::Class*>      enums;
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
  TSStringToOneT<ClassInfo*> classInfo;

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
  TSStringToOneT<Slot> ifaceSlotMap;

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
   * to know what we determined the last time we were allowed to do
   * that so we can return it again.
   */
  ContextRetTyMap contextualReturnTypes{};
};

//////////////////////////////////////////////////////////////////////

namespace { struct DepTracker; };

struct AnalysisIndex::IndexData {
  IndexData(AnalysisIndex& index,
            AnalysisWorklist& worklist)
    : index{index}
    , worklist{worklist}
    , deps{std::make_unique<DepTracker>(*this)} {}

  IndexData(const IndexData&) = delete;
  IndexData& operator=(const IndexData&) = delete;

  AnalysisIndex& index;
  AnalysisWorklist& worklist;
  std::unique_ptr<DepTracker> deps;

  std::vector<SString> classNames;
  std::vector<SString> funcNames;
  std::vector<SString> unitNames;

  TSStringToOneT<std::unique_ptr<php::Class>> classes;
  TSStringToOneT<std::unique_ptr<ClassInfo2>> cinfos;
  TSStringToOneT<std::unique_ptr<MethodsWithoutCInfo>> minfos;

  FSStringToOneT<std::unique_ptr<php::Func>> funcs;
  FSStringToOneT<std::unique_ptr<FuncInfo2>> finfos;

  SStringToOneT<std::unique_ptr<php::Unit>> units;

  SStringToOneT<php::Constant*> constants;
  TSStringToOneT<php::TypeAlias*> typeAliases;

  std::vector<FuncInfo2*> finfosByIdx;

  // Anything on these lists is known to definitely not exist.
  TSStringSet badClasses;
  FSStringSet badFuncs;
  SStringSet badConstants;
  TSStringSet badTypeAliases;

  SStringSet dynamicConstants;

  // AnalysisIndex maintains a stack of the contexts being analyzed
  // (we can have multiple because of inline interp).
  std::vector<Context> contexts;

  size_t foldableInterpNestingLevel{0};
  size_t contextualInterpNestingLevel{0};

  // Once the index is frozen, no further updates to it are allowed
  // (will assert). We only gather dependencies when the index is
  // frozen.
  bool frozen{false};
};

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

// Obtain the current (most recently pushed) context. This corresponds
// to the context currently being analyzed.
const Context& current_context(const AnalysisIndex::IndexData& index) {
  always_assert_flog(
    !index.contexts.empty(),
    "Accessing current context without any contexts active"
  );
  return index.contexts.back();
}

// Obtain the context to use for the purposes of dependency
// tracking. This is the first context pushed. This differs from
// current_context() because of inline interp. If we're inline
// interp-ing a function, we still want to attribute the dependencies
// to the context which started the inline interp.
const Context& context_for_deps(const AnalysisIndex::IndexData& index) {
  always_assert_flog(
    !index.contexts.empty(),
    "Accessing dependency context without any contexts active"
  );
  return index.contexts.front();
}

//////////////////////////////////////////////////////////////////////

FuncOrCls fc_from_context(const Context& ctx) {
  if (ctx.cls) return ctx.cls;
  assertx(ctx.func);
  return ctx.func;
}

//////////////////////////////////////////////////////////////////////

// Record the dependencies of all classes and functions being
// processed with an AnalysisIndex. These dependencies will ultimately
// be reported back to the master AnalysisScheduler, but will also
// drive the worklist on the local analysis job.
struct DepTracker {
  explicit DepTracker(const AnalysisIndex::IndexData& index)
    : index{index} {}

  using Type = AnalysisDeps::Type;
  using Func = AnalysisDeps::Func;
  using Class = AnalysisDeps::Class;
  using Constant = AnalysisDeps::Constant;
  using TypeAlias = AnalysisDeps::TypeAlias;

  // Register dependencies on various entities to the current
  // dependency context.

  void add(Class c) {
    if (!index.frozen) return;
    auto const fc = context();
    auto& d = deps[fc];
    if (!(fc.right() && fc.right()->name->tsame(c.name)) && d.add(c)) {
      FTRACE(2, "{} now depends on class {}\n", HHBBC::show(fc), c.name);
    }
    // Class either exists or not and won't change within the job, so
    // nothing to record for worklist.
  }

  void add(const php::Func& f, Type t = Type::None) {
    auto const fc = context();
    if (index.frozen) {
      if (auto const added = deps[fc].add(f, t)) {
        FTRACE(
          2, "{} now depends on {}{} {}\n",
          HHBBC::show(fc), displayAdded(added),
          f.cls ? "method" : "func",
          func_fullname(f)
        );
      }
    } else {
      // Record dependency for worklist if anything can change within
      // the job.
      t &= AnalysisDeps::kValidForChanges;
      if (t == Type::None) return;
      funcs[&f][fc] |= t;
    }
  }

  void add(MethRef m, Type t = Type::None) {
    auto const fc = context();
    if (index.frozen) {
      if (auto const added = deps[fc].add(m, t)) {
        FTRACE(2, "{} now depends on {}method {}\n",
               HHBBC::show(fc), displayAdded(added), display(m));
      }
    } else {
      // Record dependency for worklist if anything can change within
      // the job.
      t &= AnalysisDeps::kValidForChanges;
      if (t == Type::None) return;
      if (auto const p = from(m)) funcs[p][fc] |= t;
    }
  }

  void add(Func f, Type t = Type::None) {
    auto const fc = context();
    if (index.frozen) {
      if (auto const added = deps[fc].add(f, t)) {
        FTRACE(2, "{} now depends on {}func {}\n",
               HHBBC::show(fc), displayAdded(added), f.name);
      }
    } else {
      // Record dependency for worklist if anything can change within
      // the job.
      t &= AnalysisDeps::kValidForChanges;
      if (t == Type::None) return;
      if (auto const p = folly::get_ptr(index.funcs, f.name)) {
        funcs[p->get()][fc] |= t;
      }
    }
  }

  void add(ConstIndex cns) {
    auto const fc = context();
    if (index.frozen) {
      if (deps[fc].add(cns)) {
        FTRACE(2, "{} now depends on class constant {}\n",
               HHBBC::show(fc), display(cns));
      }
    } else if (auto const p = from(cns)) {
      clsConstants[p].emplace(fc);
    }
  }

  void add(Constant cns) {
    auto const fc = context();
    if (index.frozen) {
      if (deps[fc].add(cns)) {
        FTRACE(2, "{} now depends on constant {}\n", HHBBC::show(fc), cns.name);
      }
    } else if (auto const p = folly::get_default(index.constants, cns.name)) {
      constants[p].emplace(fc);
    }
  }

  void add(TypeAlias typeAlias) {
    if (!index.frozen) return;
    auto const fc = context();
    if (!deps[fc].add(typeAlias)) return;
    FTRACE(2, "{} now depends on type-alias {}\n",
           HHBBC::show(fc), typeAlias.name);
  }

  // Mark that the given entity has changed in some way. This not only
  // results in the change being reported back to the
  // AnalysisScheduler, but will reschedule any work locally which has
  // a dependency.

  void update(const php::Func& f, Type t) {
    if (t == Type::None) return;
    assertx(AnalysisDeps::isValidForChanges(t));
    assertx(!index.frozen);
    FTRACE(
      2, "{} {} {} changed, scheduling\n",
      f.cls ? "method" : "func",
      func_fullname(f),
      show(t)
    );
    changes.changed(f, t);
    schedule(folly::get_ptr(funcs, &f), t);
  }

  void update(const php::Const& cns, ConstIndex idx) {
    assertx(!index.frozen);
    FTRACE(2, "constant {}::{} changed, scheduling\n", idx.cls, cns.name);
    changes.changed(idx);
    schedule(folly::get_ptr(clsConstants, &cns));
  }

  void update(const php::Constant& cns) {
    assertx(!index.frozen);
    FTRACE(2, "constant {} changed, scheduling\n", cns.name);
    changes.changed(cns);
    schedule(folly::get_ptr(constants, &cns));
  }

  AnalysisDeps take(FuncOrCls fc) {
    auto it = deps.find(fc);
    if (it == end(deps)) return AnalysisDeps{};
    return std::move(it->second);
  }

  AnalysisChangeSet& getChanges() { return changes; }

private:

  // Return appropriate entity to attribute the dependency to. If
  // we're analyzing a function within a class, use the class. If it's
  // a top-level function, use that.
  FuncOrCls context() const {
    auto const& ctx = context_for_deps(index);
    if (ctx.cls) return ctx.cls;
    assertx(ctx.func);
    return ctx.func;
  }

  const php::Func* from(MethRef m) const {
    if (auto const cls = folly::get_ptr(index.classes, m.cls)) {
      assertx(m.idx < cls->get()->methods.size());
      return cls->get()->methods[m.idx].get();
    }
    return nullptr;
  }

  const php::Const* from(ConstIndex cns) const {
    if (auto const cls = folly::get_ptr(index.classes, cns.cls)) {
      assertx(cns.idx < cls->get()->constants.size());
      return &cls->get()->constants[cns.idx];
    }
    return nullptr;
  }

  std::string display(MethRef m) const {
    if (auto const p = from(m)) return func_fullname(*p);
    return show(m);
  }

  std::string display(ConstIndex cns) const {
    if (auto const p = from(cns)) {
      return folly::sformat("{}::{}", p->cls, p->name);
    }
    return show(cns);
  }

  static std::string displayAdded(Type t) {
    auto out = show(t - Type::Meta);
    if (!out.empty()) folly::format(&out, " of ");
    return out;
  }

  using FuncOrClsSet =
    hphp_fast_set<FuncOrCls, FuncOrClsHasher>;
  using FuncOrClsToType =
    hphp_fast_map<FuncOrCls, Type, FuncOrClsHasher>;

  void schedule(const FuncOrClsSet* fcs) {
   if (!fcs || fcs->empty()) return;
    TinyVector<FuncOrCls, 4> v;
    v.insert(begin(*fcs), end(*fcs));
    addToWorklist(v);
  }

  void schedule(const FuncOrClsToType* fcs, Type t) {
    assertx(!(t & Type::Meta));
    if (!fcs || fcs->empty()) return;
    TinyVector<FuncOrCls, 4> v;
    for (auto const [fc, t2] : *fcs) {
      if (t & t2) v.emplace_back(fc);
    }
    addToWorklist(v);
  }

  void addToWorklist(TinyVector<FuncOrCls, 4>& fcs) {
    if (fcs.empty()) return;
    std::sort(
      fcs.begin(), fcs.end(),
      [] (FuncOrCls fc1, FuncOrCls fc2) {
        if (auto const f1 = fc1.left()) {
          auto const f2 = fc2.left();
          return !f2 || string_data_lt_func{}(f1->name, f2->name);
        }
        auto const c1 = fc1.right();
        auto const c2 = fc2.right();
        return c2 && string_data_lt_type{}(c1->name, c2->name);
      }
    );
    Trace::Indent _;
    for (auto const fc : fcs) index.worklist.schedule(fc);
  }

  const AnalysisIndex::IndexData& index;
  AnalysisChangeSet changes;
  hphp_fast_map<FuncOrCls, AnalysisDeps, FuncOrClsHasher> deps;

  hphp_fast_map<const php::Func*, FuncOrClsToType> funcs;
  hphp_fast_map<const php::Const*, FuncOrClsSet> clsConstants;
  hphp_fast_map<const php::Constant*, FuncOrClsSet> constants;
};

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

FuncInfo2& func_info(AnalysisIndex::IndexData& data, const php::Func& f) {
  assertx(f.idx < data.finfosByIdx.size());
  auto const fi = data.finfosByIdx[f.idx];
  assertx(fi->func == &f);
  return *fi;
}

//////////////////////////////////////////////////////////////////////

// Obtain the php::Func* represented by a MethRef.
const php::Func* func_from_meth_ref(const IndexData& index,
                                    const MethRef& meth) {
  auto const cls = index.classes.at(meth.cls);
  assertx(meth.idx < cls->methods.size());
  return cls->methods[meth.idx].get();
}

const php::Func* func_from_meth_ref(const AnalysisIndex::IndexData& index,
                                    const MethRef& meth) {
  index.deps->add(AnalysisDeps::Class { meth.cls });
  auto const cls = folly::get_ptr(index.classes, meth.cls);
  if (!cls) {
    always_assert_flog(
      !index.badClasses.count(meth.cls),
      "MethRef references non-existent class {}\n",
      meth.cls
    );
    return nullptr;
  }
  assertx(meth.idx < cls->get()->methods.size());
  return cls->get()->methods[meth.idx].get();
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

// Defined here so that AnalysisIndex::IndexData is a complete type.

void ClassGraph::storeAuxs(AnalysisIndex::IndexData& i) const {
  // Get the current context and store this ClassGraph on it's aux
  // list.
  auto const fc = fc_from_context(context_for_deps(i));
  if (auto const c = fc.right()) {
    if (!c->cinfo || c->cinfo == cinfo2()) return;
    if (!c->cinfo->auxClassGraphs.emplace(*this).second) return;
    FTRACE(2, "{} now stores {} as an auxiliary ClassGraph\n", c->name, name());
  } else if (auto const f = fc.left()) {
    auto& fi = func_info(i, *f);
    if (!fi.auxClassGraphs) {
      using T = decltype(fi.auxClassGraphs)::element_type;
      fi.auxClassGraphs = std::make_unique<T>();
    }
    if (!fi.auxClassGraphs->emplace(*this).second) return;
    FTRACE(2, "{} now stores {} as an auxiliary ClassGraph\n", f->name, name());
  }
}

// Ensure ClassGraph is not missing
void ClassGraph::ensure() const {
  assertx(this_);
  auto const i = table().index;
  if (!i || !i->frozen) return;
  if (this_->isMissing()) {
    i->deps->add(AnalysisDeps::Class { name() });
  }
  storeAuxs(*i);
}

// Ensure ClassGraph is not missing and has complete child
// information.
void ClassGraph::ensureWithChildren() const {
  assertx(this_);
  auto const i = table().index;
  if (!i || !i->frozen) return;
  if (this_->isMissing() ||
      (!this_->hasCompleteChildren() && !this_->isConservative())) {
    i->deps->add(AnalysisDeps::Class { name() });
  }
  storeAuxs(*i);
}

// Ensure ClassGraph is not missing and has an associated ClassInfo2
// (strongest condition).
void ClassGraph::ensureCInfo() const {
  auto const i = table().index;
  if (!i || !i->frozen) return;
  i->deps->add(AnalysisDeps::Class { name() });
}

//////////////////////////////////////////////////////////////////////

namespace {

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
  SString name{nullptr};
  // The number of classes which implements this interface (used to
  // prioritize lower slots for more heavily used interfaces).
  size_t usage{0};
  TSStringSet conflicts;
  template <typename SerDe> void serde(SerDe& sd) {
    sd(name)(usage)(conflicts, string_data_lt_type{});
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
      return string_data_lt_type{}(a.name, b.name);
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
    ClassGraph::init();
  }
  static void fini() { ClassGraph::destroy(); }

  static bool run(std::unique_ptr<ClassInfo2> cinfo,
                  std::unique_ptr<php::Class> cls) {
    SCOPE_ASSERT_DETAIL("class") { return cls->name->toCppString(); };

    // ClassGraph stored in a ClassInfo should not be missing, always
    // have the ClassInfo stored, and have complete children
    // information.
    always_assert(cinfo->classGraph);
    always_assert(!cinfo->classGraph.isMissing());
    always_assert(cinfo->classGraph.name()->tsame(cinfo->name));
    always_assert(cinfo->classGraph.cinfo2() == cinfo.get());
    if (is_closure_base(cinfo->name)) {
      // The closure base class is special. We don't store it's
      // children information because it's too large.
      always_assert(!cinfo->classGraph.hasCompleteChildren());
      always_assert(cinfo->classGraph.isConservative());
    } else {
      always_assert(cinfo->classGraph.hasCompleteChildren() ||
                    cinfo->classGraph.isConservative());
    }

    // This class and withoutNonRegular should be equivalent when
    // ignoring non-regular classes. The withoutNonRegular class
    // should be a fixed-point.
    if (auto const without = cinfo->classGraph.withoutNonRegular()) {
      always_assert(without.hasCompleteChildren() ||
                    without.isConservative());
      always_assert(without.subSubtypeOf(cinfo->classGraph, false, false));
      always_assert(cinfo->classGraph.subSubtypeOf(without, false, false));
      always_assert(without.withoutNonRegular() == without);
      always_assert(cinfo->classGraph.mightBeRegular() ||
                    cinfo->classGraph.mightHaveRegularSubclass());
      always_assert(IMPLIES(cinfo->classGraph.mightBeRegular(),
                            without == cinfo->classGraph));
    } else if (!is_used_trait(*cls)) {
      always_assert(!cinfo->classGraph.mightBeRegular());
      always_assert(!cinfo->classGraph.mightHaveRegularSubclass());
    }

    // AttrNoOverride is a superset of AttrNoOverrideRegular
    always_assert(
      IMPLIES(!(cls->attrs & AttrNoOverrideRegular),
              !(cls->attrs & AttrNoOverride))
    );

    // Override attrs and what we know about the subclasses should be in
    // agreement.
    if (cls->attrs & AttrNoOverride) {
      always_assert(!cinfo->classGraph.mightHaveRegularSubclass());
      always_assert(!cinfo->classGraph.mightHaveNonRegularSubclass());
    } else if (cls->attrs & AttrNoOverrideRegular) {
      always_assert(!cinfo->classGraph.mightHaveRegularSubclass());
      always_assert(cinfo->classGraph.mightHaveNonRegularSubclass());
    }

    // Make sure the information stored on the ClassInfo matches that
    // which the ClassGraph reports.
    if (cls->attrs & AttrNoMock) {
      always_assert(!cinfo->isMocked);
      always_assert(!cinfo->isSubMocked);
    } else {
      always_assert(cinfo->isSubMocked);
    }

    always_assert(
      bool(cls->attrs & AttrNoExpandTrait) ==
      cinfo->classGraph.usedTraits().empty()
    );

    for (auto const& [name, mte] : cinfo->methods) {
      // Interface method tables should only contain its own methods.
      if (cls->attrs & AttrInterface) {
        always_assert(mte.meth().cls->tsame(cinfo->name));
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

      if (cinfo->name->tsame(s_Closure.get()) || is_closure_name(cinfo->name)) {
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

    // If the class is marked as having not having bad initial prop
    // values, all of it's properties should have
    // AttrInitialSatisfiesTC set. Likewise, if it is, at least one
    // property should not have it set.
    if (!cinfo->hasBadInitialPropValues) {
      auto const all = std::all_of(
        begin(cls->properties),
        end(cls->properties),
        [] (const php::Prop& p) {
          return p.attrs & AttrInitialSatisfiesTC;
        }
      );
      always_assert(all);
    } else {
      auto const someBad = std::any_of(
        begin(cls->properties),
        end(cls->properties),
        [] (const php::Prop& p) {
          return !(p.attrs & AttrInitialSatisfiesTC);
        }
      );
      always_assert(someBad);
    }

    if (is_closure_name(cinfo->name)) {
      assertx(cinfo->classGraph.hasCompleteChildren());
      // Closures have no children.
      auto const subclasses = cinfo->classGraph.children();
      always_assert(subclasses.size() == 1);
      always_assert(subclasses[0].name()->tsame(cinfo->name));
    } else if (cinfo->classGraph.hasCompleteChildren()) {
      // Otherwise the children list is non-empty, contains this
      // class, and contains only unique elements.
      auto const subclasses = cinfo->classGraph.children();
      always_assert(
        std::find_if(
          begin(subclasses),
          end(subclasses),
          [&] (ClassGraph g) { return g.name()->tsame(cinfo->name); }
        ) != end(subclasses)
      );
      auto cpy = subclasses;
      std::sort(begin(cpy), end(cpy));
      cpy.erase(std::unique(begin(cpy), end(cpy)), end(cpy));
      always_assert(cpy.size() == subclasses.size());
    }

    // The base list is non-empty, and the last element is this class.
    auto const bases = cinfo->classGraph.bases();
    always_assert(!bases.empty());
    always_assert(cinfo->classGraph == bases.back());
    if (is_closure_base(cinfo->name)) {
      always_assert(bases.size() == 1);
    } else if (is_closure_name(cinfo->name)) {
      always_assert(bases.size() == 2);
      always_assert(bases[0].name()->tsame(s_Closure.get()));
    }

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
      TSStringSet classes;
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
    co_await coro::co_reschedule_on_current_executor;

    if (work.empty()) co_return;

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
    auto config = co_await index.configRef->getCopy();
    auto outputs = co_await index.client->exec(
      s_checkCInfoInvariantsJob,
      std::move(config),
      std::move(inputs),
      std::move(metadata)
    );
    assertx(outputs.size() == work.size());

    co_return;
  };

  auto const runFF = [&] (std::vector<SString> work) -> coro::Task<void> {
    co_await coro::co_reschedule_on_current_executor;

    if (work.empty()) co_return;

    auto inputs = from(work)
      | map([&] (SString name) {
          return std::make_tuple(nameToFuncFamilyGroup.at(name));
        })
      | as<std::vector>();

    Client::ExecMetadata metadata{
      .job_key = folly::sformat("check func-family invariants {}", work[0])
    };
    auto config = co_await index.configRef->getCopy();
    auto outputs = co_await index.client->exec(
      s_checkFuncFamilyInvariantsJob,
      std::move(config),
      std::move(inputs),
      std::move(metadata)
    );
    assertx(outputs.size() == work.size());

    co_return;
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
  coro::blockingWait(coro::collectAllRange(std::move(tasks)));
}

void check_local_invariants(const IndexData& index, const ClassInfo* cinfo) {
  SCOPE_ASSERT_DETAIL("class") { return cinfo->cls->name->toCppString(); };

  // AttrNoOverride is a superset of AttrNoOverrideRegular
  always_assert(
    IMPLIES(!(cinfo->cls->attrs & AttrNoOverrideRegular),
            !(cinfo->cls->attrs & AttrNoOverride))
  );

  always_assert(cinfo->classGraph);
  always_assert(!cinfo->classGraph.isMissing());
  always_assert(cinfo->classGraph.name()->tsame(cinfo->cls->name));
  always_assert(cinfo->classGraph.cinfo() == cinfo);
  if (is_closure_base(cinfo->cls->name)) {
    // The closure base class is special. We don't store it's children
    // information because it's too large.
    always_assert(!cinfo->classGraph.hasCompleteChildren());
    always_assert(cinfo->classGraph.isConservative());
  } else {
    always_assert(cinfo->classGraph.hasCompleteChildren() ||
                  cinfo->classGraph.isConservative());
  }

  // This class and withoutNonRegular should be equivalent when
  // ignoring non-regular classes. The withoutNonRegular class should
  // be a fixed-point.
  if (auto const without = cinfo->classGraph.withoutNonRegular()) {
    always_assert(without.hasCompleteChildren() ||
                  without.isConservative());
    always_assert(without.subSubtypeOf(cinfo->classGraph, false, false));
    always_assert(cinfo->classGraph.subSubtypeOf(without, false, false));
    always_assert(without.withoutNonRegular() == without);
    always_assert(cinfo->classGraph.mightBeRegular() ||
                  cinfo->classGraph.mightHaveRegularSubclass());
    always_assert(IMPLIES(cinfo->classGraph.mightBeRegular(),
                          without == cinfo->classGraph));
  } else if (!is_used_trait(*cinfo->cls)) {
    always_assert(!cinfo->classGraph.mightBeRegular());
    always_assert(!cinfo->classGraph.mightHaveRegularSubclass());
  }

  // Override attrs and what we know about the subclasses should be in
  // agreement.
  if (cinfo->cls->attrs & AttrNoOverride) {
    always_assert(!cinfo->classGraph.mightHaveRegularSubclass());
    always_assert(!cinfo->classGraph.mightHaveNonRegularSubclass());
  } else if (cinfo->cls->attrs & AttrNoOverrideRegular) {
    always_assert(!cinfo->classGraph.mightHaveRegularSubclass());
    always_assert(cinfo->classGraph.mightHaveNonRegularSubclass());
  }

  if (cinfo->cls->attrs & AttrNoMock) {
    always_assert(!cinfo->isMocked);
    always_assert(!cinfo->isSubMocked);
  }

  // An AttrNoExpand class shouldn't have any used traits.
  always_assert(
    bool(cinfo->cls->attrs & AttrNoExpandTrait) ==
    cinfo->usedTraits.empty()
  );

  for (size_t idx = 0; idx < cinfo->cls->methods.size(); ++idx) {
    // Each method in a class has an entry in its ClassInfo method
    // table.
    auto const& m = cinfo->cls->methods[idx];
    auto const it = cinfo->methods.find(m->name);
    always_assert(it != cinfo->methods.end());
    always_assert(it->second.meth().cls->tsame(cinfo->cls->name));
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
      always_assert(mte.meth().cls->tsame(cinfo->cls->name));
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

  // If the class is marked as having not having bad initial prop
  // values, all of it's properties should have AttrInitialSatisfiesTC
  // set. Likewise, if it is, at least one property should not have it
  // set.
  if (!cinfo->hasBadInitialPropValues) {
    auto const all = std::all_of(
      begin(cinfo->cls->properties),
      end(cinfo->cls->properties),
      [] (const php::Prop& p) {
        return p.attrs & AttrInitialSatisfiesTC;
      }
    );
    always_assert(all);
  } else {
    auto const someBad = std::any_of(
      begin(cinfo->cls->properties),
      end(cinfo->cls->properties),
      [] (const php::Prop& p) {
        return !(p.attrs & AttrInitialSatisfiesTC);
      }
    );
    always_assert(someBad);
  }

  if (is_closure_name(cinfo->cls->name)) {
    assertx(cinfo->classGraph.hasCompleteChildren());
    // Closures have no children.
    auto const subclasses = cinfo->classGraph.children();
    always_assert(subclasses.size() == 1);
    always_assert(subclasses[0].name()->tsame(cinfo->cls->name));
  } else if (cinfo->classGraph.hasCompleteChildren()) {
    // Otherwise the children list is non-empty, contains this
    // class, and contains only unique elements.
    auto const subclasses = cinfo->classGraph.children();
    always_assert(
      std::find_if(
        begin(subclasses),
        end(subclasses),
        [&] (ClassGraph g) { return g.name()->tsame(cinfo->cls->name); }
      ) != end(subclasses)
    );
    auto cpy = subclasses;
    std::sort(begin(cpy), end(cpy));
    cpy.erase(std::unique(begin(cpy), end(cpy)), end(cpy));
    always_assert(cpy.size() == subclasses.size());
  }

  // The base list is non-empty, and the last element is this class.
  auto const bases = cinfo->classGraph.bases();
  always_assert(!bases.empty());
  always_assert(cinfo->classGraph == bases.back());
  if (is_closure_base(cinfo->cls->name)) {
    always_assert(bases.size() == 1);
  } else if (is_closure_name(cinfo->cls->name)) {
    always_assert(bases.size() == 2);
    always_assert(bases[0].name()->tsame(s_Closure.get()));
  }
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
          return string_data_lt_type{}(last.ptr()->cls->name, pf.ptr()->cls->name);
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

Type adjust_closure_context(const IIndex& index, const CallContext& ctx) {
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

  auto const adjustedCtx = adjust_closure_context(
    IndexAdaptor { *data.m_index },
    callCtx
  );
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
        lookup_constraint(IndexAdaptor { *data.m_index }, ctx, constraint).upper
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
      IndexAdaptor { *data.m_index },
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

Index::ReturnType context_sensitive_return_type(AnalysisIndex::IndexData& data,
                                                const CallContext& callCtx,
                                                Index::ReturnType returnType) {
  constexpr size_t maxNestingLevel = 2;

  using R = Index::ReturnType;

  auto const& func = *callCtx.callee;
  auto const& finfo = func_info(data, func);

  auto const& caller = *context_for_deps(data).func;

  auto const adjustedCtx = adjust_closure_context(
    AnalysisIndexAdaptor { data.index },
    callCtx
  );
  returnType.t = return_with_context(std::move(returnType.t), adjustedCtx);

  auto const checkParam = [&] (size_t i) {
    auto const& constraint = func.params[i].typeConstraint;
    if (constraint.hasConstraint() &&
        !constraint.isTypeVar() &&
        !constraint.isTypeConstant()) {
      return callCtx.args[i].strictlyMoreRefined(
        lookup_constraint(
          AnalysisIndexAdaptor { data.index },
          Context { func.unit, &func, func.cls },
          constraint
        ).upper
      );
    }
    return callCtx.args[i].strictSubtypeOf(TInitCell);
  };

  // TODO(#3788877): more heuristics here would be useful.
  auto const tryContextSensitive = [&] {
    if (func.noContextSensitiveAnalysis ||
        func.params.empty() ||
        data.contextualInterpNestingLevel + 1 >= maxNestingLevel ||
        returnType.t.is(BBottom)) {
      return false;
    }

    data.deps->add(func, AnalysisDeps::RetParam);
    if (finfo.retParam != NoLocalId &&
        callCtx.args.size() > finfo.retParam &&
        checkParam(finfo.retParam)) {
      return true;
    }

    if (!options.ContextSensitiveInterp) return false;

    auto const numParams = func.params.size();
    if (callCtx.args.size() < numParams) return true;
    for (size_t i = 0; i < numParams; ++i) {
      if (checkParam(i)) return true;
    }
    return false;
  }();

  if (!tryContextSensitive) return returnType;

  data.deps->add(func, AnalysisDeps::Bytecode);
  if (!func.rawBlocks) {
    ITRACE_MOD(
      Trace::hhbbc, 4,
      "Skipping inline interp of {} because bytecode not present\n",
      func_fullname(func)
    );
    return R{ TInitCell, false };
  }

  auto contextType = [&] {
    ++data.contextualInterpNestingLevel;
    SCOPE_EXIT { --data.contextualInterpNestingLevel; };

    auto const wf = php::WideFunc::cns(&func);
    auto fa = analyze_func_inline(
      AnalysisIndexAdaptor { data.index },
      AnalysisContext {
        func.unit,
        wf,
        func.cls,
        &context_for_deps(data)
      },
      adjustedCtx,
      callCtx.args
    );
    return R{
      return_with_context(std::move(fa.inferredReturn), std::move(adjustedCtx)),
      fa.effectFree
    };
  }();

  if (!returnType.t.subtypeOf(BUnc)) {
    // If the context insensitive return type could be non-static, staticness
    // could be a result of temporary context sensitive bytecode optimizations.
    contextType.t = loosen_staticness(std::move(contextType.t));
  }

  ITRACE_MOD(
    Trace::hhbbc, 4,
    "Context sensitive type: {}, context insensitive type: {}\n",
    show(contextType.t), show(returnType.t)
  );

  auto const error_context = [&] {
    using namespace folly::gen;
    return folly::sformat(
      "{} calling {} (context: {}, args: {})",
      func_fullname(caller),
      func_fullname(func),
      show(callCtx.context),
      from(callCtx.args)
        | map([] (const Type& t) { return show(t); })
        | unsplit<std::string>(",")
    );
  };

  always_assert_flog(
    contextType.t.subtypeOf(returnType.t),
    "Context sensitive return type for {} is {} ",
    "which is not at least as refined as context insensitive "
    "return type {}\n",
    error_context(),
    show(contextType.t),
    show(returnType.t)
  );
  always_assert_flog(
    contextType.effectFree || !returnType.effectFree,
    "Context sensitive effect-free for {} is {} ",
    "which is not at least as refined as context insensitive "
    "effect-free {}\n",
    error_context(),
    contextType.effectFree,
    returnType.effectFree
  );

  return contextType;
}

//////////////////////////////////////////////////////////////////////

template<typename F> auto
visit_parent_cinfo(const ClassInfo* cinfo, F fun) -> decltype(fun(cinfo)) {
  for (auto ci = cinfo; ci != nullptr; ci = ci->parent) {
    if (auto const ret = fun(ci)) return ret;
    for (auto const trait : ci->usedTraits) {
      if (auto const ret = visit_parent_cinfo(trait, fun)) {
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
  return adjust_type_for_prop(
    IndexAdaptor { index },
    cls,
    &prop.typeConstraint,
    ty
  );
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
    IndexAdaptor { *data.m_index }, *knownCls, &prop->typeConstraint, TCell
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
      return clsCtx &&
        (clsCtx->classGraph.exactSubtypeOf(cls->classGraph, true, true) ||
         cls->classGraph.exactSubtypeOf(clsCtx->classGraph, true, true));
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
      if (t.moreRefined(
            lookup_constraint(IndexAdaptor { index }, ctx, tc, t).lower)
         ) {
      return R{ t, TriBool:: No };
    }
    // Otherwise adjust the type. If we get a Bottom we'll definitely
    // throw. We already know the type doesn't completely satisfy the
    // constraint, so we'll at least maybe throw.
    auto adjusted =
      adjust_type_for_prop(IndexAdaptor { index }, *ctx.cls, &tc, t);
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
              IndexAdaptor { *data.m_index },
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
  std::vector<SString> uninstantiable;
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
    [&] (SString cls) -> const TSStringSet& {
      auto const [d, _] = getDeps(cls);
      return *d;
    }
  );
  return build_hierarchical_work(buckets, numClasses, getDeps, getIdx);
}

template <typename GetDeps, typename GetIdx>
std::vector<HierarchicalWorkBucket>
build_hierarchical_work(std::vector<std::vector<SString>>& buckets,
                        size_t numClasses,
                        const GetDeps& getDeps,
                        const GetIdx& getIdx) {
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

      // Gather up all dependencies for this bucket
      TSStringSet deps;
      for (auto const cls : bucket) {
        auto const d = getDeps(cls).first;
        deps.insert(begin(*d), end(*d));
      }

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
        } else if (getDeps(d).second) {
          // Otherwise keep it as a dependency, but only if it's
          // actually instantiable.
          depOut.emplace_back(d);
        }
      }

      // Split off any uninstantiable classes in the bucket.
      auto const bucketEnd = std::partition(
        begin(bucket),
        end(bucket),
        [&] (SString cls) { return getDeps(cls).second; }
      );
      std::vector<SString> uninstantiable{bucketEnd, end(bucket)};
      bucket.erase(bucketEnd, end(bucket));

      // Keep deterministic ordering. Make sure there's no duplicates.
      std::sort(bucket.begin(), bucket.end(), string_data_lt_type{});
      std::sort(depOut.begin(), depOut.end(), string_data_lt_type{});
      std::sort(uninstantiable.begin(), uninstantiable.end(),
                string_data_lt_type{});
      assertx(std::adjacent_find(bucket.begin(), bucket.end()) == bucket.end());
      assertx(std::adjacent_find(depOut.begin(), depOut.end()) == depOut.end());
      assertx(
        std::adjacent_find(uninstantiable.begin(), uninstantiable.end()) ==
        uninstantiable.end()
      );
      return HierarchicalWorkBucket{
        std::move(bucket),
        std::move(depOut),
        std::move(uninstantiable)
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
    ClassGraph::init();
  }
  static void fini() { ClassGraph::destroy(); }

  /*
   * Metadata representing results of flattening. This is information
   * that the local coordinator (as opposed to later remote jobs) will
   * need to consume.
   */
  struct OutputMeta {
    // Classes which have been determined to be uninstantiable
    // (therefore have no result output data).
    TSStringSet uninstantiable;
    // New closures produced from trait flattening. Such new closures
    // will require "fixups" in the php::Program data.
    struct NewClosure {
      SString unit;
      SString context;
      SString name;
      template <typename SerDe> void serde(SerDe& sd) {
        sd(unit)(context)(name);
      }
    };
    std::vector<NewClosure> newClosures;
    // Report parents of each class. A class is a parent of another if
    // it would appear on a subclass list. The parents of a closure
    // are not reported because that's implicit.
    struct Parents {
      std::vector<SString> names;
      template <typename SerDe> void serde(SerDe& sd) { sd(names); }
    };
    std::vector<Parents> parents;
    // Classes which are interfaces.
    TSStringSet interfaces;
    // Classes which have 86init functions. A class can gain a 86init
    // from flattening even if it didn't have it before.
    TSStringSet with86init;
    // The types used by the type-constraints of input classes and
    // functions.
    std::vector<TSStringSet> classTypeUses;
    std::vector<TSStringSet> funcTypeUses;
    std::vector<InterfaceConflicts> interfaceConflicts;
    template <typename SerDe> void serde(SerDe& sd) {
      ScopedStringDataIndexer _;
      sd(uninstantiable, string_data_lt_type{})
        (newClosures)
        (parents)
        (interfaces, string_data_lt_type{})
        (with86init, string_data_lt_type{})
        (classTypeUses, string_data_lt_type{})
        (funcTypeUses, string_data_lt_type{})
        (interfaceConflicts)
        ;
    }
  };

  /*
   * Job returns a list of (potentially modified) php::Class, a list
   * of new ClassInfo2, a list of (potentially modified) php::Func,
   * and metadata for the entire job. The order of the lists reflects
   * the order of the input classes and functions (skipping over
   * classes marked as uninstantiable in the metadata).
   */
  using Output = Multi<
    Variadic<std::unique_ptr<php::Class>>,
    Variadic<std::unique_ptr<php::ClassBytecode>>,
    Variadic<std::unique_ptr<ClassInfo2>>,
    Variadic<std::unique_ptr<php::Func>>,
    Variadic<std::unique_ptr<FuncInfo2>>,
    Variadic<std::unique_ptr<MethodsWithoutCInfo>>,
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
                    Variadic<std::unique_ptr<php::Class>> uninstantiable,
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
        TSStringToOneT<php::Class*> out;
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

    for (auto const& cls : uninstantiable.vals) {
      always_assert(index.m_uninstantiable.emplace(cls->name).second);
    }

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
      assertx(cinfo->name->tsame(cls->name));

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
        string_data_lt_type{}
      );

      // Trait flattening may produce new closures, so those need to
      // be added to the local index as well.
      for (auto& [c, i] : closures) {
        ITRACE(5, "adding state for closure '{}' to local index\n", c->name);
        assertx(c->name->tsame(i->name));
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
    Variadic<std::unique_ptr<MethodsWithoutCInfo>> outMethods;
    OutputMeta outMeta;
    TSStringSet outNames;

    outClasses.vals.reserve(classes.vals.size() + newClosures.size());
    outInfos.vals.reserve(classes.vals.size() + newClosures.size());
    outMeta.newClosures.reserve(classes.vals.size());
    outNames.reserve(classes.vals.size());
    outMeta.parents.reserve(classes.vals.size() + newClosures.size());

    auto const makeMethodsWithoutCInfo = [&] (const php::Class& cls) {
      always_assert(outMeta.uninstantiable.emplace(cls.name).second);
      // Even though the class is uninstantiable, we still need to
      // create FuncInfos for it's methods. These are stored
      // separately (there's no ClassInfo to store it in!)
      auto methods = std::make_unique<MethodsWithoutCInfo>();
      methods->cls = cls.name;
      for (auto const& func : cls.methods) {
        methods->finfos.emplace_back(make_func_info(index, *func));
      }
      outMethods.vals.emplace_back(std::move(methods));
    };

    // If a closure's context is uninstantiable, then so is the
    // closure.
    auto const isContextInstantiable = [&] (const php::Class& cls) {
      if (!cls.closureContextCls) return true;
      if (index.uninstantiable(cls.closureContextCls)) {
        always_assert(!index.m_classInfos.count(cls.closureContextCls));
        return false;
      }
      return true;
    };

    // Do the processing which relies on a fully accessible
    // LocalIndex

    TSStringToOneT<InterfaceConflicts> ifaceConflicts;
    for (auto& cls : classes.vals) {
      auto const cinfoIt = index.m_classInfos.find(cls->name);
      if (cinfoIt == end(index.m_classInfos) || !isContextInstantiable(*cls)) {
        ITRACE(
          4, "{} discovered to be not instantiable, instead "
          "creating MethodsWithoutCInfo for it\n",
          cls->name
        );
        always_assert(
          IMPLIES(!cls->closureContextCls, index.uninstantiable(cls->name))
        );
        makeMethodsWithoutCInfo(*cls);
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

      // Record interface conflicts

      // Only consider normal or abstract classes
      if (cls->attrs &
          (AttrInterface | AttrTrait | AttrEnum | AttrEnumClass)) {
        continue;
      }

      auto const interfaces = cinfo->classGraph.interfaces();
      for (auto const i1 : interfaces) {
        auto& conflicts = ifaceConflicts[i1.name()];
        conflicts.name = i1.name();
        ++conflicts.usage;
        for (auto const i2 : interfaces) {
          if (i1 == i2) continue;
          conflicts.conflicts.emplace(i2.name());
        }
      }
    }

    outMeta.interfaceConflicts.reserve(ifaceConflicts.size());
    for (auto& [_, c] : ifaceConflicts) {
      outMeta.interfaceConflicts.emplace_back(std::move(c));
    }
    std::sort(
      begin(outMeta.interfaceConflicts),
      end(outMeta.interfaceConflicts),
      [] (auto const& c1, auto const& c2) {
        return string_data_lt_type{}(c1.name, c2.name);
      }
    );

    std::sort(
      begin(newClosures),
      end(newClosures),
      [] (const std::unique_ptr<php::Class>& a,
          const std::unique_ptr<php::Class>& b) {
        if (a->unit != b->unit) return string_data_lt{}(a->unit, b->unit);
        return string_data_lt_type{}(a->name, b->name);
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

    // We don't process classes marked as uninstantiable beforehand,
    // except for creating method FuncInfos for them.
    for (auto const& cls : uninstantiable.vals) {
      ITRACE(
        4, "{} already known to be not instantiable, creating "
        "MethodsWithoutCInfo for it\n",
        cls->name
      );
      makeMethodsWithoutCInfo(*cls);
    }

    // Now move the classes out of LocalIndex and into the output. At
    // this point, it's not safe to access the LocalIndex unless
    // you're sure something hasn't been moved yet.
    for (auto& cls : classes.vals) {
      auto const name = cls->name;

      // Check if this class has a 86*init function (it might have
      // already or might have gained one from trait flattening).
      auto const has86init =
        std::any_of(
          begin(cls->methods), end(cls->methods),
          [] (auto const& m) { return is_86init_func(*m); }
        );
      if (has86init) outMeta.with86init.emplace(name);

      auto const cinfoIt = index.m_classInfos.find(name);
      if (cinfoIt == end(index.m_classInfos) || !isContextInstantiable(*cls)) {
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

      // We always know the subclass status of closures and the
      // closure base class.
      if (is_closure_base(*cls)) {
        cinfo->classGraph.setClosureBase();
      } else if (is_closure(*cls)) {
        cinfo->classGraph.setComplete();
      }

      outClasses.vals.emplace_back(std::move(cls));
      outInfos.vals.emplace_back(std::move(cinfo));
    }

    for (auto& clo : newClosures) {
      assertx(!is_closure_base(*clo));
      assertx(clo->closureContextCls);
      // Only return the closures for classes we were actually
      // requested to flatten.
      if (!outNames.count(clo->closureContextCls)) continue;

      index.m_ctx = clo.get();
      SCOPE_EXIT { index.m_ctx = nullptr; };

      outMeta.newClosures.emplace_back(
        OutputMeta::NewClosure{
          clo->unit,
          clo->closureContextCls,
          clo->name
        }
      );

      auto& cinfo = index.m_classInfos.at(clo->name);

      // Closures are always leafs.
      cinfo->classGraph.setComplete();
      assertx(!cinfo->classGraph.mightHaveRegularSubclass());
      assertx(!cinfo->classGraph.mightHaveNonRegularSubclass());

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
      bytecode->cls = cls->name;
      bytecode->methodBCs.reserve(cls->methods.size());
      for (auto& method : cls->methods) {
        bytecode->methodBCs.emplace_back(
          method->name,
          std::move(method->rawBlocks)
        );
      }
      outBytecode.vals.emplace_back(std::move(bytecode));
    }

    return std::make_tuple(
      std::move(outClasses),
      std::move(outBytecode),
      std::move(outInfos),
      std::move(funcs),
      std::move(funcInfos),
      std::move(outMethods),
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

    TSStringToOneT<const php::Class*> m_classes;
    TSStringToOneT<std::unique_ptr<ClassInfo2>> m_classInfos;
    TSStringToOneT<std::unique_ptr<State>> m_states;

    hphp_fast_map<
      const php::Class*,
      hphp_fast_set<const php::Class*>
    > m_classClosures;

    TSStringSet m_uninstantiable;

    TSStringToOneT<TypeMapping> m_typeMappings;
    TSStringSet m_missingTypes;

    const php::Class& cls(SString name) const {
      if (m_ctx->name->tsame(name)) return *m_ctx;
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

    const php::Const& cns(const ConstIndex& idx) const {
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
    const TSStringToOneT<php::Class*>& classes
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
        return string_data_lt_type{}(c1->name, c2->name);
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
        always_assert(cls.parentName->tsame(s_Closure.get()));
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

    // Create a ClassGraph for this class. If we decide to not keep
    // the ClassInfo, reset the ClassGraph to keep it from ending up
    // in the graph.
    cinfo->classGraph = ClassGraph::create(cls);
    auto success = false;
    SCOPE_EXIT { if (!success) cinfo->classGraph.reset(); };

    // Assume the class isn't overridden. This is true for leafs and
    // non-leafs will get updated when we build subclass information.
    if (!is_closure_base(cls)) {
      attribute_setter(cls.attrs, true, AttrNoOverride);
      attribute_setter(cls.attrs, true, AttrNoOverrideRegular);
    } else {
      attribute_setter(cls.attrs, false, AttrNoOverride);
      attribute_setter(cls.attrs, false, AttrNoOverrideRegular);
    }

    // Assume this. If not a leaf, will be updated in
    // BuildSubclassList job.
    attribute_setter(cls.attrs, true, AttrNoMock);

    if (cls.parentName) {
      assertx(!is_closure_base(cls));
      assertx(is_closure(cls) == cls.parentName->tsame(s_Closure.get()));

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
      if (!enforce_sealing(*cinfo, cls, parent)) return nullptr;

      cinfo->parent = cls.parentName;
      cinfo->hasConstProp |= parentInfo.hasConstProp;
      cinfo->hasReifiedParent |= parentInfo.hasReifiedParent;

      state.m_parents.emplace_back(&parent);
      cinfo->classGraph.setBase(parentInfo.classGraph);
    } else if (!cinfo->hasReifiedGeneric) {
      attribute_setter(cls.attrs, true, AttrNoReifiedInit);
    }

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
      if (!enforce_sealing(*cinfo, cls, iface)) return nullptr;

      cinfo->hasReifiedParent |= ifaceInfo.hasReifiedParent;

      state.m_parents.emplace_back(&iface);
      cinfo->classGraph.addParent(ifaceInfo.classGraph);
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
      if (!enforce_sealing(*cinfo, cls, e)) return nullptr;

      for (auto const iface : einfo.classGraph.declInterfaces()) {
        cinfo->classGraph.addParent(iface);
      }
    }

    auto const clsHasModuleLevelTrait =
      cls.userAttributes.count(s___ModuleLevelTrait.get());
    if (clsHasModuleLevelTrait &&
        (!(cls.attrs & AttrTrait) || (cls.attrs & AttrInternal))) {
      ITRACE(2,
             "Making class-info failed for `{}' because "
             "attribute <<__ModuleLevelTrait>> can only be "
             "specified on public traits\n",
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

      assertx(!is_closure(trait));
      if (!(trait.attrs & AttrTrait)) {
        ITRACE(2,
               "Making class-info failed for `{}' because `{}' "
               "is not a trait\n",
               cls.name, tname);
        return nullptr;
      }
      if (!enforce_sealing(*cinfo, cls, trait)) return nullptr;

      cinfo->hasConstProp |= traitInfo.hasConstProp;
      cinfo->hasReifiedParent |= traitInfo.hasReifiedParent;

      state.m_parents.emplace_back(&trait);
      cinfo->classGraph.addParent(traitInfo.classGraph);
    }

    if (cls.attrs & AttrEnum) {
      auto const baseType = [&] {
        auto const& base = cls.enumBaseTy;
        if (!base.isUnresolved()) return base.type();
        auto const tm = index.typeMapping(base.typeName());
        if (!tm) return AnnotType::Unresolved;
        // enums cannot use case types
        assertx(!tm->value.isUnion());
        return tm->value.type();
      }();
      if (!enumSupportsAnnot(baseType)) {
        ITRACE(2,
               "Making class-info failed for `{}' because {} "
               "is not a valid enum base type\n",
               cls.name, annotName(baseType));
        return nullptr;
      }
    }

    if (!build_methods(index, cls, *cinfo, state))    return nullptr;
    if (!build_properties(index, cls, *cinfo, state)) return nullptr;
    if (!build_constants(index, cls, *cinfo, state))  return nullptr;

    if (auto const closures = index.classClosures(cls)) {
      for (auto const clo : *closures) {
        assertx(is_closure(*clo));
        assertx(
          clo->closureContextCls && clo->closureContextCls->tsame(cls.name)
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
        return string_data_lt_type{}(a->name, b->name);
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
        always_assert(state.m_parents[0]->name->tsame(s_Closure.get()));
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
      always_assert(!is_mock_class(&cls));
    }

    ITRACE(2, "new class-info: {}\n", cls.name);
    if (Trace::moduleEnabled(Trace::hhbbc_index, 3)) {
      if (cinfo->parent) {
        ITRACE(3, "           parent: {}\n", cinfo->parent);
      }
      auto const cg = cinfo->classGraph;
      for (auto const DEBUG_ONLY base : cg.bases()) {
        ITRACE(3, "             base: {}\n", base.name());
      }
      for (auto const DEBUG_ONLY iface : cls.interfaceNames) {
        ITRACE(3, "  decl implements: {}\n", iface);
      }
      for (auto const DEBUG_ONLY iface : cg.interfaces()) {
        ITRACE(3, "       implements: {}\n", iface.name());
      }
      for (auto const DEBUG_ONLY e : cls.includedEnumNames) {
        ITRACE(3, "             enum: {}\n", e);
      }
      for (auto const DEBUG_ONLY trait : cls.usedTraitNames) {
        ITRACE(3, "             uses: {}\n", trait);
      }
      for (auto const DEBUG_ONLY closure : cinfo->closures) {
        ITRACE(3, "          closure: {}\n", closure);
      }
    }

    // We're going to use this ClassInfo.
    success = true;
    return cinfo;
  }

  static bool enforce_sealing(const ClassInfo2& cinfo,
                              const php::Class& cls,
                              const php::Class& parent) {
    if (is_mock_class(&cls)) return true;
    if (!(parent.attrs & AttrSealed)) return true;
    auto const it = parent.userAttributes.find(s___Sealed.get());
    assertx(it != parent.userAttributes.end());
    assertx(tvIsArrayLike(it->second));
    auto allowed = false;
    IterateV(
      it->second.m_data.parr,
      [&] (TypedValue v) {
        assertx(tvIsStringLike(v));
        if (tvAssertStringLike(v)->tsame(cinfo.name)) {
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

    for (auto const iface : cls.interfaceNames) {
      if (!merge_properties(cinfo, state, index.state(iface))) {
        return false;
      }
    }
    for (auto const trait : cls.usedTraitNames) {
      if (!merge_properties(cinfo, state, index.state(trait))) {
        return false;
      }
    }
    for (auto const e : cls.includedEnumNames) {
      if (!merge_properties(cinfo, state, index.state(e))) {
        return false;
      }
    }

    if (cls.attrs & AttrInterface) return true;

    auto const cannotDefineInternalProperties =
      // public traits cannot define internal properties unless they
      // have the __ModuleLevelTrait attribute
      ((cls.attrs & AttrTrait) && (cls.attrs & AttrPublic)) &&
        !(cls.userAttributes.count(s___ModuleLevelTrait.get()));

    for (auto const& p : cls.properties) {
      if (cannotDefineInternalProperties && (p.attrs & AttrInternal)) {
        ITRACE(2,
               "Adding property failed for `{}' because property `{}' "
               "is internal and public traits cannot define internal properties\n",
               cinfo.name, p.name);
        return false;
      }
      if (!add_property(cinfo, state, p.name, p, cinfo.name, false)) {
        return false;
      }
    }

    // There's no need to do this work if traits have been flattened
    // already, or if the top level class has no traits.  In those
    // cases, we might be able to rule out some instantiations, but it
    // doesn't seem worth it.
    if (cls.attrs & AttrNoExpandTrait) return true;

    for (auto const traitName : cls.usedTraitNames) {
      auto const& trait = index.cls(traitName);
      auto const& traitInfo = index.classInfo(traitName);
      for (auto const& p : trait.properties) {
        if (!add_property(cinfo, state, p.name, p, cinfo.name, true)) {
          return false;
        }
      }
      for (auto const& p : traitInfo.traitProps) {
        if (!add_property(cinfo, state, p.name, p, cinfo.name, true)) {
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
                           bool trait) {
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

    if (cinfo.name->tsame(prevSrc)) {
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
      if (!add_property(cinfo, dst, name, prop, src, false)) {
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

    for (auto const iname : cls.interfaceNames) {
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
          ConstIndex { cls.name, idx },
          false
        );
        if (!added) return false;
      }
      return true;
    };

    auto const addTraitConstants = [&] {
      for (auto const tname : cls.usedTraitNames) {
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

    for (auto const ename : cls.includedEnumNames) {
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
      if (existing->second.cls->tsame(c.cls)) {
        state.m_traitCns.emplace_back(c);
        state.m_traitCns.back().isFromTrait = true;
      }
    };
    for (auto const tname : cls.usedTraitNames) {
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
      if (cnsIdx.cls->tsame(cls.name)) continue;

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
                           const ConstIndex& cnsIdx,
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
    if (existingIdx.cls->tsame(cnsIdx.cls)) return true;

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
        auto const& cls = index.cls(cinfo.name);
        for (auto const iface : cls.interfaceNames) {
          if (existingIdx.cls->tsame(iface)) {
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
    for (auto const iname : cls.interfaceNames) {
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
      assertx(cls.usedTraitNames.empty());
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
    auto const clsHasModuleLevelTrait =
      cls.userAttributes.count(s___ModuleLevelTrait.get());

    // Now add our methods.
    for (auto const& m : cls.methods) {
      if ((cls.attrs & AttrTrait) &&
          (!((cls.attrs & AttrInternal) || clsHasModuleLevelTrait)) &&
          (m->attrs & AttrInternal)) {
        ITRACE(2,
            "Adding methods failed for `{}' because "
            "method `{}' is internal and public traits "
            "cannot define internal methods unless they have "
            "the <<__ModuleLevelTrait>> attribute\n",
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
      for (auto const tname : cls.usedTraitNames) {
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
          if (emplaced.first->second.meth().cls->tsame(cls.name)) continue;
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
                bool requiresFromOriginalModule,
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
      meth->cls = clone.get();
      assertx(meth->clsIdx == i);
      if (!meth->originalFilename) meth->originalFilename = meth->unit;
      if (!meth->originalUnit)     meth->originalUnit = meth->unit;
      if (!meth->originalClass)    meth->originalClass = closure.name;
      meth->requiresFromOriginalModule = requiresFromOriginalModule;
      meth->unit = newContext.unit;

      clone->methods[i] =
        clone_closures(index, std::move(meth), requiresFromOriginalModule, clonedClosures);
      if (!clone->methods[i]) return nullptr;
    }

    return clone;
  }

  static std::unique_ptr<php::Func>
  clone_closures(const LocalIndex& index,
                 std::unique_ptr<php::Func> cloned,
                 bool requiresFromOriginalModule,
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
        requiresFromOriginalModule,
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
    const bool requiresFromOriginalModule = [&] () {
      if (RO::EvalModuleLevelTraits && orig.fromModuleLevelTrait &&
         !orig.requiresFromOriginalModule &&
         orig.originalModuleName != dstCls.moduleName) {
        return true;
      } else {
        return orig.requiresFromOriginalModule;
      }
    }();
    cloned->requiresFromOriginalModule = requiresFromOriginalModule;

    // cloned method isn't in any method table yet, so trash its
    // index.
    cloned->clsIdx = std::numeric_limits<uint32_t>::max();
    return clone_closures(index, std::move(cloned), requiresFromOriginalModule, clonedClosures);
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

    for (auto const tname : cls.usedTraitNames) {
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
      cls.attrs |= AttrNoExpandTrait;
      return {};
    }

    ITRACE(4, "flatten traits: {}\n", cls.name);
    Trace::Indent indent;

    assertx(!is_closure(cls));

    auto traitHasConstProp = cls.hasConstProp;
    for (auto const tname : cls.usedTraitNames) {
      auto const& trait = index.cls(tname);
      auto const& tinfo = index.classInfo(tname);
      if (!(trait.attrs & AttrNoExpandTrait)) {
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
      if (mte.meth().cls->tsame(cls.name)) continue;
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
    for (auto const tname : cls.usedTraitNames) {
      auto const& tinfo = index.classInfo(tname);
      cinfo.classGraph.flattenTraitInto(tinfo.classGraph);
    }

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

    for (auto const tname : cls.usedTraitNames) {
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
        return string_data_lt_type{}(a->name, b->name);
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
          assertx(!it->second.meth().cls->tsame(cls.name));
          it->second.setMeth(MethRef { cls.name, clone->clsIdx });
        } else {
          auto const [existing, emplaced] =
            cinfo.methods.emplace(clone->name, MethTabEntry { *clone });
          if (!emplaced) {
            assertx(existing->second.meth().cls->tsame(cls.name));
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
        assertx(clo->closureContextCls->tsame(cls.name));
        assertx(clo->unit == cls.unit);

        assertx(clo->usedTraitNames.empty());
        State cloState;
        auto cloinfo = make_info(index, *clo, cloState);
        assertx(cloinfo);
        assertx(cloState.m_traitCns.empty());
        assertx(cloState.m_cnsFromTrait.empty());
        assertx(cloState.m_parents.size() == 1);
        assertx(cloState.m_parents[0]->name->tsame(s_Closure.get()));

        cinfo.closures.emplace_back(clo->name);
        newClosures.emplace_back(
          NewClosure { std::move(clo), std::move(cloinfo) }
        );
      }

      std::sort(
        newClosures.begin(),
        newClosures.end(),
        [] (auto const& p1, auto const& p2) {
          return string_data_lt_type{}(p1.cls->name, p2.cls->name);
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
        for (auto const iname : cinfo.classGraph.interfaces()) {
          auto const& iface = index.classInfo(iname.name());
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

    for (auto const tname : cls.usedTraitNames) {
      auto const& trait = index.cls(tname);
      for (auto const& req : trait.requirements) {
        if (reqs.emplace(req).second) cls.requirements.emplace_back(req);
      }
    }

    cls.attrs |= AttrNoExpandTrait;
    return newClosures;
  }

  static std::unique_ptr<FuncInfo2> make_func_info(const LocalIndex& index,
                                                   const php::Func& f) {
    auto finfo = std::make_unique<FuncInfo2>();
    finfo->name = f.name;
    return finfo;
  }

  static bool resolve_one(TypeConstraint& tc,
                          const TypeConstraint& tv,
                          SString firstEnum,
                          TSStringSet* uses,
                          bool isProp,
                          bool isUnion) {
    assertx(!tv.isUnion());
    // Whatever it's an alias of isn't valid, so leave unresolved.
    if (tv.isUnresolved()) return false;
    if (isProp && !propSupportsAnnot(tv.type())) return false;
    auto const value = [&] () -> SString {
      // Store the first enum encountered during resolution. This
      // lets us fixup the type later if needed.
      if (firstEnum) return firstEnum;
      if (tv.isSubObject()) {
        auto clsName = tv.clsName();
        assertx(clsName);
        return clsName;
      }
      return nullptr;
    }();
    if (isUnion) tc.unresolve();
    tc.resolveType(tv.type(), tv.isNullable(), value);
    assertx(IMPLIES(isProp, tc.validForProp()));
    if (uses && value) uses->emplace(value);
    return true;
  }

  // Update a type constraint to it's ultimate type, or leave it as
  // unresolved if it resolves to nothing valid. Record the new type
  // in case it needs to be fixed up later.
  static void update_type_constraint(const LocalIndex& index,
                                     TypeConstraint& tc,
                                     bool isProp,
                                     TSStringSet* uses) {
    always_assert(IMPLIES(isProp, tc.validForProp()));

    if (!tc.isUnresolved()) {
      // Any TC already resolved is assumed to be correct.
      if (uses) {
        for (auto& part : eachTypeConstraintInUnion(tc)) {
          if (auto clsName = part.clsName()) {
            uses->emplace(clsName);
          }
        }
      }
      return;
    }
    auto const name = tc.typeName();

    if (tc.isUnion()) {
      // This is a union that contains unresolved names.
      not_implemented(); // TODO(T151885113)
    }

    // This is an unresolved name that can resolve to either a single type or
    // a union.

    // Is this name a type-alias or enum?
    if (auto const tm = index.typeMapping(name)) {
      if (tm->value.isUnion()) {
        auto flags =
          tc.flags() & (TypeConstraintFlags::Nullable
                        | TypeConstraintFlags::TypeVar
                        | TypeConstraintFlags::Soft
                        | TypeConstraintFlags::TypeConstant
                        | TypeConstraintFlags::DisplayNullable
                        | TypeConstraintFlags::UpperBound);
        std::vector<TypeConstraint> members;
        for (auto& tv : eachTypeConstraintInUnion(tm->value)) {
          TypeConstraint copy = tv;
          copy.addFlags(flags);
          if (!resolve_one(copy, tv, tm->firstEnum, uses, isProp, true)) {
            return;
          }
          members.emplace_back(std::move(copy));
        }
        tc = TypeConstraint::makeUnion(name, members);
        return;
      }

      // This unresolved name resolves to a single type.
      assertx(!tm->value.isUnion());
      resolve_one(tc, tm->value, tm->firstEnum, uses, isProp, false);
      return;
    }

    // Not a type-alias or enum. If it's explicitly marked as missing,
    // leave it unresolved. Otherwise assume it's an object with that
    // name.
    if (index.missingType(name)) return;
    tc.resolveType(AnnotType::SubObject, tc.isNullable(), name);
    if (uses) uses->emplace(name);
  }

  static void update_type_constraints(const LocalIndex& index,
                                      php::Func& func,
                                      TSStringSet* uses) {
    for (auto& p : func.params) {
      update_type_constraint(index, p.typeConstraint, false, uses);
      for (auto& ub : p.upperBounds.m_constraints) {
        update_type_constraint(index, ub, false, uses);
      }
    }
    update_type_constraint(index, func.retTypeConstraint, false, uses);
    for (auto& ub : func.returnUBs.m_constraints) {
      update_type_constraint(index, ub, false, uses);
    }
  }

  static void update_type_constraints(const LocalIndex& index,
                                      php::Class& cls,
                                      TSStringSet* uses) {
    if (cls.attrs & AttrEnum) {
      update_type_constraint(index, cls.enumBaseTy, false, uses);
    }
    for (auto& meth : cls.methods) update_type_constraints(index, *meth, uses);
    for (auto& prop : cls.properties) {
      update_type_constraint(index, prop.typeConstraint, true, uses);
      for (auto& ub : prop.ubs.m_constraints) {
        update_type_constraint(index, ub, true, uses);
      }
    }
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

    auto const isClosure = is_closure(cls);

    cinfo.hasBadRedeclareProp = false;
    for (auto& prop : cls.properties) {
      assertx(!(prop.attrs & AttrNoBadRedeclare));
      assertx(!(prop.attrs & AttrNoImplicitNullable));

      auto const noBadRedeclare = [&] {
        // Closures should never have redeclared properties.
        if (isClosure) return true;
        // Static and private properties never redeclare anything so
        // need not be considered.
        if (prop.attrs & (AttrStatic | AttrPrivate)) return true;

        for (auto const base : cinfo.classGraph.bases()) {
          if (base.name()->tsame(cls.name)) continue;

          auto& baseCInfo = index.classInfo(base.name());
          auto& baseCls = index.cls(base.name());

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
          update_type_constraint(
            index,
            parentProp->typeConstraint,
            true,
            nullptr
          );

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


Job<FlattenJob> s_flattenJob;

/*
 * For efficiency reasons, we want to do class flattening all in one
 * pass. So, we use assign_hierarchical_work (described above) to
 * calculate work buckets to allow us to do this.
 *
 * - The "root" classes are the leaf classes in the hierarchy. These are
 *   the buckets which are not dependencies of anything.
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
    TSStringSet deps;
    std::vector<SString> closures;
    // All types mentioned in type-constraints in this class.
    std::vector<SString> unresolvedTypes;
    size_t idx; // Index into allCls vector
    bool isClosure{false};
    bool uninstantiable{false};
    SString closureContext{nullptr};
  };
  TSStringToOneT<ClassMeta> cls;
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
  FSStringToOneT<FuncMeta> func;
  std::vector<SString> allFuncs;
  TSStringToOneT<TypeMapping> typeMappings;
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
      Optional<TSStringSet> seen;
      TypeConstraintFlags flags =
        typeMapping->value.flags() & (TypeConstraintFlags::Nullable
                                      | TypeConstraintFlags::TypeVar
                                      | TypeConstraintFlags::Soft
                                      | TypeConstraintFlags::TypeConstant
                                      | TypeConstraintFlags::DisplayNullable
                                      | TypeConstraintFlags::UpperBound);
      auto firstEnum = typeMapping->firstEnum;

      auto enumMeta = folly::get_ptr(meta.cls, typeMapping->name);

      std::vector<TypeConstraint> tvu;

      for (auto const& tc : eachTypeConstraintInUnion(typeMapping->value)) {
        const auto type = tc.type();
        const auto value = tc.typeName();
        auto name = value;

        if (type != AnnotType::Unresolved) {
          // If the type-mapping is already resolved, we mainly take it
          // as is. The exception is if it's an enum, in which case we
          // validate the underlying base type.
          assertx(type != AnnotType::SubObject);
          if (!enumMeta) {
            tvu.emplace_back(tc);
            continue;
          }
          if (!enumSupportsAnnot(type)) {
            FTRACE(
              2, "Type-mapping '{}' is invalid because it resolves to "
              "invalid enum type {}\n",
              typeMapping->name,
              annotName(type)
            );
            tvu.emplace_back(AnnotType::Unresolved, tc.flags(), value);
            continue;
          }
          tvu.emplace_back(type, tc.flags() | flags, value);
          continue;
        }

        std::queue<LSString> queue;
        queue.push(name);

        for (size_t rounds = 0;; ++rounds) {
          if (queue.empty()) break;
          name = normalizeNS(queue.back());
          queue.pop();

          if (auto const next = folly::get_ptr(meta.typeMappings, name)) {
            flags |= next->value.flags() & (TypeConstraintFlags::Nullable
                                            | TypeConstraintFlags::TypeVar
                                            | TypeConstraintFlags::Soft
                                            | TypeConstraintFlags::TypeConstant
                                            | TypeConstraintFlags::DisplayNullable
                                            | TypeConstraintFlags::UpperBound);
            if (!firstEnum) firstEnum = next->firstEnum;

            if (enumMeta && next->firstEnum) {
              enumMeta->deps.emplace(next->firstEnum);
            }

            for (auto const& next_tc : eachTypeConstraintInUnion(next->value)) {
              auto next_type = next_tc.type();
              auto next_value = next_tc.typeName();
              if (next_type == AnnotType::Unresolved) {
                queue.push(next_value);
                continue;
              }
              assertx(next_type != AnnotType::SubObject);
              if (firstEnum && !enumSupportsAnnot(next_type)) {
                FTRACE(
                  2, "Type-mapping '{}' is invalid because it resolves to "
                  "invalid enum type {}{}\n",
                  typeMapping->name,
                  annotName(next_type),
                  firstEnum->tsame(typeMapping->name)
                    ? "" : folly::sformat(" (via {})", firstEnum)
                );
                tvu.emplace_back(AnnotType::Unresolved, tc.flags() | flags, name);
                continue;
              }
              tvu.emplace_back(next_type, tc.flags() | flags, next_value);
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

            tvu.emplace_back(
              firstEnum ? AnnotType::Unresolved : AnnotType::SubObject,
              tc.flags() | flags,
              name
            );
            break;
          } else {
            FTRACE(
              2, "Type-mapping '{}' is invalid because it involves "
              "non-existent type '{}'{}\n",
              typeMapping->name,
              name,
              (firstEnum && !firstEnum->tsame(typeMapping->name))
                ? folly::sformat(" (via {})", firstEnum) : ""
            );
            tvu.emplace_back(AnnotType::Unresolved, tc.flags() | flags, name);
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
                TypeConstraint{AnnotType::Unresolved, flags, name},
              };
            }
          }
        }
      }
      assertx(!tvu.empty());
      auto value = TypeConstraint::makeUnion(typeMapping->name, std::move(tvu));
      return TypeMapping { typeMapping->name, firstEnum, value };
    }
  );

  for (auto& after : resolved) {
    auto const name = after.name;
    using namespace folly::gen;
    FTRACE(
      4, "Type-mapping '{}' flattened to {}{}\n",
      name,
      after.value.debugName(),
      (after.firstEnum && !after.firstEnum->tsame(name))
        ? folly::sformat(" (via {})", after.firstEnum) : ""
    );
    if (after.value.isUnresolved() && meta.cls.count(name)) {
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
  std::vector<SString> uninstantiable;
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
    TSStringSet deps;
    // Whether this class is instantiable
    bool instantiable{false};
  };
  std::vector<LockFreeLazy<DepLookup>> allDeps{meta.allCls.size()};

  // Look up all of the transitive dependencies for the given class.
  auto const findAllDeps = [&] (SString cls,
                                TSStringSet& visited,
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
          if (lookup.instantiable || meta.cls.count(d)) {
            out.deps.emplace(d);
          }
          out.deps.insert(begin(lookup.deps), end(lookup.deps));
          if (lookup.instantiable) return;
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
        };

        for (auto const d : deps)       onDep(d);
        for (auto const clo : closures) onDep(clo);
        return out;
      }
    );
  };

  // If a closure's context class is not uninstantiable, then the
  // closure is likewise not instantiable. Calculate it here (we
  // cannot do it in findAllDeps because it creates a circular
  // dependency between the closure and the context class).
  parallel::for_each(
    meta.allCls,
    [&] (SString cls) {
      auto& clsMeta = meta.cls.at(cls);
      if (!clsMeta.closureContext) return;
      TSStringSet visited;
      auto const& lookup =
        findAllDeps(clsMeta.closureContext, visited, findAllDeps);
      if (!lookup.instantiable) {
        if (!clsMeta.uninstantiable) {
          FTRACE(
            4,
            "{} is not instantiable because the "
            "closure context {} is not instantiable\n",
            cls, clsMeta.closureContext
          );
        }
        clsMeta.uninstantiable = true;
      }
    }
  );
  // Reset the lookup results so that further lookups will correctly
  // incorporate non-instantiable closures.
  parallel::for_each(allDeps, [] (LockFreeLazy<DepLookup>& l) { l.reset(); });

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
      TSStringSet visited;
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
        std::move(funcBuckets[0]),
        std::move(assignments[0].uninstantiable)
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
          {},
          std::move(assignment.uninstantiable)
        }
      );
    }
    for (auto& bucket : funcBuckets) {
      work.emplace_back(
        FlattenClassesWork{ {}, {}, std::move(bucket), {} }
      );
    }
  }

  if (Trace::moduleEnabled(Trace::hhbbc_index, 5)) {
    for (size_t i = 0; i < work.size(); ++i) {
      auto const& [classes, deps, funcs, uninstantiable] = work[i];
      FTRACE(5, "flatten work item #{}:\n", i);
      FTRACE(5, "  classes ({}):\n", classes.size());
      for (auto const DEBUG_ONLY c : classes) FTRACE(5, "    {}\n", c);
      FTRACE(5, "  deps ({}):\n", deps.size());
      for (auto const DEBUG_ONLY d : deps) FTRACE(5, "    {}\n", d);
      FTRACE(5, "  funcs ({}):\n", funcs.size());
      for (auto const DEBUG_ONLY f : funcs) FTRACE(5, "    {}\n", f);
      FTRACE(5, "  uninstantiable classes ({}):\n", uninstantiable.size());
      for (auto const DEBUG_ONLY c : uninstantiable) FTRACE(5, "    {}\n", c);
    }
  }

  return work;
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
  TSStringToOneT<Meta> meta;
  // All classes to be processed
  std::vector<SString> all;
};

// Metadata used to drive the init-types pass. This is produced from
// flattening classes and added to when building subclasses.
struct InitTypesMetadata {
  struct ClsMeta {
    // Dependencies of the class. A dependency is a class in a
    // property/param/return type-hint.
    TSStringSet deps;
    TSStringSet candidateRegOnlyEquivs;
  };
  struct FuncMeta {
    // Same as ClsMeta, but for the func
    TSStringSet deps;
  };
  // Modifications to make to an unit
  struct Fixup {
    std::vector<SString> addClass;
    std::vector<SString> removeFunc;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(addClass)(removeFunc);
    }
  };
  TSStringToOneT<ClsMeta> classes;
  FSStringToOneT<FuncMeta> funcs;
  SStringToOneT<Fixup> fixups;
  SStringToOneT<std::vector<FuncFamilyEntry>> nameOnlyFF;
};

std::tuple<SubclassMetadata, InitTypesMetadata, std::vector<InterfaceConflicts>>
flatten_classes(IndexData& index, IndexFlattenMetadata meta) {
  trace_time trace("flatten classes", index.sample);
  trace.ignore_client_stats();

  using namespace folly::gen;

  struct ClassUpdate {
    SString name;
    UniquePtrRef<php::Class> cls;
    UniquePtrRef<php::ClassBytecode> bytecode;
    UniquePtrRef<ClassInfo2> cinfo;
    SString unitToAddTo;
    SString closureContext;
    TSStringSet typeUses;
    bool isInterface{false};
    bool has86init{false};
    CompactVector<SString> parents;
  };
  struct FuncUpdate {
    SString name;
    UniquePtrRef<php::Func> func;
    UniquePtrRef<FuncInfo2> finfo;
    TSStringSet typeUses;
  };
  struct MethodUpdate {
    SString name;
    UniquePtrRef<MethodsWithoutCInfo> methods;
  };
  using Update = boost::variant<ClassUpdate, FuncUpdate, MethodUpdate>;
  using UpdateVec = std::vector<Update>;

  tbb::concurrent_hash_map<
    SString,
    InterfaceConflicts,
    string_data_hash_tsame
  > ifaceConflicts;

  auto const run = [&] (FlattenClassesWork work) -> coro::Task<UpdateVec> {
    co_await coro::co_reschedule_on_current_executor;

    if (work.classes.empty() &&
        work.funcs.empty() &&
        work.uninstantiable.empty()) {
      assertx(work.deps.empty());
      co_return UpdateVec{};
    }

    Client::ExecMetadata metadata{
      .job_key = folly::sformat(
        "flatten classes {}",
        work.classes.empty()
          ? (work.uninstantiable.empty()
             ? work.funcs[0]
             : work.uninstantiable[0])
          : work.classes[0]
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
    auto uninstantiableRefs = from(work.uninstantiable)
      | map([&] (SString c) { return index.classRefs.at(c); })
      | as<std::vector>();

    // Gather any type-mappings or missing types referenced by these
    // classes or funcs.
    std::vector<TypeMapping> typeMappings;
    std::vector<SString> missingTypes;
    {
      TSStringSet seen;

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
          return string_data_lt_type{}(a.name, b.name);
        }
      );
      std::sort(begin(missingTypes), end(missingTypes), string_data_lt_type{});
    }

    auto [typeMappingsRef, missingTypesRef, config] = co_await
      coro::collectAll(
        index.client->store(std::move(typeMappings)),
        index.client->store(std::move(missingTypes)),
        index.configRef->getCopy()
      );

    auto results = co_await
      index.client->exec(
        s_flattenJob,
        std::move(config),
        singleton_vec(
          std::make_tuple(
            std::move(classes),
            std::move(deps),
            std::move(bytecode),
            std::move(funcs),
            std::move(uninstantiableRefs),
            std::move(typeMappingsRef),
            std::move(missingTypesRef)
          )
        ),
        std::move(metadata)
      );
    // Every flattening job is a single work-unit, so we should only
    // ever get one result for each one.
    assertx(results.size() == 1);
    auto& [clsRefs, bytecodeRefs, cinfoRefs, funcRefs,
           finfoRefs, methodRefs, classMetaRef] = results[0];
    assertx(clsRefs.size() == cinfoRefs.size());
    assertx(clsRefs.size() == bytecodeRefs.size());
    assertx(funcRefs.size() == work.funcs.size());
    assertx(funcRefs.size() == finfoRefs.size());

    // We need the output metadata, but everything else stays
    // uploaded.
    auto clsMeta = co_await index.client->load(std::move(classMetaRef));
    assertx(methodRefs.size() == clsMeta.uninstantiable.size());

    // Create the updates by combining the job output (but skipping
    // over uninstantiable classes).
    UpdateVec updates;
    updates.reserve(work.classes.size() * 3);

    size_t outputIdx = 0;
    size_t parentIdx = 0;
    size_t methodIdx = 0;
    for (auto const name : work.classes) {
      if (clsMeta.uninstantiable.count(name)) {
        assertx(methodIdx < methodRefs.size());
        updates.emplace_back(
          MethodUpdate{ name, std::move(methodRefs[methodIdx]) }
        );
        ++methodIdx;
        continue;
      }
      assertx(outputIdx < clsRefs.size());
      assertx(outputIdx < clsMeta.classTypeUses.size());

      auto const& flattenMeta = meta.cls.at(name);
      updates.emplace_back(
        ClassUpdate{
          name,
          std::move(clsRefs[outputIdx]),
          std::move(bytecodeRefs[outputIdx]),
          std::move(cinfoRefs[outputIdx]),
          nullptr,
          flattenMeta.closureContext,
          std::move(clsMeta.classTypeUses[outputIdx]),
          (bool)clsMeta.interfaces.count(name),
          (bool)clsMeta.with86init.count(name)
        }
      );

      // Ignore closures. We don't run the build subclass pass for
      // closures, so we don't need information for them.
      if (!flattenMeta.isClosure) {
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

    for (auto const& [unit, context, name] : clsMeta.newClosures) {
      assertx(outputIdx < clsRefs.size());
      assertx(outputIdx < clsMeta.classTypeUses.size());
      updates.emplace_back(
        ClassUpdate{
          name,
          std::move(clsRefs[outputIdx]),
          std::move(bytecodeRefs[outputIdx]),
          std::move(cinfoRefs[outputIdx]),
          unit,
          context,
          std::move(clsMeta.classTypeUses[outputIdx])
        }
      );
      ++outputIdx;
    }
    assertx(outputIdx == clsRefs.size());
    assertx(outputIdx == clsMeta.classTypeUses.size());

    for (auto const name : work.uninstantiable) {
      assertx(clsMeta.uninstantiable.count(name));
      assertx(methodIdx < methodRefs.size());
      updates.emplace_back(
        MethodUpdate{ name, std::move(methodRefs[methodIdx]) }
      );
      ++methodIdx;
    }
    assertx(methodIdx == methodRefs.size());

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

    for (auto const& c : clsMeta.interfaceConflicts) {
      decltype(ifaceConflicts)::accessor acc;
      ifaceConflicts.insert(acc, c.name);
      acc->second.name = c.name;
      acc->second.usage += c.usage;
      acc->second.conflicts.insert(begin(c.conflicts), end(c.conflicts));
    }

    co_return updates;
  };

  // Calculate the grouping of classes into work units for flattening,
  // perform the flattening, and gather all updates from the jobs.
  auto allUpdates = [&] {
    auto assignments = flatten_classes_assign(meta);

    trace_time trace2("flatten classes work", index.sample);
    return coro::blockingWait(coro::collectAllRange(
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

  SubclassMetadata subclassMeta;
  InitTypesMetadata initTypesMeta;

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
            initTypesMeta.fixups[u->unitToAddTo].addClass.emplace_back(u->name);
            always_assert(
              index.classToUnit.emplace(u->name, u->unitToAddTo).second
            );
          }
        }
        for (auto& [unit, deletions] : meta.unitDeletions) {
          initTypesMeta.fixups[unit].removeFunc = std::move(deletions);
        }
      },
      [&] {
        // A class which didn't have an 86*init function previously
        // can gain one due to trait flattening. Update that here.
        for (auto const& updates : allUpdates) {
          for (auto const& update : updates) {
            auto u = boost::get<ClassUpdate>(&update);
            if (!u || !u->has86init) continue;
            index.classesWith86Inits.emplace(u->name);
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
            if (is_closure_name(u->name) || is_closure_base(u->name)) {
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

        std::sort(begin(all), end(all), string_data_lt_type{});
        // Make sure there's no duplicates:
        assertx(std::adjacent_find(begin(all), end(all)) == end(all));

        for (size_t i = 0; i < all.size(); ++i) meta[all[i]].idx = i;
      },
      [&] {
        for (auto& updates : allUpdates) {
          for (auto& update : updates) {
            auto u = boost::get<MethodUpdate>(&update);
            if (!u) continue;
            always_assert(
              index.uninstantiableClsMethRefs.emplace(
                u->name,
                std::move(u->methods)
              ).second
            );
          }
        }
      },
      [&] {
        for (auto& updates : allUpdates) {
          for (auto& update : updates) {
            if (auto const u = boost::get<ClassUpdate>(&update)) {
              if (!u->closureContext) {
                auto& meta = initTypesMeta.classes[u->name];
                meta.deps.insert(begin(u->typeUses), end(u->typeUses));
              } else {
                auto& meta = initTypesMeta.classes[u->closureContext];
                meta.deps.insert(begin(u->typeUses), end(u->typeUses));
                index.classToClosures[u->closureContext].emplace(u->name);
              }
            } else if (auto const u = boost::get<FuncUpdate>(&update)) {
              auto& meta = initTypesMeta.funcs[u->name];
              assertx(meta.deps.empty());
              meta.deps.insert(begin(u->typeUses), end(u->typeUses));
            }
          }
        }
      }
    );
  }

  return std::make_tuple(
    std::move(subclassMeta),
    std::move(initTypesMeta),
    [&] {
      std::vector<InterfaceConflicts> out;
      out.reserve(ifaceConflicts.size());
      for (auto& [_, c] : ifaceConflicts) out.emplace_back(std::move(c));
      return out;
    }()
  );
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
    ClassGraph::init();
  }
  static void fini() { ClassGraph::destroy(); }

  // Aggregated data for some group of classes. The data can either
  // come from a split node, or inferred from a group of classes.
  struct Data {
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
    TSStringSet mockedClasses;

    bool hasConstProp{false};
    bool hasReifiedGeneric{false};

    bool isSubMocked{false};

    // The meaning of these differ depending on whether the ClassInfo
    // contains just it's info, or all of it's subclass info.
    bool hasRegularClass{false};
    bool hasRegularClassFull{false};

    template <typename SerDe> void serde(SerDe& sd) {
      sd(methods, string_data_lt{})
        (propsWithImplicitNullable, string_data_lt{})
        (mockedClasses, string_data_lt_type{})
        (hasConstProp)
        (hasReifiedGeneric)
        (isSubMocked)
        (hasRegularClass)
        (hasRegularClassFull)
        ;
    }
  };

  // Split node. Used to wrap a Data when summarizing some subset of a
  // class' children.
  struct Split {
    Split() = default;
    Split(SString name, SString cls) : name{name}, cls{cls} {}

    SString name;
    SString cls;
    // ClassGraph for the class this split contributes to. NB: Since
    // this split only represents a subset of the class' children, it
    // should *not* hasCompleteChildren().
    ClassGraph classGraph;
    CompactVector<SString> children;
    Data data;

    template <typename SerDe> void serde(SerDe& sd) {
      ScopedStringDataIndexer _;
      sd(name)
        (cls)
        (classGraph, nullptr)
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
    std::vector<std::vector<SString>> regOnlyEquivCandidates;

    template <typename SerDe> void serde(SerDe& sd) {
      ScopedStringDataIndexer _;
      sd(funcFamilyDeps, std::less<FuncFamily2::Id>{})
        (newFuncFamilyIds)
        (nameOnly)
        (regOnlyEquivCandidates)
        ;
    }
  };
  using Output = Multi<
    Variadic<std::unique_ptr<ClassInfo2>>,
    Variadic<std::unique_ptr<Split>>,
    Variadic<std::unique_ptr<php::Class>>,
    Variadic<FuncFamilyGroup>,
    Variadic<std::unique_ptr<ClassInfo2>>,
    OutputMeta
  >;

  // Each job takes the list of classes and splits which should be
  // produced, dependency classes and splits (which are not updated),
  // edges between classes and splits, and func families (needed by
  // dependency classes). Leafs are like deps, except they'll be
  // considered as part of calculating the name-only func families
  // because its possible for them to be disjoint from classes.
  // (normal deps are not considered as their data is guaranteed to
  // be included by a class).
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

    if (debug) {
      for (auto const& cinfo : classes.vals) {
        always_assert(!cinfo->classGraph.isMissing());
        always_assert(!cinfo->classGraph.hasCompleteChildren());
        always_assert(!cinfo->classGraph.isConservative());
      }
      for (auto const& cinfo : leafs.vals) {
        always_assert(!cinfo->classGraph.isMissing());
        always_assert(!cinfo->classGraph.isConservative());
      }
      for (auto const& cinfo : deps.vals) {
        always_assert(!cinfo->classGraph.isMissing());
      }
    }

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
      assertx(split->children.empty());
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

    index.aggregateData.reserve(index.top.size());

    OutputMeta meta;

    // Mark all of the classes (including leafs) as being complete
    // since their subclass lists are correct.
    for (auto& cinfo : leafs.vals) {
      if (cinfo->classGraph.hasCompleteChildren()) continue;
      cinfo->classGraph.setComplete();
    }
    for (auto& cinfo : classes.vals) {
      if (cinfo->classGraph.hasCompleteChildren() ||
          cinfo->classGraph.isConservative()) {
        continue;
      }
      cinfo->classGraph.setComplete();
    }

    // Store the regular-only equivalent classes in the output
    // metadata. This will be used in the init-types pass.
    meta.regOnlyEquivCandidates.reserve(classes.vals.size());
    for (auto& cinfo : classes.vals) {
      meta.regOnlyEquivCandidates.emplace_back();
      auto& candidates = meta.regOnlyEquivCandidates.back();
      for (auto const g : cinfo->classGraph.candidateRegOnlyEquivs()) {
        candidates.emplace_back(g.name());
      }
    }

    // If there's no classes or splits, this job is doing nothing but
    // calculating name only func family entries (so should have at
    // least one leaf).
    if (!index.top.empty()) {
      build_children(index, edges.vals);
      process_roots(index, classes.vals, splits.vals);
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
      std::move(leafs),
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
    TSStringToOneT<ClassInfo2*> classInfos;
    // All splits, whether inputs or dependencies.
    TSStringToOneT<Split*> splits;
    // All php::Class, whether inputs or dependencies.
    TSStringToOneT<php::Class*> classes;

    // ClassInfos and splits which are inputs (IE, we want to
    // calculate data for).
    TSStringSet top;

    // Aggregated data for an input
    TSStringToOneT<Data> aggregateData;

    // Mapping of input ClassInfos/splits to all of their subclasses
    // present in this Job. Some of the children may be splits, which
    // means some subset of the children were processed in another
    // Job.
    TSStringToOneT<std::vector<SString>> children;

    // The leafs in this job. This isn't necessarily an actual leaf,
    // but one whose children haven't been provided in this job
    // (because they've already been processed). Classes which are
    // leafs have information in their ClassInfo2 which reflect all of
    // their subclasses, otherwise just their own information.
    TSStringSet leafs;

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
    TSStringToOneT<TSStringSet> children;
    // First record direct children. This can be inferred from the
    // parents of all present ClassInfos:

    // Everything starts out as a leaf.
    index.leafs.reserve(index.classInfos.size());
    for (auto const [name, _] : index.classInfos) {
      index.leafs.emplace(name);
    }

    auto const onParent = [&] (SString parent, const ClassInfo2* child) {
      // Due to how work is divided, a class might have parents not
      // present in this job. Ignore those.
      if (!index.classInfos.count(parent)) return;
      children[parent].emplace(child->name);
      // If you're a parent, you're not a leaf.
      index.leafs.erase(parent);
    };

    for (auto const [name, cinfo] : index.classInfos) {
      if (cinfo->parent) onParent(cinfo->parent, cinfo);
      for (auto const iface : cinfo->classGraph.declInterfaces()) {
        onParent(iface.name(), cinfo);
      }
      for (auto const trait : cinfo->classGraph.usedTraits()) {
        onParent(trait.name(), cinfo);
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
      children[edge.cls].emplace(edge.split);
    }

    // Every "top" ClassInfo also has itself as a subclass (this
    // matches the semantics of the subclass list and simplifies the
    // processing).
    for (auto const name : index.top) {
      if (auto const split = folly::get_default(index.splits, name, nullptr)) {
        // Copy the children list out of the split and add it to the
        // map.
        auto& c = children[name];
        for (auto const child : split->children) {
          assertx(index.classInfos.count(child) ||
                  index.splits.count(child));
          c.emplace(child);
        }
        split->children.clear();
      }
    }

    // Calculate the indegree for all children. The indegree for a given node
    // differs depending on the top used, so these are calculated separately.
    auto const getIndegree = [&](SString root) {
      TSStringSet visited;
      TSStringSet toExplore{root};
      TSStringSet toExploreNext;
      TSStringToOneT<uint32_t> indegree;

      while (!toExplore.empty()) {
        toExploreNext.clear();
        for (auto const child : toExplore) {
          if (visited.count(child)) continue;
          visited.emplace(child);
          auto const it = children.find(child);
          // May not exist in children if processed in earlier round.
          if (it == end(children)) continue;
          for (auto const c : it->second) {
            indegree[c]++;
            toExploreNext.emplace(c);
          }
        }
        std::swap(toExplore, toExploreNext);
      }
      return indegree;
    };

    // Topological sort the transitive children for each node.
    for (auto& [name, _] : children) {
      auto indegree = getIndegree(name);
      std::vector<SString> sorted{name};

      int sortedBegin = 0;
      int sortedEnd = sorted.size();

      while (sortedBegin != sortedEnd) {
        for (int i = sortedBegin; i < sortedEnd; i++) {
          auto const cls = sorted[i];
          auto const it = children.find(cls);
          if (it == end(children)) continue;
          for (auto const c : it->second) {
            indegree[c]--;
            if (indegree[c] == 0) sorted.emplace_back(c);
          }
        }
        sortedBegin = sortedEnd;
        sortedEnd = sorted.size();
      }
      assertx(indegree.size() + 1 == sorted.size());
      index.children[name] = std::move(sorted);
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
  static Data build_data(LocalIndex& index, SString clsname) {
    // Does this name represent a class?
    if (auto const cinfo = folly::get_default(index.classInfos, clsname)) {
      // It's a class. We need to build a Data from what's in the
      // ClassInfo. If the ClassInfo hasn't been processed already
      // (it's a leaf or its the first round), the data will reflect
      // just that class. However if the ClassInfo has been processed
      // (it's a dependencies and it's past the first round), it will
      // reflect any subclasses of that ClassInfo as well.
      Data data;

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
        for (auto const p : cinfo->classGraph.directParents()) {
          data.mockedClasses.emplace(p.name());
        }
      }
      data.isSubMocked = cinfo->isMocked || cinfo->isSubMocked;

      data.hasRegularClass = cinfo->isRegularClass;
      data.hasRegularClassFull =
        data.hasRegularClass || cinfo->classGraph.mightHaveRegularSubclass();
      if (!data.hasRegularClass && index.leafs.count(clsname)) {
        data.hasRegularClass = data.hasRegularClassFull;
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
    auto const split = folly::get_default(index.splits, clsname);
    always_assert(split != nullptr);
    assertx(split->children.empty());
    // Split already contains the Data, so nothing to do but return
    // it.
    return split->data;
  }

  static void update_data(Data& data, Data childData) {
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
          if (childData.hasRegularClassFull) {
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
    data.hasRegularClassFull |= childData.hasRegularClassFull;
  }

  // Obtain a Data for the given class/split named "top".
  // @param calculatedAcc: an accumulator passed in to track nodes we process
  // while processing children recursively
  static Data aggregate_data(LocalIndex& index,
                             SString top,
                             TSStringSet& calculatedAcc) {
    assertx(index.top.contains(top));

    auto const& children = [&]() -> const std::vector<SString>& {
      auto const it = index.children.find(top);
      always_assert(it != end(index.children));
      assertx(!it->second.empty());
      return it->second;
    }();

    auto const it = index.aggregateData.find(top);
    if (it != end(index.aggregateData)) {
      for (auto const child : children) calculatedAcc.emplace(child);
      return it->second;
    }

    Data data;
    auto first = true;
    // Set of children calculated for current top to ensure we don't duplicate work.
    TSStringSet calculatedForTop;

    // For each child of the class/split (for classes this includes
    // the top class itself), we create a Data, then union it together
    // with the rest.
    size_t childIdx = 0;
    while (calculatedForTop.size() < children.size()) {
      auto child = children[childIdx++];
      if (calculatedForTop.contains(child)) continue;
      // Top Splits have no associated data yet.
      if (index.top.count(child) && index.splits.count(child)) {
        calculatedForTop.emplace(child);
        continue;
      }

      auto childData = [&]() {
        if (index.top.contains(child) && !child->tsame(top)) {
          return aggregate_data(index, child, calculatedForTop);
        } else {
          calculatedForTop.emplace(child);
          return build_data(index, child);
        }
      }();

      // The first Data has nothing to union with, so just use it as is.
      if (first) {
        data = std::move(childData);
        first = false;
        continue;
      }
      update_data(data, std::move(childData));
    }

    for (auto const cls : calculatedForTop) calculatedAcc.emplace(cls);
    always_assert(index.aggregateData.emplace(top, data).second);
    return data;
  }

   // Obtain a Data for the given class/split named "top".
  static Data aggregate_data(LocalIndex& index, SString top) {
    TSStringSet calculated;
    return aggregate_data(index, top, calculated);
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

      // These are just copied directly from Data.
      cinfo->subHasConstProp = data.hasConstProp;
      cinfo->subHasReifiedGeneric = data.hasReifiedGeneric;

      auto& cls = index.cls(cinfo->name);

      // This class is mocked if its on the mocked classes list.
      cinfo->isMocked = (bool)data.mockedClasses.count(cinfo->name);
      cinfo->isSubMocked = data.isSubMocked || cinfo->isMocked;
      attribute_setter(cls.attrs, !cinfo->isSubMocked, AttrNoMock);

      // We can use whether we saw regular/non-regular subclasses to
      // infer if this class is overridden.
      if (cinfo->classGraph.mightHaveRegularSubclass()) {
        attribute_setter(cls.attrs, false, AttrNoOverrideRegular);
        attribute_setter(cls.attrs, false, AttrNoOverride);
      } else if (cinfo->classGraph.mightHaveNonRegularSubclass()) {
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
        [&] {
          if (cinfo->initialNoReifiedInit) return true;
          if (cinfo->parent) return false;
          if (cls.attrs & AttrInterface) return true;
          return !data.hasReifiedGeneric;
        }(),
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

            if (cinfo->isRegularClass ||
                cinfo->classGraph.mightHaveRegularSubclass()) {
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
                (cinfo->isRegularClass ||
                 cinfo->classGraph.mightHaveRegularSubclass())) {
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
      assertx(split->children.empty());
      split->data = aggregate_data(index, split->name);
      split->classGraph = ClassGraph::get(split->cls);
    }
  }
};

Job<BuildSubclassListJob> s_buildSubclassJob;

struct SubclassWork {
  TSStringToOneT<std::unique_ptr<BuildSubclassListJob::Split>> allSplits;
  struct Bucket {
    std::vector<SString> classes;
    std::vector<SString> deps;
    std::vector<SString> splits;
    std::vector<SString> splitDeps;
    std::vector<SString> leafs;
    std::vector<BuildSubclassListJob::EdgeToSplit> edges;
    size_t cost{0};
  };
  std::vector<std::vector<Bucket>> buckets;
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

// Dependency information for a class or split node.
struct DepData {
  // Transitive dependencies (children) for this class.
  TSStringSet deps;
  // Any split nodes which are dependencies of this class.
  TSStringSet edges;
  // The number of direct children of this class which will be
  // processed this round.
  size_t processChildren{0};
};


// Given a set of roots, greedily add roots and their children to buckets
// via DFS traversal.
template <typename GetDeps>
std::vector<HierarchicalWorkBucket>
dfs_bucketize(SubclassMetadata& subclassMeta,
              std::vector<SString> roots,
              TSStringToOneT<std::vector<SString>>& splitImmDeps,
              size_t kMaxBucketSize,
              size_t maxClassIdx,
              bool alwaysCreateNew,
              const TSStringSet& leafs,
              const TSStringSet& processed, // already processed
              const GetDeps& getDeps) {
  TSStringSet visited;
  std::vector<std::vector<SString>> rootsToProcess;
  rootsToProcess.emplace_back();
  std::vector<size_t> rootsCost;

  auto const depsSize = [&] (SString cls) {
    return getDeps(cls, getDeps).deps.size();
  };

  size_t cost = 0;

  auto finishBucket = [&]() {
    if (!cost) return;
    rootsToProcess.emplace_back();
    rootsCost.emplace_back(cost);
    cost = 0;
  };

  auto addRoot = [&](SString c) {
    rootsToProcess.back().emplace_back(c);
    cost += depsSize(c);
  };

  auto const processSubgraph = [&](SString cls) {
    assertx(!processed.count(cls));

    addRoot(cls);
    for (auto const& child : getDeps(cls, getDeps).deps) {
      if (processed.count(child)) continue;
      if (visited.count(child)) continue;
      visited.insert(child);
      // Leaves use special leaf-promotion logic in assign_hierarchial_work
      if (leafs.count(child)) continue;
      addRoot(child);
    }
    if (cost < kMaxBucketSize) return;
    finishBucket();
  };

  // Visit immediate children. Recurse until you find a node that has small
  // enough transitive deps.
  auto const visitSubgraph = [&](SString root, auto const& self) {
    if (processed.count(root) || visited.count(root)) return false;
    if (!depsSize(root)) return false;
    auto progress = false;
    visited.insert(root);

    assertx(IMPLIES(splitImmDeps.count(root), depsSize(root) <= kMaxBucketSize));
    if (depsSize(root) <= kMaxBucketSize) {
      processSubgraph(root);
      progress = true;
    } else {
      auto const immChildren = [&] {
        auto const it = subclassMeta.meta.find(root);
        if (it == end(subclassMeta.meta)) {
          auto const it2 = splitImmDeps.find(root);
          always_assert(it2 != end(splitImmDeps));
          return it2->second;
        }
        return it->second.children;
      }();
      for (auto const& child : immChildren) progress |= self(child, self);
    }
    return progress;
  };

  // Sort the roots to keep it deterministic
  std::sort(
    begin(roots), end(roots),
    [&] (SString& a, SString& b) {
      return getDeps(a, getDeps).deps.size() > getDeps(b, getDeps).deps.size();
    }
  );

  auto progress = false;
  for (auto const& i : roots) {
    assertx(depsSize(i)); // Should never be processing one leaf
    progress |= visitSubgraph(i, visitSubgraph);
  }
  assertx(progress);
  finishBucket();

  if (rootsToProcess.back().empty()) rootsToProcess.pop_back();

  auto buckets = parallel::gen(
    rootsToProcess.size(),
    [&] (size_t bucketIdx) {
      auto numBuckets = (rootsCost[bucketIdx] + (kMaxBucketSize/2)) / kMaxBucketSize;
      if (!numBuckets) numBuckets = 1;
      return consistently_bucketize_by_num_buckets(rootsToProcess[bucketIdx],
        alwaysCreateNew ? rootsToProcess[bucketIdx].size() : numBuckets);
    }
  );

  std::vector<std::vector<SString>> flattened;
  for (auto& b : buckets) {
    flattened.insert(flattened.end(), b.begin(), b.end());
  }

  auto work = build_hierarchical_work(
    flattened,
    maxClassIdx,
    [&] (SString c) {
      auto const& deps = getDeps(c, getDeps).deps;
      return std::make_pair(&deps, true);
    },
    [&] (SString c) -> Optional<size_t> {
      if (!leafs.count(c)) return std::nullopt;
      return subclassMeta.meta.at(c).idx;
    }
  );
  return work;
}

// For each round:
// While toProcess is not empty:
// 1. Find transitive dep counts
// 2. For each class, calculate splits, find roots, find rootLeafs
// 3. For rootLeafs, consistently hash to make buckets
// 4. For roots, assign subgraphs to buckets via greedy DFS. If buckets get too big,
//    split them via consistent hashing.
SubclassWork build_subclass_lists_assign(SubclassMetadata subclassMeta) {
  trace_time trace{"build subclass lists assign"};
  trace.ignore_client_stats();

  constexpr size_t kBucketSize = 2000;
  constexpr size_t kMaxBucketSize = 25000;

  SubclassWork out;

  auto const maxClassIdx = subclassMeta.all.size();

  // A processed class/split is considered processed once it's
  // assigned to a bucket in a round. Once considered processed, it
  // will have no dependencies.
  TSStringSet processed;

  TSStringToOneT<std::unique_ptr<DepData>> splitDeps;
  TSStringToOneT<std::unique_ptr<BuildSubclassListJob::Split>> splitPtrs;
  TSStringSet leafs;
  TSStringToOneT<std::vector<SString>> splitImmDeps;

  // Keep creating rounds until all of the classes are assigned to a
  // bucket in a round.
  auto toProcess = std::move(subclassMeta.all);
  TSStringSet tp;
  if (debug) tp.insert(toProcess.begin(), toProcess.end());

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

    // This class will be processed this round and is a root.
    struct Root { SString cls; };
    struct RootLeaf { SString cls; };
    struct Child { SString cls; };
    // This class' children should be split. The class' child list
    // will be replaced with the new child list and splits created.
    struct Split {
      SString cls;
      std::vector<SString> children;
      struct Data {
        SString name;
        std::unique_ptr<DepData> deps;
        std::unique_ptr<BuildSubclassListJob::Split> ptr;
        std::vector<SString> children;
      };
      std::vector<Data> splits;
    };
    using Action = boost::variant<Root, Split, Child, RootLeaf>;

    auto const actions = parallel::map(
      toProcess,
      [&] (SString cls) {
        auto const& meta = subclassMeta.meta.at(cls);

        if (!willSplitChildren(cls)) {
          if (!meta.parents.empty()) return Action{ Child{cls} };
          if (meta.children.empty()) return Action{ RootLeaf{cls} };
          return Action{ Root{cls} };
        }

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
            [&] (SString child) -> const TSStringSet& {
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
            auto split = std::make_unique<BuildSubclassListJob::Split>(name, cls);
            std::vector<SString> children;

            for (auto const child : buckets[i]) {
              split->children.emplace_back(child);
              children.emplace_back(child);
              auto const& childDeps = findDeps(child, findDeps).deps;
              deps->deps.insert(begin(childDeps), end(childDeps));
              deps->deps.emplace(child);
            }

            std::sort(
              begin(split->children),
              end(split->children),
              string_data_lt_type{}
            );

            splits.emplace_back(
              Split::Data{name, std::move(deps), std::move(split), std::move(children)}
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
        for (auto const& [name, _, _2, _3] : split.splits) {
          split.children.emplace_back(name);
        }

        return Action{ std::move(split) };
      }
    );

    assertx(actions.size() == toProcess.size());
    std::vector<SString> roots;
    roots.reserve(actions.size());
    std::vector<SString> rootLeafs;

    for (auto const& action : actions) {
      match<void>(
        action,
        [&] (Root r) {
          assertx(!subclassMeta.meta.at(r.cls).children.empty());
          roots.emplace_back(r.cls);
        },
        [&] (RootLeaf r) {
          assertx(subclassMeta.meta.at(r.cls).children.empty());
          rootLeafs.emplace_back(r.cls);
          leafs.emplace(r.cls);
        },
        [&] (Child n) {
          auto const& meta = subclassMeta.meta.at(n.cls);
          if (meta.children.empty()) leafs.emplace(n.cls);
        },
        [&] (const Split& s) {
          auto& meta = subclassMeta.meta.at(s.cls);
          meta.children = s.children;
          if (meta.parents.empty()) {
            roots.emplace_back(s.cls);
          }
          auto& splits = const_cast<std::vector<Split::Data>&>(s.splits);
          for (auto& [name, deps, ptr, children] : splits) {
            splitImmDeps.emplace(name, children);
            roots.emplace_back(name);
            splitDeps.emplace(name, std::move(deps));
            splitPtrs.emplace(name, std::move(ptr));
          }
        }
      );
    }

    auto work = dfs_bucketize(subclassMeta,
                              roots,
                              splitImmDeps,
                              kMaxBucketSize,
                              maxClassIdx,
                              round > 0,
                              leafs,
                              processed,
                              findDeps);

    // Bucketize root leafs.
    // These are cheaper since we will only be calculating
    // name-only func family entries.
    auto rootLeafBuckets = consistently_bucketize(rootLeafs, kMaxBucketSize);
    auto rootLeafWork = parallel::gen(
      rootLeafBuckets.size(),
      [&] (size_t idx) {
        return HierarchicalWorkBucket{
          std::move(rootLeafBuckets[idx])
        };
      }
    );

    work.insert(work.end(), rootLeafWork.begin(), rootLeafWork.end());

    std::vector<SString> markProcessed;
    markProcessed.reserve(actions.size());

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
      assertx(w.uninstantiable.empty());
      out.buckets.back().emplace_back();
      auto& bucket = out.buckets.back().back();
      // Separate out any of the "roots" which are actually leafs.
      for (auto const cls : w.classes) {
        bucket.cost += depsSize(cls);
        markProcessed.emplace_back(cls);
        if (leafs.count(cls)) {
          leafs.erase(cls);
          bucket.leafs.emplace_back(cls);
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
          if (string_data_lt_type{}(a.cls, b.cls)) return true;
          if (string_data_lt_type{}(b.cls, a.cls)) return false;
          return string_data_lt_type{}(a.split, b.split);
        }
      );
      std::sort(begin(bucket.leafs), end(bucket.leafs), string_data_lt_type{});
    }

    std::sort(
      begin(out.buckets.back()), end(out.buckets.back()),
      [] (const SubclassWork::Bucket& a,
          const SubclassWork::Bucket& b) {
            return a.cost > b.cost;
      }
    );

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

  // Ensure we create an output for everything exactly once
  if (debug) {
    for (size_t round = 0; round < out.buckets.size(); ++round) {
      auto const& r = out.buckets[round];
      for (size_t i = 0; i < r.size(); ++i) {
        auto const& bucket = r[i];
        for (auto const c : bucket.classes) always_assert(tp.erase(c));
        for (auto const l : bucket.leafs) always_assert(tp.erase(l));
      }
    }
    assertx(tp.empty());
  }

  if (Trace::moduleEnabled(Trace::hhbbc_index, 4)) {
    for (size_t round = 0; round < out.buckets.size(); ++round) {
      size_t nc = 0;
      size_t ns = 0;
      size_t nd = 0;
      size_t nsd = 0;
      size_t nl = 0;

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
        for (auto const DEBUG_ONLY c : bucket.leafs) FTRACE(6, "    {}\n", c);
        FTRACE(5, "  edges ({}):\n", bucket.edges.size());
        for (DEBUG_ONLY auto const& e : bucket.edges) {
          FTRACE(6, "    {} -> {}\n", e.cls, e.split);
        }
        nc += bucket.classes.size();
        ns += bucket.splits.size();
        nd += bucket.deps.size();
        nsd += bucket.splitDeps.size();
        nl += bucket.leafs.size();
      }
      FTRACE(4, "BSL round #{} stats\n"
        " {} buckets\n"
        " {} classes\n"
        " {} splits\n"
        " {} deps\n"
        " {} split deps\n"
        " {} leafs\n",
        round, r.size(), nc, ns, nd, nsd, nl
      );
    }
  }

  return out;
}

void build_subclass_lists(IndexData& index,
                          SubclassMetadata meta,
                          InitTypesMetadata& initTypesMeta) {
  trace_time tracer{"build subclass lists", index.sample};
  tracer.ignore_client_stats();

  using namespace folly::gen;

  // Mapping of splits to their Ref. We only upload a split when we're
  // going to run a job which it is part of the output.
  TSStringToOneT<UniquePtrRef<BuildSubclassListJob::Split>> splitsToRefs;

  FSStringToOneT<hphp_fast_set<FuncFamily2::Id>> funcFamilyDeps;

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
    std::vector<std::pair<SString, UniquePtrRef<ClassInfo2>>> leafs;
    std::vector<std::pair<SString, FuncFamilyEntry>> nameOnly;
    std::vector<std::pair<SString, SString>> candidateRegOnlyEquivs;
  };

  auto const run = [&] (SubclassWork::Bucket bucket, size_t round)
    -> coro::Task<Updates> {
    co_await coro::co_reschedule_on_current_executor;

    if (bucket.classes.empty() &&
        bucket.splits.empty() &&
        bucket.leafs.empty()) {
      assertx(bucket.splitDeps.empty());
      co_return Updates{};
    }

    // We shouldn't get closures or Closure in any of this.
    if (debug) {
      for (auto const c : bucket.classes) {
        always_assert(!c->tsame(s_Closure.get()));
        always_assert(!is_closure_name(c));
      }
      for (auto const c : bucket.deps) {
        always_assert(!c->tsame(s_Closure.get()));
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
    auto [splitRefs, edges, config] = co_await coro::collectAll(
      index.client->storeMulti(std::move(splits)),
      index.client->storeMulti(std::move(bucket.edges)),
      index.configRef->getCopy()
    );

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

    auto results = co_await
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
      );
    // Every job is a single work-unit, so we should only ever get one
    // result for each one.
    assertx(results.size() == 1);
    auto& [cinfoRefs, outSplitRefs, clsRefs, ffRefs, leafRefs, outMetaRef]
      = results[0];
    assertx(cinfoRefs.size() == bucket.classes.size());
    assertx(outSplitRefs.size() == bucket.splits.size());
    assertx(clsRefs.size() == bucket.classes.size());
    assertx(leafRefs.size() == bucket.leafs.size());

    auto outMeta = co_await index.client->load(std::move(outMetaRef));
    assertx(outMeta.newFuncFamilyIds.size() == ffRefs.size());
    assertx(outMeta.funcFamilyDeps.size() == cinfoRefs.size());
    assertx(outMeta.regOnlyEquivCandidates.size() == cinfoRefs.size());

    Updates updates;
    updates.classes.reserve(bucket.classes.size());
    updates.splits.reserve(bucket.splits.size());
    updates.funcFamilies.reserve(outMeta.newFuncFamilyIds.size());
    updates.funcFamilyDeps.reserve(outMeta.funcFamilyDeps.size());
    updates.nameOnly.reserve(outMeta.nameOnly.size());
    updates.leafs.reserve(bucket.leafs.size());

    for (size_t i = 0, size = bucket.classes.size(); i < size; ++i) {
      updates.classes.emplace_back(bucket.classes[i], cinfoRefs[i], clsRefs[i]);
    }
    for (size_t i = 0, size = bucket.splits.size(); i < size; ++i) {
      updates.splits.emplace_back(bucket.splits[i], outSplitRefs[i]);
    }
    for (size_t i = 0, size = bucket.leafs.size(); i < size; ++i) {
      updates.leafs.emplace_back(bucket.leafs[i], leafRefs[i]);
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
    for (size_t i = 0, size = outMeta.regOnlyEquivCandidates.size();
         i < size; ++i) {
      auto const name = bucket.classes[i];
      for (auto const c : outMeta.regOnlyEquivCandidates[i]) {
        updates.candidateRegOnlyEquivs.emplace_back(name, c);
      }
    }

    co_return updates;
  };

  {
    trace_time tracer2{"build subclass lists work", index.sample};

    for (size_t roundNum = 0; roundNum < work.buckets.size(); ++roundNum) {
      auto& round = work.buckets[roundNum];
      // In each round, run all of the work for each bucket
      // simultaneously, gathering up updates from each job.
      auto const updates = coro::blockingWait(coro::collectAllRange(
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
          for (auto const& [name, cinfo] : u.leafs) {
            index.classInfoRefs.at(name) = cinfo;
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
              initTypesMeta.nameOnlyFF[name].emplace_back(std::move(entry));
            }
            for (auto [name, candidate] : u.candidateRegOnlyEquivs) {
              initTypesMeta.classes[name]
                .candidateRegOnlyEquivs.emplace(candidate);
            }
          }
        }
      );
    }
  }

  splitsToRefs.clear();
  funcFamilyDeps.clear();
  work.buckets.clear();
  work.allSplits.clear();
}

//////////////////////////////////////////////////////////////////////

/*
 * Initialize the return-types of functions and methods from their
 * type-hints. Also set AttrInitialSatisfiesTC on properties if
 * appropriate (which must be done after types are initialized).
 */
struct InitTypesJob {
  static std::string name() { return "hhbbc-init-types"; }
  static void init(const Config& config) {
    process_init(config.o, config.gd, false);
    ClassGraph::init();
  }
  static void fini() { ClassGraph::destroy(); }

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
                    Variadic<std::unique_ptr<ClassInfo2>> cinfoDeps) {
    LocalIndex index;

    for (auto const& cls : classes.vals) {
      always_assert(index.classes.emplace(cls->name, cls.get()).second);
    }

    // All of the classes which might be a regular only equivalent
    // have been provided to the job. So, we can now definitely set
    // the regular only equivalent (if necessary). We need to do this
    // before setting the initial types because we need that
    // information to canonicalize.
    for (auto const& cinfo : cinfos.vals) {
      always_assert(index.classInfos.emplace(cinfo->name, cinfo.get()).second);
      cinfo->classGraph.setRegOnlyEquivs();
      // If this is a regular class, we don't need the "expanded"
      // method family information anymore, so clear it here to save
      // memory.
      if (cinfo->isRegularClass) {
        folly::erase_if(
          cinfo->methodFamilies,
          [&] (auto const& e) { return !cinfo->methods.count(e.first); }
        );
      }
    }
    for (auto const& cinfo : cinfoDeps.vals) {
      always_assert(index.classInfos.emplace(cinfo->name, cinfo.get()).second);
      cinfo->classGraph.setRegOnlyEquivs();
    }

    assertx(classes.vals.size() == cinfos.vals.size());
    for (size_t i = 0, size = classes.vals.size(); i < size; ++i) {
      auto& cls = classes.vals[i];
      auto& cinfo = cinfos.vals[i];
      assertx(cls->name->tsame(cinfo->name));
      assertx(cinfo->funcInfos.size() == cls->methods.size());

      unresolve_missing(index, *cls);
      set_bad_initial_prop_values(index, *cls, *cinfo);
      for (size_t j = 0, size = cls->methods.size(); j < size; ++j) {
        auto const& func = cls->methods[j];
        auto& finfo = cinfo->funcInfos[j];
        assertx(func->name == finfo->name);
        assertx(finfo->returnTy.is(BInitCell));
        finfo->returnTy = initial_return_type(index, *func);
      }
    }

    assertx(funcs.vals.size() == finfos.vals.size());
    for (size_t i = 0, size = funcs.vals.size(); i < size; ++i) {
      auto const& func = funcs.vals[i];
      auto& finfo = finfos.vals[i];
      assertx(func->name == finfo->name);
      assertx(finfo->returnTy.is(BInitCell));
      unresolve_missing(index, *func);
      finfo->returnTy = initial_return_type(index, *func);
    }

    return std::make_tuple(
      std::move(classes),
      std::move(cinfos),
      std::move(funcs),
      std::move(finfos)
    );
  }

private:

  struct LocalIndex {
    TSStringToOneT<const ClassInfo2*> classInfos;
    TSStringToOneT<const php::Class*> classes;
  };

  static void unresolve_missing(const LocalIndex& index, TypeConstraint& tc) {
    if (!tc.isSubObject()) return;
    auto const name = tc.clsName();
    if (index.classInfos.count(name)) return;
    FTRACE(
      4, "Unresolving type-constraint for '{}' because it does not exist\n",
      name
    );
    tc.unresolve();
  }

  static void unresolve_missing(const LocalIndex& index, php::Func& func) {
    for (auto& p : func.params) {
      unresolve_missing(index, p.typeConstraint);
      for (auto& ub : p.upperBounds.m_constraints) unresolve_missing(index, ub);
    }
    unresolve_missing(index, func.retTypeConstraint);
    for (auto& ub : func.returnUBs.m_constraints) unresolve_missing(index, ub);
  }

  static void unresolve_missing(const LocalIndex& index, php::Class& cls) {
    if (cls.attrs & AttrEnum) unresolve_missing(index, cls.enumBaseTy);
    for (auto& meth : cls.methods) unresolve_missing(index, *meth);
    for (auto& prop : cls.properties) {
      unresolve_missing(index, prop.typeConstraint);
      for (auto& ub : prop.ubs.m_constraints) unresolve_missing(index, ub);
    }
  }

  static Type initial_return_type(const LocalIndex& index, const php::Func& f) {
    auto const ty = [&] {
      // Return type of native functions is calculated differently.
      if (f.isNative) return native_function_return_type(&f);

      if ((f.attrs & AttrBuiltin) || f.isMemoizeWrapper) return TInitCell;

      if (f.isGenerator) {
        if (f.isAsync) {
          // Async generators always return AsyncGenerator object.
          return objExact(res::Class::get(s_AsyncGenerator.get()));
        }
        // Non-async generators always return Generator object.
        return objExact(res::Class::get(s_Generator.get()));
      }

      auto const make_type = [&] (const TypeConstraint& tc) {
        auto lookup = type_from_constraint(
          tc,
          TInitCell,
          [&] (SString name) -> Optional<res::Class> {
            if (auto const ci = folly::get_default(index.classInfos, name)) {
              auto const c = res::Class::get(*ci);
              assertx(c.isComplete());
              return c;
            }
            return std::nullopt;
          },
          [&] () -> Optional<Type> {
            if (!f.cls) return std::nullopt;
            auto const& cls = [&] () -> const php::Class& {
              if (!f.cls->closureContextCls) return *f.cls;
              auto const c =
                folly::get_default(index.classes, f.cls->closureContextCls);
              always_assert_flog(
                c,
                "When processing return-type for {}, "
                "tried to access missing class {}",
                func_fullname(f),
                f.cls->closureContextCls
              );
              return *c;
            }();
            if (cls.attrs & AttrTrait) return std::nullopt;
            auto const c = res::Class::get(cls.name);
            assertx(c.isComplete());
            return subCls(c);
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
        return wait_handle(std::move(ret));
      }
      return ret;
    }();

    FTRACE(3, "Initial return type for {}: {}\n",
           func_fullname(f), show(ty));
    return ty;
  }

  static void set_bad_initial_prop_values(const LocalIndex& index,
                                          php::Class& cls,
                                          ClassInfo2& cinfo) {
    assertx(cinfo.hasBadInitialPropValues);

    auto const isClosure = is_closure(cls);

    cinfo.hasBadInitialPropValues = false;
    for (auto& prop : cls.properties) {
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
            initial,
            [&] (SString name) -> Optional<res::Class> {
              if (auto const ci = folly::get_default(index.classInfos, name)) {
                auto const c = res::Class::get(*ci);
                assertx(c.isComplete());
                return c;
              }
              return std::nullopt;
            },
            [&] () -> Optional<Type> {
              auto const& ctx = [&] () -> const php::Class& {
                if (!cls.closureContextCls) return cls;
                auto const c =
                  folly::get_default(index.classes, cls.closureContextCls);
                always_assert_flog(
                  c,
                  "When processing bad initial prop values for {}, "
                  "tried to access missing class {}",
                  cls.name,
                  cls.closureContextCls
                );
                return *c;
              }();
              if (ctx.attrs & AttrTrait) return std::nullopt;
              auto const c = res::Class::get(ctx.name);
              assertx(c.isComplete());
              return subCls(c);
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
  static std::string name() { return "hhbbc-unit-fixup"; }
  static void init(const Config& config) {
    process_init(config.o, config.gd, false);
  }
  static void fini() {}

  static std::unique_ptr<php::Unit> run(std::unique_ptr<php::Unit> unit,
                                        const InitTypesMetadata::Fixup& fixup) {
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
 * BuildSubclassListJob produces name-only func family entries. This
 * job merges entries for the same name into one.
 */
struct AggregateNameOnlyJob: public BuildSubclassListJob {
  static std::string name() { return "hhbbc-aggregate-name-only"; }

  struct OutputMeta {
    std::vector<std::vector<FuncFamily2::Id>> newFuncFamilyIds;
    std::vector<FuncFamilyEntry> nameOnly;
    template <typename SerDe> void serde(SerDe& sd) {
      ScopedStringDataIndexer _;
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

Job<InitTypesJob> s_initTypesJob;
Job<UnitFixupJob> s_unitFixupJob;
Job<AggregateNameOnlyJob> s_aggregateNameOnlyJob;

// Initialize return-types, fixup units, and aggregate name-only
// func-families all at once.
void init_types(IndexData& index, InitTypesMetadata meta) {
  trace_time tracer{"init types", index.sample};

  constexpr size_t kTypesBucketSize = 2000;
  constexpr size_t kFixupsBucketSize = 3000;
  constexpr size_t kAggregateBucketSize = 3000;

  auto typeBuckets = consistently_bucketize(
    [&] {
      // Temporarily suppress case collision logging
      auto oldLogLevel = RuntimeOption::EvalLogTsameCollisions;
      RuntimeOption::EvalLogTsameCollisions = 0;
      SCOPE_EXIT { RuntimeOption::EvalLogTsameCollisions = oldLogLevel; };

      std::vector<SString> roots;
      roots.reserve(meta.classes.size() + meta.funcs.size());
      for (auto const& [name, _] : meta.classes) {
        roots.emplace_back(name);
      }
      for (auto const& [name, _] : meta.funcs) {
        // A class and a func could have the same name. Avoid
        // duplicates. If we do have a name collision it just means
        // the func and class will be assigned to the same bucket.
        if (meta.classes.count(name)) continue;
        roots.emplace_back(name);
      }
      return roots;
    }(),
    kTypesBucketSize
  );

  auto fixupBuckets = consistently_bucketize(
    [&] {
      std::vector<SString> sorted;
      sorted.reserve(meta.fixups.size());
      for (auto& [unit, _] : meta.fixups) sorted.emplace_back(unit);
      std::sort(sorted.begin(), sorted.end(), string_data_lt{});
      return sorted;
    }(),
    kFixupsBucketSize
  );

  auto aggregateBuckets = consistently_bucketize(
    [&] {
      std::vector<SString> sorted;
      sorted.reserve(meta.nameOnlyFF.size());
      for (auto const& [name, entries] : meta.nameOnlyFF) {
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
    kAggregateBucketSize
  );

  // We want to avoid updating any Index data-structures until after
  // all jobs have read their inputs. We use the latch to block tasks
  // until all tasks have passed the point of reading their inputs.
  CoroLatch typesLatch{typeBuckets.size()};

  auto const runTypes = [&] (std::vector<SString> work) -> coro::Task<void> {
    co_await coro::co_reschedule_on_current_executor;

    if (work.empty()) {
      typesLatch.count_down();
      co_return;
    }

    std::vector<UniquePtrRef<php::Class>> classes;
    std::vector<UniquePtrRef<ClassInfo2>> cinfos;
    std::vector<UniquePtrRef<php::Func>> funcs;
    std::vector<UniquePtrRef<FuncInfo2>> finfos;
    std::vector<UniquePtrRef<ClassInfo2>> cinfoDeps;

    TSStringSet roots;
    std::vector<SString> classNames;
    std::vector<SString> funcNames;

    roots.reserve(work.size());
    classNames.reserve(work.size());

    for (auto const w : work) {
      if (meta.classes.count(w)) {
        always_assert(roots.emplace(w).second);
        classNames.emplace_back(w);
        if (auto const closures = folly::get_ptr(index.classToClosures, w)) {
          for (auto const clo : *closures) {
            always_assert(roots.emplace(clo).second);
            classNames.emplace_back(clo);
          }
        }
      }
      if (meta.funcs.count(w)) funcNames.emplace_back(w);
    }

    // Add a dependency to the job. A class is a dependency if it
    // shows up in a class' type-hints, or if it's a potential
    // reg-only equivalent.
    auto const addDep = [&] (SString dep, bool addEquiv) {
      if (!meta.classes.count(dep) || roots.count(dep)) return;
      cinfoDeps.emplace_back(index.classInfoRefs.at(dep));
      if (!addEquiv) return;
      if (auto const cls = folly::get_ptr(meta.classes, dep)) {
        for (auto const d : cls->candidateRegOnlyEquivs) {
          if (!meta.classes.count(d) || roots.count(d)) continue;
          cinfoDeps.emplace_back(index.classInfoRefs.at(d));
        }
      }
    };

    for (auto const w : work) {
      if (auto const cls = folly::get_ptr(meta.classes, w)) {
        classes.emplace_back(index.classRefs.at(w));
        cinfos.emplace_back(index.classInfoRefs.at(w));
        if (auto const closures = folly::get_ptr(index.classToClosures, w)) {
          for (auto const clo : *closures) {
            classes.emplace_back(index.classRefs.at(clo));
            cinfos.emplace_back(index.classInfoRefs.at(clo));
          }
        }
        for (auto const d : cls->deps) addDep(d, true);
        for (auto const d : cls->candidateRegOnlyEquivs) addDep(d, false);
      }
      // Not else if. A name can correspond to both a class and a
      // func.
      if (auto const func = folly::get_ptr(meta.funcs, w)) {
        funcs.emplace_back(index.funcRefs.at(w));
        finfos.emplace_back(index.funcInfoRefs.at(w));
        for (auto const d : func->deps) addDep(d, true);
      }
    }
    addDep(s_Awaitable.get(), true);
    addDep(s_AsyncGenerator.get(), true);
    addDep(s_Generator.get(), true);

    // Record that we've read our inputs
    typesLatch.count_down();

    std::sort(begin(cinfoDeps), end(cinfoDeps));
    cinfoDeps.erase(
      std::unique(begin(cinfoDeps), end(cinfoDeps)),
      end(cinfoDeps)
    );

    auto config = co_await index.configRef->getCopy();

    Client::ExecMetadata metadata{
      .job_key = folly::sformat("init types {}", work[0])
    };

    auto results = co_await
      index.client->exec(
        s_initTypesJob,
        std::move(config),
        singleton_vec(
          std::make_tuple(
            std::move(classes),
            std::move(cinfos),
            std::move(funcs),
            std::move(finfos),
            std::move(cinfoDeps)
          )
        ),
        std::move(metadata)
      );
    assertx(results.size() == 1);
    auto& [classRefs, cinfoRefs, funcRefs, finfoRefs] = results[0];
    assertx(classRefs.size() == classNames.size());
    assertx(cinfoRefs.size() == classNames.size());
    assertx(funcRefs.size() == funcNames.size());
    assertx(finfoRefs.size() == funcNames.size());

    // Wait for all tasks to finish reading from the Index ref tables
    // before starting to overwrite them.
    co_await typesLatch.wait();

    for (size_t i = 0, size = classNames.size(); i < size; ++i) {
      auto const name = classNames[i];
      index.classRefs.at(name) = std::move(classRefs[i]);
      index.classInfoRefs.at(name) = std::move(cinfoRefs[i]);
    }
    for (size_t i = 0, size = funcNames.size(); i < size; ++i) {
      auto const name = funcNames[i];
      index.funcRefs.at(name) = std::move(funcRefs[i]);
      index.funcInfoRefs.at(name) = std::move(finfoRefs[i]);
    }

    co_return;
  };

  auto const runFixups = [&] (std::vector<SString> units) -> coro::Task<void> {
    co_await coro::co_reschedule_on_current_executor;

    if (units.empty()) co_return;

    std::vector<InitTypesMetadata::Fixup> fixups;

    // Gather up the fixups and ensure a deterministic ordering.
    fixups.reserve(units.size());
    for (auto const unit : units) {
      auto f = std::move(meta.fixups.at(unit));
      assertx(!f.addClass.empty() || !f.removeFunc.empty());
      std::sort(f.addClass.begin(), f.addClass.end(), string_data_lt_type{});
      std::sort(f.removeFunc.begin(), f.removeFunc.end(), string_data_lt{});
      fixups.emplace_back(std::move(f));
    }
    auto fixupRefs = co_await index.client->storeMulti(std::move(fixups));
    assertx(fixupRefs.size() == units.size());

    std::vector<
      std::tuple<UniquePtrRef<php::Unit>, Ref<InitTypesMetadata::Fixup>>
    > inputs;
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

    auto config = co_await index.configRef->getCopy();
    auto outputs = co_await index.client->exec(
      s_unitFixupJob,
      std::move(config),
      std::move(inputs),
      std::move(metadata)
    );
    assertx(outputs.size() == units.size());

    // Every unit is already in the Index table, so we can overwrite
    // them without locking.
    for (size_t i = 0, size = units.size(); i < size; ++i) {
      index.unitRefs.at(units[i]) = std::move(outputs[i]);
    }

    co_return;
  };

  struct AggregateUpdates {
    std::vector<
      std::pair<FuncFamily2::Id, Ref<FuncFamilyGroup>>
    > funcFamilies;
  };

  auto const runAggregate = [&] (std::vector<SString> names)
    -> coro::Task<AggregateUpdates> {
    co_await coro::co_reschedule_on_current_executor;

    if (names.empty()) co_return AggregateUpdates{};

    std::vector<std::pair<SString, std::vector<FuncFamilyEntry>>> entries;
    std::vector<Ref<FuncFamilyGroup>> funcFamilies;

    entries.reserve(names.size());
    // Extract out any func families the entries refer to, so they can
    // be provided to the job.
    for (auto const n : names) {
      auto& e = meta.nameOnlyFF.at(n);
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

    auto [entriesRef, config] = co_await coro::collectAll(
      index.client->store(std::move(entries)),
      index.configRef->getCopy()
    );

    Client::ExecMetadata metadata{
      .job_key = folly::sformat("aggregate name-only {}", names[0])
    };

    auto results = co_await
      index.client->exec(
        s_aggregateNameOnlyJob,
        std::move(config),
        singleton_vec(
          std::make_tuple(std::move(entriesRef), std::move(funcFamilies))
        ),
        std::move(metadata)
      );
    assertx(results.size() == 1);
    auto& [ffRefs, outMetaRef] = results[0];

    auto outMeta = co_await index.client->load(std::move(outMetaRef));
    assertx(outMeta.newFuncFamilyIds.size() == ffRefs.size());
    assertx(outMeta.nameOnly.size() == names.size());

    // Update the dummy entries with the actual result.
    for (size_t i = 0, size = names.size(); i < size; ++i) {
      auto& old = index.nameOnlyMethodFamilies.at(names[i]);
      assertx(boost::get<FuncFamilyEntry::None>(&old.m_meths));
      old = std::move(outMeta.nameOnly[i]);
    }

    AggregateUpdates updates;
    updates.funcFamilies.reserve(outMeta.newFuncFamilyIds.size());
    for (size_t i = 0, size = outMeta.newFuncFamilyIds.size(); i < size; ++i) {
      auto const ref = ffRefs[i];
      for (auto const& id : outMeta.newFuncFamilyIds[i]) {
        updates.funcFamilies.emplace_back(id, ref);
      }
    }

    co_return updates;
  };

  auto const runAggregateCombine = [&] (auto tasks) -> coro::Task<void> {
    co_await coro::co_reschedule_on_current_executor;

    auto const updates = co_await coro::collectAllRange(std::move(tasks));

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

    co_return;
  };

  using namespace folly::gen;

  std::vector<coro::TaskWithExecutor<void>> tasks;
  tasks.reserve(typeBuckets.size() + fixupBuckets.size() + 1);

  // Temporarily suppress case collision logging
  auto oldTypeLogLevel = RuntimeOption::EvalLogTsameCollisions;
  auto oldFuncLogLevel = RuntimeOption::EvalLogFsameCollisions;
  RuntimeOption::EvalLogTsameCollisions = 0;
  RuntimeOption::EvalLogFsameCollisions = 0;
  SCOPE_EXIT {
    RuntimeOption::EvalLogTsameCollisions = oldTypeLogLevel;
    RuntimeOption::EvalLogFsameCollisions = oldFuncLogLevel;
  };

  for (auto& work : typeBuckets) {
    tasks.emplace_back(
      runTypes(std::move(work)).scheduleOn(index.executor->sticky())
    );
  }
  for (auto& work : fixupBuckets) {
    tasks.emplace_back(
      runFixups(std::move(work)).scheduleOn(index.executor->sticky())
    );
  }
  auto subTasks = from(aggregateBuckets)
    | move
    | map([&] (std::vector<SString>&& work) {
        return runAggregate(
          std::move(work)
        ).scheduleOn(index.executor->sticky());
      })
    | as<std::vector>();
  tasks.emplace_back(
    runAggregateCombine(
      std::move(subTasks)
    ).scheduleOn(index.executor->sticky())
  );

  coro::blockingWait(coro::collectAllRange(std::move(tasks)));
}

//////////////////////////////////////////////////////////////////////

Index::Input::UnitMeta make_native_unit_meta(IndexData& index) {
  auto unit = make_native_unit();
  auto const name = unit->filename;

  std::vector<std::pair<SString, bool>> constants;
  constants.reserve(unit->constants.size());
  for (auto const& cns : unit->constants) {
    constants.emplace_back(cns->name, type(cns->val) == KindOfUninit);
  }

  auto unitRef = coro::blockingWait(index.client->store(std::move(unit)));
  Index::Input::UnitMeta meta{ std::move(unitRef), name };
  meta.constants = std::move(constants);
  return meta;
}

// Set up the async state, populate the (initial) table of
// extern-worker refs in the Index, and build some metadata needed for
// class flattening.
IndexFlattenMetadata make_remote(IndexData& index,
                                 Config config,
                                 Index::Input input,
                                 std::unique_ptr<TicketExecutor> executor,
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
  index.configRef = std::make_unique<CoroAsyncValue<Ref<Config>>>(
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
      auto const isTypeAlias = typeMapping.isTypeAlias;
      always_assert_flog(
        flattenMeta.typeMappings.emplace(name, std::move(typeMapping)).second,
        "Duplicate type-mapping: {}",
        name
      );
      if (isTypeAlias) {
        always_assert(index.typeAliasToUnit.emplace(name, unit.name).second);
      }
    }

    always_assert_flog(
      index.unitRefs.emplace(unit.name, std::move(unit.unit)).second,
      "Duplicate unit: {}",
      unit.name
    );

    for (auto const& [cnsName, hasInit] : unit.constants) {
      always_assert_flog(
        index.constantToUnit.emplace(
          cnsName,
          std::make_pair(unit.name, hasInit)
        ).second,
        "Duplicate constant: {}",
        cnsName
      );
    }
  }

  for (auto& cls : input.classes) {
    FTRACE(5, "class {} -> {}\n", cls.name, cls.cls.id().toString());
    always_assert_flog(
      index.classRefs.emplace(cls.name, std::move(cls.cls)).second,
      "Duplicate class: {}",
      cls.name
    );
    always_assert(index.classToUnit.emplace(cls.name, cls.unit).second);

    auto& meta = flattenMeta.cls[cls.name];
    if (cls.closureContext) {
      assertx(cls.isClosure);
      if (!cls.closureDeclInFunc) {
        flattenMeta.cls[cls.closureContext].closures.emplace_back(cls.name);
        index.classToClosures[cls.closureContext].emplace(cls.name);
        meta.closureContext = cls.closureContext;
      } else {
        index.funcToClosures[cls.closureContext].emplace(cls.name);
        meta.closureContext = nullptr;
      }
    }
    meta.isClosure = cls.isClosure;
    meta.deps.insert(begin(cls.dependencies), end(cls.dependencies));
    meta.unresolvedTypes = std::move(cls.unresolvedTypes);
    meta.idx = flattenMeta.allCls.size();
    flattenMeta.allCls.emplace_back(cls.name);

    if (cls.has86init) index.classesWith86Inits.emplace(cls.name);

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

    if (func.methCaller) {
      // If this meth caller a duplicate of one we've already seen?
      auto const [existing, emplaced] =
        methCallerUnits.emplace(func.name, func.unit);
      if (!emplaced) {
        // It is. The duplicate shouldn't be in the same unit,
        // however.
        always_assert_flog(
          existing->second != func.unit,
          "Duplicate meth-caller {} in same unit {}",
          func.name,
          func.unit
        );
        // The winner is the one with the unit with the "lesser"
        // name. This is completely arbitrary.
        if (string_data_lt{}(func.unit, existing->second)) {
          // This one wins. Schedule the older entry for deletion and
          // take over it's position in the map.
          FTRACE(
            4, "  meth caller {} from unit {} taking priority over unit {}",
            func.name, func.unit, existing->second
          );
          flattenMeta.unitDeletions[existing->second].emplace_back(func.name);
          existing->second = func.unit;
          index.funcRefs.at(func.name) = std::move(func.func);
          index.funcToUnit.at(func.name) = func.unit;
        } else {
          // This one loses. Schedule it for deletion.
          flattenMeta.unitDeletions[func.unit].emplace_back(func.name);
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

    index.funcToUnit.emplace(func.name, func.unit);
    if (Constant::nameFromFuncName(func.name)) {
      index.constantInitFuncs.emplace(func.name);
    }

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

    if (bc.methCaller) {
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
      if (bc.unit != unit) {
        FTRACE(
          4,
          "Bytecode for meth-caller func {} in unit {} "
          "skipped because the meth-caller was dropped\n",
          bc.name, bc.unit
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

  if (debug) {
    for (auto const& [cns, unitAndInit] : index.constantToUnit) {
      if (!unitAndInit.second) continue;
      if (is_native_unit(unitAndInit.first)) continue;
      auto const initName = Constant::funcNameFromName(cns);
      always_assert_flog(
        index.funcRefs.count(initName) > 0,
        "Constant {} is marked as having initialization func {}, "
        "but it does not exist",
        cns, initName
      );
    }
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
    ClassGraph::init();
  }
  static void fini() { ClassGraph::destroy(); }

  struct Bundle {
    std::vector<std::unique_ptr<php::Class>> classes;
    std::vector<std::unique_ptr<ClassInfo2>> classInfos;
    std::vector<std::unique_ptr<php::ClassBytecode>> classBytecode;
    std::vector<std::unique_ptr<php::Func>> funcs;
    std::vector<std::unique_ptr<FuncInfo2>> funcInfos;
    std::vector<std::unique_ptr<php::FuncBytecode>> funcBytecode;
    std::vector<std::unique_ptr<php::Unit>> units;
    std::vector<FuncFamilyGroup> funcFamilies;
    std::vector<std::unique_ptr<MethodsWithoutCInfo>> methInfos;

    template <typename SerDe> void serde(SerDe& sd) {
      ScopedStringDataIndexer _;
      ClassGraph::ScopedSerdeState _2;
      sd(classes)
        (classInfos)
        (classBytecode)
        (funcs)
        (funcInfos)
        (funcBytecode)
        (units)
        (funcFamilies)
        (methInfos)
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
                    Variadic<FuncFamilyGroup> funcFamilies,
                    Variadic<std::unique_ptr<MethodsWithoutCInfo>> methInfos) {
    Bundle bundle;
    bundle.classes.reserve(classes.vals.size());
    bundle.classInfos.reserve(classInfos.vals.size());
    bundle.classBytecode.reserve(classBytecode.vals.size());
    bundle.funcs.reserve(funcs.vals.size());
    bundle.funcInfos.reserve(funcInfos.vals.size());
    bundle.funcBytecode.reserve(funcBytecode.vals.size());
    bundle.units.reserve(units.vals.size());
    bundle.funcFamilies.reserve(funcFamilies.vals.size());
    bundle.methInfos.reserve(methInfos.vals.size());
    for (auto& c : classes.vals) {
      bundle.classes.emplace_back(std::move(c));
    }
    for (auto& c : classInfos.vals) {
      c->auxClassGraphs.clear();
      bundle.classInfos.emplace_back(std::move(c));
    }
    for (auto& b : classBytecode.vals) {
      bundle.classBytecode.emplace_back(std::move(b));
    }
    for (auto& f : funcs.vals) {
      bundle.funcs.emplace_back(std::move(f));
    }
    for (auto& f : funcInfos.vals) {
      f->auxClassGraphs.reset();
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
    for (auto& m : methInfos.vals) {
      bundle.methInfos.emplace_back(std::move(m));
    }
    return bundle;
  }
};

Job<AggregateJob> s_aggregateJob;

void remote_func_info_to_local(IndexData& index,
                               const php::Func& func,
                               FuncInfo2& rfinfo) {
  assertx(func.name == rfinfo.name);
  auto finfo = func_info(index, &func);
  assertx(finfo->returnTy.is(BInitCell));
  finfo->returnTy = std::move(rfinfo.returnTy);
  finfo->returnRefinements = rfinfo.returnRefinements;
  finfo->retParam = rfinfo.retParam;
  finfo->effectFree = rfinfo.effectFree;
  finfo->unusedParams = rfinfo.unusedParams;
}

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
      remote_func_info_to_local(index, *it->second, *rfinfo);
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
          return string_data_lt_type{}(a.ptr()->cls->name, b.ptr()->cls->name);
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
      if (rcinfo->parent) cinfo->parent = get(rcinfo->parent);

      if (!(cinfo->cls->attrs & AttrNoExpandTrait)) {
        auto const traits = rcinfo->classGraph.usedTraits();
        cinfo->usedTraits.reserve(traits.size());
        for (auto const trait : traits) {
          cinfo->usedTraits.emplace_back(get(trait.name()));
        }
        cinfo->usedTraits.shrink_to_fit();
      }
      cinfo->traitProps = std::move(rcinfo->traitProps);

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

      for (size_t i = 0, size = cinfo->cls->constants.size(); i < size; ++i) {
        auto const& cns = cinfo->cls->constants[i];
        if (cns.kind != ConstModifiers::Kind::Value) continue;
        if (!cns.val.has_value())                    continue;
        if (cns.val->m_type != KindOfUninit)         continue;
        if (i >= cinfo->clsConstTypes.size()) {
          cinfo->clsConstTypes.resize(i+1, ClsConstInfo { TInitCell, 0 });
        }
        cinfo->clsConstTypes[i] = folly::get_default(
          rcinfo->clsConstantInfo,
          cns.name,
          ClsConstInfo { TInitCell, 0 }
        );
      }
      cinfo->clsConstTypes.shrink_to_fit();

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

      cinfo->hasBadRedeclareProp = rcinfo->hasBadRedeclareProp;
      cinfo->hasBadInitialPropValues = rcinfo->hasBadInitialPropValues;
      cinfo->hasConstProp = rcinfo->hasConstProp;
      cinfo->hasReifiedParent = rcinfo->hasReifiedParent;
      cinfo->subHasConstProp = rcinfo->subHasConstProp;
      cinfo->isMocked = rcinfo->isMocked;
      cinfo->isSubMocked = rcinfo->isSubMocked;

      cinfo->classGraph = rcinfo->classGraph;
      cinfo->classGraph.setCInfo(*cinfo);

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
          if (!cinfo->classGraph.mightHaveRegularSubclass()) continue;
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
        // This is rare. Only happens with unflattened traits, so
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
        remote_func_info_to_local(index, *func, *rfi);
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

  // These aren't needed below so we can free them immediately.
  decltype(index.classToClosures){}.swap(index.classToClosures);
  decltype(index.funcToClosures){}.swap(index.funcToClosures);
  decltype(index.classesWith86Inits){}.swap(index.classesWith86Inits);
  decltype(index.classToUnit){}.swap(index.classToUnit);
  decltype(index.funcToUnit){}.swap(index.funcToUnit);
  decltype(index.constantToUnit){}.swap(index.constantToUnit);
  decltype(index.constantInitFuncs){}.swap(index.constantInitFuncs);

  // Unlike other cases, we want to bound each bucket to roughly the
  // same total byte size (since ultimately we're going to download
  // everything).
  auto const usingSubprocess = index.client->usingSubprocess();
  // We can be more aggressive in subprocess mode because there's no
  // actual aggregation.
  auto const sizePerBucket = usingSubprocess
    ? 256*1024*1024
    : 128*1024*1024;

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
      index.funcInfoRefs.size() + index.funcFamilyRefs.size() +
      index.uninstantiableClsMethRefs.size()
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
    for (auto const& [name, ref] : index.uninstantiableClsMethRefs) {
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

  std::vector<std::unique_ptr<MethodsWithoutCInfo>> remoteMethInfos;
  remoteMethInfos.reserve(index.uninstantiableClsMethRefs.size());

  hphp_fast_set<FuncFamily2::Id> remoteFuncFamilyIds;
  std::vector<std::unique_ptr<FuncFamily2>> remoteFuncFamilies;
  remoteFuncFamilies.reserve(index.funcFamilyRefs.size());

  auto const run = [&] (std::vector<SString> chunks) -> coro::Task<void> {
    co_await coro::co_reschedule_on_current_executor;

    if (chunks.empty()) co_return;

    std::vector<UniquePtrRef<php::Class>> classes;
    std::vector<UniquePtrRef<ClassInfo2>> classInfos;
    std::vector<UniquePtrRef<php::ClassBytecode>> classBytecode;
    std::vector<UniquePtrRef<php::Func>> funcs;
    std::vector<UniquePtrRef<FuncInfo2>> funcInfos;
    std::vector<UniquePtrRef<php::FuncBytecode>> funcBytecode;
    std::vector<UniquePtrRef<php::Unit>> units;
    std::vector<Ref<FuncFamilyGroup>> funcFamilies;
    std::vector<UniquePtrRef<MethodsWithoutCInfo>> methInfos;

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
      if (auto const r = folly::get_optional(index.uninstantiableClsMethRefs,
                                             name)) {
        if (ids.emplace(r->id()).second) {
          methInfos.emplace_back(*r);
        }
      }
    }

    AggregateJob::Bundle chunk;
    if (!usingSubprocess) {
      Client::ExecMetadata metadata{
        .job_key = folly::sformat("aggregate {}", chunks[0])
      };

      // Aggregate the data, which makes downloading it more efficient.
      auto config = co_await index.configRef->getCopy();
      auto outputs = co_await index.client->exec(
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
            std::move(funcFamilies),
            std::move(methInfos)
          )
        ),
        std::move(metadata)
      );
      assertx(outputs.size() == 1);

      // Download the aggregate chunks.
      chunk = co_await index.client->load(std::move(outputs[0]));
    } else {
      // If we're using subprocess mode, we don't need to aggregate
      // and we can just download the items directly.
      auto [c, cinfo, cbc, f, finfo, fbc, u, ff, minfo] =
        co_await coro::collectAll(
          index.client->load(std::move(classes)),
          index.client->load(std::move(classInfos)),
          index.client->load(std::move(classBytecode)),
          index.client->load(std::move(funcs)),
          index.client->load(std::move(funcInfos)),
          index.client->load(std::move(funcBytecode)),
          index.client->load(std::move(units)),
          index.client->load(std::move(funcFamilies)),
          index.client->load(std::move(methInfos))
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
      chunk.methInfos.insert(
        end(chunk.methInfos),
        std::make_move_iterator(begin(minfo)),
        std::make_move_iterator(end(minfo))
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
      remoteMethInfos.insert(
        end(remoteMethInfos),
        std::make_move_iterator(begin(chunk.methInfos)),
        std::make_move_iterator(end(chunk.methInfos))
      );
      for (auto& group : chunk.funcFamilies) {
        for (auto& ff : group.m_ffs) {
          if (remoteFuncFamilyIds.emplace(ff->m_id).second) {
            remoteFuncFamilies.emplace_back(std::move(ff));
          }
        }
      }
    }

    co_return;
  };

  // We're going to load ClassGraphs concurrently.
  ClassGraph::initConcurrent();

  {
    // Temporarily suppress case collision logging
    auto oldTypeLogLevel = RuntimeOption::EvalLogTsameCollisions;
    auto oldFuncLogLevel = RuntimeOption::EvalLogFsameCollisions;
    RuntimeOption::EvalLogTsameCollisions = 0;
    RuntimeOption::EvalLogFsameCollisions = 0;
    SCOPE_EXIT {
      RuntimeOption::EvalLogTsameCollisions = oldTypeLogLevel;
      RuntimeOption::EvalLogFsameCollisions = oldFuncLogLevel;
    };

    coro::blockingWait(coro::collectAllRange(
      from(buckets)
        | move
        | map([&] (std::vector<SString> chunks) {
            return run(std::move(chunks)).scheduleOn(index.executor->sticky());
          })
        | as<std::vector>()
    ));
  }

  // Deserialization done.
  ClassGraph::stopConcurrent();

  // We've used any refs we need. Free them now to save memory.
  decltype(index.unitRefs){}.swap(index.unitRefs);
  decltype(index.classRefs){}.swap(index.classRefs);
  decltype(index.funcRefs){}.swap(index.funcRefs);
  decltype(index.classInfoRefs){}.swap(index.classInfoRefs);
  decltype(index.funcInfoRefs){}.swap(index.funcInfoRefs);
  decltype(index.funcFamilyRefs){}.swap(index.funcFamilyRefs);
  decltype(index.classBytecodeRefs){}.swap(index.classBytecodeRefs);
  decltype(index.funcBytecodeRefs){}.swap(index.funcBytecodeRefs);
  decltype(index.uninstantiableClsMethRefs){}.swap(
    index.uninstantiableClsMethRefs
  );

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

  // Convert any "orphan" FuncInfo2s (those representing methods for a
  // class without a ClassInfo).
  parallel::for_each(
    remoteMethInfos,
    [&] (std::unique_ptr<MethodsWithoutCInfo>& meths) {
      auto const cls = folly::get_default(index.classes, meths->cls);
      always_assert_flog(
        cls,
        "php::Class for {} not found in index",
        meths->cls
      );
      assertx(cls->methods.size() == meths->finfos.size());
      for (size_t i = 0, size = cls->methods.size(); i < size; ++i) {
        remote_func_info_to_local(index, *cls->methods[i], *meths->finfos[i]);
      }
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
             std::unique_ptr<TicketExecutor> executor,
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
  auto [subclassMeta, initTypesMeta, ifaceConflicts] =
    flatten_classes(*m_data, std::move(flattenMeta));
  build_subclass_lists(*m_data, std::move(subclassMeta), initTypesMeta);
  init_types(*m_data, std::move(initTypesMeta));
  compute_iface_vtables(*m_data, std::move(ifaceConflicts));
  check_invariants(*m_data);
}

// Defined here so IndexData is a complete type for the unique_ptr
// destructor.
Index::~Index() = default;

Index::Index(Index&&) = default;
Index& Index::operator=(Index&&) = default;

//////////////////////////////////////////////////////////////////////

const php::Program& Index::program() const {
  return *m_data->program;
}

StructuredLogEntry* Index::sample() const {
  return m_data->sample;
}

//////////////////////////////////////////////////////////////////////

TicketExecutor& Index::executor() const {
  return *m_data->executor;
}

Client& Index::client() const {
  return *m_data->client;
}

const CoroAsyncValue<Ref<Config>>& Index::configRef() const {
  return *m_data->configRef;
}

//////////////////////////////////////////////////////////////////////

const TSStringSet& Index::classes_with_86inits() const {
  return m_data->classesWith86Inits;
}

const FSStringSet& Index::constant_init_funcs() const {
  return m_data->constantInitFuncs;
}

//////////////////////////////////////////////////////////////////////

void Index::make_local() {
  HHBBC::make_local(*m_data);
  check_local_invariants(*m_data);
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

const php::Class* Index::lookup_class(SString name) const {
  return folly::get_default(m_data->classes, name);
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
        if (auto const ts = resolve_type_structure(
              IndexAdaptor { *this },
              nullptr,
              *typeAlias
            ).sarray()
           ) {
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
        if (auto const resolved = resolve_type_structure(
              IndexAdaptor { *this },
              cns,
              *cinfo->cls
            ).sarray()
           ) {
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
      if (!cinfo->classGraph.hasCompleteChildren()) return;

      for (auto& cns : const_cast<php::Class*>(cinfo->cls)->constants) {
        assertx(cns.invariance == php::Const::Invariance::None);
        if (cns.kind != ConstModifiers::Kind::Type) continue;
        if (!cns.val.has_value()) continue;
        if (!cns.resolvedTypeStructure) continue;

        auto const checkClassname =
          tvIsString(cns.resolvedTypeStructure->get(s_classname));

        // Assume it doesn't change
        auto invariance = php::Const::Invariance::Same;
        for (auto const g : cinfo->classGraph.children()) {
          assertx(!g.isMissing());
          assertx(g.hasCompleteChildren());
          auto const s = g.cinfo();
          assertx(s);
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
  return res::Class::get(*it->second);
}

Optional<res::Class> Index::resolve_class(const php::Class& cls) const {
  return resolve_class(cls.name);
}

const php::TypeAlias* Index::lookup_type_alias(SString name) const {
  auto const it = m_data->typeAliases.find(name);
  if (it == m_data->typeAliases.end()) return nullptr;
  return it->second;
}

Index::ClassOrTypeAlias Index::lookup_class_or_type_alias(SString name) const {
  auto const rcls = resolve_class(name);
  if (rcls) {
    auto const cls = [&] () -> const php::Class* {
      if (auto const ci = rcls->cinfo()) return ci->cls;
      return m_data->classes.at(rcls->name());
    }();
    return ClassOrTypeAlias{cls, nullptr, true};
  }
  if (auto const ta = lookup_type_alias(name)) {
    return ClassOrTypeAlias{nullptr, ta, true};
  }
  return ClassOrTypeAlias{nullptr, nullptr, false};
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
    auto const cinfo = dcls.cls().cinfo();
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

  auto const DEBUG_ONLY allIncomplete = !debug || std::all_of(
    begin(dcls.isect()), end(dcls.isect()),
    [] (res::Class c) { return !c.hasCompleteChildren(); }
  );

  for (auto const i : dcls.isect()) {
    auto const cinfo = i.cinfo();
    if (!cinfo) continue;

    auto const func = process(cinfo, false, dcls.containsNonRegular());
    match<void>(
      func.val,
      [&] (Func::MethodName) {},
      [&] (Func::Method m) {
        if (singleMethod) {
          assertx(missing != TriBool::Yes);
          assertx(isect.families.empty());
          if (singleMethod != m.finfo->func) {
            assertx(allIncomplete);
            singleMethod = nullptr;
            missing = TriBool::Yes;
          } else {
            missing = TriBool::No;
          }
        } else if (missing != TriBool::Yes) {
          singleMethod = m.finfo->func;
          isect.families.clear();
          missing = TriBool::No;
        }
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
        if (singleMethod) {
          assertx(missing != TriBool::Yes);
          assertx(isect.families.empty());
          if (singleMethod != m.finfo->func) {
            assertx(allIncomplete);
            singleMethod = nullptr;
            missing = TriBool::Yes;
          }
        } else if (missing != TriBool::Yes) {
          singleMethod = m.finfo->func;
          isect.families.clear();
        }
      },
      [&] (Func::MissingMethod) {
        assertx(IMPLIES(missing == TriBool::No, allIncomplete));
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
      [&] (Func::MissingFunc)      { always_assert(false); },
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
    if (missing == TriBool::Yes) {
      return Func { Func::MissingMethod { dcls.smallestCls().name(), name } };
    }
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
  auto const general = [&] (bool includeNonRegular, SString maybeCls) {
    assertx(name != s_construct.get());

    // We don't produce name-only global func families for special
    // methods, so be conservative. We don't call special methods in a
    // context where we'd expect to not know the class, so it's not
    // worthwhile. The same goes for __invoke and __debuginfo, which
    // is corresponds to every closure, and gets too large without
    // much value.
    if (!has_name_only_func_family(name)) {
      return Func { Func::MethodName { maybeCls, name } };
    }

    // Lookup up the name-only global func families for this name. If
    // we don't have one, the method cannot exist because it contains
    // every method with that name in the program.
    auto const famIt = m_data->methodFamilies.find(name);
    if (famIt == end(m_data->methodFamilies)) {
      return Func { Func::MissingMethod { maybeCls, name } };
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
      return Func { Func::MissingMethod { maybeCls, name } };
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
        return Func { Func::MethodName { cinfo->cls->name, name } };
      }
      // We're only considering this class, not it's subclasses. Since
      // it doesn't exist here, the resolution will always fail.
      if (isExact) {
        return Func { Func::MissingMethod { cinfo->cls->name, name } };
      }
      // The method isn't present on this class, but it might be in
      // the subclasses. In most cases try a general lookup to get a
      // slightly better type than nothing.
      if (includeNonRegular ||
          !(cinfo->cls->attrs & (AttrInterface|AttrAbstract))) {
        return general(includeNonRegular, cinfo->cls->name);
      }

      // A special case is if we're only considering regular classes,
      // and this is an interface or abstract class. For those, we
      // "expand" the method families table to include any methods
      // defined in *all* regular subclasses. This is needed to
      // preserve monotonicity. Check this now.
      auto const famIt = cinfo->methodFamilies.find(name);
      // If no entry, treat it pessimistically like the rest of the
      // cases.
      if (famIt == end(cinfo->methodFamilies)) {
        return general(false, cinfo->cls->name);
      }

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
      return Func { Func::MethodName { cinfo->cls->name, name } };
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
        if (!cinfo->classGraph.exactSubtypeOf(ctxCInfo->classGraph,
                                              true,
                                              true)) {
          return nullptr;
        }
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
      if (isExact) {
        return Func { Func::MissingMethod { cinfo->cls->name, name } };
      }
      if (meth.noOverrideRegular()) {
        // The method isn't overridden in a subclass, but we can't
        // use the base class either. This leaves two cases. Either
        // the method isn't overridden because there are no regular
        // subclasses (in which case there's no resolution at all), or
        // because there's regular subclasses, but they use the same
        // method (in which case the result is just ftarget).
        if (!cinfo->classGraph.mightHaveRegularSubclass()) {
          return Func { Func::MissingMethod { cinfo->cls->name, name } };
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
          return Func { Func::MissingMethod { cinfo->cls->name, name } };
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
    if (isClass) {
      return Func { Func::MethodName { nullptr, s_construct.get() } };
    }
    return resolve_ctor(thisType);
  }

  if (isClass) {
    if (!is_specialized_cls(thisType)) return general(true, nullptr);
  } else if (!is_specialized_obj(thisType)) {
    return general(false, nullptr);
  }

  auto const& dcls = isClass ? dcls_of(thisType) : dobj_of(thisType);
  return rfunc_from_dcls(
    dcls,
    name,
    process,
    [&] (bool i) { return general(i, dcls.smallestCls().name()); }
  );
}

res::Func Index::resolve_ctor(const Type& obj) const {
  assertx(obj.subtypeOf(BObj));

  using Func = res::Func;

  // Can't say anything useful if we don't know the object type.
  if (!is_specialized_obj(obj)) {
    return Func { Func::MethodName { nullptr, s_construct.get() } };
  }

  auto const& dcls = dobj_of(obj);
  return rfunc_from_dcls(
    dcls,
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
        return Func {
          Func::MethodName { cinfo->cls->name, s_construct.get() }
        };
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
            (isExact || !cinfo->classGraph.mightHaveRegularSubclass())) {
          return Func {
            Func::MissingMethod { cinfo->cls->name, s_construct.get() }
          };
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
            return Func {
              Func::MethodName { cinfo->cls->name, s_construct.get() }
            };
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
      return Func {
        Func::MethodName { dcls.smallestCls().name(), s_construct.get() }
      };
    }
  );
}

res::Func Index::resolve_func(SString name) const {
  name = normalizeNS(name);
  auto const it = m_data->funcs.find(name);
  if (it == end(m_data->funcs)) {
    return res::Func { res::Func::MissingFunc { name } };
  }
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
    auto const cinfo = dcls.cls().cinfo();
    if (!cinfo) return false;
    if (dcls.containsNonRegular() || is_regular_class(*cinfo->cls)) {
      f(cinfo);
    }
    return true;
  } else if (dcls.isSub()) {
    auto unresolved = false;
    res::Class::visitEverySub(
      std::array<res::Class, 1>{dcls.cls()},
      dcls.containsNonRegular(),
      [&] (res::Class c) {
        if (c.hasCompleteChildren()) {
          if (auto const cinfo = c.cinfo()) return f(cinfo);
        }
        unresolved = true;
        return false;
      }
    );
    return !unresolved;
  } else {
    auto const& isect = dcls.isect();
    assertx(isect.size() > 1);

    auto unresolved = false;
    res::Class::visitEverySub(
      isect,
      dcls.containsNonRegular(),
      [&] (res::Class c) {
        if (c.hasCompleteChildren()) {
          if (auto const cinfo = c.cinfo()) return f(cinfo);
        }
        unresolved = true;
        return false;
      }
    );
    return !unresolved;
  }
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
      auto mightThrow = bool(ci->cls->attrs & AttrInternal);
      if (!mightThrow) {
        auto const unit = lookup_class_unit(*ci->cls);
        auto const moduleName = unit->moduleName;
        auto const packageInfo = unit->packageInfo;
        if (auto const activeDeployment = packageInfo.getActiveDeployment()) {
          if (!packageInfo.moduleInDeployment(
                moduleName, *activeDeployment, DeployKind::Hard)) {
            mightThrow = true;
          }
        }
      }
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
    auto const cinfo = dcls.cls().cinfo();
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
    auto const cinfo = dcls.cls().cinfo();
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
      : resolve_type_structure(IndexAdaptor { *this }, cns, *ci->cls);

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

  auto const ctxType = adjust_closure_context(
    IndexAdaptor { *this },
    calleeCtx
  );

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
      IndexAdaptor { *this },
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
    [&] (res::Func::MissingFunc)       { return R{ TBottom, false }; },
    [&] (res::Func::MissingMethod)     { return R{ TBottom, false }; },
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
    [&] (res::Func::MissingFunc)       { return R { TBottom, false }; },
    [&] (res::Func::MissingMethod)     { return R { TBottom, false }; },
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
    IndexAdaptor { *this },
    *cls,
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
    IndexAdaptor { *this },
    *cls,
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
              IndexAdaptor { *this },
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
    if (!rCtx) return conservative();
    ctxCls = rCtx->cinfo();
    if (!ctxCls) return conservative();
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
        dcls.isSub() && !sname && cinfo != start.cinfo()
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
  if (auto const ci = cls.cinfo()) {
    return class_init_might_raise(*m_data, ctx, ci);
  } else if (cls.cinfo2()) {
    // Not implemented yet
    always_assert(false);
  } else {
    return true;
  }
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
        IndexAdaptor { *this }, unctx(val), ignoreConst, mustBeReadOnly
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
        IndexAdaptor { *this }, sname, unctx(val), ignoreConst, mustBeReadOnly
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
    ctxCls = rCtx->cinfo();
    if (!ctxCls) return unknownCls();
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
        dcls.isSub() && !sname && cinfo != start->cinfo()
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

  // Both a pinit and a sinit can have resolved property values. When
  // analyzing constants we'll process each function separately and
  // potentially in different threads. Both will want to inspect the
  // property Attrs and the hasBadInitialPropValues. So, if we reach
  // here, take a lock to ensure both don't stomp on each other.
  static std::array<std::mutex, 256> locks;
  auto& lock = locks[pointer_hash<const php::Class>{}(ctx.cls) % locks.size()];
  std::lock_guard<std::mutex> _{lock};

  auto const noBad = std::all_of(
    begin(props), end(props),
    [] (const php::Prop& prop) {
      return bool(prop.attrs & AttrInitialSatisfiesTC);
    }
  );

  if (cinfo->hasBadInitialPropValues) {
    if (noBad) {
      cinfo->hasBadInitialPropValues = false;
      find_deps(*m_data, ctx.cls, Dep::PropBadInitialValues, deps);
    }
  } else {
    // If it's false, another thread got here before us and set it to
    // false.
    always_assert(noBad);
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
          IndexAdaptor { *this },
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

FuncOrCls AnalysisWorklist::next() {
  if (list.empty()) return FuncOrCls{};
  auto n = list.front();
  in.erase(n);
  list.pop_front();
  return n;
}

void AnalysisWorklist::schedule(FuncOrCls fc) {
  if (!in.emplace(fc).second) return;
  ITRACE(2, "scheduling {} onto worklist\n", show(fc));
  list.emplace_back(fc);
}

//////////////////////////////////////////////////////////////////////

bool AnalysisDeps::add(Class c) {
  return classes.emplace(c.name).second;
}

bool AnalysisDeps::add(ConstIndex cns) {
  // Dependency on class constant implies a dependency on the class as
  // well.
  add(Class { cns.cls });
  return clsConstants.emplace(cns).second;
}

bool AnalysisDeps::add(Constant cns) {
  // Dependency on top-level constant implies a dependency on the
  // 86cinit initialized as well (which may not even exist).
  add(Func { HPHP::Constant::funcNameFromName(cns.name) }, Type::Meta);
  return constants.emplace(cns.name).second;
}

bool AnalysisDeps::add(TypeAlias typeAlias) {
  return typeAliases.emplace(typeAlias.name).second;
}

AnalysisDeps::Type AnalysisDeps::add(const php::Func& f, Type t) {
  return f.cls
    ? add(MethRef { f }, t)
    : add(Func { f.name }, t);
}

AnalysisDeps::Type AnalysisDeps::add(MethRef m, Type t) {
  add(Class { m.cls });
  return merge(methods[m], t | Type::Meta);
}

AnalysisDeps::Type AnalysisDeps::add(Func f, Type t) {
  return merge(funcs[f.name], t | Type::Meta);
}

AnalysisDeps::Type AnalysisDeps::merge(Type& o, Type n) {
  auto const added = n - o;
  o |= n;
  return added;
}

AnalysisDeps& AnalysisDeps::operator|=(const AnalysisDeps& o) {
  for (auto const [name, type] : o.funcs) funcs[name] |= type;
  for (auto const& [meth, type] : o.methods) methods[meth] |= type;
  classes.insert(begin(o.classes), end(o.classes));
  clsConstants.insert(begin(o.clsConstants), end(o.clsConstants));
  constants.insert(begin(o.constants), end(o.constants));
  typeAliases.insert(begin(o.typeAliases), end(o.typeAliases));
  return *this;
}

std::string show(AnalysisDeps::Type t) {
  using T = AnalysisDeps::Type;
  std::string out;
  auto const add = [&] (const char* s) {
    folly::format(&out, "{}{}", out.empty() ? "" : ",", s);
  };
  if (t & T::Meta)          add("meta");
  if (t & T::RetType)       add("return type");
  if (t & T::ScalarRetType) add("scalar return type");
  if (t & T::RetParam)      add("returned param");
  if (t & T::UnusedParams)  add("unused params");
  if (t & T::Bytecode)      add("bytecode");
  return out;
}

//////////////////////////////////////////////////////////////////////

void AnalysisChangeSet::changed(ConstIndex idx) {
  clsConstants.emplace(idx);
}

void AnalysisChangeSet::changed(const php::Constant& c) {
  constants.emplace(c.name);
}

void AnalysisChangeSet::changed(const php::Func& f, Type t) {
  assertx(AnalysisDeps::isValidForChanges(t));
  if (f.cls) {
    methods[MethRef { f }] |= t;
  } else {
    funcs[f.name] |= t;
  }
}

//////////////////////////////////////////////////////////////////////

namespace {

template <typename V, typename H, typename E, typename C>
std::vector<SString>
map_to_sorted_key_vec(const hphp_fast_map<SString, V, H, E>& m,
                      const C& c) {
  std::vector<SString> keys;
  keys.reserve(m.size());
  for (auto const& [k, _] : m) keys.emplace_back(k);
  std::sort(begin(keys), end(keys), c);
  return keys;
}

template <typename V, typename H, typename E, typename C>
std::vector<V> map_to_sorted_vec(const hphp_fast_map<SString, V, H, E>& m,
                                 const C& c) {
  auto const keys = map_to_sorted_key_vec(m, c);
  std::vector<V> out;
  out.reserve(keys.size());
  for (auto const k : keys) out.emplace_back(m.at(k));
  return out;
}

}

std::vector<SString> AnalysisInput::classNames() const {
  return map_to_sorted_key_vec(classes, string_data_lt_type{});
}

std::vector<SString> AnalysisInput::funcNames() const {
  return map_to_sorted_key_vec(funcs, string_data_lt_func{});
}

std::vector<SString> AnalysisInput::unitNames() const {
  return map_to_sorted_key_vec(units, string_data_lt{});
}

std::vector<SString> AnalysisInput::cinfoNames() const {
  using namespace folly::gen;
  return from(classNames())
    | filter([&] (SString n) { return (bool)cinfos.count(n); })
    | as<std::vector>();
}

std::vector<SString> AnalysisInput::minfoNames() const {
  using namespace folly::gen;
  return from(classNames())
    | filter([&] (SString n) { return (bool)minfos.count(n); })
    | as<std::vector>();
}

AnalysisInput::Tuple AnalysisInput::toTuple(Ref<Meta> meta) const {
  return Tuple{
    map_to_sorted_vec(classes, string_data_lt_type{}),
    map_to_sorted_vec(funcs, string_data_lt_func{}),
    map_to_sorted_vec(units, string_data_lt{}),
    map_to_sorted_vec(classBC, string_data_lt_type{}),
    map_to_sorted_vec(funcBC, string_data_lt_func{}),
    map_to_sorted_vec(cinfos, string_data_lt_type{}),
    map_to_sorted_vec(finfos, string_data_lt_func{}),
    map_to_sorted_vec(minfos, string_data_lt_type{}),
    map_to_sorted_vec(depClasses, string_data_lt_type{}),
    map_to_sorted_vec(depFuncs, string_data_lt_func{}),
    map_to_sorted_vec(depUnits, string_data_lt{}),
    std::move(meta)
  };
}

//////////////////////////////////////////////////////////////////////

AnalysisScheduler::AnalysisScheduler(Index& index) : index{index} {}

void AnalysisScheduler::registerClass(SString name) {
  FTRACE(5, "AnalysisScheduler: registering class {}\n", name);
  always_assert(classState.try_emplace(name).second);
  classesToSchedule.emplace(name);
  classNames.emplace_back(name);
}

void AnalysisScheduler::registerFunc(SString name) {
  FTRACE(5, "AnalysisScheduler: registering func {}\n", name);
  always_assert(funcState.try_emplace(name).second);
  funcsToSchedule.emplace(name);
  funcNames.emplace_back(name);
  // If this func is a 86cinit, then register the associated constant
  // as well.
  if (auto const cns = Constant::nameFromFuncName(name)) {
    FTRACE(5, "AnalysisScheduler: registering constant {}\n", cns);
    always_assert(cnsChanged.try_emplace(cns).second);
  }
}

// Called when an analysis job reports back its changes. This makes
// any dependencies affected by the change eligible to run in the next
// analysis round. ChangeGroup is all the things in the same job (used
// for sanity checking).
void AnalysisScheduler::recordChanges(const AnalysisChangeSet& changed,
                                      const ChangeGroup& group) {
  for (auto const [name, type] : changed.funcs) {
    FTRACE(4, "AnalysisScheduler: func {} changed ({})\n", name, show(type));
    auto state = folly::get_ptr(funcState, name);
    always_assert_flog(
      state,
      "Trying to mark un-tracked func {} changed",
      name
    );
    always_assert_flog(
      group.funcs.count(name),
      "Trying to mark func {} as changed from wrong shard",
      name
    );
    assertx(AnalysisDeps::isValidForChanges(type));
    assertx(state->changed == Type::None);
    state->changed = type;
  }

  for (auto const [meth, type] : changed.methods) {
    FTRACE(4, "AnalysisScheduler: method {} changed ({})\n",
           show(meth), show(type));
    auto state = folly::get_ptr(classState, meth.cls);
    always_assert_flog(
      state,
      "Trying to mark method for un-tracked class {} changed",
      meth.cls
    );
    always_assert_flog(
      group.classes.count(meth.cls),
      "Trying to mark method for class {} as changed from wrong shard",
      meth.cls
    );
    assertx(AnalysisDeps::isValidForChanges(type));
    auto& t = state->methodChanges.ensure(meth.idx);
    assertx(t == Type::None);
    t = type;
  }

  for (auto const cns : changed.clsConstants) {
    FTRACE(4, "AnalysisScheduler: class constant {} changed\n", show(cns));
    auto state = folly::get_ptr(classState, cns.cls);
    always_assert_flog(
      state,
      "Trying to mark method for un-tracked class {} changed",
      cns.cls
    );
    always_assert_flog(
      group.classes.count(cns.cls),
      "Trying to mark constant for class {} as changed from wrong shard",
      cns.cls
    );
    if (cns.idx >= state->cnsChanges.size()) {
      state->cnsChanges.resize(cns.idx+1);
    }
    assertx(!state->cnsChanges[cns.idx]);
    state->cnsChanges[cns.idx] = true;
  }

  for (auto const name : changed.constants) {
    FTRACE(4, "AnalysisScheduler: constant {} changed\n", name);
    auto state = folly::get_ptr(cnsChanged, name);
    always_assert_flog(
      state,
      "Trying to mark un-tracked constant {} changed",
      name
    );
    auto const unit = folly::get_ptr(index.m_data->constantToUnit, name);
    always_assert_flog(
      unit && group.units.count(unit->first),
      "Trying to mark constant {} as changed from wrong shard",
      name
    );
    assertx(!state->load(std::memory_order_acquire));
    state->store(true, std::memory_order_release);
  }
}

// Update the dependencies stored in the scheduler to take into
// account the new set of dependencies reported by an analysis job.
void AnalysisScheduler::updateDepState(AnalysisOutput& output, ChangeGroup g) {
  // Every entity in the same analysis job will share the same
  // ChangeGroup, so use a std::shared_ptr here so we can share it
  // among all of them.
  auto const group = std::make_shared<ChangeGroup>(std::move(g));

  auto const update = [&] (DepState& state, AnalysisDeps d) {
    assertx(!state.group);
    assertx(!state.newDeps);
    state.newDeps.emplace(std::move(d));
    state.group = group;
  };

  for (size_t i = 0, size = output.classNames.size(); i < size; ++i) {
    auto const name = output.classNames[i];
    auto it = classState.find(name);
    always_assert_flog(
      it != end(classState),
      "Trying to set deps for un-tracked class {}",
      name
    );
    update(it->second.depState, std::move(output.meta.classDeps[i]));
  }
  for (size_t i = 0, size = output.funcNames.size(); i < size; ++i) {
    auto const name = output.funcNames[i];
    auto it = funcState.find(name);
    always_assert_flog(
      it != end(funcState),
      "Trying to set deps for un-tracked func {}",
      name
    );
    update(it->second.depState, std::move(output.meta.funcDeps[i]));
  }
}

// Record the output of an analys job. This means updating the various
// Refs to their new versions, recording new dependencies, and
// recording what has changed (to schedule the next round).
void AnalysisScheduler::record(AnalysisOutput output) {
  auto const numClasses = output.classNames.size();
  auto const numCInfos = output.cinfoNames.size();
  auto const numMInfos = output.minfoNames.size();
  auto const numFuncs = output.funcNames.size();
  auto const numUnits = output.unitNames.size();
  assertx(numClasses == output.classes.size());
  assertx(numClasses == output.clsBC.size());
  assertx(numCInfos == output.cinfos.size());
  assertx(numMInfos == output.minfos.size());
  assertx(numClasses == output.meta.classDeps.size());
  assertx(numFuncs == output.funcs.size());
  assertx(numFuncs == output.funcBC.size());
  assertx(numFuncs == output.finfos.size());
  assertx(numFuncs == output.meta.funcDeps.size());
  assertx(numUnits == output.units.size());

  // Update Ref mappings:

  for (size_t i = 0; i < numUnits; ++i) {
    auto const name = output.unitNames[i];
    index.m_data->unitRefs.at(name) = std::move(output.units[i]);
  }

  for (size_t i = 0; i < numClasses; ++i) {
    auto const name = output.classNames[i];
    index.m_data->classRefs.at(name) = std::move(output.classes[i]);
    index.m_data->classBytecodeRefs.at(name) = std::move(output.clsBC[i]);
  }
  for (size_t i = 0; i < numCInfos; ++i) {
    auto const name = output.cinfoNames[i];
    index.m_data->classInfoRefs.at(name) =
      output.cinfos[i].cast<std::unique_ptr<ClassInfo2>>();
  }
  for (size_t i = 0; i < numMInfos; ++i) {
    auto const name = output.minfoNames[i];
    index.m_data->uninstantiableClsMethRefs.at(name) =
      output.minfos[i].cast<std::unique_ptr<MethodsWithoutCInfo>>();
  }
  for (size_t i = 0; i < numFuncs; ++i) {
    auto const name = output.funcNames[i];
    index.m_data->funcRefs.at(name) = std::move(output.funcs[i]);
    index.m_data->funcBytecodeRefs.at(name) = std::move(output.funcBC[i]);
    index.m_data->funcInfoRefs.at(name) =
      output.finfos[i].cast<std::unique_ptr<FuncInfo2>>();
  }

  // ChangeGroup represents all entities in a particular analysis job.
  ChangeGroup group;
  group.classes.insert(begin(output.classNames), end(output.classNames));
  group.funcs.insert(begin(output.funcNames), end(output.funcNames));
  group.units.insert(begin(output.unitNames), end(output.unitNames));

  recordChanges(output.meta.changed, group);
  updateDepState(output, std::move(group));

  // If the analysis job optimized away any 86cinit functions, record
  // that here so they can be later removed from our tables.
  if (!output.meta.removedFuncs.empty()) {
    // This is relatively rare, so a lock is fine.
    std::lock_guard<std::mutex> _{funcsToRemoveLock};
    funcsToRemove.insert(
      begin(output.meta.removedFuncs),
      end(output.meta.removedFuncs)
    );
  }
}

// Remove metadata for any 86cinit function that an analysis job
// optimized away. This must be done *after* calculating the next
// round of work.
void AnalysisScheduler::removeFuncs() {
  if (funcsToRemove.empty()) return;
  for (auto const name : funcsToRemove) {
    FTRACE(4, "AnalysisScheduler: removing function {}\n", name);
    always_assert(index.m_data->funcRefs.erase(name));
    always_assert(index.m_data->funcBytecodeRefs.erase(name));
    always_assert(index.m_data->funcInfoRefs.erase(name));
    always_assert(index.m_data->funcToUnit.erase(name));
    always_assert(!funcsToSchedule.count(name));
    always_assert(funcState.erase(name));
    if (auto const cns = Constant::nameFromFuncName(name)) {
      always_assert(index.m_data->constantInitFuncs.erase(name));
      always_assert(cnsChanged.erase(cns));
      index.m_data->constantToUnit.at(cns).second = false;
    }
  }
  funcNames.erase(
    std::remove_if(
      begin(funcNames), end(funcNames),
      [&] (SString name) { return funcsToRemove.count(name); }
    ),
    end(funcNames)
  );
  funcsToRemove.clear();
}

// Calculate any classes or functions which should be scheduled to be
// analyzed in the next round.
void AnalysisScheduler::findToSchedule() {
  static const AnalysisDeps empty;

  auto const checkChanged = [&] (Type old, Type nue, Type changed) {
    auto const added = nue - old;
    if (added != Type::None) return added;
    return changed & nue;
  };

  // Check if the given entity (class or function) needs to run again
  // due to one of its dependencies changing (or if it previously
  // registered a new dependency).
  auto const check = [&] (SString name, DepState& d) {
    SCOPE_EXIT {
      if (d.newDeps) {
        d.deps = std::move(d.newDeps);
        d.newDeps.reset();
      }
      d.group.reset();
    };

    // The algorithm for these are all similar: Compare the old
    // dependencies with the new dependencies. If the dependency is
    // new, or if it's not the same as the old, check the
    // ClassGroup. If they're in the same ClassGroup, ignore it (this
    // entity already incorporated the change inside the analysis
    // job). Otherwise schedule this class or func to run.

    auto const& old = d.deps.has_value() ? *d.deps : empty;
    auto const& nue = d.newDeps.has_value() ? *d.newDeps : old;

    for (auto const [meth, newT] : nue.methods) {
      auto const state = folly::get_ptr(classState, meth.cls);
      if (!state) {
        auto const changed = checkChanged(
          folly::get_default(old.methods, meth, Type::None) |
          (old.classes.count(meth.cls) ? Type::Meta : Type::None),
          newT,
          Type::None
        );
        if (changed != Type::None) {
          FTRACE(
            4, "AnalysisScheduler: {} new/changed dependency on method {} ({}),"
            " scheduling\n",
            name, show(meth), show(changed)
          );
          return true;
        }
        continue;
      }
      if (d.group && d.group->classes.count(meth.cls)) {
        FTRACE(
          5, "AnalysisScheduler: ignoring method dependency {} -> {}\n",
          name, show(meth)
        );
        continue;
      }
      auto const changed = checkChanged(
        folly::get_default(old.methods, meth, Type::None) |
        (old.classes.count(meth.cls) ? Type::Meta : Type::None),
        newT,
        state->methodChanges.get_default(meth.idx, Type::None)
      );
      if (changed != Type::None) {
        FTRACE(
          4, "AnalysisScheduler: {} new/changed dependency on method {} ({}), "
          "scheduling\n",
          name, show(meth), show(changed)
        );
        return true;
      }
    }

    for (auto const cls : nue.classes) {
      if (!classState.count(cls)) {
        if (!old.classes.count(cls)) {
          FTRACE(
            4, "AnalysisScheduler: new class dependency {} -> {}\n",
            name, cls
          );
          return true;
        }
        continue;
      }
      if (d.group && d.group->classes.count(cls)) {
        FTRACE(
          5, "AnalysisScheduler: ignoring class dependency {} -> {}\n",
          name, cls
        );
        continue;
      }
      if (!old.classes.count(cls)) {
        FTRACE(
          4, "AnalysisScheduler: new class dependency {} -> {}\n",
          name, cls
        );
        return true;
      }
    }

    for (auto const [func, newT] : nue.funcs) {
      auto const state = folly::get_ptr(funcState, func);
      if (!state) {
        auto const changed = checkChanged(
          folly::get_default(old.funcs, func, Type::None),
          newT,
          Type::None
        );
        if (changed != Type::None) {
          FTRACE(
            4, "AnalysisScheduler: {} new/changed dependency on func {} ({}), "
            "scheduling\n",
            name, func, show(changed)
          );
          return true;
        }
        continue;
      }
      if (d.group && d.group->funcs.count(func)) {
        FTRACE(
          5, "AnalysisScheduler: ignoring func dependency {} -> {}\n",
          name, func
        );
        continue;
      }
      auto const changed = checkChanged(
        folly::get_default(old.funcs, func, Type::None),
        newT,
        state->changed
      );
      if (changed != Type::None) {
        FTRACE(
          4, "AnalysisScheduler: {} new/changed dependency on func {} ({}), "
          "scheduling\n",
          name, func, show(changed)
        );
        return true;
      }
    }

    for (auto const cns : nue.clsConstants) {
      auto const state = folly::get_ptr(classState, cns.cls);
      if (!state) {
        if (!old.clsConstants.count(cns) && !old.classes.count(cns.cls)) {
          FTRACE(
            4, "AnalysisScheduler: new class constant dependency {} -> {}\n",
            name, show(cns)
          );
          return true;
        }
        continue;
      }
      if (d.group && d.group->classes.count(cns.cls)) {
        FTRACE(
          5, "AnalysisScheduler: ignoring class constant dependency {} -> {}\n",
          name, show(cns)
        );
        continue;
      }
      if (!old.clsConstants.count(cns) && !old.classes.count(cns.cls)) {
        FTRACE(
          4, "AnalysisScheduler: new class constant dependency {} -> {}\n",
          name, show(cns)
        );
        return true;
      }
      if (cns.idx < state->cnsChanges.size() && state->cnsChanges[cns.idx]) {
        FTRACE(
          4, "AnalysisScheduler: {} depends on changed class constant {}, "
          "scheduling\n",
          name, show(cns)
        );
        return true;
      }
    }

    for (auto const cns : nue.constants) {
      auto const changed = folly::get_ptr(cnsChanged, cns);
      if (!changed) {
        if (!old.constants.count(cns) &&
            !old.funcs.count(Constant::funcNameFromName(cns))) {
          FTRACE(
            4, "AnalysisScheduler: new constant dependency {} -> {}\n",
            name, cns
          );
          return true;
        }
        continue;
      }
      auto const unit = folly::get_ptr(index.m_data->constantToUnit, cns);
      assertx(unit != nullptr);
      if (d.group && d.group->units.count(unit->first)) {
        FTRACE(
          5, "AnalysisScheduler: ignoring constant dependency {} -> {}\n",
          name, cns
        );
        continue;
      }
      if (!old.constants.count(cns) &&
          !old.funcs.count(Constant::funcNameFromName(cns))) {
        FTRACE(
          4, "AnalysisScheduler: new constant dependency {} -> {}\n",
          name, cns
        );
        return true;
      }
      if (changed->load(std::memory_order_acquire)) {
        FTRACE(
          4, "AnalysisScheduler: {} depends on changed constant {}, "
          "scheduling\n",
          name, cns
        );
        return true;
      }
    }

    for (auto const typeAlias : nue.typeAliases) {
      auto const unit =
        folly::get_default(index.m_data->typeAliasToUnit, typeAlias);
      if (unit && d.group && d.group->units.count(unit)) {
        FTRACE(
          5, "AnalysisScheduler: ignoring type-alias dependency {} -> {}\n",
          name, typeAlias
        );
        continue;
      }
      if (!old.typeAliases.count(typeAlias)) {
        FTRACE(
          4, "AnalysisScheduler: new type-alias dependency {} -> {}\n",
          name, typeAlias
        );
        return true;
      }
    }

    return false;
  };

  // The return type for these lambdas should be bool, but parallel
  // doesn't handle a bool return type properly (because it tries to
  // form pointers to elements of std::vector<bool>), so force it to
  // be larger.
  auto const classBits = parallel::map(
    classNames,
    [&] (SString name) -> uint8_t {
      return check(name, classState.at(name).depState);
    }
  );
  auto const funcBits = parallel::map(
    funcNames,
    [&] (SString name) -> uint8_t {
      return check(name, funcState.at(name).depState);
    }
  );

  // Turn the "bits" into names.
  for (size_t i = 0, size = classNames.size(); i < size; ++i) {
    if (classBits[i]) classesToSchedule.emplace(classNames[i]);
  }
  for (size_t i = 0, size = funcNames.size(); i < size; ++i) {
    if (funcBits[i]) funcsToSchedule.emplace(funcNames[i]);
  }
}

// Reset any recorded changes from analysis jobs, in preparation for
// another round.
void AnalysisScheduler::resetChanges() {
  parallel::for_each(
    classNames,
    [&] (SString name) {
      auto& state = classState.at(name);
      std::fill(
        begin(state.methodChanges),
        end(state.methodChanges),
        Type::None
      );
      state.cnsChanges.reset();
    }
  );
  parallel::for_each(
    funcNames,
    [&] (SString name) {
      funcState.at(name).changed = Type::None;
      if (auto const cns = Constant::nameFromFuncName(name)) {
        cnsChanged.at(cns).store(false, std::memory_order_release);
      }
    }
  );
}

// Called when all analysis jobs are finished. "Finalize" the changes
// and determine what should run in the next analysis round.
void AnalysisScheduler::recordingDone() {
  findToSchedule();
  removeFuncs();
  resetChanges();
}

void AnalysisScheduler::addClassToInput(SString name,
                                        AnalysisInput& input) const {
  input.classes.emplace(name, index.m_data->classRefs.at(name));
  input.classBC.emplace(name, index.m_data->classBytecodeRefs.at(name));
  if (auto const ref = folly::get_ptr(index.m_data->classInfoRefs, name)) {
    input.cinfos.emplace(name, ref->cast<AnalysisIndexCInfo>());
  } else if (auto const ref =
             folly::get_ptr(index.m_data->uninstantiableClsMethRefs, name)) {
    input.minfos.emplace(name, ref->cast<AnalysisIndexMInfo>());
    input.meta.badClasses.emplace(name);
  } else {
    input.meta.badClasses.emplace(name);
  }
  if (!input.m_key) input.m_key = name;
}

void AnalysisScheduler::addFuncToInput(SString name,
                                       AnalysisInput& input) const {
  input.funcs.emplace(name, index.m_data->funcRefs.at(name));
  input.funcBC.emplace(name, index.m_data->funcBytecodeRefs.at(name));
  input.finfos.emplace(
    name,
    index.m_data->funcInfoRefs.at(name).cast<AnalysisIndexFInfo>()
  );
  if (!input.m_key) input.m_key = name;
}

void AnalysisScheduler::addUnitToInput(SString name,
                                       AnalysisInput& input) const {
  input.units.emplace(name, index.m_data->unitRefs.at(name));
  if (!input.m_key) input.m_key = name;
}

void AnalysisScheduler::addDepClassToInput(SString cls,
                                           SString depSrc,
                                           bool addBytecode,
                                           AnalysisInput& input) const {
  if (input.classes.count(cls)) return;

  auto const badClass = [&] {
    if (input.meta.badClasses.emplace(cls).second) {
      FTRACE(4, "AnalysisScheduler: adding bad class {} to {} dep inputs\n",
             cls, depSrc);
    }
  };

  auto const clsRef = folly::get_ptr(index.m_data->classRefs, cls);
  if (!clsRef) return badClass();

  if (!input.depClasses.emplace(cls, *clsRef).second) {
    // If we already added the class and don't need to add the
    // bytecode, nothing more to do.
    if (!addBytecode) return;
  } else {
    FTRACE(
      4, "AnalysisScheduler: adding class {} to {} dep inputs\n",
      cls, depSrc
    );
  }

  if (auto const r = folly::get_ptr(index.m_data->classInfoRefs, cls)) {
    input.cinfos.emplace(cls, r->cast<AnalysisIndexCInfo>());
  } else if (auto const r =
             folly::get_ptr(index.m_data->uninstantiableClsMethRefs, cls)) {
    input.minfos.emplace(cls, r->cast<AnalysisIndexMInfo>());
    badClass();
  } else {
    badClass();
  }

  if (addBytecode) {
    if (input.classBC.emplace(cls,
                              index.m_data->classBytecodeRefs.at(cls)).second) {
      FTRACE(
        4, "AnalysisScheduler: adding class {} bytecode to {} dep inputs\n",
        cls, depSrc
      );
    }
  }

  addDepUnitToInput(index.m_data->classToUnit.at(cls), depSrc, input);

  auto const& closures = folly::get_default(index.m_data->classToClosures, cls);
  for (auto const clo : closures) {
    addDepClassToInput(clo, depSrc, addBytecode, input);
  }
}

void AnalysisScheduler::addDepFuncToInput(SString func,
                                          SString depSrc,
                                          Type type,
                                          AnalysisInput& input) const {
  assertx(type != Type::None);
  if (input.funcs.count(func)) return;

  auto const funcRef = folly::get_ptr(index.m_data->funcRefs, func);
  if (!funcRef) {
    if (input.meta.badFuncs.emplace(func).second) {
      FTRACE(4, "AnalysisScheduler: adding bad func {} to {} dep inputs\n",
             func, depSrc);
    }
    return;
  }

  if (!input.depFuncs.emplace(func, *funcRef).second) {
    // If we already added the func and don't need to add the
    // bytecode, nothing more to do.
    if (!(type & Type::Bytecode)) return;
  } else {
    FTRACE(
      4, "AnalysisScheduler: adding func {} to {} dep inputs\n",
      func, depSrc
    );
  }

  input.finfos.emplace(
    func,
    index.m_data->funcInfoRefs.at(func).cast<AnalysisIndexFInfo>()
  );

  if (type & Type::Bytecode) {
    if (input.funcBC.emplace(func,
                             index.m_data->funcBytecodeRefs.at(func)).second) {
      FTRACE(
        4, "AnalysisScheduler: adding func {} bytecode to {} dep inputs\n",
        func, depSrc
      );
    }
  }

  addDepUnitToInput(index.m_data->funcToUnit.at(func), depSrc, input);

  auto const& closures = folly::get_default(index.m_data->funcToClosures, func);
  for (auto const clo : closures) {
    addDepClassToInput(clo, depSrc, type & Type::Bytecode, input);
  }
}

void AnalysisScheduler::addDepConstantToInput(SString cns,
                                              SString depSrc,
                                              AnalysisInput& input) const {
  auto const unit = folly::get_ptr(index.m_data->constantToUnit, cns);
  if (!unit) {
    if (input.meta.badConstants.emplace(cns).second) {
      FTRACE(4, "AnalysisScheduler: adding bad constant {} to {} dep inputs\n",
             cns, depSrc);
    }
    return;
  }

  addDepUnitToInput(unit->first, depSrc, input);
  if (!unit->second || is_native_unit(unit->first)) return;

  auto const initName = Constant::funcNameFromName(cns);
  always_assert_flog(
    index.m_data->funcRefs.count(initName),
    "Constant {} is missing expected initialization function {}",
    cns, initName
  );

  addDepFuncToInput(initName, depSrc, Type::Meta, input);
}

void AnalysisScheduler::addDepUnitToInput(SString unit,
                                          SString depSrc,
                                          AnalysisInput& input) const {
  if (input.units.count(unit)) return;
  if (!input.depUnits.emplace(unit, index.m_data->unitRefs.at(unit)).second) {
    return;
  }
  FTRACE(4, "AnalysisScheduler: adding unit {} to {} dep inputs\n",
         unit, depSrc);
}

void AnalysisScheduler::addDepTypeAliasToInput(SString typeAlias,
                                               SString depSrc,
                                               AnalysisInput& input) const {
  auto const unit =
    folly::get_default(index.m_data->typeAliasToUnit, typeAlias);
  if (!unit) {
    if (input.meta.badTypeAliases.emplace(typeAlias).second) {
      FTRACE(4, "AnalysisScheduler: adding bad type-alias {} to {} dep inputs\n",
             typeAlias, depSrc);
    }
    return;
  }
  addDepUnitToInput(unit, depSrc, input);
}

// For every input in the AnalysisInput, add any associated
// dependencies for those inputs.
void AnalysisScheduler::addDepsToInput(AnalysisInput& input) const {
  auto const add = [&] (SString n,
                        const DepState& depState,
                        SString unit,
                        const TSStringSet& closures) {
    for (auto const clo : closures) {
      addDepClassToInput(clo, n, true, input);
    }
    addDepUnitToInput(unit, n, input);
    if (!depState.deps.has_value()) return;
    auto const& d = *depState.deps;
    for (auto const cls : d.classes) {
      addDepClassToInput(cls, n, false, input);
    }
    for (auto const [meth, type] : d.methods) {
      if (type != Type::None) {
        addDepClassToInput(
          meth.cls,
          n,
          type & Type::Bytecode,
          input
        );
      }
    }
    for (auto const [func, type] : d.funcs) {
      if (type != Type::None) addDepFuncToInput(func, n, type, input);
    }
    for (auto const cns : d.clsConstants) {
      addDepClassToInput(cns.cls, n, false, input);
    }
    for (auto const cns : d.constants) {
      addDepConstantToInput(cns, n, input);
    }
    for (auto const typeAlias : d.typeAliases) {
      addDepTypeAliasToInput(typeAlias, n, input);
    }
  };
  for (auto const& [name, _] : input.classes) {
    add(
      name,
      classState.at(name).depState,
      index.m_data->classToUnit.at(name),
      folly::get_default(index.m_data->classToClosures, name)
    );
  }
  for (auto const& [name, _] : input.funcs) {
    add(
      name,
      funcState.at(name).depState,
      index.m_data->funcToUnit.at(name),
      folly::get_default(index.m_data->funcToClosures, name)
    );
  }
}

// Group the work that needs to run into buckets of the given size.
std::vector<AnalysisInput> AnalysisScheduler::schedule(size_t bucketSize) {
  FTRACE(2, "AnalysisScheduler: scheduling {} items into buckets of size {}\n",
         workItems(), bucketSize);

  std::vector<SString> items{
    begin(classesToSchedule),
    end(classesToSchedule)
  };

  SStringToOneT<FSStringSet> unitsToFuncs;
  for (auto const func : funcsToSchedule) {
    if (Constant::nameFromFuncName(func)) {
      auto const unit = index.m_data->funcToUnit.at(func);
      if (auto funcs = folly::get_ptr(unitsToFuncs, unit)) {
        funcs->emplace(func);
      } else {
        items.emplace_back(unit);
        unitsToFuncs[unit].emplace(func);
      }
    } else {
      items.emplace_back(func);
    }
  }
  if (items.empty()) return {};

  // Turn the buckets of names into buckets of AnalysisInput.
  auto const inputs = parallel::map(
    consistently_bucketize(items, bucketSize),
    [&] (const std::vector<SString>& bucket) {
      AnalysisInput input;
      // Add all the inputs. These are the items which will actually
      // be processed in the job.
      for (auto const item : bucket) {
        if (classesToSchedule.count(item)) addClassToInput(item, input);
        if (funcsToSchedule.count(item))   addFuncToInput(item, input);
        if (auto const funcs = folly::get_ptr(unitsToFuncs, item)) {
          for (auto const f : *funcs) addFuncToInput(f, input);
          addUnitToInput(item, input);
        }
      }
      // Add any dependencies for the above inputs. These won't be
      // processed in the job, but they're needed to process the above
      // inputs.
      addDepsToInput(input);
      return input;
    }
  );

  classesToSchedule.clear();
  funcsToSchedule.clear();
  FTRACE(2, "AnalysisScheduler: scheduled {} buckets\n", inputs.size());
  return inputs;
}

//////////////////////////////////////////////////////////////////////

namespace {

// If we optimized a top-level constant's value to a scalar, we no
// longer need the associated 86cinit function. This fixes up the
// metadata to remove it.
FSStringSet strip_unneeded_constant_inits(AnalysisIndex::IndexData& index) {
  FSStringSet stripped;
  for (auto const name : index.funcNames) {
    auto const cnsName = Constant::nameFromFuncName(name);
    if (!cnsName) continue;
    auto const it = index.constants.find(cnsName);
    if (it == end(index.constants)) continue;
    auto const& cns = *it->second;
    if (type(cns.val) == KindOfUninit) continue;
    stripped.emplace(name);
  }
  if (stripped.empty()) return stripped;

  for (auto const name : stripped) {
    index.deps->getChanges().remove(*index.funcs.at(name));
    index.funcs.erase(name);
    index.finfos.erase(name);
  }

  index.funcNames.erase(
    std::remove_if(
      begin(index.funcNames), end(index.funcNames),
      [&] (SString f) { return stripped.count(f); }
    ),
    end(index.funcNames)
  );

  for (auto& [_, unit] : index.units) {
    unit->funcs.erase(
      std::remove_if(
        begin(unit->funcs), end(unit->funcs),
        [&] (SString f) { return stripped.count(f); }
      ),
      end(unit->funcs)
    );
  }

  return stripped;
}

}

//////////////////////////////////////////////////////////////////////

AnalysisIndex::AnalysisIndex(
  AnalysisWorklist& worklist,
  VU<php::Class> classes,
  VU<php::Func> funcs,
  VU<php::Unit> units,
  VU<php::ClassBytecode> clsBC,
  VU<php::FuncBytecode> funcBC,
  V<AnalysisIndexCInfo> cinfos,
  V<AnalysisIndexFInfo> finfos,
  V<AnalysisIndexMInfo> minfos,
  VU<php::Class> depClasses,
  VU<php::Func> depFuncs,
  VU<php::Unit> depUnits,
  AnalysisInput::Meta meta
) : m_data{std::make_unique<IndexData>(*this, worklist)}
{
  m_data->badClasses = std::move(meta.badClasses);
  m_data->badFuncs = std::move(meta.badFuncs);
  m_data->badConstants = std::move(meta.badConstants);
  m_data->badTypeAliases = std::move(meta.badTypeAliases);

  TSStringSet depClassNames;
  depClassNames.reserve(depClasses.size());

  m_data->classNames.reserve(classes.size());
  m_data->classes.reserve(classes.size() + depClasses.size());
  for (auto& cls : classes) {
    auto const name = cls->name;
    always_assert(
      m_data->classes.emplace(name, std::move(cls)).second
    );
    m_data->classNames.emplace_back(name);
  }
  for (auto& cls : depClasses) {
    auto const name = cls->name;
    always_assert(
      m_data->classes.emplace(name, std::move(cls)).second
    );
    depClassNames.emplace(name);
  }

  FSStringSet depFuncNames;
  depFuncNames.reserve(depFuncs.size());

  m_data->funcNames.reserve(funcs.size());
  m_data->funcs.reserve(funcs.size() + depFuncs.size());
  for (auto& func : funcs) {
    auto const name = func->name;
    always_assert(
      m_data->funcs.emplace(name, std::move(func)).second
    );
    m_data->funcNames.emplace_back(name);
  }
  for (auto& func : depFuncs) {
    auto const name = func->name;
    always_assert(
      m_data->funcs.emplace(name, std::move(func)).second
    );
    depFuncNames.emplace(name);
  }

  auto const assignFInfoIdx = [&] (php::Func& func, FuncInfo2& finfo) {
    always_assert(func.idx == std::numeric_limits<uint32_t>::max());
    always_assert(!finfo.func);
    finfo.func = &func;
    func.idx = m_data->finfosByIdx.size();
    m_data->finfosByIdx.emplace_back(&finfo);
  };

  m_data->cinfos.reserve(cinfos.size());
  for (auto& wrapper : cinfos) {
    auto cinfo = wrapper.ptr.release();
    auto const name = cinfo->name;
    auto& cls = m_data->classes.at(name);
    always_assert(!cls->cinfo);
    always_assert(!cinfo->cls);
    cls->cinfo = cinfo;
    cinfo->cls = cls.get();

    auto const numMethods = cls->methods.size();
    always_assert(cinfo->funcInfos.size() == numMethods);
    for (size_t i = 0; i < numMethods; ++i) {
      assignFInfoIdx(*cls->methods[i], *cinfo->funcInfos[i]);
    }

    always_assert(m_data->cinfos.emplace(name, cinfo).second);
  }

  m_data->finfos.reserve(finfos.size());
  for (auto& wrapper : finfos) {
    auto finfo = wrapper.ptr.release();
    auto const name = finfo->name;
    auto& func = m_data->funcs.at(name);
    assignFInfoIdx(*func, *finfo);
    always_assert(m_data->finfos.emplace(name, finfo).second);
  }

  m_data->minfos.reserve(minfos.size());
  for (auto& wrapper : minfos) {
    auto minfo = wrapper.ptr.release();
    auto const name = minfo->cls;
    auto& cls = m_data->classes.at(name);
    always_assert(!cls->cinfo);
    auto const numMethods = cls->methods.size();
    always_assert(minfo->finfos.size() == numMethods);
    for (size_t i = 0; i < numMethods; ++i) {
      assignFInfoIdx(*cls->methods[i], *minfo->finfos[i]);
    }
    always_assert(m_data->minfos.emplace(name, minfo).second);
  }

  for (auto& bc : clsBC) {
    auto& cls = m_data->classes.at(bc->cls);
    always_assert(bc->methodBCs.size() == cls->methods.size());
    for (size_t i = 0, size = bc->methodBCs.size(); i < size; ++i) {
      always_assert(bc->methodBCs[i].name == cls->methods[i]->name); // XXX split diff
      cls->methods[i]->rawBlocks = std::move(bc->methodBCs[i].bc);
    }
  }
  for (auto& bc : funcBC) {
    auto& func = m_data->funcs.at(bc->name);
    func->rawBlocks = std::move(bc->bc);
  }

  m_data->unitNames.reserve(units.size());
  m_data->units.reserve(units.size() + depUnits.size());

  auto const addUnit = [&] (std::unique_ptr<php::Unit> unit) {
    auto const isNative = is_native_unit(*unit);
    for (auto& cns : unit->constants) {
      always_assert(m_data->constants.emplace(cns->name, cns.get()).second);
      assertx(!m_data->badConstants.count(cns->name));
      if (isNative && type(cns->val) == KindOfUninit) {
        m_data->dynamicConstants.emplace(cns->name);
      }
    }
    for (auto& typeAlias : unit->typeAliases) {
      always_assert(
        m_data->typeAliases.emplace(typeAlias->name, typeAlias.get()).second
      );
      assertx(!m_data->badTypeAliases.count(typeAlias->name));
    }
    auto const name = unit->filename;
    always_assert(m_data->units.emplace(name, std::move(unit)).second);
  };
  for (auto& unit : units) {
    m_data->unitNames.emplace_back(unit->filename);
    addUnit(std::move(unit));
  }
  for (auto& unit : depUnits) addUnit(std::move(unit));

  for (auto const& [_, cls] : m_data->classes) {
    if (!cls->cinfo) continue;
    always_assert(cls.get() == cls->cinfo->cls);
  }
  for (auto& [_, cinfo]: m_data->cinfos) {
    always_assert(cinfo->cls);
    always_assert(cinfo.get() == cinfo->cls->cinfo);
  }
  for (size_t i = 0, size = m_data->finfosByIdx.size(); i < size; ++i) {
    auto finfo = m_data->finfosByIdx[i];
    always_assert(finfo->func);
    always_assert(finfo->func->idx == i);
  }

  ClassGraph::setAnalysisIndex(*m_data);
}

AnalysisIndex::~AnalysisIndex() {
  ClassGraph::clearAnalysisIndex();
}

void AnalysisIndex::start() { ClassGraph::init(); }
void AnalysisIndex::stop()  { ClassGraph::destroy(); }

void AnalysisIndex::push_context(const Context& ctx) {
  m_data->contexts.emplace_back(ctx);
}

void AnalysisIndex::pop_context() {
  assertx(!m_data->contexts.empty());
  m_data->contexts.pop_back();
}

void AnalysisIndex::freeze() {
  FTRACE(2, "Freezing index...\n");
  assertx(!m_data->frozen);
  m_data->frozen = true;
}

const php::Unit& AnalysisIndex::lookup_func_unit(const php::Func& f) const {
  auto const it = m_data->units.find(f.unit);
  always_assert_flog(
    it != end(m_data->units),
    "Attempting to access missing unit {} for func {}",
    f.unit, func_fullname(f)
  );
  return *it->second;
}

const php::Unit& AnalysisIndex::lookup_class_unit(const php::Class& c) const {
  auto const it = m_data->units.find(c.unit);
  always_assert_flog(
    it != end(m_data->units),
    "Attempting to access missing unit {} for class {}",
    c.unit, c.name
  );
  return *it->second;
}

const php::Class*
AnalysisIndex::lookup_const_class(const php::Const& cns) const {
  m_data->deps->add(AnalysisDeps::Class { cns.cls });
  if (auto const c = folly::get_ptr(m_data->classes, cns.cls)) {
    return c->get();
  }
  return nullptr;
}

const php::Class&
AnalysisIndex::lookup_closure_context(const php::Class& cls) const {
  always_assert_flog(
    !cls.closureContextCls,
    "AnalysisIndex does not yet support closure contexts (for {})",
    cls.name
  );
  return cls;
}

res::Func AnalysisIndex::resolve_func(SString n) const {
  n = normalizeNS(n);
  m_data->deps->add(AnalysisDeps::Func { n });
  if (auto const finfo = folly::get_ptr(m_data->finfos, n)) {
    return res::Func { res::Func::Fun2 { finfo->get() } };
  }
  if (m_data->badFuncs.count(n)) {
    return res::Func { res::Func::MissingFunc { n } };
  }
  return res::Func { res::Func::FuncName { n } };
}

Optional<res::Class> AnalysisIndex::resolve_class(SString n) const {
  n = normalizeNS(n);
  if (m_data->typeAliases.count(n)) {
    // If it's a type-alias it's definitely not a class, so no need to
    // go further.
    m_data->deps->add(AnalysisDeps::TypeAlias { n });
    return std::nullopt;
  }
  m_data->deps->add(AnalysisDeps::Class { n });
  if (auto const cinfo = folly::get_ptr(m_data->cinfos, n)) {
    return res::Class::get(*cinfo->get());
  }
  if (m_data->badClasses.count(n)) return std::nullopt;
  return res::Class::getOrCreate(n);
}

Optional<res::Class> AnalysisIndex::resolve_class(const php::Class& cls) const {
  m_data->deps->add(AnalysisDeps::Class { cls.name });
  if (cls.cinfo) return res::Class::get(*cls.cinfo);
  return std::nullopt;
}

res::Func AnalysisIndex::resolve_func_or_method(const php::Func& f) const {
  m_data->deps->add(f);
  if (!f.cls) return res::Func { res::Func::Fun2 { &func_info(*m_data, f) } };
  return res::Func { res::Func::Method2 { &func_info(*m_data, f) } };
}

Type AnalysisIndex::lookup_constant(SString name) const {
  m_data->deps->add(AnalysisDeps::Constant { name });

  if (auto const cns = folly::get_default(m_data->constants, name, nullptr)) {
    if (type(cns->val) != KindOfUninit) return from_cell(cns->val);
    if (m_data->dynamicConstants.count(name)) return TInitCell;
    auto const fname = Constant::funcNameFromName(name);
    auto const fit = m_data->finfos.find(fname);
    // We might have the unit present by chance, but without an
    // explicit dependence on the constant, we might not have the init
    // func present.
    if (fit == end(m_data->finfos)) return TInitCell;
    return unctx(fit->second->returnTy);
  }
  return m_data->badConstants.count(name) ? TBottom : TInitCell;
}

std::vector<std::pair<SString, ClsConstInfo>>
AnalysisIndex::lookup_class_constants(const php::Class& cls) const {
  std::vector<std::pair<SString, ClsConstInfo>> out;
  out.reserve(cls.constants.size());
  for (auto const& cns : cls.constants) {
    if (cns.kind != ConstModifiers::Kind::Value) continue;
    if (!cns.val) continue;
    if (cns.val->m_type != KindOfUninit) {
      out.emplace_back(cns.name, ClsConstInfo{ from_cell(*cns.val), 0 });
    } else if (!cls.cinfo) {
      out.emplace_back(cns.name, ClsConstInfo{ TInitCell, 0 });
    } else {
      out.emplace_back(
        cns.name,
        folly::get_default(
          cls.cinfo->clsConstantInfo,
          cns.name,
          ClsConstInfo{ TInitCell, 0 }
        )
      );
    }
  }
  return out;
}

ClsConstLookupResult
AnalysisIndex::lookup_class_constant(const Type& cls,
                                     const Type& name) const {
  ITRACE(4, "lookup_class_constant: ({}) {}::{}\n",
         show(current_context(*m_data)), show(cls), show(name));
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

  auto const& dcls = dcls_of(cls);
  if (dcls.isExact()) {
    auto const rcls = dcls.cls();
    m_data->deps->add(AnalysisDeps::Class { rcls.name() });

    auto const cinfo = rcls.cinfo2();
    if (!cinfo) return conservative();

    ITRACE(4, "{}:\n", cinfo->name);
    Trace::Indent _;

    auto const idxIt = cinfo->clsConstants.find(sname);
    if (idxIt == end(cinfo->clsConstants)) return notFound();
    auto const& idx = idxIt->second;

    assertx(!m_data->badClasses.count(idx.cls));

    m_data->deps->add(AnalysisDeps::Class { idx.cls });
    auto const cnsClsIt = m_data->classes.find(idx.cls);
    if (cnsClsIt == end(m_data->classes)) return conservative();
    auto const& cnsCls = cnsClsIt->second;

    assertx(idx.idx < cnsCls->constants.size());
    auto const& cns = cnsCls->constants[idx.idx];
    if (cns.kind != ConstModifiers::Kind::Value || !cns.val.has_value()) {
      return notFound();
    }

    auto const r = [&] {
      if (type(*cns.val) != KindOfUninit) {
        // Fully resolved constant with a known value. We don't need
        // to register a dependency on the constant because the value
        // will never change.
        auto mightThrow = bool(cinfo->cls->attrs & AttrInternal);
        if (!mightThrow) {
          auto const& unit = lookup_class_unit(*cinfo->cls);
          auto const moduleName = unit.moduleName;
          auto const packageInfo = unit.packageInfo;
          if (auto const activeDeployment = packageInfo.getActiveDeployment()) {
            if (!packageInfo.moduleInDeployment(
                  moduleName, *activeDeployment, DeployKind::Hard)) {
              mightThrow = true;
            }
          }
        }
        return R{ from_cell(*cns.val), TriBool::Yes, mightThrow };
      }

      ITRACE(4, "(dynamic)\n");
      m_data->deps->add(idx);
      if (!cnsCls->cinfo) return conservative();
      auto const info =
        folly::get_ptr(cnsCls->cinfo->clsConstantInfo, cns.name);
      return R{
        info ? info->type : TInitCell,
        TriBool::Yes,
        true
      };
    }();
    ITRACE(4, "-> {}\n", show(r));
    return r;
  }

  // Subclasses not yet implemented
  return conservative();
}

ClsTypeConstLookupResult
AnalysisIndex::lookup_class_type_constant(
  const Type& cls,
  const Type& name,
  const Index::ClsTypeConstLookupResolver& resolver
) const {
  ITRACE(4, "lookup_class_type_constant: ({}) {}::{}\n",
         show(current_context(*m_data)), show(cls), show(name));
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

  auto const& dcls = dcls_of(cls);
  if (dcls.isExact()) {
    auto const rcls = dcls.cls();
    m_data->deps->add(AnalysisDeps::Class { rcls.name() });

    auto const cinfo = rcls.cinfo2();
    if (!cinfo) return conservative();

    ITRACE(4, "{}:\n", cinfo->name);
    Trace::Indent _;

    auto const idxIt = cinfo->clsConstants.find(sname);
    if (idxIt == end(cinfo->clsConstants)) return notFound();
    auto const& idx = idxIt->second;

    assertx(!m_data->badClasses.count(idx.cls));

    m_data->deps->add(AnalysisDeps::Class { idx.cls });
    auto const cnsClsIt = m_data->classes.find(idx.cls);
    if (cnsClsIt == end(m_data->classes)) return conservative();
    auto const& cnsCls = cnsClsIt->second;

    assertx(idx.idx < cnsCls->constants.size());
    auto const& cns = cnsCls->constants[idx.idx];
    if (cns.kind != ConstModifiers::Kind::Type) return notFound();
    if (!cns.val.has_value()) return abstract();

    assertx(tvIsDict(*cns.val));
    ITRACE(4, "({}) {}\n", cns.cls, show(dict_val(val(*cns.val).parr)));

    // If we've been given a resolver, use it. Otherwise resolve it in
    // the normal way.
    auto resolved = resolver
      ? resolver(cns, *cinfo->cls)
      : resolve_type_structure(
        AnalysisIndexAdaptor { *this }, cns, *cinfo->cls
      );

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
  }

  // Subclasses not yet implemented
  return conservative();
}

PropState AnalysisIndex::lookup_private_props(const php::Class& cls) const {
  // Private property tracking not yet implemented, so be
  // conservative.
  return make_unknown_propstate(
    AnalysisIndexAdaptor { *this },
    cls,
    [&] (const php::Prop& prop) {
      return (prop.attrs & AttrPrivate) && !(prop.attrs & AttrStatic);
    }
  );
}

PropState AnalysisIndex::lookup_private_statics(const php::Class& cls) const {
  // Private static property tracking not yet implemented, so be
  // conservative.
  return make_unknown_propstate(
    AnalysisIndexAdaptor { *this },
    cls,
    [&] (const php::Prop& prop) {
      return (prop.attrs & AttrPrivate) && (prop.attrs & AttrStatic);
    }
  );
}

Index::ReturnType AnalysisIndex::lookup_return_type(MethodsInfo* methods,
                                                    res::Func rfunc) const {
  using R = Index::ReturnType;

  auto const meth = [&] (const FuncInfo2& finfo) {
    if (methods) {
      if (auto ret = methods->lookupReturnType(*finfo.func)) {
        return R{ unctx(std::move(ret->t)), ret->effectFree };
      }
    }
    m_data->deps->add(*finfo.func, AnalysisDeps::Type::RetType);
    return R{ unctx(finfo.returnTy), finfo.effectFree };
  };

  return match<R>(
    rfunc.val,
    [&] (res::Func::FuncName f) {
      m_data->deps->add(
        AnalysisDeps::Func { f.name },
        AnalysisDeps::Type::RetType
      );
      return R{ TInitCell, false };
    },
    [&] (res::Func::MethodName m) {
      // If we know the name of the class, we can register a
      // dependency on it. If not, nothing we can do.
      if (m.cls) m_data->deps->add(AnalysisDeps::Class { m.cls });
      return R{ TInitCell, false };
    },
    [&] (res::Func::Fun)                -> R { always_assert(false); },
    [&] (res::Func::Method)             -> R { always_assert(false); },
    [&] (res::Func::MethodFamily)       -> R { always_assert(false); },
    [&] (res::Func::MethodOrMissing)    -> R { always_assert(false); },
    [&] (res::Func::MissingFunc)        { return R{ TBottom, false }; },
    [&] (res::Func::MissingMethod)      { return R{ TBottom, false }; },
    [&] (const res::Func::Isect&)       -> R { always_assert(false); },
    [&] (res::Func::Fun2 f) {
      m_data->deps->add(*f.finfo->func, AnalysisDeps::Type::RetType);
      return R{ unctx(f.finfo->returnTy), f.finfo->effectFree };
    },
    [&] (res::Func::Method2 m)          { return meth(*m.finfo); },
    [&] (res::Func::MethodFamily2)      -> R { always_assert(false); },
    [&] (res::Func::MethodOrMissing2 m) { return meth(*m.finfo); },
    [&] (const res::Func::Isect2&)      -> R { always_assert(false); }
  );
}

Index::ReturnType
AnalysisIndex::lookup_return_type(MethodsInfo* methods,
                                  const CompactVector<Type>& args,
                                  const Type& context,
                                  res::Func rfunc) const {
  using R = Index::ReturnType;

  auto ty = lookup_return_type(methods, rfunc);

  auto const contextual = [&] (const FuncInfo2& finfo) {
    return context_sensitive_return_type(
      *m_data,
      { finfo.func, args, context },
      std::move(ty)
    );
  };

  return match<R>(
    rfunc.val,
    [&] (res::Func::FuncName)           { return ty; },
    [&] (res::Func::MethodName)         { return ty; },
    [&] (res::Func::Fun)                -> R { always_assert(false); },
    [&] (res::Func::Method)             -> R { always_assert(false); },
    [&] (res::Func::MethodFamily)       -> R { always_assert(false); },
    [&] (res::Func::MethodOrMissing)    -> R { always_assert(false); },
    [&] (res::Func::MissingFunc)        { return R{ TBottom, false }; },
    [&] (res::Func::MissingMethod)      { return R{ TBottom, false }; },
    [&] (const res::Func::Isect&)       -> R { always_assert(false); },
    [&] (res::Func::Fun2 f)             { return contextual(*f.finfo); },
    [&] (res::Func::Method2 m)          { return contextual(*m.finfo); },
    [&] (res::Func::MethodFamily2)      -> R { always_assert(false); },
    [&] (res::Func::MethodOrMissing2 m) { return contextual(*m.finfo); },
    [&] (const res::Func::Isect2&)      -> R { always_assert(false); }
  );
}

Index::ReturnType
AnalysisIndex::lookup_foldable_return_type(const CallContext& calleeCtx) const {
  constexpr size_t maxNestingLevel = 2;

  using R = Index::ReturnType;

  auto const& func = *calleeCtx.callee;
  auto const ctxType =
    adjust_closure_context(AnalysisIndexAdaptor { *this }, calleeCtx);

  // Don't fold functions when staticness mismatches
  if (!func.isClosureBody) {
    if ((func.attrs & AttrStatic) && ctxType.couldBe(TObj)) {
      return R{ TInitCell, false };
    }
    if (!(func.attrs & AttrStatic) && ctxType.couldBe(TCls)) {
      return R{ TInitCell, false };
    }
  }

  auto const& finfo = func_info(*m_data, func);
  if (finfo.effectFree && is_scalar(finfo.returnTy)) {
    return R{ finfo.returnTy, finfo.effectFree };
  }

  auto const& caller = *context_for_deps(*m_data).func;

  m_data->deps->add(
    func,
    AnalysisDeps::Type::ScalarRetType |
    AnalysisDeps::Type::Bytecode
  );
  if (m_data->foldableInterpNestingLevel > maxNestingLevel) {
    return R{ TInitCell, false };
  }

  if (!func.rawBlocks) {
    ITRACE_MOD(
      Trace::hhbbc, 4,
      "Skipping inline interp of {} because bytecode not present\n",
      func_fullname(func)
    );
    return R{ TInitCell, false };
  }

  auto const contextualRet = [&] () -> Optional<Type> {
    ++m_data->foldableInterpNestingLevel;
    SCOPE_EXIT { --m_data->foldableInterpNestingLevel; };

    auto const wf = php::WideFunc::cns(&func);
    auto const fa = analyze_func_inline(
      AnalysisIndexAdaptor { *this },
      AnalysisContext {
        func.unit,
        wf,
        func.cls,
        &context_for_deps(*m_data)
      },
      ctxType,
      calleeCtx.args,
      nullptr,
      CollectionOpts::EffectFreeOnly
    );
    if (!fa.effectFree) return std::nullopt;
    return fa.inferredReturn;
  }();

  if (!contextualRet) {
    ITRACE_MOD(
      Trace::hhbbc, 4,
      "Foldable inline analysis failed due to possible side-effects\n"
    );
    return R{ TInitCell, false };
  }

  ITRACE_MOD(
    Trace::hhbbc, 4,
    "Foldable return type: {}\n",
    show(*contextualRet)
  );

  auto const error_context = [&] {
    using namespace folly::gen;
    return folly::sformat(
      "{} calling {} (context: {}, args: {})",
      func_fullname(caller),
      func_fullname(func),
      show(calleeCtx.context),
      from(calleeCtx.args)
        | map([] (const Type& t) { return show(t); })
        | unsplit<std::string>(",")
    );
  };

  always_assert_flog(
    contextualRet->subtypeOf(finfo.returnTy),
    "Context sensitive return type for {} is {} "
    "which not at least as refined as context insensitive "
    "return type {}\n",
    error_context(),
    show(*contextualRet),
    show(finfo.returnTy)
  );
  if (!is_scalar(*contextualRet)) return R{ TInitCell, false };

  return R{ *contextualRet, true };
}

std::pair<Index::ReturnType, size_t>
AnalysisIndex::lookup_return_type_raw(const php::Func& f) const {
  auto const& finfo = func_info(*m_data, f);
  return std::make_pair(
    Index::ReturnType{ finfo.returnTy, finfo.effectFree },
    finfo.returnRefinements
  );
}

bool AnalysisIndex::func_depends_on_arg(const php::Func& func,
                                        size_t arg) const {
  m_data->deps->add(func, AnalysisDeps::Type::UnusedParams);
  auto const& finfo = func_info(*m_data, func);
  return arg >= finfo.unusedParams.size() || !finfo.unusedParams.test(arg);
}

res::Func AnalysisIndex::resolve_method(const Type& thisType,
                                        SString name) const {
  assertx(thisType.subtypeOf(BCls) || thisType.subtypeOf(BObj));

  using Func = res::Func;

  auto const general = [&] (SString maybeCls) {
    assertx(name != s_construct.get());
    return Func { Func::MethodName { maybeCls, name } };
  };

  auto const isClass = thisType.subtypeOf(BCls);
  if (name == s_construct.get()) {
    if (isClass) {
      return Func { Func::MethodName { nullptr, s_construct.get() } };
    }
    return resolve_ctor(thisType);
  }

  if (isClass) {
    if (!is_specialized_cls(thisType)) return general(nullptr);
  } else if (!is_specialized_obj(thisType)) {
    return general(nullptr);
  }

  // Be pessimistic for intersections right now
  auto const& dcls = isClass ? dcls_of(thisType) : dobj_of(thisType);
  if (dcls.isIsect()) return general(dcls.smallestCls().name());

  auto const rcls = dcls.cls();
  m_data->deps->add(AnalysisDeps::Class { rcls.name() });

  auto const cinfo = rcls.cinfo2();
  if (!cinfo) return general(rcls.name());

  auto const isExact = dcls.isExact();

  auto const meth = folly::get_ptr(cinfo->methods, name);
  if (!meth) {
    // We don't store metadata for special methods, so be pessimistic
    // (the lack of a method entry does not mean the call might fail
    // at runtme).
    if (is_special_method_name(name)) {
      return Func { Func::MethodName { cinfo->name, name } };
    }
    // We're only considering this class, not it's subclasses. Since
    // it doesn't exist here, the resolution will always fail.
    if (isExact) {
      return Func { Func::MissingMethod { cinfo->name, name } };
    }
    // The method isn't present on this class, but it might be in the
    // subclasses. In most cases try a general lookup to get a
    // slightly better type than nothing.
    return general(cinfo->name);
  }

  m_data->deps->add(meth->meth());
  auto const func = func_from_meth_ref(*m_data, meth->meth());
  if (!func) return general(cinfo->name);

  // We don't store method family information about special methods
  // and they have special inheritance semantics.
  if (is_special_method_name(name)) {
    // If we know the class exactly, we can use ftarget.
    if (isExact) {
      return Func { Func::Method2 { &func_info(*m_data, *func) } };
    }
    // The method isn't overwritten, but they don't inherit, so it
    // could be missing.
    if (meth->attrs & AttrNoOverride) {
      return Func { Func::MethodOrMissing2 { &func_info(*m_data, *func) } };
    }
    // Otherwise be pessimistic.
    return Func { Func::MethodName { cinfo->name, name } };
  }

  // Private method handling: Private methods have special lookup
  // rules. If we're in the context of a particular class, and that
  // class defines a private method, an instance of the class will
  // always call that private method (even if overridden) in that
  // context.
  assertx(cinfo->cls);
  auto const& ctx = current_context(*m_data);
  if (ctx.cls == cinfo->cls) {
    // The context matches the current class. If we've looked up a
    // private method (defined on this class), then that's what
    // we'll call.
    if ((meth->attrs & AttrPrivate) && meth->topLevel()) {
      return Func { Func::Method2 { &func_info(*m_data, *func) } };
    }
  } else if ((meth->attrs & AttrPrivate) || meth->hasPrivateAncestor()) {
    // Otherwise the context doesn't match the current class. If the
    // looked up method is private, or has a private ancestor, there's
    // a chance we'll call that method (or ancestor). Otherwise
    // there's no private method in the inheritance tree we'll call.
    auto conservative = false;
    auto const ancestor = [&] () -> const php::Func* {
      if (!ctx.cls) return nullptr;
      m_data->deps->add(AnalysisDeps::Class { ctx.cls->name });
      // Look up the ClassInfo corresponding to the context.
      auto const ctxCInfo = ctx.cls->cinfo;
      if (!ctxCInfo) return nullptr;
      // Is this context a parent of our class?
      if (!cinfo->classGraph.isChildOf(ctxCInfo->classGraph)) {
        return nullptr;
      }
      // It is. See if it defines a private method.
      auto const ctxMeth = folly::get_ptr(ctxCInfo->methods, name);
      if (!ctxMeth) return nullptr;
      // If it defines a private method, use it.
      if ((ctxMeth->attrs & AttrPrivate) && ctxMeth->topLevel()) {
        m_data->deps->add(ctxMeth->meth());
        auto const ctxFunc = func_from_meth_ref(*m_data, ctxMeth->meth());
        if (!ctxFunc) conservative = true;
        return ctxFunc;
      }
      // Otherwise do normal lookup.
      return nullptr;
    }();
    if (ancestor) {
      return Func { Func::Method2 { &func_info(*m_data, *ancestor) } };
    } else if (conservative) {
      return Func { Func::MethodName { cinfo->name, name } };
    }
  }

  // If we're only including regular subclasses, and this class itself
  // isn't regular, the result may not necessarily include func.
  if (!isClass && !is_regular_class(*cinfo->cls)) {
    // We're not including this base class. If we're exactly this
    // class, there's no method at all. It will always be missing.
    if (isExact) {
      return Func { Func::MissingMethod { cinfo->name, name } };
    }
    if (meth->noOverrideRegular()) {
      // The method isn't overridden in a subclass, but we can't
      // use the base class either. This leaves two cases. Either
      // the method isn't overridden because there are no regular
      // subclasses (in which case there's no resolution at all), or
      // because there's regular subclasses, but they use the same
      // method (in which case the result is just func).
      if (!cinfo->classGraph.mightHaveRegularSubclass()) {
        return Func { Func::MissingMethod { cinfo->name, name } };
      }
      return Func { Func::Method2 { &func_info(*m_data, *func) } };
    }
  } else if (isExact ||
             meth->attrs & AttrNoOverride ||
             (!isClass && meth->noOverrideRegular())) {
    // Either we want all classes, or the base class is regular. If
    // the method isn't overridden we know it must be just func (the
    // override bits include it being missing in a subclass, so we
    // know it cannot be missing either).
    return Func { Func::Method2 { &func_info(*m_data, *func) } };
  }

  // Be pessimistic for the rest of cases
  return general(cinfo->name);
}

res::Func AnalysisIndex::resolve_ctor(const Type& obj) const {
  assertx(obj.subtypeOf(BObj));

  using Func = res::Func;

  // Can't say anything useful if we don't know the object type.
  if (!is_specialized_obj(obj)) {
    return Func { Func::MethodName { nullptr, s_construct.get() } };
  }

  auto const& dcls = dobj_of(obj);
  // Non-exact case not yet implemented.
  always_assert_flog(
    dcls.isExact(),
    "Encountered non-exact class in resolve_ctor: {}",
    show(obj)
  );

  auto const rcls = dcls.cls();
  m_data->deps->add(AnalysisDeps::Class { rcls.name() });

  auto const cinfo = rcls.cinfo2();
  if (!cinfo) {
    return Func {
      Func::MethodName { rcls.name(), s_construct.get() }
    };
  }

  // See if this class has a ctor.
  auto const meth = folly::get_ptr(cinfo->methods, s_construct.get());
  if (!meth) {
    // There's no ctor on this class. This doesn't mean the ctor won't
    // exist at runtime, it might get the default ctor, so we have to
    // be conservative.
    return Func { Func::MethodName { cinfo->name, s_construct.get() } };
  }
  m_data->deps->add(meth->meth());

  // We have a ctor, but it might be overridden in a subclass.
  assertx(!(meth->attrs & AttrStatic));
  auto const func = func_from_meth_ref(*m_data, meth->meth());
  if (!func) {
    // Relevant function doesn't exist on the AnalysisIndex. Be
    // conservative.
    return Func { Func::MethodName { cinfo->name, s_construct.get() } };
  }
  assertx(!(func->attrs & AttrStatic));

  // If this class isn't regular, this resolution will always fail.
  if (!is_regular_class(*cinfo->cls)) {
    return Func { Func::MissingMethod { cinfo->name, s_construct.get() } };
  }
  return Func { Func::Method2 { &func_info(*m_data, *func) } };
}

std::pair<const php::TypeAlias*, bool>
AnalysisIndex::lookup_type_alias(SString name) const {
  if (m_data->classes.count(name)) {
    // If it's a class, it's definitely not a type-alias so we don't
    // need to go any further.
    m_data->deps->add(AnalysisDeps::Class { name });
    return std::make_pair(nullptr, false);
  }
  m_data->deps->add(AnalysisDeps::TypeAlias { name });
  if (auto const ta = folly::get_default(m_data->typeAliases, name)) {
    return std::make_pair(ta, true);
  }
  return std::make_pair(nullptr, !m_data->badTypeAliases.count(name));
}

Index::ClassOrTypeAlias
AnalysisIndex::lookup_class_or_type_alias(SString n) const {
  n = normalizeNS(n);
  if (auto const cls = folly::get_ptr(m_data->classes, n)) {
    m_data->deps->add(AnalysisDeps::Class { n });
    return Index::ClassOrTypeAlias{cls->get(), nullptr, true};
  }
  if (auto const ta = folly::get_default(m_data->typeAliases, n)) {
    m_data->deps->add(AnalysisDeps::TypeAlias { n });
    return Index::ClassOrTypeAlias{nullptr, ta, true};
  }

  // It could be either, so register a dependency on both.
  m_data->deps->add(AnalysisDeps::Class { n });
  m_data->deps->add(AnalysisDeps::TypeAlias { n });

  return Index::ClassOrTypeAlias{
    nullptr,
    nullptr,
    !m_data->badClasses.count(n) ||
    !m_data->badTypeAliases.count(n)
  };
}

PropMergeResult AnalysisIndex::merge_static_type(
    PublicSPropMutations& publicMutations,
    PropertiesInfo& privateProps,
    const Type& cls,
    const Type& name,
    const Type& val,
    bool checkUB,
    bool ignoreConst,
    bool mustBeReadOnly) const {
  // Not yet implemented
  return PropMergeResult{ TInitCell, TriBool::Maybe };
}

void AnalysisIndex::refine_constants(const FuncAnalysisResult& fa) {
  auto const& func = *fa.ctx.func;
  if (func.cls) return;

  auto const name = Constant::nameFromFuncName(func.name);
  if (!name) return;

  auto const cns = folly::get_default(m_data->constants, name);
  always_assert_flog(
    cns,
    "Attempting to refine constant {} "
    "which we don't have meta-data for",
    name
  );
  auto const val = tv(fa.inferredReturn);
  if (!val) {
    always_assert_flog(
      type(cns->val) == KindOfUninit,
      "Constant value invariant violated in {}.\n"
      "    Value went from {} to {}",
      name,
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
      name,
      show(from_cell(cns->val)),
      show(fa.inferredReturn)
    );
  } else {
    always_assert_flog(
      !m_data->frozen,
      "Attempting to refine constant {} to {} when index is frozen",
      name,
      show(fa.inferredReturn)
    );
    cns->val = *val;
    m_data->deps->update(*cns);
  }
}

void AnalysisIndex::refine_class_constants(const FuncAnalysisResult& fa) {
  auto const resolved = fa.resolvedInitializers.left();
  if (!resolved || resolved->empty()) return;

  assertx(fa.ctx.func->cls);
  auto& constants = fa.ctx.func->cls->constants;

  for (auto const& c : *resolved) {
    assertx(c.first < constants.size());
    auto& cns = constants[c.first];
    assertx(cns.kind == ConstModifiers::Kind::Value);
    always_assert(cns.val.has_value());
    always_assert(type(*cns.val) == KindOfUninit);

    auto cinfo = fa.ctx.func->cls->cinfo;
    if (auto const val = tv(c.second.type)) {
      assertx(type(*val) != KindOfUninit);
      always_assert_flog(
        !m_data->frozen,
        "Attempting to refine class constant {}::{} to {} "
        "when index is frozen",
        fa.ctx.func->cls->name,
        cns.name,
        show(c.second.type)
      );

      cns.val = *val;
      if (cinfo) cinfo->clsConstantInfo.erase(cns.name);
      m_data->deps->update(
        cns,
        ConstIndex { fa.ctx.func->cls->name, c.first }
      );
    } else if (cinfo) {
      auto const old = folly::get_default(
        cinfo->clsConstantInfo,
        cns.name,
        ClsConstInfo{ TInitCell, 0 }
      );

      if (c.second.type.strictlyMoreRefined(old.type)) {
        always_assert(c.second.refinements > old.refinements);
        always_assert_flog(
          !m_data->frozen,
          "Attempting to refine class constant {}::{} to {} "
          "when index is frozen",
          cinfo->name,
          cns.name,
          show(c.second.type)
        );
        cinfo->clsConstantInfo.insert_or_assign(cns.name, c.second);
        m_data->deps->update(cns, ConstIndex { cinfo->name, c.first });
      } else {
        always_assert_flog(
          c.second.type.moreRefined(old.type),
          "Class constant type invariant violated for {}::{}\n"
          "    {} is not at least as refined as {}\n",
          fa.ctx.func->cls->name,
          cns.name,
          show(c.second.type),
          show(old.type)
        );
      }
    }
  }
}

void AnalysisIndex::refine_return_info(const FuncAnalysisResult& fa) {
  auto const& func = *fa.ctx.func;
  auto& finfo = func_info(*m_data, func);

  auto const error_loc = [&] {
    return folly::sformat("{} {}", func.unit, func_fullname(func));
  };

  auto changes = AnalysisDeps::Type::None;

  if (finfo.retParam == NoLocalId) {
    // This is just a heuristic; it doesn't mean that the value passed
    // in was returned, but that the value of the parameter at the
    // point of the RetC was returned. We use it to make (heuristic)
    // decisions about whether to do inline interps, so we only allow
    // it to change once. (otherwise later passes might not do the
    // inline interp, and get worse results, which breaks
    // monotonicity).
    if (fa.retParam != NoLocalId) {
      finfo.retParam = fa.retParam;
      changes |= AnalysisDeps::Type::RetParam;
    }
  } else {
    always_assert_flog(
      finfo.retParam == fa.retParam,
      "Index return param invariant violated in {}.\n"
      "    Went from {} to {}\n",
      finfo.retParam,
      fa.retParam,
      error_loc()
    );
  }

  auto const unusedParams = ~fa.usedParams;
  if (finfo.unusedParams != unusedParams) {
    always_assert_flog(
      (finfo.unusedParams | unusedParams) == unusedParams,
      "Index unused params decreased in {}.\n",
      error_loc()
    );
    finfo.unusedParams = unusedParams;
    changes |= AnalysisDeps::Type::UnusedParams;
  }

  if (fa.inferredReturn.strictlyMoreRefined(finfo.returnTy)) {
    if (finfo.returnRefinements < options.returnTypeRefineLimit) {
      finfo.returnTy = fa.inferredReturn;
      finfo.returnRefinements += fa.localReturnRefinements + 1;
      if (finfo.returnRefinements > options.returnTypeRefineLimit) {
        FTRACE(1, "maxed out return type refinements at {}\n", error_loc());
      }
      changes |= AnalysisDeps::Type::RetType;
      if (is_scalar(finfo.returnTy)) {
        changes |= AnalysisDeps::Type::ScalarRetType;
      }
    } else {
      FTRACE(1, "maxed out return type refinements at {}\n", error_loc());
    }
  } else {
    always_assert_flog(
      fa.inferredReturn.moreRefined(finfo.returnTy),
      "Index return type invariant violated in {}.\n"
      "   {} is not at least as refined as {}\n",
      error_loc(),
      show(fa.inferredReturn),
      show(finfo.returnTy)
    );
  }

  always_assert_flog(
    !finfo.effectFree || fa.effectFree,
    "Index effect-free invariant violated in {}.\n"
    "    Went from true to false\n",
    error_loc()
  );

  if (finfo.effectFree != fa.effectFree) {
    finfo.effectFree = fa.effectFree;
    changes |= AnalysisDeps::Type::RetType;
  }

  always_assert_flog(
    !m_data->frozen || changes == AnalysisDeps::Type::None,
    "Attempting to refine return info for {} ({}) "
    "when index is frozen",
    error_loc(),
    show(changes)
  );

  if (changes & AnalysisDeps::Type::RetType) {
    if (auto const name = Constant::nameFromFuncName(func.name)) {
      auto const cns = folly::get_default(m_data->constants, name, nullptr);
      always_assert_flog(
        cns,
        "Attempting to update constant {} type, but constant is not present!",
        name
      );
      m_data->deps->update(*cns);
    }
  }
  m_data->deps->update(func, changes);
}

void AnalysisIndex::update_prop_initial_values(const FuncAnalysisResult& fa) {
  auto const resolved = fa.resolvedInitializers.right();
  if (!resolved || resolved->empty()) return;

  assertx(fa.ctx.cls);
  auto& props = const_cast<php::Class*>(fa.ctx.cls)->properties;

  auto changed = false;
  for (auto const& [idx, info] : *resolved) {
    assertx(idx < props.size());
    auto& prop = props[idx];

    if (info.satisfies) {
      if (!(prop.attrs & AttrInitialSatisfiesTC)) {
        always_assert_flog(
          !m_data->frozen,
          "Attempting to update AttrInitialSatisfiesTC for {}::{} "
          "when index is frozen",
          fa.ctx.cls->name,
          prop.name
        );
        attribute_setter(prop.attrs, true, AttrInitialSatisfiesTC);
        changed = true;
      }
    } else {
      always_assert_flog(
        !(prop.attrs & AttrInitialSatisfiesTC),
        "AttrInitialSatisfiesTC invariant violated for {}::{}\n"
        "  Went from true to false",
        fa.ctx.cls->name, prop.name
      );
    }

    always_assert_flog(
      IMPLIES(!(prop.attrs & AttrDeepInit), !info.deepInit),
      "AttrDeepInit invariant violated for {}::{}\n"
      "  Went from false to true",
      fa.ctx.cls->name, prop.name
    );
    if (bool(prop.attrs & AttrDeepInit) != info.deepInit) {
      always_assert_flog(
        !m_data->frozen,
        "Attempting to update AttrDeepInit for {}::{} "
        "when index is frozen",
        fa.ctx.cls->name,
        prop.name
      );
      attribute_setter(prop.attrs, info.deepInit, AttrDeepInit);
    }

    if (type(info.val) != KindOfUninit) {
      always_assert_flog(
        !m_data->frozen,
        "Attempting to update property initial value for {}::{} "
        "to {} when index is frozen",
        fa.ctx.cls->name,
        prop.name,
        show(from_cell(info.val))
      );
      always_assert_flog(
        type(prop.val) == KindOfUninit ||
        from_cell(prop.val) == from_cell(info.val),
        "Property initial value invariant violated for {}::{}\n"
        "  Value went from {} to {}",
        fa.ctx.cls->name, prop.name,
        show(from_cell(prop.val)), show(from_cell(info.val))
      );
      prop.val = info.val;
    } else {
      always_assert_flog(
        type(prop.val) == KindOfUninit,
        "Property initial value invariant violated for {}::{}\n"
        " Value went from {} to not set",
        fa.ctx.cls->name, prop.name,
        show(from_cell(prop.val))
      );
    }
  }
  if (!changed) return;

  auto const cinfo = fa.ctx.cls->cinfo;
  if (!cinfo) return;

  assertx(cinfo->hasBadInitialPropValues);
  auto const noBad = std::all_of(
    begin(props), end(props),
    [] (const php::Prop& prop) {
      return bool(prop.attrs & AttrInitialSatisfiesTC);
    }
  );

  if (noBad) {
    cinfo->hasBadInitialPropValues = false;
  }
}

void AnalysisIndex::update_bytecode(FuncAnalysisResult& fa) {
  auto func = php::WideFunc::mut(const_cast<php::Func*>(fa.ctx.func));
  auto const update = HHBBC::update_bytecode(func, std::move(fa.blockUpdates));
  if (update == UpdateBCResult::None) return;

  always_assert_flog(
    !m_data->frozen,
    "Attempting to update bytecode for {} when index is frozen",
    func_fullname(*fa.ctx.func)
  );

  if (update == UpdateBCResult::ChangedAnalyze ||
      fa.ctx.func->name == s_86cinit.get()) {
    ITRACE(2, "Updated bytecode for {} in a way that requires re-analysis\n",
           func_fullname(*fa.ctx.func));
    m_data->worklist.schedule(fc_from_context(fa.ctx));
  }

  m_data->deps->update(*fa.ctx.func, AnalysisDeps::Type::Bytecode);
}

// Finish using the AnalysisIndex and calculate the output to be
// returned back from the job.
AnalysisIndex::Output AnalysisIndex::finish() {
  Variadic<std::unique_ptr<php::Class>> classes;
  Variadic<std::unique_ptr<php::Func>> funcs;
  Variadic<std::unique_ptr<php::Unit>> units;
  Variadic<std::unique_ptr<php::ClassBytecode>> clsBC;
  Variadic<std::unique_ptr<php::FuncBytecode>> funcBC;
  Variadic<AnalysisIndexCInfo> cinfos;
  Variadic<AnalysisIndexFInfo> finfos;
  Variadic<AnalysisIndexMInfo> minfos;
  AnalysisOutput::Meta meta;

  assertx(m_data->frozen);

  // Remove any 86cinits that are now unneeded.
  meta.removedFuncs = strip_unneeded_constant_inits(*m_data);

  classes.vals.reserve(m_data->classNames.size());
  clsBC.vals.reserve(m_data->classNames.size());
  cinfos.vals.reserve(m_data->classNames.size());
  meta.classDeps.reserve(m_data->classNames.size());
  for (auto const name : m_data->classNames) {
    auto& cls = m_data->classes.at(name);

    meta.classDeps.emplace_back(m_data->deps->take(cls.get()));

    clsBC.vals.emplace_back(
      std::make_unique<php::ClassBytecode>(cls->name)
    );
    auto& bc = *clsBC.vals.back();
    for (auto& meth : cls->methods) {
      bc.methodBCs.emplace_back(meth->name, std::move(meth->rawBlocks));
    }

    if (auto cinfo = folly::get_ptr(m_data->cinfos, name)) {
      AnalysisIndexCInfo acinfo;
      acinfo.ptr = decltype(acinfo.ptr){cinfo->release()};
      cinfos.vals.emplace_back(std::move(acinfo));
    } else if (auto minfo = folly::get_ptr(m_data->minfos, name)) {
      AnalysisIndexMInfo aminfo;
      aminfo.ptr = decltype(aminfo.ptr){minfo->release()};
      minfos.vals.emplace_back(std::move(aminfo));
    }

    classes.vals.emplace_back(std::move(cls));
  }

  funcs.vals.reserve(m_data->funcNames.size());
  funcBC.vals.reserve(m_data->funcNames.size());
  finfos.vals.reserve(m_data->funcNames.size());
  meta.funcDeps.reserve(m_data->funcNames.size());
  for (auto const name : m_data->funcNames) {
    assertx(!meta.removedFuncs.count(name));

    auto& func = m_data->funcs.at(name);
    auto& finfo = m_data->finfos.at(name);

    meta.funcDeps.emplace_back(m_data->deps->take(func.get()));

    funcBC.vals.emplace_back(
      std::make_unique<php::FuncBytecode>(name, std::move(func->rawBlocks))
    );
    funcs.vals.emplace_back(std::move(func));

    AnalysisIndexFInfo afinfo;
    afinfo.ptr = decltype(afinfo.ptr){finfo.release()};
    finfos.vals.emplace_back(std::move(afinfo));
  }

  units.vals.reserve(m_data->unitNames.size());
  for (auto const name : m_data->unitNames) {
    auto& unit = m_data->units.at(name);
    units.vals.emplace_back(std::move(unit));
  }

  meta.changed = std::move(m_data->deps->getChanges());

  return std::make_tuple(
    std::move(classes),
    std::move(funcs),
    std::move(units),
    std::move(clsBC),
    std::move(funcBC),
    std::move(cinfos),
    std::move(finfos),
    std::move(minfos),
    std::move(meta)
  );
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

#define UNIMPLEMENTED always_assert_flog(false, "{} not implemented for AnalysisIndex", __func__)

void AnalysisIndexAdaptor::push_context(const Context& ctx) const {
  index.push_context(ctx);
}

void AnalysisIndexAdaptor::pop_context() const {
  index.pop_context();
}

const php::Unit* AnalysisIndexAdaptor::lookup_func_unit(const php::Func& func) const {
  return &index.lookup_func_unit(func);
}
const php::Unit* AnalysisIndexAdaptor::lookup_class_unit(const php::Class& cls) const {
  return &index.lookup_class_unit(cls);
}
const php::Class* AnalysisIndexAdaptor::lookup_const_class(const php::Const& cns) const {
  return index.lookup_const_class(cns);
}
const php::Class* AnalysisIndexAdaptor::lookup_closure_context(const php::Class& cls) const {
  return &index.lookup_closure_context(cls);
}

const CompactVector<const php::Class*>*
AnalysisIndexAdaptor::lookup_closures(const php::Class*) const {
  UNIMPLEMENTED;
}

const hphp_fast_set<const php::Func*>*
AnalysisIndexAdaptor::lookup_extra_methods(const php::Class*) const {
  UNIMPLEMENTED;
}

Optional<res::Class> AnalysisIndexAdaptor::resolve_class(SString n) const {
  return index.resolve_class(n);
}
Optional<res::Class>
AnalysisIndexAdaptor::resolve_class(const php::Class& c) const {
  return index.resolve_class(c);
}
std::pair<const php::TypeAlias*, bool>
AnalysisIndexAdaptor::lookup_type_alias(SString n) const {
  return index.lookup_type_alias(n);
}
Index::ClassOrTypeAlias
AnalysisIndexAdaptor::lookup_class_or_type_alias(SString n) const {
  return index.lookup_class_or_type_alias(n);
}

res::Func AnalysisIndexAdaptor::resolve_func_or_method(const php::Func& f) const {
  return index.resolve_func_or_method(f);
}
res::Func AnalysisIndexAdaptor::resolve_func(SString f) const {
  return index.resolve_func(f);
}
res::Func AnalysisIndexAdaptor::resolve_method(Context,
                                               const Type& t,
                                               SString n) const {
  return index.resolve_method(t, n);
}
res::Func AnalysisIndexAdaptor::resolve_ctor(const Type& obj) const {
  return index.resolve_ctor(obj);
}

std::vector<std::pair<SString, ClsConstInfo>>
AnalysisIndexAdaptor::lookup_class_constants(const php::Class& cls) const {
  return index.lookup_class_constants(cls);
}

ClsConstLookupResult
AnalysisIndexAdaptor::lookup_class_constant(Context,
                                            const Type& cls,
                                            const Type& name) const {
  return index.lookup_class_constant(cls, name);
}

ClsTypeConstLookupResult
AnalysisIndexAdaptor::lookup_class_type_constant(
  const Type& cls,
  const Type& name,
  const Index::ClsTypeConstLookupResolver& resolver
) const {
  return index.lookup_class_type_constant(cls, name, resolver);
}

Type AnalysisIndexAdaptor::lookup_constant(Context, SString n) const {
  return index.lookup_constant(n);
}
bool AnalysisIndexAdaptor::func_depends_on_arg(const php::Func* f,
                                               size_t arg) const {
  return index.func_depends_on_arg(*f, arg);
}
Index::ReturnType
AnalysisIndexAdaptor::lookup_foldable_return_type(Context,
                                                  const CallContext& callee) const {
  return index.lookup_foldable_return_type(callee);
}
Index::ReturnType AnalysisIndexAdaptor::lookup_return_type(Context,
                                                           MethodsInfo* methods,
                                                           res::Func func,
                                                           Dep) const {
  return index.lookup_return_type(methods, func);
}
Index::ReturnType AnalysisIndexAdaptor::lookup_return_type(Context,
                                                           MethodsInfo* methods,
                                                           const CompactVector<Type>& args,
                                                           const Type& context,
                                                           res::Func func,
                                                           Dep) const {
  return index.lookup_return_type(methods, args, context, func);
}

std::pair<Index::ReturnType, size_t>
AnalysisIndexAdaptor::lookup_return_type_raw(const php::Func* f) const {
  return index.lookup_return_type_raw(*f);
}
CompactVector<Type>
AnalysisIndexAdaptor::lookup_closure_use_vars(const php::Func*, bool) const {
  UNIMPLEMENTED;
}

PropState AnalysisIndexAdaptor::lookup_private_props(const php::Class* cls,
                                                     bool) const {
  return index.lookup_private_props(*cls);
}
PropState AnalysisIndexAdaptor::lookup_private_statics(const php::Class* cls,
                                                       bool) const {
  return index.lookup_private_statics(*cls);
}

PropLookupResult AnalysisIndexAdaptor::lookup_static(Context,
                                                     const PropertiesInfo&,
                                                     const Type&,
                                                     const Type& name) const {
  // Not implemented yet, be conservative.

  auto const sname = [&] () -> SString {
    // Treat non-string names conservatively, but the caller should be
    // checking this.
    if (!is_specialized_string(name)) return nullptr;
    return sval_of(name);
  }();

  return PropLookupResult{
    TInitCell,
    sname,
    TriBool::Maybe,
    TriBool::Maybe,
    TriBool::Maybe,
    TriBool::Maybe,
    TriBool::Maybe,
    true
  };
}

Type AnalysisIndexAdaptor::lookup_public_prop(const Type&, const Type&) const {
  return TInitCell;
}

PropMergeResult
AnalysisIndexAdaptor::merge_static_type(Context,
                                        PublicSPropMutations& publicMutations,
                                        PropertiesInfo& privateProps,
                                        const Type& cls,
                                        const Type& name,
                                        const Type& val,
                                        bool checkUB,
                                        bool ignoreConst,
                                        bool mustBeReadOnly) const {
  return index.merge_static_type(
    publicMutations,
    privateProps,
    cls,
    name,
    val,
    checkUB,
    ignoreConst,
    mustBeReadOnly
  );
}

bool AnalysisIndexAdaptor::using_class_dependencies() const {
  return false;
}

#undef UNIMPLEMENTED

//////////////////////////////////////////////////////////////////////

template <typename T>
void AnalysisIndexParam<T>::Deleter::operator()(T* t) const {
  delete t;
}

template <typename T>
void AnalysisIndexParam<T>::serde(BlobEncoder& sd) const {
  sd(ptr);
}

template <typename T>
void AnalysisIndexParam<T>::serde(BlobDecoder& sd) {
  sd(ptr);
}

template struct AnalysisIndexParam<ClassInfo2>;
template struct AnalysisIndexParam<FuncInfo2>;
template struct AnalysisIndexParam<MethodsWithoutCInfo>;

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::ClassInfo2);
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::FuncInfo2);
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::FuncFamily2);
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::MethodsWithoutCInfo);
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::BuildSubclassListJob::Split);

//////////////////////////////////////////////////////////////////////

}
