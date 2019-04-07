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
  SrcKey sk, FPInvOffset spOff, int optIndex, Op fpushOff)
  : transID(id)
  , optIndex(optIndex)
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

inline ControlFlowInfo opcodeControlFlowInfo(const Op op, bool inlining) {
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
    case Op::RetC:
    case Op::RetM:
    case Op::RetCSuspended:
    case Op::Exit:
    case Op::Fatal:
    case Op::IterNext:
    case Op::IterNextK:
    case Op::LIterNext:
    case Op::LIterNextK:
    case Op::IterInit: // May branch to fail case.
    case Op::IterInitK: // Ditto
    case Op::LIterInit: // Ditto
    case Op::LIterInitK: // Ditto
    case Op::IterBreak:
    case Op::Throw:
    case Op::Eval:
    case Op::NativeImpl:
    case Op::BreakTraceHint:
    case Op::MemoGet:
    case Op::MemoGetEager:
      return ControlFlowInfo::BreaksBB;
    case Op::Await:
    case Op::AwaitAll:
      return inlining ? ControlFlowInfo::ChangesPC : ControlFlowInfo::BreaksBB;
    case Op::FCall:
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
  return opcodeControlFlowInfo(op, false) >= ControlFlowInfo::ChangesPC;
}

inline bool opcodeBreaksBB(const Op op, bool inlining) {
  if (op == Op::ClsCns) {
    // side exits if it misses in the RDS, and may produce an overly
    // specific type without guarding if the class comes from an
    // object (during form_region, the class will appear to be a
    // specific type, but during irgen, it will probably be a generic
    // type).

    // We can't mark it BreaksBB because BreaksBB => opcodeChangesPC
    return true;
  }
  return opcodeControlFlowInfo(op, inlining) == ControlFlowInfo::BreaksBB;
}

inline bool opcodeIgnoresInnerType(const Op op) {
  switch (op) {
    case Op::PopV:
    case Op::RetC:
    case Op::RetCSuspended:
    case Op::RetM:
      return true;
    default:
      return false;
  }
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
