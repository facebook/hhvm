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

#include "hphp/util/algorithm.h"
#include "hphp/util/assertions.h"
#include "hphp/util/hash-set.h"
#include "hphp/util/lock-free-lazy.h"
#include "hphp/util/match.h"

#include "hphp/zend/zend-string.h"

namespace HPHP::HHBBC {

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

//////////////////////////////////////////////////////////////////////

// HHBBC consumes a LOT of memory, so we keep representation types small.
template <typename T, size_t Expected, size_t Actual = sizeof(T)>
constexpr bool CheckSize() { static_assert(Expected == Actual); return true; };
static_assert(CheckSize<php::Block, 24>(), "");
static_assert(CheckSize<php::Local, use_lowptr ? 12 : 16>(), "");
static_assert(CheckSize<php::Param, use_lowptr ? 64 : 96>(), "");
static_assert(CheckSize<php::Func, use_lowptr ? 176 : 224>(), "");

// Likewise, we also keep the bytecode and immediate types small.
static_assert(CheckSize<Bytecode, use_lowptr ? 32 : 40>(), "");
static_assert(CheckSize<MKey, 16>(), "");
static_assert(CheckSize<IterArgs, 16>(), "");
static_assert(CheckSize<FCallArgs, 8>(), "");
static_assert(CheckSize<RepoAuthType, 8>(), "");

//////////////////////////////////////////////////////////////////////

/*
 * One-to-many case insensitive map, where the keys are static strings
 * and the values are some kind of pointer.
 */
template<class T> using SStringToMany =
  std::unordered_multimap<
    SString,
    T*,
    string_data_hash,
    string_data_same
  >;

/*
 * One-to-one case insensitive map, where the keys are static strings
 * and the values are some T.
 *
 * Elements are not stable under insert/erase.
 */
template<class T> using ISStringToOneT =
  hphp_fast_map<
    SString,
    T,
    string_data_hash,
    string_data_isame
  >;

/*
 * One-to-one case sensitive map, where the keys are static strings
 * and the values are some T.
 *
 * Elements are not stable under insert/erase.
 *
 * Static strings are always uniquely defined by their pointer, so
 * pointer hashing/comparison is sufficient.
 */
template<class T> using SStringToOneT = hphp_fast_map<SString, T>;

/*
 * One-to-one case sensitive map, where the keys are static strings
 * and the values are some T.
 *
 * Elements are stable under insert/erase.
 *
 * Static strings are always uniquely defined by their pointer, so
 * pointer hashing/comparison is sufficient.
 */
template<class T> using SStringToOneNodeT = hphp_hash_map<SString, T>;

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
 * The `initializerType' is for use during refine_public_statics, and
 * inferredType will always be a supertype of initializerType.
 */
struct PublicSPropEntry {
  Type inferredType;
  Type initializerType;
  const php::Prop* prop;
  uint32_t refinements;
  /*
   * This flag is set during analysis to indicate that we resolved the
   * initial value (and updated it on the php::Class). This doesn't
   * need to be atomic, because only one thread can resolve the value
   * (the one processing the 86sinit), and it's been joined by the
   * time we read the flag in refine_public_statics.
   */
  bool initialValueResolved;
  bool everModified;
};

/*
 * Entries in the ClassInfo method table need to track some additional
 * information.
 *
 * The reason for this is that we need to record attributes of the
 * class hierarchy.
 */
struct MethTabEntry {
  MethTabEntry(const php::Func* func, Attr a, bool hpa, bool tl) :
      func(func), attrs(a), hasPrivateAncestor(hpa), topLevel(tl) {}
  const php::Func* func = nullptr;
  // A method could be imported from a trait, and its attributes changed
  Attr attrs {};
  bool hasAncestor = false;
  bool hasPrivateAncestor = false;
  // This method came from the ClassInfo that owns the MethTabEntry,
  // or one of its used traits.
  bool topLevel = false;
  uint32_t idx = 0;
};

}

struct res::Func::MethTabEntryPair :
      SStringToOneNodeT<MethTabEntry>::value_type {};

namespace {

using MethTabEntryPair = res::Func::MethTabEntryPair;

inline MethTabEntryPair* mteFromElm(
  SStringToOneNodeT<MethTabEntry>::value_type& elm) {
  return static_cast<MethTabEntryPair*>(&elm);
}

inline const MethTabEntryPair* mteFromElm(
  const SStringToOneNodeT<MethTabEntry>::value_type& elm) {
  return static_cast<const MethTabEntryPair*>(&elm);
}

inline MethTabEntryPair*
mteFromIt(SStringToOneNodeT<MethTabEntry>::iterator it) {
  return static_cast<MethTabEntryPair*>(&*it);
}

using ContextRetTyMap = tbb::concurrent_hash_map<
  CallContext,
  Type,
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
   * VerifyParamType does not count as a use in this context.
   */
  std::bitset<64> unusedParams;

  /*
   * List of all func families this function belongs to.
   */
  CompactVector<FuncFamily*> families;
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
using MethTabEntryPair = res::Func::MethTabEntryPair;

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
 */
struct res::Func::FuncFamily {
  using PFuncVec = CompactVector<const MethTabEntryPair*>;

  explicit FuncFamily(PFuncVec&& v) : m_v{std::move(v)} {}
  FuncFamily(FuncFamily&& o) noexcept : m_v(std::move(o.m_v)) {}
  FuncFamily& operator=(FuncFamily&& o) noexcept {
    m_v = std::move(o.m_v);
    return *this;
  }
  FuncFamily(const FuncFamily&) = delete;
  FuncFamily& operator=(const FuncFamily&) = delete;

  const PFuncVec& possibleFuncs() const {
    return m_v;
  };

  friend auto begin(const FuncFamily& ff) { return ff.m_v.begin(); }
  friend auto end(const FuncFamily& ff) { return ff.m_v.end(); }

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

  LockFreeLazy<Type> m_returnTy;
  PFuncVec m_v;
  const StaticInfo* m_static;
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

namespace {

struct PFuncVecHasher {
  size_t operator()(const FuncFamily::PFuncVec& v) const {
    return folly::hash::hash_range(
      v.begin(),
      v.end(),
      0,
      pointer_hash<MethTabEntryPair>{}
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

//////////////////////////////////////////////////////////////////////

/*
 * Known information about a particular possible instantiation of a
 * PHP class.  The php::Class will be marked AttrUnique if there is a
 * unique ClassInfo with the same name.
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
   * A vector of the used traits, in class order, mirroring the
   * php::Class usedTraitNames vector.
   */
  CompactVector<const ClassInfo*> usedTraits;

  /*
   * A list of extra properties supplied by this class's used traits.
   */
  CompactVector<php::Prop> traitProps;

  /*
   * A list of extra consts supplied by this class's used traits.
   */
  CompactVector<php::Const> traitConsts;

  /*
   * A (case-sensitive) map from class method names to the php::Func
   * associated with it.  This map is flattened across the inheritance
   * hierarchy.
   */
  SStringToOneNodeT<MethTabEntry> methods;

  /*
   * A (case-sensitive) map from class method names to associated
   * FuncFamily objects that group the set of possibly-overriding
   * methods.
   *
   * Note that this does not currently encode anything for interface
   * methods.
   *
   * Invariant: methods on this class with AttrNoOverride or
   * AttrPrivate will not have an entry in this map.
   */
  SStringToOneT<FuncFamily*> methodFamilies;
  // Resolutions to single entries do not require a FuncFamily (this
  // saves space).
  SStringToOneT<const MethTabEntryPair*> singleMethodFamilies;

  /*
   * Subclasses of this class, including this class itself.
   *
   * For interfaces, this is the list of instantiable classes that
   * implement this interface.
   *
   * For traits, this is the list of classes that use the trait where
   * the trait wasn't flattened into the class (including the trait
   * itself).
   *
   * The elements in this vector are sorted by their pointer value.
   */
  CompactVector<ClassInfo*> subclassList;

  /*
   * For interfaces, the list of non-instantiable classes that
   * implement this interface. This is needed in a few places for
   * interface canonicalization.
   */
  CompactVector<ClassInfo*> abstractSubclassList;

  /*
   * A vector of ClassInfo that encodes the inheritance hierarchy,
   * unless this ClassInfo represents an interface.
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
    // calculated (on top of InterfaceInfo being lazily calculated).
    using CouldBeSet = hphp_fast_set<const ClassInfo*>;
    mutable LockFreeLazy<CouldBeSet> lazyCouldBe;

    // The set of interfaces which this interface is a subtype
    // of. That is, every implementation of this interface also
    // implements those interfaces. Every interface is a subtypeOf
    // itself, but itself is not stored here (space optimization).
    hphp_fast_set<const ClassInfo*> subtypeOf;

    // Non-nullptr if there's a single class which is a super class of
    // all implementations of this interface, nullptr otherwise.
    const ClassInfo* commonBase;
    // If two interfaces are equivalent (which means they are
    // implemented by the exact same set of classes), this will point
    // to the "canonical" one that should be used. Always not
    // nullptr. If there's no equivalent, it will point to this
    // ClassInfo.
    const ClassInfo* equivalent;
  };
  // Don't access this directly, use interfaceInfo().
  LockFreeLazyPtr<InterfaceInfo> lazyInterfaceInfo;

  // Obtain the InterfaceInfo or CouldBeSet for this interface
  // (calculating it if necessary). This class must be an interface.
  const InterfaceInfo& interfaceInfo();
  const InterfaceInfo::CouldBeSet& couldBe();

  /*
   * Property types for public static properties, declared on this exact class
   * (i.e. not flattened in the hierarchy).
   *
   * These maps always have an entry for each public static property declared
   * in this class, so it can also be used to check if this class declares a
   * public static property of a given name.
   *
   * Note: the effective type we can assume a given static property may hold is
   * not just the value in these maps.
   */
  SStringToOneT<PublicSPropEntry> publicStaticProps;

  struct PreResolveState {
    hphp_fast_map<SString, std::pair<php::Prop, const ClassInfo*>> pbuildNoTrait;
    hphp_fast_map<SString, std::pair<php::Prop, const ClassInfo*>> pbuildTrait;
    hphp_fast_set<SString> constsFromTraits;
  };
  std::unique_ptr<PreResolveState> preResolveState;

  /*
   * Flags to track if this class is mocked, or if any of its dervied classes
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
   * Return true if this is derived from o.
   */
  bool derivedFrom(const ClassInfo& o) const {
    if (this == &o) return true;
    // If o is an interface, see if this declared it.
    if (o.cls->attrs & AttrInterface) return implInterfaces.count(o.cls->name);
    // Otherwise check for direct inheritance.
    if (baseList.size() >= o.baseList.size()) {
      return baseList[o.baseList.size() - 1] == &o;
    }
    return false;
  }

  /*
   * Flags about the existence of various magic methods, or whether
   * any derived classes may have those methods.  The non-derived
   * flags imply the derived flags, even if the class is final, so you
   * don't need to check both in those situations.
   */
  struct MagicFnInfo {
    bool thisHas{false};
    bool derivedHas{false};
  };
  MagicFnInfo magicBool;
};

struct MagicMapInfo {
  StaticString name;
  ClassInfo::MagicFnInfo ClassInfo::*pmem;
};

const MagicMapInfo magicMethods[] {
  { StaticString{"__toBoolean"}, &ClassInfo::magicBool },
};

const ClassInfo::InterfaceInfo::CouldBeSet& ClassInfo::couldBe() {
  return interfaceInfo().lazyCouldBe.get(
    [this] {
      // For every implementation of this interface, add all of the
      // interfaces it implements, and all of its parent classes.
      InterfaceInfo::CouldBeSet couldBe;
      auto const process = [&] (const ClassInfo* sub) {
        for (auto const& [_, impl] : sub->implInterfaces) {
          if (impl == this) continue;
          couldBe.emplace(impl);
        }
        auto c = sub;
        do {
          // If we already added it, all subsequent parents are also
          // added, so we can stop.
          if (!couldBe.emplace(c).second) break;
          c = c->parent;
        } while (c);
      };
      for (auto const sub : subclassList)         process(sub);
      for (auto const sub : abstractSubclassList) process(sub);
      return couldBe;
    }
  );
}

const ClassInfo::InterfaceInfo& ClassInfo::interfaceInfo() {
  assertx(cls->attrs & AttrInterface);
  return lazyInterfaceInfo.get(
    [this] {
      auto info = std::make_unique<ClassInfo::InterfaceInfo>();

      auto const commonAncestor = [] (const ClassInfo* c1,
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
      };

      // Start out with the info from the first implementation.
      if (!subclassList.empty()) {
        info->commonBase = subclassList[0];
        for (auto const& [_, impl] : subclassList[0]->implInterfaces) {
          if (impl == this) continue; // Skip ourself
          info->subtypeOf.emplace(impl);
        }
      } else if (!abstractSubclassList.empty()) {
        auto const sub = abstractSubclassList[0];
        info->commonBase = sub;
        for (auto const& [_, impl] : sub->implInterfaces) {
          if (impl == this) continue; // Skip ourself
          info->subtypeOf.emplace(impl);
        }
      } else {
        info->commonBase = nullptr;
      }

      // Update the common base and subtypeOf list for every
      // implementation. We're only a subtype of an interface if
      // *every* implementation of us implements that interface, so
      // the set can only shrink.
      auto const process = [&] (const ClassInfo* sub) {
        if (info->commonBase) {
          info->commonBase = commonAncestor(info->commonBase, sub);
        }

        folly::erase_if(
          info->subtypeOf,
          [&] (const ClassInfo* i) {
            return !sub->implInterfaces.count(i->cls->name);
          }
        );

        return info->commonBase || !info->subtypeOf.empty();
      };

      for (auto const sub : subclassList) {
        if (!process(sub)) break;
      }
      for (auto const sub : abstractSubclassList) {
        if (!process(sub)) break;
      }

      // Compute equivalency. Two interfaces are equivalent if one is
      // a subtype of another, and their implementations all implement
      // the other interface. We canonicalize to the interface with
      // the name which compares less.
      info->equivalent = this;
      for (auto const maybeEquiv : info->subtypeOf) {
        if (info->equivalent->cls->name->compare(maybeEquiv->cls->name) < 0) {
          continue;
        }
        auto const isEquiv = [&] {
          for (auto const sub : maybeEquiv->subclassList) {
            if (!sub->implInterfaces.count(info->equivalent->cls->name)) {
              return false;
            }
          }
          for (auto const sub : maybeEquiv->abstractSubclassList) {
            if (!sub->implInterfaces.count(info->equivalent->cls->name)) {
              return false;
            }
          }
          return true;
        }();
        if (isEquiv) info->equivalent = const_cast<ClassInfo*>(maybeEquiv);
      }

      // Special case: If this interface has a common base, and the
      // common base implements the interface (which it may not), then
      // it's the best "equivalent" for this interface.
      if (info->commonBase &&
          info->commonBase->implInterfaces.count(info->equivalent->cls->name)) {
        info->equivalent = info->commonBase;
      }

      return info.release();
    }
  );
}

//////////////////////////////////////////////////////////////////////

namespace {

uint32_t numNVArgs(const php::Func& f) {
  uint32_t cnt = f.params.size();
  return cnt && f.params[cnt - 1].isVariadic ? cnt - 1 : cnt;
}

PrepKind func_param_prep(const php::Func* f, uint32_t paramId) {
  auto const sz = f->params.size();
  if (paramId >= sz) return PrepKind{TriBool::No, TriBool::No};
  PrepKind kind;
  kind.inOut = yesOrNo(f->params[paramId].inout);
  kind.readonly = yesOrNo(f->params[paramId].readonly);
  return kind;
}

}

//////////////////////////////////////////////////////////////////////

namespace res {

bool Class::same(const Class& o) const {
  return val == o.val;
}

bool Class::exactSubtypeOf(const Class& o) const {
  // An unresolved class is only a subtype of another if they're the
  // same.
  if (val.left() || o.val.left()) return same(o);
  // If the lhs is an interface, it cannot be a subtype of
  // anything. The interface class itself has no parents (and
  // implements nothing), so cannot be a subtype of anything
  // (including itself).
  auto const c1 = val.right();
  if (c1->cls->attrs & AttrInterface) return false;
  // This does the correct check if the rhs is an interface.
  return same(o) || c1->derivedFrom(*o.val.right());
}

bool Class::subSubtypeOf(const Class& o) const {
  // Class with the same name is always a subtype.
  if (same(o)) return true;
  // We know they're not the same, so cannot be a subtype if any are
  // not resolved.
  if (val.left() || o.val.left()) return false;
  auto const c1 = val.right();
  auto const c2 = o.val.right();
  if (c1->cls->attrs & AttrInterface) {
    // lhs is an interface. Since this is the "sub" variant, it means
    // any implementation of the interface (not the interface class
    // itself).
    auto& info = c1->interfaceInfo();
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
  // lhs is not an interface. See if it derives from the rhs (which
  // does the correct check if the rhs is an interface).
  return c1->derivedFrom(*c2);
}

bool Class::exactCouldBe(const Class& o) const {
  // Two unresolved classes with different names cannot be a subtype
  // of one, so in the exact case cannot match.
  if (val.left()) return same(o);
  // lhs is resolved, but if the rhs is not, cannot match for the same
  // reason.
  if (o.val.left()) return false;
  // For the same reason as exactSubtypeOf(), an interface (for the
  // exact case) cannot match.
  auto const c1 = val.right();
  if (c1->cls->attrs & AttrInterface) return false;
  // Otherwise equivalent to exactSubtypeOf().
  return same(o) || c1->derivedFrom(*o.val.right());
}

bool Class::subCouldBe(const Class& o) const {
  // Classes with the same name always can be each other.
  if (same(o)) return true;
  // Two unresolved classes always can be each other.
  if (val.left()) return o.val.left();
  // But an unresolved and resolved class can never be each other.
  if (o.val.left()) return false;

  auto const c1 = val.right();
  auto const c2 = o.val.right();
  if (c1->cls->attrs & AttrInterface) {
    // If lhs is an interface, see if the rhs is in the CouldBeSet
    // (this works if rhs is an interface or base class).
    return c1->couldBe().count(c2);
  } else if (c2->cls->attrs & AttrInterface) {
    // Same situation, but reversed
    return c2->couldBe().count(c1);
  }

  // Both types are non-interfaces so they "could be" if they are in
  // an inheritance relationship
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

Optional<res::Class> Class::canonicalizeInterface() const {
  return val.match(
    [&] (SString) -> Optional<res::Class> { return *this; },
    [&] (ClassInfo* cinfo) -> Optional<res::Class> {
      if (!(cinfo->cls->attrs & AttrInterface)) return *this;
      // No implementations is Bottom
      if (cinfo->subclassList.empty() &&
          cinfo->abstractSubclassList.empty()) return std::nullopt;
      // Otherwise replace it with its equivalent (which may be
      // itself).
      return Class { const_cast<ClassInfo*>(cinfo->interfaceInfo().equivalent) };
    }
  );
}

bool Class::couldBeOverriden() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return !(cinfo->cls->attrs & AttrNoOverride);
    }
  );
}

bool Class::couldHaveMagicBool() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return cinfo->magicBool.derivedHas;
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
  return res::Class { parent };
}

const php::Class* Class::cls() const {
  return val.right() ? val.right()->cls : nullptr;
}

void
Class::forEachSubclass(const std::function<void(const php::Class*)>& f) const {
  auto const cinfo = val.right();
  assertx(cinfo);
  for (auto const& s : cinfo->subclassList) f(s->cls);
}

std::string show(const Class& c) {
  return c.val.match(
    [] (SString s) {
      return folly::sformat("{}*", s);
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
// *all* the classes in the range. If a class is unresolved, it will
// be passed, as is, to the callable. If the callable returns false,
// iteration is stopped. If includeAbstract is true, non-instantiable
// subclasses are visited (normally they are skipped).
template <typename F>
void Class::visitEverySub(folly::Range<const Class*> classes,
                          bool includeAbstract,
                          const F& f) {
  assertx(!classes.empty());

  // Simple case: if there's only one class, just iterate over the
  // subclass list.
  if (classes.size() == 1) {
    auto const cinfo = classes.front().val.right();
    if (!cinfo) {
      f(classes.front());
    } else {
      for (auto const sub : cinfo->subclassList) {
        if (!f(Class { sub })) {
          includeAbstract = false;
          break;
        }
      }
      if (includeAbstract) {
        for (auto const sub : cinfo->abstractSubclassList) {
          if (!f(Class { sub })) break;
        }
      }
    }
    return;
  }

  // Otherwise we need to find all of the classes in common:
  std::vector<ClassInfo*> common;

  // Find the first resolved class, and use that to initialize the
  // list of subclasses.
  auto const numClasses = classes.size();
  int idx = 0;
  while (idx < numClasses) {
    if (auto const cinfo = classes[idx].val.right()) {
      std::set_union(
        begin(cinfo->subclassList),
        end(cinfo->subclassList),
        includeAbstract
          ? begin(cinfo->abstractSubclassList)
          : end(cinfo->abstractSubclassList),
        end(cinfo->abstractSubclassList),
        std::back_inserter(common)
      );
      ++idx;
      break;
    }
    if (!f(classes[idx])) return;
    ++idx;
  }

  // Now process the result, removing any subclasses which aren't a
  // subclass of all of the classes.
  std::vector<ClassInfo*> newCommon;
  while (idx < numClasses) {
    if (auto const cinfo = classes[idx].val.right()) {
      newCommon.clear();
      auto it1 = begin(common);
      auto it2 = begin(cinfo->subclassList);
      auto it3 = includeAbstract
        ? begin(cinfo->abstractSubclassList)
        : end(cinfo->abstractSubclassList);
      auto const end1 = end(common);
      auto const end2 = end(cinfo->subclassList);
      auto const end3 = end(cinfo->abstractSubclassList);
      while (it1 != end1 && (it2 != end2 || it3 != end3)) {
        if (it2 != end2) {
          if (*it1 == *it2) {
            newCommon.emplace_back(*it1);
            ++it1;
            ++it2;
            continue;
          } else if (*it2 < *it1) {
            ++it2;
            continue;
          }
        }

        if (it3 != end3) {
          if (*it1 == *it3) {
            newCommon.emplace_back(*it1);
            ++it1;
            ++it3;
            continue;
          } else if (*it3 < *it1) {
            ++it3;
            continue;
          }
        }

        ++it1;
      }

      std::swap(common, newCommon);
      ++idx;
      continue;
    }
    if (!f(classes[idx])) return;
    ++idx;
  }

  // We have the final list. Iterate over these and report them to the
  // callable.
  for (auto const c : common) {
    if (!f(Class { c })) break;
  }
}

// Given a list of classes, put them in canonical form for a
// DCls::IsectSet. It is assumed that couldBe is true between all of
// the classes in the list, but nothing is assumed otherwise.
TinyVector<Class, 2> Class::canonicalizeIsects(const TinyVector<Class, 8>& in) {
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

    // Non-interfaces should precede interfaces
    if (c1->cls->attrs & AttrInterface) {
      if (!(c2->cls->attrs & AttrInterface)) {
        return 1;
      }
    } else if (c2->cls->attrs & AttrInterface) {
      return -1;
    }
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
        if (i == j || !c2.subSubtypeOf(c1)) continue;
        // c2 is a subtype of c1. If c1 is not a subtype of c2, then
        // c2 is preferred and we return true to drop c1.
        if (!c1.subSubtypeOf(c2)) return true;
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
                                    bool isSub2) {
  TinyVector<Class, 8> common;
  Optional<ClassInfo*> commonBase;
  Optional<hphp_fast_set<ClassInfo*>> commonInterfaces;

  // The algorithm for unioning together two intersection lists is
  // simple. For every class which is "in" either the first or second
  // list, track the interfaces which are implemented by all of the
  // classes, and the common base class amongst all of them. Build a
  // list of these classes and normalize them.

  auto const processSub = [&] (ClassInfo* cinfo) {
    assertx(!(cinfo->cls->attrs & AttrInterface));

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

  auto const processIface = [&] (ClassInfo* cinfo) {
    assertx(cinfo->cls->attrs & AttrInterface);
    auto& info = cinfo->interfaceInfo();

    // The logic for processing an interface is similar to processSub,
    // except we use the interface's common base (if any).
    auto const ifaceCommon = const_cast<ClassInfo*>(info.commonBase);
    if (!commonBase) {
      commonBase = ifaceCommon;
    } else {
      commonBase = commonAncestor(*commonBase, ifaceCommon);
    }

    // Instead of implInterfaces (which isn't meaningful for an
    // interface), we use the set of interfaces it is a subtype of.
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

  auto const processList = [&] (folly::Range<const Class*> classes, bool isSub) {
    if (classes.size() == 1) {
      // If the list is just a single class, we can process things
      // more efficiently.
      auto const cinfo = classes[0].val.right();
      assertx(cinfo);
      if (cinfo->cls->attrs & AttrInterface) {
        // An "exact" interface is just the interface class (not the
        // implementations). These classes have no parents and
        // implement nothing, so any union with them is going to
        // result in TObj/TCls. Return false to bail out and signal
        // this.
        if (!isSub) return false;
        // A "sub" interface. We'll process it's implementations.
        processIface(cinfo);
      } else {
        processSub(cinfo);
      }
      return true;
    }

    // The list has multiple classes. This is more expensive, we need
    // to visit every subclass in the intersection of the classes on
    // the list.
    visitEverySub(
      classes,
      true,
      [&] (res::Class c) {
        assertx(c.val.right());
        // We'll only "visit" non-interface sub-classes, so only use
        // processSub here.
        processSub(c.val.right());
        // No point in continuing if there's nothing in common left.
        return *commonBase || !commonInterfaces->empty();
      }
    );
    return true;
  };

  assertx(!classes1.empty());
  assertx(!classes2.empty());
  assertx(IMPLIES(!isSub1, classes1.size() == 1));
  assertx(IMPLIES(!isSub2, classes2.size() == 1));

  // Sanity check that if the list starts with an unresolved class,
  // then the rest of the list contains nothing but unresolved
  // classes. If the list starts with a resolved class, processList
  // will assert if the rest are not resolved.
  auto const unresolved1 = (bool)classes1[0].val.left();
  auto const unresolved2 = (bool)classes2[0].val.left();
  if (debug) {
    if (unresolved1) {
      for (auto const c : classes1) always_assert(c.val.left());
    }
    if (unresolved2) {
      for (auto const c : classes2) always_assert(c.val.left());
    }
  }

  // Treat lists of unresolved classes separately (since we cannot
  // look at their subclasses). If one list is all unresolved classes
  // and the other isn't (and vice versa), there's nothing in common,
  // so return an empty list (this will become TObj or
  // TCls). Otherwise return the unresolved classes which are in both
  // lists. The result is guaranteed to already be canonicalized.
  if (unresolved1) {
    if (unresolved2) {
      TinyVector<Class, 2> out;
      for (auto const c1 : classes1) {
        for (auto const c2 : classes2) {
          if (c1.same(c2)) out.emplace_back(c1);
        }
      }
      return out;
    }
    return {};
  } else if (unresolved2) {
    return {};
  }

  // Otherwise process both lists and build the common sets.
  if (!processList(classes1, isSub1)) return {};
  if (!processList(classes2, isSub2)) return {};
  // Combine the common classes
  if (commonBase && *commonBase) {
    common.emplace_back(Class { *commonBase });
  }
  if (commonInterfaces) {
    for (auto const i : *commonInterfaces) {
      common.emplace_back(Class { i });
    }
  }

  // And canonicalize
  return canonicalizeIsects(common);
}

TinyVector<Class, 2> Class::intersect(folly::Range<const Class*> classes1,
                                      folly::Range<const Class*> classes2) {
  TinyVector<Class, 8> common;
  Optional<ClassInfo*> commonBase;
  Optional<hphp_fast_set<ClassInfo*>> commonInterfaces;

  // The algorithm for intersecting two intersection lists is similar
  // to unioning, except we only need to consider the classes which
  // are subclasses of all the classes in *both* lists.

  assertx(!classes1.empty());
  assertx(!classes2.empty());

  auto const unresolved1 = (bool)classes1[0].val.left();
  auto const unresolved2 = (bool)classes2[0].val.left();
  if (debug) {
    if (unresolved1) {
      for (auto const c : classes1) always_assert(c.val.left());
    }
    if (unresolved2) {
      for (auto const c : classes2) always_assert(c.val.left());
    }
  }

  // Intersection of a list of all resolved classes and a list of all
  // unresolved classes (or vice versa) is always going to be the
  // empty set (an unresolved class cannot be a resolved class
  // ever). Otherwise if both are unresolved, just combine the lists
  // and canonicalize them.
  if (unresolved1) {
    if (unresolved2) {
      for (auto const c : classes1) common.emplace_back(c);
      for (auto const c : classes2) common.emplace_back(c);
      return canonicalizeIsects(common);
    }
    return {};
  } else if (unresolved2) {
    return {};
  }

  // Since we're calculating the intersection, we only have to visit
  // one list, and check against the other.
  visitEverySub(
    classes1,
    true,
    [&] (res::Class c) {
      // Must have a cinfo because we checked above all the classes
      // were resolved.
      auto const cinfo = c.val.right();
      assertx(cinfo);

      // Could this class be a class in the other list? If not, ignore
      // it (it's not part of the intersection result).
      for (auto const other : classes2) {
        if (!c.exactCouldBe(other)) return true;
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
  return canonicalizeIsects(common);
}

Func::Func(Rep val)
  : val(val)
{}

SString Func::name() const {
  return match<SString>(
    val,
    [&] (FuncName s)   { return s.name; },
    [&] (MethodName s) { return s.name; },
    [&] (FuncInfo* fi) { return fi->func->name; },
    [&] (const MethTabEntryPair* mte) { return mte->first; },
    [&] (FuncFamily* fa) -> SString {
      return fa->possibleFuncs().front()->first;
    },
    [&] (MethodOrMissing m) { return m.mte->first; },
    [&] (const Isect& i) -> SString {
      assertx(i.families.size() > 1);
      return i.families[0]->possibleFuncs().front()->first;
    }
  );
}

const php::Func* Func::exactFunc() const {
  using Ret = const php::Func*;
  return match<Ret>(
    val,
    [&](FuncName)                    { return Ret{}; },
    [&](MethodName)                  { return Ret{}; },
    [&](FuncInfo* fi)                { return fi->func; },
    [&](const MethTabEntryPair* mte) { return mte->second.func; },
    [&](FuncFamily*)                 { return Ret{}; },
    [&](MethodOrMissing)             { return Ret{}; },
    [&](const Isect&)                { return Ret{}; }
  );
}

bool Func::isFoldable() const {
  return match<bool>(
    val,
    [&](FuncName)   { return false; },
    [&](MethodName) { return false; },
    [&](FuncInfo* fi) {
      return fi->func->attrs & AttrIsFoldable;
    },
    [&](const MethTabEntryPair* mte) {
      return mte->second.func->attrs & AttrIsFoldable;
    },
    [&](FuncFamily*)     { return false; },
    [&](MethodOrMissing) { return false; },
    [&](const Isect&)    { return false; }
  );
}

bool Func::couldHaveReifiedGenerics() const {
  return match<bool>(
    val,
    [&](FuncName s) { return true; },
    [&](MethodName) { return true; },
    [&](FuncInfo* fi) { return fi->func->isReified; },
    [&](const MethTabEntryPair* mte) {
      return mte->second.func->isReified;
    },
    [&](FuncFamily* fa) { return fa->m_static->m_maybeReified; },
    [&](MethodOrMissing m) {
      return m.mte->second.func->isReified;
    },
    [&](const Isect& i) {
      for (auto const ff : i.families) {
        if (!ff->m_static->m_maybeReified) return false;
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
    [&](FuncInfo* fi) {
      return dyn_call_error_level(fi->func) > 0;
    },
    [&](const MethTabEntryPair* mte) {
      return dyn_call_error_level(mte->second.func) > 0;
    },
    [&](FuncFamily* fa) { return fa->m_static->m_maybeCaresAboutDynCalls; },
    [&](MethodOrMissing m) {
      return dyn_call_error_level(m.mte->second.func) > 0;
    },
    [&](const Isect& i) {
      for (auto const ff : i.families) {
        if (!ff->m_static->m_maybeCaresAboutDynCalls) return false;
      }
      return true;
    }
  );
}

bool Func::mightBeBuiltin() const {
  return match<bool>(
    val,
    // Builtins are always uniquely resolvable unless renaming is
    // involved.
    [&](FuncName s) { return s.renamable; },
    [&](MethodName) { return true; },
    [&](FuncInfo* fi) { return fi->func->attrs & AttrBuiltin; },
    [&](const MethTabEntryPair* mte) {
      return mte->second.func->attrs & AttrBuiltin;
    },
    [&](FuncFamily* fa) { return fa->m_static->m_maybeBuiltin; },
    [&](MethodOrMissing m) {
      return m.mte->second.func->attrs & AttrBuiltin;
    },
    [&](const Isect& i) {
      for (auto const ff : i.families) {
        if (!ff->m_static->m_maybeBuiltin) return false;
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
    [&] (FuncInfo* fi) { return numNVArgs(*fi->func); },
    [&] (const MethTabEntryPair* mte) { return numNVArgs(*mte->second.func); },
    [&] (FuncFamily* fa) { return fa->m_static->m_minNonVariadicParams; },
    [&] (MethodOrMissing m) { return numNVArgs(*m.mte->second.func); },
    [&] (const Isect& i) {
      uint32_t nv = 0;
      for (auto const ff : i.families) {
        nv = std::max(nv, ff->m_static->m_minNonVariadicParams);
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
    [&] (FuncInfo* fi) { return numNVArgs(*fi->func); },
    [&] (const MethTabEntryPair* mte) { return numNVArgs(*mte->second.func); },
    [&] (FuncFamily* fa) { return fa->m_static->m_maxNonVariadicParams; },
    [&] (MethodOrMissing m) { return numNVArgs(*m.mte->second.func); },
    [&] (const Isect& i) {
      auto nv = std::numeric_limits<uint32_t>::max();
      for (auto const ff : i.families) {
        nv = std::min(nv, ff->m_static->m_maxNonVariadicParams);
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
    [&] (FuncInfo* fi) { return &fi->func->requiredCoeffects; },
    [&] (const MethTabEntryPair* mte) {
      return &mte->second.func->requiredCoeffects;
    },
    [&] (FuncFamily* fa) {
      return fa->m_static->m_requiredCoeffects.get_pointer();
    },
    [&] (MethodOrMissing m) {
      return &m.mte->second.func->requiredCoeffects;
    },
    [&] (const Isect& i) {
      const RuntimeCoeffects* coeffects = nullptr;
      for (auto const ff : i.families) {
        if (!ff->m_static->m_requiredCoeffects) continue;
        assertx(
          IMPLIES(
            coeffects,
            *coeffects == *ff->m_static->m_requiredCoeffects
          )
        );
        if (!coeffects) {
          coeffects = ff->m_static->m_requiredCoeffects.get_pointer();
        }
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
    [&] (FuncInfo* fi) { return &fi->func->coeffectRules; },
    [&] (const MethTabEntryPair* mte) {
      return &mte->second.func->coeffectRules;
    },
    [&] (FuncFamily* fa) {
      return fa->m_static->m_coeffectRules.get_pointer();
    },
    [&] (MethodOrMissing m) {
      return &m.mte->second.func->coeffectRules;
    },
    [&] (const Isect& i) {
      const CompactVector<CoeffectRule>* coeffects = nullptr;
      for (auto const ff : i.families) {
        if (!ff->m_static->m_coeffectRules) continue;
        assertx(
          IMPLIES(
            coeffects,
            std::is_permutation(
              begin(*coeffects),
              end(*coeffects),
              begin(*ff->m_static->m_coeffectRules),
              end(*ff->m_static->m_coeffectRules)
            )
          )
        );
        if (!coeffects) {
          coeffects = ff->m_static->m_coeffectRules.get_pointer();
        }
      }
      return coeffects;
    }
  );
}

std::string show(const Func& f) {
  auto ret = f.name()->toCppString();
  match<void>(
    f.val,
    [&](Func::FuncName s)        { if (s.renamable) ret += '?'; },
    [&](Func::MethodName)        {},
    [&](FuncInfo*)               { ret += "*"; },
    [&](const MethTabEntryPair*) { ret += "*"; },
    [&](FuncFamily*)             { ret += "+"; },
    [&](Func::MethodOrMissing)   { ret += "-"; },
    [&](const Func::Isect&)      { ret += "&"; }
  );
  return ret;
}

}

//////////////////////////////////////////////////////////////////////

using IfaceSlotMap = hphp_fast_map<const php::Class*, Slot>;

// Inferred class constant type from a 86cinit.
struct ClsConstInfo {
  Type type;
  size_t refinements = 0;
};

struct Index::IndexData {
  explicit IndexData(Index* index) : m_index{index} {}
  IndexData(const IndexData&) = delete;
  IndexData& operator=(const IndexData&) = delete;
  ~IndexData() {
    if (compute_iface_vtables.joinable()) {
      compute_iface_vtables.join();
    }
  }

  Index* m_index;

  bool frozen{false};
  bool ever_frozen{false};

  std::unique_ptr<php::Program> program;

  ISStringToOneT<php::Class*>      classes;
  SStringToMany<php::Func>         methods;
  ISStringToOneT<php::Func*>       funcs;
  ISStringToOneT<php::TypeAlias*>  typeAliases;
  ISStringToOneT<php::Class*>      enums;
  SStringToOneT<php::Constant*>    constants;
  SStringToOneT<php::Module*>      modules;
  SStringToOneT<php::Unit*>        units;

  /*
   * Func families representing methods with a particular name (across
   * all classes). If only one method with a particular name exists,
   * it will be present in singleMethodFamilies instead (which saves
   * space by not requiring a FuncFamily).
   */
  SStringToOneT<FuncFamily*>             methodFamilies;
  SStringToOneT<const MethTabEntryPair*> singleMethodFamilies;

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
   * All the ClassInfos, sorted topologically (ie all the parents,
   * interfaces and traits used by the ClassInfo at index K will have
   * indices less than K). This mostly drops out of the way ClassInfos
   * are created; it would be hard to create the ClassInfos for the
   * php::Class X (or even know how many to create) without knowing
   * all the ClassInfos that were created for X's dependencies.
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
  IfaceSlotMap ifaceSlotMap;

  hphp_hash_map<
    const php::Class*,
    CompactVector<Type>
  > closureUseVars;

  struct ClsConstTypesHasher {
    bool operator()(const std::pair<const php::Class*, SString>& k) const {
      return hash_int64_pair(uintptr_t(k.first), k.second->hash());
    }
  };
  struct ClsConstTypesEquals {
    bool operator()(const std::pair<const php::Class*, SString>& a,
                    const std::pair<const php::Class*, SString>& b) const {
      return a.first == b.first && a.second->same(b.second);
    }
  };
  folly_concurrent_hash_map_simd<
    std::pair<const php::Class*, SString>,
    ClsConstInfo,
    ClsConstTypesHasher,
    ClsConstTypesEquals
  > clsConstTypes;

  // Cache for lookup_class_constant
  folly_concurrent_hash_map_simd<
    std::pair<const php::Class*, SString>,
    ClsConstLookupResult<>,
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

  std::thread compute_iface_vtables;
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

DependencyContext dep_context(IndexData& data, const Context& ctx) {
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

std::mutex func_info_mutex;

FuncInfo* create_func_info(IndexData& data, const php::Func* f) {
  auto fi = &data.funcInfo[f->idx];
  if (UNLIKELY(fi->func == nullptr)) {
    if (f->isNative) {
      std::lock_guard<std::mutex> g{func_info_mutex};
      if (fi->func) {
        assertx(fi->func == f);
        return fi;
      }
      // We'd infer this anyway when we look at the bytecode body
      // (NativeImpl) for the HNI function, but just initializing it
      // here saves on whole-program iterations.
      fi->returnTy = native_function_return_type(f);
    }
    fi->func = f;
  }

  assertx(fi->func == f);
  return fi;
}

FuncInfo* func_info(IndexData& data, const php::Func* f) {
  auto const fi = &data.funcInfo[f->idx];
  return fi;
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

struct TraitMethod {
  using class_type = const ClassInfo*;
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
    return traitCls->cls->name;
  }

  // Return the name for the trait method.
  static const string_type methName(method_type meth) {
    return meth->name;
  }

  // Return the name of the trait where the method was originally defined
  static origin_type originalClass(method_type meth) {
    return meth->originalClass ? meth->originalClass : meth->cls->name;
  }

  // Is-a methods.
  static bool isTrait(class_type traitCls) {
    return traitCls->cls->attrs & AttrTrait;
  }
  static bool isAbstract(Attr modifiers) {
    return modifiers & AttrAbstract;
  }

  // Whether to exclude methods with name `methName' when adding.
  static bool exclude(string_type methName) {
    return Func::isSpecial(methName);
  }

