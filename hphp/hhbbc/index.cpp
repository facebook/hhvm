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

#include <unordered_map>
#include <mutex>
#include <map>
#include <cstdio>
#include <cstdlib>

#include <boost/next_prior.hpp>
#include <boost/range/iterator_range.hpp>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_unordered_map.h>
#include <algorithm>
#include <memory>
#include <unordered_set>
#include <vector>

#include "folly/String.h"
#include "folly/Format.h"
#include "folly/Hash.h"
#include "folly/Memory.h"
#include "folly/Optional.h"
#include "folly/Lazy.h"

#include "hphp/util/assertions.h"
#include "hphp/util/match.h"

#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit-util.h"

#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/func-util.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc_index);

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_construct("__construct");
const StaticString s_call("__call");
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
boost::iterator_range<typename MultiMap::const_iterator>
find_range(const MultiMap& map, typename MultiMap::key_type key) {
  auto const pair = map.equal_range(key);
  return boost::make_iterator_range(pair.first, pair.second);
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
   * A set of the interface class info structures.  This
   * includes both declared and inherited interfaces.
   */
  std::set<borrowed_ptr<ClassInfo>> allInterfaces;

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
   * If this class is an interface, it maps from interface method
   * names to associated FuncFamily objects that group the set of
   * methods implemented.
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
   * Flags about the existence of various magic methods, or whether
   * any derived classes may have those methods.  The non-derived
   * flags imply the derived flags, even if the class is final, so you
   * don't need to check both in those situations.
   */
  bool hasMagicCall              = false;
  bool hasMagicCallStatic        = false;
  bool derivedHasMagicCall       = false;
  bool derivedHasMagicCallStatic = false;
};

//////////////////////////////////////////////////////////////////////

