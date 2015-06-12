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
#ifndef incl_HHBBC_INDEX_H_
#define incl_HHBBC_INDEX_H_

#include <memory>
#include <mutex>
#include <tuple>
#include <vector>
#include <map>

#include <boost/variant.hpp>
#include <tbb/concurrent_hash_map.h>

#include <folly/Optional.h>
#include <folly/Hash.h>

#include "hphp/util/either.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/vm/type-constraint.h"

#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/misc.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

struct Type;
struct Index;
struct PublicSPropIndexer;

namespace php {
struct Class;
struct Func;
struct Unit;
struct Program;
}

//////////////////////////////////////////////////////////////////////

/*
 * This module contains functions for building and querying an Index
 * of data relating to "resolved" versions of the names in of a
 * php::Program.  It also records dependencies so it is possible to
 * tell which parts of the program may be interested in new inferred
 * information about other parts of the program.
 *
 * The main entry point here is the Index class.  The Index is built
 * after parse time, and then analysis can query it for information.
 */

//////////////////////////////////////////////////////////////////////

/*
 * A Context is a (unit, func, class) triple, where cls and func
 * fields may be null in some situations.  Most queries to the Index
 * need a "context", to allow recording dependencies.
 */
struct Context { borrowed_ptr<php::Unit> unit;
                 borrowed_ptr<php::Func> func;
                 borrowed_ptr<php::Class> cls; };

inline bool operator==(Context a, Context b) {
  return a.unit == b.unit && a.func == b.func && a.cls == b.cls;
}

inline bool operator<(Context a, Context b) {
  return std::make_tuple(a.unit, a.func, a.cls) <
         std::make_tuple(b.unit, b.func, b.cls);
}

std::string show(Context);

/*
 * Context for a call to a function.  This means the types and number
 * of arguments, and where it is being called from.
 *
 * TODO(#3788877): add type of $this if it is going to be an object
 * method, and the LSB class type if static.
 */
struct CallContext {
  Context caller;
  std::vector<Type> args;
};

inline bool operator==(const CallContext& a, const CallContext& b) {
  return a.caller == b.caller &&
         a.args == b.args;
}

/*
 * State of properties on a class.  Map from property name to its
 * Type.
 */
using PropState = std::map<SString,Type>;

//////////////////////////////////////////////////////////////////////

// private types
struct IndexData;
struct FuncFamily;
struct FuncInfo;
struct ClassInfo;

//////////////////////////////////////////////////////////////////////

/*
 * References to "resolved" entities with information in the index are
 * in the res:: namespace.
 *
 * These represent handles to program entities that may have variable
 * amounts of information.  For example, we may know the name of a
 * class in a res::Class, but not know for sure which php::Class
 * struct is actually associated with it.
 */
namespace res {

/*
 * A resolved runtime Class, for a particular php::Class.
 *
 * Provides various lookup tables that allow querying the Class'
 * information.
 */
struct Class {
  /*
   * Returns whether two classes are definitely same at runtime.  If
   * this function returns false, they still *may* be the same at
   * runtime.
   */
  bool same(const Class&) const;

  /*
   * Returns true if this class is definitely going to be a subtype
   * of `o' at runtime.  If this function returns false, this may
   * still be a subtype of `o' at runtime, it just may not be known.
   * A typical example is with "non unique" classes.
   */
  bool subtypeOf(const Class& o) const;

  /*
   * If this function return false, it is known that this class
   * is in no subtype relationship with the argument Class 'o'.
   * Returns true if this class could be a subtype of `o' at runtime.
   * When true is returned the two classes may still be unrelated but it is
   * not possible to tell. A typical example is with "non unique" classes.
   */
  bool couldBe(const Class& o) const;

  /*
   * Returns the name of this class.  Non-null guarantee.
   */
  SString name() const;

  /*
   * Whether this class could possibly be an interface or a trait.
   *
   * When returning false, it is known that this class is not an interface
   * or a trait. When returning true, it's possible that this class is not
   * an interface or trait but the system cannot tell.
   */
  bool couldBeInterfaceOrTrait() const;

  /*
   * Returns whether this type has the no override attribute, that is, if it
   * is a final class (explicitly marked by the user or known by the static
   * analysis).
   *
   * When returning false the class is guaranteed to be final.  When returning
   * true the system cannot tell though the class may still be final.
   */
  bool couldBeOverriden() const;

  /*
   * Whether this class (or its subtypes) could possibly have have
   * certain magic methods.
   */
  bool couldHaveMagicGet() const;

