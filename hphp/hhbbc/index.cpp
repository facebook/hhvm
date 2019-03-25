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
#include <unordered_set>
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

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-comparisons.h"

#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/trait-method-import-data.h"
#include "hphp/runtime/vm/unit-util.h"

#include "hphp/hhbbc/type-builtins.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/context.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/options-util.h"
#include "hphp/hhbbc/parallel.h"
#include "hphp/hhbbc/analyze.h"

#include "hphp/util/algorithm.h"
#include "hphp/util/assertions.h"
#include "hphp/util/match.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc_index);

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_construct("__construct");
const StaticString s_call("__call");
const StaticString s_get("__get");
const StaticString s_set("__set");
const StaticString s_isset("__isset");
const StaticString s_unset("__unset");
const StaticString s_callStatic("__callStatic");
const StaticString s_toBoolean("__toBoolean");
const StaticString s_invoke("__invoke");
const StaticString s_86cinit("86cinit");
const StaticString s_86pinit("86pinit");
const StaticString s_86sinit("86sinit");
const StaticString s_86linit("86linit");
const StaticString s_Closure("Closure");
const StaticString s_AsyncGenerator("HH\\AsyncGenerator");
const StaticString s_Generator("Generator");

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

/*
 * One-to-one case insensitive map, where the keys are static strings
 * and the values are some T.
 *
 * Elements are not stable under insert/erase.
 */
template<class T> using ISStringToOneFastT =
  hphp_fast_map<
    SString,
    T,
    string_data_hash,
    string_data_isame
  >;

/*
 * One-to-one case insensitive map, where the keys are static strings
 * and the values are some kind of pointer.
 */
template<class T> using ISStringToOne = ISStringToOneT<T*>;

template<class MultiMap>
folly::Range<typename MultiMap::const_iterator>
find_range(const MultiMap& map, typename MultiMap::key_type key) {
  auto const pair = map.equal_range(key);
  return folly::range(pair.first, pair.second);
}

// Like find_range, but copy them into a temporary buffer instead of
// returning iterators, so you can still mutate the underlying
// multimap.
template<class MultiMap>
std::vector<typename MultiMap::value_type>
copy_range(const MultiMap& map, typename MultiMap::key_type key) {
  auto range = find_range(map, key);
  return std::vector<typename MultiMap::value_type>(begin(range), end(range));
}

//////////////////////////////////////////////////////////////////////

enum class Dep : uintptr_t {
  /* This dependency should trigger when the return type changes */
  ReturnTy = (1u << 0),
  /* This dependency should trigger when a DefCns is resolved */
  ConstVal = (1u << 1),
  /* This dependency should trigger when a class constant is resolved */
  ClsConst = (1u << 2),
  /* This dependency should trigger when the bad initial prop value bit for a
   * class changes */
  PropBadInitialValues = (1u << 3),
  /* This dependency should trigger when a public static property with a
   * particular name changes */
  PublicSPropName = (1u << 4),
  /* This dependency means that we refused to do inline analysis on
   * this function due to inline analysis depth. The dependency will
   * trigger if the target function becomes effect-free, or gets a
   * literal return value.
   */
  InlineDepthLimit = (1u << 5),
};

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
  const TypeConstraint* tc;
  uint32_t refinements;
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
      ISStringToOneT<MethTabEntry>::value_type {};

namespace {

using MethTabEntryPair = res::Func::MethTabEntryPair;

inline MethTabEntryPair* mteFromElm(
  ISStringToOneT<MethTabEntry>::value_type& elm) {
  return static_cast<MethTabEntryPair*>(&elm);
}

inline const MethTabEntryPair* mteFromElm(
  const ISStringToOneT<MethTabEntry>::value_type& elm) {
  return static_cast<const MethTabEntryPair*>(&elm);
}

inline MethTabEntryPair* mteFromIt(ISStringToOneT<MethTabEntry>::iterator it) {
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
      ret[prop.name].ty = TGen;
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
  uint32_t returnRefinments{0};

  /*
   * Whether the function is effectFree.
   */
  bool effectFree{false};

  /*
   * Type info for local statics.
   */
  CompactVector<Type> localStaticTypes;

  /*
   * Bitset representing which parameters definitely don't affect the
   * result of the function, assuming it produces one. Note that
   * VerifyParamType does not count as a use in this context.
   */
  std::bitset<64> unusedParams;
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
 *
 * Carefully pack it into 8 bytes, so that hphp_fast_map will use
 * F14VectorMap.
 */
struct res::Func::FuncFamily {
  using PFuncVec = CompactVector<const MethTabEntryPair*>;
  static_assert(sizeof(PFuncVec) == sizeof(uintptr_t),
                "CompactVector must be layout compatible with a pointer");

  struct Holder {
    Holder(const Holder& o) : bits{o.bits} {}
    explicit Holder(PFuncVec&& o) : v{std::move(o)} {}
    explicit Holder(uintptr_t b) : bits{b & ~3} {}
    Holder& operator=(const Holder&) = delete;
    ~Holder() {}
    const PFuncVec* operator->() const { return &v; }
    uintptr_t val() const { return bits; }
    friend auto begin(const Holder& h) { return h->begin(); }
    friend auto end(const Holder& h) { return h->end(); }
  private:
    union {
      uintptr_t bits;
      PFuncVec  v;
    };
  };

  FuncFamily(PFuncVec&& v,
             bool containsInterceptables,
             bool isCtor) : m_raw{Holder{std::move(v)}.val()} {
    if (containsInterceptables) m_raw |= 1;
    if (isCtor) m_raw |= 2;
  }
  FuncFamily(FuncFamily&& o) noexcept : m_raw(o.m_raw) {
    o.m_raw = 0;
  }
  ~FuncFamily() {
    Holder{m_raw & ~3}->~PFuncVec();
  }
  FuncFamily& operator=(const FuncFamily&) = delete;

  bool containsInterceptables() const { return m_raw & 1; };
  bool isCtor() const { return m_raw & 2; }
  const Holder possibleFuncs() const {
    return Holder{m_raw & ~3};
  };
private:
  uintptr_t m_raw;
};

//////////////////////////////////////////////////////////////////////

/*
 * Known information about a particular possible instantiation of a
 * PHP class.  The php::Class will be marked AttrUnique if there is a
 * unique ClassInfo with the same name, and no interfering class_aliases.
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
   * A (case-sensitive) map from class constant name to the php::Const
   * that it came from.  This map is flattened across the inheritance
   * hierarchy.
   */
  hphp_fast_map<SString,const php::Const*> clsConstants;

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
   * A (case-insensitive) map from class method names to the php::Func
   * associated with it.  This map is flattened across the inheritance
   * hierarchy.
   */
  ISStringToOneT<MethTabEntry> methods;

  /*
   * A (case-insensitive) map from class method names to associated
   * FuncFamily objects that group the set of possibly-overriding
   * methods.
   *
   * Note that this does not currently encode anything for interface
   * methods.
   *
   * Invariant: methods on this class with AttrNoOverride or
   * AttrPrivate will not have an entry in this map.
   */
  ISStringToOneFastT<FuncFamily> methodFamilies;

  /*
   * The constructor for this class, if we know what it is.
   */
  const MethTabEntryPair* ctor = nullptr;

  /*
   * Subclasses of this class, including this class itself (unless it
   * is an interface).
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
   * not just the value in these maps.  To handle mutations of public statics
   * where the name is known, but not which class was affected, these always
   * need to be unioned with values from IndexData::unknownClassSProps.
   */
  hphp_hash_map<SString,PublicSPropEntry> publicStaticProps;

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
   * Flags about the existence of various magic methods, or whether
   * any derived classes may have those methods.  The non-derived
   * flags imply the derived flags, even if the class is final, so you
   * don't need to check both in those situations.
   */
  struct MagicFnInfo {
    bool thisHas{false};
    bool derivedHas{false};
  };
  MagicFnInfo
    magicCall,
    magicCallStatic,
    magicGet,
    magicSet,
    magicIsset,
    magicUnset,
    magicBool;
};

using MagicMapInfo = struct {
  ClassInfo::MagicFnInfo ClassInfo::*pmem;
  Attr attrBit;
};

const std::vector<std::pair<SString,MagicMapInfo>> magicMethodMap {
  { s_call.get(),       { &ClassInfo::magicCall,       AttrNone } },
  { s_callStatic.get(), { &ClassInfo::magicCallStatic, AttrNone } },
  { s_toBoolean.get(),  { &ClassInfo::magicBool,       AttrNone } },
  { s_get.get(),        { &ClassInfo::magicGet,   AttrNoOverrideMagicGet } },
  { s_set.get(),        { &ClassInfo::magicSet,   AttrNoOverrideMagicSet } },
  { s_isset.get(),      { &ClassInfo::magicIsset, AttrNoOverrideMagicIsset } },
  { s_unset.get(),      { &ClassInfo::magicUnset, AttrNoOverrideMagicUnset } }
};

//////////////////////////////////////////////////////////////////////

namespace res {

Class::Class(const Index* idx,
             Either<SString,ClassInfo*> val)
  : index(idx)
  , val(val)
{}

// Class type operations here are very conservative for now.

bool Class::same(const Class& o) const {
  return val == o.val;
}

bool Class::subtypeOf(const Class& o) const {
  auto s1 = val.left();
  auto s2 = o.val.left();
  if (s1 || s2) return s1 == s2;
  auto c1 = val.right();
  auto c2 = o.val.right();

  // If c2 is an interface, see if c1 declared it.
  if (c2->cls->attrs & AttrInterface) {
    if (c1->implInterfaces.count(c2->cls->name)) {
      return true;
    }
    return false;
  }

  // Otherwise check for direct inheritance.
  if (c1->baseList.size() >= c2->baseList.size()) {
    return c1->baseList[c2->baseList.size() - 1] == c2;
  }
  return false;
}

bool Class::couldBe(const Class& o) const {
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

bool Class::couldBeInterfaceOrTrait() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return (cinfo->cls->attrs & (AttrInterface | AttrTrait));
    }
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

bool Class::couldBeOverriden() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return !(cinfo->cls->attrs & AttrNoOverride);
    }
  );
}

