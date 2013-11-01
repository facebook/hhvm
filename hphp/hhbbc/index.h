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
#ifndef incl_HHBBC_INDEX_H_
#define incl_HHBBC_INDEX_H_

#include <memory>
#include <mutex>
#include <tuple>
#include <vector>

#include <boost/variant.hpp>

#include "folly/Optional.h"

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
 * A Context is a (unit, func, class) triple, where the class field is
 * optionally nullptr.  Most queries to the Index need a "context", to
 * allow recording dependencies.
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

//////////////////////////////////////////////////////////////////////

// private types
struct IndexData;
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
   * Returns true if this class is definitely going to be a subclass
   * of `o' at runtime.  If this function returns false, this may
   * still be a subtype of `o' at runtime, it just may not be known.
   */
  bool subtypeOf(const Class& o) const;

  /*
   * Returns true if this class could be a subclass of `o' at runtime.
   * If this function return false, it is known that this class
   * definitely is not a subclass of `o'.
   */
  bool couldBe(const Class& o) const;

  /*
   * Returns the name of this class.  Non-null guarantee.
   */
  SString name() const;

private:
  Class(borrowed_ptr<const Index>, SStringOr<ClassInfo>);

private:
  friend std::string show(const Class&);
  friend struct ::HPHP::HHBBC::Index;
  borrowed_ptr<const Index> index;
  SStringOr<ClassInfo> val;
};

/*
 * Reference to a function in the program.  We may only know the name
 * of the function, or we may have a bunch of information about it.
 */
struct Func {
  /*
   * Returns whether two res::Funcs definitely mean the func at
   * runtime.
   *
   * Explain how a res::Func can represent an unknown func but just
   * the name.
   */
  bool same(const Func&) const;

  /*
   * Returns the name of this function.  Non-null guarantee.
   */
  SString name() const;

private:
  Func(borrowed_ptr<const Index>, SStringOr<FuncInfo>);

private:
  friend std::string show(const Func&);
  friend struct ::HPHP::HHBBC::Index;
  borrowed_ptr<const Index> index;
  SStringOr<FuncInfo> val;
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
  explicit Index(borrowed_ptr<php::Program>, const Options&);

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
   * Return the best known return type for a particular function.
   * Returns TInitGen at worst.
   */
  Type lookup_return_type(Context, res::Func) const;

  /*
   * Returns the parameter preparation kind (if known) for parameter
   * `paramId' on the given resolved Func.
   */
  PrepKind lookup_param_prep(Context, res::Func, uint32_t paramId) const;

  /*
   * Refine the return type for a function, based on a round of
   * analysis.
   *
   * No threads should be reading from this Index when this function
   * is called.
   *
   * Returns: the set of Contexts that depended on the return type of
   * this php::Func.
   */
  std::vector<Context> refine_return_type(borrowed_ptr<const php::Func>, Type);

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
