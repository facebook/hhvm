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
#include <folly/Hash.h>

#include "hphp/util/compact-vector.h"
#include "hphp/util/either.h"
#include "hphp/util/tiny-vector.h"
#include "hphp/util/tribool.h"

#include "hphp/runtime/base/repo-auth-type.h"

#include "hphp/runtime/vm/coeffects.h"
#include "hphp/runtime/vm/type-constraint.h"

#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/misc.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

struct Type;
struct Index;
struct PublicSPropMutations;
struct FuncAnalysisResult;
struct Context;
struct ContextHash;
struct CallContext;
struct PropertiesInfo;
struct MethodsInfo;

struct TypeStructureResolution;

struct DCls;

extern const Type TCell;

namespace php {
struct Class;
struct Prop;
struct Const;
struct Func;
struct Unit;
struct Program;
struct TypeAlias;
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
  /* This dependency should trigger when a public static property changes */
  PublicSProp = (1u << 4),
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
 * a php::Class, or a public static property.
 */

enum class DependencyContextType : uint16_t {
  Func,
  Class,
  Prop,
  FuncFamily
};

using DependencyContext = CompactTaggedPtr<const void, DependencyContextType>;

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

using DependencyContextSet = hphp_fast_set<DependencyContext,
                                           DependencyContextHash,
                                           DependencyContextEquals>;

std::string show(Context);

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
  bool everModified;

