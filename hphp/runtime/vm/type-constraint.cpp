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

#include "hphp/runtime/vm/type-constraint.h"

#include <folly/Format.h>
#include <folly/MapUtil.h>

#include "hphp/util/trace.h"

#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/ext/std/ext_std_function.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

//////////////////////////////////////////////////////////////////////

TypeConstraint::TypeMap TypeConstraint::s_typeNamesToTypes;

MaybeDataType TypeConstraint::typeNameToMaybeDataType(
    const StringData* typeName) {
  const Type* type = typeNameToType(typeName);
  return type ? type->dt : folly::none;
}

const TypeConstraint::Type* TypeConstraint::typeNameToType(
    const StringData* typeName) {
  if (UNLIKELY(s_typeNamesToTypes.empty())) {
    const struct Pair {
      const StringData* name;
      Type type;
    } pairs[] = {
      { makeStaticString("HH\\bool"),   { KindOfBoolean, MetaType::Precise }},
      { makeStaticString("HH\\int"),    { KindOfInt64,   MetaType::Precise }},
      { makeStaticString("HH\\float"),  { KindOfDouble,  MetaType::Precise }},
      { makeStaticString("HH\\string"), { KindOfString,  MetaType::Precise }},
      { makeStaticString("array"),      { KindOfArray,   MetaType::Precise }},
      { makeStaticString("HH\\resource"), { KindOfResource,
                                                         MetaType::Precise }},
      { makeStaticString("HH\\num"),    { KindOfDouble,  MetaType::Number }},
      { makeStaticString("HH\\arraykey"), { KindOfString, MetaType::ArrayKey }},
      { makeStaticString("self"),       { KindOfObject,  MetaType::Self }},
      { makeStaticString("parent"),     { KindOfObject,  MetaType::Parent }},
      { makeStaticString("callable"),   { KindOfObject,  MetaType::Callable }},
    };
    for (unsigned i = 0; i < sizeof(pairs) / sizeof(Pair); ++i) {
      s_typeNamesToTypes[pairs[i].name] = pairs[i].type;
    }
  }
  return folly::get_ptr(s_typeNamesToTypes, typeName);
}

//////////////////////////////////////////////////////////////////////

void TypeConstraint::init() {
  if (isTypeVar()) {
    // We kept the type variable type constraint to correctly check child
    // classes implementing abstract methods or interfaces.
    m_type.dt = folly::none;
    m_type.metatype = MetaType::Precise;
    return;
  }

  if (m_typeName == nullptr) {
    m_type.dt = folly::none;
    m_type.metatype = MetaType::Precise;
    return;
  }

  Type dtype;
  TRACE(5, "TypeConstraint: this %p type %s, nullable %d\n",
        this, m_typeName->data(), isNullable());
  auto const mptr = typeNameToType(m_typeName);
  if (mptr) dtype = *mptr;
  if (!mptr ||
      !(isHHType() || dtype.dt == KindOfArray ||
        dtype.dt == KindOfBoolean ||
        dtype.dt == KindOfString ||
        dtype.dt == KindOfInt64 ||
        dtype.dt == KindOfDouble ||
        dtype.dt == KindOfResource ||
        dtype.metatype == MetaType::ArrayKey ||
        dtype.metatype == MetaType::Number ||
        dtype.metatype == MetaType::Parent ||
        dtype.metatype == MetaType::Self ||
        dtype.metatype == MetaType::Callable)) {
    TRACE(5, "TypeConstraint: this %p no such type %s, treating as object\n",
          this, m_typeName->data());
    m_type = { KindOfObject, MetaType::Precise };
    m_namedEntity = NamedEntity::get(m_typeName);
    TRACE(5, "TypeConstraint: NamedEntity: %p\n", m_namedEntity);
    return;
  }
  m_type = dtype;
  assert(m_type.dt != KindOfStaticString);
  assert(IMPLIES(isParent(), m_type.dt == KindOfObject));
  assert(IMPLIES(isSelf(), m_type.dt == KindOfObject));
  assert(IMPLIES(isCallable(), m_type.dt == KindOfObject));
}

