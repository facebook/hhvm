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
#include <cinttypes>
#include <iostream>
#include <vector>
#include <stack>
#include <list>
#include <cassert>
#include <type_traits>
#include <boost/noncopyable.hpp>
#include <boost/checked_delete.hpp>
#include "util/asm-x64.h"
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
static const TCA kIRDirectJmpInactive = NULL;
// Fixup Jcc;Jmp branches out of trace using REQ_BIND_JMPCC_FIRST/SECOND
static const TCA kIRDirectJccJmpActive = (TCA)0x01;
// Optimize Jcc exit from trace when fall through path stays in trace
static const TCA kIRDirectJccActive = (TCA)0x02;
// Optimize guard exit from beginning of trace
static const TCA kIRDirectGuardActive = (TCA)0x03;

#define PUNT(instr) do {                             \
  throw FailedIRGen(__FILE__, __LINE__, #instr);     \
} while(0)


// XXX TODO: define another namespace for opcodes
// TODO: Make sure the MayModRefs column is correct... (MayRaiseError too...)

/*
 * Flags on opcodes.  See doc/ir.specification for details on the meaning
 * of these flags.
 *
 * Note that the flags in the opcodes table below are not
 * authoritative---just useful defaults for each opcode.  Exceptions
 * are made in some cases based on operand types in the IRInstruction
 * wrappers that access each flag, so most uses of these flags should
 * go through there.
 */
enum OpcodeFlag : uint64_t {
  NoFlags          = 0x0000,
  HasDest          = 0x0001,
  CanCSE           = 0x0002,
  Essential        = 0x0004,
  MemEffects       = 0x0008,
  CallsNative      = 0x0010,
  ConsumesRC       = 0x0020,
  ProducesRC       = 0x0040,
  MayModifyRefs    = 0x0080,
  Rematerializable = 0x0100, // TODO: implies HasDest
  MayRaiseError    = 0x0200,
};

