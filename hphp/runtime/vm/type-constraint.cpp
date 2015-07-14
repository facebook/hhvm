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

const StaticString s___invoke("__invoke");

void TypeConstraint::init() {
  if (m_typeName == nullptr || isTypeVar() || isTypeConstant()) {
    m_type = Type::Mixed;
    return;
  }
  TRACE(5, "TypeConstraint: this %p type %s, nullable %d\n",
        this, m_typeName->data(), isNullable());
  auto const mptr = nameToAnnotType(m_typeName);
  if (mptr) {
    m_type = *mptr;
    assert(getAnnotDataType(m_type) != KindOfStaticString);
    return;
  }
  TRACE(5, "TypeConstraint: this %p no such type %s, treating as object\n",
        this, m_typeName->data());
  m_type = Type::Object;
  m_namedEntity = NamedEntity::get(m_typeName);
  TRACE(5, "TypeConstraint: NamedEntity: %p\n", m_namedEntity);
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
                   !strcasecmp(stripped, "noreturn") ||
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
    VMRegAnchor _;
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
static
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

  assert(!def || !klass);
  return std::make_pair(def, klass);
}

}

MaybeDataType TypeConstraint::underlyingDataTypeResolved() const {
  assert(!isSelf() && !isParent() && !isCallable());
  assert(IMPLIES(
    !hasConstraint() || isTypeVar() || isTypeConstant(),
    isMixed()));

  if (!isPrecise()) return folly::none;

  auto t = underlyingDataType();
  assert(t);

  // If we aren't a class or type alias, nothing special to do.
  if (!isObject()) return t;

  assert(t == KindOfObject);
  auto p = getTypeAliasOrClassWithAutoload(m_namedEntity, m_typeName);
  auto td = p.first;
  auto c = p.second;

  // See if this is a type alias.
  if (td) {
    if (td->type != Type::Object) {
      t = (getAnnotMetaType(td->type) != MetaType::Precise)
        ? folly::none : MaybeDataType(getAnnotDataType(td->type));
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
  assert(tv->m_type != KindOfObject);
  assert(isObject());

  auto p = getTypeAliasOrClassWithAutoload(m_namedEntity, m_typeName);
  auto td = p.first;
  auto c = p.second;

  // Common case is that we actually find the alias:
  if (td) {
    if (td->nullable && tv->m_type == KindOfNull) return true;
    auto result = annotCompat(tv->m_type, td->type,
      td->klass ? td->klass->name() : nullptr);
    switch (result) {
      case AnnotAction::Pass: return true;
      case AnnotAction::Fail: return false;
      case AnnotAction::CallableCheck:
        return is_callable(tvAsCVarRef(tv));
      case AnnotAction::ObjectCheck: break;
    }
    assert(result == AnnotAction::ObjectCheck);
    assert(td->type == AnnotType::Object);
    // Fall through to the check below, since this could be a type
    // alias to an enum type
    c = td->klass;
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

bool TypeConstraint::checkTypeAliasObj(const Class* cls) const {
  assert(isObject() && m_namedEntity && m_typeName);
  // Look up the type alias (autoloading if necessary)
  // and fail if we can't find it
  auto const td = getTypeAliasWithAutoload(m_namedEntity, m_typeName);
  if (!td) {
    return false;
  }
  // We found the type alias, check if an object of type cls
  // is compatible
  switch (getAnnotMetaType(td->type)) {
    case AnnotMetaType::Precise:
      return td->type == AnnotType::Object && td->klass &&
             cls->classof(td->klass);
    case AnnotMetaType::Mixed:
      return true;
    case AnnotMetaType::Callable:
      return cls->lookupMethod(s___invoke.get()) != nullptr;
    case AnnotMetaType::Self:
    case AnnotMetaType::Parent:
    case AnnotMetaType::Number:
    case AnnotMetaType::ArrayKey:
      // Self and Parent should never happen, because type
      // aliases are not allowed to use those MetaTypes
      return false;
  }
  not_reached();
}

bool TypeConstraint::check(TypedValue* tv, const Func* func) const {
  assert(hasConstraint() && !isTypeVar() && !isMixed() && !isTypeConstant());

  // This is part of the interpreter runtime; perf matters.
  if (tv->m_type == KindOfRef) {
    tv = tv->m_data.pref->tv();
  }

  if (isNullable() && tv->m_type == KindOfNull) {
    return true;
  }

  if (tv->m_type == KindOfObject) {
    // Perfect match seems common enough to be worth skipping the hash
    // table lookup.
    const Class *c = nullptr;
    if (isObject()) {
      if (m_typeName->isame(tv->m_data.pobj->getVMClass()->name())) {
        if (isProfileRequest()) InstanceBits::profile(m_typeName);
        return true;
      }
      // We can't save the Class* since it moves around from request
      // to request.
      assert(m_namedEntity);
      c = Unit::lookupClass(m_namedEntity);
    } else {
      switch (metaType()) {
        case MetaType::Self:
          selfToClass(func, &c);
          break;
        case MetaType::Parent:
          parentToClass(func, &c);
          break;
        case MetaType::Callable:
          return is_callable(tvAsCVarRef(tv));
        case MetaType::Precise:
        case MetaType::Number:
        case MetaType::ArrayKey:
          return false;
        case MetaType::Mixed:
          // We assert'd at the top of this function that the
          // metatype cannot be Mixed
          not_reached();
      }
    }
    if (isProfileRequest() && c) {
      InstanceBits::profile(c->preClass()->name());
    }
    if (c && tv->m_data.pobj->instanceof(c)) {
      return true;
    }
    return isObject() && checkTypeAliasObj(tv->m_data.pobj->getVMClass());
  }

  auto const result = annotCompat(tv->m_type, m_type, m_typeName);
  switch (result) {
    case AnnotAction::Pass: return true;
    case AnnotAction::Fail: return false;
    case AnnotAction::CallableCheck:
      return is_callable(tvAsCVarRef(tv));
    case AnnotAction::ObjectCheck:
      assert(isObject());
      return checkTypeAliasNonObj(tv);
  }
  not_reached();
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
    case KindOfObject:        return tv->m_data.pobj->getClassName().c_str();
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
          func->fullName(),
          name,
          givenType
        ).str();
    }
    if (RuntimeOption::EvalCheckReturnTypeHints >= 2 && !isSoft()) {
      raise_return_typehint_error(msg);
    } else {
      raise_warning_unsampled(msg);
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
        id + 1, func->fullName(), name, givenType, id + 1
      ).str()
    );
    tvCastToArrayInPlace(tv);
    return;
  }
  // Handle parameter type constraint failures
  if (isExtended() && isSoft()) {
    // Soft extended type hints raise warnings instead of recoverable
    // errors, to ease migration.
    raise_warning_unsampled(
      folly::format(
        "Argument {} to {}() must be of type {}, {} given",
        id + 1, func->fullName(), name, givenType
      ).str()
    );
  } else if (isExtended() && isNullable()) {
    raise_typehint_error(
      folly::format(
        "Argument {} to {}() must be of type {}, {} given",
        id + 1, func->fullName(), name, givenType
      ).str()
    );
  } else {
    auto cls = Unit::lookupClass(m_typeName);
    if (cls && isInterface(cls)) {
      raise_typehint_error(
        folly::format(
          "Argument {} passed to {}() must implement interface {}, {} given",
          id + 1, func->fullName(), name, givenType
        ).str()
      );
    } else {
      raise_typehint_error(
        folly::format(
          "Argument {} passed to {}() must be an instance of {}, {} given",
          id + 1, func->fullName(), name, givenType
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
