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

#include "folly/Format.h"
#include "folly/Hash.h"
#include "folly/Memory.h"

#include "hphp/runtime/vm/runtime.h"

#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/class-util.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_construct("__construct");
const StaticString s_86ctor("86ctor");

//////////////////////////////////////////////////////////////////////

struct context_hash {
  size_t operator()(const Context& ctx) const {
    return folly::hash::hash_combine(ctx.unit, ctx.func, ctx.cls);
  }
};

template<class T>
struct resolved_map_hash {
  size_t operator()(std::pair<Context,borrowed_ptr<const T>> pair) const {
    return folly::hash::hash_combine(context_hash()(pair.first), pair.second);
  }
};

/*
 * One-to-many case insensitive map, where the keys are static strings
 * and the values are some kind of borrowed_ptr.
 */
template<class T> using ISStringToMany =
  std::unordered_multimap<
    SString,
    borrowed_ptr<const T>,
    string_data_hash,
    string_data_isame
  >;

/*
 * One-to-one case insensitive map, where the keys are static strings
 * and the values are some kind of borrowed_ptr.
 */
template<class T> using ISStringToOne =
  std::unordered_map<
    SString,
    borrowed_ptr<const T>,
    string_data_hash,
    string_data_isame
  >;

/*
 * Map from a context and a php program structure to a
 * runtime-resolved version of the structure.
 */
template<class T, class U> using ResolvedMap =
  tbb::concurrent_unordered_map<
    std::pair<Context,borrowed_ptr<const T>>,
    U*,
    resolved_map_hash<const T>
  >;

using G = std::lock_guard<std::mutex>;

template<class MultiMap>
boost::iterator_range<typename MultiMap::const_iterator>
find_range(const MultiMap& map, typename MultiMap::key_type key) {
  auto const pair = map.equal_range(key);
  return boost::make_iterator_range(pair.first, pair.second);
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

}

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
};

/*
 * Currently known information about a PHP class.
 *
 * Similar to Funcs, we should never put information in here that
 * isn't known to be 100% accurate.
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
  borrowed_ptr<const ClassInfo> parent = nullptr;

  /*
   * A vector of the declared interfaces class info structures.  This
   * is in declaration order mirroring the php::Class interfaceNames
   * vector, and do not include inherited interfaces.
   */
  std::vector<borrowed_ptr<const ClassInfo>> declInterfaces;

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
  ISStringToOne<php::Func> methods;

  /*
   * The constructor for this class, if we know what it is.
   */
  borrowed_ptr<const php::Func> ctor = nullptr;

  /*
   * A vector of ClassInfo that encodes the inheritance hierarchy.
   */
  std::vector<borrowed_ptr<ClassInfo>> baseList;
};

//////////////////////////////////////////////////////////////////////

namespace res {

Class::Class(borrowed_ptr<const Index> idx, SStringOr<ClassInfo> val)
  : index(idx)
  , val(val)
{}

// Class type operations here are very conservative for now.

bool Class::same(const Class& o) const {
  if (auto s = val.str()) return s == o.val.str();
  return val.other() == o.val.other();
}

bool Class::subtypeOf(const Class& o) const {
  auto s1 = val.str();
  auto s2 = o.val.str();
  if (s1 || s2) return s1 == s2;
  auto c1 = val.other();
  auto c2 = o.val.other();
  if (c1->baseList.size() >= c2->baseList.size()) {
    return c1->baseList[c2->baseList.size() - 1] == c2;
  }
  return false;
}

bool Class::couldBe(const Class& o) const {
  // If either types are not unique return true
  if (val.str() || o.val.str()) return true;

  auto c1 = val.other();
  auto c2 = o.val.other();
  // if one or the other is an interface return true for now.
  // TODO: TASK #3621433
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
  return val.str() ? val.str() : val.other()->cls->name;
}

bool Class::couldBeOverriden() const {
  return val.str() ? true : !(val.other()->cls->attrs & AttrNoOverride);
}

folly::Optional<Class> Class::commonAncestor(const Class& o) const {
  if (val.str() || o.val.str()) return folly::none;
  auto c1 = val.other();
  auto c2 = o.val.other();
  // walk the arrays of base classes until they match. For common ancestors
  // to exist they must be on both sides of the baseList at the same positions
  ClassInfo* ancestor = nullptr;
  auto it1 = c1->baseList.begin();
  auto it2 = c2->baseList.begin();
  while (it1 != c1->baseList.end() && it2 != c2->baseList.end()) {
    if (*it1 != *it2) break;
    ancestor = *it1;
    it1++; it2++;
  }
  if (ancestor == nullptr) {
    return folly::none;
  }
  return res::Class { index, SStringOr<ClassInfo>(ancestor) };
}

std::string show(const Class& c) {
  if (auto s = c.val.str()) return s->data();
  return folly::format(
    "{}*", c.val.other()->cls->name->data()
  ).str();
}

Func::Func(borrowed_ptr<const Index> idx, SStringOr<FuncInfo> val)
  : index(idx)
  , val(val)
{}

bool Func::same(const Func& o) const {
  if (auto s = val.str()) {
    return s == o.val.str();
  }
  return val.other() == o.val.other();
}

SString Func::name() const {
  if (auto s = val.str()) return s;
  return val.other()->func->name;
}

std::string show(const Func& f) {
  std::string ret = f.name()->data();
  if (f.val.other()) ret += "*";
  return ret;
}

}

