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
#include <boost/algorithm/string.hpp>
#include <boost/intrusive/list.hpp>

#include "folly/Conv.h"
#include "folly/Format.h"
#include "folly/Range.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"
#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/vm/jit/physreg.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/runtime-type.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/class.h"

namespace HPHP {
// forward declaration
class StringData;
namespace JIT {

using HPHP::Transl::TCA;
using HPHP::Transl::RegSet;
using HPHP::Transl::PhysReg;
using HPHP::Transl::ConditionCode;

struct IRInstruction;
struct SSATmp;
struct Block;
struct IRTrace;

class FailedIRGen : public std::runtime_error {
 public:
  const char* const file;
  const int         line;
  const char* const func;

  FailedIRGen(const char* _file, int _line, const char* _func)
    : std::runtime_error(folly::format("FailedIRGen @ {}:{} in {}",
                                       _file, _line, _func).str())
    , file(_file)
    , line(_line)
    , func(_func)
  {}
};

class FailedCodeGen : public std::runtime_error {
 public:
  const char*    file;
  const int      line;
  const char*    func;
  const Offset   bcOff;
  const Func*    vmFunc;

  FailedCodeGen(const char* _file, int _line, const char* _func,
                uint32_t _bcOff, const Func* _vmFunc)
    : std::runtime_error(folly::format("FailedCodeGen @ {}:{} in {}. {}@{}",
                                       _file, _line, _func,
                                       _vmFunc->fullName()->data(), _bcOff)
                         .str())
    , file(_file)
    , line(_line)
    , func(_func)
    , bcOff(_bcOff)
    , vmFunc(_vmFunc)
  {}
};

#define SPUNT(instr) do {                           \
  throw FailedIRGen(__FILE__, __LINE__, instr);     \
} while(0)
#define PUNT(instr) SPUNT(#instr)

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
 *     DArith    single dst has a type based on arithmetic type rules
 *     DMulti    multiple dests. type and number depend on instruction
 *     DSetElem  single dst is a subset of CountedStr|Nullptr depending on
 *               sources
 *     DStk(x)   up to two dests. x should be another D* macro and indicates
 *               the type of the first dest, if any. the second (or first,
 *               depending on the presence of a primary destination), will be
 *               of type Type::StkPtr. implies ModifiesStack.
 *     DBuiltin  single dst for CallBuiltin. This can return complex data
 *               types such as (Type::Str | Type::Null)
 *     DSubtract(N,t) single dest has type of src N with t removed
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
 *     SSpills       SpillStack's variadic source list
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
O(CheckType,                    DParam, S(Gen,Nullptr),          E|CRc|PRc|P) \
O(AssertType,                   DParam, S(Gen,Nullptr,Cls),    C|E|CRc|PRc|P) \
O(CheckTypeMem,                     ND, S(PtrToGen),                       E) \
O(GuardLoc,                         ND, S(FramePtr),                       E) \
O(GuardStk,                  D(StkPtr), S(StkPtr),                         E) \
O(CheckLoc,                         ND, S(FramePtr),                       E) \
O(CheckStk,                  D(StkPtr), S(StkPtr),                         E) \
O(CastStk,                   D(StkPtr), S(StkPtr),                  Mem|N|Er) \
O(CoerceStk,                 D(StkPtr), S(StkPtr),                  Mem|N|Er) \
O(AssertStk,                 D(StkPtr), S(StkPtr),                         E) \
O(AssertStkVal,              D(StkPtr), S(StkPtr) S(Gen),                  E) \
O(CheckDefinedClsEq,                ND, NA,                                E) \
O(GuardRefs,                        ND, S(Func)                               \
                                          S(Int)                              \
                                          S(Int)                              \
                                          S(Int)                              \
                                          S(Int),                          E) \
O(AssertLoc,                        ND, S(FramePtr),                       E) \
O(OverrideLoc,                      ND, S(FramePtr),                       E) \
O(SmashLocals,                      ND, S(FramePtr),                       E) \
O(BeginCatch,                       ND, NA,                            E|Mem) \
O(EndCatch,                         ND, S(StkPtr),                     E|Mem) \
O(LdUnwinderValue,              DParam, NA,                              PRc) \
O(DeleteUnwinderException,          ND, NA,                          N|E|Mem) \
O(OpAdd,                        DArith, S(Int,Dbl) S(Int,Dbl),             C) \
O(OpSub,                        DArith, S(Int,Dbl) S(Int,Dbl),             C) \
O(OpMul,                        DArith, S(Int,Dbl) S(Int,Dbl),             C) \
O(OpDiv,                        DArith, S(Int,Dbl) S(Int,Dbl),             C) \
O(OpMod,                        D(Int), S(Int) S(Int),                     C) \
O(OpBitAnd,                     D(Int), S(Int) S(Int),                     C) \
O(OpBitOr,                      D(Int), S(Int) S(Int),                     C) \
O(OpBitXor,                     D(Int), S(Int) S(Int),                     C) \
O(OpBitNot,                     D(Int), S(Int),                            C) \
O(OpLogicXor,                  D(Bool), S(Bool) S(Bool),                   C) \
O(OpNot,                       D(Bool), S(Bool),                           C) \
                                                                              \
O(ConvBoolToArr,                D(Arr), S(Bool),                         C|N) \
O(ConvDblToArr,                 D(Arr), S(Dbl),                          C|N) \
O(ConvIntToArr,                 D(Arr), S(Int),                          C|N) \
O(ConvObjToArr,                 D(Arr), S(Obj),                      N|CRc|K) \
O(ConvStrToArr,                 D(Arr), S(Str),                        N|CRc) \
O(ConvCellToArr,                D(Arr), S(Cell),                     N|CRc|K) \
                                                                              \
O(ConvArrToBool,               D(Bool), S(Arr),                          C|N) \
O(ConvDblToBool,               D(Bool), S(Dbl),                            C) \
O(ConvIntToBool,               D(Bool), S(Int),                            C) \
O(ConvStrToBool,               D(Bool), S(Str),                            N) \
O(ConvCellToBool,              D(Bool), S(Cell),                           N) \
                                                                              \
O(ConvArrToDbl,                 D(Dbl), S(Arr),                          C|N) \
O(ConvBoolToDbl,                D(Dbl), S(Bool),                           C) \
O(ConvIntToDbl,                 D(Dbl), S(Int),                            C) \
O(ConvObjToDbl,                 D(Dbl), S(Obj),                       N|Er|K) \
O(ConvStrToDbl,                 D(Dbl), S(Str),                          N|K) \
O(ConvCellToDbl,                D(Dbl), S(Cell),                      N|Er|K) \
                                                                              \
O(ConvArrToInt,                 D(Int), S(Arr),                          C|N) \
O(ConvBoolToInt,                D(Int), S(Bool),                           C) \
O(ConvDblToInt,                 D(Int), S(Dbl),                          C|N) \
O(ConvObjToInt,                 D(Int), S(Obj),                       N|Er|K) \
O(ConvStrToInt,                 D(Int), S(Str),                            N) \
O(ConvCellToInt,                D(Int), S(Cell),                      N|Er|K) \
                                                                              \
O(ConvCellToObj,                D(Obj), S(Cell),                     N|CRc|K) \
                                                                              \
O(ConvBoolToStr,          D(StaticStr), S(Bool),                           C) \
O(ConvDblToStr,                 D(Str), S(Dbl),                            N) \
O(ConvIntToStr,                 D(Str), S(Int),                            N) \
O(ConvObjToStr,                 D(Str), S(Obj),                   N|Er|CRc|K) \
O(ConvCellToStr,                D(Str), S(Cell),                  N|Er|CRc|K) \
                                                                              \
O(ExtendsClass,                D(Bool), S(Cls) C(Cls),                     C) \
O(InstanceOf,                  D(Bool), S(Cls) S(Cls),                   C|N) \
O(InstanceOfIface,             D(Bool), S(Cls) CStr,                     C|N) \
O(IsTypeMem,                   D(Bool), S(PtrToGen),                      NA) \
O(IsNTypeMem,                  D(Bool), S(PtrToGen),                      NA) \
/*    name                      dstinfo srcinfo                      flags */ \
O(OpGt,                        D(Bool), S(Gen) S(Gen),                   C|N) \
O(OpGte,                       D(Bool), S(Gen) S(Gen),                   C|N) \
O(OpLt,                        D(Bool), S(Gen) S(Gen),                   C|N) \
O(OpLte,                       D(Bool), S(Gen) S(Gen),                   C|N) \
O(OpEq,                        D(Bool), S(Gen) S(Gen),                   C|N) \
O(OpNeq,                       D(Bool), S(Gen) S(Gen),                   C|N) \
O(OpSame,                      D(Bool), S(Gen) S(Gen),                   C|N) \
O(OpNSame,                     D(Bool), S(Gen) S(Gen),                   C|N) \
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
O(JmpInstanceOfBitmask,        D(None), S(Cls) CStr,                       E) \
O(JmpNInstanceOfBitmask,       D(None), S(Cls) CStr,                       E) \
O(JmpIsType,                   D(None), SUnk,                              E) \
O(JmpIsNType,                  D(None), SUnk,                              E) \
/*    name                      dstinfo srcinfo                      flags */ \
O(JmpZero,                     D(None), SNum,                              E) \
O(JmpNZero,                    D(None), SNum,                              E) \
O(Jmp_,                        D(None), SUnk,                            T|E) \
O(ReqBindJmpGt,                     ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpGte,                    ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpLt,                     ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpLte,                    ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpEq,                     ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpNeq,                    ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpSame,                   ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpNSame,                  ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpInstanceOfBitmask,      ND, S(Cls) CStr,                     T|E) \
O(ReqBindJmpNInstanceOfBitmask,     ND, S(Cls) CStr,                     T|E) \
O(ReqBindJmpZero,                   ND, SNum,                            T|E) \
O(ReqBindJmpNZero,                  ND, SNum,                            T|E) \
O(SideExitGuardLoc,                 ND, S(FramePtr),                       E) \
O(SideExitGuardStk,          D(StkPtr), S(StkPtr),                         E) \
/*    name                      dstinfo srcinfo                      flags */ \
O(JmpIndirect,                      ND, S(TCA),                          T|E) \
O(ExitWhenSurprised,                ND, NA,                                E) \
O(ExitOnVarEnv,                     ND, S(FramePtr),                       E) \
O(ReleaseVVOrExit,                  ND, S(FramePtr),                     N|E) \
O(RaiseError,                       ND, S(Str),            E|N|Mem|Refs|T|Er) \
O(RaiseWarning,                     ND, S(Str),              E|N|Mem|Refs|Er) \
O(CheckInit,                        ND, S(Gen),                           NF) \
O(CheckInitMem,                     ND, S(PtrToGen) C(Int),               NF) \
O(AssertNonNull, DSubtract(0, Nullptr), S(Nullptr,CountedStr),            NF) \
O(Unbox,                     DUnbox(0), S(Gen),                           NF) \
O(Box,                         DBox(0), S(Init),             E|N|Mem|CRc|PRc) \
O(UnboxPtr,               D(PtrToCell), S(PtrToGen),                      NF) \
O(BoxPtr,            D(PtrToBoxedCell), S(PtrToGen),                   N|Mem) \
O(LdStack,                      DParam, S(StkPtr),                        NF) \
O(LdLoc,                        DParam, S(FramePtr),                      NF) \
O(LdStackAddr,                  DParam, S(StkPtr),                         C) \
O(LdLocAddr,                    DParam, S(FramePtr),                       C) \
O(LdMem,                        DParam, S(PtrToGen) C(Int),               NF) \
O(LdProp,                       DParam, S(Obj) C(Int),                    NF) \
O(LdRef,                        DParam, S(BoxedCell),                     NF) \
O(LdThis,                       D(Obj), S(FramePtr),                       C) \
O(LdRetAddr,                D(RetAddr), S(FramePtr),                      NF) \
O(LdConst,                      DParam, NA,                                C) \
O(DefConst,                     DParam, NA,                                C) \
O(LdCtx,                        D(Ctx), S(FramePtr) S(Func),               C) \
O(LdCctx,                      D(Cctx), S(FramePtr),                       C) \
O(LdCls,                        D(Cls), S(Str) C(Cls),     C|E|N|Refs|Er|Mem) \
O(LdClsCached,                  D(Cls), CStr,              C|E|N|Refs|Er|Mem) \
O(LdClsCachedSafe,              D(Cls), CStr,                              C) \
O(LdClsCtx,                     D(Cls), S(Ctx),                            C) \
O(LdClsCctx,                    D(Cls), S(Cctx),                           C) \
O(LdClsCns,                     DParam, NA,                               NF) \
O(LookupClsCns,                 DParam, NA,                  E|Refs|Er|N|Mem) \
O(LdCns,                        DParam, CStr,                             NF) \
O(LookupCns,                    DParam, CStr,                E|Refs|Er|N|Mem) \
O(LdClsMethodCache,         D(FuncCls), C(Str)                                \
                                          C(Str)                              \
                                          C(NamedEntity)                      \
                                          S(FramePtr)                         \
                                          S(StkPtr),       N|C|E|Refs|Er|Mem) \
O(LdClsMethodFCache,        D(FuncCtx), C(Cls)                                \
                                          CStr                                \
                                          S(Obj,Cls,Ctx)                      \
                                          S(FramePtr),              N|C|E|Er) \
O(GetCtxFwdCall,                D(Ctx), S(Ctx) S(Func),                    C) \
O(LdClsMethod,                 D(Func), S(Cls) C(Int),                     C) \
O(LdPropAddr,              D(PtrToGen), S(Obj) C(Int),                     C) \
O(LdClsPropAddr,           D(PtrToGen), S(Cls) S(Str) C(Cls),       C|E|N|Er) \
O(LdClsPropAddrCached,     D(PtrToGen), S(Cls) CStr CStr C(Cls),    C|E|N|Er) \
O(LdObjMethod,                      ND, S(Cls) CStr S(StkPtr),   E|N|Refs|Er) \
O(LdGblAddrDef,            D(PtrToGen), S(Str),                      E|N|CRc) \
O(LdGblAddr,               D(PtrToGen), S(Str),                            N) \
O(LdObjClass,                   D(Cls), S(Obj),                            C) \
O(LdFunc,                      D(Func), S(Str),                   E|N|CRc|Er) \
O(LdFuncCached,                D(Func), CStr,                       N|C|E|Er) \
O(LdFuncCachedSafe,            D(Func), CStr,                              C) \
O(LdARFuncPtr,                 D(Func), S(StkPtr,FramePtr) C(Int),         C) \
O(LdSSwitchDestFast,            D(TCA), S(Gen),                            N) \
O(LdSSwitchDestSlow,            D(TCA), S(Gen),                  E|N|Refs|Er) \
O(LdSwitchDblIndex,             D(Int), S(Dbl) S(Int) S(Int),              N) \
O(LdSwitchStrIndex,             D(Int), S(Str) S(Int) S(Int),          CRc|N) \
O(LdSwitchObjIndex,             D(Int), S(Obj) S(Int) S(Int),       CRc|N|Er) \
O(JmpSwitchDest,                    ND, S(Int),                          T|E) \
O(AllocObj,                     D(Obj), S(Cls),                            N) \
O(AllocObjFast,                 D(Obj), C(Cls),                            N) \
O(LdClsCtor,                   D(Func), S(Cls),                       C|Er|N) \
O(CreateCl,                     D(Obj), C(Cls)                                \
                                          C(Int)                              \
                                          S(FramePtr)                         \
                                          S(StkPtr),                   Mem|N) \
O(NewArray,                     D(Arr), C(Int),                        N|PRc) \
O(NewTuple,                     D(Arr), C(Int) S(StkPtr),    E|Mem|N|PRc|CRc) \
O(LdRaw,                        DParam, SUnk,                             NF) \
O(FreeActRec,                D(FramePtr), S(FramePtr),                   Mem) \
/*    name                      dstinfo srcinfo                      flags */ \
O(Call,                      D(StkPtr), SUnk,                 E|Mem|CRc|Refs) \
O(CallArray,                 D(StkPtr), S(StkPtr),          E|Mem|N|CRc|Refs) \
O(CallBuiltin,                DBuiltin, SUnk,            E|Mem|Refs|Er|N|PRc) \
O(NativeImpl,                       ND, C(Func) S(FramePtr),    E|Mem|N|Refs) \
O(RetCtrl,                          ND, S(StkPtr)                             \
                                          S(FramePtr)                         \
                                          S(RetAddr),                T|E|Mem) \
O(StRetVal,                         ND, S(FramePtr) S(Gen),        E|Mem|CRc) \
O(RetAdjustStack,            D(StkPtr), S(FramePtr),                       E) \
O(StMem,                            ND, S(PtrToGen)                           \
                                          C(Int) S(Gen),      E|Mem|CRc|Refs) \
O(StMemNT,                          ND, S(PtrToGen)                           \
                                          C(Int) S(Gen),           E|Mem|CRc) \
O(StProp,                           ND, S(Obj) S(Int) S(Gen), E|Mem|CRc|Refs) \
O(StPropNT,                         ND, S(Obj) S(Int) S(Gen),      E|Mem|CRc) \
O(StLoc,                            ND, S(FramePtr) S(Gen),        E|Mem|CRc) \
O(StLocNT,                          ND, S(FramePtr) S(Gen),        E|Mem|CRc) \
O(StRef,                       DBox(1), S(BoxedCell) S(Cell), E|Mem|CRc|Refs) \
O(StRefNT,                     DBox(1), S(BoxedCell) S(Cell),      E|Mem|CRc) \
O(StRaw,                            ND, SUnk,                          E|Mem) \
O(LdStaticLocCached,      D(BoxedCell), C(CacheHandle),                   NF) \
O(StaticLocInit,          D(BoxedCell), CStr                                  \
                                          S(FramePtr)                         \
                                          S(Cell),               PRc|E|N|Mem) \
O(StaticLocInitCached,    D(BoxedCell), CStr                                  \
                                          S(FramePtr)                         \
                                          S(Cell)                             \
                                            C(CacheHandle),      PRc|E|N|Mem) \
O(SpillStack,                D(StkPtr), S(StkPtr) C(Int) SSpills,        CRc) \
O(SpillFrame,                D(StkPtr), S(StkPtr)                             \
                                          S(FramePtr)                         \
                                          S(Func,FuncCls,FuncCtx,Null)        \
                                          S(Ctx,Cls,InitNull),           CRc) \
O(CufIterSpillFrame,         D(StkPtr), S(StkPtr)                             \
                                          S(FramePtr),                    NF) \
O(ExceptionBarrier,          D(StkPtr), S(StkPtr),                         E) \
O(ReqBindJmp,                       ND, NA,                              T|E) \
O(ReqBindJmpNoIR,                   ND, NA,                              T|E) \
O(ReqRetranslateNoIR,               ND, NA,                              T|E) \
O(ReqRetranslate,                   ND, NA,                              T|E) \
O(SyncABIRegs,                      ND, S(FramePtr) S(StkPtr),             E) \
O(Mov,                         DofS(0), SUnk,                            C|P) \
O(LdAddr,                      DofS(0), SUnk,                              C) \
O(IncRef,                      DofS(0), S(Gen),                    Mem|PRc|P) \
O(DecRefLoc,                        ND, S(FramePtr),            N|E|Mem|Refs) \
O(DecRefStack,                      ND, S(StkPtr),              N|E|Mem|Refs) \
O(DecRefThis,                       ND, S(FramePtr),            N|E|Mem|Refs) \
O(GenericRetDecRefs,         D(StkPtr), S(FramePtr) C(Int),     E|N|Mem|Refs) \
O(DecRef,                           ND, S(Gen),           N|E|Mem|CRc|Refs|K) \
O(DecRefMem,                        ND, S(PtrToGen)                           \
                                          C(Int),           N|E|Mem|CRc|Refs) \
O(DecRefNZ,                         ND, S(Gen),                      Mem|CRc) \
O(DecRefNZOrBranch,                 ND, S(Gen),                      Mem|CRc) \
O(DefLabel,                     DMulti, NA,                                E) \
O(DefInlineFP,             D(FramePtr), S(StkPtr) S(StkPtr),              NF) \
O(InlineReturn,                     ND, S(FramePtr),                       E) \
O(DefFP,                   D(FramePtr), NA,                                E) \
O(DefSP,                     D(StkPtr), S(FramePtr),                       E) \
O(ReDefSP,                   D(StkPtr), S(FramePtr) S(StkPtr),            NF) \
O(StashGeneratorSP,          D(StkPtr), S(StkPtr),                        NF) \
O(ReDefGeneratorSP,          D(StkPtr), S(StkPtr),                        NF) \
O(VerifyParamCls,                   ND, S(Cls)                                \
                                          S(Cls)                              \
                                          C(Int)                              \
                                          C(Int),            E|N|Mem|Refs|Er) \
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
O(Concat,                       D(Str), S(Gen) S(Gen),    N|Mem|CRc|PRc|Refs) \
O(ArrayAdd,                     D(Arr), S(Arr) S(Arr),         N|Mem|CRc|PRc) \
O(AKExists,                    D(Bool), S(Cell) S(Cell),                 C|N) \
O(InterpOne,                 D(StkPtr), S(FramePtr) S(StkPtr),                \
                                                             E|N|Mem|Refs|Er) \
O(InterpOneCF,                      ND, S(FramePtr) S(StkPtr)                 \
                                          C(Int),          T|E|N|Mem|Refs|Er) \
O(Spill,                       DofS(0), SUnk,                            Mem) \
O(Reload,                      DofS(0), SUnk,                            Mem) \
O(CreateContFunc,               D(Obj), C(Func) C(Func),             E|N|PRc) \
O(CreateContMeth,               D(Obj), C(Func) C(Func) S(Ctx),      E|N|PRc) \
O(ContEnter,                        ND, S(FramePtr)                           \
                                          S(TCA) C(Int) S(FramePtr),   E|Mem) \
O(ContPreNext,                      ND, S(Obj),                        E|Mem) \
O(ContStartedCheck,                 ND, S(Obj),                            E) \
O(ContSetRunning,                   ND, S(Obj)                                \
                                          C(Bool),                     E|Mem) \
O(ContDone,                         ND, S(Obj),                        E|Mem) \
O(ContValid,                   D(Bool), S(Obj),                            E) \
O(ContIncKey,                       ND, S(Obj),                        E|Mem) \
O(ContUpdateIdx,                    ND, S(Obj)                                \
                                          S(Int),                      E|Mem) \
O(IterInit,                    D(Bool), S(Arr,Obj)                            \
                                          S(FramePtr)                         \
                                          C(Int)                              \
                                          C(Int),           E|N|Mem|Refs|CRc) \
O(IterInitK,                   D(Bool), S(Arr,Obj)                            \
                                          S(FramePtr)                         \
                                          C(Int)                              \
                                          C(Int)                              \
                                          C(Int),           E|N|Mem|Refs|CRc) \
O(IterNext,                    D(Bool), S(FramePtr)                           \
                                          C(Int) C(Int),        E|N|Mem|Refs) \
O(IterNextK,                   D(Bool), S(FramePtr)                           \
                                          C(Int) C(Int) C(Int), E|N|Mem|Refs) \
O(WIterInit,                   D(Bool), S(Arr,Obj)                            \
                                          S(FramePtr)                         \
                                          C(Int)                              \
                                          C(Int),           E|N|Mem|Refs|CRc) \
O(WIterInitK,                  D(Bool), S(Arr,Obj)                            \
                                          S(FramePtr)                         \
                                          C(Int)                              \
                                          C(Int)                              \
                                          C(Int),           E|N|Mem|Refs|CRc) \
O(WIterNext,                   D(Bool), S(FramePtr)                           \
                                          C(Int) C(Int),        E|N|Mem|Refs) \
O(WIterNextK,                  D(Bool), S(FramePtr)                           \
                                          C(Int) C(Int) C(Int), E|N|Mem|Refs) \
O(MIterInit,                   D(Bool), S(BoxedCell)                          \
                                          S(FramePtr)                         \
                                          C(Int)                              \
                                          C(Int),           E|N|Mem|Refs|CRc) \
O(MIterInitK,                  D(Bool), S(BoxedCell)                          \
                                          S(FramePtr)                         \
                                          C(Int)                              \
                                          C(Int)                              \
                                          C(Int),           E|N|Mem|Refs|CRc) \
O(MIterNext,                   D(Bool), S(FramePtr)                           \
                                          C(Int) C(Int),        E|N|Mem|Refs) \
O(MIterNextK,                  D(Bool), S(FramePtr)                           \
                                          C(Int) C(Int) C(Int), E|N|Mem|Refs) \
O(IterFree,                         ND, S(FramePtr),            E|N|Mem|Refs) \
O(MIterFree,                        ND, S(FramePtr),            E|N|Mem|Refs) \
O(DecodeCufIter,               D(Bool), S(Arr,Obj,Str)                        \
                                          S(FramePtr),          E|N|Mem|Refs) \
O(CIterFree,                        ND, S(FramePtr),            E|N|Mem|Refs) \
O(DefMIStateBase,         D(PtrToCell), NA,                               NF) \
O(BaseG,                   D(PtrToGen), C(TCA)                                \
                                          S(Str)                              \
                                          S(PtrToCell),      E|N|Mem|Refs|Er) \
O(PropX,                   D(PtrToGen), C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Cell)                             \
                                          S(PtrToCell),      E|N|Mem|Refs|Er) \
