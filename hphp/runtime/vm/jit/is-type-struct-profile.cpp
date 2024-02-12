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

#include "hphp/runtime/vm/jit/is-type-struct-profile.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/configs/hhir.h"
#include "hphp/runtime/base/type-structure.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"

namespace HPHP::jit {

bool IsTypeStructProfile::shouldOptimize() const {
  auto const threshold = Cfg::HHIR::IsTypeStructProfileThreshold;
  return m_class >= threshold * m_total;
}

void IsTypeStructProfile::update(const ArrayData* ad) {
  m_total++;
  if (!ad->isStatic() || ad->size() != 2) return;
  auto const kind = get_ts_kind(ad);
  if (kind != TypeStructure::Kind::T_class) return;
  m_class++;
}

void IsTypeStructProfile::reduce(IsTypeStructProfile& l,
                                 const IsTypeStructProfile& r) {
  l.m_total += r.m_total;
  l.m_class += r.m_class;
}

std::string IsTypeStructProfile::toString() const {
  return folly::to<std::string>("class:", m_class, ",total:", m_total);
}

folly::dynamic IsTypeStructProfile::toDynamic() const {
  return folly::dynamic::object("class", m_class)("total", m_total);
}

}
