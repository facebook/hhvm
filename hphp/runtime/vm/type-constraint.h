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

#ifndef incl_HPHP_TYPE_CONSTRAINT_H_
#define incl_HPHP_TYPE_CONSTRAINT_H_

#include "hphp/runtime/base/annot-type.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/type-profile.h"

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
  enum Flags : uint8_t {
    NoFlags = 0x0,

    /*
     * Nullable type hints check they are either the specified type,
     * or null.
     */
    Nullable = 0x1,

    /*
     * This flag indicates either EnableHipHopSyntax was true, or the
     * type came from a <?hh file and EnableHipHopSyntax was false.
     */
    HHType = 0x2,

    /*
     * Extended hints are hints that do not apply to normal, vanilla
     * php.  For example "?Foo".
     */
    ExtendedHint = 0x4,

    /*
     * Indicates that a type constraint is a type variable. For example,
     * the constraint on $x is a TypeVar.
     * class Foo<T> {
     *   public function bar(T $x) { ... }
     * }
     */
    TypeVar = 0x8,

    /*
     * Soft type hints: triggers warning, but never fatals
     * E.g. "@int"
     */
    Soft = 0x10,

    /*
     * Indicates a type constraint is a type constant, which is similar to a
     * type alias defined inside a class. For instance, the constraint on $x
     * is a TypeConstant:
     *
     * class Foo {
     *   const type T = int;
     *   public function bar(Foo::T $x) { ... }
     * }
     */
    TypeConstant = 0x20,

    /*
     * Indicates that a Object type-constraint was resolved by hhbbc,
     * and the actual type is in m_type. When set, Object is guaranteed
     * to be an object, not a type-alias.
     */
    Resolved = 0x40,

    /*
     * Indicates that no mock object can satisfy this constraint.  This is
     * resolved by HHBBC.
     */
    NoMockObjects = 0x80
  };

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
    , m_typeName(nullptr)
    , m_namedEntity(nullptr)
  {
    init();
  }

  TypeConstraint(const StringData* typeName, Flags flags)
    : m_flags(flags)
    , m_typeName(typeName)
    , m_namedEntity(nullptr)
  {
    assertx(!(flags & Flags::Resolved));
    init();
  }

  template<class SerDe>
  void serde(SerDe& sd) {
    sd(m_typeName)
      (m_flags)
      ;
    if (m_flags & Flags::Resolved) {
      sd(m_type);
    }
    if (SerDe::deserializing) {
      init();
    }
  }

  TypeConstraint(const TypeConstraint&) = default;
  TypeConstraint& operator=(const TypeConstraint&) = default;

  void resolveType(AnnotType t, bool nullable) {
    assertx(m_type == AnnotType::Object);
    assertx(t != AnnotType::Object);
    auto flags = m_flags | Flags::Resolved;
    if (nullable) flags |= Flags::Nullable;
    m_flags = static_cast<Flags>(flags);
    m_type = t;
  }

  void setNoMockObjects() {
    auto flags = m_flags | Flags::NoMockObjects;
    m_flags = static_cast<Flags>(flags);
  }

  /*
   * Returns: whether this constraint implies any runtime checking at
   * all.  If this function returns false, it means the
   * VerifyParamType would be a no-op.
   */
  bool hasConstraint() const { return m_typeName; }

  /*
   * Read access to various members.
   */
  const StringData* typeName() const { return m_typeName; }
  const NamedEntity* namedEntity() const { return m_namedEntity; }
  Flags flags() const { return m_flags; }

  /*
   * Access to the "meta type" for this TypeConstraint.
   */
  MetaType metaType() const { return getAnnotMetaType(m_type); }

  /*
   * Returns the underlying DataType for this TypeConstraint.
   */
  MaybeDataType underlyingDataType() const {
    if (isVArray() || isDArray() || isVArrayOrDArray()) {
      return KindOfArray;
    }
    auto const dt = getAnnotDataType(m_type);
    return (dt != KindOfUninit || isPrecise())
      ? MaybeDataType(dt)
      : folly::none;
  }

  /*
   * Returns the underlying DataType for this TypeConstraint,
   * chasing down type aliases.
   */
  MaybeDataType underlyingDataTypeResolved() const;

  bool isCheckable() const {
    return hasConstraint() && !isMixed() && !isTypeVar() && !isTypeConstant();
  }

  /*
   * Predicates for various properties of the type constraint.
   */
  bool isNullable() const { return m_flags & Nullable; }
  bool isSoft()     const { return m_flags & Soft; }
  bool isHHType()   const { return m_flags & HHType; }
  bool isExtended() const { return m_flags & ExtendedHint; }
  bool isTypeVar()  const { return m_flags & TypeVar; }
  bool isTypeConstant() const { return m_flags & TypeConstant; }
  bool isResolved() const { return m_flags & Resolved; }
  bool couldSeeMockObject() const { return !(m_flags & NoMockObjects); }

  bool isPrecise()  const { return metaType() == MetaType::Precise; }
  bool isMixed()    const { return m_type == Type::Mixed; }
  bool isSelf()     const { return m_type == Type::Self; }
  bool isThis()     const { return m_type == Type::This; }
  bool isParent()   const { return m_type == Type::Parent; }
  bool isCallable() const { return m_type == Type::Callable; }
  bool isNumber()   const { return m_type == Type::Number; }
  bool isNoReturn() const { return m_type == Type::NoReturn; }
  bool isArrayKey() const { return m_type == Type::ArrayKey; }
  bool isArrayLike() const { return m_type == Type::ArrayLike; }

  bool isArray()    const {
    return m_type == Type::Array ||
      isVArray() || isDArray() || isVArrayOrDArray();
  }
  bool isDict()     const {
    return m_type == Type::Dict ||
      (RuntimeOption::EvalHackArrDVArrs && m_type == Type::DArray);
  }
  bool isVec()      const {
    return m_type == Type::Vec ||
      (RuntimeOption::EvalHackArrDVArrs && m_type == Type::VArray);
  }
  bool isKeyset()   const { return m_type == Type::Keyset; }
  bool isVecOrDict() const {
    return m_type == Type::VecOrDict ||
      (RuntimeOption::EvalHackArrDVArrs && m_type == Type::VArrOrDArr);
  }

  bool isObject()   const { return m_type == Type::Object; }
  bool isInt()      const { return m_type == Type::Int; }
  bool isString()   const { return m_type == Type::String; }

  bool isVArray()   const {
    return !RuntimeOption::EvalHackArrDVArrs && m_type == Type::VArray;
  }
  bool isDArray()   const {
    return !RuntimeOption::EvalHackArrDVArrs && m_type == Type::DArray;
  }
  bool isVArrayOrDArray() const {
    return !RuntimeOption::EvalHackArrDVArrs && m_type == Type::VArrOrDArr;
  }

  AnnotType type()  const { return m_type; }

  bool validForProp() const {
    return !isSelf() && !isParent() && !isCallable() && !isNoReturn();
  }

  /*
   * A string representation of this type constraint.
   */
  std::string fullName() const {
    std::string name;
    if (isSoft()) {
      name += '@';
    }
    if (isNullable() && isExtended()) {
      name += '?';
    }
    name += m_typeName->data();
    if (isNullable() && !isExtended()) {
      name += " (defaulted to null)";
    }
    return name;
  }

  /*
   * Format this TypeConstraint for display to the user. Context is used to
   * optionally resolve Self, Parent, and This to their class names. Extra will
   * cause the resolved type (if any) to be appended to the name.
   */
  std::string displayName(const Class* context = nullptr,
                          bool extra = false) const;

  /*
   * Obtain an initial value suitable for this type-constraint. Where possible,
   * the initial value is chosen to satisfy the type-constraint, but this isn't
   * always possible (for example, for objects).
   */
  Cell defaultValue() const {
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
  enum class EquivalentResult {
    Pass,    // Equivalent
    DVArray, // Not equivalent because of d/varray mismatch
    Fail     // Not equivalent
  };
  EquivalentResult equivalentForProp(const TypeConstraint& other) const;

  /*
   * Returns: whether two TypeConstraints are compatible, in the sense
   * required for PHP inheritance where a method with parameter
   * typehints is overridden.
   */
  bool compat(const TypeConstraint& other) const;

  /*
   * Normal check if this type-constraint is compatible with the given value
   * (using the given context). This can invoke the autoloader and is always
   * exact. This should not be used for property type-hints (which behave
   * slightly differently) and the context is required. The context determines
   * the meaning of Self, Parent, and This type-constraints.
   */
  bool check(const TypedValue* tv, const Class* context) const {
    return checkImpl<CheckMode::Exact>(tv, context);
  }

  /*
   * Assert that this type-constraint is compatible with the given value. This
   * is meant for use in assertions and is conservative. It will not invoke the
   * autoloader, but can consult already loaded classes or type-aliases. It will
   * only return false if the value definitely does not satisfy the
   * type-constraint, true otherwise.
   */
  bool assertCheck(const TypedValue* tv) const {
    return checkImpl<CheckMode::Assert>(tv, nullptr);
  }

  /*
   * Check if this type-constraint is compatible with the given value in *all
   * contexts*. That is, regardless of what classes or type-aliases are
   * currently loaded. A type which passes this check will never need to be
   * checked at run-time.
   */
  bool alwaysPasses(const TypedValue* tv) const {
    if (!isCheckable()) return true;
    return checkImpl<CheckMode::AlwaysPasses>(tv, nullptr);
  }

  bool checkTypeAliasObj(const Class* cls) const {
    return checkTypeAliasObjImpl<false>(cls);
  }

  // NB: Can throw if the check fails.
  void verifyParam(TypedValue* tv, const Func* func, int paramNum) const;
  void verifyReturn(TypedValue* tv, const Func* func) const;
  void verifyReturnNonNull(TypedValue* tv, const Func* func) const;
  void verifyOutParam(const TypedValue* tv, const Func* func,
                      int paramNum) const;
  void verifyProperty(const TypedValue* tv,
                      const Class* thisCls,
                      const Class* declCls,
                      const StringData* propName) const;
  void verifyStaticProperty(const TypedValue* tv,
                            const Class* thisCls,
                            const Class* declCls,
                            const StringData* propName) const;

  void verifyFail(const Func* func, TypedValue* tv, int id) const;
  void verifyParamFail(const Func* func, TypedValue* tv, int paramNum) const;
  void verifyOutParamFail(const Func* func, const TypedValue* tv,
                          int paramNum) const;
  void verifyReturnFail(const Func* func, TypedValue* tv) const {
    verifyFail(func, tv, ReturnId);
  }
  void verifyPropFail(const Class* thisCls, const Class* declCls,
                      const TypedValue* tv, const StringData* propName,
                      bool isStatic) const;

private:
  void init();

  enum class CheckMode {
    Exact, // Do an exact check with autoloading
    ExactProp, // Do an exact prop check with autoloading
    AlwaysPasses, // Don't check environment at all. Return false if not sure.
    Assert // Check loaded classes/type-aliases, but don't autoload. Return true
           // if not sure.
  };

  template <CheckMode>
  bool checkImpl(const TypedValue* tv, const Class* context) const;

  template <bool, bool>
  bool checkTypeAliasNonObj(const TypedValue* tv) const;

  template <bool>
  bool checkTypeAliasObjImpl(const Class* cls) const;

  void verifyFail(const Func* func, TypedValue* tv, int id,
                  bool useStrictTypes) const;

  folly::Optional<AnnotType> checkDVArray(const Cell*) const;

private:
  // m_type represents the type to check on.  We don't know whether a
  // bare name is a class/interface name or a type alias or an enum,
  // so when this is set to Type::Object we may have to resolve a type
  // alias or enum and test for a different DataType (see annotCompat()
  // for details).
  Type m_type;
  Flags m_flags;
  LowStringPtr m_typeName;
  LowPtr<const NamedEntity> m_namedEntity;
};

//////////////////////////////////////////////////////////////////////

inline TypeConstraint::Flags
operator|(TypeConstraint::Flags a, TypeConstraint::Flags b) {
  return TypeConstraint::Flags(static_cast<int>(a) | static_cast<int>(b));
}

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

const char* describe_actual_type(const TypedValue* tv, bool isHHType);

bool call_uses_strict_types(const Func* func);

}

#endif
