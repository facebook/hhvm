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
#pragma once

#include <memory>
#include <tuple>
#include <vector>
#include <map>
#include <exception>

#include <boost/variant.hpp>
#include <tbb/concurrent_hash_map.h>

#include <folly/synchronization/Baton.h>
#include <folly/Optional.h>
#include <folly/Hash.h>

#include "hphp/util/compact-vector.h"
#include "hphp/util/either.h"
#include "hphp/util/tribool.h"

#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/vm/type-constraint.h"

#include "hphp/hhbbc/misc.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

struct Type;
struct Index;
struct PublicSPropMutations;
struct FuncAnalysisResult;
struct Context;
struct ContextHash;
struct CallContext;
struct PropertiesInfo;

extern const Type TCell;

namespace php {
struct Class;
struct Prop;
struct Record;
struct Const;
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

/*
 * A DependencyContext encodes enough of the context to record a dependency - a
 * php::Func, if we're doing private property analysis and its a suitable class,
 * a php::Class, or a public static property with a particular name.
 */

enum class DependencyContextType : uint16_t {
  Func,
  Class,
  PropName
};

using DependencyContext = CompactTaggedPtr<const void, DependencyContextType>;

struct DependencyContextLess {
  bool operator()(const DependencyContext& a,
                  const DependencyContext& b) const {
    return a.getOpaque() < b.getOpaque();
  }
};

struct DependencyContextEquals {
  bool operator()(const DependencyContext& a,
                  const DependencyContext& b) const {
    return a.getOpaque() == b.getOpaque();
  }
};

struct DependencyContextHash {
  size_t operator()(const DependencyContext& d) const {
    return pointer_hash<void>{}(reinterpret_cast<void*>(d.getOpaque()));
  }
};

struct DependencyContextHashCompare : DependencyContextHash {
  bool equal(const DependencyContext& a, const DependencyContext& b) const {
    return a.getOpaque() == b.getOpaque();
  }
  size_t hash(const DependencyContext& d) const { return (*this)(d); }
};

using DependencyContextSet = hphp_hash_set<DependencyContext,
                                           DependencyContextHash,
                                           DependencyContextEquals>;
using ContextSet = hphp_hash_set<Context, ContextHash>;

std::string show(Context);

using ConstantMap = hphp_hash_map<SString, TypedValue>;

/*
 * State of properties on a class.  Map from property name to its
 * Type.
 */
template <typename T = Type> // NB: The template param here is to
                             // break a cyclic dependency on Type.
struct PropStateElem {
  T ty;
  const TypeConstraint* tc = nullptr;
  Attr attrs;

  bool operator==(const PropStateElem<T>& o) const {
    return ty == o.ty && tc == o.tc && attrs == o.attrs;
  }
};
using PropState = std::map<LSString,PropStateElem<>>;

/*
 * The result of Index::lookup_static
 */
template <typename T = Type> // NB: The template parameter is here to
                             // break a cyclic dependency on Type.
struct PropLookupResult {
  T ty; // The best known type of the property (TBottom if not found)
  SString name; // The statically known name of the string, if any
  TriBool found; // If the property was found
  TriBool isConst; // If the property is AttrConst
  TriBool readOnly; // If the property is AttrIsReadOnly
  TriBool lateInit; // If the property is AttrLateInit
  bool classInitMightRaise; // If class initialization during the
                            // property access can raise (unlike the
                            // others, this is only no or maybe).
};

template <typename T>
inline PropLookupResult<T>& operator|=(PropLookupResult<T>& a,
                                       const PropLookupResult<T>& b) {
  assertx(a.name == b.name);
  a.ty |= b.ty;
  a.found |= b.found;
  a.isConst |= b.isConst;
  a.readOnly |= b.readOnly;
  a.lateInit |= b.lateInit;
  a.classInitMightRaise |= b.classInitMightRaise;
  return a;
}

std::string show(const PropLookupResult<Type>&);

/*
 * The result of Index::merge_static_type
 */
template <typename T = Type> // NB: The template parameter is here to
                             // break a cyclic dependency on Type
struct PropMergeResult {
  T adjusted; // The merged type, potentially adjusted according to
              // the prop's type-constraint (it's the subtype of the
              // merged type that would succeed).
  TriBool throws; // Whether the mutation this merge represents
                  // can throw.
};

template <typename T>
inline PropMergeResult<T>& operator|=(PropMergeResult<T>& a,
                                      const PropMergeResult<T>& b) {
  a.adjusted |= b.adjusted;
  a.throws |= b.throws;
  return a;
}

std::string show(const PropMergeResult<Type>&);

//////////////////////////////////////////////////////////////////////

// private types
struct ClassInfo;
struct RecordInfo;

//////////////////////////////////////////////////////////////////////

/*
 * References to "resolved" entities with information in the index are
 * in the res:: namespace.
 *
 * These represent handles to program entities that may have variable
 * amounts of information.  For example, we may know the name of a
 * class in a res::Class, but do not know for sure which php::Class
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
  bool mustBeSubtypeOf(const Class& o) const;

  /*
   * Returns false if this class is definitely not going to be a subtype
   * of `o' at runtime.  If this function returns true, this may
   * still not be a subtype of `o' at runtime, it just may not be known.
   * A typical example is with "non unique" classes.
   */
  bool maybeSubtypeOf(const Class& o) const;

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
   * Whether this class could possibly be an interface/interface or trait.
   *
   * True means it might be, false means it is not.
   */
  bool couldBeInterface() const;

