/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef incl_VM_FUNC_INLINE_H_
#define incl_VM_FUNC_INLINE_H_

namespace HPHP {
namespace VM {

inline ALWAYS_INLINE Func** getCachedFuncAddr(unsigned offset) {
  ASSERT(offset != 0u);
  return (Func**)Transl::TargetCache::handleToPtr(offset);
}

inline ALWAYS_INLINE void setCachedFunc(Func* func, bool debugger) {
  ASSERT(!func->isMethod());
  Func** funcAddr = getCachedFuncAddr(func->getCachedOffset());
  if (UNLIKELY(*funcAddr != NULL)) {
    if (*funcAddr == func) return;
    if (!(*funcAddr)->isIgnoreRedefinition()) {
      raise_error(Strings::FUNCTION_ALREADY_DEFINED, func->name()->data());
    }
  }
  *funcAddr = func;
  if (UNLIKELY(debugger)) phpDefFuncHook(func);
}

} } // HPHP::VM

#endif
