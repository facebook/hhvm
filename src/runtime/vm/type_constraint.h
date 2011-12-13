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

namespace HPHP {
namespace VM {

class TypeConstraint {
protected:
  DataType m_baseType;
  const StringData* m_typeName;
  bool m_nullable;
  typedef hphp_string_imap<DataType> TypeMap;
  static TypeMap s_typeNamesToTypes;

public:
  // We'll need a default constructor to use this with the STL containers.
  // We have no business using default-constructed constraints, though.
  TypeConstraint()
    : m_baseType(KindOfInvalid), m_typeName(NULL),
      m_nullable(false) { }

  TypeConstraint(const std::string& typeName, bool nullable);

  bool exists() const {
    return m_typeName;
  }

  const char* typeName() const { return m_typeName->data(); }

  // Passing a closure for ExecutionContext::lookup here gets around an annoying
  // layering issue; without it we'd need to choose between inlining this
  // function and a circular dependency with ExecutionContext.
  typedef std::tr1::function<Class* (const StringData*)> ClassGetter;


  inline static bool equivDataTypes(DataType t1, DataType t2) {
    return
      (t1 == t2) ||
      (IS_STRING_TYPE(t1) && IS_STRING_TYPE(t2)) ||
      (IS_INT_TYPE(t1) && IS_INT_TYPE(t2)) ||
      (IS_NULL_TYPE(t1) && IS_NULL_TYPE(t2));
  }

  bool check(const TypedValue* tv, ClassGetter fLookupClass) const {
    ASSERT(exists());

    // This is part of the interpreter runtime; perf matters.
    if (tv->m_type == KindOfVariant) {
      tv = tv->m_data.ptv;
    }
    if (m_nullable && IS_NULL_TYPE(tv->m_type)) return true;

    if (tv->m_type == KindOfObject) {
      if (m_baseType != KindOfObject) return false;
      // Perfect match seems common enough to be worth skipping the hash
      // table lookup.
      if (m_typeName->isame(tv->m_data.pobj->getVMClass()->name())) return true;
      // Drat. We can't save the Class* since it moves around from request
      // to request.
      Class *c = fLookupClass(m_typeName);
      return c && tv->m_data.pobj->instanceof(c);
    }
    return equivDataTypes(m_baseType, tv->m_type);
  }
};

}
}



#endif /* TYPE_CONSTRAINT_H_ */