  /*
   * Whether this class must be an interface.
   *
   * True means it is, false means it might not be.
   */
  bool mustBeInterface() const;
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
   * a magic toBoolean() method.
   */
  bool couldHaveMagicBool() const;

  /*
   * Whether this class could possibly have a derived class that is mocked.
   * Including itself.
   */
  bool couldHaveMockedDerivedClass() const;

  /*
   * Whether this class could possibly be mocked.
   */
  bool couldBeMocked() const;

  /*
   * Whether this class could have reified generics
   */
  bool couldHaveReifiedGenerics() const;

  /*
   * Returns whether this resolved class might distinguish being constructed
   * dynamically versus being constructed normally (IE, might raise a notice).
   */
  bool mightCareAboutDynConstructs() const;

  /*
   * Whether this class (or clases derived from it) could have const props.
   */
  bool couldHaveConstProp() const;
  bool derivedCouldHaveConstProp() const;

  /*
   * Returns the Class that is the first common ancestor between 'this' and 'o'.
   * If there is no common ancestor folly::none is returned
   */
  folly::Optional<Class> commonAncestor(const Class& o) const;

  /*
   * Returns the res::Class for this Class's parent if there is one,
   * or nullptr.
   */
  folly::Optional<Class> parent() const;

  /*
   * Returns true if we have a ClassInfo for this Class.
   */
  bool resolved() const {
    return val.right() != nullptr;
  }

  /*
   * Returns the php::Class for this Class if there is one, or
   * nullptr.
   */
  const php::Class* cls() const;

private:
  explicit Class(Either<SString,ClassInfo*>);
  template <bool> bool subtypeOfImpl(const Class&) const;

private:
  friend std::string show(const Class&);
  friend struct ::HPHP::HHBBC::Index;
  friend struct ::HPHP::HHBBC::PublicSPropMutations;
  Either<SString,ClassInfo*> val;
};

/*
 * A resolved runtime Record, for a particular php::Record.
 *
 * Provides various lookup tables that allow querying the Record's
 * information.
 */
struct Record {
  /*
   * Returns whether two records are definitely same at runtime.  If
   * this function returns false, they still *may* be the same at
   * runtime.
   */
  bool same(const Record&) const;

  /*
   * Returns true if this record is definitely going to be a subtype
   * of `o' at runtime.  If this function returns false, this may
   * still be a subtype of `o' at runtime, it just may not be known.
   * A typical example is with "non unique" records.
   */
  bool mustBeSubtypeOf(const Record& o) const;

  /*
   * Returns false if this record is definitely not going to be a subtype
   * of `o' at runtime.  If this function returns true, this may
   * still not be a subtype of `o' at runtime, it just may not be known.
   */
  bool maybeSubtypeOf(const Record& o) const;

  /*
   * If this function return false, it is known that this record
   * is in no subtype relationship with the argument record 'o'.
   * Returns true if this record could be a subtype of `o' at runtime.
   * When true is returned the two records may still be unrelated but it is
   * not possible to tell. A typical example is with "non unique" records.
   */
  bool couldBe(const Record& o) const;