#define IR_OPCODES                                                      \
  /* checks */                                                          \
  OPC(GuardType,         (HasDest|CanCSE|Essential))                    \
  OPC(GuardLoc,          (Essential))                                   \
  OPC(GuardStk,          (HasDest|Essential))                           \
  OPC(AssertStk,         (HasDest|Essential))                           \
  OPC(GuardRefs,         (Essential))                                   \
  OPC(AssertLoc,         (Essential))                                   \
                                                                        \
  /* arith ops (integer) */                                             \
  OPC(OpAdd,             (HasDest|CanCSE))                              \
  OPC(OpSub,             (HasDest|CanCSE))                              \
  OPC(OpAnd,             (HasDest|CanCSE))                              \
  OPC(OpOr,              (HasDest|CanCSE))                              \
  OPC(OpXor,             (HasDest|CanCSE))                              \
  OPC(OpMul,             (HasDest|CanCSE))                              \
                                                                        \
  /* convert from src operand's type to destination type */             \
  OPC(Conv,              (HasDest|CanCSE|CallsNative))                  \
                                                                        \
  /* predicates that can't be branch-fused */                           \
  OPC(ExtendsClass,      (HasDest|CanCSE))                              \
                                                                        \
  /* branch-fusable query operators returning bool */                   \
  /* (TODO(#2058842): enum order currently matters here) */             \
  OPC(OpGt,              (HasDest|CanCSE))                              \
  OPC(OpGte,             (HasDest|CanCSE))                              \
  OPC(OpLt,              (HasDest|CanCSE))                              \
  OPC(OpLte,             (HasDest|CanCSE))                              \
  OPC(OpEq,              (HasDest|CanCSE))                              \
  OPC(OpNeq,             (HasDest|CanCSE))                              \
  /* XXX validate that we don't call helpers with side effects */       \
  /* and ref count consumption/production for any of these query */     \
  /* operations and their corresponding conditional branches */         \
  OPC(OpSame,            (HasDest|CanCSE|CallsNative))                  \
  OPC(OpNSame,           (HasDest|CanCSE|CallsNative))                  \
                                                                        \
  OPC(InstanceOf,        (HasDest|CanCSE|CallsNative))                  \
  OPC(NInstanceOf,       (HasDest|CanCSE|CallsNative))                  \
  OPC(InstanceOfBitmask, (HasDest|CanCSE))                              \
  OPC(NInstanceOfBitmask,(HasDest|CanCSE))                              \
                                                                        \
  /* isset, empty, and istype queries (unary) */                        \
  OPC(IsSet,             (HasDest|CanCSE))                              \
  OPC(IsType,            (HasDest|CanCSE))                              \
  OPC(IsNSet,            (HasDest|CanCSE))                              \
  OPC(IsNType,           (HasDest|CanCSE))                              \
                                                                        \
  /* conditional branches & jump */                                     \
  /* there is a conditional branch for each of the above query */       \
  /* operators to enable generating efficient comparison-and-branch */  \
  /* instruction sequences */                                           \
  OPC(JmpGt,             (HasDest|Essential))                           \
  OPC(JmpGte,            (HasDest|Essential))                           \
  OPC(JmpLt,             (HasDest|Essential))                           \
  OPC(JmpLte,            (HasDest|Essential))                           \
  OPC(JmpEq,             (HasDest|Essential))                           \
  OPC(JmpNeq,            (HasDest|Essential))                           \
  OPC(JmpSame,           (HasDest|Essential))                           \
  OPC(JmpNSame,          (HasDest|Essential))                           \
  OPC(JmpInstanceOf,     (HasDest|Essential|CallsNative))               \
  OPC(JmpNInstanceOf,    (HasDest|Essential|CallsNative))               \
  OPC(JmpInstanceOfBitmask,                                             \
                         (HasDest|Essential))                           \
  OPC(JmpNInstanceOfBitmask,                                            \
                         (HasDest|Essential))                           \
  OPC(JmpIsSet,          (HasDest|Essential))                           \
  OPC(JmpIsType,         (HasDest|Essential))                           \
  OPC(JmpIsNSet,         (HasDest|Essential))                           \
  OPC(JmpIsNType,        (HasDest|Essential))                           \
    /* keep preceeding conditional branches contiguous */               \
  OPC(JmpZero,           (HasDest|Essential))                           \
  OPC(JmpNZero,          (HasDest|Essential))                           \
  OPC(Jmp_,              (HasDest|Essential))                           \
  OPC(ExitWhenSurprised, (Essential))                                   \
  OPC(ExitOnVarEnv,      (Essential))                                   \
  OPC(ReleaseVVOrExit,   (Essential))                                   \
  OPC(CheckInit,         (NoFlags))                                     \
                                                                        \
  OPC(Unbox,             (HasDest|ProducesRC))                          \
  OPC(Box,               (HasDest|Essential|MemEffects|                 \
                          CallsNative|ConsumesRC|                       \
                          ProducesRC))                                  \
  OPC(UnboxPtr,          (HasDest))                                     \
                                                                        \
  /* loads */                                                           \
  OPC(LdStack,           (HasDest))                                     \
  OPC(LdLoc,             (HasDest))                                     \
  OPC(LdStackAddr,       (HasDest|CanCSE))                              \
  OPC(LdLocAddr,         (HasDest|CanCSE))                              \
  OPC(LdMem,             (HasDest))                                     \
  OPC(LdProp,            (HasDest))                                     \
  OPC(LdRef,             (HasDest))                                     \
  OPC(LdThis,            (HasDest|CanCSE|Rematerializable))             \
  OPC(LdRetAddr,         (HasDest))                                     \
  OPC(LdHome,            (HasDest|CanCSE))                              \
  OPC(LdConst,           (HasDest|CanCSE|Rematerializable))             \
  OPC(DefConst,          (HasDest|CanCSE))                              \
  OPC(LdCls,             (HasDest|CanCSE|MayModifyRefs|                 \
                          Rematerializable|MayRaiseError))              \
  OPC(LdClsCns,          (HasDest|CanCSE))                              \
  OPC(LdClsMethodCache,  (HasDest|CanCSE|MayRaiseError))                \
  OPC(LdClsMethod,       (HasDest|CanCSE))                              \
  /* XXX TODO Create version of LdClsPropAddr that doesn't check */     \
  OPC(LdPropAddr,        (HasDest|CanCSE))                              \
  OPC(LdClsPropAddr,     (HasDest|CanCSE|Essential|                     \
                          Rematerializable|MayRaiseError))              \
  /* helper call to MethodCache::Lookup */                              \
  OPC(LdObjMethod,       (HasDest|CanCSE|Essential|                     \
                          CallsNative|MayModifyRefs|                    \
                          MayRaiseError))                               \
  OPC(LdObjClass,        (HasDest|CanCSE))                              \
  OPC(LdCachedClass,     (HasDest|CanCSE|Rematerializable))             \
  /* helper call to FuncCache::lookup */                                \
  OPC(LdFunc,            (HasDest|Essential|CallsNative|                \
                          ConsumesRC|MayRaiseError))                    \
  /* helper call for FPushFuncD(FixedFuncCache::lookup) */              \
  OPC(LdFixedFunc,       (HasDest|CanCSE|Essential|                     \
                          MayRaiseError))                               \
  OPC(LdCurFuncPtr,      (HasDest|CanCSE|Rematerializable))             \
  OPC(LdARFuncPtr,       (HasDest|CanCSE))                              \
  OPC(LdFuncCls,         (HasDest|CanCSE|Rematerializable))             \
  OPC(LdContLocalsPtr,   (HasDest|CanCSE|Rematerializable))             \
  OPC(NewObj,            (HasDest|Essential|MemEffects|                 \
                          CallsNative|ProducesRC))                      \
  OPC(NewArray,          (HasDest|Essential|MemEffects|                 \
                          CallsNative|ProducesRC))                      \
  OPC(NewTuple,          (HasDest|Essential|MemEffects|                 \
                          CallsNative|ConsumesRC|ProducesRC))           \
  OPC(LdRaw,             (HasDest))                                     \
  OPC(DefActRec,         (HasDest|MemEffects))                          \
  OPC(FreeActRec,        (HasDest|MemEffects))                          \
  OPC(Call,              (HasDest|Essential|MemEffects|                 \
                          ConsumesRC|MayModifyRefs))                    \
  OPC(NativeImpl,        (Essential|MemEffects|CallsNative|             \
                          MayModifyRefs))                               \
  OPC(RetCtrl,           (Essential|MemEffects))                        \
  OPC(RetVal,            (Essential|MemEffects|ConsumesRC))             \
  OPC(RetAdjustStack,    (HasDest|Essential))                           \
  /* stores */                                                          \
  OPC(StMem,             (Essential|MemEffects|ConsumesRC|              \
                          MayModifyRefs))                               \
  OPC(StMemNT,           (Essential|MemEffects|ConsumesRC))             \
  OPC(StProp,            (Essential|MemEffects|ConsumesRC|              \
                          MayModifyRefs))                               \
  OPC(StPropNT,          (Essential|MemEffects|ConsumesRC))             \
  OPC(StLoc,             (Essential|MemEffects|ConsumesRC))             \
  OPC(StLocNT,           (Essential|MemEffects|ConsumesRC))             \
  OPC(StRef,             (HasDest|Essential|MemEffects|                 \
                          ConsumesRC|MayModifyRefs))                    \
  OPC(StRefNT,           (HasDest|Essential|MemEffects|                 \
                          ConsumesRC))                                  \
  OPC(StRaw,             (Essential|MemEffects))                        \
  OPC(SpillStack,        (HasDest|Essential|MemEffects|                 \
                          ConsumesRC))                                  \
  /* Update ExitTrace entries in sync with ExitType below */            \
  OPC(ExitTrace,         (Essential))                                   \
  OPC(ExitTraceCc,       (Essential))                                   \
  OPC(ExitSlow,          (Essential))                                   \
  OPC(ExitSlowNoProgress,(Essential))                                   \
  OPC(ExitGuardFailure,  (Essential))                                   \
  /* */                                                                 \
  OPC(Mov,               (HasDest|CanCSE))                              \
  OPC(LdAddr,            (HasDest|CanCSE))                              \
  OPC(IncRef,            (HasDest|MemEffects|ProducesRC))               \
  OPC(DecRefLoc,         (Essential|MemEffects|MayModifyRefs))          \
  OPC(DecRefStack,       (Essential|MemEffects|MayModifyRefs))          \
  OPC(DecRefThis,        (Essential|MemEffects|MayModifyRefs))          \
  OPC(GenericRetDecRefs, (HasDest|Essential|MemEffects|CallsNative|     \
                          MayModifyRefs))                               \
  OPC(DecRef,            (Essential|MemEffects|ConsumesRC|              \
                          MayModifyRefs))                               \
  OPC(DecRefMem,         (Essential|MemEffects|ConsumesRC|              \
                          MayModifyRefs))                               \
  /* DecRefNZ only decrements the ref count, and doesn't modify Refs. */ \
  /* DecRefNZ also doesn't run dtor, so we don't mark it essential. */  \
  OPC(DecRefNZ,          (MemEffects|ConsumesRC))                       \
  OPC(DefLabel,          (Essential))                                   \
  OPC(Marker,            (Essential))                                   \
  OPC(DefFP,             (HasDest|Essential))                           \
  OPC(DefSP,             (HasDest|Essential))                           \
                                                                        \
  /* runtime helpers */                                                 \
 /* XXX check consume ref count */                                      \
  OPC(RaiseUninitWarning,(Essential|MemEffects|CallsNative|             \
                          MayModifyRefs|MayRaiseError))                 \
  OPC(Print,             (Essential|MemEffects|CallsNative|             \
                          ConsumesRC))                                  \
  OPC(AddElem,           (HasDest|MemEffects|CallsNative|               \
                          ConsumesRC|ProducesRC|MayModifyRefs))         \
  OPC(AddNewElem,        (HasDest|MemEffects|CallsNative|               \
                          ConsumesRC|ProducesRC))                       \
  OPC(DefCns,            (HasDest|CanCSE|Essential|                     \
                          MemEffects|CallsNative))                      \
  OPC(Concat,            (HasDest|MemEffects|CallsNative|               \
                          ConsumesRC|ProducesRC|MayModifyRefs))         \
  OPC(ArrayAdd,          (HasDest|MemEffects|CallsNative|               \
                          ConsumesRC|ProducesRC))                       \
  OPC(DefCls,            (CanCSE|Essential|CallsNative))                \
  OPC(DefFunc,           (CanCSE|Essential|CallsNative))                \
  OPC(InterpOne,         (HasDest|Essential|MemEffects|                 \
                          CallsNative|MayModifyRefs|                    \
                          MayRaiseError))                               \
                                                                        \
  /* for register allocation */                                         \
  OPC(Spill,             (HasDest|MemEffects))                          \
  OPC(Reload,            (HasDest|MemEffects))                          \
  OPC(AllocSpill,        (Essential|MemEffects))                        \
  OPC(FreeSpill,         (Essential|MemEffects))                        \
  /* continuation support */                                            \
  OPC(CreateCont,        (HasDest|Essential|MemEffects|                 \
                          CallsNative|ProducesRC))                      \
  OPC(FillContLocals,    (Essential|MemEffects|CallsNative))            \
  OPC(FillContThis,      (Essential|MemEffects))                        \
  OPC(ContEnter,         (Essential|MemEffects))                        \
  OPC(UnlinkContVarEnv,  (Essential|MemEffects|CallsNative))            \
  OPC(LinkContVarEnv,    (Essential|MemEffects|CallsNative))            \
  OPC(ContRaiseCheck,    (Essential))                                   \
  OPC(ContPreNext,       (Essential|MemEffects))                        \
  OPC(ContStartedCheck,  (Essential))                                   \
  /*  */                                                                \
  OPC(IterInit,          (HasDest|CallsNative|MemEffects|MayModifyRefs| \
                          ConsumesRC))                                  \
  OPC(IterInitK,         (HasDest|CallsNative|MemEffects|MayModifyRefs| \
                          ConsumesRC))                                  \
  OPC(IterNext,          (HasDest|CallsNative|MemEffects|MayModifyRefs))\
  OPC(IterNextK,         (HasDest|CallsNative|MemEffects|MayModifyRefs))\
                                                                        \
  /* vector instruction helpers */                                      \
  OPC(DefMIStateBase,    (HasDest))                                     \
  OPC(PropX,             (HasDest|Essential|MemEffects|CallsNative|     \
                          MayModifyRefs|MayRaiseError))                 \
  OPC(CGetProp,          (HasDest|Essential|MemEffects|CallsNative|     \
                          MayModifyRefs|MayRaiseError))                 \
  OPC(CGetElem,          (HasDest|Essential|MemEffects|CallsNative|     \
                          MayModifyRefs|MayRaiseError))                 \
                                                                        \
  /* misc */                                                            \
  OPC(IncStat,           (Essential|MemEffects))                        \
  OPC(DbgAssertRefCount, (Essential))                                   \
  OPC(Nop,               (NoFlags))                                     \
  /* */

