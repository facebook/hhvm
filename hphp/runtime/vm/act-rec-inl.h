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

#include "hphp/util/compilation-flags.h"

#ifndef NDEBUG
#include "hphp/runtime/vm/func.h"
#endif

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

inline const Func* ActRec::func() const {
  return m_func;
}

///////////////////////////////////////////////////////////////////////////////

inline int32_t ActRec::numArgs() const {
  return m_numArgsAndFlags & kNumArgsMask;
}

inline ActRec::Flags ActRec::flags() const {
  return static_cast<Flags>(m_numArgsAndFlags & kFlagsMask);
}

inline bool ActRec::useWeakTypes() const {
  return flags() & UseWeakTypes;
}

inline bool ActRec::localsDecRefd() const {
  return flags() & LocalsDecRefd;
}

inline bool ActRec::resumed() const {
  return (flags() & kExecutionModeMask) == InResumed;
}

inline bool ActRec::isFCallAwait() const {
  return (flags() & kExecutionModeMask) == IsFCallAwait;
}

inline bool ActRec::mayNeedStaticWaitHandle() const {
  return !(m_numArgsAndFlags & (InResumed|IsFCallAwait));
}

inline bool ActRec::magicDispatch() const {
  return (flags() & kExecutionModeMask) == MagicDispatch;
}

inline uint32_t ActRec::encodeNumArgsAndFlags(uint32_t numArgs, Flags flags) {
  assert((numArgs & kFlagsMask) == 0);
  assert((uint32_t{flags} & kNumArgsMask) == 0);
  return numArgs | flags;
}

inline void ActRec::initNumArgs(uint32_t numArgs) {
  m_numArgsAndFlags = encodeNumArgsAndFlags(numArgs, Flags::None);
}

inline void ActRec::setNumArgs(uint32_t numArgs) {
  m_numArgsAndFlags = encodeNumArgsAndFlags(numArgs, flags());
}

inline void ActRec::setUseWeakTypes() {
  m_numArgsAndFlags |= UseWeakTypes;
}

inline void ActRec::setLocalsDecRefd() {
  m_numArgsAndFlags |= LocalsDecRefd;
}

inline void ActRec::setResumed() {
  assert((flags() & ~IsFCallAwait) == Flags::None);
  m_numArgsAndFlags = encodeNumArgsAndFlags(numArgs(), InResumed);
}

inline void ActRec::setFCallAwait() {
  assert(flags() == Flags::None);
  m_numArgsAndFlags = encodeNumArgsAndFlags(numArgs(), IsFCallAwait);
}

inline void ActRec::setMagicDispatch(StringData* invName) {
  assertx(!resumed());
  m_numArgsAndFlags |= MagicDispatch;
  m_invName = invName;
}

inline StringData* ActRec::clearMagicDispatch() {
  assert(magicDispatch());
  auto const invName = getInvName();
  m_numArgsAndFlags = encodeNumArgsAndFlags(
    numArgs(),
    static_cast<Flags>(flags() & ~MagicDispatch)
  );
  trashVarEnv();
  return invName;
}

///////////////////////////////////////////////////////////////////////////////

inline void* ActRec::encodeThis(ObjectData* obj) {
  return obj;
}

inline void* ActRec::encodeClass(const Class* cls) {
  return cls
    ? (void*)(reinterpret_cast<uintptr_t>(cls) | kHasClassBit)
    : nullptr;
}

inline bool ActRec::checkThis(void* p) {
  assertx(p);
  return !(reinterpret_cast<uintptr_t>(p) & kHasClassBit);
}

inline bool ActRec::checkThisOrNull(void* p) {
  return !(reinterpret_cast<uintptr_t>(p) & kHasClassBit);
}

inline ObjectData* ActRec::decodeThis(void* p) {
  return checkThisOrNull(p) ? (ObjectData*)p : nullptr;
}

inline Class* ActRec::decodeClass(void* p) {
  return checkThisOrNull(p) ? nullptr :
    (Class*)(reinterpret_cast<uintptr_t>(p) - kHasClassBit);
}

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
  return checkThis(m_thisUnsafe);
}

inline bool ActRec::hasClass() const {
  assertx(m_func->implCls());
  return !checkThis(m_thisUnsafe);
}

inline void* ActRec::getThisOrClass() const {
  assertx(m_func->implCls());
  return m_thisUnsafe;
}

inline ObjectData* ActRec::getThisUnsafe() const {
  return m_thisUnsafe;
}

inline ObjectData* ActRec::getThis() const {
  assert(hasThis());
  return m_thisUnsafe;
}

inline Class* ActRec::getClass() const {
  assert(hasClass());
  return reinterpret_cast<Class*>(
    reinterpret_cast<uintptr_t>(m_clsUnsafe) - kHasClassBit);
}

inline void ActRec::setThis(ObjectData* val) {
  assertx(m_func->implCls() && !m_func->isStaticInProlog());
  m_thisUnsafe = val;
}

inline void ActRec::setClass(Class* val) {
  assertx(val && m_func->implCls() && !(m_func->attrs() & AttrRequiresThis));
  m_clsUnsafe = reinterpret_cast<Class*>(
    reinterpret_cast<uintptr_t>(val) | kHasClassBit);
}

inline void ActRec::trashThis() {
  if (debug) m_thisUnsafe = reinterpret_cast<ObjectData*>(kTrashedThisSlot);
}

/////////////////////////////////////////////////////////////////////////////

inline void ActRec::trashVarEnv() {
  if (debug) setVarEnv(reinterpret_cast<VarEnv*>(kTrashedVarEnvSlot));
}

inline bool ActRec::checkVarEnv() const {
  assert(m_varEnv != reinterpret_cast<VarEnv*>(kTrashedVarEnvSlot));
  return true;
}

inline bool ActRec::hasVarEnv() const {
  assert(checkVarEnv());
  assert(!magicDispatch());
  return m_varEnv && !(reinterpret_cast<uintptr_t>(m_varEnv) & kExtraArgsBit);
}

inline bool ActRec::hasExtraArgs() const {
  assert(checkVarEnv());
  return reinterpret_cast<uintptr_t>(m_extraArgs) & kExtraArgsBit;
}

inline VarEnv* ActRec::getVarEnv() const {
  assert(hasVarEnv());
  return m_varEnv;
}

inline ExtraArgs* ActRec::getExtraArgs() const {
  if (!hasExtraArgs()) return nullptr;
  return reinterpret_cast<ExtraArgs*>(
    reinterpret_cast<uintptr_t>(m_extraArgs) - kExtraArgsBit);
}

inline StringData* ActRec::getInvName() const {
  assert(magicDispatch());
  assert(checkVarEnv());
  return m_invName;
}

inline void ActRec::setVarEnv(VarEnv* val) {
  m_varEnv = val;
}

inline void ActRec::setExtraArgs(ExtraArgs* val) {
  m_extraArgs = reinterpret_cast<ExtraArgs*>(
    reinterpret_cast<uintptr_t>(val) | kExtraArgsBit);
}

///////////////////////////////////////////////////////////////////////////////

}
