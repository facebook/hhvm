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

#ifndef incl_HPHP_TRANSLATOR_INL_H_
#error "translator-inl.h should only be included by translator.h"
#endif

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////
// TransContext.

inline TransContext::TransContext(
  TransID id, TransKind kind, TransFlags flags,
  SrcKey sk, FPInvOffset spOff, Op fpushOff)
  : transID(id)
  , kind(kind)
  , flags(flags)
  , initSpOffset(spOff)
  , callerFPushOp(fpushOff)
  , func(sk.valid() ? sk.func() : nullptr)
  , initBcOffset(sk.offset())
  , hasThis(sk.hasThis())
  , prologue(sk.prologue())
  , resumeMode(sk.resumeMode())
{}

inline SrcKey TransContext::srcKey() const {
  if (prologue) {
    assertx(resumeMode == ResumeMode::None);
    return SrcKey { func, initBcOffset, SrcKey::PrologueTag{} };
  }
  return SrcKey { func, initBcOffset, resumeMode, hasThis };
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
    case Op::YieldFromDelegate:
    case Op::Await:
    case Op::AwaitAll:
    case Op::FCallAwait:
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
    case Op::ContEnterDelegate:
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
  std::string p = show(loc);
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
