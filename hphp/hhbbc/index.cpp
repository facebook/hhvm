/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <folly/Format.h>
#include <folly/Hash.h>
#include <folly/Lazy.h>
#include <folly/MapUtil.h>
#include <folly/Memory.h>
#include <folly/Optional.h>
#include <folly/Range.h>
#include <folly/String.h>

#include "hphp/util/assertions.h"
#include "hphp/util/match.h"

#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit-util.h"

#include "hphp/hhbbc/type-builtins.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/options-util.h"
#include "hphp/hhbbc/analyze.h"

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
const StaticString s_86ctor("86ctor");

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

using G = std::lock_guard<std::mutex>;

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

enum class Dep : uintptr_t { ReturnTy = 0x1 };

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
 * (Currently only return types.)
 */
using DepMap =
  tbb::concurrent_hash_map<
    borrowed_ptr<const php::Func>,
    std::map<Context,Dep>
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
 * The reason for this is that in php, you can override private
 * methods with public or protected ones, which is a feature of a
 * given class hierarchy (ClassInfo), not a property of the class
 * definition itself.  When there's a private ancestor, we need to do
 * additional checks in resolve_method to make sure we're not possibly
 * calling from an ancestor class that defined a private method of
 * that name, since it will call that one instead.
 */
struct MethTabEntry {
  borrowed_ptr<const php::Func> func = nullptr;
  bool hasPrivateAncestor = false;
};

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
struct FuncFamily {
  bool containsInterceptables = false;
  std::vector<borrowed_ptr<FuncInfo>> possibleFuncs;
};

//////////////////////////////////////////////////////////////////////

/*
 * Currently inferred information about a PHP function.
 *
 * Nothing in this structure can ever be untrue.  The way the
 * algorithm works, whatever is in here must be factual (even if it is
 * not complete information), because we may deduce other facts based
 * on it.
 */
struct FuncInfo {
  /*
   * Pointer to the underlying php::Func this has information about.
   */
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
   * Whether $this can be null or not on entry to the method. Only applies
   * to method and it's always false for functions.
   */
  bool thisAvailable = false;

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
  ContextRetTyMap contextualReturnTypes;
};

