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
const StaticString s_86ctor("86ctor");
const StaticString s_86cinit("86cinit");
const StaticString s_Closure("Closure");
const StaticString s_AsyncGenerator("HH\\AsyncGenerator");
const StaticString s_Generator("Generator");

//////////////////////////////////////////////////////////////////////

/*
 * One-to-many case insensitive map, where the keys are static strings
 * and the values are some kind of borrowed_ptr.
 */
template<class T> using ISStringToMany =
  std::unordered_multimap<
    SString,
    borrowed_ptr<T>,
    string_data_hash,
    string_data_isame
  >;

/*
 * One-to-one case insensitive map, where the keys are static strings
 * and the values are some T.
 */
template<class T> using ISStringToOneT =
  std::unordered_map<
    SString,
    T,
    string_data_hash,
    string_data_isame
  >;

/*
 * One-to-one case insensitive map, where the keys are static strings
 * and the values are some kind of borrowed_ptr.
 */
template<class T> using ISStringToOne = ISStringToOneT<borrowed_ptr<T>>;

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
  ReturnTy = 0x1,
  /* This dependency should trigger when a DefCns is resolved */
  ConstVal = 0x2,
  /* This dependency should trigger when a class constant is resolved */
  ClsConst = 0x4,
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
    borrowed_ptr<const php::Func>,
    std::map<DependencyContext,Dep,DependencyContextLess>
  >;

//////////////////////////////////////////////////////////////////////

enum class PublicSPropState {
  Unrefined,    // refine_public_statics never called
  Invalid,      // analyzed, but we know nothing (m_everything_bad case)
  Valid,
};

/*
 * Each ClassInfo has a table of public static properties with these entries.
 * The `initializerType' is for use during refine_public_statics, and
 * inferredType will always be a supertype of initializerType.
 */
struct PublicSPropEntry {
  Type inferredType;
  Type initializerType;
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
  MethTabEntry(borrowed_ptr<const php::Func> func, Attr a, bool hpa, bool tl) :
      func(func), attrs(a), hasPrivateAncestor(hpa), topLevel(tl) {}
  borrowed_ptr<const php::Func> func = nullptr;
  // A method could be imported from a trait, and its attributes changed
  Attr attrs {};
  bool hasAncestor = false;
  bool hasPrivateAncestor = false;
  // This method came from the ClassInfo that owns the MethTabEntry,
  // or one of its used traits.
  bool topLevel = false;
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

inline MethTabEntryPair* mteFromIt(ISStringToOneT<MethTabEntry>::iterator it) {
  return static_cast<MethTabEntryPair*>(&*it);
}

struct CallContextHashCompare {
  bool equal(const CallContext& a, const CallContext& b) const {
    return a == b;
  }

  size_t hash(const CallContext& c) const {
    auto ret = folly::hash::hash_combine(
      c.caller.func,
      c.args.size()
    );
    for (auto& t : c.args) {
      ret = folly::hash::hash_combine(ret, t.hash());
    }
    return ret;
  }
};

// Note: CallContext contains the caller Context primarily to reduce
// the contention in this tbb.  (And because everywhere you need a
// CallContext you also need that caller Context.)
using ContextRetTyMap = tbb::concurrent_hash_map<
  CallContext,
  Type,
  CallContextHashCompare
>;

//////////////////////////////////////////////////////////////////////

template<class Filter>
PropState make_unknown_propstate(borrowed_ptr<const php::Class> cls,
                                 Filter filter) {
  auto ret = PropState{};
  for (auto& prop : cls->properties) {
    if (filter(prop)) {
      ret[prop.name] = TGen;
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
  borrowed_ptr<const php::Func> func = nullptr;
  /*
   * The best-known return type of the function, if we have any
   * information.  May be TBottom if the function is known to never
   * return (e.g. always throws).
   */
  Type returnTy = TInitGen;

  /*
   * The number of times we've refined returnTy.
   */
  uint32_t returnRefinments{0};

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
   * Type info for local statics.
   */
  CompactVector<Type> localStaticTypes;

  /*
   * Whether the function is effectFree.
   */
  bool effectFree{false};
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
  borrowed_ptr<const php::Func> func;
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
  bool containsInterceptables = false;
  bool isCtor = false;
  std::vector<const MethTabEntryPair*> possibleFuncs;
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
  borrowed_ptr<const php::Class> cls = nullptr;

  /*
   * The info for the parent of this Class.
   */
  borrowed_ptr<ClassInfo> parent = nullptr;

  /*
   * A vector of the declared interfaces class info structures.  This is in
   * declaration order mirroring the php::Class interfaceNames vector, and does
   * not include inherited interfaces.
   */
  std::vector<borrowed_ptr<const ClassInfo>> declInterfaces;

  /*
   * A (case-insensitive) map from interface names supported by this class to
   * their ClassInfo structures, flattened across the hierarchy.
   */
  ISStringToOneT<borrowed_ptr<const ClassInfo>> implInterfaces;

  /*
   * A (case-sensitive) map from class constant name to the php::Const
   * that it came from.  This map is flattened across the inheritance
   * hierarchy.
   */
  std::unordered_map<SString,borrowed_ptr<const php::Const>> clsConstants;

  /*
   * A vector of the used traits, in class order, mirroring the
   * php::Class usedTraitNames vector.
   */
  CompactVector<borrowed_ptr<const ClassInfo>> usedTraits;

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
  ISStringToOne<FuncFamily> methodFamilies;

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
  std::vector<borrowed_ptr<ClassInfo>> subclassList;

  /*
   * A vector of ClassInfo that encodes the inheritance hierarchy,
   * unless this ClassInfo represents an interface.
   *
   * This is the list of base classes for this class in inheritance
   * order.
   */
  std::vector<borrowed_ptr<ClassInfo>> baseList;

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
  std::unordered_map<SString,PublicSPropEntry> publicStaticProps;

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
  ClassInfo::MagicFnInfo (ClassInfo::*pmem);
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

Class::Class(borrowed_ptr<const Index> idx,
             Either<SString,borrowed_ptr<ClassInfo>> val)
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
    [] (borrowed_ptr<ClassInfo> ci) { return ci->cls->name.get(); }
  );
}

bool Class::couldBeInterfaceOrTrait() const {
  return val.match(
    [] (SString) { return true; },
    [] (borrowed_ptr<ClassInfo> cinfo) {
      return (cinfo->cls->attrs & (AttrInterface | AttrTrait));
    }
  );
}

bool Class::couldBeInterface() const {
  return val.match(
    [] (SString) { return true; },
    [] (borrowed_ptr<ClassInfo> cinfo) {
      return cinfo->cls->attrs & AttrInterface;
    }
  );
}

bool Class::couldBeOverriden() const {
  return val.match(
    [] (SString) { return true; },
    [] (borrowed_ptr<ClassInfo> cinfo) {
      return !(cinfo->cls->attrs & AttrNoOverride);
    }
  );
}

bool Class::couldHaveMagicGet() const {
  return val.match(
    [] (SString) { return true; },
    [] (borrowed_ptr<ClassInfo> cinfo) {
      return cinfo->magicGet.derivedHas;
    }
  );
}

bool Class::couldHaveMagicBool() const {
  return val.match(
    [] (SString) { return true; },
    [] (borrowed_ptr<ClassInfo> cinfo) {
      return cinfo->magicBool.derivedHas;
    }
  );
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

borrowed_ptr<const php::Class> Class::cls() const {
  return val.right() ? val.right()->cls : nullptr;
}

std::string show(const Class& c) {
  return c.val.match(
    [] (SString s) -> std::string {
      return s->data();
    },
    [] (borrowed_ptr<ClassInfo> cinfo) {
      return folly::sformat("{}*", cinfo->cls->name);
    }
  );
}

Func::Func(borrowed_ptr<const Index> idx, Rep val)
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
    [&] (FuncName s) { return s.name; },
    [&] (MethodName s) { return s.name; },
    [&] (borrowed_ptr<FuncInfo> fi) { return fi->func->name; },
    [&] (borrowed_ptr<const MethTabEntryPair> mte) { return mte->first; },
    [&] (borrowed_ptr<FuncFamily> fa) -> SString {
      if (fa->isCtor) return s_construct.get();
      auto const name = fa->possibleFuncs.front()->first;
      if (debug) {
        for (DEBUG_ONLY auto const f : fa->possibleFuncs) {
          assert(f->first->isame(name));
        }
      }
      return name;
    }
  );
}

borrowed_ptr<const php::Func> Func::exactFunc() const {
  using Ret = borrowed_ptr<const php::Func>;
  return match<Ret>(
    val,
    [&](FuncName /*s*/)                           { return Ret{}; },
    [&](MethodName /*s*/)                         { return Ret{}; },
    [&](borrowed_ptr<FuncInfo> fi)                { return fi->func; },
    [&](borrowed_ptr<const MethTabEntryPair> mte) { return mte->second.func; },
    [&](borrowed_ptr<FuncFamily> /*fa*/)          { return Ret{}; }
  );
}

bool Func::cantBeMagicCall() const {
  return match<bool>(
    val,
    [&](FuncName)                             { return true; },
    [&](MethodName)                           { return false; },
    [&](borrowed_ptr<FuncInfo>)               { return true; },
    [&](borrowed_ptr<const MethTabEntryPair>) { return true; },
    [&](borrowed_ptr<FuncFamily>)             { return true; }
  );
}

bool Func::mightReadCallerFrame() const {
  return match<bool>(
    val,
    // Only non-method builtins can read the caller's frame and builtins are
    // always uniquely resolvable in repo mode.
    [&](FuncName /*s*/) {
      return !RuntimeOption::RepoAuthoritative;
    },
    [&](MethodName /*s*/) { return false; },
    [&](borrowed_ptr<FuncInfo> fi) {
      return fi->func->attrs & AttrReadsCallerFrame;
    },
    [&](borrowed_ptr<const MethTabEntryPair>) { return false; },
    [&](borrowed_ptr<FuncFamily> fa) {
      for (auto const pf : fa->possibleFuncs) {
        if (pf->second.func->attrs & AttrReadsCallerFrame) return true;
      }
      return false;
    });
}

bool Func::mightWriteCallerFrame() const {
  return match<bool>(
    val,
    // Only non-method builtins can write to the caller's frame and builtins are
    // always uniquely resolvable in repo mode.
    [&](FuncName /*s*/) {
      return !RuntimeOption::RepoAuthoritative;
    },
    [&](MethodName /*s*/) { return false; },
    [&](borrowed_ptr<FuncInfo> fi) {
      return fi->func->attrs & AttrWritesCallerFrame;
    },
    [&](borrowed_ptr<const MethTabEntryPair>) { return false; },
    [&](borrowed_ptr<FuncFamily> fa) {
      for (auto const pf : fa->possibleFuncs) {
        if (pf->second.func->attrs & AttrWritesCallerFrame) return true;
      }
      return false;
    });
}

bool Func::isFoldable() const {
  return match<bool>(val, [&](FuncName /*s*/) { return false; },
                     [&](MethodName /*s*/) { return false; },
                     [&](borrowed_ptr<FuncInfo> fi) {
                       return fi->func->attrs & AttrIsFoldable;
                     },
                     [&](borrowed_ptr<const MethTabEntryPair> mte) {
                       return mte->second.func->attrs & AttrIsFoldable;
                     },
                     [&](borrowed_ptr<FuncFamily> fa) {
                       return false;
                     });
}

bool Func::mightBeSkipFrame() const {
  return match<bool>(
    val,
    // Only builtins can be skip frame and non-method builtins are always
    // uniquely resolvable. Methods are more complicated though.
    [&](FuncName /*s*/) { return false; },
    [&](MethodName /*s*/) { return true; },
    [&](borrowed_ptr<FuncInfo> fi) { return fi->func->attrs & AttrSkipFrame; },
    [&](borrowed_ptr<const MethTabEntryPair> mte) {
      return mte->second.func->attrs & AttrSkipFrame;
    },
    [&](borrowed_ptr<FuncFamily> fa) {
      for (auto const pf : fa->possibleFuncs) {
        if (pf->second.func->attrs & AttrSkipFrame) return true;
      }
      return false;
    });
}

std::string show(const Func& f) {
  std::string ret = f.name()->data();
  match<void>(f.val,
              [&](Func::FuncName) {}, [&](Func::MethodName) {},
              [&](borrowed_ptr<FuncInfo> /*fi*/) { ret += "*"; },
              [&](borrowed_ptr<const MethTabEntryPair> /*mte*/) { ret += "*"; },
              [&](borrowed_ptr<FuncFamily> /*fa*/) { ret += "+"; });
  return ret;
}

}

