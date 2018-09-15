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

#include "hphp/runtime/vm/unit-util.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template <class TraitMethod, class Ops>
inline void
TraitMethodImportData<TraitMethod, Ops>
::add(const TraitMethod& tm, const String& name) {
  if (Ops::exclude(name)) return;

  if (tm.modifiers & AttrTakesInOutParams) {
    assertx(name == Ops::methName(tm.method));
    auto const wrapper = Ops::findTraitMethod(tm.trait,
                                              stripInOutSuffix(name));
    assertx(wrapper);
    m_inoutMethods.emplace(wrapper, tm);
    return;
  }

  auto const found = m_dataForName.count(name);

  m_dataForName[name].methods.push_back(tm);
  if (!found) m_orderedNames.push_back(name);
}

template <class TraitMethod, class Ops>
inline void
TraitMethodImportData<TraitMethod, Ops>
::add(const TraitMethod& tm,
      const String& aliasedName,
      const String& origName) {
  if (Ops::exclude(aliasedName)) return;

  add(tm, aliasedName);

  assertx(m_dataForName.count(origName));
  m_dataForName[origName].aliases.push_back(aliasedName);
}

template <class TraitMethod, class Ops>
inline void
TraitMethodImportData<TraitMethod, Ops>
::erase(const String& name) {
  // We don't bother erasing `name' from any name lists---since it will not
  // correspond to any NameData, it will be skipped during finalization anyway.
  m_dataForName.erase(name);
}