  /*
   * Returns the Class that is the first common ancestor between 'this' and 'o'.
   * If there is no common ancestor folly::none is returned
   */
  folly::Optional<Class> commonAncestor(const Class& o) const;

private:
  Class(borrowed_ptr<const Index>, Either<SString,borrowed_ptr<ClassInfo>>);

private:
  friend std::string show(const Class&);
  friend struct ::HPHP::HHBBC::Index;
  friend struct ::HPHP::HHBBC::PublicSPropIndexer;
  borrowed_ptr<const Index> index;
  Either<SString,borrowed_ptr<ClassInfo>> val;
};

/*
 * This is an abstraction layer to represent possible runtime function
 * resolutions.
 *
 * Internally, this may only know the name of the function (or method), or we
 * may know exactly which source-code-level function it refers to, or we may
 * only have ruled it down to one of a few functions in a class hierarchy.  The
 * interpreter can treat all these cases the same way using this.
 */
struct Func {
  /*
   * Returns whether two res::Funcs definitely mean the func at
   * runtime.
   *
   * Note: this is potentially pessimistic for its use in ActRec state
   * merging right now, but not incorrect.
   */
  bool same(const Func&) const;

  /*
   * Returns the name of this function.  Non-null guarantee.
   */
  SString name() const;

  /*
   * Returns whether this resolved function could possibly be going through a
   * magic call, in the magic way.
   *
   * That is, if was resolved as part of a direct call to an __call method,
   * this will say true.  If it was resolved as part as some normal method
   * call, and we haven't proven that there's no way an __call dispatch could
   * be involved, this will say false.
   */
  bool cantBeMagicCall() const;

private:
  friend struct ::HPHP::HHBBC::Index;
  struct FuncName {
    bool operator==(FuncName o) const { return name == o.name; }
    SString name;
  };
  struct MethodName {
    bool operator==(MethodName o) const { return name == o.name; }
    SString name;
  };
  using Rep = boost::variant< FuncName
                            , MethodName
                            , borrowed_ptr<FuncInfo>
                            , borrowed_ptr<FuncFamily>
                            >;

private:
  Func(borrowed_ptr<const Index>, Rep);
  friend std::string show(const Func&);

private:
  borrowed_ptr<const Index> index;
  Rep val;
};

/*
 * Produce a trace-able string for a res::Func or res::Class.
 */
std::string show(const Func&);
std::string show(const Class&);

}

//////////////////////////////////////////////////////////////////////

/*
 * This class encapsulates the known facts about the program, with a
 * whole-program view.
 *
 * This structure contains unowned pointers into the php::Program it
 * was created for.  It should not out-live the Program.
 *
 * The const member functions of this class are thread safe for
 * concurrent reads and writes.  The non-const functions should be
 * called in a single threaded context only (they are used during the
 * "update" step in between whole program analysis rounds).
 */
struct Index {
  /*
   * Create an Index for a php::Program.  Performs some initial
   * analysis of the program.
   */
  explicit Index(borrowed_ptr<php::Program>);

  /*
   * This class must not be destructed after its associated
   * php::Program.
   */
  ~Index();

  /*
   * The index operates in two modes: frozen, and unfrozen.
   *
   * Conceptually, the index is mutable and may acquire new
   * information until it has been frozen, and once frozen, it retains
   * the information it had at the point it was frozen.
   *
   * The reason this exists is because certain functions on the index
   * may cause it to need to consult information in the bodies of
   * functions other than the Context passed in.  Specifically, if the
   * interpreter tries to look up the return type for a callee in a
   * given CallContext, the index may choose to recursively invoke
   * type inference on that callee's function body to see if more
   * precise information can be determined, unless it is frozen.
   *
   * This is fine until the final pass, because all bytecode is
   * read-only at that stage.  However, in the final pass, other
   * threads might be optimizing a callee's bytecode and changing it,
   * so we should not be reading from it to perform type inference
   * concurrently.  Freezing the index tells it it can't do that
   * anymore.
   *
   * These are the functions to query and transition to frozen state.
   */
  bool frozen() const;
  void freeze();

  /*
   * The Index contains a Builder for an ArrayTypeTable.
   *
   * If we're creating assert types with options.InsertAssertions, we
   * need to keep track of which array types exist in the whole
   * program in order to include it in the repo.
   */
  std::unique_ptr<ArrayTypeTable::Builder>& array_table_builder() const;

  /*
   * Find all the closures created inside the context of a given
   * php::Class.
   */
  std::vector<borrowed_ptr<php::Class>>
    lookup_closures(borrowed_ptr<const php::Class>) const;

  /*
   * Try to resolve which class will be the class named `name' from a
   * given context, if we can resolve it to a single class.
   *
   * Note, the returned class may or may not be *defined* at the
   * program point you care about (it could be non-hoistable, even
   * though it's unique, for example).
   *
   * Returns folly::none if we can't prove the supplied name must be a
   * object type.  (E.g. if there are type aliases.)
   */
  folly::Optional<res::Class> resolve_class(Context, SString name) const;

