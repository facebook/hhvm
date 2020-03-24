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

#include "hphp/util/compilation-flags.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

inline const Func* ActRec::func() const {
  return m_func;
}

inline const Unit* ActRec::unit() const {
  func()->validate();
  return func()->unit();
}

inline ActRec* ActRec::sfp() const {
  if (UNLIKELY(((uintptr_t)m_sfp - s_stackLimit) < s_stackSize)) {
    return nullptr;
  }
  return m_sfp;
}

///////////////////////////////////////////////////////////////////////////////

inline bool ActRec::localsDecRefd() const {
  return m_callOffAndFlags & (1 << LocalsDecRefd);
}

inline bool ActRec::isAsyncEagerReturn() const {
  return m_callOffAndFlags & (1 << AsyncEagerRet);
}

inline Offset ActRec::callOffset() const {
  return m_callOffAndFlags >> CallOffsetStart;
}

inline void ActRec::initCallOffset(Offset offset) {
  m_callOffAndFlags = offset << CallOffsetStart;
}

inline uint32_t ActRec::encodeCallOffsetAndFlags(Offset offset,
                                                 uint32_t flags) {
  assertx(!(flags & ~((1 << AsyncEagerRet)|(1 << LocalsDecRefd))));
  return (offset << CallOffsetStart) | flags;
}

inline void ActRec::setLocalsDecRefd() {
  m_callOffAndFlags |= 1 << LocalsDecRefd;
}

inline int32_t ActRec::numArgs() const {
  return m_numArgs;
}

inline void ActRec::setNumArgs(uint32_t numArgs) {
  m_numArgs = numArgs;
}

///////////////////////////////////////////////////////////////////////////////

inline void ActRec::setThisOrClass(void* objOrCls) {
  assertx(m_func->implCls());
  setThisOrClassAllowNull(objOrCls);
}

inline void ActRec::setThisOrClassAllowNull(void* objOrCls) {
  m_thisUnsafe = (ObjectData*)objOrCls;
}

inline bool ActRec::hasThis() const {
  assertx(m_func->implCls());
  assertx(reinterpret_cast<uintptr_t>(m_thisUnsafe) != kTrashedThisSlot);
  return use_lowptr ? !is_low_mem(m_thisUnsafe) : !m_func->isStatic();
}

inline bool ActRec::hasClass() const {
  assertx(m_func->implCls());
  assertx(reinterpret_cast<uintptr_t>(m_thisUnsafe) != kTrashedThisSlot);
  return use_lowptr ? is_low_mem(m_thisUnsafe) : m_func->isStatic();
}

inline bool ActRec::hasThisInPrologue() const {
  assertx(m_func->implCls());
  return use_lowptr
    ? !is_low_mem(m_thisUnsafe)
    : !m_func->isStaticInPrologue();
}

inline bool ActRec::hasClassInPrologue() const {
  assertx(m_func->implCls());
  return use_lowptr
    ? is_low_mem(m_thisUnsafe)
    : m_func->isStaticInPrologue();
}

inline void* ActRec::getThisOrClass() const {
  assertx(m_func->implCls());
  return m_thisUnsafe;
}

inline ObjectData* ActRec::getThisUnsafe() const {
  return m_thisUnsafe;
}

inline ObjectData* ActRec::getThis() const {
  assertx(hasThis());
  return m_thisUnsafe;
}

inline Class* ActRec::getClass() const {
  assertx(hasClass());
  return m_clsUnsafe;
}

inline ObjectData* ActRec::getThisInPrologue() const {
  assertx(hasThisInPrologue());
  return m_thisUnsafe;
}

inline Class* ActRec::getClassInPrologue() const {
  assertx(hasClassInPrologue());
  return m_clsUnsafe;
}

inline void ActRec::setThis(ObjectData* val) {
  assertx(m_func->implCls() && !m_func->isStaticInPrologue());
  m_thisUnsafe = val;
}

inline void ActRec::setClass(Class* val) {
  assertx(val && m_func->implCls() && (m_func->isStaticInPrologue()));
  m_clsUnsafe = val;
}

inline void ActRec::trashThis() {
  if (debug) m_thisUnsafe = reinterpret_cast<ObjectData*>(kTrashedThisSlot);
}

/////////////////////////////////////////////////////////////////////////////

inline void ActRec::trashVarEnv() {
  if (debug) setVarEnv(reinterpret_cast<VarEnv*>(kTrashedVarEnvSlot));
}

inline bool ActRec::checkVarEnv() const {
  assertx(m_varEnv != reinterpret_cast<VarEnv*>(kTrashedVarEnvSlot));
  assertx((uintptr_t(m_varEnv) >> 48) == 0);
  return true;
}

inline bool ActRec::hasVarEnv() const {
  assertx(checkVarEnv());
  return m_varEnv;
}

inline VarEnv* ActRec::getVarEnv() const {
  assertx(hasVarEnv());
  return m_varEnv;
}

inline void ActRec::setVarEnv(VarEnv* val) {
  m_varEnv = val;
}

///////////////////////////////////////////////////////////////////////////////

inline RxLevel ActRec::rxMinLevel() const {
  if (func()->isRxConditional()) return RxLevel::None;
  return func()->rxLevel();
}

///////////////////////////////////////////////////////////////////////////////

}
