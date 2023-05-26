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
  : m_s{FuncId::Invalid, 0, encodeResumeMode(ResumeMode::None)}
{}

inline SrcKey::SrcKey(const Func* f, Offset off, ResumeMode resumeMode)
  : m_s{f->getFuncId(), (uint32_t)off, encodeResumeMode(resumeMode)}
{
  assertx((uint32_t)off >> kNumOffsetBits == 0);
}

inline SrcKey::SrcKey(const Func* f, PC pc, ResumeMode resumeMode)
  : m_s{f->getFuncId(), (uint32_t)f->offsetOf(pc), encodeResumeMode(resumeMode)}
{
  assertx((uint32_t)f->offsetOf(pc) >> kNumOffsetBits == 0);
}

inline SrcKey::SrcKey(FuncId funcId, Offset off, ResumeMode resumeMode)
  : m_s{funcId, (uint32_t)off, encodeResumeMode(resumeMode)}
{
  assertx((uint32_t)off >> kNumOffsetBits == 0);
}

inline SrcKey::SrcKey(const Func* f, uint32_t numArgs, PrologueTag)
  : m_s{f->getFuncId(), numArgs, encodePrologue()}
{
  assertx(numArgs >> kNumOffsetBits == 0);
}

inline SrcKey::SrcKey(const Func* f, uint32_t numArgs, FuncEntryTag)
  : m_s{f->getFuncId(), numArgs, encodeFuncEntry()}
{
  assertx(numArgs >> kNumOffsetBits == 0);
}

inline SrcKey::SrcKey(SrcKey other, Offset off)
  : m_s{other.funcID(), (uint32_t)off, other.m_s.m_resumeModeAndTags}
{
  assertx((uint32_t)off >> kNumOffsetBits == 0);
}

inline SrcKey::SrcKey(AtomicInt in)
  : m_atomicInt{in}
{}

inline SrcKey SrcKey::fromAtomicInt(AtomicInt in) {
  return SrcKey {in};
}

///////////////////////////////////////////////////////////////////////////////

inline bool SrcKey::valid() const {
  auto const funcID = m_s.m_funcID;
  assertx(
    funcID.isInvalid() || funcID.isDummy() ||
    (prologue() && numEntryArgs() <= func()->numNonVariadicParams() + 1) ||
    (funcEntry() && numEntryArgs() <= func()->numNonVariadicParams()) ||
    (!prologue() && !funcEntry() && offset() < func()->bclen())
  );
  return !funcID.isInvalid();
}

inline SrcKey::AtomicInt SrcKey::toAtomicInt() const {
  return m_atomicInt;
}

inline size_t SrcKey::stableHash() const {
  return folly::hash::hash_combine(
    func()->stableHash(),
    m_s.m_offsetOrNumArgs,
    m_s.m_resumeModeAndTags
  );
}

inline FuncId SrcKey::funcID() const {
  assertx(!m_s.m_funcID.isInvalid());
  return m_s.m_funcID;
}

inline Offset SrcKey::offset() const {
  assertx(!prologue() && !funcEntry());
  return m_s.m_offsetOrNumArgs;
}

inline Offset SrcKey::entryOffset() const {
  assertx(prologue() || funcEntry());
  return func()->getEntryForNumArgs(numEntryArgs());
}

inline uint32_t SrcKey::numEntryArgs() const {
  assertx(prologue() || funcEntry());
  return m_s.m_offsetOrNumArgs;
}

inline std::string SrcKey::printableOffset() const {
  auto const off = !prologue() && !funcEntry() ? offset() : entryOffset();
  return std::to_string(off);
}

inline bool SrcKey::hasThis() const {
  if (!func()->cls()) return false;
  return prologue() ? !func()->isStaticInPrologue() : !func()->isStatic();
}

inline ResumeMode SrcKey::resumeMode() const {
  return m_s.m_resumeModeAndTags < 3
    ? (ResumeMode)m_s.m_resumeModeAndTags : ResumeMode::None;
}