O_STK(PropDX,              D(PtrToGen), C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Cell)                             \
                                          S(PtrToCell),VProp|E|N|Mem|Refs|Er) \
O(CGetProp,                    D(Cell), C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Cell)                             \
                                          S(PtrToCell),      E|N|Mem|Refs|Er) \
O_STK(VGetProp,           D(BoxedCell), C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Cell)                             \
                                          S(PtrToCell),VProp|E|N|Mem|Refs|Er) \
O_STK(BindProp,                     ND, C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Cell)                             \
                                          S(BoxedCell)                        \
                                          S(PtrToCell),VProp|E|N|Mem|Refs|Er) \
O_STK(SetProp,                      ND, C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Cell)                             \
                                          S(Cell),     VProp|E|N|Mem|Refs|Er) \
O(UnsetProp,                        ND, C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Cell),           E|N|Mem|Refs|Er) \
O_STK(SetOpProp,               D(Cell), C(TCA)                                \
                                          S(Obj,PtrToGen)                     \
                                          S(Cell)                             \
                                          S(Cell)                             \
                                          S(PtrToCell)                        \
                                          C(Int),      VProp|E|N|Mem|Refs|Er) \
O_STK(IncDecProp,              D(Cell), C(TCA)                                \
                                          S(Obj,PtrToGen)                     \
                                          S(Cell)                             \
                                          S(PtrToCell)                        \
                                          C(Int),      VProp|E|N|Mem|Refs|Er) \
