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

#include <hphp/runtime/base/datatype.h>
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
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/ext/std/ext_std_function.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

using ObjectConstraint = TypeConstraint::ObjectConstraint;

//////////////////////////////////////////////////////////////////////

const StaticString s___invoke("__invoke");

void TypeConstraint::init() {
  initSingle();
}

void ObjectConstraint::init(AnnotType const type) {
  bool isObject = type == Type::Object;
  bool isUnresolved = type == Type::Unresolved;
  if (isObject) {
    m_namedType = NamedType::get(m_clsName);
  } else if (isUnresolved) {
    m_namedType = NamedType::get(m_typeName);
  }
  FTRACE(5, "TypeConstraint: this {} NamedType: {}\n",
        this, m_namedType.get());
}

void TypeConstraint::initSingle() {
  auto& single = m_u.single;
  if (single.object.m_typeName == nullptr || isTypeVar() || isTypeConstant()) {
    single.type = Type::Mixed;
    return;
  }
  FTRACE(5, "TypeConstraint: this {} type {}, nullable {}\n",
        this, typeName(), isNullable());
  auto const mptr = nameToAnnotType(typeName());
  if (mptr) {
    single.type = *mptr;
    assertx(single.type != Type::Object);
    assertx(getAnnotDataType(single.type) != KindOfPersistentString);
    return;
  }
  if (m_flags & TypeConstraintFlags::Resolved) {
    TRACE(5, "TypeConstraint: this %p pre-resolved type %s, treating as %s\n",
          this, typeName()->data(), tname(getAnnotDataType(single.type)).c_str());
  } else {
    TRACE(5, "TypeConstraint: this %p no such type %s, marking as unresolved\n",
          this, typeName()->data());
    single.type = Type::Unresolved;
  }
  single.object.init(single.type);
}

bool ObjectConstraint::operator==(const ObjectConstraint& o) const {
  // The named entity is defined based on the typeName() and is redundant to
  // include in the equality operation.
  return m_clsName == o.m_clsName && m_typeName == o.m_typeName;
}

bool TypeConstraint::operator==(const TypeConstraint& o) const {
  if (m_flags != o.m_flags) return false;

  auto& a = m_u.single;
  auto& b = o.m_u.single;
  return a.type == b.type && a.object == b.object;
}

size_t ObjectConstraint::stableHash() const {
  size_t clsHash = m_clsName ? m_clsName->hashStatic() : 0;
  size_t typeName = m_typeName ? m_typeName->hashStatic() : 0;
  return folly::hash::hash_combine(clsHash, typeName);
}

size_t TypeConstraint::stableHash() const {
  return folly::hash::hash_combine(
    std::hash<TypeConstraintFlags>()(m_flags),
    std::hash<AnnotType>()(m_u.single.type),
    m_u.single.object.stableHash()
  );
}

std::string TypeConstraint::displayName(const Class* context /*= nullptr*/,
                                        bool extra /* = false */) const {
  const StringData* tn = typeName();
  std::string name;
  if (isSoft()) {
    name += '@';
  }
  if ((m_flags & TypeConstraintFlags::DisplayNullable) && isExtended()) {
    name += '?';
  }

  const char* str = tn ? tn->data() : "";
  auto len = tn ? tn->size() : 0;
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

  if (extra && m_flags & TypeConstraintFlags::Resolved) {
    const char* str = nullptr;
    switch (m_u.single.type) {
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
      case AnnotType::Classname: str = "classname"; break;
      case AnnotType::Object:   str = clsName()->data(); break;
      case AnnotType::This:
      case AnnotType::Mixed:
      case AnnotType::Callable:
        break;
      case AnnotType::Unresolved:
        not_reached();
    }
    if (str) folly::format(&name, " ({})", str);
  }
  return name;
}