  // TraitMethod constructor.
  static TraitMethod traitMethod(class_type traitCls,
                                 method_type traitMeth,
                                 const PreClass::TraitAliasRule& rule) {
    return TraitMethod { traitCls, traitMeth, rule.modifiers() };
  }

  // Register a trait alias once the trait class is found.
  static void addTraitAlias(const ClassInfo* /*cls*/,
                            const PreClass::TraitAliasRule& /*rule*/,
                            class_type /*traitCls*/) {
    // purely a runtime thing... nothing to do
  }

  // Trait class/method finders.
  static class_type findSingleTraitWithMethod(class_type cls,
                                              string_type origMethName) {
    class_type traitCls = nullptr;

    for (auto const t : cls->usedTraits) {
      // Note: m_methods includes methods from parents/traits recursively.
      if (t->methods.count(origMethName)) {
        if (traitCls != nullptr) {
          return nullptr;
        }
        traitCls = t;
      }
    }
    return traitCls;
  }

  static class_type findTraitClass(class_type cls,
                                   string_type traitName) {
    for (auto const t : cls->usedTraits) {
      if (traitName->isame(t->cls->name)) return t;
    }
    return nullptr;
  }

  static method_type findTraitMethod(class_type traitCls,
                                     string_type origMethName) {
    auto it = traitCls->methods.find(origMethName);
    if (it == traitCls->methods.end()) return nullptr;
    return it->second.func;
  }

  // Errors.
  static void errorUnknownMethod(string_type methName) {
    throw TMIException(folly::sformat("Unknown method '{}'", methName));
  }
  static void errorUnknownTrait(string_type traitName) {
    throw TMIException(folly::sformat("Unknown trait '{}'", traitName));
  }
  static void errorDuplicateMethod(class_type cls,
                                   string_type methName,
                                   const std::vector<const StringData*>&) {
    auto const& m = cls->cls->methods;
    if (std::find_if(m.begin(), m.end(),
                     [&] (auto const& f) {
                       return f->name->same(methName);
                     }) != m.end()) {
      // the duplicate methods will be overridden by the class method.
      return;
    }
    throw TMIException(folly::sformat("DuplicateMethod: {}", methName));
  }
  static void errorInconsistentInsteadOf(class_type cls,
                                         string_type methName) {
    throw TMIException(folly::sformat("InconsistentInsteadOf: {} {}",
                                      methName, cls->cls->name));
  }
  static void errorMultiplyExcluded(string_type traitName,
                                    string_type methName) {
    throw TMIException(folly::sformat("MultiplyExcluded: {}::{}",
                                      traitName, methName));
  }
};

using TMIData = TraitMethodImportData<TraitMethod,
                                      TMIOps>;

struct ClsPreResolveUpdates {
  TinyVector<std::unique_ptr<ClassInfo>> newInfos;
  TinyVector<ClassInfo*> updateDeps;

  struct CnsHash {
    size_t operator()(const ClassInfo::ConstIndex& cns) const {
      return hash_int64_pair((uintptr_t)cns.cls, cns.idx);
    }
  };
  struct CnsEquals {
    bool operator()(const ClassInfo::ConstIndex& cns1,
                    const ClassInfo::ConstIndex& cns2) const {
      return
        cns1.cls == cns2.cls &&
        cns1.idx == cns2.idx;
    }
  };

  hphp_fast_map<
    const php::Class*,
    hphp_fast_set<php::Func*>
  > extraMethods;
  hphp_fast_map<
    const php::Class*,
    CompactVector<php::Class*>
  > closures;
  CompactVector<php::Class*> newClosures;
  CompactVector<
    std::tuple<std::unique_ptr<php::Class>, php::Unit*, uint32_t>
  > newClasses;
  CompactVector<php::Func*> newMethods;

  uint32_t nextClassId = 0;
};

// Keep track of order of closure creation to make the logic more
// deterministic.
struct ClonedClosureMap {
  using Tuple = std::tuple<const php::Class*,
                           std::unique_ptr<php::Class>,
                           uint32_t>;

  bool empty() const { return ordered.empty(); }

  CompactVector<Tuple>::iterator find(const php::Class* cls) {
    auto const it = map.find(cls);
    if (it == map.end()) return ordered.end();
    auto const idx = it->second;
    assertx(idx < ordered.size());
    assertx(std::get<0>(ordered[idx]) == cls);
    return ordered.begin() + idx;
  }

  bool emplace(const php::Class* cls,
               std::unique_ptr<php::Class> clo,
               uint32_t id) {
    auto const inserted = map.emplace(cls, ordered.size()).second;
    if (!inserted) return false;
    ordered.emplace_back(cls, std::move(clo), id);
    return true;
  }

  CompactVector<Tuple>::iterator begin() {
    return ordered.begin();
  }
  CompactVector<Tuple>::iterator end() {
    return ordered.end();
  }

private:
  hphp_fast_map<const php::Class*, size_t> map;
  CompactVector<Tuple> ordered;
};

std::unique_ptr<php::Func> clone_meth(IndexData& index,
                                      php::Class* newContext,
                                      const php::Func* origMeth,
                                      SString name,
                                      Attr attrs,
                                      ClsPreResolveUpdates& updates,
                                      ClonedClosureMap& clonedClosures);
/*
 * Make a flattened table of the constants on this class.
 */