std::string TypeConstraint::displayName(const Func* func /*= nullptr*/) const {
  const StringData* tn = typeName();
  std::string name;
  if (isSoft()) {
    name += '@';
  }
  if (isNullable() && isExtended()) {
    name += '?';
  }
  if (func && isSelf()) {
    selfToTypeName(func, &tn);
    name += tn->data();
  } else if (func && isParent()) {
    parentToTypeName(func, &tn);
    name += tn->data();
  } else {
    const char* str = tn->data();
    auto len = tn->size();
    if (len > 3 && tolower(str[0]) == 'h' && tolower(str[1]) == 'h' &&
        str[2] == '\\') {
      bool strip = false;
      const char* stripped = str + 3;
      switch (len - 3) {
        case 3:
          strip = (!strcasecmp(stripped, "int") ||
                   !strcasecmp(stripped, "num"));
          break;
        case 4: strip = !strcasecmp(stripped, "bool"); break;
        case 5: strip = !strcasecmp(stripped, "float"); break;
        case 6: strip = !strcasecmp(stripped, "string"); break;
        case 8:
          strip = (!strcasecmp(stripped, "resource") ||
                   !strcasecmp(stripped, "arraykey"));
          break;
        default:
          break;
      }
      if (strip) {
        str = stripped;
      }
    }
    name += str;
  }
  return name;
}

bool TypeConstraint::compat(const TypeConstraint& other) const {
  if (other.isExtended() || isExtended()) {
    /*
     * Rely on the ahead of time typechecker---checking here can
     * make it harder to convert a base class or interface to <?hh,
     * because derived classes that are still <?php would all need
     * to be modified.
     */
    return true;
  }

  if (m_typeName == other.m_typeName) {
    return true;
  }

  if (m_typeName && other.m_typeName) {
    if (m_typeName->isame(other.m_typeName)) {
      return true;
    }

    const Class* cls = Unit::lookupClass(m_typeName);
    const Class* otherCls = Unit::lookupClass(other.m_typeName);

    return cls && otherCls && cls == otherCls;
  }

  return false;
}

namespace {

/*
 * Look up a TypeAliasReq for the supplied NamedEntity (which must be the
 * NamedEntity for `name'), invoking autoload if necessary for types but not
 * for classes.
 *
 * We don't need to autoload classes because it is impossible to have an
 * instance of a class if it's not defined.  However, we need to autoload
 * typedefs because they can affect whether VerifyParamType would succeed.
 */
const TypeAliasReq* getTypeAliasWithAutoload(const NamedEntity* ne,
                                             const StringData* name) {
  auto def = ne->getCachedTypeAlias();
  if (!def) {
    String nameStr(const_cast<StringData*>(name));
    if (!AutoloadHandler::s_instance->autoloadType(nameStr)) {
      return nullptr;
    }
    def = ne->getCachedTypeAlias();
  }
  return def;
}

/*
 * Look up a TypeAliasReq or a Class for the supplied NamedEntity
 * (which must be the NamedEntity for `name'), invoking autoload if
 * necessary.
 *
 * This is useful when looking up a type annotation that could be either a
 * type alias or an enum class; enum classes are strange in that it
 * *is* possible to have an instance of them even if they are not defined.
 */
std::pair<const TypeAliasReq*, Class*> getTypeAliasOrClassWithAutoload(
    const NamedEntity* ne,
    const StringData* name) {

  auto def = ne->getCachedTypeAlias();
  Class *klass = nullptr;
  if (!def) {
    klass = Unit::lookupClass(ne);
    // We don't have the class or the typedef, so autoload.
    if (!klass) {
      String nameStr(const_cast<StringData*>(name));
      if (AutoloadHandler::s_instance->autoloadClassOrType(nameStr)) {
        // Autoload succeeded, try to grab a typedef and if that doesn't work,
        // a class.
        def = ne->getCachedTypeAlias();
        if (!def) {
          klass = Unit::lookupClass(ne);
        }
      }
    }
  }

  return std::make_pair(def, klass);
}

}

MaybeDataType TypeConstraint::underlyingDataTypeResolved() const {
  assert(!isSelf() && !isParent());
  if (!hasConstraint()) return folly::none;

  auto t = underlyingDataType();

  // If we aren't a class or type alias, nothing special to do.
  if (!isObjectOrTypeAlias()) return t;

  auto p = getTypeAliasOrClassWithAutoload(m_namedEntity, m_typeName);
  auto td = p.first;
  auto c = p.second;

  // See if this is a type alias.
  if (td) {
    if (td->kind != KindOfObject) {
      t = td->kind;
    } else {
      c = td->klass;
    }
  }

  // If the underlying type is a class, see if it is an enum and get that.
  if (c && isEnum(c)) {
    t = c->enumBaseTy();
  }

  return t;
}

