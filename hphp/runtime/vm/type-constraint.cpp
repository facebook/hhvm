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
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/ext/std/ext_std_function.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

//////////////////////////////////////////////////////////////////////

const StaticString s___invoke("__invoke"),
  s_array{"array"},
  s_varray{"HH\\varray"},
  s_darray{"HH\\darray"},
  s_varray_or_darray{"HH\\varray_or_darray"},
  s_vec{"HH\\vec"},
  s_dict{"HH\\dict"};

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

std::string TypeConstraint::displayName(const Class* context /*= nullptr*/,
                                        bool extra /* = false */) const {
  const StringData* tn = typeName();
  std::string name;
  if (isSoft()) {
    name += '@';
  }
  if (isNullable() && isExtended()) {
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
                   !strcasecmp(stripped, "this"));
          break;
        case 5: strip = !strcasecmp(stripped, "float"); break;
        case 6: strip = !strcasecmp(stripped, "string"); break;
        case 7: strip = !strcasecmp(stripped, "nonnull"); break;
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
      case AnnotType::NoReturn: str = "noreturn"; break;
      case AnnotType::Null:     str = "null"; break;
      case AnnotType::Bool:     str = "bool"; break;
      case AnnotType::Int:      str = "int";  break;
      case AnnotType::Float:    str = "float"; break;
      case AnnotType::String:   str = "string"; break;
      case AnnotType::Array:    str = "array"; break;
      case AnnotType::Resource: str = "resource"; break;
      case AnnotType::Dict:     str = "dict"; break;
      case AnnotType::Vec:      str = "vec"; break;
      case AnnotType::Keyset:   str = "keyset"; break;
      case AnnotType::Number:   str = "num"; break;
      case AnnotType::ArrayKey: str = "arraykey"; break;
      case AnnotType::VArray:   str = "varray"; break;
      case AnnotType::DArray:   str = "darray"; break;
      case AnnotType::VArrOrDArr: str = "varray_or_darray"; break;
      case AnnotType::VecOrDict: str = "vec_or_dict"; break;
      case AnnotType::ArrayLike: str = "arraylike"; break;
      case AnnotType::Nonnull:  str = "nonnull"; break;
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

    auto const is_array = [](const StringData* str) {
      return str->isame(s_array.get()) ||
             str->isame(s_varray.get()) ||
             str->isame(s_darray.get()) ||
             str->isame(s_varray_or_darray.get());
    };
    if (is_array(m_typeName) && is_array(other.m_typeName)) {
      return true;
    }

    const Class* cls = Unit::lookupClass(m_typeName);
    const Class* otherCls = Unit::lookupClass(other.m_typeName);

    return cls && otherCls && (isHHType() ? otherCls->classof(cls)
                                          : cls == otherCls);
  }

  return false;
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
std::pair<const TypeAliasReq*, Class*>
getTypeAliasOrClassWithAutoload(const NamedEntity* ne, const StringData* name) {

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

  assertx(!def || !klass);
  return std::make_pair(def, klass);
}

}

