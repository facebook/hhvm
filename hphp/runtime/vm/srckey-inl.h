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

#include "hphp/runtime/vm/func.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

inline SrcKey::SrcKey()
  : m_funcID{InvalidFuncId}
  , m_offset{0}
  , m_resumeModeAndPrologue{encodeResumeMode(ResumeMode::None)}
{}

inline SrcKey::SrcKey(const Func* f, Offset off, ResumeMode resumeMode)
  : m_funcID{f->getFuncId()}
  , m_offset{(uint32_t)off}
  , m_resumeModeAndPrologue{encodeResumeMode(resumeMode)}
{
  assertx((uint32_t)off >> 31 == 0);
}

inline SrcKey::SrcKey(const Func* f, PC pc, ResumeMode resumeMode)
  : m_funcID{f->getFuncId()}
  , m_offset{(uint32_t)f->offsetOf(pc)}
  , m_resumeModeAndPrologue{encodeResumeMode(resumeMode)}
{
  assertx((uint32_t)f->offsetOf(pc) >> 31 == 0);
}

inline SrcKey::SrcKey(FuncId funcId, Offset off, ResumeMode resumeMode)
  : m_funcID{funcId}
  , m_offset{(uint32_t)off}
  , m_resumeModeAndPrologue{encodeResumeMode(resumeMode)}
{
  assertx((uint32_t)off >> 31 == 0);
}

inline SrcKey::SrcKey(const Func* f, Offset off, PrologueTag)
  : m_funcID{f->getFuncId()}
  , m_offset{(uint32_t)off}
  , m_resumeModeAndPrologue{encodePrologue()}
{
  assertx((uint32_t)off >> 31 == 0);
}

inline SrcKey::SrcKey(const Func* f, PC pc, PrologueTag)
  : m_funcID{f->getFuncId()}
  , m_offset{(uint32_t)f->offsetOf(pc)}
  , m_resumeModeAndPrologue{encodePrologue()}
{
  assertx((uint32_t)f->offsetOf(pc) >> 31 == 0);
}

inline SrcKey::SrcKey(FuncId funcId, Offset off, PrologueTag)
  : m_funcID{funcId}
  , m_offset{(uint32_t)off}
  , m_resumeModeAndPrologue{encodePrologue()}
{
  assertx((uint32_t)off >> 31 == 0);
}

inline SrcKey::SrcKey(SrcKey other, Offset off)
  : m_funcID{other.funcID()}
  , m_offset{(uint32_t)off}
  , m_resumeModeAndPrologue{other.m_resumeModeAndPrologue}
{
  assertx((uint32_t)off >> 31 == 0);
}

inline SrcKey::SrcKey(AtomicInt in)
  : m_atomicInt{in}
{}

inline SrcKey SrcKey::fromAtomicInt(AtomicInt in) {
  return SrcKey { in };
}

///////////////////////////////////////////////////////////////////////////////

inline bool SrcKey::valid() const {
  return m_funcID != InvalidFuncId;
}

inline SrcKey::AtomicInt SrcKey::toAtomicInt() const {
  return m_atomicInt;
}

inline FuncId SrcKey::funcID() const {
  assertx(m_funcID != InvalidFuncId);
  return m_funcID;
}

inline int SrcKey::offset() const {
  return m_offset;
}

inline bool SrcKey::hasThis() const {
  if (!func()->cls()) return false;
  return prologue() ? !func()->isStaticInPrologue() : !func()->isStatic();
}

inline bool SrcKey::prologue() const {
  return m_resumeModeAndPrologue == 3;
}

inline ResumeMode SrcKey::resumeMode() const {
  return prologue() ? ResumeMode::None : (ResumeMode)m_resumeModeAndPrologue;
}

inline uint32_t SrcKey::encodePrologue() {
  return 3;
}

inline uint32_t SrcKey::encodeResumeMode(ResumeMode resumeMode) {
  assertx((uint8_t)resumeMode < 3);
  return (uint8_t)resumeMode;
}

inline const Func* SrcKey::func() const {
  return Func::fromFuncId(m_funcID);
}

inline const Unit* SrcKey::unit() const {
  return func()->unit();
}

inline Op SrcKey::op() const {
  return func()->getOp(offset());
}

inline PC SrcKey::pc() const {
  return func()->at(offset());
}

///////////////////////////////////////////////////////////////////////////////

inline void SrcKey::setOffset(Offset o) {
  assertx((uint32_t)o >> 31 == 0);
  m_offset = (uint32_t)o;
}

inline OffsetSet SrcKey::succOffsets() const {
  return instrSuccOffsets(pc(), func());
}

inline void SrcKey::advance(const Func* f) {
  m_offset += instrLen((f ? f : func())->at(offset()));
}

inline SrcKey SrcKey::advanced(const Func* f) const {
  auto tmp = *this;
  tmp.advance(f);
  return tmp;
}

inline bool SrcKey::operator==(const SrcKey& r) const {
  return m_atomicInt == r.m_atomicInt;
}

inline bool SrcKey::operator<(const SrcKey& r) const {
  return m_atomicInt < r.m_atomicInt;
}

///////////////////////////////////////////////////////////////////////////////

}
