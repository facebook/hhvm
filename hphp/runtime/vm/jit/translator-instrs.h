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

#define IRREGULAR_INSTRS \
  CASE(AGetL) \
  CASE(Add) \
  CASE(AddO) \
  CASE(Array) \
  CASE(AssertObjL) \
  CASE(AssertObjStk) \
  CASE(AssertTL) \
  CASE(AssertTStk) \
  CASE(Await) \
  CASE(BareThis) \
  CASE(BindM) \
  CASE(BoxR) \
  CASE(BoxRNop) \
  CASE(BreakTraceHint) \
  CASE(CGetL) \
  CASE(CGetL2) \
  CASE(CGetM) \
  CASE(CIterFree) \
  CASE(CheckProp) \
  CASE(ClsCnsD) \
  CASE(Cns) \
  CASE(CnsE) \
  CASE(CnsU) \
  CASE(ContCheck) \
  CASE(ContEnter) \
  CASE(ContRaise) \
  CASE(CreateCl) \
  CASE(CreateCont) \
  CASE(DecodeCufIter) \
  CASE(DefCls) \
  CASE(DefCns) \
  CASE(DefFunc) \
  CASE(Div) \
  CASE(Double) \
  CASE(EmptyM) \
  CASE(FCall) \
  CASE(FCallArray) \
  CASE(FCallBuiltin) \
  CASE(FCallD) \
  CASE(FPassG) \
  CASE(FPassL) \
  CASE(FPassM) \
  CASE(FPassR) \
  CASE(FPassS) \
  CASE(FPassV) \
  CASE(FPassVNop) \
  CASE(FPushClsMethod) \
  CASE(FPushClsMethodD) \
  CASE(FPushClsMethodF) \
  CASE(FPushCtor) \
  CASE(FPushCtorD) \
  CASE(FPushCufIter) \
  CASE(FPushFunc) \
  CASE(FPushFuncD) \
  CASE(FPushFuncU) \
  CASE(FPushObjMethodD) \
  CASE(IncDecL) \
  CASE(IncDecM) \
  CASE(IncStat) \
  CASE(InitProp) \
  CASE(InitThisLoc) \
  CASE(InstanceOfD) \
  CASE(Int) \
  CASE(IssetL) \
  CASE(IssetM) \
  CASE(IterBreak) \
  CASE(IterFree) \
  CASE(IterInit) \
  CASE(IterInitK) \
  CASE(IterNext) \
  CASE(IterNextK) \
  CASE(Jmp) \
  CASE(JmpNS) \
  CASE(MIterFree) \
  CASE(MIterInit) \
  CASE(MIterInitK) \
  CASE(MIterNext) \
  CASE(MIterNextK) \
  CASE(Mod) \
  CASE(NewArray) \
  CASE(NewCol) \
  CASE(NewPackedArray) \
  CASE(NewStructArray) \
  CASE(NopDefCls) \
  CASE(PredictTL) \
  CASE(PredictTStk) \
  CASE(PushL) \
  CASE(RetC) \
  CASE(RetV) \
  CASE(SSwitch) \
  CASE(SetM) \
  CASE(SetOpL) \
  CASE(SetOpM) \
  CASE(SetWithRefLM) \
  CASE(SetWithRefRM) \
  CASE(StaticLoc) \
  CASE(StaticLocInit) \
  CASE(String) \
  CASE(Switch) \
  CASE(UnboxR) \
  CASE(UnboxRNop) \
  CASE(UnsetL) \
  CASE(UnsetM) \
  CASE(VGetL) \
  CASE(VGetM) \
  CASE(VerifyParamType) \
  CASE(WIterInit) \
  CASE(WIterInitK) \
  CASE(WIterNext) \
  CASE(WIterNextK) \
  CASE(Yield) \
  CASE(YieldK) \
  /* */

#define REGULAR_INSTRS \
  CASE(AGetC) \
  CASE(AKExists) \
  CASE(Abs) \
  CASE(AddElemC) \
  CASE(AddNewElemC) \
  CASE(ArrayIdx) \
  CASE(BindG) \
  CASE(BindS) \
  CASE(BitNot) \
  CASE(CGetG) \
  CASE(CGetS) \
  CASE(CastArray) \
  CASE(CastDouble) \
  CASE(CastInt) \
  CASE(CastObject) \
  CASE(CastString) \
  CASE(Ceil) \
  CASE(CheckThis) \
  CASE(ClassExists) \
  CASE(Clone) \
  CASE(ColAddElemC) \
  CASE(ColAddNewElemC) \
  CASE(Concat) \
  CASE(ContCurrent) \
  CASE(ContKey) \
  CASE(ContStopped) \
  CASE(ContValid) \
  CASE(Dup) \
  CASE(EmptyG) \
  CASE(EmptyS) \
  CASE(False) \
  CASE(Floor) \
  CASE(Idx) \
  CASE(InstanceOf) \
  CASE(InterfaceExists) \
  CASE(IssetG) \
  CASE(IssetS) \
  CASE(LateBoundCls) \
  CASE(NativeImpl) \
  CASE(Nop) \
  CASE(Not) \
  CASE(Null) \
  CASE(NullUninit) \
  CASE(Parent) \
  CASE(PopA) \
  CASE(PopC) \
  CASE(PopR) \
  CASE(PopV) \
  CASE(Print) \
  CASE(Self) \
  CASE(SetG) \
  CASE(SetS) \
  CASE(Shl) \
  CASE(Shr) \
  CASE(Sqrt) \
  CASE(Strlen) \
  CASE(This) \
  CASE(TraitExists) \
  CASE(True) \
  CASE(VGetG) \
  CASE(VGetS) \
  CASE(VerifyRetTypeC) \
  CASE(VerifyRetTypeV) \
  CASE(Xor) \
  /* */


#define INSTRS \
  REGULAR_INSTRS \
  IRREGULAR_INSTRS

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
