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

#include "hphp/runtime/vm/type-alias.h"

#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/bespoke/logging-profile.h"
#include "hphp/runtime/vm/frame-restore.h"
#include "hphp/runtime/vm/named-entity-defs.h"
#include "hphp/runtime/vm/unit.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {

TypeAlias resolveTypeAlias(const PreTypeAlias* thisType, bool failIsFatal) {
  /*
   * If this type alias is a KindOfObject and the name on the right
   * hand side was another type alias, we will bind the name to the
   * other side for this request (i.e. resolve that type alias now).
   *
   * We need to inspect the right hand side and figure out what it was
   * first.
   *
   * If the right hand side was a class, we need to autoload and
   * ensure it exists at this point.
   */

  /*
   * If the right hand side is already defined, don't invoke the
   * autoloader at all, this means we have to check for both a type
   * alias and a class before attempting to load them via the
   * autoloader.
   */
  TypeAlias req(thisType);
  TypeConstraintFlags flags =
    thisType->value.flags() & (TypeConstraintFlags::Nullable
                               | TypeConstraintFlags::TypeVar
                               | TypeConstraintFlags::Soft
                               | TypeConstraintFlags::TypeConstant
                               | TypeConstraintFlags::UpperBound);

  std::vector<TypeConstraint> parts;
  auto const typeAliasFromClass = [&](Class* klass) {
    if (isEnum(klass)) {
      // If the class is an enum, pull out the actual base type.
      if (auto const enumType = klass->enumBaseTy()) {
        auto t = enumDataTypeToAnnotType(*enumType);
        assertx(t != AnnotType::Object);
        parts.emplace_back(t, flags);
      } else {
        parts.emplace_back(AnnotType::ArrayKey, flags);
      }
    } else {
      parts.emplace_back(AnnotType::Object, flags, TypeConstraint::ClassConstraint{*klass});
    }
  };

  auto const from = [&](const TypeAlias& ta) {
    if (ta.invalid) {
      req.invalid = true;
      return;
    }
    TypeConstraint value = ta.value;
    value.addFlags(flags);
    parts.emplace_back(value);
  };

  for (auto const& tc : eachTypeConstraintInUnion(thisType->value)) {
    auto type = tc.type();
    auto typeName = tc.typeName();
    if (type != AnnotType::Object && type != AnnotType::Unresolved) {
      parts.emplace_back(type, flags);
      continue;
    }
    auto targetNE = NamedType::getOrCreate(typeName);

    if (auto klass = targetNE->getCachedClass()) {
      typeAliasFromClass(klass);
      continue;
    }

    if (auto targetTd = targetNE->getCachedTypeAlias()) {
      assertx(type != AnnotType::Object);
      from(*targetTd);
      if (req.invalid) return req;
      continue;
    }

    if (failIsFatal &&
        AutoloadHandler::s_instance->autoloadTypeOrTypeAlias(
          StrNR(const_cast<StringData*>(typeName))
        )) {
      if (auto klass = targetNE->getCachedClass()) {
        typeAliasFromClass(klass);
        continue;
      }
      if (auto targetTd = targetNE->getCachedTypeAlias()) {
        assertx(type != AnnotType::Object);
        from(*targetTd);
        if (req.invalid) return req;
        continue;
      }
    }
    // could not resolve, it is invalid
    req.invalid = true;
    return req;
  }

  req.value = [&] {
    try {
      return TypeConstraint::makeUnion(req.name(), parts);
    } catch (const FatalErrorException& ex) {
      raise_error("%s in %s on line %d\n", ex.what(), thisType->unit->origFilepath()->data(), thisType->line0);
    }
  }();
  return req;
}

///////////////////////////////////////////////////////////////////////////////
} // namespace

bool TypeAlias::compat(const PreTypeAlias& alias) const {
  // FIXME(T116316964): can't compare type of unresolved PreTypeAlias

  if (value.isNullable() != alias.value.isNullable()) return false;

  auto view0 = eachTypeConstraintInUnion(value);
  auto view1 = eachTypeConstraintInUnion(alias.value);
  auto it0 = view0.begin();
  auto it1 = view1.begin();

  while (it0 != view0.end() && it1 != view1.end()) {
    auto tc0 = *it0++;
    auto type = tc0.type();
    auto klass = type == AnnotType::Object ? tc0.clsNamedType()->getCachedClass() : nullptr;
    auto tc1 = *it1++;
    auto ptype = tc1.type();
    auto value = tc1.typeName();
    auto const preType =
      ptype == AnnotType::Unresolved ? AnnotType::Object : ptype;
    if (ptype == AnnotType::Mixed && type == AnnotType::Mixed) continue;
    if (preType == type && Class::lookup(value) == klass) continue;
    return false;
  }
  return true;
}

