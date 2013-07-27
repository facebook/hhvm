/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
  CASE(PopC) \
  CASE(PopV) \
  CASE(PopR) \
  CASE(UnboxR) \
  CASE(Null) \
  CASE(NullUninit) \
  CASE(True) \
  CASE(False) \
  CASE(Int) \
  CASE(Double) \
  CASE(String) \
  CASE(Array) \
  CASE(NewArray) \
  CASE(NewTuple) \
  CASE(NewCol) \
  CASE(Nop) \
  CASE(AddElemC) \
  CASE(AddNewElemC) \
  CASE(ColAddElemC) \
  CASE(ColAddNewElemC) \
  CASE(Cns) \
  CASE(DefCns) \
  CASE(ClsCnsD) \
  CASE(Concat) \
  CASE(Add) \
  CASE(Xor) \
  CASE(Not) \
  CASE(Mod) \
  CASE(BitNot) \
  CASE(CastInt) \
  CASE(CastString) \
  CASE(CastDouble) \
  CASE(CastArray) \
  CASE(CastObject) \
  CASE(Print) \
  CASE(Jmp) \
  CASE(Switch) \
  CASE(SSwitch) \
  CASE(RetC) \
  CASE(RetV) \
  CASE(NativeImpl) \
  CASE(AGetC) \
  CASE(AGetL) \
  CASE(CGetL) \
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
  CASE(This) \
  CASE(BareThis) \
  CASE(CheckThis) \
  CASE(InitThisLoc) \
  CASE(FCall) \
  CASE(FCallArray) \
  CASE(FCallBuiltin) \
  CASE(VerifyParamType) \
  CASE(InstanceOfD) \
  CASE(StaticLocInit) \
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
  CASE(ReqDoc) \
  CASE(DefCls) \
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
  CASE(UnpackCont) \
  CASE(ContSuspend) \
  CASE(ContSuspendK) \
  CASE(ContRetC) \
  CASE(ContCheck) \
  CASE(ContRaise) \
  CASE(ContValid) \
  CASE(ContKey) \
  CASE(ContCurrent) \
  CASE(ContStopped) \
  CASE(ContHandle) \
  CASE(Strlen) \
  CASE(IncStat) \
  CASE(ArrayIdx) \
  CASE(FPushCufIter) \
  CASE(CIterFree) \
  CASE(LateBoundCls) \
  CASE(IssetS) \
  CASE(IssetG) \
  CASE(UnsetG) \
  CASE(EmptyS) \
  CASE(EmptyG) \
  CASE(VGetS) \
  CASE(BindS) \
  CASE(BindG) \
  CASE(IterFree) \
  CASE(MIterFree) \
  CASE(IterBreak) \
  CASE(FPassV) \
  CASE(UnsetN) \
  CASE(DecodeCufIter) \
  CASE(Shl) \
  CASE(Shr) \
  CASE(Div) \
  CASE(Floor) \
  CASE(Ceil) \

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
  CASE(CheckTypeOp)

// PSEUDOINSTR_DISPATCH is a switch() fragment that routes opcodes to their
// shared handlers, as per the PSEUDOINSTRS macro.
#define PSEUDOINSTR_DISPATCH(func)                \
  case Op::BitAnd:                                \
  case Op::BitOr:                                 \
  case Op::BitXor:                                \
  case Op::Sub:                                   \
  case Op::Mul:                                   \
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
  case Op::IssetL:                                \
  case Op::IsNullL:                               \
  case Op::IsStringL:                             \
  case Op::IsArrayL:                              \
  case Op::IsIntL:                                \
  case Op::IsObjectL:                             \
  case Op::IsBoolL:                               \
  case Op::IsDoubleL:                             \
  case Op::IsNullC:                               \
  case Op::IsStringC:                             \
  case Op::IsArrayC:                              \
  case Op::IsIntC:                                \
  case Op::IsObjectC:                             \
  case Op::IsBoolC:                               \
  case Op::IsDoubleC:                             \
    func(CheckTypeOp, i)

#endif
