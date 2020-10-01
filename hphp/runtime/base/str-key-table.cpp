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

#include "hphp/runtime/base/str-key-table.h"

#include "hphp/runtime/base/string-data.h"

namespace HPHP {

bool StrKeyTable::mayContain(const StringData* sd) const {
  if (!sd->isStatic()) return true;
  return m_table.test(sd->hash() & kStrKeyTableMask);
}

void StrKeyTable::add(const StringData* sd) {
  m_table.set(sd->hash() & kStrKeyTableMask);
}

void StrKeyTable::reset() {
  m_table.reset();
}

} // namespace HPHP
