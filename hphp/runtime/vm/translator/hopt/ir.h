/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef incl_HPHP_VM_IR_H_
#define incl_HPHP_VM_IR_H_

#include <algorithm>
#include <cstdarg>
#include <iostream>
#include <vector>
#include <stack>
#include <list>
#include <forward_list>
#include <cassert>
#include <type_traits>
#include <boost/noncopyable.hpp>
#include <boost/checked_delete.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>
#include "util/asm-x64.h"
#include "util/trace.h"
#include "runtime/ext/ext_continuation.h"
#include "runtime/vm/translator/physreg.h"
#include "runtime/vm/translator/abi-x64.h"
#include "runtime/vm/translator/types.h"
#include "runtime/vm/translator/runtime-type.h"
#include "runtime/vm/translator/translator-runtime.h"
#include "runtime/base/types.h"
#include "runtime/vm/func.h"
#include "runtime/vm/class.h"
#include "folly/Range.h"
#include "folly/Conv.h"
#include <boost/intrusive/list.hpp>

namespace HPHP {
// forward declaration
class StringData;
namespace VM {
namespace JIT {

using namespace HPHP::VM::Transl;

class FailedIRGen : public std::exception {
 public:
  const char* file;
  const int   line;
  const char* func;
  FailedIRGen(const char* _file, int _line, const char* _func) :
      file(_file), line(_line), func(_func) { }
};

// Flags to identify if a branch should go to a patchable jmp in astubs
// happens when instructions have been moved off the main trace to the exit path.
static const TCA kIRDirectJmpInactive = nullptr;
// Fixup Jcc;Jmp branches out of trace using REQ_BIND_JMPCC_FIRST/SECOND
static const TCA kIRDirectJccJmpActive = (TCA)0x01;
// Optimize Jcc exit from trace when fall through path stays in trace
static const TCA kIRDirectJccActive = (TCA)0x02;
// Optimize guard exit from beginning of trace
static const TCA kIRDirectGuardActive = (TCA)0x03;

#define SPUNT(instr) do {                           \
  throw FailedIRGen(__FILE__, __LINE__, instr);     \
} while(0)
#define PUNT(instr) SPUNT(#instr)
#define PUNT_WITH_TX64(instr) do {              \
    if (!RuntimeOption::EvalHHIRDisableTx64) {  \
      PUNT(inst);                               \
    }                                           \
  } while (false)

//////////////////////////////////////////////////////////////////////

/*
 * The instruction table below uses the following notation.  To use
 * it, you have to define these symbols to do what you want, and then
 * instantiate IR_OPCODES.
 *
 * dstinfo:
 *
 *   Contains a description of how to compute the type of the
 *   destination(s) of an instruction from its sources.
 *
 *     ND        instruction has no destination
 *     D(type)   single dst has a specific type
 *     DofS(N)   single dst has the type of src N
 *     DUnbox(N) single dst has unboxed type of src N
 *     DBox(N)   single dst has boxed type of src N
 *     DParam    single dst has type of the instruction's type parameter
 *     DMulti    multiple dests. type and number depend on instruction
 *     DVector   single dst depends on semantics of the vector instruction
 *     DStk(x)   up to two dests. x should be another D* macro and indicates
 *               the type of the first dest, if any. the second (or first,
 *               depending on the presence of a primary destination), will be
 *               of type Type::StkPtr. implies ModifiesStack.
 *     DBuiltin  single dst for CallBuiltin. This can return complex data
 *               types such as (Type::Str | Type::Null)
 *
 * srcinfo:
 *
 *   Contains a series of tests on the source parameters in order.
 *
 *     NA            instruction takes no sources
 *     SUnk          intructions sources are not yet documented/checked
 *     S(t1,...,tn)  source must be a subtype of {t1|..|tn}
 *     C(type)       source must be a constant, and subtype of type
 *     CStr          same as C(StaticStr)
 *     SNumInt       same as S(Int,Bool)
 *     SNum          same as S(Int,Bool,Dbl)
 *
 * flags:
 *
 *   See doc/ir.specification for the meaning of these flag various.
 *
 *   The flags in this opcode table supply default values for the
 *   querying functions in IRInstruction---those functions involve
 *   additional logic (based on operand types, etc) on a
 *   per-instruction basis.
 *
 *   The following abbreviations are used in this table:
 *
 *      NF    no flags
 *      C     canCSE
 *      E     isEssential
 *      N     callsNative
 *      PRc   producesRC
 *      CRc   consumesRC
 *      Refs  mayModifyRefs
 *      Rm    isRematerializable
 *      Er    mayRaiseError
 *      Mem   hasMemEffects
 *      T     isTerminal
 *      P     passthrough
 *      K     killsSource
 *      VProp VectorProp
 *      VElem VectorElem
 */

#define O_STK(name, dst, src, flags)            \
  O(name, dst, src, StkFlags(flags))            \
  O(name ## Stk, DStk(dst), src, flags)

#define IR_OPCODES                                                            \
/*    name                      dstinfo srcinfo                      flags */ \
O(GuardType,                    DParam, S(Gen),                C|E|CRc|PRc|P) \
O(GuardLoc,                         ND, S(StkPtr),                         E) \
O(GuardStk,                  D(StkPtr), S(StkPtr) C(Int),                  E) \
O(CastStk,                   D(StkPtr), S(StkPtr) C(Int),           Mem|N|Er) \
O(AssertStk,                 D(StkPtr), S(StkPtr) C(Int),                  E) \
O(GuardRefs,                        ND, SUnk,                              E) \
O(AssertLoc,                        ND, S(StkPtr),                         E) \
O(OpAdd,                        DParam, SNum SNum,                         C) \
O(OpSub,                        DParam, SNum SNum,                         C) \
O(OpAnd,                        D(Int), SNumInt SNumInt,                   C) \
O(OpOr,                         D(Int), SNum SNum,                         C) \
O(OpXor,                        D(Int), SNumInt SNumInt,                   C) \
O(OpMul,                        DParam, SNum SNum,                         C) \
O(ConvBoolToArr,                D(Arr), S(Bool),                         C|N) \
O(ConvDblToArr,                 D(Arr), S(Dbl),                          C|N) \
O(ConvIntToArr,                 D(Arr), S(Int),                          C|N) \
O(ConvObjToArr,                 D(Arr), S(Obj),                        N|CRc) \
O(ConvStrToArr,                 D(Arr), S(Str),                        N|CRc) \
O(ConvGenToArr,                 D(Arr), S(Gen),                        N|CRc) \
O(ConvToBool,                  D(Bool), S(Gen),                          C|N) \
O(ConvArrToDbl,                 D(Dbl), S(Arr),                        N|CRc) \
O(ConvBoolToDbl,                D(Dbl), S(Bool),                         C|N) \
O(ConvIntToDbl,                 D(Dbl), S(Int),                          C|N) \
O(ConvObjToDbl,                 D(Dbl), S(Obj),                     N|Er|CRc) \
O(ConvStrToDbl,                 D(Dbl), S(Str),                        N|CRc) \
O(ConvGenToDbl,                 D(Dbl), S(Gen),                     N|Er|CRc) \
O(ConvToInt,                    D(Int), S(Gen),                          C|N) \
O(ConvToObj,                    D(Obj), S(Gen),                          C|N) \
O(ConvToStr,                    D(Str), S(Gen),                          C|N) \
O(ExtendsClass,                D(Bool), S(Cls) C(Cls),                     C) \
O(IsTypeMem,                   D(Bool), S(PtrToGen),                      NA) \
O(IsNTypeMem,                  D(Bool), S(PtrToGen),                      NA) \
                                                                              \
  /* TODO(#2058842): order currently matters for the 'query ops' here */      \
O(OpGt,                        D(Bool), S(Gen) S(Gen),                   C|N) \
O(OpGte,                       D(Bool), S(Gen) S(Gen),                   C|N) \
O(OpLt,                        D(Bool), S(Gen) S(Gen),                   C|N) \
O(OpLte,                       D(Bool), S(Gen) S(Gen),                   C|N) \
O(OpEq,                        D(Bool), S(Gen) S(Gen),                   C|N) \
O(OpNeq,                       D(Bool), S(Gen) S(Gen),                   C|N) \
O(OpSame,                      D(Bool), S(Gen) S(Gen),                   C|N) \
O(OpNSame,                     D(Bool), S(Gen) S(Gen),                   C|N) \
O(InstanceOf,                  D(Bool), S(Cls) S(Cls) C(Bool),           C|N) \
O(NInstanceOf,                 D(Bool), S(Cls) S(Cls) C(Bool),           C|N) \
O(InstanceOfBitmask,           D(Bool), S(Cls) CStr,                       C) \
O(NInstanceOfBitmask,          D(Bool), S(Cls) CStr,                       C) \
O(IsType,                      D(Bool), S(Cell),                           C) \
O(IsNType,                     D(Bool), S(Cell),                           C) \
  /* there is a conditional branch for each of the above query ops */         \
O(JmpGt,                       D(None), S(Gen) S(Gen),                     E) \
O(JmpGte,                      D(None), S(Gen) S(Gen),                     E) \
O(JmpLt,                       D(None), S(Gen) S(Gen),                     E) \
O(JmpLte,                      D(None), S(Gen) S(Gen),                     E) \
O(JmpEq,                       D(None), S(Gen) S(Gen),                     E) \
O(JmpNeq,                      D(None), S(Gen) S(Gen),                     E) \
O(JmpSame,                     D(None), S(Gen) S(Gen),                     E) \
O(JmpNSame,                    D(None), S(Gen) S(Gen),                     E) \
O(JmpInstanceOf,               D(None), S(Cls) S(Cls) C(Bool),           E|N) \
O(JmpNInstanceOf,              D(None), S(Cls) S(Cls) C(Bool),           E|N) \
O(JmpInstanceOfBitmask,        D(None), S(Cls) CStr,                       E) \
O(JmpNInstanceOfBitmask,       D(None), S(Cls) CStr,                       E) \
O(JmpIsType,                   D(None), SUnk,                              E) \
O(JmpIsNType,                  D(None), SUnk,                              E) \
  /* TODO(#2058842) keep preceeding conditional branches contiguous */        \
                                                                              \
/*    name                      dstinfo srcinfo                      flags */ \
O(JmpZero,                     D(None), SNum,                              E) \
O(JmpNZero,                    D(None), SNum,                              E) \
O(Jmp_,                        D(None), SUnk,                            T|E) \
O(JmpIndirect,                      ND, S(TCA),                          T|E) \
O(ExitWhenSurprised,                ND, NA,                                E) \
O(ExitOnVarEnv,                     ND, S(StkPtr),                         E) \
O(ReleaseVVOrExit,                  ND, S(StkPtr),                       N|E) \
O(RaiseError,                       ND, S(Str),               E|N|Mem|Refs|T) \
O(CheckInit,                        ND, S(Gen),                           NF) \
O(CheckInitMem,                     ND, S(PtrToGen) C(Int),               NF) \
O(Unbox,                     DUnbox(0), S(Gen),                           NF) \
O(Box,                         DBox(0), S(Init),             E|N|Mem|CRc|PRc) \
O(UnboxPtr,               D(PtrToCell), S(PtrToGen),                      NF) \
O(BoxPtr,            D(PtrToBoxedCell), S(PtrToGen),                   N|Mem) \
O(LdStack,                      DParam, S(StkPtr) C(Int),                 NF) \
O(LdLoc,                        DParam, S(StkPtr),                        NF) \
O(LdStackAddr,                  DParam, S(StkPtr) C(Int),                  C) \
O(LdLocAddr,                    DParam, S(StkPtr),                         C) \
O(LdMem,                        DParam, S(PtrToGen) C(Int),               NF) \
O(LdProp,                       DParam, S(Obj) C(Int),                    NF) \
O(LdRef,                        DParam, S(BoxedCell),                     NF) \
O(LdThis,                       D(Obj), S(StkPtr),                      C|Rm) \
O(LdRetAddr,                D(RetAddr), S(StkPtr),                        NF) \
O(LdConst,                      DParam, NA,                             C|Rm) \
O(DefConst,                     DParam, NA,                                C) \
O(LdCtx,                        D(Ctx), S(StkPtr) S(Func),              C|Rm) \
O(LdCctx,                      D(Cctx), S(StkPtr),                      C|Rm) \
O(LdCls,                        D(Cls), S(Str) C(Cls),     C|E|N|Refs|Er|Mem) \
O(LdClsCached,                  D(Cls), CStr,              C|E|N|Refs|Er|Mem) \
O(LdCachedClass,                D(Cls), CStr,                              C) \
O(LdClsCtx,                     D(Cls), S(Ctx),                            C) \
O(LdClsCctx,                    D(Cls), S(Cctx),                           C) \
O(LdClsCns,                     DParam, CStr CStr,                         C) \
O(LookupClsCns,                 DParam, CStr CStr,           E|Refs|Er|N|Mem) \
O(LdClsMethodCache,         D(FuncCls), SUnk,              N|C|E|Refs|Er|Mem) \
O(LdClsMethodFCache,        D(FuncCtx), C(Cls) CStr S(Obj,Cls,Ctx), N|C|E|Er) \
O(GetCtxFwdCall,                D(Ctx), S(Ctx) S(Func),                    C) \
O(LdClsMethod,                 D(Func), S(Cls) C(Int),                     C) \
O(LdPropAddr,              D(PtrToGen), S(Obj) C(Int),                     C) \
O(LdClsPropAddr,           D(PtrToGen), S(Cls) S(Str) C(Cls),       C|E|N|Er) \
O(LdClsPropAddrCached,     D(PtrToGen), S(Cls) CStr CStr C(Cls),    C|E|N|Er) \
O(LdObjMethod,                      ND, S(Cls) CStr S(StkPtr),   E|N|Refs|Er) \
O(LdGblAddrDef,            D(PtrToGen), S(Str),                      E|N|CRc) \
O(LdGblAddr,               D(PtrToGen), S(Str),                        N    ) \
O(LdObjClass,                   D(Cls), S(Obj),                            C) \
O(LdFunc,                      D(Func), S(Str),                   E|N|CRc|Er) \
O(LdFixedFunc,                 D(Func), CStr,                       N|C|E|Er) \
O(LdARFuncPtr,                 D(Func), S(StkPtr) C(Int),                  C) \
O(LdContLocalsPtr,        D(PtrToCell), S(Obj),                            C) \
O(LdSSwitchDestFast,            D(TCA), S(Gen),                            N) \
O(LdSSwitchDestSlow,            D(TCA), S(Gen),                  E|N|Refs|Er) \
O(LdSwitchDblIndex,             D(Int), S(Dbl) S(Int) S(Int),              N) \
O(LdSwitchStrIndex,             D(Int), S(Str) S(Int) S(Int),          CRc|N) \
O(LdSwitchObjIndex,             D(Int), S(Obj) S(Int) S(Int),       CRc|N|Er) \
O(JmpSwitchDest,                    ND, S(Int),                          T|E) \
O(NewObj,                    D(StkPtr), C(Int)                                \
                                          S(Str,Cls)                          \
                                          S(StkPtr)                           \
                                          S(StkPtr),     E|Mem|N|Refs|PRc|Er) \
O(NewArray,                     D(Arr), C(Int),                  E|Mem|N|PRc) \
O(NewTuple,                     D(Arr), C(Int) S(StkPtr),    E|Mem|N|PRc|CRc) \
O(LdRaw,                        DParam, SUnk,                             NF) \
O(DefActRec,                 D(ActRec), S(StkPtr)                             \
                                          S(Func,FuncCls,FuncCtx,Null)        \
                                          S(Ctx,Cls,InitNull)                 \
                                          C(Int)                              \
                                          S(Str,Null),                   Mem) \
O(FreeActRec,                D(StkPtr), S(StkPtr),                       Mem) \
/*    name                      dstinfo srcinfo                      flags */ \
O(Call,                      D(StkPtr), SUnk,                 E|Mem|CRc|Refs) \
O(CallBuiltin,                DBuiltin, SUnk,            E|Mem|Refs|Er|N|PRc) \
O(NativeImpl,                       ND, C(Func) S(StkPtr),      E|Mem|N|Refs) \
  /* XXX: why does RetCtrl sometimes get PtrToGen */                          \
O(RetCtrl,                          ND, S(StkPtr,PtrToGen)                    \
                                          S(StkPtr)                           \
                                          S(RetAddr),                T|E|Mem) \
O(RetVal,                           ND, S(StkPtr) S(Gen),          E|Mem|CRc) \
O(RetAdjustStack,            D(StkPtr), S(StkPtr),                         E) \
O(StMem,                            ND, S(PtrToGen)                           \
                                          C(Int) S(Gen),      E|Mem|CRc|Refs) \
O(StMemNT,                          ND, S(PtrToGen)                           \
                                          C(Int) S(Gen),           E|Mem|CRc) \
O(StProp,                           ND, S(Obj) S(Int) S(Gen), E|Mem|CRc|Refs) \
O(StPropNT,                         ND, S(Obj) S(Int) S(Gen),      E|Mem|CRc) \
O(StLoc,                            ND, S(StkPtr) S(Gen),          E|Mem|CRc) \
O(StLocNT,                          ND, S(StkPtr) S(Gen),          E|Mem|CRc) \
O(StRef,                       DBox(1), SUnk,                 E|Mem|CRc|Refs) \
O(StRefNT,                     DBox(1), SUnk,                      E|Mem|CRc) \
O(StRaw,                            ND, SUnk,                          E|Mem) \
O(SpillStack,                D(StkPtr), SUnk,                      E|Mem|CRc) \
O(ExitTrace,                        ND, SUnk,                            T|E) \
O(ExitTraceCc,                      ND, SUnk,                            T|E) \
O(ExitSlow,                         ND, SUnk,                            T|E) \
O(ExitSlowNoProgress,               ND, SUnk,                            T|E) \
O(ExitGuardFailure,                 ND, SUnk,                            T|E) \
O(SyncVMRegs,                       ND, S(StkPtr) S(StkPtr),               E) \
O(Mov,                         DofS(0), SUnk,                            C|P) \
O(LdAddr,                      DofS(0), SUnk,                              C) \
O(IncRef,                      DofS(0), S(Gen),                    Mem|PRc|P) \
O(DecRefLoc,                        ND, S(StkPtr),              N|E|Mem|Refs) \
O(DecRefStack,                      ND, S(StkPtr) C(Int),       N|E|Mem|Refs) \
O(DecRefThis,                       ND, SUnk,                   N|E|Mem|Refs) \
O(DecRefKillThis,                   ND, S(Gen),           N|E|Mem|CRc|Refs|K) \
O(GenericRetDecRefs,         D(StkPtr), S(StkPtr)                             \
                                          S(Gen) C(Int),        E|N|Mem|Refs) \
O(DecRef,                           ND, S(Gen),           N|E|Mem|CRc|Refs|K) \
O(DecRefMem,                        ND, S(PtrToGen)                           \
                                          C(Int),           N|E|Mem|CRc|Refs) \
O(DecRefNZ,                         ND, S(Gen),                      Mem|CRc) \
O(DecRefNZOrBranch,                 ND, S(Gen),                      Mem|CRc) \
O(DefLabel,                     DMulti, SUnk,                              E) \
O(Marker,                           ND, NA,                                E) \
O(DefFP,                     D(StkPtr), NA,                                E) \
O(DefSP,                     D(StkPtr), S(StkPtr) C(Int),                  E) \
O(VerifyParamCls,                   ND, S(Cls) S(Cls) C(Int),E|N|Mem|Refs|Er) \
O(VerifyParamCallable,              ND, S(Cell) C(Int),      E|N|Mem|Refs|Er) \
O(VerifyParamFail,                  ND, C(Int),              E|N|Mem|Refs|Er) \
O(RaiseUninitLoc,                   ND, S(Str),              E|N|Mem|Refs|Er) \
O(WarnNonObjProp,                   ND, NA,                  E|N|Refs|Er|Mem) \
O(ThrowNonObjProp,                  ND, NA,                T|E|N|Refs|Er|Mem) \
O(RaiseUndefProp,                   ND, S(Obj) CStr,         E|N|Refs|Er|Mem) \
O(PrintStr,                         ND, S(Str),                  E|N|Mem|CRc) \
O(PrintInt,                         ND, S(Int),                  E|N|Mem|CRc) \
O(PrintBool,                        ND, S(Bool),                 E|N|Mem|CRc) \
O(AddElemStrKey,                D(Arr), S(Arr)                                \
                                          S(Str)                              \
                                          S(Cell),        N|Mem|CRc|PRc|Refs) \
O(AddElemIntKey,                D(Arr), S(Arr)                                \
                                          S(Int)                              \
                                          S(Cell),        N|Mem|CRc|PRc|Refs) \
O(AddNewElem,                   D(Arr), SUnk,                  N|Mem|CRc|PRc) \
/*    name                      dstinfo srcinfo                      flags */ \
O(DefCns,                      D(Bool), C(Str) S(Cell),          E|N|Mem|CRc) \
O(Concat,                       D(Str), S(Gen) S(Gen),    N|Mem|CRc|PRc|Refs) \
O(ArrayAdd,                     D(Arr), SUnk,                  N|Mem|CRc|PRc) \
O(DefCls,                           ND, SUnk,                          C|E|N) \
O(DefFunc,                          ND, SUnk,                       C|E|N|Er) \
O(AKExists,                    D(Bool), S(Cell) S(Cell),                 C|N) \
O(InterpOne,                 D(StkPtr), SUnk,                E|N|Mem|Refs|Er) \
O(Spill,                       DofS(0), SUnk,                            Mem) \
O(Reload,                      DofS(0), SUnk,                            Mem) \
O(AllocSpill,                       ND, C(Int),                        E|Mem) \
O(FreeSpill,                        ND, C(Int),                        E|Mem) \
O(CreateCont,                   D(Obj), C(TCA)                                \
                                          S(StkPtr)                           \
                                          C(Bool)                             \
                                          C(Func)                             \
                                          C(Func),               E|N|Mem|PRc) \
O(FillContLocals,                   ND, S(StkPtr)                             \
                                          C(Func)                             \
                                          C(Func)                             \
                                          S(Obj),                    E|N|Mem) \
O(FillContThis,                     ND, S(Obj)                                \
                                          S(PtrToCell) C(Int),         E|Mem) \
O(ContEnter,                        ND, SUnk,                          E|Mem) \
O(UnlinkContVarEnv,                 ND, S(StkPtr),                   E|N|Mem) \
O(LinkContVarEnv,                   ND, S(StkPtr),                   E|N|Mem) \
O(ContRaiseCheck,                   ND, S(Obj),                            E) \
O(ContPreNext,                      ND, S(Obj),                        E|Mem) \
O(ContStartedCheck,                 ND, S(Obj),                            E) \
O(IterInit,                    D(Bool), S(Arr,Obj)                            \
                                          S(StkPtr)                           \
                                          C(Int)                              \
                                          C(Int),           E|N|Mem|Refs|CRc) \
O(IterInitK,                   D(Bool), S(Arr,Obj)                            \
                                          S(StkPtr)                           \
                                          C(Int)                              \
                                          C(Int)                              \
                                          C(Int),           E|N|Mem|Refs|CRc) \
O(IterNext,                    D(Bool), S(StkPtr) C(Int) C(Int),E|N|Mem|Refs) \
O(IterNextK,                   D(Bool), S(StkPtr)                             \
                                          C(Int) C(Int) C(Int), E|N|Mem|Refs) \
O(IterFree,                         ND, S(StkPtr) C(Int),       E|N|Mem|Refs) \
O(DefMIStateBase,         D(PtrToCell), NA,                               NF) \
O(BaseG,                   D(PtrToGen), C(TCA)                                \
                                          S(Str)                              \
                                          S(PtrToCell),      E|N|Mem|Refs|Er) \
O(PropX,                   D(PtrToGen), C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Gen)                              \
                                          S(PtrToCell),      E|N|Mem|Refs|Er) \
O_STK(PropDX,              D(PtrToGen), C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Gen)                              \
                                          S(PtrToCell),VProp|E|N|Mem|Refs|Er) \