/*
 * Known information about a particular possible instantiation of a
 * PHP class.  This is 1:1 with php::Class iff the php::Class has
 * AttrUnique.
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
  borrowed_ptr<const php::Func> ctor = nullptr;

  /*
   * Subclasses of this class, including this class itself (unless it
   * is an interface).
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
    magicUnset;
};

using MagicMapInfo = struct {
  ClassInfo::MagicFnInfo (ClassInfo::*pmem);
  Attr attrBit;
};

const std::vector<std::pair<SString,MagicMapInfo>> magicMethodMap {
  { s_call.get(),       { &ClassInfo::magicCall,       AttrNone } },
  { s_callStatic.get(), { &ClassInfo::magicCallStatic, AttrNone } },
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
    [] (borrowed_ptr<ClassInfo> ci) { return ci->cls->name; }
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

std::string show(const Class& c) {
  return c.val.match(
    [] (SString s) -> std::string {
      return s->data();
    },
    [] (borrowed_ptr<ClassInfo> cinfo) {
      return folly::format("{}*", cinfo->cls->name).str();
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
    [&] (borrowed_ptr<FuncFamily> fa) {
      auto const name = fa->possibleFuncs.front()->func->name;
      if (debug) {
        for (DEBUG_ONLY auto& f : fa->possibleFuncs) {
          assert(f->func->name->isame(name));
        }
      }
      return name;
    }
  );
}

bool Func::cantBeMagicCall() const {
  return match<bool>(
    val,
    [&] (FuncName s)                  { return true; },
    [&] (MethodName s)                { return false; },
    [&] (borrowed_ptr<FuncInfo> fi)   { return true; },
    [&] (borrowed_ptr<FuncFamily> fa) { return true; }
  );
}

std::string show(const Func& f) {
  std::string ret = f.name()->data();
  match<void>(
    f.val,
    [&] (Func::FuncName) {},
    [&] (Func::MethodName) {},
    [&] (borrowed_ptr<FuncInfo> fi) { ret += "*"; },
    [&] (borrowed_ptr<FuncFamily> fa) { ret += "+"; }
  );
  return ret;
}

}

//////////////////////////////////////////////////////////////////////

using IfaceSlotMap = std::unordered_map<borrowed_ptr<const php::Class>, Slot>;

struct IndexData {
  IndexData() = default;
  IndexData(const IndexData&) = delete;
  IndexData& operator=(const IndexData&) = delete;
  ~IndexData() = default;

  bool frozen{false};

  std::unique_ptr<ArrayTypeTable::Builder> arrTableBuilder;

  ISStringToMany<const php::Class>     classes;
  ISStringToMany<const php::Func>      methods;
  ISStringToMany<const php::Func>      funcs;
  ISStringToMany<const php::TypeAlias> typeAliases;
  ISStringToMany<const php::Class>     enums;

  // Map from each class to all the closures that are allocated in
  // functions of that class.
  std::unordered_map<
    borrowed_ptr<const php::Class>,
    std::unordered_set<borrowed_ptr<php::Class>>
  > classClosureMap;

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

  std::mutex funcInfoLock;
  std::unordered_map<borrowed_ptr<const php::Func>,FuncInfo> funcInfo;

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

  // For now we only need dependencies for function return types.
  DepMap dependencyMap;
};

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

void add_dependency(IndexData& data,
                    borrowed_ptr<const php::Func> src,
                    const Context& dst,
                    Dep newMask) {
  DepMap::accessor acc;
  data.dependencyMap.insert(acc, src);
  auto& current = acc->second[dst];
  current = current | newMask;
}

// Caller must ensure we are synchronized (either hold funcInfoLock or
// be in a single threaded situation).
borrowed_ptr<FuncInfo> create_func_info(IndexData& data,
                                        borrowed_ptr<const php::Func> f) {
  auto& ret = data.funcInfo[f];
  if (ret.func) return &ret;
  ret.func = f;
  if (f->nativeInfo) {
    // We'd infer this anyway when we look at the bytecode body
    // (NativeImpl) for the HNI function, but just initializing it
    // here saves on whole-program iterations.
    ret.returnTy = native_function_return_type(f);
  }
  ret.thisAvailable = false;
  return &ret;
}

std::vector<Context> find_deps(IndexData& data,
                               borrowed_ptr<const php::Func> src,
                               Dep mask) {
  std::vector<Context> ret;
  DepMap::const_accessor acc;
  if (data.dependencyMap.find(acc, src)) {
    for (auto& kv : acc->second) {
      if (has_dep(kv.second, mask)) ret.push_back(kv.first);
    }
  }
  return ret;
}

bool build_cls_info_rec(borrowed_ptr<ClassInfo> rleaf,
                        borrowed_ptr<const ClassInfo> rparent) {
  if (!rparent) return true;

  if (!build_cls_info_rec(rleaf, rparent->parent)) return false;
  for (auto& iface : rparent->declInterfaces) {
    if (!build_cls_info_rec(rleaf, iface)) return false;
  }

  auto const isIface = rparent->cls->attrs & AttrInterface;

  /*
   * Make a flattened table of all the interfaces implemented by the class.
   */
  if (isIface) {
    rleaf->implInterfaces[rparent->cls->name] = rparent;
  }

  /*
   * Make a table of all the constants on this class.
   *
   * Duplicate class constants override parent class constants, but
   * for interfaces it's an error to have a duplicate constant, unless
   * it just happens from implementing the same interface more than
   * once, or the constant is abstract.
   *
   * Note: hphpc doesn't actually check for this case, but since with
   * HardConstProp we're potentially doing propagation of these
   * constants without autoload, it seems like it could be potentially
   * surprising to propagate incorrect constants from mis-declared
   * classes that would fatal if they were ever defined.
   */
  for (auto& c : rparent->cls->constants) {
    auto& cptr = rleaf->clsConstants[c.name];
    if (isIface && cptr) {
      if (cptr->val.hasValue() && c.val.hasValue() &&
          cptr->cls != rparent->cls) {
        return false;
      }
    }
    cptr = &c;
  }

  /*
   * Make a table of the methods on this class, excluding interface methods.
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
  if (!isIface) {
    for (auto& m : rparent->cls->methods) {
      auto& ent = rleaf->methods[m->name];
      if (ent.func) {
        if (ent.func->attrs & AttrFinal) {
          if (!is_mock_class(rleaf->cls)) return false;
        }
        if (ent.func->attrs & AttrPrivate) {
          ent.hasPrivateAncestor =
            ent.hasPrivateAncestor || ent.func->cls != rleaf->cls;
        }
      }
      ent.func = borrow(m);
    }
  }

  return true;
}

borrowed_ptr<const php::Func> find_constructor(borrowed_ptr<ClassInfo> cinfo) {
  if (cinfo->cls->attrs & (AttrInterface|AttrTrait)) {
    return nullptr;
  }

  auto cit = cinfo->methods.find(s_construct.get());
  if (cit != end(cinfo->methods) && cit->second.func->cls == cinfo->cls) {
    return cit->second.func;
  }

  // Try old style class name constructors.  We need to check
  // AttrTrait on the function in case it came from a pre-flattened
  // trait (that doesn't count as a constructor).
  assert(!(cinfo->cls->attrs & AttrTrait) &&
         "We shouldn't be resolving traits right now");
  cit = cinfo->methods.find(cinfo->cls->name);
  if (cit != end(cinfo->methods)) {
    if (cit->second.func->cls == cinfo->cls) return cit->second.func;
  }

  // Parent class constructor if it isn't named 86ctor.
  if (cinfo->parent && cinfo->parent->ctor &&
      cinfo->parent->ctor->name != s_86ctor.get()) {
    return cinfo->parent->ctor;
  }

  // Use the generated 86ctor.  Unless the class is abstract, this
  // must exist at this point or the bytecode is ill-formed.
  if (cinfo->cls->attrs & AttrAbstract) return nullptr;
  cit = cinfo->methods.find(s_86ctor.get());
  if (cit == end(cinfo->methods)) {
    always_assert(!"no 86ctor found on class");
  }
  return cit->second.func;
}

/*
 * Note: a cyclic inheritance chain will blow this up, but right now
 * we'll never get here in that case because hphpc currently just
 * modifies classes not to have that situation.  TODO(#3649211).
 *
 * This function return false if we are certain instantiating rleaf
 * would be a fatal at runtime.
 */
