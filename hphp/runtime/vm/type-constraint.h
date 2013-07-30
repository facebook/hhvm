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
#ifndef incl_HPHP_TYPE_CONSTRAINT_H_
#define incl_HPHP_TYPE_CONSTRAINT_H_

#include <string>
#include <tr1/functional>

#include "hphp/runtime/base/types.h"
#include "hphp/util/case_insensitive.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/type-profile.h"

namespace HPHP {

class Func;

class TypeConstraint {
public:
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
  };

private:
  enum class MetaType {
    Precise,
    Self,
    Parent,
    Callable
  };

  struct Type {
    DataType m_dt;
    MetaType m_metatype;
    constexpr bool isParent() const {
      return m_metatype == MetaType::Parent;
    }
    constexpr bool isSelf() const {
      return m_metatype == MetaType::Self;
    }
    constexpr bool isCallable() const {
      return m_metatype == MetaType::Callable;
    }
    constexpr bool isPrecise() const {
      return m_metatype == MetaType::Precise;
    }
  };

  // m_type represents the DataType to check on.  We don't know
  // whether a bare name is a class/interface name or a typedef, so
  // when this is set to KindOfObject we may have to look up a typedef
  // name and test for a different DataType.
  Type m_type;
  Flags m_flags;
  const StringData* m_typeName;
  const NamedEntity* m_namedEntity;
  typedef hphp_hash_map<const StringData*, Type,
                        string_data_hash, string_data_isame> TypeMap;
  static TypeMap s_typeNamesToTypes;

  void init();

public:
  void verifyFail(const Func* func, int paramNum, const TypedValue* tv) const;

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

  TypeConstraint(const TypeConstraint&) = default;
  TypeConstraint& operator=(const TypeConstraint&) = default;

  bool hasConstraint() const { return m_typeName; }

  const StringData* typeName() const { return m_typeName; }
  const NamedEntity* namedEntity() const { return m_namedEntity; }

  bool nullable() const { return m_flags & Nullable; }
  bool hhType() const { return m_flags & HHType; }
  Flags flags() const { return m_flags; }

  bool isSelf() const {
    return m_type.isSelf();
  }

  bool isParent() const {
    return m_type.isParent();
  }

  bool isCallable() const {
    return m_type.isCallable();
  }

  bool isPrecise() const {
    return m_type.isPrecise();
  }

  bool isObjectOrTypedef() const {
    assert(IMPLIES(isParent(), m_type.m_dt == KindOfObject));
    assert(IMPLIES(isSelf(), m_type.m_dt == KindOfObject));
    assert(IMPLIES(isCallable(), m_type.m_dt == KindOfObject));
    return m_type.m_dt == KindOfObject;
  }

  bool isExtended() const { return m_flags & ExtendedHint; }

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

  static bool equivDataTypes(DataType t1, DataType t2) {
    return
      (t1 == t2) ||
      (IS_STRING_TYPE(t1) && IS_STRING_TYPE(t2)) ||
      (IS_NULL_TYPE(t1) && IS_NULL_TYPE(t2));
  }

  // General check for any constraint.
  bool check(const TypedValue* tv, const Func* func) const;

  // Check a constraint when !isObjectOrTypedef().
  bool checkPrimitive(DataType dt) const;

  // Typedef checks when we know tv is or is not an object.
  bool checkTypedefObj(const TypedValue* tv) const;
  bool checkTypedefNonObj(const TypedValue* tv) const;

  // NB: will throw if the check fails.
  void verify(const TypedValue* tv,
              const Func* func, int paramNum) const {
    if (UNLIKELY(!check(tv, func))) {
      verifyFail(func, paramNum, tv);
    }
  }

  // Can not be private as it needs to be used by the translator
  void selfToClass(const Func* func, const Class **cls) const;
  void parentToClass(const Func* func, const Class **cls) const;

private:
  void selfToTypeName(const Func* func, const StringData **typeName) const;
  void parentToTypeName(const Func* func, const StringData **typeName) const;
};

inline TypeConstraint::Flags
operator|(TypeConstraint::Flags a, TypeConstraint::Flags b) {
  return TypeConstraint::Flags(static_cast<int>(a) | static_cast<int>(b));
}

}

#endif
