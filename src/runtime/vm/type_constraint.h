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

namespace HPHP {
namespace VM {

class Func;

class TypeConstraint {
protected:
  DataType m_baseType;
  bool m_nullable;
  const StringData* m_typeName;
  const NamedEntity* m_namedEntity;
  typedef hphp_hash_map<const StringData*, DataType,
                        string_data_hash, string_data_isame> TypeMap;
  static TypeMap s_typeNamesToTypes;

public:
  void verifyFail(const Func* func, int paramNum, const TypedValue* tv) const;

  explicit TypeConstraint(const StringData* typeName=NULL, bool nullable=false);

  bool exists() const { return m_typeName; }

  const StringData* typeName() const { return m_typeName; }

  bool nullable() const { return m_nullable; }

  bool isSelf() const {
    return m_baseType == KindOfSelf;
  }

  bool isParent() const {
    return m_baseType == KindOfParent;
  }

  bool isObject() const {
    return m_baseType == KindOfObject ||
           m_baseType == KindOfSelf || m_baseType == KindOfParent;
  }

  bool compat(const TypeConstraint& other) const {
    if (!hphpiCompat) {
      // php 5.4.0RC6 allows 'int' compatible to Int but not integer
      return (m_typeName == other.m_typeName
              || (m_typeName != NULL && other.m_typeName != NULL
                  && m_typeName->isame(other.m_typeName)));
    } else {
      // For now, be compatible with hphpi: int $x != Int $x
      return (m_typeName == other.m_typeName
              || (m_typeName != NULL && other.m_typeName != NULL
                  && m_typeName->same(other.m_typeName)));
    }
  }

  inline static bool equivDataTypes(DataType t1, DataType t2) {
    return
      (t1 == t2) ||
      (IS_STRING_TYPE(t1) && IS_STRING_TYPE(t2)) ||
      (IS_NULL_TYPE(t1) && IS_NULL_TYPE(t2));
  }

  bool check(const TypedValue* tv, const Func* func) const {
    ASSERT(exists());

    // This is part of the interpreter runtime; perf matters.
    if (tv->m_type == KindOfRef) {
      tv = tv->m_data.pref->tv();
    }
    if (m_nullable && IS_NULL_TYPE(tv->m_type)) return true;

    if (tv->m_type == KindOfObject) {
      if (!isObject()) return false;
      // Perfect match seems common enough to be worth skipping the hash
      // table lookup.
      if (m_typeName->isame(tv->m_data.pobj->getVMClass()->name())) return true;
      const Class *c = NULL;
      if (isSelf() || isParent()) {
        if (isSelf()) {
          selfToClass(func, &c);
        } else if (isParent()) {
          parentToClass(func, &c);
        }
      } else {
        // We can't save the Class* since it moves around from request
        // to request.
        ASSERT(m_namedEntity);
        c = Unit::lookupClass(m_namedEntity);
      }
      return c && tv->m_data.pobj->instanceof(c);
    }
    return equivDataTypes(m_baseType, tv->m_type);
  }

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
}



#endif /* TYPE_CONSTRAINT_H_ */