bool build_cls_info(borrowed_ptr<ClassInfo> cinfo) {
  if (!build_cls_info_rec(cinfo, cinfo)) return false;

  cinfo->ctor = find_constructor(cinfo);

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

void add_unit_to_index(IndexData& index, const php::Unit& unit) {
  for (auto& c : unit.classes) {
    if (c->attrs & AttrEnum) {
      index.enums.insert({c->name, borrow(c)});
    }

    index.classes.insert({c->name, borrow(c)});

    for (auto& m : c->methods) {
      index.methods.insert({m->name, borrow(m)});

      if (is_interceptable_function(borrow(c), borrow(m))) {
        m->attrs = m->attrs | AttrInterceptable;
      }
    }

    if (c->closureContextCls) {
      index.classClosureMap[c->closureContextCls].insert(borrow(c));
    }
  }

  for (auto& f : unit.funcs) {
    if (is_interceptable_function(nullptr, borrow(f))) {
      f->attrs = f->attrs | AttrInterceptable;
    }
    index.funcs.insert({f->name, borrow(f)});
  }

  for (auto& ta : unit.typeAliases) {
    index.typeAliases.insert({ta->name, borrow(ta)});
  }
}

struct NamingEnv {
  struct Define;

  borrowed_ptr<ClassInfo> try_lookup(SString name) const {
    auto const it = names.find(name);
    return it == end(names) ? nullptr : it->second;
  }

  borrowed_ptr<ClassInfo> lookup(SString name) const {
    auto ret = try_lookup(name);
    always_assert(ret && "NamingEnv::lookup failed unexpectedly");
    return ret;
  }

private:
  ISStringToOne<ClassInfo> names;
};

struct NamingEnv::Define {
  explicit Define(NamingEnv& env, SString n, borrowed_ptr<ClassInfo> ci)
    : env(env)
    , n(n)
  {
    ITRACE(2, "defining {}\n", n->data());
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
  if (cls->attrs & AttrTrait) return;

  // Recurse with all combinations of bases and interfaces in the
  // naming environment.
  if (cls->parentName) {
    if (!env.try_lookup(cls->parentName)) {
      for (auto& kv : copy_range(index.classInfo, cls->parentName)) {
        NamingEnv::Define def{env, cls->parentName, kv.second};
        resolve_combinations(index, env, cls);
      }
      return;
    }
  }
  for (auto& iname : cls->interfaceNames) {
    if (!env.try_lookup(iname)) {
      for (auto& kv : copy_range(index.classInfo, iname)) {
        NamingEnv::Define def{env, iname, kv.second};
        resolve_combinations(index, env, cls);
      }
      return;
    }
  }

  // Everything is defined in the naming environment here.  (We
  // returned early if something didn't exist.)

  auto cinfo = folly::make_unique<ClassInfo>();
  cinfo->cls = cls;
  if (cls->parentName) {
    cinfo->parent   = env.lookup(cls->parentName);
    cinfo->baseList = cinfo->parent->baseList;
    if (cinfo->parent->cls->attrs & AttrInterface) return;
  }
  cinfo->baseList.push_back(borrow(cinfo));

  for (auto& iname : cls->interfaceNames) {
    auto const iface = env.lookup(iname);
    if (!(iface->cls->attrs & AttrInterface)) return;
    cinfo->declInterfaces.push_back(iface);
  }

  if (!build_cls_info(borrow(cinfo))) return;

  ITRACE(2, "  resolved: {}\n", cls->name->data());
  index.allClassInfos.push_back(std::move(cinfo));
  index.classInfo.emplace(cls->name, borrow(index.allClassInfos.back()));
}

void preresolve(IndexData& index, NamingEnv& env, SString clsName) {
  if (index.classInfo.count(clsName)) return;

  // TODO(#3649211): we'll need to handle inheritance cycles here
  // after hphpc is fixed not to just remove them.

  ITRACE(2, "preresolve: {}\n", clsName->data());
  for (auto& kv : find_range(index.classes, clsName)) {
    if (kv.second->parentName) {
      preresolve(index, env, kv.second->parentName);
    }
    for (auto& i : kv.second->interfaceNames) {
      preresolve(index, env, i);
    }
    resolve_combinations(index, env, kv.second);
  }
}

void compute_subclass_list(IndexData& index) {
  for (auto& cinfo : index.allClassInfos) {
    for (auto& cparent : cinfo->baseList) {
      cparent->subclassList.push_back(borrow(cinfo));
    }
  }
}

void define_func_family(IndexData& index,
                        borrowed_ptr<ClassInfo> cinfo,
                        SString name,
                        borrowed_ptr<const php::Func> func) {
  index.funcFamilies.push_back(folly::make_unique<FuncFamily>());
  auto const family = borrow(index.funcFamilies.back());

  for (auto& cleaf : cinfo->subclassList) {
    auto const leafFnIt = cleaf->methods.find(name);
    if (leafFnIt == end(cleaf->methods)) continue;
    if (leafFnIt->second.func->attrs & AttrInterceptable) {
      family->containsInterceptables = true;
    }
    auto const finfo = create_func_info(index, leafFnIt->second.func);
    family->possibleFuncs.push_back(finfo);
  }

  std::sort(begin(family->possibleFuncs), end(family->possibleFuncs));
  family->possibleFuncs.erase(
    std::unique(begin(family->possibleFuncs), end(family->possibleFuncs)),
    end(family->possibleFuncs)
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
  always_assert(!family->possibleFuncs.empty());

  cinfo->methodFamilies.emplace(name, family);
}

void define_func_families(IndexData& index) {
  for (auto& cinfo : index.allClassInfos) {
    for (auto& kv : cinfo->methods) {
      auto const func = kv.second.func;

      if (func->attrs & (AttrPrivate|AttrNoOverride)) continue;
      if (is_special_method_name(func->name))         continue;

      define_func_family(index, borrow(cinfo), kv.first, func);
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
  std::vector<borrowed_ptr<const php::Class>> ifaces;
  std::unordered_map<borrowed_ptr<const php::Class>, int> iface_uses;

  // Build up the conflict sets.
  for (auto& cinfo : index.allClassInfos) {
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

  // Finally, sort and reassign slots so the most frequently used slots come
  // first. This slightly reduces the number of wasted vtable vector entries at
  // runtime.
  std::vector<Slot> slots;
  slots.reserve(max_slot + 1);
  for (Slot i = 0; i <= max_slot; ++i) slots.emplace_back(i);
  auto slot_cmp = [&](Slot a, Slot b) { return slot_uses[a] > slot_uses[b]; };
  std::sort(begin(slots), end(slots), slot_cmp);

  std::vector<Slot> slots_permute(max_slot + 1, 0);
  for (size_t i = 0; i <= max_slot; ++i) slots_permute[slots[i]] = i;
  for (auto& pair : index.ifaceSlotMap) {
    pair.second = slots_permute[pair.second];
  }

  if (Trace::moduleEnabledRelease(Trace::hhbbc_iface)) {
    trace_interfaces(index, cg);
  }
}

void mark_magic_on_parents(ClassInfo& cinfo, ClassInfo& derived) {
  for (auto& kv : magicMethodMap) {
    if ((derived.*kv.second.pmem).thisHas) {
      (cinfo.*kv.second.pmem).derivedHas = true;
    }
  }
  if (cinfo.parent) return mark_magic_on_parents(*cinfo.parent, derived);
}

void find_magic_methods(IndexData& index) {
  for (auto& cinfo : index.allClassInfos) {
    auto& methods         = cinfo->methods;
    bool any = false;
    for (auto& kv : magicMethodMap) {
      bool const found = methods.find(kv.first) != end(methods);
      any = any || found;
      (cinfo.get()->*kv.second.pmem).thisHas = found;
    }
    if (any) mark_magic_on_parents(*cinfo, *cinfo);
  }
}

// We want const qualifiers on various index data structures for php object
// pointers, but during index creation time we need to manipulate some of their
// attributes (changing the representation).  These little wrappers keep the
// const_casting out of the main line of code below.
template<class PhpObject>
void add_attribute(borrowed_ptr<const PhpObject> obj, Attr attr) {
  const_cast<borrowed_ptr<PhpObject>>(obj)->attrs = obj->attrs | attr;
}
template<class PhpObject>
void remove_attribute(borrowed_ptr<const PhpObject> obj, Attr attr) {
  auto const newAttrs = static_cast<Attr>(obj->attrs & ~attr);
  const_cast<borrowed_ptr<PhpObject>>(obj)->attrs = newAttrs;
}

void mark_no_override_classes(IndexData& index) {
  for (auto& cinfo : index.allClassInfos) {
    if (cinfo->subclassList.size() == 1 &&
        !(cinfo->cls->attrs & AttrInterface)) {
      assert(cinfo->subclassList.front() == borrow(cinfo));
      if (!(cinfo->cls->attrs & AttrNoOverride)) {
        FTRACE(2, "Adding AttrNoOverride to {}\n", cinfo->cls->name->data());
      }
      add_attribute(cinfo->cls, AttrNoOverride);
    }

    for (auto& kv : magicMethodMap) {
      if (kv.second.attrBit == AttrNone) continue;
      if (!(cinfo.get()->*kv.second.pmem).derivedHas) {
        FTRACE(2, "Adding no-override of {} to {}\n",
          kv.first->data(),
          cinfo->cls->name->data());
        add_attribute(cinfo->cls, kv.second.attrBit);
      }
    }
  }
}

void mark_no_override_functions(IndexData& index) {
  // First, mark every (non-interface, non-special) method as AttrNoOverride.
  for (auto& meth : index.methods) {
    if (meth.second->cls->attrs & AttrInterface) continue;
    if (is_special_method_name(meth.second->name)) continue;
    add_attribute(meth.second, AttrNoOverride);
  }

  // Then run through every ClassInfo, and for each of its parent classes clear
  // the AttrNoOverride flag if it has a different Func with the same name.
  for (auto& cinfo : index.allClassInfos) {
    for (auto& ancestor : cinfo->baseList) {
      if (ancestor == cinfo.get()) continue;

      for (auto& derivedMethod : cinfo->methods) {
        auto const it = ancestor->methods.find(derivedMethod.first);
        if (it == end(ancestor->methods)) continue;
        if (it->second.func != derivedMethod.second.func) {
          if (it->second.func->attrs & AttrNoOverride) {
            FTRACE(2, "Removing AttrNoOverride on {}::{}\n",
              it->second.func->cls->name->data(),
              it->second.func->name->data());
          }
          remove_attribute(it->second.func, AttrNoOverride);
        }
      }
    }
  }
}

void mark_unique_type_aliases(IndexData& index) {
  for (auto& ta : index.typeAliases) {
    auto& name = ta.first;
    if (index.typeAliases.count(name) == 1 && index.classes.count(name) == 0) {
      const_cast<borrowed_ptr<php::TypeAlias>>(ta.second)->attrs
        = AttrUnique | AttrPersistent;
    }
  }
}

//////////////////////////////////////////////////////////////////////

void check_invariants(borrowed_ptr<const ClassInfo> cinfo) {
  // All the following invariants only apply to non-interfaces.
  if (cinfo->cls->attrs & AttrInterface) return;

  // For non-interface classes, each method in a php class has an
  // entry in its ClassInfo method table, and if it's not special,
  // AttrNoOverride, or private, an entry in the family table.
  for (auto& m : cinfo->cls->methods) {
    always_assert(cinfo->methods.count(m->name));
    if (m->attrs & (AttrNoOverride|AttrPrivate)) continue;
    if (is_special_method_name(m->name)) continue;
    always_assert(cinfo->methodFamilies.count(m->name));
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
    always_assert(info.thisHas ==
      (cinfo->methods.find(kv.first) != end(cinfo->methods)));

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
    if (cls->attrs & AttrTrait)     continue;

    auto const range = find_range(data.classInfo, name);
    if (begin(range) != end(range)) {
      always_assert(std::next(begin(range)) == end(range));
    }
  }

  // Note: it's tempting to say every unique ClassInfo object has an
  // AttrUnique class, but that may or may not apply right now.
  // Depends on hphpc.

  // Every FuncFamily is non-empty and contain functions with the same
  // name.
  for (auto& ffam : data.funcFamilies) {
    always_assert(!ffam->possibleFuncs.empty());
    auto const name = ffam->possibleFuncs.front()->func->name;
    for (auto& finfo : ffam->possibleFuncs) {
      always_assert(finfo->func->name->isame(name));
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
  bool const tryContextSensitive =
    options.ContextSensitiveInterp &&
    !finfo->func->params.empty() &&
    interp_nesting_level + 1 < max_interp_nexting_level;
  if (!tryContextSensitive) {
    return callInsensitiveType;
  }

  ContextRetTyMap::accessor acc;
  if (finfo->contextualReturnTypes.insert(acc, callCtx)) {
    acc->second = TTop;
  }

  auto const contextType = [&] {
    if (index.frozen()) return acc->second;

    ++interp_nesting_level;
    SCOPE_EXIT { --interp_nesting_level; };

    auto const calleeCtx = Context {
      finfo->func->unit,
      const_cast<borrowed_ptr<php::Func>>(finfo->func),
      finfo->func->cls
    };
    return analyze_func_inline(index, calleeCtx, callCtx.args).inferredReturn;
  }();

  if (!index.frozen()) {
    if (contextType.strictSubtypeOf(acc->second)) {
      acc->second = contextType;
    }
  }

  // TODO(#4441939): we can't do anything if it's a strict subtype of
  // array because of the lack of intersection types.  See below.
  if (contextType.strictSubtypeOf(TArr)) return callInsensitiveType;

  /*
   * Note: it may seem like the context sensitive return type should
   * always be at least as good as the context insensitive one, but
   * this doesn't hold in general, because we have a constant maximum
   * nesting depth.  I.e. we could get a better type in the normal,
   * context insensitive case because we're doing 'inlining'
   * (context-sensitive) interprets on the callees instead, and there
   * must be some maximum depth that we'll do that.
   *
   * For example, if the max_interp_nexting_level is two, consider the
   * following functions:
   *
   *       Function:                         Context-insensitive return type
   *
   *    function f($x) { return $x; }                    InitCell
   *
   *    function g($x) { return f($x ? 1 : 2); }         Int
   *
   * Now given the following:
   *
   *    function foo() { return g(true); }
   *
   * We'll inline interpret g, but not f (exceeds maximum depth), so
   * we'll determine g returned InitCell here, which is worse than
   * it's context-insensitive type of Int.
   */
  if (!contextType.subtypeOf(callInsensitiveType)) {
    FTRACE(1, "{} <= {} didn't happen on {} called from {} ({})\n",
      show(contextType),
      show(callInsensitiveType),
      finfo->func->name->data(),
      callCtx.caller.func->name->data(),
      callCtx.caller.cls ? callCtx.caller.cls->name->data() : "");
    return callInsensitiveType;
  }

  /*
   * TODO(#4441939): for this to be safe for the index invariants on
   * return types, we need to be intersecting the types here.  This is
   * because aggregate types (various array subtypes) could have some
   * parts become more refined when inferring it in a
   * context-sensitive way, while other parts are less refined.  If
   * you take one or the other it's possible for normal context
   * insensitive return types in the index to get bigger instead of
   * getting smaller.
   */
  return contextType;
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
  if (begin(range) == end(range)) {
    /*
     * We can assume it's by value, because either we're calling a function
     * that doesn't exist (about to fatal), or we're going to an __call (which
     * never takes parameters by reference).
     *
     * Or if we've got AllFuncsInterceptable we need to assume someone could
     * rename a function to the new name.
     */
    return options.AllFuncsInterceptable ? PrepKind::Unknown : PrepKind::Val;
  }

  struct FuncFind {
    using F = borrowed_ptr<const php::Func>;
    static F get(std::pair<SString,F> p) { return p.second; }
    static F get(borrowed_ptr<FuncInfo> finfo) { return finfo->func; }
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

PublicSPropEntry lookup_public_static_impl(
  const IndexData& data,
  borrowed_ptr<const ClassInfo> cinfo,
  SString prop
) {
  auto const noInfo = PublicSPropEntry{TInitGen, TInitGen, true};

  if (data.publicSPropState != PublicSPropState::Valid) {
    return noInfo;
  }

  auto const knownClsPart = [&] () -> borrowed_ptr<const PublicSPropEntry> {
    for (auto ci = cinfo; ci != nullptr; ci = ci->parent) {
      auto const it = ci->publicStaticProps.find(prop);
      if (it != end(ci->publicStaticProps)) {
        return &it->second;
      }
    }
    return nullptr;
  }();
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

Index::Index(borrowed_ptr<php::Program> program)
  : m_data(folly::make_unique<IndexData>())
{
  trace_time tracer("create index");

  m_data->arrTableBuilder.reset(new ArrayTypeTable::Builder());

  for (auto& u : program->units) {
    add_unit_to_index(*m_data, *u);
  }

  NamingEnv env;
  for (auto& u : program->units) {
    Trace::Bump bumper{Trace::hhbbc, kSystemLibBump, is_systemlib_part(*u)};
    for (auto& c : u->classes) {
      preresolve(*m_data, env, c->name);
    }
  }

  // Part of the index building routines happens before the various asserted
  // index invariants hold.  These each may depend on computations from
  // previous functions, so be careful changing the order here.
  compute_subclass_list(*m_data);
  mark_no_override_functions(*m_data);
  define_func_families(*m_data);        // uses AttrNoOverride functions
  find_magic_methods(*m_data);          // uses the subclass lists
  compute_iface_vtables(*m_data);

  check_invariants(*m_data);

  mark_no_override_classes(*m_data);
  mark_unique_type_aliases(*m_data);
}

// Defined here so IndexData is a complete type for the unique_ptr
// destructor.
Index::~Index() {}

//////////////////////////////////////////////////////////////////////

std::vector<borrowed_ptr<php::Class>>
Index::lookup_closures(borrowed_ptr<const php::Class> cls) const {
  std::vector<borrowed_ptr<php::Class>> ret;
  auto const it = m_data->classClosureMap.find(cls);
  if (it != end(m_data->classClosureMap)) {
    ret.assign(begin(it->second), end(it->second));
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////

folly::Optional<res::Class> Index::resolve_class(Context ctx,
                                                 SString clsName) const {
  clsName = normalizeNS(clsName);

  // We know it has to name a class only if there's no type alias with this
  // name.
  //
  // TODO(#3519401): when we start unfolding type aliases, we could
  // look at whether it is an alias for a specific class here.
  // (Note this might need to split into a different API: type
  // aliases aren't allowed everywhere we're doing resolve_class
  // calls.)
  if (m_data->typeAliases.count(clsName)) {
    return folly::none;
  }

  auto name_only = [&] () -> folly::Optional<res::Class> {
    // We also refuse to have name-only resolutions of enums, so that
    // all name only resolutions can be treated as objects.
    if (!m_data->enums.count(clsName)) {
      return res::Class { this, clsName };
    }
    return folly::none;
  };

  /*
   * If there's only one preresolved ClassInfo, we can give out a
   * specific res::Class for it.  (Any other possible resolutions were
   * known to fatal, or it was actually unique.)
   */
  auto const classes = find_range(m_data->classInfo, clsName);
  for (auto it = begin(classes); it != end(classes); ++it) {
    auto const cinfo = it->second;
    if ((cinfo->cls->attrs & AttrUnique) || std::next(it) == end(classes)) {
      if (debug && std::next(it) != end(classes)) {
        std::fprintf(stderr, "non unique \"unique\" class: %s\n",
          cinfo->cls->name->data());
        for (; it != end(classes); ++it) {
          std::fprintf(stderr, "   and %s\n", cinfo->cls->name->data());
        }
        assert(0);
      }
      return res::Class { this, cinfo };
    }
    break;
  }

  return name_only();
}

std::pair<res::Class,borrowed_ptr<php::Class>>
Index::resolve_closure_class(Context ctx, SString name) const {
  auto const rcls = resolve_class(ctx, name);

  // Closure classes must be unique and defined in the unit that uses
  // the CreateCl opcode, so resolution must succeed.
  always_assert_flog(
    rcls.hasValue() && rcls->val.right(),
    "A Closure class ({}) failed to resolve",
    name->data()
  );

  return {
    *rcls,
    const_cast<borrowed_ptr<php::Class>>(rcls->val.right()->cls)
  };
}

res::Class Index::builtin_class(SString name) const {
  auto const rcls = resolve_class(Context {}, name);
  if (!rcls) {
    std::fprintf(stderr, "failed to resolve a builtin class: %s\n",
      name->data());
    std::abort();
  }
  assert(rcls->val.right() &&
    (rcls->val.right()->cls->attrs & AttrBuiltin));
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
  auto const contextHasPrivateWithSameName = folly::lazy([&]() -> bool {
    if (!ctx.cls) return false;
    // We can use any representative ClassInfo for the context class
    // to check this, since the private method list cannot change
    // for different realizations of the class.
    auto const range = find_range(m_data->classInfo, ctx.cls->name);
    if (begin(range) == end(range)) {
      // This class had no pre-resolved ClassInfos, which means it
      // always fatals in any way it could be defined, so it doesn't
      // matter what we return here (as all methods in the the context
      // class are unreachable code).
      return true;
    }
    auto const ctxInfo = begin(range)->second;
    auto const iter    = ctxInfo->methods.find(name);
    if (iter != end(ctxInfo->methods)) {
      return iter->second.func->attrs & AttrPrivate;
    }
    return false;
  });

  /*
   * Look up the method in the target class.
   */
  auto const methIt = cinfo->methods.find(name);
  if (methIt == end(cinfo->methods)) return name_only();
  if (methIt->second.func->attrs & AttrInterceptable) return name_only();
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
      if (contextHasPrivateWithSameName()) {
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
    if (ftarget->attrs & AttrPublic) return false;
    // An anonymous context won't have access if it wasn't public.
    if (!ctx.cls) return true;
    // If the calling context class is the same as the target class,
    // and method is defined on this class, it must be accessible.
    if (ctx.cls == cinfo->cls && ftarget->cls == cinfo->cls) {
      return false;
    }
    // If the method is private, the above case is the only case where
    // we'd know it was accessible.
    if (ftarget->attrs & AttrPrivate) return true;
    /*
     * For the protected method case: if the context class must be
     * derived from the class that first defined the protected method
     * we know it is accessible.
     */
    if (must_be_derived_from(ctx.cls, ftarget->cls)) {
      return false;
    }
    /*
     * On the other hand, if the class that defined the method must be
     * derived from the context class, it is going to be accessible as
     * long as the context class does not define a private method with
     * the same name.  (If it did, we'd be calling that private
     * method, which currently we don't ever resolve---we've removed
     * it from the method table in the classInfo.)
     */
    if (must_be_derived_from(ftarget->cls, ctx.cls)) {
      if (!contextHasPrivateWithSameName()) {
        return false;
      }
    }
    // Other cases we're not sure about (maybe some non-unique classes
    // got in the way).  Conservatively return that it might be
    // inaccessible.
    return true;
  };

  switch (dcls.type) {
  case DCls::Exact:
    if (cinfo->magicCall.thisHas || cinfo->magicCallStatic.thisHas) {
      if (couldBeInaccessible()) return name_only();
    }
    return do_resolve(ftarget);
  case DCls::Sub:
    if (cinfo->magicCall.derivedHas || cinfo->magicCallStatic.derivedHas) {
      if (couldBeInaccessible()) return name_only();
    }
    if (ftarget->attrs & AttrNoOverride) {
      return do_resolve(ftarget);
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

folly::Optional<res::Func> Index::resolve_ctor(Context ctx,
                                               res::Class rcls) const {
  auto const cinfo = rcls.val.right();
  if (!cinfo || !cinfo->ctor) return folly::none;
  if (cinfo->ctor->attrs & AttrInterceptable) return folly::none;
  return do_resolve(cinfo->ctor);
}

template<class FuncRange>
res::Func
Index::resolve_func_helper(const FuncRange& funcs, SString name) const {
  auto name_only = [&] {
    return res::Func { this, res::Func::FuncName { name } };
  };

  if (begin(funcs) == end(funcs))              return name_only();
  if (std::next(begin(funcs)) != end(funcs))   return name_only();
  auto const func = begin(funcs)->second;
  if (!(func->attrs & AttrUnique))             return name_only();
  if (func->attrs & AttrInterceptable)         return name_only();

  return do_resolve(func);
}

res::Func Index::resolve_func(Context ctx, SString name) const {
  name = normalizeNS(name);
  auto const funcs = find_range(m_data->funcs, name);
  return resolve_func_helper(funcs, name);
}

folly::Optional<res::Func>
Index::resolve_func_fallback(Context ctx,
                             SString nsName,
                             SString fallbackName) const {
  assert(!needsNSNormalization(nsName));
  assert(!needsNSNormalization(fallbackName));

  // It's possible that in some requests nsName might succeed, while
  // in others fallbackName must succeed.  Both ranges must be
  // considered before we can decide which function we're after.
  auto const r1 = find_range(m_data->funcs, nsName);
  auto const r2 = find_range(m_data->funcs, fallbackName);
  if (begin(r1) != end(r1) && begin(r2) != end(r2)) {
    // It could come from either at runtime.  (We could try to rule it
    // out by figuring out if one must be defined based on the
    // ctx.unit, but it's unlikely to matter for now.)
    return folly::none;
  }
  return begin(r1) == end(r1) ? resolve_func_helper(r2, fallbackName)
                              : resolve_func_helper(r1, nsName);
}

Type Index::lookup_constraint(Context ctx, const TypeConstraint& tc) const {
  assert(IMPLIES(
    !tc.hasConstraint() || tc.isTypeVar() || tc.isTypeConstant(),
    tc.isMixed()));

  /*
   * Soft hints (@Foo) are not checked.
   */
  if (tc.isSoft()) return TCell;

  switch (tc.metaType()) {
    case AnnotMetaType::Precise: {
      auto const mainType = [&]() -> const Type {
        auto const dt = tc.underlyingDataType();
        assert(dt.hasValue());

        switch (*dt) {
        case KindOfUninit:       return TCell;
        case KindOfNull:         return TNull;
        case KindOfBoolean:      return TBool;
        case KindOfInt64:        return TInt;
        case KindOfDouble:       return TDbl;
        case KindOfString:       return TStr;
        case KindOfStaticString: return TStr;
        case KindOfArray:        return TArr;
        case KindOfResource:     return TRes;
        case KindOfObject:
          /*
           * If we can resolve the class, we'll give an object of a type
           * according to that resolution.  Some classes in hhvm can be
           * "enums", which we need to handle as non-object types.  A name-only
           * resolution is guaranteed to be some kind of non-enum,
           * non-type-alias object (see resolve_class).
           */
          if (auto const rcls = resolve_class(ctx, tc.typeName())) {
            if (auto const cinfo = rcls->val.right()) {
              if (cinfo->cls->attrs & AttrEnum) {
                return lookup_constraint(ctx, cinfo->cls->enumBaseTy);
              }
            }
            return interface_supports_non_objects(rcls->name())
              ? TInitCell // none of these interfaces support Uninits
              : subObj(*rcls);
          }
          return TInitCell;
        case KindOfClass:
        case KindOfRef:
          always_assert_flog(false, "Unexpected DataType");
          break;
        }
        return TInitCell;
      }();

      return (mainType == TInitCell || !tc.isNullable())
        ? mainType
        : opt(mainType);
    }
    case AnnotMetaType::Mixed:
      /*
       * Here we handle "mixed", typevars, and some other ignored
       * typehints (ex. "(function(..): ..)" typehints).
       */
      return TCell;
    case AnnotMetaType::Self:
    case AnnotMetaType::Parent:
    case AnnotMetaType::Callable:
      break;
    case AnnotMetaType::Number:
      return tc.isNullable() ? TOptNum : TNum;
    case AnnotMetaType::ArrayKey:
      // TODO(3774082): Support TInt | TStr type constraint
      return TInitCell;
  }

  return TCell;
}

bool Index::satisfies_constraint(Context ctx, const Type t,
                                 const TypeConstraint& tc) const {
  return t.subtypeOf(satisfies_constraint_helper(ctx, tc));
}

Type Index::satisfies_constraint_helper(Context ctx,
                                        const TypeConstraint& tc) const {
  assert(IMPLIES(
    !tc.hasConstraint() || tc.isTypeVar() || tc.isTypeConstant(),
    tc.isMixed()));

  switch (tc.metaType()) {
    case AnnotMetaType::Precise: {
      auto const mainType = [&]() -> const Type {
        auto const dt = tc.underlyingDataType();
        assert(dt.hasValue());
        switch (*dt) {
        case KindOfUninit:       return TBottom;
        case KindOfNull:         return TNull;
        case KindOfBoolean:      return TBool;
        case KindOfInt64:        return TInt;
        case KindOfDouble:       return TDbl;
        case KindOfString:       return TStr;
        case KindOfStaticString: return TStr;
        case KindOfArray:        return TArr;
        case KindOfResource:     return TRes;
        case KindOfObject:
          if (auto const rcls = resolve_class(ctx, tc.typeName())) {
            if (auto const cinfo = rcls->val.right()) {
              if (cinfo->cls->attrs & AttrEnum) {
                return
                  satisfies_constraint_helper(ctx, cinfo->cls->enumBaseTy);
              }
            }
            return subObj(*rcls);
          }
          return TBottom;
        case KindOfClass:
        case KindOfRef:
          always_assert_flog(false, "Unexpected DataType");
          break;
        }
        return TBottom;
      }();
      return (mainType == TBottom || !tc.isNullable()) ? mainType
        : opt(mainType);
    }
    case AnnotMetaType::Mixed:
      /*
       * Here we handle "mixed", typevars, and some other ignored
       * typehints (ex. "(function(..): ..)" typehints).
       */
      return TGen;
    case AnnotMetaType::Self:
    case AnnotMetaType::Parent:
    case AnnotMetaType::Callable:
      break;
    case AnnotMetaType::Number:
      return tc.isNullable() ? TOptNum : TNum;
    case AnnotMetaType::ArrayKey:
      // TODO(3774082): Support TInt | TStr type constraint
      break;
  }

  return TBottom;
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
      // This is a class constant that needs an 86cinit to run.  It
      // would be good to eventually be able to analyze these.
      return TInitCell;
    }
    return from_cell(it->second->val.value());
  }
  return TInitCell;
}

const StaticString s_array_map("array_map");
const StaticString s_array_filter("array_filter");

Type Index::lookup_return_type(Context ctx, res::Func rfunc) const {
  return match<Type>(
    rfunc.val,
    [&] (res::Func::FuncName s) {
      // The HHAS systemlib functions are not currently visible to hhbbc, but
      // we know they can't return references.
      if (s.name->isame(s_array_map.get()) ||
          s.name->isame(s_array_filter.get())) {
        return TInitCell;
      }
      return TInitGen;
    },
    [&] (res::Func::MethodName s) { return TInitGen; },
    [&] (borrowed_ptr<FuncInfo> finfo) {
      add_dependency(*m_data, finfo->func, ctx, Dep::ReturnTy);
      return finfo->returnTy;
    },
    [&] (borrowed_ptr<FuncFamily> fam) {
      auto ret = TBottom;
      for (auto& f : fam->possibleFuncs) {
        add_dependency(*m_data, f->func, ctx, Dep::ReturnTy);
        ret = union_of(ret, f->returnTy);
      }
      return ret;
    }
  );
}

Type Index::lookup_return_type(CallContext callCtx, res::Func rfunc) const {
  return match<Type>(
    rfunc.val,
    [&] (res::Func::FuncName) {
      return lookup_return_type(callCtx.caller, rfunc);
    },
    [&] (res::Func::MethodName) {
      return lookup_return_type(callCtx.caller, rfunc);
    },
    [&] (borrowed_ptr<FuncInfo> finfo) {
      add_dependency(*m_data, finfo->func, callCtx.caller, Dep::ReturnTy);
      return context_sensitive_return_type(*this, finfo, callCtx);
    },
    [&] (borrowed_ptr<FuncFamily> fam) {
      return lookup_return_type(callCtx.caller, rfunc);
    }
  );
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
  auto it = m_data->funcInfo.find(f);
  if (it != end(m_data->funcInfo)) return it->second.returnTy;
  return TInitGen;
}

bool Index::lookup_this_available(borrowed_ptr<const php::Func> f) const {
  G g(m_data->funcInfoLock);
  auto it = m_data->funcInfo.find(f);
  return it != end(m_data->funcInfo) ? it->second.thisAvailable : false;
}

PrepKind Index::lookup_param_prep(Context ctx,
                                  res::Func rfunc,
                                  uint32_t paramId) const {
  return match<PrepKind>(
    rfunc.val,
    [&] (res::Func::FuncName s) {
      return prep_kind_from_set(find_range(m_data->funcs, s.name), paramId);
    },
    [&] (res::Func::MethodName s) {
      /*
       * If we think it's supposed to be PrepKind::Ref, we still can't be sure
       * unless we go though some effort to guarantee that it can't be going to
       * an __call function magically (which will never take anything by ref).
       */
      auto const kind = prep_kind_from_set(
        find_range(m_data->methods, s.name),
        paramId
      );
      return kind == PrepKind::Ref ? PrepKind::Unknown : kind;
    },
    [&] (borrowed_ptr<FuncInfo> finfo) {
      return func_param_prep(finfo->func, paramId);
    },
    [&] (borrowed_ptr<FuncFamily> fam) {
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

Type Index::lookup_public_static(Type cls, Type name) const {
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
  if (!vname || (vname && vname->m_type != KindOfStaticString)) {
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

Slot
Index::lookup_iface_vtable_slot(borrowed_ptr<const php::Class> cls) const {
  return folly::get_default(m_data->ifaceSlotMap, cls, kInvalidSlot);
}

//////////////////////////////////////////////////////////////////////

std::vector<Context>
Index::refine_return_type(borrowed_ptr<const php::Func> func, Type t) {
  auto const fdata = create_func_info(*m_data, func);

  always_assert_flog(
    t.subtypeOf(fdata->returnTy),
    "Index return type invariant violated in {} {}{}.\n"
    "   {} is not a subtype of {}\n",
    func->unit->filename->data(),
    func->cls ? folly::to<std::string>(func->cls->name->data(), "::")
              : std::string{},
    func->name->data(),
    show(t),
    show(fdata->returnTy)
  );

  if (!t.strictSubtypeOf(fdata->returnTy)) return {};
  if (fdata->returnRefinments + 1 < options.returnTypeRefineLimit) {
    fdata->returnTy = t;
    ++fdata->returnRefinments;
    return find_deps(*m_data, func, Dep::ReturnTy);
  }
  FTRACE(1, "maxed out return type refinements on {}:{}\n",
    func->unit->filename->data(), func->name->data());
  return {};
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
    if (current[i].strictSubtypeOf(vars[i])) {
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
      cinfo->cls->name->data(),
      name->data(),
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
}

std::unique_ptr<ArrayTypeTable::Builder>& Index::array_table_builder() const {
  return m_data->arrTableBuilder;
}

//////////////////////////////////////////////////////////////////////

res::Func Index::do_resolve(borrowed_ptr<const php::Func> f) const {
  G g(m_data->funcInfoLock);
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

void PublicSPropIndexer::merge(Context ctx, Type tcls, Type name, Type val) {
  auto const vname = tv(name);

  FTRACE(2, "merge_public_static: {} {} {}\n",
    show(tcls), show(name), show(val));

  // Figure out which class this can affect.  If we have a DCls::Sub we have to
  // assume it could affect any subclass, so we repeat this merge for all exact
  // class types deriving from that base.
  auto const maybe_cinfo = [&]() -> folly::Optional<borrowed_ptr<ClassInfo>> {
    if (!is_specialized_cls(tcls)) {
      return nullptr;
    }
    auto const dcls = dcls_of(tcls);
    switch (dcls.type) {
    case DCls::Exact:
      return dcls.cls.val.right();
    case DCls::Sub:
      if (!dcls.cls.val.right()) return nullptr;
      for (auto& sub : dcls.cls.val.right()->subclassList) {
        auto const rcls = res::Class { m_index, sub };
        merge(ctx, clsExact(rcls), name, val);
      }
      return folly::none;
    }
    not_reached();
  }();
  if (!maybe_cinfo) return;

  auto const cinfo = *maybe_cinfo;
  bool const unknownName = !vname ||
    (vname && vname->m_type != KindOfStaticString);

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
      acc->second = union_of(acc->second, val);
    }
    return;
  }

  /*
   * We don't know the name, but we know something about the class.  We need to
   * merge the type for every property in the class hierarchy.
   */
  if (unknownName) {
    for (auto ci = cinfo; ci != nullptr; ci = ci->parent) {
      for (auto& kv : ci->publicStaticProps) {
        merge(ctx, tcls, sval(kv.first), val);
      }
    }
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
  auto const affectedCInfo = [&]() -> borrowed_ptr<ClassInfo> {
    for (auto ci = cinfo; ci != nullptr; ci = ci->parent) {
      if (ci->publicStaticProps.count(vname->m_data.pstr)) {
        return ci;
      }
    }
    return nullptr;
  }();

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
    acc->second = union_of(acc->second, val);
  }
}

//////////////////////////////////////////////////////////////////////

}}