O(CGetProp,                    D(Cell), C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Gen)                              \
                                          S(PtrToCell),      E|N|Mem|Refs|Er) \
O_STK(VGetProp,           D(BoxedCell), C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Gen)                              \
                                          S(PtrToCell),VProp|E|N|Mem|Refs|Er) \
O_STK(BindProp,                     ND, C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Gen)                              \
                                          S(BoxedCell)                        \
                                          S(PtrToCell),VProp|E|N|Mem|Refs|Er) \
O_STK(SetProp,                 DVector, C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Gen)                              \
                                          S(Cell),     VProp|E|N|Mem|Refs|Er) \
O(UnsetProp,                        ND, C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Gen),            E|N|Mem|Refs|Er) \
O_STK(SetOpProp,               D(Cell), C(TCA)                                \
                                          S(Obj,PtrToGen)                     \
                                          S(Gen)                              \
                                          S(Cell)                             \
                                          S(PtrToCell),VProp|E|N|Mem|Refs|Er) \
O_STK(IncDecProp,              D(Cell), C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Gen)                              \
                                          S(PtrToCell),VProp|E|N|Mem|Refs|Er) \
O(EmptyProp,                   D(Bool), C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Gen),            E|N|Mem|Refs|Er) \
O(IssetProp,                   D(Bool), C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Gen),            E|N|Mem|Refs|Er) \
O(ElemX,                   D(PtrToGen), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Gen)                              \
                                          S(PtrToCell),      E|N|Mem|Refs|Er) \
