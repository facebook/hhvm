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

#include "hphp/runtime/vm/coeffects.h"

#include "hphp/runtime/base/coeffects-config.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(coeffects);

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const std::string RuntimeCoeffects::toString() const {
  // Pretend to be StaticCoeffects, this is safe since RuntimeCoeffects is a
  // subset of StaticCoeffects
  auto const data = StaticCoeffects::fromValue(m_data);
  auto const list = CoeffectsConfig::toStringList(data);
  if (list.empty()) return "defaults";
  if (list.size() == 1 && list[0] == "pure") return "";
  return folly::join(", ", list);
}

bool RuntimeCoeffects::canCallWithWarning(const RuntimeCoeffects o) const {
  auto const promoted =
    RuntimeCoeffects::fromValue(o.m_data | CoeffectsConfig::warningMask());
  return canCall(promoted);
}

const folly::Optional<std::string> StaticCoeffects::toString() const {
  auto const list = CoeffectsConfig::toStringList(*this);
  if (list.empty()) return folly::none;
  return folly::join(" ", list);
}

RuntimeCoeffects StaticCoeffects::toAmbient() const {
  auto const locals =
    (((~m_data) >> 1) & m_data) & CoeffectsConfig::escapeMask();
  auto const val = m_data - locals;
  FTRACE(5, "Converting {:016b} to ambient {:016b}\n", m_data, val);
  return RuntimeCoeffects::fromValue(val);
}

RuntimeCoeffects StaticCoeffects::toRequired() const {
  auto const locals =
    (((~m_data) >> 1) & m_data) & CoeffectsConfig::escapeMask();
  // This converts the 01 (local) pattern to 10 (shallow) pattern
  // (m_data | (locals << 1)) & (~locals)
  // => m_data - locals + 2 * locals
  // => m_data + locals
  auto const val = m_data + locals;
  FTRACE(5, "Converting {:016b} to required {:016b}\n", m_data, val);
  return RuntimeCoeffects::fromValue(val);
}

StaticCoeffects& StaticCoeffects::operator|=(const StaticCoeffects o) {
  return (*this = CoeffectsConfig::combine(*this, o));
}

///////////////////////////////////////////////////////////////////////////////
}