O(EmptyProp,                   D(Bool), C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Cell),           E|N|Mem|Refs|Er) \
O(IssetProp,                   D(Bool), C(TCA)                                \
                                          C(Cls)                              \
                                          S(Obj,PtrToGen)                     \
                                          S(Cell),           E|N|Mem|Refs|Er) \
O(ElemX,                   D(PtrToGen), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Cell)                             \
                                          S(PtrToCell),      E|N|Mem|Refs|Er) \
O_STK(ElemDX,              D(PtrToGen), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Cell)                             \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O_STK(ElemUX,              D(PtrToGen), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Cell)                             \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O(ArrayGet,                    D(Cell), C(TCA)                                \
                                          S(Arr)                              \
                                          S(Int,Str),     C|N|PRc|Refs|Mem|Er)\
O(VectorGet,                   D(Cell), C(TCA)                                \
                                          S(Obj)                              \
                                          S(Int),            E|N|Mem|Refs|Er) \
O(PairGet,                     D(Cell), C(TCA)                                \
                                          S(Obj)                              \
                                          S(Int),            E|N|Mem|Refs|Er) \
O(MapGet,                      D(Cell), C(TCA)                                \
                                          S(Obj)                              \
                                          S(Int,Str),        E|N|Mem|Refs|Er) \