O_STK(ElemDX,              D(PtrToGen), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Gen)                              \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O_STK(ElemUX,              D(PtrToGen), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Gen)                              \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O(ArrayGet,                    D(Cell), C(TCA)                                \
                                          S(Arr)                              \
                                          S(Int,Str),    E|N|PRc|Refs|Mem|Er) \
O(CGetElem,                    D(Cell), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Gen)                              \
                                          S(PtrToCell),      E|N|Mem|Refs|Er) \
O_STK(VGetElem,           D(BoxedCell), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Gen)                              \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O_STK(BindElem,                     ND, C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Gen)                              \
                                          S(BoxedCell)                        \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O(ArraySet,                     D(Arr), C(TCA)                                \
                                          S(Arr)                              \
                                          S(Int,Str)                          \
                                          S(Cell),    E|N|PRc|CRc|Refs|Mem|K) \
O(ArraySetRef,                      ND, C(TCA)                                \
                                          S(Arr)                              \
                                          S(Int,Str)                          \
                                          S(Cell)                             \
                                          S(BoxedArr),E|N|PRc|CRc|Refs|Mem|K) \
O_STK(SetElem,                 DVector, C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Gen)                              \
                                          S(Cell),     VElem|E|N|Mem|Refs|Er) \
O_STK(UnsetElem,                    ND, C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Gen),      VElem|E|N|Mem|Refs|Er) \
O_STK(SetOpElem,               D(Cell), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Gen)                              \
                                          S(Cell)                             \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O_STK(IncDecElem,              D(Cell), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Gen)                              \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O_STK(SetNewElem,              DVector, S(PtrToGen)                           \
                                          S(Gen),      VElem|E|N|Mem|Refs|Er) \
O_STK(BindNewElem,                  ND, S(PtrToGen)                           \
                                          S(BoxedCell)                        \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O(ArrayIsset,                  D(Bool), C(TCA)                                \
                                          S(Arr)                              \
                                          S(Int,Str),        E|N|Mem|Refs|Er) \
O(IssetElem,                   D(Bool), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Gen)                              \
                                          S(PtrToCell),      E|N|Mem|Refs|Er) \
O(EmptyElem,                   D(Bool), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Gen)                              \
                                          S(PtrToCell),      E|N|Mem|Refs|Er) \
O(IncStat,                          ND, C(Int) C(Int) C(Bool),         E|Mem) \
O(IncStatGrouped,                   ND, CStr CStr C(Int),            E|N|Mem) \
O(DbgAssertRefCount,                ND, SUnk,                            N|E) \
O(DbgAssertPtr,                     ND, S(PtrToGen),                     N|E) \
O(Nop,                              ND, NA,                               NF) \
/* */

enum Opcode : uint16_t {
#define O(name, dsts, srcs, flags) name,
  IR_OPCODES
#undef O
  IR_NUM_OPCODES
};

//////////////////////////////////////////////////////////////////////

/*
 * Some IRInstructions with compile-time-only constants may carry
 * along extra data in the form of one of these structures.
 *
 * Note that this isn't really appropriate for compile-time constants
 * that are actually representing user values (we want them to be
 * visible to optimization passes, allocatable to registers, etc),
 * just compile-time metadata.
 *
 * These types must:
 *
 *   - Derive from IRExtraData (for overloading purposes)
 *   - Be arena-allocatable (no non-trivial destructors)
 *   - Either CopyConstructible, or implement a clone member
 *     function that takes an arena to clone to
 *
 * In addition, for extra data used with a cse-able instruction:
 *
 *   - Implement an equals() member that indicates equality for CSE
 *     purposes.
 *   - Implement a hash() method.
 *
 * Finally, optionally they may implement a show() method for use in
 * debug printouts.
 */

/*
 * Traits that returns the type of the extra C++ data structure for a
 * given instruction, if it has one, along with some other information
 * about the type.
 */
template<Opcode op> struct OpHasExtraData { enum { value = 0 }; };
template<Opcode op> struct IRExtraDataType;

//////////////////////////////////////////////////////////////////////

struct IRExtraData {};

struct LdSSwitchData : IRExtraData {
  struct Elm {
    const StringData* str;
    Offset            dest;
  };

  explicit LdSSwitchData() = default;
  LdSSwitchData(const LdSSwitchData&) = delete;
  LdSSwitchData& operator=(const LdSSwitchData&) = delete;

  LdSSwitchData* clone(Arena& arena) const {
    LdSSwitchData* target = new (arena) LdSSwitchData;
    target->func       = func;
    target->numCases   = numCases;
    target->defaultOff = defaultOff;
    target->cases      = new (arena) Elm[numCases];
    std::copy(cases, cases + numCases, const_cast<Elm*>(target->cases));
    return target;
  }

  const Func* func;
  int64_t     numCases;
  const Elm*  cases;
  Offset      defaultOff;
};

struct JmpSwitchData : IRExtraData {
  JmpSwitchData* clone(Arena& arena) const {
    JmpSwitchData* sd = new (arena) JmpSwitchData;
    sd->func       = func;
    sd->base       = base;
    sd->bounded    = bounded;
    sd->cases      = cases;
    sd->defaultOff = defaultOff;
    sd->targets    = new (arena) Offset[cases];
    std::copy(targets, targets + cases, const_cast<Offset*>(sd->targets));
    return sd;
  }

  const Func* func;
  int64_t base;        // base of switch case
  bool    bounded;     // whether switch is bounded or not
  int32_t cases;       // number of cases
  Offset  defaultOff;  // offset of default case
  Offset* targets;     // offsets for all targets
};

struct MarkerData : IRExtraData {
  uint32_t    bcOff;    // the bytecode offset in unit
  int32_t     stackOff; // stack off from start of trace
  const Func* func;     // which func are we in
};

struct LocalId : IRExtraData {
  explicit LocalId(uint32_t id)
    : locId(id)
  {}

  bool equals(LocalId o) const { return locId == o.locId; }
  size_t hash() const { return std::hash<uint32_t>()(locId); }
  std::string show() const { return folly::to<std::string>(locId); }

  uint32_t locId;
};

struct ConstData : IRExtraData {
  template<class T>
  explicit ConstData(T data)
    : m_dataBits(0)
  {
    static_assert(sizeof(T) <= sizeof m_dataBits,
                  "Constant data was larger than supported");
    static_assert(std::is_pod<T>::value,
                  "Constant data wasn't a pod?");
    std::memcpy(&m_dataBits, &data, sizeof data);
  }

  template<class T>
  T as() const {
    T ret;
    std::memcpy(&ret, &m_dataBits, sizeof ret);
    return ret;
  }

  bool equals(ConstData o) const { return m_dataBits == o.m_dataBits; }
  size_t hash() const { return std::hash<uintptr_t>()(m_dataBits); }

private:
  uintptr_t m_dataBits;
};

/*
 * EdgeData is linked list node that tracks the set of Jmp_'s that pass values
 * to a particular block.  Each such Jmp_ has one node, and the block points
 * to the list head.
 */
struct EdgeData : IRExtraData {
  struct IRInstruction* jmp;  // owner of this edge
  EdgeData* next;             // next edge to same target
};

/*
 * ExitData contains the address of a jmp instruction we can smash later
 * if we start a new tracelet at this exit point.
 */
struct ExitData : IRExtraData {
  explicit ExitData(IRInstruction* toSmash) : toSmash(toSmash) {}
  IRInstruction* toSmash;

  std::string show() const;
};

//////////////////////////////////////////////////////////////////////

#define X(op, data)                                                   \
  template<> struct IRExtraDataType<op> { typedef data type; };       \
  template<> struct OpHasExtraData<op> { enum { value = 1 }; };       \
  static_assert(boost::has_trivial_destructor<data>::value,           \
                "IR extra data type must be trivially destructible")

