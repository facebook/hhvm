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

#include <hphp/runtime/base/datatype.h>
#include "hphp/runtime/base/annot-type.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/type-constraint-flags.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/containers.h"

#include "hphp/util/functional.h"

#include <functional>
#include <string>

namespace HPHP {
struct Func;
struct StringData;
}

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * TypeConstraint represents the metadata required to check a PHP
 * function or method parameter typehint at runtime.
 */
struct TypeConstraint {
  /*
   * Special type constraints use a "Type", instead of just
   * underlyingDataType().
   *
   * See underlyingDataType().
   */
  using Type = AnnotType;
  using MetaType = AnnotMetaType;

  static const int32_t ReturnId = -1;

  TypeConstraint()
    : m_flags(NoFlags)
    , m_clsName(nullptr)
    , m_typeName(nullptr)
    , m_namedType(nullptr)
  {
    init();
  }

  TypeConstraint(const StringData* typeName, TypeConstraintFlags flags)
    : m_flags(flags)
    , m_clsName(nullptr)
    , m_typeName(typeName)
    , m_namedType(nullptr)
  {
    init();
  }

  template<class SerDe>
  void serde(SerDe& sd) {
    sd(m_typeName)
      (m_flags)
      ;
    if (m_flags & TypeConstraintFlags::Resolved) {
      sd(m_type)
        (m_clsName)
        ;
    }
    if (SerDe::deserializing) {
      init();
    }
  }

  TypeConstraint(const TypeConstraint&) = default;
  TypeConstraint& operator=(const TypeConstraint&) = default;

  bool operator==(const TypeConstraint& o) const;

  size_t stableHash() const;

  void resolveType(AnnotType t, bool nullable, LowStringPtr clsName);
  void unresolve();

  void addFlags(TypeConstraintFlags flags) {
    m_flags = static_cast<TypeConstraintFlags>(m_flags | flags);
  }

  /*
   * Returns: whether this constraint implies any runtime checking at
   * all.  If this function returns false, it means the parameter type
   * verification would be a no-op.
   */
  bool hasConstraint() const { return m_typeName; }

  /*
   * Read access to various members.
   */
  const StringData* clsName() const { return m_clsName; }
  const StringData* typeName() const { return m_typeName; }
  const NamedType* clsNamedType() const {
    assertx(isObject());
    return m_namedType;
  }
  const NamedType* typeNamedType() const {
    assertx(isUnresolved());
    return m_namedType;
  }
  const NamedType* anyNamedType() const { return m_namedType; }
  TypeConstraintFlags flags() const { return m_flags; }

  /*
   * Access to the "meta type" for this TypeConstraint.
   */
  MetaType metaType() const { return getAnnotMetaType(m_type); }

  /*
   * If this->isUnresolved(), resolve it using the autoloader.
   */
  TinyVector<TypeConstraint> resolvedWithAutoload() const;

  /*
   * Returns the underlying DataType for this TypeConstraint.
   */
  MaybeDataType underlyingDataType() const {
    if (!isPrecise()) return std::nullopt;
    if (isObject() && interface_supports_non_objects(m_clsName)) {
      return std::nullopt;
    }
    return MaybeDataType(getAnnotDataType(m_type));
  }

  /*
   * Returns the underlying DataType for this TypeConstraint,
   * chasing down type aliases.
   */
  MaybeDataType underlyingDataTypeResolved() const;

  /*
   * Check if this TypeConstraint *might* imply a runtime check. If it returns
   * false, it definitely doesn't. If it returns true, it might still be a mixed
   * type at runtime (because of a type-alias for example).
   */
  bool isCheckable() const {
    return hasConstraint()
        && !isMixed()
        && !isTypeVar()
        && !isTypeConstant()
        && !(isUpperBound() && RuntimeOption::EvalEnforceGenericsUB == 0);
  }

  /*
   * Check if this TypeConstraint definitely does not require a runtime check
   * (equivalent to mixed). This may invoke the autoloader and will resolve
   * type-aliases, so is exact.
   */
  bool isMixedResolved() const;

  /*
   * Check if this TypeConstraint may not require a runtime check (equivalent to
   * mixed). If it returns false, it definitely is not. If it returns true, it
   * still might not be (because of a type-alias for example). This will not
   * invoke the autoloader.
   */
  bool maybeMixed() const;