MaybeDataType TypeConstraint::underlyingDataTypeResolved() const {
  assertx(!isSelf() && !isParent() && !isCallable());
  assertx(IMPLIES(
    !hasConstraint() || isTypeVar() || isTypeConstant(),
    isMixed()));

  if (!isPrecise()) {
    if (isVArray() || isDArray() || isVArrayOrDArray()) return KindOfArray;
    return folly::none;
  }

  auto t = underlyingDataType();
  assertx(t);

  // If we aren't a class or type alias, nothing special to do.
  if (!isObject()) return t;

  assertx(t == KindOfObject);
  auto p = getTypeAliasOrClassWithAutoload(m_namedEntity, m_typeName);
  auto td = p.first;
  auto c = p.second;

  // See if this is a type alias.
  if (td) {
    if (td->type != Type::Object) {
      auto const metatype = getAnnotMetaType(td->type);
      if (metatype == MetaType::Precise) {
        t = getAnnotDataType(td->type);
      } else if (metatype == MetaType::VArray || metatype == MetaType::DArray ||
                 metatype == MetaType::VArrOrDArr) {
        t = KindOfArray;
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

  if (type() == other.type()) return false;
  if (!RuntimeOption::EvalHackArrCompatTypeHintNotices) {
    return !isArray() || !other.isArray();
  }
  return true;
}

TypeConstraint::EquivalentResult
TypeConstraint::equivalentForProp(const TypeConstraint& other) const {
  assertx(validForProp());
  assertx(other.validForProp());

  if (isSoft() != other.isSoft()) return EquivalentResult::Fail;

  if (isObject() && other.isObject() &&
      isNullable() == other.isNullable() &&
      m_typeName->isame(other.m_typeName)) {
    // We can avoid having to resolve the type-hint if they have the same name.
    return EquivalentResult::Pass;
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
      case MetaType::VArray:
      case MetaType::DArray:
      case MetaType::VArrOrDArr:
      case MetaType::ArrayLike:
      case MetaType::Precise:
        if (!tc.isObject()) {
          return std::make_tuple(tc.type(), nullptr, tc.isNullable());
        }
        break;
      case MetaType::NoReturn:
      case MetaType::Self:
      case MetaType::Parent:
      case MetaType::Callable:
      case MetaType::Mixed:
        always_assert(false);
    }

    assertx(tc.isObject());

    const TypeAliasReq* tyAlias;
    Class* klass;
    std::tie(tyAlias, klass) =
      getTypeAliasOrClassWithAutoload(tc.m_namedEntity, tc.m_typeName);

    auto nullable = tc.isNullable();
    auto at = AnnotType::Object;
    if (tyAlias) {
      nullable |= tyAlias->nullable;
      at = tyAlias->type;
      klass = (at == Type::Object) ? tyAlias->klass : nullptr;
    }

    if (klass && isEnum(klass)) {
      auto const maybeDt = klass->enumBaseTy();
      at = maybeDt ? dataTypeToAnnotType(*maybeDt) : AnnotType::ArrayKey;
      klass = nullptr;
    }

    if (at == AnnotType::Mixed) nullable = false;
    return std::make_tuple(at, klass, nullable);
  };

  auto const resolved1 = resolve(*this);
  auto const resolved2 = resolve(other);
  if (resolved1 != resolved2) {
    // The resolutions aren't the same. This still might be okay once you take
    // into account d/varrays.
    auto const resType1 = std::get<0>(resolved1);
    auto const resType2 = std::get<0>(resolved2);
    auto const isArray1 =
      resType1 == AnnotType::Array ||
      resType1 == AnnotType::VArray ||
      resType1 == AnnotType::DArray ||
      resType1 == AnnotType::VArrOrDArr;
    auto const isArray2 =
      resType2 == AnnotType::Array ||
      resType2 == AnnotType::VArray ||
      resType2 == AnnotType::DArray ||
      resType2 == AnnotType::VArrOrDArr;
    if (!isArray1 || !isArray2 ||
        std::get<2>(resolved1) != std::get<2>(resolved2)) {
      return EquivalentResult::Fail;
    }
    return RuntimeOption::EvalHackArrCompatTypeHintNotices
      ? EquivalentResult::DVArray
      : EquivalentResult::Pass;
  }

  // The resolutions are the same. However, both could have failed to resolve to
  // anything. In that case, rely on their name.
  auto const resType = std::get<0>(resolved1);
  if (resType == AnnotType::Object && !std::get<1>(resolved1)) {
    return m_typeName->isame(other.m_typeName)
      ? EquivalentResult::Pass
      : EquivalentResult::Fail;
  }
  return EquivalentResult::Pass;
}

template <bool Assert, bool ForProp>
bool TypeConstraint::checkTypeAliasNonObj(const TypedValue* tv) const {
  assertx(tv->m_type != KindOfObject);
  assertx(isObject());

  auto const p = [&]() -> std::pair<const TypeAliasReq*, Class*> {
    if (!Assert) {
      return getTypeAliasOrClassWithAutoload(m_namedEntity, m_typeName);
    }
    if (auto const def = m_namedEntity->getCachedTypeAlias()) {
      return std::make_pair(def, nullptr);
    }
    if (auto const klass = Unit::lookupClass(m_namedEntity)) {
      return std::make_pair(nullptr, klass);
    }
    return std::make_pair(nullptr, nullptr);
  }();
  auto td = p.first;
  auto c = p.second;

  if (Assert && !td && !c) return true;

  // Common case is that we actually find the alias:
  if (td) {
    assertx(td->type != AnnotType::Self && td->type != AnnotType::Parent);
    if (td->nullable && tv->m_type == KindOfNull) return true;
    auto result = annotCompat(tv->m_type, td->type,
                              td->klass ? td->klass->name() : nullptr);
    switch (result) {
      case AnnotAction::Pass: return true;
      case AnnotAction::Fail: return false;
      case AnnotAction::CallableCheck:
        return !ForProp && (Assert || is_callable(tvAsCVarRef(tv)));
      case AnnotAction::ObjectCheck: break;
      case AnnotAction::VArrayCheck:
        assertx(tvIsArray(tv));
        return Assert || tv->m_data.parr->isVArray();
      case AnnotAction::DArrayCheck:
        assertx(tvIsArray(tv));
        return Assert || tv->m_data.parr->isDArray();
      case AnnotAction::VArrayOrDArrayCheck:
        assertx(tvIsArray(tv));
        return Assert || !tv->m_data.parr->isNotDVArray();
      case AnnotAction::NonVArrayOrDArrayCheck:
        assertx(tvIsArray(tv));
        return Assert || tv->m_data.parr->isNotDVArray();
    }
    assertx(result == AnnotAction::ObjectCheck);
    assertx(td->type == AnnotType::Object);
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
      return isIntType(tv->m_type) || isStringType(tv->m_type);
    }
  }
  return false;
}

template <bool Assert>
bool TypeConstraint::checkTypeAliasObjImpl(const Class* cls) const {
  assertx(isObject() && m_namedEntity && m_typeName);

  // Look up the type alias (autoloading if necessary)
  // and fail if we can't find it
  auto const td = [&]{
    if (!Assert) {
      return getTypeAliasWithAutoload(m_namedEntity, m_typeName);
    }
    return m_namedEntity->getCachedTypeAlias();
  }();
  if (!td) return Assert;

  // We found the type alias, check if an object of type cls
  // is compatible
  switch (getAnnotMetaType(td->type)) {
    case AnnotMetaType::Precise:
      return td->type == AnnotType::Object && td->klass &&
        cls->classof(td->klass);
    case AnnotMetaType::Mixed:
    case AnnotMetaType::Nonnull:
      return true;
    case AnnotMetaType::Callable:
      return cls->lookupMethod(s___invoke.get()) != nullptr;
    case AnnotMetaType::NoReturn:
    case AnnotMetaType::Number:
    case AnnotMetaType::ArrayKey:
    case AnnotMetaType::This:
    case AnnotMetaType::VArray:
    case AnnotMetaType::DArray:
    case AnnotMetaType::VArrOrDArr:
    case AnnotMetaType::VecOrDict:
    case AnnotMetaType::ArrayLike:
      return false;
    case AnnotMetaType::Self:
    case AnnotMetaType::Parent:
      // These should never happen, because type aliases are not allowed to use
      // those MetaTypes
      always_assert(false);
  }
  not_reached();
}

template bool TypeConstraint::checkTypeAliasObjImpl<false>(const Class*) const;
template bool TypeConstraint::checkTypeAliasObjImpl<true>(const Class*) const;

template <TypeConstraint::CheckMode Mode>
bool TypeConstraint::checkImpl(const TypedValue* tv,
                               const Class* context) const {
  assertx(isCheckable());
  assertx(tvIsPlausible(*tv));

  auto const isAssert = Mode == CheckMode::Assert;
  auto const isPasses = Mode == CheckMode::AlwaysPasses;
  auto const isProp   = Mode == CheckMode::ExactProp;

  // We shouldn't provide a context for the conservative checks.
  assertx(!isAssert || !context);
  assertx(!isPasses || !context);
  assertx(!isProp   || validForProp());

  tv = tvToCell(tv);
  if (isNullable() && tv->m_type == KindOfNull) return true;

  if (tv->m_type == KindOfObject) {
    // Perfect match seems common enough to be worth skipping the hash
    // table lookup.
    const Class *c = nullptr;
    if (isObject()) {
      if (m_typeName->isame(tv->m_data.pobj->getVMClass()->name())) {
        return true;
      }
      if (!isPasses) {
        // We can't save the Class* since it moves around from request to
        // request.
        assertx(m_namedEntity);
        c = Unit::lookupClass(m_namedEntity);
      }
    } else {
      switch (metaType()) {
        case MetaType::Self:
          assertx(!isProp);
          if (isAssert) return true;
          if (isPasses) return false;
          c = context;
          break;
        case MetaType::This:
          if (isAssert) return true;
          if (isPasses) return false;
          if (isProp) return tv->m_data.pobj->getVMClass() == context;
          switch (RuntimeOption::EvalThisTypeHintLevel) {
            case 0:   // Like Mixed.
              return true;
              break;
            case 1:   // Like Self.
              c = context;
              break;
            case 2:   // Soft this in irgen verifyTypeImpl and verifyFail.
            case 3:   // Hard this.
              if (auto const cls = getThis()) {
                return tv->m_data.pobj->getVMClass() == cls;
              }
              return false;
          }
          break;
        case MetaType::Parent:
          assertx(!isProp);
          if (isAssert) return true;
          if (isPasses) return false;
          if (context) c = context->parent();
          break;
        case MetaType::Callable:
          assertx(!isProp);
          if (isAssert) return true;
          if (isPasses) return false;
          return is_callable(tvAsCVarRef(tv));
        case MetaType::Precise:
        case MetaType::NoReturn:
        case MetaType::Number:
        case MetaType::ArrayKey:
        case MetaType::VArray:
        case MetaType::DArray:
        case MetaType::VArrOrDArr:
        case MetaType::VecOrDict:
        case MetaType::ArrayLike:
          return false;
        case MetaType::Nonnull:
          return true;
        case MetaType::Mixed:
          // We assert'd at the top of this function that the
          // metatype cannot be Mixed
          not_reached();
      }
    }
    if (c && tv->m_data.pobj->instanceof(c)) {
      return true;
    }
    return isObject() && !isPasses &&
      checkTypeAliasObjImpl<isAssert>(tv->m_data.pobj->getVMClass());
  }

  auto const result = annotCompat(tv->m_type, m_type, m_typeName);
  switch (result) {
    case AnnotAction::Pass: return true;
    case AnnotAction::Fail: return false;
    case AnnotAction::CallableCheck:
      assertx(!isProp);
      if (isAssert) return true;
      if (isPasses) return false;
      return is_callable(tvAsCVarRef(tv));
    case AnnotAction::ObjectCheck:
      assertx(isObject());
      return !isPasses && checkTypeAliasNonObj<isAssert, isProp>(tv);
    case AnnotAction::VArrayCheck:
      // Since d/varray type-hints are always soft, we can never assert on their
      // correctness.
      assertx(tvIsArray(tv));
      return isAssert || tv->m_data.parr->isVArray();
    case AnnotAction::DArrayCheck:
      assertx(tvIsArray(tv));
      return isAssert || tv->m_data.parr->isDArray();
    case AnnotAction::VArrayOrDArrayCheck:
      assertx(tvIsArray(tv));
      return isAssert || !tv->m_data.parr->isNotDVArray();
    case AnnotAction::NonVArrayOrDArrayCheck:
      assertx(tvIsArray(tv));
      return isAssert || tv->m_data.parr->isNotDVArray();
  }
  not_reached();
}

template bool TypeConstraint::checkImpl<TypeConstraint::CheckMode::Exact>(
  const TypedValue*,
  const Class*
) const;
template bool TypeConstraint::checkImpl<TypeConstraint::CheckMode::ExactProp>(
  const TypedValue*,
  const Class*
) const;
template bool TypeConstraint::checkImpl<TypeConstraint::CheckMode::AlwaysPasses>(
  const TypedValue*,
  const Class*
) const;
template bool TypeConstraint::checkImpl<TypeConstraint::CheckMode::Assert>(
  const TypedValue*,
  const Class*
) const;

void TypeConstraint::verifyParam(TypedValue* tv,
                                 const Func* func,
                                 int paramNum) const {
  if (UNLIKELY(!check(tv, func->cls()))) {
    verifyParamFail(func, tv, paramNum);
  }
}

void TypeConstraint::verifyReturn(TypedValue* tv, const Func* func) const {
  if (UNLIKELY(!check(tv, func->cls()))) {
    verifyReturnFail(func, tv);
  }
}

void TypeConstraint::verifyOutParam(const TypedValue* tv,
                                    const Func* func,
                                    int paramNum) const {
  if (UNLIKELY(!check(tv, func->cls()))) {
    verifyOutParamFail(func, tv, paramNum);
  }
}

void TypeConstraint::verifyProperty(const TypedValue* tv,
                                    const Class* thisCls,
                                    const Class* declCls,
                                    const StringData* propName) const {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(validForProp());
  if (UNLIKELY(!checkImpl<CheckMode::ExactProp>(tv, thisCls))) {
    verifyPropFail(thisCls, declCls, tv, propName, false);
  }
}

void TypeConstraint::verifyStaticProperty(const TypedValue* tv,
                                          const Class* thisCls,
                                          const Class* declCls,
                                          const StringData* propName) const {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(validForProp());
  if (UNLIKELY(!checkImpl<CheckMode::ExactProp>(tv, thisCls))) {
    verifyPropFail(thisCls, declCls, tv, propName, true);
  }
}

void TypeConstraint::verifyReturnNonNull(TypedValue* tv,
                                         const Func* func) const {
  const auto DEBUG_ONLY tc = func->returnTypeConstraint();
  assertx(!tc.isNullable());
  if (UNLIKELY(cellIsNull(tv))) {
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

const char* describe_actual_type(const TypedValue* tv, bool isHHType) {
  tv = tvToCell(tv);
  switch (tv->m_type) {
    case KindOfUninit:        return "undefined variable";
    case KindOfNull:          return "null";
    case KindOfBoolean:       return "bool";
    case KindOfInt64:         return "int";
    case KindOfDouble:        return isHHType ? "float" : "double";
    case KindOfPersistentString:
    case KindOfString:        return "string";
    case KindOfPersistentVec:
    case KindOfVec:           return "HH\\vec";
    case KindOfPersistentDict:
    case KindOfDict:          return "HH\\dict";
    case KindOfPersistentKeyset:
    case KindOfKeyset:        return "HH\\keyset";
    case KindOfPersistentArray:
    case KindOfArray:         return "array";
    case KindOfObject:        return tv->m_data.pobj->getClassName().c_str();
    case KindOfResource:
      return tv->m_data.pres->data()->o_getClassName().c_str();
    case KindOfFunc:          return "func";
    case KindOfClass:         return "class";
    case KindOfRef:
      break;
  }
  not_reached();
}

bool call_uses_strict_types(const Func* callee) {
  auto outer = [&] {
    if (vmfp()->sfp()) return vmfp()->sfp()->func();
    auto const next = g_context->getPrevVMState(vmfp());
    return next ? next->func() : nullptr;
  };
  auto const caller =

  (vmfp()->func() == callee) ? outer() : vmfp()->func();
  if (!caller) {
    // For example, HHBBC calling Foldable functions
    return true;
  }

  if (callee->isBuiltin()) {
    return caller->unit()->useStrictTypesForBuiltins();
  }

  if (LIKELY(RuntimeOption::EnableHipHopSyntax)) {
    return true;
  }

  /* In PHP, if you have a function 'foo', and call array_map('foo', ...),
   * array_map makes a call to 'foo', and that should *never* be strict in PHP
   * mode, even if both foo and the call to array_map are in a strict file.
   */
  if (caller->isBuiltin() && !callee->isBuiltin()) {
    return callee->unit()->isHHFile();
  }

  // Neither is builtin
  return caller->unit()->useStrictTypes();
}

ALWAYS_INLINE
folly::Optional<AnnotType> TypeConstraint::checkDVArray(const Cell* c) const {
  auto const check = [&](AnnotType at) -> folly::Optional<AnnotType> {
    switch (at) {
      case AnnotType::Array:
        assertx(!c->m_data.parr->isNotDVArray());
        break;
      case AnnotType::VArray:
        assertx(!c->m_data.parr->isVArray());
        break;
      case AnnotType::DArray:
        assertx(!c->m_data.parr->isDArray());
        break;
      case AnnotType::VArrOrDArr:
        assertx(c->m_data.parr->isNotDVArray());
        break;
      default:
        return folly::none;
    }
    return at;
  };
  if (LIKELY(!RuntimeOption::EvalHackArrCompatTypeHintNotices)) {
    return folly::none;
  }
  if (!isArrayType(c->m_type)) return folly::none;
  if (isArray()) return check(m_type);
  if (!isObject()) return folly::none;
  if (auto alias = getTypeAliasWithAutoload(m_namedEntity, m_typeName)) {
    return check(alias->type);
  }
  return folly::none;
}

void TypeConstraint::verifyParamFail(const Func* func, TypedValue* tv,
                                     int paramNums) const {
  verifyFail(func, tv, paramNums);
  assertx(
    isSoft() || !RuntimeOption::EvalHardTypeHints ||
    (isThis() && couldSeeMockObject()) ||
    (RuntimeOption::EvalHackArrCompatTypeHintNotices &&
     isArrayType(tv->m_type)) ||
    check(tv, func->cls())
  );
}

void TypeConstraint::verifyOutParamFail(const Func* func,
                                        const TypedValue* tv,
                                        int paramNum) const {
  auto const c = tvToCell(tv);
  if (auto const at = checkDVArray(c)) {
    raise_hackarr_compat_type_hint_outparam_notice(
      func, c->m_data.parr, *at, paramNum
    );
    return;
  }

  raise_return_typehint_error(
    folly::sformat(
      "Argument {} returned from {}() as an inout parameter must be of type "
      "{}, {} given",
      paramNum + 1,
      func->fullDisplayName(),
      displayName(func->cls()),
      describe_actual_type(tv, isHHType())
    )
  );
}

void TypeConstraint::verifyPropFail(const Class* thisCls,
                                    const Class* declCls,
                                    const TypedValue* tv,
                                    const StringData* propName,
                                    bool isStatic) const {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);
  assertx(validForProp());

  auto const c = tvToCell(tv);
  if (auto const at = checkDVArray(c)) {
    raise_hackarr_compat_type_hint_property_notice(
      declCls, c->m_data.parr, *at, propName, isStatic
    );
    return;
  }

  if (UNLIKELY(isThis() && c->m_type == KindOfObject)) {
    auto const valCls = c->m_data.pobj->getVMClass();
    if (valCls->preClass()->userAttributes().count(s___MockClass.get()) &&
        valCls->parent() == thisCls) {
      return;
    }
  }

  raise_property_typehint_error(
    folly::sformat(
      "{} '{}::{}' declared as type {}, {} assigned",
      isStatic ? "Static property" : "Property",
      declCls->name(),
      propName,
      displayName(nullptr),
      describe_actual_type(c, isHHType())
    ),
    isSoft()
  );
}

void TypeConstraint::verifyFail(const Func* func, TypedValue* tv,
                                int id) const {
  VMRegAnchor _;
  std::string name = displayName(func->cls());
  auto const givenType = describe_actual_type(tv, isHHType());

  auto const c = tvToCell(tv);

  if (auto const at = checkDVArray(c)) {
    if (id == ReturnId) {
      raise_hackarr_compat_type_hint_ret_notice(
        func,
        c->m_data.parr,
        *at
      );
    } else {
      raise_hackarr_compat_type_hint_param_notice(
        func,
        c->m_data.parr,
        *at,
        id
      );
    }
    return;
  }

  if (UNLIKELY(isThis() && c->m_type == KindOfObject)) {
    Class* cls = c->m_data.pobj->getVMClass();
    auto const thisClass = getThis();
    if (cls->preClass()->userAttributes().count(s___MockClass.get()) &&
        cls->parent() == thisClass) {
      return;
    }
  }

  const bool useStrictTypes = (id == ReturnId) ?
    LIKELY(RuntimeOption::EnableHipHopSyntax) ||
    func->isBuiltin() ||
    func->unit()->useStrictTypes() :
    call_uses_strict_types(func);

  if (UNLIKELY(!useStrictTypes)) {
    if (auto dt = underlyingDataType()) {
      // In non-strict mode we may be able to coerce a type failure. For object
      // typehints there is no possible coercion in the failure case, but HNI
      // builtins currently only guard on kind not class so the following wil
      // generate false positives for objects.
      if (*dt != KindOfObject) {
        // HNI conversions implicitly unbox references, this behavior is wrong,
        // in particular it breaks the way type conversion works for PHP 7
        // scalar type hints
        if (isRefType(tv->m_type)) {
          auto inner = tv->m_data.pref->var()->asTypedValue();
          if (tvCoerceParamInPlace(inner, *dt, func->isBuiltin())) {
            tvAsVariant(tv) = tvAsVariant(inner);
            return;
          }
        } else {
          if (tvCoerceParamInPlace(tv, *dt, func->isBuiltin())) {
            return;
          }
        }
      }
    }
  } else if (UNLIKELY((!func->unit()->isHHFile() || func->isBuiltin()) &&
                      !RuntimeOption::EnableHipHopSyntax)) {
    // PHP 7 allows for a widening conversion from Int to Float. We still ban
    // this in HH files.
    if (auto dt = underlyingDataType()) {
      if (*dt == KindOfDouble && tv->m_type == KindOfInt64 &&
          tvCoerceParamToDoubleInPlace(tv, func->isBuiltin())) {
        return;
      }
    }
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
          "Value returned from {}{} {}() must be of type {}, {} given",
          func->isAsync() ? "async " : "",
          func->preClass() ? "method" : "function",
          func->fullDisplayName(),
          name,
          givenType
        ).str();
    }
    if (RuntimeOption::EvalCheckReturnTypeHints >= 2 && !isSoft()
        && (!isThis() || RuntimeOption::EvalThisTypeHintLevel != 2)) {
      raise_return_typehint_error(msg);
    } else {
      raise_warning_unsampled(msg);
    }
    return;
  }

  // Handle implicit collection->array conversion for array parameter type
  // constraints
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
        id + 1, func->fullDisplayName(), name, givenType, id + 1
      ).str()
    );
    tvCastToArrayInPlace(tv);
    return;
  }

  // Handle parameter type constraint failures
  if (isExtended() &&
      (isSoft() || (isThis() && RuntimeOption::EvalThisTypeHintLevel == 2))) {
    // Soft extended type hints raise warnings instead of recoverable
    // errors, to ease migration.
    raise_warning_unsampled(
      folly::format(
        "Argument {} to {}() must be of type {}, {} given",
        id + 1, func->fullDisplayName(), name, givenType
      ).str()
    );
  } else if (isExtended() && isNullable()) {
    raise_typehint_error(
      folly::format(
        "Argument {} to {}() must be of type {}, {} given",
        id + 1, func->fullDisplayName(), name, givenType
      ).str()
    );
  } else {
    auto cls = Unit::lookupClass(m_typeName);
    if (cls && isInterface(cls)) {
      raise_typehint_error(
        folly::format(
          "Argument {} passed to {}() must implement interface {}, {} given",
          id + 1, func->fullDisplayName(), name, givenType
        ).str()
      );
    } else {
      raise_typehint_error(
        folly::format(
          "Argument {} passed to {}() must be an instance of {}, {} given",
          id + 1, func->fullDisplayName(), name, givenType
        ).str()
      );
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
      assertx(dt.hasValue());
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
        case KindOfPersistentArray:
        case KindOfArray:
        case KindOfResource:
        case KindOfNull:         return MK::None;
        case KindOfUninit:
        case KindOfRef:
        case KindOfFunc:
        case KindOfClass:
          always_assert_flog(false, "Unexpected DataType");
      }
      not_reached();
    }
    case AnnotMetaType::ArrayKey:
      return tc.isNullable() ? MK::None : MK::IntOrStr;
    case AnnotMetaType::Mixed:
    case AnnotMetaType::NoReturn:
    case AnnotMetaType::Nonnull:
    case AnnotMetaType::Self:
    case AnnotMetaType::This:
    case AnnotMetaType::Parent:
    case AnnotMetaType::Callable:
    case AnnotMetaType::Number:
    case AnnotMetaType::VArray:
    case AnnotMetaType::DArray:
    case AnnotMetaType::VArrOrDArr:
    case AnnotMetaType::VecOrDict:
    case AnnotMetaType::ArrayLike:
      return MK::None;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////
}
