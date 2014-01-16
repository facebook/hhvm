/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP { namespace RDS {

//////////////////////////////////////////////////////////////////////

Link<RefData> bindStaticLocal(const Func* func, const StringData* name) {
  return bind<RefData>(
    StaticLocal { func->getFuncId(), name },
    Mode::Normal
  );
}

Link<TypedValue> bindClassConstant(const StringData* clsName,
                                   const StringData* cnsName) {
  return bind<TypedValue,kTVSimdAlign>(
    ClsConstant { clsName, cnsName },
    Mode::Normal
  );
}

//////////////////////////////////////////////////////////////////////

}}

