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
#ifndef incl_HPHP_TRANSLATOR_INSTRS_H_
#define incl_HPHP_TRANSLATOR_INSTRS_H_

/*
 * Macros used for dispatch in the translator.
 */

#define INSTRS \
  CASE(BreakTraceHint) \
  CASE(PopA) \
  CASE(PopC) \
  CASE(PopV) \
  CASE(PopR) \
  CASE(UnboxR) \
  CASE(BoxR) \
  CASE(UnboxRNop) \
  CASE(BoxRNop) \
  CASE(Null) \
  CASE(NullUninit) \
  CASE(True) \
  CASE(False) \
  CASE(Int) \
  CASE(Double) \
  CASE(String) \
  CASE(Array) \
  CASE(NewArray) \
  CASE(NewPackedArray) \
  CASE(NewStructArray) \
  CASE(NewCol) \
  CASE(Clone) \
  CASE(Nop) \
  CASE(AddElemC) \
  CASE(AddNewElemC) \
  CASE(ColAddElemC) \
  CASE(ColAddNewElemC) \
  CASE(Cns) \
  CASE(CnsE) \
  CASE(CnsU) \
  CASE(DefCns) \
  CASE(ClsCnsD) \
  CASE(Concat) \
  CASE(Abs) \
  CASE(Add) \
  CASE(AddO) \
  CASE(Xor) \
  CASE(Not) \
  CASE(Mod) \
  CASE(Sqrt) \
  CASE(BitNot) \
  CASE(CastInt) \
  CASE(CastString) \
  CASE(CastDouble) \
  CASE(CastArray) \
  CASE(CastObject) \
  CASE(Print) \
  CASE(Jmp) \
  CASE(JmpNS) \
  CASE(Switch) \
  CASE(SSwitch) \
  CASE(RetC) \
  CASE(RetV) \
  CASE(NativeImpl) \
  CASE(AGetC) \
  CASE(AGetL) \
  CASE(CGetL) \
  CASE(PushL) \
  CASE(CGetL2) \
  CASE(CGetS) \
  CASE(CGetM) \
  CASE(CGetG) \
  CASE(VGetL) \
  CASE(VGetG) \
  CASE(VGetM) \
  CASE(IssetM) \
  CASE(EmptyM) \
  CASE(AKExists) \
  CASE(SetS) \
  CASE(SetG) \
  CASE(SetM) \
  CASE(SetWithRefLM) \
  CASE(SetWithRefRM) \
  CASE(SetOpL) \
  CASE(SetOpM) \
  CASE(IncDecL) \
  CASE(IncDecM) \
  CASE(UnsetL) \
  CASE(UnsetM) \
  CASE(BindM) \
  CASE(FPushFuncD) \
  CASE(FPushFuncU) \
  CASE(FPushFunc) \
  CASE(FPushClsMethod) \
  CASE(FPushClsMethodD) \
  CASE(FPushClsMethodF) \
  CASE(FPushObjMethodD) \
  CASE(FPushCtor) \
  CASE(FPushCtorD) \
  CASE(FPassR) \
  CASE(FPassL) \
  CASE(FPassM) \
  CASE(FPassS) \
  CASE(FPassG) \
  CASE(FPassV) \
  CASE(FPassVNop) \
  CASE(This) \
  CASE(BareThis) \
  CASE(CheckThis) \
  CASE(InitThisLoc) \
  CASE(FCall) \
  CASE(FCallD) \
  CASE(FCallArray) \
  CASE(FCallBuiltin) \
  CASE(VerifyParamType) \
  CASE(VerifyRetTypeC) \
  CASE(VerifyRetTypeV) \
  CASE(InstanceOfD) \
  CASE(InstanceOf) \
  CASE(StaticLocInit) \
  CASE(StaticLoc) \
  CASE(IterInit) \
  CASE(IterInitK) \
  CASE(IterNext) \
  CASE(IterNextK) \
  CASE(WIterInit) \
  CASE(WIterInitK) \
  CASE(WIterNext) \
  CASE(WIterNextK) \
  CASE(MIterInit) \
  CASE(MIterInitK) \
  CASE(MIterNext) \
  CASE(MIterNextK) \
  CASE(DefCls) \
  CASE(NopDefCls) \
  CASE(DefFunc) \
  CASE(Self) \
  CASE(Parent) \
  CASE(ClassExists) \
  CASE(InterfaceExists) \
  CASE(TraitExists) \
  CASE(Dup) \
  CASE(CreateCl) \
  CASE(CreateCont) \
  CASE(ContEnter) \
  CASE(ContRaise) \
  CASE(ContSuspend) \
  CASE(ContSuspendK) \
  CASE(ContRetC) \
  CASE(ContCheck) \
  CASE(ContValid) \
  CASE(ContKey) \
  CASE(ContCurrent) \
  CASE(ContStopped) \
  CASE(AsyncAwait) \
  CASE(AsyncESuspend) \
  CASE(AsyncResume) \
  CASE(AsyncWrapResult) \
  CASE(Strlen) \
  CASE(IncStat) \
  CASE(Idx) \
  CASE(ArrayIdx) \
  CASE(FPushCufIter) \
  CASE(CIterFree) \
  CASE(LateBoundCls) \
  CASE(IssetS) \
  CASE(IssetG) \
  CASE(IssetL) \
  CASE(EmptyS) \
  CASE(EmptyG) \
  CASE(VGetS) \
  CASE(BindS) \
  CASE(BindG) \
  CASE(IterFree) \
  CASE(MIterFree) \
  CASE(IterBreak) \
  CASE(DecodeCufIter) \
  CASE(Shl) \
  CASE(Shr) \
  CASE(Div) \
  CASE(Floor) \
  CASE(Ceil) \
  CASE(CheckProp) \
  CASE(InitProp) \
  CASE(AssertTL) \
  CASE(AssertTStk) \
  CASE(AssertObjL) \
  CASE(AssertObjStk) \
  CASE(PredictTL) \
  CASE(PredictTStk) \
  /* */

  // These are instruction-like functions which cover more than one
  // opcode.
