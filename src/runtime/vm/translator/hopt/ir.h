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

#ifndef _IR_H_
#define _IR_H_

#include <boost/noncopyable.hpp>
#include <algorithm>
#include <boost/checked_delete.hpp>
#include <stdarg.h>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <stack>
#include <list>
#include <assert.h>
#include "runtime/vm/translator/asm-x64.h"
#include "runtime/vm/translator/types.h"
#include "runtime/vm/translator/runtime-type.h"
#include "runtime/base/types.h"
#include "runtime/vm/func.h"
#include "runtime/vm/class.h"
#include "util/arena.h"

namespace HPHP {
// forward declaration
class StringData;
namespace VM {
namespace JIT {

using HPHP::VM::Transl::TCA;
using HPHP::VM::Transl::register_name_t;

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

#define PUNT(instr) do {                             \
  throw FailedIRGen(__FILE__, __LINE__, __func__);   \
  } while(0)

// XXX TODO: define another namespace for opcodes
// TODO: Make sure the MayModRefs column is correct... (MayRaiseError too...)
//
// (A) Opcode Name
//
// (B) Has Dest: whether or not the instruction produces a result.
//     This is sometimes the same value as their input, for example AddElem
//     to array, which returns the array which it took as an input.
//
// (C) Can CSE: whether or not the instruction is safe to elide through
//     common subexpression elimination.
//
// (D) Is Essential: whether or not the instruction is essential. Non-essential
//     instructions may be eliminated by optimizations.
//
// (E) hasMemEffects: if true, then this instruction cannot be rolled
//     back because it has side effects on memory. Use for setting
//     lastExitSlowInvalid in trace builder.
//
// (F) isNativeHelper: This instruction calls out to a native helper.
//     The register allocator uses this to optimize register spills
//     around native calls and to bias register assignment for arguments
//     and return values.
//
// (G) consumesRefCount: if true, then this instruction decrefs its
//     sources. Some native helpers do this. When generating this
//     instruction, we need to incref its source operands before
//     passing them to this instruction.
//
// (H) producesRefCount: if true, then this instruction produces an
//     incref'ed value. Some native helpers do this. We need to decref
//     the value produces by this instruction after its last use.
//
// (I) MayModifyRefs: if true, this instruction may modify references/variants,
//     either directly or indirectly by reentering the VM.
//     This requires the JIT to be pessimistic about live references.
//
// (J) Is Rematerializable: if true, this instruction is rematerializable.
//
// (K) May Raise Error: whether this instruction has a helper than can raise an
//     error (has an implicit exit edge)
#define IR_OPCODES                         \
  /* checks */                             \
  OPC(GuardType,         1,  1,  1,  0,  0,  0,  0,  0,  0,  0)               \
  OPC(GuardRefs,         0,  1,  1,  0,  0,  0,  0,  0,  0,  0)/* XXX validate */  \
                                           \
  /* arith ops (integer) */                \
  OPC(OpAdd,             1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(OpSub,             1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(OpAnd,             1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(OpOr,              1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(OpXor,             1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(OpMul,             1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
                                           \
  /* convert from src operand's type to destination type */ \
  OPC(Conv,              1,  1,  0,  0,  1,  0,  0,  0,  0,  0) \
                                           \
  /* query operators returning bool */     \
  /* comparisons (binary) */               \
  OPC(OpGt,              1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(OpGte,             1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(OpLt,              1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(OpLte,             1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(OpEq,              1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(OpNeq,             1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  /* XXX validate that we don't call helpers with side effects */   \
  /* and ref count consumption/production for any of these query */ \
  /* operations and their corresponding conditional branches */     \
  OPC(OpSame,            1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(OpNSame,           1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
                                                            \
  /* XXX TODO check instanceof's hasEffects, isNative, RefCount, MayReenter */\
  OPC(InstanceOfD,       1,  1,  0,  0,  0,  0,  0,  0,  0,  0)               \
  OPC(NInstanceOfD,      1,  1,  0,  0,  0,  0,  0,  0,  0,  0)               \
                                                            \
  /* isset, empty, and istype queries (unary) */            \
  OPC(IsSet,             1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(IsType,            1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(IsNSet,            1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(IsNType,           1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
                                                            \
  /* conditional branches & jump */                         \
  /* there is a conditional branch for each of the above query */        \
  /* operators to enable generating efficieng comparison-and-branch */   \
  /* instruction sequences */                               \
  OPC(JmpGt,             1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(JmpGte,            1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(JmpLt,             1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(JmpLte,            1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(JmpEq,             1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(JmpNeq,            1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(JmpZero,           1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(JmpNZero,          1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(JmpSame,           1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(JmpNSame,          1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
    /* keep preceeding conditional branches contiguous */       \
  OPC(JmpInstanceOfD,    1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(JmpNInstanceOfD,   1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(JmpIsSet,          1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(JmpIsType,         1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(JmpIsNSet,         1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(JmpIsNType,        1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(Jmp_,              1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(ExitWhenSurprised, 0,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(ExitOnVarEnv,      0,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(CheckUninit,       0,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
                                           \
  OPC(Unbox,             1,  0,  0,  0,  0,  0,  1,  0,  0,  0) \
  OPC(Box,               1,  0,  1,  1,  1,  1,  1,  0,  0,  0) \
  OPC(UnboxPtr,          1,  0,  0,  0,  0,  0,  0,  0,  0,  0) \
                                            \
  /* loads */                               \
  OPC(LdStack,           1,  0,  0,  0,  0,  0,  1,  0,  0,  0) \
  OPC(LdLoc,             1,  0,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(LdStackAddr,       1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(LdLocAddr,         1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(LdMemNR,           1,  0,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(LdPropNR,          1,  0,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(LdRefNR,           1,  0,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(LdThis,            1,  1,  1,  0,  0,  0,  0,  0,  1,  0) \
  /* LdThisNc avoids check */                               \
  OPC(LdThisNc,          1,  1,  0,  0,  0,  0,  0,  0,  1,  0) \
  OPC(LdVarEnv,          1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(LdRetAddr,         1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(LdHome,            1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(LdConst,           1,  1,  0,  0,  0,  0,  0,  0,  1,  0) \
  OPC(DefConst,          1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(LdCls,             1,  1,  0,  0,  0,  0,  0,  1,  1,  1) \
  /* XXX cg doesn't support the version without a label*/   \
  OPC(LdClsCns,          1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(LdClsMethod,       1,  1,  0,  0,  0,  0,  0,  0,  1,  1) \
  /* XXX TODO Create version of LdClsPropAddr that doesn't check */ \
  OPC(LdPropAddr,        1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(LdClsPropAddr,     1,  1,  1,  0,  0,  0,  0,  0,  1,  1) \
  /* helper call to MethodCache::Lookup */                  \
  OPC(LdObjMethod,       1,  1,  1,  0,  1,  0,  0,  1,  0,  1) \
  OPC(LdObjClass,        1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(LdCachedClass,     1,  1,  0,  0,  0,  0,  0,  0,  1,  0) \
  /* helper call to FuncCache::lookup */                    \
  OPC(LdFunc,            1,  0,  1,  0,  1,  1,  0,  0,  0,  1) \
  /* helper call for FPushFuncD(FixedFuncCache::lookup) */  \
  OPC(LdFixedFunc,       1,  1,  1,  0,  0,  0,  0,  0,  0,  1) \
  OPC(LdCurFuncPtr,      1,  1,  0,  0,  0,  0,  0,  0,  1,  0) \
  OPC(LdARFuncPtr,       1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(LdFuncCls,         1,  1,  0,  0,  0,  0,  0,  0,  0,  1) \
  OPC(NewObj,            1,  0,  1,  1,  1,  0,  1,  0,  0,  0) \
  OPC(LdRaw,             1,  0,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(AllocActRec,       1,  0,  0,  1,  0,  1,  0,  0,  0,  0) \
  OPC(FreeActRec,        1,  0,  0,  1,  0,  0,  0,  0,  0,  0) \
  OPC(Call,              1,  0,  1,  1,  0,  1,  0,  1,  0,  0) \
  OPC(NativeImpl,        0,  0,  1,  1,  1,  0,  0,  1,  0,  0) \
  /* return control to caller */                            \
  OPC(RetCtrl,           0,  0,  1,  1,  0,  0,  0,  0,  0,  0) \
  /* transfer return value from callee to caller; update sp */ \
  OPC(RetVal,            1,  0,  0,  1,  0,  1,  0,  0,  0,  0) \
  /* stores */                              \
  OPC(StMem,             0,  0,  1,  1,  0,  1,  0,  1,  0,  0) \
  OPC(StMemNT,           0,  0,  1,  1,  0,  1,  0,  0,  0,  0) \
  OPC(StProp,            0,  0,  1,  1,  0,  1,  0,  1,  0,  0) \
  OPC(StPropNT,          0,  0,  1,  1,  0,  1,  0,  0,  0,  0) \
  OPC(StLoc,             0,  0,  1,  1,  0,  1,  0,  0,  0,  0) \
  OPC(StLocNT,           0,  0,  1,  1,  0,  1,  0,  0,  0,  0) \
  OPC(StRef,             1,  0,  1,  1,  0,  1,  0,  1,  0,  0) \
  OPC(StRefNT,           1,  0,  1,  1,  0,  1,  0,  0,  0,  0) \
  OPC(StRaw,             0,  0,  1,  1,  0,  0,  0,  0,  0,  0) \
  OPC(SpillStack,        1,  0,  1,  1,  0,  1,  0,  0,  0,  0) \
  OPC(SpillStackAllocAR, 1,  0,  1,  1,  0,  1,  0,  0,  0,  0) \
  /* Update ExitTrace entries in sync with ExitType below */    \
  OPC(ExitTrace,         0,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(ExitTraceCc,       0,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(ExitSlow,          0,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(ExitSlowNoProgress,0,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(ExitGuardFailure,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  /* */                                                     \
  OPC(Mov,               1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(IncRef,            1,  0,  0,  1,  0,  0,  1,  0,  0,  0) \
  OPC(DecRefLoc,         0,  0,  1,  1,  0,  0,  0,  1,  0,  0) \
  OPC(DecRefStack,       0,  0,  1,  1,  0,  0,  0,  1,  0,  0) \
  OPC(DecRefThis,        0,  0,  1,  1,  0,  0,  0,  1,  0,  0) \
  OPC(DecRefLocals,      0,  0,  1,  1,  1,  0,  0,  1,  0,  0) \
  OPC(DecRefLocalsThis,  0,  0,  1,  1,  1,  0,  0,  1,  0,  0) \
  OPC(DecRef,            0,  0,  1,  1,  0,  1,  0,  1,  0,  0) \
  /* DecRefNZ only decrements the ref count, and doesn't modify Refs. */ \
  /* DecRefNZ also doesn't run dtor, so we mark it as non-essential. */ \
  OPC(DecRefNZ,          0,  0,  0,  1,  0,  1,  0,  0,  0,  0) \
  OPC(DefLabel,          0,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(Marker,            0,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(DefFP,             1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(DefSP,             1,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
                                            \
  /* runtime helpers */                     \
 /* XXX check consume ref count */ \
  OPC(RaiseUninitWarning,0,  0,  1,  1,  1,  0,  0,  1,  0,  1) \
  OPC(Print,             0,  0,  1,  1,  1,  1,  0,  0,  0,  0) \
  OPC(AddElem,           1,  0,  0,  1,  1,  1,  1,  1,  0,  0) \
  OPC(AddNewElem,        1,  0,  0,  1,  1,  1,  1,  0,  0,  0) \
  OPC(DefCns,            1,  1,  1,  1,  1,  0,  0,  0,  0,  0) \
  OPC(Concat,            1,  0,  0,  1,  1,  1,  1,  1,  0,  0) \
  OPC(ArrayAdd,          1,  0,  0,  1,  1,  1,  1,  0,  0,  0) \
  OPC(DefCls,            0,  1,  1,  0,  1,  0,  0,  0,  0,  0) \
  OPC(DefFunc,           0,  1,  1,  0,  1,  0,  0,  0,  0,  0) \
                                                        \
  OPC(InterpOne,         1,  0,  1,  1,  1,  0,  0,  1,  0,  1) \
  /* for register allocation */                         \
  OPC(Spill,             1,  0,  0,  1,  0,  0,  0,  0,  0,  0) \
  OPC(Reload,            1,  0,  0,  1,  0,  0,  0,  0,  0,  0) \
  OPC(AllocSpill,        0,  0,  1,  1,  0,  0,  0,  0,  0,  0) \
  OPC(FreeSpill,         0,  0,  1,  1,  0,  0,  0,  0,  0,  0) \
  /* continuation support */                                    \
  OPC(LdContThisOrCls,   1,  1,  0,  0,  0,  0,  0,  0,  0,  0) \
  OPC(CreateCont,        1,  0,  1,  1,  1,  0,  1,  0,  0,  0) \
  OPC(FillContLocals,    0,  0,  1,  1,  1,  0,  0,  0,  0,  0) \
  OPC(FillContThis,      0,  0,  1,  1,  0,  0,  0,  0,  0,  0) \
  OPC(UnpackCont,        1,  0,  1,  1,  1,  0,  0,  0,  0,  0) \
  OPC(ExitOnContVars,    0,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(PackCont,          0,  0,  1,  1,  1,  0,  0,  0,  0,  0) \
  OPC(ContRaiseCheck,    0,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(ContPreNext,       0,  0,  1,  1,  0,  0,  0,  0,  0,  0) \
  OPC(ContStartedCheck,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0) \
  OPC(AssertRefCount,    0,  0,  1,  0,  0,  0,  0,  0,  0,  0) \

enum Opcode {
#define OPC(name, hasDst, canCSE, essential, hasMemEffects, isNative, \
            consumesRefCount, producesRefCount, mayModRefs, rematerializable, \
            error) \
  name,
  IR_OPCODES
  #undef OPC
  /* */
  IR_NUM_OPCODES
};


static inline bool isCmpOp(Opcode opc) {
  return (opc >= OpGt && opc <= NInstanceOfD);
}

static inline Opcode cmpToJmpOp(Opcode opc) {
  ASSERT(isCmpOp(opc));
  return (Opcode)(JmpGt + (opc - OpGt));
}

static inline bool isQueryOp(Opcode opc) {
  return (opc >= OpGt && opc <= IsNType);
}

static inline bool isTypeQueryOp(Opcode opc) {
  return (opc == IsType || opc == IsNType);
}
static inline Opcode queryToJmpOp(Opcode opc) {
  ASSERT(isQueryOp(opc));
  return (Opcode)(JmpGt + (opc - OpGt));
}

extern Opcode queryNegateTable[];

static inline Opcode negateQueryOp(Opcode opc) {
  ASSERT(isQueryOp(opc));
  return queryNegateTable[opc - OpGt];
}

extern Opcode queryCommuteTable[];

static inline Opcode commuteQueryOp(Opcode opc) {
  ASSERT(opc >= OpGt && opc <= OpNSame);
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

extern const char* OpcodeStrings[];

class Type {
public:
  // This mostly parallels the DataType enum in runtime/base/types.h, but it's
  // not the same as DataType. See typeToDataType below.
  //    type name,    debug string
  #define IR_TYPES \
    IRT(None,            "Void")  /* Corresponds to HPHP::TypeOfInvalid */ \
    IRT(Uninit,          "Unin")  \
    IRT(Null,            "Null")  \
    IRT(Bool,            "Bool")  \
    IRT(Int,             "Int")   /* HPHP::DataType::KindOfInt64 */ \
    IRT(Dbl,             "Dbl")   \
    IRT(Placeholder,     "ERROR") /* Nothing in VM types enum at this position */ \
    IRT(StaticStr,       "Sstr")  \
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
    IRT(ClassRef,        "Cls&")   \
    IRT(FuncRef,         "Func&") \
    IRT(VarEnvRef,       "VarEnv&")\
    IRT(FuncClassRef,    "FuncClass&") /* this has both a Func& and a Class& */\
    IRT(SP,              "StkP")  /* any pointer into VM stack: VmSP or VmFP */\
    IRT(TCA,             "TCA")                                         \

  enum Tag {
    #define IRT(type, name)  type,
    IR_TYPES
    #undef IRT
    TAG_ENUM_COUNT
  };

  static inline bool isBoxed(Tag t) {
    return (t > Cell && t < Gen);
  }

  static inline bool isUnboxed(Tag t) {
    return (t <= Type::Cell && t != Type::None);
  }

  static inline bool isRefCounted(Tag t) {
    return t >= Type::Str && t <= Type::Gen;
  }

  static inline bool isStaticallyKnown(Tag t) {
    return t != Type::Cell && t != Type::Gen && t != Type::None;
  }

  static inline bool isStaticallyKnownUnboxed(Tag t) {
    return isUnboxed(t) && t != Type::Cell;
  }

  static inline bool needsStaticBitCheck(Tag t) {
    return (t == Type::Cell || t == Type::Gen ||
            t == Type::Str  || t == Type::Arr);
  }

  static inline bool isInit(Tag t) {
    // returns true if definitely not uninitialized
    return ((t > Type::Uninit && t < Type::Cell) || isBoxed(t));
  }
  // returns true if t1 is a more refined that t2
  static inline bool isMoreRefined(Tag t1, Tag t2) {
    return ((t2 == Gen && t1 < Gen) ||
            (t2 == Cell && t1 < Cell) ||
            (t2 == BoxedCell && t1 < BoxedCell && t1 > Cell) ||
            (t2 == Str && t1 == StaticStr) ||
            (t2 == BoxedStr && t1 == BoxedStaticStr));
  }
  static inline bool isString(Tag t) {
    return t == Type::Str || t == Type::StaticStr;
  }
  static inline bool isNull(Tag t) {
    return t == Type::Null || t == Type::Uninit;
  }
  static inline Tag getInnerType(Tag t) {
    if (!isBoxed(t)) {
      return None;
    }
    return (Tag)(t - BoxedUninit + Uninit);
  }

  static inline Tag getBoxedType(Tag t) {
    if (t == None) {
      // translator-x64 sometimes gives us an inner type of KindOfInvalid and
      // an outer type of KindOfRef
      return BoxedCell;
    }
    if (t == Uninit) {
      return BoxedNull;
    }
    ASSERT(isUnboxed(t));
    return (Tag)(BoxedUninit + (t - Uninit));
  }

  static inline Tag getValueType(Tag t) {
    if (isBoxed(t)) {
      return getInnerType(t);
    }
    return t;
  }

  static const char* Strings[];

  // translates a compiler Type::Type to a HPHP::DataType
  static inline DataType toDataType(Tag type) {
    if (type == Type::ClassRef) {
      return KindOfClass;
    }
    if (isBoxed(type)) {
      return KindOfRef;
    }
    ASSERT(type < Type::Cell);
    return (DataType)(type - 1);
  }

  static inline Tag fromDataType(DataType outerType, DataType innerType) {
    switch (outerType) {
      case KindOfClass   : return Type::ClassRef;
      case KindOfRef     : return getBoxedType(fromDataType(innerType,
                                                            KindOfInvalid));
      default            : return (Tag)(outerType + 1);
    }
  }

  static inline Tag fromRuntimeType(const Transl::RuntimeType& rtt) {
    return fromDataType(rtt.outerType(), rtt.innerType());
  }

  static inline bool canRunDtor(Type::Tag ty) {
    return ty == Type::Obj || ty == Type::BoxedObj ||
           ty == Type::Arr || ty == Type::BoxedArr ||
           !Type::isStaticallyKnown(ty);
  }
}; // class Type

// Must be arena-allocatable.  (Destructor is not called.)
class Local {
public:
  Local(uint32 i) : m_id(i) {}
  uint32 getId() {return m_id;}
  void print(std::ostream& os) {
    os << "h" << m_id;
  }
private:
  const uint32 m_id;
};

class SSATmp;
class Trace;
class CodeGenerator;
class IRFactory;
class Simplifier;

bool isRefCounted(SSATmp* opnd);

const unsigned NUM_FIXED_SRCS = 2;

class LabelInstruction;

// All IRInstruction subclasses must be arena-allocatable.
// (Destructors are not called.)
class IRInstruction {
public:
  Opcode     getOpcode()   const       { return (Opcode)m_op; }
  void       setOpcode(Opcode newOpc)  { m_op = newOpc; }
  Type::Tag  getType()     const       { return (Type::Tag)m_type; }
  void       setType(Type::Tag t)      { m_type = t; }
  uint32     getNumSrcs()  const       { return m_numSrcs; }
  uint32     getNumExtendedSrcs() const{
    return (m_numSrcs <= NUM_FIXED_SRCS ? 0 : m_numSrcs - NUM_FIXED_SRCS);
  }
  SSATmp*    getSrc(uint32 i) const;
  void       setSrc(uint32 i, SSATmp* newSrc);
  SSATmp*    getDst()      const       { return m_dst; }
  void       setDst(SSATmp* newDst)    { m_dst = newDst; }
  TCA        getTCA()      const       { return m_tca; }
  void       setTCA(TCA    newTCA)     { m_tca = newTCA; }
  uint32     getId()       const       { return m_id; }
  void       setId(uint32 newId)       { m_id = newId; }
  void       setAsmAddr(void* addr)    { m_asmAddr = addr; }
  void*      getAsmAddr()              { return m_asmAddr; }
  uint16     getLiveOutRegs() const    { return m_liveOutRegs; }
  void       setLiveOutRegs(uint16 s)  { m_liveOutRegs = s; }
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
  virtual uint32 hash();
  virtual SSATmp* simplify(Simplifier*);
  virtual void print(std::ostream& ostream);
  void print();
  virtual void genCode(CodeGenerator* cg);
  virtual IRInstruction* clone(IRFactory* factory);
  typedef std::list<IRInstruction*> List;
  typedef std::list<IRInstruction*>::iterator Iterator;
  typedef std::list<IRInstruction*>::reverse_iterator ReverseIterator;
  static  bool canCSE(Opcode opc);
  virtual bool canCSE() const { return IRInstruction::canCSE(getOpcode()); }
  static  bool hasDst(Opcode opc);
  virtual bool hasDst() const { return IRInstruction::hasDst(getOpcode()); }
  static  bool isRematerializable(Opcode opc);
  virtual bool isRematerializable() const {
    return IRInstruction::isRematerializable(getOpcode());
  }
  static  bool hasMemEffects(Opcode opc);
  virtual bool hasMemEffects() const {
    return IRInstruction::hasMemEffects(getOpcode());
  }
  static  bool isNative(Opcode opc);
  virtual bool isNative()  const {
    return IRInstruction::isNative(getOpcode());
  }
  static  bool consumesReferences(Opcode opc);
  virtual bool consumesReferences() const {
    return IRInstruction::consumesReferences(getOpcode());
  }
  bool consumesReference(int srcNo) const;
  static  bool producesReference(Opcode opc);
  virtual bool producesReference()  const {
    return IRInstruction::producesReference(getOpcode());
  }
  static  bool mayModifyRefs(Opcode opc);
  virtual bool mayModifyRefs()  const {
    Opcode opc = getOpcode();
    // DecRefNZ does not have side effects other than decrementing the ref
    // count. Therefore, its MayModifyRefs should be false.
    if (opc == DecRef) {
      if (this->isControlFlowInstruction() || Type::isString(m_type)) {
        // If the decref has a target label, then it exits if the destructor
        // has to be called, so it does not have any side effects on the main
        // trace.
        return false;
      }
      if (Type::isBoxed(m_type)) {
        Type::Tag innerType = Type::getInnerType(m_type);
        return (innerType == Type::Obj || innerType == Type::Arr);
      }
    }
    return IRInstruction::mayModifyRefs(opc);
  }

  void printDst(std::ostream& ostream);
  void printSrc(std::ostream& ostream, uint32 srcIndex);
  void printOpcode(std::ostream& ostream);
  void printSrcs(std::ostream& ostream);

protected:
  friend class IRFactory;
  friend class TraceBuilder;
  friend class HhbcTranslator;
  friend class MemMap;

 public:
  IRInstruction(Opcode o, Type::Tag t, LabelInstruction *l = NULL)
      : m_op(o), m_type(t), m_id(0), m_numSrcs(0),
        m_liveOutRegs(0), m_dst(NULL), m_asmAddr(NULL), m_label(l),
        m_parent(NULL), m_tca(NULL)
  {
    m_srcs[0] = m_srcs[1] = NULL;
  }

  IRInstruction(Opcode o, Type::Tag t, SSATmp* src, LabelInstruction *l = NULL)
      : m_op(o), m_type(t), m_id(0),  m_numSrcs(1),
        m_liveOutRegs(0), m_dst(NULL), m_asmAddr(NULL), m_label(l),
        m_parent(NULL), m_tca(NULL)
  {
    m_srcs[0] = src; m_srcs[1] = NULL;
  }

  IRInstruction(Opcode o,
                Type::Tag t,
                SSATmp* src0,
                SSATmp* src1,
                LabelInstruction *l = NULL)
      : m_op(o), m_type(t), m_id(0), m_numSrcs(2),
        m_liveOutRegs(0), m_dst(NULL), m_asmAddr(NULL), m_label(l),
        m_parent(NULL), m_tca(NULL)
  {
    m_srcs[0] = src0; m_srcs[1] = src1;
  }

  IRInstruction(IRInstruction* inst)
      : m_op(inst->m_op),
        m_type(inst->m_type),
        m_id(0),
        m_numSrcs(inst->m_numSrcs),
        m_liveOutRegs(0),
        m_dst(NULL),
        m_asmAddr(NULL),
        m_label(inst->m_label),
        m_parent(NULL), m_tca(NULL)
  {
    m_srcs[0] = inst->m_srcs[0];
    m_srcs[1] = inst->m_srcs[1];
  }

  virtual SSATmp* getExtendedSrc(uint32 i) const;
  virtual void    setExtendedSrc(uint32 i, SSATmp* newSrc);

  // fields
  Opcode            m_op;
  Type::Tag         m_type;
  uint32            m_id;
  uint16            m_numSrcs;
  uint16            m_liveOutRegs;
  SSATmp*           m_srcs[NUM_FIXED_SRCS];
  SSATmp*           m_dst;
  void*             m_asmAddr;
  LabelInstruction* m_label;
  Trace*            m_parent;
  TCA               m_tca;
};

class ExtendedInstruction : public IRInstruction {
public:
  virtual SSATmp* simplify(Simplifier*);
  virtual IRInstruction* clone(IRFactory* factory);
  virtual void genCode(CodeGenerator* cg);

  virtual SSATmp* getExtendedSrc(uint32 i) const;
  virtual void    setExtendedSrc(uint32 i, SSATmp* newSrc);
  void appendExtendedSrc(IRFactory& irFactory, SSATmp* src);

  virtual SSATmp** getExtendedSrcs() { return m_extendedSrcs; }
protected:
  friend class IRFactory;
  friend class TraceBuilder;
  ExtendedInstruction(IRFactory& irFactory,
                      Opcode o,
                      Type::Tag t,
                      uint32   nOpnds,
                      SSATmp** opnds,
                      LabelInstruction* l = NULL)
      : IRInstruction(o, t, l) {
    initExtendedSrcs(irFactory, nOpnds, opnds);
  }
  ExtendedInstruction(IRFactory& irFactory,
                      Opcode o,
                      Type::Tag t,
                      SSATmp* src,
                      uint32 nOpnds,
                      SSATmp** opnds,
                      LabelInstruction* l = NULL)
      : IRInstruction(o, t, src, l) {
    initExtendedSrcs(irFactory, nOpnds, opnds);
  }
  ExtendedInstruction(IRFactory& irFactory,
                      Opcode o,
                      Type::Tag t,
                      SSATmp* src1,
                      SSATmp* src2,
                      uint32 nOpnds,
                      SSATmp** opnds,
                      LabelInstruction* l = NULL)
      : IRInstruction(o, t, src1, src2, l) {
    initExtendedSrcs(irFactory, nOpnds, opnds);
  }
  ExtendedInstruction(IRFactory& irFactory,
                      Opcode o,
                      Type::Tag t,
                      SSATmp* src1,
                      SSATmp* src2,
                      SSATmp* src3,
                      uint32 nOpnds,
                      SSATmp** opnds,
                      LabelInstruction* l = NULL)
      : IRInstruction(o, t, src1, src2, l) {
    initExtendedSrcs(irFactory, src3, nOpnds, opnds);
  }

  ExtendedInstruction(IRFactory& irFactory,
                      ExtendedInstruction* inst)
    : IRInstruction(inst)
  {
    if (m_numSrcs < NUM_FIXED_SRCS) {
      return;
    }
    uint32 nOpnds = m_numSrcs - NUM_FIXED_SRCS;
    m_numSrcs = NUM_FIXED_SRCS; // will be incremented in initExtendedSrcs
    // Only operands after NUM_FIXED_SRCS are kept in inst->extendedSrcs
    initExtendedSrcs(irFactory, nOpnds, inst->m_extendedSrcs);
  }
  void initExtendedSrcs(IRFactory&, uint32 nOpnds, SSATmp** opnds);
  void initExtendedSrcs(IRFactory&, SSATmp* src, uint32 nOpnds,
                        SSATmp** opnds);


  SSATmp** m_extendedSrcs;
};

class TypeInstruction : public IRInstruction {
public:
  Type::Tag   getSrcType() { return m_srcType; }
  virtual void print(std::ostream& ostream);
  virtual void genCode(CodeGenerator* cg);
  virtual bool equals(IRInstruction* inst) const;
  virtual uint32 hash();
  virtual SSATmp* simplify(Simplifier*);
  virtual IRInstruction* clone(IRFactory* factory);
protected:
  friend class IRFactory;
  friend class TraceBuilder;
  TypeInstruction(Opcode o, Type::Tag typ,
                  Type::Tag dstType, SSATmp* src)
      : IRInstruction(o, dstType, src), m_srcType(typ)
  {
  }
  TypeInstruction(TypeInstruction* inst)
      : IRInstruction(inst), m_srcType(inst->m_srcType) {
  }
  Type::Tag m_srcType;
};

class ConstInstruction : public IRInstruction {
public:
  bool getValAsBool() {
    ASSERT(m_type == Type::Bool);
    return m_boolVal;
  }
  int64 getValAsInt() {
    ASSERT(m_type == Type::Int);
    return m_intVal;
  }
  int64 getValAsRawInt() {
    return m_intVal;
  }
  double getValAsDbl() {
    ASSERT(m_type == Type::Dbl);
    return m_dblVal;
  }
  const StringData* getValAsStr() {
    ASSERT(m_type == Type::StaticStr);
    return m_strVal;
  }
  const ArrayData* getValAsArr() {
    ASSERT(m_type == Type::Arr);
    return m_arrVal;
  }
  const Func* getValAsFunc() {
    ASSERT(m_type == Type::FuncRef);
    return m_func;
  }
  const Class* getValAsClass() {
    ASSERT(m_type == Type::ClassRef);
    return m_clss;
  }
  const VarEnv* getValAsVarEnv() {
    ASSERT(m_type == Type::VarEnvRef);
    return m_varEnv;
  }
  const TCA getValAsTCA() {
    ASSERT(m_type == Type::TCA);
    return m_tca;
  }
  const bool isEmptyArray() {
    return m_arrVal == HphpArray::GetStaticEmptyArray();
  }
  Local* getLocal() {
    ASSERT(m_type == Type::Home);
    return m_local;
  }
  uintptr_t getValAsBits() { return m_bits; }

  void printConst(std::ostream& ostream);
  virtual bool isConstInstruction() const {return true;}
  virtual void print(std::ostream& ostream);
  virtual void genCode(CodeGenerator* cg);
  virtual bool equals(IRInstruction* inst) const;
  virtual uint32 hash();
  virtual IRInstruction* clone(IRFactory* factory);
 protected:
  friend class IRFactory;
  friend class TraceBuilder;
  ConstInstruction(Opcode opc, Type::Tag type, int64 val) :
      IRInstruction(opc, type) {
    ASSERT(opc == DefConst || opc == LdConst);
    ASSERT(type <= Type::StaticStr);
    m_intVal = val;
  }
  ConstInstruction(Opcode opc, Type::Tag t) : IRInstruction(opc, t) {
    ASSERT(opc == DefConst || opc == LdConst);
    m_intVal = 0;
  }
  ConstInstruction(Opcode opc, int64 val) : IRInstruction(opc, Type::Int) {
    ASSERT(opc == DefConst || opc == LdConst);
    m_intVal = val;
  }
  ConstInstruction(Opcode opc, double val) : IRInstruction(opc, Type::Dbl) {
    ASSERT(opc == DefConst || opc == LdConst);
    m_dblVal = val;
  }
  ConstInstruction(Opcode opc, const StringData* val)
      : IRInstruction(opc, Type::StaticStr) {
    ASSERT(opc == DefConst || opc == LdConst);
    m_strVal = val;
  }
  ConstInstruction(Opcode opc, const ArrayData* val)
      : IRInstruction(opc, Type::Arr) {
    ASSERT(opc == DefConst || opc == LdConst);
    m_arrVal = val;
  }
  ConstInstruction(Opcode opc, bool val) : IRInstruction(opc, Type::Bool) {
    ASSERT(opc == DefConst || opc == LdConst);
    m_intVal = 0;
    m_boolVal = val;
  }
  ConstInstruction(SSATmp* src, Local* l)
      : IRInstruction(LdHome, Type::Home, src) {
    m_local = l;
  }
  ConstInstruction(Opcode opc, const Func* f)
      : IRInstruction(opc, Type::FuncRef) {
    ASSERT(opc == DefConst || opc == LdConst);
    m_func = f;
  }
  ConstInstruction(Opcode opc, const Class* f)
    : IRInstruction(opc, Type::ClassRef) {
    ASSERT(opc == DefConst || opc == LdConst);
    m_clss = f;
  }
  ConstInstruction(ConstInstruction* inst)
      : IRInstruction(inst), m_strVal(inst->m_strVal) {
  }
private:
  union {
    uintptr_t         m_bits;
    bool              m_boolVal;
    int64             m_intVal;
    double            m_dblVal;
    const StringData* m_strVal;
    const ArrayData*  m_arrVal;
    Local*            m_local; // for LdHome opcode
    const Func*       m_func;
    const Class*      m_clss;
    const VarEnv*     m_varEnv;
    const TCA         m_tca;
  };
  friend class CodeGenerator;
  void setValAsRawInt(int64 intVal) {
    m_intVal = intVal;
  }
};

class LabelInstruction : public IRInstruction {
public:
  Trace*      getTrace() const    { return m_trace; }
  void        setTrace(Trace* t)  { m_trace = t; }
  uint32      getLabelId() const  { return m_labelId; }
  int32       getStackOff() const { return m_stackOff; }
  const Func* getFunc() const     { return m_func; }

  virtual void genCode(CodeGenerator* cg);
  virtual void print(std::ostream& ostream);
  virtual bool equals(IRInstruction* inst) const;
  virtual uint32 hash();
  virtual SSATmp* simplify(Simplifier*);
  virtual IRInstruction* clone(IRFactory* factory);

  void    prependPatchAddr(TCA addr);
  void*   getPatchAddr();

protected:
  friend class IRFactory;
  friend class TraceBuilder;
  friend class CodeGenerator;
  LabelInstruction(uint32 id) : IRInstruction(DefLabel, Type::None),
                                m_labelId(id),
                                m_stackOff(0),
                                m_patchAddr(0),
                                m_trace(NULL) {
  }
  LabelInstruction(Opcode opc, uint32 id) : IRInstruction(opc,Type::None),
                                            m_labelId(id),
                                            m_stackOff(0),
                                            m_patchAddr(0),
                                            m_trace(NULL) {
  }

  LabelInstruction(Opcode opc, uint32 bcOff, const Func* f, int32 spOff)
      : IRInstruction(opc,Type::None),
        m_labelId(bcOff),
        m_stackOff(spOff),
        m_patchAddr(0),
        m_func(f)
  {
  }

  LabelInstruction(LabelInstruction* inst)
      : IRInstruction(inst),
        m_labelId(inst->m_labelId),
        m_stackOff(inst->m_stackOff),
        m_patchAddr(0),
        m_trace(inst->m_trace) // copies func also
  {
  }
  uint32 m_labelId;  // for Marker instructions: the bytecode offset in unit
  int32  m_stackOff; // for Marker instructions: stack off from start of trace
  void*  m_patchAddr; // Support patching forward jumps
  union {
    Trace* m_trace;     // for DefLabel instructions
    const Func* m_func; // for Marker instructions
  };
};

class SSATmp {
public:
  uint32            getId() const { return m_id; }
  IRInstruction*    getInstruction() const { return m_inst; }
  Type::Tag         getType() const { return m_inst->getType(); }
  uint32            getLastUseId() { return m_lastUseId; }
  void              setLastUseId(uint32 newId) { m_lastUseId = newId; }
  uint32            getUseCount() { return m_useCount; }
  void              setUseCount(uint32 count) { m_useCount = count; }
  void              incUseCount() { m_useCount++; }
  uint32            decUseCount() { return --m_useCount; }
  int32             getAnalysisValue() { return m_analysis; }
  void              setAnalysisValue(int val) { m_analysis = val; }
  uint32            getNumAssignedLocs() const;
  bool              isAssignedReg(uint32 index) const;
  bool              isAssignedMmxReg(uint32 index) const;
  bool              isAssignedSpillLoc(uint32 index) const;
  register_name_t   getAssignedLoc() { return m_assignedLoc[0]; }
  register_name_t   getAssignedLoc(uint32 index);
  void              setAssignedLoc(register_name_t loc, uint32 index) {
    m_assignedLoc[index] = loc;
  }
  uint32            getSpillLoc(uint32 index) const;
  void              setSpillLoc(uint32 spillLoc, uint32 index);
  register_name_t   getMmxReg(uint32 index) const;
  void              setMmxReg(register_name_t mmxReg, uint32 index);
  bool              isConst() const { return m_inst->isConstInstruction(); }
  bool              getConstValAsBool();
  int64             getConstValAsInt();
  int64             getConstValAsRawInt();
  double            getConstValAsDbl();
  const StringData* getConstValAsStr();
  const ArrayData*  getConstValAsArr();
  const Func*       getConstValAsFunc();
  const Class*      getConstValAsClass();
  const Local*      getConstValAsLocal();
  uintptr_t         getConstValAsBits();
  void              print(std::ostream& ostream, bool printLastUse = false);
  void              print();
  static const uint32 MaxNumAssignedLoc = 2;

  // Used for Jcc to Jmp elimination
  void              setTCA(TCA tca);
  TCA               getTCA();

private:
  friend class IRFactory;
  friend class TraceBuilder;
  friend class LinearScan;

  // May only be created via IRFactory.  Note that this class is never
  // destructed, so don't add complex members.
  SSATmp(uint32 opndId, IRInstruction* i) : m_inst(i),
                                            m_id(opndId),
                                            m_lastUseId(0),
                                            m_useCount(0) {
    m_assignedLoc[0] = m_assignedLoc[1] = Transl::reg::noreg;
    m_analysis = -1;
  }

  IRInstruction*  m_inst;
  const uint32    m_id;
  uint32          m_lastUseId;
  uint16          m_useCount;
  // m_analysis is a scratch field that various analysis & optimization
  // passes (e.g., the register allocator) can use
  // for register spilling:
  //   -1: not spilled
  //   otherwise: spilled slot
  int32           m_analysis;
  // assignedLoc[0] is always the value of this tmp and for
  // Cell or Gen types, assignedLoc[1] is the runtime type
  // loc < LinearScan::NumRegs: general purpose registers
  // LinearScan::NumRegs <= loc < LinearScan::FirstSpill: MMX registers
  // LinearScan::FistSpill <= loc: spill location
  register_name_t   m_assignedLoc[MaxNumAssignedLoc]; // register allocation
};

class IRFactory {
public:
  IRFactory() : m_nextLabelId(0), m_nextOpndId(0) {}

  IRInstruction* cloneInstruction(IRInstruction* inst);
  ExtendedInstruction* cloneInstruction(ExtendedInstruction* inst);
  ConstInstruction* cloneInstruction(ConstInstruction* inst);
  TypeInstruction* cloneInstruction(TypeInstruction* inst);
  LabelInstruction* cloneInstruction(LabelInstruction* inst);

  IRInstruction* guardRefs(SSATmp* funcPtr,
                           SSATmp* nParams,
                           SSATmp* bitsPtr,
                           SSATmp* firstBitNum,
                           SSATmp* mask64,
                           SSATmp* vals64,
                           LabelInstruction* exitLabel = NULL);

  IRInstruction* ldLoc(SSATmp* home);
  IRInstruction* ldLoc(SSATmp* home,
                       Type::Tag type,
                       LabelInstruction*);
  ConstInstruction* defConst(int64 val);
  IRInstruction* verifyParamType(SSATmp* src,
                                 SSATmp* tc, SSATmp* constraint,
                                 LabelInstruction*);
  IRInstruction* spillStack(SSATmp* sp,
                            SSATmp* stackAdjustment,
                            uint32 numOpnds,
                            SSATmp** opnds,
                            bool allocActRec = false);
  IRInstruction* exitTrace(TraceExitType::ExitType,
                           SSATmp* func,
                           SSATmp* pc,
                           SSATmp* sp,
                           SSATmp* fp);
  IRInstruction* exitTrace(TraceExitType::ExitType,
                           SSATmp* func,
                           SSATmp* pc,
                           SSATmp* sp,
                           SSATmp* fp,
                           SSATmp* notTakenPC);
  IRInstruction* allocActRec(SSATmp* stkPtr,
                             SSATmp* framePtr,
                             SSATmp* func,
                             SSATmp* objOrCls,
                             SSATmp* numArgs,
                             SSATmp* magicName);
  IRInstruction* freeActRec(SSATmp* framePtr);
  IRInstruction* call(SSATmp* actRec,
                      SSATmp* returnBcOffset,
                      SSATmp* func,
                      uint32 numArgs,
                      SSATmp** args);
  IRInstruction* incRef(SSATmp* obj);
  IRInstruction* decRef(SSATmp* obj, LabelInstruction* dtorLabel);
  LabelInstruction* defLabel();
  LabelInstruction* marker(uint32 bcOff, const Func* func, int32 spOff);
  IRInstruction* decRefLoc(SSATmp* home, LabelInstruction* exit);
  IRInstruction* decRefStack(Type::Tag type,
                             SSATmp* sp,
                             SSATmp* index,
                             LabelInstruction* exit);
  IRInstruction* decRefThis(SSATmp* fp, LabelInstruction* exit);
  IRInstruction* decRefLocalsThis(SSATmp* fp, SSATmp* numLocals);
  IRInstruction* decRefLocals(SSATmp* fp, SSATmp* numLocals);

  IRInstruction* retVal(SSATmp* fp, SSATmp* val);
  IRInstruction* retVal(SSATmp* fp);
  IRInstruction* retCtrl(SSATmp* sp, SSATmp* fp, SSATmp* retAddr);
  template<Type::Tag T> TypeInstruction* isType(SSATmp* src) {
    return new TypeInstruction(IsType, T, Type::Bool, src);
  }
  IRInstruction* spill(SSATmp* src);
  IRInstruction* reload(SSATmp* slot);
  IRInstruction* allocSpill(SSATmp* numSlots);
  IRInstruction* freeSpill(SSATmp* numSlots);

  Local* getLocal(uint32 id) {
    if (id >= m_locals.size()) {
      m_locals.resize(id+1);
    }
    Local* opnd = m_locals[id];
    if (opnd == NULL) {
      m_locals[id] = opnd = new (m_arena) Local(id);
    }
    return opnd;
  }

  SSATmp* getSSATmp(IRInstruction* inst) {
    SSATmp* tmp = new (m_arena) SSATmp(m_nextOpndId++, inst);
    inst->setDst(tmp);
    return tmp;
  }

  uint32 getNumLocals() { return m_locals.size(); }
  uint32 getNumSSATmps() { return m_nextOpndId; }

  Arena& arena() { return m_arena; }

private:
  uint32 m_nextLabelId;
  uint32 m_nextOpndId;
  std::vector<Local*> m_locals;

  // SSATmp, IRInstruction, and Local objects are allocated here.
  Arena m_arena;
};

class Trace : boost::noncopyable {
public:
  explicit Trace(LabelInstruction* label, uint32 bcOff, bool isMain) {
    appendInstruction(label);
    label->setTrace(this);
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
  void print(std::ostream& ostream, bool printAsm, bool isExit = false);
  void print();  // default to std::cout and printAsm == true

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

void optimizeTrace(Trace*, IRFactory* irFactory);
void eliminateDeadCode(Trace*);
void numberInstructions(Trace*);
uint32 numberInstructions(Trace* trace,
                          uint32 nextId,
                          bool followControlFlow = true);
/*
 * Clears the IRInstructions' ids, and the SSATmps' use count and last use id.
 */
void resetIds(Trace* trace);
int getLocalIdFromHomeOpnd(SSATmp* srcHome);

static inline bool isConvIntOrPtrToBool(IRInstruction* instr) {
  if (!(instr->getOpcode() == Conv &&
        instr->getType() == Type::Bool)) {
    return false;
  }

  switch (instr->getSrc(0)->getType()) {
    case Type::Int          :
    case Type::FuncRef      :
    case Type::ClassRef     :
    case Type::FuncClassRef :
    case Type::VarEnvRef    :
    case Type::TCA          :
      return true;
    default:
      return false;
  }
}

}}}

#endif
