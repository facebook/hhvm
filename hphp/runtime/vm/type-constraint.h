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

#ifndef incl_HPHP_TYPE_CONSTRAINT_H_
#define incl_HPHP_TYPE_CONSTRAINT_H_

#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/base/annot-type.h"

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
     * is a TypeConstant
     * class Foo {
     *   const type T = int;
     *   public function bar(Foo::T $x) { ... }
     */
     TypeConstant = 0x20,
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
    init();
  }

  template<class SerDe>
  void serde(SerDe& sd) {
    sd(m_typeName)
      (m_flags)
      ;
    if (SerDe::deserializing) {
      init();
    }
  }

  TypeConstraint(const TypeConstraint&) = default;
  TypeConstraint& operator=(const TypeConstraint&) = default;

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

  /*
   * Predicates for various properties of the type constraint.
   */
  bool isNullable() const { return m_flags & Nullable; }
  bool isSoft()     const { return m_flags & Soft; }
  bool isHHType()   const { return m_flags & HHType; }
  bool isExtended() const { return m_flags & ExtendedHint; }
  bool isTypeVar()  const { return m_flags & TypeVar; }
  bool isTypeConstant() const { return m_flags & TypeConstant; }

  bool isPrecise()  const { return metaType() == MetaType::Precise; }
  bool isMixed()    const { return m_type == Type::Mixed; }
  bool isSelf()     const { return m_type == Type::Self; }
  bool isParent()   const { return m_type == Type::Parent; }
  bool isCallable() const { return m_type == Type::Callable; }
  bool isNumber()   const { return m_type == Type::Number; }
  bool isArrayKey() const { return m_type == Type::ArrayKey; }

  bool isArray()    const { return m_type == Type::Array; }
  bool isObject()  const { return m_type == Type::Object; }

  AnnotType type() const { return m_type; }

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

  std::string displayName(const Func* func = nullptr) const;

  /*
   * Returns: whether two TypeConstraints are compatible, in the sense
   * required for PHP inheritance where a method with parameter
   * typehints is overridden.
   */
  bool compat(const TypeConstraint& other) const;

  // General check for any constraint.
  bool check(TypedValue* tv, const Func* func) const;

  bool checkTypeAliasObj(const Class* cls) const;
  bool checkTypeAliasNonObj(const TypedValue* tv) const;

  // NB: will throw if the check fails.
  void verifyParam(TypedValue* tv, const Func* func, int paramNum) const {
    if (UNLIKELY(!check(tv, func))) {
      verifyParamFail(func, tv, paramNum);
    }
  }
  void verifyReturn(TypedValue* tv, const Func* func) const {
    if (UNLIKELY(!check(tv, func))) {
      verifyReturnFail(func, tv);
    }
  }

  // Can not be private; used by the translator.
  void selfToClass(const Func* func, const Class **cls) const;
  void parentToClass(const Func* func, const Class **cls) const;
  void verifyFail(const Func* func, TypedValue* tv, int id) const;
  void verifyParamFail(const Func* func, TypedValue* tv,
                       int paramNum) const {
    verifyFail(func, tv, paramNum);
  }
  void verifyReturnFail(const Func* func, TypedValue* tv) const {
    verifyFail(func, tv, ReturnId);
  }

private:
  void init();
  void selfToTypeName(const Func* func, const StringData **typeName) const;
  void parentToTypeName(const Func* func, const StringData **typeName) const;

private:
  // m_type represents the type to check on.  We don't know whether a
  // bare name is a class/interface name or a type alias or an enum,
  // so when this is set to Type::Object we may have to resolve a type
  // alias or enum and test for a different DataType (see annotCompat()
  // for details).
  Type m_type;
  Flags m_flags;
  LowStringPtr m_typeName;
  const NamedEntity* m_namedEntity;
};

//////////////////////////////////////////////////////////////////////

inline TypeConstraint::Flags
operator|(TypeConstraint::Flags a, TypeConstraint::Flags b) {
  return TypeConstraint::Flags(static_cast<int>(a) | static_cast<int>(b));
}

//////////////////////////////////////////////////////////////////////

}

#endif
