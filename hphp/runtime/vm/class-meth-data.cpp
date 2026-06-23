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
#include "hphp/runtime/vm/class-meth-data.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/util/hash-map.h"

namespace HPHP {

ClsMethData::ClsMethData(Class* cls, Func* func)
  : m_cls{cls}
  , m_func{func} {
  assertx(cls);
  assertx(func);
}

ClsMethData::cls_meth_t ClsMethData::make(Class* cls, Func* func) {
  return ClsMethData(cls, func);
}

bool ClsMethData::validate() const {
  getCls()->validate();
  getFunc()->validate();
  return true;
}

OptString ClsMethData::getClsStr() const {
  return getCls()->nameStr();
}

OptString ClsMethData::getFuncStr() const {
  return getFunc()->nameStr();
}

bool ClsMethData::isPersistent() const {
  return getCls()->isPersistent() && getFunc()->isPersistent();
}

} // namespace HPHP