enum Opcode : uint16_t {
#define OPC(name, flags) name,
  IR_OPCODES
#undef OPC
  IR_NUM_OPCODES
};

inline bool isCmpOp(Opcode opc) {
  return (opc >= OpGt && opc <= OpNSame);
}

// A "query op" is any instruction returning Type::Bool that is both
// branch-fusable and negateable.
inline bool isQueryOp(Opcode opc) {
  return (opc >= OpGt && opc <= IsNType);
}

inline bool isTypeQueryOp(Opcode opc) {
  return (opc == IsType || opc == IsNType);
}

inline Opcode queryToJmpOp(Opcode opc) {
  assert(isQueryOp(opc));
  return (Opcode)(JmpGt + (opc - OpGt));
}

inline bool isQueryJmpOp(Opcode opc) {
  return opc >= JmpGt && opc <= JmpIsNType;
}

inline Opcode queryJmpToQueryOp(Opcode opc) {
  assert(isQueryJmpOp(opc));
  return Opcode(OpGt + (opc - JmpGt));
}

/*
 * Right now branch fusion is too indiscriminate to handle fusing
 * with potentially expensive-to-repeat operations.  TODO(#2053369)
 */
inline bool disableBranchFusion(Opcode opc) {
  return opc == InstanceOf ||
         opc == NInstanceOf ||
         opc == InstanceOfBitmask ||
         opc == NInstanceOfBitmask;
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

const char* opcodeName(Opcode opcode);

class Type {
public:
  // This mostly parallels the DataType enum in runtime/base/types.h, but it's
  // not the same as DataType. See typeToDataType below.
  //    type name,    debug string
  #define IR_TYPES \
    IRT(None,            "None")  /* Corresponds to HPHP::TypeOfInvalid */ \
    IRT(Uninit,          "Unin")  \
    IRT(Null,            "Null")  \
    IRT(Bool,            "Bool")  \
    IRT(Int,             "Int") /* HPHP::DataType::KindOfInt64 */      \
    IRT(Dbl,             "Dbl")   \
    IRT(Placeholder,     "ERROR") /* Nothing in VM types enum at this position */ \
    IRT(StaticStr,       "Sstr")  \
    IRT(UncountedInit,   "UncountedInit") /* One of {Null,Bool,Int,Dbl,SStr} */\
    IRT(Uncounted,       "Uncounted") /* 1 of: {Unin,Null,Bool,Int,Dbl,SStr} */\
    IRT(Str,             "Str")   \
    IRT(Arr,             "Arr")   \
    IRT(Obj,             "Obj")   \
    IRT(Cell,            "Cell")  /* any type above, but statically unknown */ \
    /* Variant types */           \
    IRT(BoxedUninit,     "Unin&")  /* Unused */ \
    IRT(BoxedNull,       "Null&") \
    IRT(BoxedBool,       "Bool&") \
    IRT(BoxedInt,        "Int&")  /* HPHP::DataType::KindOfInt64 */ \
    IRT(BoxedDbl,        "Dbl&")  \
    IRT(BoxedPlaceholder,"ERROR") /* Nothing in VM types enum at this position */ \
    IRT(BoxedStaticStr,  "SStr&") \
    IRT(BoxedStr,        "Str&")  \
    IRT(BoxedArr,        "Arr&")  \
    IRT(BoxedObj,        "Obj&")  \
    IRT(BoxedCell,       "Cell&") /* any Boxed* types but statically unknown */ \
    IRT(Gen,             "Gen")   /* Generic type value, (cell or variant but statically unknown) */ \
    IRT(PtrToCell,       "Cell*") \
    IRT(PtrToGen,        "Gen*")  \
    IRT(Home,            "Home")  /* HPHP::DataType defines this as -2 */ \
    /* runtime metadata */        \
    IRT(ClassPtr,        "Cls*")   \
    IRT(FuncPtr,         "Func*") \
    IRT(VarEnvPtr,       "VarEnv*")\
    IRT(FuncClassPtr,    "FuncClass*") /* this has both a Func* and a Class* */\
    IRT(RetAddr,         "RetAddr") /* Return address */ \
    IRT(StkPtr,          "StkPtr") /* any pointer into VM stack: VmSP or VmFP */\
    IRT(TCA,             "TCA") \
    IRT(ActRec,          "ActRec") \
    /*  */

  enum Tag : uint16_t {
#define IRT(type, name)  type,
    IR_TYPES
#undef IRT
    TAG_ENUM_COUNT
  };

  static const Tag RefCountThreshold = Uncounted;

  static bool isBoxed(Tag t) {
    return (t > Cell && t < Gen);
  }

  static bool isUnboxed(Tag t) {
    return (t <= Type::Cell && t != Type::None);
  }

  static bool isPtr(Tag t) {
    return t == Type::PtrToCell || t == Type::PtrToGen;
  }

  static bool isRefCounted(Tag t) {
    return (t > RefCountThreshold && t <= Gen);
  }

  static bool isStaticallyKnown(Tag t) {
    return (t != Cell       &&
            t != Gen        &&
            t != Uncounted  &&
            t != UncountedInit);
  }

  static bool isStaticallyKnownUnboxed(Tag t) {
    return isStaticallyKnown(t) && isUnboxed(t);
  }

  static bool needsStaticBitCheck(Tag t) {
    return (t == Cell ||
            t == Gen  ||
            t == Str  ||
            t == Arr);
  }

  // returns true if definitely not uninitialized
  static bool isInit(Tag t) {
    return ((t != Uninit && isStaticallyKnown(t)) ||
            t == UncountedInit);
  }

  static bool mayBeUninit(Tag t) {
    return (t == Uninit    ||
            t == Uncounted ||
            t == Cell      ||
            t == Gen);
  }

  /*
   * Returns true if t1 is a strict subtype of t2.
   */
  static bool isMoreRefined(Tag t1, Tag t2) {
    return ((t2 == Gen           && t1 < Gen)                    ||
            (t2 == Cell          && t1 < Cell)                   ||
            (t2 == BoxedCell     && t1 < BoxedCell && t1 > Cell) ||
            (t2 == Str           && t1 == StaticStr)             ||
            (t2 == BoxedStr      && t1 == BoxedStaticStr)        ||
            (t2 == Uncounted     && t1 < Uncounted)              ||
            (t2 == UncountedInit && t1 < UncountedInit && t1 > Uninit));
  }

  /*
   * Returns true if t1 is a non-strict subtype of t2.
   */
  static bool subtypeOf(Tag t1, Tag t2) {
    return t1 == t2 || isMoreRefined(t1, t2);
  }

  /*
   * Returns the most refined of two types.
   *
   * Pre: the types must not be completely unrelated.
   */
  static Tag getMostRefined(Tag t1, Tag t2) {
    if (isMoreRefined(t1, t2)) return t1;
    if (isMoreRefined(t2, t1)) return t2;
    if (t1 == t2) return t1;
    always_assert(false);
  }

  static bool isString(Tag t) {
    return (t == Str || t == StaticStr);
  }

  static bool isNull(Tag t) {
    return (t == Null || t == Uninit);
  }

  static Tag getInnerType(Tag t) {
    assert(isBoxed(t));
    switch (t) {
      case BoxedUninit    : return Uninit;
      case BoxedNull      : return Null;
      case BoxedBool      : return Bool;
      case BoxedInt       : return Int;
      case BoxedDbl       : return Dbl;
      case BoxedStaticStr : return StaticStr;
      case BoxedStr       : return Str;
      case BoxedArr       : return Arr;
      case BoxedObj       : return Obj;
      case BoxedCell      : return Cell;
      default             : not_reached();
    }
  }

  static Tag box(Tag t) {
    if (t == None) {
      // translator-x64 sometimes gives us an inner type of KindOfInvalid and
      // an outer type of KindOfRef
      return BoxedCell;
    }
    if (t == Uninit) {
      return BoxedNull;
    }
    assert(isUnboxed(t));
    switch (t) {
      case Uninit     : return BoxedUninit;
      case Null       : return BoxedNull;
      case Bool       : return BoxedBool;
      case Int        : return BoxedInt;
      case Dbl        : return BoxedDbl;
      case StaticStr  : return BoxedStaticStr;
      case Str        : return BoxedStr;
      case Arr        : return BoxedArr;
      case Obj        : return BoxedObj;
      case Cell       : return BoxedCell;
      default         : not_reached();
    }
  }

  static Tag unbox(Tag t) {
    assert(t == Gen || isMoreRefined(t, Gen));
    if (isBoxed(t)) {
      return getInnerType(t);
    }
    if (t == Gen) return Cell;
    return t;
  }

  static Tag derefPtr(Tag t) {
    switch (t) {
      case PtrToCell  : return Cell;
      case PtrToGen   : return Gen;
      default         : not_reached();
    }
  }

  static const char* Strings[];

  // translates a compiler Type::Type to a HPHP::DataType
  static DataType toDataType(Tag type) {
    switch (type) {
      case None          : return KindOfInvalid;
      case Uninit        : return KindOfUninit;
      case Null          : return KindOfNull;
      case Bool          : return KindOfBoolean;
      case Int           : return KindOfInt64;
      case Dbl           : return KindOfDouble;
      case StaticStr     : return KindOfStaticString;
      case Str           : return KindOfString;
      case Arr           : return KindOfArray;
      case Obj           : return KindOfObject;
      case ClassPtr      : return KindOfClass;
      case UncountedInit : return KindOfUncountedInit;
      case Uncounted     : return KindOfUncounted;
      case Gen           : return KindOfAny;
      default: {
        assert(isBoxed(type));
        return KindOfRef;
      }
    }
  }

  static Tag fromDataType(DataType outerType,
                          DataType innerType = KindOfInvalid) {
    switch (outerType) {
      case KindOfInvalid       : return None;
      case KindOfUninit        : return Uninit;
      case KindOfNull          : return Null;
      case KindOfBoolean       : return Bool;
      case KindOfInt64         : return Int;
      case KindOfDouble        : return Dbl;
      case KindOfStaticString  : return StaticStr;
      case KindOfString        : return Str;
      case KindOfArray         : return Arr;
      case KindOfObject        : return Obj;
      case KindOfClass         : return ClassPtr;
      case KindOfUncountedInit : return UncountedInit;
      case KindOfUncounted     : return Uncounted;
      case KindOfAny           : return Gen;
      case KindOfRef           :
        return box(fromDataType(innerType, KindOfInvalid));
      default                  : not_reached();
    }
  }

  static Tag fromRuntimeType(const Transl::RuntimeType& rtt) {
    return fromDataType(rtt.outerType(), rtt.innerType());
  }

  static bool canRunDtor(Type::Tag t) {
    return (t == Obj       ||
            t == BoxedObj  ||
            t == Arr       ||
            t == BoxedArr  ||
            t == Cell      ||
            t == BoxedCell ||
            t == Gen);
  }
}; // class Type

bool cmpOpTypesMayReenter(Opcode, Type::Tag t0, Type::Tag t1);

class RawMemSlot {
 public:

  enum Kind {
    ContLabel, ContDone, ContShouldThrow, ContRunning, ContARPtr,
    StrLen, FuncNumParams, FuncRefBitVec, FuncBody, MisBaseStrOff,
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
      default: not_reached();
    }
  }

  int64 getOffset()   const { return m_offset; }
  int32 getSize()     const { return m_size; }
  Type::Tag getType() const { return m_type; }
  bool allowExtra()   const { return m_allowExtra; }

 private:
  RawMemSlot(int64 offset, int32 size, Type::Tag type, bool allowExtra = false)
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
    static RawMemSlot m(MISOFF(baseStrOff), sz::byte, Type::Bool, true);
    return m;
  }

  int64 m_offset;
  int32 m_size;
  Type::Tag m_type;
  bool m_allowExtra; // Used as a flag to ensure that extra offets are
                     // only used with RawMemSlots that support it
};

