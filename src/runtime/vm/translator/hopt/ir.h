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
#include <boost/noncopyable.hpp>
#include <boost/checked_delete.hpp>
#include "util/asm-x64.h"
#include "util/arena.h"
#include "runtime/ext/ext_continuation.h"
#include "runtime/vm/translator/physreg.h"
#include "runtime/vm/translator/abi-x64.h"
#include "runtime/vm/translator/types.h"
#include "runtime/vm/translator/runtime-type.h"
#include "runtime/base/types.h"
#include "runtime/vm/func.h"
#include "runtime/vm/class.h"

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
 * Flags on opcodes.  See ir.specification for details on the meaning
 * of these flags.
 */
enum OpcodeFlag : uint64_t {
  HasDest          = 0x0001,
  CanCSE           = 0x0002,
  Essential        = 0x0004,
  MemEffects       = 0x0008,
  CallsNative      = 0x0010,
  ConsumesRC       = 0x0020,
  ProducesRC       = 0x0040,
  MayModifyRefs    = 0x0080,
  Rematerializable = 0x0100, // TODO: implies HasDest
  MayRaiseError    = 0x0200, // TODO: should it imply CallsNative?
};

#define IR_OPCODES                                                      \
  /* checks */                                                          \
  OPC(GuardType,         (HasDest|CanCSE|Essential))                    \
  OPC(GuardLoc,          (Essential))                                   \
  OPC(GuardStk,          (Essential))                                   \
  OPC(GuardRefs,         (CanCSE|Essential))                            \
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
  /* query operators returning bool */                                  \
  /* comparisons (binary) */                                            \
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
  /* XXX TODO check instanceof's hasEffects, isNative, RefCount, MayReenter */ \
  OPC(InstanceOfD,       (HasDest|CanCSE))                              \
  OPC(NInstanceOfD,      (HasDest|CanCSE))                              \
                                                                        \
  /* isset, empty, and istype queries (unary) */                        \
  OPC(IsSet,             (HasDest|CanCSE))                              \
  OPC(IsType,            (HasDest|CanCSE))                              \
  OPC(IsNSet,            (HasDest|CanCSE))                              \
  OPC(IsNType,           (HasDest|CanCSE))                              \
                                                                        \
  /* conditional branches & jump */                                     \
  /* there is a conditional branch for each of the above query */       \
  /* operators to enable generating efficieng comparison-and-branch */  \
  /* instruction sequences */                                           \
  OPC(JmpGt,             (HasDest|Essential))                           \
  OPC(JmpGte,            (HasDest|Essential))                           \
  OPC(JmpLt,             (HasDest|Essential))                           \
  OPC(JmpLte,            (HasDest|Essential))                           \
  OPC(JmpEq,             (HasDest|Essential))                           \
  OPC(JmpNeq,            (HasDest|Essential))                           \
  OPC(JmpZero,           (HasDest|Essential))                           \
  OPC(JmpNZero,          (HasDest|Essential))                           \
  OPC(JmpSame,           (HasDest|Essential))                           \
  OPC(JmpNSame,          (HasDest|Essential))                           \
    /* keep preceeding conditional branches contiguous */               \
  OPC(JmpInstanceOfD,    (HasDest|Essential))                           \
  OPC(JmpNInstanceOfD,   (HasDest|Essential))                           \
  OPC(JmpIsSet,          (HasDest|Essential))                           \
  OPC(JmpIsType,         (HasDest|Essential))                           \
  OPC(JmpIsNSet,         (HasDest|Essential))                           \
  OPC(JmpIsNType,        (HasDest|Essential))                           \
  OPC(Jmp_,              (HasDest|Essential))                           \
  OPC(ExitWhenSurprised, (Essential))                                   \
  OPC(ExitOnVarEnv,      (Essential))                                   \
  OPC(CheckUninit,       (Essential))                                   \
                                                                        \
  OPC(Unbox,             (HasDest|ProducesRC))                          \
  OPC(Box,               (HasDest|Essential|MemEffects|                 \
                          CallsNative|ConsumesRC|                       \
                          ProducesRC))                                  \
  OPC(UnboxPtr,          (HasDest))                                     \
                                                                        \
  /* loads */                                                           \
  OPC(LdStack,           (HasDest|ProducesRC))                          \
  OPC(LdLoc,             (HasDest))                                     \
  OPC(LdStackAddr,       (HasDest|CanCSE))                              \
  OPC(LdLocAddr,         (HasDest|CanCSE))                              \
  OPC(LdMemNR,           (HasDest))                                     \
  OPC(LdPropNR,          (HasDest))                                     \
  OPC(LdRefNR,           (HasDest))                                     \
  OPC(LdThis,            (HasDest|CanCSE|Essential|                     \
                          Rematerializable))                            \
  /* LdThisNc avoids check */                                           \
  OPC(LdThisNc,          (HasDest|CanCSE|Rematerializable))             \
  OPC(LdVarEnv,          (HasDest|CanCSE))                              \
  OPC(LdRetAddr,         (HasDest))                                     \
  OPC(LdHome,            (HasDest|CanCSE))                              \
  OPC(LdConst,           (HasDest|CanCSE|Rematerializable))             \
  OPC(DefConst,          (HasDest|CanCSE))                              \
  OPC(LdCls,             (HasDest|CanCSE|MayModifyRefs|                 \
                          Rematerializable|MayRaiseError))              \
  /* XXX cg doesn't support the version without a label*/               \
  OPC(LdClsCns,          (HasDest|CanCSE))                              \
  OPC(LdClsMethodCache,  (HasDest|CanCSE|                               \
                          Rematerializable|MayRaiseError))              \
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
  OPC(NewObj,            (HasDest|Essential|MemEffects|                 \
                          CallsNative|ProducesRC))                      \
  OPC(NewArray,          (HasDest|Essential|MemEffects|                 \
                          CallsNative|ProducesRC))                      \
  OPC(NewTuple,          (HasDest|Essential|MemEffects|                 \
                          CallsNative|ConsumesRC|ProducesRC))           \
  OPC(LdRaw,             (HasDest))                                     \
                          /* XXX: why does AllocActRec consume rc? */   \
  OPC(AllocActRec,       (HasDest|MemEffects|ConsumesRC))               \
  OPC(FreeActRec,        (HasDest|MemEffects))                          \
  OPC(Call,              (HasDest|Essential|MemEffects|                 \
                          ConsumesRC|MayModifyRefs))                    \
  OPC(NativeImpl,        (Essential|MemEffects|CallsNative|             \
                          MayModifyRefs))                               \
  /* return control to caller */                                        \
  OPC(RetCtrl,           (Essential|MemEffects))                        \
  /* transfer return value from callee to caller; update sp */          \
  OPC(RetVal,            (HasDest|MemEffects|ConsumesRC))               \
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
  OPC(SpillStackAllocAR, (HasDest|Essential|MemEffects|                 \
                          ConsumesRC))                                  \
  /* Update ExitTrace entries in sync with ExitType below */            \
  OPC(ExitTrace,         (Essential))                                   \
  OPC(ExitTraceCc,       (Essential))                                   \
  OPC(ExitSlow,          (Essential))                                   \
  OPC(ExitSlowNoProgress,(Essential))                                   \
  OPC(ExitGuardFailure,  (Essential))                                   \
  /* */                                                                 \
  OPC(Mov,               (HasDest|CanCSE))                              \
  OPC(IncRef,            (HasDest|MemEffects|ProducesRC))               \
  OPC(DecRefLoc,         (Essential|MemEffects|MayModifyRefs))          \
  OPC(DecRefStack,       (Essential|MemEffects|MayModifyRefs))          \
  OPC(DecRefThis,        (Essential|MemEffects|MayModifyRefs))          \
  OPC(DecRefLocals,      (Essential|MemEffects|CallsNative|             \
                          MayModifyRefs))                               \
  OPC(DecRefLocalsThis,  (Essential|MemEffects|CallsNative|             \
                          MayModifyRefs))                               \
  OPC(DecRef,            (Essential|MemEffects|ConsumesRC|              \
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
                         /* XXX: shouldn't DefCls/DefFunc MayRaiseError? */ \
  OPC(DefCls,            (CanCSE|Essential|CallsNative))                \
  OPC(DefFunc,           (CanCSE|Essential|CallsNative))                \
                                                                        \
  OPC(InterpOne,         (HasDest|Essential|MemEffects|                 \
                          CallsNative|MayModifyRefs|                    \
                          MayRaiseError))                               \
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
  OPC(UnlinkContVarEnv,  (Essential|MemEffects|CallsNative))            \
  OPC(LinkContVarEnv,    (Essential|MemEffects|CallsNative))            \
  OPC(ContRaiseCheck,    (Essential))                                   \
  OPC(ContPreNext,       (Essential|MemEffects))                        \
  OPC(ContStartedCheck,  (Essential))                                   \
                                                                        \
  OPC(IncStat,           (Essential|MemEffects))                        \
  OPC(AssertRefCount,    (Essential))                                   \
  /* */

enum Opcode {
#define OPC(name, flags) name,
  IR_OPCODES
#undef OPC
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

const char* opcodeName(Opcode opcode);
bool opcodeHasFlags(Opcode opcode, uint64_t flag);

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

  static const Tag RefCountThreshold = Uncounted;

  static inline bool isBoxed(Tag t) {
    return (t > Cell && t < Gen);
  }

  static inline bool isUnboxed(Tag t) {
    return (t <= Type::Cell && t != Type::None);
  }

  static inline bool isRefCounted(Tag t) {
    return (t > RefCountThreshold && t <= Gen);
  }

  static inline bool isStaticallyKnown(Tag t) {
    return (t != Cell       &&
            t != Gen        &&
            t != Uncounted  &&
            t != UncountedInit);
  }

  static inline bool isStaticallyKnownUnboxed(Tag t) {
    return isStaticallyKnown(t) && isUnboxed(t);
  }

  static inline bool needsStaticBitCheck(Tag t) {
    return (t == Cell ||
            t == Gen  ||
            t == Str  ||
            t == Arr);
  }

  // returns true if definitely not uninitialized
  static inline bool isInit(Tag t) {
    return ((t != Uninit && isStaticallyKnown(t)) ||
            isBoxed(t));
  }

  static inline bool mayBeUninit(Tag t) {
    return (t == Uninit    ||
            t == Uncounted ||
            t == Cell      ||
            t == Gen);
  }

  // returns true if t1 is a more refined that t2
  static inline bool isMoreRefined(Tag t1, Tag t2) {
    return ((t2 == Gen           && t1 < Gen)                    ||
            (t2 == Cell          && t1 < Cell)                   ||
            (t2 == BoxedCell     && t1 < BoxedCell && t1 > Cell) ||
            (t2 == Str           && t1 == StaticStr)             ||
            (t2 == BoxedStr      && t1 == BoxedStaticStr)        ||
            (t2 == Uncounted     && t1 < Uncounted)              ||
            (t2 == UncountedInit && t1 < UncountedInit && t1 > Uninit));
  }

  static inline bool isString(Tag t) {
    return (t == Str || t == StaticStr);
  }

  static inline bool isNull(Tag t) {
    return (t == Null || t == Uninit);
  }

  static inline Tag getInnerType(Tag t) {
    ASSERT(isBoxed(t));
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

  static inline Tag getValueType(Tag t) {
    if (isBoxed(t)) {
      return getInnerType(t);
    }
    return t;
  }

  static const char* Strings[];

  // translates a compiler Type::Type to a HPHP::DataType
  static inline DataType toDataType(Tag type) {
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
      case ClassRef      : return KindOfClass;
      case UncountedInit : return KindOfUncountedInit;
      case Uncounted     : return KindOfUncounted;
      case Gen           : return KindOfAny;
      default: {
        ASSERT(isBoxed(type));
        return KindOfRef;
      }
    }
  }

  static inline Tag fromDataType(DataType outerType, DataType innerType) {
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
      case KindOfClass         : return ClassRef;
      case KindOfUncountedInit : return UncountedInit;
      case KindOfUncounted     : return Uncounted;
      case KindOfAny           : return Gen;
      case KindOfRef           :
        return getBoxedType(fromDataType(innerType, KindOfInvalid));
      default                  : not_reached();
    }
  }

  static inline Tag fromRuntimeType(const Transl::RuntimeType& rtt) {
    return fromDataType(rtt.outerType(), rtt.innerType());
  }

  static inline bool canRunDtor(Type::Tag t) {
    return (t == Obj       ||
            t == BoxedObj  ||
            t == Arr       ||
            t == BoxedArr  ||
            t == Cell      ||
            t == BoxedCell ||
            t == Gen);
  }
}; // class Type

class RawMemSlot {
 public:

  enum Kind {
    ContLabel, ContDone, ContShouldThrow, ContRunning,
    StrLen, FuncNumParams, FuncRefBitVec,
    MaxKind
  };

  static RawMemSlot& Get(Kind k) {
    switch (k) {
      case ContLabel:       return GetContLabel();
      case ContDone:        return GetContDone();
      case ContShouldThrow: return GetContShouldThrow();
      case ContRunning:     return GetContRunning();
      case StrLen:          return GetStrLen();
      case FuncNumParams:   return GetFuncNumParams();
      case FuncRefBitVec:   return GetFuncRefBitVec();
      default: not_reached();
    }
  }

  int64 getOffset()   { return m_offset; }
  int32 getSize()     { return m_size; }
  Type::Tag getType() { return m_type; }

 private:
  RawMemSlot(int64 offset, int32 size, Type::Tag type)
    : m_offset(offset), m_size(size), m_type(type) { }

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

  int64 m_offset;
  int32 m_size;
  Type::Tag m_type;
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
  virtual uint32 hash();
  virtual SSATmp* simplify(Simplifier*);
  virtual void print(std::ostream& ostream);
  void print();
  void genCode(CodeGenerator* cg);
  virtual IRInstruction* clone(IRFactory* factory);
  typedef std::list<IRInstruction*> List;
  typedef std::list<IRInstruction*>::iterator Iterator;
  typedef std::list<IRInstruction*>::reverse_iterator ReverseIterator;

  /*
   * Helper accessors for the OpcodeFlag bits for this instruction.
   *
   * Note that these wrappers may have additional logic beyond just
   * checking the corresponding flags bit.
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

  void printDst(std::ostream& ostream);
  void printSrc(std::ostream& ostream, uint32 srcIndex);
  void printOpcode(std::ostream& ostream);
  void printSrcs(std::ostream& ostream);

protected:
  friend class IRFactory;
  friend class TraceBuilder;
  friend class HhbcTranslator;

 public:
  IRInstruction(Opcode o, Type::Tag t, LabelInstruction *l = NULL)
      : m_op(o), m_type(t), m_id(0), m_numSrcs(0),
        m_dst(NULL), m_asmAddr(NULL), m_label(l),
        m_parent(NULL), m_tca(NULL)
  {
    m_srcs[0] = m_srcs[1] = NULL;
  }

  IRInstruction(Opcode o, Type::Tag t, SSATmp* src, LabelInstruction *l = NULL)
      : m_op(o), m_type(t), m_id(0),  m_numSrcs(1),
        m_dst(NULL), m_asmAddr(NULL), m_label(l),
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
        m_dst(NULL), m_asmAddr(NULL), m_label(l),
        m_parent(NULL), m_tca(NULL)
  {
    m_srcs[0] = src0; m_srcs[1] = src1;
  }

  IRInstruction(IRInstruction* inst)
      : m_op(inst->m_op),
        m_type(inst->m_type),
        m_id(0),
        m_numSrcs(inst->m_numSrcs),
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
  RegSet            m_liveOutRegs;
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
  bool getValAsBool() const {
    ASSERT(m_type == Type::Bool);
    return m_boolVal;
  }
  int64 getValAsInt() const {
    ASSERT(m_type == Type::Int);
    return m_intVal;
  }
  int64 getValAsRawInt() const {
    return m_intVal;
  }
  double getValAsDbl() const {
    ASSERT(m_type == Type::Dbl);
    return m_dblVal;
  }
  const StringData* getValAsStr() const {
    ASSERT(m_type == Type::StaticStr);
    return m_strVal;
  }
  const ArrayData* getValAsArr() const {
    ASSERT(m_type == Type::Arr);
    return m_arrVal;
  }
  const Func* getValAsFunc() const {
    ASSERT(m_type == Type::FuncRef);
    return m_func;
  }
  const Class* getValAsClass() const {
    ASSERT(m_type == Type::ClassRef);
    return m_clss;
  }
  const VarEnv* getValAsVarEnv() const {
    ASSERT(m_type == Type::VarEnvRef);
    return m_varEnv;
  }
  TCA getValAsTCA() const {
    ASSERT(m_type == Type::TCA);
    return m_tca;
  }
  bool isEmptyArray() const {
    return m_arrVal == HphpArray::GetStaticEmptyArray();
  }
  Local getLocal() const {
    ASSERT(m_type == Type::Home);
    return m_local;
  }
  uintptr_t getValAsBits() const { return m_bits; }

  void printConst(std::ostream& ostream) const;
  virtual bool isConstInstruction() const {return true;}
  virtual void print(std::ostream& ostream);
  virtual bool equals(IRInstruction* inst) const;
  virtual uint32 hash();
  virtual IRInstruction* clone(IRFactory* factory);
 protected:
  friend class IRFactory;
  friend class TraceBuilder;
  friend class LinearScan;
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
  ConstInstruction(SSATmp* src, Local l)
      : IRInstruction(LdHome, Type::Home, src) {
    new (&m_local) Local(l);
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
    Local             m_local; // for LdHome opcode
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
  Type::Tag         getType() const { return m_inst->getType(); }
  uint32            getLastUseId() { return m_lastUseId; }
  void              setLastUseId(uint32 newId) { m_lastUseId = newId; }
  uint32            getUseCount() { return m_useCount; }
  void              setUseCount(uint32 count) { m_useCount = count; }
  void              incUseCount() { m_useCount++; }
  uint32            decUseCount() { return --m_useCount; }
  bool              isConst() const { return m_inst->isConstInstruction(); }
  bool              getConstValAsBool() const;
  int64             getConstValAsInt() const;
  int64             getConstValAsRawInt() const;
  double            getConstValAsDbl() const;
  const StringData* getConstValAsStr() const;
  const ArrayData*  getConstValAsArr() const;
  const Func*       getConstValAsFunc() const;
  const Class*      getConstValAsClass() const;
  uintptr_t         getConstValAsBits() const;
  void              print(std::ostream& ostream, bool printLastUse = false);
  void              print();

  // Used for Jcc to Jmp elimination
  void              setTCA(TCA tca);
  TCA               getTCA() const;

  /*
   * Returns whether or not a given register index is allocated to a
   * register, or returns false if it is spilled.
   *
   * Right now, we only spill both at the same time and only Spill and
   * Reload instructions need to deal with SSATmps that are spilled.
   */
  bool              hasReg(uint32 i) const { return !m_isSpilled &&
                                                m_regs[i] != reg::noreg; }

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
  PhysReg     getReg() { ASSERT(!m_isSpilled); return m_regs[0]; }
  PhysReg     getReg(uint32 i) { ASSERT(!m_isSpilled); return m_regs[i]; }
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
  SpillInfo   getSpillInfo(int idx) const { ASSERT(m_isSpilled);
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

class IRFactory {
public:
  IRFactory()
    : m_nextLabelId(0)
    , m_nextOpndId(0)
  {}

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

  SSATmp* getSSATmp(IRInstruction* inst) {
    SSATmp* tmp = new (m_arena) SSATmp(m_nextOpndId++, inst);
    inst->setDst(tmp);
    return tmp;
  }

  uint32 getNumSSATmps() { return m_nextOpndId; }
  Arena& arena() { return m_arena; }

private:
  uint32 m_nextLabelId;
  uint32 m_nextOpndId;

  // SSATmp and IRInstruction objects are allocated here.
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
