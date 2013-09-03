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

#include "hphp/runtime/vm/type-constraint.h"

#include "hphp/util/base.h"
#include "hphp/util/trace.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/base/builtin-functions.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

namespace {

// TODO(#2322864): this is a hack until we can get rid of the "Xhp"
// psuedo-type.
const StaticString s_xhp("Xhp");
bool blacklistedName(const StringData* sd) {
  if (!sd) return false;
  return sd->isame(s_xhp.get());
}

}

TypeConstraint::TypeMap TypeConstraint::s_typeNamesToTypes;

void TypeConstraint::init() {
  if (UNLIKELY(s_typeNamesToTypes.empty())) {
    const struct Pair {
      const StringData* name;
      Type type;
    } pairs[] = {
      { StringData::GetStaticString("bool"),     { KindOfBoolean,
                                                   MetaType::Precise }},
      { StringData::GetStaticString("boolean"),  { KindOfBoolean,
                                                   MetaType::Precise }},

      { StringData::GetStaticString("int"),      { KindOfInt64,
                                                   MetaType::Precise }},
      { StringData::GetStaticString("integer"),  { KindOfInt64,
                                                   MetaType::Precise }},

      { StringData::GetStaticString("real"),     { KindOfDouble,
                                                   MetaType::Precise }},
      { StringData::GetStaticString("double"),   { KindOfDouble,
                                                   MetaType::Precise }},
      { StringData::GetStaticString("float"),    { KindOfDouble,
                                                   MetaType::Precise }},

      { StringData::GetStaticString("string"),   { KindOfString,
                                                   MetaType::Precise }},

      { StringData::GetStaticString("array"),    { KindOfArray,
                                                   MetaType::Precise }},

      { StringData::GetStaticString("resource"), { KindOfResource,
                                                   MetaType::Precise }},

      { StringData::GetStaticString("self"),     { KindOfObject,
                                                   MetaType::Self }},
      { StringData::GetStaticString("parent"),   { KindOfObject,
                                                   MetaType::Parent }},
      { StringData::GetStaticString("callable"), { KindOfObject,
                                                   MetaType::Callable }},
    };
    for (unsigned i = 0; i < sizeof(pairs) / sizeof(Pair); ++i) {
      s_typeNamesToTypes[pairs[i].name] = pairs[i].type;
    }
  }

  if (m_typeName && isExtended()) {
    assert(nullable() &&
           "Only nullable extended type hints are implemented");
  }

  if (blacklistedName(m_typeName)) {
    m_typeName = nullptr;
  }
  if (m_typeName == nullptr) {
    m_type.m_dt = KindOfInvalid;
    m_type.m_metatype = MetaType::Precise;
    return;
  }

  Type dtype;
  TRACE(5, "TypeConstraint: this %p type %s, nullable %d\n",
        this, m_typeName->data(), nullable());
  if (!mapGet(s_typeNamesToTypes, m_typeName, &dtype) ||
      !(hhType() || dtype.m_dt == KindOfArray || dtype.isParent() ||
        dtype.isSelf())) {
    TRACE(5, "TypeConstraint: this %p no such type %s, treating as object\n",
          this, m_typeName->data());
    m_type = { KindOfObject, MetaType::Precise };
    m_namedEntity = Unit::GetNamedEntity(m_typeName);
    TRACE(5, "TypeConstraint: NamedEntity: %p\n", m_namedEntity);
    return;
  }
  m_type = dtype;
  assert(m_type.m_dt != KindOfStaticString);
  assert(IMPLIES(isParent(), m_type.m_dt == KindOfObject));
  assert(IMPLIES(isSelf(), m_type.m_dt == KindOfObject));
  assert(IMPLIES(isCallable(), m_type.m_dt == KindOfObject));
}

/*
 * Note:
 *
 * We don't need to autoload classes because you can't have an
 * instance of a class if it's not defined.  However, we need to
 * autoload typedefs because they can affect whether the
 * VerifyParamType would succeed.
 */
const TypedefReq* getTypedefWithAutoload(const NamedEntity* ne,
                                         const StringData* name) {
  auto def = ne->getCachedTypedef();
  if (!def) {
    String nameStr(const_cast<StringData*>(name));
    if (!AutoloadHandler::s_instance->autoloadType(nameStr)) {
      return nullptr;
    }
    def = ne->getCachedTypedef();
  }
  return def;
}