struct Local {
  explicit Local(uint32_t id) : m_id(id) {}
  uint32_t getId() const { return m_id; }
  void print(std::ostream& os) const {
    os << "h" << m_id;
  }
private:
  const uint32_t m_id;
};

class SSATmp;
class Trace;
class CodeGenerator;
class IRFactory;
class Simplifier;

bool isRefCounted(SSATmp* opnd);

class LabelInstruction;

typedef folly::Range<SSATmp**> SSARange;

/*
 * All IRInstruction subclasses must be arena-allocatable.
 * (Destructors are not called when they come from IRFactory.)
 */
struct IRInstruction {
  typedef std::list<IRInstruction*> List;
  typedef std::list<IRInstruction*>::iterator Iterator;
  typedef std::list<IRInstruction*>::reverse_iterator ReverseIterator;
  enum IId { kTransient = 0xffffffff };

  explicit IRInstruction(Opcode op,
                         uint32_t numSrcs = 0,
                         SSATmp** srcs = nullptr)
    : m_op(op)
    , m_typeParam(Type::None)
    , m_numSrcs(numSrcs)
    , m_iid(kTransient)
    , m_id(0)
    , m_srcs(srcs)
    , m_dst(nullptr)
    , m_asmAddr(nullptr)
    , m_label(nullptr)
    , m_parent(nullptr)
    , m_tca(nullptr)
  {}