  /*
   * Resolve a closure class.
   *
   * Returns both a resolved Class, and the actual php::Class for the
   * closure.  This function should only be used with class names are
   * guaranteed to be closures (for example, the name supplied to a
   * CreateCl opcode).
   */
  std::pair<res::Class,borrowed_ptr<php::Class>>
    resolve_closure_class(Context ctx, SString name) const;

  /*
   * Return a resolved class for a builtin class.
   *
   * Pre: `name' must be the name of a class defined in a systemlib.
   */
  res::Class builtin_class(SString name) const;

  /*
   * Try to resolve a function named `name' from a given context.
   *
   * Note, the returned function may or may not be defined at the
   * program point (it could require a function autoload that might
   * fail).
   */
  res::Func resolve_func(Context, SString name) const;

  /*
   * Try to resolve a function using namespace-style fallback lookup.
   *
   * The name `name' is tried first, and `fallback' is used if this
   * isn't found.  Both names must already be namespace-normalized.
   * Resolution can fail because there are possible situations where
   * we don't know which will be called at runtime.
   *
   * Note: the returned function may or may not be defined at the
   * program point (it could require a function autoload that might
   * fail).
   */
  folly::Optional<res::Func> resolve_func_fallback(Context,
                                                   SString name,
                                                   SString fallback) const;

  /*
   * Try to resolve a class method named `name' with a given Context
   * and class type.
   *
   * Pre: clsType.subtypeOf(TCls)
   */
  res::Func resolve_method(Context, Type clsType, SString name) const;

  /*
   * Try to resolve a class constructor for the supplied class.
   *
   * Returns: folly::none if we can't figure out which constructor
   * this would call.
   */
  folly::Optional<res::Func> resolve_ctor(Context, res::Class) const;

  /*
   * Give the Type in our type system that matches an hhvm
   * TypeConstraint, subject to the information in this Index.
   *
   * This function returns a subtype of Cell, although TypeConstraints
   * at runtime can match reference parameters.  The caller should
   * make sure to handle that case.
   *
   * For soft constraints (@), this function returns Cell.
   *
   * For some non-soft constraints (such as "Stringish"), this
   * function may return a Type that is a strict supertype of the
   * constraint's type.
   */
  Type lookup_constraint(Context, const TypeConstraint&) const;

  /*
   * If this function returns true, it is safe to assume that Type t
   * will always satisfy TypeConstraint tc at run time.
   */
  bool satisfies_constraint(Context, Type t, const TypeConstraint& tc) const;

  /*
   * Lookup what the best known Type for a class constant would be,
   * using a given Index and Context, if a class of that name were
   * loaded.
   */
  Type lookup_class_constant(Context, res::Class, SString cns) const;

  /*
   * Return the best known return type for a resolved function, in a
   * context insensitive way.  Returns TInitGen at worst.
   */
  Type lookup_return_type(Context, res::Func) const;

  /*
   * Return the best known return type for a resolved function, given
   * the supplied calling context.  Returns TInitGen at worst.
   *
   * During analyze phases, this function may re-enter analyze in
   * order to interpret the callee with these argument types.
   */
  Type lookup_return_type(CallContext, res::Func) const;

  /*
   * Look up the return type for an unresolved function.  The
   * interpreter should not use this routine---it's for stats or debug
   * dumps.
   *
   * Nothing may be writing to the index when this function is used,
   * but concurrent readers are allowed.
   */
  Type lookup_return_type_raw(borrowed_ptr<const php::Func>) const;

  /*
   * Return the best known types of a closure's used variables (on
   * entry to the closure).  The function is the closure body.
   */
  std::vector<Type>
    lookup_closure_use_vars(borrowed_ptr<const php::Func>) const;

  /*
   * Return the availability of $this on entry to the provided method.
   * If the Func provided is not a method of a class false is
   * returned.
   */
  bool lookup_this_available(borrowed_ptr<const php::Func>) const;

  /*
   * Returns the parameter preparation kind (if known) for parameter
   * `paramId' on the given resolved Func.
   */
  PrepKind lookup_param_prep(Context, res::Func, uint32_t paramId) const;

  /*
   * Returns the control-flow insensitive inferred private instance
   * property types for a Class.  The Class doesn't need to be
   * resolved, because private properties don't depend on the
   * inheritance hierarchy.
   *
   * The Index tracks the largest types for private properties that
   * are guaranteed to hold at any program point.
   */
  PropState lookup_private_props(borrowed_ptr<const php::Class>) const;

  /*
   * Returns the control-flow insensitive inferred private static
   * property types for a Class.  The class doesn't need to be
   * resolved for the same reasons as for instance properties.
   *
   * The Index tracks the largest types for private static properties
   * that are guaranteed to hold at any program point.
   */
  PropState lookup_private_statics(borrowed_ptr<const php::Class>) const;