bool build_class_constants(IndexData& index,
                           ClassInfo* cinfo,
                           ClsPreResolveUpdates& updates) {
  if (cinfo->parent) cinfo->clsConstants = cinfo->parent->clsConstants;

  auto const add = [&] (const ClassInfo::ConstIndex& cns, bool fromTrait) {
    auto insert = cinfo->clsConstants.emplace(cns->name, cns);
    if (insert.second) {
      if (fromTrait) {
        cinfo->preResolveState->constsFromTraits.emplace(cns->name);
      }
      return true;
    }
    auto& existing = insert.first->second;

    // Same constant (from an interface via two different paths) is ok
    if (existing->cls == cns->cls) return true;

    if (existing->kind != cns->kind) {
      ITRACE(
        2,
        "build_class_constants failed for `{}' because `{}' was defined by "
        "`{}' as a {} and by `{}' as a {}\n",
        cinfo->cls->name,
        cns->name,
        cns->cls->name,
        ConstModifiers::show(cns->kind),
        existing->cls->name,
        ConstModifiers::show(existing->kind)
      );
      return false;
    }

    // Ignore abstract constants
    if (cns->isAbstract && !cns->val) return true;

    // if the existing constant in the map is concrete, then don't
    // overwrite it with an incoming abstract constant's default
    if (!existing->isAbstract && cns->isAbstract) {
      return true;
    }

    if (existing->val) {
      // A constant from a declared interface collides with a constant
      // (Excluding constants from interfaces a trait implements)
      // Need this check otherwise constants from traits that conflict with
      // declared interfaces will silently lose and not conflict in the runtime
      // Type and Context constants can be overriden.
      if (cns->kind == ConstModifiers::Kind::Value &&
          !existing->isAbstract &&
          existing->cls->attrs & AttrInterface &&
          !(cns->cls->attrs & AttrInterface && fromTrait)) {
        for (auto const& interface : cinfo->declInterfaces) {
          if (existing->cls == interface->cls) {
            ITRACE(
              2,
              "build_class_constants failed for `{}' because "
              "`{}' was defined by both `{}' and `{}'\n",
              cinfo->cls->name,
              cns->name,
              cns->cls->name,
              existing->cls->name
            );
            return false;
          }
        }
      }

      if (!RO::EvalTraitConstantInterfaceBehavior) {
        // Constants from traits silently lose
        if (fromTrait) return true;
      }

      if ((cns->cls->attrs & AttrInterface ||
           (RO::EvalTraitConstantInterfaceBehavior && (cns->cls->attrs & AttrTrait))) &&
          existing->isAbstract) {
        // because existing has val, this covers the case where it is
        // abstract with default allow incoming to win
      } else {
        // A constant from an interface or from an included enum collides
        // with an existing constant.
        if (cns->cls->attrs & (AttrInterface | AttrEnum | AttrEnumClass) ||
            (RO::EvalTraitConstantInterfaceBehavior && (cns->cls->attrs & AttrTrait))) {
          ITRACE(
            2,
            "build_class_constants failed for `{}' because "
            "`{}' was defined by both `{}' and `{}'\n",
            cinfo->cls->name,
            cns->name,
            cns->cls->name,
            existing->cls->name
          );
          return false;
        }
      }
    }

    existing = cns;
    if (fromTrait) {
      cinfo->preResolveState->constsFromTraits.emplace(cns->name);
    } else {
      cinfo->preResolveState->constsFromTraits.erase(cns->name);
    }
    return true;
  };

  for (auto const iface : cinfo->declInterfaces) {
    for (auto const& cns : iface->clsConstants) {
      if (!add(cns.second,
               iface->preResolveState->constsFromTraits.count(cns.first))) {
        return false;
      }
    }
  }

  auto const addShallowConstants = [&]() {
    for (uint32_t idx = 0; idx < cinfo->cls->constants.size(); ++idx) {
      auto const cns = ClassInfo::ConstIndex { cinfo->cls, idx };
      if (!add(cns, false)) return false;
    }
    return true;
  };

  auto const addTraitConstants = [&]() {
    for (auto const trait : cinfo->usedTraits) {
      for (auto const& cns : trait->clsConstants) {
        if (!add(cns.second, true)) return false;
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

  for (auto const ienum : cinfo->includedEnums) {
    for (auto const& cns : ienum->clsConstants) {
      if (!add(cns.second, false)) return false;
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
    auto const& existing = cinfo->clsConstants.find(c.name);
    if (existing->second->cls == c.cls) {
      cinfo->traitConsts.emplace_back(c);
      cinfo->traitConsts.back().isFromTrait = true;
    }
  };
  for (auto const t : cinfo->usedTraits) {
    for (auto const& c : t->cls->constants) addTraitConst(c);
    for (auto const& c : t->traitConsts)    addTraitConst(c);
  }

  if (!(cinfo->cls->attrs & (AttrAbstract | AttrInterface | AttrTrait))) {
    // If we are in a concrete class, concretize the defaults of
    // inherited abstract constants
    auto const cls = const_cast<php::Class*>(cinfo->cls);
    for (auto t : cinfo->clsConstants) {
      auto const& cns = *t.second;
      if (cns.isAbstract && cns.val) {
        if (cns.val.value().m_type == KindOfUninit) {
          // We need to copy the constant's initializer into this class
          auto const& cns_86cinit = cns.cls->methods.back().get();
          assertx(cns_86cinit->name == s_86cinit.get());

          std::unique_ptr<php::Func> empty;
          auto& current_86cinit = [&] () -> std::unique_ptr<php::Func>& {
            for (auto& m : cls->methods) {
              if (m->name == cns_86cinit->name) return m;
            }
            return empty;
          }();

          if (!current_86cinit) {
            ClonedClosureMap clonedClosures;
            current_86cinit = clone_meth(
              index,
              cls,
              cns_86cinit,
              cns_86cinit->name,
              cns_86cinit->attrs,
              updates,
              clonedClosures
            );
            assertx(clonedClosures.empty());
            DEBUG_ONLY auto res = cinfo->methods.emplace(
              current_86cinit->name,
              MethTabEntry { current_86cinit.get(), current_86cinit->attrs, false, true }
            );
            assertx(res.second);
            cls->methods.push_back(std::move(current_86cinit));
          } else {
            append_86cinit(current_86cinit.get(), *cns_86cinit);
          }

        }
        auto concretizedCns = cns;
        concretizedCns.cls = cls;
        concretizedCns.isAbstract = false;

        // this is similar to trait constant flattening
        cls->constants.push_back(concretizedCns);
        cinfo->clsConstants[concretizedCns.name].cls = cls;
        cinfo->clsConstants[concretizedCns.name].idx = cls->constants.size() - 1;
      }
    }
  }

  return true;
}

bool build_class_impl_interfaces(ClassInfo* cinfo) {
  if (cinfo->parent) cinfo->implInterfaces = cinfo->parent->implInterfaces;

  for (auto const ienum : cinfo->includedEnums) {
    cinfo->implInterfaces.insert(
      ienum->implInterfaces.begin(),
      ienum->implInterfaces.end()
    );
  }

  for (auto const iface : cinfo->declInterfaces) {
    cinfo->implInterfaces.insert(
      iface->implInterfaces.begin(),
      iface->implInterfaces.end()
    );
  }

  for (auto const trait : cinfo->usedTraits) {
    cinfo->implInterfaces.insert(
      trait->implInterfaces.begin(),
      trait->implInterfaces.end()
    );
  }

  if (cinfo->cls->attrs & AttrInterface) {
    cinfo->implInterfaces.emplace(cinfo->cls->name, cinfo);
  }

  return true;
}

bool build_class_properties(ClassInfo* cinfo) {
  if (cinfo->parent) {
    cinfo->preResolveState->pbuildNoTrait =
      cinfo->parent->preResolveState->pbuildNoTrait;
    cinfo->preResolveState->pbuildTrait =
      cinfo->parent->preResolveState->pbuildNoTrait;
  }

  auto const add = [&] (auto& m,
                        SString name,
                        const php::Prop& p,
                        const ClassInfo* cls,
                        bool add) {
    auto res = m.emplace(name, std::make_pair(p, cls));
    if (res.second) {
      if (add) cinfo->traitProps.emplace_back(p);
      return true;
    }

    auto const& prev = res.first->second.first;

    if (cinfo == res.first->second.second) {
      if ((prev.attrs ^ p.attrs) &
          (AttrStatic | AttrPublic | AttrProtected | AttrPrivate) ||
          (!(p.attrs & AttrSystemInitialValue) &&
           !(prev.attrs & AttrSystemInitialValue) &&
           !Class::compatibleTraitPropInit(prev.val, p.val))) {
        ITRACE(2,
               "build_class_properties failed for `{}' because "
               "two declarations of `{}' at the same level had "
               "different attributes\n",
               cinfo->cls->name, p.name);
        return false;
      }
      return true;
    }

    if (!(prev.attrs & AttrPrivate)) {
      if ((prev.attrs ^ p.attrs) & AttrStatic) {
        ITRACE(2,
               "build_class_properties failed for `{}' because "
               "`{}' was defined both static and non-static\n",
               cinfo->cls->name, p.name);
        return false;
      }
      if (p.attrs & AttrPrivate) {
        ITRACE(2,
               "build_class_properties failed for `{}' because "
               "`{}' was re-declared private\n",
               cinfo->cls->name, p.name);
        return false;
      }
      if (p.attrs & AttrProtected && !(prev.attrs & AttrProtected)) {
        ITRACE(2,
               "build_class_properties failed for `{}' because "
               "`{}' was redeclared protected from public\n",
               cinfo->cls->name, p.name);
        return false;
      }
    }

    if (add) cinfo->traitProps.emplace_back(p);
    res.first->second = std::make_pair(p, cls);
    return true;
  };

  auto const merge = [&] (const ClassInfo::PreResolveState& src) {
    for (auto const& p : src.pbuildNoTrait) {
      if (!add(cinfo->preResolveState->pbuildNoTrait, p.first,
               p.second.first, p.second.second, false)) {
        return false;
      }
    }
    for (auto const& p : src.pbuildTrait) {
      if (!add(cinfo->preResolveState->pbuildTrait, p.first,
               p.second.first, p.second.second, false)) {
        return false;
      }
    }
    return true;
  };

  for (auto const iface : cinfo->declInterfaces) {
    if (!merge(*iface->preResolveState)) return false;
  }

  for (auto const trait : cinfo->usedTraits) {
    if (!merge(*trait->preResolveState)) return false;
  }

  for (auto const ienum : cinfo->includedEnums) {
    if (!merge(*ienum->preResolveState)) return false;
  }

  if (!(cinfo->cls->attrs & AttrInterface)) {
    for (auto const& p : cinfo->cls->properties) {
      if (!add(cinfo->preResolveState->pbuildNoTrait,
               p.name, p, cinfo, false)) {
        return false;
      }
    }

    // There's no need to do this work if traits have been flattened
    // already, or if the top level class has no traits.  In those
    // cases, we might be able to rule out some ClassInfo
    // instantiations, but it doesn't seem worth it.

    if (!(cinfo->cls->attrs & AttrNoExpandTrait)) {
      for (auto const trait : cinfo->usedTraits) {
        for (auto const& p : trait->cls->properties) {
          if (!add(cinfo->preResolveState->pbuildNoTrait,
                   p.name, p, cinfo, true)) {
            return false;
          }
        }
        for (auto const& p : trait->traitProps) {
          if (!add(cinfo->preResolveState->pbuildNoTrait,
                   p.name, p, cinfo, true)) {
            return false;
          }
        }
      }
    }
  }

  return true;
}

const StaticString s___EnableMethodTraitDiamond("__EnableMethodTraitDiamond");

bool enable_method_trait_diamond(const ClassInfo* cinfo) {
  assertx(cinfo->cls);
  auto const cls_attrs = cinfo->cls->userAttributes;
  return cls_attrs.find(s___EnableMethodTraitDiamond.get()) != cls_attrs.end();
}

/*
 * Make a flattened table of the methods on this class.
 *
 * Duplicate method names override parent methods, unless the parent method
 * is final and the class is not a __MockClass, in which case this class
 * definitely would fatal if ever defined.
 *
 * Note: we're leaving non-overridden privates in their subclass method
 * table, here.  This isn't currently "wrong", because calling it would be a
 * fatal, but note that resolve_method needs to be pretty careful about
 * privates and overriding in general.
 */
bool build_class_methods(const IndexData& index,
                         ClassInfo* cinfo,
                         ClsPreResolveUpdates& updates) {
  if (cinfo->cls->attrs & AttrInterface) return true;

  auto const methodOverride = [&] (auto& it,
                                   const php::Func* meth,
                                   Attr attrs,
                                   SString name) {
    if (it->second.func->attrs & AttrFinal) {
      if (!is_mock_class(cinfo->cls)) {
        ITRACE(2,
               "build_class_methods failed for `{}' because "
               "it tried to override final method `{}::{}'\n",
               cinfo->cls->name,
               it->second.func->cls->name, name);
        return false;
      }
    }
    ITRACE(9,
           "  {}: overriding method {}::{} with {}::{}\n",
           cinfo->cls->name,
           it->second.func->cls->name, it->second.func->name,
           meth->cls->name, name);
    if (it->second.func->attrs & AttrPrivate) {
      it->second.hasPrivateAncestor = true;
    }
    it->second.func = meth;
    it->second.attrs = attrs;
    it->second.hasAncestor = true;
    it->second.topLevel = true;
    assertx(it->first == name);
    return true;
  };

  // If there's a parent, start by copying its methods
  if (auto const rparent = cinfo->parent) {
    for (auto& mte : rparent->methods) {
      // don't inherit the 86* methods.
      if (HPHP::Func::isSpecial(mte.first)) continue;
      auto const res = cinfo->methods.emplace(mte.first, mte.second);
      assertx(res.second);
      res.first->second.topLevel = false;
      ITRACE(9,
             "  {}: inheriting method {}::{}\n",
             cinfo->cls->name,
             rparent->cls->name, mte.first);
      continue;
    }
  }

  uint32_t idx = cinfo->methods.size();

  // Now add our methods.
  for (auto& m : cinfo->cls->methods) {
    auto res = cinfo->methods.emplace(
      m->name,
      MethTabEntry { m.get(), m->attrs, false, true }
    );
    if (res.second) {
      res.first->second.idx = idx++;
      ITRACE(9,
             "  {}: adding method {}::{}\n",
             cinfo->cls->name,
             cinfo->cls->name, m->name);
      continue;
    }
    if (m->attrs & AttrTrait && m->attrs & AttrAbstract) {
      // abstract methods from traits never override anything.
      continue;
    }
    if (!methodOverride(res.first, m.get(), m->attrs, m->name)) return false;
  }

  // If our traits were previously flattened, we're done.
  if (cinfo->cls->attrs & AttrNoExpandTrait) return true;

  try {
    TMIData tmid;
    for (auto const t : cinfo->usedTraits) {
      std::vector<const MethTabEntryPair*> methods(t->methods.size());
      for (auto& m : t->methods) {
        if (HPHP::Func::isSpecial(m.first)) continue;
        assertx(!methods[m.second.idx]);
        methods[m.second.idx] = mteFromElm(m);
      }
      for (auto const m : methods) {
        if (!m) continue;
        TraitMethod traitMethod { t, m->second.func, m->second.attrs };
        tmid.add(traitMethod, m->first);
      }
      if (auto const it = index.classClosureMap.find(t->cls);
          it != index.classClosureMap.end()) {
        for (auto const& c : it->second) {
          auto const invoke = find_method(c, s_invoke.get());
          assertx(invoke);
          updates.extraMethods[cinfo->cls].emplace(invoke);
        }
      }
    }

    auto traitMethods = tmid.finish(cinfo, enable_method_trait_diamond(cinfo));
    // Import the methods.
    for (auto const& mdata : traitMethods) {
      auto method = mdata.tm.method;
      auto attrs = mdata.tm.modifiers;
      if (attrs == AttrNone) {
        attrs = method->attrs;
      } else {
        Attr attrMask = (Attr)(AttrPublic | AttrProtected | AttrPrivate |
                               AttrAbstract | AttrFinal);
        attrs = (Attr)((attrs         &  attrMask) |
                       (method->attrs & ~attrMask));
      }
      auto res = cinfo->methods.emplace(
        mdata.name,
        MethTabEntry { method, attrs, false, true }
      );
      if (res.second) {
        res.first->second.idx = idx++;
        ITRACE(9,
               "  {}: adding trait method {}::{} as {}\n",
               cinfo->cls->name,
               method->cls->name, method->name, mdata.name);
      } else {
        if (attrs & AttrAbstract) continue;
        if (res.first->second.func->cls == cinfo->cls) continue;
        if (!methodOverride(res.first, method, attrs, mdata.name)) {
          return false;
        }
        res.first->second.idx = idx++;
      }
      updates.extraMethods[cinfo->cls].emplace(
        const_cast<php::Func*>(method)
      );
    }
  } catch (TMIOps::TMIException& ex) {
    ITRACE(2,
           "build_class_methods failed for `{}' importing traits: {}\n",
           cinfo->cls->name, ex.what());
    return false;
  }

  return true;
}

const StaticString s___Sealed("__Sealed");

bool enforce_in_maybe_sealed_parent_whitelist(
  const ClassInfo* cls,
  const ClassInfo* parent) {
  // if our parent isn't sealed, then we're fine.
  if (!parent || !(parent->cls->attrs & AttrSealed)) return true;
  const UserAttributeMap& parent_attrs = parent->cls->userAttributes;
  assertx(parent_attrs.find(s___Sealed.get()) != parent_attrs.end());
  const auto& parent_sealed_attr = parent_attrs.find(s___Sealed.get())->second;
  bool in_sealed_whitelist = false;
  IterateV(parent_sealed_attr.m_data.parr,
           [&in_sealed_whitelist, cls](TypedValue v) -> bool {
             if (v.m_data.pstr->same(cls->cls->name)) {
               in_sealed_whitelist = true;
               return true;
             }
             return false;
           });
  return in_sealed_whitelist;
}

/*
 * This function return false if instantiating the cinfo would be a
 * fatal at runtime.
 */
bool build_cls_info(IndexData& index,
                    ClassInfo* cinfo,
                    ClsPreResolveUpdates& updates) {
  if (!enforce_in_maybe_sealed_parent_whitelist(cinfo, cinfo->parent)) {
    return false;
  }

  for (auto const iface : cinfo->declInterfaces) {
    if (!enforce_in_maybe_sealed_parent_whitelist(cinfo, iface)) {
      return false;
    }
  }
  for (auto const trait : cinfo->usedTraits) {
    if (!enforce_in_maybe_sealed_parent_whitelist(cinfo, trait)) {
      return false;
    }
  }
  for (auto const ienum : cinfo->includedEnums) {
    if (!enforce_in_maybe_sealed_parent_whitelist(cinfo, ienum)) {
      return false;
    }
  }

  if (!build_class_constants(index, cinfo, updates)) return false;
  if (!build_class_impl_interfaces(cinfo)) return false;
  if (!build_class_properties(cinfo)) return false;
  if (!build_class_methods(index, cinfo, updates)) return false;
  return true;
}

//////////////////////////////////////////////////////////////////////

uint32_t func_num_inout(const php::Func* func) {
  if (!func->hasInOutArgs) return 0;
  uint32_t count = 0;
  for (auto& p : func->params) count += p.inout;
  return count;
}

TriBool func_supports_AER(const php::Func* func) {
  // Async functions always support async eager return, and no other
  // functions support it yet.
  return yesOrNo(func->isAsync && !func->isGenerator);
}

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
    assertx(cnsPair.second.m_type != KindOfUninit ||
            cnsPair.second.dynamic());
    auto pc = new php::Constant {
      cnsPair.first,
      cnsPair.second,
      AttrUnique | AttrPersistent
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
  assertx(!(c.attrs & AttrNoOverride));

  if (c.attrs & AttrEnum) {
    add_symbol_to_index(index.enums, &c, "enum");
  }

  add_symbol_to_index(index.classes, &c, "class", index.typeAliases);

  for (auto& m : c.methods) {
    attribute_setter(m->attrs, false, AttrNoOverride);
    index.methods.insert({m->name, m.get()});
    m->idx = index.nextFuncId++;
  }
}

bool add_func_to_index(IndexData& index, php::Func& func) {
  if ((func.attrs & AttrIsMethCaller) && index.funcs.count(func.name)) {
    return false;
  }
  add_symbol_to_index(index.funcs, &func, "function");
  func.idx = index.nextFuncId++;
  return true;
}

void add_program_to_index(IndexData& index) {
  trace_time timer{"add program to index"};

  auto& program = *index.program;
  for (auto const& u : program.units) {
    add_unit_to_index(index, *u);
  }
  for (auto const& c : program.classes) {
    add_class_to_index(index, *c);
  }

  for (auto const& c : program.classes) {
    if (!c->closureContextCls) continue;
    auto& s = index.classClosureMap[index.classes.at(c->closureContextCls)];
    s.emplace_back(c.get());
  }

  program.funcs.erase(
    std::remove_if(
      program.funcs.begin(),
      program.funcs.end(),
      [&] (const std::unique_ptr<php::Func>& f) {
        if (add_func_to_index(index, *f)) return false;

        auto unit = index.units.at(f->unit);
        unit->funcs.erase(
          std::remove(
            unit->funcs.begin(),
            unit->funcs.end(),
            f->name
          ),
          unit->funcs.end()
        );

        return true;
      }
    ),
    program.funcs.end()
  );
}

//////////////////////////////////////////////////////////////////////

struct ClassInfoData {
  // Map from name to types that directly use that name (as parent,
  // interface or trait).
  ISStringToOneT<CompactVector<const php::Class*>> users;
  // Map from types to number of dependencies, used in
  // conjunction with users field above.
  hphp_fast_map<const php::Class*, uint32_t> depCounts;

  uint32_t cqFront{};
  uint32_t cqBack{};
  std::vector<const php::Class*> queue;
  bool hasPseudoCycles{};
};

std::unique_ptr<php::Func> clone_meth_helper(
  IndexData& index,
  php::Class* newContext,
  const php::Func* origMeth,
  std::unique_ptr<php::Func> cloneMeth,
  ClsPreResolveUpdates& updates,
  ClonedClosureMap& clonedClosures
);

std::unique_ptr<php::Class> clone_closure(IndexData& index,
                                          const php::Class* newContext,
                                          php::Class* cls,
                                          ClsPreResolveUpdates& updates,
                                          ClonedClosureMap& clonedClosures) {
  auto clone = std::make_unique<php::Class>(*cls);
  assertx(clone->closureContextCls);
  clone->closureContextCls = newContext->name;
  clone->unit = newContext->unit;
  auto i = 0;
  for (auto& cloneMeth : clone->methods) {
    cloneMeth = clone_meth_helper(index,
                                  clone.get(),
                                  cls->methods[i++].get(),
                                  std::move(cloneMeth),
                                  updates,
                                  clonedClosures);
    if (!cloneMeth) return nullptr;
  }
  return clone;
}

std::unique_ptr<php::Func> clone_meth_helper(
  IndexData& index,
  php::Class* newContext,
  const php::Func* origMeth,
  std::unique_ptr<php::Func> cloneMeth,
  ClsPreResolveUpdates& preResolveUpdates,
  ClonedClosureMap& clonedClosures) {

  cloneMeth->cls  = newContext;
  cloneMeth->idx  = index.nextFuncId.fetch_add(1, std::memory_order_relaxed);
  if (!cloneMeth->originalFilename) {
    cloneMeth->originalFilename = origMeth->unit;
  }
  if (!cloneMeth->originalUnit) {
    cloneMeth->originalUnit = origMeth->unit;
  }
  cloneMeth->unit = newContext->unit;
  cloneMeth->originalClass = origMeth->originalClass
    ? origMeth->originalClass
    : origMeth->cls->name;

  preResolveUpdates.newMethods.emplace_back(cloneMeth.get());

  if (!origMeth->hasCreateCl) return cloneMeth;

  auto const origUnit = index.units.at(origMeth->unit);

  auto const recordClosure = [&] (uint32_t& clsId) {
    auto const cls = index.classes.at(origUnit->classes[clsId]);
    auto it = clonedClosures.find(cls);
    if (it == clonedClosures.end()) {
      auto cloned = clone_closure(
        index,
        newContext->closureContextCls
          ? index.classes.at(newContext->closureContextCls)
          : newContext,
        cls,
        preResolveUpdates,
        clonedClosures
      );
      if (!cloned) return false;
      clsId = preResolveUpdates.nextClassId++;
      always_assert(clonedClosures.emplace(cls, std::move(cloned), clsId));
    } else {
      clsId = std::get<2>(*it);
    }
    return true;
  };

  auto mf = php::WideFunc::mut(cloneMeth.get());
  hphp_fast_map<size_t, hphp_fast_map<size_t, uint32_t>> updates;

  for (size_t bid = 0; bid < mf.blocks().size(); bid++) {
    auto const b = mf.blocks()[bid].get();
    for (size_t ix = 0; ix < b->hhbcs.size(); ix++) {
      auto const& bc = b->hhbcs[ix];
      switch (bc.op) {
        case Op::CreateCl: {
          auto clsId = bc.CreateCl.arg2;
          if (!recordClosure(clsId)) return nullptr;
          updates[bid][ix] = clsId;
          break;
        }
        default:
          break;
      }
    }
  }

  for (auto const& elm : updates) {
    auto const blk = mf.blocks()[elm.first].mutate();
    for (auto const& ix : elm.second) {
      blk->hhbcs[ix.first].CreateCl.arg2 = ix.second;
    }
  }

  return cloneMeth;
}

std::unique_ptr<php::Func> clone_meth(IndexData& index,
                                      php::Class* newContext,
                                      const php::Func* origMeth,
                                      SString name,
                                      Attr attrs,
                                      ClsPreResolveUpdates& updates,
                                      ClonedClosureMap& clonedClosures) {

  auto cloneMeth  = std::make_unique<php::Func>(*origMeth);
  cloneMeth->name = name;
  cloneMeth->attrs = attrs | AttrTrait;
  return clone_meth_helper(index, newContext, origMeth,
                           std::move(cloneMeth), updates,
                           clonedClosures);
}

bool merge_inits(IndexData& index,
                 std::vector<std::unique_ptr<php::Func>>& clones,
                 ClassInfo* cinfo,
                 ClsPreResolveUpdates& updates,
                 ClonedClosureMap& clonedClosures,
                 SString xinitName) {
  auto const cls = const_cast<php::Class*>(cinfo->cls);
  std::unique_ptr<php::Func> empty;
  auto& xinit = [&] () -> std::unique_ptr<php::Func>& {
    for (auto& m : cls->methods) {
      if (m->name == xinitName) return m;
    }
    return empty;
  }();

  auto merge_one = [&] (const php::Func* func) {
    if (!xinit) {
      ITRACE(5, "  - cloning {}::{} as {}::{}\n",
             func->cls->name, func->name, cls->name, xinitName);
      xinit = clone_meth(index, cls, func, func->name, func->attrs,
                         updates, clonedClosures);
      return xinit != nullptr;
    }

    ITRACE(5, "  - appending {}::{} into {}::{}\n",
           func->cls->name, func->name, cls->name, xinitName);
    if (xinitName == s_86cinit.get()) {
      return append_86cinit(xinit.get(), *func);
    } else {
      return append_func(xinit.get(), *func);
    }
  };

  for (auto t : cinfo->usedTraits) {
    auto it = t->methods.find(xinitName);
    if (it != t->methods.end()) {
      if (!merge_one(it->second.func)) {
        ITRACE(5, "merge_xinits: failed to merge {}::{}\n",
               it->second.func->cls->name, it->second.func->name);
        return false;
      }
    }
  }

  assertx(xinit);
  if (empty) {
    ITRACE(5, "merge_xinits: adding {}::{} to method table\n",
           xinit->cls->name, xinit->name);
    assertx(&empty == &xinit);
    clones.push_back(std::move(xinit));
  }

  return true;
}

bool merge_xinits(IndexData& index,
                  Attr attr,
                  std::vector<std::unique_ptr<php::Func>>& clones,
                  ClassInfo* cinfo,
                  ClsPreResolveUpdates& updates,
                  ClonedClosureMap& clonedClosures) {
  auto const xinitName = [&]() {
    switch (attr) {
    case AttrNone  : return s_86pinit.get();
    case AttrStatic: return s_86sinit.get();
    case AttrLSB   : return s_86linit.get();
    default: always_assert(false);
    }
  }();

  auto const xinitMatch = [&](Attr prop_attrs) {
    auto mask = AttrStatic | AttrLSB;
    switch (attr) {
    case AttrNone: return (prop_attrs & mask) == AttrNone;
    case AttrStatic: return (prop_attrs & mask) == AttrStatic;
    case AttrLSB: return (prop_attrs & mask) == mask;
    default: always_assert(false);
    }
  };

  for (auto const& p : cinfo->traitProps) {
    if (xinitMatch(p.attrs) &&
        p.val.m_type == KindOfUninit &&
        !(p.attrs & AttrLateInit)) {
      ITRACE(5, "merge_xinits: {}: Needs merge for {}{}prop `{}'\n",
              cinfo->cls->name, attr & AttrStatic ? "static " : "",
              attr & AttrLSB ? "lsb " : "", p.name);
      return merge_inits(index, clones, cinfo,
                         updates, clonedClosures, xinitName);
    }
  }
  return true;
}

bool merge_cinits(IndexData& index,
                  std::vector<std::unique_ptr<php::Func>>& clones,
                  ClassInfo* cinfo,
                  ClsPreResolveUpdates& updates,
                  ClonedClosureMap& clonedClosures) {
  auto const xinitName = s_86cinit.get();
  for (auto const& c : cinfo->traitConsts) {
    if (c.val && c.val->m_type == KindOfUninit) {
      return merge_inits(index, clones, cinfo,
                         updates, clonedClosures, xinitName);
    }
  }
  return true;
}

void rename_closure(const IndexData& index,
                    php::Class* cls,
                    ClsPreResolveUpdates& updates,
                    size_t idx) {
  auto n = cls->name->slice();
  auto const p = n.find(';');
  if (p != std::string::npos) {
    n = n.subpiece(0, p);
  }
  auto const newName = makeStaticString(
    folly::sformat(
      "{};{}-{}",
      n, idx+1, string_sha1(cls->unit->slice())
    )
  );
  always_assert(!index.classes.count(newName));
  cls->name = newName;
  updates.newClosures.emplace_back(cls);
}

void preresolve(IndexData&,
                const php::Class*,
                ClsPreResolveUpdates&);

void flatten_traits(IndexData& index,
                    ClassInfo* cinfo,
                    ClsPreResolveUpdates& updates) {
  bool hasConstProp = false;
  for (auto const t : cinfo->usedTraits) {
    if (t->usedTraits.size() && !(t->cls->attrs & AttrNoExpandTrait)) {
      ITRACE(5, "Not flattening {} because of {}\n",
             cinfo->cls->name, t->cls->name);
      return;
    }
    if (is_noflatten_trait(t->cls)) {
      ITRACE(5, "Not flattening {} because {} is annotated with __NoFlatten\n",
             cinfo->cls->name, t->cls->name);
      return;
    }
    if (t->cls->hasConstProp) hasConstProp = true;
  }
  auto const cls = const_cast<php::Class*>(cinfo->cls);
  if (hasConstProp) cls->hasConstProp = true;
  std::vector<MethTabEntryPair*> methodsToAdd;
  for (auto& ent : cinfo->methods) {
    if (!ent.second.topLevel || ent.second.func->cls == cinfo->cls) {
      continue;
    }
    always_assert(ent.second.func->cls->attrs & AttrTrait);
    methodsToAdd.push_back(mteFromElm(ent));
  }

  auto const it = updates.extraMethods.find(cinfo->cls);

  if (!methodsToAdd.empty()) {
    assertx(it != updates.extraMethods.end());
    std::sort(begin(methodsToAdd), end(methodsToAdd),
              [] (const MethTabEntryPair* a, const MethTabEntryPair* b) {
                return a->second.idx < b->second.idx;
              });
  } else if (debug && it != updates.extraMethods.end()) {
    // When building the ClassInfos, we proactively added all closures
    // from usedTraits to classExtraMethodMap; but now we're going to
    // start from the used methods, and deduce which closures actually
    // get pulled in. Its possible *none* of the methods got used, in
    // which case, we won't need their closures either. To be safe,
    // verify that the only things in classExtraMethodMap are
    // closures.
    for (DEBUG_ONLY auto const f : it->second) {
      assertx(f->isClosureBody);
    }
  }

  std::vector<std::unique_ptr<php::Func>> clones;
  ClonedClosureMap clonedClosures;

  for (auto const ent : methodsToAdd) {
    auto clone = clone_meth(index, cls, ent->second.func, ent->first,
                            ent->second.attrs, updates, clonedClosures);
    if (!clone) {
      ITRACE(5, "Not flattening {} because {}::{} could not be cloned\n",
             cls->name, ent->second.func->cls->name, ent->first);
      return;
    }

    clone->attrs |= AttrTrait;
    ent->second.attrs |= AttrTrait;
    ent->second.func = clone.get();
    clones.push_back(std::move(clone));
  }

  if (cinfo->traitProps.size()) {
    if (!merge_xinits(index, AttrNone, clones, cinfo,
                      updates, clonedClosures) ||
        !merge_xinits(index, AttrStatic, clones, cinfo,
                      updates, clonedClosures) ||
        !merge_xinits(index, AttrLSB, clones, cinfo,
                      updates, clonedClosures)) {
      ITRACE(5, "Not flattening {} because we couldn't merge the 86xinits\n",
             cls->name);
      return;
    }
  }

  // flatten initializers for constants in traits
  if (cinfo->traitConsts.size()) {
    if (!merge_cinits(index, clones, cinfo, updates, clonedClosures)) {
      ITRACE(5, "Not flattening {} because we couldn't merge the 86cinits\n",
             cls->name);
      return;
    }
  }

  // We're now committed to flattening.
  ITRACE(3, "Flattening {}\n", cls->name);
  if (it != updates.extraMethods.end()) it->second.clear();
  for (auto const& p : cinfo->traitProps) {
    ITRACE(5, "  - prop {}\n", p.name);
    cls->properties.push_back(p);
    cls->properties.back().attrs |= AttrTrait;
  }
  cinfo->traitProps.clear();

  for (auto const& c : cinfo->traitConsts) {
    ITRACE(5, "  - const {}\n", c.name);
    cls->constants.push_back(c);
    cls->constants.back().cls = cls;
    cinfo->clsConstants[c.name].cls = cls;
    cinfo->clsConstants[c.name].idx = cls->constants.size()-1;
    cinfo->preResolveState->constsFromTraits.erase(c.name);
  }
  cinfo->traitConsts.clear();

  if (clones.size()) {
    auto cinit = cls->methods.size() &&
      cls->methods.back()->name == s_86cinit.get() ?
      std::move(cls->methods.back()) : nullptr;
    if (cinit) cls->methods.pop_back();
    for (auto& clone : clones) {
      if (is_special_method_name(clone->name)) {
        DEBUG_ONLY auto res = cinfo->methods.emplace(
          clone->name,
          MethTabEntry { clone.get(), clone->attrs, false, true }
        );
        assertx(res.second);
      }
      ITRACE(5, "  - meth {}\n", clone->name);
      cinfo->methods.find(clone->name)->second.func = clone.get();
      if (clone->name == s_86cinit.get()) {
        cinit = std::move(clone);
        continue;
      }
      cls->methods.push_back(std::move(clone));
    }
    if (cinit) cls->methods.push_back(std::move(cinit));

    if (!clonedClosures.empty()) {
      auto& closures = updates.closures[cls];
      for (auto& [orig, clo, idx] : clonedClosures) {
        rename_closure(index, clo.get(), updates, idx);
        ITRACE(5, "  - closure {} as {}\n", orig->name, clo->name);
        assertx(clo->closureContextCls == cls->name);
        assertx(clo->unit == cls->unit);
        closures.emplace_back(clo.get());
        updates.newClasses.emplace_back(
          std::move(clo),
          const_cast<php::Unit*>(index.units.at(cls->unit)),
          idx
        );
        preresolve(index, closures.back(), updates);
      }
    }
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

  hphp_fast_set<PreClass::ClassRequirement, EqHash, EqHash> reqs;

  for (auto const t : cinfo->usedTraits) {
    for (auto const& req : t->cls->requirements) {
      if (reqs.empty()) {
        for (auto const& r : cls->requirements) {
          reqs.insert(r);
        }
      }
      if (reqs.insert(req).second) cls->requirements.push_back(req);
    }
  }

  cls->attrs |= AttrNoExpandTrait;
}

/*
 * Given a static representation of a Hack class, find a possible resolution
 * of the class along with all classes, interfaces and traits in its hierarchy.
 *
 * Returns the resultant ClassInfo, or nullptr if the Hack class
 * cannot be instantiated at runtime.
 */
ClassInfo* resolve_combinations(IndexData& index,
                                const php::Class* cls,
                                ClsPreResolveUpdates& updates) {
  auto cinfo = std::make_unique<ClassInfo>();
  cinfo->cls = cls;
  auto const& map = index.classInfo;
  if (cls->parentName) {
    cinfo->parent   = map.at(cls->parentName);
    cinfo->baseList = cinfo->parent->baseList;
    if (cinfo->parent->cls->attrs & (AttrInterface | AttrTrait)) {
      ITRACE(2,
             "Resolve combinations failed for `{}' because "
             "its parent `{}' is not a class\n",
             cls->name, cls->parentName);
      return nullptr;
    }
  }
  cinfo->baseList.push_back(cinfo.get());

  for (auto& iname : cls->interfaceNames) {
    auto const iface = map.at(iname);
    if (!(iface->cls->attrs & AttrInterface)) {
      ITRACE(2,
             "Resolve combinations failed for `{}' because `{}' "
             "is not an interface\n",
             cls->name, iname);
      return nullptr;
    }
    cinfo->declInterfaces.push_back(iface);
  }

  for (auto& included_enum_name : cls->includedEnumNames) {
    auto const included_enum = map.at(included_enum_name);
    auto const want_attr = cls->attrs & (AttrEnum | AttrEnumClass);
    if (!(included_enum->cls->attrs & want_attr)) {
      ITRACE(2,
             "Resolve combinations failed for `{}' because `{}' "
             "is not an enum{}\n",
             cls->name, included_enum_name,
             want_attr & AttrEnumClass ? " class" : "");
      return nullptr;
    }
    cinfo->includedEnums.push_back(included_enum);
  }

  for (auto& tname : cls->usedTraitNames) {
    auto const trait = map.at(tname);
    if (!(trait->cls->attrs & AttrTrait)) {
      ITRACE(2,
             "Resolve combinations failed for `{}' because `{}' "
             "is not a trait\n",
             cls->name, tname);
      return nullptr;
    }
    cinfo->usedTraits.push_back(trait);
  }

  cinfo->preResolveState = std::make_unique<ClassInfo::PreResolveState>();
  if (!build_cls_info(index, cinfo.get(), updates)) return nullptr;

  ITRACE(2, "  resolved: {}\n", cls->name);
  if (Trace::moduleEnabled(Trace::hhbbc_index, 3)) {
    for (auto const DEBUG_ONLY& iface : cinfo->implInterfaces) {
      ITRACE(3, "    implements: {}\n", iface.second->cls->name);
    }
    for (auto const DEBUG_ONLY& trait : cinfo->usedTraits) {
      ITRACE(3, "          uses: {}\n", trait->cls->name);
    }
  }
  cinfo->baseList.shrink_to_fit();
  updates.newInfos.emplace_back(std::move(cinfo));
  return updates.newInfos.back().get();
}

void preresolve(IndexData& index,
                const php::Class* type,
                ClsPreResolveUpdates& updates) {
  ITRACE(2, "preresolve class: {}:{}\n", type->name, (void*)type);

  auto const resolved = [&] {
    Trace::Indent indent;
    if (debug) {
      if (type->parentName) {
        assertx(index.classInfo.count(type->parentName));
      }
      for (DEBUG_ONLY auto& i : type->interfaceNames) {
        assertx(index.classInfo.count(i));
      }
      for (DEBUG_ONLY auto& t : type->usedTraitNames) {
        assertx(index.classInfo.count(t));
      }
    }
    return resolve_combinations(index, type, updates);
  }();

  ITRACE(3, "preresolve: {}:{} ({} resolutions)\n",
         type->name, (void*)type, resolved ? 1 : 0);

  if (resolved) {
    updates.updateDeps.emplace_back(resolved);

    if (!(type->attrs & AttrNoExpandTrait) &&
        !type->usedTraitNames.empty() &&
        index.classes.count(type->name) == 1) {
      Trace::Indent indent;
      flatten_traits(index, resolved, updates);
    }
  }
}

void compute_subclass_list_rec(IndexData& index,
                               ClassInfo* cinfo,
                               ClassInfo* csub) {
  for (auto const ctrait : csub->usedTraits) {
    auto const ct = const_cast<ClassInfo*>(ctrait);
    ct->subclassList.push_back(cinfo);
    compute_subclass_list_rec(index, cinfo, ct);
  }
}

void compute_included_enums_list_rec(IndexData& index,
                                     ClassInfo* cinfo,
                                     ClassInfo* csub) {
  for (auto const cincluded_enum : csub->includedEnums) {
    auto const cie = const_cast<ClassInfo*>(cincluded_enum);
    cie->subclassList.push_back(cinfo);
    compute_included_enums_list_rec(index, cinfo, cie);
  }
}

void compute_subclass_list(IndexData& index) {
  trace_time _("compute subclass list");
  auto fixupTraits = false;
  auto fixupEnums = false;
  auto const AnyEnum = AttrEnum | AttrEnumClass;
  for (auto& cinfo : index.allClassInfos) {
    if (cinfo->cls->attrs & AttrInterface) continue;
    for (auto& cparent : cinfo->baseList) {
      cparent->subclassList.push_back(cinfo.get());
    }
    if (!(cinfo->cls->attrs & AttrNoExpandTrait) &&
        cinfo->usedTraits.size()) {
      fixupTraits = true;
      compute_subclass_list_rec(index, cinfo.get(), cinfo.get());
    }
    // Add the included enum lists if cinfo is an enum
    if ((cinfo->cls->attrs & AnyEnum) &&
        cinfo->cls->includedEnumNames.size()) {
      fixupEnums = true;
      compute_included_enums_list_rec(index, cinfo.get(), cinfo.get());
    }
    // Also add classes to their interface's subclassLists
    for (auto& ipair : cinfo->implInterfaces) {
      auto impl = const_cast<ClassInfo*>(ipair.second);
      if (cinfo->cls->attrs & (AttrTrait | AnyEnum | AttrAbstract)) {
        impl->abstractSubclassList.emplace_back(cinfo.get());
      } else {
        impl->subclassList.push_back(cinfo.get());
      }
    }
  }

  parallel::for_each(
    index.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      auto& sub = cinfo->subclassList;
      auto& abstract = cinfo->abstractSubclassList;
      std::sort(begin(sub), end(sub));
      std::sort(begin(abstract), end(abstract));

      if ((fixupTraits && cinfo->cls->attrs & AttrTrait) ||
          (fixupEnums && cinfo->cls->attrs & AnyEnum)) {
        // traits and enums can be reached by multiple paths, so we need to
        // uniquify their subclassLists.
        sub.erase(std::unique(begin(sub), end(sub)), end(sub));
      }
      sub.shrink_to_fit();
      abstract.shrink_to_fit();
    }
  );
}

bool define_func_family(IndexData& index, ClassInfo* cinfo,
                        SString name, const php::Func* func = nullptr) {
  FuncFamily::PFuncVec funcs{};
  // If cinfo was provided, we're calculating a func family with a set
  // of methods off a class. Otherwise, we're calculating a func
  // family for all methods with the given name.
  if (cinfo) {
    for (auto const cleaf : cinfo->subclassList) {
      auto const leafFn = [&] () -> const MethTabEntryPair* {
          auto const leafFnIt = cleaf->methods.find(name);
          if (leafFnIt == end(cleaf->methods)) return nullptr;
          return mteFromIt(leafFnIt);
        }();
      if (!leafFn) continue;
      funcs.emplace_back(leafFn);
    }
  } else {
    auto const range = index.methods.equal_range(name);
    for (auto it = range.first; it != range.second; ++it) {
      auto const func = it->second;
      assertx(func->cls);
      // Only include methods for classes which have a ClassInfo,
      // which means they're instantiatable.
      auto const cinfoIt = index.classInfo.find(func->cls->name);
      if (cinfoIt == index.classInfo.end()) continue;
      auto const cinfo = cinfoIt->second;
      auto const methIt = cinfo->methods.find(name);
      if (methIt == cinfo->methods.end()) continue;
      funcs.emplace_back(mteFromIt(methIt));
    }
  }

  if (funcs.empty()) return false;

  std::sort(
    begin(funcs), end(funcs),
    [&] (const MethTabEntryPair* a, const MethTabEntryPair* b) {
      // We want a canonical order for the family. Putting the
      // one corresponding to cinfo first makes sense, because
      // the first one is used as the name for FCall*Method* hint,
      // after that, sort by name so that different case spellings
      // come in the same order.
      if (a->second.func == b->second.func)   return false;
      if (func) {
        if (b->second.func == func) return false;
        if (a->second.func == func) return true;
      }
      if (auto d = a->first->compare(b->first)) {
        if (!func) {
          if (b->first == name) return false;
          if (a->first == name) return true;
        }
        return d < 0;
      }
      return std::less<const void*>{}(a->second.func, b->second.func);
    }
  );
  funcs.erase(
    std::unique(
      begin(funcs), end(funcs),
      [] (const MethTabEntryPair* a, const MethTabEntryPair* b) {
        return a->second.func == b->second.func;
      }
    ),
    end(funcs)
  );

  funcs.shrink_to_fit();

  if (Trace::moduleEnabled(Trace::hhbbc_index, 4)) {
    FTRACE(4, "define_func_family: {}::{}:\n",
           cinfo ? cinfo->cls->name->data() : "*", name);
    for (auto const DEBUG_ONLY func : funcs) {
      FTRACE(4, "  {}::{}\n",
             func->second.func->cls->name,
             func->second.func->name);
    }
  }

  // Single func resolutions are stored separately. They don't need a
  // FuncFamily and this saves space.
  if (funcs.size() == 1) {
    if (cinfo) {
      cinfo->singleMethodFamilies.emplace(name, funcs[0]);
    } else {
      auto it = index.singleMethodFamilies.find(name);
      assertx(it != index.singleMethodFamilies.end());
      assertx(!it->second);
      it->second = funcs[0];
    }
    return true;
  }

  // Otherwise re-use an existing identical FuncFamily, or create a
  // new one.
  auto const ff = [&] {
    auto it = index.funcFamilies.find(funcs);
    if (it != index.funcFamilies.end()) return it->first.get();
    return index.funcFamilies.insert(
      std::make_unique<FuncFamily>(std::move(funcs)),
      false
    ).first->first.get();
  }();

  if (cinfo) {
    cinfo->methodFamilies.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(name),
      std::forward_as_tuple(ff)
    );
  } else {
    auto it = index.methodFamilies.find(name);
    assertx(it != index.methodFamilies.end());
    assertx(!it->second);
    it->second = ff;
  }

  return true;
}

void build_abstract_func_families(IndexData& data, ClassInfo* cinfo) {
  std::vector<SString> extras;

  // We start by collecting the list of methods shared across all
  // subclasses of cinfo (including indirectly). And then add the
  // public methods which are not constructors and have no private
  // ancestors to the method families of cinfo. Note that this set
  // may be larger than the methods declared on cinfo and may also
  // be missing methods declared on cinfo. In practice this is the
  // set of methods we can depend on having accessible given any
  // object which is known to implement cinfo.
  auto it = cinfo->subclassList.begin();
  while (true) {
    if (it == cinfo->subclassList.end()) return;
    auto const sub = *it++;
    assertx(!(sub->cls->attrs & AttrInterface));
    if (sub == cinfo || (sub->cls->attrs & AttrAbstract)) continue;
    for (auto& par : sub->methods) {
      if (!par.second.hasPrivateAncestor &&
          (par.second.attrs & AttrPublic) &&
          !cinfo->methodFamilies.count(par.first) &&
          !cinfo->singleMethodFamilies.count(par.first) &&
          !cinfo->methods.count(par.first)) {
        extras.push_back(par.first);
      }
    }
    if (!extras.size()) return;
    break;
  }

  auto end = extras.end();
  while (it != cinfo->subclassList.end()) {
    auto const sub = *it++;
    assertx(!(sub->cls->attrs & AttrInterface));
    if (sub == cinfo || (sub->cls->attrs & AttrAbstract)) continue;
    for (auto nameIt = extras.begin(); nameIt != end;) {
      auto const meth = sub->methods.find(*nameIt);
      if (meth == sub->methods.end() ||
          !(meth->second.attrs & AttrPublic) ||
          meth->second.hasPrivateAncestor) {
        *nameIt = *--end;
        if (end == extras.begin()) return;
      } else {
        ++nameIt;
      }
    }
  }
  extras.erase(end, extras.end());

  // We can ignore abstract subclasses in most cases, except when it's
  // a leaf. This is needed to preserve monotonicity when looking up
  // FuncFamily results. Otherwise we could look up a type from an
  // interface's FuncFamily, then later narrow the interface to the
  // abstract class which implements it. If the abstract class doesn't
  // implement the method, they'll be no FuncFamily, and we'll return
  // InitCell (a potential monotonicity violation). This is an ugly
  // workaround, but a true fix requires more work.
  for (auto const sub : cinfo->abstractSubclassList) {
    assertx(!(sub->cls->attrs & AttrInterface));
    if (extras.empty()) break;
    if (sub == cinfo || !(sub->cls->attrs & AttrAbstract)) continue;
    if (sub->subclassList.size() > 1) continue;
    extras.erase(
      std::remove_if(
        extras.begin(), extras.end(),
        [&] (SString extra) {
          auto const meth = sub->methods.find(extra);
          if (meth == sub->methods.end()) return true;
          if (!(meth->second.attrs & AttrPublic)) return true;
          if (meth->second.hasPrivateAncestor) return true;
          return false;
        }
      ),
      extras.end()
    );
  }

  if (Trace::moduleEnabled(Trace::hhbbc_index, 5)) {
    FTRACE(5, "Adding extra methods to {}:\n", cinfo->cls->name);
    for (auto const DEBUG_ONLY extra : extras) {
      FTRACE(5, "  {}\n", extra);
    }
  }

  hphp_fast_set<SString> added;

  for (auto name : extras) {
    if (define_func_family(data, cinfo, name) &&
        (cinfo->cls->attrs & AttrInterface)) {
      added.emplace(name);
    }
  }

  if (cinfo->cls->attrs & AttrInterface) {
    for (auto& m : cinfo->cls->methods) {
      if (added.count(m->name)) {
        cinfo->methods.emplace(
          m->name,
          MethTabEntry { m.get(), m->attrs, false, true }
        );
      }
    }
  }
  return;
}

// Calculate the StaticInfo for the given FuncFamily, and assign it
// the pointer to the unique allocation corresponding to it.
void build_func_family_static_info(IndexData& index, FuncFamily* ff) {
  auto const& possible = ff->possibleFuncs();
  assertx(!possible.empty());

  // Calculate the StaticInfo from all possible functions:

  auto info =  [&] {
    FuncFamily::StaticInfo info;

    auto const func = possible.front()->second.func;
    info.m_numInOut = func_num_inout(func);
    info.m_isReadonlyReturn = yesOrNo(func->isReadonlyReturn);
    info.m_isReadonlyThis = yesOrNo(func->isReadonlyThis);
    info.m_maybeReified = func->isReified;
    info.m_maybeCaresAboutDynCalls = (dyn_call_error_level(func) > 0);
    info.m_maybeBuiltin = (func->attrs & AttrBuiltin);
    info.m_minNonVariadicParams =
      info.m_maxNonVariadicParams = numNVArgs(*func);
    info.m_requiredCoeffects = func->requiredCoeffects;
    info.m_coeffectRules = func->coeffectRules;

    for (size_t i = 0; i < func->params.size(); ++i) {
      info.m_paramPreps.emplace_back(func_param_prep(func, i));
    }
    for (auto const pf : possible) {
      if (pf->second.attrs & AttrAbstract) continue;
      info.m_supportsAER = func_supports_AER(pf->second.func);
      break;
    }
    return info;
  }();

  auto const addToParamPreps = [&] (const php::Func* f) {
    if (f->params.size() > info.m_paramPreps.size()) {
      info.m_paramPreps.resize(
        f->params.size(),
        PrepKind{TriBool::No, TriBool::No}
      );
    }
    for (size_t i = 0; i < info.m_paramPreps.size(); ++i) {
      auto const prep = func_param_prep(f, i);
      info.m_paramPreps[i].inOut |= prep.inOut;
      info.m_paramPreps[i].readonly |= prep.readonly;
    }
  };

  for (auto const pf : possible) {
    auto const func = pf->second.func;

    if (info.m_numInOut && *info.m_numInOut != func_num_inout(func)) {
      info.m_numInOut.reset();
    }
    info.m_isReadonlyReturn |= yesOrNo(func->isReadonlyReturn);
    info.m_isReadonlyThis |= yesOrNo(func->isReadonlyThis);
    info.m_maybeReified |= func->isReified;
    info.m_maybeCaresAboutDynCalls |= (dyn_call_error_level(func) > 0);
    info.m_maybeBuiltin |= (func->attrs & AttrBuiltin);
    addToParamPreps(func);

    if (info.m_supportsAER != TriBool::Maybe &&
        !(pf->second.attrs & AttrAbstract)) {
      info.m_supportsAER |= func_supports_AER(func);
    }

    auto const numNV = numNVArgs(*func);
    info.m_minNonVariadicParams =
      std::min(info.m_minNonVariadicParams, numNV);
    info.m_maxNonVariadicParams =
      std::max(info.m_maxNonVariadicParams, numNV);

    if (info.m_requiredCoeffects &&
        *info.m_requiredCoeffects != func->requiredCoeffects) {
      info.m_requiredCoeffects.reset();
    }

    if (info.m_coeffectRules) {
      if (!std::is_permutation(
            info.m_coeffectRules->begin(),
            info.m_coeffectRules->end(),
            func->coeffectRules.begin(),
            func->coeffectRules.end())) {
        info.m_coeffectRules.reset();
      }
    }
  }

  // Modify the info to make it more likely to match an existing one:

  // Any param beyond the size of m_paramPreps is implicitly
  // TriBool::No, so we can drop trailing entries which are
  // TriBool::No.
  while (!info.m_paramPreps.empty()) {
    auto& back = info.m_paramPreps.back();
    if (back.inOut != TriBool::No || back.readonly != TriBool::No) break;
    info.m_paramPreps.pop_back();
  }

  // Sort the coeffect rules to increase matching.
  if (info.m_coeffectRules) {
    std::sort(info.m_coeffectRules->begin(), info.m_coeffectRules->end());
  }

  // See if the info already exists in the set. If it doesn't exist,
  // add it. Otherwise use the already created one.
  ff->m_static = [&] {
    auto const it = index.funcFamilyStaticInfos.find(info);
    if (it != index.funcFamilyStaticInfos.end()) return it->first.get();
    return index.funcFamilyStaticInfos.insert(
      std::make_unique<FuncFamily::StaticInfo>(std::move(info)),
      false
    ).first->first.get();
  }();
}

void define_func_families(IndexData& index) {
  trace_time tracer("define_func_families");

  // Calculate func families for classes:
  parallel::for_each(
    index.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      if (cinfo->cls->attrs & AttrTrait) return;
      FTRACE(4, "Defining func families for {}\n", cinfo->cls->name);
      if (!(cinfo->cls->attrs & AttrInterface)) {
        for (auto& kv : cinfo->methods) {
          auto const mte = mteFromElm(kv);

          if (mte->second.attrs & AttrNoOverride) continue;
          if (is_special_method_name(mte->first)) continue;

          // We need function family for constructor even if it is private,
          // as `new static()` may still call a non-private constructor from
          // subclass.
          if (mte->first != s_construct.get() &&
              mte->second.attrs & AttrPrivate) {
            continue;
          }

          define_func_family(index, cinfo.get(), mte->first, mte->second.func);
        }
      }
      if (cinfo->cls->attrs & (AttrInterface | AttrAbstract)) {
        build_abstract_func_families(index, cinfo.get());
      }
    }
  );

  // Then calculate func families for methods with particular names:
  {
    // Build a list of all unique method names. Pre-allocate entries
    // in methodFamilies and singleMethodFamilies for each one (we
    // don't know which ones will be used). This lets us insert into
    // the maps from multiple threads safely (we don't have to mutate
    // the actual maps).
    std::vector<SString> allMethods;
    for (auto const& [name, _] : index.methods) {
      if (!allMethods.empty() && allMethods.back() == name) continue;
      allMethods.emplace_back(name);
      auto const DEBUG_ONLY inserted1 =
        index.methodFamilies.emplace(name, nullptr);
      assertx(inserted1.second);
      auto const DEBUG_ONLY inserted2 =
        index.singleMethodFamilies.emplace(name, nullptr);
      assertx(inserted2.second);
    }

    // Populate the maps
    parallel::for_each(
      allMethods,
      [&] (SString method) {
        define_func_family(index, nullptr, method, nullptr);
      }
    );

    // Now clean any remaining nullptr entries out of the maps. These
    // correspond to method names which got no func families.
    for (auto it = index.methodFamilies.begin();
         it != index.methodFamilies.end();) {
      if (!it->second) {
        it = index.methodFamilies.erase(it);
      } else {
        ++it;
      }
    }

    for (auto it = index.singleMethodFamilies.begin();
         it != index.singleMethodFamilies.end();) {
      if (!it->second) {
        it = index.singleMethodFamilies.erase(it);
      } else {
        ++it;
      }
    }
  }

  // Now that all of the FuncFamilies have been created, generate the
  // back links from FuncInfo to their FuncFamilies.
  std::vector<FuncFamily*> work;
  work.reserve(index.funcFamilies.size());
  for (auto const& kv : index.funcFamilies) work.emplace_back(kv.first.get());

  // Different threads can touch the same FuncInfo, so use sharded
  // locking scheme.
  std::array<std::mutex, 256> locks;

  parallel::for_each(
    work,
    [&] (FuncFamily* ff) {
      build_func_family_static_info(index, ff);

      for (auto const pf : ff->possibleFuncs()) {
        auto finfo = create_func_info(index, pf->second.func);
        auto& lock = locks[pointer_hash<FuncInfo>{}(finfo) % locks.size()];
        std::lock_guard<std::mutex> _{lock};
        finfo->families.emplace_back(ff);
      }
    }
  );

  parallel::for_each(
    index.funcInfo,
    [&] (FuncInfo& fi) { fi.families.shrink_to_fit(); }
  );
}

/*
 * ConflictGraph maintains lists of interfaces that conflict with each other
 * due to being implemented by the same class.
 */
struct ConflictGraph {
  void add(const php::Class* i, const php::Class* j) {
    if (i == j) return;
    map[i].insert(j);
  }

  hphp_fast_map<const php::Class*,
                hphp_fast_set<const php::Class*>> map;
};

/*
 * Trace information about interface conflict sets and the vtables computed
 * from them.
 */
void trace_interfaces(const IndexData& index, const ConflictGraph& cg) {
  // Compute what the vtable for each Class will look like, and build up a list
  // of all interfaces.
  struct Cls {
    const ClassInfo* cinfo;
    std::vector<const php::Class*> vtable;
  };
  std::vector<Cls> classes;
  std::vector<const php::Class*> ifaces;
  size_t total_slots = 0, empty_slots = 0;
  for (auto& cinfo : index.allClassInfos) {
    if (cinfo->cls->attrs & AttrInterface) {
      ifaces.emplace_back(cinfo->cls);
      continue;
    }
    if (cinfo->cls->attrs & (AttrTrait | AttrEnum | AttrEnumClass)) continue;

    classes.emplace_back(Cls{cinfo.get()});
    auto& vtable = classes.back().vtable;
    for (auto& pair : cinfo->implInterfaces) {
      auto it = index.ifaceSlotMap.find(pair.second->cls);
      assertx(it != end(index.ifaceSlotMap));
      auto const slot = it->second;
      if (slot >= vtable.size()) vtable.resize(slot + 1);
      vtable[slot] = pair.second->cls;
    }

    total_slots += vtable.size();
    for (auto iface : vtable) if (iface == nullptr) ++empty_slots;
  }

  Slot max_slot = 0;
  for (auto const& pair : index.ifaceSlotMap) {
    max_slot = std::max(max_slot, pair.second);
  }

  // Sort the list of class vtables so the largest ones come first.
  auto class_cmp = [&](const Cls& a, const Cls& b) {
    return a.vtable.size() > b.vtable.size();
  };
  std::sort(begin(classes), end(classes), class_cmp);

  // Sort the list of interfaces so the biggest conflict sets come first.
  auto iface_cmp = [&](const php::Class* a, const php::Class* b) {
    return cg.map.at(a).size() > cg.map.at(b).size();
  };
  std::sort(begin(ifaces), end(ifaces), iface_cmp);

  std::string out;
  folly::format(&out, "{} interfaces, {} classes\n",
                ifaces.size(), classes.size());
  folly::format(&out,
                "{} vtable slots, {} empty vtable slots, max slot {}\n",
                total_slots, empty_slots, max_slot);
  folly::format(&out, "\n{:-^80}\n", " interface slots & conflict sets");
  for (auto iface : ifaces) {
    auto cgIt = cg.map.find(iface);
    if (cgIt == end(cg.map)) break;
    auto& conflicts = cgIt->second;

    folly::format(&out, "{:>40} {:3} {:2} [", iface->name,
                  conflicts.size(),
                  folly::get_default(index.ifaceSlotMap, iface));
    auto sep = "";
    for (auto conflict : conflicts) {
      folly::format(&out, "{}{}", sep, conflict->name);
      sep = ", ";
    }
    folly::format(&out, "]\n");
  }

  folly::format(&out, "\n{:-^80}\n", " class vtables ");
  for (auto& item : classes) {
    if (item.vtable.empty()) break;

    folly::format(&out, "{:>30}: [", item.cinfo->cls->name);
    auto sep = "";
    for (auto iface : item.vtable) {
      folly::format(&out, "{}{}", sep, iface ? iface->name->data() : "null");
      sep = ", ";
    }
    folly::format(&out, "]\n");
  }

  Trace::traceRelease("%s", out.c_str());
}

/*
 * Find the lowest Slot that doesn't conflict with anything in the conflict set
 * for iface.
 */
Slot find_min_slot(const php::Class* iface,
                   const IfaceSlotMap& slots,
                   const ConflictGraph& cg) {
  auto const& cit = cg.map.find(iface);
  if (cit == cg.map.end() || cit->second.empty()) {
    // No conflicts. This is the only interface implemented by the classes that
    // implement it.
    return 0;
  }

  boost::dynamic_bitset<> used;

  for (auto const& c : cit->second) {
    auto const it = slots.find(c);
    if (it == slots.end()) continue;
    auto const slot = it->second;

    if (used.size() <= slot) used.resize(slot + 1);
    used.set(slot);
  }
  used.flip();
  return used.any() ? used.find_first() : used.size();
}

/*
 * Compute vtable slots for all interfaces. No two interfaces implemented by
 * the same class will share the same vtable slot.
 */
void compute_iface_vtables(IndexData& index) {
  trace_time tracer("compute interface vtables");

  ConflictGraph cg;
  std::vector<const php::Class*>             ifaces;
  hphp_fast_map<const php::Class*, int> iface_uses;

  // Build up the conflict sets.
  for (auto& cinfo : index.allClassInfos) {
    // Gather interfaces.
    if (cinfo->cls->attrs & AttrInterface) {
      ifaces.emplace_back(cinfo->cls);
      // Make sure cg.map has an entry for every interface - this simplifies
      // some code later on.
      cg.map[cinfo->cls];
      continue;
    }

    // Only worry about classes with methods that can be called.
    if (cinfo->cls->attrs & (AttrTrait | AttrEnum | AttrEnumClass)) continue;

    for (auto& ipair : cinfo->implInterfaces) {
      ++iface_uses[ipair.second->cls];
      for (auto& jpair : cinfo->implInterfaces) {
        cg.add(ipair.second->cls, jpair.second->cls);
      }
    }
  }

  if (ifaces.size() == 0) return;

  // Sort interfaces by usage frequencies.
  // We assign slots greedily, so sort the interface list so the most
  // frequently implemented ones come first.
  auto iface_cmp = [&](const php::Class* a, const php::Class* b) {
    return iface_uses[a] > iface_uses[b];
  };
  std::sort(begin(ifaces), end(ifaces), iface_cmp);

  // Assign slots, keeping track of the largest assigned slot and the total
  // number of uses for each slot.
  Slot max_slot = 0;
  hphp_fast_map<Slot, int> slot_uses;
  for (auto* iface : ifaces) {
    auto const slot = find_min_slot(iface, index.ifaceSlotMap, cg);
    index.ifaceSlotMap[iface] = slot;
    max_slot = std::max(max_slot, slot);

    // Interfaces implemented by the same class never share a slot, so normal
    // addition is fine here.
    slot_uses[slot] += iface_uses[iface];
  }

  // Make sure we have an initialized entry for each slot for the sort below.
  for (Slot slot = 0; slot < max_slot; ++slot) {
    assertx(slot_uses.count(slot));
  }

  // Finally, sort and reassign slots so the most frequently used slots come
  // first. This slightly reduces the number of wasted vtable vector entries at
  // runtime.
  auto const slots = sort_keys_by_value(
    slot_uses,
    [&] (int a, int b) { return a > b; }
  );

  std::vector<Slot> slots_permute(max_slot + 1, 0);
  for (size_t i = 0; i <= max_slot; ++i) slots_permute[slots[i]] = i;

  // re-map interfaces to permuted slots
  for (auto& pair : index.ifaceSlotMap) {
    pair.second = slots_permute[pair.second];
  }

  if (Trace::moduleEnabledRelease(Trace::hhbbc_iface)) {
    trace_interfaces(index, cg);
  }
}

void mark_magic_on_parents(ClassInfo& cinfo, ClassInfo& derived) {
  auto any = false;
  for (const auto& mm : magicMethods) {
    if ((derived.*mm.pmem).thisHas) {
      auto& derivedHas = (cinfo.*mm.pmem).derivedHas;
      if (!derivedHas) {
        derivedHas = any = true;
      }
    }
  }
  if (!any) return;
  if (cinfo.parent) mark_magic_on_parents(*cinfo.parent, derived);
  for (auto iface : cinfo.declInterfaces) {
    mark_magic_on_parents(*const_cast<ClassInfo*>(iface), derived);
  }
}

bool has_magic_method(const ClassInfo* cinfo, SString name) {
  if (name == s_toBoolean.get()) {
    // note that "having" a magic method includes the possibility that
    // a parent class has it. This can't happen for the collection
    // classes, because they're all final; but for SimpleXMLElement,
    // we need to search.
    while (cinfo->parent) cinfo = cinfo->parent;
    return has_magic_bool_conversion(cinfo->cls->name);
  }
  return cinfo->methods.find(name) != end(cinfo->methods);
}

void find_magic_methods(IndexData& index) {
  trace_time tracer("find magic methods");

  for (auto& cinfo : index.allClassInfos) {
    bool any = false;
    for (const auto& mm : magicMethods) {
      bool const found = has_magic_method(cinfo.get(), mm.name.get());
      any = any || found;
      (cinfo.get()->*mm.pmem).thisHas = found;
    }
    if (any) mark_magic_on_parents(*cinfo, *cinfo);
  }
}

void find_mocked_classes(IndexData& index) {
  trace_time tracer("find mocked classes");

  for (auto& cinfo : index.allClassInfos) {
    if (is_mock_class(cinfo->cls) && cinfo->parent) {
      cinfo->parent->isMocked = true;
    }
  }

  parallel::for_each(
    index.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      for (auto const sub : cinfo->subclassList) {
        if (sub->isMocked) {
          cinfo->isSubMocked = true;
          break;
        }
      }
    }
  );
}

void mark_const_props(IndexData& index) {
  trace_time tracer("mark const props");

  for (auto& cinfo : index.allClassInfos) {
    assertx(!cinfo->hasConstProp);
    cinfo->hasConstProp = [&]{
      if (cinfo->cls->hasConstProp) return true;
      if (cinfo->parent && cinfo->parent->hasConstProp) return true;
      if (!(cinfo->cls->attrs & AttrNoExpandTrait)) {
        for (auto const t : cinfo->usedTraits) {
          if (t->cls->hasConstProp) return true;
        }
      }
      return false;
    }();
  }

  parallel::for_each(
    index.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      assertx(!cinfo->subHasConstProp);
      for (auto const sub : cinfo->subclassList) {
        if (sub->hasConstProp) {
          cinfo->subHasConstProp = true;
          break;
        }
      }
    }
  );
}