  IRInstruction(const IRInstruction&) = delete;

  /*
   * Construct an IRInstruction as a deep copy of `inst', using
   * arena to allocate memory for its srcs/dests.
   */
  explicit IRInstruction(Arena& arena, const IRInstruction* inst,
                         IId iid);

  /*
   * Replace an instruction in place with a Nop.  This sometimes may
   * be a result of a simplification pass.
   */
  void convertToNop();

  /*
   * Deep-copy an IRInstruction, using factory to allocate memory for
   * the IRInstruction itself, and its srcs/dests.
   */
  virtual IRInstruction* clone(IRFactory* factory) const;

  Opcode     getOpcode()   const       { return m_op; }
  void       setOpcode(Opcode newOpc)  { m_op = newOpc; }
  Type::Tag  getTypeParam() const      { return m_typeParam; }
  void       setTypeParam(Type::Tag t) { m_typeParam = t; }
  uint32_t   getNumSrcs()  const       { return m_numSrcs; }
  void       setNumSrcs(uint32_t i)    {
    assert(i <= m_numSrcs);
    m_numSrcs = i;
  }
  SSATmp*    getSrc(uint32 i) const;
  void       setSrc(uint32 i, SSATmp* newSrc);
  void       appendSrc(Arena&, SSATmp*);
  SSARange   getSrcs() const {
    return SSARange(m_srcs, m_numSrcs);
  }
  SSATmp*    getDst()      const       { return m_dst; }
  void       setDst(SSATmp* newDst)    { m_dst = newDst; }
  SSARange   getDsts() {
    return SSARange(&m_dst, m_dst ? 1 : 0);
  }
  folly::Range<SSATmp* const*> getDsts() const {
    return folly::makeRange(&m_dst, m_dst ? (&m_dst) + 1 : &m_dst);
  }
  TCA        getTCA()      const       { return m_tca; }
  void       setTCA(TCA    newTCA)     { m_tca = newTCA; }