  /*
   * Returns false if this is a final record.
   */
  bool couldBeOverriden() const;

  /*
   * Returns the name of this record.  Non-null guarantee.
   */
  SString name() const;

  /*
   * Returns the res::Record for this Record's parent if there is one,
   * or nullptr.
   */
  folly::Optional<Record> parent() const;

  /*
   * Returns the Record that is the first common ancestor between
   * 'this' and 'o'.
   * If there is no common ancestor folly::none is returned
   */
  folly::Optional<Record> commonAncestor(const Record&) const;

  /*
   * Returns true if we have a RecordInfo for this Record.
   */
  bool resolved() const {
    return val.right() != nullptr;
  }

  /*
   * Returns the php::Record for this Record if there is one, or
   * nullptr.
   */
  const php::Record* rec() const;

private:
  explicit Record(Either<SString,RecordInfo*>);
  template <bool> bool subtypeOfImpl(const Record&) const;

private:
  friend std::string show(const Record&);
  friend struct ::HPHP::HHBBC::Index;
  Either<SString,RecordInfo*> val;
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
   * Returns the name of this function.  Non-null guarantee.
   */
  SString name() const;

  /*
   * If this resolved function represents exactly one php::Func, return it.
   */
  const php::Func* exactFunc() const;

  /*
   * Returns whether this resolved function is definitely safe to constant fold.
   */
  bool isFoldable() const;

  /*
   * Whether this function could have reified generics
   */
  bool couldHaveReifiedGenerics() const;

  /*
   * Returns whether this resolved function might distinguish being called
   * dynamically versus being called normally (IE, might raise a notice).
   */
  bool mightCareAboutDynCalls() const;

  /*
   * Returns whether this resolved function might be a builtin.
   */
  bool mightBeBuiltin() const;

  /*
   * Minimum/maximum bound on the number of non-variadic parameters of the
   * function.
   */
  uint32_t minNonVariadicParams() const;
  uint32_t maxNonVariadicParams() const;

  struct FuncInfo;
  struct MethTabEntryPair;
  struct FuncFamily;

private:
  friend struct ::HPHP::HHBBC::Index;
  struct FuncName {
    FuncName(SString n, bool r) : name{n}, renamable{r} {}
    bool operator==(FuncName o) const { return name == o.name; }
    SString name;
    bool renamable;
  };
  struct MethodName {
    bool operator==(MethodName o) const { return name == o.name; }
    SString name;
  };
  using Rep = boost::variant< FuncName
                            , MethodName
                            , FuncInfo*
                            , const MethTabEntryPair*
                            , FuncFamily*
                            >;

private:
  Func(const Index*, Rep);
  friend std::string show(const Func&);

private:
  const Index* index;
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

  struct NonUniqueSymbolException : std::exception {
    explicit NonUniqueSymbolException(std::string msg) : msg(msg) {}
    const char* what() const noexcept override { return msg.c_str(); }
  private:
    std::string msg;
  };

  /*
   * Create an Index for a php::Program.  Performs some initial
   * analysis of the program.
   */
  explicit Index(php::Program*);

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
  void thaw();

  /*
   * Throw away data structures that won't be needed during or after
   * the final pass. Currently the dependency map, which can take a
   * long time to destroy.
   */
  void cleanup_for_final();

  /*
   * Throw away data structures that won't be needed after the emit
   * stage.
   */
  void cleanup_post_emit();

  /*
   * The Index contains a Builder for an ArrayTypeTable.
   *
   * If we're creating assert types with options.InsertAssertions, we
   * need to keep track of which array types exist in the whole
   * program in order to include it in the repo.
   */
  std::unique_ptr<ArrayTypeTable::Builder>& array_table_builder() const;

  /*
   * Try to resolve which record will be the record named `name`,
   * if we can resolve it to a single record.
   *
   * Note, the returned record may or may not be *defined* at the
   * program point you care about (it could be non-hoistable, even
   * though it's unique, for example).
   *
   * Returns folly::none if we can't prove the supplied name must be a
   * record type.  (E.g. if there are type aliases.)
   */
  folly::Optional<res::Record> resolve_record(SString name) const;

  /*
   * Find all the closures created inside the context of a given
   * php::Class.
   */
  const CompactVector<const php::Class*>*
    lookup_closures(const php::Class*) const;