template <class TraitMethod, class Ops>
inline void
TraitMethodImportData<TraitMethod, Ops>
::setModifiers(const String& name,
               typename TraitMethod::class_type trait,
               Attr mods) {
  auto& methods = m_dataForName[name].methods;

  for (auto& tm : methods) {
    if (tm.trait == trait) {
      tm.modifiers = mods;
      return;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

template <class TraitMethod, class Ops>
inline void
TraitMethodImportData<TraitMethod, Ops>
::applyPrecRule(const PreClass::TraitPrecRule& rule,
                typename TraitMethod::class_type ctx) {
  auto methName          = rule.methodName();
  auto selectedTraitName = rule.selectedTraitName();
  auto otherTraitNames   = rule.otherTraitNames();

  auto it = m_dataForName.find(methName);
  if (it == m_dataForName.end()) {
    Ops::errorUnknownMethod(methName);
    return;
  }

  if (otherTraitNames.find(selectedTraitName) != otherTraitNames.end()) {
    Ops::errorInconsistentInsteadOf(Ops::findTraitClass(ctx, selectedTraitName),
                                    methName);
  }

  bool foundSelectedTrait = false;
  auto& methods = it->second.methods;

  for (auto next = methods.begin(); next != methods.end(); ) {
    auto cur = next++;
    auto availTraitName = Ops::clsName(cur->trait);

    if (availTraitName == selectedTraitName) {
      foundSelectedTrait = true;
    } else if (otherTraitNames.count(availTraitName)) {
      otherTraitNames.erase(availTraitName);
      methods.erase(cur);
    }
  }

  if (!foundSelectedTrait) {
    Ops::errorUnknownTrait(selectedTraitName);
  }
  for (auto const& traitName : otherTraitNames) {
    if (auto trait = Ops::findTraitClass(ctx, traitName)) {
      if (Ops::findTraitMethod(trait, methName)) {
        // The trait exists, and defines the method, but it wasn't in methods,
        // so it must have been removed by a previous prec rule.
        Ops::errorMultiplyExcluded(traitName, methName);
      }
    } else {
      Ops::errorUnknownTrait(traitName);
    }
  }
}

template <class TraitMethod, class Ops>
inline void
TraitMethodImportData<TraitMethod, Ops>
::applyAliasRule(const PreClass::TraitAliasRule& rule,
                 typename TraitMethod::class_type ctx) {
  auto traitName    = rule.traitName();
  auto origMethName = rule.origMethodName();
  auto newMethName  = rule.newMethodName();
  auto modifiers    = rule.modifiers();

  typename TraitMethod::class_type traitCls;
  if (traitName->empty()) {
    traitCls = Ops::findSingleTraitWithMethod(ctx, origMethName);
  } else {
    traitCls = Ops::findTraitClass(ctx, traitName);
  }

  if (!traitCls || !Ops::isTrait(traitCls)) {
    Ops::errorUnknownTrait(traitName);
  }

  Ops::addTraitAlias(ctx, rule, traitCls);

  auto traitMeth = Ops::findTraitMethod(traitCls, origMethName);
  if (!traitMeth) {
    Ops::errorUnknownMethod(origMethName);
  }

  if (origMethName == newMethName) {
    setModifiers(origMethName, traitCls, modifiers);
  } else {
    add(Ops::traitMethod(traitCls, traitMeth, rule),
        newMethName, origMethName);
  }
}

/*
 * Remove trait abstract methods that are either (a) implemented by other
 * traits, or (b) duplicated.
 */
template <class TraitMethod, class Ops>
inline void
TraitMethodImportData<TraitMethod, Ops>
::removeSpareTraitAbstractMethods() {
  for (auto& nameData : m_dataForName) {
    auto& methods = nameData.second.methods;

    bool hasNonAbstractMeth = false;
    unsigned numAbstractMeths = 0;

    for (auto const& tm : methods) {
      if (!Ops::isAbstract(tm.modifiers)) {
        hasNonAbstractMeth = true;
      } else {
        ++numAbstractMeths;
      }
    }

    if (hasNonAbstractMeth || numAbstractMeths > 1) {
      bool firstAbstractMeth = true;

      // Erase spare abstract declarations.
      for (auto next = methods.begin(); next != methods.end(); ) {
        auto cur = next++;

        if (Ops::isAbstract(cur->modifiers)) {
          if (hasNonAbstractMeth || !firstAbstractMeth) {
            methods.erase(cur);
          }
          firstAbstractMeth = false;
        }
      }
    }
  }
}

template <class TraitMethod, class Ops>
inline auto
TraitMethodImportData<TraitMethod, Ops>
::finish(typename TraitMethod::class_type ctx) {
  removeSpareTraitAbstractMethods();

  std::unordered_set<String> seenNames;
  std::vector<MethodData> output;

  seenNames.reserve(m_orderedNames.size());
  output.reserve(m_orderedNames.size());

  auto process = [&] (const String& name) {
    if (seenNames.count(name)) return;
    auto const& methods = m_dataForName[name].methods;

    // The rules eliminated this method from all traits.
    if (methods.size() == 0) return;

    if (methods.size() > 1) {
      // This may or may not actually throw; if it doesn't, the client is okay
      // with the duplication.
      Ops::errorDuplicateMethod(ctx, name);
    }

    seenNames.insert(name);
    auto const &front = *methods.begin();
    output.push_back({name, front});
    auto it = m_inoutMethods.find(front.method);
    if (it != m_inoutMethods.end()) {
      auto tm = front;
      tm.method = it->second.method;
      auto userName = Ops::methName(front.method);
      auto internalName = Ops::methName(tm.method);
      auto sfx = folly::StringPiece(
          internalName->data() + userName->size(),
          internalName->data() + internalName->size()
      );
      output.push_back(
          {
            makeStaticString(folly::to<std::string>(name->slice(), sfx)), tm
          }
      );
    }
  };

  for (auto const& name : m_orderedNames) {
    auto const& nameData = m_dataForName[name];

    // Aliases of a given method name are always ordered immediately before
    // that name (in the order the aliases were declared in), even if
    // precedence rules override them.
    for (auto const& alias : nameData.aliases) {
      process(alias);
    }
    process(name);
  }

  return output;
}

///////////////////////////////////////////////////////////////////////////////
}