  /*
   * An instruction's 'id' has different meanings depending on the
   * compilation phase.
   */
  uint32     getId()       const       { return m_id; }
  void       setId(uint32 newId)       { m_id = newId; }

  /*
   * Instruction id (iid) is stable and useful as an array index.
   */
  uint32     getIId()      const       {
    assert(m_iid != kTransient);
    return m_iid;
  }

  void       setAsmAddr(void* addr)    { m_asmAddr = addr; }
  void*      getAsmAddr() const        { return m_asmAddr; }
  RegSet     getLiveOutRegs() const    { return m_liveOutRegs; }
  void       setLiveOutRegs(RegSet s)  { m_liveOutRegs = s; }
  bool       isDefConst() const {
    return (m_op == DefConst || m_op == LdHome);
  }
  Trace*     getParent() const { return m_parent; }
  void       setParent(Trace* parentTrace) { m_parent = parentTrace; }

  void setLabel(LabelInstruction* l) { m_label = l; }
  LabelInstruction* getLabel() const { return m_label; }
  bool isControlFlowInstruction() const { return m_label != NULL; }

  virtual bool isConstInstruction() const { return false; }
  virtual bool equals(IRInstruction* inst) const;
  virtual size_t hash() const;
  virtual void print(std::ostream& ostream) const;
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
  bool hasMemEffects() const;
  bool isRematerializable() const;
  bool isNative() const;
  bool consumesReferences() const;
  bool consumesReference(int srcNo) const;
  bool producesReference() const;
  bool mayModifyRefs() const;
  bool mayRaiseError() const;
  bool isEssential() const;

  void printDst(std::ostream& ostream) const;
  void printSrc(std::ostream& ostream, uint32 srcIndex) const;
  void printOpcode(std::ostream& ostream) const;
  void printSrcs(std::ostream& ostream) const;

private:
  bool mayReenterHelper() const;

private:
  Opcode            m_op;
  Type::Tag         m_typeParam;
  uint16            m_numSrcs;
  const IId         m_iid;
  uint32            m_id;
  SSATmp**          m_srcs;
  RegSet            m_liveOutRegs;
  SSATmp*           m_dst;
  void*             m_asmAddr;
  LabelInstruction* m_label;
  Trace*            m_parent;
  TCA               m_tca;
};

class ConstInstruction : public IRInstruction {
public:
  ConstInstruction(Opcode opc, Type::Tag t) : IRInstruction(opc) {
    assert(opc == DefConst || opc == LdConst);
    setTypeParam(t);
    m_intVal = 0;
  }

  template<class T>
  ConstInstruction(Opcode opc,
                   T val,
                   typename std::enable_if<
                     std::is_integral<T>::value && !std::is_same<T,bool>::value
                   >::type* = 0)
    : IRInstruction(opc)
  {
    assert(opc == DefConst || opc == LdConst);
    setTypeParam(Type::Int);
    m_intVal = val;
  }

  ConstInstruction(Opcode opc, double val) : IRInstruction(opc) {
    assert(opc == DefConst || opc == LdConst);
    setTypeParam(Type::Dbl);
    m_dblVal = val;
  }
  ConstInstruction(Opcode opc, const StringData* val) : IRInstruction(opc) {
    assert(opc == DefConst || opc == LdConst);
    setTypeParam(Type::StaticStr);
    m_strVal = val;
  }
  ConstInstruction(Opcode opc, const ArrayData* val) : IRInstruction(opc) {
    assert(opc == DefConst || opc == LdConst);
    setTypeParam(Type::Arr);
    m_arrVal = val;
  }
  ConstInstruction(Opcode opc, bool val) : IRInstruction(opc) {
    assert(opc == DefConst || opc == LdConst);
    setTypeParam(Type::Bool);
    m_intVal = 0;
    m_boolVal = val;
  }
  ConstInstruction(uint32_t numSrcs, SSATmp** srcs, Local l)
    : IRInstruction(LdHome, numSrcs, srcs)
  {
    setTypeParam(Type::Home);
    new (&m_local) Local(l);
  }
  ConstInstruction(Opcode opc, const Func* f) : IRInstruction(opc) {
    assert(opc == DefConst || opc == LdConst);
    setTypeParam(Type::FuncPtr);
    m_func = f;
  }
  ConstInstruction(Opcode opc, const Class* f) : IRInstruction(opc) {
    assert(opc == DefConst || opc == LdConst);
    setTypeParam(Type::ClassPtr);
    m_clss = f;
  }
  ConstInstruction(Opcode opc, TCA tca) : IRInstruction(opc) {
    assert(opc == DefConst || opc == LdConst);
    setTypeParam(Type::TCA);
    m_tca = tca;
  }
  explicit ConstInstruction(Arena& arena, const ConstInstruction* inst, IId iid)
    : IRInstruction(arena, inst, iid)
    , m_strVal(inst->m_strVal) {
  }

