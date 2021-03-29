/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <boost/variant.hpp>

#include <folly/Format.h>
#include <folly/MapUtil.h>

#include "hphp/util/trace.h"

#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-util.h"
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
    assertx(getAnnotDataType(m_type) != KindOfPersistentString);
    return;
  }
  if (m_flags & Flags::Resolved) {
    TRACE(5, "TypeConstraint: this %p pre-resolved type %s, treating as %s\n",
          this, m_typeName->data(), tname(getAnnotDataType(m_type)).c_str());
  } else {
    TRACE(5, "TypeConstraint: this %p no such type %s, treating as object\n",
          this, m_typeName->data());
    m_type = Type::Object;
  }
  m_namedEntity = NamedEntity::get(m_typeName);
  TRACE(5, "TypeConstraint: this %p NamedEntity: %p\n",
        this, m_namedEntity.get());
}

bool TypeConstraint::operator==(const TypeConstraint& o) const {
  // The named entity is defined based on the m_typeName and is redundant to
  // include in the equality operation.
  return m_flags == o.m_flags && m_type == o.m_type &&
         m_typeName == o.m_typeName;
}

std::string TypeConstraint::displayName(const Class* context /*= nullptr*/,
                                        bool extra /* = false */) const {
  const StringData* tn = typeName();
  std::string name;
  if (isSoft()) {
    name += '@';
  }
  if ((m_flags & Flags::DisplayNullable) && isExtended()) {
    name += '?';
  }
  if (isSelf()) {
    if (context) tn = context->name();
    name += tn->data();
  } else if (isParent()) {
    if (context) {
      if (auto const parent = context->parent()) {
        tn = parent->name();
      }
    }
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
        case 4:
          strip = (!strcasecmp(stripped, "bool") ||
                   !strcasecmp(stripped, "this") ||
                   !strcasecmp(stripped, "null"));
          break;
        case 5: strip = !strcasecmp(stripped, "float"); break;
        case 6: strip = !strcasecmp(stripped, "string"); break;
        case 7:
          strip = (!strcasecmp(stripped, "nonnull") ||
                   !strcasecmp(stripped, "nothing"));
          break;
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
  if (extra && m_flags & Flags::Resolved && m_type != AnnotType::Object) {
    const char* str = nullptr;
    switch (m_type) {
      case AnnotType::Nothing:  str = "nothing"; break;
      case AnnotType::NoReturn: str = "noreturn"; break;
      case AnnotType::Null:     str = "null"; break;
      case AnnotType::Bool:     str = "bool"; break;
      case AnnotType::Int:      str = "int";  break;
      case AnnotType::Float:    str = "float"; break;
      case AnnotType::String:   str = "string"; break;
      case AnnotType::Resource: str = "resource"; break;
      case AnnotType::Dict:     str = "dict"; break;
      case AnnotType::Vec:      str = "vec"; break;
      case AnnotType::Keyset:   str = "keyset"; break;
      case AnnotType::Number:   str = "num"; break;
      case AnnotType::ArrayKey: str = "arraykey"; break;
      case AnnotType::VecOrDict: str = "vec_or_dict"; break;
      case AnnotType::ArrayLike: str = "AnyArray"; break;
      case AnnotType::Nonnull:  str = "nonnull"; break;
      case AnnotType::Record:    str = "record"; break;
      case AnnotType::Classname: str = "classname"; break;
      case AnnotType::Self:
      case AnnotType::This:
      case AnnotType::Parent:
      case AnnotType::Object:
      case AnnotType::Mixed:
      case AnnotType::Callable:
        break;
    }
    if (str) folly::format(&name, " ({})", str);
  }
  return name;
}

namespace {

const Class* getThis() {
  auto const ar = vmfp();
  if (ar->func()->cls()) {
    if (ar->hasThis()) {
      return ar->getThis()->getVMClass();
    } else {
      assertx(ar->hasClass());
      return ar->getClass();
    }
  }
  return nullptr;
}

/*
 * Look up a TypeAlias for the supplied NamedEntity (which must be the
 * NamedEntity for `name'), invoking autoload if necessary for types but not
 * for classes.
 *
 * We don't need to autoload classes because it is impossible to have an
 * instance of a class if it's not defined.  However, we need to autoload
 * typedefs because they can affect whether VerifyParamType would succeed.
 */
const TypeAlias* getTypeAliasWithAutoload(const NamedEntity* ne,
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
 * Look up a TypeAlias or a Class for the supplied NamedEntity
 * (which must be the NamedEntity for `name'), invoking autoload if
 * necessary.
 *
 * This is useful when looking up a type annotation that could be either a
 * type alias or an enum class; enum classes are strange in that it
 * *is* possible to have an instance of them even if they are not defined.
 */
boost::variant<const TypeAlias*, RecordDesc*, Class*>
getNamedTypeWithAutoload(const NamedEntity* ne,
                         const StringData* name) {

  if (auto def = ne->getCachedTypeAlias()) {
    return def;
  }
  if (auto rec = Unit::lookupRecordDesc(ne)) {
    return rec;
  }
  Class *klass = nullptr;
  klass = Class::lookup(ne);
  // We don't have the class, record or the typedef, so autoload.
  if (!klass) {
    String nameStr(const_cast<StringData*>(name));
    if (AutoloadHandler::s_instance->autoloadNamedType(nameStr)) {
      // Autoload succeeded, try to grab a typedef, or record, or a class.
      if (auto def = ne->getCachedTypeAlias()) {
        return def;
      }
      if (auto rec = Unit::lookupRecordDesc(ne)) {
        return rec;
      }
      klass = Class::lookup(ne);
    }
  }
  return klass;
}
}

MaybeDataType TypeConstraint::underlyingDataTypeResolved() const {
  assertx(!isSelf() && !isParent() && !isCallable());
  assertx(IMPLIES(
    !hasConstraint() || isTypeVar() || isTypeConstant(),
    isMixed()));

  if (!isPrecise()) return folly::none;

  auto t = underlyingDataType();
  assertx(t);

  // If we aren't a class or type alias, nothing special to do.
  if (!isObject()) return t;

  assertx(t == KindOfObject);
  auto p = getNamedTypeWithAutoload(m_namedEntity, m_typeName);

  if (boost::get<RecordDesc*>(&p)) return KindOfRecord;

  auto ptd = boost::get<const TypeAlias*>(&p);
  auto td = ptd ? *ptd : nullptr;
  auto pc = boost::get<Class*>(&p);
  auto c = pc ? *pc : nullptr;

  // See if this is a type alias.
  if (td) {
    if (td->type != Type::Object) {
      auto const metatype = getAnnotMetaType(td->type);
      if (metatype == MetaType::Precise) {
        t = getAnnotDataType(td->type);
      } else {
        t = folly::none;
      }
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

bool TypeConstraint::isMixedResolved() const {
  if (!isCheckable()) return true;
  // isCheckable() implies !isMixed(), so if its not an unresolved object here,
  // we know it cannot be mixed.
  if (!isObject() || isResolved()) return false;
  auto v = getNamedTypeWithAutoload(m_namedEntity, m_typeName);
  auto const pTyAlias = boost::get<const TypeAlias*>(&v);
  return pTyAlias && (*pTyAlias)->type == AnnotType::Mixed;
}

bool TypeConstraint::maybeMixed() const {
  if (!isCheckable()) return true;
  // isCheckable() implies !isMixed(), so if its not an unresolved object here,
  // we know it cannot be mixed.
  if (!isObject() || isResolved()) return false;
  if (auto const def = m_namedEntity->getCachedTypeAlias()) {
    return def->type == AnnotType::Mixed;
  }
  // If its a known class, its definitely not mixed. Otherwise it might be.
  return !Class::lookup(m_namedEntity);
}

bool
TypeConstraint::maybeInequivalentForProp(const TypeConstraint& other) const {
  assertx(validForProp());
  assertx(other.validForProp());

  if (isSoft() != other.isSoft()) return true;

  if (!isCheckable()) return other.isCheckable();
  if (!other.isCheckable()) return true;

  if (isNullable() != other.isNullable()) return true;

  if (isObject()) {
    // Type-hints with the same name should always be the same thing
    return !other.isObject() || !m_typeName->isame(other.m_typeName);
  }
  if (other.isObject()) return true;
  return type() != other.type();
}

bool TypeConstraint::equivalentForProp(const TypeConstraint& other) const {
  assertx(validForProp());
  assertx(other.validForProp());

  if (isSoft() != other.isSoft()) return false;

  if (isObject() && other.isObject() &&
      isNullable() == other.isNullable() &&
      m_typeName->isame(other.m_typeName)) {
    // We can avoid having to resolve the type-hint if they have the same name.
    return true;
  }

  auto const resolve = [&] (const TypeConstraint& tc)
    -> std::tuple<AnnotType, Class*, bool> {
    if (!tc.isCheckable()) {
      return std::make_tuple(AnnotType::Mixed, nullptr, false);
    }

    switch (tc.metaType()) {
      case MetaType::This:
      case MetaType::Number:
      case MetaType::ArrayKey:
      case MetaType::Nonnull:
      case MetaType::VecOrDict:
      case MetaType::ArrayLike:
      case MetaType::Classname:
      case MetaType::Precise:
        if (!tc.isObject()) {
          return std::make_tuple(tc.type(), nullptr, tc.isNullable());
        }
        break;
      case MetaType::Nothing:
      case MetaType::NoReturn:
      case MetaType::Self:
      case MetaType::Parent:
      case MetaType::Callable:
      case MetaType::Mixed:
        always_assert(false);
    }

    assertx(tc.isObject());

    const TypeAlias* tyAlias = nullptr;
    Class* klass = nullptr;
    RecordDesc* rec = nullptr;
    auto v =
      getNamedTypeWithAutoload(tc.m_namedEntity, tc.m_typeName);
    if (auto pT = boost::get<const TypeAlias*>(&v)) {
      tyAlias = *pT;
    }
    if (auto pR = boost::get<RecordDesc*>(&v)) {
      rec = *pR;
    }
    if (auto pK = boost::get<Class*>(&v)) {
      klass = *pK;
    }
    auto nullable = tc.isNullable();
    auto at = AnnotType::Object;
    if (tyAlias) {
      nullable |= tyAlias->nullable;
      at = tyAlias->type;
      klass = (at == Type::Object) ? tyAlias->klass : nullptr;
    }

    if (rec) {
      at = AnnotType::Record;
    }

    if (klass && isEnum(klass)) {
      auto const maybeDt = klass->enumBaseTy();
      at = maybeDt ? enumDataTypeToAnnotType(*maybeDt) : AnnotType::ArrayKey;
      klass = nullptr;
    }

    if (at == AnnotType::Mixed) nullable = false;
    return std::make_tuple(at, klass, nullable);
  };

  auto const resolved1 = resolve(*this);
  auto const resolved2 = resolve(other);
  if (resolved1 != resolved2) return false;

  // The resolutions are the same. However, both could have failed to resolve to
  // anything. In that case, rely on their name.
  auto const resType = std::get<0>(resolved1);
  if (resType == AnnotType::Object && !std::get<1>(resolved1)) {
    return m_typeName->isame(other.m_typeName);
  }
  return true;
}

template <bool Assert, bool ForProp>
bool TypeConstraint::checkNamedTypeNonObj(tv_rval val) const {
  assertx(val.type() != KindOfObject);
  assertx(isObject() || isRecord());

  auto const p = [&]() ->
    boost::variant<const TypeAlias*, RecordDesc*, Class*> {
    if (!Assert) {
      return getNamedTypeWithAutoload(m_namedEntity, m_typeName);
    }
    if (auto const def = m_namedEntity->getCachedTypeAlias()) {
      return def;
    }
    if (auto const rec = Unit::lookupRecordDesc(m_namedEntity)) {
      return rec;
    }
    return Class::lookup(m_namedEntity);
  }();
  auto ptd = boost::get<const TypeAlias*>(&p);
  auto td = ptd ? *ptd : nullptr;
  auto prec = boost::get<RecordDesc*>(&p);
  auto rec = prec ? *prec : nullptr;
  auto pc = boost::get<Class*>(&p);
  auto c = pc ? *pc : nullptr;

  if (Assert && !td && !rec && !c) return true;

  // Common case is that we actually find the alias:
  if (td) {
    assertx(td->type != AnnotType::Self && td->type != AnnotType::Parent);
    if (td->nullable && val.type() == KindOfNull) return true;
    auto result = annotCompat(val.type(), td->type,
                              td->klass ? td->klass->name() : nullptr);
    switch (result) {
      case AnnotAction::Pass: return true;
      case AnnotAction::Fail: return false;
      case AnnotAction::CallableCheck:
        return !ForProp && (Assert || is_callable(tvAsCVarRef(*val)));
      case AnnotAction::ObjectCheck:
        assertx(td->type == AnnotType::Object);
        c = td->klass;
        break;
      case AnnotAction::WarnClass:
      case AnnotAction::ConvertClass:
      case AnnotAction::WarnLazyClass:
      case AnnotAction::ConvertLazyClass:
        return false; // verifyFail will deal with the conversion/warning
      case AnnotAction::ClsMethCheck:
        return false;
      case AnnotAction::RecordCheck:
        assertx(result == AnnotAction::RecordCheck);
        rec = td->rec;
        break;
      case AnnotAction::WarnClassname:
        assertx(isClassType(val.type()) || isLazyClassType(val.type()));
        assertx(RuntimeOption::EvalClassPassesClassname);
        assertx(RuntimeOption::EvalClassnameNotices);
        if (!Assert) raise_notice(Strings::CLASS_TO_CLASSNAME);
        return true;
    }
    assertx (result == AnnotAction::ObjectCheck ||
             result == AnnotAction::RecordCheck);
    // Fall through to the check below, since this could be a type alias to
    // an enum type or a record
  }

  if (rec) {
    return isRecordType(val.type()) && val.val().prec->instanceof(rec);
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
      return equivDataTypes(*dt, val.type());
    } else {
      return isIntType(val.type()) || isStringType(val.type());
    }
  }
  return false;
}

namespace {
template <typename T>
bool isValid(const TypeConstraint*);
template <>
bool isValid<Class>(const TypeConstraint* tc) { return tc->isObject(); }
template <>
bool isValid<RecordDesc>(const TypeConstraint* tc) { return tc->isRecord(); }

template <typename T>
bool isInstanceOf(const T*, const TypeAlias*);
template<>
bool isInstanceOf<Class>(const Class* type, const TypeAlias* td) {
  return td->type == AnnotType::Object && td->klass &&
    type->classof(td->klass);
}
template<>
bool isInstanceOf<RecordDesc>(const RecordDesc* type, const TypeAlias* td) {
  return td->type == AnnotType::Record && td->rec &&
    type->recordDescOf(td->rec);
}

template<typename T>
bool methodExists(const T*);
template<>
bool methodExists<Class>(const Class* cls) {
  return cls->lookupMethod(s___invoke.get()) != nullptr;
}
template<> bool methodExists<RecordDesc>(const RecordDesc*) {
  assertx(false);
  return false;
}
} // end anonymous namespace

template <typename T, bool Assert>
bool TypeConstraint::checkTypeAliasImpl(const T* type) const {
  assertx(isValid<T>(this) && m_namedEntity && m_typeName);

  // Look up the type alias (autoloading if necessary)
  // and fail if we can't find it
  auto const td = [&]{
    if (!Assert) {
      return getTypeAliasWithAutoload(m_namedEntity, m_typeName);
    }
    return m_namedEntity->getCachedTypeAlias();
  }();
  if (!td) return Assert;

  // We found the type alias, check if an object or record of type 'type'
  // is compatible
  switch (getAnnotMetaType(td->type)) {
    case AnnotMetaType::Precise:
      return isInstanceOf(type, td);
    case AnnotMetaType::Mixed:
    case AnnotMetaType::Nonnull:
      return true;
    case AnnotMetaType::Callable:
      return methodExists(type);
    case AnnotMetaType::Nothing:
    case AnnotMetaType::NoReturn:
    case AnnotMetaType::Number:
    case AnnotMetaType::ArrayKey:
    case AnnotMetaType::This:
    case AnnotMetaType::VecOrDict:
    case AnnotMetaType::ArrayLike:
    case AnnotMetaType::Classname:  // TODO: T83332251
      return false;
    case AnnotMetaType::Self:
    case AnnotMetaType::Parent:
      // These should never happen, because type aliases are not allowed to use
      // those MetaTypes
      always_assert(false);
  }
  not_reached();
}

template
bool TypeConstraint::checkTypeAliasImpl<RecordDesc, false>(
    const RecordDesc*) const;

template <TypeConstraint::CheckMode Mode>
bool TypeConstraint::checkImpl(tv_rval val,
                               const Class* context) const {
  assertx(isCheckable());
  assertx(tvIsPlausible(*val));

  auto const isAssert = Mode == CheckMode::Assert;
  auto const isPasses = Mode == CheckMode::AlwaysPasses;
  auto const isProp   = Mode == CheckMode::ExactProp;
  DEBUG_ONLY auto const isRecField = Mode == CheckMode::ExactRecField;

  // We shouldn't provide a context for the conservative checks.
  assertx(!isAssert || !context);
  assertx(!isPasses || !context);
  assertx(!isProp   || validForProp());
  assertx(!isRecField || validForRecField());

  if (isNullable() && val.type() == KindOfNull) return true;

  if (val.type() == KindOfObject) {
    // Perfect match seems common enough to be worth skipping the hash
    // table lookup.
    const Class *c = nullptr;
    if (isObject()) {
      if (m_typeName->isame(val.val().pobj->getVMClass()->name())) {
        return true;
      }
      // We can't save the Class* since it might move around from request to
      // request.
      assertx(m_namedEntity);
      c = Class::lookup(m_namedEntity);
      // If we're being conservative we can only use the class if its persistent
      // (otherwise what we infer may not be valid in all requests).
      if (isPasses && c && !classHasPersistentRDS(c)) c = nullptr;
    } else {
      switch (metaType()) {
        case MetaType::Self:
          assertx(!isProp);
          assertx(!isRecField);
          if (isAssert) return true;
          if (isPasses) return false;
          c = context;
          break;
        case MetaType::This:
          assertx(!isRecField);
          if (isAssert) return true;
          if (isPasses) return false;
          if (isProp) return val.val().pobj->getVMClass() == context;
          if (auto const cls = getThis()) {
            return val.val().pobj->getVMClass() == cls;
          }
          return false;
        case MetaType::Parent:
          assertx(!isProp);
          assertx(!isRecField);
          if (isAssert) return true;
          if (isPasses) return false;
          if (context) c = context->parent();
          break;
        case MetaType::Callable:
          assertx(!isProp);
          assertx(!isRecField);
          if (isAssert) return true;
          if (isPasses) return false;
          return is_callable(tvAsCVarRef(*val));
        case MetaType::Nothing:
        case MetaType::NoReturn:
          assertx(!isProp);
          assertx(!isRecField);
          // fallthrogh
        case MetaType::Precise:
        case MetaType::Number:
        case MetaType::ArrayKey:
        case MetaType::VecOrDict:
        case MetaType::ArrayLike:
        case MetaType::Classname:
          return false;
        case MetaType::Nonnull:
          return true;
        case MetaType::Mixed:
          // We assert'd at the top of this function that the
          // metatype cannot be Mixed
          not_reached();
      }
    }
    if (c && val.val().pobj->instanceof(c)) {
      return true;
    }
    return isObject() && !isPasses &&
      checkTypeAliasImpl<Class, isAssert>(val.val().pobj->getVMClass());
  }

  auto const result = annotCompat(val.type(), m_type, m_typeName);
  switch (result) {
    case AnnotAction::Pass: return true;
    case AnnotAction::Fail: return false;
    case AnnotAction::CallableCheck:
      assertx(!isProp);
      assertx(!isRecField);
      if (isAssert) return true;
      if (isPasses) return false;
      return is_callable(tvAsCVarRef(*val));
    case AnnotAction::ObjectCheck:
      assertx(isObject());
      return !isPasses && checkNamedTypeNonObj<isAssert, isProp>(val);
    case AnnotAction::WarnClass:
    case AnnotAction::ConvertClass:
    case AnnotAction::WarnLazyClass:
    case AnnotAction::ConvertLazyClass:
      return false; // verifyFail will handle the conversion/warning
    case AnnotAction::ClsMethCheck:
      return false;
    case AnnotAction::RecordCheck:
      assertx(isRecord());
      return !isPasses && checkNamedTypeNonObj<isAssert, isProp>(val);
    case AnnotAction::WarnClassname:
      if (isPasses)  return false;
      assertx(isClassType(val.type()) || isLazyClassType(val.type()));
      assertx(RuntimeOption::EvalClassPassesClassname);
      assertx(RuntimeOption::EvalClassnameNotices);
      if (!isAssert) raise_notice(Strings::CLASS_TO_CLASSNAME);
      return true;
  }
  not_reached();
}

template bool TypeConstraint::checkImpl<TypeConstraint::CheckMode::Exact>(
  tv_rval,
  const Class*
) const;
template bool TypeConstraint::checkImpl<TypeConstraint::CheckMode::ExactProp>(
  tv_rval,
  const Class*
) const;
template bool TypeConstraint::checkImpl<TypeConstraint::CheckMode::AlwaysPasses>(
  tv_rval,
  const Class*
) const;
template bool TypeConstraint::checkImpl<TypeConstraint::CheckMode::Assert>(
  tv_rval,
  const Class*
) const;

bool TypeConstraint::alwaysPasses(const StringData* clsName) const {
  if (!isCheckable()) return true;

  if (isObject()) {
    // Same name is always a match.
    if (m_typeName->isame(clsName)) return true;

    assertx(m_namedEntity);
    auto const c1 = Class::lookup(clsName);
    auto const c2 = Class::lookup(m_namedEntity);
    // If both names map to persistent classes we can just check for a subtype
    // relationship.
    if (c1 && c2 &&
        classHasPersistentRDS(c1) &&
        classHasPersistentRDS(c2) &&
        c1->classof(c2)) return true;

    auto const result = annotCompat(KindOfObject, m_type, m_typeName);
    switch (result) {
      case AnnotAction::Pass:
        return true;
      case AnnotAction::Fail:
      case AnnotAction::CallableCheck:
      case AnnotAction::ObjectCheck:
        return false;
      case AnnotAction::WarnClass:
      case AnnotAction::ConvertClass:
      case AnnotAction::WarnLazyClass:
      case AnnotAction::ConvertLazyClass:
      case AnnotAction::ClsMethCheck:
      case AnnotAction::RecordCheck:
      case AnnotAction::WarnClassname:
        // Can't get these with objects
        break;
    }
    not_reached();
  }

  switch (metaType()) {
    case MetaType::Self:
    case MetaType::This:
    case MetaType::Parent:
    case MetaType::Callable:
    case MetaType::Precise:
    case MetaType::Nothing:
    case MetaType::NoReturn:
    case MetaType::Number:
    case MetaType::ArrayKey:
    case MetaType::VecOrDict:
    case MetaType::ArrayLike:
    case MetaType::Classname:
      return false;
    case MetaType::Nonnull:
      return true;
    case MetaType::Mixed:
      // We check at the top of this function that the metatype cannot be
      // Mixed
      break;
  }
  not_reached();
}

bool TypeConstraint::alwaysPasses(DataType dt) const {
  // Use the StringData* overflow for objects.
  assertx(dt != KindOfObject);
  if (!isCheckable()) return true;

  if (isNullable() && dt == KindOfNull) return true;

  auto const result = annotCompat(dt, m_type, m_typeName);
  switch (result) {
    case AnnotAction::Pass:
      return true;
    case AnnotAction::Fail:
    case AnnotAction::CallableCheck:
    case AnnotAction::ObjectCheck:
    case AnnotAction::WarnClass:
    case AnnotAction::ConvertClass:
    case AnnotAction::WarnLazyClass:
    case AnnotAction::ConvertLazyClass:
    case AnnotAction::ClsMethCheck:
    case AnnotAction::RecordCheck:
    case AnnotAction::WarnClassname:
      return false;
  }
  not_reached();
}

void TypeConstraint::verifyParam(tv_lval val,
                                 const Func* func,
                                 int paramNum) const {
  if (UNLIKELY(!check(val, func->cls()))) {
    verifyParamFail(func, val, paramNum);
  }
}

void TypeConstraint::verifyReturn(TypedValue* tv, const Func* func) const {
  if (UNLIKELY(!check(tv, func->cls()))) {
    verifyReturnFail(func, tv);
  }
}

void TypeConstraint::verifyOutParam(TypedValue* tv,
                                    const Func* func,
                                    int paramNum) const {
  if (UNLIKELY(!check(tv, func->cls()))) {
    verifyOutParamFail(func, tv, paramNum);
  }
}

void TypeConstraint::verifyProperty(tv_lval val,
                                    const Class* thisCls,
                                    const Class* declCls,
                                    const StringData* propName) const {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(validForProp());
  if (UNLIKELY(!checkImpl<CheckMode::ExactProp>(val, thisCls))) {
    verifyPropFail(thisCls, declCls, val, propName, false);
  }
}

void TypeConstraint::verifyStaticProperty(tv_lval val,
                                          const Class* thisCls,
                                          const Class* declCls,
                                          const StringData* propName) const {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(validForProp());
  if (UNLIKELY(!checkImpl<CheckMode::ExactProp>(val, thisCls))) {
    verifyPropFail(thisCls, declCls, val, propName, true);
  }
}

void TypeConstraint::verifyRecField(tv_rval val,
                                 const StringData* recordName,
                                 const StringData* fieldName) const {
  assertx(validForRecField());
  if (UNLIKELY(!checkImpl<CheckMode::ExactRecField>(val, nullptr))) {
    verifyRecFieldFail(val, recordName, fieldName);
  }
}

void TypeConstraint::verifyReturnNonNull(TypedValue* tv,
                                         const Func* func) const {
  const auto DEBUG_ONLY tc = func->returnTypeConstraint();
  assertx(!tc.isNullable());
  if (UNLIKELY(tvIsNull(tv))) {
    verifyReturnFail(func, tv);
  } else if (debug) {
    auto vm = &*g_context;
    always_assert_flog(
      check(tv, func->cls()),
      "HHBBC incorrectly converted VerifyRetTypeC to VerifyRetNonNull in {}:{}",
      vm->getContainingFileName()->data(),
      vm->getLine()
    );
  }
}

std::string describe_actual_type(tv_rval val) {
  // Validate TV to catch possible memory corruptions.
  // We suspect, some rare unexplained typehint notices are caused by memory
  // corruption. This would detect a problem sooner and give us more info to
  // debug.
  always_assert(tvIsPlausible(val.tv()));

  switch (val.type()) {
    case KindOfUninit:        return "undefined variable";
    case KindOfNull:          return "null";
    case KindOfBoolean:       return "bool";
    case KindOfInt64:         return "int";
    case KindOfDouble:        return "float";
    case KindOfPersistentString:
    case KindOfString:        return "string";
    case KindOfPersistentVec:
    case KindOfVec:           {
      return val.val().parr->isLegacyArray() ? "varray" : "HH\\vec";
    }
    case KindOfPersistentDict:
    case KindOfDict:          {
      return val.val().parr->isLegacyArray() ? "darray" : "HH\\dict";
    }
    case KindOfPersistentKeyset:
    case KindOfKeyset:        return "HH\\keyset";
    case KindOfResource:
      return val.val().pres->data()->o_getClassName().c_str();
    case KindOfRFunc:         return "reified function";
    case KindOfFunc:          return "func";
    case KindOfClass:         return "class";
    case KindOfLazyClass:        return "lazy class";
    case KindOfClsMeth:       return "clsmeth";
    case KindOfRClsMeth:      return "reified clsmeth";
    case KindOfRecord:
      return val.val().prec->record()->name()->data();
    case KindOfObject: {
      auto const obj = val.val().pobj;
      auto const cls = obj->getVMClass();
      auto const name = cls->name()->toCppString();
      if (!cls->hasReifiedGenerics()) return name;
      auto const generics = getClsReifiedGenericsProp(cls, obj);
      return folly::sformat("{}{}", name, mangleReifiedGenericsName(generics));
    }
  }
  not_reached();
}

bool TypeConstraint::checkStringCompatible() const {
  if (isString() || isArrayKey() ||
      (isObject() && interface_supports_string(m_typeName))) {
    return true;
  }
  if (!isObject()) return false;
  auto p = getNamedTypeWithAutoload(m_namedEntity, m_typeName);
  if (auto ptd = boost::get<const TypeAlias*>(&p)) {
    auto td = *ptd;
    return td->type == AnnotType::String ||
           td->type == AnnotType::ArrayKey;
  }
  if (auto pc = boost::get<Class*>(&p)) {
    auto c = *pc;
    if (isEnum(c)) {
      auto dt = c->enumBaseTy();
      return !dt || isStringType(dt);
    }
  }

  return false;
}

bool TypeConstraint::convertClsMethToArrLike() const {
  auto const result = annotCompat(KindOfClsMeth, type(), typeName());
  return result == AnnotAction::ClsMethCheck;
}

void TypeConstraint::verifyParamFail(const Func* func, tv_lval val,
                                     int paramNums) const {
  verifyFail(func, val, paramNums);
  assertx(
    isSoft() ||
    (isThis() && couldSeeMockObject()) ||
    (RO::EvalEnforceGenericsUB < 2 && isUpperBound()) ||
    check(val, func->cls())
  );
}

namespace {

template <typename F>
void castClsMeth(tv_lval c, F make) {
  assertx(type(c) == KindOfClsMeth);
  auto const a = make(
    val(c).pclsmeth->getClsStr(),
    val(c).pclsmeth->getFuncStr()
  ).detach();
  tvDecRefClsMeth(c);
  val(c).parr = a;
  type(c) = a->toDataType();
}

}

void TypeConstraint::verifyOutParamFail(const Func* func,
                                        TypedValue* c,
                                        int paramNum) const {
  if ((isClassType(c->m_type) || isLazyClassType(c->m_type)) &&
      checkStringCompatible()) {
    if (RuntimeOption::EvalClassStringHintNotices) {
      raise_notice(Strings::CLASS_TO_STRING_IMPLICIT);
    }
    c->m_data.pstr = isClassType(c->m_type) ?
      const_cast<StringData*>(c->m_data.pclass->name()) :
      const_cast<StringData*>(c->m_data.plazyclass.name());
    c->m_type = KindOfPersistentString;
    return;
  }

  if (isClsMethType(c->m_type) && convertClsMethToArrLike()) {
    if (RuntimeOption::EvalVecHintNotices) {
      raise_clsmeth_compat_type_hint_outparam_notice(
        func, displayName(func->cls()), paramNum);
    }
    castClsMeth(c, make_varray<String,String>);
    return;
  }

  std::string msg = folly::sformat(
      "Argument {} returned from {}() as an inout parameter must be {} "
      "{}, {} given",
      paramNum + 1,
      func->fullName(),
      isUpperBound() ? "upper-bounded by" : "of type",
      displayName(func->cls()),
      describe_actual_type(c)
  );

  if (!isSoft() && (!isUpperBound() || RuntimeOption::EvalEnforceGenericsUB >= 2)) {
    raise_return_typehint_error(msg);
  } else {
    raise_warning_unsampled(msg);
  }
}

void TypeConstraint::verifyRecFieldFail(tv_rval val,
                                     const StringData* recordName,
                                     const StringData* fieldName) const {
  assertx(validForRecField());

  raise_record_field_typehint_error(
    folly::sformat(
      "Record field '{}::{}' declared as type {}, {} assigned",
      recordName,
      fieldName,
      displayName(nullptr),
      describe_actual_type(val)
    ),
    isSoft()
  );
}
void TypeConstraint::verifyPropFail(const Class* thisCls,
                                    const Class* declCls,
                                    tv_lval val,
                                    const StringData* propName,
                                    bool isStatic) const {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(validForProp());

  if (UNLIKELY(isThis() && val.type() == KindOfObject)) {
    auto const valCls = val.val().pobj->getVMClass();
    if (valCls->preClass()->userAttributes().count(s___MockClass.get()) &&
        valCls->parent() == thisCls) {
      return;
    }
  }

  if ((isClassType(val.type()) || isLazyClassType(val.type())) &&
      checkStringCompatible()) {
    if (RuntimeOption::EvalClassStringHintNotices) {
      raise_notice(Strings::CLASS_TO_STRING_IMPLICIT);
    }
    val.val().pstr = isClassType(val.type()) ?
      const_cast<StringData*>(val.val().pclass->name()) :
      const_cast<StringData*>(val.val().plazyclass.name());
    val.type() = KindOfPersistentString;
    return;
  }

  if (isClsMethType(val.type()) && convertClsMethToArrLike()) {
    if (RuntimeOption::EvalVecHintNotices) {
      raise_clsmeth_compat_type_hint_property_notice(
        declCls, propName, displayName(nullptr), isStatic);
    }
    // Only trigger coercion logic if property type hints are soft
    if ((RO::EvalCheckPropTypeHints != 3) ||
        (isUpperBound() && RO::EvalEnforceGenericsUB < 2)) return;
    castClsMeth(val, make_varray<String,String>);
    return;
  }

  raise_property_typehint_error(
    folly::sformat(
      "{} '{}::{}' {} type {}, {} assigned",
      isStatic ? "Static property" : "Property",
      declCls->name(),
      propName,
      isUpperBound() ? "upper-bounded by" : "declared as",
      displayName(nullptr),
      describe_actual_type(val)
    ),
    isSoft(),
    isUpperBound()
  );
}

void TypeConstraint::verifyFail(const Func* func, tv_lval c,
                                int id) const {
  VMRegAnchor _;
  std::string name = displayName(func->cls());
  auto const givenType = describe_actual_type(c);

  if (UNLIKELY(isThis() && c.type() == KindOfObject)) {
    Class* cls = val(c).pobj->getVMClass();
    auto const thisClass = getThis();
    if (cls->preClass()->userAttributes().count(s___MockClass.get()) &&
        cls->parent() == thisClass) {
      return;
    }
  }

  if ((isClassType(c.type()) || isLazyClassType(c.type())) &&
      checkStringCompatible()) {
    if (RuntimeOption::EvalClassStringHintNotices) {
      raise_notice(Strings::CLASS_TO_STRING_IMPLICIT);
    }
    val(c).pstr = isClassType(c.type()) ?
      const_cast<StringData*>(val(c).pclass->name()) : // TODO (T61651936)
      const_cast<StringData*>(val(c).plazyclass.name());
    c.type() = KindOfPersistentString;
    return;
  }

  if (isClsMethType(c.type()) && convertClsMethToArrLike()) {
    if (RuntimeOption::EvalVecHintNotices) {
      auto const i = id == ReturnId ? folly::none : folly::make_optional(id);
      raise_clsmeth_compat_type_hint(func, name, i);
    }
    castClsMeth(c, make_varray<String,String>);
    return;
  }

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
          "Value returned from {}{} {}() must be {} {}, {} given",
          func->isAsync() ? "async " : "",
          func->preClass() ? "method" : "function",
          func->fullName(),
          isUpperBound() ? "upper-bounded by" : "of type",
          name,
          givenType
        ).str();
    }
    if (!isSoft() && (!isUpperBound() || RuntimeOption::EvalEnforceGenericsUB >= 2)) {
      raise_return_typehint_error(msg);
    } else {
      raise_warning_unsampled(msg);
    }
    return;
  }

  // Handle parameter type constraint failures
  if (isExtended() &&
      (isSoft() ||
      (isUpperBound() && RuntimeOption::EvalEnforceGenericsUB < 2))) {
    // Soft extended type hints raise warnings instead of recoverable
    // errors, to ease migration.
    raise_warning_unsampled(
      folly::format(
        "Argument {} to {}() must be {} {}, {} given",
        id + 1, func->fullName(),
        isUpperBound() ? "upper-bounded by" : "of type",
        name, givenType
      ).str()
    );
  } else if (isExtended() && isNullable()) {
    raise_typehint_error(
      folly::format(
        "Argument {} to {}() must be {} {}, {} given",
        id + 1, func->fullName(),
        isUpperBound() ? "upper-bounded by" : "of type",
        name, givenType
      ).str()
    );
  } else {
    auto cls = Class::lookup(m_typeName);
    if (cls && isInterface(cls)) {
      auto const msg =
        folly::format(
          "Argument {} passed to {}() must {} interface {}, {} given",
          id + 1, func->fullName(),
          isUpperBound() ? "be upper-bounded by" : "implement",
          name, givenType
        ).str();
      if (isUpperBound() && RuntimeOption::EvalEnforceGenericsUB < 2) {
        raise_warning_unsampled(msg);
      } else {
        raise_typehint_error(msg);
      }
    } else {
      auto const msg =
        folly::format(
          "Argument {} passed to {}() must be {} {}, {} given",
          id + 1, func->fullName(),
          isUpperBound() ? "upper-bounded by" : "an instance of",
          name, givenType
        ).str();
      if (isUpperBound() && RuntimeOption::EvalEnforceGenericsUB < 2) {
        raise_warning_unsampled(msg);
      } else {
        raise_typehint_error(msg);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////

MemoKeyConstraint memoKeyConstraintFromTC(const TypeConstraint& tc) {
  using MK = MemoKeyConstraint;

  // Soft constraints aren't useful because they're not enforced.
  if (!tc.hasConstraint() || tc.isTypeVar() ||
      tc.isTypeConstant() || tc.isSoft()) {
    return MK::None;
  }

  // Only a subset of possible type-constraints are useful to use. Namely,
  // single types which might be nullable, and int/string combination.
  switch (tc.metaType()) {
    case AnnotMetaType::Precise: {
      auto const dt = tc.underlyingDataType();
      assertx(dt.has_value());
      switch (*dt) {
        case KindOfBoolean:
          return tc.isNullable() ? MK::BoolOrNull : MK::Bool;
        case KindOfInt64:
          return tc.isNullable() ? MK::IntOrNull : MK::Int;
        case KindOfPersistentString:
        case KindOfString:
          return tc.isNullable() ? MK::StrOrNull : MK::Str;
        case KindOfObject:
          return tc.isNullable() ? MK::ObjectOrNull : MK::Object;
        case KindOfDouble:
          return tc.isNullable() ? MK::DblOrNull : MK::Dbl;
        case KindOfPersistentVec:
        case KindOfVec:
        case KindOfPersistentDict:
        case KindOfDict:
        case KindOfPersistentKeyset:
        case KindOfKeyset:
        case KindOfClsMeth:
        case KindOfResource:
        case KindOfRecord:
        case KindOfNull:         return MK::None;
        case KindOfUninit:
        case KindOfRFunc:
        case KindOfFunc:
        case KindOfRClsMeth:
        case KindOfClass:
        case KindOfLazyClass:
          always_assert_flog(false, "Unexpected DataType");
      }
      not_reached();
    }
    case AnnotMetaType::ArrayKey:
      return tc.isNullable() ? MK::None : MK::IntOrStr;
    case AnnotMetaType::Mixed:
    case AnnotMetaType::Nothing:
    case AnnotMetaType::NoReturn:
    case AnnotMetaType::Nonnull:
    case AnnotMetaType::Self:
    case AnnotMetaType::This:
    case AnnotMetaType::Parent:
    case AnnotMetaType::Callable:
    case AnnotMetaType::Number:
    case AnnotMetaType::VecOrDict:
    case AnnotMetaType::ArrayLike:
    case AnnotMetaType::Classname:
      return MK::None;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

bool tcCouldBeReified(const Func* func, uint32_t paramId) {
  auto const& tc = paramId == TypeConstraint::ReturnId
    ? func->returnTypeConstraint() : func->params()[paramId].typeConstraint;
  auto const isReifiedGenericInClosure = [&] {
    if (!func->isClosureBody()) return false;
    auto const lNames = func->localNames();
    auto const nParams = func->numParams();
    auto const nDeclaredProps = func->baseCls()->numDeclProperties();
    assertx(nParams + nDeclaredProps <= func->numNamedLocals());
    // Skip over params
    for (int i = nParams; i < nParams + nDeclaredProps; ++i) {
      assertx(lNames[i]);  // Closures can't have names removed.
      if (isMangledReifiedGenericInClosure(lNames[i])) return true;
    }
    return false;
  };
  return tc.isTypeVar() &&
         (func->hasReifiedGenerics() ||
          (func->cls() && func->cls()->hasReifiedGenerics()) ||
          isReifiedGenericInClosure());
}

//////////////////////////////////////////////////////////////////////
void applyFlagsToUB(TypeConstraint& ub, const TypeConstraint& tc) {
  auto const tcFlags = tc.flags() & ~TypeConstraint::Flags::TypeVar;
  ub.addFlags(static_cast<TypeConstraint::Flags>(tcFlags));
}
}
