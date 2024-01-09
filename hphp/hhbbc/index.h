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
#include "hphp/hhbbc/type-structure.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

struct Index;
struct AnalysisIndex;
struct PublicSPropMutations;
struct FuncAnalysisResult;
struct Context;
struct ContextHash;
struct CallContext;
struct PropertiesInfo;
struct MethodsInfo;

namespace php {
struct Class;
struct ClassBytecode;
struct Prop;
struct Const;
struct Func;
struct FuncBytecode;
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
struct PropStateElem {
  Type ty;
  const TypeConstraint* tc = nullptr;
  Attr attrs;
  bool everModified;

  bool operator==(const PropStateElem& o) const {
    return
      ty == o.ty &&
      tc == o.tc &&
      attrs == o.attrs &&
      everModified == o.everModified;
  }
};
using PropState = std::map<LSString,PropStateElem>;

/*
 * The result of Index::lookup_static
 */
struct PropLookupResult {
  Type ty; // The best known type of the property (TBottom if not found)
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

inline PropLookupResult& operator|=(PropLookupResult& a,
                                    const PropLookupResult& b) {
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

std::string show(const PropLookupResult&);

/*
 * The result of Index::merge_static_type
 */
struct PropMergeResult {
  Type adjusted; // The merged type, potentially adjusted according to
                 // the prop's type-constraint (it's the subtype of
                 // the merged type that would succeed).
  TriBool throws; // Whether the mutation this merge represents
                  // can throw.
};

inline PropMergeResult& operator|=(PropMergeResult& a,
                                   const PropMergeResult& b) {
  a.adjusted |= b.adjusted;
  a.throws |= b.throws;
  return a;
}

std::string show(const PropMergeResult&);

/*
 * The result of Index::lookup_class_constant
 */
struct ClsConstLookupResult {
  Type ty;            // The best known type of the constant (might not be a
                      // scalar).
  TriBool found;      // If the constant was found
  bool mightThrow;    // If accessing the constant can throw
};

inline ClsConstLookupResult& operator|=(ClsConstLookupResult& a,
                                        const ClsConstLookupResult& b) {
  a.ty |= b.ty;
  a.found |= b.found;
  a.mightThrow |= b.mightThrow;
  return a;
}

std::string show(const ClsConstLookupResult&);

/*
 * The result of Index::lookup_class_type_constant
 */
struct ClsTypeConstLookupResult {
  TypeStructureResolution resolution; // The result from resolving
                                      // the type-structure
  TriBool found;    // If the constant was found
  TriBool abstract; // If the constant was abstract (this only applies
                    // to the subset which wasn't found).
};

inline ClsTypeConstLookupResult& operator|=(
    ClsTypeConstLookupResult& a,
    const ClsTypeConstLookupResult& b) {
  a.resolution |= b.resolution;
  if (a.found == TriBool::Yes) {
    a.abstract = b.abstract;
  } else if (b.found != TriBool::Yes) {
    a.abstract |= b.abstract;
  }
  a.found |= b.found;
  return a;
}

std::string show(const ClsTypeConstLookupResult&);

//////////////////////////////////////////////////////////////////////

/*
 * Represents a class constant, pointing to where the constant was
 * originally declared (the class name and it's position in the
 * class' constant table).
 */
struct ConstIndex {
  using Idx = uint32_t;

  ConstIndex() = default;
  ConstIndex(SString cls, Idx idx)
    : cls{cls}, idx{idx} {}

  SString cls;
  Idx idx;

  bool operator==(const ConstIndex& o) const {
    return idx == o.idx && cls->tsame(o.cls);
  }
  bool operator<(const ConstIndex& o) const {
    if (idx != o.idx) return idx < o.idx;
    return string_data_lt_type{}(cls, o.cls);
  }

  struct Hasher {
    size_t operator()(const ConstIndex& idx) const {
      return folly::hash::hash_combine(idx.cls->hash(), idx.idx);
    }
  };

  template <typename SerDe> void serde(SerDe& sd) {
    sd(cls)(idx);
  }
};

std::string show(const ConstIndex&);

//////////////////////////////////////////////////////////////////////

// Inferred class constant type from a 86cinit.
struct ClsConstInfo {
  Type type;
  size_t refinements = 0;
  template <typename SerDe> void serde(SerDe& sd) {
    sd(type)(refinements);
  }
};

using ResolvedConstants =
  CompactVector<std::pair<ConstIndex::Idx, ClsConstInfo>>;

//////////////////////////////////////////////////////////////////////
/*
 * Represents a method, without requiring an explicit pointer to a
 * php::Func (so can be used across remote workers).
 */
struct MethRef {
  MethRef() = default;
  explicit MethRef(const php::Func& f)
    : cls{f.cls->name}, idx{f.clsIdx} {}
  MethRef(SString cls, uint32_t idx)
    : cls{cls}, idx{idx} {}

  SString cls{nullptr};
  // Index in the class' methods table.
  uint32_t idx{std::numeric_limits<uint32_t>::max()};

  bool operator==(const MethRef& o) const {
    return cls->tsame(o.cls) && idx == o.idx;
  }
  bool operator!=(const MethRef& o) const {
    return !(*this == o);
  }
  bool operator<(const MethRef& o) const {
    // The ordering for MethRef is arbitrary. Compare by idx and then
    // by the class name's hash to avoid having to do the more
    // expensive string comparison.
    if (idx != o.idx) return idx < o.idx;
    auto const hash1 = cls->hash();
    auto const hash2 = o.cls->hash();
    if (hash1 != hash2) return hash1 < hash2;
    return string_data_lt_type{}(cls, o.cls);
  }

  struct Hash {
    size_t operator()(const MethRef& m) const {
      return folly::hash::hash_combine(m.cls->hash(), m.idx);
    }
  };

  template <typename SerDe> void serde(SerDe& sd) {
    sd(cls)(idx);
  }
};

using MethRefSet = hphp_fast_set<MethRef, MethRef::Hash>;

std::string show(const MethRef&);

//////////////////////////////////////////////////////////////////////

struct PropInitInfo {
  TypedValue val;
  bool satisfies;
  bool deepInit;
};
using ResolvedPropInits = CompactVector<std::pair<size_t, PropInitInfo>>;

//////////////////////////////////////////////////////////////////////

// private types
struct ClassGraph;
struct ClassInfo;
struct ClassInfo2;
struct FuncInfo2;
struct FuncFamily2;
struct MethodsWithoutCInfo;

//////////////////////////////////////////////////////////////////////

// Opaque input to an AnalysisIndex. Avoids exposing AnalysisIndex
// internals to those who need to convey the data into a remote
// worker.
template <typename T> struct AnalysisIndexParam {
  AnalysisIndexParam() = default;

  void serde(BlobEncoder&) const;
  void serde(BlobDecoder&);
private:
  struct Deleter { void operator()(T*) const; };
  std::unique_ptr<T, Deleter> ptr;

  friend struct ::HPHP::HHBBC::AnalysisIndex;
};

using AnalysisIndexCInfo = AnalysisIndexParam<ClassInfo2>;
using AnalysisIndexFInfo = AnalysisIndexParam<FuncInfo2>;
using AnalysisIndexMInfo = AnalysisIndexParam<MethodsWithoutCInfo>;

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
  bool exactSubtypeOfExact(const Class& o,
                           bool nonRegularL,
                           bool nonRegularR) const;
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
  bool exactCouldBeExact(const Class& o,
                         bool nonRegularL,
                         bool nonRegularR) const;
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
   * or std::nullopt.
   */
  Optional<Class> parent() const;

  /*
   * Returns the php::Class for this Class if there is one, or
   * nullptr.
   */
  const php::Class* cls() const;

  bool hasCompleteChildren() const;
  bool isComplete() const;

  /*
   * Invoke the given function on every possible subclass of this
   * class (including itself), providing the name and the Attr bits of
   * the subclass. Only the Attr bits corresponding to the type of
   * class are provided (AttrEnum/AttrTrait/AttrInterface). Returns
   * false and doesn't call the function if this class does not know
   * it's complete children, true otherwise.
   */
  bool forEachSubclass(const std::function<void(SString, Attr)>&) const;

  using ClassVec = TinyVector<Class, 4>;

  /*
   * Given two lists of classes, calculate the union between them (in
   * canonical form). A list of size 1 represents a single class, and
   * a larger list represents an intersection of classes. The input
   * lists are assumed to be in canonical form. If the output is an
   * empty list, the union is *all* classes (corresponding to TObj or
   * TCls). This function is really an implementation detail of
   * union_of() and not a general purpose interface.
   */
  static ClassVec combine(folly::Range<const Class*> classes1,
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
  static ClassVec intersect(folly::Range<const Class*> classes1,
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
  static ClassVec removeNonRegular(folly::Range<const Class*> classes);

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
   * Convert this class to/from an opaque integer. The integer is
   * "pointerish" (has upper bits cleared), so can be used in
   * something like CompactTaggedPtr. It is not, however, guaranteed
   * to be aligned (lower bits may be set).
   */
  uintptr_t toOpaque() const { return (uintptr_t)opaque.p; }
  static Class fromOpaque(uintptr_t o) { return Class{Opaque{(void*)o}}; }

  size_t hash() const { return toOpaque(); }

  /*
   * Obtain with a given name or associated with the given ClassInfo
   * (must already exist).
   */
  static Class get(SString);
  static Class get(const ClassInfo&);
  static Class get(const ClassInfo2&);

  /*
   * Obtain with a given name. If does not already exist, create an
   * unresolved Class and return that.
   */
  static Class getOrCreate(SString);

  /*
   * Make an unresolved Class representing the given name. The name
   * cannot be an existing class.
   */
  static Class getUnresolved(SString);

  void serde(BlobEncoder&) const;
  static Class makeForSerde(BlobDecoder&);

private:
  ClassGraph graph() const;
  ClassInfo* cinfo() const;
  ClassInfo2* cinfo2() const;

  template <typename F>
  static void visitEverySub(folly::Range<const Class*>, bool, const F&);

  friend std::string show(const Class&);

  friend struct ::HPHP::HHBBC::Index;
  friend struct ::HPHP::HHBBC::AnalysisIndex;

  struct Opaque { void* p; };
  Opaque opaque;

  explicit Class(ClassGraph);
  explicit Class(Opaque o): opaque{o} {}
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
   * Returns the (fully qualified) name of this function.
   */
  std::string name() const;

  /*
   * If this resolved function represents exactly one php::Func, return it.
   */
  const php::Func* exactFunc() const;

  /*
   * Whether this function definitely exists or definitely does not
   * exist.
   */
  TriBool exists() const;

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
   * Return if the function supports async eager return.
   */
  TriBool supportsAsyncEagerReturn() const;

  /*
   * Returns the number of inout parameters expected by func (if known).
   */
  Optional<uint32_t> lookupNumInoutParams() const;

  /*
   * Returns the parameter preparation kind (if known) for parameter
   * `paramId' on this function.
   */
  PrepKind lookupParamPrep(uint32_t paramId) const;

  /*
   * Returns whether the function's return value is readonly
   */
  TriBool lookupReturnReadonly() const;

  /*
   * Returns whether the function is marked as readonly
   */
  TriBool lookupReadonlyThis() const;

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
  friend struct ::HPHP::HHBBC::AnalysisIndex;

  struct FuncName {
    SString name;
  };
  struct MethodName {
    SString cls;
    SString name;
  };
  struct Fun {
    const FuncInfo* finfo;
  };
  struct Fun2 {
    const FuncInfo2* finfo;
  };
  struct Method {
    const FuncInfo* finfo;
  };
  struct Method2 {
    const FuncInfo2* finfo;
  };
  // Like Method, but the method is not guaranteed to actually exist
  // (this only matters for things like exactFunc()).
  struct MethodOrMissing {
    const FuncInfo* finfo;
  };
  struct MethodOrMissing2 {
    const FuncInfo2* finfo;
  };
  // Method/Func is known to not exist
  struct MissingFunc {
    SString name;
  };
  struct MissingMethod {
    SString cls;
    SString name;
  };
  // Group of methods (a wrapper around a FuncFamily).
  struct MethodFamily {
    FuncFamily* family;
    bool regularOnly;
  };
  struct MethodFamily2 {
    const FuncFamily2* family;
    bool regularOnly;
  };
  // Simultaneously a group of func families. Any data must be
  // intersected across all of the func families in the list. Used for
  // method resolution on a DCls where isIsect() is true.
  struct Isect {
    CompactVector<FuncFamily*> families;
    bool regularOnly{false};
  };
  struct Isect2 {
    CompactVector<const FuncFamily2*> families;
    bool regularOnly{false};
  };
  using Rep = boost::variant< FuncName
                            , MethodName
                            , Fun
                            , Fun2
                            , Method
                            , Method2
                            , MethodFamily
                            , MethodFamily2
                            , MethodOrMissing
                            , MethodOrMissing2
                            , MissingFunc
                            , MissingMethod
                            , Isect
                            , Isect2
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
  // If an enum, this is the same value as name. Otherwise it's the
  // first enum encountered when resolving a type-alias.
  LSString firstEnum;
  TypeConstraint value;
  bool isTypeAlias;

  bool operator==(const TypeMapping& o) const {
    return name->tsame(o.name);
  }
  bool operator<(const TypeMapping& o) const {
    return string_data_lt_type{}(name, o.name);
  }

  template <typename SerDe> void serde(SerDe& sd) {
    sd(name)(firstEnum)(value)(isTypeAlias);
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
      LSString unit;
      bool isClosure;
      // Whether this closure was declared inside a class or func
      // (which determines the meaning of closureContext).
      bool closureDeclInFunc;
      bool has86init;
      // If this class is an enum, the type-mapping representing it's
      // base type.
      Optional<TypeMapping> typeMapping;
      std::vector<SString> unresolvedTypes;
    };

    struct FuncMeta {
      R<php::Func> func;
      LSString name;
      LSString unit;
      bool methCaller;
      std::vector<SString> unresolvedTypes;
    };

    struct UnitMeta {
      R<php::Unit> unit;
      LSString name;
      std::vector<TypeMapping> typeMappings;
      std::vector<std::pair<SString, bool>> constants;
    };

    struct FuncBytecodeMeta {
      R<php::FuncBytecode> bc;
      LSString name;
      LSString unit;
      bool methCaller;
    };

    struct ClassBytecodeMeta {
      R<php::ClassBytecode> bc;
      LSString name;
    };

    static std::vector<SString> makeDeps(const php::Class&);

    std::vector<ClassMeta> classes;
    std::vector<UnitMeta> units;
    std::vector<FuncMeta> funcs;
    std::vector<ClassBytecodeMeta> classBC;
    std::vector<FuncBytecodeMeta> funcBC;
  };

  /*
   * Create an Index for a php::Program.  Performs some initial
   * analysis of the program.
   */
  Index(Input,
        Config,
        std::unique_ptr<TicketExecutor>,
        std::unique_ptr<extern_worker::Client>,
        DisposeCallback,
        StructuredLogEntry*);
  ~Index();

  Index(const Index&) = delete;
  Index(Index&&);
  Index& operator=(const Index&) = delete;
  Index& operator=(Index&&);

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
   * Prepare the index for local execution. This marks the boundary
   * between running in extern-worker and running in the "classic"
   * way.
   */
  void make_local();

  /*
   * Access the StructuredLogEntry that the Index is using (if any).
   */
  StructuredLogEntry* sample() const;

  /*
   * Obtain the extern-worker related state that the Index used.
   */
  TicketExecutor& executor() const;
  extern_worker::Client& client() const;
  const CoroAsyncValue<extern_worker::Ref<Config>>& configRef() const;

  /*
   * The names of all classes which has a 86*init function.
   */
  const TSStringSet& classes_with_86inits() const;

  /*
   * The names of all top-level functions which are initializers for
   * "dynamic" constants.
   */
  const FSStringSet& constant_init_funcs() const;

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
   * Look up the php::Class with the given name, returning nullptr if
   * it does not exist. The presence of a php::Class does not
   * necessarily mean the class is actually instantiable or won't
   * fatal when referenced.
   */
  const php::Class* lookup_class(SString) const;

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
   * Resolve the given class name to a res::Class.
   *
   * Returns std::nullopt if no such class with that name exists, or
   * if the class is not definable.
   */
  Optional<res::Class> resolve_class(SString name) const;
  Optional<res::Class> resolve_class(const php::Class&) const;

  /*
   * Find a type-alias with the given name. If a nullptr is returned,
   * then no type-alias exists with that name.
   */
  const php::TypeAlias* lookup_type_alias(SString name) const;

  /*
   * Find a class or a type-alias with the given name. This can be
   * more efficient than doing two different lookups.
   */
  struct ClassOrTypeAlias {
    // At most one of these will be non-null.
    const php::Class* cls;
    const php::TypeAlias* typeAlias;
    bool maybeExists;
  };
  ClassOrTypeAlias lookup_class_or_type_alias(SString name) const;

  /*
   * Resolve the given php::Func, which can be a function or
   * method. resolved() is guaranteed to be true for the returned
   * res::Func.
   */
  res::Func resolve_func_or_method(const php::Func&) const;

  /*
   * Try to resolve a function named `name'.
   */
  res::Func resolve_func(SString name) const;

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
  ClsConstLookupResult
  lookup_class_constant(Context ctx, const Type& cls, const Type& name) const;

  /*
   * Retrieve the information the Index knows about all of the class
   * constants defined on the given class. This does not register any
   * dependency.
   */
  std::vector<std::pair<SString, ClsConstInfo>>
  lookup_class_constants(const php::Class&) const;

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

  ClsTypeConstLookupResult
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
  bool func_depends_on_arg(const php::Func* func, size_t arg) const;

  /*
   * Return type knowledge. The type and whether it's effect-free.
   */
  struct ReturnType {
    Type t;
    bool effectFree{false};
  };

  /*
   * If func is effect-free when called with args, and it returns a constant,
   * return that constant; otherwise return TInitCell.
   */
  ReturnType lookup_foldable_return_type(Context ctx,
                                         const CallContext& calleeCtx) const;

  /*
   * Return the best known return type for a resolved function, in a
   * context insensitive way.  Returns TInitCell at worst.
   */
  ReturnType lookup_return_type(Context, MethodsInfo*, res::Func,
                                Dep dep = Dep::ReturnTy) const;

  /*
   * Return the best known return type for a resolved function and
   * whether it is effect-free, given the supplied calling context.
   * Returns TInitCell at worst.
   *
   * During analyze phases, this function may re-enter analyze in
   * order to interpret the callee with these argument types.
   */
  ReturnType lookup_return_type(Context caller,
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
   * information, so should not be used during analysis.
   *
   * Nothing may be writing to the index when this function is used,
   * but concurrent readers are allowed.
   */
  std::pair<ReturnType, size_t> lookup_return_type_raw(const php::Func*) const;

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
  PropLookupResult lookup_static(Context ctx,
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
  bool using_class_dependencies() const;

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
  PropMergeResult merge_static_type(Context ctx,
                                    PublicSPropMutations& publicMutations,
                                    PropertiesInfo& privateProps,
                                    const Type& cls,
                                    const Type& name,
                                    const Type& val,
                                    bool checkUB = false,
                                    bool ignoreConst = false,
                                    bool mustBeReadOnly = false) const;

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
  void refine_class_constants(const Context& ctx,
                              const ResolvedConstants& resolved,
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
   * Update the initial values for properties inferred from
   * 86[p/s]init functions during analysis. The modification of the
   * relevant php::Prop instances must be done here when we know
   * nobody else is reading them.
   */
  void update_prop_initial_values(const Context&,
                                  const ResolvedPropInits&,
                                  DependencyContextSet&);

  struct IndexData;

private:
  friend struct AnalysisScheduler;
  friend struct PublicSPropMutations;

  template <typename F>
  bool visit_every_dcls_cls(const DCls&, const F&) const;

  template <typename P, typename G>
  res::Func rfunc_from_dcls(const DCls&, SString, const P&, const G&) const;

private:
  std::unique_ptr<IndexData> m_data;
};

//////////////////////////////////////////////////////////////////////

// Abstracts away particular Index implementation from the analysis
// logic.
struct IIndex {
  IIndex() = default;
  IIndex(const IIndex&) = delete;
  IIndex(IIndex&&) = delete;
  IIndex& operator=(const IIndex&) = delete;
  IIndex& operator=(IIndex&&) = delete;

  virtual const php::Unit* lookup_func_unit(const php::Func&) const = 0;

  virtual const php::Unit* lookup_class_unit(const php::Class&) const = 0;

  virtual const php::Class* lookup_const_class(const php::Const&) const = 0;

  virtual const php::Class* lookup_closure_context(const php::Class&) const = 0;

  virtual const CompactVector<const php::Class*>*
  lookup_closures(const php::Class*) const = 0;

  virtual const hphp_fast_set<const php::Func*>*
  lookup_extra_methods(const php::Class*) const = 0;

  virtual Optional<res::Class> resolve_class(SString) const = 0;
  virtual Optional<res::Class> resolve_class(const php::Class&) const = 0;

  virtual std::pair<const php::TypeAlias*, bool>
  lookup_type_alias(SString) const = 0;

  virtual Index::ClassOrTypeAlias lookup_class_or_type_alias(SString) const = 0;

  virtual res::Func resolve_func_or_method(const php::Func&) const = 0;

  virtual res::Func resolve_func(SString) const = 0;

  virtual res::Func resolve_method(Context,
                                   const Type& thisType,
                                   SString name) const = 0;

  virtual res::Func resolve_ctor(const Type& obj) const = 0;

  virtual std::vector<std::pair<SString, ClsConstInfo>>
  lookup_class_constants(const php::Class&) const = 0;

  virtual ClsConstLookupResult
  lookup_class_constant(Context, const Type& cls, const Type& name) const = 0;

  virtual ClsTypeConstLookupResult lookup_class_type_constant(
      const Type& cls,
      const Type& name,
      const Index::ClsTypeConstLookupResolver& resolver = {}
  ) const = 0;

  virtual Type lookup_constant(Context, SString) const = 0;

  virtual bool func_depends_on_arg(const php::Func* func, size_t arg) const = 0;

  virtual Index::ReturnType
  lookup_foldable_return_type(Context, const CallContext&) const = 0;

  virtual Index::ReturnType
  lookup_return_type(Context, MethodsInfo*, res::Func,
                     Dep dep = Dep::ReturnTy) const = 0;

  virtual Index::ReturnType
  lookup_return_type(Context caller,
                     MethodsInfo*,
                     const CompactVector<Type>& args,
                     const Type& context,
                     res::Func,
                     Dep dep = Dep::ReturnTy) const = 0;

  virtual std::pair<Index::ReturnType, size_t>
  lookup_return_type_raw(const php::Func*) const = 0;

  virtual CompactVector<Type>
  lookup_closure_use_vars(const php::Func*,
                          bool move = false) const = 0;

  virtual PropState lookup_private_props(const php::Class*,
                                         bool move = false) const = 0;

  virtual PropState lookup_private_statics(const php::Class*,
                                           bool move = false) const = 0;

  virtual PropLookupResult lookup_static(Context,
                                         const PropertiesInfo& privateProps,
                                         const Type& cls,
                                         const Type& name) const = 0;

  virtual Type lookup_public_prop(const Type& obj, const Type& name) const = 0;

  virtual PropMergeResult
  merge_static_type(Context ctx,
                    PublicSPropMutations& publicMutations,
                    PropertiesInfo& privateProps,
                    const Type& cls,
                    const Type& name,
                    const Type& val,
                    bool checkUB = false,
                    bool ignoreConst = false,
                    bool mustBeReadOnly = false) const = 0;

  virtual bool using_class_dependencies() const = 0;
private:
  virtual void push_context(const Context&) const = 0;
  virtual void pop_context() const = 0;

  friend struct ContextPusher;
};

//////////////////////////////////////////////////////////////////////

// Push the given Context onto the Index's stack of Contexts, and
// remove it automatically.
struct ContextPusher {
  ContextPusher(const IIndex& index, const Context& ctx) : index{index} {
    index.push_context(ctx);
  }
  ~ContextPusher() { index.pop_context(); }
  const IIndex& index;
};

//////////////////////////////////////////////////////////////////////

struct IndexAdaptor : public IIndex {
  explicit IndexAdaptor(const Index& index)
   : index{const_cast<Index&>(index)} {}

  const php::Unit* lookup_func_unit(const php::Func& f) const override {
    return index.lookup_func_unit(f);
  }
  const php::Unit* lookup_class_unit(const php::Class& c) const override {
    return index.lookup_class_unit(c);
  }
  const php::Class* lookup_const_class(const php::Const& c) const override {
    return index.lookup_const_class(c);
  }
  const php::Class* lookup_closure_context(const php::Class& c) const override {
    return index.lookup_closure_context(c);
  }
  const CompactVector<const php::Class*>*
  lookup_closures(const php::Class* c) const override {
    return index.lookup_closures(c);
  }
  const hphp_fast_set<const php::Func*>*
  lookup_extra_methods(const php::Class* c) const override {
    return index.lookup_extra_methods(c);
  }
  Optional<res::Class> resolve_class(SString c) const override {
    return index.resolve_class(c);
  }
  Optional<res::Class> resolve_class(const php::Class& c) const override {
    return index.resolve_class(c);
  }
  std::pair<const php::TypeAlias*, bool>
  lookup_type_alias(SString a) const override {
    auto const ta = index.lookup_type_alias(a);
    return std::make_pair(ta, (bool)ta);
  }
  Index::ClassOrTypeAlias
  lookup_class_or_type_alias(SString name) const override {
    return index.lookup_class_or_type_alias(name);
  }
  res::Func resolve_func_or_method(const php::Func& f) const override {
    return index.resolve_func_or_method(f);
  }
  res::Func resolve_func(SString s) const override {
    return index.resolve_func(s);
  }
  res::Func resolve_method(Context c, const Type& t, SString n) const override {
    return index.resolve_method(c, t, n);
  }
  res::Func resolve_ctor(const Type& o) const override {
    return index.resolve_ctor(o);
  }
  std::vector<std::pair<SString, ClsConstInfo>>
  lookup_class_constants(const php::Class& c) const override {
    return index.lookup_class_constants(c);
  }
  ClsConstLookupResult lookup_class_constant(Context c,
                                             const Type& t,
                                             const Type& n) const override {
    return index.lookup_class_constant(c, t, n);
  }
  ClsTypeConstLookupResult
  lookup_class_type_constant(
    const Type& c,
    const Type& n,
    const Index::ClsTypeConstLookupResolver& r = {}
  ) const override {
    return index.lookup_class_type_constant(c, n, r);
  }
  Type lookup_constant(Context c, SString s) const override {
    return index.lookup_constant(c, s);
  }
  bool func_depends_on_arg(const php::Func* f, size_t p) const override {
    return index.func_depends_on_arg(f, p);
  }
  Index::ReturnType
  lookup_foldable_return_type(Context c1,
                              const CallContext& c2) const override {
    return index.lookup_foldable_return_type(c1, c2);
  }
  Index::ReturnType lookup_return_type(Context c, MethodsInfo* m, res::Func f,
                                       Dep d = Dep::ReturnTy) const override {
    return index.lookup_return_type(c, m, f, d);
  }
  Index::ReturnType lookup_return_type(Context c,
                                       MethodsInfo* m,
                                       const CompactVector<Type>& a,
                                       const Type& t,
                                       res::Func f,
                                       Dep d = Dep::ReturnTy) const override {
    return index.lookup_return_type(c, m, a, t, f, d);
  }
  std::pair<Index::ReturnType, size_t>
  lookup_return_type_raw(const php::Func* f) const override {
    return index.lookup_return_type_raw(f);
  }
  CompactVector<Type>
  lookup_closure_use_vars(const php::Func* f, bool m = false) const override {
    return index.lookup_closure_use_vars(f, m);
  }
  PropState lookup_private_props(const php::Class* c,
                                 bool m = false) const override {
    return index.lookup_private_props(c, m);
  }
  PropState lookup_private_statics(const php::Class* c,
                                   bool m = false) const override {
    return index.lookup_private_statics(c, m);
  }
  PropLookupResult lookup_static(Context c1,
                                 const PropertiesInfo& p,
                                 const Type& c2,
                                 const Type& n) const override {
    return index.lookup_static(c1, p, c2, n);
  }
  Type lookup_public_prop(const Type& o, const Type& n) const override {
    return index.lookup_public_prop(o, n);
  }
  PropMergeResult
  merge_static_type(Context ctx,
                    PublicSPropMutations& publicMutations,
                    PropertiesInfo& privateProps,
                    const Type& cls,
                    const Type& name,
                    const Type& val,
                    bool checkUB = false,
                    bool ignoreConst = false,
                    bool mustBeReadOnly = false) const override {
    return index.merge_static_type(
      ctx, publicMutations, privateProps,
      cls, name, val, checkUB, ignoreConst,
      mustBeReadOnly
    );
  }
  bool using_class_dependencies() const override {
    return index.using_class_dependencies();
  }

private:
  void push_context(const Context&) const override {}
  void pop_context() const override {}

  Index& index;
};

//////////////////////////////////////////////////////////////////////

// Represents all of the things a class or func might depend up for
// analysis. If any of these things change, the class or func will be
// scheduled to be analyzed again.
struct AnalysisDeps {
  // Some dependencies (for example, functions) can have multiple
  // "types" (IE, return-type, bytecode, etc). A class or func's
  // dependency on a function will be some union of these types.
  enum Type : uint8_t {
    None          = 0,
    // The existence of this thing and basic metadata and nothing
    // more.
    Meta          = (1 << 0),
    // Return-type of function
    RetType       = (1 << 1),
    // Return-type of function but only if it becomes a scalar
    ScalarRetType = (1 << 2),
    // Whether any parameters are returned unchanged
    RetParam      = (1 << 3),
    // Whether any parameters are unused
    UnusedParams  = (1 << 4),
    // Bytecode of class or function
    Bytecode      = (1 << 5),
  };

  // Some types can change as a result of analysis and hence can
  // trigger reschedules. The only type right now that cannot change
  // is "Meta" (since it represents the basic metadata of a class or
  // function which doesn't change).
  static constexpr Type kValidForChanges = static_cast<Type>(
    Type::RetType |
    Type::ScalarRetType |
    Type::RetParam |
    Type::UnusedParams |
    Type::Bytecode
  );

  static constexpr bool isValidForChanges(Type t) {
    return
      t != Type::None &&
      (t & kValidForChanges) == t;
  }

  // Add dependencies on various entities

  struct Class { SString name; };
  struct Func { SString name; };
  struct Constant { SString name; };
  struct TypeAlias { SString name; };

  bool add(Class);

  bool add(ConstIndex);
  bool add(Constant);
  bool add(TypeAlias);

  Type add(const php::Func&, Type);
  Type add(MethRef, Type);
  Type add(Func, Type);

  // Combine two sets of dependencies into one.
  AnalysisDeps& operator|=(const AnalysisDeps&);

  template <typename SerDe> void serde(SerDe& sd) {
    sd(funcs, string_data_lt_func{})
      (methods, std::less<>{})
      (classes, string_data_lt_type{})
      (clsConstants, std::less<>{})
      (constants, string_data_lt{})
      (typeAliases, string_data_lt_type{})
      ;
  }

private:
  FSStringToOneT<Type> funcs;
  hphp_fast_map<MethRef, Type, MethRef::Hash> methods;

  TSStringSet classes;
  hphp_fast_set<ConstIndex, ConstIndex::Hasher> clsConstants;
  SStringSet constants;
  TSStringSet typeAliases;

  static Type merge(Type&, Type);

  friend struct AnalysisScheduler;
};

// Make Type enum behave like bitset.
inline AnalysisDeps::Type operator|(AnalysisDeps::Type a,
                                    AnalysisDeps::Type b) {
  using T = std::underlying_type_t<AnalysisDeps::Type>;
  return static_cast<AnalysisDeps::Type>(
    static_cast<T>(a) | static_cast<T>(b)
  );
}
inline AnalysisDeps::Type& operator|=(AnalysisDeps::Type& a,
                                      AnalysisDeps::Type b) {
  a = a | b;
  return a;
}
inline AnalysisDeps::Type operator&(AnalysisDeps::Type a,
                                    AnalysisDeps::Type b) {
  using T = std::underlying_type_t<AnalysisDeps::Type>;
  return static_cast<AnalysisDeps::Type>(
    static_cast<T>(a) & static_cast<T>(b)
  );
}
inline AnalysisDeps::Type& operator&=(AnalysisDeps::Type& a,
                                      AnalysisDeps::Type b) {
  a = a & b;
  return a;
}
inline AnalysisDeps::Type operator-(AnalysisDeps::Type a,
                                    AnalysisDeps::Type b) {
  using T = std::underlying_type_t<AnalysisDeps::Type>;
  return static_cast<AnalysisDeps::Type>(
    static_cast<T>(a) & ~static_cast<T>(b)
  );
}

std::string show(AnalysisDeps::Type);

//////////////////////////////////////////////////////////////////////

// Represents the changes made to a class or function during
// analysis. This is the other side of AnalysisDeps. A function or
// class marking a change in its AnalysisChangeSet will trigger other
// classes or functions which have a dependency on the same type to be
// re-analyzed.
struct AnalysisChangeSet {
  using Type = AnalysisDeps::Type;

  void changed(ConstIndex);
  void changed(const php::Constant&);
  void changed(const php::Func&, Type);

  void remove(const php::Func& f) { funcs.erase(f.name); }

  template <typename SerDe> void serde(SerDe& sd) {
    sd(funcs, string_data_lt_func{})
      (methods, std::less<>{})
      (constants, string_data_lt{})
      (clsConstants, std::less<>{})
      ;
  }
private:
  FSStringToOneT<Type> funcs;
  hphp_fast_map<MethRef, Type, MethRef::Hash> methods;
  SStringSet constants;
  hphp_fast_set<ConstIndex, ConstIndex::Hasher> clsConstants;

  friend struct AnalysisScheduler;
};

//////////////////////////////////////////////////////////////////////

// Wraps up all of the inputs to a particular analysis job without
// exposing implementation details. These are produced from
// AnalysisScheduler::schedule().
struct AnalysisInput {
  std::vector<SString> classNames() const;
  std::vector<SString> funcNames() const;
  std::vector<SString> unitNames() const;

  std::vector<SString> cinfoNames() const;
  std::vector<SString> minfoNames() const;

  bool empty() const {
    return classes.empty() && funcs.empty() && units.empty();
  }
  SString key() const { return m_key; }

  struct Meta {
    TSStringSet badClasses;
    FSStringSet badFuncs;
    SStringSet badConstants;
    TSStringSet badTypeAliases;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(badClasses, string_data_lt_type{})
        (badFuncs, string_data_lt_func{})
        (badConstants, string_data_lt{})
        (badTypeAliases, string_data_lt_type{})
        ;
    }
  };
  Meta takeMeta() { return std::move(meta); }

  // Produces input for the analysis job.
  using Tuple = std::tuple<
    UniquePtrRefVec<php::Class>,
    UniquePtrRefVec<php::Func>,
    UniquePtrRefVec<php::Unit>,
    UniquePtrRefVec<php::ClassBytecode>,
    UniquePtrRefVec<php::FuncBytecode>,
    RefVec<AnalysisIndexCInfo>,
    RefVec<AnalysisIndexFInfo>,
    RefVec<AnalysisIndexMInfo>,
    UniquePtrRefVec<php::Class>,
    UniquePtrRefVec<php::Func>,
    UniquePtrRefVec<php::Unit>,
    extern_worker::Ref<Meta>
  >;
  Tuple toTuple(extern_worker::Ref<Meta>) const;
private:
  SString m_key{nullptr};

  TSStringToOneT<UniquePtrRef<php::Class>> classes;
  FSStringToOneT<UniquePtrRef<php::Func>> funcs;
  SStringToOneT<UniquePtrRef<php::Unit>> units;

  TSStringToOneT<UniquePtrRef<php::ClassBytecode>> classBC;
  FSStringToOneT<UniquePtrRef<php::FuncBytecode>> funcBC;

  TSStringToOneT<extern_worker::Ref<AnalysisIndexCInfo>> cinfos;
  FSStringToOneT<extern_worker::Ref<AnalysisIndexFInfo>> finfos;
  TSStringToOneT<extern_worker::Ref<AnalysisIndexMInfo>> minfos;

  TSStringToOneT<UniquePtrRef<php::Class>> depClasses;
  FSStringToOneT<UniquePtrRef<php::Func>> depFuncs;
  SStringToOneT<UniquePtrRef<php::Unit>> depUnits;

  Meta meta;

  friend struct AnalysisScheduler;
};

//////////////////////////////////////////////////////////////////////

// These represent the output of an analysis job without exposing
// implementation details. This is consumed by
// AnalysisScheduler::record().
struct AnalysisOutput {
  struct Meta {
    std::vector<AnalysisDeps> funcDeps;
    std::vector<AnalysisDeps> classDeps;
    AnalysisChangeSet changed;
    FSStringSet removedFuncs;
    template <typename SerDe> void serde(SerDe& sd) {
      ScopedStringDataIndexer _;
      sd(funcDeps)
        (classDeps)
        (changed)
        (removedFuncs, string_data_lt_func{})
        ;
    }
  };

  std::vector<SString> classNames;
  std::vector<SString> cinfoNames;
  std::vector<SString> minfoNames;

  UniquePtrRefVec<php::Class> classes;
  UniquePtrRefVec<php::ClassBytecode> clsBC;
  RefVec<AnalysisIndexCInfo> cinfos;

  std::vector<SString> funcNames;
  UniquePtrRefVec<php::Func> funcs;
  UniquePtrRefVec<php::FuncBytecode> funcBC;
  RefVec<AnalysisIndexFInfo> finfos;

  RefVec<AnalysisIndexMInfo> minfos;

  std::vector<SString> unitNames;
  UniquePtrRefVec<php::Unit> units;

  Meta meta;
};

//////////////////////////////////////////////////////////////////////

// Scheduler to coordinate analysis rounds. This encapsulates all of
// the logic to track dependencies and changes in a way so that the
// user doesn't need to know any implementation details.
struct AnalysisScheduler {
  explicit AnalysisScheduler(Index&);

  // Register a class or function with the given name to be
  // tracked. If a class or function isn't tracked, it won't be
  // eligible for scheduling (though it still might be pulled in as a
  // dependency).
  void registerClass(SString);
  void registerFunc(SString);

  // Record the output of an analysis job. This can be called in a
  // multi-threaded context.
  void record(AnalysisOutput);

  // Called when all analysis jobs for this round have finished. This
  // calculates what has changed and what needs to be re-analyzed.
  void recordingDone();

  // Schedule the work that needs to run into buckets of (roughly) the
  // given size.
  std::vector<AnalysisInput> schedule(size_t bucketSize);

  size_t workItems() const {
    return classesToSchedule.size() + funcsToSchedule.size();
  }

private:
  // A change group represents all the entities in a given analysis
  // job. The idea is that if a dependency is in the same job as you,
  // even if it changed, you don't need to be rescheduled (because you
  // would have picked up the change locally).
  struct ChangeGroup {
    TSStringSet classes;
    FSStringSet funcs;
    SStringSet units;
  };

  using Type = AnalysisDeps::Type;

  void removeFuncs();
  void findToSchedule();
  void resetChanges();

  void recordChanges(const AnalysisChangeSet&, const ChangeGroup&);
  void updateDepState(AnalysisOutput&, ChangeGroup);

  void addClassToInput(SString, AnalysisInput&) const;
  void addFuncToInput(SString, AnalysisInput&) const;
  void addUnitToInput(SString, AnalysisInput&) const;
  void addDepsToInput(AnalysisInput&) const;
  void addDepConstantToInput(SString, SString, AnalysisInput&) const;
  void addDepTypeAliasToInput(SString, SString, AnalysisInput&) const;
  void addDepUnitToInput(SString, SString, AnalysisInput&) const;
  void addDepClassToInput(SString, SString, bool, AnalysisInput&) const;
  void addDepFuncToInput(SString, SString, Type, AnalysisInput&) const;

  Index& index;

  std::vector<SString> classNames;
  std::vector<SString> funcNames;

  struct DepState {
    Optional<AnalysisDeps> deps;
    Optional<AnalysisDeps> newDeps;
    std::shared_ptr<ChangeGroup> group;
  };

  struct FuncState {
    DepState depState;
    Type changed{};
  };
  struct ClassState {
    DepState depState;
    CompactVector<Type> methodChanges;
    boost::dynamic_bitset<> cnsChanges;
  };

  FSStringToOneT<FuncState> funcState;
  TSStringToOneT<ClassState> classState;
  SStringToOneNodeT<std::atomic<bool>> cnsChanged;

  FSStringSet funcsToSchedule;
  TSStringSet classesToSchedule;

  FSStringSet funcsToRemove;
  std::mutex funcsToRemoveLock;
};

//////////////////////////////////////////////////////////////////////

// Worklist for handling dependencies within the same analysis job. If
// a class or function updates something, if another class or function
// within the same job has a dependency, that will be re-analyzed
// within the same job. This speeds up convergence by avoiding another
// whole roundtrip through the scheduler.
struct AnalysisWorklist {
  FuncOrCls next();

  void schedule(FuncOrCls);
private:
  hphp_fast_set<FuncOrCls, FuncOrClsHasher> in;
  std::deque<FuncOrCls> list;
};

//////////////////////////////////////////////////////////////////////

// Equivalent of Index, but for an analysis job.
struct AnalysisIndex {
  template<typename T> using V = std::vector<T>;
  template<typename T> using VU = V<std::unique_ptr<T>>;

  AnalysisIndex(AnalysisWorklist&,
                VU<php::Class>,
                VU<php::Func>,
                VU<php::Unit>,
                VU<php::ClassBytecode>,
                VU<php::FuncBytecode>,
                V<AnalysisIndexCInfo>,
                V<AnalysisIndexFInfo>,
                V<AnalysisIndexMInfo>,
                VU<php::Class>,
                VU<php::Func>,
                VU<php::Unit>,
                AnalysisInput::Meta);
  ~AnalysisIndex();

  // Must be called in the worker's init() and fini() functions.
  static void start();
  static void stop();

  void freeze();

  const php::Unit& lookup_func_unit(const php::Func&) const;

  const php::Unit& lookup_class_unit(const php::Class&) const;

  const php::Class* lookup_const_class(const php::Const&) const;

  const php::Class& lookup_closure_context(const php::Class&) const;

  Optional<res::Class> resolve_class(SString) const;
  Optional<res::Class> resolve_class(const php::Class&) const;

  res::Class builtin_class(SString) const;

  Type lookup_constant(SString) const;

  std::vector<std::pair<SString, ClsConstInfo>>
  lookup_class_constants(const php::Class&) const;

  ClsConstLookupResult
  lookup_class_constant(const Type&, const Type&) const;

  ClsTypeConstLookupResult
  lookup_class_type_constant(
    const Type& cls,
    const Type& name,
    const Index::ClsTypeConstLookupResolver& resolver = {}) const;

  PropState lookup_private_props(const php::Class&) const;
  PropState lookup_private_statics(const php::Class&) const;

  Index::ReturnType lookup_return_type(MethodsInfo*, res::Func) const;
  Index::ReturnType lookup_return_type(MethodsInfo*,
                                       const CompactVector<Type>&,
                                       const Type&,
                                       res::Func) const;
  Index::ReturnType lookup_foldable_return_type(const CallContext&) const;

  std::pair<Index::ReturnType, size_t>
  lookup_return_type_raw(const php::Func& f) const;

  bool func_depends_on_arg(const php::Func&, size_t) const;

  res::Func resolve_func(SString) const;

  res::Func resolve_method(const Type&, SString) const;
  res::Func resolve_ctor(const Type&) const;
  res::Func resolve_func_or_method(const php::Func&) const;

  std::pair<const php::TypeAlias*, bool> lookup_type_alias(SString) const;

  Index::ClassOrTypeAlias lookup_class_or_type_alias(SString) const;

  PropMergeResult
  merge_static_type(PublicSPropMutations&,
                    PropertiesInfo&,
                    const Type&,
                    const Type&,
                    const Type&,
                    bool checkUB = false,
                    bool ignoreConst = false,
                    bool mustBeReadOnly = false) const;

  void refine_constants(const FuncAnalysisResult&);
  void refine_class_constants(const FuncAnalysisResult&);
  void refine_return_info(const FuncAnalysisResult&);
  void update_prop_initial_values(const FuncAnalysisResult&);
  void update_bytecode(FuncAnalysisResult&);

  using Output = extern_worker::Multi<
    extern_worker::Variadic<std::unique_ptr<php::Class>>,
    extern_worker::Variadic<std::unique_ptr<php::Func>>,
    extern_worker::Variadic<std::unique_ptr<php::Unit>>,
    extern_worker::Variadic<std::unique_ptr<php::ClassBytecode>>,
    extern_worker::Variadic<std::unique_ptr<php::FuncBytecode>>,
    extern_worker::Variadic<AnalysisIndexCInfo>,
    extern_worker::Variadic<AnalysisIndexFInfo>,
    extern_worker::Variadic<AnalysisIndexMInfo>,
    AnalysisOutput::Meta
  >;
  Output finish();

  struct IndexData;
private:
  std::unique_ptr<IndexData> const m_data;

  void push_context(const Context&);
  void pop_context();

  friend struct AnalysisIndexAdaptor;
};

//////////////////////////////////////////////////////////////////////

struct AnalysisIndexAdaptor : public IIndex {
  explicit AnalysisIndexAdaptor(const AnalysisIndex& index)
    : index{const_cast<AnalysisIndex&>(index)} {}

  const php::Unit* lookup_func_unit(const php::Func&) const override;
  const php::Unit* lookup_class_unit(const php::Class&) const override;
  const php::Class* lookup_const_class(const php::Const&) const override;
  const php::Class* lookup_closure_context(const php::Class&) const override;

  const CompactVector<const php::Class*>*
  lookup_closures(const php::Class*) const override;

  const hphp_fast_set<const php::Func*>*
  lookup_extra_methods(const php::Class*) const override;

  Optional<res::Class> resolve_class(SString) const override;
  Optional<res::Class> resolve_class(const php::Class&) const override;

  std::pair<const php::TypeAlias*, bool>
  lookup_type_alias(SString) const override;

  Index::ClassOrTypeAlias lookup_class_or_type_alias(SString) const override;

  res::Func resolve_func_or_method(const php::Func&) const override;
  res::Func resolve_func(SString) const override;
  res::Func resolve_method(Context, const Type&, SString) const override;
  res::Func resolve_ctor(const Type&) const override;

  std::vector<std::pair<SString, ClsConstInfo>>
  lookup_class_constants(const php::Class&) const override;

  ClsConstLookupResult lookup_class_constant(Context,
                                             const Type&,
                                             const Type&) const override;

  ClsTypeConstLookupResult
  lookup_class_type_constant(
    const Type&,
    const Type&,
    const Index::ClsTypeConstLookupResolver& r = {}) const override;

  Type lookup_constant(Context, SString) const override;
  bool func_depends_on_arg(const php::Func*, size_t) const override;
  Index::ReturnType
  lookup_foldable_return_type(Context, const CallContext&) const override;
  Index::ReturnType lookup_return_type(Context, MethodsInfo*, res::Func,
                                       Dep d = Dep::ReturnTy) const override;
  Index::ReturnType lookup_return_type(Context,
                                       MethodsInfo*,
                                       const CompactVector<Type>&,
                                       const Type&,
                                       res::Func,
                                       Dep d = Dep::ReturnTy) const override;

  std::pair<Index::ReturnType, size_t>
  lookup_return_type_raw(const php::Func*) const override;

  CompactVector<Type>
  lookup_closure_use_vars(const php::Func*, bool m = false) const override;

  PropState lookup_private_props(const php::Class*, bool m = false) const override;
  PropState lookup_private_statics(const php::Class*, bool m = false) const override;

  PropLookupResult lookup_static(Context,
                                 const PropertiesInfo&,
                                 const Type&,
                                 const Type&) const override;

  Type lookup_public_prop(const Type&, const Type&) const override;

  PropMergeResult
  merge_static_type(Context,
                    PublicSPropMutations&,
                    PropertiesInfo&,
                    const Type&,
                    const Type&,
                    const Type&,
                    bool checkUB = false,
                    bool ignoreConst = false,
                    bool mustBeReadOnly = false) const override;

  bool using_class_dependencies() const override;

private:
  void push_context(const Context&) const override;
  void pop_context() const override;

  AnalysisIndex& index;
};

//////////////////////////////////////////////////////////////////////

/*
 * Used for collecting all mutations of public static property types.
 */
struct PublicSPropMutations {
  explicit PublicSPropMutations(bool enabled = true);
private:
  friend struct Index;

  struct KnownKey {
    ClassInfo* cinfo;
    SString prop;
    bool operator==(const KnownKey& o) const {
      return cinfo == o.cinfo && prop == o.prop;
    }
    struct Hasher {
      size_t operator()(const KnownKey& k) const {
        return folly::hash::hash_combine(k.cinfo, k.prop);
      }
    };
  };

  using UnknownMap = SStringToOneT<Type>;
  using KnownMap = hphp_fast_map<KnownKey, Type, KnownKey::Hasher>;

  // Public static property mutations are actually rare, so defer allocating the
  // maps until we actually see one.
  struct Data {
    bool m_nothing_known{false};
    UnknownMap m_unknown;
    KnownMap m_known;
  };
  std::unique_ptr<Data> m_data;
  bool m_enabled;

  Data& get();

  void mergeKnown(const ClassInfo* ci, const php::Prop& prop, const Type& val);
  void mergeUnknownClass(SString prop, const Type& val);
  void mergeUnknown(Context);
};

//////////////////////////////////////////////////////////////////////

}