namespace {

/*
 * Look up a TypeAlias for the supplied NamedType (which must be the
 * NamedType for `name'), invoking autoload if necessary for types but not
 * for classes.
 *
 * We don't need to autoload classes because it is impossible to have an
 * instance of a class if it's not defined.  However, we need to autoload
 * typedefs because they can affect whether the parameter type verification
 * would succeed.
 */
const TypeAlias* getTypeAliasWithAutoload(const NamedType* ne,
                                          const StringData* name) {
  auto def = ne->getCachedTypeAlias();
  if (!def) {
    VMRegAnchor _;
    String nameStr(const_cast<StringData*>(name));
    if (!AutoloadHandler::s_instance->autoloadTypeAlias(nameStr)) {
      return nullptr;
    }
    def = ne->getCachedTypeAlias();
  }
  return def;
}

/*
 * Look up a TypeAlias or a Class for the supplied NamedType
 * (which must be the NamedType for `name'), invoking autoload if
 * necessary.
 *
 * This is useful when looking up a type annotation that could be either a
 * type alias or an enum class; enum classes are strange in that it
 * *is* possible to have an instance of them even if they are not defined.
 */
boost::variant<const TypeAlias*, Class*>
getNamedTypeWithAutoload(const NamedType* ne,
                         const StringData* name) {

  if (auto def = ne->getCachedTypeAlias()) {
    return def;
  }
  Class *klass = nullptr;
  klass = Class::lookup(ne);
  // We don't have the class or the typedef, so autoload.
  if (!klass) {
    String nameStr(const_cast<StringData*>(name));
    if (AutoloadHandler::s_instance->autoloadTypeOrTypeAlias(nameStr)) {
      // Autoload succeeded, try to grab a typedef or a class.
      if (auto def = ne->getCachedTypeAlias()) {
        return def;
      }
      klass = Class::lookup(ne);
    }
  }
  return klass;
}

} // namespace

TinyVector<TypeConstraint> TypeConstraint::resolvedWithAutoload() const {
  auto copy = *this;

  // Nothing to do if we are not unresolved.
  if (!isUnresolved()) return {copy};

  auto const p = getNamedTypeWithAutoload(typeNamedType(), typeName());

  // Type alias.
  if (auto const ptd = boost::get<const TypeAlias*>(&p)) {
    auto const td = *ptd;
    TinyVector<TypeConstraint> result;
    for (auto const& [type, klass] : td->typeAndClassUnion()) {
      auto copy = *this;
      auto const typeName = klass ? klass->name() : nullptr;
      copy.resolveType(type, td->nullable, typeName);
      result.push_back(copy);
    }
    return result;
  }

  // Enum.
  if (auto const pcls = boost::get<Class*>(&p)) {
    auto const cls = *pcls;
    if (cls && isEnum(cls)) {
      auto const type = cls->enumBaseTy()
        ? enumDataTypeToAnnotType(*cls->enumBaseTy())
        : AnnotType::ArrayKey;
      copy.resolveType(type, false, nullptr);
      return {copy};
    }
  }

  // Existing or non-existing class.
  copy.resolveType(AnnotType::Object, false, typeName());
  return {copy};
}

MaybeDataType TypeConstraint::underlyingDataTypeResolved() const {
  assertx(!isCallable());
  assertx(IMPLIES(
    !hasConstraint() || isTypeVar() || isTypeConstant(),
    isMixed()));

  auto const resolved = resolvedWithAutoload();
  // Used for enums only -- enums cannot use union types
  if (resolved.size() != 1) return std::nullopt;
  return resolved[0].isPrecise() ? resolved[0].underlyingDataType() : std::nullopt;
}

bool TypeConstraint::isMixedResolved() const {
  if (!isCheckable()) return true;
  // isCheckable() implies !isMixed(), so if its not an unresolved object here,
  // we know it cannot be mixed.
  if (!isUnresolved()) return false;
  auto const resolved = resolvedWithAutoload();
  return std::any_of(resolved.begin(), resolved.end(),
                     [](const TypeConstraint& tc) { return !tc.isCheckable(); });
}