  bool getValAsBool() const {
    assert(getTypeParam() == Type::Bool);
    return m_boolVal;
  }
  int64 getValAsInt() const {
    assert(getTypeParam() == Type::Int);
    return m_intVal;
  }
  int64 getValAsRawInt() const {
    return m_intVal;
  }
  double getValAsDbl() const {
    assert(getTypeParam() == Type::Dbl);
    return m_dblVal;
  }
  const StringData* getValAsStr() const {
    assert(getTypeParam() == Type::StaticStr);
    return m_strVal;
  }
  const ArrayData* getValAsArr() const {
    assert(getTypeParam() == Type::Arr);
    return m_arrVal;
  }
  const Func* getValAsFunc() const {
    assert(getTypeParam() == Type::FuncPtr);
    return m_func;
  }
  const Class* getValAsClass() const {
    assert(getTypeParam() == Type::ClassPtr);
    return m_clss;
  }
  const VarEnv* getValAsVarEnv() const {
    assert(getTypeParam() == Type::VarEnvPtr);
    return m_varEnv;
  }
  TCA getValAsTCA() const {
    assert(getTypeParam() == Type::TCA);
    return m_tca;
  }
  bool isEmptyArray() const {
    return m_arrVal == HphpArray::GetStaticEmptyArray();
  }
  Local getLocal() const {
    assert(getTypeParam() == Type::Home);
    return m_local;
  }
  uintptr_t getValAsBits() const { return m_bits; }

  void printConst(std::ostream& ostream) const;
  virtual bool isConstInstruction() const {return true;}
  virtual void print(std::ostream& ostream) const;
  virtual bool equals(IRInstruction* inst) const;
  virtual size_t hash() const;
  virtual IRInstruction* clone(IRFactory* factory) const;

private:
  union {
    uintptr_t         m_bits;
    bool              m_boolVal;
    int64             m_intVal;
    double            m_dblVal;
    const StringData* m_strVal;
    const ArrayData*  m_arrVal;
    Local             m_local; // for LdHome opcode
    const Func*       m_func;
    const Class*      m_clss;
    const VarEnv*     m_varEnv;
    TCA               m_tca;
  };
};

class LabelInstruction : public IRInstruction {
public:
  explicit LabelInstruction(uint32 id, const Func* func)
    : IRInstruction(DefLabel)
    , m_labelId(id)
    , m_patchAddr(0)
    , m_func(func)
  {}

  explicit LabelInstruction(Arena& arena, const LabelInstruction* inst, IId iid)
    : IRInstruction(arena, inst, iid)
    , m_labelId(inst->m_labelId)
    , m_patchAddr(0)
    , m_func(inst->m_func)
  {}

  uint32      getLabelId() const  { return m_labelId; }
  const Func* getFunc() const     { return m_func; }

  virtual void print(std::ostream& ostream) const;
  virtual bool equals(IRInstruction* inst) const;
  virtual size_t hash() const;
  virtual IRInstruction* clone(IRFactory* factory) const;

  void    prependPatchAddr(TCA addr);
  void*   getPatchAddr();

private:
  const uint32 m_labelId;
  void*  m_patchAddr; // Support patching forward jumps
  const Func* m_func; // which func are we in
};

class MarkerInstruction : public IRInstruction {
public:
  MarkerInstruction(uint32 bcOff, const Func* f, int32 spOff)
    : IRInstruction(Marker)
    , m_bcOff(bcOff)
    , m_stackOff(spOff)
    , m_func(f)
  {}

  explicit MarkerInstruction(Arena& arena, const MarkerInstruction* inst,
                             IId iid)
    : IRInstruction(arena, inst, iid)
    , m_bcOff(inst->m_bcOff)
    , m_stackOff(inst->m_stackOff)
    , m_func(inst->m_func)
  {}

  uint32      getBcOff() const    { return m_bcOff; }
  int32       getStackOff() const { return m_stackOff; }
  const Func* getFunc() const     { return m_func; }

  virtual void print(std::ostream& ostream) const;
  virtual bool equals(IRInstruction* inst) const;
  virtual size_t hash() const;
  virtual IRInstruction* clone(IRFactory* factory) const;

private:
  uint32      m_bcOff;    // the bytecode offset in unit
  int32       m_stackOff; // stack off from start of trace
  const Func* m_func;     // which func are we in
};

/*
 * Return the output type from a given IRInstruction.
 *
 * The destination type is always predictable from the types of the
 * inputs and any type parameters to the instruction.
 */
Type::Tag outputType(const IRInstruction*);

/*
 * Assert that an instruction has operands of allowed types.
 */
void assertOperandTypes(const IRInstruction*);

struct SpillInfo {
  enum Type { MMX, Memory };

  explicit SpillInfo(RegNumber r) : m_type(MMX), m_val(int(r)) {}
  explicit SpillInfo(uint32_t v)  : m_type(Memory), m_val(v) {}

  Type      type() const { return m_type; }
  RegNumber mmx()  const { return RegNumber(m_val); }
  uint32_t  mem()  const { return m_val; }

private:
  Type     m_type : 1;
  uint32_t m_val : 31;
};

inline std::ostream& operator<<(std::ostream& os, SpillInfo si) {
  switch (si.type()) {
  case SpillInfo::MMX:
    os << "mmx" << reg::regname(RegXMM(int(si.mmx())));
    break;
  case SpillInfo::Memory:
    os << "spill[" << si.mem() << "]";
    break;
  }
  return os;
}