O(StableMapGet,                D(Cell), C(TCA)                                \
                                          S(Obj)                              \
                                          S(Int,Str),        E|N|Mem|Refs|Er) \
O(CGetElem,                    D(Cell), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Cell)                             \
                                          S(PtrToCell),      E|N|Mem|Refs|Er) \
O_STK(VGetElem,           D(BoxedCell), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Cell)                             \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O_STK(BindElem,                     ND, C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Cell)                             \
                                          S(BoxedCell)                        \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O(ArraySet,                     D(Arr), C(TCA)                                \
                                          S(Arr)                              \
                                          S(Int,Str)                          \
                                          S(Cell),    E|N|PRc|CRc|Refs|Mem|K) \
O(VectorSet,                        ND, C(TCA)                                \
                                          S(Obj)                              \
                                          S(Int)                              \
                                          S(Cell),           E|N|Mem|Refs|Er) \
O(MapSet,                           ND, C(TCA)                                \
                                          S(Obj)                              \
                                          S(Int,Str)                          \
                                          S(Cell),           E|N|Mem|Refs|Er) \
O(StableMapSet,                     ND, C(TCA)                                \
                                          S(Obj)                              \
                                          S(Int,Str)                          \
                                          S(Cell),           E|N|Mem|Refs|Er) \