bool TypeConstraint::maybeMixed() const {
  if (!isCheckable()) return true;
  // isCheckable() implies !isMixed(), so if its not an unresolved object here,
  // we know it cannot be mixed.
  if (!isUnresolved()) return false;
  if (auto const def = typeNamedType()->getCachedTypeAlias()) {
    auto const it = def->typeAndClassUnion();
    return std::any_of(it.begin(), it.end(),
                       [] (auto tcu) { return tcu.type == AnnotType::Mixed; });
  }
  // If its a known class, its definitely not mixed. Otherwise it might be.
  return !Class::lookup(typeNamedType());
}

bool
TypeConstraint::maybeInequivalentForProp(const TypeConstraint& other) const {
  assertx(validForProp());
  assertx(other.validForProp());

  if (isSoft() != other.isSoft()) return true;

  if (!isCheckable()) return other.isCheckable();
  if (!other.isCheckable()) return true;

  if (isNullable() != other.isNullable()) return true;

  if (isObject() && other.isObject()) return !clsName()->isame(other.clsName());

  if (isObject() || isUnresolved()) {
    // Type-hints with the same name should always be the same thing
    // TODO: take advantage of clsName() if one of them is object
    return
      (!other.isObject() && !other.isUnresolved()) ||
      !typeName()->isame(other.typeName());
  }
  if (other.isObject() || other.isUnresolved()) return true;
  return type() != other.type();
}

bool TypeConstraint::equivalentForProp(const TypeConstraint& other) const {
  assertx(validForProp());
  assertx(other.validForProp());

  if (isSoft() != other.isSoft()) return false;

  if (isObject() && other.isObject()) return clsName()->isame(other.clsName());

  if ((isObject() || isUnresolved()) &&
      (other.isObject() || other.isUnresolved()) &&
      isNullable() == other.isNullable() &&
      typeName()->isame(other.typeName())) {
    // We can avoid having to resolve the type-hint if they have the same name.
    // TODO: take advantage of clsName() if one of them is object
    return true;
  }

  auto const resolve = [&] (const TypeConstraint& origTC) {
    auto const resolved = origTC.resolvedWithAutoload();
    std::vector<std::tuple<AnnotType, const StringData*, bool>> result;

    for (auto const tc : resolved) {
      if (!tc.isCheckable()) {
        result.emplace_back(AnnotType::Mixed, nullptr, false);
        continue;
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
          result.emplace_back(tc.type(), tc.clsName(), tc.isNullable());
          continue;
        case MetaType::Nothing:
        case MetaType::NoReturn:
        case MetaType::Callable:
        case MetaType::Mixed:
        case MetaType::Unresolved:
          always_assert(false);
      }
    }
    return result;
  };

  return resolve(*this) == resolve(other);
}

