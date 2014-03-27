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
  enum Flags {
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
  };

  /*
   * Special type constraints use a "MetaType", instead of just
   * underlyingDataType().
   *
   * See underlyingDataType().
   */
  enum class MetaType { Precise, Self, Parent, Callable, Number };

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
  MetaType metaType() const { return m_type.metatype; }

  /*
   * Returns the underlying DataType for this TypeConstraint.
   *
   * This DataType is probably not relevant if the metaType is not
   * MetaType::Precise, and also is a bit special when it is
   * KindOfObject: it may either be a type alias or a class, depending
   * on what typeName() means in a given request.
   */
  DataType underlyingDataType() const { return m_type.dt; }

  /*
   * Predicates for various properties of the type constraint.
   */
  bool isNullable() const { return m_flags & Nullable; }
  bool isSoft()     const { return m_flags & Soft; }
  bool isHHType()   const { return m_flags & HHType; }
  bool isExtended() const { return m_flags & ExtendedHint; }
  bool isTypeVar()  const { return m_flags & TypeVar; }
  bool isSelf()     const { return metaType() == MetaType::Self; }
  bool isParent()   const { return metaType() == MetaType::Parent; }
  bool isCallable() const { return metaType() == MetaType::Callable; }
  bool isNumber()   const { return metaType() == MetaType::Number; }
  bool isPrecise()  const { return metaType() == MetaType::Precise; }

  bool isArray()    const { return m_type.dt == KindOfArray; }

  bool isObjectOrTypeAlias() const {
    assert(IMPLIES(isNumber(), m_type.dt != KindOfObject));
    assert(IMPLIES(isParent(), m_type.dt == KindOfObject));
    assert(IMPLIES(isSelf(), m_type.dt == KindOfObject));
    assert(IMPLIES(isCallable(), m_type.dt == KindOfObject));
    return m_type.dt == KindOfObject;
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

  std::string displayName(const Func* func = nullptr) const;

  /*
   * Returns: whether two TypeConstraints are compatible, in the sense
   * required for PHP inheritance where a method with parameter
   * typehints is overridden.
   */
  bool compat(const TypeConstraint& other) const {
    if (other.isExtended() || isExtended()) {
      /*
       * Rely on the ahead of time typechecker---checking here can
       * make it harder to convert a base class or interface to <?hh,
       * because derived classes that are still <?php would all need
       * to be modified.
       */
      return true;
    }
    return (m_typeName == other.m_typeName
            || (m_typeName != nullptr && other.m_typeName != nullptr
                && m_typeName->isame(other.m_typeName)));
  }

  // General check for any constraint.
  bool check(TypedValue* tv, const Func* func) const;

  // Check a constraint when !isObjectOrTypeAlias().
  bool checkPrimitive(DataType dt) const;

  // Checks a constraint that is a type alias when we know tv is or is
  // not an object.
  bool checkTypeAliasObj(const TypedValue* tv) const;
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
  struct Type {
    DataType dt;
    MetaType metatype;
  };
  typedef hphp_hash_map<const StringData*, Type,
                        string_data_hash, string_data_isame> TypeMap;

private:
  static TypeMap s_typeNamesToTypes;

private:
  void init();
  void selfToTypeName(const Func* func, const StringData **typeName) const;
  void parentToTypeName(const Func* func, const StringData **typeName) const;

private:
  // m_type represents the DataType to check on.  We don't know
  // whether a bare name is a class/interface name or a type alias, so
  // when this is set to KindOfObject we may have to look up a type
  // alias name and test for a different DataType.
  Type m_type;
  Flags m_flags;
  const StringData* m_typeName;
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