  bool operator==(const PropStateElem<T>& o) const {
    return
      ty == o.ty &&
      tc == o.tc &&
      attrs == o.attrs &&
      everModified == o.everModified;
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
  TriBool readOnly; // If the property is AttrIsReadonly
  TriBool lateInit; // If the property is AttrLateInit
  TriBool internal; // If the property is Internal
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
  a.internal |= b.internal;
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

/*
 * The result of Index::lookup_class_constant
 */
template <typename T = Type> // NB: The template parameter is here to
                             // break a cyclic dependency on Type
struct ClsConstLookupResult {
  T ty;            // The best known type of the constant (might not be a
                   // scalar).
  TriBool found;   // If the constant was found
  bool mightThrow; // If accessing the constant can throw
};

template <typename T>
inline ClsConstLookupResult<T>& operator|=(ClsConstLookupResult<T>& a,
                                           const ClsConstLookupResult<T>& b) {
  a.ty |= b.ty;
  a.found |= b.found;
  a.mightThrow |= b.mightThrow;
  return a;
}

std::string show(const ClsConstLookupResult<Type>&);

/*
 * The result of Index::lookup_class_type_constant
 */
template <typename T = TypeStructureResolution>
struct ClsTypeConstLookupResult {
  T resolution;     // The result from resolving the type-structure
  TriBool found;    // If the constant was found
  TriBool abstract; // If the constant was abstract (this only applies
                    // to the subset which wasn't found).
};

template <typename T>
inline ClsTypeConstLookupResult<T>& operator|=(
    ClsTypeConstLookupResult<T>& a,
    const ClsTypeConstLookupResult<T>& b) {
  a.resolution |= b.resolution;
  if (a.found == TriBool::Yes) {
    a.abstract = b.abstract;
  } else if (b.found != TriBool::Yes) {
    a.abstract |= b.abstract;
  }
  a.found |= b.found;
  return a;
}

std::string show(const ClsTypeConstLookupResult<TypeStructureResolution>&);

//////////////////////////////////////////////////////////////////////

// private types
struct ClassInfo;
struct UnresolvedClassMaker;

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
   * Returns whether two (exact) classes are definitely same at
   * runtime. This ignores all complications due to non-regular
   * classes and is usually not what you want to use.
   */
  bool same(const Class& o) const;

  /*
   * Returns true if this class is a subtype of 'o'. That is, every
   * subclass of 'this' (including 'this' itself) is a subclass of
   * 'o'. For the exact variant, 'this' is considered to be exact
   * (that is, exactly that class). For the sub variant, 'this' could
   * also be a subclass. This distinction only matters for non-regular
   * classes. 'o' is considered to be a subclass except for
   * exactSubtypeOfExact. This is needed since two exact classes, even
   * if the same, may not be a sub-type of one another if
   * nonRegularL/nonRegularR differ.
   *
   * Classes can have implementations/subclasses which aren't
   * "regular" classes (interfaces/traits/abstract/etc). Whether these
   * will be considered as part of the check (on either side) is
   * context dependent and specified by nonRegularL and nonRegularR.
   */
  bool exactSubtypeOf(const Class& o, bool nonRegularL, bool nonRegularR) const;
  bool exactSubtypeOfExact(
      const Class& o, bool nonRegularL, bool nonRegularR
  ) const;
  bool subSubtypeOf(const Class& o, bool nonRegularL, bool nonRegularR) const;

  /*
   * Returns false if this class has no subclasses in common with
   * 'o'. This is equivalent to saying that their intersection is
   * empty. For the exact variant, 'this' is considered to be exactly
   * that class (no sub-classes). For the sub variant, 'this' might be
   * that class, or any subclass of that class.
   *
   * Since couldBe is symmetric, this covers all of the cases. 'o' is
   * considered to be a subclass, except for exactCouldBeExact. This
   * is needed since two exact classes, even if the same, may not have
   * anything in common if nonRegularL/nonRegularR differ.
   *
   * Classes can have implementations/subclasses which aren't
   * "regular" classes (interfaces/traits/abstract/etc). Whether these
   * will be considered as part of the check (on either side) is
   * context dependent and specified by nonRegularL and nonRegularR.
   */
  bool exactCouldBe(const Class& o, bool nonRegularL, bool nonRegularR) const;
  bool exactCouldBeExact(
      const Class& o, bool nonRegularL, bool nonRegularR
  ) const;
  bool subCouldBe(const Class& o, bool nonRegularL, bool nonRegularR) const;

  /*
   * Returns the name of this class.  Non-null guarantee.
   */
  SString name() const;

  /*
   * Returns whether this class is a final class as determined by
   * static analysis. The first variant considers all classes, while
   * the second variant only considers potentially overriding regular
   * classes (this distinction is important if you already have an
   * instance of the class, as abstract classes cannot be
   * instantiated).
   *
   * NB: Traits never can be overridden and will always return false
   * (their subclass list reflects users which the trait will be
   * flattened into at runtime).
   *
   * When returning false the class is guaranteed to be final. When
   * returning true the system cannot tell though the class may still
   * be final.
   */
  bool couldBeOverridden() const;
  bool couldBeOverriddenByRegular() const;

  /*
   * Returns whether this class might be a regular/non-regular class
   * at runtime. For resolved classes this check is precise, but for
   * unresolved classes this will always conservatively return
   * true. This only checks the class itself and says nothing about
   * any potential subclasses of this.
   */
  bool mightBeRegular() const;
  bool mightBeNonRegular() const;

  /*
   * Whether this class (or its subtypes) might be a non-regular
   * class. For resolved classes this check is precise, but for
   * unresolved classes this will always conservatively return true.
   */
  bool mightContainNonRegular() const;

  /*
   * Return the best class equivalent to this, but with any
   * non-regular classes removed. By equivalent, we mean the most
   * specific representable class which contains all of the same
   * regular subclasses that the original did. If std::nullopt is
   * returned, this class contains no regular classes (and therefore
   * the result is a Bottom). The returned classes may be the original
   * class, and for interfaces and abstract classes, the result may
   * not be an interface or abstract class.
   */
  Optional<res::Class> withoutNonRegular() const;

  /*
   * Whether this class (or its subtypes) could possibly have have
   * a magic toBoolean() method.
   */
  bool couldHaveMagicBool() const;

  /*
   * Whether this class could possibly have a sub-class that is
   * mocked, including itself.
   */
  bool couldHaveMockedSubClass() const;

  /*
   * Whether this class could possibly be mocked.
   */
  bool couldBeMocked() const;

  /*
   * Whether this class could have reified generics
   */
  bool couldHaveReifiedGenerics() const;

  /*
   * Whether this class must have reified generics
   */
  bool mustHaveReifiedGenerics() const;

  /*
   * Whether this class could have a reified parent
   */
  bool couldHaveReifiedParent() const;

  /*
   * Whether this class must have a reified parent
   */
  bool mustHaveReifiedParent() const;

  /*
   * Returns whether this resolved class might distinguish being constructed
   * dynamically versus being constructed normally (IE, might raise a notice).
   */
  bool mightCareAboutDynConstructs() const;

  /*
   * Whether this class (or clases derived from it) could have const props.
   */
  bool couldHaveConstProp() const;
  bool subCouldHaveConstProp() const;

  /*
   * Returns the res::Class for this Class's parent if there is one,
   * or nullptr.
   */
  Optional<Class> parent() const;

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

  /*
   * Invoke the given function on every possible subclass of this
   * class. This must be a resolved class.
   */
  void forEachSubclass(const std::function<void(const php::Class*)>&) const;

  /*
   * Given two lists of classes, calculate the union between them (in
   * canonical form). A list of size 1 represents a single class, and
   * a larger list represents an intersection of classes. The input
   * lists are assumed to be in canonical form. If the output is an
   * empty list, the union is *all* classes (corresponding to TObj or
   * TCls). This function is really an implementation detail of
   * union_of() and not a general purpose interface.
   */
  static TinyVector<Class, 2> combine(folly::Range<const Class*> classes1,
                                      folly::Range<const Class*> classes2,
                                      bool isSub1,
                                      bool isSub2,
                                      bool nonRegular1,
                                      bool nonRegular2);
  /*
   * Given two lists of classes, calculate the intersection between
   * them (in canonical form). A list of size 1 represents a single
   * class, and a larger list represents an intersection of
   * classes. The input lists are assumed to be in canonical form. If
   * the output is an empty list, the intersection is empty
   * (equivalent to Bottom). This function is really an implementation
   * detail of intersection_of() and not a general purpose interface.
   */
  static TinyVector<Class, 2> intersect(folly::Range<const Class*> classes1,
                                        folly::Range<const Class*> classes2,
                                        bool nonRegular1,
                                        bool nonRegular2,
                                        bool& nonRegularOut);

  /*
   * Given a list of classes, return a new list of classes
   * representing all of the non-regular classes removed (and
   * canonicalized). If the output list is empty, there are no regular
   * classes.
   */
  static TinyVector<Class, 2>
  removeNonRegular(folly::Range<const Class*> classes);

  /*
   * Given two lists of classes, calculate whether their intersection
   * is non-empty. This is equivalent to calling intersect and
   * checking the result, but more efficient.
   */
  static bool couldBeIsect(folly::Range<const Class*> classes1,
                           folly::Range<const Class*> classes2,
                           bool nonRegular1,
                           bool nonRegular2);

  /*
   * Produce an unresolved class representing the base class of a wait
   * handle (this will be a sub-class of Awaitable). Since this is
   * unresolved, it does not require an Index.
   */
  static Class unresolvedWaitHandle();

  /*
   * Convert this class to/from an opaque integer. The integer is
   * "pointerish" (has upper bits cleared), so can be used in
   * something like CompactTaggedPtr. It is not, however, guaranteed
   * to be aligned (lower bits may be set).
   */
  uintptr_t toOpaque() const { return val.toOpaque(); }
  static Class fromOpaque(uintptr_t o) {
    return Class{decltype(val)::fromOpaque(o)};
  }

  size_t hash() const { return val.toOpaque(); }

  /*
   * NB: Serd-ing a Class only encodes the name. Deserializing it
   * always produces a name-only unresolved class, regardless of the
   * original. If necessary, the Class must be manually resolved
   * afterwards.
   */
  template <typename SerDe> void serde(SerDe& sd) {
    static_assert(!SerDe::deserializing);
    sd(name());
  }

  template <typename SerDe> static Class makeForSerde(SerDe& sd) {
    SString n;
    sd(n);
    assertx(n);
    return Class{n};
  }
private:
  explicit Class(Either<SString,ClassInfo*> val) : val{val} {}

  template <typename F>
  static void visitEverySub(folly::Range<const Class*>, bool, const F&);
  static ClassInfo* commonAncestor(ClassInfo*, ClassInfo*);
  static TinyVector<Class, 2> canonicalizeIsects(const TinyVector<Class, 8>&, bool);
private:
  friend std::string show(const Class&);
  friend struct ::HPHP::HHBBC::Index;
  friend struct ::HPHP::HHBBC::PublicSPropMutations;
  friend struct ::HPHP::HHBBC::ClassInfo;
  friend struct ::HPHP::HHBBC::UnresolvedClassMaker;
  Either<SString,ClassInfo*> val;
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

  /*
   * Coeffects
   */
  const RuntimeCoeffects* requiredCoeffects() const;
  // Returns nullptr if we cant tell whether there are coeffect rules
  const CompactVector<CoeffectRule>* coeffectRules() const;

  struct FuncInfo;
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
  struct Method {
    const php::Func* func;
  };
  // Like Method, but the method is not guaranteed to actually exist
  // (this only matters for things like exactFunc()).
  struct MethodOrMissing {
    const php::Func* func;
  };
  // Method/Func is known to not exist
  struct Missing {
    SString name;
  };
  // Group of methods (a wrapper around a FuncFamily).
  struct MethodFamily {
    FuncFamily* family;
    bool regularOnly;
  };
  // Simultaneously a group of func families. Any data must be
  // intersected across all of the func families in the list. Used for
  // method resolution on a DCls where isIsect() is true.
  struct Isect {
    CompactVector<FuncFamily*> families;
    bool regularOnly{false};
  };
  using Rep = boost::variant< FuncName
                            , MethodName
                            , FuncInfo*
                            , Method
                            , MethodFamily
                            , MethodOrMissing
                            , Missing
                            , Isect
                            >;

private:
  explicit Func(Rep);
  friend std::string show(const Func&);

private:
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
 * A type which is an alias to another. This includes standard
 * type-aliases, but also enums (which are aliases of their underlying
 * base type). Type mappings can alias to another type mapping.
 */
struct TypeMapping {
  LSString name;
  LSString value;
  // If an enum, this is the same value as name. Otherwise it's the
  // first enum encountered when resolving a type-alias.
  LSString firstEnum;
  AnnotType type;
  bool nullable;
  template <typename SerDe> void serde(SerDe& sd) {
    sd(name)(value)(firstEnum)(type)(nullable);
  }
};

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
  // The input used to build the Index is largely the extern-worker
  // refs representing the program components. However, some
  // additional metadata is needed locally to know what the refs
  // represent (and schedule some initial jobs).
  struct Input {
    template <typename T> using R = extern_worker::Ref<std::unique_ptr<T>>;

    struct ClassMeta {
      R<php::Class> cls;
      LSString name;
      std::vector<SString> dependencies;
      LSString closureContext;
      bool isClosure;
      // If this class is an enum, the type-mapping representing it's
      // base type.
      Optional<TypeMapping> typeMapping;
      std::vector<SString> unresolvedTypes;
    };

    struct FuncMeta {
      R<php::Func> func;
      LSString name;
      LSString methCallerUnit; // nullptr if not MethCaller
      std::vector<SString> unresolvedTypes;
    };

    struct UnitMeta {
      R<php::Unit> unit;
      LSString name;
      std::vector<TypeMapping> typeMappings;
    };

    static std::vector<SString> makeDeps(const php::Class&);

    std::vector<ClassMeta> classes;
    std::vector<UnitMeta> units;
    std::vector<FuncMeta> funcs;
  };

  /*
   * Create an Index for a php::Program.  Performs some initial
   * analysis of the program.
   */
  Index(Input,
        Config,
        std::unique_ptr<coro::TicketExecutor>,
        std::unique_ptr<extern_worker::Client>,
        DisposeCallback,
        StructuredLogEntry*);
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
   * Access the StructuredLogEntry that the Index is using (if any).
   */
  StructuredLogEntry* sample() const;

  /*
   * Access the php::Program this Index is analyzing.
   */
  const php::Program& program() const;

  /*
   * Obtain a pointer to the unit which defined the given func.
   */
  const php::Unit* lookup_func_unit(const php::Func&) const;
  const php::Unit* lookup_func_original_unit(const php::Func&) const;

  /*
   * Obtain a pointer to the unit which defined the given class.
   */
  const php::Unit* lookup_class_unit(const php::Class&) const;

  /*
   * Obtain a pointer to the class which defines the given class
   * constant.
   */
  const php::Class* lookup_const_class(const php::Const&) const;

  /*
   * Obtain a pointer to the class which serves as the context for the
   * given class. For non-closures, this is just the input, but may be
   * different in closures.
   */
  const php::Class* lookup_closure_context(const php::Class&) const;

  /*
   * Call the given callback for each (top-level) func defined in the
   * given Unit.
   */
  void for_each_unit_func(const php::Unit&,
                          std::function<void(const php::Func&)>) const;
  void for_each_unit_func_mutable(php::Unit&,
                                  std::function<void(php::Func&)>);

  /*
   * Call the given callback for each class defined in the given Unit.
   */
  void for_each_unit_class(const php::Unit&,
                           std::function<void(const php::Class&)>) const;
  void for_each_unit_class_mutable(php::Unit&,
                                   std::function<void(php::Class&)>);

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
   * Find a res::Class for a given php::Class.
   *
   * Returns std::nullopt if the given php::Class is not actually
   * definable.
   */
  Optional<res::Class> resolve_class(const php::Class*) const;

  /*
   * Resolve the given class name to a res::Class.
   *
   * Returns std::nullopt if no such class with that name exists, or
   * if the class is not definable.
   */
  Optional<res::Class> resolve_class(SString name) const;

  /*
   * Resolve the given class name to a name-only res::Class. This is
   * meant for use in tests.
   */
  res::Class resolve_class_name_only(SString name) const;

  /*
   * Find a type-alias with the given name. If a nullptr is returned,
   * then no type-alias exists with that name.
   */
  const php::TypeAlias* lookup_type_alias(SString name) const;

  /*
   * Try to resolve self/parent types for the given context
   */
  Optional<res::Class> selfCls(const Context& ctx) const;
  Optional<res::Class> parentCls(const Context& ctx) const;

  /*
   * Resolve a closure class.
   *
   * Returns both a resolved Class, and the actual php::Class for the
   * closure.
   */
  std::pair<res::Class, const php::Class*>
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
   * Try to resolve a method named `name' with a this type of
   * `thisType' within a given Context.
   *
   * The type of `thisType' determines if the method is meant to be
   * static or not. A this type of BCls is a static method and a BObj
   * is for an instance method.
   *
   * Note: a resolved method does not imply the function is actually
   * callable at runtime. You still need to apply visibility checks,
   * etc.
   *
   * Pre: thisType.subtypeOf(BCls) || thisType.subtypeOf(BObj)
   */
  res::Func resolve_method(Context, const Type& thisType, SString name) const;

  /*
   * Resolve a class constructor for the supplied object type.
   *
   * Pre: obj.subtypeOf(BObj)
   */
  res::Func resolve_ctor(const Type& obj) const;

  /*
   * Return a resolved class representing the base class of a wait
   * handle (this will be a sub-class of Awaitable).
   */
  res::Class wait_handle_class() const;

  /*
   * Resolve a type-constraint into its equivalent set of HHBBC types.
   *
   * In general, a type-constraint cannot be represented exactly by a
   * single HHBBC type, so a lower and upper bound is provided
   * instead.
   *
   * A "candidate" type can be provided which will be applied to the
   * type-constraint and can further constrain the output types. This
   * is useful for the magic interfaces, whose lower-bound cannot be
   * precisely represented by a single type.
   */
  template <typename T = Type>
  struct ConstraintType {
    // Lower bound of constraint. Any type which is a subtype of this
    // is guaranteed to pass a type-check without any side-effects.
    T lower;
    // Upper bound of constraint. Any type which does not intersect
    // with this is guaranteed to always fail a type-check.
    T upper;
    // If this type-constraint might promote a "classish" type to a
    // static string as a side-effect.
    TriBool coerceClassToString{TriBool::No};
    // Whether this type-constraint might map to a mixed
    // type-hint. The mixed type-hint has special semantics when it
    // comes to properties.
    bool maybeMixed{false};
  };

  ConstraintType<>
  lookup_constraint(const Context&, const TypeConstraint&,
                    const Type& candidate = TCell) const;

  /*
   * Returns true if the type constraint can contain a reified type
   * Currently, only classes and interfaces are supported
   */
  bool could_have_reified_type(Context ctx, const TypeConstraint& tc) const;

  /*
   * Returns a tuple containing a type after the parameter type
   * verification, a flag indicating whether the verification is a
   * no-op (because it always passes without any conversion), and a
   * flag indicating whether the verification is effect free (the
   * verification could convert a type without causing a side-effect).
   */
  std::tuple<Type, bool, bool>
  verify_param_type(const Context& ctx, uint32_t paramId, const Type& t) const;

  /*
   * Lookup metadata about the constant access `cls'::`name', in the
   * current context `ctx'. The returned metadata not only includes
   * the best known type of the constant, but whether it is definitely
   * found, and whether accessing the constant might throw.  This
   * function is responsible for walking the class hierarchy to find
   * all possible constants and combining the results.  This is
   * intended to be the source of truth about constants during
   * analysis.
   *
   * This function only looks up non-type, non-context constants.
   */
  ClsConstLookupResult<>
  lookup_class_constant(Context ctx, const Type& cls, const Type& name) const;

  /*
   * Lookup metadata about the constant access `cls'::`name', where
   * that constant is meant to be a type-constant. The returned
   * metadata includes the best known type of the resolved
   * type-structure, whether it was found, and whether it was
   * abstract. This is intended to be the source of truth about
   * type-constants during analysis. The returned type-structure type
   * will always be static.
   *
   * By default, lookup_class_type_constant calls
   * resolve_type_structure to resolve any found type-structure. This
   * behavior can be overridden by providing a customer resolver.
   */
  using ClsTypeConstLookupResolver =
    std::function<TypeStructureResolution(const php::Const&,const php::Class&)>;

  ClsTypeConstLookupResult<>
  lookup_class_type_constant(
    const Type& cls,
    const Type& name,
    const ClsTypeConstLookupResolver& resolver = {}) const;

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
                                   const CallContext& calleeCtx) const;