bool Class::couldHaveMagicGet() const {
  return val.match(
    [] (SString) { return true; },
    [] (ClassInfo* cinfo) {
      return cinfo->magicGet.derivedHas;
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
  if (RuntimeOption::EvalForbidDynamicCalls > 0) {
    return val.match(
      [] (SString) { return true; },
      [] (ClassInfo* cinfo) {
        return !(cinfo->cls->attrs & AttrDynamicallyConstructible);
      }
    );
  }
  return false;
}

folly::Optional<Class> Class::commonAncestor(const Class& o) const {
  if (val.left() || o.val.left()) return folly::none;
  auto const c1 = val.right();
  auto const c2 = o.val.right();
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
  return res::Class { index, ancestor };
}

folly::Optional<res::Class> Class::parent() const {
  if (!val.right()) return folly::none;
  auto parent = val.right()->parent;
  if (!parent) return folly::none;
  return res::Class { index, parent };
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

bool Func::same(const Func& o) const {
  /*
   * TODO(#3666699): function name case sensitivity here shouldn't
   * break equality.
   */
  return val == o.val;
}

SString Func::name() const {
  return match<SString>(
    val,
    [&] (FuncName s)   { return s.name; },
    [&] (MethodName s) { return s.name; },
    [&] (FuncInfo* fi) { return fi->func->name; },
    [&] (const MethTabEntryPair* mte) { return mte->first; },
    [&] (FuncFamily* fa) -> SString {
      if (fa->isCtor()) return s_construct.get();
      auto const name = fa->possibleFuncs()->front()->first;
      if (debug) {
        for (DEBUG_ONLY auto const f : fa->possibleFuncs()) {
          assert(f->first->isame(name));
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

bool Func::cantBeMagicCall() const {
  return match<bool>(
    val,
    [&](FuncName)                { return true; },
    [&](MethodName)              { return false; },
    [&](FuncInfo*)               { return true; },
    [&](const MethTabEntryPair*) { return true; },
    [&](FuncFamily*)             { return true; }
  );
}

bool Func::isFoldable() const {
  return match<bool>(val,
                     [&](FuncName)   { return false; },
                     [&](MethodName) { return false; },
                     [&](FuncInfo* fi) {
                       return fi->func->attrs & AttrIsFoldable;
                     },
                     [&](const MethTabEntryPair* mte) {
                       return mte->second.func->attrs & AttrIsFoldable;
                     },
                     [&](FuncFamily* fa) {
                       return false;
                     });
}

bool Func::mightBeSkipFrame() const {
  return match<bool>(
    val,
    // Only builtins can be skip frame and non-method builtins are always
    // uniquely resolvable unless renaming is involved.
    [&](FuncName s) { return s.renamable; },
    [&](MethodName) { return true; },
    [&](FuncInfo* fi) { return fi->func->attrs & AttrSkipFrame; },
    [&](const MethTabEntryPair* mte) {
      return mte->second.func->attrs & AttrSkipFrame;
    },
    [&](FuncFamily* fa) {
      for (auto const pf : fa->possibleFuncs()) {
        if (pf->second.func->attrs & AttrSkipFrame) return true;
      }
      return false;
    });
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
    });
}

bool Func::mightCareAboutDynCalls() const {
  if (RuntimeOption::EvalNoticeOnBuiltinDynamicCalls && mightBeBuiltin()) {
    return true;
  }
  if (RuntimeOption::EvalForbidDynamicCalls > 0) {
    auto const res = match<bool>(
      val,
      [&](FuncName) { return true; },
      [&](MethodName) { return true; },
      [&](FuncInfo* fi) {
        return !(fi->func->attrs & AttrDynamicallyCallable);
      },
      [&](const MethTabEntryPair* mte) {
        return !(mte->second.func->attrs & AttrDynamicallyCallable);
      },
      [&](FuncFamily* fa) {
        for (auto const pf : fa->possibleFuncs()) {
          if (!(pf->second.func->attrs & AttrDynamicallyCallable)) return true;
        }
        return false;
      }
    );
    if (res) return true;
  }
  return false;
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

std::string show(const Func& f) {
  auto ret = f.name()->toCppString();
  match<void>(f.val,
              [&](Func::FuncName s) { if (s.renamable) ret += '?'; },
              [&](Func::MethodName) {},
              [&](FuncInfo* /*fi*/) { ret += "*"; },
              [&](const MethTabEntryPair* /*mte*/) { ret += "*"; },
              [&](FuncFamily* /*fa*/) { ret += "+"; });
  return ret;
}

}

//////////////////////////////////////////////////////////////////////

using IfaceSlotMap = hphp_hash_map<const php::Class*, Slot>;

struct Index::IndexData {
  explicit IndexData(Index* index) : m_index{index} {}
  IndexData(const IndexData&) = delete;
  IndexData& operator=(const IndexData&) = delete;
  ~IndexData() = default;

  Index* m_index;

  bool frozen{false};
  bool ever_frozen{false};
  bool any_interceptable_functions{false};

  std::unique_ptr<ArrayTypeTable::Builder> arrTableBuilder;

  ISStringToMany<const php::Class>       classes;
  ISStringToMany<const php::Func>        methods;
  ISStringToOneT<uint64_t>               method_ref_params_by_name;
  ISStringToMany<const php::Func>        funcs;
  ISStringToMany<const php::TypeAlias>   typeAliases;
  ISStringToMany<const php::Class>       enums;
  hphp_fast_map<SString, ConstInfo> constants;
  hphp_fast_set<SString, string_data_hash, string_data_isame> classAliases;

  // Map from each class to all the closures that are allocated in
  // functions of that class.
  hphp_hash_map<
    const php::Class*,
    CompactVector<const php::Class*>
  > classClosureMap;

  hphp_hash_map<
    const php::Class*,
    hphp_fast_set<php::Func*>
  > classExtraMethodMap;

  // Map from every interface to the list of instantiable classes which can
  // implement it.
  hphp_fast_map<
    const php::Class*,
    CompactVector<ClassInfo*>
  > ifaceImplementerMap;

  /*
   * Map from each class name to ClassInfo objects for all
   * not-known-to-be-impossible resolutions of the class at runtime.
   *
   * If the class is unique, there will only be one resolution.
   * Otherwise there will be one for each possible path through the
   * inheritance hierarchy, potentially excluding cases that we know
   * would definitely fatal when defined.
   */
  ISStringToMany<ClassInfo> classInfo;

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

  // If this is true, we don't know anything about public static properties and
  // must be pessimistic. We start in this state (before we've analyzed any
  // mutations) and remain in it if we see a mutation where both the name and
  // class are unknown.
  bool allPublicSPropsUnknown{true};

  // Best known types for public static properties where we knew the name, but
  // not the class. The type we're allowed to assume for a public static
  // property is the union of the ClassInfo-specific type with the unknown class
  // type that's stored here. The second value is the number of times the type
  // has been refined.
  hphp_hash_map<SString, std::pair<Type, uint32_t>> unknownClassSProps;

  // The set of gathered public static property mutations for each function. The
  // inferred types for the public static properties is the union of all these
  // mutations. If a function is not analyzed in a particular analysis round,
  // its mutations are left unchanged from the previous round.
  hphp_hash_map<const php::Func*, PublicSPropMutations> publicSPropMutations;

  /*
   * Map from interfaces to their assigned vtable slots, computed in
   * compute_iface_vtables().
   */
  IfaceSlotMap ifaceSlotMap;

  hphp_fast_map<
    const php::Class*,
    CompactVector<Type>
  > closureUseVars;

  bool useClassDependencies;
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
   * Vector of class aliases that need to be added to the index when
   * its safe to do so (see update_class_aliases).
   */
  std::vector<std::pair<SString, SString>> pending_class_aliases;
  std::mutex pending_class_aliases_mutex;
};

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

using IndexData = Index::IndexData;

DependencyContext make_dep(const php::Func* func) {
  return DependencyContext{DependencyContextType::Func, func};
}
DependencyContext make_dep(const php::Class* cls) {
  return DependencyContext{DependencyContextType::Class, cls};
}
DependencyContext make_dep(SString name) {
  return DependencyContext{DependencyContextType::PropName, name};
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
        assert(fi->func == f);
        return fi;
      }
      // We'd infer this anyway when we look at the bytecode body
      // (NativeImpl) for the HNI function, but just initializing it
      // here saves on whole-program iterations.
      fi->returnTy = native_function_return_type(f);
    }
    fi->func = f;
  }

  assert(fi->func == f);
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
  DepMap::const_accessor acc;
  if (data.dependencyMap.find(acc, make_dep(src))) {
    for (auto& kv : acc->second) {
      if (has_dep(kv.second, mask)) deps.insert(kv.first);
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

  static bool isAsync(method_type meth) {
    return meth->isAsync;
  }
  static bool isStatic(method_type meth) {
    return meth->attrs & AttrStatic;
  }
  static bool isFinal(method_type meth) {
    return meth->attrs & AttrFinal;
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
                                   string_type methName) {
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
  static void errorInconsistentAttr(string_type traitName,
                                    string_type methName,
                                    const char* attr) {
    throw TMIException(folly::sformat(
      "Redeclaration of trait method '{}::{}' is inconsistent about '{}'",
      traitName, methName, attr
    ));
  }
  static void errorRedeclaredNotFinal(string_type traitName,
                                      string_type methName) {
    throw TMIException(folly::sformat(
      "Redeclaration of final trait method '{}::{}' must also be final",
      traitName, methName
    ));
  }

};

using TMIData = TraitMethodImportData<TraitMethod,
                                      TMIOps>;

struct BuildClsInfo {
  IndexData& index;
  ClassInfo* rleaf;
  hphp_hash_map<SString, std::pair<php::Prop, const ClassInfo*>,
                string_data_hash, string_data_same> pbuilder;
};

/*
 * Make a flattened table of the constants on this class.
 */
bool build_class_constants(BuildClsInfo& info,
                           const ClassInfo* rparent,
                           bool fromTrait) {
  for (auto& c : rparent->cls->constants) {
    auto& cptr = info.rleaf->clsConstants[c.name];
    if (!cptr) {
      cptr = &c;
      continue;
    }

    // Same constant (from an interface via two different paths) is ok
    if (cptr->cls == rparent->cls) continue;

    if (cptr->isTypeconst != c.isTypeconst) {
      ITRACE(2,
             "build_cls_info_rec failed for `{}' because `{}' was defined by "
             "`{}' as a {}constant and by `{}' as a {}constant\n",
             info.rleaf->cls->name, c.name,
             rparent->cls->name, c.isTypeconst ? "type " : "",
             cptr->cls->name, cptr->isTypeconst ? "type " : "");
      return false;
    }

    // Ignore abstract constants
    if (!c.val) continue;

    if (cptr->val) {
      // Constants from interfaces implemented by traits silently lose
      if (fromTrait) continue;

      // A constant from an interface collides with an existing constant.
      if (rparent->cls->attrs & AttrInterface) {
        ITRACE(2,
               "build_cls_info_rec failed for `{}' because "
               "`{}' was defined by both `{}' and `{}'\n",
               info.rleaf->cls->name, c.name,
               rparent->cls->name, cptr->cls->name);
        return false;
      }
    }

    cptr = &c;
  }
  return true;
}

bool build_class_properties(BuildClsInfo& info,
                            const ClassInfo* rparent) {
  // There's no need to do this work if traits have been flattened
  // already, or if the top level class has no traits.  In those
  // cases, we might be able to rule out some ClassInfo
  // instantiations, but it doesn't seem worth it.
  if (info.rleaf->cls->attrs & AttrNoExpandTrait) return true;
  if (info.rleaf->usedTraits.empty()) return true;

  auto addProp = [&] (const php::Prop& p, bool add) {
    auto ent = std::make_pair(p, rparent);
    auto res = info.pbuilder.emplace(p.name, ent);
    if (res.second) {
      if (add) info.rleaf->traitProps.push_back(p);
      return true;
    }
    auto& prevProp = res.first->second.first;
    if (rparent == res.first->second.second) {
      assertx(rparent == info.rleaf);
      if ((prevProp.attrs ^ p.attrs) &
          (AttrStatic | AttrPublic | AttrProtected | AttrPrivate) ||
          (!(p.attrs & AttrSystemInitialValue) &&
           !(prevProp.attrs & AttrSystemInitialValue) &&
           !Class::compatibleTraitPropInit(prevProp.val, p.val))) {
        ITRACE(2,
               "build_class_properties failed for `{}' because "
               "two declarations of `{}' at the same level had "
               "different attributes\n",
               info.rleaf->cls->name, p.name);
        return false;
      }
      return true;
    }

    if (!(prevProp.attrs & AttrPrivate)) {
      if ((prevProp.attrs ^ p.attrs) & AttrStatic) {
        ITRACE(2,
               "build_class_properties failed for `{}' because "
               "`{}' was defined both static and non-static\n",
               info.rleaf->cls->name, p.name);
        return false;
      }
      if (p.attrs & AttrPrivate) {
        ITRACE(2,
               "build_class_properties failed for `{}' because "
               "`{}' was re-declared private\n",
               info.rleaf->cls->name, p.name);
        return false;
      }
      if (p.attrs & AttrProtected && !(prevProp.attrs & AttrProtected)) {
        ITRACE(2,
               "build_class_properties failed for `{}' because "
               "`{}' was redeclared protected from public\n",
               info.rleaf->cls->name, p.name);
        return false;
      }
    }
    if (add && res.first->second.second != rparent) {
      info.rleaf->traitProps.push_back(p);
    }
    res.first->second = ent;
    return true;
  };

  for (auto const& p : rparent->cls->properties) {
    if (!addProp(p, false)) return false;
  }

  if (rparent == info.rleaf) {
    for (auto t : rparent->usedTraits) {
      for (auto const& p : t->cls->properties) {
        if (!addProp(p, true)) return false;
      }
      for (auto const& p : t->traitProps) {
        if (!addProp(p, true)) return false;
      }
    }
  } else {
    for (auto const& p : rparent->traitProps) {
      if (!addProp(p, false)) return false;
    }
  }

  return true;
}

void build_methods_for_iface(IndexData& data, ClassInfo* iface) {
  std::vector<SString> names;
  auto const impls = data.ifaceImplementerMap.find(iface->cls);
  if (impls == data.ifaceImplementerMap.end()) return;

  // We start by collecting the list of methods shared across all classes which
  // implement iface (including indirectly). And then add the public methods
  // which are not constructors and have no private ancestors to the method
  // families of iface. Note that this set may be larger than the methods
  // declared on iface and may also be missing methods declared on iface. In
  // practice this is the set of methods we can depend on having accessible
  // given any object which is known to implement iface.
  auto it = impls->second.begin();
  for (auto& par : (*it)->methods) names.push_back(par.first);

  while (++it != impls->second.end()) {
    auto& methods = (*it)->methods;
    for (auto nameIt = names.begin(); nameIt != names.end();) {
      if (!methods.count(*nameIt)) nameIt = names.erase(nameIt);
      else ++nameIt;
    }
  }

  hphp_fast_set<SString> added;

  auto add_method = [&] (SString name) {
    res::Func::FuncFamily::PFuncVec funcs;
    bool containsInterceptables = false;

    hphp_hash_set<const php::Func*> seen;

    for (auto cinfo : impls->second) {
      assertx(!(cinfo->cls->attrs & AttrInterface));

      auto methIt = cinfo->methods.find(name);
      assertx(methIt != cinfo->methods.end());
      auto mte = mteFromIt(methIt);

      // Don't create method families for interfaces with non-public methods
      // or methods which may be constructors. We won't always know from context
      // which implementer we are referring to and whether we share a common
      // context. In practice interfaces generally declare public methods.
      if (cinfo->ctor == mte || !(mte->second.attrs & AttrPublic)) return;

      // If the method has a private ancestor we won't be able to determine from
      // context if a given resolution should be for the private ancestor or the
      // interface method. In theory we could dump all of the private ancestors
      // into the family too but as is noted above we also don't currently
      // handle non-public resolution.
      if (mte->second.hasPrivateAncestor) return;

      if (mte->second.attrs & AttrInterceptable) {
        containsInterceptables = true;
      }

      // Avoid adding duplicate entries to the list
      if (seen.emplace(mte->second.func).second) funcs.push_back(mte);
    }

    if (!funcs.empty()) {
      funcs.shrink_to_fit();
      iface->methodFamilies.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(name),
        std::forward_as_tuple(std::move(funcs),
                              containsInterceptables,
                              false)
      );
      added.emplace(name);
    }
  };

  for (auto name : names) {
    add_method(name);
  }

  for (auto& m : iface->cls->methods) {
    if (added.count(m->name)) {
      iface->methods.emplace(
        m->name,
        MethTabEntry { m.get(), m->attrs, false, true }
      );
    }
  }
  return;
}

void build_iface_methods(IndexData& data) {
  trace_time tracer("build interface methods");
  parallel::for_each(
    data.allClassInfos,
    [&] (const std::unique_ptr<ClassInfo>& info) {
      if (info->cls->attrs & AttrInterface) {
        build_methods_for_iface(data, info.get());
      }
    }
  );
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
bool build_class_methods(BuildClsInfo& info) {

  auto methodOverride = [&] (auto& it,
                             const php::Func* meth,
                             Attr attrs,
                             SString name) {
    if (it->second.func->attrs & AttrFinal) {
      if (!is_mock_class(info.rleaf->cls)) {
        ITRACE(2,
               "build_class_methods failed for `{}' because "
               "it tried to override final method `{}::{}'\n",
               info.rleaf->cls->name,
               it->second.func->cls->name, name);
        return false;
      }
    }
    ITRACE(9,
           "  {}: overriding method {}::{} with {}::{}\n",
           info.rleaf->cls->name,
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
      info.rleaf->methods.erase(it);
      it = info.rleaf->methods.emplace(name, mte).first;
    }
    return true;
  };

  // If there's a parent, start by copying its methods
  if (auto const rparent = info.rleaf->parent) {
    for (auto& mte : rparent->methods) {
      // don't inherit the 86* methods.
      if (HPHP::Func::isSpecial(mte.first)) continue;
      auto const res = info.rleaf->methods.emplace(mte.first, mte.second);
      assertx(res.second);
      res.first->second.topLevel = false;
      ITRACE(9,
             "  {}: inheriting method {}::{}\n",
             info.rleaf->cls->name,
             rparent->cls->name, mte.first);
      continue;
    }
  }

  uint32_t idx = info.rleaf->methods.size();

  // Now add our methods.
  for (auto& m : info.rleaf->cls->methods) {
    auto res = info.rleaf->methods.emplace(
      m->name,
      MethTabEntry { m.get(), m->attrs, false, true }
    );
    if (res.second) {
      res.first->second.idx = idx++;
      ITRACE(9,
             "  {}: adding method {}::{}\n",
             info.rleaf->cls->name,
             info.rleaf->cls->name, m->name);
      continue;
    }
    if (m->attrs & AttrTrait && m->attrs & AttrAbstract) {
      // abstract methods from traits never override anything.
      continue;
    }
    if (!methodOverride(res.first, m.get(), m->attrs, m->name)) return false;
  }

  // If our traits were previously flattened, we're done.
  if (info.rleaf->cls->attrs & AttrNoExpandTrait) return true;

  try {
    TMIData tmid;
    for (auto const t : info.rleaf->usedTraits) {
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
      for (auto const c : info.index.classClosureMap[t->cls]) {
        auto const invoke = find_method(c, s_invoke.get());
        assertx(invoke);
        info.index.classExtraMethodMap[info.rleaf->cls].insert(invoke);
      }
    }

    for (auto const& precRule : info.rleaf->cls->traitPrecRules) {
      tmid.applyPrecRule(precRule, info.rleaf);
    }
    auto const& aliasRules = info.rleaf->cls->traitAliasRules;
    tmid.applyAliasRules(aliasRules.begin(), aliasRules.end(), info.rleaf);
    auto traitMethods = tmid.finish(info.rleaf);
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
      auto res = info.rleaf->methods.emplace(
        mdata.name,
        MethTabEntry { method, attrs, false, true }
      );
      if (res.second) {
        res.first->second.idx = idx++;
        ITRACE(9,
               "  {}: adding trait method {}::{} as {}\n",
               info.rleaf->cls->name,
               method->cls->name, method->name, mdata.name);
      } else {
        if (attrs & AttrAbstract) continue;
        if (res.first->second.func->cls == info.rleaf->cls) continue;
        if (!methodOverride(res.first, method, attrs, mdata.name)) {
          return false;
        }
        res.first->second.idx = idx++;
      }
      info.index.classExtraMethodMap[info.rleaf->cls].insert(
        const_cast<php::Func*>(method));
    }
  } catch (TMIOps::TMIException& ex) {
    ITRACE(2,
           "build_class_methods failed for `{}' importing traits: {}\n",
           info.rleaf->cls->name, ex.what());
    return false;
  }

  return true;
}

bool enforce_in_maybe_sealed_parent_whitelist(
  const ClassInfo* cls,
  const ClassInfo* parent);

bool build_cls_info_rec(BuildClsInfo& info,
                        const ClassInfo* rparent,
                        bool fromTrait) {
  if (!rparent) return true;
  if (!enforce_in_maybe_sealed_parent_whitelist(rparent, rparent->parent)) {
    return false;
  }
  if (!build_cls_info_rec(info, rparent->parent, false)) {
    return false;
  }

  for (auto const iface : rparent->declInterfaces) {
    if (!enforce_in_maybe_sealed_parent_whitelist(rparent, iface)) {
      return false;
    }
    if (!build_cls_info_rec(info, iface, fromTrait)) {
      return false;
    }
  }

  for (auto const trait : rparent->usedTraits) {
    if (!enforce_in_maybe_sealed_parent_whitelist(rparent, trait)) {
      return false;
    }
    if (!build_cls_info_rec(info, trait, true)) return false;
  }

  if (rparent->cls->attrs & AttrInterface) {
    /*
     * Make a flattened table of all the interfaces implemented by the class.
     */
    info.rleaf->implInterfaces[rparent->cls->name] = rparent;
  } else {
    if (!fromTrait &&
        !build_class_properties(info, rparent)) {
      return false;
    }

    // We don't need a method table for interfaces, and rather than
    // building the table recursively from scratch we just use the
    // parent's already constructed method table, and this class's
    // local method table (and traits if necessary).
    if (rparent == info.rleaf) {
      if (!build_class_methods(info)) return false;
    }
  }

  if (!build_class_constants(info, rparent, fromTrait)) return false;

  return true;
}

const StaticString s___Sealed("__Sealed");
bool enforce_in_maybe_sealed_parent_whitelist(
  const ClassInfo* cls,
  const ClassInfo* parent) {
  // if our parent isn't sealed, then we're fine.
  if (!parent || !(parent->cls->attrs & AttrSealed)) return true;
  const UserAttributeMap& parent_attrs = parent->cls->userAttributes;
  assert(parent_attrs.find(s___Sealed.get()) != parent_attrs.end());
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

bool find_constructor(ClassInfo* cinfo) {
  if (cinfo->cls->attrs & (AttrInterface|AttrTrait)) {
    return true;
  }

  auto find_toplevel = [&] (SString name) -> const MethTabEntryPair* {
    auto const cit = cinfo->methods.find(name);
    if (cit != end(cinfo->methods) && cit->second.topLevel) {
      return mteFromIt(cit);
    }
    return nullptr;
  };

  auto const construct       = find_toplevel(s_construct.get());
  auto const named_construct = find_toplevel(cinfo->cls->name);

  if (construct) {
    if (named_construct) {
      // If both constructors exist, and at least one came
      // from a trait we'll fatal at runtime.
      if (named_construct->second.func->cls->attrs & AttrTrait ||
          construct->second.func->cls->attrs & AttrTrait) {
        ITRACE(2,
               "find_constructor failed for `{}' due to colliding constructors"
               "from traits: {} and {}\n",
               named_construct->second.func->cls->name,
               construct->second.func->cls->name);
        return false;
      }
    }
    cinfo->ctor = construct;
    return true;
  }

  if (named_construct) {
    cinfo->ctor = named_construct;
    return true;
  }

  // Parent class constructor if it exists
  if (cinfo->parent && cinfo->parent->ctor) {
    cinfo->ctor = cinfo->parent->ctor;
    return true;
  }

  // We'll use SystemLib::s_nullfunc, but thats equivalent to no constructor
  return true;
}

/*
 * Note: a cyclic inheritance chain will blow this up, but right now
 * we'll never get here in that case because hphpc currently just
 * modifies classes not to have that situation.  TODO(#3649211).
 *
 * This function return false if we are certain instantiating cinfo
 * would be a fatal at runtime.
 */
bool build_cls_info(IndexData& index, ClassInfo* cinfo) {
  auto info = BuildClsInfo{ index, cinfo };
  if (!build_cls_info_rec(info, cinfo, false)) return false;
  if (!find_constructor(cinfo)) return false;
  return true;
}

//////////////////////////////////////////////////////////////////////

void add_system_constants_to_index(IndexData& index) {
  for (auto cnsPair : Native::getConstants()) {
    auto& c = index.constants[cnsPair.first];
    assertx(cnsPair.second.m_type != KindOfUninit ||
            cnsPair.second.dynamic());
    auto t = cnsPair.second.dynamic() ?
      TInitCell : from_cell(cnsPair.second);
    c.func = nullptr;
    c.type = t;
    c.system = true;
    c.readonly = false;
  }
}

//////////////////////////////////////////////////////////////////////

// We want const qualifiers on various index data structures for php
// object pointers, but during index creation time we need to
// manipulate some of their attributes (changing the representation).
// This little wrapper keeps the const_casting out of the main line of
// code below.
void attribute_setter(const Attr& attrs, bool set, Attr attr) {
  attrSetter(const_cast<Attr&>(attrs), set, attr);
}

void add_unit_to_index(IndexData& index, const php::Unit& unit) {
  hphp_hash_map<
    const php::Class*,
    hphp_hash_set<const php::Class*>
  > closureMap;

  for (auto& c : unit.classes) {
    if (c->attrs & AttrEnum) {
      index.enums.insert({c->name, c.get()});
    }

    index.classes.insert({c->name, c.get()});

    for (auto& m : c->methods) {
      attribute_setter(m->attrs, false, AttrNoOverride);
      index.methods.insert({m->name, m.get()});
      if (m->attrs & AttrInterceptable) {
        index.any_interceptable_functions = true;
      }

      if (RuntimeOption::RepoAuthoritative) {
        uint64_t refs = 0, cur = 1;
        bool anyByRef = false;
        for (auto& p : m->params) {
          if (p.byRef) {
            refs |= cur;
            anyByRef = true;
          }
          // It doesn't matter that we lose parameters beyond the 64th,
          // for those, we'll conservatively check everything anyway.
          cur <<= 1;
        }
        if (anyByRef) {
          // Multiple methods with the same name will be combined in the same
          // cell, thus we use |=. This only makes sense in WholeProgram mode
          // since we use this field to check that no functions uses its n-th
          // parameter byref, which requires global knowledge.
          index.method_ref_params_by_name[m->name] |= refs;
        }
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

  for (auto& f : unit.funcs) {
    /*
     * A function can be defined with the same name as a builtin in the
     * repo. Any such attempts will fatal at runtime, so we can safely ignore
     * any such definitions. This ensures that names referring to builtins are
     * always fully resolvable.
     */
    auto const funcs = index.funcs.equal_range(f->name);
    if (funcs.first != funcs.second) {
      auto const& old_func = funcs.first->second;
      // If there is a builtin, it will always be the first (and only) func on
      // the list.
      if (old_func->attrs & AttrBuiltin) {
        always_assert(!(f->attrs & AttrBuiltin));
        continue;
      }
      if (f->attrs & AttrBuiltin) index.funcs.erase(funcs.first, funcs.second);
    }
    if (f->attrs & AttrInterceptable) index.any_interceptable_functions = true;
    index.funcs.insert({f->name, f.get()});
  }

  for (auto& ta : unit.typeAliases) {
    index.typeAliases.insert({ta->name, ta.get()});
  }

  for (auto& ca : unit.classAliases) {
    index.classAliases.insert(ca.first);
    index.classAliases.insert(ca.second);
  }
}

struct NamingEnv {
  NamingEnv(php::Program* program, IndexData& index) :
      program{program}, index{index} {}

  struct Define;
  struct Seen;

  ClassInfo* try_lookup(SString name) const {
    auto const it = names.find(name);
    return it == end(names) ? nullptr : it->second;
  }

  ClassInfo* lookup(SString name) const {
    auto ret = try_lookup(name);
    always_assert(ret && "NamingEnv::lookup failed unexpectedly");
    return ret;
  }

  // Return true when the name has been seen before.
  // This is intended only for checking circular dependencies in
  // pre-resolution time.
  bool seen(SString name) const {
    return names.count(name);
  }

  php::Program*                              program;
  IndexData&                                 index;
private:
  ISStringToOne<ClassInfo>                   names;
};

struct NamingEnv::Seen {

  // Add to the seen set.
  explicit Seen(NamingEnv& env, SString name): env(env), name(name) {
    ITRACE(3, "visiting {}\n", name);
    assert(!env.seen(name));

    // A name can not be simultaneously in "defined" and "visiting" state.
    // When one reaches the "define" case, it is already preresolved, and thus
    // it has been removed from the "visiting" set since we've done visiting it.
    env.names[name] = nullptr;
  }

  // Remove from the seen set when going out-of-scope
  ~Seen() {
    env.names.erase(name);
  }

  // Prevent copying
  Seen(const Seen&)            = delete;
  Seen& operator=(const Seen&) = delete;

private:
  Trace::Indent indent;
  NamingEnv& env;
  SString name;
};

struct NamingEnv::Define {
  explicit Define(NamingEnv& env, SString n, ClassInfo* ci,
                  const php::Class* cls)
      : env(env), n(n) {
    ITRACE(2, "defining {} for {}\n", n, cls->name);
    always_assert(!env.names.count(n));
    env.names[n] = ci;
  }
  ~Define() {
    env.names.erase(n);
  }

  Define(const Define&) = delete;
  Define& operator=(const Define&) = delete;

private:
  Trace::Indent indent;
  NamingEnv& env;
  SString n;
};

using ClonedClosureMap = hphp_hash_map<
  php::Class*,
  std::pair<std::unique_ptr<php::Class>, uint32_t>
>;

std::unique_ptr<php::Func> clone_meth_helper(
  php::Class* newContext,
  const php::Func* origMeth,
  std::unique_ptr<php::Func> cloneMeth,
  std::atomic<uint32_t>& nextFuncId,
  uint32_t& nextClass,
  ClonedClosureMap& clonedClosures);

std::unique_ptr<php::Class> clone_closure(php::Class* newContext,
                                          php::Class* cls,
                                          std::atomic<uint32_t>& nextFuncId,
                                          uint32_t& nextClass,
                                          ClonedClosureMap& clonedClosures) {
  auto clone = std::make_unique<php::Class>(*cls);
  assertx(clone->closureContextCls);
  clone->closureContextCls = newContext;
  clone->unit = newContext->unit;
  auto i = 0;
  for (auto& cloneMeth : clone->methods) {
    cloneMeth = clone_meth_helper(clone.get(),
                                  cls->methods[i++].get(),
                                  std::move(cloneMeth),
                                  nextFuncId,
                                  nextClass,
                                  clonedClosures);
    if (!cloneMeth) return nullptr;
  }
  return clone;
}

std::unique_ptr<php::Func> clone_meth_helper(
  php::Class* newContext,
  const php::Func* origMeth,
  std::unique_ptr<php::Func> cloneMeth,
  std::atomic<uint32_t>& nextFuncId,
  uint32_t& nextClass,
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

  auto const recordClosure = [&] (uint32_t* clsId) {
    auto const cls = origMeth->unit->classes[*clsId].get();
    auto& elm = clonedClosures[cls];
    if (!elm.first) {
      elm.first = clone_closure(newContext->closureContextCls ?
                                newContext->closureContextCls : newContext,
                                cls, nextFuncId, nextClass, clonedClosures);
      if (!elm.first) return false;
      elm.second = nextClass++;
    }
    *clsId = elm.second;
    return true;
  };

  hphp_fast_map<size_t, hphp_fast_map<size_t, uint32_t>> updates;
  for (size_t bid = 0; bid < cloneMeth->blocks.size(); bid++) {
    auto const b = cloneMeth->blocks[bid].get();
    for (size_t ix = 0; ix < b->hhbcs.size(); ix++) {
      auto const& bc = b->hhbcs[ix];
      switch (bc.op) {
        case Op::CreateCl: {
          auto clsId = bc.CreateCl.arg2;
          if (!recordClosure(&clsId)) return nullptr;
          updates[bid][ix] = clsId;
          break;
        }
        case Op::DefCls:
        case Op::DefClsNop:
          return nullptr;
        default:
          break;
      }
    }
  }

  for (auto elm : updates) {
    auto& cblk = cloneMeth->blocks[elm.first];
    auto const blk = cblk.mutate();
    for (auto const& ix : elm.second) {
      blk->hhbcs[ix.first].CreateCl.arg2 = ix.second;
    }
  }

  return cloneMeth;
}

std::unique_ptr<php::Func> clone_meth(php::Class* newContext,
                                      const php::Func* origMeth,
                                      SString name,
                                      Attr attrs,
                                      std::atomic<uint32_t>& nextFuncId,
                                      uint32_t& nextClass,
                                      ClonedClosureMap& clonedClosures) {

  auto cloneMeth  = std::make_unique<php::Func>(*origMeth);
  cloneMeth->name = name;
  cloneMeth->attrs = attrs | AttrTrait;
  return clone_meth_helper(newContext, origMeth, std::move(cloneMeth),
                           nextFuncId, nextClass, clonedClosures);
}

bool merge_xinits(Attr attr,
                  std::vector<std::unique_ptr<php::Func>>& clones,
                  ClassInfo* cinfo,
                  std::atomic<uint32_t>& nextFuncId,
                  uint32_t& nextClass,
                  ClonedClosureMap& clonedClosures) {
  auto const cls = const_cast<php::Class*>(cinfo->cls);
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

  auto const needsXinit = [&] {
    for (auto const& p : cinfo->traitProps) {
      if (xinitMatch(p.attrs) &&
          p.val.m_type == KindOfUninit &&
          !(p.attrs & AttrLateInit)) {
        ITRACE(5, "merge_xinits: {}: Needs merge for {}{}prop `{}'\n",
               cls->name, attr & AttrStatic ? "static " : "",
               attr & AttrLSB ? "lsb " : "", p.name);
        return true;
      }
    }
    return false;
  }();

  if (!needsXinit) return true;

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
      xinit = clone_meth(cls, func, func->name, func->attrs, nextFuncId,
                         nextClass, clonedClosures);
      return xinit != nullptr;
    }

    ITRACE(5, "  - appending {}::{} into {}::{}\n",
           func->cls->name, func->name, cls->name, xinitName);
    return append_func(xinit.get(), *func);
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
    DEBUG_ONLY auto res = cinfo->methods.emplace(
      xinit->name,
      MethTabEntry { xinit.get(), xinit->attrs, false, true }
    );
    assertx(res.second);
    clones.push_back(std::move(xinit));
  }

  return true;
}

void rename_closure(NamingEnv& env, php::Class* cls) {
  auto n = cls->name->toCppString();
  auto p = n.find(';');
  int id = 0;
  if (p != std::string::npos) {
    id = atoi(n.c_str() + p + 1);
    if (id < 0) id = 0;
    n = n.substr(0, p);
  }
  while (id < INT_MAX) {
    auto const newName = makeStaticString(folly::sformat("{};{}", n, ++id));
    if (env.index.classes.count(newName)) continue;
    cls->name = newName;
    env.index.classes.emplace(newName, cls);
    return;
  }
}

void flatten_traits(NamingEnv& env, ClassInfo* cinfo) {
  for (auto t : cinfo->usedTraits) {
    if (t->usedTraits.size() && !(t->cls->attrs & AttrNoExpandTrait)) {
      ITRACE(5, "Not flattening {} because of {}\n",
             cinfo->cls->name, t->cls->name);
      return;
    }
  }
  auto const cls = const_cast<php::Class*>(cinfo->cls);
  std::vector<MethTabEntryPair*> methodsToAdd;
  for (auto& ent : cinfo->methods) {
    if (!ent.second.topLevel || ent.second.func->cls == cinfo->cls) {
      continue;
    }
    always_assert(ent.second.func->cls->attrs & AttrTrait);
    methodsToAdd.push_back(mteFromElm(ent));
  }

  auto const it = env.index.classExtraMethodMap.find(cinfo->cls);

  if (!methodsToAdd.empty()) {
    assertx(it != env.index.classExtraMethodMap.end());
    std::sort(begin(methodsToAdd), end(methodsToAdd),
              [] (const MethTabEntryPair* a, const MethTabEntryPair* b) {
                return a->second.idx < b->second.idx;
              });
  } else if (debug && it != env.index.classExtraMethodMap.end()) {
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
  uint32_t nextClassId = cls->unit->classes.size();
  for (auto const ent : methodsToAdd) {
    auto clone = clone_meth(cls, ent->second.func, ent->first,
                            ent->second.attrs, env.program->nextFuncId,
                            nextClassId, clonedClosures);
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
    if (!merge_xinits(AttrNone, clones, cinfo,
                      env.program->nextFuncId, nextClassId, clonedClosures) ||
        !merge_xinits(AttrStatic, clones, cinfo,
                      env.program->nextFuncId, nextClassId, clonedClosures) ||
        !merge_xinits(AttrLSB, clones, cinfo,
                      env.program->nextFuncId, nextClassId, clonedClosures)) {
      ITRACE(5, "Not flattening {} because we couldn't merge the 86xinits\n",
             cls->name);
      return;
    }
  }

  // We're now committed to flattening.
  ITRACE(3, "Flattening {}\n", cls->name);
  if (it != env.index.classExtraMethodMap.end()) it->second.clear();
  for (auto const& p : cinfo->traitProps) {
    ITRACE(5, "  - prop {}\n", p.name);
    cls->properties.push_back(p);
    cls->properties.back().attrs |= AttrTrait;
  }
  cinfo->traitProps.clear();

  if (clones.size()) {
    auto cinit = cls->methods.size() &&
      cls->methods.back()->name == s_86cinit.get() ?
      std::move(cls->methods.back()) : nullptr;
    if (cinit) cls->methods.pop_back();
    for (auto& clone : clones) {
      ITRACE(5, "  - meth {}\n", clone->name);
      cinfo->methods.find(clone->name)->second.func = clone.get();
      cls->methods.push_back(std::move(clone));
    }
    if (cinit) cls->methods.push_back(std::move(cinit));

    if (clonedClosures.size()) {
      auto& classClosures = env.index.classClosureMap[cls];
      cls->unit->classes.resize(nextClassId);
      for (auto& ent : clonedClosures) {
        auto const clo = ent.second.first.get();
        rename_closure(env, clo);
        ITRACE(5, "  - closure {} as {}\n", ent.first->name, clo->name);
        assertx(clo->closureContextCls == cls);
        assertx(clo->unit == cls->unit);
        classClosures.push_back(clo);

        cls->unit->classes[ent.second.second] = std::move(ent.second.first);
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

void resolve_combinations(NamingEnv& env,
                          const php::Class* cls) {

  auto resolve_one = [&] (SString name) {
    if (env.try_lookup(name)) return true;
    auto any = false;
    for (auto& kv : copy_range(env.index.classInfo, name)) {
      NamingEnv::Define def{env, name, kv.second, cls};
      resolve_combinations(env, cls);
      any = true;
    }
    if (!any) {
      ITRACE(2,
             "Resolve combinations failed for `{}' because "
             "there were no resolutions of `{}'\n",
             cls->name, name);
    }
    return false;
  };

  // Recurse with all combinations of bases and interfaces in the
  // naming environment.
  if (cls->parentName) {
    if (!resolve_one(cls->parentName)) return;
  }
  for (auto& iname : cls->interfaceNames) {
    if (!resolve_one(iname)) return;
  }
  for (auto& tname : cls->usedTraitNames) {
    if (!resolve_one(tname)) return;
  }

  // Everything is defined in the naming environment here.  (We
  // returned early if something didn't exist.)

  auto cinfo = std::make_unique<ClassInfo>();
  cinfo->cls = cls;
  if (cls->parentName) {
    cinfo->parent   = env.lookup(cls->parentName);
    cinfo->baseList = cinfo->parent->baseList;
    if (cinfo->parent->cls->attrs & (AttrInterface | AttrTrait)) {
      ITRACE(2,
             "Resolve combinations failed for `{}' because "
             "its parent `{}' is not a class\n",
             cls->name, cls->parentName);
      return;
    }
  }
  cinfo->baseList.push_back(cinfo.get());

  for (auto& iname : cls->interfaceNames) {
    auto const iface = env.lookup(iname);
    if (!(iface->cls->attrs & AttrInterface)) {
      ITRACE(2,
             "Resolve combinations failed for `{}' because `{}' "
             "is not an interface\n",
             cls->name, iname);
      return;
    }
    cinfo->declInterfaces.push_back(iface);
  }

  for (auto& tname : cls->usedTraitNames) {
    auto const trait = env.lookup(tname);
    if (!(trait->cls->attrs & AttrTrait)) {
      ITRACE(2,
             "Resolve combinations failed for `{}' because `{}' "
             "is not a trait\n",
             cls->name, tname);
      return;
    }
    cinfo->usedTraits.push_back(trait);
  }

  if (!build_cls_info(env.index, cinfo.get())) return;

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
  env.index.classInfo.emplace(cls->name, cinfo.get());
  env.index.allClassInfos.push_back(std::move(cinfo));
}

void preresolve(NamingEnv& env, SString clsName) {
  if (env.index.classInfo.count(clsName)) return;

  ITRACE(2, "preresolve: {}\n", clsName);
  if (env.seen(clsName)) {
    ITRACE(3, "Circular inheritance detected: {}\n", clsName);
    return;
  }
  NamingEnv::Seen seen(env, clsName);
  {
    Trace::Indent indent;
    auto process_one = [&] (const php::Class* cls) {
      if (cls->parentName) {
        preresolve(env, cls->parentName);
      }
      for (auto& i : cls->interfaceNames) {
        preresolve(env, i);
      }
      for (auto& t : cls->usedTraitNames) {
        preresolve(env, t);
      }
      resolve_combinations(env, cls);
    };
    auto const classRange = find_range(env.index.classes, clsName);
    [&] {
      if (begin(classRange) == end(classRange)) {
        return;
      }
      if (std::next(begin(classRange)) == end(classRange)) {
        return process_one(begin(classRange)->second);
      }
      for (auto& kv : classRange) {
        if (is_systemlib_part(*kv.second->unit)) {
          return process_one(kv.second);
        }
      }
      for (auto& kv : classRange) {
        process_one(kv.second);
      }
    }();
  }

  ITRACE(3, "preresolve: {} ({} resolutions)\n",
         clsName, env.index.classInfo.count(clsName));

  if (options.FlattenTraits) {
    auto const range = find_range(env.index.classInfo, clsName);
    if (begin(range) != end(range) && std::next(begin(range)) == end(range)) {
      Trace::Indent indent;
      auto const cinfo = begin(range)->second;
      if (!(cinfo->cls->attrs & AttrNoExpandTrait) &&
          !cinfo->usedTraits.empty()) {
        flatten_traits(env, cinfo);
      }
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

void compute_subclass_list(IndexData& index) {
  auto fixupTraits = false;
  for (auto& cinfo : index.allClassInfos) {
    for (auto& cparent : cinfo->baseList) {
      cparent->subclassList.push_back(cinfo.get());
    }
    if (!(cinfo->cls->attrs & AttrNoExpandTrait) &&
        cinfo->usedTraits.size()) {
      fixupTraits = true;
      compute_subclass_list_rec(index, cinfo.get(), cinfo.get());
    }
  }

  for (auto& cinfo : index.allClassInfos) {
    auto& sub = cinfo->subclassList;
    if (fixupTraits && cinfo->cls->attrs & AttrTrait) {
      // traits can be reached by multiple paths, so we need to uniquify
      // their subclassLists.
      std::sort(begin(sub), end(sub));
      sub.erase(
        std::unique(begin(sub), end(sub)),
        end(sub)
      );
    }
    sub.shrink_to_fit();
  }
}

void define_func_family(IndexData& index, ClassInfo* cinfo,
                        SString name, const MethTabEntryPair* mte) {
  FuncFamily::PFuncVec funcs{};
  auto containsInterceptables = false;
  auto const isCtor = mte == cinfo->ctor;
  for (auto& cleaf : cinfo->subclassList) {
    auto const leafFn = [&] () -> const MethTabEntryPair* {
      if (isCtor) return cleaf->ctor;
      auto const leafFnIt = cleaf->methods.find(name);
      if (leafFnIt == end(cleaf->methods)) return nullptr;
      return mteFromIt(leafFnIt);
    }();
    if (!leafFn) continue;
    if (leafFn->second.func->attrs & AttrInterceptable) {
      containsInterceptables = true;
    }
    funcs.push_back(leafFn);
  }

  std::sort(begin(funcs), end(funcs),
            [&] (const MethTabEntryPair* a, const MethTabEntryPair* b) {
              // We want a canonical order for the family. Putting the
              // one corresponding to cinfo first makes sense, because
              // the first one is used as the name for FCall hint, after
              // that, sort by name so that different case spellings
              // come in the same order.
              if (a->second.func == b->second.func)   return false;
              if (b->second.func == mte->second.func) return false;
              if (a->second.func == mte->second.func) return true;
              if (auto d = a->first->compare(b->first)) {
                return d < 0;
              }
              return std::less<const void*>{}(a->second.func, b->second.func);
            });
  funcs.erase(
    std::unique(begin(funcs), end(funcs),
                [] (const MethTabEntryPair* a, const MethTabEntryPair* b) {
                  return a->second.func == b->second.func;
                }),
    end(funcs)
  );

  /*
   * Note: right now abstract functions are part of the family.
   *
   * They have bytecode bodies that just fatal, so it won't hurt
   * return type inference (we'll just add an extra TBottom to the
   * return type union).  It can hurt parameter reffiness if the
   * abstract function is declared with reffiness that doesn't match
   * the overloads---it's still actually possible to call the abstract
   * function in some cases, including in a late-bound context if it
   * is static, so the reffiness declaration on the abstract method
   * actually matters.  It could be avoided on an instance---but we're
   * not trying to notice those cases right now.
   *
   * For now we're leaving that alone, which gives the invariant that
   * every FuncFamily we create here is non-empty.
   */
  always_assert(!funcs.empty());
  funcs.shrink_to_fit();

  cinfo->methodFamilies.emplace(
    std::piecewise_construct,
    std::forward_as_tuple(name),
    std::forward_as_tuple(std::move(funcs),
                          containsInterceptables,
                          isCtor)
  );
}

void define_func_families(IndexData& index) {
  for (auto& cinfo : index.allClassInfos) {
    if (cinfo->cls->attrs & AttrTrait) continue;
    auto didCtor = false;
    for (auto& kv : cinfo->methods) {
      auto const mte = mteFromElm(kv);

      if (mte->second.attrs & AttrNoOverride) continue;
      if (mte == cinfo->ctor) {
        define_func_family(index, cinfo.get(), s_construct.get(), mte);
        didCtor = true;
        continue;
      }
      if (mte->second.attrs & AttrPrivate) continue;
      if (is_special_method_name(mte->first)) continue;

      define_func_family(index, cinfo.get(), mte->first, mte);
    }
    if (cinfo->ctor && !didCtor) {
      define_func_family(index, cinfo.get(), s_construct.get(), cinfo->ctor);
    }
  }
}

/*
 * ConflictGraph maintains lists of interfaces that conflict with each other
 * due to being implemented by the same class.
 */
struct ConflictGraph {
  void add(const php::Class* i, const php::Class* j) {
    if (i == j) return;
    auto& conflicts = map[i];
    if (std::find(conflicts.begin(), conflicts.end(), j) != conflicts.end()) {
      return;
    }
    conflicts.push_back(j);
  }

  hphp_hash_map<const php::Class*,
                     std::vector<const php::Class*>> map;
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
    if (cinfo->cls->attrs & (AttrTrait | AttrEnum | AttrAbstract)) continue;

    classes.emplace_back(Cls{cinfo.get()});
    auto& vtable = classes.back().vtable;
    for (auto& pair : cinfo->implInterfaces) {
      auto it = index.ifaceSlotMap.find(pair.second->cls);
      assert(it != end(index.ifaceSlotMap));
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
  auto const& conflicts = cg.map.at(iface);
  if (conflicts.empty()) {
    // No conflicts. This is the only interface implemented by the classes that
    // implement it.
    return 0;
  }

  boost::dynamic_bitset<> used;

  for (auto& c : conflicts) {
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

    // Only worry about classes that can be instantiated. If an abstract class
    // has any concrete subclasses, those classes will make sure the right
    // entries are in the conflict sets.
    if (cinfo->cls->attrs & (AttrTrait | AttrEnum | AttrAbstract)) continue;

    for (auto& ipair : cinfo->implInterfaces) {
      ++iface_uses[ipair.second->cls];
      index.ifaceImplementerMap[ipair.second->cls].push_back(cinfo.get());
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
    assert(slot_uses.count(slot));
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
  for (auto& kv : magicMethodMap) {
    if ((derived.*kv.second.pmem).thisHas) {
      auto& derivedHas = (cinfo.*kv.second.pmem).derivedHas;
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
  for (auto& cinfo : index.allClassInfos) {
    bool any = false;
    for (auto& kv : magicMethodMap) {
      bool const found = has_magic_method(cinfo.get(), kv.first);
      any = any || found;
      (cinfo.get()->*kv.second.pmem).thisHas = found;
    }
    if (any) mark_magic_on_parents(*cinfo, *cinfo);
  }
}

void find_mocked_classes(IndexData& index) {
  for (auto& cinfo : index.allClassInfos) {
    if (is_mock_class(cinfo->cls) && cinfo->parent) {
      cinfo->parent->isMocked = true;
      for (auto c = cinfo->parent; c; c = c->parent) {
        c->isDerivedMocked = true;
      }
    }
  }
}

void mark_no_override_classes(IndexData& index) {
  for (auto& cinfo : index.allClassInfos) {
    auto const set = [&] {
      if (!(cinfo->cls->attrs & AttrUnique) ||
          cinfo->cls->attrs & AttrInterface) {
        return false;
      }
      return cinfo->subclassList.size() == 1;
    }();
    attribute_setter(cinfo->cls->attrs, set, AttrNoOverride);

    for (auto& kv : magicMethodMap) {
      if (kv.second.attrBit == AttrNone) continue;
      if (!(cinfo.get()->*kv.second.pmem).derivedHas) {
        FTRACE(2, "Adding no-override of {} to {}\n",
          kv.first->data(),
          cinfo->cls->name);
        attribute_setter(cinfo->cls->attrs, true, kv.second.attrBit);
      }
    }
  }
}

void mark_no_override_methods(IndexData& index) {
  // We removed any AttrNoOverride flags from all methods while adding
  // the units to the index.  Now start by marking every
  // (non-interface, non-special) method as AttrNoOverride.
  for (auto& cinfo : index.allClassInfos) {
    if (cinfo->cls->attrs & AttrInterface) continue;
    if (!(cinfo->cls->attrs & AttrUnique)) continue;

    for (auto& m : cinfo->methods) {
      if (!(is_special_method_name(m.first))) {
        FTRACE(9, "Pre-setting AttrNoOverride on {}::{}\n",
               m.second.func->cls->name, m.first);
        attribute_setter(m.second.attrs, true, AttrNoOverride);
        attribute_setter(m.second.func->attrs, true, AttrNoOverride);
      }
    }
  }

  // Then run through every ClassInfo, and for each of its parent classes clear
  // the AttrNoOverride flag if it has a different Func with the same name.
  for (auto& cinfo : index.allClassInfos) {
    for (auto& ancestor : cinfo->baseList) {
      if (ancestor == cinfo.get()) continue;

      auto removeNoOverride = [] (auto it) {
        assertx(it->second.attrs & AttrNoOverride ||
                !(it->second.func->attrs & AttrNoOverride));
        if (it->second.attrs & AttrNoOverride) {
          FTRACE(2, "Removing AttrNoOverride on {}::{}\n",
                 it->second.func->cls->name, it->first);
          attribute_setter(it->second.attrs, false, AttrNoOverride);
          attribute_setter(it->second.func->attrs, false, AttrNoOverride);
        }
      };

      for (auto& derivedMethod : cinfo->methods) {
        if (&derivedMethod == cinfo->ctor) {
          if (ancestor->ctor && ancestor->ctor != cinfo->ctor) {
            removeNoOverride(ancestor->ctor);
          }
        } else {
          auto const it = ancestor->methods.find(derivedMethod.first);
          if (it == end(ancestor->methods)) continue;
          if (it->second.func != derivedMethod.second.func) {
            removeNoOverride(it);
          }
        }
      }
    }
  }
}

template <class T, class F>
void mark_unique_entities(ISStringToMany<T>& entities, F marker) {
  for (auto it = entities.begin(), end = entities.end(); it != end; ) {
    auto first = it++;
    auto flag = true;
    while (it != end && it->first->isame(first->first)) {
      marker(it++->second, false);
      flag = false;
    }
    marker(first->second, flag);
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
  folly::F14FastSet<const php::Class*> needsinit;

  // Find all classes that still need their 86reifiedinit methods
  for (auto& cinfo : index.allClassInfos) {
    auto ual = cinfo->cls->userAttributes;
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
      FTRACE(2, "Adding AttrNoReifiedInit on class {}", cinfo->cls->name);
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
      if (cinfo->ctor && m.get() == cinfo->ctor->second.func) {
        always_assert(cinfo->methodFamilies.count(s_construct.get()));
        continue;
      }
      if (is_special_method_name(m->name)) continue;
      always_assert(cinfo->methodFamilies.count(m->name));
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

  for (auto& kv : magicMethodMap) {
    auto& info = cinfo->*kv.second.pmem;

    // Magic method flags should be consistent with the method table.
    always_assert(info.thisHas == has_magic_method(cinfo, kv.first));

    // Non-'derived' flags (thisHas) about magic methods imply the derived
    // ones.
    always_assert(!info.thisHas || info.derivedHas);
  }

  // Every FuncFamily is non-empty and contain functions with the same
  // name (unless its a family of ctors).
  for (auto const& mfam: cinfo->methodFamilies) {
    always_assert(!mfam.second.possibleFuncs()->empty());
    if (mfam.second.isCtor()) continue;
    auto const name = mfam.second.possibleFuncs()->front()->first;
    for (auto const pf : mfam.second.possibleFuncs()) {
      always_assert(pf->first->isame(name));
    }
  }
}

void check_invariants(IndexData& data) {
  if (!debug) return;

  // Every AttrUnique non-trait class has a unique ClassInfo object,
  // or no ClassInfo object in the case that instantiating it would've
  // fataled.
  for (auto& kv : data.classes) {
    auto const name = kv.first;
    auto const cls  = kv.second;
    if (!(cls->attrs & AttrUnique)) continue;

    auto const range = find_range(data.classInfo, name);
    if (begin(range) != end(range)) {
      always_assert(std::next(begin(range)) == end(range));
    }
  }

  for (auto& cinfo : data.allClassInfos) {
    check_invariants(cinfo.get());
  }
}

//////////////////////////////////////////////////////////////////////

Type context_sensitive_return_type(IndexData& data,
                                   CallContext callCtx) {
  constexpr auto max_interp_nexting_level = 2;
  static __thread uint32_t interp_nesting_level;
  auto const finfo = func_info(data, callCtx.callee);
  auto const returnType = return_with_context(finfo->returnTy, callCtx.context);

  auto checkParam = [&] (int i) {
    if (RuntimeOption::EvalHardTypeHints) {
      auto const constraint = finfo->func->params[i].typeConstraint;
      if (constraint.hasConstraint() &&
          !constraint.isTypeVar() &&
          !constraint.isTypeConstant()) {
        auto ctx = Context {
          finfo->func->unit,
          const_cast<php::Func*>(finfo->func),
          finfo->func->cls
        };
        auto t = loosen_dvarrayness(
          data.m_index->lookup_constraint(ctx, constraint));
        if (!callCtx.args[i].moreRefined(t)) return true;
        if (!callCtx.args[i].equivalentlyRefined(t)) return true;
        return false;
      }
    }
    return callCtx.args[i].strictSubtypeOf(TInitCell);
  };

  // TODO(#3788877): more heuristics here would be useful.
  bool const tryContextSensitive = [&] {
    if (finfo->func->params.empty() ||
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

  auto maybe_loosen_staticness = [&] (const Type& ty) {
    return returnType.subtypeOf(BUnc) ? ty : loosen_staticness(ty);
  };

  {
    ContextRetTyMap::const_accessor acc;
    if (data.contextualReturnTypes.find(acc, callCtx)) {
      if (data.frozen || acc->second == TBottom || is_scalar(acc->second)) {
        return maybe_loosen_staticness(acc->second);
      }
    }
  }

  if (data.frozen) {
    return returnType;
  }

  auto contextType = [&] {
    ++interp_nesting_level;
    SCOPE_EXIT { --interp_nesting_level; };

    auto const calleeCtx = Context {
      finfo->func->unit,
      const_cast<php::Func*>(finfo->func),
      finfo->func->cls
    };
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

  auto ret = intersection_of(std::move(returnType),
                             std::move(contextType));

  ContextRetTyMap::accessor acc;
  if (data.contextualReturnTypes.insert(acc, callCtx) ||
      ret.strictSubtypeOf(acc->second)) {
    acc->second = ret;
  }

  if (!interp_nesting_level) {
    ret = maybe_loosen_staticness(ret);
    FTRACE(3, "Context sensitive result: {}\n", show(ret));
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

PrepKind func_param_prep(const php::Func* func,
                         uint32_t paramId) {
  if (func->attrs & AttrInterceptable) return PrepKind::Unknown;
  if (paramId >= func->params.size()) {
    if (func->attrs & AttrVariadicByRef) {
      return PrepKind::Ref;
    }
    return PrepKind::Val;
  }
  return func->params[paramId].byRef ? PrepKind::Ref : PrepKind::Val;
}

template<class PossibleFuncRange>
PrepKind prep_kind_from_set(PossibleFuncRange range, uint32_t paramId) {

  /*
   * In sinlge-unit mode, the range is not complete. Without konwing all
   * possible resolutions, HHBBC cannot deduce anything about by-ref vs by-val.
   * So the caller should make sure not calling this in single-unit mode.
   */
  assert(RuntimeOption::RepoAuthoritative);

  if (begin(range) == end(range)) {
    /*
     * We can assume it's by value, because either we're calling a function
     * that doesn't exist (about to fatal), or we're going to an __call (which
     * never takes parameters by reference).
     *
     * Or if we've got AllFuncsInterceptable we need to assume someone could
     * rename a function to the new name.
     */
    return RuntimeOption::EvalJitEnableRenameFunction ?
      PrepKind::Unknown : PrepKind::Val;
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
    case PrepKind::Ref:
      if (prep && *prep != PrepKind::Ref) return PrepKind::Unknown;
      prep = PrepKind::Ref;
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

PublicSPropEntry lookup_public_static_impl(
  const IndexData& data,
  const ClassInfo* cinfo,
  SString prop
) {
  auto const noInfo = PublicSPropEntry{TInitGen, TInitGen, nullptr, 0, true};

  if (data.allPublicSPropsUnknown) return noInfo;

  const ClassInfo* knownCInfo = nullptr;
  auto const knownClsPart = visit_parent_cinfo(
    cinfo,
    [&] (const ClassInfo* ci) -> const PublicSPropEntry* {
      auto const it = ci->publicStaticProps.find(prop);
      if (it != end(ci->publicStaticProps)) {
        knownCInfo = ci;
        return &it->second;
      }
      return nullptr;
    }
  );

  auto const unkPart = [&]() -> const Type* {
    auto unkIt = data.unknownClassSProps.find(prop);
    if (unkIt != end(data.unknownClassSProps)) {
      return &unkIt->second.first;
    }
    return nullptr;
  }();

  if (knownClsPart == nullptr) {
    return noInfo;
  }

  // NB: Inferred type can be TBottom here if the property is never set to a
  // value which can satisfy its type constraint. Such properties can't exist at
  // runtime.

  if (unkPart != nullptr) {
    return PublicSPropEntry {
      union_of(
        knownClsPart->inferredType,
        *unkPart
      ),
      knownClsPart->initializerType,
      nullptr,
      0,
      true
    };
  }
  return *knownClsPart;
}

PublicSPropEntry lookup_public_static_impl(
  const IndexData& data,
  const php::Class* cls,
  SString name
) {
  auto const classes = find_range(data.classInfo, cls->name);
  if (begin(classes) == end(classes) ||
      std::next(begin(classes)) != end(classes)) {
    return PublicSPropEntry{TInitGen, TInitGen, nullptr, 0, true};
  }
  return lookup_public_static_impl(data, begin(classes)->second, name);
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

  if (!prop) return TGen;
  // Make sure its non-static and public. Otherwise its another function's
  // problem.
  if (prop->attrs & (AttrStatic | AttrPrivate | AttrLateInitSoft)) return TGen;

  // Get a type corresponding to its declared type-hint (if any).
  auto ty = adjust_type_for_prop(
    *data.m_index, *knownCls, &prop->typeConstraint, TGen
  );
  // We might have to include the initial value which might be outside of the
  // type-hint.
  auto initialTy = loosen_all(from_cell(prop->val));
  if (!initialTy.subtypeOf(TUninit) && (prop->attrs & AttrSystemInitialValue)) {
    ty |= initialTy;
  }
  return ty;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

Index::Index(php::Program* program,
             rebuild* rebuild_exception)
  : m_data(std::make_unique<IndexData>(this))
{
  trace_time tracer("create index");

  m_data->arrTableBuilder.reset(new ArrayTypeTable::Builder());

  add_system_constants_to_index(*m_data);

  if (rebuild_exception) {
    for (auto& ca : rebuild_exception->class_aliases) {
      m_data->classAliases.insert(ca.first);
      m_data->classAliases.insert(ca.second);
    }
    rebuild_exception->class_aliases.clear();
  }

  for (auto& u : program->units) {
    add_unit_to_index(*m_data, *u);
  }

  {
    NamingEnv env{program, *m_data};
    for (auto& u : program->units) {
      Trace::Bump bumper{Trace::hhbbc, kSystemLibBump, is_systemlib_part(*u)};
      // iterate by index, because preresolve can add closures to the
      // end of u->classes via flatten_traits (which need to be
      // visited after they're added).
      for (size_t idx = 0; idx < u->classes.size(); idx++) {
        auto const c = u->classes[idx].get();
        // Classes with no possible resolutions won't get visited in the
        // mark_persistent pass; make sure everything starts off with
        // the attributes clear.
        attrSetter(c->attrs, false, AttrUnique | AttrPersistent);

        // Manually set closure classes to be unique to maintain invariance.
        if (is_closure(*c)) {
          attrSetter(c->attrs, true, AttrUnique);
        }
        preresolve(env, c->name);
      }
    }
  }

  mark_unique_entities(m_data->typeAliases,
                       [&] (const php::TypeAlias* ta, bool flag) {
                         attribute_setter(
                           ta->attrs,
                           flag &&
                           !m_data->classInfo.count(ta->name) &&
                           !m_data->classAliases.count(ta->name),
                           AttrUnique);
                       });

  // Iterate allClassInfos so that we visit parent classes before
  // child classes.
  for (auto& cinfo : m_data->allClassInfos) {
    auto const set = [&] {
      if (m_data->classInfo.count(cinfo->cls->name) != 1 ||
          m_data->typeAliases.count(cinfo->cls->name) ||
          m_data->classAliases.count(cinfo->cls->name)) {
        return false;
      }
      if (cinfo->parent && !(cinfo->parent->cls->attrs & AttrUnique)) {
        return false;
      }
      for (auto const i : cinfo->declInterfaces) {
        if (!(i->cls->attrs & AttrUnique)) return false;
      }
      for (auto const t : cinfo->usedTraits) {
        if (!(t->cls->attrs & AttrUnique)) return false;
      }
      return true;
    }();
    attribute_setter(cinfo->cls->attrs, set, AttrUnique);
  }

  mark_unique_entities(m_data->funcs,
                       [&] (const php::Func* func, bool flag) {
                         attribute_setter(func->attrs, flag, AttrUnique);
                       });

  m_data->funcInfo.resize(program->nextFuncId);

  // Part of the index building routines happens before the various asserted
  // index invariants hold.  These each may depend on computations from
  // previous functions, so be careful changing the order here.
  compute_subclass_list(*m_data);
  clean_86reifiedinit_methods(*m_data); // uses the base class lists
  mark_no_override_methods(*m_data);    // uses AttrUnique
  define_func_families(*m_data);        // uses AttrNoOverride functions
  find_magic_methods(*m_data);          // uses the subclass lists
  find_mocked_classes(*m_data);
  compute_iface_vtables(*m_data);
  build_iface_methods(*m_data);

  check_invariants(*m_data);

  mark_no_override_classes(*m_data);    // uses AttrUnique

  if (RuntimeOption::EvalCheckReturnTypeHints == 3) {
    trace_time tracer("initialize return types");
    std::vector<const php::Func*> all_funcs;
    all_funcs.reserve(m_data->funcs.size() + m_data->methods.size());
    for (auto const fn : m_data->funcs) {
      all_funcs.push_back(fn.second);
    }
    for (auto const fn : m_data->methods) {
      all_funcs.push_back(fn.second);
    }

    parallel::for_each(all_funcs, [&] (const php::Func* f) {
      init_return_type(f);
    });
  }
}

// Defined here so IndexData is a complete type for the unique_ptr
// destructor.
Index::~Index() {}

//////////////////////////////////////////////////////////////////////

void Index::mark_persistent_classes_and_functions(php::Program& program) {
  for (auto& unit : program.units) {
    for (auto& f : unit->funcs) {
      attribute_setter(f->attrs,
                       unit->persistent && (f->attrs & AttrUnique),
                       AttrPersistent);
    }

    for (auto& t : unit->typeAliases) {
      attribute_setter(t->attrs,
                       unit->persistent && (t->attrs & AttrUnique),
                       AttrPersistent);
    }
  }

  auto check_persistent = [&] (const ClassInfo& cinfo) {
    if (cinfo.parent && !(cinfo.parent->cls->attrs & AttrPersistent)) {
      return false;
    }

    for (auto const intrf : cinfo.declInterfaces) {
      if (!(intrf->cls->attrs & AttrPersistent)) return false;
    }

    return true;
  };

  for (auto& c : m_data->allClassInfos) {
    attribute_setter(c->cls->attrs,
                     (c->cls->unit->persistent ||
                      c->cls->parentName == s_Closure.get()) &&
                     (c->cls->attrs & AttrUnique) &&
                     check_persistent(*c),
                     AttrPersistent);
  }
}

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
      auto const equiv = [&] {
        auto const& tc1 = prop->typeConstraint;
        auto const& tc2 = pprop->typeConstraint;
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
  for (auto& info : find_range(m_data->classInfo, cls.name)) {
    auto const cinfo = info.second;
    if (cinfo->cls != &cls) continue;
    cinfo->hasBadRedeclareProp = possibleOverride;
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

bool Index::register_class_alias(SString orig, SString alias) const {
  auto check = [&] (SString name) {
    if (m_data->classAliases.count(name)) return true;

    auto const classes = find_range(m_data->classInfo, name);
    if (begin(classes) != end(classes)) {
      return !(begin(classes)->second->cls->attrs & AttrUnique);
    }
    auto const tas = find_range(m_data->typeAliases, name);
    if (begin(tas) == end(tas)) return true;
    return !(begin(tas)->second->attrs & AttrUnique);
  };
  if (check(orig) && check(alias)) return true;
  if (m_data->ever_frozen) return false;
  std::lock_guard<std::mutex> lock{m_data->pending_class_aliases_mutex};
  m_data->pending_class_aliases.emplace_back(orig, alias);
  return true;
}

void Index::update_class_aliases() {
  if (m_data->pending_class_aliases.empty()) return;
  FTRACE(1, "Index needs rebuilding due to {} class aliases\n",
         m_data->pending_class_aliases.size());
  throw rebuild { std::move(m_data->pending_class_aliases) };
}

const CompactVector<const php::Class*>*
Index::lookup_closures(const php::Class* cls) const {
  auto const it = m_data->classClosureMap.find(cls);
  if (it != end(m_data->classClosureMap)) {
    return &it->second;
  }
  return nullptr;
}

const hphp_fast_set<php::Func*>*
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

  ClassInfo* result = nullptr;

  auto const classes = find_range(m_data->classInfo, cls->name);
  for (auto it = begin(classes); it != end(classes); ++it) {
    auto const cinfo = it->second;
    if (cinfo->cls == cls) {
      if (result) {
        result = nullptr;
        break;
      }
      result = cinfo;
    }
  }

  // The function is supposed to return a cinfo if we can uniquely resolve cls.
  // In repo mode, if there is only one cinfo, return it.
  // In non-repo mode, we don't know all the cinfo's. So "only one cinfo" does
  // not mean anything unless it is a built-in and we disable rename/intercept.
  if (result && (RuntimeOption::RepoAuthoritative ||
                 (!RuntimeOption::EvalJitEnableRenameFunction &&
                  cls->attrs & AttrBuiltin))) {
    return res::Class { this, result };
  }

  // We know its a class, not an enum or type alias, so return
  // by name
  return res::Class { this, cls->name.get() };
}

folly::Optional<res::Class> Index::resolve_class(Context ctx,
                                                 SString clsName) const {
  clsName = normalizeNS(clsName);

  if (ctx.cls && ctx.cls->name->isame(clsName)) return resolve_class(ctx.cls);

  /*
   * If there's only one preresolved ClassInfo, we can give out a
   * specific res::Class for it.  (Any other possible resolutions were
   * known to fatal, or it was actually unique.)
   */
  auto const classes = find_range(m_data->classInfo, clsName);
  for (auto it = begin(classes); it != end(classes); ++it) {
    auto const cinfo = it->second;
    if (cinfo->cls->attrs & AttrUnique) {
      if (debug &&
          (std::next(it) != end(classes) ||
           m_data->typeAliases.count(clsName))) {
        std::fprintf(stderr, "non unique \"unique\" class: %s\n",
          cinfo->cls->name->data());
        while (++it != end(classes)) {
          std::fprintf(stderr, "   and %s\n", it->second->cls->name->data());
        }
        auto const typeAliases = find_range(m_data->typeAliases, clsName);

        for (auto ta = begin(typeAliases); ta != end(typeAliases); ++ta) {
          std::fprintf(stderr, "   and type-alias %s\n",
                       ta->second->name->data());
        }
        always_assert(0);
      }
      return res::Class { this, cinfo };
    }
    break;
  }

  // We refuse to have name-only resolutions of enums, or typeAliases,
  // so that all name only resolutions can be treated as objects.
  if (!m_data->enums.count(clsName) &&
      !m_data->typeAliases.count(clsName)) {
    return res::Class { this, clsName };
  }

  return folly::none;
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

Index::ResolvedInfo<folly::Optional<res::Class>>
Index::resolve_type_name(SString inName) const {
  auto const res = resolve_type_name_internal(inName);
  return {
    res.type,
    res.nullable,
    res.value.isNull()
      ? folly::none
      : folly::make_optional(res::Class{this, res.value})
  };
}

Index::ResolvedInfo<Either<SString,ClassInfo*>>
Index::resolve_type_name_internal(SString inName) const {
  folly::Optional<hphp_fast_set<const void*>> seen;

  auto nullable = false;
  auto name = inName;

  for (unsigned i = 0; ; ++i) {
    name = normalizeNS(name);
    auto const classes = find_range(m_data->classInfo, name);
    auto const cls_it = begin(classes);
    if (cls_it != end(classes)) {
      auto const cinfo = cls_it->second;
      if (!(cinfo->cls->attrs & AttrUnique)) {
        if (!m_data->enums.count(name) && !m_data->typeAliases.count(name)) {
          break;
        }
        return { AnnotType::Object, false, nullptr };
      }
      if (!(cinfo->cls->attrs & AttrEnum)) {
        return { AnnotType::Object, nullable, cinfo };
      }
      auto const& tc = cinfo->cls->enumBaseTy;
      assert(!tc.isNullable());
      if (tc.type() != AnnotType::Object) {
        auto const type = tc.type() == AnnotType::Mixed ?
          AnnotType::ArrayKey : tc.type();
        return { type, nullable, tc.typeName() };
      }
      name = tc.typeName();
    } else {
      auto const typeAliases = find_range(m_data->typeAliases, name);
      auto const ta_it = begin(typeAliases);
      if (ta_it == end(typeAliases)) break;
      auto const ta = ta_it->second;
      if (!(ta->attrs & AttrUnique)) {
        return { AnnotType::Object, false, nullptr };
      }
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
        return { AnnotType::Object, false, nullptr };
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

Index::ConstraintResolution Index::resolve_class_or_type_alias(
  const Context& ctx, SString name, const Type& candidate) const {

  auto const res = resolve_type_name_internal(name);

  if (res.nullable && candidate.subtypeOf(BInitNull)) return TInitNull;

  if (res.type == AnnotType::Object) {
    auto resolve = [&] (const res::Class& rcls) -> folly::Optional<Type> {
      if (!interface_supports_non_objects(rcls.name()) ||
          candidate.subtypeOrNull(BObj)) {
        return subObj(rcls);
      }

      if (candidate.subtypeOrNull(BArr)) {
        if (interface_supports_array(rcls.name())) return TArr;
      } else if (candidate.subtypeOrNull(BVec)) {
        if (interface_supports_vec(rcls.name())) return TVec;
      } else if (candidate.subtypeOrNull(BDict)) {
        if (interface_supports_dict(rcls.name())) return TDict;
      } else if (candidate.subtypeOrNull(BKeyset)) {
        if (interface_supports_keyset(rcls.name())) return TKeyset;
      } else if (candidate.subtypeOrNull(BStr)) {
        if (interface_supports_string(rcls.name())) return TStr;
      } else if (candidate.subtypeOrNull(BInt)) {
        if (interface_supports_int(rcls.name())) return TInt;
      } else if (candidate.subtypeOrNull(BDbl)) {
        if (interface_supports_double(rcls.name())) return TDbl;
      }
      return folly::none;
    };

    if (res.value.isNull()) return ConstraintResolution{ folly::none, true };

    auto ty = res.value.right() ?
      resolve({ this, res.value.right() }) :
      resolve({ this, res.value.left() });

    if (ty && res.nullable) *ty = opt(std::move(*ty));
    return ConstraintResolution{ std::move(ty), false };
  }

  return get_type_for_annotated_type(ctx, res.type, res.nullable,
                                     res.value.left(), candidate);
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
    rcls.hasValue() &&
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

  // Interfaces may have more method families than methods, so look at the
  // method families first. Note that interfaces only have methods and method
  // families for which all methods are public on all implementer classes and
  // no methods have private ancestors. This avoids the need for context checks
  // which would be difficult as the base class is unknown. Interfaces are
  // generally used to declare public methods so this trade-off is unlikely to
  // cost anything in practice.
  if (cinfo->cls->attrs & AttrInterface) {
    auto methIt = cinfo->methodFamilies.find(name);
    if (methIt == end(cinfo->methodFamilies)) return name_only();
    if (methIt->second.possibleFuncs()->size() == 1) {
      return res::Func { this, methIt->second.possibleFuncs()->front() };
    }
    // If there was a sole implementer we can resolve to a single method, even
    // if the method was not declared on the interface itself.
    return res::Func { this, &methIt->second };
  }

  /*
   * Whether or not the context class has a private method with the
   * same name as the method we're trying to call.
   */
  auto const contextMayHavePrivateWithSameName = folly::lazy([&]() -> bool {
    if (!ctx.cls) return false;
    auto const range = find_range(m_data->classInfo, ctx.cls->name);
    if (begin(range) == end(range)) {
      // This class had no pre-resolved ClassInfos, which means it
      // always fatals in any way it could be defined, so it doesn't
      // matter what we return here (as all methods in the context
      // class are unreachable code).
      return true;
    }
    // Because of traits, each instantiation of the class could have
    // different private methods; we need to check them all.
    for (auto ctxInfo : range) {
      auto const iter = ctxInfo.second->methods.find(name);
      if (iter != end(ctxInfo.second->methods) &&
          iter->second.attrs & AttrPrivate &&
          iter->second.topLevel) {
        return true;
      }
    }
    return false;
  });

  /*
   * Look up the method in the target class.
   */
  auto const methIt = cinfo->methods.find(name);
  if (methIt == end(cinfo->methods)) return name_only();
  if (methIt->second.attrs & AttrInterceptable) return name_only();
  auto const ftarget = methIt->second.func;

  // We need to revisit the hasPrivateAncestor code if we start being
  // able to look up methods on interfaces (currently they have empty
  // method tables).
  assert(!(cinfo->cls->attrs & AttrInterface));

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

  /*
   * Note: this currently isn't exhaustively checking accessibility,
   * except in cases where we must do a little bit of it for
   * correctness.
   *
   * It is generally ok to resolve a method that won't actually be
   * called as long, as we only do so in cases where it will fatal at
   * runtime.
   *
   * So, in the presence of magic methods, we must handle the fact
   * that attempting to call an inaccessible method will instead call
   * the magic method, if it exists.  Note that if any class derives
   * from a class and adds magic methods, it can change still change
   * dispatch to call that method instead of fatalling.
   */

  // If false, this method is definitely accessible.  If true, it may
  // or may not be accessible.
  auto const couldBeInaccessible = [&] {
    // Public is always accessible.
    if (methIt->second.attrs & AttrPublic) return false;
    // An anonymous context won't have access if it wasn't public.
    if (!ctx.cls) return true;
    // If the calling context class is the same as the target class,
    // and the method is defined on this class or is protected, it
    // must be accessible.
    if (ctx.cls == cinfo->cls &&
        (methIt->second.topLevel || methIt->second.attrs & AttrProtected)) {
      return false;
    }
    // If the method is private, the above case is the only case where
    // we'd know it was accessible.
    if (methIt->second.attrs & AttrPrivate) return true;
    /*
     * For the protected method case: if the context class must be
     * derived from the class that first defined the protected method
     * we know it is accessible.  First check against the class of the
     * method (or cinfo for trait methods).
     */
    if (must_be_derived_from(
          ctx.cls,
          ftarget->cls->attrs & AttrTrait ? cinfo->cls : ftarget->cls)) {
      return false;
    }
    if (methIt->second.hasAncestor ||
        (ftarget->cls->attrs & AttrTrait && !methIt->second.topLevel)) {
      // Now we have find the first class that defined the method, and
      // check if *that* is an ancestor of the context class.
      auto parent = cinfo->parent;
      while (true) {
        assertx(parent);
        auto it = parent->methods.find(name);
        assertx(it != parent->methods.end());
        if (!it->second.hasAncestor && it->second.topLevel) {
          if (must_be_derived_from(ctx.cls, parent->cls)) return false;
          break;
        }
        parent = parent->parent;
      }
    }
    /*
     * On the other hand, if the class that defined the method must be
     * derived from the context class, it is going to be accessible as
     * long as the context class does not define a private method with
     * the same name.  (If it did, we'd be calling that private
     * method, which currently we don't ever resolve---we've removed
     * it from the method table in the classInfo.)
     */
    if (must_be_derived_from(cinfo->cls, ctx.cls)) {
      if (!contextMayHavePrivateWithSameName()) {
        return false;
      }
    }
    // Other cases we're not sure about (maybe some non-unique classes
    // got in the way).  Conservatively return that it might be
    // inaccessible.
    return true;
  };

  auto resolve = [&] {
    create_func_info(*m_data, ftarget);
    return res::Func { this, mteFromIt(methIt) };
  };

  switch (dcls.type) {
  case DCls::Exact:
    if (cinfo->magicCall.thisHas || cinfo->magicCallStatic.thisHas) {
      if (couldBeInaccessible()) return name_only();
    }
    return resolve();
  case DCls::Sub:
    if (cinfo->magicCall.derivedHas || cinfo->magicCallStatic.derivedHas) {
      if (couldBeInaccessible()) return name_only();
    }
    if (methIt->second.attrs & AttrNoOverride) {
      return resolve();
    }
    if (!options.FuncFamilies) return name_only();

    {
      auto const famIt = cinfo->methodFamilies.find(name);
      if (famIt == end(cinfo->methodFamilies)) {
        return name_only();
      }
      if (famIt->second.containsInterceptables()) {
        return name_only();
      }
      return res::Func { this, &famIt->second };
    }
  }
  not_reached();
}

folly::Optional<res::Func>
Index::resolve_ctor(Context /*ctx*/, res::Class rcls, bool exact) const {
  auto const cinfo = rcls.val.right();
  if (!cinfo || !cinfo->ctor) return folly::none;
  if (exact || cinfo->ctor->second.attrs & AttrNoOverride) {
    if (cinfo->ctor->second.attrs & AttrInterceptable) return folly::none;
    create_func_info(*m_data, cinfo->ctor->second.func);
    return res::Func { this, cinfo->ctor };
  }

  if (!options.FuncFamilies) return folly::none;

  auto const famIt = cinfo->methodFamilies.find(s_construct.get());
  if (famIt == end(cinfo->methodFamilies)) return folly::none;
  if (famIt->second.containsInterceptables()) return folly::none;
  return res::Func { this, &famIt->second };
}

template<class FuncRange>
res::Func
Index::resolve_func_helper(const FuncRange& funcs, SString name) const {
  auto name_only = [&] (bool renamable) {
    return res::Func { this, res::Func::FuncName { name, renamable } };
  };

  // no resolution
  if (begin(funcs) == end(funcs)) return name_only(false);

  auto const func = begin(funcs)->second;
  if (func->attrs & AttrInterceptable) return name_only(true);

  // multiple resolutions
  if (std::next(begin(funcs)) != end(funcs)) {
    assert(!(func->attrs & AttrUnique));
    if (debug && any_interceptable_functions()) {
      for (auto const DEBUG_ONLY f : funcs) {
        assertx(!(f.second->attrs & AttrInterceptable));
      }
    }
    return name_only(false);
  }

  // single resolution, in whole-program mode, that's it
  if (RuntimeOption::RepoAuthoritative) {
    assert(func->attrs & AttrUnique);
    return do_resolve(func);
  }

  // single-unit mode, check builtins
  if (func->attrs & AttrBuiltin) {
    assert(func->attrs & AttrUnique);
    return do_resolve(func);
  }

  // single-unit, non-builtin, not renamable
  return name_only(false);
}

res::Func Index::resolve_func(Context /*ctx*/, SString name) const {
  name = normalizeNS(name);
  auto const funcs = find_range(m_data->funcs, name);
  return resolve_func_helper(funcs, name);
}

std::pair<folly::Optional<res::Func>, folly::Optional<res::Func>>
Index::resolve_func_fallback(Context /*ctx*/, SString nsName,
                             SString fallbackName) const {
  assert(!needsNSNormalization(nsName));
  assert(!needsNSNormalization(fallbackName));

  // It's possible that in some requests nsName might succeed, while
  // in others fallbackName must succeed.  Both ranges must be
  // considered before we can decide which function we're after.
  auto const r1 = find_range(m_data->funcs, nsName);
  auto const r2 = find_range(m_data->funcs, fallbackName);
  if ((begin(r1) != end(r1) && begin(r2) != end(r2)) ||
      !RuntimeOption::RepoAuthoritative) {
    // It could come from either at runtime.  (We could try to rule it
    // out by figuring out if one must be defined based on the
    // ctx.unit, but it's unlikely to matter for now.)
    return std::make_pair(
      resolve_func_helper(r1, nsName),
      resolve_func_helper(r2, fallbackName)
    );
  }

  assert(RuntimeOption::RepoAuthoritative);
  if (begin(r2) == end(r2)) {
    return std::make_pair(resolve_func_helper(r1, nsName), folly::none);
  } else {
    return std::make_pair(folly::none, resolve_func_helper(r2, fallbackName));
  }
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
  assertx(IMPLIES(!tc.isCheckable(), tc.isMixed()));

  if (getSuperType) {
    /*
     * Soft hints (@Foo) are not checked.
     */
    if (tc.isSoft()) return TCell;
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
  auto const res = get_type_for_annotated_type(
    Context { nullptr, nullptr, &propCls },
    tc.type(),
    tc.isNullable(),
    tc.typeName(),
    TGen
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
      case KindOfPersistentShape:
      case KindOfShape:        not_implemented();
      case KindOfPersistentArray:
      case KindOfArray:        return TPArr;
      case KindOfResource:     return TRes;
      case KindOfClsMeth:      return TClsMeth;
      case KindOfObject:
        return resolve_class_or_type_alias(ctx, name, candidate);
      case KindOfUninit:
      case KindOfRef:
      case KindOfFunc:
      case KindOfClass:
      case KindOfRecord:
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
    case AnnotMetaType::NoReturn:
      return TBottom;
    case AnnotMetaType::Nonnull:
      if (candidate.subtypeOf(BInitNull)) return TBottom;
      if (!candidate.couldBe(BInitNull))  return candidate;
      if (is_opt(candidate))              return unopt(candidate);
      break;
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
    case AnnotMetaType::VArray:
      assertx(!RuntimeOption::EvalHackArrDVArrs);
      return TVArr;
    case AnnotMetaType::DArray:
      assertx(!RuntimeOption::EvalHackArrDVArrs);
      return TDArr;
    case AnnotMetaType::VArrOrDArr:
      assertx(!RuntimeOption::EvalHackArrDVArrs);
      return TArr;
    case AnnotMetaType::VecOrDict:
      if (candidate.subtypeOf(BVec)) return TVec;
      if (candidate.subtypeOf(BDict)) return TDict;
      break;
    case AnnotMetaType::ArrayLike:
      if (candidate.subtypeOf(BVArr)) return TVArr;
      if (candidate.subtypeOf(BDArr)) return TDArr;
      if (candidate.subtypeOf(BArr)) return TArr;
      if (candidate.subtypeOf(BVec)) return TVec;
      if (candidate.subtypeOf(BDict)) return TDict;
      if (candidate.subtypeOf(BKeyset)) return TKeyset;
      break;
    }
    return ConstraintResolution{ folly::none, false };
  }();

  if (mainType.type && nullable && !mainType.type->couldBe(BInitNull)) {
    mainType.type = opt(*mainType.type);
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
  if (t.moreRefined(loosen_dvarrayness(tcType))) {
    // For d/varrays, we might satisfy the constraint, but still not want to
    // optimize away the type-check (because we'll raise a notice on a d/varray
    // mismatch), so do some additional checking here to rule that out.
    if (!RuntimeOption::EvalHackArrCompatTypeHintNotices) return true;
    if (!tcType.subtypeOrNull(BArr) || tcType.subtypeOf(BNull)) return true;
    assertx(t.subtypeOrNull(BArr));
    if (tcType.subtypeOrNull(BVArr)) return t.subtypeOrNull(BVArr);
    if (tcType.subtypeOrNull(BDArr)) return t.subtypeOrNull(BDArr);
    if (tcType.subtypeOrNull(BPArr)) return t.subtypeOrNull(BPArr);
  }
  return false;
}

bool Index::could_have_reified_type(const TypeConstraint& tc) const {
  if (!tc.isObject()) return false;
  auto const name = tc.typeName();
  auto const resolved = resolve_type_name_internal(name);
  if (resolved.type != AnnotType::Object) return false;
  res::Class rcls{this, resolved.value};
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

bool Index::is_effect_free(res::Func rfunc) const {
  return match<bool>(
    rfunc.val,
    [&](res::Func::FuncName)   { return false; },
    [&](res::Func::MethodName) { return false; },
    [&](FuncInfo* finfo) {
      return finfo->effectFree;
    },
    [&](const MethTabEntryPair* mte) {
      return func_info(*m_data, mte->second.func)->effectFree;
    },
    [&](FuncFamily* fam) {
      return false;
    });
}

bool Index::any_interceptable_functions() const {
  return m_data->any_interceptable_functions;
}

Type Index::lookup_class_constant(Context ctx,
                                  res::Class rcls,
                                  SString cnsName) const {
  if (rcls.val.left()) return TInitCell;
  auto const cinfo = rcls.val.right();

  auto const it = cinfo->clsConstants.find(cnsName);
  if (it != end(cinfo->clsConstants)) {
    if (!it->second->val.hasValue() || it->second->isTypeconst) {
      // This is an abstract class constant or typeconstant
      return TInitCell;
    }
    if (it->second->val.value().m_type == KindOfUninit) {
      // This is a class constant that needs an 86cinit to run.
      // We'll add a dependency to make sure we're re-run if it
      // resolves anything.
      auto const cinit = it->second->cls->methods.back().get();
      assert(cinit->name == s_86cinit.get());
      add_dependency(*m_data, cinit, ctx, Dep::ClsConst);
      return TInitCell;
    }
    return from_cell(it->second->val.value());
  }
  return TInitCell;
}

folly::Optional<Type> Index::lookup_constant(Context ctx,
                                             SString cnsName) const {
  auto it = m_data->constants.find(cnsName);
  if (it == m_data->constants.end()) {
    // flag to indicate that the constant isn't in the index yet.
    if (options.HardConstProp) return folly::none;
    return TInitCell;
  }

  if (it->second.func &&
      !it->second.readonly &&
      !it->second.system &&
      !tv(it->second.type)) {
    // we might refine the type
    add_dependency(*m_data, it->second.func, ctx, Dep::ConstVal);
  }

  return it->second.type;
}

folly::Optional<Cell> Index::lookup_persistent_constant(SString cnsName) const {
  if (!options.HardConstProp) return folly::none;
  auto it = m_data->constants.find(cnsName);
  if (it == m_data->constants.end()) return folly::none;
  return tv(it->second.type);
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
  if ((func->attrs & AttrStatic) && ctxType.couldBe(TObj)) return TTop;
  if (!(func->attrs & AttrStatic) && ctxType.couldBe(TCls)) return TTop;

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
        func->cls ? func->cls->name : empty_string().get(),
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
      func->cls ? func->cls->name : empty_string().get(),
      func->cls ? "::" : "",
      func->name,
      showArgs(calleeCtx.args),
      CallContextHashCompare{}.hash(calleeCtx));
    return TTop;
  }

  if (!interp_nesting_level) {
    base_ctx = ctx;
  } else if (interp_nesting_level > max_interp_nexting_level) {
    add_dependency(*m_data, func, base_ctx, Dep::InlineDepthLimit);
    return TTop;
  }

  auto const contextType = [&] {
    ++interp_nesting_level;
    SCOPE_EXIT { --interp_nesting_level; };

    auto const fa = analyze_func_inline(
      *this,
      Context { func->unit, const_cast<php::Func*>(func), func->cls },
      calleeCtx.context,
      calleeCtx.args,
      CollectionOpts::TrackConstantArrays |
      CollectionOpts::EffectFreeOnly
    );
    return fa.effectFree ? fa.inferredReturn : TTop;
  }();

  if (!is_scalar(contextType)) {
    return TTop;
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

Type Index::lookup_return_type(Context ctx, res::Func rfunc) const {
  return match<Type>(
    rfunc.val,
    [&](res::Func::FuncName)   { return TInitCell; },
    [&](res::Func::MethodName) { return TInitCell; },
    [&](FuncInfo* finfo) {
      add_dependency(*m_data, finfo->func, ctx, Dep::ReturnTy);
      return unctx(finfo->returnTy);
    },
    [&](const MethTabEntryPair* mte) {
      add_dependency(*m_data, mte->second.func, ctx, Dep::ReturnTy);
      auto const finfo = func_info(*m_data, mte->second.func);
      if (!finfo->func) return TInitCell;
      return unctx(finfo->returnTy);
    },
    [&](FuncFamily* fam) {
      auto ret = TBottom;
      for (auto const pf : fam->possibleFuncs()) {
        add_dependency(*m_data, pf->second.func, ctx, Dep::ReturnTy);
        auto const finfo = func_info(*m_data, pf->second.func);
        if (!finfo->func) return TInitCell;
        ret |= unctx(finfo->returnTy);
      }
      return ret;
    });
}

Type Index::lookup_return_type(Context caller,
                               const CompactVector<Type>& args,
                               const Type& context,
                               res::Func rfunc) const {
  return match<Type>(
    rfunc.val,
    [&](res::Func::FuncName) {
      return lookup_return_type(caller, rfunc);
    },
    [&](res::Func::MethodName) {
      return lookup_return_type(caller, rfunc);
    },
    [&](FuncInfo* finfo) {
      add_dependency(*m_data, finfo->func, caller, Dep::ReturnTy);
      return context_sensitive_return_type(*m_data,
                                           { finfo->func, args, context });
    },
    [&](const MethTabEntryPair* mte) {
      add_dependency(*m_data, mte->second.func, caller, Dep::ReturnTy);
      auto const finfo = func_info(*m_data, mte->second.func);
      if (!finfo->func) return TInitCell;
      return context_sensitive_return_type(*m_data,
                                           { finfo->func, args, context });
    },
    [&] (FuncFamily* fam) {
      auto ret = TBottom;
      for (auto& pf : fam->possibleFuncs()) {
        add_dependency(*m_data, pf->second.func, caller, Dep::ReturnTy);
        auto const finfo = func_info(*m_data, pf->second.func);
        if (!finfo->func) ret |= TInitCell;
        else ret |= return_with_context(finfo->returnTy, context);
      }
      return ret;
    }
  );
}

CompactVector<Type>
Index::lookup_closure_use_vars(const php::Func* func,
                               bool move) const {
  assert(func->isClosureBody);

  auto const numUseVars = closure_num_use_vars(func);
  if (!numUseVars) return {};
  auto const it = m_data->closureUseVars.find(func->cls);
  if (it == end(m_data->closureUseVars)) {
    return CompactVector<Type>(numUseVars, TGen);
  }
  if (move) return std::move(it->second);
  return it->second;
}

Type Index::lookup_return_type_raw(const php::Func* f) const {
  auto it = func_info(*m_data, f);
  if (it->func) {
    assertx(it->func == f);
    return it->returnTy;
  }
  return TInitCell;
}

Type Index::lookup_return_type_and_clear(
  const php::Func* f) const {
  auto it = func_info(*m_data, f);

  it->localStaticTypes.clear();

  return std::move(it->returnTy);
}

CompactVector<Type>
Index::lookup_local_static_types(const php::Func* f) const {
  auto it = func_info(*m_data, f);
  if (it->func) {
    assertx(it->func == f);
    return it->localStaticTypes;
  }
  return {};
}

bool Index::lookup_this_available(const php::Func* f) const {
  return (f->attrs & AttrRequiresThis) && !f->isClosureBody;
}

PrepKind Index::lookup_param_prep(Context /*ctx*/, res::Func rfunc,
                                  uint32_t paramId) const {
  return match<PrepKind>(
    rfunc.val,
    [&] (res::Func::FuncName s) {
      if (!RuntimeOption::RepoAuthoritative || s.renamable) return PrepKind::Unknown;
      return prep_kind_from_set(find_range(m_data->funcs, s.name), paramId);
    },
    [&] (res::Func::MethodName s) {
      if (!RuntimeOption::RepoAuthoritative) return PrepKind::Unknown;
      auto const it = m_data->method_ref_params_by_name.find(s.name);
      if (it == end(m_data->method_ref_params_by_name)) {
        // There was no entry, so no method by this name takes a parameter
        // by reference.
        return PrepKind::Val;
      }
      /*
       * If we think it's supposed to be PrepKind::Ref, we still can't be sure
       * unless we go through some effort to guarantee that it can't be going
       * to an __call function magically (which will never take anything by
       * ref).
       */
      if (paramId < sizeof(it->second) * CHAR_BIT) {
        return ((it->second >> paramId) & 1) ?
          PrepKind::Unknown : PrepKind::Val;
      }
      auto const kind = prep_kind_from_set(
        find_range(m_data->methods, s.name),
        paramId
      );
      return kind == PrepKind::Ref ? PrepKind::Unknown : kind;
    },
    [&] (FuncInfo* finfo) {
      return func_param_prep(finfo->func, paramId);
    },
    [&] (const MethTabEntryPair* mte) {
      return func_param_prep(mte->second.func, paramId);
    },
    [&] (FuncFamily* fam) {
      assert(RuntimeOption::RepoAuthoritative);
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

Type Index::lookup_public_static(Context ctx,
                                 const Type& cls,
                                 const Type& name) const {
  if (!is_specialized_cls(cls)) return TInitGen;

  auto const vname = tv(name);
  if (!vname || vname->m_type != KindOfPersistentString) return TInitGen;
  auto const sname = vname->m_data.pstr;

  if (ctx.unit) add_dependency(*m_data, sname, ctx, Dep::PublicSPropName);

  auto const dcls = dcls_of(cls);
  if (dcls.cls.val.left()) return TInitGen;
  auto const cinfo = dcls.cls.val.right();

  switch (dcls.type) {
    case DCls::Sub: {
      auto ty = TBottom;
      for (auto const sub : cinfo->subclassList) {
        ty |= lookup_public_static_impl(
          *m_data,
          sub,
          sname
        ).inferredType;
      }
      return ty;
    }
    case DCls::Exact:
      return lookup_public_static_impl(
        *m_data,
        cinfo,
        sname
      ).inferredType;
  }
  always_assert(false);
}

Type Index::lookup_public_static(Context ctx,
                                 const php::Class* cls,
                                 SString name) const {
  if (ctx.unit) add_dependency(*m_data, name, ctx, Dep::PublicSPropName);
  return lookup_public_static_impl(*m_data, cls, name).inferredType;
}

bool Index::lookup_public_static_immutable(const php::Class* cls,
                                           SString name) const {
  return !lookup_public_static_impl(*m_data, cls, name).everModified;
}

bool Index::lookup_public_static_maybe_late_init(const Type& cls,
                                                 const Type& name) const {
  auto const cinfo = [&] () -> const ClassInfo* {
    if (!is_specialized_cls(cls)) {
      return nullptr;
    }
    auto const dcls = dcls_of(cls);
    switch (dcls.type) {
    case DCls::Sub:   return nullptr;
    case DCls::Exact: return dcls.cls.val.right();
    }
    not_reached();
  }();
  if (!cinfo) return true;

  auto const vname = tv(name);
  if (!vname || (vname && vname->m_type != KindOfPersistentString)) {
    return true;
  }
  auto const sname = vname->m_data.pstr;

  auto isLateInit = false;
  visit_parent_cinfo(
    cinfo,
    [&] (const ClassInfo* ci) -> bool {
      for (auto const& prop : ci->cls->properties) {
        if (prop.name == sname) {
          isLateInit = prop.attrs & AttrLateInit;
          return true;
        }
      }
      return false;
    }
  );
  return isLateInit;
}

Type Index::lookup_public_prop(const Type& cls, const Type& name) const {
  if (!is_specialized_cls(cls)) return TGen;

  auto const vname = tv(name);
  if (!vname || vname->m_type != KindOfPersistentString) return TGen;
  auto const sname = vname->m_data.pstr;

  auto const dcls = dcls_of(cls);
  if (dcls.cls.val.left()) return TGen;
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
  auto const classes = find_range(m_data->classInfo, cls->name);
  if (begin(classes) == end(classes) ||
      std::next(begin(classes)) != end(classes)) {
    return TGen;
  }
  return lookup_public_prop_impl(*m_data, begin(classes)->second, name);
}

bool Index::lookup_class_init_might_raise(Context ctx, res::Class cls) const {
  return cls.val.match(
    []  (SString) { return true; },
    [&] (ClassInfo* cinfo) {
      // Check this class and all of its parents for possible inequivalent
      // redeclarations or bad initial values.
      do {
        // Be conservative for now if we have unflattened traits.
        if (!cinfo->traitProps.empty()) return true;
        if (cinfo->hasBadRedeclareProp) return true;
        if (cinfo->hasBadInitialPropValues) {
          add_dependency(*m_data, cinfo->cls, ctx, Dep::PropBadInitialValues);
          return true;
        }
        cinfo = cinfo->parent;
      } while (cinfo);
      return false;
    }
  );
}

Slot
Index::lookup_iface_vtable_slot(const php::Class* cls) const {
  return folly::get_default(m_data->ifaceSlotMap, cls, kInvalidSlot);
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
  for (auto const& cinfo : m_data->allClassInfos) {
    for (auto const& prop : cinfo->cls->properties) {
      if (!(prop.attrs & AttrPublic) || !(prop.attrs & AttrStatic)) {
        continue;
      }

      /*
       * If the initializer type is TUninit, it means an 86sinit provides the
       * actual initialization type or it is AttrLateInit.  So we don't want to
       * include the Uninit (which isn't really a user-visible type for the
       * property) or by the time we union things in we'll have inferred nothing
       * much.
       *
       * If the property is AttrLateInitSoft, it can be anything because of the
       * default value, so give the initial value as TInitGen and don't honor
       * the type-constraint, which will keep us from inferring anything.
       */
      auto const initial = [&] {
        if (prop.attrs & AttrLateInitSoft) return TInitGen;
        auto const tyRaw = from_cell(prop.val);
        if (tyRaw.subtypeOf(BUninit)) return TBottom;
        if (prop.attrs & AttrSystemInitialValue) return tyRaw;
        return adjust_type_for_prop(
          *this, *cinfo->cls, &prop.typeConstraint, tyRaw
        );
      }();

      auto const tc = (prop.attrs & AttrLateInitSoft)
        ? nullptr
        : &prop.typeConstraint;

      cinfo->publicStaticProps[prop.name] =
        PublicSPropEntry {
          union_of(
            adjust_type_for_prop(*this, *cinfo->cls, tc, TInitGen),
            initial
          ),
          initial,
          tc,
          0,
          true
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
  auto const func = fa.ctx.func;
  for (auto const& it : fa.cnsMap) {
    if (it.second.m_type == kReadOnlyConstant) {
      // this constant was read, but there was nothing mentioning it
      // in the index. Should only happen on the first iteration. We
      // need to reprocess this func.
      assert(fa.readsUntrackedConstants);
      // if there's already an entry, we don't want to do anything,
      // otherwise just insert a dummy entry to indicate that it was
      // read.
      m_data->constants.emplace(it.first,
                                ConstInfo {func, TInitCell, false, true});
      continue;
    }

    if (it.second.m_type == kDynamicConstant || !is_pseudomain(func)) {
      // two definitions, or a non-pseuodmain definition
      auto& c = m_data->constants[it.first];
      if (!c.system) {
        c.func = nullptr;
        c.type = TInitCell;
        c.readonly = false;
      }
      continue;
    }

    auto t = it.second.m_type == KindOfUninit ?
      TInitCell : from_cell(it.second);

    assertx(t.equivalentlyRefined(unctx(t)));

    auto const res = m_data->constants.emplace(it.first, ConstInfo {func, t});

    if (res.second || res.first->second.system) continue;

    if (res.first->second.readonly) {
      res.first->second.func = func;
      res.first->second.type = t;
      res.first->second.readonly = false;
      continue;
    }

    if (res.first->second.func != func) {
      res.first->second.func = nullptr;
      res.first->second.type = TInitCell;
      continue;
    }

    assertx(t.moreRefined(res.first->second.type));
    if (!t.equivalentlyRefined(res.first->second.type)) {
      res.first->second.type = t;
      find_deps(*m_data, func, Dep::ConstVal, deps);
    }
  }
  if (fa.readsUntrackedConstants) deps.emplace(dep_context(*m_data, fa.ctx));
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

void Index::refine_local_static_types(
  const php::Func* func,
  const CompactVector<Type>& localStaticTypes) {

  auto const finfo = create_func_info(*m_data, func);
  if (localStaticTypes.empty()) {
    finfo->localStaticTypes.clear();
    return;
  }

  finfo->localStaticTypes.resize(localStaticTypes.size(), TTop);
  for (auto i = size_t{0}; i < localStaticTypes.size(); i++) {
    auto& indexTy = finfo->localStaticTypes[i];
    auto const& newTy = unctx(localStaticTypes[i]);
    always_assert_flog(
      newTy.moreRefined(indexTy),
      "Index local static type invariant violated in {} {}{}.\n"
      "   Static Local {}: {} is not a subtype of {}\n",
      func->unit->filename,
      func->cls ? folly::to<std::string>(func->cls->name->data(), "::")
      : std::string{},
      func->name->data(),
      local_string(*func, i),
      show(newTy),
      show(indexTy)
    );
    if (!newTy.strictlyMoreRefined(indexTy)) continue;
    indexTy = newTy;
  }
}

void Index::init_return_type(const php::Func* func) {
  if ((func->attrs & AttrBuiltin) || func->isMemoizeWrapper) {
    return;
  }

  auto make_type = [&] (const TypeConstraint& tc) {
    if (tc.isSoft() ||
        (RuntimeOption::EvalThisTypeHintLevel != 3 && tc.isThis())) {
      return TBottom;
    }
    return loosen_dvarrayness(
      lookup_constraint(
        Context {
          func->unit,
            const_cast<php::Func*>(func),
            func->cls && func->cls->closureContextCls ?
            func->cls->closureContextCls : func->cls
            },
        tc)
    );
  };

  auto const finfo = create_func_info(*m_data, func);

  auto tcT = make_type(func->retTypeConstraint);
  if (tcT == TBottom) return;

  if (func->attrs & AttrTakesInOutParams) {
    std::vector<Type> types;
    types.emplace_back(intersection_of(TInitCell, std::move(tcT)));
    for (auto& p : func->params) {
      if (!p.inout) continue;
      auto t = make_type(p.typeConstraint);
      if (t == TBottom) return;
      types.emplace_back(intersection_of(TInitCell, std::move(t)));
    }
    tcT = vec(std::move(types));
  }

  tcT = to_cell(std::move(tcT));
  if (is_specialized_obj(tcT)) {
    if (dobj_of(tcT).cls.couldBeInterfaceOrTrait()) {
      tcT = is_opt(tcT) ? TOptObj : TObj;
    }
  } else {
    tcT = loosen_all(std::move(tcT));
  }
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
  auto const& t = fa.inferredReturn;
  auto const func = fa.ctx.func;
  auto const finfo = create_func_info(*m_data, func);

  auto error_loc = [&] {
    return folly::sformat(
        "{} {}{}",
        func->unit->filename,
        func->cls ?
        folly::to<std::string>(func->cls->name->data(), "::") : std::string{},
        func->name
    );
  };

  auto dep = Dep{};
  if (finfo->retParam != fa.retParam) {
    dep = Dep::ReturnTy;
    always_assert_flog(
        fa.retParam != NoLocalId,
        "Index retParam went from {} to unset in {}.\n",
        finfo->retParam,
        error_loc()
    );
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
    if (finfo->returnRefinments + 1 < options.returnTypeRefineLimit) {
      finfo->returnTy = t;
      ++finfo->returnRefinments;
      dep = is_scalar(t) ?
        Dep::ReturnTy | Dep::InlineDepthLimit : Dep::ReturnTy;
    } else {
      FTRACE(1, "maxed out return type refinements at {}\n", error_loc());
    }
  } else {
    always_assert_flog(
        t.moreRefined(finfo->returnTy),
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
    dep = Dep::InlineDepthLimit;
  }


  if (dep != Dep{}) find_deps(*m_data, func, dep, deps);
}

bool Index::refine_closure_use_vars(const php::Class* cls,
                                    const CompactVector<Type>& vars) {
  assert(is_closure(*cls));

  for (auto i = uint32_t{0}; i < vars.size(); ++i) {
    always_assert_flog(
      vars[i].equivalentlyRefined(unctx(vars[i])),
      "Closure cannot have a used var with a context dependent type"
    );
  }

  auto& current = m_data->closureUseVars[cls];
  always_assert(current.empty() || current.size() == vars.size());
  if (current.empty()) {
    current = vars;
    return true;
  }

  auto changed = false;
  for (auto i = uint32_t{0}; i < vars.size(); ++i) {
    always_assert(vars[i].subtypeOf(current[i]));
    if (vars[i].strictSubtypeOf(current[i])) {
      changed = true;
      current[i] = vars[i];
    }
  }

  return changed;
}

template<class Container>
void refine_private_propstate(Container& cont,
                              const php::Class* cls,
                              const PropState& state) {
  assertx(!is_used_trait(*cls));
  auto it = cont.find(cls);
  if (it == end(cont)) {
    cont[cls] = state;
    return;
  }
  for (auto& kv : state) {
    auto& target = it->second[kv.first].ty;
    assertx(it->second[kv.first].tc == kv.second.tc);
    always_assert_flog(
      kv.second.ty.moreRefined(target),
      "PropState refinement failed on {}::${} -- {} was not a subtype of {}\n",
      cls->name->data(),
      kv.first->data(),
      show(kv.second.ty),
      show(target)
    );
    target = kv.second.ty;
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
    always_assert(m_data->allPublicSPropsUnknown);
    return;
  }

  auto const firstRefinement = m_data->allPublicSPropsUnknown;
  m_data->allPublicSPropsUnknown = false;

  if (firstRefinement) {
    // If this is the first refinement, reschedule any dependency which looked
    // at the public static property state previously.
    always_assert(m_data->unknownClassSProps.empty());
    for (auto const& dependency : m_data->dependencyMap) {
      if (dependency.first.tag() != DependencyContextType::PropName) continue;
      for (auto const& kv : dependency.second) {
        if (has_dep(kv.second, Dep::PublicSPropName)) deps.insert(kv.first);
      }
    }
  }

  // Refine unknown class state
  for (auto const& kv : unknown) {
    // We can't keep context dependent types in public properties.
    auto newType = unctx(kv.second);
    auto it = m_data->unknownClassSProps.find(kv.first);
    if (it == end(m_data->unknownClassSProps)) {
      // If this is the first refinement, our previous state was effectively
      // TGen for everything, so inserting a type into the map can only
      // refine. However, if this isn't the first refinement, a name not present
      // in the map means that its TBottom, so we shouldn't be inserting
      // anything.
      always_assert(firstRefinement);
      m_data->unknownClassSProps.emplace(
        kv.first,
        std::make_pair(std::move(newType), 0)
      );
      continue;
    }

    /*
     * We may only shrink the types we recorded for each property. (If a
     * property type ever grows, the interpreter could infer something
     * incorrect at some step.)
     */
    always_assert(!firstRefinement);
    always_assert_flog(
      newType.subtypeOf(it->second.first),
      "Static property index invariant violated for name {}:\n"
      "  {} was not a subtype of {}",
      kv.first->data(),
      show(newType),
      show(it->second.first)
    );

    // Put a limit on the refinements to ensure termination. Since we only ever
    // refine types, we can stop at any point and maintain correctness.
    if (it->second.second + 1 < options.publicSPropRefineLimit) {
      if (newType.strictSubtypeOf(it->second.first)) {
        find_deps(*m_data, it->first, Dep::PublicSPropName, deps);
      }
      it->second.first = std::move(newType);
      ++it->second.second;
    } else {
      FTRACE(
        1, "maxed out public static property refinements for name {}\n",
        kv.first->data()
      );
    }
  }

  // If we didn't see a mutation among all the functions for a particular name,
  // it means the type is TBottom. Iterate through the unknown class state and
  // remove any entries which we didn't see a mutation for.
  if (!firstRefinement) {
    auto it = begin(m_data->unknownClassSProps);
    auto last = end(m_data->unknownClassSProps);
    while (it != last) {
      auto const unknownIt = unknown.find(it->first);
      if (unknownIt == end(unknown)) {
        if (unknownIt->second != TBottom) {
          find_deps(*m_data, unknownIt->first, Dep::PublicSPropName, deps);
        }
        it = m_data->unknownClassSProps.erase(it);
      } else {
        ++it;
      }
    }
  }

  // Refine known class state
  for (auto const& cinfo : m_data->allClassInfos) {
    for (auto& kv : cinfo->publicStaticProps) {
      auto const newType = [&] {
        auto const it = known.find(
          PublicSPropMutations::KnownKey { cinfo.get(), kv.first }
        );
        // If we didn't see a mutation, the type is TBottom.
        if (it == end(known)) return TBottom;
        // We can't keep context dependent types in public properties.
        return adjust_type_for_prop(
          *this, *cinfo->cls, kv.second.tc, unctx(it->second)
        );
      }();

      // The type from the indexer doesn't contain the in-class initializer
      // types. Add that here.
      auto effectiveType = union_of(newType, kv.second.initializerType);

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
      always_assert(newType == TBottom || kv.second.everModified);

      // Put a limit on the refinements to ensure termination. Since we only
      // ever refine types, we can stop at any point and still maintain
      // correctness.
      if (kv.second.refinements + 1 < options.publicSPropRefineLimit) {
        if (effectiveType.strictSubtypeOf(kv.second.inferredType)) {
          find_deps(*m_data, kv.first, Dep::PublicSPropName, deps);
        }
        kv.second.inferredType = std::move(effectiveType);
        kv.second.everModified = newType != TBottom;
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

void Index::refine_bad_initial_prop_values(const php::Class* cls,
                                           bool value,
                                           DependencyContextSet& deps) {
   assertx(!is_used_trait(*cls));

   for (auto& info : find_range(m_data->classInfo, cls->name)) {
    auto const cinfo = info.second;
    if (cinfo->cls != cls) continue;
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
}

bool Index::frozen() const {
  return m_data->frozen;
}

void Index::freeze() {
  m_data->frozen = true;
  m_data->ever_frozen = true;
}

template<typename T>
void clobber(T& t) {
  if (debug) {
    char*p = (char*)&t;
    for (auto i = sizeof(t); i--; ) p[i] ^= 0xa5;
  }
}

#define CLEAR(x)                                \
  (x).clear();                                  \
  clobber(x);                                   \
  SCOPE_EXIT { clobber(x); };

void Index::cleanup_for_emit(folly::Baton<>* done) {
  CLEAR(m_data->classes);
  CLEAR(m_data->methods);
  CLEAR(m_data->method_ref_params_by_name);
  CLEAR(m_data->funcs);
  CLEAR(m_data->typeAliases);
  CLEAR(m_data->classAliases);

  CLEAR(m_data->classClosureMap);
  CLEAR(m_data->classExtraMethodMap);

  /*
   * allClassInfos, is what's keeping the ClassInfos alive, and Type's
   * can still have references to them. In addition, we can still use
   * classInfos from lookup_public_static, so we can't clear either
   * member here.
   */

  CLEAR(m_data->dependencyMap);
  CLEAR(m_data->foldableReturnTypeMap);

  if (done) done->wait();
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
  auto const clsClasses    = find_range(m_data->classInfo, cls->name);
  auto const parentClasses = find_range(m_data->classInfo, parent->name);
  for (auto& kvCls : clsClasses) {
    auto const rCls = res::Class { this, kvCls.second };
    for (auto& kvPar : parentClasses) {
      auto const rPar = res::Class { this, kvPar.second };
      if (!rCls.subtypeOf(rPar)) return false;
    }
  }
  return true;
}

// Return true if any possible definition of one php::Class could
// derive from another at runtime, or vice versa.
bool
Index::could_be_related(const php::Class* cls,
                        const php::Class* parent) const {
  if (cls == parent) return true;
  auto const clsClasses    = find_range(m_data->classInfo, cls->name);
  auto const parentClasses = find_range(m_data->classInfo, parent->name);
  for (auto& kvCls : clsClasses) {
    auto const rCls = res::Class { this, kvCls.second };
    for (auto& kvPar : parentClasses) {
      auto const rPar = res::Class { this, kvPar.second };
      if (rCls.couldBe(rPar)) return true;
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////

void PublicSPropMutations::merge(const Index& index,
                                 Context ctx,
                                 const Type& tcls,
                                 const Type& name,
                                 const Type& val) {
  // Figure out which class this can affect.  If we have a DCls::Sub we have to
  // assume it could affect any subclass, so we repeat this merge for all exact
  // class types deriving from that base.
  if (is_specialized_cls(tcls)) {
    auto const dcls = dcls_of(tcls);
    if (auto const cinfo = dcls.cls.val.right()) {
      switch (dcls.type) {
        case DCls::Exact:
          return merge(index, ctx, cinfo, name, val);
        case DCls::Sub:
          for (auto const sub : cinfo->subclassList) {
            merge(index, ctx, sub, name, val);
          }
          return;
      }
      not_reached();
    }
  }

  merge(index, ctx, nullptr, name, val);
}

void PublicSPropMutations::merge(const Index& index,
                                 Context ctx,
                                 ClassInfo* cinfo,
                                 const Type& name,
                                 const Type& val) {
  FTRACE(2, "merge_public_static: {} {} {}\n",
         cinfo ? cinfo->cls->name->data() : "<unknown>", show(name), show(val));

  auto get = [this] () -> Data& {
    if (!m_data) m_data = std::make_unique<Data>();
    return *m_data;
  };

  auto const vname = tv(name);
  auto const unknownName = !vname ||
    (vname && vname->m_type != KindOfPersistentString);

  if (!cinfo) {
    if (unknownName) {
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
      return;
    }

    auto const res = get().m_unknown.emplace(vname->m_data.pstr, val);
    if (!res.second) res.first->second |= val;
    return;
  }

  /*
   * We don't know the name, but we know something about the class.  We need to
   * merge the type for every property in the class hierarchy.
   */
  if (unknownName) {
    visit_parent_cinfo(cinfo,
                         [&] (const ClassInfo* ci) {
                           for (auto& kv : ci->publicStaticProps) {
                             merge(index, ctx, cinfo, sval(kv.first), val);
                           }
                           return false;
                         });
    return;
  }

  /*
   * Here we know both the ClassInfo and the static property name, but it may
   * not actually be on this ClassInfo.  In php, you can access base class
   * static properties through derived class names, and the access affects the
   * property with that name on the most-recently-inherited-from base class.
   *
   * If the property is not found as a public property anywhere in the
   * hierarchy, we don't want to merge this type.  Note we don't have to worry
   * about the case that there is a protected property in between, because this
   * is a fatal at class declaration time (you can't redeclare a public static
   * property with narrower access in a subclass).
   */
  auto const affectedInfo = (
    visit_parent_cinfo(
      cinfo,
      [&] (const ClassInfo* ci) ->
          folly::Optional<std::pair<ClassInfo*, const TypeConstraint*>> {
        auto const it = ci->publicStaticProps.find(vname->m_data.pstr);
        if (it != end(ci->publicStaticProps)) {
          return std::make_pair(
            const_cast<ClassInfo*>(ci),
            it->second.tc
          );
        }
        return folly::none;
      }
    )
  );

  if (!affectedInfo) {
    // Either this was a mutation that's going to fatal (property doesn't
    // exist), or it's a private static or a protected static.  We aren't in
    // that business here, so we don't need to record anything.
    return;
  }

  auto const affectedCInfo = affectedInfo->first;
  auto const affectedTC = affectedInfo->second;

  auto const adjusted =
    adjust_type_for_prop(index, *affectedCInfo->cls, affectedTC, val);

  // Merge the property type.
  auto const res = get().m_known.emplace(
    KnownKey { affectedCInfo, vname->m_data.pstr },
    adjusted
  );
  if (!res.second) res.first->second |= adjusted;
}

void PublicSPropMutations::merge(const Index& index,
                                 Context ctx,
                                 const php::Class& cls,
                                 const Type& name,
                                 const Type& val) {
  auto range = find_range(index.m_data->classInfo, cls.name);
  for (auto const& pair : range) {
    auto const cinfo = pair.second;
    if (cinfo->cls != &cls) continue;
    // Note that this works for both traits and regular classes
    for (auto const sub : cinfo->subclassList) {
      merge(index, ctx, sub, name, val);
    }
  }
}

//////////////////////////////////////////////////////////////////////

}}