  /*
   * Predicates for various properties of the type constraint.
   */
  bool isNullable() const { return m_flags & Nullable; }
  bool isSoft()     const { return m_flags & Soft; }
  bool isExtended() const { return m_flags & ExtendedHint; }
  bool isTypeVar()  const { return m_flags & TypeVar; }
  bool isTypeConstant() const { return m_flags & TypeConstant; }
  bool isUpperBound() const { return m_flags & UpperBound; }

  bool isPrecise()  const { return metaType() == MetaType::Precise; }
  bool isMixed()    const { return m_type == Type::Mixed; }
  bool isThis()     const { return m_type == Type::This; }
  bool isCallable() const { return m_type == Type::Callable; }
  bool isNumber()   const { return m_type == Type::Number; }
  bool isNothing()  const { return m_type == Type::Nothing; }
  bool isNoReturn() const { return m_type == Type::NoReturn; }
  bool isArrayKey() const { return m_type == Type::ArrayKey; }
  bool isDict()     const { return m_type == Type::Dict; }
  bool isVec()      const { return m_type == Type::Vec; }
  bool isKeyset()   const { return m_type == Type::Keyset; }
  bool isObject()   const { return m_type == Type::Object; }
  bool isInt()      const { return m_type == Type::Int; }
  bool isString()   const { return m_type == Type::String; }
  bool isArrayLike() const { return m_type == Type::ArrayLike; }
  bool isVecOrDict() const { return m_type == Type::VecOrDict; }
  bool isClassname() const { return m_type == Type::Classname; }
  bool isUnresolved() const { return m_type == Type::Unresolved; }

  // Returns true if we should convert a ClsMeth to a varray for this typehint.
  bool convertClsMethToArrLike() const;

  AnnotType type()  const { return m_type; }

  bool validForProp() const { return propSupportsAnnot(m_type); }

  void validForPropResolved(const Class* declCls,
                            const StringData* propName) const {
    assertx(validForProp());
    if (!isUnresolved()) return;
    auto const r = resolvedWithAutoload();
    auto const b = std::all_of(
      std::begin(r), std::end(r),
      [] (const TypeConstraint& tc) { return tc.validForProp(); }
    );
    if (!b) validForPropFail(declCls, propName);
  }

  bool validForEnumBase() const {
    auto const resolvedVec = resolvedWithAutoload();
    if (resolvedVec.size() != 1) return false;
    auto const resolved = resolvedVec[0];
    return resolved.isInt() || resolved.isString() ||
           resolved.isArrayKey() || resolved.isClassname() ||
           resolved.isNothing();
  }

  /*
   * Format this TypeConstraint for display to the user. Context is used to
   * optionally resolve This to its class name. Extra will cause the resolved
   * type (if any) to be appended to the name.
   */
  std::string displayName(const Class* context = nullptr,
                          bool extra = false) const;

  /*
   * Obtain an initial value suitable for this type-constraint. Where possible,
   * the initial value is chosen to satisfy the type-constraint, but this isn't
   * always possible (for example, for objects).
   */
  TypedValue defaultValue() const {
    // Nullable type-constraints should always default to null, as Hack
    // guarantees this.
    if (!isCheckable() || isNullable()) return make_tv<KindOfNull>();
    return annotDefaultValue(m_type);
  }

  /*
   * Returns whether this and another type-constraint might not be equivalent at
   * runtime. Two type-constraints are equivalent if they allow exactly the same
   * values. This function is conservative and will return true if not
   * sure. This function will not autoload or check loaded classes of
   * type-aliases. Only meant for property type-hints.
   */
  bool maybeInequivalentForProp(const TypeConstraint& other) const;

  /*
   * Returns whether this and another type-constraint are definitely
   * equivalent. Unlike maybeInequivalentForProp(), this function is exact and
   * can autoload. Only meant for property type-hints.
   */
  bool equivalentForProp(const TypeConstraint& other) const;

  /*
   * Normal check if this type-constraint is compatible with the given value
   * (using the given context). This can invoke the autoloader and is always
   * exact. This should not be used for property type-hints (which behave
   * slightly differently) and the context is required.  Instead use checkProp.
   * The context determines the meaning of This type-constraint.
   */
  bool check(tv_rval val, const Class* context) const {
    return checkImpl<CheckMode::Exact>(val, context);
  }
  bool checkProp(tv_rval val, const Class* context) const {
    return checkImpl<CheckMode::ExactProp>(val, context);
  }