bool TypeConstraint::checkTypeAliasNonObj(const TypedValue* tv) const {
  assert(tv->m_type != KindOfObject); // this checks when tv is not an object
  assert(!isSelf() && !isParent());

  auto p = getTypeAliasOrClassWithAutoload(m_namedEntity, m_typeName);
  auto td = p.first;
  auto c = p.second;

  // Common case is that we actually find the alias:
  if (td) {
    if (td->nullable && tv->m_type == KindOfNull) return true;
    return td->any || equivDataTypes(td->kind, tv->m_type);
  }

  // Otherwise, this isn't a proper type alias, but it *might* be a
  // first-class enum. Check if the type is an enum and check the
  // constraint if it is. We only need to do this when the underlying
  // type is not an object, since only int and string can be enums.
  if (c && isEnum(c)) {
    auto dt = c->enumBaseTy();
    // For an enum, if the underlying type is mixed, we still require
    // it is either an int or a string!
    if (dt) {
      return equivDataTypes(*dt, tv->m_type);
    } else {
      return IS_INT_TYPE(tv->m_type) || IS_STRING_TYPE(tv->m_type);
    }
  }
  return false;
}

bool TypeConstraint::checkTypeAliasObj(const TypedValue* tv) const {
  assert(tv->m_type == KindOfObject); // this checks when tv is an object
  assert(!isSelf() && !isParent() && !isCallable());

  auto const td = getTypeAliasWithAutoload(m_namedEntity, m_typeName);
  if (!td) return false;
  if (td->nullable && tv->m_type == KindOfNull) return true;
  if (td->kind != KindOfObject) return td->any;
  return td->klass && tv->m_data.pobj->instanceof(td->klass);
}

bool TypeConstraint::check(TypedValue* tv, const Func* func) const {
  assert(hasConstraint());

  // This is part of the interpreter runtime; perf matters.
  if (tv->m_type == KindOfRef) {
    tv = tv->m_data.pref->tv();
  }
  if (isNullable() && tv->m_type == KindOfNull) return true;

  if (isNumber()) {
    return IS_INT_TYPE(tv->m_type) || IS_DOUBLE_TYPE(tv->m_type);
  }

  if (isArrayKey()) {
    return IS_INT_TYPE(tv->m_type) || IS_STRING_TYPE(tv->m_type);
  }

  if (tv->m_type == KindOfObject) {
    if (!isObjectOrTypeAlias()) return false;
    // Perfect match seems common enough to be worth skipping the hash
    // table lookup.
    if (m_typeName->isame(tv->m_data.pobj->getVMClass()->name())) {
      if (isProfileRequest()) InstanceBits::profile(m_typeName);
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
        return HHVM_FN(is_callable)(tvAsCVarRef(tv));
      }
    } else {
      // We can't save the Class* since it moves around from request
      // to request.
      assert(m_namedEntity);
      c = Unit::lookupClass(m_namedEntity);
    }
    if (isProfileRequest() && c) {
      InstanceBits::profile(c->preClass()->name());
    }
    if (c && tv->m_data.pobj->instanceof(c)) {
      return true;
    }
    return !selfOrParentOrCallable && checkTypeAliasObj(tv);
  }

  if (isObjectOrTypeAlias()) {
    do {
      switch (tv->m_type) {
        case KindOfInt64:
          if (interface_supports_int(m_typeName)) {
            return true;
          }
          continue;

        case KindOfDouble:
          if (interface_supports_double(m_typeName)) {
            return true;
          }
          continue;

        case KindOfStaticString:
        case KindOfString:
          if (interface_supports_string(m_typeName)) {
            return true;
          }
          continue;

        case KindOfArray:
          if (interface_supports_array(m_typeName)) {
            return true;
          }
          continue;

        case KindOfUninit:
        case KindOfNull:
        case KindOfBoolean:
        case KindOfObject:
        case KindOfResource:
          continue;

        case KindOfRef:
        case KindOfClass:
          break;
      }
      not_reached();
    } while (0);

    if (isCallable()) {
      return HHVM_FN(is_callable)(tvAsCVarRef(tv));
    }
    return isPrecise() && checkTypeAliasNonObj(tv);
  }

  return m_type.dt && equivDataTypes(*m_type.dt, tv->m_type);
}