O(ArraySetRef,                      ND, C(TCA)                                \
                                          S(Arr)                              \
                                          S(Int,Str)                          \
                                          S(Cell)                             \
                                          S(BoxedArr),E|N|PRc|CRc|Refs|Mem|K) \
O_STK(SetElem,                DSetElem, C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Cell)                             \
                                          S(Cell),     VElem|E|N|Mem|Refs|Er) \
O_STK(SetWithRefElem,               ND, C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Cell)                             \
                                          S(PtrToGen)                         \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O_STK(UnsetElem,                    ND, C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Cell),     VElem|E|N|Mem|Refs|Er) \
O_STK(SetOpElem,               D(Cell), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Cell)                             \
                                          S(Cell)                             \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O_STK(IncDecElem,              D(Cell), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Cell)                             \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O_STK(SetNewElem,                   ND, S(PtrToGen)                           \
                                          S(Cell),     VElem|E|N|Mem|Refs|Er) \
O_STK(SetWithRefNewElem,            ND, C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(PtrToGen)                         \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O_STK(BindNewElem,                  ND, S(PtrToGen)                           \
                                          S(BoxedCell)                        \
                                          S(PtrToCell),VElem|E|N|Mem|Refs|Er) \
O(ArrayIsset,                  D(Bool), C(TCA)                                \
                                          S(Arr)                              \
                                          S(Int,Str),        E|N|Mem|Refs|Er) \
O(VectorIsset,                 D(Bool), C(TCA)                                \
                                          S(Obj)                              \
                                          S(Int),            E|N|Mem|Refs)    \
O(PairIsset,                   D(Bool), C(TCA)                                \
                                          S(Obj)                              \
                                          S(Int),            E|N|Mem|Refs)    \
O(MapIsset,                    D(Bool), C(TCA)                                \
                                          S(Obj)                              \
                                          S(Int,Str),        E|N|Mem|Refs)    \
O(StableMapIsset,              D(Bool), C(TCA)                                \
                                          S(Obj)                              \
                                          S(Int,Str),        E|N|Mem|Refs)    \
O(IssetElem,                   D(Bool), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Cell)                             \
                                          S(PtrToCell),      E|N|Mem|Refs|Er) \