  /*
   * Return the best known return type for a resolved function, in a
   * context insensitive way.  Returns TInitCell at worst.
   */
  Type lookup_return_type(Context, MethodsInfo*, res::Func,
                          Dep dep = Dep::ReturnTy) const;
  Type lookup_return_type(Context, MethodsInfo*, const php::Func*,
                          Dep dep = Dep::ReturnTy) const;

  /*
   * Return the best known return type for a resolved function, given
   * the supplied calling context.  Returns TInitCell at worst.
   *
   * During analyze phases, this function may re-enter analyze in
   * order to interpret the callee with these argument types.
   */
  Type lookup_return_type(Context caller,
                          MethodsInfo*,
                          const CompactVector<Type>& args,
                          const Type& context,
                          res::Func,
                          Dep dep = Dep::ReturnTy) const;

  /*
   * Look up raw return type information for an unresolved
   * function. This is the best known return type, and the number of
   * refinements done to that type.
   *
   * This function does not register a dependency on the return type
   * information.
   *
   * Nothing may be writing to the index when this function is used,
   * but concurrent readers are allowed.
   */
  std::pair<Type, size_t> lookup_return_type_raw(const php::Func*) const;

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
  Optional<uint32_t> lookup_num_inout_params(Context, res::Func) const;

  /*
   * Returns whether the function's return value is readonly
   */
  TriBool lookup_return_readonly(Context, res::Func) const;