void mark_has_reified_parent(IndexData& index) {
  trace_time tracer("mark has reified parent");

  for (auto& cinfo : index.allClassInfos) {
    if (cinfo->cls->hasReifiedGenerics) {
      for (auto& c : cinfo->subclassList) {
        c->hasReifiedParent = true;
      }
    }
  }
}

void mark_no_override_classes(IndexData& index) {
  trace_time tracer("mark no override classes");

  for (auto& cinfo : index.allClassInfos) {
    // We cleared all the NoOverride flags while building the
    // index. Set them as necessary.
    if (!(cinfo->cls->attrs & AttrInterface) &&
        cinfo->subclassList.size() == 1) {
      attribute_setter(cinfo->cls->attrs, true, AttrNoOverride);
    }
  }
}

void mark_no_override_methods(IndexData& index) {
  trace_time tracer("mark no override methods");

  // We removed any AttrNoOverride flags from all methods while adding
  // the units to the index.  Now start by marking every
  // (non-interface, non-special) method as AttrNoOverride.
  parallel::for_each(
    index.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      if (cinfo->cls->attrs & AttrInterface) return;

      for (auto& m : cinfo->methods) {
        if (!(is_special_method_name(m.first))) {
          FTRACE(9, "Pre-setting AttrNoOverride on {}::{}\n",
                 m.second.func->cls->name, m.first);
          attribute_setter(m.second.attrs, true, AttrNoOverride);
          attribute_setter(m.second.func->attrs, true, AttrNoOverride);
        }
      }
    }
  );

  // Then run through every ClassInfo, and for each of its parent
  // classes clear the AttrNoOverride flag if it has a different Func
  // with the same name.
  auto const updates = parallel::map(
    index.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      hphp_fast_set<MethTabEntry*> changes;

      for (auto const& ancestor : cinfo->baseList) {
        if (ancestor == cinfo.get()) continue;

        for (auto const& derivedMethod : cinfo->methods) {
          auto const it = ancestor->methods.find(derivedMethod.first);
          if (it == end(ancestor->methods)) continue;
          if (it->second.func != derivedMethod.second.func) {
            FTRACE(2, "Removing AttrNoOverride on {}::{}\n",
                   it->second.func->cls->name, it->first);
            changes.emplace(&it->second);
          }
        }
      }

      return changes;
    }
  );

  for (auto const& u : updates) {
    for (auto& mte : u) {
      assertx(mte->attrs & AttrNoOverride ||
              !(mte->func->attrs & AttrNoOverride));
      if (mte->attrs & AttrNoOverride) {
        attribute_setter(mte->attrs, false, AttrNoOverride);
        attribute_setter(mte->func->attrs, false, AttrNoOverride);
      }
    }
  }
}