bool TypeConstraint::checkPrimitive(DataType dt) const {
  assert(m_type.dt != KindOfObject);
  assert(dt != KindOfRef);
  if (isNullable() && dt == KindOfNull) return true;
  if (isNumber()) { return IS_INT_TYPE(dt) || IS_DOUBLE_TYPE(dt); }
  if (isArrayKey()) { return IS_INT_TYPE(dt) || IS_STRING_TYPE(dt); }
  return m_type.dt && equivDataTypes(*m_type.dt, dt);
}

static const char* describe_actual_type(const TypedValue* tv, bool isHHType) {
  tv = tvToCell(tv);
  switch (tv->m_type) {
    case KindOfUninit:        return "undefined variable";
    case KindOfNull:          return "null";
    case KindOfBoolean:       return "bool";
    case KindOfInt64:         return "int";
    case KindOfDouble:        return isHHType ? "float" : "double";
    case KindOfStaticString:
    case KindOfString:        return "string";
    case KindOfArray:         return "array";
    case KindOfObject:        return tv->m_data.pobj->o_getClassName().c_str();
    case KindOfResource:      return tv->m_data.pres->o_getClassName().c_str();

    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

void TypeConstraint::verifyFail(const Func* func, TypedValue* tv,
                                int id) const {
  VMRegAnchor _;
  std::string name = displayName(func);
  auto const givenType = describe_actual_type(tv, isHHType());
  // Handle return type constraint failures
  if (id == ReturnId) {
    std::string msg;
    if (func->isClosureBody()) {
      msg =
        folly::format(
          "Value returned from {}closure must be of type {}, {} given",
          func->isAsync() ? "async " : "",
          name,
          givenType
        ).str();
    } else {
      msg =
        folly::format(
          "Value returned from {}{} {}() must be of type {}, {} given",
          func->isAsync() ? "async " : "",
          func->preClass() ? "method" : "function",
          func->fullName()->data(),
          name,
          givenType
        ).str();
    }
    if (RuntimeOption::EvalCheckReturnTypeHints >= 2 && !isSoft() &&
        (!func->isClosureBody() ||
         !RuntimeOption::EvalSoftClosureReturnTypeHints)) {
      raise_return_typehint_error(msg);
    } else {
      raise_debugging(msg);
    }
    return;
  }
  // Handle implicit collection->array conversion for array parameter type
  // constraints
  auto c = tvToCell(tv);
  if (isArray() && !isSoft() && !func->mustBeRef(id) &&
      c->m_type == KindOfObject && c->m_data.pobj->isCollection()) {
    // To ease migration, the 'array' type constraint will implicitly cast
    // collections to arrays, provided the type constraint is not soft and
    // the parameter is not by reference. We raise a notice to let the user
    // know that there was a type mismatch and that an implicit conversion
    // was performed.
    raise_notice(
      folly::format(
        "Argument {} to {}() must be of type {}, {} given; argument {} was "
        "implicitly cast to array",
        id + 1, func->fullName()->data(), name, givenType, id + 1
      ).str()
    );
    tvCastToArrayInPlace(tv);
    return;
  }
  // Handle parameter type constraint failures
  if (isExtended() && isSoft()) {
    // Soft extended type hints raise warnings instead of recoverable
    // errors, to ease migration.
    raise_debugging(
      folly::format(
        "Argument {} to {}() must be of type {}, {} given",
        id + 1, func->fullName()->data(), name, givenType
      ).str()
    );
  } else if (isExtended() && isNullable()) {
    raise_typehint_error(
      folly::format(
        "Argument {} to {}() must be of type {}, {} given",
        id + 1, func->fullName()->data(), name, givenType
      ).str()
    );
  } else {
    auto cls = Unit::lookupClass(m_typeName);
    if (cls && isInterface(cls)) {
      raise_typehint_error(
        folly::format(
          "Argument {} passed to {}() must implement interface {}, {} given",
          id + 1, func->fullName()->data(), name, givenType
        ).str()
      );
    } else {
      raise_typehint_error(
        folly::format(
          "Argument {} passed to {}() must be an instance of {}, {} given",
          id + 1, func->fullName()->data(), name, givenType
        ).str()
      );
    }
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

//////////////////////////////////////////////////////////////////////

}