  /*
   * Assert that this type-constraint is compatible with the given value. This
   * is meant for use in assertions and is conservative. It will not invoke the
   * autoloader, but can consult already loaded classes or type-aliases. It will
   * only return false if the value definitely does not satisfy the
   * type-constraint, true otherwise.
   */
  bool assertCheck(tv_rval val) const {
    return checkImpl<CheckMode::Assert>(val, nullptr);
  }

  /*
   * Check if this type-constraint is compatible with the given value in *all
   * contexts*. That is, regardless of which classes or type-aliases might be
   * loaded. A type which passes this check will never need to be checked at
   * run-time. This will utilize persistent classes but nothing else.
   */
  bool alwaysPasses(tv_rval val) const {
    if (!isCheckable()) return true;
    return checkImpl<CheckMode::AlwaysPasses>(val, nullptr);
  }
  // Same as the above, but uses a type instead of an actual value. The
  // StringData* variant is for objects.
  bool alwaysPasses(const StringData* clsName) const;
  bool alwaysPasses(DataType dt) const;

  bool checkTypeAliasObj(const Class* cls) const {
    return checkTypeAliasImpl<false>(cls);
  }

  // NB: Can throw if the check fails.
  void verifyParam(tv_lval val,
                   const Class* ctx,
                   const Func* func,
                   int paramNum) const;
  void verifyReturn(TypedValue* tv, const Class* ctx, const Func* func) const;
  void verifyReturnNonNull(TypedValue* tv,
                           const Class* ctx,
                           const Func* func) const;
  void verifyOutParam(TypedValue* tv,
                      const Class* ctx,
                      const Func* func,
                      int paramNum) const;
  // TODO(T61738946): We can take a tv_rval here once we remove support for
  // coercing class_meth types.
  void verifyProperty(tv_lval val,
                      const Class* thisCls,
                      const Class* declCls,
                      const StringData* propName) const;
  void verifyStaticProperty(tv_lval val,
                            const Class* thisCls,
                            const Class* declCls,
                            const StringData* propName) const;

  void verifyParamFail(tv_lval val,
                       const Class* ctx,
                       const Func* func,
                       int paramNum) const;
  void verifyOutParamFail(TypedValue* tv,
                          const Class* ctx,
                          const Func* func,
                          int paramNum) const;
  void verifyReturnFail(TypedValue* tv,
                        const Class* ctx,
                        const Func* func) const {
    verifyFail(tv, ctx, func, ReturnId);
  }
  // TODO(T61738946): We can take a tv_rval here once we remove support for
  // coercing class_meth types.
  void verifyPropFail(const Class* thisCls, const Class* declCls,
                      tv_lval val, const StringData* propName,
                      bool isStatic) const;

  bool maybeStringCompatible() const;

  // Whether input may need to be coerced.
  bool mayCoerce() const {
    return maybeStringCompatible();
  }

  /**
   * Return the correct `DataType` that represents this type constraint as used
   * as a type within systemlib, specifically in the context of a `__Native`
   * function. For example:
   * - Nullable and soft types are always represented as `KindOfMixed`
   * - Unresolved types are always represented as `KindOfObejct`
   * - Otherwise, grab the result of `underlyingDataType`
   */
  MaybeDataType asSystemlibType() const;

private:
  void init();

  // There are a few cases where a type constraint does not pass, but we don't
  // raise an error. Some of these cases are resolved by mutating val instead.
  //
  // If this method returns true, we don't need to raise the type error.
  bool tryCommonCoercions(tv_lval val, const Class* ctx,
                          const Class* propDecl) const;

  enum class CheckMode {
    Exact, // Do an exact check with autoloading
    ExactProp, // Do an exact prop check with autoloading
    AlwaysPasses, // Don't check environment at all. Return false if not sure.
    Assert // Check loaded classes/type-aliases, but don't autoload. Return true
           // if not sure.
  };

  template <CheckMode>
  bool checkImpl(tv_rval val, const Class* context) const;

  template <bool, bool>
  bool checkNamedTypeNonObj(tv_rval val) const;

  template <bool> bool checkTypeAliasImpl(const Class* type) const;

  void verifyFail(tv_lval val,
                  const Class* ctx,
                  const Func* func,
                  int id) const;

  bool checkStringCompatible() const;

  void validForPropFail(const Class*, const StringData*) const;

private:
  // m_type represents the type to check on.  We don't know whether a
  // bare name is a class/interface name or a type alias or an enum,
  // so when this is set to Type::Unresolved we may have to resolve a type
  // alias or enum and test for a different DataType (see annotCompat()
  // for details).
  Type m_type;
  TypeConstraintFlags m_flags;
  LowStringPtr m_clsName;   // valid iff isObject() or if an enum
  LowStringPtr m_typeName;
  LowPtr<const NamedType> m_namedType;
};

