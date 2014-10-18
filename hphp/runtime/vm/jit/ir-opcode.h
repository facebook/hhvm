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

#ifndef incl_HPHP_VM_IR_OPCODE_H_
#define incl_HPHP_VM_IR_OPCODE_H_

#include <type_traits>
#include <vector>

#include "folly/Range.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/base/types.h"

namespace HPHP {
//////////////////////////////////////////////////////////////////////
struct StringData;

namespace jit {
//////////////////////////////////////////////////////////////////////

struct IRUnit;
struct IRInstruction;
struct SSATmp;
struct LocalStateHook;

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
 *     ND          instruction has no destination
 *     D(type)     single dst has a specific type
 *     DofS(N)     single dst has the type of src N
 *     DUnbox(N)   single dst has unboxed type of src N
 *     DBox(N)     single dst has boxed type of src N
 *     DRefineS(N) single dst's type is the intersection of src N and paramType
 *     DParam      single dst has type of the instruction's type parameter
 *     DLdRef      single dst has type of the instruction's type parameter,
 *                 loosened to allow efficient type checks
 *     DAllocObj   single dst has a type of a newly allocated object; may be a
 *                   specialized object type if the class is known
 *     DArrPacked  single dst has a packed array type
 *     DArrElem    single dst has type based on reading an array element
 *     DThis       single dst has type Obj<ctx>, where ctx is the
 *                   current context class
 *     DMulti      multiple dests. type and number depend on instruction
 *     DSetElem    single dst is a subset of CountedStr|Nullptr depending on
 *                   sources
 *     DStk(x)     up to two dests. x should be another D* macro and indicates
 *                   the type of the first dest, if any. the second (or first,
 *                   depending on the presence of a primary destination), will
 *                   be of type Type::StkPtr. implies ModifiesStack.
 *     DBuiltin    single dst for CallBuiltin. This can return complex data
 *                   types such as (Type::Str | Type::Null)
 *     DSubtract(N,t) single dest has type of src N with t removed
 *     DLdRaw      single dst has type determined by RawMemData
 *     DCns        single dst's type is the union of legal types for PHP
 *                   constants
 *
 * srcinfo:
 *
 *   Contains a series of tests on the source parameters in order.
 *
 *     NA               instruction takes no sources
 *     S(t1,...,tn)     source must be a subtype of {t1|..|tn}
 *     AK(<kind>)       source must be an array with specified kind
 *     C(type)          source must be a constant, and subtype of type
 *     CStr             same as C(StaticStr)
 *     SVar(t1,...,tn)  variadic source list, all subtypes of {t1|..|tn}
 *
 * flags:
 *
 *   See doc/ir.specification for the meaning of these flags.
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
 *      Er    mayRaiseError
 *      PRc   producesRC
 *      CRc   consumesRC
 *      T     isTerminal
 *      B     isBranch
 *      P     passthrough
 *      K     killsSource
 *      MProp MInstrProp
 *      MElem MInstrElem
 */

#define O_STK(name, dst, src, flags)            \
  O(name, dst, src, StkFlags(flags))            \
  O(name ## Stk, DStk(dst), src, flags)

#define IR_OPCODES                                                            \
/*    name                      dstinfo srcinfo                      flags */ \
O(CheckType,               DRefineS(0), S(Gen),                        B|E|P) \
O(AssertType,              DRefineS(0), S(Gen,Cls),                    C|E|P) \
O(CheckTypeMem,                     ND, S(PtrToGen),                     B|E) \
O(GuardLoc,                         ND, S(FramePtr),                       E) \
O(GuardStk,                  D(StkPtr), S(StkPtr),                         E) \
O(CheckLoc,                         ND, S(FramePtr),                     B|E) \
O(CheckStk,                  D(StkPtr), S(StkPtr),                       B|E) \
O(EndGuards,                        ND, NA,                                E) \
O(CastStk,                   D(StkPtr), S(StkPtr),                        Er) \
O(CastStkIntToDbl,           D(StkPtr), S(StkPtr),                         E) \
O(CoerceStk,                 D(StkPtr), S(StkPtr),                        Er) \
O(AssertStk,                 D(StkPtr), S(StkPtr),                         E) \
O(ProfileStr,                       ND, S(PtrToStr),                       E) \
O(CheckDefinedClsEq,                ND, NA,                              B|E) \
O(GuardRefs,                        ND, S(Func)                               \
                                          S(Int)                              \
                                          C(Int)                              \
                                          S(Int)                              \
                                          S(Int),                          E) \
O(CheckRefs,                        ND, S(Func)                               \
                                          S(Int)                              \
                                          C(Int)                              \
                                          S(Int)                              \
                                          S(Int),                        B|E) \
O(AssertLoc,                        ND, S(FramePtr),                       E) \
O(TrackLoc,                         ND, S(Gen),                            E) \
O(BeginCatch,                       ND, NA,                                E) \
O(EndCatch,                         ND, S(FramePtr) S(StkPtr),           E|T) \
O(TryEndCatch,                      ND, S(FramePtr) S(StkPtr),             E) \
O(CheckSideExit,                    ND, S(FramePtr) S(StkPtr),           B|E) \
O(LdUnwinderValue,              DParam, NA,                              PRc) \
O(DeleteUnwinderException,          ND, NA,                               NF) \
O(AddInt,                       D(Int), S(Int) S(Int),                     C) \
O(SubInt,                       D(Int), S(Int) S(Int),                     C) \
O(MulInt,                       D(Int), S(Int) S(Int),                     C) \
O(AddDbl,                       D(Dbl), S(Dbl) S(Dbl),                     C) \
O(SubDbl,                       D(Dbl), S(Dbl) S(Dbl),                     C) \
O(MulDbl,                       D(Dbl), S(Dbl) S(Dbl),                     C) \
O(DivDbl,                       D(Dbl), S(Dbl) S(Dbl),                   B|C) \
O(Mod,                          D(Int), S(Int) S(Int),                     C) \
O(Sqrt,                         D(Dbl), S(Dbl),                            C) \
O(AbsDbl,                       D(Dbl), S(Dbl),                            C) \
O(AndInt,                       D(Int), S(Int) S(Int),                     C) \
O(OrInt,                        D(Int), S(Int) S(Int),                     C) \
O(XorInt,                       D(Int), S(Int) S(Int),                     C) \
O(XorBool,                     D(Bool), S(Bool) S(Bool),                   C) \
O(Shl,                          D(Int), S(Int) S(Int),                     C) \
O(Shr,                          D(Int), S(Int) S(Int),                     C) \
                                                                              \
O(AddIntO,                      D(Int), S(Int) S(Int),                   B|C) \
O(SubIntO,                      D(Int), S(Int) S(Int),                   B|C) \
O(MulIntO,                      D(Int), S(Int) S(Int),                   B|C) \
                                                                              \
O(CoerceCellToBool,            D(Bool), S(Cell),                          Er) \
O(CoerceCellToInt,              D(Int), S(Cell),                          Er) \
O(CoerceStrToInt,               D(Int), S(Str),                           Er) \
O(CoerceCellToDbl,              D(Dbl), S(Cell),                          Er) \
O(CoerceStrToDbl,               D(Dbl), S(Str),                           Er) \
                                                                              \
O(ConvBoolToArr,                D(Arr), S(Bool),                       C|PRc) \
O(ConvDblToArr,                 D(Arr), S(Dbl),                        C|PRc) \
O(ConvIntToArr,                 D(Arr), S(Int),                        C|PRc) \
O(ConvObjToArr,                 D(Arr), S(Obj),                 Er|PRc|CRc|K) \
O(ConvStrToArr,                 D(Arr), S(Str),                      PRc|CRc) \
O(ConvCellToArr,                D(Arr), S(Cell),                Er|PRc|CRc|K) \
                                                                              \
O(ConvArrToBool,               D(Bool), S(Arr),                           NF) \
O(ConvDblToBool,               D(Bool), S(Dbl),                            C) \
O(ConvIntToBool,               D(Bool), S(Int),                            C) \
O(ConvStrToBool,               D(Bool), S(Str),                           NF) \
O(ConvObjToBool,               D(Bool), S(Obj),                           NF) \
O(ConvCellToBool,              D(Bool), S(Cell),                          NF) \
                                                                              \
O(ConvArrToDbl,                 D(Dbl), S(Arr),                           NF) \
O(ConvBoolToDbl,                D(Dbl), S(Bool),                           C) \
O(ConvIntToDbl,                 D(Dbl), S(Int),                            C) \
O(ConvObjToDbl,                 D(Dbl), S(Obj),                           Er) \
O(ConvStrToDbl,                 D(Dbl), S(Str),                           NF) \
O(ConvCellToDbl,                D(Dbl), S(Cell),                          Er) \
                                                                              \
O(ConvArrToInt,                 D(Int), S(Arr),                           NF) \
O(ConvBoolToInt,                D(Int), S(Bool),                           C) \
O(ConvDblToInt,                 D(Int), S(Dbl),                            C) \
O(ConvObjToInt,                 D(Int), S(Obj),                         Er|K) \
O(ConvStrToInt,                 D(Int), S(Str),                           NF) \
O(ConvCellToInt,                D(Int), S(Cell),                        Er|K) \
                                                                              \
O(ConvCellToObj,                D(Obj), S(Cell),                Er|CRc|PRc|K) \
                                                                              \
O(ConvBoolToStr,          D(StaticStr), S(Bool),                           C) \
O(ConvDblToStr,                 D(Str), S(Dbl),                          PRc) \
O(ConvIntToStr,                 D(Str), S(Int),                          PRc) \
O(ConvObjToStr,                 D(Str), S(Obj),                       PRc|Er) \
O(ConvResToStr,                 D(Str), S(Res),                       PRc|Er) \
O(ConvCellToStr,                D(Str), S(Cell),                      PRc|Er) \
                                                                              \
O(ExtendsClass,                D(Bool), S(Cls) C(Cls),                     C) \
O(ClsNeq,                      D(Bool), S(Cls),                            C) \
O(IsWaitHandle,                D(Bool), S(Obj),                            C) \
O(OODeclExists,                D(Bool), S(Str) S(Bool),                 E|Er) \
O(InstanceOf,                  D(Bool), S(Cls) S(Cls),                     C) \
O(InstanceOfIface,             D(Bool), S(Cls) CStr,                       C) \
O(InterfaceSupportsArr,        D(Bool), S(Str),                            C) \
O(InterfaceSupportsStr,        D(Bool), S(Str),                            C) \
O(InterfaceSupportsInt,        D(Bool), S(Str),                            C) \
O(InterfaceSupportsDbl,        D(Bool), S(Str),                            C) \
O(IsTypeMem,                   D(Bool), S(PtrToGen),                      NF) \
O(IsNTypeMem,                  D(Bool), S(PtrToGen),                      NF) \
/*    name                      dstinfo srcinfo                      flags */ \
O(Gt,                          D(Bool), S(Gen) S(Gen),                     C) \
O(GtX,                         D(Bool), S(Gen) S(Gen),                  Er|C) \
O(Gte,                         D(Bool), S(Gen) S(Gen),                     C) \
O(GteX,                        D(Bool), S(Gen) S(Gen),                  Er|C) \
O(Lt,                          D(Bool), S(Gen) S(Gen),                     C) \
O(LtX,                         D(Bool), S(Gen) S(Gen),                  Er|C) \
O(Lte,                         D(Bool), S(Gen) S(Gen),                     C) \
O(LteX,                        D(Bool), S(Gen) S(Gen),                  Er|C) \
O(Eq,                          D(Bool), S(Gen) S(Gen),                     C) \
O(EqX,                         D(Bool), S(Gen) S(Gen),                  Er|C) \
O(Neq,                         D(Bool), S(Gen) S(Gen),                     C) \
O(NeqX,                        D(Bool), S(Gen) S(Gen),                  Er|C) \
O(Same,                        D(Bool), S(Gen) S(Gen),                     C) \
O(NSame,                       D(Bool), S(Gen) S(Gen),                     C) \
O(GtInt,                       D(Bool), S(Int) S(Int),                     C) \
O(GteInt,                      D(Bool), S(Int) S(Int),                     C) \
O(LtInt,                       D(Bool), S(Int) S(Int),                     C) \
O(LteInt,                      D(Bool), S(Int) S(Int),                     C) \
O(EqInt,                       D(Bool), S(Int) S(Int),                     C) \
O(NeqInt,                      D(Bool), S(Int) S(Int),                     C) \
O(GtDbl,                       D(Bool), S(Dbl) S(Dbl),                     C) \
O(GteDbl,                      D(Bool), S(Dbl) S(Dbl),                     C) \
O(LtDbl,                       D(Bool), S(Dbl) S(Dbl),                     C) \
O(LteDbl,                      D(Bool), S(Dbl) S(Dbl),                     C) \
O(EqDbl,                       D(Bool), S(Dbl) S(Dbl),                     C) \
O(NeqDbl,                      D(Bool), S(Dbl) S(Dbl),                     C) \
O(Floor,                        D(Dbl), S(Dbl),                            C) \
O(Ceil,                         D(Dbl), S(Dbl),                            C) \
O(InstanceOfBitmask,           D(Bool), S(Cls) CStr,                       C) \
O(NInstanceOfBitmask,          D(Bool), S(Cls) CStr,                       C) \
  /* there is a conditional branch for each of the above fusable query ops */ \
O(IsType,                      D(Bool), S(Cell),                           C) \
O(IsNType,                     D(Bool), S(Cell),                           C) \
O(IsScalarType,                D(Bool), S(Cell),                           C) \
O(JmpGt,                            ND, S(Gen) S(Gen),                   B|E) \
O(JmpGte,                           ND, S(Gen) S(Gen),                   B|E) \
O(JmpLt,                            ND, S(Gen) S(Gen),                   B|E) \
O(JmpLte,                           ND, S(Gen) S(Gen),                   B|E) \
O(JmpEq,                            ND, S(Gen) S(Gen),                   B|E) \
O(JmpNeq,                           ND, S(Gen) S(Gen),                   B|E) \
O(JmpSame,                          ND, S(Gen) S(Gen),                   B|E) \
O(JmpNSame,                         ND, S(Gen) S(Gen),                   B|E) \
O(JmpGtInt,                         ND, S(Int) S(Int),                   B|E) \
O(JmpGteInt,                        ND, S(Int) S(Int),                   B|E) \
O(JmpLtInt,                         ND, S(Int) S(Int),                   B|E) \
O(JmpLteInt,                        ND, S(Int) S(Int),                   B|E) \
O(JmpEqInt,                         ND, S(Int) S(Int),                   B|E) \
O(JmpNeqInt,                        ND, S(Int) S(Int),                   B|E) \
O(JmpInstanceOfBitmask,             ND, S(Cls) CStr,                     B|E) \
O(JmpNInstanceOfBitmask,            ND, S(Cls) CStr,                     B|E) \
/*    name                      dstinfo srcinfo                      flags */ \
O(JmpZero,                          ND, S(Int,Bool),                     B|E) \
O(JmpNZero,                         ND, S(Int,Bool),                     B|E) \
O(Jmp,                              ND, SVar(Top),                     B|T|E) \
O(ReqBindJmpGt,                     ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpGte,                    ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpLt,                     ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpLte,                    ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpEq,                     ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpNeq,                    ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpGtInt,                  ND, S(Int) S(Int),                   T|E) \
O(ReqBindJmpGteInt,                 ND, S(Int) S(Int),                   T|E) \
O(ReqBindJmpLtInt,                  ND, S(Int) S(Int),                   T|E) \
O(ReqBindJmpLteInt,                 ND, S(Int) S(Int),                   T|E) \
O(ReqBindJmpEqInt,                  ND, S(Int) S(Int),                   T|E) \
O(ReqBindJmpNeqInt,                 ND, S(Int) S(Int),                   T|E) \
O(ReqBindJmpSame,                   ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpNSame,                  ND, S(Gen) S(Gen),                   T|E) \
O(ReqBindJmpInstanceOfBitmask,      ND, S(Cls) CStr,                     T|E) \
O(ReqBindJmpNInstanceOfBitmask,     ND, S(Cls) CStr,                     T|E) \
O(ReqBindJmpZero,                   ND, S(Int,Bool),                     T|E) \
O(ReqBindJmpNZero,                  ND, S(Int,Bool),                     T|E) \
O(SideExitJmpGt,                    ND, S(Gen) S(Gen),                     E) \
O(SideExitJmpGte,                   ND, S(Gen) S(Gen),                     E) \
O(SideExitJmpLt,                    ND, S(Gen) S(Gen),                     E) \
O(SideExitJmpLte,                   ND, S(Gen) S(Gen),                     E) \
O(SideExitJmpEq,                    ND, S(Gen) S(Gen),                     E) \
O(SideExitJmpNeq,                   ND, S(Gen) S(Gen),                     E) \
O(SideExitJmpGtInt,                 ND, S(Int) S(Int),                     E) \
O(SideExitJmpGteInt,                ND, S(Int) S(Int),                     E) \
O(SideExitJmpLtInt,                 ND, S(Int) S(Int),                     E) \
O(SideExitJmpLteInt,                ND, S(Int) S(Int),                     E) \
O(SideExitJmpEqInt,                 ND, S(Int) S(Int),                     E) \
O(SideExitJmpNeqInt,                ND, S(Int) S(Int),                     E) \
O(SideExitJmpSame,                  ND, S(Int) S(Int),                     E) \
O(SideExitJmpNSame,                 ND, S(Int) S(Int),                     E) \
O(SideExitJmpInstanceOfBitmask,                                               \
                                    ND, S(Cls) CStr,                       E) \
O(SideExitJmpNInstanceOfBitmask,                                              \
                                    ND, S(Cls) CStr,                       E) \
O(SideExitJmpZero,                  ND, S(Int,Bool),                       E) \
O(SideExitJmpNZero,                 ND, S(Int,Bool),                       E) \
O(SideExitGuardLoc,                 ND, S(FramePtr),                       E) \
O(SideExitGuardStk,          D(StkPtr), S(StkPtr),                         E) \
/*    name                      dstinfo srcinfo                      flags */ \
O(JmpIndirect,                      ND, S(TCA),                          T|E) \
O(CheckSurpriseFlags,               ND, NA,                              B|E) \
O(SurpriseHook,                     ND, NA,                             Er|E) \
O(FunctionSuspendHook,              ND, S(FramePtr,PtrToGen) C(Bool),   Er|E) \
O(FunctionReturnHook,               ND, S(FramePtr) S(Gen),             Er|E) \
O(ReleaseVVOrExit,                  ND, S(FramePtr),                     B|E) \
O(RaiseError,                       ND, S(Str),                       E|T|Er) \
O(RaiseWarning,                     ND, S(Str),                         E|Er) \
O(RaiseNotice,                      ND, S(Str),                         E|Er) \
O(RaiseArrayIndexNotice,            ND, S(Int),                         E|Er) \
O(CheckInit,                        ND, S(Gen),                            B) \
O(CheckInitMem,                     ND, S(PtrToGen) C(Int),                B) \
O(CheckCold,                        ND, NA,                              B|E) \
O(CheckNullptr,                     ND, S(CountedStr,Nullptr),       B|E|CRc) \
O(CheckNonNull,  DSubtract(0, Nullptr), S(Nullptr,Func,PtrToGen,TCA),      B) \
O(CheckBounds,                      ND, S(Int) S(Int),                  E|Er) \
O(LdVectorSize,                 D(Int), S(Obj),                            E) \
O(CheckPackedArrayBounds,           ND, AK(Packed) S(Int),               B|E) \
O(CheckPackedArrayElemNull,    D(Bool), AK(Packed) S(Int),                 E) \
O(VectorHasImmCopy,                 ND, S(Obj),                            B) \
O(VectorDoCow,                      ND, S(Obj),                            E) \
O(AssertNonNull, DSubtract(0, Nullptr), S(Nullptr,CountedStr,Func),        P) \
O(Box,                         DBox(0), S(Gen),                    E|CRc|PRc) \
O(UnboxPtr,               D(PtrToCell), S(PtrToGen),                      NF) \
O(BoxPtr,            D(PtrToBoxedCell), S(PtrToGen),                      NF) \
O(LdVectorBase,           D(PtrToCell), S(Obj),                            E) \
O(LdPairBase,             D(PtrToCell), S(Obj),                            E) \
O(LdStack,                      DParam, S(StkPtr),                        NF) \
O(LdLoc,                        DParam, S(FramePtr),                      NF) \
O(LdStackAddr,                  DParam, S(StkPtr),                         C) \
O(LdLocAddr,                    DParam, S(FramePtr),                       C) \
O(LdMem,                        DParam, S(PtrToGen) C(Int),                B) \
O(LdProp,                       DParam, S(Obj) C(Int),                     B) \
O(LdElem,                      D(Cell), S(PtrToCell) S(Int),               E) \
O(LdPackedArrayElem,          DArrElem, AK(Packed) S(Int),                 E) \
O(LdRef,                        DLdRef, S(BoxedCell),                      B) \
O(LdThis,                        DThis, S(FramePtr),                     B|C) \
O(LdRetAddr,                D(RetAddr), S(FramePtr),                      NF) \
O(LdGbl,                        DParam, S(FramePtr),                       B) \
O(DefConst,                     DParam, NA,                                C) \
O(Conjure,                      DParam, NA,                               NF) \
O(ConvClsToCctx,               D(Cctx), S(Cls),                            C) \
O(LdCtx,                        D(Ctx), S(FramePtr),                       C) \
O(LdCctx,                      D(Cctx), S(FramePtr),                       C) \
O(LdCls,                        D(Cls), S(Str) C(Cls),                C|E|Er) \
O(LdClsCached,                  D(Cls), CStr,                         C|E|Er) \
O(LdClsCachedSafe,              D(Cls), CStr,                              B) \
O(LdClsCtx,                     D(Cls), S(Ctx),                            C) \
O(LdClsCctx,                    D(Cls), S(Cctx),                           C) \
O(LdClsCns,                     DParam, NA,                                B) \
O(LookupClsRDSHandle,     D(RDSHandle), S(Str),                            C) \
O(DerefClsRDSHandle,            D(Cls), S(RDSHandle),                     NF) \
O(LookupClsCns,           D(Uncounted), NA,                             E|Er) \
O(LdCns,                          DCns, CStr,                            PRc) \
O(LookupCns,                      DCns, CStr,                       E|Er|PRc) \
O(LookupCnsE,                     DCns, CStr,                       E|Er|PRc) \
O(LookupCnsU,                     DCns, CStr CStr,                  E|Er|PRc) \
O(LookupClsMethod,                  ND, S(Cls)                                \
                                          S(Str)                              \
                                          S(StkPtr)                           \
                                          S(FramePtr),                  E|Er) \
O(LdClsMethodCacheFunc,D(Func|Nullptr), NA,                               NF) \
O(LdClsMethodCacheCls,         D(Cctx), NA,                               NF) \
O(LookupClsMethodCache,D(Func|Nullptr), S(FramePtr),                    E|Er) \
O(LdClsMethodFCacheFunc,                                                      \
                       D(Func|Nullptr), NA,                               NF) \
O(LookupClsMethodFCache,                                                      \
                       D(Func|Nullptr), C(Cls)                                \
                                          S(FramePtr),                  E|Er) \
O(GetCtxFwdCallDyn,             D(Ctx), S(Ctx),                          PRc) \
O(GetCtxFwdCall,                D(Ctx), S(Ctx) C(Func),                  PRc) \
O(LdClsMethod,                 D(Func), S(Cls) C(Int),                     C) \
O(LdPropAddr,              D(PtrToGen), S(Obj) C(Int),                     C) \
O(LdClsPropAddrKnown,           DParam, C(Cls) CStr,                       C) \
O(LdClsPropAddrOrNull,                                                        \
                   D(PtrToGen|Nullptr), S(Cls) S(Str) C(Cls),         C|E|Er) \
O(LdClsPropAddrOrRaise,    D(PtrToGen), S(Cls) S(Str) C(Cls),         C|E|Er) \
O(LdClsInitData,          D(PtrToCell), S(Cls),                            C) \
O(LdObjMethod,                      ND, S(Cls) S(StkPtr),               E|Er) \
O(LdObjInvoke,                 D(Func), S(Cls),                            B) \
O(LdGblAddrDef,            D(PtrToGen), S(Str),                            E) \
O(LdGblAddr,               D(PtrToGen), S(Str),                            B) \
O(LdObjClass,                   D(Cls), S(Obj),                            C) \
O(LdArrFuncCtx,                     ND, S(Arr)                                \
                                          S(StkPtr)                           \
                                          S(FramePtr),                  E|Er) \
O(LdArrFPushCuf,                    ND, S(Arr)                                \
                                          S(StkPtr)                           \
                                          S(FramePtr),                  E|Er) \
O(LdStrFPushCuf,                    ND, S(Str)                                \
                                          S(StkPtr)                           \
                                          S(FramePtr),                  E|Er) \
O(LdFunc,                      D(Func), S(Str),                     E|CRc|Er) \
O(LdFuncCached,                D(Func), NA,                             E|Er) \
O(LdFuncCachedU,               D(Func), NA,                             E|Er) \
O(LdFuncCachedSafe,            D(Func), NA,                                B) \
O(LdARFuncPtr,                 D(Func), S(StkPtr,FramePtr) C(Int),         C) \
O(LdBindAddr,                   D(TCA), NA,                               NF) \
O(LdSSwitchDestFast,            D(TCA), S(Gen),                           NF) \
O(LdSSwitchDestSlow,            D(TCA), S(Gen),                         E|Er) \
O(LdSwitchDblIndex,             D(Int), S(Dbl) S(Int) S(Int),             NF) \
O(LdSwitchStrIndex,             D(Int), S(Str) S(Int) S(Int),            CRc) \
O(LdSwitchObjIndex,             D(Int), S(Obj) S(Int) S(Int),         CRc|Er) \
O(JmpSwitchDest,                    ND, S(Int),                          T|E) \
O(AllocObj,                  DAllocObj, S(Cls),                           Er) \
                                                                              \
O(ConstructInstance,         DAllocObj, NA,                               Er) \
O(CheckInitProps,                   ND, NA,                              B|E) \
O(InitProps,                        ND, NA,                             E|Er) \
O(CheckInitSProps,                  ND, NA,                              B|E) \
O(InitSProps,                       ND, NA,                             E|Er) \
O(NewInstanceRaw,            DAllocObj, NA,                               NF) \
O(InitObjProps,                     ND, S(Obj),                            E) \
O(CustomInstanceInit,          DofS(0), S(Obj),                           Er) \
                                                                              \
O(RegisterLiveObj,                  ND, S(Obj),                            E) \
O(LdClsCtor,                   D(Func), S(Cls),                         C|Er) \
O(LdClsName,              D(StaticStr), S(Cls),                            C) \
O(StClosureFunc,                    ND, S(Obj),                            E) \
O(StClosureArg,                     ND, S(Obj) S(Gen),                 CRc|E) \
O(StClosureCtx,                     ND, S(Obj) S(Ctx,Nullptr),         CRc|E) \
O(NewArray,                     D(Arr), C(Int),                          PRc) \
O(NewLikeArray,                 D(Arr), S(Arr) C(Int),                   PRc) \
O(NewMixedArray,                D(Arr), C(Int),                          PRc) \
O(NewVArray,                    D(Arr), C(Int),                          PRc) \
O(NewMIArray,                   D(Arr), C(Int),                          PRc) \
O(NewMSArray,                   D(Arr), C(Int),                          PRc) \
O(NewPackedArray,           DArrPacked, S(StkPtr),                 E|PRc|CRc) \
O(AllocPackedArray,         DArrPacked, NA,                            E|PRc) \
O(InitPackedArray,                  ND, S(Arr) S(Gen),                 E|CRc) \
O(InitPackedArrayLoop,              ND, S(Arr) S(StkPtr),              E|CRc) \
O(NewStructArray,               D(Arr), S(StkPtr),                 E|PRc|CRc) \
O(NewCol,                       D(Obj), C(Int) C(Int),                   PRc) \
O(Clone,                        D(Obj), S(Obj),                     E|PRc|Er) \
O(LdRaw,                        DLdRaw, S(Str,Obj,Func),                  NF) \
O(FreeActRec,              D(FramePtr), S(FramePtr),                      NF) \
/*    name                      dstinfo srcinfo                      flags */ \
O(Call,                      D(StkPtr), S(StkPtr) S(FramePtr),             E) \
O(CallArray,                 D(StkPtr), S(StkPtr),                     E|CRc) \
O(CallBuiltin,                DBuiltin, SVar(PtrToGen,Gen),         E|Er|PRc) \
O(NativeImpl,                       ND, S(FramePtr),                       E) \
O(Halt,                             ND, NA,                              T|E) \
O(RetCtrl,                          ND, S(StkPtr)                             \
                                          S(FramePtr)                         \
                                          S(RetAddr),                    T|E) \
O(StRetVal,                         ND, S(FramePtr) S(Gen),            E|CRc) \
O(RetAdjustStack,            D(StkPtr), S(FramePtr),                       E) \
O(StMem,                            ND, S(PtrToGen)                           \
                                          C(Int) S(Gen),               E|CRc) \
O(StProp,                           ND, S(Obj) C(Int) S(Gen),          E|CRc) \
O(StLoc,                            ND, S(FramePtr) S(Gen),            E|CRc) \
O(StLocNT,                          ND, S(FramePtr) S(Gen),            E|CRc) \
O(StGbl,                            ND, S(FramePtr) S(Gen),            E|CRc) \
O(StRef,                       DBox(1), S(BoxedCell) S(Cell),        E|CRc|P) \
O(StRaw,                            ND, S(Obj) S(Int),                     E) \
O(StElem,                           ND, S(PtrToCell)                          \
                                          S(Int)                              \
                                          S(Cell),                     E|CRc) \
O(LdStaticLocCached,      D(BoxedCell), NA,                               NF) \
O(CheckStaticLocInit,               ND, S(BoxedCell),                      B) \
O(ClosureStaticLocInit,   D(BoxedCell), CStr                                  \
                                          S(FramePtr)                         \
                                          S(Cell),                         E) \
O(StaticLocInitCached,              ND, S(BoxedCell) S(Cell),              E) \
O(SpillStack,                D(StkPtr), S(StkPtr)                             \
                                          C(Int)                              \
                                          SVar(StackElem),               CRc) \
O(SpillFrame,                D(StkPtr), S(StkPtr)                             \
                                          S(Func,Nullptr)                     \
                                          S(Ctx,Cls,Nullptr),            CRc) \
O(CufIterSpillFrame,         D(StkPtr), S(StkPtr)                             \
                                          S(FramePtr),                    NF) \
O(ExceptionBarrier,          D(StkPtr), S(StkPtr),                         E) \
O(ReqBindJmp,                       ND, NA,                              T|E) \
O(ReqRetranslateOpt,                ND, NA,                              T|E) \
O(ReqRetranslate,                   ND, NA,                              T|E) \
O(SyncABIRegs,                      ND, S(FramePtr) S(StkPtr),             E) \
O(EagerSyncVMRegs,                  ND, S(FramePtr) S(StkPtr),             E) \
O(Mov,                         DofS(0), S(Top),                          C|P) \
O(LdMIStateAddr,               DofS(0), S(PtrToGen) C(Int),                C) \
O(IncRef,                           ND, S(Gen),                            E) \
O(TakeStack,                        ND, S(StackElem),                      E) \
O(IncRefCtx,                        ND, S(Ctx),                            E) \
O(DecRefLoc,                        ND, S(FramePtr),                       E) \
O(DecRefStack,                      ND, S(StkPtr),                         E) \
O(DecRefThis,                       ND, S(FramePtr),                       E) \
O(GenericRetDecRefs,                ND, S(FramePtr),                       E) \
O(DecRef,                           ND, S(Gen),                      E|K|CRc) \
O(DecRefNZ,                         ND, S(Gen),                        E|CRc) \
O(DecRefMem,                        ND, S(PtrToGen)                           \
                                          C(Int),                      E|CRc) \
O(TakeRef,                          ND, S(Gen),                           NF) \
O(DefLabel,                     DMulti, NA,                                E) \
O(DefInlineFP,             D(FramePtr), S(StkPtr) S(StkPtr) S(FramePtr),  NF) \
O(InlineReturn,                     ND, S(FramePtr),                       E) \
O(DefFP,                   D(FramePtr), NA,                                E) \
O(DefSP,                     D(StkPtr), S(FramePtr),                       E) \
O(ReDefSP,                   D(StkPtr), S(StkPtr) S(FramePtr),            NF) \
O(VerifyParamCls,                   ND, S(Cls)                                \
                                          S(Cls)                              \
                                          C(Int)                              \
                                          C(Int),                       E|Er) \
O(VerifyParamCallable,              ND, S(Gen) C(Int),                  E|Er) \
O(VerifyParamFail,                  ND, C(Int),                         E|Er) \
O(RaiseUninitLoc,                   ND, S(Str),                         E|Er) \
O(WarnNonObjProp,                   ND, NA,                             E|Er) \
O(ThrowNonObjProp,                  ND, NA,                           T|E|Er) \
O(RaiseUndefProp,                   ND, S(Obj) CStr,                    E|Er) \
O(PrintStr,                         ND, S(Str),                     E|Er|CRc) \
O(PrintInt,                         ND, S(Int),                     E|Er|CRc) \
O(PrintBool,                        ND, S(Bool),                    E|Er|CRc) \
O(VerifyRetCls,                     ND, S(Cls)                                \
                                          S(Cls)                              \
                                          C(Int)                              \
                                          S(Cell),                      E|Er) \
O(VerifyRetCallable,                ND, S(Gen),                         E|Er) \
O(VerifyRetFail,                    ND, S(Gen),                         E|Er) \
O(AddElemStrKey,                D(Arr), S(Arr)                                \
                                          S(Str)                              \
                                          S(Cell),                Er|CRc|PRc) \
O(AddElemIntKey,                D(Arr), S(Arr)                                \
                                          S(Int)                              \
                                          S(Cell),                Er|CRc|PRc) \
O(AddNewElem,                   D(Arr), S(Arr) S(Cell),           Er|CRc|PRc) \
O(ColAddElemC,                  D(Obj), S(Obj)                                \
                                         S(Cell)                              \
                                         S(Cell),                   Er|CRc|P) \
O(ColAddNewElemC,               D(Obj), S(Obj) S(Cell),             Er|CRc|P) \
O(ColIsEmpty,                  D(Bool), S(Obj),                           NF) \
O(ColIsNEmpty,                 D(Bool), S(Obj),                           NF) \
/*    name                      dstinfo srcinfo                      flags */ \
O(ConcatStrStr,                 D(Str), S(Str) S(Str),            Er|CRc|PRc) \
O(ConcatIntStr,                 D(Str), S(Int) S(Str),                Er|PRc) \
O(ConcatStrInt,                 D(Str), S(Str) S(Int),            Er|CRc|PRc) \
O(ConcatCellCell,               D(Str), S(Cell) S(Cell),          Er|CRc|PRc) \
O(ConcatStr3,                   D(Str), S(Str) S(Str) S(Str),     Er|CRc|PRc) \
O(ConcatStr4,                   D(Str), S(Str)                                \
                                          S(Str)                              \
                                          S(Str)                              \
                                          S(Str),                 Er|CRc|PRc) \
O(ArrayAdd,                     D(Arr), S(Arr) S(Arr),            Er|CRc|PRc) \
O(AKExists,                    D(Bool), S(Cell) S(Cell),                  NF) \
O(InterpOne,                 D(StkPtr), S(StkPtr) S(FramePtr),                \
                                                                        E|Er) \
O(InterpOneCF,               D(StkPtr), S(StkPtr) S(FramePtr),                \
                                                                      T|E|Er) \
O(CreateCont,                   D(Obj), S(FramePtr) C(Int)                    \
                                          S(TCA,Nullptr) C(Int),       E|PRc) \
O(ContEnter,                 D(StkPtr), S(StkPtr)                             \
                                          S(FramePtr)                         \
                                          S(FramePtr)                         \
                                          S(TCA)                              \
                                          C(Int),                          E) \
O(ContPreNext,                      ND, S(Obj) C(Bool),                  B|E) \
O(ContStartedCheck,                 ND, S(Obj),                          B|E) \
O(ContValid,                   D(Bool), S(Obj),                            E) \
O(ContArIncKey,                     ND, S(FramePtr),                       E) \
O(ContArUpdateIdx,                  ND, S(FramePtr) S(Int),                E) \
O(LdContActRec,                 DParam, S(Obj),                            C) \
O(LdContArRaw,                  DLdRaw, S(FramePtr),                      NF) \
O(StContArRaw,                      ND, S(FramePtr) S(Int,TCA,Nullptr),    E) \
O(LdContArValue,                DParam, S(FramePtr),                     PRc) \
O(StContArValue,                    ND, S(FramePtr) S(Cell),           E|CRc) \
O(LdContArKey,                  DParam, S(FramePtr),                     PRc) \
O(StContArKey,                      ND, S(FramePtr) S(Gen),            E|CRc) \
O(StAsyncArRaw,                     ND, S(FramePtr)                           \
                                          S(Int,TCA,Nullptr),              E) \
O(StAsyncArResult,                  ND, S(FramePtr) S(Cell),           E|CRc) \
O(LdAsyncArParentChain,         D(ABC), S(FramePtr),                      NF) \
O(AFWHBlockOn,                      ND, S(FramePtr) S(Obj),            E|CRc) \
O(LdWHState,                    D(Int), S(Obj),                           NF) \
O(LdWHResult,                  D(Cell), S(Obj),                           NF) \
O(LdAFWHActRec,                 DParam, S(Obj),                            C) \
O(LdResumableArObj,             D(Obj), S(FramePtr),                       C) \
O(CreateAFWH,                   D(Obj), S(FramePtr)                           \
                                          C(Int)                              \
                                          S(TCA,Nullptr)                      \
                                          C(Int)                              \
                                          S(Obj),               E|Er|CRc|PRc) \
O(CreateSSWH,                   D(Obj), S(Cell),                     CRc|PRc) \
O(AFWHPrepareChild,                 ND, S(FramePtr) S(Obj),             E|Er) \
O(ABCUnblock,                       ND, S(ABC),                            E) \
O(IterInit,                    D(Bool), S(Arr,Obj)                            \
                                          S(FramePtr),              Er|E|CRc) \
O(IterInitK,                   D(Bool), S(Arr,Obj)                            \
                                          S(FramePtr),              E|Er|CRc) \
O(IterNext,                    D(Bool), S(FramePtr),                    Er|E) \
O(IterNextK,                   D(Bool), S(FramePtr),                    Er|E) \
O(WIterInit,                   D(Bool), S(Arr,Obj)                            \
                                          S(FramePtr),              E|Er|CRc) \
O(WIterInitK,                  D(Bool), S(Arr,Obj)                            \
                                          S(FramePtr),              E|Er|CRc) \
O(WIterNext,                   D(Bool), S(FramePtr),                    Er|E) \
O(WIterNextK,                  D(Bool), S(FramePtr),                    Er|E) \
O(MIterInit,                   D(Bool), S(BoxedCell)                          \
                                          S(FramePtr),                  Er|E) \
O(MIterInitK,                  D(Bool), S(BoxedCell)                          \
                                          S(FramePtr),                  Er|E) \
O(MIterNext,                   D(Bool), S(FramePtr),                       E) \
O(MIterNextK,                  D(Bool), S(FramePtr),                       E) \
O(IterFree,                         ND, S(FramePtr),                       E) \
O(MIterFree,                        ND, S(FramePtr),                       E) \
O(DecodeCufIter,               D(Bool), S(Arr,Obj,Str)                        \
                                          S(FramePtr),                  Er|E) \
O(CIterFree,                        ND, S(FramePtr),                       E) \
O(DefMIStateBase,         D(PtrToCell), NA,                               NF) \
O(BaseG,                   D(PtrToGen), S(Str),                        E|Er) \
O(PropX,                   D(PtrToGen), S(Obj,PtrToGen)                       \
                                          S(Cell)                             \
                                          S(PtrToCell),               E|Er) \
O_STK(PropDX,              D(PtrToGen), S(Obj,PtrToGen)                       \
                                          S(Cell)                             \
                                          S(PtrToCell),         MProp|E|Er) \
O(CGetProp,                    D(Cell), S(Obj,PtrToGen)                       \
                                          S(Cell)                             \
                                          S(PtrToCell),           E|PRc|Er) \
O_STK(VGetProp,           D(BoxedCell), S(Obj,PtrToGen)                       \
                                          S(Cell)                             \
                                          S(PtrToCell),                       \
                                                            MProp|E|PRc|Er) \
O_STK(BindProp,                     ND, S(Obj,PtrToGen)                       \
                                          S(Cell)                             \
                                          S(BoxedCell)                        \
                                          S(PtrToCell),         MProp|E|Er) \
O_STK(SetProp,                      ND, S(Obj,PtrToGen)                       \
                                          S(Cell)                             \
                                          S(Cell),              MProp|E|Er) \
O(UnsetProp,                        ND, S(Obj,PtrToGen)                       \
                                          S(Cell),                    E|Er) \
O_STK(SetOpProp,               D(Cell), S(Obj,PtrToGen)                       \
                                          S(Cell)                             \
                                          S(Cell)                             \
                                          S(PtrToCell),     MProp|E|PRc|Er) \
O_STK(IncDecProp,              D(Cell), S(Obj,PtrToGen)                       \
                                          S(Cell)                             \
                                          S(PtrToCell),     MProp|E|PRc|Er) \
O(EmptyProp,                   D(Bool), S(Obj,PtrToGen)                       \
                                          S(Cell),                    E|Er) \
O(IssetProp,                   D(Bool), S(Obj,PtrToGen)                       \
                                          S(Cell),                    E|Er) \
O(ElemX,                   D(PtrToGen), S(PtrToGen)                           \
                                          S(Cell)                             \
                                          S(PtrToCell),                 E|Er) \
O(ElemArray,               D(PtrToGen), S(PtrToArr) S(Int,Str),         E|Er) \
O(ElemArrayW,              D(PtrToGen), S(PtrToArr) S(Int,Str),         E|Er) \
O_STK(ElemDX,              D(PtrToGen), S(PtrToGen)                           \
                                          S(Cell)                             \
                                          S(PtrToCell),           MElem|E|Er) \
O_STK(ElemUX,              D(PtrToGen), S(PtrToGen)                           \
                                          S(Cell)                             \
                                          S(PtrToCell),           MElem|E|Er) \
O(ArrayGet,                    D(Cell), S(Arr)                                \
                                          S(Int,Str),                 PRc|Er) \
O(StringGet,              D(StaticStr), S(Str) S(Int),                PRc|Er) \
O(MapGet,                      D(Cell), S(Obj)                                \
                                          S(Int,Str),               E|PRc|Er) \
O(CGetElem,                    D(Cell), S(PtrToGen)                           \
                                          S(Cell)                             \
                                          S(PtrToCell),             E|PRc|Er) \
O_STK(VGetElem,           D(BoxedCell), S(PtrToGen)                           \
                                          S(Cell)                             \
                                          S(PtrToCell),                       \
                                                              MElem|E|PRc|Er) \
O_STK(BindElem,                     ND, S(PtrToGen)                           \
                                          S(Cell)                             \
                                          S(BoxedCell)                        \
                                          S(PtrToCell),           MElem|E|Er) \
O(ArraySet,                     D(Arr), S(Arr)                                \
                                          S(Int,Str)                          \
                                          S(Cell),            E|PRc|CRc|K|Er) \
O(MapSet,                           ND, S(Obj)                                \
                                          S(Int,Str)                          \
                                          S(Cell),                      E|Er) \
O(ArraySetRef,                      ND, S(Arr)                                \
                                          S(Int,Str)                          \
                                          S(Cell)                             \
                                          S(BoxedArr),            E|CRc|K|Er) \
O_STK(SetElem,                DSetElem, S(PtrToGen)                           \
                                          S(Cell)                             \
                                          S(Cell),            MElem|E|PRc|Er) \
O_STK(SetWithRefElem,               ND, S(PtrToGen)                           \
                                          S(Cell)                             \
                                          S(PtrToGen)                         \
                                          S(PtrToCell),           MElem|E|Er) \
O_STK(UnsetElem,                    ND, S(PtrToGen) S(Cell),      MElem|E|Er) \
O_STK(SetOpElem,               D(Cell), S(PtrToGen)                           \
                                          S(Cell)                             \
                                          S(Cell)                             \
                                          S(PtrToCell),     MElem|E|PRc|Er) \
O_STK(IncDecElem,              D(Cell), S(PtrToGen)                           \
                                          S(Cell)                             \
                                          S(PtrToCell),     MElem|E|PRc|Er) \
O_STK(SetNewElem,                   ND, S(PtrToGen)                           \
                                          S(Cell),                MElem|E|Er) \
O_STK(SetNewElemArray,              ND, S(PtrToArr)                           \
                                          S(Cell),                MElem|E|Er) \
O_STK(SetWithRefNewElem,            ND, S(PtrToGen)                           \
                                          S(PtrToGen)                         \
                                          S(PtrToCell),           MElem|E|Er) \
O_STK(BindNewElem,                  ND, S(PtrToGen)                           \
                                          S(BoxedCell)                        \
                                          S(PtrToCell),           MElem|E|Er) \
O(ArrayIsset,                  D(Bool), S(Arr) S(Int,Str),              E|Er) \
O(StringIsset,                 D(Bool), S(Str) S(Int),                    NF) \
O(VectorIsset,                 D(Bool), S(Obj) S(Int),                     E) \
O(PairIsset,                   D(Bool), S(Obj) S(Int),                     E) \
O(MapIsset,                    D(Bool), S(Obj) S(Int,Str),                 E) \
O(IssetElem,                   D(Bool), S(PtrToGen)                           \
                                          S(Cell)                             \
                                          S(PtrToCell),                 E|Er) \
O(EmptyElem,                   D(Bool), S(PtrToGen)                           \
                                          S(Cell)                             \
                                          S(PtrToCell),                 E|Er) \
O(IncStat,                          ND, C(Int) C(Int) C(Bool),             E) \
O(TypeProfileFunc,                  ND, S(Gen) S(Func),                    E) \
O(ProfileArray,                     ND, S(Arr),                            E) \
O(IncStatGrouped,                   ND, CStr CStr C(Int),                  E) \
O(RBTrace,                          ND, NA,                                E) \
O(IncTransCounter,                  ND, NA,                                E) \
O(IncProfCounter,                   ND, NA,                                E) \
O(Count,                        D(Int), S(Cell),                          Er) \
O(CountArray,                   D(Int), S(Arr),                           NF) \
O(CountArrayFast,               D(Int), S(Arr),                           NF) \
O(CountCollection,              D(Int), S(Obj),                           NF) \
O(ArrayIdx,                    D(Cell), S(Arr)                                \
                                          S(Int,Str)                          \
                                          S(Cell),                     E|PRc) \
O(GenericIdx,                  D(Cell), S(Cell)                               \
                                          S(Cell)                             \
                                          S(Cell),                  E|PRc|Er) \
O(ZeroErrorLevel,               D(Int), NA,                                E) \
O(RestoreErrorLevel,                ND, S(Int),                            E) \
O(Nop,                              ND, NA,                               NF) \
O(DbgAssertRefCount,                ND, S(Counted,StaticStr,StaticArr),    E) \
O(DbgAssertPtr,                     ND, S(PtrToGen),                       E) \
O(DbgAssertType,                    ND, S(Cell),                           E) \
O(DbgAssertRetAddr,                 ND, NA,                                E) \

enum class Opcode : uint16_t {
#define O(name, ...) name,
  IR_OPCODES
#undef O
};
#define O(name, ...) UNUSED auto constexpr name = Opcode::name;
  IR_OPCODES
#undef O

#define O(...) +1
size_t constexpr kNumOpcodes = IR_OPCODES;
#undef O

/*
 * Returns true for instructions that refine the types of values with
 * a runtime check.
 */
bool isGuardOp(Opcode opc);

/*
 * A "query op" is any instruction returning Type::Bool that is
 * negateable.
 */
bool isQueryOp(Opcode opc);

/*
 * Return true if opc is an int comparison operator
 */
bool isIntQueryOp(Opcode opc);

/*
 * Return the int-query opcode for the given non-int-query opcode
 */
Opcode queryToIntQueryOp(Opcode opc);

/*
 * Return true if opc is a dbl comparison operator
 */
bool isDblQueryOp(Opcode opc);

/*
 * Return the dbl-query opcode for the given non-dbl-query opcode
 */
Opcode queryToDblQueryOp(Opcode opc);

/*
 * A "fusable query op" is any instruction returning Type::Bool that
 * has a corresponding "query jump op" for branch fusion.
 */
bool isFusableQueryOp(Opcode opc);

/*
 * A "query jump op" is a conditional jump instruction that
 * corresponds to one of the fusable query op instructions.
 */
bool isQueryJmpOp(Opcode opc);

/*
 * Translate a query op into a conditional jump that does the same
 * test (a "query jump op").
 *
 * Pre: isFusableQueryOp(opc)
 */
Opcode queryToJmpOp(Opcode opc);

/*
 * Translate a "query jump op" to a query op.
 *
 * Pre: isQueryJmpOp(opc);
 */
Opcode queryJmpToQueryOp(Opcode opc);

/*
 * Convert a jump to its corresponding side exit.
 */
Opcode jmpToSideExitJmp(Opcode opc);

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
 *
 * Pre: opc is a 2-argument query op.
 */
Opcode commuteQueryOp(Opcode opc);

const char* opcodeName(Opcode opcode);

bool opHasExtraData(Opcode op);

enum OpcodeFlag : uint64_t {
  NoFlags          = 0,
  HasDest          = 1ULL <<  0,
  CanCSE           = 1ULL <<  1,
  Essential        = 1ULL <<  2,
  Branch           = 1ULL <<  3,
  HasStackVersion  = 1ULL <<  4,
  ConsumesRC       = 1ULL <<  5,
  ProducesRC       = 1ULL <<  6,
  MInstrProp       = 1ULL <<  7,
  MInstrElem       = 1ULL <<  8,
  MayRaiseError    = 1ULL <<  9,
  Terminal         = 1ULL << 10, // has no next instruction
  NaryDest         = 1ULL << 11, // has 0 or more destinations
  HasExtra         = 1ULL << 12,
  Passthrough      = 1ULL << 13,
  KillsSources     = 1ULL << 14,
  ModifiesStack    = 1ULL << 15,
};

bool hasEdges(Opcode opc);
bool opcodeHasFlags(Opcode opc, uint64_t flags);
Opcode getStackModifyingOpcode(Opcode opc);

bool isRefCounted(SSATmp* opnd);

using SrcRange = folly::Range<SSATmp**>;
using DstRange = folly::Range<SSATmp*>;

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
 * Check that an instruction has operands of allowed types.
 */
bool checkOperandTypes(const IRInstruction*, const IRUnit* unit = nullptr);


int minstrBaseIdx(Opcode opc);
int minstrBaseIdx(const IRInstruction* inst);

struct MInstrEffects {
  static bool supported(Opcode op);
  static bool supported(const IRInstruction* inst);

  /*
   * MInstrEffects::get is used to allow multiple different consumers to deal
   * with the side effects of vector instructions. It takes an instruction and
   * a LocalStateHook, which is defined in frame-state.h.
   */
  static void get(const IRInstruction*, LocalStateHook&);

  explicit MInstrEffects(const IRInstruction* inst);
  MInstrEffects(Opcode op, Type base);
  MInstrEffects(Opcode op, SSATmp* base);
  MInstrEffects(Opcode opc, const std::vector<SSATmp*>& srcs);

  Type baseType;
  bool baseTypeChanged;
  bool baseValChanged;

private:
  void init(const Opcode op, const Type base);
};

using TcaRange = folly::Range<TCA>;

/*
 * Counts the number of cells a SpillStack will logically push.  (Not
 * including the number it pops.)  That is, for each SSATmp in the
 * spill sources, this totals up whether it is an ActRec or a cell.
 */
int32_t spillValueCells(const IRInstruction* spillStack);

} // namespace jit
} // namespace HPHP

namespace std {
  template<> struct hash<HPHP::jit::Opcode> {
    size_t operator()(HPHP::jit::Opcode op) const { return uint16_t(op); }
  };
  template<> struct hash<HPHP::jit::Type> {
    size_t operator()(HPHP::jit::Type t) const { return t.hash(); }
  };
}

namespace folly {
template<> struct FormatValue<HPHP::jit::Opcode> {
  explicit FormatValue(HPHP::jit::Opcode op) : m_op(op) {}

  template<typename Callback> void format(FormatArg& arg, Callback& cb) const {
    format_value::formatString(opcodeName(m_op), arg, cb);
  }

 private:
  HPHP::jit::Opcode m_op;
};
}

#include "hphp/runtime/vm/jit/ir-opcode-inl.h"

#endif