#define PSEUDOINSTRS \
  CASE(BinaryArithOp) \
  CASE(SameOp) \
  CASE(EqOp) \
  CASE(LtGtOp) \
  CASE(UnaryBooleanOp) \
  CASE(BranchOp) \
  CASE(AssignToLocalOp) \
  CASE(FPushCufOp) \
  CASE(FPassCOp) \
  CASE(CheckTypeLOp) \
  CASE(CheckTypeCOp)

// PSEUDOINSTR_DISPATCH is a switch() fragment that routes opcodes to their
// shared handlers, as per the PSEUDOINSTRS macro.
#define PSEUDOINSTR_DISPATCH(func)                \
  case Op::BitAnd:                                \
  case Op::BitOr:                                 \
  case Op::BitXor:                                \
  case Op::Sub:                                   \
  case Op::Mul:                                   \
  case Op::SubO:                                  \
  case Op::MulO:                                  \
    func(BinaryArithOp, i)                        \
  case Op::Same:                                  \
  case Op::NSame:                                 \
    func(SameOp, i)                               \
  case Op::Eq:                                    \
  case Op::Neq:                                   \
    func(EqOp, i)                                 \
  case Op::Lt:                                    \
  case Op::Lte:                                   \
  case Op::Gt:                                    \
  case Op::Gte:                                   \
    func(LtGtOp, i)                               \
  case Op::EmptyL:                                \
  case Op::CastBool:                              \
    func(UnaryBooleanOp, i)                       \
  case Op::JmpZ:                                  \
  case Op::JmpNZ:                                 \
    func(BranchOp, i)                             \
  case Op::SetL:                                  \
  case Op::BindL:                                 \
    func(AssignToLocalOp, i)                      \
  case Op::FPassC:                                \
  case Op::FPassCW:                               \
  case Op::FPassCE:                               \
    func(FPassCOp, i)                             \
  case Op::FPushCuf:                              \
  case Op::FPushCufF:                             \
  case Op::FPushCufSafe:                          \
    func(FPushCufOp, i)                           \
  case Op::IsTypeL:                               \
    func(CheckTypeLOp, i)                         \
  case Op::IsTypeC:                               \
    func(CheckTypeCOp, i)

#endif
