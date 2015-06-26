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

#ifndef incl_HPHP_TRANSLATOR_INL_H_
#error "translator-inl.h should only be included by translator.h"
#endif

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////
// Translator accessors.

inline ProfData* Translator::profData() const {
  return m_profData.get();
}

inline const SrcDB& Translator::getSrcDB() const {
  return m_srcDB;
}

inline SrcRec* Translator::getSrcRec(SrcKey sk) {
  // XXX: Add a insert-or-find primitive to THM.
  if (SrcRec* r = m_srcDB.find(sk)) return r;
  assertx(s_writeLease.amOwner());

  auto rec = m_srcDB.insert(sk);
  if (RuntimeOption::EvalEnableReusableTC) {
    recordFuncSrcRec(sk.func(), rec);
  }

  return rec;
}

///////////////////////////////////////////////////////////////////////////////
// Translator configuration.

inline TransKind Translator::mode() const {
  return m_mode;
}

inline void Translator::setMode(TransKind mode) {
  m_mode = mode;
}

inline bool Translator::useAHot() const {
  return m_useAHot;
}

inline void Translator::setUseAHot(bool val) {
  m_useAHot = val;
}

///////////////////////////////////////////////////////////////////////////////
// TransDB.

inline bool Translator::isTransDBEnabled() {
  return debug ||
         RuntimeOption::EvalDumpTC ||
         RuntimeOption::EvalDumpIR;
}

inline const TransRec* Translator::getTransRec(TCA tca) const {
  if (!isTransDBEnabled()) return nullptr;

  TransDB::const_iterator it = m_transDB.find(tca);
  if (it == m_transDB.end()) {
    return nullptr;
  }
  if (it->second >= m_translations.size()) {
    return nullptr;
  }
  return &m_translations[it->second];
}

inline const TransRec* Translator::getTransRec(TransID transId) const {
  if (!isTransDBEnabled()) return nullptr;

  always_assert(transId < m_translations.size());
  return &m_translations[transId];
}

inline TransID Translator::getCurrentTransID() const {
  return m_translations.size();
}

///////////////////////////////////////////////////////////////////////////////

inline Lease& Translator::WriteLease() {
  return s_writeLease;
}

///////////////////////////////////////////////////////////////////////////////
// TransContext.

inline TransContext::TransContext(TransID id, SrcKey sk, FPInvOffset spOff)
  : transID(id)
  , initSpOffset(spOff)
  , func(sk.valid() ? sk.func() : nullptr)
  , initBcOffset(sk.offset())
  , prologue(sk.prologue())
  , resumed(sk.resumed())
{}

inline SrcKey TransContext::srcKey() const {
  if (prologue) {
    assertx(!resumed);
    return SrcKey { func, initBcOffset, SrcKey::PrologueTag{} };
  }
  return SrcKey { func, initBcOffset, resumed };
}

///////////////////////////////////////////////////////////////////////////////
// Control flow information.

inline ControlFlowInfo opcodeControlFlowInfo(const Op op) {
  switch (op) {
    case Op::Jmp:
    case Op::JmpNS:
    case Op::JmpZ:
    case Op::JmpNZ:
    case Op::Switch:
    case Op::SSwitch:
    case Op::CreateCont:
    case Op::Yield:
    case Op::YieldK:
    case Op::Await:
    case Op::RetC:
    case Op::RetV:
    case Op::Exit:
    case Op::Fatal:
    case Op::IterNext:
    case Op::IterNextK:
    case Op::MIterNext:
    case Op::MIterNextK:
    case Op::WIterNext:
    case Op::WIterNextK:
    case Op::IterInit: // May branch to fail case.
    case Op::IterInitK: // Ditto
    case Op::MIterInit: // Ditto
    case Op::MIterInitK: // Ditto
    case Op::WIterInit: // Ditto
    case Op::WIterInitK: // Ditto
    case Op::DecodeCufIter: // Ditto
    case Op::IterBreak:
    case Op::Throw:
    case Op::Unwind:
    case Op::Eval:
    case Op::NativeImpl:
    case Op::BreakTraceHint:
      return ControlFlowInfo::BreaksBB;
    case Op::FCall:
    case Op::FCallD:
    case Op::FCallArray:
    case Op::FCallUnpack:
    case Op::ContEnter:
    case Op::ContRaise:
    case Op::Incl:
    case Op::InclOnce:
    case Op::Req:
    case Op::ReqOnce:
    case Op::ReqDoc:
      return ControlFlowInfo::ChangesPC;
    default:
      return ControlFlowInfo::None;
  }
}

inline bool opcodeChangesPC(const Op op) {
  return opcodeControlFlowInfo(op) >= ControlFlowInfo::ChangesPC;
}

inline bool opcodeBreaksBB(const Op op) {
  return opcodeControlFlowInfo(op) == ControlFlowInfo::BreaksBB;
}

///////////////////////////////////////////////////////////////////////////////
// Input and output information.

inline std::string InputInfo::pretty() const {
  std::string p = loc.pretty();
  if (dontBreak) p += ":dc";
  if (dontGuard) p += ":dg";
  if (dontGuardInner) p += ":dgi";
  return p;
}

inline std::string InputInfoVec::pretty() const {
  std::string retval;
  for (size_t i = 0; i < size(); i++) {
    retval += (*this)[i].pretty();
    if (i != size() - 1) {
      retval += std::string(" ");
    }
  }
  return retval;
}

///////////////////////////////////////////////////////////////////////////////
// InstrFlags.

namespace InstrFlags {

inline Operands operator|(const Operands& l, const Operands& r) {
  return Operands(int(r) | int(l));
}

}

///////////////////////////////////////////////////////////////////////////////
}}