const StaticString s__Reified("__Reified");

/*
 * Emitter adds a 86reifiedinit method to all classes that have reified
 * generics. All base classes also need to have this method so that when we
 * call parent::86reifeidinit(...), there is a stopping point.
 * Since while emitting we do not know whether a base class will have
 * reified parents, during JIT time we need to add 86reifiedinit
 * unless AttrNoReifiedInit attribute is set. At this phase,
 * we set AttrNoReifiedInit attribute on classes do not have any
 * reified classes that extend it.
 */
void clean_86reifiedinit_methods(IndexData& index) {
  trace_time tracer("clean 86reifiedinit methods");
  hphp_fast_set<const php::Class*> needsinit;

  // Find all classes that still need their 86reifiedinit methods
  for (auto const& cinfo : index.allClassInfos) {
    auto const& ual = cinfo->cls->userAttributes;
    // Each class that has at least one reified generic has an attribute
    // __Reified added by the emitter
    auto has_reification = ual.find(s__Reified.get()) != ual.end();
    if (!has_reification) continue;
    // Add the base class for this reified class
    needsinit.emplace(cinfo->baseList[0]->cls);
  }

  // Add AttrNoReifiedInit to the base classes that do not need this method
  for (auto& cinfo : index.allClassInfos) {
    if (cinfo->parent == nullptr && needsinit.count(cinfo->cls) == 0) {
      FTRACE(2, "Adding AttrNoReifiedInit on class {}\n", cinfo->cls->name);
      attribute_setter(cinfo->cls->attrs, true, AttrNoReifiedInit);
    }
  }
}

//////////////////////////////////////////////////////////////////////

void check_invariants(const ClassInfo* cinfo) {
  // All the following invariants only apply to classes
  if (cinfo->cls->attrs & AttrInterface) return;

  if (!(cinfo->cls->attrs & AttrTrait)) {
    // For non-interface classes, each method in a php class has an
    // entry in its ClassInfo method table, and if it's not special,
    // AttrNoOverride, or private, an entry in the family table.
    for (auto& m : cinfo->cls->methods) {
      auto const it = cinfo->methods.find(m->name);
      always_assert(it != cinfo->methods.end());
      if (it->second.attrs & (AttrNoOverride|AttrPrivate)) continue;
      if (is_special_method_name(m->name)) continue;
      always_assert(
        cinfo->methodFamilies.count(m->name) ||
        cinfo->singleMethodFamilies.count(m->name)
      );
    }
  }

  // The subclassList is non-empty, contains this ClassInfo, and
  // contains only unique elements.
  always_assert(!cinfo->subclassList.empty());
  always_assert(std::find(begin(cinfo->subclassList),
                          end(cinfo->subclassList),
                          cinfo) != end(cinfo->subclassList));
  auto cpy = cinfo->subclassList;
  std::sort(begin(cpy), end(cpy));
  cpy.erase(
    std::unique(begin(cpy), end(cpy)),
    end(cpy)
  );
  always_assert(cpy.size() == cinfo->subclassList.size());

  // The baseList is non-empty, and the last element is this class.
  always_assert(!cinfo->baseList.empty());
  always_assert(cinfo->baseList.back() == cinfo);

  for (const auto& mm : magicMethods) {
    const auto& info = cinfo->*mm.pmem;

    // Magic method flags should be consistent with the method table.
    always_assert(info.thisHas == has_magic_method(cinfo, mm.name.get()));

    // Non-'derived' flags (thisHas) about magic methods imply the derived
    // ones.
    always_assert(!info.thisHas || info.derivedHas);
  }

  // Every FuncFamily has more than function and contain functions
  // with the same name (unless its a family of ctors). methodFamilies
  // and singleMethodFamilies should have disjoint keys.
  for (auto const& mfam: cinfo->methodFamilies) {
    always_assert(mfam.second->possibleFuncs().size() > 1);
    auto const name = mfam.second->possibleFuncs().front()->first;
    for (auto const pf : mfam.second->possibleFuncs()) {
      always_assert(pf->first->same(name));
    }
    always_assert(!cinfo->singleMethodFamilies.count(mfam.first));
  }
  for (auto const& mfam : cinfo->singleMethodFamilies) {
    always_assert(!cinfo->methodFamilies.count(mfam.first));
  }
}

void check_invariants(IndexData& data) {
  if (!debug) return;

  for (auto& cinfo : data.allClassInfos) {
    check_invariants(cinfo.get());
  }
}

//////////////////////////////////////////////////////////////////////

Type adjust_closure_context(const Index& index, const CallContext& ctx) {
  if (ctx.callee->cls && ctx.callee->cls->closureContextCls) {
    auto withClosureContext = Context {
      index.lookup_func_unit(*ctx.callee),
      ctx.callee,
      index.lookup_closure_context(*ctx.callee->cls)
    };
    if (auto const rcls = index.selfCls(withClosureContext)) {
      return setctx(subObj(*rcls));
    }
    return TObj;
  }
  return ctx.context;
}