  /*
   * Find all the extra methods associated with a class from its
   * traits.
   */
  const hphp_fast_set<const php::Func*>*
    lookup_extra_methods(const php::Class*) const;

  /*
   * Try to find a res::Class for a given php::Class.
   *
   * Note, the returned class may or may not be *defined* at the
   * program point you care about (it could be non-hoistable, even
   * though it's unique, for example).
   *
   * Returns a name-only resolution if there are no legal
   * instantiations of the class, or if there is more than one.
   */
  res::Class resolve_class(const php::Class*) const;

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
   * Try to resolve self/parent types for the given context
   */
  folly::Optional<res::Class> selfCls(const Context& ctx) const;
  folly::Optional<res::Class> parentCls(const Context& ctx) const;

  template <typename T>
  struct ResolvedInfo {
    AnnotType                               type;
    bool                                    nullable;
    T value;
  };

  /*
   * Try to resolve name, looking through TypeAliases and enums.
   */
  ResolvedInfo<boost::variant<boost::blank,res::Class,res::Record>>
  resolve_type_name(SString name) const;

  /*
   * Resolve a closure class.
   *
   * Returns both a resolved Class, and the actual php::Class for the
   * closure.
   */
  std::pair<res::Class,php::Class*>
    resolve_closure_class(Context ctx, int32_t idx) const;

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
   * Pre: clsType.subtypeOf(BCls)
   */
  res::Func resolve_method(Context, Type clsType, SString name) const;

  /*
   * Try to resolve a class constructor for the supplied class type.
   *
   * Returns: folly::none if it can't at least figure out a func
   * family for the call.
   */
  folly::Optional<res::Func>
  resolve_ctor(Context, res::Class rcls, bool exact) const;

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
   *
   * If something is known about the type of the object against which
   * the constraint will be checked, it can be passed in to help
   * disambiguate certain constraints (useful because we don't support
   * arbitrary unions, or intersection).
   */
  Type lookup_constraint(Context, const TypeConstraint&,
                         const Type& t = TCell) const;

  /*
   * If this function returns true, it is safe to assume that Type t
   * will always satisfy TypeConstraint tc at run time.
   */
  bool satisfies_constraint(Context, const Type& t,
                            const TypeConstraint& tc) const;

  /*
   * Returns true if the given type-hint (declared on the given class) might not
   * be enforced at runtime (IE, it might map to mixed or be soft).
   */
  bool prop_tc_maybe_unenforced(const php::Class& propCls,
                                const TypeConstraint& tc) const;

  /*
   * Returns true if the type constraint can contain a reified type
   * Currently, only classes and interfaces are supported
   */
  bool could_have_reified_type(Context ctx, const TypeConstraint& tc) const;

  /*
   * Lookup what the best known Type for a class constant would be,
   * using a given Index and Context, if a class of that name were
   * loaded.
   * If allow_tconst is not set, type constants will not be returned.
   * lookup_class_const_ptr version returns the statically known version
   * of the const if it can find it, otherwise returns nullptr.
   */
  Type lookup_class_constant(Context, res::Class, SString cns,
                             bool allow_tconst) const;
  const php::Const* lookup_class_const_ptr(Context, res::Class, SString cns,
                                           bool allow_tconst) const;

  /*
   * Lookup what the best known Type for a constant would be, using a
   * given Index and Context, if a constant of that name were defined.
   */
  Type lookup_constant(Context ctx, SString cnsName) const;

  /*
   * Return true if the return value of the function might depend on arg.
   */
  bool func_depends_on_arg(const php::Func* func, int arg) const;

  /*
   * If func is effect-free when called with args, and it returns a constant,
   * return that constant; otherwise return TInitCell.
   */
  Type lookup_foldable_return_type(Context ctx,
                                   const php::Func* func,
                                   Type ctxType,
                                   CompactVector<Type> args) const;
  /*
   * Return the best known return type for a resolved function, in a
   * context insensitive way.  Returns TInitCell at worst.
   */
  Type lookup_return_type(Context, res::Func, Dep dep = Dep::ReturnTy) const;