//////////////////////////////////////////////////////////////////////

struct IndexData {
  IndexData() = default;
  IndexData(const IndexData&) = delete;
  IndexData& operator=(const IndexData&) = delete;
  ~IndexData() = default;

  bool isComprehensive = false;

  ISStringToMany<php::Class>     classes;
  ISStringToMany<php::Func>      methods;
  ISStringToMany<php::Func>      funcs;
  ISStringToMany<php::TypeAlias> typeAliases;

  // Map from each class to all the closures that are allocated in
  // functions of that class.
  std::unordered_map<
    borrowed_ptr<const php::Class>,
    std::unordered_set<borrowed_ptr<php::Class>>
  > classClosureMap;

  std::mutex classInfoLock;
  std::unordered_map<
    borrowed_ptr<const php::Class>,
    std::unique_ptr<ClassInfo>
  > classInfo;

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

// Note: a cyclic inheritance chain will blow this up, but right now
// we'll never get here in that case because hphpc will bail on it
// first.
bool build_cls_info_rec(borrowed_ptr<ClassInfo> rleaf,
                        borrowed_ptr<const ClassInfo> rparent) {
  if (!rparent) return true;

  if (!build_cls_info_rec(rleaf, rparent->parent)) return false;
  for (auto& iface : rparent->declInterfaces) {
    if (!build_cls_info_rec(rleaf, iface)) return false;
  }

  auto const isIface = rparent->cls->attrs & AttrInterface;

  /*
   * Make a table of all the constants on this class.
   *
   * Duplicate class constants override parent class constants, but
   * for interfaces it's an error to have a duplicate constant.
   */
  for (auto& c : rparent->cls->constants) {
    auto& cptr = rleaf->clsConstants[c.name];
    /*
     * Note: hphpc doesn't actually check for this case, but since
     * we're doing propagation without autoload it seems pretty
     * broken.
     *
     * XXX; if the same interface comes from multiple parts of the
     * inheritance chain this is actually fine, but we're rejecting
     * the case ...
     */
    if (isIface && cptr) return false;

    cptr = &c;
  }

  /*
   * Make a table of the methods on this class, excluding interface
   * methods (since they aren't callable unless they are overridden).
   *
   * Duplicate method names override parent methods, unless the parent
   * method is final, in which case it's a resolution error.
   */
  if (!isIface) {
    for (auto& m : rparent->cls->methods) {
      if (m->attrs & AttrAbstract) continue;
      auto& mptr = rleaf->methods[m->name];
      if (mptr) {
        if (mptr->attrs & AttrFinal) return false;
        if (!(mptr->attrs & AttrPrivate)) {
          assert(!(mptr->attrs & AttrNoOverride));
        }
      }
      mptr = borrow(m);
    }
  }

  return true;
}

borrowed_ptr<const php::Func> find_constructor(borrowed_ptr<ClassInfo> cinfo) {
  if (cinfo->cls->attrs & (AttrInterface|AttrTrait)) {
    return nullptr;
  }

  auto cit = cinfo->methods.find(s_construct.get());
  if (cit != end(cinfo->methods) && cit->second->cls == cinfo->cls) {
    return cit->second;
  }

  // Try old style class name constructors.  We need to check
  // AttrTrait on the function in case it came from a pre-flattened
  // trait (that doesn't count as a constructor).
  assert(!(cinfo->cls->attrs & AttrTrait) &&
         "We shouldn't be resolving traits right now");
  cit = cinfo->methods.find(cinfo->cls->name);
  if (cit != end(cinfo->methods)) {
    if (cit->second->cls == cinfo->cls) return cit->second;
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
  return cit->second;
}

bool build_cls_info(borrowed_ptr<ClassInfo> cinfo) {
  if (!build_cls_info_rec(cinfo, cinfo)) return false;
  cinfo->ctor = find_constructor(cinfo);
  return true;
}

//////////////////////////////////////////////////////////////////////

void add_unit_to_index(IndexData& index, const php::Unit& unit) {
  for (auto& c : unit.classes) {
    index.classes.insert({c->name, borrow(c)});

    for (auto& m : c->methods) {
      index.methods.insert({m->name, borrow(m)});
    }

    if (c->closureContextCls) {
      index.classClosureMap[c->closureContextCls].insert(borrow(c));
    }
  }

  for (auto& f : unit.funcs) {
    if (options.InterceptableFunctions.count(std::string{f->name->data()})) {
      f->attrs = f->attrs | AttrDynamicInvoke;
    }
    index.funcs.insert({f->name, borrow(f)});
  }

  for (auto& ta : unit.typeAliases) {
    index.typeAliases.insert({ta->name, borrow(ta)});
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

Index::Index(borrowed_ptr<php::Program> program)
  : m_data(folly::make_unique<IndexData>())
{
  trace_time tracer("create index");
  for (auto& u : program->units) add_unit_to_index(*m_data, *u);
  m_data->isComprehensive = true;
}

Index::Index(borrowed_ptr<php::Unit> unit)
  : m_data(folly::make_unique<IndexData>())
{
  add_unit_to_index(*m_data, *unit);
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

/*
 * For now, both resolve_class and resolve_func only look up entities
 * that are globally unique.
 *
 * We could plausibly do something based on the source context, where
 * we can return things from the same unit.
 */

folly::Optional<res::Class> Index::resolve_class(Context ctx,
                                                 SString clsName) const {
  clsName = normalizeNS(clsName);
  auto name_only = [&] () -> folly::Optional<res::Class> {
    // We know it has to name a class only if there's no type alias
    // with this name.
    //
    // TODO(#3519401): when we start unfolding type aliases, we could
    // look at whether it is an alias for a specific class here.
    if (!m_data->typeAliases.count(clsName)) {
      return res::Class { this, SStringOr<ClassInfo>(clsName) };
    }
    return folly::none;
  };

  if (!m_data->isComprehensive) return name_only();

  auto const cls = [&] () -> borrowed_ptr<const php::Class> {
    auto const classes = find_range(m_data->classes, clsName);
    for (auto it = begin(classes); it != end(classes); ++it) {
      auto const cls = it->second;

      if (cls->attrs & AttrUnique) {
        if (debug && boost::next(it) != end(classes)) {
          std::fprintf(stderr, "non unique \"unique\" class: %s\n",
            cls->name->data());
          for (; it != end(classes); ++it) {
            std::fprintf(stderr, "   and %s\n", it->second->name->data());
          }
          assert(0);
        }
        return cls;
      }

      /*
       * TODO_3: this isn't quite working in single_unit, because when
       * not in whole program mode we're throwing away AlwaysHoistable
       * and turning it into MaybeHoistable ...
       */
      if (false /* disabled for now */ && cls->unit == ctx.unit &&
          cls->hoistability == PreClass::AlwaysHoistable) {
        if (debug) {
          // AlwaysHoistable should imply a unique name within this
          // unit:
          while (++it != end(classes)) {
            always_assert(it->second->unit != ctx.unit);
          }
        }

        // We used the unit information from the context, but not the
        // current function, so it gets nulled out.
        return cls;
      }
    }
    return nullptr;
  }();

  if (!cls) return name_only();

  {
    G g(m_data->classInfoLock);
    auto it = m_data->classInfo.find(cls);
    if (it != end(m_data->classInfo)) {
      return res::Class { this, SStringOr<ClassInfo>(borrow(it->second)) };
    }
  }

  auto cinfo = folly::make_unique<ClassInfo>();
  cinfo->cls = cls;

  if (cls->parentName) {
    auto const parent = resolve_class(ctx, cls->parentName);
    if (!parent.hasValue() || !parent->val.other()) {
      // Deriving from some sort of malformed base class.
      return name_only();
    }
    cinfo->parent = parent->val.other();
    cinfo->baseList = cinfo->parent->baseList;
  } else {
    cinfo->parent = nullptr;
  }
  cinfo->baseList.push_back(borrow(cinfo));

  for (auto& ifaceName : cls->interfaceNames) {
    auto const iface = resolve_class(ctx, ifaceName);
    if (!iface || !iface->val.other() ||
        !(iface->val.other()->cls->attrs & AttrInterface)) {
      // Deriving from a malformed interface.  (E.g., "implements Foo"
      // when Foo is actually a class.)
      return name_only();
    }
    cinfo->declInterfaces.push_back(iface->val.other());
  }

  if (!build_cls_info(borrow(cinfo))) {
    // Resolution failure.  (E.g. the class is not correctly formed.)
    return name_only();
  }

  {
    G g(m_data->classInfoLock);
    auto& mapinfo = m_data->classInfo[cls];
    if (!mapinfo) mapinfo = std::move(cinfo);
    return res::Class { this, SStringOr<ClassInfo>(borrow(mapinfo)) };
  }
}

res::Class Index::builtin_class(Context ctx, SString name) const {
  auto const rcls = resolve_class(ctx, name);
  if (!rcls) {
    std::fprintf(stderr, "failed to resolve a builtin class: %s\n",
      name->data());
    std::abort();
  }
  assert(rcls->val.other() && (rcls->val.other()->cls->attrs & AttrBuiltin));
  return *rcls;
}

folly::Optional<res::Func> Index::resolve_method(Context ctx,
                                                 Type clsType,
                                                 SString name) const {
  if (!clsType.strictSubtypeOf(TCls)) return folly::none;

  auto const dcls  = dcls_of(clsType);
  auto const cinfo = dcls.cls.val.other();

  if (!cinfo) return folly::none;

  /*
   * Note: this isn't checking accessibility, which might not be
   * great.  We could infer things that are wrong ... but we will just
   * fatal at run time.  HPHPc appears to do the same.
   */
  auto const it = cinfo->methods.find(name);
  if (it != end(cinfo->methods)) {
    if (dcls.type == DCls::Exact) {
      return do_resolve(it->second);
    }
    if (it->second->attrs & AttrNoOverride) {
      return do_resolve(it->second);
    }
  }

  return folly::none;
}

folly::Optional<res::Func> Index::resolve_ctor(Context ctx,
                                               res::Class rcls) const {
  auto const cinfo = rcls.val.other();
  if (!cinfo || !cinfo->ctor) return folly::none;
  return do_resolve(cinfo->ctor);
}

res::Func Index::resolve_func(Context ctx, SString name) const {
  name = normalizeNS(name);
  auto name_only = [&] {
    return res::Func { this, SStringOr<FuncInfo>(name) };
  };

  if (!m_data->isComprehensive) return name_only();

  auto const funcs = find_range(m_data->funcs, name);
  if (begin(funcs) == end(funcs)) return name_only();
  if (boost::next(begin(funcs)) != end(funcs)) return name_only();
  auto const func = begin(funcs)->second;
  if (!(func->attrs & AttrUnique)) return name_only();
  if (func->attrs & AttrDynamicInvoke) return name_only();

  return do_resolve(func);
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
        case KindOfString:        return TStr;
        case KindOfStaticString:  return TStr;
        case KindOfArray:         return TArr;
        case KindOfInt64:         return TInt;
        case KindOfBoolean:       return TBool;
        case KindOfDouble:        return TDbl;
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
        case KindOfResource: // Note, some day we may have resource hints.
          break;
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
  if (rcls.val.str()) return TInitUnc;
  auto const cinfo = rcls.val.other();

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
  auto const finfo = rfunc.val.other();
  if (!finfo) return TInitGen;

  add_dependency(*m_data, finfo->func, ctx, Dep::ReturnTy);
  return finfo->returnTy;
}

PrepKind Index::lookup_param_prep(Context ctx,
                                  res::Func rfunc,
                                  uint32_t paramId) const {
  auto finfo = rfunc.val.other();
  if (!finfo) return PrepKind::Unknown;
  // Parameter preparation modes never change during analysis, so
  // there's no need to register a dependency.

  // Note: there are some "variadic by ref" builtins, but we can't
  // currently have a resolved func if it is a builtin.
  if (paramId >= finfo->func->params.size()) return PrepKind::Val;

  return finfo->func->params[paramId].byRef ? PrepKind::Ref : PrepKind::Val;
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
  assert(t.subtypeOf(fdata->returnTy));
  if (t.strictSubtypeOf(fdata->returnTy)) {
    fdata->returnTy = t;
    return find_deps(*m_data, func, Dep::ReturnTy);
  }
  return {};
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

//////////////////////////////////////////////////////////////////////

res::Func Index::do_resolve(borrowed_ptr<const php::Func> f) const {
  G g(m_data->funcInfoLock);
  auto const finfo = create_func_info(*m_data, f);
  return res::Func { this, SStringOr<FuncInfo>(finfo) };
};

//////////////////////////////////////////////////////////////////////

}}