//////////////////////////////////////////////////////////////////////

using IfaceSlotMap = std::unordered_map<borrowed_ptr<const php::Class>, Slot>;

struct Index::IndexData {
  IndexData() = default;
  IndexData(const IndexData&) = delete;
  IndexData& operator=(const IndexData&) = delete;
  ~IndexData() = default;

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
  std::unordered_map<SString, ConstInfo> constants;
  std::unordered_set<SString,
                     string_data_hash,
                     string_data_isame>  classAliases;

  // Map from each class to all the closures that are allocated in
  // functions of that class.
  std::unordered_map<
    borrowed_ptr<const php::Class>,
    CompactVector<borrowed_ptr<const php::Class>>
  > classClosureMap;

  std::unordered_map<
    borrowed_ptr<const php::Class>,
    std::set<borrowed_ptr<php::Func>>
  > classExtraMethodMap;

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

  std::vector<std::unique_ptr<ClassInfo>>  allClassInfos;
  std::vector<std::unique_ptr<FuncFamily>> funcFamilies;

  std::vector<FuncInfo> funcInfo;

  // Private instance and static property types are stored separately
  // from ClassInfo, because you don't need to resolve a class to get
  // at them.
  std::unordered_map<
    borrowed_ptr<const php::Class>,
    PropState
  > privatePropInfo;
  std::unordered_map<
    borrowed_ptr<const php::Class>,
    PropState
  > privateStaticPropInfo;

  /*
   * Public static property information.
   *
   * We have state for whether any of it is valid (before we've analyzed for
   * it, or if the program contains /any/ modifications of static properties
   * where both the name and class are unknown).
   *
   * Each ClassInfo has a map of known largest static property types, valid if
   * PublicSPropState is true, but we also have information here about types
   * that may exist in static properties by name, when we didn't know the
   * class.  The Type we're allowed to assume any static property contains is
   * the union of the ClassInfo-specific type with the unknown class type for
   * that property name that's stored here.
   */
  PublicSPropState publicSPropState;
  PropState unknownClassSProps;

  /*
   * Map from interfaces to their assigned vtable slots, computed in
   * compute_iface_vtables().
   */
  IfaceSlotMap ifaceSlotMap;

