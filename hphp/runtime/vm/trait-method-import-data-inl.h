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

/*
 * If a method has been imported multiple times via multiple use chains, remove
 * the duplicates.  Duplicates are detected comparing (name, originalClass)
 * information, which uniquely identifies a method.
 */
template <class TraitMethod, class Ops>
inline void
TraitMethodImportData<TraitMethod, Ops>
::removeDiamondDuplicates(const bool enableMethodTraitDiamond) {
  for (auto& nameData : m_dataForName) {
    auto& methods = nameData.second.methods;

    for (auto& method : methods) {
      nameData.second.methodOriginsWithDuplicates.push_back(Ops::clsName(method.trait));
    }

    if (RO::EvalDiamondTraitMethods && enableMethodTraitDiamond) {
      std::set<typename TraitMethod::origin_type> origins;
      methods.erase(
        std::remove_if(methods.begin(), methods.end(),
                      [&](const auto &m) {
                        return !origins.insert(Ops::originalClass(m.method)).second;
                      }),
        methods.end());
    }
  }
}

template <class TraitMethod, class Ops>
inline auto
TraitMethodImportData<TraitMethod, Ops>
::finish(typename TraitMethod::class_type ctx, const bool enableMethodTraitDiamond) {
  removeSpareTraitAbstractMethods();
  removeDiamondDuplicates(enableMethodTraitDiamond);

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
      // For error reporting, we keep the
      auto const& methodOriginsWithDuplicates = m_dataForName[name].methodOriginsWithDuplicates;
      Ops::errorDuplicateMethod(ctx, name, methodOriginsWithDuplicates);
    }

    seenNames.insert(name);
    auto const &front = *methods.begin();
    output.push_back({name, front});
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