  /*
   * Return the best known return type for a resolved function, given
   * the supplied calling context.  Returns TInitCell at worst.
   *
   * During analyze phases, this function may re-enter analyze in
   * order to interpret the callee with these argument types.
   */
  Type lookup_return_type(Context caller,
                          const CompactVector<Type>& args,
                          const Type& context,
                          res::Func,
                          Dep dep = Dep::ReturnTy) const;

  /*
   * Look up the return type for an unresolved function.  The
   * interpreter should not use this routine---it's for stats or debug
   * dumps.
   *
   * Nothing may be writing to the index when this function is used,
   * but concurrent readers are allowed.
   */
  Type lookup_return_type_raw(const php::Func*) const;

  /*
   * Return the best known types of a closure's used variables (on
   * entry to the closure).  The function is the closure body.
   *
   * If move is true, the value will be moved out of the index. This
   * should only be done at emit time. (note that the only other user
   * of this info is analysis, which only uses it when processing the
   * owning class, so its safe to kill after emitting the owning
   * unit).
   */
  CompactVector<Type>
    lookup_closure_use_vars(const php::Func*,
                            bool move = false) const;

  /*
   * Return the availability of $this on entry to the provided method.
   * If the Func provided is not a method of a class false is
   * returned.
   */
  bool lookup_this_available(const php::Func*) const;

  /*
   * Returns the parameter preparation kind (if known) for parameter
   * `paramId' on the given resolved Func.
   */
  PrepKind lookup_param_prep(Context, res::Func, uint32_t paramId) const;

  /*
   * Returns the number of inout parameters expected by func (if known).
   */
  folly::Optional<uint32_t> lookup_num_inout_params(Context, res::Func) const;

  /*
   * Returns the control-flow insensitive inferred private instance
   * property types for a Class.  The Class doesn't need to be
   * resolved, because private properties don't depend on the
   * inheritance hierarchy.
   *
   * The Index tracks the largest types for private properties that
   * are guaranteed to hold at any program point.
   *
   * If move is true, the value will be moved out of the index. This
   * should only be done at emit time. (note that the only other user
   * of this info is analysis, which only uses it when processing the
   * owning class, so its safe to kill after emitting the owning
   * unit).
   */
  PropState lookup_private_props(const php::Class*,
                                 bool move = false) const;

  /*
   * Returns the control-flow insensitive inferred private static
   * property types for a Class.  The class doesn't need to be
   * resolved for the same reasons as for instance properties.
   *
   * The Index tracks the largest types for private static properties
   * that are guaranteed to hold at any program point.
   *
   * If move is true, the value will be moved out of the index. This
   * should only be done at emit time. (note that the only other user
   * of this info is analysis, which only uses it when processing the
   * owning class, so its safe to kill after emitting the owning
   * unit).
   */
  PropState lookup_private_statics(const php::Class*,
                                   bool move = false) const;
  PropState lookup_public_statics(const php::Class*) const;

  /*
   * Lookup metadata about the static property access `cls'::`name',
   * in the current context `ctx'. The returned metadata not only
   * includes the best known type of the property, but whether it is
   * definitely found, and whether the access might raise for various
   * reasons. This function is responsible for walking the class
   * hierarchy to find the appropriate property while applying
   * accessibility rules. This is intended to be the source of truth
   * about static properties during analysis.
   */
  PropLookupResult<> lookup_static(Context ctx,
                                   const PropertiesInfo& privateProps,
                                   const Type& cls,
                                   const Type& name) const;

  /*
   * Lookup if initializing (which is a side-effect of several bytecodes) the
   * given class might raise.
   */
  bool lookup_class_init_might_raise(Context, res::Class) const;

  /*
   * Lookup the best known type for a public (non-static) property. Since we
   * don't do analysis on public properties, this just inferred from the
   * property's type-hint (if enforced).
   */
  Type lookup_public_prop(const Type& cls, const Type& name) const;
  Type lookup_public_prop(const php::Class* cls, SString name) const;

  /*
   * We compute the interface vtables in a separate thread. It needs
   * to be joined (in single threaded context) before calling
   * lookup_iface_vtable_slot.
   */
  void join_iface_vtable_thread() const;

  /*
   * Returns the computed vtable slot for the given class, if it's an interface
   * that was given a vtable slot. No two interfaces implemented by the same
   * class will share the same vtable slot. May return kInvalidSlot, if the
   * given class isn't an interface or if it wasn't assigned a slot.
   */
  Slot lookup_iface_vtable_slot(const php::Class*) const;