size_t TypeAlias::stableHash() const {
  return folly::hash::hash_combine(
    name()->hashStatic(),
    unit()->sn()
  );
}

const Array TypeAlias::resolvedTypeStructure() const {
  auto const ts = m_preTypeAlias->resolvedTypeStructure;
  if (ts.isNull() || !ts.get()->isVanilla()) return ts;

  auto newTs = Array(ts.get());
  bespoke::profileArrLikeTypeAlias(this, &newTs);
  return newTs;
}

void TypeAlias::setResolvedTypeStructure(ArrayData* ad) {
  auto const preTA = const_cast<PreTypeAlias*>(m_preTypeAlias);
  preTA->resolvedTypeStructure = ad;
}

const TypeAlias* TypeAlias::lookup(const StringData* name,
                                   bool* persistent) {
  auto ne = NamedType::getOrCreate(name);
  auto target = ne->getCachedTypeAlias();
  if (persistent) *persistent = ne->isPersistentTypeAlias();
  return target;
}

const TypeAlias* TypeAlias::load(const StringData* name,
                                 bool* persistent) {
  auto ne = NamedType::getOrCreate(name);
  auto target = ne->getCachedTypeAlias();
  if (!target) {
    if (AutoloadHandler::s_instance->autoloadTypeOrTypeAlias(
          StrNR(const_cast<StringData*>(name))
        )) {
      target = ne->getCachedTypeAlias();
    } else {
      return nullptr;
    }
  }

  if (persistent) *persistent = ne->isPersistentTypeAlias();
  return target;
}

const TypeAlias* TypeAlias::def(const PreTypeAlias* thisType, bool failIsFatal) {
  auto nameList = NamedType::getOrCreate(thisType->name);

  /*
   * Check if this name already was defined as a type alias, and if so
   * make sure it is compatible.
   */
  if (auto current = nameList->getCachedTypeAlias()) {
    auto raiseIncompatible = [&] {
      FrameRestore _(thisType);
      raise_error("The type %s is already defined to an incompatible type",
                  thisType->name->data());
    };
    if (nameList->isPersistentTypeAlias()) {
      // We may have cached the fully resolved type in a previous request.
      if (resolveTypeAlias(thisType, failIsFatal) != *current) {
        if (!failIsFatal) return nullptr;
        raiseIncompatible();
      }
      return current;
    }
    if (!current->compat(*thisType)) {
      if (!failIsFatal) return nullptr;
      raiseIncompatible();
    }
    assertx(!RO::RepoAuthoritative);
    return current;
  }

  // There might also be a class with this name already.
  auto existingKind = nameList->checkSameName<PreTypeAlias>();
  if (existingKind) {
    if (!failIsFatal) return nullptr;
    FrameRestore _(thisType);
    raise_error("The name %s is already defined as a %s",
                thisType->name->data(), existingKind);
    not_reached();
  }

  auto resolved = resolveTypeAlias(thisType, failIsFatal);
  if (resolved.invalid) {
    if (!failIsFatal) return nullptr;
    FrameRestore _(thisType);
    std::vector<folly::StringPiece> names;
    for (auto const& tc : eachClassTypeConstraintInUnion(thisType->value)) {
      names.push_back(tc.typeName()->slice());
    }
    std::string combined = folly::join("|", names);
    raise_error("Unknown type or class %s", combined.c_str());
    not_reached();
  }

  auto const isPersistent = (thisType->attrs & AttrPersistent);
  if (debug && isPersistent) {
    for (DEBUG_ONLY auto const& tc : eachClassTypeConstraintInUnion(resolved.value)) {
      DEBUG_ONLY auto klass = tc.clsNamedType()->getCachedClass();
      assertx(classHasPersistentRDS(klass));
    }
  }

  nameList->m_cachedTypeAlias.bind(
    isPersistent ? rds::Mode::Persistent : rds::Mode::Normal,
    rds::LinkName{"TypeAlias", thisType->name},
    &resolved
  );
  if (!nameList->m_cachedTypeAlias.isPersistent()) {
    nameList->setCachedTypeAlias(resolved);
  }
  return nameList->getCachedTypeAlias();
}

///////////////////////////////////////////////////////////////////////////////
}
