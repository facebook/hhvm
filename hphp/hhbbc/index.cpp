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
#include <folly/Optional.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <folly/concurrency/ConcurrentHashMap.h>

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-comparisons.h"

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
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/wide-func.h"

#include "hphp/util/algorithm.h"
#include "hphp/util/assertions.h"
#include "hphp/util/hash-set.h"
#include "hphp/util/lock-free-lazy.h"
#include "hphp/util/match.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc_index);

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
static_assert(CheckSize<php::Param, use_lowptr ? 64 : 88>(), "");
static_assert(CheckSize<php::Func, use_lowptr ? 176 : 208>(), "");

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
template<class T> using ISStringToMany =
  std::unordered_multimap<
    SString,
    T*,
    string_data_hash,
    string_data_isame
  >;

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
 */
template<class T> using ISStringToOneT =
  hphp_hash_map<
    SString,
    T,
    string_data_hash,
    string_data_isame
  >;

template<class T> using SStringToOneT =
  hphp_hash_map<
    SString,
    T,
    string_data_hash,
    string_data_same
  >;

/*
 * One-to-one case sensitive map, where the keys are static strings
 * and the values are some T.
 *
 * Elements are not stable under insert/erase.
 */
template<class T> using SStringToOneFastT =
  hphp_fast_map<
    SString,
    T,
    string_data_hash,
    string_data_same
  >;

template<class T> using SStringToOneFastT =
  hphp_fast_map<
    SString,
    T,
    string_data_hash,
    string_data_same
  >;

/*
 * One-to-one case insensitive map, where the keys are static strings
 * and the values are some kind of pointer.
 */
template<class T> using ISStringToOne = ISStringToOneT<T*>;

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
    std::map<DependencyContext,Dep,DependencyContextLess>,
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
      SStringToOneT<MethTabEntry>::value_type {};

namespace {

using MethTabEntryPair = res::Func::MethTabEntryPair;

inline MethTabEntryPair* mteFromElm(
  SStringToOneT<MethTabEntry>::value_type& elm) {
  return static_cast<MethTabEntryPair*>(&elm);
}

inline const MethTabEntryPair* mteFromElm(
  const SStringToOneT<MethTabEntry>::value_type& elm) {
  return static_cast<const MethTabEntryPair*>(&elm);
}

inline MethTabEntryPair* mteFromIt(SStringToOneT<MethTabEntry>::iterator it) {
  return static_cast<MethTabEntryPair*>(&*it);
}

struct CallContextHashCompare {
  bool equal(const CallContext& a, const CallContext& b) const {
    return a == b;
  }