  /*
   * Lookup the best known type for a public static property, with a given
   * class and name.
   *
   * This function will always return TInitGen before refine_public_statics has
   * been called, or if the AnalyzePublicStatics option is off.
   */
  Type lookup_public_static(Type cls, Type name) const;
  Type lookup_public_static(borrowed_ptr<const php::Class>, SString name) const;

  /*
   * Returns whether a public static property is known to be immutable.  This
   * is used to add AttrPersistent flags to static properties, and relies on
   * AnalyzePublicStatics (without this flag it will always return false).
   */
  bool lookup_public_static_immutable(borrowed_ptr<const php::Class>,
                                      SString name) const;

  /*
   * Returns the computed vtable slot for the given class, if it's an interface
   * that was given a vtable slot. No two interfaces implemented by the same
   * class will share the same vtable slot. May return kInvalidSlot, if the
   * given class isn't an interface or if it wasn't assigned a slot.
   */
  Slot lookup_iface_vtable_slot(borrowed_ptr<const php::Class>) const;

  /*
   * Refine the return type for a function, based on a round of
   * analysis.
   *
   * No other threads should be calling functions on this Index when
   * this function is called.
   *
   * Returns: the set of Contexts that depended on the return type of
   * this php::Func.
   */
  std::vector<Context> refine_return_type(borrowed_ptr<const php::Func>, Type);

  /*
   * Refine the used var types for a closure, based on a round of
   * analysis.
   *
   * No other threads should be calling functions on this Index when
   * this function is called.
   *
   * Returns: true if the types have changed.
   */
  bool refine_closure_use_vars(borrowed_ptr<const php::Class>,
                               const std::vector<Type>&);

  /*
   * Refine the private property types for a class, based on a round
   * of analysis.
   *
   * No other threads should be calling functions on this Index when
   * this function is called.
   */
  void refine_private_props(borrowed_ptr<const php::Class> cls,
                            const PropState&);

  /*
   * Refine the static private property types for a class, based on a
   * round of analysis.
   *
   * No other threads should be calling functions on this Index when
   * this function is called.
   */
  void refine_private_statics(borrowed_ptr<const php::Class> cls,
                              const PropState&);

  /*
   * After a whole program pass using PublicSPropIndexer, the types can be
   * reflected into the index for use during another type inference pass.
   *
   * No other threads should be calling functions on this Index or on the
   * provided PublicSPropIndexer when this function is called.
   */
  void refine_public_statics(const PublicSPropIndexer&);

private:
  Index(const Index&) = delete;
  Index& operator=(Index&&) = delete;

private:
  template<class FuncRange>
  res::Func resolve_func_helper(const FuncRange&, SString) const;
  res::Func do_resolve(borrowed_ptr<const php::Func>) const;
  bool must_be_derived_from(borrowed_ptr<const php::Class>,
                            borrowed_ptr<const php::Class>) const;
  bool could_be_related(borrowed_ptr<const php::Class>,
                        borrowed_ptr<const php::Class>) const;
  Type satisfies_constraint_helper(Context, const TypeConstraint&) const;

private:
  std::unique_ptr<IndexData> const m_data;
};

//////////////////////////////////////////////////////////////////////

/*
 * Indexer object used for collecting information about public static property
 * types.  See analyze_public_statics in whole-program.cpp for details about
 * how it is used.
 */
struct PublicSPropIndexer {
  explicit PublicSPropIndexer(borrowed_ptr<const Index> index)
    : m_index(index)
  {}

  /*
   * Called by the interpreter during analyze_func_collect when a
   * PublicSPropIndexer is active.  This function must be called anywhere the
   * interpreter does something that could change the type of public static
   * properties named `name' on classes of type `cls' to `val'.
   *
   * Note that if cls and name are both too generic this object will have to
   * give up all information it knows about any public static properties.
   *
   * This routine may be safely called concurrently by multiple analysis
   * threads.
   */
  void merge(Context ctx, Type cls, Type name, Type val);

private:
  friend struct Index;

  struct KnownKey {
    bool operator==(KnownKey o) const {
      return cinfo == o.cinfo && prop == o.prop;
    }

    friend size_t tbb_hasher(KnownKey k) {
      return folly::hash::hash_combine(k.cinfo, k.prop);
    }

    borrowed_ptr<ClassInfo> cinfo;
    SString prop;
  };

  using UnknownMap = tbb::concurrent_hash_map<SString,Type>;
  using KnownMap = tbb::concurrent_hash_map<KnownKey,Type>;

private:
  borrowed_ptr<const Index> m_index;
  std::atomic<bool> m_everything_bad{false};
  UnknownMap m_unknown;
  KnownMap m_known;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