  /*
   * Return the DependencyContext for ctx.
   */
  DependencyContext dependency_context(const Context& ctx) const;

  /*
   * Determine whether to use class-at-a-time, or function-at-a-time
   * dependencies.
   *
   * Must be called in single-threaded context.
   */
  void use_class_dependencies(bool f);

  /*
   * Merge the type `val' into the known type for static property
   * `cls'::`name'. Depending on what we know about `cls' and `name',
   * this might affect multiple properties. This function is
   * responsible for walking the class hierarchy to find the
   * appropriate property while applying accessibility
   * rules. Mutations of AttrConst properties are ignored unless
   * `ignoreConst' is set to true. If `checkUB' is true, upper-bound
   * type constraints are consulted in addition to the normal type
   * constraints.
   *
   * The result tells you the subtype of val that would be
   * successfully set (according to the type constraints), and if the
   * mutation would throw or not.
   */
  PropMergeResult<> merge_static_type(Context ctx,
                                      PublicSPropMutations& publicMutations,
                                      PropertiesInfo& privateProps,
                                      const Type& cls,
                                      const Type& name,
                                      const Type& val,
                                      bool checkUB = false,
                                      bool ignoreConst = false,
                                      bool mustBeReadOnly = false) const;

  /*
   * Initialize the initial types for public static properties. This should be
   * done after rewriting initial property values, as that affects the types.
   */
  void init_public_static_prop_types();

  /*
   * Refine the types of the class constants defined by an 86cinit,
   * based on a round of analysis.
   *
   * No other threads should be using ctx.cls->constants or deps when
   * this function is called.
   *
   * Merges the set of Contexts that depended on the constants defined
   * by this 86cinit.
   */
  void refine_class_constants(
    const Context& ctx,
    const CompactVector<std::pair<size_t, TypedValue>>& resolved,
    DependencyContextSet& deps);

  /*
   * Refine the types of the constants defined by a function, based on
   * a round of analysis.
   *
   * Constants not defined by a pseudomain are considered unknowable
   *
   * No other threads should be calling functions on this Index when
   * this function is called.
   *
   * Merges the set of Contexts that depended on the constants defined
   * by this php::Func into deps.
   */
  void refine_constants(const FuncAnalysisResult& fa,
                        DependencyContextSet& deps);

  /*
   * Refine the return type for a function, based on a round of
   * analysis.
   *
   * No other threads should be calling functions on this Index when
   * this function is called.
   *
   * Merges the set of Contexts that depended on the return type of
   * this php::Func into deps.
   */
  void refine_return_info(const FuncAnalysisResult& fa,
                          DependencyContextSet& deps);

  /*
   * Refine the used var types for a closure, based on a round of
   * analysis.
   *
   * No other threads should be calling functions on this Index when
   * this function is called.
   *
   * Returns: true if the types have changed.
   */
  bool refine_closure_use_vars(const php::Class*,
                               const CompactVector<Type>&);

  /*
   * Refine the private property types for a class, based on a round
   * of analysis.
   *
   * No other threads should be calling functions on this Index when
   * this function is called.
   */
  void refine_private_props(const php::Class* cls,
                            const PropState&);

  /*
   * Refine the static private property types for a class, based on a
   * round of analysis.
   *
   * No other threads should be calling functions on this Index when
   * this function is called.
   */
  void refine_private_statics(const php::Class* cls,
                              const PropState&);

  /*
   * Record in the index that the given set of public static property mutations
   * has been found while analyzing the given function. During a round of
   * analysis, the mutations are gathered from the analysis results for each
   * function, recorded in the index, and then refine_public_statics is called
   * to process the mutations and update the index.
   *
   * No other threads should be calling functions on this Index when this
   * function is called.
   */
  void record_public_static_mutations(const php::Func& func,
                                      PublicSPropMutations mutations);