class SSATmp {
public:
  uint32            getId() const { return m_id; }
  IRInstruction*    getInstruction() const { return m_inst; }
  void              setInstruction(IRInstruction* i) { m_inst = i; }
  Type::Tag         getType() const { return m_type; }
  void              setType(Type::Tag t) { m_type = t; }
  uint32            getLastUseId() { return m_lastUseId; }
  void              setLastUseId(uint32 newId) { m_lastUseId = newId; }
  uint32            getUseCount() { return m_useCount; }
  void              setUseCount(uint32 count) { m_useCount = count; }
  void              incUseCount() { m_useCount++; }
  uint32            decUseCount() { return --m_useCount; }
  bool              isConst() const { return m_inst->isConstInstruction(); }
  bool              isBoxed() const { return Type::isBoxed(getType()); }
  bool              isString() const { return isA(Type::Str); }
  bool              getConstValAsBool() const;
  int64             getConstValAsInt() const;
  int64             getConstValAsRawInt() const;
  double            getConstValAsDbl() const;
  const StringData* getConstValAsStr() const;
  const ArrayData*  getConstValAsArr() const;
  const Func*       getConstValAsFunc() const;
  const Class*      getConstValAsClass() const;
  uintptr_t         getConstValAsBits() const;
  TCA               getConstValAsTCA() const;
  void              print(std::ostream& ostream,
                          bool printLastUse = false) const;
  void              print() const;

  // Used for Jcc to Jmp elimination
  void              setTCA(TCA tca);
  TCA               getTCA() const;

  /*
   * Returns: Type::subtypeOf(getType(), tag).
   *
   * This should be used for most checks on the types of IRInstruction
   * sources.
   */
  bool isA(Type::Tag tag) const {
    return Type::subtypeOf(getType(), tag);
  }

  /*
   * Returns whether or not a given register index is allocated to a
   * register, or returns false if it is spilled.
   *
   * Right now, we only spill both at the same time and only Spill and
   * Reload instructions need to deal with SSATmps that are spilled.
   */
  bool hasReg(uint32 i = 0) const {
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
  PhysReg     getReg() const { assert(!m_isSpilled); return m_regs[0]; }
  PhysReg     getReg(uint32 i) const { assert(!m_isSpilled); return m_regs[i]; }
  void        setReg(PhysReg reg, uint32 i) { m_regs[i] = reg; }

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
  SSATmp(uint32 opndId, IRInstruction* i)
    : m_inst(i)
    , m_id(opndId)
    , m_type(outputType(i))
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
  const uint32    m_id;
  Type::Tag       m_type; // type when defined
  uint32          m_lastUseId;
  uint16          m_useCount;
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

class Trace : boost::noncopyable {
public:
  explicit Trace(LabelInstruction* label, uint32 bcOff, bool isMain) {
    appendInstruction(label);
    m_bcOff = bcOff;
    m_lastAsmAddress = NULL;
    m_firstAsmAddress = NULL;
    m_firstAstubsAddress = NULL;
    m_isMain = isMain;
  }

  ~Trace() {
    std::for_each(m_exitTraces.begin(), m_exitTraces.end(),
                  boost::checked_deleter<Trace>());
  }

  IRInstruction::List& getInstructionList() {
    return m_instructionList;
  }
  LabelInstruction* getLabel() const {
    return (LabelInstruction*)*m_instructionList.begin();
  }
  IRInstruction* prependInstruction(IRInstruction* inst) {
    // jump over the trace's label
    std::list<IRInstruction*>::iterator it = m_instructionList.begin();
    ++it;
    m_instructionList.insert(it, inst);
    inst->setParent(this);
    return inst;
  }
  IRInstruction* appendInstruction(IRInstruction* inst) {
    m_instructionList.push_back(inst);
    inst->setParent(this);
    return inst;
  }

  uint32 getBcOff() { return m_bcOff; }
  void setLastAsmAddress(uint8* addr) { m_lastAsmAddress = addr; }
  void setFirstAsmAddress(uint8* addr) { m_firstAsmAddress = addr; }
  void setFirstAstubsAddress(uint8* addr) { m_firstAstubsAddress = addr; }
  uint8* getLastAsmAddress() { return m_lastAsmAddress; }
  uint8* getFirstAsmAddress() { return m_firstAsmAddress; }
  Trace* addExitTrace(Trace* exit) {
    m_exitTraces.push_back(exit);
    return exit;
  }
  bool isMain() const { return m_isMain; }

  typedef std::list<Trace*> List;
  typedef std::list<Trace*>::iterator Iterator;

  List& getExitTraces() { return m_exitTraces; }
  void print(std::ostream& ostream, bool printAsm,
             bool isExit = false) const;
  void print() const;  // default to std::cout and printAsm == true

private:
  // offset of the first bytecode in this trace; 0 if this trace doesn't
  // represent a bytecode boundary.
  uint32 m_bcOff;
  // instructions in main trace starting with a label
  std::list<IRInstruction*> m_instructionList;
  // traces to which this trace exits
  List m_exitTraces;
  uint8* m_firstAsmAddress;
  uint8* m_firstAstubsAddress;
  uint8* m_lastAsmAddress;
  bool m_isMain;
};

/*
 * Some utility micro-passes used from other major passes.
 */
void numberInstructions(Trace*);
uint32 numberInstructions(Trace* trace,
                          uint32 nextId,
                          bool followControlFlow = true);

/*
 * Remove any instruction whose id field == DEAD
 */
void removeDeadInstructions(Trace* trace);

/*
 * Remove any instruction if live[iid] == false
 */
void removeDeadInstructions(Trace* trace, const boost::dynamic_bitset<>& live);

/*
 * Clears the IRInstructions' ids, and the SSATmps' use count and last use id
 * for the given trace and all its exit traces.
 */
void resetIds(Trace* trace);

int getLocalIdFromHomeOpnd(SSATmp* srcHome);

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
constexpr int kSpillStackActRecExtraArgs = 4;

inline bool isConvIntOrPtrToBool(IRInstruction* instr) {
  if (!(instr->getOpcode() == Conv &&
        instr->getTypeParam() == Type::Bool)) {
    return false;
  }

  switch (instr->getSrc(0)->getType()) {
    case Type::Int          :
    case Type::FuncPtr      :
    case Type::ClassPtr     :
    case Type::FuncClassPtr :
    case Type::VarEnvPtr    :
    case Type::TCA          :
      return true;
    default:
      return false;
  }
}

}}}

namespace std {
  template<> struct hash<HPHP::VM::JIT::Opcode> {
    size_t operator()(HPHP::VM::JIT::Opcode op) const { return op; }
  };
  template<> struct hash<HPHP::VM::JIT::Type::Tag> {
    size_t operator()(HPHP::VM::JIT::Type::Tag t) const { return t; }
  };
}

#endif