  std::unordered_map<
    borrowed_ptr<const php::Class>,
    std::vector<Type>
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

DependencyContext dep_context(IndexData& data, const Context& ctx) {
  if (!ctx.cls || !data.useClassDependencies) return ctx.func;
  auto const cls = ctx.cls->closureContextCls ?
    ctx.cls->closureContextCls : ctx.cls;
  if (is_used_trait(*cls)) return ctx.func;
  return cls;
}

void add_dependency(IndexData& data,
                    borrowed_ptr<const php::Func> src,
                    const Context& dst,
                    Dep newMask) {
  if (data.frozen) return;

  auto d = dep_context(data, dst);
  DepMap::accessor acc;
  data.dependencyMap.insert(acc, src);
  auto& current = acc->second[d];
  current = current | newMask;
}

std::mutex func_info_mutex;

borrowed_ptr<FuncInfo> create_func_info(IndexData& data,
                                        borrowed_ptr<const php::Func> f) {
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

borrowed_ptr<FuncInfo> func_info(IndexData& data,
                                 borrowed_ptr<const php::Func> f) {
  auto const fi = &data.funcInfo[f->idx];
  return fi;
}

void find_deps(IndexData& data,
               borrowed_ptr<const php::Func> src,
               Dep mask,
               DependencyContextSet& deps) {
  DepMap::const_accessor acc;
  if (data.dependencyMap.find(acc, src)) {
    for (auto& kv : acc->second) {
      if (has_dep(kv.second, mask)) deps.insert(kv.first);
    }
  }
}

struct TraitMethod {
  using class_type = const ClassInfo*;
  using method_type = const php::Func*;
  using modifiers_type = Attr;

  TraitMethod(class_type trait_, method_type method_, modifiers_type modifiers_)
      : trait(trait_)
      , method(method_)
      , modifiers(modifiers_)
    {}

  const class_type trait;
  const method_type method;
  modifiers_type modifiers;
};

struct TMIOps {
  using prec_type  = const PreClass::TraitPrecRule&;
  using alias_type = const PreClass::TraitAliasRule&;
  using string_type = LSString;
  using class_type = TraitMethod::class_type;
  using method_type = TraitMethod::method_type;
  using modifiers_type = TraitMethod::modifiers_type;

  struct TMIException : std::exception {
    explicit TMIException(std::string msg) : msg(msg) {}
    const char* what() const noexcept override { return msg.c_str(); }
  private:
    std::string msg;
  };

  // Whether `str' is empty.
  static bool strEmpty(string_type str) { return str->empty(); }

  // Return the name for the trait class.
  static const string_type clsName(class_type traitCls) {
    return traitCls->cls->name;
  }

  // Is-a methods.
  static bool isTrait(class_type traitCls) {
    return traitCls->cls->attrs & AttrTrait;
  }
  static bool isAbstract(modifiers_type modifiers) {
    return modifiers & AttrAbstract;
  }

  // Whether to exclude methods with name `methName' when adding.
  static bool exclude(string_type methName) {
    return Func::isSpecial(methName);
  }

  // TraitMethod constructor.
  static TraitMethod traitMethod(class_type traitCls,
                                 method_type traitMeth,
                                 alias_type rule) {
    return TraitMethod { traitCls, traitMeth, rule.modifiers() };
  }

  // Accessors for the precedence rule type.
  static string_type precMethodName(prec_type rule) {
    return rule.methodName();
  }
  static string_type precSelectedTraitName(prec_type rule) {
    return rule.selectedTraitName();
  }
  static TraitNameSet      precOtherTraitNames(prec_type rule) {
    return rule.otherTraitNames();
  }

  // Accessors for the alias rule type.
  static string_type aliasTraitName(alias_type rule) {
    return rule.traitName();
  }

  static string_type aliasOrigMethodName(alias_type rule) {
    return rule.origMethodName();
  }
  static string_type aliasNewMethodName(alias_type rule) {
    return rule.newMethodName();
  }
  static modifiers_type aliasModifiers(alias_type rule) {
    return rule.modifiers();
  }

  // Register a trait alias once the trait class is found.
  static void addTraitAlias(const ClassInfo* cls, alias_type rule,
                            class_type traitCls) {
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

  static method_type findTraitMethod(class_type cls,
                                     class_type traitCls,
                                     string_type origMethName) {
    auto it = traitCls->methods.find(origMethName);
    if (it == traitCls->methods.end()) return nullptr;
    return it->second.func;
  }

  // Errors.
  static void errorUnknownMethod(prec_type rule) {
    throw TMIException(folly::sformat("Unknown method '{}'",
                                      rule.methodName()));
  }
  static void errorUnknownMethod(alias_type rule,
                                 string_type methName) {
    throw TMIException(folly::sformat("Unknown method '{}'", methName));
  }
  template <class Rule>
  static void errorUnknownTrait(const Rule& rule,
                                string_type traitName) {
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
  static void errorMultiplyExcluded(prec_type rule,
                                    string_type traitName,
                                    string_type methName) {
    throw TMIException(folly::sformat("MultiplyExcluded: {}::{}",
                                      traitName, methName));
  }
};

using TMIData = TraitMethodImportData<TraitMethod,
                                      TMIOps,
                                      SString,
                                      string_data_hash,
                                      string_data_isame>;

bool build_cls_info_rec(IndexData& index,
                        borrowed_ptr<ClassInfo> rleaf,
                        borrowed_ptr<const ClassInfo> rparent,
                        bool fromTrait) {
  if (!rparent) return true;

  auto const isIface = rparent->cls->attrs & AttrInterface;

  if (!build_cls_info_rec(index, rleaf, rparent->parent, false)) return false;
  for (auto const iface : rparent->declInterfaces) {
    if (!build_cls_info_rec(index, rleaf, iface, fromTrait)) return false;
  }
  for (auto const trait : rparent->usedTraits) {
    if (!build_cls_info_rec(index, rleaf, trait, true)) return false;
  }

  /*
   * Make a flattened table of all the interfaces implemented by the class.
   */
  if (isIface) {
    rleaf->implInterfaces[rparent->cls->name] = rparent;
  }

  for (auto& c : rparent->cls->constants) {
    auto& cptr = rleaf->clsConstants[c.name];
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
             rleaf->cls->name, c.name,
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
      if (isIface) {
        ITRACE(2,
               "build_cls_info_rec failed for `{}' because "
               "`{}' was defined by both `{}' and `{}'\n",
               rleaf->cls->name, c.name,
               rparent->cls->name, cptr->cls->name);
        return false;
      }
    }

    cptr = &c;
  }

  /*
   * Make a table of the methods on this class, excluding interface
   * methods (and trait methods, since they've already been
   * flattenned).
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
  if (!isIface &&
      (rparent == rleaf || !(rparent->cls->attrs & AttrTrait))) {
    auto methodOverride = [&] (auto& it,
                               const php::Func* meth,
                               Attr attrs,
                               SString name) {
      if (it->second.func->attrs & AttrFinal) {
        if (!is_mock_class(rparent->cls)) {
          ITRACE(2,
                 "build_cls_info_rec failed for `{}' because "
                 "`{}' tried to override final method `{}::{}'\n",
                 rleaf->cls->name,
                 rparent->cls->name, it->second.func->cls->name, name);
          return false;
        }
      }
      ITRACE(9,
             "  {}: overriding method {}::{} with {}::{}\n",
             rleaf->cls->name,
             it->second.func->cls->name, it->second.func->name,
             meth->cls->name, name);
      if (it->second.func->attrs & AttrPrivate) {
        it->second.hasPrivateAncestor = true;
      }
      it->second.func = meth;
      it->second.attrs = attrs;
      it->second.hasAncestor = true;
      it->second.topLevel = rparent == rleaf;
      if (it->first != name) {
        auto mte = it->second;
        rleaf->methods.erase(it);
        it = rleaf->methods.emplace(name, mte).first;
      }
      return true;
    };
    for (auto& m : rparent->cls->methods) {
      auto res = rleaf->methods.emplace(
        m->name,
        MethTabEntry { borrow(m), m->attrs, false, rparent == rleaf }
      );
      if (res.second) {
        ITRACE(9,
               "  {}: adding method {}::{}\n",
               rleaf->cls->name,
               rparent->cls->name, m->name);
        continue;
      }
      if (m->attrs & AttrTrait && m->attrs & AttrAbstract) {
        // abstract methods from traits never override anything.
        continue;
      }
      if (!methodOverride(res.first, borrow(m), m->attrs, m->name)) {
        return false;
      }
    }

    if (!(rparent->cls->attrs & AttrNoExpandTrait)) {
      try {
        TMIData tmid;
        for (auto const t : rparent->usedTraits) {
          for (auto const& m : t->methods) {
            auto const meth = m.second.func;

            TraitMethod traitMethod { t, meth, meth->attrs };
            tmid.add(traitMethod, m.first);
          }
          if (rparent == rleaf) {
            for (auto const c : index.classClosureMap[t->cls]) {
              auto const invoke = find_method(c, s_invoke.get());
              assertx(invoke);
              index.classExtraMethodMap[rleaf->cls].insert(invoke);
            }
          }
        }

        for (auto const& precRule : rparent->cls->traitPrecRules) {
          tmid.applyPrecRule(precRule, rparent);
        }
        for (auto const& aliasRule : rparent->cls->traitAliasRules) {
          tmid.applyAliasRule(aliasRule, rparent);
        }
        auto traitMethods = tmid.finish(rparent);

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
          auto res = rleaf->methods.emplace(
            mdata.name,
            MethTabEntry { method, attrs, false, rparent == rleaf }
          );
          if (res.second) {
            ITRACE(9,
                   "  {}: adding trait method {}::{}\n",
                   rleaf->cls->name,
                   method->cls->name, method->name);
          } else {
            if (attrs & AttrAbstract) continue;
            if (res.first->second.func->cls == rparent->cls) continue;
            if (!methodOverride(res.first, method, attrs, mdata.name)) {
              return false;
            }
          }
          if (rparent == rleaf) {
            index.classExtraMethodMap[rleaf->cls].insert(
              const_cast<php::Func*>(method));
          }
        }
      } catch (TMIOps::TMIException& ex) {
        ITRACE(2,
               "build_cls_info_rec failed for `{}' importing traits: {}\n",
               rleaf->cls->name, ex.what());
        return false;
      }
    }
  }

  return true;
}

bool find_constructor(borrowed_ptr<ClassInfo> cinfo) {
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

  // Parent class constructor if it isn't named 86ctor.
  if (cinfo->parent && cinfo->parent->ctor &&
      cinfo->parent->ctor->first != s_86ctor.get()) {
    cinfo->ctor = cinfo->parent->ctor;
    return true;
  }

  // Use the generated 86ctor.  Unless the class is abstract, this
  // must exist at this point or the bytecode is ill-formed.
  if (cinfo->cls->attrs & AttrAbstract) return true;
  cinfo->ctor = find_toplevel(s_86ctor.get());
  always_assert(cinfo->ctor);
  return true;
}

/*
 * Note: a cyclic inheritance chain will blow this up, but right now
 * we'll never get here in that case because hphpc currently just
 * modifies classes not to have that situation.  TODO(#3649211).
 *
 * This function return false if we are certain instantiating rleaf
 * would be a fatal at runtime.
 */
bool build_cls_info(IndexData& index, borrowed_ptr<ClassInfo> cinfo) {
  if (!build_cls_info_rec(index, cinfo, cinfo, false)) return false;

  if (!find_constructor(cinfo)) return false;

  for (auto& prop : cinfo->cls->properties) {
    if (!(prop.attrs & AttrPublic) || !(prop.attrs & AttrStatic)) {
      continue;
    }

    /*
     * If the initializer type is TUninit, it means an 86sinit provides the
     * actual initialization type.  So we don't want to include the Uninit
     * (which isn't really a user-visible type for the property) or by the time
     * we union things in we'll have inferred nothing much.
     */
    auto const tyRaw = from_cell(prop.val);
    auto const ty = tyRaw.subtypeOf(TUninit) ? TBottom : tyRaw;
    cinfo->publicStaticProps[prop.name] = PublicSPropEntry { ty, ty, false };
  }

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
  std::unordered_map<
    borrowed_ptr<const php::Class>,
    std::unordered_set<borrowed_ptr<const php::Class>>
  > closureMap;

  for (auto& c : unit.classes) {
    if (c->attrs & AttrEnum) {
      index.enums.insert({c->name, borrow(c)});
    }

    index.classes.insert({c->name, borrow(c)});

    for (auto& m : c->methods) {
      attribute_setter(m->attrs, false, AttrNoOverride);
      index.methods.insert({m->name, borrow(m)});
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
      closureMap[c->closureContextCls].insert(borrow(c));
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
    index.funcs.insert({f->name, borrow(f)});
  }

  for (auto& ta : unit.typeAliases) {
    index.typeAliases.insert({ta->name, borrow(ta)});
  }

  for (auto& ca : unit.classAliases) {
    index.classAliases.insert(ca.first);
    index.classAliases.insert(ca.second);
  }
}

struct NamingEnv {
  struct Define;
  struct Seen;

  borrowed_ptr<ClassInfo> try_lookup(SString name) const {
    auto const it = names.find(name);
    return it == end(names) ? nullptr : it->second;
  }

  borrowed_ptr<ClassInfo> lookup(SString name) const {
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

private:
  ISStringToOne<ClassInfo> names;
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
  explicit Define(NamingEnv& env, SString n, borrowed_ptr<ClassInfo> ci,
                  borrowed_ptr<const php::Class> cls)
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

void resolve_combinations(IndexData& index,
                          NamingEnv& env,
                          borrowed_ptr<const php::Class> cls) {

  auto resolve_one = [&] (SString name) {
    if (env.try_lookup(name)) return true;
    auto any = false;
    for (auto& kv : copy_range(index.classInfo, name)) {
      NamingEnv::Define def{env, name, kv.second, cls};
      resolve_combinations(index, env, cls);
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
  cinfo->baseList.push_back(borrow(cinfo));

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

  if (!build_cls_info(index, borrow(cinfo))) return;

  ITRACE(2, "  resolved: {}\n", cls->name);
  if (Trace::moduleEnabled(Trace::hhbbc_index, 3)) {
    for (auto const DEBUG_ONLY& iface : cinfo->implInterfaces) {
      ITRACE(3, "    implements: {}\n", iface.second->cls->name);
    }
    for (auto const DEBUG_ONLY& trait : cinfo->usedTraits) {
      ITRACE(3, "          uses: {}\n", trait->cls->name);
    }
  }
  index.allClassInfos.push_back(std::move(cinfo));
  index.classInfo.emplace(cls->name, borrow(index.allClassInfos.back()));
}

void preresolve(IndexData& index, NamingEnv& env, SString clsName) {
  if (index.classInfo.count(clsName)) return;

  ITRACE(2, "preresolve: {}\n", clsName);
  if (env.seen(clsName)) {
    ITRACE(3, "Circular inheritance detected: {}\n", clsName);
    return;
  }
  NamingEnv::Seen seen(env, clsName);
  {
    Trace::Indent indent;
    for (auto& kv : find_range(index.classes, clsName)) {
      if (kv.second->parentName) {
        preresolve(index, env, kv.second->parentName);
      }
      for (auto& i : kv.second->interfaceNames) {
        preresolve(index, env, i);
      }
      for (auto& t : kv.second->usedTraitNames) {
        preresolve(index, env, t);
      }
      resolve_combinations(index, env, kv.second);
    }
  }
  ITRACE(3, "preresolve: {} ({} resolutions)\n",
         clsName, index.classInfo.count(clsName));
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
      cparent->subclassList.push_back(borrow(cinfo));
    }
    if (!(cinfo->cls->attrs & AttrNoExpandTrait) &&
        cinfo->usedTraits.size()) {
      fixupTraits = true;
      compute_subclass_list_rec(index, borrow(cinfo), borrow(cinfo));
    }
  }
  if (fixupTraits) {
    // traits can be reached by multiple paths, so we need to uniquify
    // their subclassLists.
    for (auto& cinfo : index.allClassInfos) {
      if (cinfo->cls->attrs & AttrTrait) {
        auto& sub = cinfo->subclassList;
        std::sort(begin(sub), end(sub));
        sub.erase(
          std::unique(begin(sub), end(sub)),
          end(sub)
        );
      }
    }
  }
}

void define_func_family(IndexData& index, borrowed_ptr<ClassInfo> cinfo,
                        SString name, const MethTabEntryPair* mte) {
  index.funcFamilies.push_back(std::make_unique<FuncFamily>());
  auto const family = borrow(index.funcFamilies.back());
  family->isCtor = mte == cinfo->ctor;
  auto& funcs = family->possibleFuncs;
  for (auto& cleaf : cinfo->subclassList) {
    auto const leafFn = [&] () -> const MethTabEntryPair* {
      if (family->isCtor) return cleaf->ctor;
      auto const leafFnIt = cleaf->methods.find(name);
      if (leafFnIt == end(cleaf->methods)) return nullptr;
      return mteFromIt(leafFnIt);
    }();
    if (!leafFn) continue;
    if (leafFn->second.func->attrs & AttrInterceptable) {
      family->containsInterceptables = true;
    }
    funcs.push_back(leafFn);
  }

  std::sort(begin(funcs), end(funcs),
            [&] (const MethTabEntryPair* a, const MethTabEntryPair* b) {
              // We want a canonical order for the family. Putting the
              // one corresponding to cinfo first makes sense, because
              // the first one is used as the name for FCallD, after
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

  cinfo->methodFamilies.emplace(name, family);
}

void define_func_families(IndexData& index) {
  for (auto& cinfo : index.allClassInfos) {
    if (cinfo->cls->attrs & AttrTrait) continue;
    auto didCtor = false;
    for (auto& kv : cinfo->methods) {
      auto const mte = mteFromElm(kv);

      if (mte->second.attrs & AttrNoOverride) continue;
      if (mte == cinfo->ctor) {
        define_func_family(index, borrow(cinfo), s_construct.get(), mte);
        didCtor = true;
        continue;
      }
      if (mte->second.attrs & AttrPrivate) continue;
      if (is_special_method_name(mte->first)) continue;

      define_func_family(index, borrow(cinfo), mte->first, mte);
    }
    if (cinfo->ctor && !didCtor) {
      define_func_family(index, borrow(cinfo), s_construct.get(), cinfo->ctor);
    }
  }
}

/*
 * ConflictGraph maintains lists of interfaces that conflict with each other
 * due to being implemented by the same class.
 */
struct ConflictGraph {
  void add(borrowed_ptr<const php::Class> i, borrowed_ptr<const php::Class> j) {
    if (i == j) return;
    auto& conflicts = map[i];
    if (std::find(conflicts.begin(), conflicts.end(), j) != conflicts.end()) {
      return;
    }
    conflicts.push_back(j);
  }

  std::unordered_map<borrowed_ptr<const php::Class>,
                     std::vector<borrowed_ptr<const php::Class>>> map;
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

    classes.emplace_back(Cls{borrow(cinfo)});
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
Slot find_min_slot(borrowed_ptr<const php::Class> iface,
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
  std::vector<borrowed_ptr<const php::Class>>             ifaces;
  std::unordered_map<borrowed_ptr<const php::Class>, int> iface_uses;

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
  std::unordered_map<Slot, int> slot_uses;
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

bool has_magic_method(borrowed_ptr<const ClassInfo> cinfo, SString name) {
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
      bool const found = has_magic_method(borrow(cinfo), kv.first);
      any = any || found;
      (cinfo.get()->*kv.second.pmem).thisHas = found;
    }
    if (any) mark_magic_on_parents(*cinfo, *cinfo);
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
  // the units to the index.  Now start by, marking every
  // (non-interface, non-special) method as AttrNoOverride.
  for (auto& cinfo : index.allClassInfos) {
    if (cinfo->cls->attrs & AttrInterface) continue;
    if (!(cinfo->cls->attrs & AttrUnique)) continue;

    for (auto& m : cinfo->methods) {
      if (!(is_special_method_name(m.first))) {
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

//////////////////////////////////////////////////////////////////////

void check_invariants(borrowed_ptr<const ClassInfo> cinfo) {
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
      if (cinfo->ctor && borrow(m) == cinfo->ctor->second.func) {
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

  // Every FuncFamily is non-empty and contain functions with the same
  // name.
  for (auto& ffam : data.funcFamilies) {
    always_assert(!ffam->possibleFuncs.empty());
    if (ffam->isCtor) continue;
    auto const name = ffam->possibleFuncs.front()->first;
    for (auto const pf : ffam->possibleFuncs) {
      always_assert(pf->first->isame(name));
    }
  }

  for (auto& cinfo : data.allClassInfos) {
    check_invariants(borrow(cinfo));
  }
}

//////////////////////////////////////////////////////////////////////

Type context_sensitive_return_type(const Index& index,
                                   borrowed_ptr<FuncInfo> finfo,
                                   CallContext callCtx) {
  auto const callInsensitiveType = finfo->returnTy;

  constexpr auto max_interp_nexting_level = 2;
  static __thread uint32_t interp_nesting_level;

  // TODO(#3788877): more heuristics here would be useful.  (And even
  // functions without params might be worth interpreting in a
  // context-sensitive way once the context includes the type of
  // $this.)
  bool const tryContextSensitive = [&] {
    if (!options.ContextSensitiveInterp ||
        finfo->func->params.empty() ||
        interp_nesting_level + 1 >= max_interp_nexting_level ||
        callInsensitiveType == TBottom) {
      return false;
    }

    if (callCtx.args.size() < finfo->func->params.size()) return true;
    auto ctx = Context {
      finfo->func->unit,
      const_cast<php::Func*>(finfo->func),
      finfo->func->cls
    };
    for (auto i = 0; i < finfo->func->params.size(); i++) {
      if (RuntimeOption::EvalHardTypeHints) {
        auto const constraint = finfo->func->params[i].typeConstraint;
        if (constraint.hasConstraint() && !constraint.isTypeVar() &&
            !constraint.isTypeConstant()) {
          auto t = index.lookup_constraint(ctx, constraint);
          if (!callCtx.args[i].subtypeOf(t)) return true;
          if (callCtx.args[i] != t) return true;
          continue;
        }
      }
      if (callCtx.args[i].strictSubtypeOf(TInitCell)) return true;
    }
    return false;
  }();

  if (!tryContextSensitive) {
    return callInsensitiveType;
  }

  auto maybe_loosen_staticness = [&] (const Type& ty) {
    return callInsensitiveType.subtypeOf(TUnc) ? ty : loosen_staticness(ty);
  };

  if (index.frozen()) {
    ContextRetTyMap::const_accessor acc;
    if (finfo->contextualReturnTypes.find(acc, callCtx)) {
      return maybe_loosen_staticness(acc->second);
    }
    return callInsensitiveType;
  }

  ContextRetTyMap::accessor acc;
  if (finfo->contextualReturnTypes.insert(acc, callCtx)) {
    acc->second = TTop;
  } else if (acc->second == TBottom || is_scalar(acc->second)) {
    return maybe_loosen_staticness(acc->second);
  }

  auto contextType = [&] {
    ++interp_nesting_level;
    SCOPE_EXIT { --interp_nesting_level; };

    auto const calleeCtx = Context {
      finfo->func->unit,
      const_cast<borrowed_ptr<php::Func>>(finfo->func),
      finfo->func->cls
    };
    return analyze_func_inline(index, calleeCtx, callCtx.args).inferredReturn;
  }();

  if (!interp_nesting_level) {
    FTRACE(3,
           "Context sensitive type: {}\n"
           "Context insensitive type: {}\n",
           show(contextType), show(callInsensitiveType));
  }

  auto ret = intersection_of(std::move(callInsensitiveType),
                             std::move(contextType));

  if (ret.strictSubtypeOf(acc->second)) {
    acc->second = ret;
  }

  if (!interp_nesting_level) {
    ret = maybe_loosen_staticness(ret);
    FTRACE(3, "Context sensitive result: {}\n", show(ret));
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

PrepKind func_param_prep(borrowed_ptr<const php::Func> func,
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
    using F = borrowed_ptr<const php::Func>;
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

template<typename F>
auto visit_public_statics(const ClassInfo* cinfo, F fun) ->
  decltype(fun(cinfo)) {
  for (auto ci = cinfo; ci != nullptr; ci = ci->parent) {
    if (auto const ret = fun(ci)) return ret;
    if (ci->cls->attrs & AttrNoExpandTrait) continue;
    for (auto ct : ci->usedTraits) {
      if (auto const ret = visit_public_statics(ct, fun)) {
        return ret;
      }
    }
  }
  return {};
}

PublicSPropEntry lookup_public_static_impl(
  const IndexData& data,
  borrowed_ptr<const ClassInfo> cinfo,
  SString prop
) {
  auto const noInfo = PublicSPropEntry{TInitGen, TInitGen, true};

  if (data.publicSPropState != PublicSPropState::Valid) {
    return noInfo;
  }

  auto const knownClsPart = visit_public_statics(
    cinfo,
    [&] (const ClassInfo* ci) -> const PublicSPropEntry* {
      auto const it = ci->publicStaticProps.find(prop);
      if (it != end(ci->publicStaticProps)) {
        return &it->second;
      }
      return nullptr;
    }
  );

  auto const unkPart = [&]() -> borrowed_ptr<const Type> {
    auto unkIt = data.unknownClassSProps.find(prop);
    if (unkIt != end(data.unknownClassSProps)) {
      return &unkIt->second;
    }
    return nullptr;
  }();

  if (knownClsPart == nullptr) {
    return noInfo;
  }

  always_assert_flog(
    !knownClsPart->inferredType.subtypeOf(TBottom),
    "A public static property had type TBottom; probably "
    "was marked uninit but didn't show up in the class 86sinit."
  );
  if (unkPart != nullptr) {
    return PublicSPropEntry {
      union_of(knownClsPart->inferredType, *unkPart),
      union_of(knownClsPart->initializerType, *unkPart),
      true
    };
  }
  return *knownClsPart;
}

PublicSPropEntry lookup_public_static_impl(
  const IndexData& data,
  borrowed_ptr<const php::Class> cls,
  SString name
) {
  auto const classes = find_range(data.classInfo, cls->name);
  if (begin(classes) == end(classes) ||
      std::next(begin(classes)) != end(classes)) {
    return PublicSPropEntry{TInitGen, TInitGen, true};
  }
  return lookup_public_static_impl(data, begin(classes)->second, name);
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

Index::Index(borrowed_ptr<php::Program> program,
             rebuild* rebuild_exception)
  : m_data(std::make_unique<IndexData>())
{
  trace_time tracer("create index");

  m_data->funcInfo.resize(program->nextFuncId);

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

  NamingEnv env;
  for (auto& u : program->units) {
    Trace::Bump bumper{Trace::hhbbc, kSystemLibBump, is_systemlib_part(*u)};
    for (auto& c : u->classes) {
      // Classes with no possible resolutions won't get visited in the
      // mark_persistent pass; make sure everything starts off with
      // the attributes clear.
      attrSetter(c->attrs, false, AttrUnique | AttrPersistent);

      // Manually set closure classes to be unique to maintain invariance.
      if (is_closure(*c)) {
        attrSetter(c->attrs, true, AttrUnique);
      }
      preresolve(*m_data, env, c->name);
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
  // Use classInfo, rather than classes, because we can't mark a
  // uniquely named class as unique, if eg a parent was non-unique.
  mark_unique_entities(m_data->classInfo,
                       [&] (ClassInfo* cinfo, bool flag) {
                         attribute_setter(
                           cinfo->cls->attrs,
                           flag &&
                           !m_data->typeAliases.count(cinfo->cls->name) &&
                           !m_data->classAliases.count(cinfo->cls->name),
                           AttrUnique);
                       });
  mark_unique_entities(m_data->funcs,
                       [&] (const php::Func* func, bool flag) {
                         attribute_setter(func->attrs, flag, AttrUnique);
                       });

  // Part of the index building routines happens before the various asserted
  // index invariants hold.  These each may depend on computations from
  // previous functions, so be careful changing the order here.
  compute_subclass_list(*m_data);
  mark_no_override_methods(*m_data);    // uses AttrUnique
  define_func_families(*m_data);        // uses AttrNoOverride functions
  find_magic_methods(*m_data);          // uses the subclass lists
  compute_iface_vtables(*m_data);

  check_invariants(*m_data);

  mark_no_override_classes(*m_data);    // uses AttrUnique

  if (RuntimeOption::EvalCheckReturnTypeHints == 3) {
    trace_time tracer("initialize return types");
    std::vector<borrowed_ptr<const php::Func>> all_funcs;
    all_funcs.reserve(m_data->funcs.size() + m_data->methods.size());
    for (auto const fn : m_data->funcs) {
      all_funcs.push_back(fn.second);
    }
    for (auto const fn : m_data->methods) {
      all_funcs.push_back(fn.second);
    }

    parallel::for_each(all_funcs, [&] (borrowed_ptr<const php::Func> f) {
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

const CompactVector<borrowed_ptr<const php::Class>>*
Index::lookup_closures(borrowed_ptr<const php::Class> cls) const {
  auto const it = m_data->classClosureMap.find(cls);
  if (it != end(m_data->classClosureMap)) {
    return &it->second;
  }
  return nullptr;
}

const std::set<borrowed_ptr<php::Func>>*
Index::lookup_extra_methods(borrowed_ptr<const php::Class> cls) const {
  if (cls->attrs & AttrNoExpandTrait) return nullptr;
  auto const it = m_data->classExtraMethodMap.find(cls);
  if (it != end(m_data->classExtraMethodMap)) {
    return &it->second;
  }
  return nullptr;
}

//////////////////////////////////////////////////////////////////////

res::Class Index::resolve_class(borrowed_ptr<const php::Class> cls) const {

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


Index::ResolvedInfo Index::resolve_type_name(SString inName) const {
  folly::Optional<std::unordered_set<const void*>> seen;

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

folly::Optional<Type> Index::resolve_class_or_type_alias(
  const Context& ctx, SString name, const Type& candidate) const {

  auto const res = resolve_type_name(name);

  if (res.nullable && candidate.subtypeOf(TInitNull)) return TInitNull;

  if (res.type == AnnotType::Object) {
    auto resolve = [&] (const res::Class& rcls) -> folly::Optional<Type> {
      if (!interface_supports_non_objects(rcls.name()) ||
          candidate.subtypeOf(TObj)) {
        return subObj(rcls);
      }

      if (candidate.subtypeOf(TOptArr)) {
        if (interface_supports_array(rcls.name())) return TArr;
      } else if (candidate.subtypeOf(TOptVec)) {
        if (interface_supports_vec(rcls.name())) return TVec;
      } else if (candidate.subtypeOf(TOptDict)) {
        if (interface_supports_dict(rcls.name())) return TDict;
      } else if (candidate.subtypeOf(TOptKeyset)) {
        if (interface_supports_keyset(rcls.name())) return TKeyset;
      } else if (candidate.subtypeOf(TOptStr)) {
        if (interface_supports_string(rcls.name())) return TStr;
      } else if (candidate.subtypeOf(TOptInt)) {
        if (interface_supports_int(rcls.name())) return TInt;
      } else if (candidate.subtypeOf(TOptDbl)) {
        if (interface_supports_double(rcls.name())) return TDbl;
      }
      return folly::none;
    };

    if (res.value.isNull()) return folly::none;

    auto ty = res.value.right() ?
      resolve({ this, res.value.right() }) :
      resolve({ this, res.value.left() });

    if (ty && res.nullable) *ty = opt(std::move(*ty));
    return ty;
  }

  return get_type_for_annotated_type(ctx, res.type, res.nullable,
                                     res.value.left(), candidate);
}

std::pair<res::Class,borrowed_ptr<php::Class>>
Index::resolve_closure_class(Context ctx, int32_t idx) const {
  auto const cls = borrow(ctx.unit->classes[idx]);
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

  /*
   * Whether or not the context class has a private method with the
   * same name as the the method we're trying to call.
   */
  auto const contextMayHavePrivateWithSameName = folly::lazy([&]() -> bool {
    if (!ctx.cls) return false;
    auto const range = find_range(m_data->classInfo, ctx.cls->name);
    if (begin(range) == end(range)) {
      // This class had no pre-resolved ClassInfos, which means it
      // always fatals in any way it could be defined, so it doesn't
      // matter what we return here (as all methods in the the context
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
      if (famIt->second->containsInterceptables) {
        return name_only();
      }
      return res::Func { this, famIt->second };
    }
  }
  not_reached();
}

folly::Optional<res::Func>
Index::resolve_ctor(Context ctx, res::Class rcls, bool exact) const {
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
  if (famIt->second->containsInterceptables) return folly::none;
  return res::Func { this, famIt->second };
}

template<class FuncRange>
res::Func
Index::resolve_func_helper(const FuncRange& funcs, SString name) const {
  auto name_only = [&] {
    return res::Func { this, res::Func::FuncName { name } };
  };

  // no resolution
  if (begin(funcs) == end(funcs)) return name_only();

  auto const func = begin(funcs)->second;

  // multiple resolutions
  if (std::next(begin(funcs)) != end(funcs)) {
    assert(!(func->attrs & AttrUnique));
    return name_only();
  }

  // single resolution
  if (func->attrs & AttrInterceptable) return name_only();

  // whole-program mode, that's it
  if (RuntimeOption::RepoAuthoritative) {
    assert(func->attrs & AttrUnique);
    return do_resolve(func);
  }

  // single-unit mode, check builtins
  if (func->attrs & AttrBuiltin) {
    assert(func->attrs & AttrUnique);
    return do_resolve(func);
  }

  // single-unit, non-builtin
  return name_only();
}

res::Func Index::resolve_func(Context /*ctx*/, SString name) const {
  name = normalizeNS(name);
  auto const funcs = find_range(m_data->funcs, name);
  return resolve_func_helper(funcs, name);
}

std::pair<res::Func, folly::Optional<res::Func>>
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
  return begin(r2) == end(r2)
    ? std::make_pair(resolve_func_helper(r1, nsName), folly::none)
    : std::make_pair(resolve_func_helper(r2, fallbackName), folly::none);
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
  assert(IMPLIES(
    !tc.hasConstraint() || tc.isTypeVar() || tc.isTypeConstant(),
    tc.isMixed()));

  if (getSuperType) {
    /*
     * Soft hints (@Foo) are not checked.
     */
    if (tc.isSoft()) return TCell;
  }

  if (auto const t = get_type_for_annotated_type(ctx,
                                                 tc.type(),
                                                 tc.isNullable(),
                                                 tc.typeName(),
                                                 candidate)) {
    return *t;
  }
  return getSuperType ? TInitCell : TBottom;
}

folly::Optional<Type> Index::get_type_for_annotated_type(
  Context ctx, AnnotType annot, bool nullable,
  SString name, const Type& candidate) const {

  if (candidate.subtypeOf(TInitNull) && nullable) {
    return TInitNull;
  }
  auto const mainType = [&]() -> const folly::Optional<Type> {
    switch (getAnnotMetaType(annot)) {
    case AnnotMetaType::Precise: {
      auto const dt = getAnnotDataType(annot);

      switch (dt) {
      case KindOfUninit:       return TBottom;
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
      case KindOfPersistentArray:
      case KindOfArray:        return TArr;
      case KindOfResource:     return TRes;
      case KindOfObject:
        return resolve_class_or_type_alias(ctx, name, candidate);
      case KindOfRef:
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
      return TGen;
    case AnnotMetaType::This:
      if (auto s = selfCls(ctx)) {
        if (!(*s).couldBeOverriden()) {
          return subObj(*s);
        }
      }
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
      if (candidate.subtypeOf(TInt)) return TInt;
      if (candidate.subtypeOf(TStr)) return TStr;
      return TArrKey;
    }
    return folly::none;
  }();

  if (!mainType || !nullable || mainType->couldBe(TInitNull)) return mainType;
  return opt(*mainType);
}

Type Index::lookup_constraint(Context ctx,
                              const TypeConstraint& tc,
                              const Type& t) const {
  return get_type_for_constraint<true>(ctx, tc, t);
}

bool Index::satisfies_constraint(Context ctx, const Type& t,
                                 const TypeConstraint& tc) const {
  return t.subtypeOf(get_type_for_constraint<false>(ctx, tc, t));
}

bool Index::is_async_func(res::Func rfunc) const {
  return match<bool>(
    rfunc.val, [&](res::Func::FuncName /*s*/) { return false; },
    [&](res::Func::MethodName /*s*/) { return false; },
    [&](borrowed_ptr<FuncInfo> finfo) {
      return finfo->func->isAsync && !finfo->func->isGenerator;
    },
    [&](borrowed_ptr<const MethTabEntryPair> mte) {
      return mte->second.func->isAsync && !mte->second.func->isGenerator;
    },
    [&](borrowed_ptr<FuncFamily> fam) {
      for (auto const pf : fam->possibleFuncs) {
        if (!pf->second.func->isAsync || pf->second.func->isGenerator) {
          return false;
        }
      }
      return true;
    });
}

bool Index::is_effect_free(res::Func rfunc) const {
  return match<bool>(
    rfunc.val,
    [&](res::Func::FuncName /*s*/) { return false; },
    [&](res::Func::MethodName /*s*/) { return false; },
    [&](borrowed_ptr<FuncInfo> finfo) {
      return finfo->effectFree;
    },
    [&](borrowed_ptr<const MethTabEntryPair> mte) {
      return func_info(*m_data, mte->second.func)->effectFree;
    },
    [&](borrowed_ptr<FuncFamily> fam) {
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
      auto const cinit = borrow(it->second->cls->methods.back());
      assert(cinit->name == s_86cinit.get());
      add_dependency(*m_data, cinit, ctx, Dep::ClsConst);
      return TInitCell;
    }
    return from_cell(it->second->val.value());
  }
  return TInitCell;
}

folly::Optional<Type> Index::lookup_constant(Context ctx,
                                             SString cnsName,
                                             SString fallbackName) const {
  auto it = m_data->constants.find(cnsName);
  if (it == m_data->constants.end()) {
    // flag to indicate that the constant isn't in the index yet.
    if (options.HardConstProp) return folly::none;
    return TInitCell;
  }

  if (it->second.readonly && fallbackName) {
    auto it2 = m_data->constants.find(fallbackName);
    if (it2 != m_data->constants.end() &&
        !it2->second.readonly) {
      it = std::move(it2);
    }
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

Type Index::lookup_foldable_return_type(Context ctx,
                                        borrowed_ptr<const php::Func> func,
                                        std::vector<Type> args) const {
  constexpr auto max_interp_nexting_level = 2;
  static __thread uint32_t interp_nesting_level;

  auto const& finfo = *func_info(*m_data, func);
  if (finfo.effectFree && is_scalar(finfo.returnTy)) {
    return finfo.returnTy;
  }
  add_dependency(*m_data, func, ctx, Dep::ReturnTy);

  auto const calleeCtx = CallContext {
    { func->unit, const_cast<php::Func*>(func), func->cls }, std::move(args)
  };

  auto showArgs DEBUG_ONLY = [] (const std::vector<Type>& a) {
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
  }

  if (frozen() || interp_nesting_level > max_interp_nexting_level) return TTop;

  auto const contextType = [&] {
    ++interp_nesting_level;
    SCOPE_EXIT { --interp_nesting_level; };

    auto const fa = analyze_func_inline(*this,
                                        calleeCtx.caller,
                                        calleeCtx.args,
                                        CollectionOpts::TrackConstantArrays |
                                        CollectionOpts::EffectFreeOnly);
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
    rfunc.val, [&](res::Func::FuncName /*s*/) { return TInitGen; },
    [&](res::Func::MethodName /*s*/) { return TInitGen; },
    [&](borrowed_ptr<FuncInfo> finfo) {
      add_dependency(*m_data, finfo->func, ctx, Dep::ReturnTy);
      return finfo->returnTy;
    },
    [&](borrowed_ptr<const MethTabEntryPair> mte) {
      add_dependency(*m_data, mte->second.func, ctx, Dep::ReturnTy);
      auto const finfo = func_info(*m_data, mte->second.func);
      if (!finfo->func) return TInitGen;
      return finfo->returnTy;
    },
    [&](borrowed_ptr<FuncFamily> fam) {
      auto ret = TBottom;
      for (auto const pf : fam->possibleFuncs) {
        add_dependency(*m_data, pf->second.func, ctx, Dep::ReturnTy);
        auto const finfo = func_info(*m_data, pf->second.func);
        if (!finfo->func) return TInitGen;
        ret |= finfo->returnTy;
      }
      return ret;
    });
}

Type Index::lookup_return_type(CallContext callCtx, res::Func rfunc) const {
  return match<Type>(
    rfunc.val,
    [&](res::Func::FuncName) {
      return lookup_return_type(callCtx.caller, rfunc);
    },
    [&](res::Func::MethodName) {
      return lookup_return_type(callCtx.caller, rfunc);
    },
    [&](borrowed_ptr<FuncInfo> finfo) {
      add_dependency(*m_data, finfo->func, callCtx.caller, Dep::ReturnTy);
      return context_sensitive_return_type(*this, finfo, callCtx);
    },
    [&](borrowed_ptr<const MethTabEntryPair> mte) {
      add_dependency(*m_data, mte->second.func, callCtx.caller, Dep::ReturnTy);
      auto const finfo = func_info(*m_data, mte->second.func);
      if (!finfo->func) return TInitGen;
      return context_sensitive_return_type(*this, finfo, callCtx);
    },
    [&](borrowed_ptr<FuncFamily> /*fam*/) {
      return lookup_return_type(callCtx.caller, rfunc);
    });
}

std::vector<Type>
Index::lookup_closure_use_vars(borrowed_ptr<const php::Func> func) const {
  assert(func->isClosureBody);

  auto const numUseVars = closure_num_use_vars(func);
  if (!numUseVars) return {};
  auto const it = m_data->closureUseVars.find(func->cls);
  if (it == end(m_data->closureUseVars)) {
    return std::vector<Type>(numUseVars, TGen);
  }
  return it->second;
}

Type Index::lookup_return_type_raw(borrowed_ptr<const php::Func> f) const {
  auto it = func_info(*m_data, f);
  if (it->func) {
    assertx(it->func == f);
    return it->returnTy;
  }
  return TInitGen;
}

CompactVector<Type>
Index::lookup_local_static_types(borrowed_ptr<const php::Func> f) const {
  auto it = func_info(*m_data, f);
  if (it->func) {
    assertx(it->func == f);
    return it->localStaticTypes;
  }
  return {};
}

bool Index::lookup_this_available(borrowed_ptr<const php::Func> f) const {
  return (f->attrs & AttrRequiresThis) && !f->isClosureBody;
}

PrepKind Index::lookup_param_prep(Context /*ctx*/, res::Func rfunc,
                                  uint32_t paramId) const {
  return match<PrepKind>(
    rfunc.val,
    [&] (res::Func::FuncName s) {
      if (!RuntimeOption::RepoAuthoritative) return PrepKind::Unknown;
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
    [&] (borrowed_ptr<FuncInfo> finfo) {
      return func_param_prep(finfo->func, paramId);
    },
    [&] (borrowed_ptr<const MethTabEntryPair> mte) {
      return func_param_prep(mte->second.func, paramId);
    },
    [&] (borrowed_ptr<FuncFamily> fam) {
      assert(RuntimeOption::RepoAuthoritative);
      return prep_kind_from_set(fam->possibleFuncs, paramId);
    }
  );
}

PropState
Index::lookup_private_props(borrowed_ptr<const php::Class> cls) const {
  auto it = m_data->privatePropInfo.find(cls);
  if (it != end(m_data->privatePropInfo)) return it->second;
  return make_unknown_propstate(
    cls,
    [&] (const php::Prop& prop) {
      return (prop.attrs & AttrPrivate) && !(prop.attrs & AttrStatic);
    }
  );
}

PropState
Index::lookup_private_statics(borrowed_ptr<const php::Class> cls) const {
  auto it = m_data->privateStaticPropInfo.find(cls);
  if (it != end(m_data->privateStaticPropInfo)) return it->second;
  return make_unknown_propstate(
    cls,
    [&] (const php::Prop& prop) {
      return (prop.attrs & AttrPrivate) && (prop.attrs & AttrStatic);
    }
  );
}

Type Index::lookup_public_static(const Type& cls, const Type& name) const {
  auto const cinfo = [&] () -> borrowed_ptr<const ClassInfo> {
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

  auto const vname = tv(name);
  if (!vname || (vname && vname->m_type != KindOfPersistentString)) {
    return TInitGen;
  }
  auto const sname = vname->m_data.pstr;

  return lookup_public_static_impl(*m_data, cinfo, sname).inferredType;
}

Type Index::lookup_public_static(borrowed_ptr<const php::Class> cls,
                                 SString name) const {
  return lookup_public_static_impl(*m_data, cls, name).inferredType;
}

bool Index::lookup_public_static_immutable(borrowed_ptr<const php::Class> cls,
                                           SString name) const {
  return !lookup_public_static_impl(*m_data, cls, name).everModified;
}

void Index::fixup_public_static(borrowed_ptr<const php::Class> cls,
                                SString name, const Type& ty) const {
  auto const classes = find_range(m_data->classInfo, cls->name);
  for (auto& cinfo : classes) {
    if (cinfo.second->cls == cls) {
      auto const it = cinfo.second->publicStaticProps.find(name);
      if (it != end(cinfo.second->publicStaticProps)) {
        it->second.inferredType = it->second.initializerType = ty;
      }
    }
  }
}

Slot
Index::lookup_iface_vtable_slot(borrowed_ptr<const php::Class> cls) const {
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

void Index::refine_constants(const FuncAnalysis& fa,
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

    assert(t.subtypeOf(res.first->second.type));
    if (t != res.first->second.type) {
      res.first->second.type = t;
      find_deps(*m_data, func, Dep::ConstVal, deps);
    }
  }
  if (fa.readsUntrackedConstants) deps.emplace(dep_context(*m_data, fa.ctx));
}

void Index::fixup_return_type(borrowed_ptr<const php::Func> func,
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

void Index::refine_effect_free(borrowed_ptr<const php::Func> func, bool flag) {
  auto const finfo = create_func_info(*m_data, func);
  always_assert_flog(
    !finfo->effectFree || flag,
    "Index effectFree changed from true to false in {} {}{}.\n",
    func->unit->filename,
    func->cls ? folly::to<std::string>(func->cls->name->data(), "::") :
    std::string{},
    func->name);

  finfo->effectFree = flag;
}

void Index::refine_local_static_types(
  borrowed_ptr<const php::Func> func,
  const CompactVector<Type>& localStaticTypes) {

  auto const finfo = create_func_info(*m_data, func);
  if (localStaticTypes.empty()) {
    finfo->localStaticTypes.clear();
    return;
  }

  finfo->localStaticTypes.resize(localStaticTypes.size(), TTop);
  for (auto i = size_t{0}; i < localStaticTypes.size(); i++) {
    auto& indexTy = finfo->localStaticTypes[i];
    auto const& newTy = localStaticTypes[i];
    always_assert_flog(
      newTy.subtypeOf(indexTy),
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
    if (!newTy.strictSubtypeOf(indexTy)) continue;
    indexTy = newTy;
  }
}

void Index::init_return_type(const php::Func* func) {
  if (func->attrs & (AttrReference | AttrBuiltin | AttrTakesInOutParams) ||
      func->isMemoizeWrapper) {
    return;
  }

  auto const constraint = func->retTypeConstraint;
  if (constraint.isSoft() ||
      (RuntimeOption::EvalThisTypeHintLevel != 3 && constraint.isThis())) {
    return;
  }

  auto const finfo = create_func_info(*m_data, func);

  auto tcT = lookup_constraint(
    Context {
      func->unit,
      const_cast<php::Func*>(func),
      func->cls && func->cls->closureContextCls ?
        func->cls->closureContextCls : func->cls
    },
    constraint);

  if (!tcT.subtypeOf(TCell)) {
    tcT = TInitCell;
  } else {
    tcT = remove_uninit(std::move(tcT));

    if (is_specialized_obj(tcT)) {
      if (dobj_of(tcT).cls.couldBeInterfaceOrTrait()) {
        tcT = is_opt(tcT) ? TOptObj : TObj;
      }
    } else {
      tcT = loosen_all(std::move(tcT));
    }
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

void Index::refine_return_type(borrowed_ptr<const php::Func> func, Type t,
                               DependencyContextSet& deps) {
  auto const finfo = create_func_info(*m_data, func);

  always_assert_flog(
    t.subtypeOf(finfo->returnTy),
    "Index return type invariant violated in {} {}{}.\n"
    "   {} is not a subtype of {}\n",
    func->unit->filename->data(),
    func->cls ? folly::to<std::string>(func->cls->name->data(), "::")
              : std::string{},
    func->name->data(),
    show(t),
    show(finfo->returnTy)
  );

  if (!t.strictSubtypeOf(finfo->returnTy)) return;
  if (finfo->returnRefinments + 1 < options.returnTypeRefineLimit) {
    finfo->returnTy = t;
    ++finfo->returnRefinments;
    find_deps(*m_data, func, Dep::ReturnTy, deps);
    return;
  }
  FTRACE(1, "maxed out return type refinements on {}:{}\n",
         func->unit->filename, func->name);
}

bool Index::refine_closure_use_vars(borrowed_ptr<const php::Class> cls,
                                    const std::vector<Type>& vars) {
  assert(is_closure(*cls));

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
void refine_propstate(Container& cont,
                      borrowed_ptr<const php::Class> cls,
                      const PropState& state) {
  assertx(!is_used_trait(*cls));
  auto it = cont.find(cls);
  if (it == end(cont)) {
    cont[cls] = state;
    return;
  }
  for (auto& kv : state) {
    auto& target = it->second[kv.first];
    always_assert_flog(
      kv.second.subtypeOf(target),
      "PropState refinement failed on {}::${} -- {} was not a subtype of {}\n",
      cls->name->data(),
      kv.first->data(),
      show(kv.second),
      show(target)
    );
    target = kv.second;
  }
}

void Index::refine_private_props(borrowed_ptr<const php::Class> cls,
                                 const PropState& state) {
  refine_propstate(m_data->privatePropInfo, cls, state);
}

void Index::refine_private_statics(borrowed_ptr<const php::Class> cls,
                                   const PropState& state) {
  refine_propstate(m_data->privateStaticPropInfo, cls, state);
}

/*
 * Note: this routine is implemented to support refining the public static
 * types repeatedly (we could get plausibly better types for them sometimes by
 * doing that), but currently the tradeoff with compile time is probably not
 * worth it, and we're only doing one pass (see whole-program.cpp).  If we add
 * other 'whole program' passes that want to iterate, iterating this one at the
 * same time would probably be mostly free, so we can consider that later.
 */
void Index::refine_public_statics(const PublicSPropIndexer& indexer) {
  if (indexer.m_everything_bad ||
      m_data->publicSPropState == PublicSPropState::Invalid) {
    m_data->publicSPropState = PublicSPropState::Invalid;
    return;
  }
  auto const firstRefinement =
    m_data->publicSPropState == PublicSPropState::Unrefined;
  m_data->publicSPropState = PublicSPropState::Valid;

  for (auto& kv : indexer.m_unknown) {
    auto it = m_data->unknownClassSProps.find(kv.first);
    if (it == end(m_data->unknownClassSProps)) {
      m_data->unknownClassSProps.emplace(kv.first, kv.second);
      continue;
    }

    assert(!firstRefinement);
    always_assert_flog(
      kv.second.subtypeOf(it->second),
      "Static property index invariant violated for name {}:\n"
      "  {} was not a subtype of {}",
      kv.first->data(),
      show(kv.second),
      show(it->second)
    );

    it->second = kv.second;
  }

  for (auto& knownInfo : indexer.m_known) {
    auto const cinfo   = knownInfo.first.cinfo;
    auto const name    = knownInfo.first.prop;
    auto const newType = knownInfo.second;
    auto const it      = cinfo->publicStaticProps.find(name);

    FTRACE(2, "refine_public_statics: {} {} <-- {}\n",
      cinfo->cls->name,
      name,
      show(newType));

    // Cases where it's not public should've already been filtered out in the
    // indexer.
    always_assert_flog(
      it != end(cinfo->publicStaticProps),
      "Attempt to merge a public static property ({}) that wasn't declared "
      "on class {}",
      name->data(),
      cinfo->cls->name->data()
    );

    // The type from the indexer doesn't contain the in-class initializer
    // types.  Add that here.
    auto const effectiveType = union_of(newType, it->second.initializerType);

    /*
     * If refine_public_statics is called more than once, the subsequent calls
     * may only shrink the types we recorded for each property.  (If a property
     * type ever grows, the interpreter could infer something incorrect at some
     * step.)
     */
    if (!firstRefinement) {
      always_assert_flog(
        effectiveType.subtypeOf(it->second.inferredType),
        "Static property index invariant violated on {}::{}:\n"
        "  {} is not a subtype of {}",
        cinfo->cls->name->data(),
        name->data(),
        show(newType),
        show(it->second.inferredType)
      );
    }

    it->second.inferredType = effectiveType;
    it->second.everModified = true;
  }
}

bool Index::frozen() const {
  return m_data->frozen;
}

void Index::freeze() {
  m_data->frozen = true;
  m_data->ever_frozen = true;
}

void Index::thaw() {
  m_data->frozen = false;
}

std::unique_ptr<ArrayTypeTable::Builder>& Index::array_table_builder() const {
  return m_data->arrTableBuilder;
}

//////////////////////////////////////////////////////////////////////

res::Func Index::do_resolve(borrowed_ptr<const php::Func> f) const {
  auto const finfo = create_func_info(*m_data, f);
  return res::Func { this, finfo };
};

// Return true if we know for sure that one php::Class must derive
// from another at runtime, in all possible instantiations.
bool Index::must_be_derived_from(borrowed_ptr<const php::Class> cls,
                                 borrowed_ptr<const php::Class> parent) const {
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
Index::could_be_related(borrowed_ptr<const php::Class> cls,
                        borrowed_ptr<const php::Class> parent) const {
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

void PublicSPropIndexer::merge(Context ctx,
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
          return merge(ctx, cinfo, name, val);
        case DCls::Sub:
          for (auto const sub : cinfo->subclassList) {
            merge(ctx, sub, name, val);
          }
          return;
      }
      not_reached();
    }
  }

  merge(ctx, nullptr, name, val);
}

void PublicSPropIndexer::merge(Context ctx, ClassInfo* cinfo,
                               const Type& name, const Type& val) {
  FTRACE(2, "merge_public_static: {} {} {}\n",
         cinfo ? cinfo->cls->name->data() : "<unknown>", show(name), show(val));

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
      m_everything_bad = true;
      return;
    }

    UnknownMap::accessor acc;
    if (m_unknown.insert(acc, vname->m_data.pstr)) {
      acc->second = val;
    } else {
      acc->second |= val;
    }
    return;
  }

  /*
   * We don't know the name, but we know something about the class.  We need to
   * merge the type for every property in the class hierarchy.
   */
  if (unknownName) {
    visit_public_statics(cinfo,
                         [&] (const ClassInfo* ci) {
                           for (auto& kv : ci->publicStaticProps) {
                             merge(ctx, cinfo, sval(kv.first), val);
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
  auto const affectedCInfo = const_cast<ClassInfo*>(
    visit_public_statics(
      cinfo,
      [&] (const ClassInfo* ci) -> const ClassInfo* {
        if (ci->publicStaticProps.count(vname->m_data.pstr)) {
          return ci;
        }
        return nullptr;
      }
    )
  );

  if (!affectedCInfo) {
    // Either this was a mutation that's going to fatal (property doesn't
    // exist), or it's a private static or a protected static.  We aren't in
    // that business here, so we don't need to record anything.
    return;
  }

  // Merge the property type.
  KnownMap::accessor acc;
  if (m_known.insert(acc, KnownKey { affectedCInfo, vname->m_data.pstr })) {
    acc->second = val;
  } else {
    acc->second |= val;
  }
}

void PublicSPropIndexer::merge(Context ctx,
                               const php::Class& cls,
                               const Type& name,
                               const Type& val) {
  auto range = find_range(m_index->m_data->classInfo, cls.name);
  for (auto const& pair : range) {
    auto const cinfo = pair.second;
    if (cinfo->cls != &cls) continue;
    // Note that this works for both traits and regular classes
    for (auto const sub : cinfo->subclassList) {
      merge(ctx, sub, name, val);
    }
  }
}

//////////////////////////////////////////////////////////////////////

}}
