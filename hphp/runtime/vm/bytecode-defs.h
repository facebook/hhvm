/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VM_BYTECODE_DEFS_H_
#define incl_HPHP_VM_BYTECODE_DEFS_H_

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

namespace HPHP {

bool isReturnHelper(void* address);

inline void ActRec::setReturnVMExit() {
  assert(isReturnHelper(jit::mcg->tx().uniqueStubs.callToExit));
  m_sfp = nullptr;
  m_savedRip =
    reinterpret_cast<uintptr_t>(jit::mcg->tx().uniqueStubs.callToExit);
  m_soff = 0;
}

}

#endif