Type context_sensitive_return_type(IndexData& data,
                                   CallContext callCtx,
                                   Type returnType) {
  constexpr auto max_interp_nexting_level = 2;
  static __thread uint32_t interp_nesting_level;
  auto const finfo = func_info(data, callCtx.callee);

  auto const adjustedCtx = adjust_closure_context(*data.m_index, callCtx);
  returnType = return_with_context(std::move(returnType), adjustedCtx);

  auto const checkParam = [&] (int i) {
    auto const constraint = finfo->func->params[i].typeConstraint;
    if (constraint.hasConstraint() &&
        !constraint.isTypeVar() &&
        !constraint.isTypeConstant()) {
      auto ctx = Context {
        data.units.at(finfo->func->unit),
        finfo->func,
        finfo->func->cls
      };
      auto t = data.m_index->lookup_constraint(ctx, constraint);
      return callCtx.args[i].strictlyMoreRefined(t);
    }
    return callCtx.args[i].strictSubtypeOf(TInitCell);
  };

  // TODO(#3788877): more heuristics here would be useful.
  auto const tryContextSensitive = [&] {
    if (finfo->func->noContextSensitiveAnalysis ||
        finfo->func->params.empty() ||
        interp_nesting_level + 1 >= max_interp_nexting_level ||
        returnType == TBottom) {
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
      if (data.frozen || acc->second == TBottom || is_scalar(acc->second)) {
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
      data.units.at(func->unit),
      wf,
      func->cls
    };
    auto const ty = analyze_func_inline(
      *data.m_index,
      calleeCtx,
      adjustedCtx,
      callCtx.args
    ).inferredReturn;
    return return_with_context(ty, adjustedCtx);
  }();

  if (!interp_nesting_level) {
    FTRACE(3,
           "Context sensitive type: {}\n"
           "Context insensitive type: {}\n",
           show(contextType), show(returnType));
  }

  if (!returnType.subtypeOf(BUnc)) {
    // If the context insensitive return type could be non-static, staticness
    // could be a result of temporary context sensitive bytecode optimizations.
    contextType = loosen_staticness(std::move(contextType));
  }

  auto ret = intersection_of(std::move(returnType), std::move(contextType));

  if (!interp_nesting_level) {
    FTRACE(3, "Context sensitive result: {}\n", show(ret));
  }

  ContextRetTyMap::accessor acc;
  if (data.contextualReturnTypes.insert(acc, callCtx) ||
      ret.strictSubtypeOf(acc->second)) {
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
PropMergeResult<> prop_tc_effects(const Index& index,
                                  const ClassInfo* ci,
                                  const php::Prop& prop,
                                  const Type& val,
                                  bool checkUB) {
  assertx(prop.typeConstraint.validForProp());

  using R = PropMergeResult<>;

  // If we're not actually checking property type-hints, everything
  // goes
  if (RuntimeOption::EvalCheckPropTypeHints <= 0) return R{ val, TriBool::No };

  auto const ctx = Context { nullptr, nullptr, ci->cls };

  auto const check = [&] (const TypeConstraint& tc, const Type& t) {
    // If the type as is satisfies the constraint, we won't throw and
    // the type is unchanged.
    if (index.satisfies_constraint(ctx, t, tc)) return R{ t, TriBool::No };
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
  for (auto ub : prop.ubs) {
    applyFlagsToUB(ub, prop.typeConstraint);
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
PropLookupResult<> lookup_static_impl(IndexData& data,
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
        auto const it = ci->publicStaticProps.find(propName);
        assertx(it != end(ci->publicStaticProps));
        return remove_uninit(it->second.inferredType);
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
    return PropLookupResult<>{
      type(prop, ci),
      propName,
      TriBool::Yes,
      yesOrNo(prop.attrs & AttrIsConst),
      yesOrNo(prop.attrs & AttrIsReadonly),
      yesOrNo(prop.attrs & AttrLateInit),
      initMightRaise
    };
  };

  auto const notFound = [&] {
    // The property definitely wasn't found.
    return PropLookupResult<>{
      TBottom,
      propName,
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
    [&] (const ClassInfo* ci) -> Optional<PropLookupResult<>> {
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
PropMergeResult<> merge_static_type_impl(IndexData& data,
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
      show(effects.adjusted), ci->cls->name, prop.name
    );

    switch (prop.attrs & (AttrPublic|AttrProtected|AttrPrivate)) {
      case AttrPublic:
      case AttrProtected:
        mergePublic(ci, prop, unctx(effects.adjusted));
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
    return PropMergeResult<>{
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
    [&] (const ClassInfo* ci) -> Optional<PropMergeResult<>> {
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
    return PropMergeResult<>{
      std::move(result->adjusted),
      maybeOrNo(class_init_might_raise(data, ctx, start))
    };
  }
  return *result;
}

//////////////////////////////////////////////////////////////////////

void buildTypeInfoData(IndexData& data, ClassInfoData& cid) {
  for (auto const& elm : data.classes) {
    auto const cls = elm.second;
    auto const addUser = [&] (SString rName) {
      cid.users[rName].push_back(cls);
      ++cid.depCounts[cls];
    };

    if (cls->parentName) addUser(cls->parentName);
    for (auto& i : cls->interfaceNames) addUser(i);
    for (auto& t : cls->usedTraitNames) addUser(t);
    for (auto& t : cls->includedEnumNames) addUser(t);

    if (!cid.depCounts.count(cls)) {
      FTRACE(5, "Adding no-dep class {}:{} to queue\n",
             cls->name, (void*)cls);
      // make sure that closure is first, because we end up calling
      // preresolve directly on closures created by trait
      // flattening, which assumes all dependencies are satisfied.
      if (cid.queue.size() && cls->name == s_Closure.get()) {
        cid.queue.push_back(cid.queue[0]);
        cid.queue[0] = cls;
      } else {
        cid.queue.push_back(cls);
      }
    } else {
      FTRACE(6, "class {}:{} has {} deps\n",
             cls->name, (void*)cls, cid.depCounts[cls]);
    }
  }
  cid.cqBack = cid.queue.size();
  cid.queue.resize(data.classes.size());
}

void updatePreResolveDeps(ClassInfoData& cid,
                          const ClsPreResolveUpdates& updates) {
  for (auto const info : updates.updateDeps) {
    auto const& users = cid.users[info->cls->name];
    for (auto const tu : users) {
      auto const it = cid.depCounts.find(tu);
      if (it == cid.depCounts.end()) {
        assertx(cid.hasPseudoCycles);
        continue;
      }
      auto& depCount = it->second;
      assertx(depCount);
      if (!--depCount) {
        cid.depCounts.erase(it);
        ITRACE(5, "  enqueue: {}:{}\n", tu->name, (void*)tu);
        cid.queue[cid.cqBack++] = tu;
      } else {
        ITRACE(6, "  depcount: {}:{} = {}\n", tu->name, (void*)tu, depCount);
      }
    }
  }
}

void commitPreResolveUpdates(IndexData& index,
                             ClassInfoData& tid,
                             std::vector<ClsPreResolveUpdates>& updates) {
  parallel::parallel(
    [&] {
      for (auto const& u : updates) updatePreResolveDeps(tid, u);
    },
    [&] {
      for (auto& u : updates) {
        for (size_t i = 0; i < u.newInfos.size(); ++i) {
          auto& cinfo = u.newInfos[i];
          auto const UNUSED it =
            index.classInfo.emplace(cinfo->cls->name, cinfo.get());
          assertx(it.second);
          index.allClassInfos.emplace_back(std::move(cinfo));
        }
      }
    },
    [&] {
      for (auto& u : updates) {
        for (auto const& p : u.extraMethods) {
          index.classExtraMethodMap[p.first].insert(
            p.second.begin(),
            p.second.end()
          );
        }
      }
    },
    [&] {
      for (auto& u : updates) {
        for (auto const& p : u.closures) {
          auto& map = index.classClosureMap[p.first];
          map.insert(map.end(), p.second.begin(), p.second.end());
        }
      }
    },
    [&] {
      for (auto& u : updates) {
        for (auto c : u.newClosures) index.classes.emplace(c->name, c);
      }
    },
    [&] {
      for (auto& u : updates) {
        for (auto& p : u.newClasses) {
          auto& cls = std::get<0>(p);
          auto unit = std::get<1>(p);
          auto const idx = std::get<2>(p);
          if (unit->classes.size() <= idx) unit->classes.resize(idx+1);
          unit->classes[idx] = cls->name;
          index.program->classes.emplace_back(std::move(cls));
        }
      }
    },
    [&] {
      for (auto const& u : updates) {
        for (auto f : u.newMethods) {
          index.methods.emplace(f->name, f);
        }
      }
    }
  );
}

void preresolveTypes(IndexData& index, ClassInfoData& cid) {
  auto round = uint32_t{0};
  while (true) {
    if (cid.cqFront == cid.cqBack) {
      // we've consumed everything where all dependencies are
      // satisfied. There may still be some pseudo-cycles that can
      // be broken though.
      //
      // eg if A extends B and B' extends A', we'll resolve B and
      // A', and then end up here, since both A and B' still have
      // one dependency. But both A and B' can be resolved at this
      // point
      for (auto it = cid.depCounts.begin();
           it != cid.depCounts.end();) {
        auto canResolve = true;
        auto const checkCanResolve = [&] (SString name) {
          if (canResolve) canResolve = index.classInfo.count(name);
        };

        auto const cls = it->first;
        if (cls->parentName) checkCanResolve(cls->parentName);
        for (auto& i : cls->interfaceNames) checkCanResolve(i);
        for (auto& t : cls->usedTraitNames) checkCanResolve(t);
        for (auto& t : cls->includedEnumNames) checkCanResolve(t);

        if (canResolve) {
          FTRACE(2, "Breaking pseudo-cycle for class {}:{}\n",
                 it->first->name, (void*)it->first);
          cid.queue[cid.cqBack++] = it->first;
          it = cid.depCounts.erase(it);
          cid.hasPseudoCycles = true;
        } else {
          ++it;
        }
      }
      if (cid.cqFront == cid.cqBack) break;
    }

    auto const workitems = cid.cqBack - cid.cqFront;
    auto updates = [&] {
      trace_time trace(
        "preresolve",
        folly::sformat("round {} -- {} work items", round, workitems)
      );

      // Aggregate the types together by their Unit. This means only
      // one thread will be processing a particular Unit at a
      // time. This lets us avoid locking access to the Unit, and also
      // keeps the flattening logic deterministic.
      using UnitGroup =
        std::pair<const php::Unit*, CompactVector<const php::Class*>>;

      hphp_fast_map<const php::Unit*, CompactVector<const php::Class*>> group;
      for (auto idx = cid.cqFront; idx < cid.cqBack; ++idx) {
        auto const t = cid.queue[idx];
        group[index.units.at(t->unit)].emplace_back(t);
      }
      std::vector<UnitGroup> worklist{group.begin(), group.end()};

      return parallel::map(
        worklist,
        [&] (UnitGroup& group) {
          Trace::Bump bumper{
            Trace::hhbbc_index, kSystemLibBump, is_systemlib_part(*group.first)
          };
          (void)bumper;

          std::sort(
            group.second.begin(), group.second.end(),
            [&] (const php::Class* a, const php::Class* b) {
              return strcmp(a->name->data(), b->name->data()) < 0;
            }
          );

          // NB: Even though we can freely access the Unit, we cannot
          // modify it in preresolve because other threads might also
          // be accessing it.
          ClsPreResolveUpdates updates;
          updates.nextClassId = group.first->classes.size();
          for (auto const t : group.second) {
            preresolve(index, t, updates);
          }
          return updates;
        }
      );
    }();

    ++round;
    cid.cqFront += workitems;

    trace_time trace("update");
    commitPreResolveUpdates(index, cid, updates);
  }

  trace_time trace("preresolve clear state");
  parallel::for_each(
    index.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      cinfo->preResolveState.reset();
    }
  );
}

// Construct a php::Program from an Index:::Input. Right now this is
// just loading the data and adding them to a php::Program. The units
// have already been constructed in extern-worker jobs.
std::unique_ptr<php::Program>
materialize_inputs(Index::Input input,
                   std::unique_ptr<coro::TicketExecutor> executor,
                   std::unique_ptr<Client> client,
                   const DisposeCallback& dispose) {
  using namespace folly::gen;

  trace_time tracer("materialize inputs");

  // For speed, split up the unit loading into chunks.
  constexpr size_t kLoadChunkSize = 500;

  struct Chunk {
    std::vector<Ref<std::unique_ptr<php::Class>>> classes;
    std::vector<Ref<std::unique_ptr<php::Func>>> funcs;
    std::vector<Ref<std::unique_ptr<php::Unit>>> units;

    size_t size() const {
      return classes.size() + funcs.size() + units.size();
    }
    bool empty() const {
      return classes.empty() && funcs.empty() && units.empty();
    }
  };
  std::vector<Chunk> chunks;
  Chunk current;

  auto const added = [&] {
    if (current.size() >= kLoadChunkSize) {
      chunks.emplace_back(std::move(current));
    }
  };
  for (auto& unit : input.units) {
    current.units.emplace_back(std::move(unit.second));
    added();
  }
  for (auto& cls : input.classes) {
    current.classes.emplace_back(std::move(cls.cls));
    added();
  }
  for (auto& func : input.funcs) {
    current.funcs.emplace_back(std::move(func.second));
    added();
  }
  if (!current.empty()) chunks.emplace_back(std::move(current));

  std::mutex lock;
  auto program = std::make_unique<php::Program>();

  auto const loadAndParse = [&] (Chunk chunk) -> coro::Task<void> {
    auto [classes, funcs, units] = HPHP_CORO_AWAIT(coro::collect(
      client->load(std::move(chunk.classes)),
      client->load(std::move(chunk.funcs)),
      client->load(std::move(chunk.units))
    ));

    {
      std::scoped_lock<std::mutex> _{lock};
      for (auto& unit : units) {
        program->units.emplace_back(std::move(unit));
      }
      for (auto& cls : classes) {
        program->classes.emplace_back(std::move(cls));
      }
      for (auto& func : funcs) {
        program->funcs.emplace_back(std::move(func));
      }
    }
    HPHP_CORO_RETURN_VOID;
  };

  std::vector<coro::TaskWithExecutor<void>> tasks;
  tasks.reserve(chunks.size());
  for (auto& c : chunks) {
    tasks.emplace_back(
      loadAndParse(std::move(c)).scheduleOn(executor->sticky())
    );
  }
  coro::wait(coro::collectRange(std::move(tasks)));

  auto const& stats = client->getStats();
  Logger::FInfo(
    "HHBBC:\n"
    "  Execs: {:,} total, {:,} cache-hits, {:,} optimistically, {:,} fallback\n"
    "  Files: {:,} total, {:,} read, {:,} queried, {:,} uploaded ({:,} bytes), {:,} fallback\n"
    "  Blobs: {:,} total, {:,} queried, {:,} uploaded ({:,} bytes), {:,} fallback\n"
    "  Cpu: {:,} usec usage, {:,} allocated cores\n"
    "  Mem: {:,} max used, {:,} reserved\n"
    "  {:,} downloads ({:,} bytes), {:,} throttles",
    stats.execs.load(),
    stats.execCacheHits.load(),
    stats.optimisticExecs.load(),
    stats.execFallbacks.load(),
    stats.files.load(),
    stats.filesRead.load(),
    stats.filesQueried.load(),
    stats.filesUploaded.load(),
    stats.fileBytesUploaded.load(),
    stats.fileFallbacks.load(),
    stats.blobs.load(),
    stats.blobsQueried.load(),
    stats.blobsUploaded.load(),
    stats.blobBytesUploaded.load(),
    stats.blobFallbacks.load(),
    stats.execCpuUsec.load(),
    stats.execAllocatedCores.load(),
    stats.execMaxUsedMem.load(),
    stats.execReservedMem.load(),
    stats.downloads.load(),
    stats.bytesDownloaded.load(),
    stats.throttles.load()
  );

  // Done with any extern-worker stuff at this point (for now).
  dispose(std::move(executor), std::move(client));
  return program;
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
             std::unique_ptr<coro::TicketExecutor> executor,
             std::unique_ptr<Client> client,
             DisposeCallback dispose)
  : m_data{std::make_unique<IndexData>(this)}
{
  trace_time tracer("create index");

  m_data->program = materialize_inputs(
    std::move(input),
    std::move(executor),
    std::move(client),
    dispose
  );

  add_system_constants_to_index(*m_data);
  add_program_to_index(*m_data);

  ClassInfoData cid;
  {
    trace_time build_class_info_data("build classinfo data");
    buildTypeInfoData(*m_data, cid);
  }

  {
    trace_time preresolve_classes("preresolve classes");
    preresolveTypes(*m_data, cid);
  }

  m_data->funcInfo.resize(m_data->nextFuncId);

  // Part of the index building routines happens before the various asserted
  // index invariants hold.  These each may depend on computations from
  // previous functions, so be careful changing the order here.
  compute_subclass_list(*m_data);
  clean_86reifiedinit_methods(*m_data); // uses the base class lists
  mark_no_override_methods(*m_data);
  find_magic_methods(*m_data);          // uses the subclass lists
  find_mocked_classes(*m_data);
  mark_const_props(*m_data);
  mark_has_reified_parent(*m_data);
  auto const logging = Trace::moduleEnabledRelease(Trace::hhbbc_time, 1);
  m_data->compute_iface_vtables = std::thread([&] {
      HphpSessionAndThread _{Treadmill::SessionKind::HHBBC};
      auto const enable =
        logging && !Trace::moduleEnabledRelease(Trace::hhbbc_time, 1);
      Trace::BumpRelease bumper(Trace::hhbbc_time, -1, enable);
      compute_iface_vtables(*m_data);
    }
  );
  define_func_families(*m_data);        // AttrNoOverride, iface_vtables,
                                        // subclass_list

  check_invariants(*m_data);

  mark_no_override_classes(*m_data);

  trace_time tracer_2("initialize return types");
  std::vector<const php::Func*> all_funcs;
  all_funcs.reserve(m_data->funcs.size() + m_data->methods.size());
  for (auto const fn : m_data->funcs) {
    all_funcs.push_back(fn.second);
  }
  for (auto const fn : m_data->methods) {
    all_funcs.push_back(fn.second);
  }
  parallel::for_each(
    all_funcs,
    [&] (const php::Func* f) { init_return_type(f); }
  );
}

// Defined here so IndexData is a complete type for the unique_ptr
// destructor.
Index::~Index() {}

//////////////////////////////////////////////////////////////////////

const php::Program& Index::program() const {
  return *m_data->program;
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

const php::Class* Index::lookup_unit_class(const php::Unit& unit, Id id) const {
  assertx(id < unit.classes.size());
  return m_data->classes.at(unit.classes[id]);
}

php::Class* Index::lookup_unit_class_mutable(php::Unit& unit, Id id) {
  assertx(id < unit.classes.size());
  return m_data->classes.at(unit.classes[id]);
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

//////////////////////////////////////////////////////////////////////

void Index::mark_no_bad_redeclare_props(php::Class& cls) const {
  /*
   * Keep a list of properties which have not yet been found to redeclare
   * anything inequivalently. Start out by putting everything on the list. Then
   * walk up the inheritance chain, removing collisions as we find them.
   */
  std::vector<php::Prop*> props;
  for (auto& prop : cls.properties) {
    if (prop.attrs & (AttrStatic | AttrPrivate)) {
      // Static and private properties never redeclare anything so need not be
      // considered.
      attribute_setter(prop.attrs, true, AttrNoBadRedeclare);
      continue;
    }
    attribute_setter(prop.attrs, false, AttrNoBadRedeclare);
    props.emplace_back(&prop);
  }

  auto currentCls = [&]() -> const ClassInfo* {
    auto const rcls = resolve_class(&cls);
    if (rcls.val.left()) return nullptr;
    return rcls.val.right();
  }();
  // If there's one more than one resolution for the class, be conservative and
  // we'll treat everything as possibly redeclaring.
  if (!currentCls) props.clear();

  while (!props.empty()) {
    auto const parent = currentCls->parent;
    if (!parent) {
      // No parent. We're done, so anything left on the prop list is
      // AttrNoBadRedeclare.
      for (auto& prop : props) {
        attribute_setter(prop->attrs, true, AttrNoBadRedeclare);
      }
      break;
    }

    auto const findParentProp = [&] (SString name) -> const php::Prop* {
      for (auto& prop : parent->cls->properties) {
        if (prop.name == name) return &prop;
      }
      for (auto& prop : parent->traitProps) {
        if (prop.name == name) return &prop;
      }
      return nullptr;
    };

    // Remove any properties which collide with the current class.

    auto const propRedeclares = [&] (php::Prop* prop) {
      auto const pprop = findParentProp(prop->name);
      if (!pprop) return false;

      // We found a property being redeclared. Check if the type-hints on
      // the two are equivalent.
      auto const equivOneTCPair =
      [&](const TypeConstraint& tc1, const TypeConstraint& tc2) {
        // Try the cheap check first, use the index otherwise. Two
        // type-constraints are equivalent if all the possible values of one
        // satisfies the other, and vice-versa.
        if (!tc1.maybeInequivalentForProp(tc2)) return true;
        return
          satisfies_constraint(
            Context{},
            lookup_constraint(Context{}, tc1),
            tc2
          ) && satisfies_constraint(
            Context{},
            lookup_constraint(Context{}, tc2),
            tc1
          );
      };
      auto const equiv = [&] {
        if (!equivOneTCPair(prop->typeConstraint, pprop->typeConstraint)) {
          return false;
        }
        for (auto ub : prop->ubs) {
          applyFlagsToUB(ub, prop->typeConstraint);
          auto foundEquiv = false;
          for (auto pub : pprop->ubs) {
            applyFlagsToUB(pub, pprop->typeConstraint);
            if (equivOneTCPair(ub, pub)) {
              foundEquiv = true;
              break;
            }
          }
          if (!foundEquiv) return false;
        }
        return true;
      };
      // If the property in the parent is static or private, the property in
      // the child isn't actually redeclaring anything. Otherwise, if the
      // type-hints are equivalent, remove this property from further
      // consideration and mark it as AttrNoBadRedeclare.
      if ((pprop->attrs & (AttrStatic | AttrPrivate)) || equiv()) {
        attribute_setter(prop->attrs, true, AttrNoBadRedeclare);
      }
      return true;
    };

    props.erase(
      std::remove_if(props.begin(), props.end(), propRedeclares),
      props.end()
    );

    currentCls = parent;
  }

  auto const possibleOverride =
    std::any_of(
      cls.properties.begin(),
      cls.properties.end(),
      [&](const php::Prop& prop) { return !(prop.attrs & AttrNoBadRedeclare); }
    );

  // Mark all resolutions of this class as having any possible bad redeclaration
  // props, even if there's not an unique resolution.
  auto const it = m_data->classInfo.find(cls.name);
  if (it != end(m_data->classInfo)) {
    auto const cinfo = it->second;
    if (cinfo->cls == &cls) {
      cinfo->hasBadRedeclareProp = possibleOverride;
    }
  }
}

/*
 * Rewrite the initial values for any AttrSystemInitialValue properties. If the
 * properties' type-hint does not admit null values, change the initial value to
 * one (if possible) to one that is not null. This is only safe to do so if the
 * property is not redeclared in a derived class or if the redeclaration does
 * not have a null system provided default value. Otherwise, a property can have
 * a null value (even if its type-hint doesn't allow it) without the JIT
 * realizing that its possible.
 *
 * Note that this ignores any unflattened traits. This is okay because
 * properties pulled in from traits which match an already existing property
 * can't change the initial value. The runtime will clear AttrNoImplicitNullable
 * on any property pulled from the trait if it doesn't match an existing
 * property.
 */
void Index::rewrite_default_initial_values() const {
  trace_time tracer("rewrite default initial values");

  /*
   * Use dataflow across the whole program class hierarchy. Start from the
   * classes which have no derived classes and flow up the hierarchy. We flow
   * the set of properties which have been assigned a null system provided
   * default value. If a property with such a null value flows into a class
   * which declares a property with the same name (and isn't static or private),
   * than that property is forced to be null as well.
   */
  using PropSet = folly::F14FastSet<SString>;
  using OutState = folly::F14FastMap<const ClassInfo*, PropSet>;
  using Worklist = folly::F14FastSet<const ClassInfo*>;

  OutState outStates;
  outStates.reserve(m_data->allClassInfos.size());

  // List of Class' still to process this iteration
  using WorkList = std::vector<const ClassInfo*>;
  using WorkSet = folly::F14FastSet<const ClassInfo*>;

  WorkList workList;
  WorkSet workSet;
  auto const enqueue = [&] (const ClassInfo& cls) {
    auto const result = workSet.insert(&cls);
    if (!result.second) return;
    workList.emplace_back(&cls);
  };

  // Start with all the leaf classes
  for (auto const& cinfo : m_data->allClassInfos) {
    auto const isLeaf = [&] {
      for (auto const& sub : cinfo->subclassList) {
        if (sub != cinfo.get()) return false;
      }
      return true;
    }();
    if (isLeaf) enqueue(*cinfo);
  }

  WorkList oldWorkList;
  int iter = 1;
  while (!workList.empty()) {
    FTRACE(
      4, "rewrite_default_initial_values round #{}: {} items\n",
      iter, workList.size()
    );
    ++iter;

    std::swap(workList, oldWorkList);
    workList.clear();
    workSet.clear();
    for (auto const& cinfo : oldWorkList) {
      // Retrieve the set of properties which are flowing into this Class and
      // have to be null.
      auto inState = [&] () -> Optional<PropSet> {
        PropSet in;
        for (auto const& sub : cinfo->subclassList) {
          if (sub == cinfo || sub->parent != cinfo) continue;
          auto const it = outStates.find(sub);
          if (it == outStates.end()) return std::nullopt;
          in.insert(it->second.begin(), it->second.end());
        }
        return in;
      }();
      if (!inState) continue;

      // Modify the in-state depending on the properties declared on this Class
      auto const cls = cinfo->cls;
      for (auto const& prop : cls->properties) {
        if (prop.attrs & (AttrStatic | AttrPrivate)) {
          // Private or static properties can't be redeclared
          inState->erase(prop.name);
          continue;
        }
        // Ignore properties which have actual user provided initial values or
        // are LateInit.
        if (!(prop.attrs & AttrSystemInitialValue) ||
            (prop.attrs & AttrLateInit)) {
          continue;
        }
        // Forced to be null, nothing to do
        if (inState->count(prop.name) > 0) continue;

        // Its not forced to be null. Find a better default value. If its null
        // anyways, force any properties this redeclares to be null as well.
        auto const defaultValue = prop.typeConstraint.defaultValue();
        if (defaultValue.m_type == KindOfNull) inState->insert(prop.name);
      }

      // Push the in-state to the out-state.
      auto const result = outStates.emplace(std::make_pair(cinfo, *inState));
      if (result.second) {
        if (cinfo->parent) enqueue(*cinfo->parent);
      } else {
        // There shouldn't be cycles in the inheritance tree, so the out state
        // of Class', once set, should never change.
        assertx(result.first->second == *inState);
      }
    }
  }

  // Now that we've processed all the classes, rewrite the property initial
  // values, unless they are forced to be nullable.
  for (auto& c : m_data->program->classes) {
    if (is_closure(*c)) continue;

    auto const out = [&] () -> Optional<PropSet> {
      Optional<PropSet> props;
      auto const range = m_data->classInfo.equal_range(c->name);
      for (auto it = range.first; it != range.second; ++it) {
        if (it->second->cls != c.get()) continue;
        auto const outStateIt = outStates.find(it->second);
        if (outStateIt == outStates.end()) return std::nullopt;
        if (!props) props.emplace();
        props->insert(outStateIt->second.begin(), outStateIt->second.end());
      }
      return props;
    }();

    for (auto& prop : c->properties) {
      auto const nullable = [&] {
        if (!(prop.attrs & (AttrStatic | AttrPrivate))) {
          if (!out || out->count(prop.name)) return true;
        }
        if (!(prop.attrs & AttrSystemInitialValue)) return false;
        return prop.typeConstraint.defaultValue().m_type == KindOfNull;
      }();

      attribute_setter(prop.attrs, !nullable, AttrNoImplicitNullable);
      if (!(prop.attrs & AttrSystemInitialValue)) continue;
      if (prop.val.m_type == KindOfUninit) {
        assertx(prop.attrs & AttrLateInit);
        continue;
      }

      prop.val = [&] {
        if (nullable) return make_tv<KindOfNull>();
        // Give the 86reified_prop a special default value to avoid
        // pessimizing the inferred type (we want it to always be a
        // vec of a specific size).
        if (prop.name == s_86reified_prop.get()) {
          return get_default_value_of_reified_list(c->userAttributes);
        }
        return prop.typeConstraint.defaultValue();
      }();
    }
  }
}

void Index::preinit_bad_initial_prop_values() {
  trace_time tracer("preinit bad initial prop values");
  parallel::for_each(
    m_data->allClassInfos,
    [&] (std::unique_ptr<ClassInfo>& cinfo) {
      if (is_used_trait(*cinfo->cls)) return;

      cinfo->hasBadInitialPropValues = false;
      for (auto& prop : const_cast<php::Class*>(cinfo->cls)->properties) {
        if (prop_might_have_bad_initial_value(*this, *cinfo->cls, prop)) {
          cinfo->hasBadInitialPropValues = true;
          prop.attrs = (Attr)(prop.attrs & ~AttrInitialSatisfiesTC);
        } else {
          prop.attrs |= AttrInitialSatisfiesTC;
        }
      }
    }
  );
}

void Index::preresolve_type_structures() {
  trace_time tracer("pre-resolve type-structures");

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
            resolve_type_structure(*this, *typeAlias).sarray()) {
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

res::Class Index::resolve_class(const php::Class* cls) const {

  auto const it = m_data->classInfo.find(cls->name);
  if (it != end(m_data->classInfo)) {
    auto const cinfo = it->second;
    if (cinfo->cls == cls) {
      return res::Class { cinfo };
    }
  }

  // We know its a class, not an enum or type alias, so return
  // by name
  return res::Class { cls->name.get() };
}

Optional<res::Class> Index::resolve_class(Context ctx,
                                          SString clsName) const {
  clsName = normalizeNS(clsName);

  if (ctx.cls) {
    if (ctx.cls->name->isame(clsName)) {
      return resolve_class(ctx.cls);
    }
    if (ctx.cls->parentName && ctx.cls->parentName->isame(clsName)) {
      if (auto const parent = resolve_class(ctx.cls).parent()) return parent;
    }
  }

  auto const it = m_data->classInfo.find(clsName);
  if (it != end(m_data->classInfo)) {
    auto const tinfo = it->second;
    /*
     * If the preresolved ClassInfo is Unique we can give it out.
     */
    assertx(tinfo->cls->attrs & AttrUnique);
    if (debug && m_data->typeAliases.count(clsName)) {
      std::fprintf(stderr, "non unique \"unique\" class: %s\n",
                   tinfo->cls->name->data());

      auto const ta = m_data->typeAliases.find(clsName);
      if (ta != end(m_data->typeAliases)) {
        std::fprintf(stderr, "   and type-alias %s\n",
                      ta->second->name->data());
      }
      always_assert(false);
    }
    return res::Class { tinfo };
  }
  // We refuse to have name-only resolutions of enums and typeAliases,
  // so that all name only resolutions can be treated as classes.
  if (!m_data->enums.count(clsName) &&
      !m_data->typeAliases.count(clsName)) {
    return res::Class { clsName };
  }

  return std::nullopt;
}

Optional<res::Class> Index::selfCls(const Context& ctx) const {
  if (!ctx.cls || is_used_trait(*ctx.cls)) return std::nullopt;
  return resolve_class(ctx.cls);
}

Optional<res::Class> Index::parentCls(const Context& ctx) const {
  if (!ctx.cls || !ctx.cls->parentName) return std::nullopt;
  if (auto const parent = resolve_class(ctx.cls).parent()) return parent;
  return resolve_class(ctx, ctx.cls->parentName);
}

const php::TypeAlias* Index::lookup_type_alias(SString name) const {
  auto const it = m_data->typeAliases.find(name);
  if (it == m_data->typeAliases.end()) return nullptr;
  return it->second;
}

Index::ResolvedInfo<Optional<res::Class>>
Index::resolve_type_name(SString inName) const {
  Optional<hphp_fast_set<const void*>> seen;

  auto nullable = false;
  auto name = inName;

  for (unsigned i = 0; ; ++i) {
    name = normalizeNS(name);
    auto const cls_it = m_data->classInfo.find(name);
    if (cls_it != end(m_data->classInfo)) {
      auto const cinfo = cls_it->second;
      assertx(cinfo->cls->attrs & AttrUnique);
      if (!(cinfo->cls->attrs & AttrEnum)) {
        return { AnnotType::Object, nullable, res::Class { cinfo } };
      }
      auto const& tc = cinfo->cls->enumBaseTy;
      assertx(!tc.isNullable());
      if (!tc.isUnresolved()) {
        auto const type = tc.isMixed() ? AnnotType::ArrayKey : tc.type();
        assertx(type != AnnotType::Object);
        return { type, nullable, {} };
      }
      name = tc.typeName();
    } else {
      auto const ta_it = m_data->typeAliases.find(name);
      if (ta_it == end(m_data->typeAliases)) break;
      auto const ta = ta_it->second;
      assertx(ta->attrs & AttrUnique);
      nullable = nullable || ta->nullable;
      if (ta->type != AnnotType::Unresolved) {
        assertx(ta->type != AnnotType::Object);
        return { ta->type, nullable, {} };
      }
      name = ta->value;
    }

    // deal with cycles. Since we don't expect to
    // encounter them, just use a counter until we hit a chain length
    // of 10, then start tracking the names we resolve.
    if (i == 10) {
      seen.emplace();
      seen->insert(name);
    } else if (i > 10) {
      if (!seen->insert(name).second) {
        return { AnnotType::Unresolved, false, {} };
      }
    }
  }

  return { AnnotType::Object, nullable, res::Class { name } };
}

struct Index::ConstraintResolution {
  /* implicit */ ConstraintResolution(Type type)
    : type{std::move(type)}
    , maybeMixed{false} {}
  ConstraintResolution(Optional<Type> type, bool maybeMixed)
    : type{std::move(type)}
    , maybeMixed{maybeMixed} {}

  Optional<Type> type;
  bool maybeMixed;
};

Index::ConstraintResolution Index::resolve_named_type(
  const Context& ctx, SString name, const Type& candidate) const {

  auto const res = resolve_type_name(name);

  if (res.nullable && candidate.subtypeOf(BInitNull)) return TInitNull;

  if (res.type == AnnotType::Unresolved) return TInitCell;

  if (res.type == AnnotType::Object) {
    auto resolve = [&] (const res::Class& rcls) -> Optional<Type> {
      if (!interface_supports_non_objects(rcls.name()) ||
          candidate.subtypeOf(BOptObj)) {
        return subObj(rcls);
      }

      if (candidate.subtypeOf(BOptVec)) {
        if (interface_supports_arrlike(rcls.name())) return TVec;
      } else if (candidate.subtypeOf(BOptDict)) {
        if (interface_supports_arrlike(rcls.name())) return TDict;
      } else if (candidate.subtypeOf(BOptKeyset)) {
        if (interface_supports_arrlike(rcls.name())) return TKeyset;
      } else if (candidate.subtypeOf(BOptStr)) {
        if (interface_supports_string(rcls.name())) return TStr;
      } else if (candidate.subtypeOf(BOptInt)) {
        if (interface_supports_int(rcls.name())) return TInt;
      } else if (candidate.subtypeOf(BOptDbl)) {
        if (interface_supports_double(rcls.name())) return TDbl;
      }
      return std::nullopt;
    };

    auto ty = resolve(*res.value);
    if (ty && res.nullable) *ty = opt(std::move(*ty));
    return ConstraintResolution{ std::move(ty), false };
  }

  return get_type_for_annotated_type(ctx, res.type, res.nullable, nullptr,
                                     candidate);
}

std::pair<res::Class, const php::Class*>
Index::resolve_closure_class(Context ctx, int32_t idx) const {
  auto const cls = lookup_unit_class(*ctx.unit, idx);
  auto const rcls = resolve_class(cls);

  // Closure classes must be unique and defined in the unit that uses
  // the CreateCl opcode, so resolution must succeed.
  always_assert_flog(
    rcls.resolved(),
    "A Closure class ({}) failed to resolve",
    cls->name
  );

  return { rcls, cls };
}

res::Class Index::builtin_class(SString name) const {
  auto const rcls = resolve_class(Context {}, name);
  always_assert_flog(
    rcls.has_value() &&
    rcls->val.right() &&
    (rcls->val.right()->cls->attrs & AttrBuiltin),
    "A builtin class ({}) failed to resolve",
    name->data()
  );
  return *rcls;
}

res::Func Index::resolve_method(Context ctx,
                                Type clsType,
                                SString name) const {
  auto const general = [&] {
    // Even if we can't resolve to a FuncFamily from the class, we
    // can still perhaps use a (less percise) func family derived
    // from just the method name.
    auto const singleMethIt = m_data->singleMethodFamilies.find(name);
    if (singleMethIt != m_data->singleMethodFamilies.end()) {
      return res::Func { res::Func::MethodOrMissing{ singleMethIt->second }};
    }
    auto const methIt = m_data->methodFamilies.find(name);
    if (methIt != m_data->methodFamilies.end()) {
      return res::Func { methIt->second };
    }
    return res::Func { res::Func::MethodName { name } };
  };

  if (!is_specialized_cls(clsType)) return general();

  // Return the appropriate res::Func for a given ClassInfo (either
  // exact or sub).
  auto const process = [&] (ClassInfo* cinfo, bool isExact) {
    // Classes may have more method families than methods. Any such
    // method families are guaranteed to all be public so we can do
    // this lookup as a last gasp before resorting to general().
    auto const find_extra_method = [&] {
      auto singleMethIt = cinfo->singleMethodFamilies.find(name);
      if (singleMethIt != cinfo->singleMethodFamilies.end()) {
        return res::Func { singleMethIt->second };
      }
      auto methIt = cinfo->methodFamilies.find(name);
      if (methIt == end(cinfo->methodFamilies)) return general();
      // If there was a sole implementer we can resolve to a single method, even
      // if the method was not declared on the interface itself.
      assertx(methIt->second->possibleFuncs().size() > 1);
      return res::Func { methIt->second };
    };

    // Interfaces *only* have the extra methods defined for all
    // subclasses
    if (cinfo->cls->attrs & AttrInterface) return find_extra_method();

    /*
     * Whether or not the context class has a private method with the
     * same name as the method we're trying to call.
     */
    auto const contextMayHavePrivateWithSameName = folly::lazy([&]() -> bool {
      if (!ctx.cls) return false;
      auto const cls_it = m_data->classInfo.find(ctx.cls->name);
      if (cls_it == end(m_data->classInfo)) {
        // This class had no pre-resolved ClassInfos, which means it
        // always fatals in any way it could be defined, so it doesn't
        // matter what we return here (as all methods in the context
        // class are unreachable code).
        return true;
      }
      // Because of traits, each instantiation of the class could have
      // different private methods; we need to check them all.
      auto const iter = cls_it->second->methods.find(name);
      if (iter != end(cls_it->second->methods) &&
          iter->second.attrs & AttrPrivate &&
          iter->second.topLevel) {
        return true;
      }
      return false;
    });

    /*
     * Look up the method in the target class.
     */
    auto const methIt = cinfo->methods.find(name);
    if (methIt == end(cinfo->methods)) {
      return isExact ? general() : find_extra_method();
    }
    auto const ftarget = methIt->second.func;

    // Be conservative around unflattened trait methods, since their cls
    // may change at runtime.
    if (ftarget->cls && is_used_trait(*ftarget->cls)) {
      return general();
    }

    // We need to revisit the hasPrivateAncestor code if we start being
    // able to look up methods on interfaces (currently they have empty
    // method tables).
    assertx(!(cinfo->cls->attrs & AttrInterface));

    /*
     * If our candidate method has a private ancestor, unless it is
     * defined on this class, we need to make sure we don't erroneously
     * resolve the overriding method if the call is coming from the
     * context the defines the private method.
     *
     * For now this just gives up if the context and the callee class
     * could be related and the context defines a private of the same
     * name.  (We should actually try to resolve that method, though.)
     */
    if (methIt->second.hasPrivateAncestor &&
        ctx.cls &&
        ctx.cls != ftarget->cls) {
      if (could_be_related(ctx.cls, cinfo->cls)) {
        if (contextMayHavePrivateWithSameName()) {
          return general();
        }
      }
    }

    auto const resolve = [&] {
      create_func_info(*m_data, ftarget);
      return res::Func { mteFromIt(methIt) };
    };

    if (isExact) return resolve();
    if (methIt->second.attrs & AttrNoOverride) {
      return resolve();
    }

    auto const singleFamIt = cinfo->singleMethodFamilies.find(name);
    if (singleFamIt != cinfo->singleMethodFamilies.end()) {
      return res::Func { singleFamIt->second };
    }
    auto const famIt = cinfo->methodFamilies.find(name);
    if (famIt == end(cinfo->methodFamilies)) return general();
    assertx(famIt->second->possibleFuncs().size() > 1);
    return res::Func { famIt->second };
  };

  auto const& dcls  = dcls_of(clsType);
  if (!dcls.isIsect()) {
    // If this isn't an intersection, we can just get the res::Func
    // for this one ClassInfo and we're done.
    auto const cinfo = dcls.cls().val.right();
    if (!cinfo) return general();
    return process(cinfo, dcls.isExact());
  }

  using Func = res::Func;

  // Otherwise get a res::Func for all members of the intersection and
  // combine them into a res::Func::Isect. Since the DCls represents a
  // class which is a subtype of every ClassInfo in the list, every
  // res::Func we get is true. This implies that it is invalid for two
  // different members of the intersection to resolve to different
  // MethTabEntryPairs, for example.

  auto maybeMissing = true;
  Func::Isect isect;
  const MethTabEntryPair* singleMte = nullptr;
  for (auto const i : dcls.isect()) {
    auto const cinfo = i.val.right();
    if (!cinfo) continue;

    auto const func = process(cinfo, false);
    match<void>(
      func.val,
      [&] (Func::MethodName) {},
      [&] (const MethTabEntryPair* mte) {
        assertx(IMPLIES(singleMte, singleMte->second.func == mte->second.func));
        if (!singleMte) singleMte = mte;
        maybeMissing = false;
      },
      [&] (FuncFamily* ff) {
        isect.families.emplace_back(ff);
        maybeMissing = false;
      },
      [&] (Func::MethodOrMissing m) {
        assertx(
          IMPLIES(singleMte, singleMte->second.func == m.mte->second.func)
        );
        if (!singleMte) singleMte = m.mte;
      },
      [&] (Func::FuncName)     { always_assert(false); },
      [&] (FuncInfo*)          { always_assert(false); },
      [&] (const Func::Isect&) { always_assert(false); }
    );
  }

  // If we got a MethTabEntryPair, that always wins. Again, every
  // res::Func is true, and MethTabEntryPair is more specific than a
  // FuncFamily, so it is preferred.
  if (singleMte) {
    // If maybeMissing is true, then *every* resolution was to a
    // MethodName or MethodOrMissing, so include that fact here by
    // using MethodOrMissing.
    if (maybeMissing) return Func { Func::MethodOrMissing { singleMte } };
    return Func { singleMte };
  }
  // We only got unresolved classes, so be pessimistic.
  if (isect.families.empty()) return general();

  // We could add a FuncFamily multiple times, so remove duplicates.
  std::sort(begin(isect.families), end(isect.families));
  isect.families.erase(
    std::unique(begin(isect.families), end(isect.families)),
    end(isect.families)
  );
  // If everything simplifies down to a single FuncFamily, just use
  // that.
  if (isect.families.size() == 1) return Func { isect.families[0] };
  return Func { std::move(isect) };
}

Optional<res::Func>
Index::resolve_ctor(Context /*ctx*/, res::Class rcls, bool exact) const {
  auto const cinfo = rcls.val.right();
  if (!cinfo) return std::nullopt;
  if (cinfo->cls->attrs & (AttrInterface|AttrTrait)) return std::nullopt;

  auto const cit = cinfo->methods.find(s_construct.get());
  if (cit == end(cinfo->methods)) return std::nullopt;

  auto const ctor = mteFromIt(cit);
  if (exact || ctor->second.attrs & AttrNoOverride) {
    create_func_info(*m_data, ctor->second.func);
    return res::Func { ctor };
  }

  auto const singleFamIt = cinfo->singleMethodFamilies.find(s_construct.get());
  if (singleFamIt != cinfo->singleMethodFamilies.end()) {
    return res::Func { singleFamIt->second};
  }
  auto const famIt = cinfo->methodFamilies.find(s_construct.get());
  if (famIt == end(cinfo->methodFamilies)) return std::nullopt;
  assertx(famIt->second->possibleFuncs().size() > 1);
  return res::Func { famIt->second };
}

res::Func
Index::resolve_func_helper(const php::Func* func, SString name) const {
  auto name_only = [&] (bool renamable) {
    return res::Func { res::Func::FuncName { name, renamable } };
  };

  // no resolution
  if (func == nullptr) return name_only(false);

  // single resolution, in whole-program mode, that's it
  assertx(func->attrs & AttrUnique);
  return do_resolve(func);
}

res::Func Index::resolve_func(Context /*ctx*/, SString name) const {
  name = normalizeNS(name);
  auto const it = m_data->funcs.find(name);
  return resolve_func_helper((it != end(m_data->funcs)) ? it->second : nullptr, name);
}

/*
 * Gets a type for the constraint.
 *
 * If getSuperType is true, the type could be a super-type of the
 * actual type constraint (eg TCell). Otherwise its guaranteed that
 * for any t, t.subtypeOf(get_type_for_constraint<false>(ctx, tc, t)
 * implies t would pass the constraint.
 *
 * The candidate type is used to disambiguate; if we're applying a
 * Traversable constraint to a TObj, we should return
 * subObj(Traversable).  If we're applying it to an Array, we should
 * return Array.
 */
template<bool getSuperType>
Type Index::get_type_for_constraint(Context ctx,
                                    const TypeConstraint& tc,
                                    const Type& candidate) const {
  assertx(IMPLIES(!tc.isCheckable(),
                   tc.isMixed() ||
                   (tc.isUpperBound() &&
                    RuntimeOption::EvalEnforceGenericsUB == 0)));

  if (getSuperType) {
    /*
     * Soft hints (@Foo) are not checked.
     * Also upper-bound type hints are not checked when they do not error.
     */
    if (tc.isSoft() ||
        (RuntimeOption::EvalEnforceGenericsUB < 2 && tc.isUpperBound())) {
      return TCell;
    }
  }

  auto const res = get_type_for_annotated_type(
    ctx,
    tc.type(),
    tc.isNullable(),
    tc.isObject() ? tc.clsName() : tc.typeName(),
    candidate
  );
  if (res.type) return *res.type;
  // If the type constraint might be mixed, then the value could be
  // uninit. Any other type constraint implies TInitCell.
  return getSuperType ? (res.maybeMixed ? TCell : TInitCell) : TBottom;
}

bool Index::prop_tc_maybe_unenforced(const php::Class& propCls,
                                     const TypeConstraint& tc) const {
  assertx(tc.validForProp());
  if (RuntimeOption::EvalCheckPropTypeHints <= 2) return true;
  if (!tc.isCheckable()) return true;
  if (tc.isSoft()) return true;
  if (tc.isUpperBound() && RuntimeOption::EvalEnforceGenericsUB < 2) {
    return true;
  }
  auto const res = get_type_for_annotated_type(
    Context { nullptr, nullptr, &propCls },
    tc.type(),
    tc.isNullable(),
    tc.isObject() ? tc.clsName() : tc.typeName(),
    TCell
  );
  return res.maybeMixed;
}

Index::ConstraintResolution Index::get_type_for_annotated_type(
  Context ctx, AnnotType annot, bool nullable,
  SString name, const Type& candidate) const {

  if (candidate.subtypeOf(BInitNull) && nullable) {
    return TInitNull;
  }

  auto mainType = [&]() -> ConstraintResolution {
    switch (getAnnotMetaType(annot)) {
    case AnnotMetaType::Precise: {
      auto const dt = getAnnotDataType(annot);

      switch (dt) {
      case KindOfNull:         return TNull;
      case KindOfBoolean:      return TBool;
      case KindOfInt64:        return TInt;
      case KindOfDouble:       return TDbl;
      case KindOfPersistentString:
      case KindOfString:       return TStr;
      case KindOfPersistentVec:
      case KindOfVec:          return TVec;
      case KindOfPersistentDict:
      case KindOfDict:         return TDict;
      case KindOfPersistentKeyset:
      case KindOfKeyset:       return TKeyset;
      case KindOfResource:     return TRes;
      case KindOfClsMeth:      return TClsMeth;
      case KindOfObject:
        return resolve_named_type(ctx, name, candidate);
      case KindOfUninit:
      case KindOfRFunc:
      case KindOfFunc:
      case KindOfRClsMeth:
      case KindOfClass:
      case KindOfLazyClass:
        always_assert_flog(false, "Unexpected DataType");
        break;
      }
      break;
    }
    case AnnotMetaType::Mixed:
      /*
       * Here we handle "mixed", typevars, and some other ignored
       * typehints (ex. "(function(..): ..)" typehints).
       */
      return { TCell, true };
    case AnnotMetaType::Nothing:
    case AnnotMetaType::NoReturn:
      return TBottom;
    case AnnotMetaType::Nonnull:
      if (candidate.subtypeOf(BInitNull)) return TBottom;
      if (!candidate.couldBe(BInitNull))  return candidate;
      return unopt(candidate);
    case AnnotMetaType::This:
      if (auto s = selfCls(ctx)) return setctx(subObj(*s));
      break;
    case AnnotMetaType::Callable:
      break;
    case AnnotMetaType::Number:
      return TNum;
    case AnnotMetaType::ArrayKey:
      if (candidate.subtypeOf(BInt)) return TInt;
      if (candidate.subtypeOf(BStr)) return TStr;
      return TArrKey;
    case AnnotMetaType::VecOrDict:
      if (candidate.subtypeOf(BVec)) return TVec;
      if (candidate.subtypeOf(BDict)) return TDict;
      return union_of(TVec, TDict);
    case AnnotMetaType::ArrayLike:
      if (candidate.subtypeOf(BVec)) return TVec;
      if (candidate.subtypeOf(BDict)) return TDict;
      if (candidate.subtypeOf(BKeyset)) return TKeyset;
      return TArrLike;
    case AnnotMetaType::Classname:
      if (candidate.subtypeOf(BStr)) return TStr;
      if (!RuntimeOption::EvalClassnameNotices) {
        if (candidate.subtypeOf(BCls)) return TCls;
        if (candidate.subtypeOf(BLazyCls)) return TLazyCls;
      }
      break;
    case AnnotMetaType::Unresolved:
      return resolve_named_type(ctx, name, candidate);
    }
    return ConstraintResolution{ std::nullopt, false };
  }();

  if (mainType.type && nullable) {
    if (mainType.type->subtypeOf(BBottom)) {
      if (candidate.couldBe(BInitNull)) {
        mainType.type = TInitNull;
      }
    } else if (!mainType.type->couldBe(BInitNull)) {
      mainType.type = opt(*mainType.type);
    }
  }
  return mainType;
}

Type Index::lookup_constraint(Context ctx,
                              const TypeConstraint& tc,
                              const Type& t) const {
  return get_type_for_constraint<true>(ctx, tc, t);
}

bool Index::satisfies_constraint(Context ctx, const Type& t,
                                 const TypeConstraint& tc) const {
  auto const tcType = get_type_for_constraint<false>(ctx, tc, t);
  return t.moreRefined(tcType);
}

bool Index::could_have_reified_type(Context ctx,
                                    const TypeConstraint& tc) const {
  if (ctx.func->isClosureBody) {
    for (auto i = ctx.func->params.size();
         i < ctx.func->locals.size();
         ++i) {
      auto const name = ctx.func->locals[i].name;
      if (!name) return false; // named locals do not appear after unnamed local
      if (isMangledReifiedGenericInClosure(name)) return true;
    }
    return false;
  }
  if (!tc.isObject() && !tc.isUnresolved()) return false;
  auto const name = tc.isObject() ? tc.clsName() : tc.typeName();
  auto const resolved = resolve_type_name(name);
  if (resolved.type == AnnotType::Unresolved) return true;
  if (resolved.type != AnnotType::Object) return false;
  return resolved.value->couldHaveReifiedGenerics();
}

std::tuple<Type, bool> Index::verify_param_type(Context ctx, uint32_t paramId,
                                                Type t) const {
  auto const& pinfo = ctx.func->params[paramId];
  bool effectFree = true;
  std::vector<const TypeConstraint*> tcs{&pinfo.typeConstraint};
  for (auto const& tc : pinfo.upperBounds) tcs.push_back(&tc);

  for (auto const tc : tcs) {
    if (!tc->isCheckable()) continue;
    if (satisfies_constraint(ctx, t, *tc)) continue;

    effectFree = false;

    if (tc->mayCoerce() && t.couldBe(BCls | BLazyCls)) {
      // Add the result of possible coercion.
      t = union_of(std::move(t), TStr);
    }

    auto tcTy = lookup_constraint(ctx, *tc);
    if (tc->isThis()) {
      auto const cls = selfCls(ctx);
      if (cls && cls->couldBeMocked()) tcTy = unctx(std::move(tcTy));
    }

    t = intersection_of(std::move(t), std::move(tcTy));
  }

  return { t, effectFree };
}

TriBool
Index::supports_async_eager_return(res::Func rfunc) const {
  return match<TriBool>(
    rfunc.val,
    [&](res::Func::FuncName)   { return TriBool::Maybe; },
    [&](res::Func::MethodName) { return TriBool::Maybe; },
    [&](FuncInfo* finfo) {
      return func_supports_AER(finfo->func);
    },
    [&](const MethTabEntryPair* mte) {
      return func_supports_AER(mte->second.func);
    },
    [&](FuncFamily* fam) { return fam->m_static->m_supportsAER; },
    [&](res::Func::MethodOrMissing m) {
      return func_supports_AER(m.mte->second.func);
    },
    [&](const res::Func::Isect& i) {
      auto aer = TriBool::Maybe;
      for (auto const ff : i.families) {
        if (ff->m_static->m_supportsAER == TriBool::Maybe) continue;
        assertx(
          IMPLIES(aer != TriBool::Maybe, aer == ff->m_static->m_supportsAER)
        );
        if (aer == TriBool::Maybe) aer = ff->m_static->m_supportsAER;
      }
      return aer;
    }
  );
}

bool Index::is_effect_free_raw(const php::Func* func) const {
  return func_info(*m_data, func)->effectFree;
}

bool Index::is_effect_free(Context ctx, const php::Func* func) const {
  add_dependency(*m_data, func, ctx, Dep::InlineDepthLimit);
  return func_info(*m_data, func)->effectFree;
}

bool Index::is_effect_free(Context ctx, res::Func rfunc) const {
  auto const processFF = [&] (FuncFamily* ff) {
    add_dependency(*m_data, ff, ctx, Dep::InlineDepthLimit);
    for (auto const mte : ff->possibleFuncs()) {
      if (!func_info(*m_data, mte->second.func)->effectFree) return false;
    }
    return true;
  };

  return match<bool>(
    rfunc.val,
    [&](res::Func::FuncName)   { return false; },
    [&](res::Func::MethodName) { return false; },
    [&](FuncInfo* finfo)       {
      add_dependency(*m_data, finfo->func, ctx, Dep::InlineDepthLimit);
      return finfo->effectFree;
    },
    [&](const MethTabEntryPair* mte) {
      add_dependency(*m_data, mte->second.func, ctx, Dep::InlineDepthLimit);
      return func_info(*m_data, mte->second.func)->effectFree;
    },
    [&](FuncFamily* fam) { return processFF(fam); },
    [&] (res::Func::MethodOrMissing m) { return false; },
    [&] (const res::Func::Isect& i) {
      for (auto const ff : i.families) {
        if (processFF(ff)) return true;
      }
      return false;
    }
  );
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
    f(cinfo);
    return true;
  } else if (dcls.isSub()) {
    auto const cinfo = dcls.cls().val.right();
    if (!cinfo) return false;
    for (auto const sub : cinfo->subclassList) {
      if (!f(sub)) break;
    }
    return true;
  }
  auto const& isect = dcls.isect();
  assertx(isect.size() > 1);

  auto unresolved = false;
  res::Class::visitEverySub(
    isect,
    false,
    [&] (res::Class c) {
      if (auto const cinfo = c.val.right()) {
        return f(cinfo);
      }
      unresolved = true;
      return false;
    }
  );
  return !unresolved;
}

ClsConstLookupResult<> Index::lookup_class_constant(Context ctx,
                                                    const Type& cls,
                                                    const Type& name) const {
  ITRACE(4, "lookup_class_constant: ({}) {}::{}\n",
         show(ctx), show(cls), show(name));
  Trace::Indent _;

  using R = ClsConstLookupResult<>;

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

    // Determine the constant's value and return it
    auto const r = [&] {
      if (cns.val->m_type == KindOfUninit) {
        // Constant is defined by a 86cinit. Use the result from
        // analysis and add a dependency. We cannot cache in this
        // case.
        cachable = false;
        if (ctx.func) {
          auto const cinit = cns.cls->methods.back().get();
          assertx(cinit->name == s_86cinit.get());
          add_dependency(*m_data, cinit, ctx, Dep::ClsConst);
        }

        ITRACE(4, "(dynamic)\n");
        auto const it =
          m_data->clsConstTypes.find(std::make_pair(cns.cls, cns.name));
        auto const type =
          (it == m_data->clsConstTypes.end()) ? TInitCell : it->second.type;
        return R{ type, TriBool::Yes, true };
      }

      // Fully resolved constant with a known value
      return R{ from_cell(*cns.val), TriBool::Yes, false };
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

ClsTypeConstLookupResult<>
Index::lookup_class_type_constant(
    const Type& cls,
    const Type& name,
    const ClsTypeConstLookupResolver& resolver) const {
  ITRACE(4, "lookup_class_type_constant: {}::{}\n", show(cls), show(name));
  Trace::Indent _;

  using R = ClsTypeConstLookupResult<>;

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
    ITRACE(4, "({}) {}\n", cns.cls->name, show(dict_val(val(*cns.val).parr)));

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
  if (iter == end(m_data->constants)) {
    return TInitCell;
  }

  auto constant = iter->second;
  if (type(constant->val) != KindOfUninit) {
    return from_cell(constant->val);
  }

  auto const func_name = Constant::funcNameFromName(cnsName);
  assertx(func_name && "func_name will never be nullptr");

  auto rfunc = resolve_func(ctx, func_name);
  return lookup_return_type(ctx, nullptr, rfunc, Dep::ConstVal);
}

bool Index::func_depends_on_arg(const php::Func* func, int arg) const {
  auto const& finfo = *func_info(*m_data, func);
  return arg >= finfo.unusedParams.size() || !finfo.unusedParams.test(arg);
}

Type Index::lookup_foldable_return_type(Context ctx,
                                        const CallContext& calleeCtx) const {
  auto const func = calleeCtx.callee;
  constexpr auto max_interp_nexting_level = 2;
  static __thread uint32_t interp_nesting_level;
  static __thread Context base_ctx;

  auto const ctxType = adjust_closure_context(*this, calleeCtx);

  // Don't fold functions when staticness mismatches
  if (!func->isClosureBody) {
    if ((func->attrs & AttrStatic) && ctxType.couldBe(TObj)) return TInitCell;
    if (!(func->attrs & AttrStatic) && ctxType.couldBe(TCls)) return TInitCell;
  }

  auto const& finfo = *func_info(*m_data, func);
  if (finfo.effectFree && is_scalar(finfo.returnTy)) return finfo.returnTy;

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

      assertx(is_scalar(acc->second));
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
    return TInitCell;
  }

  if (!interp_nesting_level) {
    base_ctx = ctx;
  } else if (interp_nesting_level > max_interp_nexting_level) {
    add_dependency(*m_data, func, base_ctx, Dep::InlineDepthLimit);
    return TInitCell;
  }

  auto const contextType = [&] {
    ++interp_nesting_level;
    SCOPE_EXIT { --interp_nesting_level; };

    auto const wf = php::WideFunc::cns(func);
    auto const fa = analyze_func_inline(
      *this,
      AnalysisContext { m_data->units.at(func->unit), wf, func->cls },
      ctxType,
      calleeCtx.args,
      CollectionOpts::EffectFreeOnly
    );
    return fa.effectFree ? fa.inferredReturn : TInitCell;
  }();

  if (!is_scalar(contextType)) return TInitCell;

  ContextRetTyMap::accessor acc;
  if (m_data->foldableReturnTypeMap.insert(acc, calleeCtx)) {
    acc->second = contextType;
  } else {
    // someone beat us to it
    assertx(acc->second == contextType);
  }
  return contextType;
}

Type Index::lookup_return_type(Context ctx,
                               MethodsInfo* methods,
                               const php::Func* f,
                               Dep dep) const {
  if (methods) {
    if (auto ret = methods->lookupReturnType(*f)) {
      return unctx(std::move(*ret));
    }
  }
  auto it = func_info(*m_data, f);
  if (it->func) {
    assertx(it->func == f);
    add_dependency(*m_data, f, ctx, dep);
    return it->returnTy;
  }
  return TInitCell;
}

Type Index::lookup_return_type(Context ctx,
                               MethodsInfo* methods,
                               res::Func rfunc,
                               Dep dep) const {
  auto const funcFamily = [&] (FuncFamily* fam) {
    add_dependency(*m_data, fam, ctx, dep);
    return fam->m_returnTy.get(
      [&] {
        auto ret = TBottom;
        for (auto const pf : fam->possibleFuncs()) {
          auto const finfo = func_info(*m_data, pf->second.func);
          if (!finfo->func) return TInitCell;
          ret |= unctx(finfo->returnTy);
          if (!ret.strictSubtypeOf(BInitCell)) return ret;
        }
        return ret;
      }
    );
  };
  auto const methTab = [&] (const MethTabEntryPair* mte) {
    if (methods) {
      if (auto ret = methods->lookupReturnType(*mte->second.func)) {
        return unctx(std::move(*ret));
      }
    }
    add_dependency(*m_data, mte->second.func, ctx, dep);
    auto const finfo = func_info(*m_data, mte->second.func);
    if (!finfo->func) return TInitCell;
    return unctx(finfo->returnTy);
  };

  return match<Type>(
    rfunc.val,
    [&] (res::Func::FuncName)   { return TInitCell; },
    [&] (res::Func::MethodName) { return TInitCell; },
    [&] (FuncInfo* finfo) {
      add_dependency(*m_data, finfo->func, ctx, dep);
      return unctx(finfo->returnTy);
    },
    [&] (const MethTabEntryPair* mte)  { return methTab(mte); },
    [&] (FuncFamily* fam)              { return funcFamily(fam); },
    [&] (res::Func::MethodOrMissing m) { return methTab(m.mte); },
    [&] (const res::Func::Isect& i) {
      auto ty = TInitCell;
      for (auto const ff : i.families) ty &= funcFamily(ff);
      return ty;
    }
  );
}

Type Index::lookup_return_type(Context caller,
                               MethodsInfo* methods,
                               const CompactVector<Type>& args,
                               const Type& context,
                               res::Func rfunc,
                               Dep dep) const {
  auto const funcFamily = [&] (FuncFamily* fam) {
    add_dependency(*m_data, fam, caller, dep);
    auto ret = fam->m_returnTy.get(
      [&] {
        auto ty = TBottom;
        for (auto const pf : fam->possibleFuncs()) {
          auto const finfo = func_info(*m_data, pf->second.func);
          if (!finfo->func) return TInitCell;
          ty |= finfo->returnTy;
          if (!ty.strictSubtypeOf(BInitCell)) return ty;
        }
        return ty;
      }
    );
    return return_with_context(std::move(ret), context);
  };
  auto const methTab = [&] (const MethTabEntryPair* mte) {
    auto const finfo = func_info(*m_data, mte->second.func);
    if (!finfo->func) return TInitCell;

    auto returnType = [&] {
      if (methods) {
        if (auto ret = methods->lookupReturnType(*mte->second.func)) {
          return *ret;
        }
      }
      add_dependency(*m_data, mte->second.func, caller, dep);
      return finfo->returnTy;
    }();

    return context_sensitive_return_type(
      *m_data,
      { finfo->func, args, context },
      std::move(returnType)
    );
  };

  return match<Type>(
    rfunc.val,
    [&] (res::Func::FuncName) {
      return lookup_return_type(caller, methods, rfunc, dep);
    },
    [&] (res::Func::MethodName) {
      return lookup_return_type(caller, methods, rfunc, dep);
    },
    [&] (FuncInfo* finfo) {
      add_dependency(*m_data, finfo->func, caller, dep);
      return context_sensitive_return_type(
        *m_data,
        { finfo->func, args, context },
        finfo->returnTy
      );
    },
    [&] (const MethTabEntryPair* mte)  { return methTab(mte); },
    [&] (FuncFamily* fam)              { return funcFamily(fam); },
    [&] (res::Func::MethodOrMissing m) { return methTab(m.mte); },
    [&] (const res::Func::Isect& i) {
      auto ty = TInitCell;
      for (auto const ff : i.families) ty &= funcFamily(ff);
      return ty;
    }
  );
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

std::pair<Type, size_t> Index::lookup_return_type_raw(const php::Func* f) const {
  auto it = func_info(*m_data, f);
  if (it->func) {
    assertx(it->func == f);
    return { it->returnTy, it->returnRefinements };
  }
  return { TInitCell, 0 };
}

bool Index::lookup_this_available(const php::Func* f) const {
  return !(f->cls->attrs & AttrTrait) && !(f->attrs & AttrStatic);
}

Optional<uint32_t> Index::lookup_num_inout_params(
  Context,
  res::Func rfunc
) const {
  return match<Optional<uint32_t>>(
    rfunc.val,
    [&] (res::Func::FuncName s) -> Optional<uint32_t> {
      if (s.renamable) return std::nullopt;
      auto const it = m_data->funcs.find(s.name);
      return it != end(m_data->funcs)
       ? func_num_inout(it->second)
       : 0;
    },
    [&] (res::Func::MethodName s) -> Optional<uint32_t> {
      return std::nullopt;
    },
    [&] (FuncInfo* finfo) {
      return func_num_inout(finfo->func);
    },
    [&] (const MethTabEntryPair* mte) {
      return func_num_inout(mte->second.func);
    },
    [&] (FuncFamily* fam) -> Optional<uint32_t> {
      return fam->m_static->m_numInOut;
    },
    [&] (res::Func::MethodOrMissing m) {
      return func_num_inout(m.mte->second.func);
    },
    [&] (const res::Func::Isect& i) {
      Optional<uint32_t> numInOut;
      for (auto const ff : i.families) {
        if (!ff->m_static->m_numInOut) continue;
        assertx(IMPLIES(numInOut, *numInOut == *ff->m_static->m_numInOut));
        if (!numInOut) numInOut = ff->m_static->m_numInOut;
      }
      return numInOut;
    }
  );
}

PrepKind Index::lookup_param_prep(Context,
                                  res::Func rfunc,
                                  uint32_t paramId) const {
  auto const fromFuncFamily = [&] (FuncFamily* ff) {
    if (paramId >= ff->m_static->m_paramPreps.size()) {
      return PrepKind{TriBool::No, TriBool::No};
    }
    return ff->m_static->m_paramPreps[paramId];
  };

  return match<PrepKind>(
    rfunc.val,
    [&] (res::Func::FuncName s) {
      if (s.renamable) return PrepKind{TriBool::Maybe, TriBool::Maybe};
      auto const it = m_data->funcs.find(s.name);
      return it != end(m_data->funcs)
        ? func_param_prep(it->second, paramId)
        : PrepKind{TriBool::No, TriBool::Yes};
    },
    [&] (res::Func::MethodName s) {
      return PrepKind{TriBool::Maybe, TriBool::Maybe};
    },
    [&] (FuncInfo* finfo) {
      return func_param_prep(finfo->func, paramId);
    },
    [&] (const MethTabEntryPair* mte) {
      return func_param_prep(mte->second.func, paramId);
    },
    [&] (FuncFamily* fam) { return fromFuncFamily(fam); },
    [&] (res::Func::MethodOrMissing m) {
      return func_param_prep(m.mte->second.func, paramId);
    },
    [&] (const res::Func::Isect& i) {
      auto inOut = TriBool::Maybe;
      auto readonly = TriBool::Maybe;

      for (auto const ff : i.families) {
        auto const prepKind = fromFuncFamily(ff);
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

TriBool Index::lookup_return_readonly(
  Context,
  res::Func rfunc
) const {
  return match<TriBool>(
    rfunc.val,
    [&] (res::Func::FuncName s) {
      if (s.renamable) return TriBool::Maybe;
      auto const it = m_data->funcs.find(s.name);
      return it != end(m_data->funcs)
        ? yesOrNo(it->second->isReadonlyReturn)
        : TriBool::No; // if the function doesnt exist, we will error anyway
    },
    [&] (res::Func::MethodName s) { return TriBool::Maybe; },
    [&] (FuncInfo* finfo) {
      return yesOrNo(finfo->func->isReadonlyReturn);
    },
    [&] (const MethTabEntryPair* mte) {
      return yesOrNo(mte->second.func->isReadonlyReturn);
    },
    [&] (FuncFamily* fam) {
      return fam->m_static->m_isReadonlyReturn;
    },
    [&] (res::Func::MethodOrMissing m) {
      return yesOrNo(m.mte->second.func->isReadonlyReturn);
    },
    [&] (const res::Func::Isect& i) {
      auto readOnly = TriBool::Maybe;
      for (auto const ff : i.families) {
        if (ff->m_static->m_isReadonlyReturn == TriBool::Maybe) continue;
        assertx(
          IMPLIES(
            readOnly != TriBool::Maybe,
            readOnly == ff->m_static->m_isReadonlyReturn
          )
        );
        if (readOnly == TriBool::Maybe) {
          readOnly = ff->m_static->m_isReadonlyReturn;
        }
      }
      return readOnly;
    }
  );
}

TriBool Index::lookup_readonly_this(
  Context,
  res::Func rfunc
) const {
  return match<TriBool>(
    rfunc.val,
    [&] (res::Func::FuncName s) {
      if (s.renamable) return TriBool::Maybe;
      auto const it = m_data->funcs.find(s.name);
      return it != end(m_data->funcs)
        ? yesOrNo(it->second->isReadonlyThis)
        : TriBool::Yes; // if the function doesnt exist, we will error anyway
    },
    [&] (res::Func::MethodName s) { return TriBool::Maybe; },
    [&] (FuncInfo* finfo) {
      return yesOrNo(finfo->func->isReadonlyThis);
    },
    [&] (const MethTabEntryPair* mte) {
      return yesOrNo(mte->second.func->isReadonlyThis);
    },
    [&] (FuncFamily* fam) {
      return fam->m_static->m_isReadonlyThis;
    },
    [&] (res::Func::MethodOrMissing m) {
      return yesOrNo(m.mte->second.func->isReadonlyThis);
    },
    [&] (const res::Func::Isect& i) {
      auto readOnly = TriBool::Maybe;
      for (auto const ff : i.families) {
        if (ff->m_static->m_isReadonlyThis == TriBool::Maybe) continue;
        assertx(
          IMPLIES(
            readOnly != TriBool::Maybe,
            readOnly == ff->m_static->m_isReadonlyThis
          )
        );
        if (readOnly == TriBool::Maybe) {
          readOnly = ff->m_static->m_isReadonlyThis;
        }
      }
      return readOnly;
    }
  );
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
      auto const it = cinfo->publicStaticProps.find(prop.name);
      assertx(it != end(cinfo->publicStaticProps));
      return std::make_pair(
        remove_uninit(it->second.inferredType),
        it->second.everModified
      );
    }();
    state.emplace(
      prop.name,
      PropStateElem<>{
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
PropLookupResult<> Index::lookup_static(Context ctx,
                                        const PropertiesInfo& privateProps,
                                        const Type& cls,
                                        const Type& name) const {
  ITRACE(4, "lookup_static: {} {}::${}\n", show(ctx), show(cls), show(name));
  Trace::Indent _;

  using R = PropLookupResult<>;

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
    auto const rCtx = resolve_class(ctx.cls);
    if (rCtx.val.left()) return conservative();
    ctxCls = rCtx.val.right();
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

Type Index::lookup_public_prop(const Type& cls, const Type& name) const {
  if (!is_specialized_cls(cls)) return TCell;

  if (!is_specialized_string(name)) return TCell;
  auto const sname = sval_of(name);

  auto ty = TBottom;
  auto const resolved = visit_every_dcls_cls(
    dcls_of(cls),
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

void Index::join_iface_vtable_thread() const {
  if (m_data->compute_iface_vtables.joinable()) {
    m_data->compute_iface_vtables.join();
  }
}

Slot
Index::lookup_iface_vtable_slot(const php::Class* cls) const {
  return folly::get_default(m_data->ifaceSlotMap, cls, kInvalidSlot);
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
PropMergeResult<> Index::merge_static_type(
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

  using R = PropMergeResult<>;

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
    return PropMergeResult<>{
      loosen_likeness(val),
      TriBool::Maybe
    };
  };

  // check if we can determine the class.
  if (!is_specialized_cls(cls)) return unknownCls();

  const ClassInfo* ctxCls = nullptr;
  if (ctx.cls) {
    auto const rCtx = resolve_class(ctx.cls);
    // We should only be not able to resolve our own context if the
    // class is not instantiable. In that case, the merge can't
    // happen.
    if (rCtx.val.left()) return R{ TBottom, TriBool::No };
    ctxCls = rCtx.val.right();
  }

  auto const mergePublic = [&] (const ClassInfo* ci,
                                const php::Prop& prop,
                                const Type& val) {
    publicMutations.mergeKnown(ci, prop, val);
  };

  auto const& dcls = dcls_of(cls);
  auto const start = dcls.cls();

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
  if (!resolved) return unknownCls();
  assertx(result.has_value());
  ITRACE(4, "union -> {}\n", show(*result));
  return *result;
}

//////////////////////////////////////////////////////////////////////

DependencyContext Index::dependency_context(const Context& ctx) const {
  return dep_context(*m_data, ctx);
}

void Index::use_class_dependencies(bool f) {
  if (f != m_data->useClassDependencies) {
    m_data->dependencyMap.clear();
    m_data->useClassDependencies = f;
  }
}

void Index::init_public_static_prop_types() {
  trace_time tracer("init public static prop types");

  for (auto const& cinfo : m_data->allClassInfos) {
    for (auto const& prop : cinfo->cls->properties) {
      if (!(prop.attrs & (AttrPublic|AttrProtected)) ||
          !(prop.attrs & AttrStatic)) {
        continue;
      }

      /*
       * If the initializer type is TUninit, it means an 86sinit provides the
       * actual initialization type or it is AttrLateInit.  So we don't want to
       * include the Uninit (which isn't really a user-visible type for the
       * property) or by the time we union things in we'll have inferred nothing
       * much.
       */
      auto const initial = [&] {
        auto const tyRaw = from_cell(prop.val);
        if (tyRaw.subtypeOf(BUninit)) return TBottom;
        if (prop.attrs & AttrSystemInitialValue) return tyRaw;
        return adjust_type_for_prop(
          *this, *cinfo->cls, &prop.typeConstraint, tyRaw
        );
      }();

      cinfo->publicStaticProps[prop.name] =
        PublicSPropEntry {
          union_of(
            adjust_type_for_prop(
              *this,
              *cinfo->cls,
              &prop.typeConstraint,
              TInitCell
            ),
            initial
          ),
          initial,
          &prop,
          0,
          false,
          true
      };
    }
  }
}

void Index::refine_class_constants(
    const Context& ctx,
    const CompactVector<std::pair<size_t, Type>>& resolved,
    DependencyContextSet& deps) {
  if (!resolved.size()) return;

  auto changed = false;
  auto& constants = ctx.func->cls->constants;

  for (auto const& c : resolved) {
    assertx(c.first < constants.size());
    auto& cnst = constants[c.first];
    assertx(cnst.kind == ConstModifiers::Kind::Value);

    auto const key = std::make_pair(ctx.func->cls, cnst.name);

    auto& types = m_data->clsConstTypes;

    always_assert(cnst.val && type(*cnst.val) == KindOfUninit);
    if (auto const val = tv(c.second)) {
      assertx(val->m_type != KindOfUninit);
      cnst.val = *val;
      // Deleting from the types map is too expensive, so just leave
      // any entry. We won't look at it if val is set.
      changed = true;
    } else {
      auto const old = [&] {
        auto const it = types.find(key);
        return (it == types.end()) ? ClsConstInfo{ TInitCell, 0 } : it->second;
      }();

      if (c.second.strictlyMoreRefined(old.type)) {
        if (old.refinements < options.returnTypeRefineLimit) {
          types.insert_or_assign(
            key,
            ClsConstInfo{ c.second, old.refinements+1 }
          );
          changed = true;
        } else {
          FTRACE(
            1, "maxed out refinements for class constant {}::{}\n",
            ctx.func->cls->name, cnst.name
          );
        }
      } else {
        always_assert_flog(
          c.second.moreRefined(old.type),
          "Class constant type invariant violated for {}::{}\n"
          "    {} is not at least as refined as {}\n",
          ctx.func->cls->name,
          cnst.name,
          show(c.second),
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
  if (func->cls != nullptr) return;

  auto const val = tv(fa.inferredReturn);
  if (!val) return;

  auto const cns_name = Constant::nameFromFuncName(func->name);
  if (!cns_name) return;

  auto& cs = fa.ctx.unit->constants;
  auto it = std::find_if(
    cs.begin(),
    cs.end(),
    [&] (auto const& c) {
      return cns_name->same(c->name);
    });
  assertx(it != cs.end() && "Did not find constant");
  (*it)->val = val.value();
  find_deps(*m_data, func, Dep::ConstVal, deps);
}

void Index::fixup_return_type(const php::Func* func,
                              Type& retTy) const {
  if (func->isGenerator) {
    if (func->isAsync) {
      // Async generators always return AsyncGenerator object.
      retTy = objExact(builtin_class(s_AsyncGenerator.get()));
    } else {
      // Non-async generators always return Generator object.
      retTy = objExact(builtin_class(s_Generator.get()));
    }
  } else if (func->isAsync) {
    // Async functions always return WaitH<T>, where T is the type returned
    // internally.
    retTy = wait_handle(*this, std::move(retTy));
  }
}

void Index::init_return_type(const php::Func* func) {
  if ((func->attrs & AttrBuiltin) || func->isMemoizeWrapper) {
    return;
  }

  auto make_type = [&] (const TypeConstraint& tc) {
    if (tc.isSoft() ||
        (RuntimeOption::EvalEnforceGenericsUB < 2 && tc.isUpperBound())) {
      return TBottom;
    }
    auto const cls = func->cls && func->cls->closureContextCls
       ? m_data->classes.at(func->cls->closureContextCls)
       : func->cls;
    return lookup_constraint(
      Context { m_data->units.at(func->unit), func, cls },
      tc
    );
  };

  auto const finfo = create_func_info(*m_data, func);

  auto tcT = make_type(func->retTypeConstraint);
  if (tcT.is(BBottom)) return;

  if (func->hasInOutArgs) {
    std::vector<Type> types;
    types.emplace_back(intersection_of(TInitCell, std::move(tcT)));
    for (auto& p : func->params) {
      if (!p.inout) continue;
      auto t = make_type(p.typeConstraint);
      if (t.is(BBottom)) return;
      types.emplace_back(intersection_of(TInitCell, std::move(t)));
    }
    tcT = vec(std::move(types));
  }

  tcT = loosen_all(to_cell(std::move(tcT)));

  FTRACE(4, "Pre-fixup return type for {}{}{}: {}\n",
         func->cls ? func->cls->name->data() : "",
         func->cls ? "::" : "",
         func->name, show(tcT));
  fixup_return_type(func, tcT);
  FTRACE(3, "Initial return type for {}{}{}: {}\n",
         func->cls ? func->cls->name->data() : "",
         func->cls ? "::" : "",
         func->name, show(tcT));
  finfo->returnTy = std::move(tcT);
}

void Index::refine_return_info(const FuncAnalysisResult& fa,
                               DependencyContextSet& deps) {
  auto const& func = fa.ctx.func;
  auto const finfo = create_func_info(*m_data, func);

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
      dep = is_scalar(fa.inferredReturn) ?
        Dep::ReturnTy | Dep::InlineDepthLimit : Dep::ReturnTy;
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
    "Index effectFree changed from true to false in {} {}{}.\n",
    func->unit,
    func->cls
      ? folly::to<std::string>(func->cls->name->data(), "::")
      : std::string{},
    func->name);

  if (finfo->effectFree != fa.effectFree) {
    finfo->effectFree = fa.effectFree;
    dep = Dep::InlineDepthLimit | Dep::ReturnTy;
  }

  if (dep != Dep{}) {
    find_deps(*m_data, func, dep, deps);
    if (resetFuncFamilies) {
      assertx(has_dep(dep, Dep::ReturnTy));
      for (auto const ff : finfo->families) {
        // Only load the deps for this func family if we're the ones
        // who successfully reset. Only one thread needs to do it.
        if (!ff->m_returnTy.reset()) continue;
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

void Index::update_static_prop_init_val(const php::Class* cls,
                                        SString name) const {
  auto const cls_it = m_data->classInfo.find(cls->name);
  if (cls_it == end(m_data->classInfo)) {
    return;
  }
  auto const cinfo = cls_it->second;
  if (cinfo->cls != cls) {
    return;
  }
  auto const it = cinfo->publicStaticProps.find(name);
  if (it != cinfo->publicStaticProps.end()) {
    it->second.initialValueResolved = true;
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
  for (auto const& cinfo : m_data->allClassInfos) {
    for (auto& kv : cinfo->publicStaticProps) {
      auto knownClsType = [&] {
        auto const it = known.find(
          PublicSPropMutations::KnownKey { cinfo.get(), kv.first }
        );
        // If we didn't see a mutation, the type is TBottom.
        return it == end(known) ? TBottom : it->second;
      }();

      auto unknownClsType = [&] {
        auto const it = unknown.find(kv.first);
        // If we didn't see a mutation, the type is TBottom.
        return it == end(unknown) ? TBottom : it->second;
      }();

      // We can't keep context dependent types in public properties.
      auto newType = adjust_type_for_prop(
        *this,
        *cinfo->cls,
        &kv.second.prop->typeConstraint,
        unctx(union_of(std::move(knownClsType), std::move(unknownClsType)))
      );

      if (!newType.is(BBottom)) {
        always_assert_flog(
          kv.second.everModified,
          "Static property index invariant violated on {}::{}:\n"
          " everModified flag went from false to true",
          cinfo->cls->name->data(),
          kv.first->data()
        );
      } else {
        kv.second.everModified = false;
      }

      if (kv.second.initialValueResolved) {
        for (auto& prop : cinfo->cls->properties) {
          if (prop.name != kv.first) continue;
          kv.second.initializerType = from_cell(prop.val);
          kv.second.initialValueResolved = false;
          break;
        }
        assertx(!kv.second.initialValueResolved);
      }

      // The type from the indexer doesn't contain the in-class initializer
      // types. Add that here.
      auto effectiveType =
        union_of(std::move(newType), kv.second.initializerType);

      /*
       * We may only shrink the types we recorded for each property. (If a
       * property type ever grows, the interpreter could infer something
       * incorrect at some step.)
       */
      always_assert_flog(
        effectiveType.subtypeOf(kv.second.inferredType),
        "Static property index invariant violated on {}::{}:\n"
        "  {} is not a subtype of {}",
        cinfo->cls->name->data(),
        kv.first->data(),
        show(effectiveType),
        show(kv.second.inferredType)
      );

      // Put a limit on the refinements to ensure termination. Since we only
      // ever refine types, we can stop at any point and still maintain
      // correctness.
      if (effectiveType.strictSubtypeOf(kv.second.inferredType)) {
        if (kv.second.refinements + 1 < options.publicSPropRefineLimit) {
          find_deps(*m_data, kv.second.prop, Dep::PublicSProp, deps);
          kv.second.inferredType = std::move(effectiveType);
          ++kv.second.refinements;
        } else {
          FTRACE(
            1, "maxed out public static property refinements for {}:{}\n",
            cinfo->cls->name->data(),
            kv.first->data()
          );
        }
      }
    }
  }
}

void Index::refine_bad_initial_prop_values(const php::Class* cls,
                                           bool value,
                                           DependencyContextSet& deps) {
  assertx(!is_used_trait(*cls));
  auto const it = m_data->classInfo.find(cls->name);
  if (it == end(m_data->classInfo)) {
    return;
  }
  auto const cinfo = it->second;
  if (cinfo->cls != cls) {
    return;
  }
  always_assert_flog(
    cinfo->hasBadInitialPropValues || !value,
    "Bad initial prop values going from false to true on {}",
    cls->name->data()
  );

  if (cinfo->hasBadInitialPropValues && !value) {
    cinfo->hasBadInitialPropValues = false;
    find_deps(*m_data, cls, Dep::PropBadInitialValues, deps);
  }
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
    (x).clear();                                \
  }

void Index::cleanup_pre_analysis() {
  trace_time _{"cleanup pre analysis"};
  CLEAR(m_data->methods);
}

void Index::cleanup_for_final() {
  trace_time _{"cleanup for final"};
  CLEAR(m_data->dependencyMap);
}

void Index::cleanup_post_emit() {
  trace_time _{"cleanup post emit"};
  {
    trace_time t{"reset allClassInfos"};
    parallel::for_each(m_data->allClassInfos, [] (auto& u) { u.reset(); });
  }
  {
    trace_time t{"reset funcInfo"};
    parallel::for_each(
      m_data->funcInfo,
      [] (auto& u) {
        u.returnTy = TBottom;
        u.families.clear();
      }
    );
  }
  {
    trace_time t{"reset program"};
    parallel::for_each(m_data->program->units, [] (auto& u) { u.reset(); });
    parallel::for_each(m_data->program->classes, [] (auto& u) { u.reset(); });
    parallel::for_each(m_data->program->funcs, [] (auto& f) { f.reset(); });
    m_data->program.reset();
  }
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

  CLEAR_PARALLEL(m_data->allClassInfos);
  CLEAR_PARALLEL(m_data->classInfo);
  CLEAR_PARALLEL(m_data->funcInfo);

  CLEAR_PARALLEL(m_data->privatePropInfo);
  CLEAR_PARALLEL(m_data->privateStaticPropInfo);
  CLEAR_PARALLEL(m_data->publicSPropMutations);
  CLEAR_PARALLEL(m_data->ifaceSlotMap);
  CLEAR_PARALLEL(m_data->closureUseVars);

  CLEAR_PARALLEL(m_data->methodFamilies);
  CLEAR_PARALLEL(m_data->singleMethodFamilies);

  CLEAR_PARALLEL(m_data->funcFamilies);
  CLEAR_PARALLEL(m_data->funcFamilyStaticInfos);

  CLEAR_PARALLEL(m_data->clsConstTypes);
  CLEAR_PARALLEL(m_data->clsConstLookupCache);

  CLEAR_PARALLEL(m_data->foldableReturnTypeMap);
  CLEAR_PARALLEL(m_data->contextualReturnTypes);

  parallel::for_each(clearers, [] (const std::function<void()>& f) { f(); });
}

void Index::thaw() {
  m_data->frozen = false;
}

//////////////////////////////////////////////////////////////////////

res::Func Index::do_resolve(const php::Func* f) const {
  auto const finfo = create_func_info(*m_data, f);
  return res::Func { finfo };
};

// Return true if we know for sure that one php::Class must derive
// from another at runtime, in all possible instantiations.
bool Index::must_be_derived_from(const php::Class* cls,
                                 const php::Class* parent) const {
  if (cls == parent) return true;
  auto const clsIt = m_data->classInfo.find(cls->name);
  auto const parentIt = m_data->classInfo.find(parent->name);
  if (clsIt == end(m_data->classInfo) || parentIt == end(m_data->classInfo)) {
    return false;
  }
  return clsIt->second->derivedFrom(*parentIt->second);
}

// Return true if any possible definition of one php::Class could
// derive from another at runtime, or vice versa.
bool Index::could_be_related(const php::Class* cls,
                             const php::Class* parent) const {
  if (cls == parent) return true;
  auto const clsIt = m_data->classInfo.find(cls->name);
  auto const parentIt = m_data->classInfo.find(parent->name);
  if (clsIt == end(m_data->classInfo) || parentIt == end(m_data->classInfo)) {
    return true;
  }
  return
    clsIt->second->derivedFrom(*parentIt->second) ||
    parentIt->second->derivedFrom(*clsIt->second);
}

//////////////////////////////////////////////////////////////////////

PublicSPropMutations::Data& PublicSPropMutations::get() {
  if (!m_data) m_data = std::make_unique<Data>();
  return *m_data;
}

void PublicSPropMutations::mergeKnown(const ClassInfo* ci,
                                      const php::Prop& prop,
                                      const Type& val) {
  ITRACE(4, "PublicSPropMutations::mergeKnown: {} {} {}\n",
         ci->cls->name->data(), prop.name, show(val));

  auto const res = get().m_known.emplace(
    KnownKey { const_cast<ClassInfo*>(ci), prop.name }, val
  );
  if (!res.second) res.first->second |= val;
}

void PublicSPropMutations::mergeUnknownClass(SString prop, const Type& val) {
  ITRACE(4, "PublicSPropMutations::mergeUnknownClass: {} {}\n",
         prop, show(val));

  auto const res = get().m_unknown.emplace(prop, val);
  if (!res.second) res.first->second |= val;
}

void PublicSPropMutations::mergeUnknown(Context ctx) {
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
