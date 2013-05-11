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

#include <util/base.h>
#include <util/trace.h>
#include <runtime/ext/ext_function.h>
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
      Type type;
    } pairs[] = {
      { StringData::GetStaticString("bool"),     { KindOfBoolean, Precise }},
      { StringData::GetStaticString("boolean"),  { KindOfBoolean, Precise }},

      { StringData::GetStaticString("int"),      { KindOfInt64, Precise }},
      { StringData::GetStaticString("integer"),  { KindOfInt64, Precise }},

      { StringData::GetStaticString("real"),     { KindOfDouble, Precise }},
      { StringData::GetStaticString("double"),   { KindOfDouble, Precise }},
      { StringData::GetStaticString("float"),    { KindOfDouble, Precise }},

      { StringData::GetStaticString("string"),   { KindOfString, Precise }},

      { StringData::GetStaticString("array"),    { KindOfArray, Precise }},

      { StringData::GetStaticString("self"),     { KindOfObject, Self }},
      { StringData::GetStaticString("parent"),   { KindOfObject, Parent }},
      { StringData::GetStaticString("callable"), { KindOfObject, Callable }},
    };
    for (unsigned i = 0; i < sizeof(pairs) / sizeof(Pair); ++i) {
      s_typeNamesToTypes[pairs[i].name] = pairs[i].type;
    }
  }

  if (typeName == nullptr) {
    m_type.m_dt = KindOfInvalid;
    m_type.m_metatype = Precise;
    return;
  }

  Type dtype;
  TRACE(5, "TypeConstraint: this %p type %s, nullable %d\n",
        this, typeName->data(), nullable);
  if (!mapGet(s_typeNamesToTypes, typeName, &dtype)) {
    TRACE(5, "TypeConstraint: this %p no such type %s, treating as object\n",
          this, typeName->data());
    m_type = { KindOfObject, Precise };
    m_namedEntity = Unit::GetNamedEntity(typeName);
    TRACE(5, "TypeConstraint: NamedEntity: %p\n", m_namedEntity);
    return;
  }
  if (RuntimeOption::EnableHipHopSyntax || dtype.m_dt == KindOfArray ||
      dtype.isParent() || dtype.isSelf()) {
    m_type = dtype;
  } else {
    m_type = { KindOfObject, Precise };
  }
  assert(m_type.m_dt != KindOfStaticString);
  assert(IMPLIES(isParent(), m_type.m_dt == KindOfObject));
  assert(IMPLIES(isSelf(), m_type.m_dt == KindOfObject));
}

/*
 * Note:
 *
 * We don't need to autoload classes because you can't have an
 * instance of a class if it's not defined.  However, we need to
 * autoload typedefs because they can affect whether the
 * VerifyParamType would succeed.
 */
static NameDef getNameDefWithAutoload(const NamedEntity* ne,
                                      const StringData* name) {
  NameDef def = ne->getCachedNameDef();
  if (!def) {
    String nameStr(const_cast<StringData*>(name));
    if (!AutoloadHandler::s_instance->autoloadType(nameStr)) {
      return NameDef();
    }
    def = ne->getCachedNameDef();
  }
  return def;
}

bool TypeConstraint::checkTypedefNonObj(const TypedValue* tv) const {
  assert(tv->m_type != KindOfObject); // this checks when tv is not an object
  assert(!isSelf() && !isParent());

  NameDef def = getNameDefWithAutoload(m_namedEntity, m_typeName);
  if (!def) return false;
  Typedef* td = def.asTypedef();
  return td && equivDataTypes(td->m_kind, tv->m_type);
}

bool TypeConstraint::checkTypedefObj(const TypedValue* tv) const {
  assert(tv->m_type == KindOfObject); // this checks when tv is an object
  assert(!isSelf() && !isParent() && !isCallable());

  NameDef def = getNameDefWithAutoload(m_namedEntity, m_typeName);
  if (!def) return false;
  Class* c = def.asClass();
  return c && tv->m_data.pobj->instanceof(c);
}

bool
TypeConstraint::check(const TypedValue* tv, const Func* func) const {
  assert(exists());

  // This is part of the interpreter runtime; perf matters.
  if (tv->m_type == KindOfRef) {
    tv = tv->m_data.pref->tv();
  }
  if (m_nullable && IS_NULL_TYPE(tv->m_type)) return true;

  if (tv->m_type == KindOfObject) {
    if (!isObjectOrTypedef()) return false;
    // Perfect match seems common enough to be worth skipping the hash
    // table lookup.
    if (m_typeName->isame(tv->m_data.pobj->getVMClass()->name())) {
      if (shouldProfile()) Class::profileInstanceOf(m_typeName);
      return true;
    }
    const Class *c = nullptr;
    const bool selfOrParentOrCallable = isSelf() || isParent() || isCallable();
    if (selfOrParentOrCallable) {
      if (isSelf()) {
        selfToClass(func, &c);
      } else if (isParent()) {
        parentToClass(func, &c);
      } else {
        assert(isCallable());
        return f_is_callable(tvAsCVarRef(tv));
      }
    } else {
      // We can't save the Class* since it moves around from request
      // to request.
      assert(m_namedEntity);
      c = Unit::lookupClass(m_namedEntity);
    }
    if (shouldProfile() && c) {
      Class::profileInstanceOf(c->preClass()->name());
    }
    if (c && tv->m_data.pobj->instanceof(c)) {
      return true;
    }
    return !selfOrParentOrCallable && checkTypedefObj(tv);
  }

  return isObjectOrTypedef() && !isCallable()
    ? checkTypedefNonObj(tv)
    : equivDataTypes(m_type.m_dt, tv->m_type);
}

bool
TypeConstraint::checkPrimitive(DataType dt) const {
  assert(m_type.m_dt != KindOfObject);
  assert(dt != KindOfRef);
  if (m_nullable && IS_NULL_TYPE(dt)) return true;
  return equivDataTypes(m_type.m_dt, dt);
}

void TypeConstraint::verifyFail(const Func* func, int paramNum,
                                const TypedValue* tv) const {
  Transl::VMRegAnchor _;
  std::ostringstream fname;
  if (func->preClass() != nullptr) {
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
  const Class* c = func->cls();
  if (c) {
    *cls = c;
  }
}

void TypeConstraint::selfToTypeName(const Func* func,
                                    const StringData **typeName) const {
  const Class* c = func->cls();
  if (c) {
    *typeName = c->name();
  }
}

void TypeConstraint::parentToClass(const Func* func, const Class **cls) const {
  Class* c1 = func->cls();
  const Class* c2 = c1 ? c1->parent() : nullptr;
  if (c2) {
    *cls = c2;
  }
}

void TypeConstraint::parentToTypeName(const Func* func,
                                      const StringData **typeName) const {
  const Class* c = nullptr;
  parentToClass(func, &c);
  if (c) {
    *typeName = c->name();
  }
}

}
} // HPHP::VM