inline bool SrcKey::prologue() const {
  return m_s.m_resumeModeAndTags == 3;
}

inline bool SrcKey::funcEntry() const {
  return m_s.m_resumeModeAndTags == 4;
}

inline bool SrcKey::trivialDVFuncEntry() const {
  if (!funcEntry()) return false;
  if (numEntryArgs() == func()->numNonVariadicParams()) return false;
  assertx(numEntryArgs() >= func()->numRequiredParams());
  return func()->params()[numEntryArgs()].hasTrivialDefaultValue();
}

inline bool SrcKey::nonTrivialFuncEntry() const {
  return funcEntry() && !trivialDVFuncEntry();
}

inline uint32_t SrcKey::encodeResumeMode(ResumeMode resumeMode) {
  assertx((uint8_t)resumeMode >> kNumModeBits == 0);
  assertx((uint8_t)resumeMode < 3);
  return (uint8_t)resumeMode;
}

inline uint32_t SrcKey::encodePrologue() {
  assertx(3 >> kNumModeBits == 0);
  return 3;
}

inline uint32_t SrcKey::encodeFuncEntry() {
  assertx(4 >> kNumModeBits == 0);
  return 4;
}

inline const Func* SrcKey::func() const {
  return Func::fromFuncId(m_s.m_funcID);
}

inline const Unit* SrcKey::unit() const {
  return func()->unit();
}

inline Op SrcKey::op() const {
  assertx(!prologue() && !funcEntry());
  return func()->getOp(offset());
}

inline PC SrcKey::pc() const {
  assertx(!prologue() && !funcEntry());
  return func()->at(offset());
}

inline int SrcKey::lineNumber() const {
  if (prologue() || funcEntry()) return func()->line1();
  return func()->getLineNumber(offset());
}

inline const PackageInfo* SrcKey::packageInfo() const {
  if (RO::RepoAuthoritative) return &RepoFile::packageInfo();
  auto const file = unit()->filepath();
  assertx(file);
  return &RepoOptions::forFile(file->data()).packageInfo();
}

///////////////////////////////////////////////////////////////////////////////

inline void SrcKey::setOffset(Offset o) {
  assertx((uint32_t)o >> kNumOffsetBits == 0);
  assertx(!prologue() && !funcEntry());
  m_s.m_offsetOrNumArgs = (uint32_t)o;
}

inline SrcKey::Set SrcKey::succSrcKeys() const {
  assertx(!prologue());
  if (funcEntry()) {
    if (trivialDVFuncEntry()) {
      return {SrcKey{func(), numEntryArgs() + 1, FuncEntryTag {}}};
    }
    return {SrcKey{func(), entryOffset(), ResumeMode::None}};
  }

  SrcKey::Set set;
  for (auto offset : instrSuccOffsets(pc(), func())) {
    set.emplace(SrcKey{*this, offset});
  }
  return set;
}

inline void SrcKey::advance(const Func* f) {
  assertx(!prologue());
  if (funcEntry()) {
    m_s.m_offsetOrNumArgs = entryOffset();
    m_s.m_resumeModeAndTags = encodeResumeMode(ResumeMode::None);
  } else {
    m_s.m_offsetOrNumArgs += instrLen((f ? f : func())->at(offset()));
  }
}

inline SrcKey SrcKey::advanced(const Func* f) const {
  auto tmp = *this;
  tmp.advance(f);
  return tmp;
}

inline SrcKey SrcKey::withFuncID(FuncId funcId) const {
  auto tmp = *this;
  tmp.m_s.m_funcID = funcId;
  return tmp;
}

inline bool SrcKey::operator==(const SrcKey& r) const {
  return m_atomicInt == r.m_atomicInt;
}

inline bool SrcKey::operator!=(const SrcKey& r) const {
  return m_atomicInt != r.m_atomicInt;
}

///////////////////////////////////////////////////////////////////////////////

}
