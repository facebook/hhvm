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
  CASE(Add) \
  CASE(AddO) \
  CASE(Await) \
  CASE(AssertRATL) \
  CASE(AssertRATStk) \
  CASE(BindM) \
  CASE(BreakTraceHint) \
  CASE(CGetM) \
  CASE(ClsCnsD) \
  CASE(ConcatN) \
  CASE(ContEnter) \
  CASE(ContRaise) \
  CASE(CreateCont) \
  CASE(DecodeCufIter) \
  CASE(DefCls) \
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
  CASE(IncDecL) \
  CASE(IncDecM) \
  CASE(IncStat) \
  CASE(InitProp) \
  CASE(IssetM) \
  CASE(IterBreak) \
  CASE(IterInit) \
  CASE(IterInitK) \
  CASE(IterNext) \
  CASE(IterNextK) \
  CASE(Jmp) \
  CASE(JmpNS) \
  CASE(MIterInit) \
  CASE(MIterInitK) \
  CASE(MIterNext) \
  CASE(MIterNextK) \
  CASE(NewStructArray) \
  CASE(RetC) \
  CASE(RetV) \
  CASE(SSwitch) \
  CASE(SetM) \
  CASE(SetOpL) \
  CASE(SetOpM) \
  CASE(SetWithRefLM) \
  CASE(SetWithRefRM) \
  CASE(Switch) \
  CASE(UnsetM) \
  CASE(VGetM) \
  CASE(WIterInit) \
  CASE(WIterInitK) \
  CASE(WIterNext) \
  CASE(WIterNextK) \
  CASE(Yield) \
  CASE(YieldK) \
  /* */

#define REGULAR_INSTRS \
  CASE(AGetC) \
  CASE(AGetL) \
  CASE(AKExists) \
  CASE(Abs) \
  CASE(AddElemC) \
  CASE(AddNewElemC) \
  CASE(Array) \
  CASE(ArrayIdx) \
  CASE(BareThis) \
  CASE(BindG) \
  CASE(BindS) \
  CASE(BitNot) \
  CASE(CGetG) \
  CASE(CGetL) \
  CASE(CGetL2) \
  CASE(CGetS) \
  CASE(CIterFree) \
  CASE(CastArray) \
  CASE(CastDouble) \
  CASE(CastInt) \
  CASE(CastObject) \
  CASE(CastString) \
  CASE(Ceil) \
  CASE(CheckProp) \
  CASE(CheckThis) \
  CASE(Clone) \
  CASE(Cns) \
  CASE(CnsE) \
  CASE(CnsU) \
  CASE(ColAddElemC) \
  CASE(ColAddNewElemC) \
  CASE(Concat) \
  CASE(ContCheck) \
  CASE(ContCurrent) \
  CASE(ContKey) \
  CASE(ContValid) \
  CASE(CreateCl) \
  CASE(DefCns) \
  CASE(DefFunc) \
  CASE(Dir) \
  CASE(Div) \
  CASE(Double) \
  CASE(Dup) \
  CASE(EmptyG) \
  CASE(EmptyS) \
  CASE(FPushClsMethodD) \
  CASE(FPushClsMethod) \
  CASE(FPushClsMethodF) \
  CASE(FPushCtor) \
  CASE(FPushCtorD) \
  CASE(FPushCufIter) \
  CASE(FPushFunc) \
  CASE(FPushFuncD) \
  CASE(FPushFuncU) \
  CASE(FPushObjMethodD) \
  CASE(False) \
  CASE(File) \
  CASE(Floor) \
  CASE(Idx) \
  CASE(InitThisLoc) \
  CASE(InstanceOf) \
  CASE(InstanceOfD) \
  CASE(Int) \
  CASE(IssetG) \
  CASE(IssetL) \
  CASE(IssetS) \
  CASE(IterFree) \
  CASE(LateBoundCls) \
  CASE(MIterFree) \
  CASE(Mod) \
  CASE(Pow) \
  CASE(NameA) \
  CASE(NativeImpl) \
  CASE(NewArray) \
  CASE(NewCol) \
  CASE(NewLikeArrayL) \
  CASE(NewMixedArray) \
  CASE(NewPackedArray) \
  CASE(Not) \
  CASE(Null) \
  CASE(NullUninit) \
  CASE(OODeclExists) \
  CASE(Parent) \
  CASE(PopA) \
  CASE(PopC) \
  CASE(PopR) \
  CASE(PopV) \
  CASE(Print) \
  CASE(PushL) \
  CASE(Self) \
  CASE(SetG) \
  CASE(SetS) \
  CASE(Shl) \
  CASE(Shr) \
  CASE(Silence) \
  CASE(Sqrt) \
  CASE(StaticLoc) \
  CASE(StaticLocInit) \
  CASE(String) \
  CASE(Strlen) \
  CASE(This) \
  CASE(True) \
  CASE(Unbox) \
  CASE(UnboxR) \
  CASE(UnsetL) \
  CASE(VGetG) \
  CASE(VGetL) \
  CASE(VGetS) \
  CASE(VerifyParamType) \
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