X(JmpSwitchDest,      JmpSwitchData);
X(LdSSwitchDestFast,  LdSSwitchData);
X(LdSSwitchDestSlow,  LdSSwitchData);
X(Marker,             MarkerData);
X(RaiseUninitLoc,     LocalId);
X(GuardLoc,           LocalId);
X(AssertLoc,          LocalId);
X(LdLocAddr,          LocalId);
X(DecRefLoc,          LocalId);
X(LdLoc,              LocalId);
X(StLoc,              LocalId);
X(StLocNT,            LocalId);
X(DefConst,           ConstData);
X(LdConst,            ConstData);
X(Jmp_,               EdgeData);
X(ExitTrace,          ExitData);
X(ExitTraceCc,        ExitData);
#undef X

//////////////////////////////////////////////////////////////////////

template<bool hasExtra, Opcode opc, class T> struct AssertExtraTypes {
  static void doassert() {
    assert(!"called getExtra on an opcode without extra data");
  }
};

template<Opcode opc, class T> struct AssertExtraTypes<true,opc,T> {
  static void doassert() {
    typedef typename IRExtraDataType<opc>::type ExtraType;
    if (!std::is_same<ExtraType,T>::value) {
      assert(!"getExtra<T> was called with an extra data "
              "type that doesn't match the opcode type");
    }
  }
};

// Asserts that Opcode opc has extradata and it is of type T.
template<class T> void assert_opcode_extra(Opcode opc) {
#define O(opcode, dstinfo, srcinfo, flags)      \
  case opcode:                                  \
    AssertExtraTypes<                           \
      OpHasExtraData<opcode>::value,opcode,T    \
    >::doassert();                              \
    break;
  switch (opc) { IR_OPCODES default: not_reached(); }
#undef O
}

//////////////////////////////////////////////////////////////////////

inline bool isCmpOp(Opcode opc) {
  return (opc >= OpGt && opc <= OpNSame);
}

// A "query op" is any instruction returning Type::Bool that is both
// branch-fusable and negateable.
inline bool isQueryOp(Opcode opc) {
  return (opc >= OpGt && opc <= IsNType);
}

inline Opcode queryToJmpOp(Opcode opc) {
  assert(isQueryOp(opc));
  return (Opcode)(JmpGt + (opc - OpGt));
}

inline bool isQueryJmpOp(Opcode opc) {
  switch (opc) {
    case JmpGt:
    case JmpGte:
    case JmpLt:
    case JmpLte:
    case JmpEq:
    case JmpNeq:
    case JmpSame:
    case JmpNSame:
    case JmpInstanceOf:
    case JmpNInstanceOf:
    case JmpInstanceOfBitmask:
    case JmpNInstanceOfBitmask:
    case JmpIsType:
    case JmpIsNType:
    case JmpZero:
    case JmpNZero:
      return true;
    default:
      return false;
  }
}

inline Opcode queryJmpToQueryOp(Opcode opc) {
  assert(isQueryJmpOp(opc));
  assert(opc != JmpZero && opc != JmpNZero);
  return Opcode(OpGt + (opc - JmpGt));
}

inline ConditionCode queryJmpToCC(Opcode opc) {
  assert(isQueryJmpOp(opc));

  switch (opc) {
    case JmpGt:                 return CC_G;
    case JmpGte:                return CC_GE;
    case JmpLt:                 return CC_L;
    case JmpLte:                return CC_LE;
    case JmpEq:                 return CC_E;
    case JmpNeq:                return CC_NE;
    case JmpSame:               return CC_E;
    case JmpNSame:              return CC_NE;
    case JmpInstanceOf:         return CC_NZ;
    case JmpNInstanceOf:        return CC_Z;
    case JmpInstanceOfBitmask:  return CC_NZ;
    case JmpNInstanceOfBitmask: return CC_Z;
    case JmpIsType:             return CC_NZ;
    case JmpIsNType:            return CC_Z;
    case JmpZero:               return CC_Z;
    case JmpNZero:              return CC_NZ;
    default:
      not_reached();
  }
}

/*
 * Right now branch fusion is too indiscriminate to handle fusing
 * with potentially expensive-to-repeat operations.  TODO(#2053369)
 */
inline bool disableBranchFusion(Opcode opc) {
  return opc == InstanceOf || opc == NInstanceOf;
}

/*
 * Return the opcode that corresponds to negation of opc.
 */
Opcode negateQueryOp(Opcode opc);

extern Opcode queryCommuteTable[];

inline Opcode commuteQueryOp(Opcode opc) {
  assert(opc >= OpGt && opc <= OpNSame);
  return queryCommuteTable[opc - OpGt];
}

namespace TraceExitType {
// Must update in sync with ExitTrace entries in OPC table above
enum ExitType {
  Normal,
  NormalCc,
  Slow,
  SlowNoProgress,
  GuardFailure
};
}

extern TraceExitType::ExitType getExitType(Opcode opc);
extern Opcode getExitOpcode(TraceExitType::ExitType);
inline bool isExitSlow(TraceExitType::ExitType t) {
  return t == TraceExitType::Slow || t == TraceExitType::SlowNoProgress;
}

const char* opcodeName(Opcode opcode);

enum OpcodeFlag : uint64_t {
  NoFlags          = 0,
  HasDest          = 1ULL <<  0,
  CanCSE           = 1ULL <<  1,
  Essential        = 1ULL <<  2,
  MemEffects       = 1ULL <<  3,
  CallsNative      = 1ULL <<  4,
  ConsumesRC       = 1ULL <<  5,
  ProducesRC       = 1ULL <<  6,
  MayModifyRefs    = 1ULL <<  7,
  Rematerializable = 1ULL <<  8, // TODO: implies HasDest
  MayRaiseError    = 1ULL <<  9,
  Terminal         = 1ULL << 10, // has no next instruction
  NaryDest         = 1ULL << 11, // has 0 or more destinations
  HasExtra         = 1ULL << 12,
  Passthrough      = 1ULL << 13,
  KillsSources     = 1ULL << 14,
  ModifiesStack    = 1ULL << 15,
  HasStackVersion  = 1ULL << 16,
  VectorProp       = 1ULL << 17,
  VectorElem       = 1ULL << 18,
};

bool opcodeHasFlags(Opcode opc, uint64_t flags);
Opcode getStackModifyingOpcode(Opcode opc);