  size_t hash(const CallContext& c) const {
    auto ret = folly::hash::hash_combine(
      c.callee,
      c.args.size(),
      c.context.hash()
    );
    for (auto& t : c.args) {
      ret = folly::hash::hash_combine(ret, t.hash());
    }
    return ret;
  }
};

using ContextRetTyMap = tbb::concurrent_hash_map<
  CallContext,
  Type,
  CallContextHashCompare
>;

//////////////////////////////////////////////////////////////////////

template<class Filter>
PropState make_unknown_propstate(const php::Class* cls,
                                 Filter filter) {
  auto ret = PropState{};
  for (auto& prop : cls->properties) {
    if (filter(prop)) {
      auto& elem = ret[prop.name];
      elem.ty = TCell;
      elem.tc = &prop.typeConstraint;
      elem.attrs = prop.attrs;
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

  PFuncVec  m_v;
  LockFreeLazy<Type> m_returnTy;
  folly::Optional<uint32_t> m_numInOut;
};

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

}

//////////////////////////////////////////////////////////////////////

/* Known information about a particular possible instantiation of a
 * PHP record. The php::Record will be marked AttrUnique if there is a unique
 * RecordInfo with a given name.
 */
struct RecordInfo {
  const php::Record* rec = nullptr;
  const RecordInfo* parent = nullptr;
  /*
   * A vector of RecordInfo that encodes the inheritance hierarchy.
   */
  CompactVector<RecordInfo*> baseList;
  const php::Record* phpType() const { return rec; }
};

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
    php::Const operator*() const {
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
   * across the inheritance hierarchy.
   */
  hphp_fast_map<SString, ConstIndex> clsConstants;

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
  SStringToOneT<MethTabEntry> methods;

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
  SStringToOneFastT<FuncFamily*> methodFamilies;
  // Resolutions to single entries do not require a FuncFamily (this
  // saves space).
  SStringToOneFastT<const MethTabEntryPair*> singleMethodFamilies;

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
   * Note, unlike baseList, the order of the elements in this vector
   * is unspecified.
   */
  CompactVector<ClassInfo*> subclassList;

  /*
   * A vector of ClassInfo that encodes the inheritance hierarchy,
   * unless this ClassInfo represents an interface.
   *
   * This is the list of base classes for this class in inheritance
   * order.
   */
  CompactVector<ClassInfo*> baseList;

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
  hphp_hash_map<SString,PublicSPropEntry> publicStaticProps;

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
  bool isDerivedMocked{false};

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
  bool derivedHasConstProp{false};

  const php::Class* phpType() const { return cls; }

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
//////////////////////////////////////////////////////////////////////

namespace res {
Record::Record(Either<SString, RecordInfo*> val) : val(val) {}

bool Record::same(const Record& o) const {
  return val == o.val;
}

bool Record::couldBe(const Record& o) const {
  // If either types are not unique return true
  if (val.left() || o.val.left()) return true;

  auto r1 = val.right();
  auto r2 = o.val.right();
  assertx(r1 && r2);
  // Both types are unique records so they "could be" if they are in an
  // inheritance relationship
  if (r1->baseList.size() >= r2->baseList.size()) {
    return r1->baseList[r2->baseList.size() - 1] == r2;
  } else {
    return r2->baseList[r1->baseList.size() - 1] == r1;
  }
}

SString Record::name() const {
  return val.match(
    [] (SString s) { return s; },
    [] (RecordInfo* ri) { return ri->rec->name.get(); }
  );
}

template <bool returnTrueOnMaybe>
bool Record::subtypeOfImpl(const Record& o) const {
  auto s1 = val.left();
  auto s2 = o.val.left();
  if (s1 || s2) return returnTrueOnMaybe || s1 == s2;
  auto r1 = val.right();
  auto r2 = o.val.right();
  assertx(r1 && r2);
  if (r1->baseList.size() >= r2->baseList.size()) {
    return r1->baseList[r2->baseList.size() - 1] == r2;
  }
  return false;
}

bool Record::mustBeSubtypeOf(const Record& o) const {
  return subtypeOfImpl<false>(o);
}

bool Record::maybeSubtypeOf(const Record& o) const {
  return subtypeOfImpl<true>(o);
}

bool Record::couldBeOverriden() const {
  return val.match(
    [] (SString) { return true; },
    [] (RecordInfo* rinfo) {
      return !(rinfo->rec->attrs & AttrFinal);
    }
  );
}

std::string show(const Record& r) {
  return r.val.match(
    [] (SString s) -> std::string {
      return s->data();
    },
    [] (RecordInfo* rinfo) {
      return folly::sformat("{}*", rinfo->rec->name);
    }
  );
}

folly::Optional<Record> Record::commonAncestor(const Record& r) const {
  if (val.left() || r.val.left()) return folly::none;
  auto const c1 = val.right();
  auto const c2 = r.val.right();
  // Walk the arrays of base classes until they match. For common ancestors
  // to exist they must be on both sides of the baseList at the same positions
  RecordInfo* ancestor = nullptr;
  auto it1 = c1->baseList.begin();
  auto it2 = c2->baseList.begin();
  while (it1 != c1->baseList.end() && it2 != c2->baseList.end()) {
    if (*it1 != *it2) break;
    ancestor = *it1;
    ++it1; ++it2;
  }
  if (ancestor == nullptr) {
    return folly::none;
  }
  return res::Record { ancestor };
}

Class::Class(Either<SString,ClassInfo*> val) : val(val) {}

// Class type operations here are very conservative for now.

bool Class::same(const Class& o) const {
  return val == o.val;
}

template <bool returnTrueOnMaybe>
bool Class::subtypeOfImpl(const Class& o) const {
  auto s1 = val.left();
  auto s2 = o.val.left();
  if (s1 || s2) return returnTrueOnMaybe || s1 == s2;
  auto c1 = val.right();
  auto c2 = o.val.right();
  return c1->derivedFrom(*c2);
}

bool Class::mustBeSubtypeOf(const Class& o) const {
  return subtypeOfImpl<false>(o);
}

bool Class::maybeSubtypeOf(const Class& o) const {
  return subtypeOfImpl<true>(o);
}

bool Class::couldBe(const Class& o) const {
  if (same(o)) return true;

  // If either types are not unique return true
  if (val.left() || o.val.left()) return true;

  auto c1 = val.right();
  auto c2 = o.val.right();
  // if one or the other is an interface return true for now.
  // TODO(#3621433): better interface stuff
  if (c1->cls->attrs & AttrInterface || c2->cls->attrs & AttrInterface) {
    return true;
  }

  // Both types are unique classes so they "could be" if they are in an
  // inheritance relationship
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

bool Class::couldBeInterface() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return cinfo->cls->attrs & AttrInterface;
    }
  );
}

bool Class::mustBeInterface() const {
  return val.match(
    [] (SString) { return false; },
    [] (ClassInfo* cinfo) {
      return cinfo->cls->attrs & AttrInterface;
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

bool Class::couldHaveMockedDerivedClass() const {
  return val.match(
    [] (SString) { return true;},
    [] (ClassInfo* cinfo) {
      return cinfo->isDerivedMocked;
    }
  );
}

bool Class::couldBeMocked() const {
  return val.match(
    [] (SString) { return true;},
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

bool Class::derivedCouldHaveConstProp() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) { return cinfo->derivedHasConstProp; }
  );
}

folly::Optional<Class> Class::commonAncestor(const Class& o) const {
  if (val.left() || o.val.left()) return folly::none;
  auto const c1 = val.right();
  auto const c2 = o.val.right();
  if (c1 == c2) return res::Class { c1 };
  // Walk the arrays of base classes until they match. For common ancestors
  // to exist they must be on both sides of the baseList at the same positions
  ClassInfo* ancestor = nullptr;
  auto it1 = c1->baseList.begin();
  auto it2 = c2->baseList.begin();
  while (it1 != c1->baseList.end() && it2 != c2->baseList.end()) {
    if (*it1 != *it2) break;
    ancestor = *it1;
    ++it1; ++it2;
  }
  if (ancestor == nullptr) {
    return folly::none;
  }
  return res::Class { ancestor };
}

folly::Optional<res::Class> Class::parent() const {
  if (!val.right()) return folly::none;
  auto parent = val.right()->parent;
  if (!parent) return folly::none;
  return res::Class { parent };
}

const php::Class* Class::cls() const {
  return val.right() ? val.right()->cls : nullptr;
}

std::string show(const Class& c) {
  return c.val.match(
    [] (SString s) -> std::string {
      return s->data();
    },
    [] (ClassInfo* cinfo) {
      return folly::sformat("{}*", cinfo->cls->name);
    }
  );
}

Func::Func(const Index* idx, Rep val)
  : index(idx)
  , val(val)
{}

SString Func::name() const {
  return match<SString>(
    val,
    [&] (FuncName s)   { return s.name; },
    [&] (MethodName s) { return s.name; },
    [&] (FuncInfo* fi) { return fi->func->name; },
    [&] (const MethTabEntryPair* mte) { return mte->first; },
    [&] (FuncFamily* fa) -> SString {
      auto const name = fa->possibleFuncs().front()->first;
      if (debug) {
        for (DEBUG_ONLY auto const f : fa->possibleFuncs()) {
          assertx(f->first->isame(name));
        }
      }
      return name;
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
    [&](FuncFamily* /*fa*/)          { return Ret{}; }
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
    [&](FuncFamily* fa) { return false; }
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
    [&](FuncFamily* fa) {
      for (auto const pf : fa->possibleFuncs()) {
        if (pf->second.func->isReified) return true;
      }
      return false;
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
    [&](FuncFamily* fa) {
      for (auto const pf : fa->possibleFuncs()) {
        if (dyn_call_error_level(pf->second.func) > 0)
          return true;
      }
      return false;
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
    [&](FuncFamily* fa) {
      for (auto const pf : fa->possibleFuncs()) {
        if (pf->second.func->attrs & AttrBuiltin) return true;
      }
      return false;
    }
  );
}

namespace {

uint32_t numNVArgs(const php::Func& f) {
  uint32_t cnt = f.params.size();
  return cnt && f.params[cnt - 1].isVariadic ? cnt - 1 : cnt;
}

}

uint32_t Func::minNonVariadicParams() const {
  return match<uint32_t>(
    val,
    [&] (FuncName) { return 0; },
    [&] (MethodName) { return 0; },
    [&] (FuncInfo* fi) { return numNVArgs(*fi->func); },
    [&] (const MethTabEntryPair* mte) { return numNVArgs(*mte->second.func); },
    [&] (FuncFamily* fa) {
      auto c = std::numeric_limits<uint32_t>::max();
      for (auto const pf : fa->possibleFuncs()) {
        c = std::min(c, numNVArgs(*pf->second.func));
      }
      return c;
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
    [&] (FuncFamily* fa) {
      uint32_t c = 0;
      for (auto const pf : fa->possibleFuncs()) {
        c = std::max(c, numNVArgs(*pf->second.func));
      }
      return c;
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
    [&](FuncFamily*)             { ret += "+"; }
  );
  return ret;
}

}

//////////////////////////////////////////////////////////////////////

using IfaceSlotMap = hphp_hash_map<const php::Class*, Slot>;
using ConstInfoConcurrentMap =
  tbb::concurrent_hash_map<SString, ConstInfo, StringDataHashCompare>;

template <typename T>
struct ResTypeHelper;

template <>
struct ResTypeHelper<res::Class> {
  using InfoT = ClassInfo;
  using InfoMapT = ISStringToOneT<InfoT*>;
  using OtherT = res::Record;
  static std::string name() { return "class"; }
};

template <>
struct ResTypeHelper<res::Record> {
  using InfoT = RecordInfo;
  using InfoMapT = ISStringToOneT<InfoT*>;
  using OtherT = res::Class;
  static std::string name() { return "record"; }
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

  std::unique_ptr<ArrayTypeTable::Builder> arrTableBuilder;

  ISStringToOneT<const php::Class*>      classes;
  SStringToMany<const php::Func>         methods;
  SStringToOneFastT<uint64_t>            method_inout_params_by_name;
  ISStringToOneT<const php::Func*>       funcs;
  ISStringToOneT<const php::TypeAlias*>  typeAliases;
  ISStringToOneT<const php::Class*>      enums;
  SStringToOneT<const php::Constant*>    constants;
  ISStringToOneT<const php::Record*>     records;

  // Map from each class to all the closures that are allocated in
  // functions of that class.
  hphp_hash_map<
    const php::Class*,
    CompactVector<const php::Class*>
  > classClosureMap;

  hphp_hash_map<
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

  /*
   * Map from each record name to RecordInfo objects if one exists.
   *
   * It may not exists if we would fatal when defining the record.
   */
  ISStringToOneT<RecordInfo*> recordInfo;

  /*
   * All the RecordInfos, sorted topologically (ie all the parents of
   * RecordInfo at index K will have indices less than K).
   * This mostly drops out of the way RecordInfos are created;
   * it would be hard to create the RecordInfos for the
   * php::Record X (or even know how many to create) without knowing
   * all the RecordInfos that were created for X's dependencies.
   */
  std::vector<std::unique_ptr<RecordInfo>> allRecordInfos;

  std::vector<FuncInfo> funcInfo;

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
  folly_concurrent_hash_map_simd<
    std::unique_ptr<FuncFamily>,
    bool,
    FuncFamilyPtrHasher,
    FuncFamilyPtrEquals
  > funcFamilies;

  /*
   * Map from interfaces to their assigned vtable slots, computed in
   * compute_iface_vtables().
   */
  IfaceSlotMap ifaceSlotMap;

  hphp_hash_map<
    const php::Class*,
    CompactVector<Type>
  > closureUseVars;

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

  template<typename T>
  const typename ResTypeHelper<T>::InfoMapT& infoMap() const;
};

template<>
const typename ResTypeHelper<res::Class>::InfoMapT&
Index::IndexData::infoMap<res::Class>() const {
  return classInfo;
}
template<>
const typename ResTypeHelper<res::Record>::InfoMapT&
Index::IndexData::infoMap<res::Record>() const {
  return recordInfo;
}

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
  auto const cls = ctx.cls->closureContextCls ?
    ctx.cls->closureContextCls : ctx.cls;
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
}

std::mutex func_info_mutex;

FuncInfo* create_func_info(IndexData& data, const php::Func* f) {
  auto fi = &data.funcInfo[f->idx];
  if (UNLIKELY(fi->func == nullptr)) {
    if (f->nativeInfo) {
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

  {
    DepMap::const_accessor acc;
    if (data.dependencyMap.find(acc, srcDep)) {
      for (auto const& kv : acc->second) {
        if (has_dep(kv.second, mask)) deps.insert(kv.first);
      }
    }
  }

  // If this is a Func dep, we need to also check if any FuncFamily
  // dependencies need to be added.
  if (srcDep.tag() != DependencyContextType::Func) return;

  auto const fi = func_info(data, static_cast<const php::Func*>(srcDep.ptr()));
  if (!fi->func) return;

  // Add any associated FuncFamilies
  for (auto const ff : fi->families) {
    DepMap::const_accessor acc;
    if (data.dependencyMap.find(acc, make_dep(ff))) {
      for (auto const& kv : acc->second) {
        if (has_dep(kv.second, mask)) deps.insert(kv.first);
      }
    }
  }
}

struct TraitMethod {
  using class_type = const ClassInfo*;
  using method_type = const php::Func*;

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
                                   const std::list<TraitMethod>&) {
    auto const& m = cls->cls->methods;
    if (std::find_if(m.begin(), m.end(),
                     [&] (auto const& f) {
                       return f->name->isame(methName);
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

template<typename T>
struct PreResolveUpdates {
  TinyVector<std::unique_ptr<T>> newInfos;
  TinyVector<T*> updateDeps;

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

  hphp_fast_set<ClassInfo::ConstIndex, CnsHash, CnsEquals> removeNoOverride;

  hphp_hash_map<
    const php::Class*,
    hphp_fast_set<const php::Func*>
  > extraMethods;
  hphp_hash_map<
    const php::Class*,
    CompactVector<const php::Class*>
  > closures;
  CompactVector<const php::Class*> newClosures;
  CompactVector<
    std::tuple<std::unique_ptr<php::Class>, php::Unit*, uint32_t>
  > newClasses;

  struct UnitPtrHashCompare {
    bool equal(const php::Unit* u1, const php::Unit* u2) const {
      return u1 == u2;
    }
    size_t hash(const php::Unit* u) const {
      return pointer_hash<const php::Unit>{}(u);
    }
  };

  using UnitNumClasses =
    tbb::concurrent_hash_map<const php::Unit*, uint32_t, UnitPtrHashCompare>;
  UnitNumClasses* numClasses = nullptr;

  uint32_t nextClass(const php::Unit& unit) {
    typename UnitNumClasses::accessor acc;
    numClasses->insert(
      acc,
      std::make_pair(&unit, unit.classes.size())
    );
    return acc->second++;
  }
};

using RecPreResolveUpdates = PreResolveUpdates<RecordInfo>;
using ClsPreResolveUpdates = PreResolveUpdates<ClassInfo>;

/*
 * Make a flattened table of the constants on this class.
 */
bool build_class_constants(ClassInfo* cinfo, ClsPreResolveUpdates& updates) {
  auto const removeNoOverride = [&] (ClassInfo::ConstIndex cns) {
    // During hhbbc/parse, all constants are pre-set to NoOverride
    ITRACE(2, "Removing NoOverride on {}::{}\n", cns->cls->name, cns->name);
    if (cns->isNoOverride) updates.removeNoOverride.emplace(cns);
  };

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

    // if the existing constant in the map is concrete, then don't overwrite it with an incoming
    // abstract constant's default
    if (!existing->isAbstract && cns->isAbstract) {
      return true;
    }

    if (existing->val) {
      // A constant from a declared interface collides with a constant
      // (Excluding constants from interfaces a trait implements)
      // Need this check otherwise constants from traits that conflict with
      // declared interfaces will silently lose and not conflict in the runtime
      // Type and Context constants can be overriden.
      if (cns->kind != ConstModifiers::Kind::Type &&
          cns->kind != ConstModifiers::Kind::Context &&
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

      // Constants from traits silently lose
      if (fromTrait) {
        removeNoOverride(cns);
        return true;
      }

      if ((cns->cls->attrs & (AttrInterface)) && existing->isAbstract) {
        // because existing has val, this covers the case where it is abstract with default
        // allow incoming to win
      } else {
        // A constant from an interface or from an included enum collides
        // with an existing constant.
        if (cns->cls->attrs & (AttrInterface | AttrEnum | AttrEnumClass)) {
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

    removeNoOverride(existing);
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

  for (uint32_t idx = 0; idx < cinfo->cls->constants.size(); ++idx) {
    auto const cns = ClassInfo::ConstIndex { cinfo->cls, idx };
    if (cinfo->cls->attrs & AttrTrait) removeNoOverride(cns);
    if (!add(cns, false)) return false;
  }

  for (auto const trait : cinfo->usedTraits) {
    for (auto const& cns : trait->clsConstants) {
      if (!add(cns.second, true)) return false;
    }
  }

  for (auto const ienum : cinfo->includedEnums) {
    for (auto const& cns : ienum->clsConstants) {
      if (!add(cns.second, true)) return false;
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
    if (it->first != name) {
      auto mte = it->second;
      cinfo->methods.erase(it);
      it = cinfo->methods.emplace(name, mte).first;
    }
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

    for (auto const& precRule : cinfo->cls->traitPrecRules) {
      tmid.applyPrecRule(precRule, cinfo);
    }
    for (auto const& aliasRule : cinfo->cls->traitAliasRules) {
      tmid.applyAliasRule(aliasRule, cinfo);
    }
    auto traitMethods = tmid.finish(cinfo);
    // Import the methods.
    for (auto const& mdata : traitMethods) {
      auto const method = mdata.tm.method;
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
      updates.extraMethods[cinfo->cls].emplace(method);
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
bool build_cls_info(const IndexData& index,
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

  if (!build_class_constants(cinfo, updates)) return false;
  if (!build_class_impl_interfaces(cinfo)) return false;
  if (!build_class_properties(cinfo)) return false;
  if (!build_class_methods(index, cinfo, updates)) return false;
  return true;
}

//////////////////////////////////////////////////////////////////////

void add_system_constants_to_index(IndexData& index) {
  for (auto cnsPair : Native::getConstants()) {
    assertx(cnsPair.second.m_type != KindOfUninit ||
            cnsPair.second.dynamic());
    auto pc = new php::Constant { nullptr, cnsPair.first, cnsPair.second, AttrUnique | AttrPersistent };
    add_symbol(index.constants, pc, "constant");
  }
}

//////////////////////////////////////////////////////////////////////

folly::Optional<uint32_t> func_num_inout(const php::Func* func) {
  if (!func->hasInOutArgs) return 0;
  uint32_t count = 0;
  for (auto& p : func->params) count += p.inout;
  return count;
}

template<typename PossibleFuncRange>
folly::Optional<uint32_t> num_inout_from_set(PossibleFuncRange range) {
  if (begin(range) == end(range)) return 0;

  struct FuncFind {
    using F = const php::Func*;
    static F get(std::pair<SString,F> p) { return p.second; }
    static F get(const MethTabEntryPair* mte) { return mte->second.func; }
  };

  folly::Optional<uint32_t> num;
  for (auto const& item : range) {
    auto const n = func_num_inout(FuncFind::get(item));
    if (!n.hasValue()) return folly::none;
    if (num.hasValue() && n != num) return folly::none;
    num = n;
  }
  return num;
}

//////////////////////////////////////////////////////////////////////

template<class T>
struct PhpTypeHelper;

template<>
struct PhpTypeHelper<php::Class> {
  template<class Fn>
  static void process_bases(const php::Class* cls, Fn&& fn) {
    if (cls->parentName) fn(cls->parentName);
    for (auto& i : cls->interfaceNames) fn(i);
    for (auto& t : cls->usedTraitNames) fn(t);
    for (auto& t : cls->includedEnumNames) fn(t);
  }

  static std::string name() { return "class"; }

  static void assert_bases(const IndexData&, const php::Class* cls);
  static void try_flatten_traits(const php::Program*, const IndexData&,
                                 const php::Class*, ClassInfo*,
                                 ClsPreResolveUpdates&);

  using Info = ClassInfo;
};

template<>
struct PhpTypeHelper<php::Record> {
  template<class Fn>
  static void process_bases(const php::Record* rec, Fn&& fn) {
    if (rec->parentName) fn(rec->parentName);
  }

  static std::string name() { return "record"; }

  static void assert_bases(const IndexData&, const php::Record* rec);
  static void try_flatten_traits(const php::Program*, const IndexData&,
                                 const php::Record*, RecordInfo*,
                                 RecPreResolveUpdates&);

  using Info = RecordInfo;
};

template<typename T>
struct TypeInfoData {
  // Map from name to types that directly use that name (as parent,
  // interface or trait).
  hphp_hash_map<SString,
                CompactVector<const T*>,
                string_data_hash,
                string_data_isame>     users;
  // Map from types to number of dependencies, used in
  // conjunction with users field above.
  hphp_hash_map<const T*, uint32_t> depCounts;

  uint32_t cqFront{};
  uint32_t cqBack{};
  std::vector<const T*> queue;
  bool hasPseudoCycles{};
};

using ClassInfoData = TypeInfoData<php::Class>;
using RecordInfoData = TypeInfoData<php::Record>;

// We want const qualifiers on various index data structures for php
// object pointers, but during index creation time we need to
// manipulate some of their attributes (changing the representation).
// This little wrapper keeps the const_casting out of the main line of
// code below.
void attribute_setter(const Attr& attrs, bool set, Attr attr) {
  attrSetter(const_cast<Attr&>(attrs), set, attr);
}

void add_unit_to_index(IndexData& index, php::Unit& unit) {
  hphp_hash_map<
    const php::Class*,
    hphp_hash_set<const php::Class*>
  > closureMap;

  for (auto& c : unit.classes) {
    assertx(!(c->attrs & AttrNoOverride));

    if (c->attrs & AttrEnum) {
      add_symbol(index.enums, c.get(), "enum");
    }

    add_symbol(index.classes, c.get(), "class", index.records, index.typeAliases);

    for (auto& m : c->methods) {
      attribute_setter(m->attrs, false, AttrNoOverride);
      index.methods.insert({m->name, m.get()});

      uint64_t refs = 0, cur = 1;
      bool anyInOut = false;
      for (auto& p : m->params) {
        if (p.inout) {
          refs |= cur;
          anyInOut = true;
        }
        // It doesn't matter that we lose parameters beyond the 64th,
        // for those, we'll conservatively check everything anyway.
        cur <<= 1;
      }
      if (anyInOut) {
        // Multiple methods with the same name will be combined in the same
        // cell, thus we use |=. This only makes sense in WholeProgram mode
        // since we use this field to check that no functions has its n-th
        // parameter as inout, which requires global knowledge.
        index.method_inout_params_by_name[m->name] |= refs;
      }
    }

    if (c->closureContextCls) {
      closureMap[c->closureContextCls].insert(c.get());
    }
  }

  if (!closureMap.empty()) {
    for (auto const& c1 : closureMap) {
      auto& s = index.classClosureMap[c1.first];
      for (auto const& c2 : c1.second) {
        s.push_back(c2);
      }
    }
  }

  for (auto i = unit.funcs.begin(); i != unit.funcs.end();) {
    auto& f = *i;
    // Deduplicate meth_caller wrappers- We just take the first one we see.
    if (f->attrs & AttrIsMethCaller && index.funcs.count(f->name)) {
      unit.funcs.erase(i);
      continue;
    }
    add_symbol(index.funcs, f.get(), "function");
    ++i;
  }

  for (auto& ta : unit.typeAliases) {
    add_symbol(index.typeAliases, ta.get(), "type alias", index.classes, index.records);
  }

  for (auto& c : unit.constants) {
    add_symbol(index.constants, c.get(), "constant");
  }

  for (auto& rec : unit.records) {
    assertx(!(rec->attrs & AttrNoOverride));
    add_symbol(index.records, rec.get(), "record", index.classes, index.typeAliases);
  }
}

template<class T>
using TypeInfo = typename std::conditional<std::is_same<T, php::Class>::value,
                                           ClassInfo, RecordInfo>::type;


void PhpTypeHelper<php::Class>::assert_bases(const IndexData& index,
                                             const php::Class* cls) {
  if (cls->parentName) {
    assertx(index.classInfo.count(cls->parentName));
  }
  for (DEBUG_ONLY auto& i : cls->interfaceNames) {
    assertx(index.classInfo.count(i));
  }
  for (DEBUG_ONLY auto& t : cls->usedTraitNames) {
    assertx(index.classInfo.count(t));
  }
}

void PhpTypeHelper<php::Record>::assert_bases(const IndexData& index,
                                              const php::Record* rec) {
  if (rec->parentName) {
    assertx(index.recordInfo.count(rec->parentName));
  }
}

using ClonedClosureMap = hphp_hash_map<
  php::Class*,
  std::pair<std::unique_ptr<php::Class>, uint32_t>
>;


std::unique_ptr<php::Func> clone_meth_helper(
  php::Unit* unit,
  php::Class* newContext,
  const php::Func* origMeth,
  std::unique_ptr<php::Func> cloneMeth,
  std::atomic<uint32_t>& nextFuncId,
  ClsPreResolveUpdates& updates,
  ClonedClosureMap& clonedClosures
);

std::unique_ptr<php::Class> clone_closure(php::Unit* unit,
                                          php::Class* newContext,
                                          php::Class* cls,
                                          std::atomic<uint32_t>& nextFuncId,
                                          ClsPreResolveUpdates& updates,
                                          ClonedClosureMap& clonedClosures) {
  auto clone = std::make_unique<php::Class>(*cls);
  assertx(clone->closureContextCls);
  clone->closureContextCls = newContext;
  clone->unit = newContext->unit;
  auto i = 0;
  for (auto& cloneMeth : clone->methods) {
    cloneMeth = clone_meth_helper(unit,
                                  clone.get(),
                                  cls->methods[i++].get(),
                                  std::move(cloneMeth),
                                  nextFuncId,
                                  updates,
                                  clonedClosures);
    if (!cloneMeth) return nullptr;
  }
  return clone;
}

std::unique_ptr<php::Func> clone_meth_helper(
  php::Unit* unit,
  php::Class* newContext,
  const php::Func* origMeth,
  std::unique_ptr<php::Func> cloneMeth,
  std::atomic<uint32_t>& nextFuncId,
  ClsPreResolveUpdates& preResolveUpdates,
  ClonedClosureMap& clonedClosures) {

  cloneMeth->cls  = newContext;
  cloneMeth->idx  = nextFuncId.fetch_add(1, std::memory_order_relaxed);
  if (!cloneMeth->originalFilename) {
    cloneMeth->originalFilename = origMeth->unit->filename;
  }
  if (!cloneMeth->originalUnit) {
    cloneMeth->originalUnit = origMeth->unit;
  }
  cloneMeth->unit = newContext->unit;

  if (!origMeth->hasCreateCl) return cloneMeth;

  auto const recordClosure = [&] (uint32_t* clsId) {
    auto const cls = origMeth->unit->classes[*clsId].get();
    auto& elm = clonedClosures[cls];
    if (!elm.first) {
      elm.first = clone_closure(unit,
                                newContext->closureContextCls ?
                                newContext->closureContextCls : newContext,
                                cls, nextFuncId, preResolveUpdates,
                                clonedClosures);
      if (!elm.first) return false;
      elm.second = preResolveUpdates.nextClass(*unit);
    }
    *clsId = elm.second;
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
          if (!recordClosure(&clsId)) return nullptr;
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

std::unique_ptr<php::Func> clone_meth(php::Unit* unit,
                                      php::Class* newContext,
                                      const php::Func* origMeth,
                                      SString name,
                                      Attr attrs,
                                      std::atomic<uint32_t>& nextFuncId,
                                      ClsPreResolveUpdates& updates,
                                      ClonedClosureMap& clonedClosures) {

  auto cloneMeth  = std::make_unique<php::Func>(*origMeth);
  cloneMeth->name = name;
  cloneMeth->attrs = attrs | AttrTrait;
  return clone_meth_helper(unit, newContext, origMeth, std::move(cloneMeth),
                           nextFuncId, updates, clonedClosures);
}

bool merge_inits(std::vector<std::unique_ptr<php::Func>>& clones,
                 php::Unit* unit,
                 ClassInfo* cinfo,
                 std::atomic<uint32_t>& nextFuncId,
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
      xinit = clone_meth(unit, cls, func, func->name, func->attrs, nextFuncId,
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

bool merge_xinits(Attr attr,
                  std::vector<std::unique_ptr<php::Func>>& clones,
                  php::Unit* unit,
                  ClassInfo* cinfo,
                  std::atomic<uint32_t>& nextFuncId,
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
      return merge_inits(clones, unit, cinfo, nextFuncId,
                         updates, clonedClosures, xinitName);
    }
  }
  return true;
}

bool merge_cinits(std::vector<std::unique_ptr<php::Func>>& clones,
                  php::Unit* unit,
                  ClassInfo* cinfo,
                  std::atomic<uint32_t>& nextFuncId,
                  ClsPreResolveUpdates& updates,
                  ClonedClosureMap& clonedClosures) {
  auto const xinitName = s_86cinit.get();
  for (auto const& c : cinfo->traitConsts) {
    if (c.val && c.val->m_type == KindOfUninit) {
      return merge_inits(clones, unit, cinfo, nextFuncId,
                         updates, clonedClosures, xinitName);
    }
  }
  return true;
}

void rename_closure(const IndexData& index,
                    php::Class* cls,
                    ClsPreResolveUpdates& updates) {
  auto n = cls->name->slice();
  auto const p = n.find(';');
  if (p != std::string::npos) {
    n = n.subpiece(0, p);
  }
  auto const newName = makeStaticString(NewAnonymousClassName(n));
  assertx(!index.classes.count(newName));
  cls->name = newName;
  updates.newClosures.emplace_back(cls);
}

template <typename T>
void preresolve(const php::Program*,
                const IndexData&,
                const T*,
                PreResolveUpdates<typename PhpTypeHelper<T>::Info>&);

void flatten_traits(const php::Program* program,
                    const IndexData& index,
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
  auto& nextFuncId = const_cast<php::Program*>(program)->nextFuncId;

  for (auto const ent : methodsToAdd) {
    auto clone = clone_meth(cls->unit, cls, ent->second.func, ent->first,
                            ent->second.attrs, nextFuncId,
                            updates, clonedClosures);
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
    if (!merge_xinits(AttrNone, clones, cls->unit, cinfo,
                      nextFuncId, updates, clonedClosures) ||
        !merge_xinits(AttrStatic, clones, cls->unit, cinfo,
                      nextFuncId, updates, clonedClosures) ||
        !merge_xinits(AttrLSB, clones, cls->unit, cinfo,
                      nextFuncId, updates, clonedClosures)) {
      ITRACE(5, "Not flattening {} because we couldn't merge the 86xinits\n",
             cls->name);
      return;
    }
  }

  // flatten initializers for constants in traits
  if (cinfo->traitConsts.size()) {
    if (!merge_cinits(clones, cls->unit, cinfo, nextFuncId, updates,
                      clonedClosures)) {
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

    if (clonedClosures.size()) {
      auto& closures = updates.closures[cls];
      for (auto& ent : clonedClosures) {
        auto clo = ent.second.first.get();
        rename_closure(index, clo, updates);
        ITRACE(5, "  - closure {} as {}\n", ent.first->name, clo->name);
        assertx(clo->closureContextCls == cls);
        assertx(clo->unit == cls->unit);
        closures.emplace_back(clo);
        updates.newClasses.emplace_back(
          std::move(ent.second.first),
          cls->unit,
          ent.second.second
        );
        preresolve(program, index, clo, updates);
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

  hphp_hash_set<PreClass::ClassRequirement, EqHash, EqHash> reqs;

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
 * Given a static representation of a Hack record, find a possible resolution
 * of the record along with all records in its hierarchy.
 */
RecordInfo* resolve_combinations(const IndexData& index,
                                 const php::Record* rec,
                                 RecPreResolveUpdates& updates) {
  auto rinfo = std::make_unique<RecordInfo>();
  rinfo->rec = rec;
  if (rec->parentName) {
    auto const parent = index.recordInfo.at(rec->parentName);
    if (parent->rec->attrs & AttrFinal) {
      ITRACE(2,
             "Resolve combinations failed for `{}' because "
             "its parent record `{}' is not abstract\n",
             rec->name, parent->rec->name);
      return nullptr;
    }
    rinfo->parent = parent;
    rinfo->baseList = rinfo->parent->baseList;
  }
  rinfo->baseList.push_back(rinfo.get());
  rinfo->baseList.shrink_to_fit();
  ITRACE(2, "  resolved: {}\n", rec->name);
  updates.newInfos.emplace_back(std::move(rinfo));
  return updates.newInfos.back().get();
}

/*
 * Given a static representation of a Hack class, find a possible resolution
 * of the class along with all classes, interfaces and traits in its hierarchy.
 *
 * Returns the resultant ClassInfo, or nullptr if the Hack class
 * cannot be instantiated at runtime.
 */
ClassInfo* resolve_combinations(const IndexData& index,
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

void PhpTypeHelper<php::Record>::try_flatten_traits(const php::Program*,
                                                    const IndexData&,
                                                    const php::Record*,
                                                    RecordInfo*,
                                                    RecPreResolveUpdates&) {}

void PhpTypeHelper<php::Class>::try_flatten_traits(
    const php::Program* program,
    const IndexData& index,
    const php::Class* cls,
    ClassInfo* cinfo,
    ClsPreResolveUpdates& updates) {
  if (options.FlattenTraits &&
      !(cls->attrs & AttrNoExpandTrait) &&
      !cls->usedTraitNames.empty() &&
      index.classes.count(cls->name) == 1) {
    Trace::Indent indent;
    flatten_traits(program, index, cinfo, updates);
  }
}

template <typename T>
void preresolve(const php::Program* program,
                const IndexData& index,
                const T* type,
                PreResolveUpdates<typename PhpTypeHelper<T>::Info>& updates) {
  ITRACE(2, "preresolve {}: {}:{}\n",
         PhpTypeHelper<T>::name(), type->name, (void*)type);

  auto const resolved = [&] {
    Trace::Indent indent;
    if (debug) {
      PhpTypeHelper<T>::assert_bases(index, type);
    }
    return resolve_combinations(index, type, updates);
  }();

  ITRACE(3, "preresolve: {}:{} ({} resolutions)\n",
         type->name, (void*)type, resolved ? 1 : 0);

  if (resolved) {
    updates.updateDeps.emplace_back(resolved);
    PhpTypeHelper<T>::try_flatten_traits(
      program, index, type, resolved, updates
    );
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
    // Also add instantiable classes to their interface's subclassLists
    if (cinfo->cls->attrs & (AttrTrait | AnyEnum | AttrAbstract)) continue;
    for (auto& ipair : cinfo->implInterfaces) {
      auto impl = const_cast<ClassInfo*>(ipair.second);
      impl->subclassList.push_back(cinfo.get());
    }
  }

  for (auto& cinfo : index.allClassInfos) {
    auto& sub = cinfo->subclassList;
    if ((fixupTraits && cinfo->cls->attrs & AttrTrait) ||
        (fixupEnums && cinfo->cls->attrs & AnyEnum)) {
      // traits and enums can be reached by multiple paths, so we need to
      // uniquify their subclassLists.
      std::sort(begin(sub), end(sub));
      sub.erase(
        std::unique(begin(sub), end(sub)),
        end(sub)
      );
    }
    sub.shrink_to_fit();
  }
}

bool define_func_family(IndexData& index, ClassInfo* cinfo,
                        SString name, const php::Func* func = nullptr) {
  FuncFamily::PFuncVec funcs{};
  for (auto const cleaf : cinfo->subclassList) {
    auto const leafFn = [&] () -> const MethTabEntryPair* {
      auto const leafFnIt = cleaf->methods.find(name);
      if (leafFnIt == end(cleaf->methods)) return nullptr;
      return mteFromIt(leafFnIt);
    }();
    if (!leafFn) continue;
    funcs.push_back(leafFn);
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
           cinfo->cls->name, name);
    for (auto const DEBUG_ONLY func : funcs) {
      FTRACE(4, "  {}::{}\n",
             func->second.func->cls->name, func->second.func->name);
    }
  }

  // Single func resolutions are stored separately. They don't need a
  // FuncFamily and this saves space.
  if (funcs.size() == 1) {
    cinfo->singleMethodFamilies.emplace(name, funcs[0]);
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

  cinfo->methodFamilies.emplace(
    std::piecewise_construct,
    std::forward_as_tuple(name),
    std::forward_as_tuple(ff)
  );

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

void define_func_families(IndexData& index) {
  trace_time tracer("define_func_families");

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
          if (!mte->first->isame(s_construct.get()) &&
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
      ff->m_numInOut = num_inout_from_set(ff->possibleFuncs());
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

  hphp_hash_map<const php::Class*,
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
  hphp_hash_map<const php::Class*, int> iface_uses;

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
  hphp_hash_map<Slot, int> slot_uses;
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
      for (auto c = cinfo->parent; c; c = c->parent) {
        c->isDerivedMocked = true;
      }
    }
  }
}

void mark_const_props(IndexData& index) {
  trace_time tracer("mark const props");

  for (auto& cinfo : index.allClassInfos) {
    auto const hasConstProp = [&]() {
      if (cinfo->cls->hasConstProp) return true;
      if (cinfo->parent && cinfo->parent->hasConstProp) return true;
      if (!(cinfo->cls->attrs & AttrNoExpandTrait)) {
        for (auto t : cinfo->usedTraits) {
          if (t->cls->hasConstProp) return true;
        }
      }
      return false;
    }();
    if (hasConstProp) {
      cinfo->hasConstProp = true;
      for (auto c = cinfo.get(); c; c = c->parent) {
        if (c->derivedHasConstProp) break;
        c->derivedHasConstProp = true;
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
      always_assert(pf->first->isame(name));
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

Type context_sensitive_return_type(IndexData& data,
                                   CallContext callCtx,
                                   Type returnType) {
  constexpr auto max_interp_nexting_level = 2;
  static __thread uint32_t interp_nesting_level;
  auto const finfo = func_info(data, callCtx.callee);
  returnType = return_with_context(std::move(returnType), callCtx.context);

  auto checkParam = [&] (int i) {
    auto const constraint = finfo->func->params[i].typeConstraint;
    if (constraint.hasConstraint() &&
        !constraint.isTypeVar() &&
        !constraint.isTypeConstant()) {
      auto ctx = Context { finfo->func->unit, finfo->func, finfo->func->cls };
      auto t = data.m_index->lookup_constraint(ctx, constraint);
      return callCtx.args[i].strictlyMoreRefined(t);
    }
    return callCtx.args[i].strictSubtypeOf(TInitCell);
  };

  // TODO(#3788877): more heuristics here would be useful.
  bool const tryContextSensitive = [&] {
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

  if (!tryContextSensitive) {
    return returnType;
  }

  {
    ContextRetTyMap::const_accessor acc;
    if (data.contextualReturnTypes.find(acc, callCtx)) {
      if (data.frozen || acc->second == TBottom || is_scalar(acc->second)) {
        return acc->second;
      }
    }
  }

  if (data.frozen) {
    return returnType;
  }

  auto contextType = [&] {
    ++interp_nesting_level;
    SCOPE_EXIT { --interp_nesting_level; };

    auto const func = finfo->func;
    auto const wf = php::WideFunc::cns(func);
    auto const calleeCtx = AnalysisContext { func->unit, wf, func->cls };
    auto const ty =
      analyze_func_inline(*data.m_index, calleeCtx,
                          callCtx.context, callCtx.args).inferredReturn;
    return return_with_context(ty, callCtx.context);
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

PrepKind func_param_prep_default() {
  return PrepKind::Val;
}

PrepKind func_param_prep(const php::Func* func,
                         uint32_t paramId) {
  if (paramId >= func->params.size()) {
    return PrepKind::Val;
  }
  return func->params[paramId].inout ? PrepKind::InOut : PrepKind::Val;
}

template<class PossibleFuncRange>
PrepKind prep_kind_from_set(PossibleFuncRange range, uint32_t paramId) {

  /*
   * In single-unit mode, the range is not complete. Without konwing all
   * possible resolutions, HHBBC cannot deduce anything about by-val vs inout.
   * So the caller should make sure not calling this in single-unit mode.
   */
  if (begin(range) == end(range)) {
    return func_param_prep_default();
  }

  struct FuncFind {
    using F = const php::Func*;
    static F get(std::pair<SString,F> p) { return p.second; }
    static F get(const MethTabEntryPair* mte) { return mte->second.func; }
  };

  folly::Optional<PrepKind> prep;
  for (auto& item : range) {
    switch (func_param_prep(FuncFind::get(item), paramId)) {
    case PrepKind::Unknown:
      return PrepKind::Unknown;
    case PrepKind::InOut:
      if (prep && *prep != PrepKind::InOut) return PrepKind::Unknown;
      prep = PrepKind::InOut;
      break;
    case PrepKind::Val:
      if (prep && *prep != PrepKind::Val) return PrepKind::Unknown;
      prep = PrepKind::Val;
      break;
    }
  }
  return *prep;
}

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
      yesOrNo(prop.attrs & AttrIsReadOnly),
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
    [&] (const ClassInfo* ci) -> folly::Optional<PropLookupResult<>> {
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
      return folly::none;
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
          if (mustBeReadOnly && !(prop.attrs & AttrIsReadOnly)) {
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
    [&] (const ClassInfo* ci) -> folly::Optional<PropMergeResult<>> {
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
        if (mustBeReadOnly && !(prop.attrs & AttrIsReadOnly)) {
          ITRACE(
            6, "{}:${} found but is mutable and must be readonly, stopping\n",
            ci->cls->name, propName
          );
          return notFound();
        }
        return merge(prop, ci);
      }
      return folly::none;
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

}

//////////////////////////////////////////////////////////////////////
namespace {
template<typename T>
void buildTypeInfoData(TypeInfoData<T>& tid,
                       const ISStringToOneT<const T*>& tmap) {
  for (auto const& elm : tmap) {
    auto const t = elm.second;
    auto const addUser = [&] (SString rName) {
      tid.users[rName].push_back(t);
      ++tid.depCounts[t];
    };
    PhpTypeHelper<T>::process_bases(t, addUser);

    if (!tid.depCounts.count(t)) {
      FTRACE(5, "Adding no-dep {} {}:{} to queue\n",
             PhpTypeHelper<T>::name(), t->name, (void*)t);
      // make sure that closure is first, because we end up calling
      // preresolve directly on closures created by trait
      // flattening, which assumes all dependencies are satisfied.
      if (tid.queue.size() && t->name == s_Closure.get()) {
        tid.queue.push_back(tid.queue[0]);
        tid.queue[0] = t;
      } else {
        tid.queue.push_back(t);
      }
    } else {
      FTRACE(6, "{} {}:{} has {} deps\n",
             PhpTypeHelper<T>::name(), t->name, (void*)t, tid.depCounts[t]);
    }
  }
  tid.cqBack = tid.queue.size();
  tid.queue.resize(tmap.size());
}

SString nameFromInfo(const RecordInfo* r) { return r->rec->name; }
SString nameFromInfo(const ClassInfo* c)  { return c->cls->name; }

template <typename T>
void updatePreResolveDeps(
    TypeInfoData<T>& tid,
    const PreResolveUpdates<typename PhpTypeHelper<T>::Info>& updates) {
  for (auto const info : updates.updateDeps) {
    auto const& users = tid.users[nameFromInfo(info)];
    for (auto const tu : users) {
      auto const it = tid.depCounts.find(tu);
      if (it == tid.depCounts.end()) {
        assertx(tid.hasPseudoCycles);
        continue;
      }
      auto& depCount = it->second;
      assertx(depCount);
      if (!--depCount) {
        tid.depCounts.erase(it);
        ITRACE(5, "  enqueue: {}:{}\n", tu->name, (void*)tu);
        tid.queue[tid.cqBack++] = tu;
      } else {
        ITRACE(6, "  depcount: {}:{} = {}\n", tu->name, (void*)tu, depCount);
      }
    }
  }
}

void commitPreResolveUpdates(IndexData& index,
                             TypeInfoData<php::Record>& tid,
                             std::vector<RecPreResolveUpdates>& updates) {
  parallel::parallel(
    [&] {
      for (auto const& u : updates) updatePreResolveDeps(tid, u);
    },
    [&] {
      for (auto& u : updates) {
        for (size_t i = 0; i < u.newInfos.size(); ++i) {
          auto& rinfo = u.newInfos[i];
          auto const UNUSED it =
            index.recordInfo.emplace(rinfo->rec->name, rinfo.get());
          assertx(it.second);
          index.allRecordInfos.emplace_back(std::move(rinfo));
        }
      }
    }
  );
}

void commitPreResolveUpdates(IndexData& index,
                             TypeInfoData<php::Class>& tid,
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
        for (auto& cns : u.removeNoOverride) {
          const_cast<php::Const*>(cns.get())->isNoOverride = false;
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
        for (auto const c : u.newClosures) index.classes.emplace(c->name, c);
      }
    },
    [&] {
      for (auto& u : updates) {
        for (auto& p : u.newClasses) {
          auto unit = std::get<1>(p);
          auto const idx = std::get<2>(p);
          if (unit->classes.size() <= idx) unit->classes.resize(idx+1);
          unit->classes[idx] = std::move(std::get<0>(p));
        }
      }
    }
  );
}

template<typename T>
void preresolveTypes(php::Program* program,
                     IndexData& index,
                     TypeInfoData<T>& tid,
                     const ISStringToOneT<TypeInfo<T>*>& tmap) {
  auto round = uint32_t{0};
  while (true) {
    if (tid.cqFront == tid.cqBack) {
      // we've consumed everything where all dependencies are
      // satisfied. There may still be some pseudo-cycles that can
      // be broken though.
      //
      // eg if A extends B and B' extends A', we'll resolve B and
      // A', and then end up here, since both A and B' still have
      // one dependency. But both A and B' can be resolved at this
      // point
      for (auto it = tid.depCounts.begin();
           it != tid.depCounts.end();) {
        auto canResolve = true;
        auto const checkCanResolve = [&] (SString name) {
          if (canResolve) canResolve = tmap.count(name);
        };
        PhpTypeHelper<T>::process_bases(it->first, checkCanResolve);
        if (canResolve) {
          FTRACE(2, "Breaking pseudo-cycle for {} {}:{}\n",
                 PhpTypeHelper<T>::name(), it->first->name, (void*)it->first);
          tid.queue[tid.cqBack++] = it->first;
          it = tid.depCounts.erase(it);
          tid.hasPseudoCycles = true;
        } else {
          ++it;
        }
      }
      if (tid.cqFront == tid.cqBack) break;
    }

    auto const workitems = tid.cqBack - tid.cqFront;
    auto updates = [&] {
      trace_time trace(
        "preresolve",
        folly::sformat("round {} -- {} work items", round, workitems)
      );

      using U = PreResolveUpdates<typename PhpTypeHelper<T>::Info>;
      typename U::UnitNumClasses numClasses;

      return parallel::gen(
        workitems,
        [&] (size_t idx) {
          auto const t = tid.queue[idx + tid.cqFront];
          Trace::Bump bumper{
            Trace::hhbbc_index, kSystemLibBump, is_systemlib_part(*t->unit)
          };
          (void)bumper;
          U updates;
          updates.numClasses = &numClasses;
          preresolve(program, index, t, updates);
          return updates;
        }
      );
    }();

    ++round;
    tid.cqFront += workitems;

    trace_time trace("update");
    commitPreResolveUpdates(index, tid, updates);
  }

  trace_time trace("preresolve clear state");
  parallel::for_each(
    index.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& cinfo) {
      cinfo->preResolveState.reset();
    }
  );
}

} //namespace

Index::Index(php::Program* program)
  : m_data(std::make_unique<IndexData>(this))
{
  trace_time tracer("create index");

  m_data->arrTableBuilder.reset(new ArrayTypeTable::Builder());

  add_system_constants_to_index(*m_data);

  {
    trace_time trace_add_units("add units to index");
    for (auto& u : program->units) {
      add_unit_to_index(*m_data, *u);
    }
  }

  RecordInfoData rid;
  {
    trace_time build_record_info_data("build recordinfo data");
    buildTypeInfoData(rid, m_data->records);
  }

  {
    trace_time preresolve_records("preresolve records");
    preresolveTypes(program, *m_data, rid, m_data->recordInfo);
  }

  ClassInfoData cid;
  {
    trace_time build_class_info_data("build classinfo data");
    buildTypeInfoData(cid, m_data->classes);
  }

  {
    trace_time preresolve_classes("preresolve classes");
    preresolveTypes(program, *m_data, cid, m_data->classInfo);
  }

  m_data->funcInfo.resize(program->nextFuncId);

  // Part of the index building routines happens before the various asserted
  // index invariants hold.  These each may depend on computations from
  // previous functions, so be careful changing the order here.
  compute_subclass_list(*m_data);
  clean_86reifiedinit_methods(*m_data); // uses the base class lists
  mark_no_override_methods(*m_data);
  find_magic_methods(*m_data);          // uses the subclass lists
  find_mocked_classes(*m_data);
  mark_const_props(*m_data);
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
void Index::rewrite_default_initial_values(php::Program& program) const {
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
      auto inState = [&] () -> folly::Optional<PropSet> {
        PropSet in;
        for (auto const& sub : cinfo->subclassList) {
          if (sub == cinfo || sub->parent != cinfo) continue;
          auto const it = outStates.find(sub);
          if (it == outStates.end()) return folly::none;
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
  for (auto& unit : program.units) {
    for (auto& c : unit->classes) {
      if (is_closure(*c)) continue;

      auto const out = [&] () -> folly::Optional<PropSet> {
        folly::Optional<PropSet> props;
        auto const range = m_data->classInfo.equal_range(c->name);
        for (auto it = range.first; it != range.second; ++it) {
          if (it->second->cls != c.get()) continue;
          auto const outStateIt = outStates.find(it->second);
          if (outStateIt == outStates.end()) return folly::none;
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

        prop.val = nullable
          ? make_tv<KindOfNull>()
          : prop.typeConstraint.defaultValue();
      }
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

template<typename T>
folly::Optional<T> Index::resolve_type_impl(SString name) const {
  auto const& infomap = m_data->infoMap<T>();
  auto const& omap = m_data->infoMap<typename ResTypeHelper<T>::OtherT>();
  auto const it = infomap.find(name);
  if (it != end(infomap)) {
    auto const tinfo = it->second;
    /*
     * If the preresolved [Class|Record]Info is Unique we can give it out.
     */
    assertx(tinfo->phpType()->attrs & AttrUnique);
    if (debug &&
        (omap.count(name) ||
          m_data->typeAliases.count(name))) {
      std::fprintf(stderr, "non unique \"unique\" %s: %s\n",
                    ResTypeHelper<T>::name().c_str(),
                    tinfo->phpType()->name->data());

      auto const ta = m_data->typeAliases.find(name);
      if (ta != end(m_data->typeAliases)) {
        std::fprintf(stderr, "   and type-alias %s\n",
                      ta->second->name->data());
      }

      auto const to = omap.find(name);
      if (to != end(omap)) {
        std::fprintf(stderr, "   and %s %s\n",
                      ResTypeHelper<typename ResTypeHelper<T>::OtherT>::
                      name().c_str(),
                      to->second->phpType()->name->data());
      }
      always_assert(0);
    }
    return T { tinfo };
  }
  // We refuse to have name-only resolutions of enums and typeAliases,
  // so that all name only resolutions can be treated as records or classes.
  if (!m_data->enums.count(name) &&
      !m_data->typeAliases.count(name) &&
      !omap.count(name)) {
    return T { name };
  }

  return folly::none;
}

folly::Optional<res::Record> Index::resolve_record(SString recName) const {
  recName = normalizeNS(recName);
  return resolve_type_impl<res::Record>(recName);
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

folly::Optional<res::Class> Index::resolve_class(Context ctx,
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

  return resolve_type_impl<res::Class>(clsName);
}

folly::Optional<res::Class> Index::selfCls(const Context& ctx) const {
  if (!ctx.cls || is_used_trait(*ctx.cls)) return folly::none;
  return resolve_class(ctx.cls);
}

folly::Optional<res::Class> Index::parentCls(const Context& ctx) const {
  if (!ctx.cls || !ctx.cls->parentName) return folly::none;
  if (auto const parent = resolve_class(ctx.cls).parent()) return parent;
  return resolve_class(ctx, ctx.cls->parentName);
}

Index::ResolvedInfo<boost::variant<boost::blank,res::Class,res::Record>>
Index::resolve_type_name(SString inName) const {
  auto const res = resolve_type_name_internal(inName);
  using Ret = boost::variant<boost::blank,res::Class,res::Record>;
  auto const val = match<Ret>(
    res.value,
    [&] (boost::blank) { return Ret{}; },
    [&] (SString s) {
      return (res.type == AnnotType::Record) ?
             Ret{res::Record{s}} : Ret{res::Class{s}};
    },
    [&] (ClassInfo* c) { return res::Class{c}; },
    [&] (RecordInfo* r) { return res::Record{r}; }
  );
  return { res.type, res.nullable, val };
}

Index::ResolvedInfo<boost::variant<boost::blank,SString,ClassInfo*,RecordInfo*>>
Index::resolve_type_name_internal(SString inName) const {
  folly::Optional<hphp_fast_set<const void*>> seen;

  auto nullable = false;
  auto name = inName;

  for (unsigned i = 0; ; ++i) {
    name = normalizeNS(name);
    auto const rec_it = m_data->recordInfo.find(name);
    if (rec_it != end(m_data->recordInfo)) {
      auto const rinfo = rec_it->second;
      assertx(rinfo->rec->attrs & AttrUnique);
      return { AnnotType::Record, nullable, rinfo };
    }
    auto const cls_it = m_data->classInfo.find(name);
    if (cls_it != end(m_data->classInfo)) {
      auto const cinfo = cls_it->second;
      assertx(cinfo->cls->attrs & AttrUnique);
      if (!(cinfo->cls->attrs & AttrEnum)) {
        return { AnnotType::Object, nullable, cinfo };
      }
      auto const& tc = cinfo->cls->enumBaseTy;
      assertx(!tc.isNullable());
      if (tc.type() != AnnotType::Object) {
        auto const type = tc.type() == AnnotType::Mixed ?
          AnnotType::ArrayKey : tc.type();
        return { type, nullable, tc.typeName() };
      }
      name = tc.typeName();
    } else {
      auto const ta_it = m_data->typeAliases.find(name);
      if (ta_it == end(m_data->typeAliases)) break;
      auto const ta = ta_it->second;
      assertx(ta->attrs & AttrUnique);
      nullable = nullable || ta->nullable;
      if (ta->type != AnnotType::Object) {
        return { ta->type, nullable, ta->value.get() };
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
        return { AnnotType::Object, false, {} };
      }
    }
  }

  return { AnnotType::Object, nullable, name };
}

struct Index::ConstraintResolution {
  /* implicit */ ConstraintResolution(Type type)
    : type{std::move(type)}
    , maybeMixed{false} {}
  ConstraintResolution(folly::Optional<Type> type, bool maybeMixed)
    : type{std::move(type)}
    , maybeMixed{maybeMixed} {}

  folly::Optional<Type> type;
  bool maybeMixed;
};

Index::ConstraintResolution Index::resolve_named_type(
  const Context& ctx, SString name, const Type& candidate) const {

  auto const res = resolve_type_name_internal(name);

  if (res.nullable && candidate.subtypeOf(BInitNull)) return TInitNull;

  if (res.type == AnnotType::Object) {
    auto resolve = [&] (const res::Class& rcls) -> folly::Optional<Type> {
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
      return folly::none;
    };

    auto const val = match<Either<SString, ClassInfo*>>(
      res.value,
      [&] (boost::blank) { return nullptr; },
      [&] (SString s) { return s; },
      [&] (ClassInfo* c) { return c; },
      [&] (RecordInfo*) { always_assert(false); return nullptr; }
    );
    if (val.isNull()) return ConstraintResolution{ folly::none, true };
    auto ty = resolve(res::Class { val });
    if (ty && res.nullable) *ty = opt(std::move(*ty));
    return ConstraintResolution{ std::move(ty), false };
  } else if (res.type == AnnotType::Record) {
    auto const val = match<Either<SString, RecordInfo*>>(
      res.value,
      [&] (boost::blank) { return nullptr; },
      [&] (SString s) { return s; },
      [&] (ClassInfo* c) { always_assert(false); return nullptr; },
      [&] (RecordInfo* r) { return r; }
    );
    if (val.isNull()) return ConstraintResolution{ folly::none, true };
    return subRecord(res::Record { val });
  }

  return get_type_for_annotated_type(ctx, res.type, res.nullable,
                                     boost::get<SString>(res.value), candidate);
}

std::pair<res::Class,php::Class*>
Index::resolve_closure_class(Context ctx, int32_t idx) const {
  auto const cls = ctx.unit->classes[idx].get();
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
  auto name_only = [&] {
    return res::Func { this, res::Func::MethodName { name } };
  };

  if (!is_specialized_cls(clsType)) {
    return name_only();
  }
  auto const dcls  = dcls_of(clsType);
  auto const cinfo = dcls.cls.val.right();
  if (!cinfo) return name_only();

  // Classes may have more method families than methods. Any such
  // method families are guaranteed to all be public so we can do this
  // lookup as a last gasp before resorting to name_only().
  auto const find_extra_method = [&] {
    auto singleMethIt = cinfo->singleMethodFamilies.find(name);
    if (singleMethIt != cinfo->singleMethodFamilies.end()) {
      return res::Func { this, singleMethIt->second };
    }
    auto methIt = cinfo->methodFamilies.find(name);
    if (methIt == end(cinfo->methodFamilies)) return name_only();
    // If there was a sole implementer we can resolve to a single method, even
    // if the method was not declared on the interface itself.
    assertx(methIt->second->possibleFuncs().size() > 1);
    return res::Func { this, methIt->second };
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
  if (methIt == end(cinfo->methods)) return find_extra_method();
  auto const ftarget = methIt->second.func;

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
        return name_only();
      }
    }
  }

  auto resolve = [&] {
    create_func_info(*m_data, ftarget);
    return res::Func { this, mteFromIt(methIt) };
  };

  switch (dcls.type) {
  case DCls::Exact:
    return resolve();
  case DCls::Sub:
    if (methIt->second.attrs & AttrNoOverride) {
      return resolve();
    }
    if (!options.FuncFamilies) return name_only();

    {
      auto const singleFamIt = cinfo->singleMethodFamilies.find(name);
      if (singleFamIt != cinfo->singleMethodFamilies.end()) {
        return res::Func { this, singleFamIt->second };
      }
      auto const famIt = cinfo->methodFamilies.find(name);
      if (famIt == end(cinfo->methodFamilies)) return name_only();
      assertx(famIt->second->possibleFuncs().size() > 1);
      return res::Func { this, famIt->second };
    }
  }
  not_reached();
}

folly::Optional<res::Func>
Index::resolve_ctor(Context /*ctx*/, res::Class rcls, bool exact) const {
  auto const cinfo = rcls.val.right();
  if (!cinfo) return folly::none;
  if (cinfo->cls->attrs & (AttrInterface|AttrTrait)) return folly::none;

  auto const cit = cinfo->methods.find(s_construct.get());
  if (cit == end(cinfo->methods)) return folly::none;

  auto const ctor = mteFromIt(cit);
  if (exact || ctor->second.attrs & AttrNoOverride) {
    create_func_info(*m_data, ctor->second.func);
    return res::Func { this, ctor };
  }

  if (!options.FuncFamilies) return folly::none;

  auto const singleFamIt = cinfo->singleMethodFamilies.find(s_construct.get());
  if (singleFamIt != cinfo->singleMethodFamilies.end()) {
    return res::Func { this, singleFamIt->second};
  }
  auto const famIt = cinfo->methodFamilies.find(s_construct.get());
  if (famIt == end(cinfo->methodFamilies)) return folly::none;
  assertx(famIt->second->possibleFuncs().size() > 1);
  return res::Func { this, famIt->second };
}

res::Func
Index::resolve_func_helper(const php::Func* func, SString name) const {
  auto name_only = [&] (bool renamable) {
    return res::Func { this, res::Func::FuncName { name, renamable } };
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
    tc.typeName(),
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
    tc.typeName(),
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
      case KindOfRecord:       // fallthrough
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
    case AnnotMetaType::Self:
      if (auto s = selfCls(ctx)) return subObj(*s);
      break;
    case AnnotMetaType::Parent:
      if (auto p = parentCls(ctx)) return subObj(*p);
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
    }
    return ConstraintResolution{ folly::none, false };
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
  if (!tc.isObject()) return false;
  auto const name = tc.typeName();
  auto const resolved = resolve_type_name_internal(name);
  if (resolved.type != AnnotType::Object) return false;
  auto const val = match<Either<SString, ClassInfo*>>(
    resolved.value,
    [&] (boost::blank) { return nullptr; },
    [&] (SString s) { return s; },
    [&] (ClassInfo* c) { return c; },
    [&] (RecordInfo*) { always_assert(false); return nullptr; }
  );
  res::Class rcls{val};
  return rcls.couldHaveReifiedGenerics();
}

folly::Optional<bool>
Index::supports_async_eager_return(res::Func rfunc) const {
  auto const supportsAER = [] (const php::Func* func) {
    // Async functions always support async eager return.
    if (func->isAsync && !func->isGenerator) return true;

    // No other functions support async eager return yet.
    return false;
  };

  return match<folly::Optional<bool>>(
    rfunc.val,
    [&](res::Func::FuncName)   { return folly::none; },
    [&](res::Func::MethodName) { return folly::none; },
    [&](FuncInfo* finfo) { return supportsAER(finfo->func); },
    [&](const MethTabEntryPair* mte) { return supportsAER(mte->second.func); },
    [&](FuncFamily* fam) -> folly::Optional<bool> {
      auto ret = folly::Optional<bool>{};
      for (auto const pf : fam->possibleFuncs()) {
        // Abstract functions are never called.
        if (pf->second.attrs & AttrAbstract) continue;
        auto const val = supportsAER(pf->second.func);
        if (ret && *ret != val) return folly::none;
        ret = val;
      }
      return ret;
    });
}

bool Index::is_effect_free(const php::Func* func) const {
  return func_info(*m_data, func)->effectFree;
}

bool Index::is_effect_free(res::Func rfunc) const {
  return match<bool>(
    rfunc.val,
    [&](res::Func::FuncName)   { return false; },
    [&](res::Func::MethodName) { return false; },
    [&](FuncInfo* finfo)       { return finfo->effectFree; },
    [&](const MethTabEntryPair* mte) {
      return func_info(*m_data, mte->second.func)->effectFree;
    },
    [&](FuncFamily* fam) {
      for (auto const mte : fam->possibleFuncs()) {
        if (!func_info(*m_data, mte->second.func)->effectFree) return false;
      }
      return true;
    }
  );
}

const php::Const* Index::lookup_class_const_ptr(Context ctx,
                                                res::Class rcls,
                                                SString cnsName,
                                                bool allow_tconst) const {
  if (rcls.val.left()) return nullptr;
  auto const cinfo = rcls.val.right();

  auto const it = cinfo->clsConstants.find(cnsName);
  if (it != end(cinfo->clsConstants)) {
    if (!it->second->val.has_value() ||
        it->second->kind == ConstModifiers::Kind::Context ||
        (!allow_tconst && it->second->kind == ConstModifiers::Kind::Type)) {
      // This is an abstract class constant, context constant or type constant
      return nullptr;
    }
    if (it->second->val.value().m_type == KindOfUninit) {
      // This is a class constant that needs an 86cinit to run.
      // We'll add a dependency to make sure we're re-run if it
      // resolves anything.
      auto const cinit = it->second->cls->methods.back().get();
      assertx(cinit->name == s_86cinit.get());
      add_dependency(*m_data, cinit, ctx, Dep::ClsConst);
      return nullptr;
    }
    return it->second.get();
  }
  return nullptr;
}

Type Index::lookup_class_constant(Context ctx,
                                  res::Class rcls,
                                  SString cnsName,
                                  bool allow_tconst) const {
  auto const cnst = lookup_class_const_ptr(ctx, rcls, cnsName, allow_tconst);
  if (!cnst) return TInitCell;
  return from_cell(cnst->val.value());
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
                                        const php::Func* func,
                                        Type ctxType,
                                        CompactVector<Type> args) const {
  constexpr auto max_interp_nexting_level = 2;
  static __thread uint32_t interp_nesting_level;
  static __thread Context base_ctx;

  // Don't fold functions when staticness mismatches
  if ((func->attrs & AttrStatic) && ctxType.couldBe(TObj)) return TInitCell;
  if (!(func->attrs & AttrStatic) && ctxType.couldBe(TCls)) return TInitCell;

  auto const& finfo = *func_info(*m_data, func);
  if (finfo.effectFree && is_scalar(finfo.returnTy)) {
    return finfo.returnTy;
  }

  auto const calleeCtx = CallContext {
    func,
    std::move(args),
    std::move(ctxType)
  };

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
      AnalysisContext { func->unit, wf, func->cls },
      calleeCtx.context,
      calleeCtx.args,
      CollectionOpts::EffectFreeOnly
    );
    return fa.effectFree ? fa.inferredReturn : TInitCell;
  }();

  if (!is_scalar(contextType)) {
    return TInitCell;
  }

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
                               res::Func rfunc,
                               Dep dep) const {
  return match<Type>(
    rfunc.val,
    [&] (res::Func::FuncName)   { return TInitCell; },
    [&] (res::Func::MethodName) { return TInitCell; },
    [&] (FuncInfo* finfo) {
      add_dependency(*m_data, finfo->func, ctx, dep);
      return unctx(finfo->returnTy);
    },
    [&] (const MethTabEntryPair* mte) {
      if (methods) {
        if (auto ret = methods->lookupReturnType(*mte->second.func)) {
          return unctx(std::move(*ret));
        }
      }
      add_dependency(*m_data, mte->second.func, ctx, dep);
      auto const finfo = func_info(*m_data, mte->second.func);
      if (!finfo->func) return TInitCell;
      return unctx(finfo->returnTy);
    },
    [&] (FuncFamily* fam) {
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
    }
  );
}

Type Index::lookup_return_type(Context caller,
                               MethodsInfo* methods,
                               const CompactVector<Type>& args,
                               const Type& context,
                               res::Func rfunc,
                               Dep dep) const {
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
    [&] (const MethTabEntryPair* mte) {
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
    },
    [&] (FuncFamily* fam) {
      add_dependency(*m_data, fam, caller, dep);
      auto ret = fam->m_returnTy.get(
        [&] {
          auto ret = TBottom;
          for (auto const pf : fam->possibleFuncs()) {
            auto const finfo = func_info(*m_data, pf->second.func);
            if (!finfo->func) return TInitCell;
            ret |= finfo->returnTy;
            if (!ret.strictSubtypeOf(BInitCell)) return ret;
          }
          return ret;
        }
      );
      return return_with_context(std::move(ret), context);
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

folly::Optional<uint32_t> Index::lookup_num_inout_params(
  Context,
  res::Func rfunc
) const {
  return match<folly::Optional<uint32_t>>(
    rfunc.val,
    [&] (res::Func::FuncName s) -> folly::Optional<uint32_t> {
      if (s.renamable) return folly::none;
      auto const it = m_data->funcs.find(s.name);
      return it != end(m_data->funcs)
       ? func_num_inout(it->second)
       : 0;
    },
    [&] (res::Func::MethodName s) -> folly::Optional<uint32_t> {
      auto const it = m_data->method_inout_params_by_name.find(s.name);
      if (it == end(m_data->method_inout_params_by_name)) {
        // There was no entry, so no method by this name takes a parameter
        // by inout.
        return 0;
      }
      auto const pair = m_data->methods.equal_range(s.name);
      return num_inout_from_set(folly::range(pair.first, pair.second));
    },
    [&] (FuncInfo* finfo) {
      return func_num_inout(finfo->func);
    },
    [&] (const MethTabEntryPair* mte) {
      return func_num_inout(mte->second.func);
    },
    [&] (FuncFamily* fam) -> folly::Optional<uint32_t> {
      return fam->m_numInOut;
    }
  );
}

PrepKind Index::lookup_param_prep(Context /*ctx*/, res::Func rfunc,
                                  uint32_t paramId) const {
  return match<PrepKind>(
    rfunc.val,
    [&] (res::Func::FuncName s) {
      if (s.renamable) return PrepKind::Unknown;
      auto const it = m_data->funcs.find(s.name);
      return it != end(m_data->funcs)
        ? func_param_prep(it->second, paramId)
        : func_param_prep_default();
    },
    [&] (res::Func::MethodName s) {
      auto const it = m_data->method_inout_params_by_name.find(s.name);
      if (it == end(m_data->method_inout_params_by_name)) {
        // There was no entry, so no method by this name takes a parameter
        // by inout.
        return PrepKind::Val;
      }
      if (paramId < sizeof(it->second) * CHAR_BIT &&
          !((it->second >> paramId) & 1)) {
        // No method of this name takes parameter paramId by inout.
        return PrepKind::Val;
      }
      auto const pair = m_data->methods.equal_range(s.name);
      return prep_kind_from_set(folly::range(pair.first, pair.second), paramId);
    },
    [&] (FuncInfo* finfo) {
      return func_param_prep(finfo->func, paramId);
    },
    [&] (const MethTabEntryPair* mte) {
      return func_param_prep(mte->second.func, paramId);
    },
    [&] (FuncFamily* fam) {
      return prep_kind_from_set(fam->possibleFuncs(), paramId);
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

    auto ty = [&] {
      if (!cinfo) return TInitCell;
      auto const it = cinfo->publicStaticProps.find(prop.name);
      assertx(it != end(cinfo->publicStaticProps));
      return remove_uninit(it->second.inferredType);
    }();
    state.emplace(
      prop.name,
      PropStateElem<>{std::move(ty), &prop.typeConstraint, prop.attrs}
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

  // First try to obtain the property name as a static string
  auto const sname = [&] () -> SString {
    // Treat non-string names conservatively, but the caller should be
    // checking this.
    if (!name.subtypeOf(BStr)) return nullptr;
    auto const vname = tv(name);
    if (!vname || vname->m_type != KindOfPersistentString) return nullptr;
    return vname->m_data.pstr;
  }();

  // Conservative result when we can't do any better. The type can be
  // anything, and anything might throw.
  auto const conservative = [&] {
    ITRACE(4, "conservative\n");
    return PropLookupResult<>{
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

  auto const dcls = dcls_of(cls);
  if (dcls.cls.val.left()) return conservative();
  auto const cinfo = dcls.cls.val.right();

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

  switch (dcls.type) {
    case DCls::Sub: {
      // We know that `cls' is at least dcls.type, but could be a
      // subclass. For every subclass (including dcls.type itself),
      // start the property lookup from there, and union together all
      // the potential results. This could potentially visit a lot of
      // parent classes redundently, so tell it not to look into
      // parent classes, unless we're processing dcls.type.
      folly::Optional<PropLookupResult<>> result;
      for (auto const sub : cinfo->subclassList) {
        auto r = lookup_static_impl(
          *m_data,
          ctx,
          ctxCls,
          privateProps,
          sub,
          sname,
          !sname && sub != cinfo
        );
        ITRACE(4, "{} -> {}\n", sub->cls->name, show(r));
        if (!result) {
          result.emplace(std::move(r));
        } else {
          *result |= r;
        }
      }
      assertx(result.has_value());
      ITRACE(4, "union -> {}\n", show(*result));
      return *result;
    }
    case DCls::Exact: {
      // We know what exactly `cls' is. Just do the property lookup
      // starting from there.
      auto const r = lookup_static_impl(
        *m_data,
        ctx,
        ctxCls,
        privateProps,
        cinfo,
        sname,
        false
      );
      ITRACE(4, "{} -> {}\n", cinfo->cls->name, show(r));
      return r;
    }
  }
  always_assert(false);
}

Type Index::lookup_public_prop(const Type& cls, const Type& name) const {
  if (!is_specialized_cls(cls)) return TCell;

  auto const vname = tv(name);
  if (!vname || vname->m_type != KindOfPersistentString) return TCell;
  auto const sname = vname->m_data.pstr;

  auto const dcls = dcls_of(cls);
  if (dcls.cls.val.left()) return TCell;
  auto const cinfo = dcls.cls.val.right();

  switch (dcls.type) {
    case DCls::Sub: {
      auto ty = TBottom;
      for (auto const sub : cinfo->subclassList) {
        ty |= lookup_public_prop_impl(
          *m_data,
          sub,
          sname
        );
      }
      return ty;
    }
    case DCls::Exact:
      return lookup_public_prop_impl(
        *m_data,
        cinfo,
        sname
      );
  }
  always_assert(false);
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
    if (!name.subtypeOf(BStr)) return nullptr;
    auto const vname = tv(name);
    if (!vname || vname->m_type != KindOfPersistentString) return nullptr;
    return vname->m_data.pstr;
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

  auto const dcls = dcls_of(cls);
  if (dcls.cls.val.left()) return unknownCls();
  auto const cinfo = dcls.cls.val.right();

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

  switch (dcls.type) {
    case DCls::Sub: {
      // We know this class is either dcls.type, or a child class of
      // it. For every child of dcls.type (including dcls.type
      // itself), do the merge starting from it. To avoid redundant
      // work, only iterate into parent classes if we're dcls.type
      // (this is only a matter of efficiency. The merge is
      // idiompotent).
      folly::Optional<PropMergeResult<>> result;
      for (auto const sub : cinfo->subclassList) {
        auto r = merge_static_type_impl(
          *m_data,
          ctx,
          mergePublic,
          privateProps,
          ctxCls,
          sub,
          sname,
          val,
          checkUB,
          ignoreConst,
          mustBeReadOnly,
          !sname && sub != cinfo
        );
        ITRACE(4, "{} -> {}\n", sub->cls->name, show(r));
        if (!result) {
          result.emplace(std::move(r));
        } else {
          *result |= r;
        }
      }
      assertx(result.has_value());
      ITRACE(4, "union -> {}\n", show(*result));
      return *result;
    }
    case DCls::Exact: {
      // We know the class exactly. Do the merge starting from only
      // it.
      auto const r = merge_static_type_impl(
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
        false
      );
      ITRACE(4, "{} -> {}\n", cinfo->cls->name, show(r));
      return r;
    }
  }
  always_assert(false);
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
          false
      };
    }
  }
}

void Index::refine_class_constants(
    const Context& ctx,
    const CompactVector<std::pair<size_t, TypedValue>>& resolved,
    DependencyContextSet& deps) {
  if (!resolved.size()) return;
  auto& constants = ctx.func->cls->constants;
  for (auto const& c : resolved) {
    assertx(c.first < constants.size());
    auto& cnst = constants[c.first];
    assertx(cnst.val && cnst.val->m_type == KindOfUninit);
    cnst.val = c.second;
  }
  find_deps(*m_data, ctx.func, Dep::ClsConst, deps);
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
      ? func->cls->closureContextCls
      : func->cls;
    return lookup_constraint(Context { func->unit, func, cls }, tc);
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

  tcT = loosen_interfaces(loosen_all(to_cell(std::move(tcT))));

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
  auto const t = loosen_interfaces(fa.inferredReturn);

  auto const error_loc = [&] {
    return folly::sformat(
      "{} {}{}",
      func->unit->filename,
      func->cls ?
      folly::to<std::string>(func->cls->name->data(), "::") : std::string{},
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

  if (t.strictlyMoreRefined(finfo->returnTy)) {
    if (finfo->returnRefinements < options.returnTypeRefineLimit) {
      finfo->returnTy = t;
      // We've modifed the return type, so reset any cached FuncFamily
      // return types.
      for (auto const ff : finfo->families) ff->m_returnTy.reset();
      dep = is_scalar(t) ?
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
      more_refined_for_index(t, finfo->returnTy),
      "Index return type invariant violated in {}.\n"
      "   {} is not at least as refined as {}\n",
      error_loc(),
      show(t),
      show(finfo->returnTy)
    );
  }

  always_assert_flog(
    !finfo->effectFree || fa.effectFree,
    "Index effectFree changed from true to false in {} {}{}.\n",
    func->unit->filename,
    func->cls ? folly::to<std::string>(func->cls->name->data(), "::") :
    std::string{},
    func->name);

  if (finfo->effectFree != fa.effectFree) {
    finfo->effectFree = fa.effectFree;
    dep = Dep::InlineDepthLimit | Dep::ReturnTy;
  }

  if (dep != Dep{}) find_deps(*m_data, func, dep, deps);
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
        more_refined_for_index(vars[i], current[i]),
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
      more_refined_for_index(kv.second.ty, target.ty),
      "PropState refinement failed on {}::${} -- {} was not a subtype of {}\n",
      cls->name->data(),
      kv.first->data(),
      show(kv.second.ty),
      show(target.ty)
    );
    target.ty = kv.second.ty;
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

void Index::cleanup_for_final() {
  trace_time _{"cleanup_for_final"};
  CLEAR(m_data->dependencyMap);
}


void Index::cleanup_post_emit(php::ProgramPtr program) {
  trace_time _{"cleanup_post_emit"};
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
    parallel::for_each(program->units, [] (auto& u) { u.reset(); });
  }
  std::vector<std::function<void()>> clearers;
  #define CLEAR_PARALLEL(x) clearers.push_back([&] CLEAR(x));
  CLEAR_PARALLEL(m_data->classes);
  CLEAR_PARALLEL(m_data->methods);
  CLEAR_PARALLEL(m_data->method_inout_params_by_name);
  CLEAR_PARALLEL(m_data->funcs);
  CLEAR_PARALLEL(m_data->typeAliases);
  CLEAR_PARALLEL(m_data->enums);
  CLEAR_PARALLEL(m_data->constants);
  CLEAR_PARALLEL(m_data->records);

  CLEAR_PARALLEL(m_data->classClosureMap);
  CLEAR_PARALLEL(m_data->classExtraMethodMap);

  CLEAR_PARALLEL(m_data->allClassInfos);
  CLEAR_PARALLEL(m_data->classInfo);
  CLEAR_PARALLEL(m_data->funcInfo);

  CLEAR_PARALLEL(m_data->privatePropInfo);
  CLEAR_PARALLEL(m_data->privateStaticPropInfo);
  CLEAR_PARALLEL(m_data->publicSPropMutations);
  CLEAR_PARALLEL(m_data->funcFamilies);
  CLEAR_PARALLEL(m_data->ifaceSlotMap);
  CLEAR_PARALLEL(m_data->closureUseVars);

  CLEAR_PARALLEL(m_data->foldableReturnTypeMap);
  CLEAR_PARALLEL(m_data->contextualReturnTypes);

  parallel::for_each(clearers, [] (const std::function<void()>& f) { f(); });
}

void Index::thaw() {
  m_data->frozen = false;
}

std::unique_ptr<ArrayTypeTable::Builder>& Index::array_table_builder() const {
  return m_data->arrTableBuilder;
}

//////////////////////////////////////////////////////////////////////

res::Func Index::do_resolve(const php::Func* f) const {
  auto const finfo = create_func_info(*m_data, f);
  return res::Func { this, finfo };
};

// Return true if we know for sure that one php::Class must derive
// from another at runtime, in all possible instantiations.
bool Index::must_be_derived_from(const php::Class* cls,
                                 const php::Class* parent) const {
  if (cls == parent) return true;
  auto const clsClass_it   = m_data->classInfo.find(cls->name);
  auto const parentClass_it = m_data->classInfo.find(parent->name);
  if (clsClass_it == end(m_data->classInfo) || parentClass_it == end(m_data->classInfo)) {
    return true;
  }

  auto const rCls = res::Class { clsClass_it->second };
  auto const rPar = res::Class { parentClass_it->second };
  return rCls.mustBeSubtypeOf(rPar);
}

// Return true if any possible definition of one php::Class could
// derive from another at runtime, or vice versa.
bool
Index::could_be_related(const php::Class* cls,
                        const php::Class* parent) const {
  if (cls == parent) return true;
  auto const clsClass_it   = m_data->classInfo.find(cls->name);
  auto const parentClass_it = m_data->classInfo.find(parent->name);
  if (clsClass_it == end(m_data->classInfo) || parentClass_it == end(m_data->classInfo)) {
    return false;
  }

  auto const rCls = res::Class { clsClass_it->second };
  auto const rPar = res::Class { parentClass_it->second };
  return rCls.couldBe(rPar);
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

}}