  /*
   * Returns whether the function is marked as readonly
   */
  TriBool lookup_readonly_this(Context, res::Func) const;

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
  Type lookup_public_prop(const Type& obj, const Type& name) const;
  Type lookup_public_prop(const php::Class* cls, SString name) const;

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
   * Initialize the initial "may have bad initial value" bit for
   * properties. By initially setting this before analysis, we save
   * redundant re-analyzes.
   */
  void preinit_bad_initial_prop_values();

  /*
   * Attempt to pre-resolve as many type-structures as possible in
   * type-constants and type-aliases.
   */
  void preresolve_type_structures();

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
    const CompactVector<std::pair<size_t, Type>>& resolved,
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
   * Rewrite the initial values of any AttrSystemInitialValue properties to
   * something more suitable for its type-hint, and add AttrNoImplicitNullable
   * where appropriate.
   *
   * This must be done before any analysis is done, as the initial values
   * affects the analysis.
   */
  void rewrite_default_initial_values() const;

  /*
   * Return true if the resolved function supports async eager return.
   */
  TriBool supports_async_eager_return(res::Func rfunc) const;

  /*
   * Return true if the function is effect free.
   */
  bool is_effect_free(Context, res::Func rfunc) const;
  bool is_effect_free(Context, const php::Func* func) const;
  bool is_effect_free_raw(const php::Func* func) const;

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

  template <typename F>
  bool visit_every_dcls_cls(const DCls&, const F&) const;

  template <typename P, typename G>
  static res::Func rfunc_from_dcls(const DCls&, SString, const P&, const G&);

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

}
