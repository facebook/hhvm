/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VM_FUNC_INLINE_H_
#define incl_HPHP_VM_FUNC_INLINE_H_

#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/vm/debugger-hook.h"

namespace HPHP {

ALWAYS_INLINE void setCachedFunc(Func* func, bool debugger) {
  assert(!func->isMethod());
  auto const handle = func->funcHandle();
  auto& funcAddr = rds::handleToRef<LowPtr<Func>>(handle);

  if (rds::isPersistentHandle(handle)) {
    assertx(funcAddr.get() == nullptr || funcAddr.get() == func);
  } else {
    assertx(rds::isNormalHandle(handle));
    if (!rds::isHandleInit(handle, rds::NormalTag{})) {
      rds::initHandle(handle);
    } else {
      if (funcAddr.get() == func) return;
      raise_error(Strings::FUNCTION_ALREADY_DEFINED, func->name()->data());
    }
  }
  funcAddr = func;

  if (UNLIKELY(debugger)) phpDebuggerDefFuncHook(func);
}

}

#endif