O(EmptyElem,                   D(Bool), C(TCA)                                \
                                          S(PtrToGen)                         \
                                          S(Cell)                             \
                                          S(PtrToCell),      E|N|Mem|Refs|Er) \
O(IncStat,                          ND, C(Int) C(Int) C(Bool),         E|Mem) \
O(IncStatGrouped,                   ND, CStr CStr C(Int),            E|N|Mem) \
O(IncTransCounter,                  ND, NA,                                E) \
O(ArrayIdx,                    D(Cell), C(TCA)                                \
                                          S(Arr)                              \
                                          S(Int,Str)                          \
                                          S(Cell),      E|N|CRc|PRc|Refs|Mem) \
O(DbgAssertRefCount,                ND, S(Counted,StaticStr,StaticArr),  N|E) \
O(DbgAssertPtr,                     ND, S(PtrToGen),                     N|E) \
O(DbgAssertType,                    ND, S(Cell),                           E) \
O(Nop,                              ND, NA,                               NF) \
/* */

enum Opcode : uint16_t {
#define O(name, dsts, srcs, flags) name,
  IR_OPCODES
#undef O

  IR_NUM_OPCODES
};

/*
 * A "query op" is any instruction returning Type::Bool that is both
 * branch-fusable and negateable.
 */
bool isQueryOp(Opcode opc);