  /*
   * If we resolve the intial value of a public property, we need to
   * tell the refine_public_statics phase about it, because the init
   * value won't be included in the mutations any more.
   *
   * Note that we can't modify the initial value here, because other
   * threads might be reading it (via loookup_public_static), so we
   * set a flag to tell us to update it during the next
   * refine_public_statics pass.
   */
  void update_static_prop_init_val(const php::Class* cls,
                                   SString name) const;
  /*
   * After a round of analysis with all the public static property mutations
   * being recorded with record_public_static_mutations, the types can be
   * reflected into the index for use during another type inference pass.
   *
   * No other threads should be calling functions on this Index when this
   * function is called.
   *
   * Merges the set of Contexts that depended on a public static property whose
   * type has changed.
   */
  void refine_public_statics(DependencyContextSet& deps);

  /*
   * Refine whether the given class has properties with initial values which
   * might violate their type-hints.
   *
   * No other threads should be calling functions on this Index when this
   * function is called.
   */
  void refine_bad_initial_prop_values(const php::Class* cls,
                                      bool value,
                                      DependencyContextSet& deps);

  /*
   * Mark any properties in cls that definitely do not redeclare a property in
   * the parent, which has an inequivalent type-hint.
   */
  void mark_no_bad_redeclare_props(php::Class& cls) const;

  /*
   * Rewrite the initial values of any AttrSystemInitialValue properties to
   * something more suitable for its type-hint, and add AttrNoImplicitNullable
   * where appropriate.
   *
   * This must be done before any analysis is done, as the initial values
   * affects the analysis.
   */
  void rewrite_default_initial_values(php::Program&) const;

  /*
   * Return true if the resolved function supports async eager return.
   */
  folly::Optional<bool> supports_async_eager_return(res::Func rfunc) const;

  /*
   * Return true if the function is effect free.
   */
  bool is_effect_free(res::Func rfunc) const;
  bool is_effect_free(const php::Func* func) const;

  /*
   * Do any necessary fixups to a return type.
   *
   * Note that eg for an async function it will map Type to
   * WaitH<Type>.
   */
  void fixup_return_type(const php::Func*, Type&) const;

  /*
   * Return true if we know for sure that one php::Class must derive
   * from another at runtime, in all possible instantiations.
   */
  bool must_be_derived_from(const php::Class*,
                            const php::Class*) const;

  struct IndexData;
private:
  Index(const Index&) = delete;
  Index& operator=(Index&&) = delete;

private:
  friend struct PublicSPropMutations;

  res::Func resolve_func_helper(const php::Func*, SString) const;
  res::Func do_resolve(const php::Func*) const;
  bool could_be_related(const php::Class*,
                        const php::Class*) const;

  template<bool getSuperType>
  Type get_type_for_constraint(Context,
                               const TypeConstraint&,
                               const Type&) const;

  struct ConstraintResolution;

  /*
   * Try to resolve name in the given context. Follows TypeAliases.
   */
  ConstraintResolution resolve_named_type(
      const Context& ctx, SString name, const Type& candidate) const;

  ConstraintResolution get_type_for_annotated_type(
    Context ctx, AnnotType annot, bool nullable,
    SString name, const Type& candidate) const;

  void init_return_type(const php::Func* func);

  ResolvedInfo<boost::variant<boost::blank,SString,ClassInfo*,RecordInfo*>>
  resolve_type_name_internal(SString name) const;

  template<typename T>
  folly::Optional<T> resolve_type_impl(SString name) const;

private:
  std::unique_ptr<IndexData> const m_data;
};

//////////////////////////////////////////////////////////////////////

/*
 * Used for collecting all mutations of public static property types.
 */
struct PublicSPropMutations {
private:
  friend struct Index;

  struct KnownKey {
    bool operator<(KnownKey o) const {
      if (cinfo != o.cinfo) return cinfo < o.cinfo;
      return prop < o.prop;
    }

    ClassInfo* cinfo;
    SString prop;
  };

  using UnknownMap = std::map<SString,Type>;
  using KnownMap = std::map<KnownKey,Type>;

  // Public static property mutations are actually rare, so defer allocating the
  // maps until we actually see one.
  struct Data {
    bool m_nothing_known{false};
    UnknownMap m_unknown;
    KnownMap m_known;
  };
  std::unique_ptr<Data> m_data;

  Data& get();

  void mergeKnown(const ClassInfo* ci, const php::Prop& prop, const Type& val);
  void mergeUnknownClass(SString prop, const Type& val);
  void mergeUnknown(Context);
};

//////////////////////////////////////////////////////////////////////

}}
