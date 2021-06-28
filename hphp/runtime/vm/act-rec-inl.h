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
  return Func::fromFuncId(m_funcId);
}

inline void ActRec::setFunc(const Func* f) {
  m_funcId = f->getFuncId();
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

inline bool ActRec::isInlined() const {
  return m_callOffAndFlags & (1 << IsInlined);
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
  assertx(!(flags & ~((1 << CallOffsetStart) - 1)));
  return (offset << CallOffsetStart) | flags;
}

inline void ActRec::setLocalsDecRefd() {
  m_callOffAndFlags |= 1 << LocalsDecRefd;
}

///////////////////////////////////////////////////////////////////////////////

inline void ActRec::setThisOrClass(void* objOrCls) {
  assertx(func()->implCls());
  setThisOrClassAllowNull(objOrCls);
}

inline void ActRec::setThisOrClassAllowNull(void* objOrCls) {
  m_thisUnsafe = (ObjectData*)objOrCls;
}

inline bool ActRec::hasThis() const {
  assertx(func()->implCls());
  assertx(reinterpret_cast<uintptr_t>(m_thisUnsafe) != kTrashedThisSlot);
  return use_lowptr ? !is_low_mem(m_thisUnsafe) : !func()->isStatic();
}

inline bool ActRec::hasClass() const {
  assertx(func()->implCls());
  assertx(reinterpret_cast<uintptr_t>(m_thisUnsafe) != kTrashedThisSlot);
  return use_lowptr ? is_low_mem(m_thisUnsafe) : func()->isStatic();
}

inline bool ActRec::hasThisInPrologue() const {
  assertx(func()->implCls());
  return use_lowptr
    ? !is_low_mem(m_thisUnsafe)
    : !func()->isStaticInPrologue();
}

inline bool ActRec::hasClassInPrologue() const {
  assertx(func()->implCls());
  return use_lowptr
    ? is_low_mem(m_thisUnsafe)
    : func()->isStaticInPrologue();
}

inline void* ActRec::getThisOrClass() const {
  assertx(func()->implCls());
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
  assertx(func()->implCls() && !func()->isStaticInPrologue());
  m_thisUnsafe = val;
}

inline void ActRec::setClass(Class* val) {
  assertx(val && func()->implCls() && (func()->isStaticInPrologue()));
  m_clsUnsafe = val;
}

inline void ActRec::trashThis() {
  if (debug) m_thisUnsafe = reinterpret_cast<ObjectData*>(kTrashedThisSlot);
}

/////////////////////////////////////////////////////////////////////////////

inline RuntimeCoeffects ActRec::requiredCoeffects() const {
  if (!func()->hasCoeffectsLocal()) {
    assertx(!func()->hasCoeffectRules());
    return func()->requiredCoeffects();
  }
  // Access 0Coeffects variable
  assertx(!localsDecRefd());
  auto const id = func()->coeffectsLocalId();
  auto const tv = reinterpret_cast<const TypedValue*>(this) - (id + 1);
  assertx(tvIsInt(tv));
  return RuntimeCoeffects::fromValue(tv->m_data.num);
}

inline RuntimeCoeffects ActRec::coeffects() const {
  auto const shallows = func()->shallowCoeffectsWithLocals();
  auto const mask = ~(shallows.value());
  return RuntimeCoeffects::fromValue(requiredCoeffects().value() & mask);
}

inline RuntimeCoeffects ActRec::providedCoeffectsForCall(bool isCtor) const {
  if (!isCtor) return coeffects();
  auto const mask = RuntimeCoeffects::write_this_props().value();
  return RuntimeCoeffects::fromValue(coeffects().value() & mask);
}

///////////////////////////////////////////////////////////////////////////////

}
