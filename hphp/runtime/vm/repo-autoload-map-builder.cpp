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

#include "hphp/runtime/vm/repo-autoload-map-builder.h"

#include "hphp/runtime/vm/constant.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/type-alias-emitter.h"

#include <boost/algorithm/string/predicate.hpp>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

void RepoAutoloadMapBuilder::addUnit(const UnitEmitter& ue) {
  auto unitSn = ue.m_sn;
  assertx(unitSn != -1 && "unitSn is invalid");

  for (auto const pce : ue.preclasses()) {
    if (!boost::starts_with(pce->name()->slice(), "Closure$")) {
      m_types.emplace(pce->name(), unitSn);
    }
  }
  for (auto& fe : ue.fevec()) {
    if (!fe->isMethod()
        && !boost::ends_with(fe->name->slice(), "$memoize_impl")) {
      m_funcs.emplace(fe->name, unitSn);
    }
  }
  for (auto& te : ue.typeAliases()) {
    m_typeAliases.emplace(te->name(), unitSn);
  }

  for (auto& c : ue.constants()) {
    m_constants.emplace(c.name, unitSn);
  }

  for (auto& m : ue.modules()) {
    m_modules.emplace(m.name, unitSn);
  }
}

const RepoAutoloadMapBuilder::CaseInsensitiveMap&
RepoAutoloadMapBuilder::getTypes() const {
  return m_types;
}

const RepoAutoloadMapBuilder::CaseInsensitiveMap&
RepoAutoloadMapBuilder::getFuncs() const {
  return m_funcs;
}

const RepoAutoloadMapBuilder::CaseSensitiveMap&
RepoAutoloadMapBuilder::getConstants() const {
  return m_constants;
}

const RepoAutoloadMapBuilder::CaseInsensitiveMap&
RepoAutoloadMapBuilder::getTypeAliases() const {
  return m_typeAliases;
}

const RepoAutoloadMapBuilder::CaseSensitiveMap&
RepoAutoloadMapBuilder::getModules() const {
  return m_modules;
}

//////////////////////////////////////////////////////////////////////

}
