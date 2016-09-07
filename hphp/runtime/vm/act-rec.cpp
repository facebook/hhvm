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

#include "hphp/runtime/vm/act-rec.h"

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"

#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"

#include "hphp/util/assertions.h"
#include "hphp/util/trace.h"

namespace HPHP {

TRACE_SET_MOD(bcinterp);

///////////////////////////////////////////////////////////////////////////////

bool isReturnHelper(void* address) {
  auto tca = reinterpret_cast<jit::TCA>(address);
  auto& u = jit::tc::ustubs();
  return tca == u.retHelper ||
         tca == u.genRetHelper ||
         tca == u.asyncGenRetHelper ||
         tca == u.retInlHelper ||
         tca == u.callToExit;
}

bool isDebuggerReturnHelper(void* address) {
  auto tca = reinterpret_cast<jit::TCA>(address);
  auto& u = jit::tc::ustubs();
  return tca == u.debuggerRetHelper ||
         tca == u.debuggerGenRetHelper ||
         tca == u.debuggerAsyncGenRetHelper;
}

///////////////////////////////////////////////////////////////////////////////

ActRec* ActRec::sfp() const {
  if (UNLIKELY(((uintptr_t)m_sfp - s_stackLimit) < s_stackSize)) {
    return nullptr;
  }
  return m_sfp;
}

const Unit* ActRec::unit() const {
  func()->validate();
  return func()->unit();
}

void ActRec::setReturn(ActRec* fp, PC pc, void* retAddr) {
  assert(fp->func()->contains(pc));
  assert(isReturnHelper(retAddr));
  m_sfp = fp;
  m_savedRip = reinterpret_cast<uintptr_t>(retAddr);
  m_soff = Offset(pc - fp->func()->getEntry());
}

void ActRec::setJitReturn(void* retAddr) {
  FTRACE(1, "Replace m_savedRip in fp {}, {:#x} -> {}, func {}\n",
         this, m_savedRip, retAddr, func()->fullName()->data());
  m_savedRip = reinterpret_cast<uintptr_t>(retAddr);
}

bool ActRec::skipFrame() const {
  return func() && func()->isSkipFrame();
}

///////////////////////////////////////////////////////////////////////////////

TypedValue* ActRec::getExtraArg(unsigned ind) const {
  assert(hasExtraArgs() || hasVarEnv());
  return hasExtraArgs() ? getExtraArgs()->getExtraArg(ind) :
         hasVarEnv()    ? getVarEnv()->getExtraArg(ind) :
         static_cast<TypedValue*>(nullptr);
}

///////////////////////////////////////////////////////////////////////////////

}