template <bool Assert, bool ForProp>
bool TypeConstraint::checkNamedTypeNonObj(tv_rval val) const {
  assertx(val.type() != KindOfObject);
  assertx(isUnresolved());

  auto const p = [&]() ->
    boost::variant<const TypeAlias*, Class*> {
    if (!Assert) {
      return getNamedTypeWithAutoload(typeNamedType(), typeName());
    }
    if (auto const def = typeNamedType()->getCachedTypeAlias()) {
      return def;
    }
    return Class::lookup(typeNamedType());
  }();
  auto ptd = boost::get<const TypeAlias*>(&p);
  auto td = ptd ? *ptd : nullptr;
  auto pc = boost::get<Class*>(&p);
  auto c = pc ? *pc : nullptr;

  if (Assert && !td && !c) return true;

  // Common case is that we actually find the alias:
  if (td) {
    if (td->nullable && val.type() == KindOfNull) return true;
    bool pass_and_raise_classname_notice = false;
    for (auto const& [type, klass] : td->typeAndClassUnion()) {
      auto result = annotCompat(val.type(), type, klass ? klass->name() : nullptr);
      switch (result) {
        case AnnotAction::Pass: return true;
        case AnnotAction::Fail: continue;
        case AnnotAction::CallableCheck:
          if (!ForProp && (Assert || is_callable(tvAsCVarRef(*val)))) return true;
          continue;
        case AnnotAction::WarnClass:
        case AnnotAction::ConvertClass:
        case AnnotAction::WarnLazyClass:
        case AnnotAction::ConvertLazyClass:
          continue; // verifyFail will deal with the conversion/warning
        case AnnotAction::WarnClassname:
          assertx(isClassType(val.type()) || isLazyClassType(val.type()));
          assertx(RuntimeOption::EvalClassPassesClassname);
          assertx(RuntimeOption::EvalClassnameNotices);
          if (!Assert) {
            pass_and_raise_classname_notice = true;
            continue;
          }
          return true;
        case AnnotAction::ObjectCheck:
        case AnnotAction::Fallback:
        case AnnotAction::FallbackCoerce:
          not_reached();
      }
    }
    if (pass_and_raise_classname_notice) {
      raise_notice(Strings::CLASS_TO_CLASSNAME);
      return true;
    }
    return false;
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

template <bool Assert>
bool TypeConstraint::checkTypeAliasImpl(const Class* cls) const {
  assertx(isUnresolved());

  // Look up the type alias (autoloading if necessary)
  // and fail if we can't find it
  auto const td = [&]{
    if (!Assert) {
      return getTypeAliasWithAutoload(typeNamedType(), typeName());
    }
    return typeNamedType()->getCachedTypeAlias();
  }();
  if (!td) return Assert;

  // We found the type alias, check if an object of type 'type' is
  // compatible
  for (auto const& [type, klass] : td->typeAndClassUnion()) {
    switch (getAnnotMetaType(type)) {
      case AnnotMetaType::Precise:
        if (type == AnnotType::Object && klass && cls->classof(klass)) {
          return true;
        }
        continue;
      case AnnotMetaType::Mixed:
      case AnnotMetaType::Nonnull:
        return true;
      case AnnotMetaType::Callable:
        if (cls->lookupMethod(s___invoke.get()) != nullptr) return true;
        continue;
      case AnnotMetaType::Nothing:
      case AnnotMetaType::NoReturn:
      case AnnotMetaType::Number:
      case AnnotMetaType::ArrayKey:
      case AnnotMetaType::This:
      case AnnotMetaType::VecOrDict:
      case AnnotMetaType::ArrayLike:
      case AnnotMetaType::Classname:  // TODO: T83332251
        continue;
      case AnnotMetaType::Unresolved:
        not_reached();
        break;
    }
  }
  return false;
}

template bool TypeConstraint::checkTypeAliasImpl<false>(
    const Class* type) const;
template bool TypeConstraint::checkTypeAliasImpl<true>(
    const Class* type) const;

template <TypeConstraint::CheckMode Mode>
bool TypeConstraint::checkImpl(tv_rval val,
                               const Class* context) const {
  assertx(isCheckable());
  assertx(tvIsPlausible(*val));

  auto const isAssert = Mode == CheckMode::Assert;
  auto const isPasses = Mode == CheckMode::AlwaysPasses;
  auto const isProp   = Mode == CheckMode::ExactProp;

  // We shouldn't provide a context for the conservative checks.
  assertx(!isAssert || !context);
  assertx(!isPasses || !context);
  assertx(!isProp   || validForProp());

  if (isNullable() && val.type() == KindOfNull) return true;

  if (val.type() == KindOfObject) {
    auto const tryCls = [&](const StringData* clsName, const NamedType* ne) {
      // Perfect match seems common enough to be worth skipping the hash
      // table lookup.
      if (clsName->isame(val.val().pobj->getVMClass()->name())) return true;

      assertx(ne);
      auto const cls = Class::lookup(ne);

      // If we're being conservative we can only use the class if its persistent
      // (otherwise what we infer may not be valid in all requests).
      return
        cls &&
        (!isPasses || classHasPersistentRDS(cls)) &&
        val.val().pobj->instanceof(cls);
    };

    if (isObject()) {
      return tryCls(clsName(), clsNamedType());
    }

    if (isUnresolved()) {
      if (tryCls(typeName(), typeNamedType())) return true;
      if (isPasses) return false;
      return checkTypeAliasImpl<isAssert>(val.val().pobj->getVMClass());
    }

    switch (metaType()) {
      case MetaType::This:
        if (isAssert) return true;
        if (isPasses) return false;
        return val.val().pobj->getVMClass() == context;
      case MetaType::Callable:
        assertx(!isProp);
        if (isAssert) return true;
        if (isPasses) return false;
        return is_callable(tvAsCVarRef(*val));
      case MetaType::Nothing:
      case MetaType::NoReturn:
        assertx(!isProp);
        [[fallthrough]];
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
        // metatype cannot be Mixed.
        not_reached();
      case MetaType::Unresolved:
        // Unresolved was handled above.
        not_reached();
    }
    not_reached();
  }

  auto const name = isObject() ? clsName() : typeName();
  auto const result = annotCompat(val.type(), m_u.single.type, name);
  switch (result) {
    case AnnotAction::Pass: return true;
    case AnnotAction::Fail: return false;
    case AnnotAction::CallableCheck:
      assertx(!isProp);
      if (isAssert) return true;
      if (isPasses) return false;
      return is_callable(tvAsCVarRef(*val));
    case AnnotAction::Fallback:
    case AnnotAction::FallbackCoerce:
      assertx(isUnresolved());
      return !isPasses && checkNamedTypeNonObj<isAssert, isProp>(val);
    case AnnotAction::WarnClass:
    case AnnotAction::ConvertClass:
    case AnnotAction::WarnLazyClass:
    case AnnotAction::ConvertLazyClass:
      return false; // verifyFail will handle the conversion/warning
    case AnnotAction::WarnClassname:
      if (isPasses)  return false;
      assertx(isClassType(val.type()) || isLazyClassType(val.type()));
      assertx(RuntimeOption::EvalClassPassesClassname);
      assertx(RuntimeOption::EvalClassnameNotices);
      if (!isAssert) raise_notice(Strings::CLASS_TO_CLASSNAME);
      return true;
    case AnnotAction::ObjectCheck:
      not_reached();
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

bool TypeConstraint::alwaysPasses(const StringData* checkedClsName) const {
  if (!isCheckable()) return true;

  auto const tryCls = [&](const StringData* clsName, const NamedType* ne) {
    // Same name is always a match.
    if (clsName->isame(checkedClsName)) return true;

    assertx(ne);
    auto const c1 = Class::lookup(checkedClsName);
    auto const c2 = Class::lookup(ne);
    // If both names map to persistent classes we can just check for a subtype
    // relationship.
    return
      c1 && c2 &&
      classHasPersistentRDS(c1) &&
      classHasPersistentRDS(c2) &&
      c1->classof(c2);
  };

  switch (metaType()) {
    case MetaType::This:
    case MetaType::Callable:
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
    case MetaType::Precise:
      if (isObject()) return tryCls(clsName(), clsNamedType());
      return false;
    case MetaType::Unresolved:
      return tryCls(typeName(), typeNamedType());
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

  auto const name = isObject() ? clsName() : typeName();
  auto const result = annotCompat(dt, m_u.single.type, name);
  switch (result) {
    case AnnotAction::Pass:
      return true;
    case AnnotAction::Fail:
    case AnnotAction::Fallback:
    case AnnotAction::FallbackCoerce:
    case AnnotAction::CallableCheck:
    case AnnotAction::ObjectCheck:
    case AnnotAction::WarnClass:
    case AnnotAction::ConvertClass:
    case AnnotAction::WarnLazyClass:
    case AnnotAction::ConvertLazyClass:
    case AnnotAction::WarnClassname:
      return false;
  }
  not_reached();
}

void TypeConstraint::verifyParam(tv_lval val,
                                 const Class* ctx,
                                 const Func* func,
                                 int paramNum) const {
  if (UNLIKELY(!check(val, ctx))) {
    verifyParamFail(val, ctx, func, paramNum);
  }
}

void TypeConstraint::verifyReturn(TypedValue* tv,
                                  const Class* ctx,
                                  const Func* func) const {
  if (UNLIKELY(!check(tv, ctx))) {
    verifyReturnFail(tv, ctx, func);
  }
}

void TypeConstraint::verifyOutParam(TypedValue* tv,
                                    const Class* ctx,
                                    const Func* func,
                                    int paramNum) const {
  if (UNLIKELY(!check(tv, ctx))) {
    verifyOutParamFail(tv, ctx, func, paramNum);
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

void TypeConstraint::verifyReturnNonNull(TypedValue* tv,
                                         const Class* ctx,
                                         const Func* func) const {
  const auto DEBUG_ONLY tc = func->returnTypeConstraint();
  assertx(!tc.isNullable());
  if (UNLIKELY(tvIsNull(tv))) {
    verifyReturnFail(tv, ctx, func);
  } else if (debug) {
    auto vm = &*g_context;
    always_assert_flog(
      check(tv, ctx),
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
      return val.val().parr->isLegacyArray() ? "varray" : annotTypeName(AnnotType::Vec);
    }
    case KindOfPersistentDict:
    case KindOfDict:          {
      return val.val().parr->isLegacyArray() ? "darray" : annotTypeName(AnnotType::Dict);
    }
    case KindOfPersistentKeyset:
    case KindOfKeyset:        return annotTypeName(AnnotType::Keyset);
    case KindOfResource: {
      auto pres = val.val().pres;
      // pres should never be null - but sometimes it is anyway so let's guard
      // against that.
      return pres ? pres->data()->o_getClassName().c_str() : "resource(null)";
    }
    case KindOfRFunc:         return "reified function";
    case KindOfFunc:          return "func";
    case KindOfClass:         return "class";
    case KindOfLazyClass:        return "lazy class";
    case KindOfClsMeth:       return "clsmeth";
    case KindOfRClsMeth:      return "reified clsmeth";
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
      (isObject() && interface_supports_string(clsName())) ||
      (isUnresolved() && interface_supports_string(typeName()))) {
    return true;
  }
  if (!isUnresolved()) return false;
  auto p = getNamedTypeWithAutoload(typeNamedType(), typeName());
  if (auto ptd = boost::get<const TypeAlias*>(&p)) {
    auto td = *ptd;
    for (auto const& [type, klass] : td->typeAndClassUnion()) {
      if (type == AnnotType::String ||
          type == AnnotType::ArrayKey ||
          (type == AnnotType::Object &&
            interface_supports_string(klass->name()))) {
        return true;
      }
    }
    return false;
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

bool TypeConstraint::tryCommonCoercions(tv_lval val, const Class* ctx,
                                        const Class* propDecl) const {
  if (ctx && isThis() && val.type() == KindOfObject) {
    auto const cls = val.val().pobj->getVMClass();
    if (cls->preClass()->userAttributes().count(s___MockClass.get()) &&
        cls->parent() == ctx) {
      return true;
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
    return true;
  }

  return false;
}

bool TypeConstraint::maybeStringCompatible() const {
  return
    isString() || isArrayKey() || isUnresolved() ||
    (isObject() && interface_supports_string(clsName()));
}

MaybeDataType TypeConstraint::asSystemlibType() const {
  // Nullable and soft types are generally unknown: don't give an exact type.
  if (isNullable() || isSoft()) return std::nullopt;
  // TODO(T124220067) `noreturn` and `nothing` are their own non-"precise" types
  // under the hood but for systemlib both of these become KindOfNull. The JIT
  // and HPHP::Native::callFunc both expect this.
  if (isNoReturn() || isNothing()) return KindOfNull;
  // This is a kludge that replicates some unfortunate implicit behavior in
  // systemlib: all unresolved types are assumed to be `KindOfObject`.
  if (isUnresolved()) return KindOfObject;
  return underlyingDataType();
}

void TypeConstraint::verifyParamFail(tv_lval val,
                                     const Class* ctx,
                                     const Func* func,
                                     int paramNums) const {
  verifyFail(val, ctx, func, paramNums);
  assertx(
    isSoft() ||
    isThis() ||
    (RO::EvalEnforceGenericsUB < 2 && isUpperBound()) ||
    check(val, ctx)
  );
}

void TypeConstraint::verifyOutParamFail(TypedValue* c,
                                        const Class* ctx,
                                        const Func* func,
                                        int paramNum) const {
  if (tryCommonCoercions(c, ctx, nullptr)) return;

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

void TypeConstraint::verifyPropFail(const Class* thisCls,
                                    const Class* declCls,
                                    tv_lval val,
                                    const StringData* propName,
                                    bool isStatic) const {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(validForProp());

  if (tryCommonCoercions(val, thisCls, declCls)) return;

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

void TypeConstraint::validForPropFail(const Class* declCls,
                                      const StringData* propName) const {
  // (Mostly) match the error that HackC produces during parsing.
  raise_error(
    "Invalid property type-hint for '%s::$%s' (via '%s')",
    declCls->name()->data(),
    propName->data(),
    typeName()->data()
  );
}

void TypeConstraint::verifyFail(tv_lval c, const Class* ctx, const Func* func,
                                int id) const {
  if (tryCommonCoercions(c, ctx, nullptr)) return;

  std::string name = displayName(func->cls());
  auto const givenType = describe_actual_type(c);

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
    auto cls = Class::lookup(typeName());
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

void TypeConstraint::resolveType(AnnotType t,
                                 bool nullable,
                                 LowStringPtr clsName) {
  assertx(m_u.single.type == AnnotType::Unresolved);
  assertx(t != AnnotType::Unresolved);
  assertx(IMPLIES(t == AnnotType::Object, clsName != nullptr));
  assertx(IMPLIES(clsName != nullptr,
                  t == AnnotType::Object || enumSupportsAnnot(t)));
  auto flags = m_flags | TypeConstraintFlags::Resolved;
  if (nullable) flags |= TypeConstraintFlags::Nullable;
  m_flags = static_cast<TypeConstraintFlags>(flags);
  m_u.single.type = t;
  m_u.single.object.m_clsName = clsName;
}

void TypeConstraint::unresolve() {
  m_flags = static_cast<TypeConstraintFlags>(m_flags & ~TypeConstraintFlags::Resolved);
  m_u.single.type = AnnotType::Unresolved;
  m_u.single.object.m_clsName = nullptr;
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
      if (!dt) return MK::None;
      switch (*dt) {
        case KindOfBoolean:
          return tc.isNullable() ? MK::BoolOrNull : MK::Bool;
        case KindOfInt64:
          return tc.isNullable() ? MK::IntOrNull : MK::Int;
        case KindOfPersistentString:
        case KindOfString:
        case KindOfLazyClass:
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
        case KindOfNull:         return MK::None;
        case KindOfUninit:
        case KindOfRFunc:
        case KindOfFunc:
        case KindOfRClsMeth:
        case KindOfClass:
          always_assert_flog(false, "Unexpected DataType");
      }
      not_reached();
    }
    case AnnotMetaType::ArrayKey:
      return tc.isNullable() ? MK::None : MK::IntOrStr;
    case AnnotMetaType::Classname:
      return tc.isNullable() ? MK::StrOrNull : MK::Str;
    case AnnotMetaType::Mixed:
    case AnnotMetaType::Nothing:
    case AnnotMetaType::NoReturn:
    case AnnotMetaType::Nonnull:
    case AnnotMetaType::This:
    case AnnotMetaType::Callable:
    case AnnotMetaType::Number:
    case AnnotMetaType::VecOrDict:
    case AnnotMetaType::ArrayLike:
    case AnnotMetaType::Unresolved:
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
  auto const tcFlags = tc.flags() & ~TypeConstraintFlags::TypeVar;
  ub.addFlags(static_cast<TypeConstraintFlags>(tcFlags));
}
}
