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

#include <utility>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

inline const Varea& Vtext::area(AreaIndex i) const {
  assertx(static_cast<unsigned>(i) < m_areas.size());
  return m_areas.at(static_cast<unsigned>(i));
}

inline Varea& Vtext::area(AreaIndex i) {
  assertx(static_cast<unsigned>(i) < m_areas.size());
  return m_areas[static_cast<unsigned>(i)];
}

inline CodeAddress Vtext::toDestAddress(CodeAddress a) {
  for (auto const& area : m_areas) {
    if (area.code.contains(a)) return area.code.toDestAddress(a);
  }
  return a;
}

///////////////////////////////////////////////////////////////////////////////

}}