/*
 * A "cmp ops" is query op that takes exactly two arguments of type
 * Gen.
 */
bool isCmpOp(Opcode opc);

/*
 * A "query jump op" is a conditional jump instruction that
 * corresponds to one of the query op instructions.
 */
bool isQueryJmpOp(Opcode opc);

/*
 * Translate a query op into a conditional jump that does the same
 * test (a "query jump op").
 *
 * Pre: isQueryOp(opc)
 */
Opcode queryToJmpOp(Opcode opc);

/*
 * Translate a "query jump op" to a query op.
 *
 * Pre: isQueryJmpOp(opc);
 */
Opcode queryJmpToQueryOp(Opcode opc);

/*
 * Convert a jump operation to its corresponding conditional
 * ReqBindJmp.
 *
 * Pre: opc is a conditional jump.
 */
Opcode jmpToReqBindJmp(Opcode opc);

/*
 * Return the opcode that corresponds to negation of opc.
 */
Opcode negateQueryOp(Opcode opc);

/*
 * Return the opcode that corresponds to commuting the arguments of
 * opc.
 */
Opcode commuteQueryOp(Opcode opc);

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
  // Unused
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

/*
 * typeForConst(T)
 *
 *   returns the Type type for a C++ type that may be used with
 *   ConstData.
 */

// The only interesting case is int/bool disambiguation.  Enums are
// treated as ints.
template<class T>
typename std::enable_if<
  std::is_integral<T>::value || std::is_enum<T>::value,
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
inline Type typeForConst(SetOpOp)            { return Type::Int; }
inline Type typeForConst(IncDecOp)           { return Type::Int; }
inline Type typeForConst(const ArrayData* ad) {
  assert(ad->isStatic());
  // TODO: Task #2124292, Reintroduce StaticArr
  return Type::Arr;
}

bool cmpOpTypesMayReenter(Opcode, Type t0, Type t1);

class RawMemSlot {
 public:

  enum Kind {
    ContLabel, ContIndex, ContARPtr,
    StrLen, FuncNumParams, ContEntry, MisCtx, MaxKind
  };