bool TypeConstraint::checkTypedefNonObj(const TypedValue* tv) const {
  assert(tv->m_type != KindOfObject); // this checks when tv is not an object
  assert(!isSelf() && !isParent());

  auto const td = getTypedefWithAutoload(m_namedEntity, m_typeName);
  if (!td) return false;
  if (td->nullable && IS_NULL_TYPE(tv->m_type)) return true;
  return td->kind == KindOfAny || equivDataTypes(td->kind, tv->m_type);
}

bool TypeConstraint::checkTypedefObj(const TypedValue* tv) const {
  assert(tv->m_type == KindOfObject); // this checks when tv is an object
  assert(!isSelf() && !isParent() && !isCallable());

  auto const td = getTypedefWithAutoload(m_namedEntity, m_typeName);
  if (!td) return false;
  if (td->nullable && IS_NULL_TYPE(tv->m_type)) return true;
  if (td->kind != KindOfObject) return td->kind == KindOfAny;
  return td->klass && tv->m_data.pobj->instanceof(td->klass);
}

bool
TypeConstraint::check(const TypedValue* tv, const Func* func) const {
  assert(hasConstraint());

  // This is part of the interpreter runtime; perf matters.
  if (tv->m_type == KindOfRef) {
    tv = tv->m_data.pref->tv();
  }
  if (nullable() && IS_NULL_TYPE(tv->m_type)) return true;

  if (tv->m_type == KindOfObject) {
    if (!isObjectOrTypedef()) return false;
    // Perfect match seems common enough to be worth skipping the hash
    // table lookup.
    if (m_typeName->isame(tv->m_data.pobj->getVMClass()->name())) {
      if (shouldProfile()) InstanceBits::profile(m_typeName);
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
      InstanceBits::profile(c->preClass()->name());
    }
    if (c && tv->m_data.pobj->instanceof(c)) {
      return true;
    }
    return !selfOrParentOrCallable && checkTypedefObj(tv);
  }

  if (isObjectOrTypedef()) {
    switch (tv->m_type) {
      case KindOfArray:
        if (interface_supports_array(m_typeName)) {
          return true;
        }
        break;
      case KindOfString:
      case KindOfStaticString:
        if (interface_supports_string(m_typeName)) {
          return true;
        }
        break;
      case KindOfInt64:
        if (interface_supports_int(m_typeName)) {
          return true;
        }
        break;
      case KindOfDouble:
        if (interface_supports_double(m_typeName)) {
          return true;
        }
        break;
      default:
        break;
    }

    if (isCallable()) {
      return f_is_callable(tvAsCVarRef(tv));
    }
    return isPrecise() && checkTypedefNonObj(tv);
  }

  return equivDataTypes(m_type.m_dt, tv->m_type);
}

bool
TypeConstraint::checkPrimitive(DataType dt) const {
  assert(m_type.m_dt != KindOfObject);
  assert(dt != KindOfRef);
  if (nullable() && IS_NULL_TYPE(dt)) return true;
  return equivDataTypes(m_type.m_dt, dt);
}

static const char* describe_actual_type(const TypedValue* tv) {
  tv = tvToCell(tv);
  switch (tv->m_type) {
  case KindOfUninit:
  case KindOfNull:          return "null";
  case KindOfBoolean:       return "bool";
  case KindOfInt64:         return "int";
  case KindOfDouble:        return "double";
  case KindOfStaticString:
  case KindOfString:        return "string";
  case KindOfArray:         return "array";
  case KindOfObject:
    return tv->m_data.pobj->o_getClassName().c_str();
  case KindOfResource:
    return tv->m_data.pres->o_getClassName().c_str();
  default:
    assert(false);
  }
  not_reached();
}

void TypeConstraint::verifyFail(const Func* func, int paramNum,
                                const TypedValue* tv) const {
  Transl::VMRegAnchor _;
  std::ostringstream fname;
  fname << func->fullName()->data() << "()";
  const StringData* tn = typeName();
  if (isSelf()) {
    selfToTypeName(func, &tn);
  } else if (isParent()) {
    parentToTypeName(func, &tn);
  }

  auto const givenType = describe_actual_type(tv);

  if (isExtended()) {
    // Extended type hints raise warnings instead of recoverable
    // errors for now, to ease migration (we used to not check these
    // at all at runtime).
    assert(nullable() &&
           "only nullable extended type hints are currently supported");
    raise_warning(
      "Argument %d to %s must be of type ?%s, %s given",
      paramNum + 1, fname.str().c_str(), tn->data(), givenType);
  } else {
    raise_recoverable_error(
      "Argument %d passed to %s must be an instance of %s, %s given",
      paramNum + 1, fname.str().c_str(), tn->data(), givenType);
  }
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

} // HPHP::VM
