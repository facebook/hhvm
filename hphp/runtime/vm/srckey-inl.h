/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
  , m_prologue{false}
  , m_resumed{false}
{}

inline SrcKey::SrcKey(const Func* f, Offset off, bool resumed)
  : m_funcID{f->getFuncId()}
  , m_offset{(uint32_t)off}
  , m_prologue{false}
  , m_resumed{resumed}
{
  assert((uint32_t)off >> 31 == 0);
}

inline SrcKey::SrcKey(const Func* f, PC pc, bool resumed)
  : m_funcID{f->getFuncId()}
  , m_offset{(uint32_t)f->unit()->offsetOf(pc)}
  , m_prologue{false}
  , m_resumed{resumed}
{
  assert((uint32_t)f->unit()->offsetOf(pc) >> 31 == 0);
}

inline SrcKey::SrcKey(FuncId funcId, Offset off, bool resumed)
  : m_funcID{funcId}
  , m_offset{(uint32_t)off}
  , m_prologue{false}
  , m_resumed{resumed}
{
  assert((uint32_t)off >> 31 == 0);
}

inline SrcKey::SrcKey(const Func* f, Offset off, PrologueTag)
  : m_funcID{f->getFuncId()}
  , m_offset{(uint32_t)off}
  , m_prologue{true}
  , m_resumed{false}
{
  assert((uint32_t)off >> 31 == 0);
}

inline SrcKey::SrcKey(const Func* f, PC pc, PrologueTag)
  : m_funcID{f->getFuncId()}
  , m_offset{(uint32_t)f->unit()->offsetOf(pc)}
  , m_prologue{true}
  , m_resumed{false}
{
  assert((uint32_t)f->unit()->offsetOf(pc) >> 31 == 0);
}

inline SrcKey::SrcKey(SrcKey other, Offset off)
  : m_funcID{other.funcID()}
  , m_offset{(uint32_t)off}
  , m_prologue{other.prologue()}
  , m_resumed{other.resumed()}
{
  assert((uint32_t)off >> 31 == 0);
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
  assert(m_funcID != InvalidFuncId);
  return m_funcID;
}

inline int SrcKey::offset() const {
  return m_offset;
}

inline bool SrcKey::prologue() const {
  return m_prologue;
}

inline bool SrcKey::resumed() const {
  return m_resumed;
}

inline const Func* SrcKey::func() const {
  return Func::fromFuncId(m_funcID);
}

inline const Unit* SrcKey::unit() const {
  return func()->unit();
}

inline Op SrcKey::op() const {
  return unit()->getOpcode(offset());
}

inline PC SrcKey::pc() const {
  return unit()->at(offset());
}

///////////////////////////////////////////////////////////////////////////////

inline void SrcKey::setOffset(Offset o) {
  assert((uint32_t)o >> 31 == 0);
  m_offset = (uint32_t)o;
}

inline OffsetSet SrcKey::succOffsets() const {
  return instrSuccOffsets((Op*)pc(), unit());
}

inline void SrcKey::advance(const Unit* u) {
  m_offset += instrLen((Op*)(u ? u : unit())->at(offset()));
}

inline SrcKey SrcKey::advanced(const Unit* u) const {
  auto tmp = *this;
  tmp.advance(u);
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