/// TypeIntersectionConstraintT is generally used for upper-bounds. It's
/// templated so we can have a common implementation with different vector
/// representation (std::vector, TinyVector, VMCompactVector) based on need.
template<typename VectorT>
struct TypeIntersectionConstraintT {
  VectorT m_constraints;

  TypeIntersectionConstraintT() = default;

  explicit TypeIntersectionConstraintT(TypeConstraint tc) {
    m_constraints.emplace_back(std::move(tc));
  }

  template<class SerDe>
  void serde(SerDe& sd) {
    sd(m_constraints);
  }

  bool isTop() const { return m_constraints.empty(); }
  bool isSimple() const { return m_constraints.size() == 1; }

  const TypeConstraint& asSimple() const {
    assertx(isSimple());
    return m_constraints[0];
  }

  TypeConstraint& asSimpleMut() {
    assertx(isSimple());
    return m_constraints[0];
  }

  void add(TypeConstraint tc) {
    m_constraints.emplace_back(std::move(tc));
  }
};

using TinyTypeIntersectionConstraint = TypeIntersectionConstraintT<TinyVector<TypeConstraint>>;
using TypeIntersectionConstraint = TypeIntersectionConstraintT<CompactVector<TypeConstraint>>;
using StdTypeIntersectionConstraint = TypeIntersectionConstraintT<std::vector<TypeConstraint>>;
using VMTypeIntersectionConstraint = TypeIntersectionConstraintT<VMCompactVector<TypeConstraint>>;

//////////////////////////////////////////////////////////////////////

/*
 * Its possible to use type constraints on function parameters to devise better
 * memoization key generation schemes. For example, if we know the
 * type-constraint limits the parameter to only ever being an integer or string,
 * then the memoization key scheme can just be the identity. This is because
 * integers and strings won't collide with each other, and we know it won't ever
 * be anything else. Without such a constraint, the string would need escaping.
 *
 * This function takes a type-constraint and returns the suitable "memo-key
 * constraint" if it corresponds to one. Note: HHBBC, the interpreter, and the
 * JIT all need to agree exactly on the scheme for each constraint. It is the
 * caller's responsibility to actually verify that type-hints are being
 * enforced. If they are not, then none of this information can be used.
 */
enum class MemoKeyConstraint {
  Int,
  IntOrNull,
  Bool,
  BoolOrNull,
  Str,
  StrOrNull,
  IntOrStr,
  Dbl,
  DblOrNull,
  Object,
  ObjectOrNull,
  None
};
MemoKeyConstraint memoKeyConstraintFromTC(const TypeConstraint&);

std::string describe_actual_type(tv_rval val);

bool tcCouldBeReified(const Func*, uint32_t);

/*
 * Check if the result of a SetOp needs to be checked against the property's
 * type-hint. If we do, we'll have to perform the SetOp on a temporary, do the
 * check, then move it into the final location. Otherwise we can just do the
 * SetOp in place.
 *
 * For now we only elide the type-check for concats when the lhs is already a
 * string or the type-hint always allows strings (since a concat always produces
 * a string). This is more than a minor optimization. If we do the concat on a
 * temporary, it will increase the ref-count of the target, meaning the concat
 * will trigger a COW. This can be a major performance hit if the target is
 * large.
 */
inline bool setOpNeedsTypeCheck(const TypeConstraint& tc,
                                SetOpOp op,
                                tv_rval lhs) {
  if (RuntimeOption::EvalCheckPropTypeHints <= 0 || !tc.isCheckable()) {
    return false;
  }
  if (RuntimeOption::EvalEnforceGenericsUB <= 0 && tc.isUpperBound()) {
    return false;
  }
  if (op != SetOpOp::ConcatEqual) return true;
  // If the target of the concat is already a string, or the type-hint always
  // allows a string, we don't need a check because the concat will always
  // produce a string, regardless of the rhs.
  if (LIKELY(isStringType(type(lhs)))) return false;
  return !tc.alwaysPasses(KindOfString);
}

// Add all flags in tc (except TypeVar) to ub.
// FIXME: applying random flags such as Resolved is super sketchy
void applyFlagsToUB(TypeConstraint& ub, const TypeConstraint& tc);
}
