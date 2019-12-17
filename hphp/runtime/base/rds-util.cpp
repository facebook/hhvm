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
#include "hphp/runtime/base/rds-util.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/named-entity.h"

namespace HPHP { namespace rds {

//////////////////////////////////////////////////////////////////////

Link<TypedValue, Mode::Normal>
bindClassConstant(const StringData* clsName, const StringData* cnsName) {
  auto ret = bind<TypedValue,Mode::Normal,kTVSimdAlign>(
    ClsConstant { clsName, cnsName }
  );
  return ret;
}

Link<TypedValue, rds::Mode::Normal>
bindStaticMemoValue(const Func* func) {
  return bind<TypedValue,Mode::Normal>(
    StaticMemoValue { func->getFuncId() }
  );
}

Link<TypedValue, rds::Mode::Normal>
attachStaticMemoValue(const Func* func) {
  return attach<TypedValue,Mode::Normal>(
    StaticMemoValue { func->getFuncId() }
  );
}

Link<MemoCacheBase*, rds::Mode::Normal>
bindStaticMemoCache(const Func* func) {
  return bind<MemoCacheBase*,Mode::Normal>(
    StaticMemoCache { func->getFuncId() }
  );
}

Link<MemoCacheBase*, rds::Mode::Normal>
attachStaticMemoCache(const Func* func) {
  return attach<MemoCacheBase*,Mode::Normal>(
    StaticMemoCache { func->getFuncId() }
  );
}

Link<TypedValue, rds::Mode::Normal>
bindLSBMemoValue(const Class* cls, const Func* func) {
  return bind<TypedValue,Mode::Normal>(
    LSBMemoValue { cls, func->getFuncId() }
  );
}

Link<TypedValue, rds::Mode::Normal>
attachLSBMemoValue(const Class* cls, const Func* func) {
  return attach<TypedValue,Mode::Normal>(
    LSBMemoValue { cls, func->getFuncId() }
  );
}

Link<MemoCacheBase*, rds::Mode::Normal>
bindLSBMemoCache(const Class* cls, const Func* func) {
  return bind<MemoCacheBase*,Mode::Normal>(
    LSBMemoCache { cls, func->getFuncId() }
  );
}

Link<MemoCacheBase*, rds::Mode::Normal>
attachLSBMemoCache(const Class* cls, const Func* func) {
  return attach<MemoCacheBase*,Mode::Normal>(
    LSBMemoCache { cls, func->getFuncId() }
  );
}

//////////////////////////////////////////////////////////////////////

}}