#define IRT_BOXES(name, bit)                                            \
  IRT(name,             (bit))                                          \
  IRT(Boxed##name,      (bit) << kBoxShift)                             \
  IRT(PtrTo##name,      (bit) << kPtrShift)                             \
  IRT(PtrToBoxed##name, (bit) << kPtrBoxShift)

#define IRT_PHP(c)                                                      \
  c(Uninit,       1ULL << 0)                                            \
  c(InitNull,     1ULL << 1)                                            \
  c(Bool,         1ULL << 2)                                            \
  c(Int,          1ULL << 3)                                            \
  c(Dbl,          1ULL << 4)                                            \
  c(StaticStr,    1ULL << 5)                                            \
  c(CountedStr,   1ULL << 6)                                            \
  c(StaticArr,    1ULL << 7)                                            \
  c(CountedArr,   1ULL << 8)                                            \
  c(Obj,          1ULL << 9)
// Boxed*:       10-19
// PtrTo*:       20-29
// PtrToBoxed*:  30-39

// This list should be in non-decreasing order of specificity
#define IRT_PHP_UNIONS(c)                                               \
  c(Null,          kUninit|kInitNull)                                   \
  c(Str,           kStaticStr|kCountedStr)                              \
  c(Arr,           kStaticArr|kCountedArr)                              \
  c(UncountedInit, kInitNull|kBool|kInt|kDbl|kStaticStr|kStaticArr)     \
  c(Uncounted,     kUncountedInit|kUninit)                              \
  c(Cell,          kUncounted|kStr|kArr|kObj)

#define IRT_RUNTIME                                                     \
  IRT(Cls,         1ULL << 40)                                          \
  IRT(Func,        1ULL << 41)                                          \
  IRT(VarEnv,      1ULL << 42)                                          \
  IRT(NamedEntity, 1ULL << 43)                                          \
  IRT(FuncCls,     1ULL << 44) /* {Func*, Cctx} */                      \
  IRT(FuncObj,     1ULL << 45) /* {Func*, Obj} */                       \
  IRT(Cctx,        1ULL << 46) /* Class* with the lowest bit set,  */   \
                               /* as stored in ActRec.m_cls field  */   \
  IRT(RetAddr,     1ULL << 47) /* Return address */                     \
  IRT(StkPtr,      1ULL << 48) /* any pointer into VM stack: VmSP or VmFP*/ \
  IRT(TCA,         1ULL << 49)                                          \
  IRT(ActRec,      1ULL << 50)                                          \
  IRT(None,        1ULL << 51)

// The definitions for these are in ir.cpp
#define IRT_UNIONS                                                      \
  IRT(Ctx,         kObj|kCctx)                                          \
  IRT(FuncCtx,     kFuncCls|kFuncObj)

// Gen, Counted, PtrToGen, and PtrToCounted are here instead of
// IRT_PHP_UNIONS because boxing them (e.g., BoxedGen, PtrToBoxedGen)
// would be nonsense types.
#define IRT_SPECIAL                                                 \
  IRT(Bottom,       0)                                              \
  IRT(Counted,      kCountedStr|kCountedArr|kObj|kBoxedCell)        \
  IRT(PtrToCounted, kCounted << kPtrShift)                          \
  IRT(Gen,          kCell|kBoxedCell)                               \
  IRT(Init,         kGen & ~kUninit)                                \
  IRT(PtrToGen,     kGen << kPtrShift)                              \
  IRT(PtrToInit,    kInit << kPtrShift)

// All types (including union types) that represent program values,
// except Gen (which is special). Boxed*, PtrTo*, and PtrToBoxed* only
// exist for these types.
#define IRT_USERLAND(c) IRT_PHP(c) IRT_PHP_UNIONS(c)

// All types with just a single bit set
#define IRT_PRIMITIVE IRT_PHP(IRT_BOXES) IRT_RUNTIME

// All types
#define IR_TYPES IRT_USERLAND(IRT_BOXES) IRT_RUNTIME IRT_UNIONS IRT_SPECIAL

class Type {
  typedef uint64_t bits_t;

  static const size_t kBoxShift = 10;
  static const size_t kPtrShift = kBoxShift * 2;
  static const size_t kPtrBoxShift = kBoxShift + kPtrShift;

  enum TypeBits {
#define IRT(name, bits) k##name = (bits),
  IR_TYPES
#undef IRT
  };

  union {
    bits_t m_bits;
    TypeBits m_typedBits;
  };

public:
# define IRT(name, ...) static const Type name;
  IR_TYPES
# undef IRT

  explicit Type(bits_t bits = kNone)
    : m_bits(bits)
  {}

  size_t hash() const {
    return hash_int64(m_bits);
  }

  bool operator==(Type other) const {
    return m_bits == other.m_bits;
  }

  bool operator!=(Type other) const {
    return !operator==(other);
  }

  Type operator|(Type other) const {
    return Type(m_bits | other.m_bits);
  }

  Type operator&(Type other) const {
    return Type(m_bits & other.m_bits);
  }

  Type operator-(Type other) const {
    return Type(m_bits & ~other.m_bits);
  }

  std::string toString() const;
  static std::string debugString(Type t);
  static Type fromString(const std::string& str);

  bool isBoxed() const {
    return subtypeOf(BoxedCell);
  }

  bool notBoxed() const {
    return subtypeOf(Cell);
  }

  bool maybeBoxed() const {
    return (*this & BoxedCell) != Bottom;
  }

  bool isPtr() const {
    return subtypeOf(PtrToGen);
  }

  bool notPtr() const {
    return (*this & PtrToGen) == Bottom;
  }

  bool isCounted() const {
    return subtypeOf(Counted);
  }

  bool maybeCounted() const {
    return (*this & Counted) != Bottom;
  }

  bool notCounted() const {
    return !maybeCounted();
  }

  /*
   * Returns true iff this is a union type. Note that this is the
   * plain old set definition of union, so Type::Str, Type::Arr, and
   * Type::Null will all return true.
   */
  bool isUnion() const {
    // This will return true iff more than 1 bit is set in
    // m_bits.
    return (m_bits & (m_bits - 1)) != 0;
  }

  /*
   * Returns true if this value has a known constant DataType enum
   * value, which allows us to avoid several checks.
   */
  bool isKnownDataType() const {
    // Calling this function with a type that can't be in a TypedValue isn't
    // meaningful
    assert(subtypeOf(Gen | Cls));
    // Str, Arr and Null are technically unions but can each be
    // represented by one data type. Same for a union that consists of
    // nothing but boxed types.
    if (isString() || isArray() || isNull() || isBoxed()) {
      return true;
    }

    return !isUnion();
  }

  /*
   * Similar to isKnownDataType, with the added restriction that the
   * type not be Boxed.
   */
  bool isKnownUnboxedDataType() const {
    return isKnownDataType() && notBoxed();
  }

  /*
   * Returns true if this requires a register to hold a DataType at
   * runtime.
   */
  bool needsReg() const {
    return subtypeOf(Gen) && !isKnownDataType();
  }

  bool needsStaticBitCheck() const {
    return (*this & (StaticStr | StaticArr)) != Bottom;
  }

  // returns true if definitely not uninitialized
  bool isInit() const {
    return !Uninit.subtypeOf(*this);
  }

  bool maybeUninit() const {
    return !isInit();
  }

  /*
   * Returns true if this is a strict subtype of t2.
   */
  bool strictSubtypeOf(Type t2) const {
    return *this != t2 && subtypeOf(t2);
  }

  /*
   * Returns true if this is a non-strict subtype of any of the arguments.
   */
  bool subtypeOf(Type t2) const {
    return (m_bits & t2.m_bits) == m_bits;
  }

  /*
   * Returns true if this is a non-strict subtype of any of the arguments.
   */
  template<typename... Types>
  bool subtypeOfAny(Type t2, Types... ts) const {
    return subtypeOf(t2) || subtypeOfAny(ts...);
  }

  bool subtypeOfAny() const {
    return false;
  }

  /*
   * Returns true if any subtype of this is a subtype of t2.
   */
  bool maybe(Type t2) const {
    return (*this & t2) != Bottom;
  }

  /*
   * Returns true if no subtypes of this are subtypes of t2.
   */
  bool not(Type t2) const {
    return !maybe(t2);
  }

  /*
   * Returns true if this is exactly equal to t2. Be careful: you
   * probably mean subtypeOf.
   */
  bool equals(Type t2) const {
    return m_bits == t2.m_bits;
  }

  /*
   * Returns the most refined of two types.
   *
   * Pre: the types must not be completely unrelated.
   */
  static Type mostRefined(Type t1, Type t2) {
    assert(t1.subtypeOf(t2) || t2.subtypeOf(t1));
    return t1.subtypeOf(t2) ? t1 : t2;
  }

  static Type binArithResultType(Type t1, Type t2) {
    if (t1.subtypeOf(Type::Dbl) || t2.subtypeOf(Type::Dbl)) {
      return Type::Dbl;
    }
    return Type::Int;
  }

  bool isArray() const {
    return subtypeOf(Arr);
  }

  bool isBool() const {
    return subtypeOf(Bool);
  }

  bool isDbl() const {
    return subtypeOf(Dbl);
  }

  bool isInt() const {
    return subtypeOf(Int);
  }

  bool isNull() const {
    return subtypeOf(Null);
  }

  bool isObj() const {
    return subtypeOf(Obj);
  }

  bool isString() const {
    return subtypeOf(Str);
  }

  Type innerType() const {
    assert(isBoxed());
    return Type(m_bits >> kBoxShift);
  }

  /*
   * unionOf: return the least common predefined supertype of t1 and
   * t2, i.e.. the most refined type t3 such that t1 <: t3 and t2 <:
   * t3. Note that arbitrary union types are possible, but this
   * function always returns one of the predefined types.
   */
  static Type unionOf(Type t1, Type t2) {
    assert(t1 != None && t2 != None);
    if (t1 == t2) return t1;
    if (t1.subtypeOf(t2)) return t2;
    if (t2.subtypeOf(t1)) return t1;
    static const Type union_types[] = {
#   define IRT(name, ...) name,
      IRT_PHP_UNIONS(IRT_BOXES)
#   undef IRT
      Gen,
      PtrToGen,
    };
    Type t12 = t1 | t2;
    for (auto u : union_types) {
      if (t12.subtypeOf(u)) return u;
    }
    not_reached();
  }

  Type box() const {
    assert(subtypeOf(Cell));
    // Boxing Uninit returns InitNull but that logic doesn't belong
    // here.
    assert(not(Uninit) || equals(Cell));
    return Type(m_bits << kBoxShift);
  }

  // This computes the type effects of the Unbox opcode.
  Type unbox() const {
    assert(subtypeOf(Gen));
    return (*this & Cell) | (*this & BoxedCell).innerType();
  }

  Type deref() const {
    assert(isPtr());
    return Type(m_bits >> kPtrShift);
  }

  Type derefIfPtr() const {
    assert(subtypeOf(Gen | PtrToGen));
    return isPtr() ? deref() : *this;
  }

  // Returns the "stripped" version of this: dereferenced and unboxed,
  // if applicable.
  Type strip() const {
    return derefIfPtr().unbox();
  }

  Type ptr() const {
    assert(!isPtr());
    assert(subtypeOf(Gen));
    return Type(m_bits << kPtrShift);
  }

  bool canRunDtor() const {
    return
      (*this & (Obj | CountedArr | BoxedObj | BoxedCountedArr))
      != Type::Bottom;
  }

  // translates a compiler Type to an HPHP::DataType
  DataType toDataType() const {
    assert(!isPtr());
    if (isBoxed()) {
      return KindOfRef;
    }

    // Order is important here: types must progress from more specific
    // to less specific to return the most specific DataType.
    if (subtypeOf(None))          return KindOfInvalid;
    if (subtypeOf(Uninit))        return KindOfUninit;
    if (subtypeOf(Null))          return KindOfNull;
    if (subtypeOf(Bool))          return KindOfBoolean;
    if (subtypeOf(Int))           return KindOfInt64;
    if (subtypeOf(Dbl))           return KindOfDouble;
    if (subtypeOf(StaticStr))     return KindOfStaticString;
    if (subtypeOf(Str))           return KindOfString;
    if (subtypeOf(Arr))           return KindOfArray;
    if (subtypeOf(Obj))           return KindOfObject;
    if (subtypeOf(Cls))           return KindOfClass;
    if (subtypeOf(UncountedInit)) return KindOfUncountedInit;
    if (subtypeOf(Uncounted))     return KindOfUncounted;
    if (subtypeOf(Gen))           return KindOfAny;
    not_reached();
  }

  static Type fromDataType(DataType outerType,
                           DataType innerType = KindOfInvalid) {
    assert(innerType != KindOfRef);

    switch (outerType) {
      case KindOfInvalid       : return None;
      case KindOfUninit        : return Uninit;
      case KindOfNull          : return InitNull;
      case KindOfBoolean       : return Bool;
      case KindOfInt64         : return Int;
      case KindOfDouble        : return Dbl;
      case KindOfStaticString  : return StaticStr;
      case KindOfString        : return Str;
      case KindOfArray         : return Arr;
      case KindOfObject        : return Obj;
      case KindOfClass         : return Cls;
      case KindOfUncountedInit : return UncountedInit;
      case KindOfUncounted     : return Uncounted;
      case KindOfAny           : return Gen;
      case KindOfRef: {
        if (innerType == KindOfInvalid) {
          return BoxedCell;
        } else {
          return fromDataType(innerType).box();
        }
      }
      default                  : not_reached();
    }
  }

  // return true if this corresponds to a type that
  // is passed by value in C++
  bool isSimpleType() {
    return subtypeOf(Type::Bool)
           || subtypeOf(Type::Int)
           || subtypeOf(Type::Dbl)
           || subtypeOf(Type::Null);
  }

  // return true if this corresponds to a type that
  // is passed by reference in C++
  bool isReferenceType() {
    return subtypeOf(Type::Str)
           || subtypeOf(Type::Arr)
           || subtypeOf(Type::Obj);
  }

  // In tx64, KindOfUnknown is used to represent Variants (Type::Cell).
  // fromDataType() maps this to Type::None, which must be mapped
  // back to Type::Cell. This is not the best place to handle this.
  // See task #208726.
  static Type fromDataTypeWithCell(DataType type) {
    Type t = fromDataType(type);
    return t.equals(Type::None) ? Type::Cell : t;
  }

  static Type fromDataTypeWithRef(DataType outerType, bool isRef) {
    Type t = fromDataTypeWithCell(outerType);
    return isRef ? t.box() : t;
  }

  static Type fromRuntimeType(const Transl::RuntimeType& rtt) {
    return fromDataType(rtt.outerType(), rtt.innerType());
  }
}; // class Type

static_assert(sizeof(Type) <= sizeof(uint64_t),
              "JIT::Type should fit in a register");

/*
 * typeForConst(T)
 *
 *   returns the Type type for a C++ type that may be used with
 *   ConstData.
 */

// The only interesting case is int/bool disambiguation.
template<class T>
typename std::enable_if<
  std::is_integral<T>::value,
  Type
>::type typeForConst(T) {
  return std::is_same<T,bool>::value ? Type::Bool : Type::Int;
}

inline Type typeForConst(const StringData*)  { return Type::StaticStr; }
inline Type typeForConst(const NamedEntity*) { return Type::NamedEntity; }
inline Type typeForConst(const Func*)        { return Type::Func; }
inline Type typeForConst(const Class*)       { return Type::Cls; }
inline Type typeForConst(const TypedValue*)  { return Type::PtrToGen; }
inline Type typeForConst(TCA)                { return Type::TCA; }
inline Type typeForConst(double)             { return Type::Dbl; }
inline Type typeForConst(const ArrayData* ad) {
  assert(ad->isStatic());
  // TODO: Task #2124292, Reintroduce StaticArr
  return Type::Arr;
}

bool cmpOpTypesMayReenter(Opcode, Type t0, Type t1);

class RawMemSlot {
 public:

  enum Kind {
    ContLabel, ContDone, ContShouldThrow, ContRunning, ContARPtr,
    StrLen, FuncNumParams, FuncRefBitVec, FuncBody, MisBaseStrOff,
    MisCtx,
    MaxKind
  };

  static RawMemSlot& Get(Kind k) {
    switch (k) {
      case ContLabel:       return GetContLabel();
      case ContDone:        return GetContDone();
      case ContShouldThrow: return GetContShouldThrow();
      case ContRunning:     return GetContRunning();
      case ContARPtr:       return GetContARPtr();
      case StrLen:          return GetStrLen();
      case FuncNumParams:   return GetFuncNumParams();
      case FuncRefBitVec:   return GetFuncRefBitVec();
      case FuncBody:        return GetFuncBody();
      case MisBaseStrOff:   return GetMisBaseStrOff();
      case MisCtx:          return GetMisCtx();
      default: not_reached();
    }
  }

  int64_t getOffset()   const { return m_offset; }
  int32_t getSize()     const { return m_size; }
  Type getType() const { return m_type; }
  bool allowExtra()   const { return m_allowExtra; }

 private:
  RawMemSlot(int64_t offset, int32_t size, Type type, bool allowExtra = false)
    : m_offset(offset), m_size(size), m_type(type), m_allowExtra(allowExtra) { }

  static RawMemSlot& GetContLabel() {
    static RawMemSlot m(CONTOFF(m_label), sz::qword, Type::Int);
    return m;
  }
  static RawMemSlot& GetContDone() {
    static RawMemSlot m(CONTOFF(m_done), sz::byte, Type::Bool);
    return m;
  }
  static RawMemSlot& GetContShouldThrow() {
    static RawMemSlot m(CONTOFF(m_should_throw), sz::byte, Type::Bool);
    return m;
  }
  static RawMemSlot& GetContRunning() {
    static RawMemSlot m(CONTOFF(m_running), sz::byte, Type::Bool);
    return m;
  }
  static RawMemSlot& GetContARPtr() {
    static RawMemSlot m(CONTOFF(m_arPtr), sz::qword, Type::StkPtr);
    return m;
  }
  static RawMemSlot& GetStrLen() {
    static RawMemSlot m(StringData::sizeOffset(), sz::dword, Type::Int);
    return m;
  }
  static RawMemSlot& GetFuncNumParams() {
    static RawMemSlot m(Func::numParamsOff(), sz::qword, Type::Int);
    return m;
  }
  static RawMemSlot& GetFuncRefBitVec() {
    static RawMemSlot m(Func::refBitVecOff(), sz::qword, Type::Int);
    return m;
  }
  static RawMemSlot& GetFuncBody() {
    static RawMemSlot m(Func::funcBodyOff(), sz::qword, Type::TCA);
    return m;
  }
  static RawMemSlot& GetMisBaseStrOff() {
    static RawMemSlot m(HHIR_MISOFF(baseStrOff), sz::byte, Type::Bool);
    return m;
  }
  static RawMemSlot& GetMisCtx() {
    static RawMemSlot m(HHIR_MISOFF(ctx), sz::qword, Type::Cls);
    return m;
  }

  int64_t m_offset;
  int32_t m_size;
  Type m_type;
  bool m_allowExtra; // Used as a flag to ensure that extra offets are
                     // only used with RawMemSlots that support it
};

class SSATmp;
class Trace;
class CodeGenerator;
struct AsmInfo;
class IRFactory;
class Simplifier;
struct Block;

bool isRefCounted(SSATmp* opnd);

using folly::Range;
typedef Range<SSATmp**> SrcRange;
typedef Range<SSATmp*> DstRange;

/*
 * IRInstructions must be arena-allocatable.
 * (Destructors are not called when they come from IRFactory.)
 */
struct IRInstruction {
  enum IId { kTransient = 0xffffffff };

  /*
   * Create an IRInstruction for the opcode `op'.
   *
   * IRInstruction creation is usually done through IRFactory or
   * TraceBuilder rather than directly.
   */
  explicit IRInstruction(Opcode op,
                         uint32_t numSrcs = 0,
                         SSATmp** srcs = nullptr)
    : m_op(op)
    , m_typeParam(Type::None)
    , m_numSrcs(numSrcs)
    , m_numDsts(0)
    , m_iid(kTransient)
    , m_id(0)
    , m_srcs(srcs)
    , m_dst(nullptr)
    , m_taken(nullptr)
    , m_block(nullptr)
    , m_tca(nullptr)
    , m_extra(nullptr)
  {}

  IRInstruction(const IRInstruction&) = delete;
  IRInstruction& operator=(const IRInstruction&) = delete;

  /*
   * Construct an IRInstruction as a deep copy of `inst', using
   * arena to allocate memory for its srcs/dests.
   */
  explicit IRInstruction(Arena& arena, const IRInstruction* inst,
                         IId iid);

  /*
   * Initialize the source list for this IRInstruction.  We must not
   * have already had our sources initialized before this function is
   * called.
   *
   * Memory for `srcs' is owned outside of this class and must outlive
   * it.
   */
  void initializeSrcs(uint32_t numSrcs, SSATmp** srcs) {
    assert(!m_srcs && !m_numSrcs);
    m_numSrcs = numSrcs;
    m_srcs = srcs;
  }

  /*
   * Return access to extra-data on this instruction, for the
   * specified opcode type.
   *
   * Pre: getOpcode() == opc
   */
  template<Opcode opc>
  const typename IRExtraDataType<opc>::type* getExtra() const {
    assert(opc == getOpcode() && "getExtra type error");
    assert(m_extra != nullptr);
    return static_cast<typename IRExtraDataType<opc>::type*>(m_extra);
  }

  template<Opcode opc>
  typename IRExtraDataType<opc>::type* getExtra() {
    assert(opc == getOpcode() && "getExtra type error");
    return static_cast<typename IRExtraDataType<opc>::type*>(m_extra);
  }

  /*
   * Return access to extra-data of type T.  Requires that
   * IRExtraDataType<opc>::type is T for this instruction's opcode.
   *
   * It's normally preferable to use the version of this function that
   * takes the opcode instead of this one.  This is for writing code
   * that is supposed to be able to handle multiple opcode types that
   * share the same kind of extra data.
   */
  template<class T> const T* getExtra() const {
    auto opcode = getOpcode();
    if (debug) assert_opcode_extra<T>(opcode);
    return static_cast<const T*>(m_extra);
  }

  /*
   * Returns whether or not this opcode has an associated extra data
   * struct.
   */
  bool hasExtra() const;

  /*
   * Set the extra-data for this IRInstruction to the given pointer.
   * Lifetime is must outlast this IRInstruction (and any of its
   * clones).
   */
  void setExtra(IRExtraData* data) { assert(!m_extra); m_extra = data; }

  /*
   * Replace an instruction in place with a Nop.  This sometimes may
   * be a result of a simplification pass.
   */
  void convertToNop();

  /*
   * Replace a branch with a Jmp; used when we have proven the branch
   * is always taken.
   */
  void convertToJmp();

  /*
   * Replace an instruction in place with a Mov. Used when we have
   * proven that the instruction's side effects are not needed.
   */
  void convertToMov();

  /*
   * Deep-copy an IRInstruction, using factory to allocate memory for
   * the IRInstruction itself, and its srcs/dests.
   */
  IRInstruction* clone(IRFactory* factory) const;

  Opcode     getOpcode()   const       { return m_op; }
  void       setOpcode(Opcode newOpc)  { m_op = newOpc; }
  Type       getTypeParam() const      { return m_typeParam; }
  void       setTypeParam(Type t)      { m_typeParam = t; }
  uint32_t   getNumSrcs()  const       { return m_numSrcs; }
  void       setNumSrcs(uint32_t i)    {
    assert(i <= m_numSrcs);
    m_numSrcs = i;
  }
  SSATmp*    getSrc(uint32_t i) const;
  void       setSrc(uint32_t i, SSATmp* newSrc);
  void       appendSrc(Arena&, SSATmp*);
  SrcRange   getSrcs() const {
    return SrcRange(m_srcs, m_numSrcs);
  }
  unsigned   getNumDsts() const     { return m_numDsts; }
  SSATmp*    getDst() const         {
    assert(!naryDst());
    return m_dst;
  }
  void       setDst(SSATmp* newDst) {
    assert(hasDst());
    m_dst = newDst;
    m_numDsts = newDst ? 1 : 0;
  }
  /*
   * Returns the ith dest of this instruction. i == 0 is treated specially: if
   * the instruction has no dests, getDst(0) will return nullptr, and if the
   * instruction is not naryDest, getDst(0) will return the single dest.
   */
  SSATmp*    getDst(unsigned i) const;
  DstRange   getDsts();
  Range<const SSATmp*> getDsts() const;
  void       setDsts(unsigned numDsts, SSATmp* newDsts) {
    assert(naryDst());
    m_numDsts = numDsts;
    m_dst = newDsts;
  }

  TCA        getTCA()      const       { return m_tca; }
  void       setTCA(TCA    newTCA)     { m_tca = newTCA; }

  /*
   * An instruction's 'id' has different meanings depending on the
   * compilation phase.
   */
  uint32_t     getId()       const       { return m_id; }
  void       setId(uint32_t newId)       { m_id = newId; }

  /*
   * Instruction id (iid) is stable and useful as an array index.
   */
  uint32_t     getIId()      const       {
    assert(m_iid != kTransient);
    return m_iid;
  }

  /*
   * Returns true if the instruction is in a transient state.  That
   * is, it's allocated on the stack and we haven't yet committed to
   * inserting it in any blocks.
   */
  bool       isTransient() const       { return m_iid == kTransient; }

  // LiveRegs is the set of registers that are live across this instruction.
  // Doesn't include dest registers, or src registers whose lifetime ends here.
  RegSet     getLiveRegs() const       { return m_liveRegs; }
  void       setLiveRegs(RegSet s)     { m_liveRegs = s; }

  Block*     getBlock() const          { return m_block; }
  void       setBlock(Block* b)        { m_block = b; }
  Trace*     getTrace() const;
  void       setTaken(Block* b);
  Block*     getTaken() const          { return m_taken; }

  bool isControlFlowInstruction() const { return m_taken != nullptr; }
  bool isBlockEnd() const { return m_taken || isTerminal(); }

  bool equals(IRInstruction* inst) const;
  size_t hash() const;
  void print(std::ostream& ostream) const;
  void print() const;
  std::string toString() const;

  /*
   * Helper accessors for the OpcodeFlag bits for this instruction.
   *
   * Note that these wrappers have additional logic beyond just
   * checking the corresponding flags bit---you should generally use
   * these when you have an actual IRInstruction instead of just an
   * Opcode enum value.
   */
  bool canCSE() const;
  bool hasDst() const;
  bool naryDst() const;
  bool hasMemEffects() const;
  bool isRematerializable() const;
  bool isNative() const;
  bool consumesReferences() const;
  bool consumesReference(int srcNo) const;
  bool producesReference() const;
  bool mayModifyRefs() const;
  bool mayRaiseError() const;
  bool isEssential() const;
  bool isTerminal() const;
  bool isPassthrough() const;
  SSATmp* getPassthroughValue() const;
  bool killsSources() const;
  bool killsSource(int srcNo) const;

  bool modifiesStack() const;
  SSATmp* modifiedStkPtr() const;
  // hasMainDst provides raw access to the HasDest flag, for instructions with
  // ModifiesStack set.
  bool hasMainDst() const;

  void printDst(std::ostream& ostream) const;
  void printSrc(std::ostream& ostream, uint32_t srcIndex) const;
  void printOpcode(std::ostream& ostream) const;
  void printSrcs(std::ostream& ostream) const;

private:
  bool mayReenterHelper() const;

private:
  Opcode            m_op;
  Type              m_typeParam;
  uint16_t          m_numSrcs;
  uint16_t          m_numDsts;
  const IId         m_iid;
  uint32_t          m_id;
  SSATmp**          m_srcs;
  RegSet            m_liveRegs;
  SSATmp*           m_dst;     // if HasDest or NaryDest
  Block*            m_taken;   // for branches, guards, and jmp
  Block*            m_block;   // block that owns this instruction
  TCA               m_tca;
  IRExtraData*      m_extra;
public:
  boost::intrusive::list_member_hook<> m_listNode; // for InstructionList
};

typedef boost::intrusive::member_hook<IRInstruction,
                                      boost::intrusive::list_member_hook<>,
                                      &IRInstruction::m_listNode>
        IRInstructionHookOption;
typedef boost::intrusive::list<IRInstruction, IRInstructionHookOption>
        InstructionList;

/*
 * Return the output type from a given IRInstruction.
 *
 * The destination type is always predictable from the types of the inputs, any
 * type parameters to the instruction, and the id of the dest.
 */
Type outputType(const IRInstruction*, int dstId = 0);

/*
 * Assert that an instruction has operands of allowed types.
 */
void assertOperandTypes(const IRInstruction*);

struct SpillInfo {
  enum Type { Memory }; // Currently only one type of spill supported.

  explicit SpillInfo(uint32_t v)  : m_type(Memory), m_val(v) {}

  Type      type() const { return m_type; }

  // return offset in 8-byte-words from stack pointer
  uint32_t  mem()  const { assert(m_type == Memory); return m_val; }

private:
  Type     m_type : 1;
  uint32_t m_val : 31;
};

inline std::ostream& operator<<(std::ostream& os, SpillInfo si) {
  switch (si.type()) {
  case SpillInfo::Memory:
    os << "spill[" << si.mem() << "]";
    break;
  }
  return os;
}

class SSATmp {
public:
  uint32_t          getId() const { return m_id; }
  IRInstruction*    getInstruction() const { return m_inst; }
  void              setInstruction(IRInstruction* i) { m_inst = i; }
  Type              getType() const { return m_type; }
  void              setType(Type t) { m_type = t; }
  uint32_t          getLastUseId() const { return m_lastUseId; }
  void              setLastUseId(uint32_t newId) { m_lastUseId = newId; }
  uint32_t          getUseCount() const { return m_useCount; }
  void              setUseCount(uint32_t count) { m_useCount = count; }
  void              incUseCount() { m_useCount++; }
  uint32_t          decUseCount() { return --m_useCount; }
  bool              isBoxed() const { return getType().isBoxed(); }
  bool              isString() const { return isA(Type::Str); }
  bool              isArray() const { return isA(Type::Arr); }
  std::string       toString() const;
  void              print(std::ostream& ostream,
                          bool printLastUse = false) const;
  void              print() const;

  // XXX: false for Null, etc.  Would rather it returns whether we
  // have a compile-time constant value.
  bool isConst() const {
    return m_inst->getOpcode() == DefConst ||
      m_inst->getOpcode() == LdConst;
  }

  /*
   * For SSATmps with a compile-time constant value, the following
   * functions allow accessing it.
   *
   * Pre: getInstruction() &&
   *   (getInstruction()->getOpcode() == DefConst ||
   *    getInstruction()->getOpcode() == LdConst)
   */
  bool               getValBool() const;
  int64_t            getValInt() const;
  int64_t            getValRawInt() const;
  double             getValDbl() const;
  const StringData*  getValStr() const;
  const ArrayData*   getValArr() const;
  const Func*        getValFunc() const;
  const Class*       getValClass() const;
  const NamedEntity* getValNamedEntity() const;
  uintptr_t          getValBits() const;
  Variant            getValVariant() const;
  TCA                getValTCA() const;

  /*
   * Returns: Type::subtypeOf(getType(), tag).
   *
   * This should be used for most checks on the types of IRInstruction
   * sources.
   */
  bool isA(Type tag) const {
    return getType().subtypeOf(tag);
  }

  /*
   * Returns whether or not a given register index is allocated to a
   * register, or returns false if it is spilled.
   *
   * Right now, we only spill both at the same time and only Spill and
   * Reload instructions need to deal with SSATmps that are spilled.
   */
  bool hasReg(uint32_t i = 0) const {
    return !m_isSpilled && m_regs[i] != InvalidReg;
  }

  /*
   * The maximum number of registers this SSATmp may need allocated.
   * This is based on the type of the temporary (some types never have
   * regs, some have two, etc).
   */
  int               numNeededRegs() const;

  /*
   * The number of regs actually allocated to this SSATmp.  This might
   * end up fewer than numNeededRegs if the SSATmp isn't really
   * being used.
   */
  int               numAllocatedRegs() const;

  /*
   * Access to allocated registers.
   *
   * Returns InvalidReg for slots that aren't allocated.
   */
  PhysReg getReg() const { assert(!m_isSpilled); return m_regs[0]; }
  PhysReg getReg(uint32_t i) const { assert(!m_isSpilled); return m_regs[i]; }
  void    setReg(PhysReg reg, uint32_t i) { m_regs[i] = reg; }
  RegSet  getRegs() const;

  /*
   * Returns information about how to spill/fill a SSATmp.
   *
   * These functions are only valid if this SSATmp is being spilled or
   * filled.  In all normal instructions (i.e. other than Spill and
   * Reload), SSATmps are assigned registers instead of spill
   * locations.
   */
  void        setSpillInfo(int idx, SpillInfo si) { m_spillInfo[idx] = si;
                                                    m_isSpilled = true; }
  SpillInfo   getSpillInfo(int idx) const { assert(m_isSpilled);
                                            return m_spillInfo[idx]; }

  /*
   * During register allocation, this is used to track spill locations
   * that are assigned to specific SSATmps.  A value of -1 is used to
   * indicate no spill slot has been assigned.
   *
   * After register allocation, use getSpillInfo to access information
   * about where we've decided to spill/fill a given SSATmp from.
   * This value doesn't have any meaning outside of the linearscan
   * pass.
   */
  int32_t           getSpillSlot() const { return m_spillSlot; }
  void              setSpillSlot(int32_t val) { m_spillSlot = val; }

private:
  friend class IRFactory;
  friend class TraceBuilder;

  // May only be created via IRFactory.  Note that this class is never
  // destructed, so don't add complex members.
  SSATmp(uint32_t opndId, IRInstruction* i, int dstId = 0)
    : m_inst(i)
    , m_id(opndId)
    , m_type(outputType(i, dstId))
    , m_lastUseId(0)
    , m_useCount(0)
    , m_isSpilled(false)
    , m_spillSlot(-1)
  {
    m_regs[0] = m_regs[1] = InvalidReg;
  }
  SSATmp(const SSATmp&);
  SSATmp& operator=(const SSATmp&);

  IRInstruction*  m_inst;
  const uint32_t    m_id;
  Type            m_type; // type when defined
  uint32_t          m_lastUseId;
  uint16_t          m_useCount;
  bool            m_isSpilled : 1;
  int32_t         m_spillSlot : 31;

  /*
   * m_regs[0] is always the value of this SSATmp.
   *
   * Cell or Gen types use two registers: m_regs[1] is the runtime
   * type.
   */
  static const int kMaxNumRegs = 2;
  union {
    PhysReg m_regs[kMaxNumRegs];
    SpillInfo m_spillInfo[kMaxNumRegs];
  };
};

int vectorBaseIdx(Opcode opc);
int vectorKeyIdx(Opcode opc);
int vectorValIdx(Opcode opc);
inline int vectorBaseIdx(const IRInstruction* inst) {
  return vectorBaseIdx(inst->getOpcode());
}
inline int vectorKeyIdx(const IRInstruction* inst) {
  return vectorKeyIdx(inst->getOpcode());
}
inline int vectorValIdx(const IRInstruction* inst) {
  return vectorValIdx(inst->getOpcode());
}

struct VectorEffects {
  static bool supported(Opcode op);
  static bool supported(const IRInstruction* inst) {
    return supported(inst->getOpcode());
  }

  /*
   * VectorEffects::get is used to allow multiple different consumers to deal
   * with the side effects of vector instructions. It takes an instruction and
   * a series of callbacks, each of which will be called when the instruction
   * has a certain effect:
   *
   * storeLocValue: This will be called when a local's value is changed.
   * setLocType: This will be called when a local's type changes and the
   *             new value is not known.
   */
  typedef std::function<void(uint32_t, SSATmp*)> StoreLocFunc;
  typedef std::function<void(uint32_t, Type)> SetLocTypeFunc;
  static void get(const IRInstruction*,
                  StoreLocFunc storeLocValue,
                  SetLocTypeFunc setLocType);

  /*
   * Given a vector instruction that defines a new StkPtr, attempts to find the
   * stack value matching the given index. If the index does not match a value
   * modified by this instruction, false is returned and the 'value' parameter
   * is set to the next StkPtr in the chain. Otherwise, true is returned and
   * the 'value' and 'type' parameters are set appropriately.
   */
  static bool getStackValue(const IRInstruction* inst, uint32_t index,
                            SSATmp*& value, Type& type);

  VectorEffects(const IRInstruction* inst) {
    int keyIdx = vectorKeyIdx(inst);
    int valIdx = vectorValIdx(inst);
    init(inst->getOpcode(),
         inst->getSrc(vectorBaseIdx(inst))->getType(),
         keyIdx == -1 ? Type::None : inst->getSrc(keyIdx)->getType(),
         valIdx == -1 ? Type::None : inst->getSrc(valIdx)->getType());
  }
  template<typename Container>
  VectorEffects(Opcode opc, const Container& srcs) {
    int keyIdx = vectorKeyIdx(opc);
    int valIdx = vectorValIdx(opc);
    init(opc,
         srcs[vectorBaseIdx(opc)]->getType(),
         keyIdx == -1 ? Type::None : srcs[keyIdx]->getType(),
         valIdx == -1 ? Type::None : srcs[valIdx]->getType());
  }
  VectorEffects(Opcode op, Type base, Type key, Type val) {
    init(op, base, key, val);
  }
  VectorEffects(Opcode op, SSATmp* base, SSATmp* key, SSATmp* val) {
    auto typeOrNone =
      [](SSATmp* val){ return val ? val->getType() : Type::None; };
    init(op, typeOrNone(base), typeOrNone(key), typeOrNone(val));
  }
  Type baseType;
  Type valType;
  bool baseTypeChanged;
  bool baseValChanged;
  bool valTypeChanged;

private:
  void init(Opcode op, const Type base, const Type key, const Type val);
};

typedef folly::Range<TCA> TcaRange;

/**
 * A Block refers to a basic block: single-entry, single-exit, list of
 * instructions.  The instruction list is an intrusive list, so each
 * instruction can only be in one block at a time.  Likewise, a block
 * can only be owned by one trace at a time.
 *
 * Block owns the InstructionList, but exposes several list methods itself
 * so usually you can use Block directly.  These methods also update
 * IRInstruction::m_block transparently.
 */
struct Block : boost::noncopyable {
  typedef InstructionList::iterator iterator;
  typedef InstructionList::const_iterator const_iterator;

  // Execution frequency hint; codegen will put Unlikely blocks in astubs.
  enum Hint { Neither, Likely, Unlikely };

  Block(unsigned id, const Func* func, IRInstruction* label)
    : m_trace(nullptr), m_func(func), m_next(nullptr), m_id(id)
    , m_preds(nullptr), m_hint(Neither) {
    push_back(label);
  }

  IRInstruction* getLabel() const {
    assert(front()->getOpcode() == DefLabel);
    return front();
  }

  uint32_t    getId() const      { return m_id; }
  const Func* getFunc() const    { return m_func; }
  Trace*      getTrace() const   { return m_trace; }
  void        setTrace(Trace* t) { m_trace = t; }
  void        setHint(Hint hint) { m_hint = hint; }
  Hint        getHint() const    { return m_hint; }

  void        addEdge(IRInstruction* jmp);
  void        removeEdge(IRInstruction* jmp);

  // return the last instruction in the block
  IRInstruction* back() const {
    assert(!m_instrs.empty());
    auto it = m_instrs.end();
    return const_cast<IRInstruction*>(&*(--it));
  }

  // return the first instruction in the block.
  IRInstruction* front() const {
    assert(!m_instrs.empty());
    return const_cast<IRInstruction*>(&*m_instrs.begin());
  }

  // return the fallthrough block.  Should be nullptr if the last
  // instruction is a Terminal.
  Block* getNext() const { return m_next; }
  void setNext(Block* b) { m_next = b; }

  // return the target block if the last instruction is a branch.
  Block* getTaken() const {
    return back()->getTaken();
  }

  // return the postorder number of this block. (updated each time
  // sortBlocks() is called.
  unsigned postId() const { return m_postid; }
  void setPostId(unsigned id) { m_postid = id; }

  void printLabel(std::ostream& ostream) const;

  // insert inst after this block's label, return an iterator to the
  // newly inserted instruction.
  iterator prepend(IRInstruction* inst) {
    assert(front()->getOpcode() == DefLabel);
    auto it = begin();
    return insert(++it, inst);
  }

  // return iterator to first instruction after the label
  iterator skipLabel() { auto it = begin(); return ++it; }

  // return iterator to last instruction
  iterator backIter() { auto it = end(); return --it; }

  // return an iterator to a specific instruction
  iterator iteratorTo(IRInstruction* inst) {
    assert(inst->getBlock() == this);
    return m_instrs.iterator_to(*inst);
  }

  // visit each src that provides a value to label->dsts[i]
  template <class Body>
  void forEachSrc(unsigned i, Body body) {
    for (const EdgeData* n = m_preds; n; n = n->next) {
      assert(n->jmp->getOpcode() == Jmp_ && n->jmp->getTaken() == this);
      body(n->jmp, n->jmp->getSrc(i));
    }
  }

  // list-compatible interface; these delegate to m_instrs but also update
  // inst.m_block
  InstructionList& getInstrs()   { return m_instrs; }
  bool             empty()       { return m_instrs.empty(); }
  iterator         begin()       { return m_instrs.begin(); }
  iterator         end()         { return m_instrs.end(); }
  const_iterator   begin() const { return m_instrs.begin(); }
  const_iterator   end()   const { return m_instrs.end(); }

  iterator insert(iterator pos, IRInstruction* inst) {
    inst->setBlock(this);
    return m_instrs.insert(pos, *inst);
  }
  void splice(iterator pos, Block* from, iterator begin, iterator end) {
    assert(from != this);
    for (auto i = begin; i != end; ++i) (*i).setBlock(this);
    m_instrs.splice(pos, from->getInstrs(), begin, end);
  }
  void push_back(IRInstruction* inst) {
    inst->setBlock(this);
    return m_instrs.push_back(*inst);
  }
  template <class Predicate> void remove_if(Predicate p) {
    m_instrs.remove_if(p);
  }
  void erase(iterator pos) {
    m_instrs.erase(pos);
  }

 private:
  InstructionList m_instrs; // instructions in this block
  Trace* m_trace;           // owner of this block.
  const Func* m_func;       // which func are we in
  Block* m_next;            // fall-through path; null if back()->isTerminal().
  const unsigned m_id;      // factory-assigned unique id of this block
  unsigned m_postid;        // postorder number of this block
  EdgeData* m_preds;        // head of list of predecessor Jmps
  Hint m_hint;              // execution frequency hint
};
typedef std::list<Block*> BlockList;

inline Trace* IRInstruction::getTrace() const {
  return m_block->getTrace();
}

/*
 * A Trace is a single-entry, multi-exit, sequence of blocks.  Typically
 * each block falls through to the next block but this is not guaranteed;
 * traces may contain internal forward-only control flow.
 */
class Trace : boost::noncopyable {
public:
  explicit Trace(Block* first, uint32_t bcOff)
    : m_bcOff(bcOff)
    , m_main(nullptr)
  {
    push_back(first);
  }

  ~Trace() {
    std::for_each(m_exitTraces.begin(), m_exitTraces.end(),
                  boost::checked_deleter<Trace>());
  }

  std::list<Block*>& getBlocks() { return m_blocks; }
  Block* front() { return *m_blocks.begin(); }
  Block* back() { auto it = m_blocks.end(); return *(--it); }
  const Block* front() const { return *m_blocks.begin(); }
  const Block* back()  const { auto it = m_blocks.end(); return *(--it); }

  Block* push_back(Block* b) {
    b->setTrace(this);
    m_blocks.push_back(b);
    return b;
  }

  const Func* getFunc() const {
    return front()->getFunc();
  }

  const Unit* getUnit() const {
    return getFunc()->unit();
  }

  uint32_t getBcOff() const { return m_bcOff; }
  Trace* addExitTrace(Trace* exit) {
    m_exitTraces.push_back(exit);
    exit->setMain(this);
    return exit;
  }
  bool isMain() const { return m_main == nullptr; }
  void setMain(Trace* t) {
    assert(m_main == nullptr);
    m_main = t;
  }
  Trace* getMain() {
    return m_main;
  }

  typedef std::list<Trace*> ExitList;
  typedef std::list<Trace*>::iterator ExitIterator;

  ExitList& getExitTraces() { return m_exitTraces; }
  std::string toString() const;
  void print(std::ostream& ostream, const AsmInfo* asmInfo = nullptr) const;
  void print() const;

private:
  // offset of the first bytecode in this trace; 0 if this trace doesn't
  // represent a bytecode boundary.
  uint32_t m_bcOff;
  std::list<Block*> m_blocks; // Blocks in main trace starting with entry block
  ExitList m_exitTraces;      // traces to which this trace exits
  Trace* m_main;              // ptr to parent trace if this is an exit trace
};

/*
 * Some utility micro-passes used from other major passes.
 */

/*
 * Remove any instruction if live[iid] == false
 */
void removeDeadInstructions(Trace* trace, const boost::dynamic_bitset<>& live);

/*
 * Compute the postorder number of each immediate dominator of each block,
 * using the postorder numbers assigned by sortCfg().
 */
typedef std::vector<int> IdomVector;
IdomVector findDominators(const BlockList& blocks);

/*
 * return true if b1 == b2 or if b1 dominates b2.
 */
bool dominates(const Block* b1, const Block* b2, const IdomVector& idoms);

/*
 * Compute a reverse postorder list of the basic blocks reachable from
 * the first block in trace.
 */
BlockList sortCfg(Trace*, const IRFactory&);

/*
 * Ensure valid SSA properties; each SSATmp must be defined exactly once,
 * only used in positions dominated by the definition.
 */
bool checkCfg(Trace*, const IRFactory&);

/*
 * Return true if trace has internal control flow (IE it has a branch
 * to itself somewhere.
 */
bool hasInternalFlow(Trace*);

/**
 * Run all optimization passes on this trace
 */
void optimizeTrace(Trace*, IRFactory* irFactory);

/*
 * Counts the number of cells a SpillStack will logically push.  (Not
 * including the number it pops.)  That is, for each SSATmp in the
 * spill sources, this totals up whether it is an ActRec or a cell.
 */
int32_t spillValueCells(IRInstruction* spillStack);

/*
 * When SpillStack takes an ActRec, it has this many extra
 * dependencies in the spill vector for the values in the ActRec.
 */
constexpr int kSpillStackActRecExtraArgs = 5;

inline bool isConvIntOrPtrToBool(IRInstruction* instr) {
  if (instr->getOpcode() != ConvToBool) {
    return false;
  }

  return instr->getSrc(0)->isA(
    Type::Int | Type::Func | Type::Cls | Type::FuncCls |
    Type::VarEnv | Type::TCA);
}

/*
 * Visit basic blocks in a preorder traversal over the dominator tree.
 * The state argument is passed by value (copied) as we move down the tree,
 * so each child in the tree gets the state after the parent was processed.
 * The body lambda should take State& (by reference) so it can modify it
 * as each block is processed.
 */
template <class State, class Body>
void forPreorderDoms(Block* block, std::forward_list<Block*> children[],
                     State state, Body body) {
  body(block, state);
  for (Block* child : children[block->postId()]) {
    forPreorderDoms(child, children, state, body);
  }
}

/*
 * Visit the main trace followed by exit traces.
 */
template <class Body>
void forEachTrace(Trace* main, Body body) {
  body(main);
  for (Trace* exit : main->getExitTraces()) {
    body(exit);
  }
}

/*
 * Visit the blocks in the main trace followed by exit trace blocks.
 */
template <class Body>
void forEachTraceBlock(Trace* main, Body body) {
  for (Block* block : main->getBlocks()) {
    body(block);
  }
  for (Trace* exit : main->getExitTraces()) {
    for (Block* block : exit->getBlocks()) {
      body(block);
    }
  }
}

/*
 * Visit the instructions in this trace, in block order.
 */
template <class BlockList, class Body>
void forEachInst(const BlockList& blocks, Body body) {
  for (Block* block : blocks) {
    for (IRInstruction& inst : *block) {
      body(&inst);
    }
  }
}

template <class Body>
void forEachInst(Trace* trace, Body body) {
  forEachInst(trace->getBlocks(), body);
}

/*
 * Visit each instruction in the main trace, then the exit traces
 */
template <class Body>
void forEachTraceInst(Trace* main, Body body) {
  forEachTrace(main, [=](Trace* t) {
    forEachInst(t, body);
  });
}

/* 
 * Some utilities related to dumping. Rather than file-by-file control, we control
 * most IR logging via the hhir trace module.
 */
static inline bool dumpIREnabled(int level = 1) {
  return HPHP::Trace::moduleEnabledRelease(HPHP::Trace::hhir, level);
}

void dumpTraceImpl(const Trace* trace, std::ostream& out,
                   const AsmInfo* asmInfo = nullptr);
void dumpTrace(int level, const Trace* trace, const char* caption,
               AsmInfo* ai = nullptr);

}}}

namespace std {
  template<> struct hash<HPHP::VM::JIT::Opcode> {
    size_t operator()(HPHP::VM::JIT::Opcode op) const { return op; }
  };
  template<> struct hash<HPHP::VM::JIT::Type> {
    size_t operator()(HPHP::VM::JIT::Type t) const { return t.hash(); }
  };
}

#endif
