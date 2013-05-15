/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#ifndef TYPE_CONSTRAINT_H_
#define TYPE_CONSTRAINT_H_

#include <string>
#include <tr1/functional>

#include <runtime/base/types.h>
#include <util/case_insensitive.h>
#include <runtime/vm/unit.h>
#include <runtime/vm/type_profile.h>

namespace HPHP {

class Func;

class TypeConstraint {
public:
  enum Flags {
    NoFlags = 0x0,
    Nullable = 0x1,
    HHType = 0x2
  };

protected:
  enum MetaType {
    Precise,
    Self,
    Parent,
    Callable
  };

  struct Type {
    DataType m_dt;
    MetaType m_metatype;
    constexpr bool isParent() const {
      return m_metatype == Parent;
    }
    constexpr bool isSelf() const {
      return m_metatype == Self;
    }
    constexpr bool isCallable() const {
      return m_metatype == Callable;
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

  explicit TypeConstraint(const StringData* typeName, Flags flags)
    : m_flags(flags), m_typeName(typeName), m_namedEntity(0) {
    init();
  }

  explicit TypeConstraint(const StringData* typeName = nullptr,
                          bool nullable = false, bool hhType = false)
    : m_flags(NoFlags), m_typeName(typeName), m_namedEntity(0) {
    if (nullable) m_flags = (Flags)(m_flags | Nullable);
    if (hhType) m_flags = (Flags)(m_flags | HHType);
    init();
  }

  bool exists() const { return m_typeName; }

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

  bool isObjectOrTypedef() const {
    assert(IMPLIES(isParent(), m_type.m_dt == KindOfObject));
    assert(IMPLIES(isSelf(), m_type.m_dt == KindOfObject));
    assert(IMPLIES(isCallable(), m_type.m_dt == KindOfObject));
    return m_type.m_dt == KindOfObject;
  }

  bool compat(const TypeConstraint& other) const {
    return (m_typeName == other.m_typeName
            || (m_typeName != nullptr && other.m_typeName != nullptr
                && m_typeName->isame(other.m_typeName)));
  }

  inline static bool equivDataTypes(DataType t1, DataType t2) {
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

}



#endif /* TYPE_CONSTRAINT_H_ */
