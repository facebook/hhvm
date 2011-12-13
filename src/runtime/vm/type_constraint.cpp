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

#include <util/base.h>
#include <util/trace.h>
#include <runtime/vm/hhbc.h>
#include <runtime/vm/class.h>
#include <runtime/vm/unit.h>
#include <runtime/vm/type_constraint.h>

namespace HPHP {
namespace VM {

TRACE_SET_MOD(runtime);

TypeConstraint::TypeMap TypeConstraint::s_typeNamesToTypes;

TypeConstraint::TypeConstraint(const std::string& typeName,
                               bool nullable) :
  m_typeName(StringData::GetStaticString(typeName)), m_nullable(nullable) {

  if (UNLIKELY(s_typeNamesToTypes.empty())) {
    const struct Pair {
      const char* name;
      DataType dt;
    } pairs[] = {
      { "bool",    KindOfBoolean },
      { "boolean", KindOfBoolean },

      { "int",     KindOfInt64 },
      { "integer", KindOfInt64 },

      { "real",    KindOfDouble },
      { "double",  KindOfDouble },
      { "float",   KindOfDouble },

      { "string",  KindOfString },

      { "array",   KindOfArray },

      { NULL,      KindOfInvalid },
    };
    for (const Pair *p = pairs; p->name; p++) {
      s_typeNamesToTypes[p->name] = p->dt;
    }
  }

  DataType dtype;
  TRACE(5, "TypeConstraint: this %p type %s, nullable %d\n",
        this, typeName.c_str(), nullable);
  if (!mapGet(s_typeNamesToTypes, typeName, &dtype)) {
    TRACE(5, "TypeConstraint: this %p no such type %s, treating as object\n",
          this, typeName.c_str());
    m_baseType = KindOfObject;
    return;
  }
  m_baseType = dtype;
  ASSERT(m_baseType != KindOfStaticString);
  ASSERT(m_baseType != KindOfInt32);
}

}
} // HPHP::VM
