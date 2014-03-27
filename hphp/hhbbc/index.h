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

#include "folly/Optional.h"

#include "hphp/util/either.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/vm/type-constraint.h"

#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/misc.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

struct Type;
struct Index;

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
   * Returns whether this type has the no override attribute, that is, if it
   * is a final class (explicitly marked by the user or known by the static
   * analysis).
   * When returning false the class is guaranteed to be final. When returning
   * true the system cannot tell though the class may still be final.
   */
  bool couldBeOverriden() const;

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
  borrowed_ptr<const Index> index;
  Either<SString,borrowed_ptr<ClassInfo>> val;
};

/*
 * This is an abstraction layer to represent possible runtime function
 * resolutions.
 *
 * Internally, this may only know the name of the function, or we may
 * know exactly which source-code-level function it refers to, or we
 * may only have ruled it down to one of a few functions in a class
 * hierarchy.  The interpreter can treat all these cases the same way
 * using this.
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

private:
  friend struct ::HPHP::HHBBC::Index;
  using Rep = boost::variant< SString
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
 * This class encapsulates the known facts about the program, possibly
 * with a whole-program view.  If the Index is created from a
 * php::Program instead of a php::Unit, it is considered a
 * "comprehensive" Index, which means it will assume it can see the
 * whole program, and may return more aggressive answers.
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
   * Create a comprehensive, whole-program-aware index.
   */
  explicit Index(borrowed_ptr<php::Program>);

  /*
   * Create a non-comprehensive index for a single php::Unit.
   */
  explicit Index(borrowed_ptr<php::Unit>);

  /*
   * This class must not be destructed after its associated
   * php::Program.
   */
  ~Index();

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
   * Try to resolve a class method named `name' with a given Context
   * and class type.
   *
   * Returns: folly::none if we can't figure out which function this
   * would call.
   *
   * Pre: clsType.subtypeOf(TCls)
   */
  folly::Optional<res::Func> resolve_method(Context,
                                            Type clsType,
                                            SString name) const;

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
   */
  Type lookup_constraint(Context, const TypeConstraint&) const;

  /*
   * Lookup what the best known Type for a class constant would be,
   * using a given Index and Context, if a class of that name were
   * loaded.
   */
  Type lookup_class_constant(Context, res::Class, SString cns) const;

  /*
   * Return the best known return type for a resolved function.
   * Returns TInitGen at worst.
   */
  Type lookup_return_type(Context, res::Func) const;

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
   * Return the availability of $this on entry to the provided method.
   * If the Func provided is not a method of a class false is returned.
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

private:
  Index(const Index&) = delete;
  Index& operator=(Index&&) = delete;

private:
  res::Func do_resolve(borrowed_ptr<const php::Func>) const;

private:
  std::unique_ptr<IndexData> const m_data;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
