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

#include "hphp/runtime/base/lazy-string-data.h"

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-emitter.h"

namespace HPHP {

const StringData* LazyStringData::get(const Unit* unit) const {
  if (m_id != kInvalidId) {
    return unit->lookupLitstrId(m_id);
  }
  return nullptr;
}

const StringData* LazyStringData::get(const UnitEmitter& ue) const {
  if (m_id != kInvalidId) {
    return ue.lookupLitstrId(m_id);
  }
  return nullptr;
}

}