  static RawMemSlot& Get(Kind k) {
    switch (k) {
      case ContLabel:       return GetContLabel();
      case ContIndex:       return GetContIndex();
      case ContARPtr:       return GetContARPtr();
      case StrLen:          return GetStrLen();
      case FuncNumParams:   return GetFuncNumParams();
      case ContEntry:       return GetContEntry();
      case MisCtx:          return GetMisCtx();
      default: not_reached();
    }
  }

  int64_t offset() const { return m_offset; }
  int32_t size() const { return m_size; }
  Type type() const { return m_type; }
  bool allowExtra() const { return m_allowExtra; }

 private:
  RawMemSlot(int64_t offset, int32_t size, Type type, bool allowExtra = false)
    : m_offset(offset), m_size(size), m_type(type), m_allowExtra(allowExtra) { }

  static RawMemSlot& GetContLabel() {
    static RawMemSlot m(CONTOFF(m_label), Transl::sz::dword, Type::Int);
    return m;
  }
  static RawMemSlot& GetContIndex() {
    static RawMemSlot m(CONTOFF(m_index), Transl::sz::qword, Type::Int);
    return m;
  }
  static RawMemSlot& GetContARPtr() {
    static RawMemSlot m(CONTOFF(m_arPtr), Transl::sz::qword, Type::StkPtr);
    return m;
  }
  static RawMemSlot& GetStrLen() {
    static RawMemSlot m(StringData::sizeOffset(), Transl::sz::dword, Type::Int);
    return m;
  }
  static RawMemSlot& GetFuncNumParams() {
    static RawMemSlot m(Func::numParamsOff(), Transl::sz::dword, Type::Int);
    return m;
  }
  static RawMemSlot& GetContEntry() {
    static RawMemSlot m(
      Func::prologueTableOff() + sizeof(HPHP::Transl::TCA),
      Transl::sz::qword, Type::TCA);
    return m;
  }
  static RawMemSlot& GetMisCtx() {
    using namespace HPHP::Transl;
    static RawMemSlot m(HHIR_MISOFF(ctx), Transl::sz::qword, Type::Cls);
    return m;
  }

  int64_t m_offset;
  int32_t m_size;
  Type m_type;
  bool m_allowExtra; // Used as a flag to ensure that extra offets are
                     // only used with RawMemSlots that support it
};

class SSATmp;
class IRTrace;
class CodeGenerator;
struct AsmInfo;
class IRFactory;
class Simplifier;
struct LifetimeInfo;
struct RegAllocInfo;

bool isRefCounted(SSATmp* opnd);

using folly::Range;
typedef Range<SSATmp**> SrcRange;
typedef Range<SSATmp*> DstRange;

/*
 * Given an SSATmp of type Cls, try to find the name of the class.
 * Returns nullptr if can't find it.
 */
const StringData* findClassName(SSATmp* cls);

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


int vectorBaseIdx(Opcode opc);
int vectorKeyIdx(Opcode opc);
int vectorValIdx(Opcode opc);
int vectorBaseIdx(const IRInstruction* inst);
int vectorKeyIdx(const IRInstruction* inst);
int vectorValIdx(const IRInstruction* inst);

struct VectorEffects {
  static bool supported(Opcode op);
  static bool supported(const IRInstruction* inst);

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

  explicit VectorEffects(const IRInstruction* inst);
  VectorEffects(Opcode op, Type base, Type key, Type val);
  VectorEffects(Opcode op, SSATmp* base, SSATmp* key, SSATmp* val);
  VectorEffects(Opcode opc, const std::vector<SSATmp*>& srcs);

  Type baseType;
  bool baseTypeChanged;
  bool baseValChanged;

private:
  void init(const Opcode op, const Type base, const Type key, const Type val);
};

struct CatchInfo {
  /* afterCall is the address after the call instruction that this catch trace
   * belongs to. It's the key used to look up catch traces by the
   * unwinder, since it's the value of %rip during unwinding. */
  TCA afterCall;

  /* savedRegs contains the caller-saved registers that were pushed onto the
   * C++ stack at the time of the call. The catch trace will pop these
   * registers (in the same order as PhysRegSaver's destructor) before doing
   * any real work to restore the register state from before the call. */
  RegSet savedRegs;

  /* rspOffset is the number of bytes pushed on the C++ stack after the
   * registers in savedRegs were saved, typically from function calls with >6
   * arguments. The catch trace will adjust rsp by this amount before popping
   * anything in savedRegs. */
  Offset rspOffset;
};

typedef folly::Range<TCA> TcaRange;

/**
 * Run all optimization passes on this trace
 */
void optimizeTrace(IRTrace*, IRFactory* irFactory);

/*
 * Counts the number of cells a SpillStack will logically push.  (Not
 * including the number it pops.)  That is, for each SSATmp in the
 * spill sources, this totals up whether it is an ActRec or a cell.
 */
int32_t spillValueCells(IRInstruction* spillStack);

bool isConvIntOrPtrToBool(IRInstruction* instr);

}}

namespace std {
  template<> struct hash<HPHP::JIT::Opcode> {
    size_t operator()(HPHP::JIT::Opcode op) const { return op; }
  };
  template<> struct hash<HPHP::JIT::Type> {
    size_t operator()(HPHP::JIT::Type t) const { return t.hash(); }
  };
}

#endif
