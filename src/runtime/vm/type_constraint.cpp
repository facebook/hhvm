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
#include <runtime/vm/func.h>
#include <runtime/vm/translator/translator-inline.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/vm/type_constraint.h>

namespace HPHP {
namespace VM {

TRACE_SET_MOD(runtime);

TypeConstraint::TypeMap TypeConstraint::s_typeNamesToTypes;

TypeConstraint::TypeConstraint(const StringData* typeName /* = NULL */,
                               bool nullable /* = false */)
    : m_nullable(nullable), m_typeName(typeName), m_namedEntity(0) {
  if (UNLIKELY(s_typeNamesToTypes.empty())) {
    const struct Pair {
      const StringData* name;
      DataType dt;
    } pairs[] = {
      { StringData::GetStaticString("bool"),    KindOfBoolean },
      { StringData::GetStaticString("boolean"), KindOfBoolean },

      { StringData::GetStaticString("int"),     KindOfInt64 },
      { StringData::GetStaticString("integer"), KindOfInt64 },

      { StringData::GetStaticString("real"),    KindOfDouble },
      { StringData::GetStaticString("double"),  KindOfDouble },
      { StringData::GetStaticString("float"),   KindOfDouble },

      { StringData::GetStaticString("string"),  KindOfString },

      { StringData::GetStaticString("array"),   KindOfArray },

      { StringData::GetStaticString("self"),    KindOfSelf },
      { StringData::GetStaticString("parent"),  KindOfParent }
    };
    for (unsigned i = 0; i < sizeof(pairs)/sizeof(Pair); ++i) {
      s_typeNamesToTypes[pairs[i].name] = pairs[i].dt;
    }
  }

  if (typeName == NULL) {
    m_baseType = KindOfInvalid;
    return;
  }
  ASSERT(!typeName->empty());
  DataType dtype;
  TRACE(5, "TypeConstraint: this %p type %s, nullable %d\n",
        this, typeName->data(), nullable);
  if (!mapGet(s_typeNamesToTypes, typeName, &dtype)) {
    TRACE(5, "TypeConstraint: this %p no such type %s, treating as object\n",
          this, typeName->data());
    m_baseType = KindOfObject;
    m_namedEntity = Unit::GetNamedEntity(typeName);
    return;
  }
  m_baseType = RuntimeOption::EnableHipHopSyntax || dtype == KindOfArray ||
               dtype == KindOfSelf || dtype == KindOfParent ?
    dtype : KindOfObject;
  ASSERT(m_baseType != KindOfStaticString);
  ASSERT(m_baseType != KindOfInt32);
}

void TypeConstraint::verifyFail(const Func* func, int paramNum,
                                const TypedValue* tv) const {
  Transl::VMRegAnchor _;
  std::ostringstream fname;
  if (func->preClass() != NULL) {
    fname << func->preClass()->name()->data() << "::"
      << func->name()->data() << "()";
  } else {
    fname << func->name()->data() << "()";
  }
  const StringData* tn = typeName();
  if (isSelf()) {
    selfToTypeName(func, &tn);
  } else if (isParent()) {
    parentToTypeName(func, &tn);
  }
  throw_unexpected_argument_type(paramNum + 1, fname.str().c_str(),
                                 tn->data(), tvAsCVarRef(tv));
}

void TypeConstraint::selfToClass(const Func* func, const Class **cls) const {
  if (hphpiCompat) {
    // hphpi: a typehint self in a trait's method in the class using a trait
    // represents the trait rather than the using class.
    const PreClass* pc = func->preClass();
    if (pc && !(pc->attrs() & AttrTrait)) {
      *cls = func->cls();
    }
    return;
  }
  // PHP 5.4: typehint self in a method in a trait is the class using the trait
  const Class* c = func->cls();
  if (c) {
    *cls = c;
  }
}

void TypeConstraint::selfToTypeName(const Func* func,
                                    const StringData **typeName) const {
  if (hphpiCompat) {
    // hphpi: a typehint self in a trait's method in the class using a trait
    // represents the trait rather than the using class.
    const PreClass* pc = func->preClass();
    if (pc) {
      *typeName = pc->name();
    }
    return;
  }
  // PHP 5.4: typehint self in a trait's method is the class using the trait
  const Class* c = func->cls();
  if (c) {
    *typeName = c->name();
  }
}

void TypeConstraint::parentToClass(const Func* func, const Class **cls) const {
  if (hphpiCompat) {
    return;
  }
  // Match 5.4 for methods defined in classes and traits
  Class* c1 = func->cls();
  const Class* c2 = c1 ? c1->parent() : NULL;
  if (c2) {
    *cls = c2;
  }
}

void TypeConstraint::parentToTypeName(const Func* func,
                                      const StringData **typeName) const {
  const Class* c = NULL;
  parentToClass(func, &c);
  if (c) {
    *typeName = c->name();
  }
}

}
} // HPHP::VM