namespace res {

Class::Class(Either<SString,borrowed_ptr<ClassInfo>> val)
  : val(val)
{}

Class::Class(Either<SString,borrowed_ptr<ClassInfo>> val,
             std::set<borrowed_ptr<ClassInfo>>&& ifc)
  : val(val)
  , ifaces(std::make_shared<std::set<borrowed_ptr<ClassInfo>>>(std::move(ifc)))
{}

bool Class::same(const Class& o) const {
  return val == o.val && ((ifaces && o.ifaces)
                           ? *ifaces == *o.ifaces
                           : ifaces == o.ifaces);
}

bool Class::subtypeOf(const Class& o) const {
  auto s1 = val.left();
  auto s2 = o.val.left();
  if (s1 || s2) return s1 == s2;
  auto c1 = val.right();
  auto c2 = o.val.right();

  auto hasInterface = [&](borrowed_ptr<ClassInfo> iface) {
    if (c1->allInterfaces.count(iface) != 0) {
      return true;
    }
    return ifaces ? ifaces->count(iface) != 0 : false;
  };

  if (c2->cls->attrs & AttrInterface && !hasInterface(c2)) {
    return false;
  }
  if (!(c2->cls->attrs & AttrInterface)) {
    if (c1->baseList.size() < c2->baseList.size()
        || c1->baseList[c2->baseList.size() - 1] != c2) {
      return false;
    }
  }
  if (o.ifaces) {
    for (auto iface : *o.ifaces) {
      if (!hasInterface(iface)) return false;
    }
  }
  return true;
}

bool Class::couldBe(const Class& o, ClsTag tag, ClsTag otherTag) const {
  // If either types are not unique return true
  if (val.left() || o.val.left()) return true;

  auto c1 = val.right();
  auto c2 = o.val.right();

  if (subtypeOf(o) || o.subtypeOf(*this)) {
    return true;
  }
  if (c1->cls->attrs & AttrInterface) {
    return (otherTag == ClsTag::Sub) && !(c2->cls->attrs & AttrNoOverride);
  }
  if (c2->cls->attrs & AttrInterface) {
    return (tag == ClsTag::Sub) && !(c1->cls->attrs & AttrNoOverride);
  }
  return false;
}

SString Class::name() const {
  return val.match(
    [] (SString s) { return s; },
    [] (borrowed_ptr<ClassInfo> ci) { return ci->cls->name; }
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

bool Class::couldBeInterface() const {
  return val.match(
    [] (SString) { return true; },
    [] (borrowed_ptr<ClassInfo> cinfo) {
      return (cinfo->cls->attrs & AttrInterface);
    }
  );
}

static void intersect(std::set<borrowed_ptr<ClassInfo>>& res,
                      const std::set<borrowed_ptr<ClassInfo>>& a,
                      const std::set<borrowed_ptr<ClassInfo>>& b,
                      std::set<borrowed_ptr<ClassInfo>>& skip) {
  std::set<borrowed_ptr<ClassInfo>> common;
  std::set<borrowed_ptr<ClassInfo>> filtered;

  std::set_intersection(a.begin(), a.end(), b.begin(), b.end(),
                        std::inserter(common, common.begin()));
  std::set_difference(common.begin(), common.end(), skip.begin(), skip.end(),
                      std::inserter(res, res.begin()));
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
  std::set<borrowed_ptr<ClassInfo>> ifc;

  /*
   * Find the intersection between the interfaces of this class and 'o'.
   * Since interfaces in the Class are spread over cinfo->allInterfaces
   * and the extra interfaces 'ifaces', we need to intersect over all
   * 4 pairs: (allInterfaces, allInterfaces), (allInterfaces, ifaces),
   * (ifaces, allInterfaces) and (ifaces, ifaces).
   *
   * Also, if ancestor is not nullptr, we need to skip the interfaces
   * of ancestor.
   */
  if (ancestor) {
    intersect(ifc, c1->allInterfaces, c2->allInterfaces,
              ancestor->allInterfaces);
  } else {
    std::set<borrowed_ptr<ClassInfo>> empty;
    intersect(ifc, c1->allInterfaces, c2->allInterfaces, empty);
  }
  if (ifaces) {
    intersect(ifc, *ifaces, c2->allInterfaces, ifc);
    if (o.ifaces) intersect(ifc, *ifaces, *o.ifaces, ifc);
  }
  if (o.ifaces) intersect(ifc, c1->allInterfaces, *o.ifaces, ifc);

  if (ancestor == nullptr) {
    if (ifc.size() != 0) {
      auto it = ifc.begin();
      ancestor = *it;
      ifc.erase(it);
    }
    if (!ancestor) return folly::none;
  }

  if (ifc.size() != 0) return res::Class { ancestor, std::move(ifc) };
  return res::Class { ancestor };
}

std::string show(const Class& c) {
  return c.val.match(
    [] (SString s) -> std::string {
      return s->data();
    },
    [&] (borrowed_ptr<ClassInfo> cinfo) {
      std::string ret;
      ret = cinfo->cls->name->data();
      if (c.ifaces) {
        for (auto i : *c.ifaces) {
          ret += "i<'";
          ret += i->cls->name->data();
        }
      }
      return ret;
    }
  );
}

Func::Func(borrowed_ptr<const Index> idx, Rep val)
  : index(idx)
  , val(val)
  , rcls(folly::none)
{}

Func::Func(borrowed_ptr<const Index> idx, Rep val,
           res::Class rcls)
  : index(idx)
  , val(val)
  , rcls(rcls)
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
    [&] (SString s) { return s; },
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

std::string show(const Func& f) {
  std::string ret = f.name()->data();
  match<void>(
    f.val,
    [&] (SString) {},
    [&] (borrowed_ptr<FuncInfo> fi) { ret += "*"; },
    [&] (borrowed_ptr<FuncFamily> fa) { ret += "+"; }
  );
  return ret;
}

}

//////////////////////////////////////////////////////////////////////

struct IndexData {
  IndexData() = default;
  IndexData(const IndexData&) = delete;
  IndexData& operator=(const IndexData&) = delete;
  ~IndexData() = default;

  std::unique_ptr<ArrayTypeTable::Builder> arrTableBuilder;

  ISStringToMany<const php::Class>     classes;
  ISStringToMany<const php::Func>      methods;
  ISStringToMany<const php::Func>      funcs;
  ISStringToMany<const php::TypeAlias> typeAliases;

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
    if (f->nativeInfo->returnType != KindOfInvalid) {
      // TODO(#3568043): always add TInitNull, because HNI doesn't
      // know about nullability.
      auto const t = from_DataType(f->nativeInfo->returnType);
      ret.returnTy = union_of(t, TInitNull);
    }
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
  for (auto& iface : rparent->allInterfaces) {
    if (!build_cls_info_rec(rleaf, iface)) return false;
  }

  auto const parentIface = rparent->cls->attrs & AttrInterface;
  auto const leafIface   = rleaf->cls->attrs & AttrInterface;

  /*
   * Make a table of all the constants on this class.
   *
   * Duplicate class constants override parent class constants, but
   * for interfaces it's an error to have a duplicate constant, unless
   * it just happens from implementing the same interface more than
   * once.
   *
   * Note: hphpc doesn't actually check for this case, but since with
   * HardConstProp we're potentially doing propagation of these
   * constants without autoload, it seems like it could be potentially
   * surprising to propagate incorrect constants from mis-declared
   * classes that would fatal if they were ever defined.
   */
  for (auto& c : rparent->cls->constants) {
    auto& cptr = rleaf->clsConstants[c.name];
    if (parentIface && cptr) {
      if (cptr->cls != rparent->cls) return false;
    }
    cptr = &c;
  }

  /*
   * Make a table of the methods on this class, excluding interface
   * methods. For interfaces, add the method only to the interface or
   * extending interfaces.
   *
   * Duplicate method names override parent methods, unless the parent
   * method is final and the class is not a __MockClass, in which case
   * this class definitely would fatal if ever defined.
   *
   * Note: we're leaving non-overridden privates in their subclass
   * method table, here.  This isn't currently "wrong", because
   * calling it would be a fatal.  TODO: re-evaluate this idea.
   */
  if (!parentIface || leafIface) {
    for (auto& m : rparent->cls->methods) {
      auto& ent = rleaf->methods[m->name];
      if (ent.func) {
        if (ent.func->attrs & AttrFinal) {
          if (!is_mock_class(rleaf->cls)) return false;
        }
        if (ent.func->attrs & AttrPrivate) {
          ent.hasPrivateAncestor =
            ent.hasPrivateAncestor || ent.func->cls != rleaf->cls;
        } else {
          assert(!(ent.func->attrs & AttrNoOverride) || parentIface);
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
  return true;
}

//////////////////////////////////////////////////////////////////////

using InterceptableMethodMap = std::map<
  std::string,
  std::set<std::string,stdltistr>,
  stdltistr
>;

InterceptableMethodMap make_interceptable_method_map() {
  auto ret = InterceptableMethodMap{};
  for (auto& str : options.InterceptableFunctions) {
    std::vector<std::string> parts;
    folly::split("::", str, parts);
    if (parts.size() != 2) continue;
    ret[parts[0]].insert(parts[1]);
  }
  return ret;
}

void add_unit_to_index(IndexData& index,
                       const InterceptableMethodMap& imethodMap,
                       const php::Unit& unit) {
  for (auto& c : unit.classes) {
    index.classes.insert({c->name, borrow(c)});

    auto const imethIt = imethodMap.find(c->name->data());

    for (auto& m : c->methods) {
      index.methods.insert({m->name, borrow(m)});

      if (imethIt != end(imethodMap) &&
          imethIt->second.count(m->name->data())) {
        m->attrs = m->attrs | AttrInterceptable;
      }
    }

    if (c->closureContextCls) {
      index.classClosureMap[c->closureContextCls].insert(borrow(c));
    }
  }

  for (auto& f : unit.funcs) {
    if (options.InterceptableFunctions.count(f->name->data())) {
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
    cinfo->parent        = env.lookup(cls->parentName);
    /*
     * Interfaces don't store a baseList, because that implies
     * a single inheritance chain, whereas interfaces can extent multiple
     * interfaces. Instead, all of an interface's parents (recursively)
     * are stored in its allInterfaces list.
     */
    if (cinfo->cls->attrs & AttrInterface) {
      cinfo->allInterfaces.insert(cinfo->parent);
    } else {
      cinfo->baseList      = cinfo->parent->baseList;
    }
    for (auto& i : cinfo->parent->allInterfaces) {
      if (cinfo->allInterfaces.count(i) == 0) {
        cinfo->allInterfaces.insert(i);
      }
    }
  }
  cinfo->baseList.push_back(borrow(cinfo));

  for (auto& iname : cls->interfaceNames) {
    auto const iface = env.lookup(iname);
    if (cinfo->allInterfaces.count(iface) == 0) {
      cinfo->allInterfaces.insert(iface);
    }
    for (auto& i : iface->allInterfaces) {
      if (cinfo->allInterfaces.count(i) == 0) {
        cinfo->allInterfaces.insert(i);
      }
    }
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
    for (auto& iface : cinfo->allInterfaces) {
      iface->subclassList.push_back(borrow(cinfo));
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
    // For interfaces, skip the method defined in the interface itself
    // or any of its extending interfaces. These apparently have an InitNull
    // return type
    if (cleaf->cls->attrs & AttrInterface) {
      continue;
    }
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
   * Interface functions are not part of the family. So, for interfaces
   * that have no implementing classes, the FuncFamily might be empty.
   */

  if (!family->possibleFuncs.empty()) {
    cinfo->methodFamilies.emplace(name, family);
  } else {
    // discard the empty funcFamily
    index.funcFamilies.pop_back();
  }
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

void mark_magic_on_parents(ClassInfo& cinfo, bool call, bool callStatic) {
  if (call) cinfo.derivedHasMagicCall = true;
  if (callStatic) cinfo.derivedHasMagicCallStatic = true;
  if (cinfo.parent) {
    mark_magic_on_parents(*cinfo.parent, call, callStatic);
  }
}

void find_magic_methods(IndexData& index) {
  for (auto& cinfo : index.allClassInfos) {
    auto& methods         = cinfo->methods;
    auto const call       = methods.find(s_call.get()) != end(methods);
    auto const callStatic = methods.find(s_callStatic.get()) != end(methods);

    cinfo->hasMagicCall       = call;
    cinfo->hasMagicCallStatic = callStatic;
    if (call || callStatic) {
      mark_magic_on_parents(*cinfo, call, callStatic);
    }
  }
}

void mark_no_override(IndexData& index) {
  for (auto& cinfo : index.allClassInfos) {
    if (cinfo->subclassList.size() == 1 &&
        !(cinfo->cls->attrs & AttrInterface)) {
      assert(cinfo->subclassList.front() == borrow(cinfo));
      if (!(cinfo->cls->attrs & AttrNoOverride)) {
        FTRACE(1, "Adding AttrNoOverride to {}\n", cinfo->cls->name->data());
      }
      const_cast<borrowed_ptr<php::Class>>(cinfo->cls)->attrs =
        cinfo->cls->attrs | AttrNoOverride;
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

  // Magic method flags should be consistent with the method table.
  always_assert(
    cinfo->hasMagicCall ==
      (cinfo->methods.find(s_call.get()) != end(cinfo->methods))
  );
  always_assert(
    cinfo->hasMagicCallStatic ==
      (cinfo->methods.find(s_callStatic.get()) != end(cinfo->methods))
  );

  // Non-'derived' flags about magic methods imply the derived ones.
  always_assert(!cinfo->hasMagicCallStatic ||
                cinfo->derivedHasMagicCallStatic);
  always_assert(!cinfo->hasMagicCall || cinfo->derivedHasMagicCall);
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
      always_assert(boost::next(begin(range)) == end(range));
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

}

//////////////////////////////////////////////////////////////////////

Index::Index(borrowed_ptr<php::Program> program)
  : m_data(folly::make_unique<IndexData>())
{
  trace_time tracer("create index");

  m_data->arrTableBuilder.reset(new ArrayTypeTable::Builder());

  auto const imethodMap = make_interceptable_method_map();
  for (auto& u : program->units) {
    add_unit_to_index(*m_data, imethodMap, *u);
  }

  NamingEnv env;
  for (auto& u : program->units) {
    Trace::Bump bumper{Trace::hhbbc, kSystemLibBump, is_systemlib_part(*u)};
    for (auto& c : u->classes) {
      preresolve(*m_data, env, c->name);
    }
  }

  compute_subclass_list(*m_data);
  define_func_families(*m_data);
  find_magic_methods(*m_data);
  check_invariants(*m_data);

  mark_no_override(*m_data);
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

  auto name_only = [&] () -> folly::Optional<res::Class> {
    // We know it has to name a class only if there's no type alias
    // with this name.
    //
    // TODO(#3519401): when we start unfolding type aliases, we could
    // look at whether it is an alias for a specific class here.
    // (Note this might need to split into a different API: type
    // aliases aren't allowed everywhere we're doing resolve_class
    // calls.)
    if (!m_data->typeAliases.count(clsName)) {
      return res::Class { clsName };
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
    if ((cinfo->cls->attrs & AttrUnique) || boost::next(it) == end(classes)) {
      if (debug && boost::next(it) != end(classes)) {
        // TODO(#3363851): There's some race here happening with
        // closures in traits intermittently.
        std::fprintf(stderr, "non unique \"unique\" class: %s\n",
          cinfo->cls->name->data());
        for (; it != end(classes); ++it) {
          std::fprintf(stderr, "   and %s\n", cinfo->cls->name->data());
        }
        assert(0);
      }
      return res::Class { cinfo };
    }
    break;
  }

  return name_only();
}

std::pair<res::Class,std::vector<borrowed_ptr<php::Class>>>
Index::resolve_closure_class(Context ctx, SString name) const {
  auto const rcls = resolve_class(ctx, name);

#if 0  // Disabled because of TODO(#3363851)

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

#else

  if (rcls.hasValue() && rcls->val.right()) {
    // This is the expected case.
    return {
      *rcls, { const_cast<borrowed_ptr<php::Class>>(rcls->val.right()->cls) }
    };
  }

  std::fprintf(stderr,
    "A Closure class (%s) failed to resolve normally---this is "
    "a bug but we're tolerating it for now.\n", name->data());

  auto ret = std::make_pair(*rcls, std::vector<borrowed_ptr<php::Class>>{});
  for (auto& c : find_range(m_data->classInfo, name)) {
    ret.second.push_back(const_cast<borrowed_ptr<php::Class>>(c.second->cls));
  }
  return ret;

#endif
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

folly::Optional<res::Func> Index::resolve_iface_method(Context ctx,
                                                       res::Class cls,
                                                       SString name) const {
  if (!options.FuncFamilies) return folly::none;

  auto cinfo = cls.val.right();

  if (cinfo->cls->attrs & AttrInterface) {
    auto const famIt = cinfo->methodFamilies.find(name);
    if (famIt != end(cinfo->methodFamilies)) {
      if (famIt->second->containsInterceptables) {
        return folly::none;
      }
      return res::Func { this, famIt->second, res::Class(cinfo) };
    }
  }

  if (cls.ifaces) {
    for (auto iface : *cls.ifaces) {
      auto const famIt = cinfo->methodFamilies.find(name);
      if (famIt == end(iface->methodFamilies)) continue;
      if (famIt->second->containsInterceptables) {
        return folly::none;
      }
      return res::Func { this, famIt->second, res::Class(iface) };
    }
  }
  return folly::none;
}

folly::Optional<res::Func> Index::resolve_method(Context ctx,
                                                 Type clsType,
                                                 SString name) const {
  if (!clsType.strictSubtypeOf(TCls)) {
    return folly::none;
  }
  auto const dcls  = dcls_of(clsType);

  auto const cinfo = dcls.cls.val.right();
  if (!cinfo) return folly::none;

  if (cinfo->cls->attrs & AttrInterface) {
    return resolve_iface_method(ctx, dcls.cls, name);
  }

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
  if (methIt == end(cinfo->methods)) {
    return resolve_iface_method(ctx, dcls.cls, name);
  }
  if (methIt->second.func->attrs & AttrInterceptable) return folly::none;
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
        return folly::none;
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
   * So, in the presense of magic methods, we must handle the fact
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
  case ClsTag::Exact:
    if (cinfo->hasMagicCall || cinfo->hasMagicCallStatic) {
      if (couldBeInaccessible()) return folly::none;
    }
    return do_resolve(ftarget);
  case ClsTag::Sub:
    if (cinfo->derivedHasMagicCall || cinfo->derivedHasMagicCallStatic) {
      if (couldBeInaccessible()) return folly::none;
    }
    if (ftarget->attrs & AttrNoOverride) {
      return do_resolve(ftarget);
    }
    if (!options.FuncFamilies) return folly::none;

    {
      auto const famIt = cinfo->methodFamilies.find(name);
      if (famIt == end(cinfo->methodFamilies)) {
        return resolve_iface_method(ctx, dcls.cls, name);
      }
      if (famIt->second->containsInterceptables) {
        return folly::none;
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
  auto name_only = [&] { return res::Func { this, name }; };

  if (begin(funcs) == end(funcs))              return name_only();
  if (boost::next(begin(funcs)) != end(funcs)) return name_only();
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
  if (!tc.hasConstraint()) return TCell;

  /*
   * Type variable constraints are not used at runtime to enforce
   * anything.
   */
  if (tc.isTypeVar()) return TCell;

  /*
   * Soft hints (@Foo) are not checked.
   */
  if (tc.isSoft()) return TCell;

  switch (tc.metaType()) {
  case TypeConstraint::MetaType::Precise:
    {
      auto const mainType = [&]() -> const Type {
        switch (tc.underlyingDataType()) {
        case KindOfString:       return TStr;
        case KindOfStaticString: return TStr;
        case KindOfArray:        return TArr;
        case KindOfInt64:        return TInt;
        case KindOfBoolean:      return TBool;
        case KindOfDouble:       return TDbl;
        case KindOfResource:     return TRes;
        case KindOfObject:
          /*
           * Type constraints only imply an object of a particular
           * type for unique classes.
           */
          if (auto const rcls = resolve_class(ctx, tc.typeName())) {
            return interface_supports_non_objects(rcls->name())
              ? TInitCell // none of these interfaces support Uninits
              : subObj(*rcls);
          }
          return TInitCell;
        default:
          break;
        }
        return TInitCell;
      }();
      return mainType == TInitCell || !tc.isNullable() ? mainType
        : opt(mainType);
    }
  case TypeConstraint::MetaType::Self:
  case TypeConstraint::MetaType::Parent:
  case TypeConstraint::MetaType::Callable:
    break;
  case TypeConstraint::MetaType::Number:
    return TNum;
  }

  return TCell;
}

Type Index::lookup_class_constant(Context ctx,
                                  res::Class rcls,
                                  SString cnsName) const {
  if (rcls.val.left()) return TInitUnc;
  auto const cinfo = rcls.val.right();

  auto const it = cinfo->clsConstants.find(cnsName);
  if (it != end(cinfo->clsConstants)) {
    if (it->second->val.m_type == KindOfUninit) {
      // This is a class constant that needs an 86cinit to run.  It
      // would be good to eventually be able to analyze these.
      return TInitUnc;
    }
    return from_cell(it->second->val);
  }
  return TInitUnc;
}

Type Index::lookup_return_type(Context ctx, res::Func rfunc) const {
  return match<Type>(
    rfunc.val,
    [&] (SString s) {
      return TInitGen;
    },
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

std::vector<Type>
Index::lookup_closure_use_vars(borrowed_ptr<const php::Func> func) const {
  assert(func->isClosureBody);

  auto const numUseVars = closure_num_use_vars(func);
  if (!numUseVars) return {};
  auto const it = m_data->closureUseVars.find(func->cls);
  if (it == end(m_data->closureUseVars)) {
    return std::vector<Type>(numUseVars, TInitGen);
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

// Parameter preparation modes never change during analysis, so
// there's no need to register a dependency.
PrepKind Index::lookup_param_prep(Context ctx,
                                  res::Func rfunc,
                                  uint32_t paramId) const {

  auto finfo_prep = [&] (borrowed_ptr<const FuncInfo> finfo) {
    if (paramId >= finfo->func->params.size()) {
      if (finfo->func->attrs & AttrVariadicByRef) {
        return PrepKind::Ref;
      }
      return PrepKind::Val;
    }
    return finfo->func->params[paramId].byRef ? PrepKind::Ref
                                              : PrepKind::Val;
  };

  return match<PrepKind>(
    rfunc.val,
    [&] (SString s) {
      return PrepKind::Unknown;
    },
    [&] (borrowed_ptr<FuncInfo> finfo) {
      return finfo_prep(finfo);
    },
    [&] (borrowed_ptr<FuncFamily> fam) {
      folly::Optional<PrepKind> prep;
      for (auto& f : fam->possibleFuncs) {
        switch (finfo_prep(f)) {
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
    assert(kv.second.subtypeOf(target));
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
    auto const rCls = res::Class { kvCls.second };
    for (auto& kvPar : parentClasses) {
      auto const rPar = res::Class { kvPar.second };
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
    auto const rCls = res::Class { kvCls.second };
    for (auto& kvPar : parentClasses) {
      auto const rPar = res::Class { kvPar.second };
      if (rCls.couldBe(rPar, ClsTag::Sub, ClsTag::Sub)) return true;
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////

}}
