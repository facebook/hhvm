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

#include "hphp/runtime/vm/jit/cow-profile.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/configs/hhir.h"

#include <algorithm>
#include <cstring>
#include <sstream>

namespace HPHP::jit {

///////////////////////////////////////////////////////////////////////////////

std::string COWProfile::toString() const {
  return folly::sformat(
    "nocow:{}({:.1f}%),total:{}",
    m_nocow,
    m_total > 0 ? (100.0 * (m_nocow / (double)m_total)) : 0.0,
    m_total
  );
}

folly::dynamic COWProfile::toDynamic() const {
  using folly::dynamic;
  return dynamic::object("nocow", m_nocow)
                        ("total", m_total)
                        ("profileType", "COWProfile");
}

COWProfile::Result COWProfile::choose() const {
  if (!m_total) return Result::None;

  auto const cold = m_total * Cfg::HHIR::COWArrayProfileThreshold;
  auto const frozen = m_total * Cfg::HHIR::ExitArrayProfileThreshold;
  if (m_nocow >= frozen) return Result::NeverCOW;
  if (m_nocow >= cold) return Result::RarelyCOW;

  auto const cow = m_total - m_nocow;
  if (cow >= frozen) return Result::AlwaysCOW;
  if (cow >= cold) return Result::UsuallyCOW;
  return Result::None;
}

void COWProfile::update(const ArrayData* ad) {
  ++m_total;
  if (!ad->cowCheck()) ++m_nocow;
}

void COWProfile::reduce(COWProfile& l, const COWProfile& r) {
  l.m_nocow += r.m_nocow;
  l.m_total += r.m_total;
}

///////////////////////////////////////////////////////////////////////////////

}
