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

#ifndef incl_HPHP_VM_HHBC_H_
#define incl_HPHP_VM_HHBC_H_

#include "folly/Optional.h"

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/hphp-value.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct Unit;

// Variable-size immediates are implemented as follows. To determine which size
// the immediate is, examine the first byte where the immediate is expected, and
// examine its low-order bit. If it is zero, it's a 1-byte immediate; otherwise,
// it's 4 bytes. The immediate has to be logical-shifted to the right by one to
// get rid of the flag bit.

// The types in this macro for MA, BLA, and SLA are meaningless since
// they are never read out of ArgUnion (they use ImmVector and
// ImmVectorO).
#define ARGTYPES                                                          \
  ARGTYPE(NA,     void*)         /* unused */                             \
  ARGTYPEVEC(MA,  int32_t)       /* Member vector immediate */            \
  ARGTYPEVEC(BLA, Offset)        /* Bytecode offset vector immediate */   \
  ARGTYPEVEC(SLA, Id)            /* String id/offset pair vector */       \
  ARGTYPEVEC(ILA, Id)            /* IterKind/IterId pair vector */        \
  ARGTYPE(IVA,    int32_t)       /* Variable size: 8 or 32-bit integer */ \
  ARGTYPE(I64A,   int64_t)       /* 64-bit Integer */                     \
  ARGTYPE(LA,     int32_t)       /* Local variable ID: 8 or 32-bit int */ \
  ARGTYPE(IA,     int32_t)       /* Iterator ID: 8 or 32-bit int */       \
  ARGTYPE(DA,     double)        /* Double */                             \
  ARGTYPE(SA,     Id)            /* Static string ID */                   \
  ARGTYPE(AA,     Id)            /* Static array ID */                    \
  ARGTYPE(BA,     Offset)        /* Bytecode offset */                    \
  ARGTYPE(OA,     unsigned char) /* Sub-opcode, untyped */                \
  ARGTYPEVEC(VSA, Id)            /* Vector of static string IDs */

enum ArgType {
#define ARGTYPE(name, type) name,
#define ARGTYPEVEC(name, type) name,
  ARGTYPES
#undef ARGTYPE
#undef ARGTYPEVEC
};

union ArgUnion {
  char bytes[0];
#define ARGTYPE(name, type) type u_##name;
#define ARGTYPEVEC(name, type) type u_##name;
  ARGTYPES
#undef ARGTYPE
#undef ARGTYPEVEC
};

const Offset InvalidAbsoluteOffset = -1;

enum FlavorDesc {
  NOV,  // None
  CV,   // Cell
  VV,   // Var
  AV,   // Classref
  RV,   // Return value (cell or var)
  FV,   // Function parameter (cell or var)
  UV,   // Uninit
  CVV,  // Cell or Var argument
  CVUV, // Cell, Var, or Uninit argument
};

enum InstrFlags {
  NF = 0x0, // No flags
  TF = 0x1, // Next instruction is not reachable via fall through or the
            //   callee returning control
  CF = 0x2, // Control flow instruction (branch, call, return, throw, etc)
  FF = 0x4, // Instruction uses current FPI
  CF_TF = (CF | TF),
  CF_FF = (CF | FF)
};

enum LocationCode {
  // Base is the object stored in a local, a cell, or $this
  LL,
  LC,
  LH,

  // Base is the global name referred to by a cell or a local.
  LGL,
  LGC,

  // Base is the name of a local, given by a cell or the value of a
  // local.
  LNL,
  LNC,

  /*
   * Base is a static property member of a class.  The S-vector takes
   * two things to define a base.  The classref portion comes at the
   * end of the M-vector, and the property name can be defined by
   * either a cell or a local immediate.
   */
  LSL,
  LSC,

  // Base is a function return value.
  LR,

  NumLocationCodes,
  InvalidLocationCode = NumLocationCodes
};

inline int numLocationCodeImms(LocationCode lc) {
  switch (lc) {
  case LL: case LGL: case LNL: case LSL:
    return 1;
  case LC: case LH: case LGC: case LNC: case LSC: case LR:
    return 0;
  default:
    not_reached();
  }
}

inline int numLocationCodeStackVals(LocationCode lc) {
  switch (lc) {
  case LL: case LH: case LGL: case LNL:
    return 0;
  case LC: case LGC: case LNC: case LSL: case LR:
    return 1;
  case LSC:
    return 2;
  default:
    not_reached();
  }
}

// Returns string representation of `lc'.  (Pointer to internal static
// data, does not need to be freed.)
const char* locationCodeString(LocationCode lc);

// Grok a LocationCode from a string---if the string doesn't represent
// a location code, returns InvalidLocationCode.  This looks at at
// most the first two bytes in `s'---the parse will not fail if there
// is more junk after the first two bytes.
LocationCode parseLocationCode(const char* s);

enum MemberCode {
  // Element and property, consuming a cell from the stack.
  MEC,
  MPC,

  // Element and property, using an immediate local id.
  MEL,
  MPL,

  // Element and property, using a string immediate
  MET,
  MPT,

  // Element, using an int64 immediate
  MEI,

  // New element operation.  (No real stack element.)
  MW,

  NumMemberCodes,
  InvalidMemberCode = NumMemberCodes
};

enum MInstrAttr {
  MIA_none         = 0x00,
  MIA_warn         = 0x01,
  MIA_define       = 0x02,
  MIA_reffy        = 0x04,
  MIA_unset        = 0x08,
  MIA_new          = 0x10,
  MIA_final_get    = 0x20,
  MIA_base         = MIA_warn | MIA_define,
  MIA_intermediate = MIA_warn | MIA_define | MIA_reffy | MIA_unset,
  MIA_intermediate_prop = MIA_warn | MIA_define | MIA_unset,
  MIA_final        = MIA_new | MIA_final_get,

  // Some warnings may conditionally be built for Zend compatibility,
  // but are off by default.
  MIA_more_warn =
#ifdef HHVM_MORE_WARNINGS
    MIA_warn
#else
    MIA_none
#endif
};

// MII(instr,  * in *M
//     attrs,  operation attributes
//     bS,     base operation suffix
//     iS,     intermediate operation suffix
//     vC,     final value count (0 or 1)
//     fN)     final new element operation name
#define MINSTRS \
  MII(CGet,   MIA_warn|MIA_final_get,            W,  W, 0, NotSuppNewElem) \
  MII(VGet,   MIA_define|MIA_reffy|MIA_new|MIA_final_get,               \
                                                 D,  D, 0, VGetNewElem) \
  MII(Isset,  MIA_final_get,                      ,   , 0, NotSuppNewElem) \
  MII(Empty,  MIA_final_get,                      ,   , 0, NotSuppNewElem) \
  MII(Set,    MIA_define|MIA_new,                D,  D, 1, SetNewElem)  \
  MII(SetOp,  MIA_more_warn|MIA_define|MIA_new|MIA_final_get,           \
                                                WD, WD, 1, SetOpNewElem) \
  MII(IncDec, MIA_more_warn|MIA_define|MIA_new|MIA_final_get,           \
                                                WD, WD, 0, IncDecNewElem) \
  MII(Bind,   MIA_define|MIA_reffy|MIA_new|MIA_final_get,               \
                                                 D,  D, 1, BindNewElem) \
  MII(Unset,  MIA_unset,                          ,  U, 0, NotSuppNewElem) \
  MII(SetWithRefL,MIA_define|MIA_reffy|MIA_new|MIA_final_get,           \
                                                 D,  D, 1, SetWithRefNewElem) \
  MII(SetWithRefR,MIA_define|MIA_reffy|MIA_new|MIA_final_get,           \
                                                 D,  D, 1, SetWithRefNewElem)

enum MInstr {
#define MII(instr, attrs, bS, iS, vC, fN) \
  MI_##instr##M,
  MINSTRS
#undef MII
};

struct MInstrInfo {
  MInstr     m_instr;
  MInstrAttr m_baseOps[NumLocationCodes];
  MInstrAttr m_intermediateOps[NumMemberCodes];
  unsigned   m_valCount;
  bool       m_newElem;
  bool       m_finalGet;
  const char* m_name;

  MInstr instr() const {
    return m_instr;
  }

  const MInstrAttr& getAttr(LocationCode lc) const {
    assert(lc < NumLocationCodes);
    return m_baseOps[lc];
  }

  const MInstrAttr& getAttr(MemberCode mc) const {
    assert(mc < NumMemberCodes);
    return m_intermediateOps[mc];
  }

  unsigned valCount() const {
    return m_valCount;
  }

  bool newElem() const {
    return m_newElem;
  }

  bool finalGet() const {
    return m_finalGet;
  }

  const char* name() const {
    return m_name;
  }
};

inline bool memberCodeHasImm(MemberCode mc) {
  return mc == MEL || mc == MPL || mc == MET || mc == MPT || mc == MEI;
}

inline bool memberCodeImmIsLoc(MemberCode mc) {
  return mc == MEL || mc == MPL;
}

inline bool memberCodeImmIsString(MemberCode mc) {
  return mc == MET || mc == MPT;
}

inline bool memberCodeImmIsInt(MemberCode mc) {
  return mc == MEI;
}

enum class MCodeImm { None, Int, String, Local };
inline MCodeImm memberCodeImmType(MemberCode mc) {
  if (!memberCodeHasImm(mc))     return MCodeImm::None;
  if (memberCodeImmIsLoc(mc))    return MCodeImm::Local;
  if (memberCodeImmIsString(mc)) return MCodeImm::String;
  if (memberCodeImmIsInt(mc))    return MCodeImm::Int;
  not_reached();
}

inline int mcodeStackVals(MemberCode mc) {
  return !memberCodeHasImm(mc) && mc != MW ? 1 : 0;
}

// Returns string representation of `mc'.  (Pointer to internal static
// data, does not need to be freed.)
const char* memberCodeString(MemberCode mc);

// Same semantics as parseLocationCode, but for member codes.
MemberCode parseMemberCode(const char*);

#define INCDEC_OPS \
  INCDEC_OP(PreInc) \
  INCDEC_OP(PostInc) \
  INCDEC_OP(PreDec) \
  INCDEC_OP(PostDec)
constexpr int kNumIncDecOps = 4;

enum class IncDecOp : uint8_t {
#define INCDEC_OP(incDecOp) incDecOp,
  INCDEC_OPS
#undef INCDEC_OP
};

#define ISTYPE_OPS                             \
  ISTYPE_OP(Null)                              \
  ISTYPE_OP(Bool)                              \
  ISTYPE_OP(Int)                               \
  ISTYPE_OP(Dbl)                               \
  ISTYPE_OP(Str)                               \
  ISTYPE_OP(Arr)                               \
  ISTYPE_OP(Obj)                               \
  ISTYPE_OP(Scalar)

enum class IsTypeOp : uint8_t {
#define ISTYPE_OP(op) op,
  ISTYPE_OPS
#undef ISTYPE_OP
};

// NB: right now hphp/hhbbc/abstract-interp.cpp depends on this enum
// being in order from smaller types to larger ones.
#define ASSERTT_OPS                             \
  ASSERTT_OP(Uninit)                            \
  ASSERTT_OP(InitNull)                          \
  ASSERTT_OP(Null)                              \
  ASSERTT_OP(Int)                               \
  ASSERTT_OP(OptInt)                            \
  ASSERTT_OP(Dbl)                               \
  ASSERTT_OP(OptDbl)                            \
  ASSERTT_OP(Res)                               \
  ASSERTT_OP(OptRes)                            \
  ASSERTT_OP(Bool)                              \
  ASSERTT_OP(OptBool)                           \
  ASSERTT_OP(SStr)                              \
  ASSERTT_OP(OptSStr)                           \
  ASSERTT_OP(Str)                               \
  ASSERTT_OP(OptStr)                            \
  ASSERTT_OP(SArr)                              \
  ASSERTT_OP(OptSArr)                           \
  ASSERTT_OP(Arr)                               \
  ASSERTT_OP(OptArr)                            \
  ASSERTT_OP(Obj)                               \
  ASSERTT_OP(OptObj)                            \
  ASSERTT_OP(InitUnc)                           \
  ASSERTT_OP(Unc)                               \
  ASSERTT_OP(InitCell)                          \
  ASSERTT_OP(Cell)                              \
  ASSERTT_OP(Ref)

enum class AssertTOp : uint8_t {
#define ASSERTT_OP(op) op,
  ASSERTT_OPS
#undef ASSERTT_OP
};

enum IterKind {
  KindOfIter  = 0,
  KindOfMIter = 1,
  KindOfCIter = 2,
};

#define FATAL_OPS                               \
  FATAL_OP(Runtime)                             \
  FATAL_OP(Parse)                               \
  FATAL_OP(RuntimeOmitFrame)

enum class FatalOp : uint8_t {
#define FATAL_OP(x) x,
  FATAL_OPS
#undef FATAL_OP
};

// Each of the setop ops maps to a binary bytecode op. We have reasons
// for using distinct bitwise representations, though. This macro records
// their correspondence for mapping either direction.
#define SETOP_OPS \
  SETOP_OP(PlusEqual,   OpAdd) \
  SETOP_OP(MinusEqual,  OpSub) \
  SETOP_OP(MulEqual,    OpMul) \
  SETOP_OP(ConcatEqual, OpConcat) \
  SETOP_OP(DivEqual,    OpDiv) \
  SETOP_OP(ModEqual,    OpMod) \
  SETOP_OP(AndEqual,    OpBitAnd) \
  SETOP_OP(OrEqual,     OpBitOr) \
  SETOP_OP(XorEqual,    OpBitXor) \
  SETOP_OP(SlEqual,     OpShl) \
  SETOP_OP(SrEqual,     OpShr)
constexpr int kNumSetOpOps = 11;

enum class SetOpOp : uint8_t {
#define SETOP_OP(setOpOp, bcOp) setOpOp,
  SETOP_OPS
#undef SETOP_OP
};

#define BARETHIS_OPS    \
  BARETHIS_OP(Notice)   \
  BARETHIS_OP(NoNotice)

enum class BareThisOp : uint8_t {
#define BARETHIS_OP(x) x,
  BARETHIS_OPS
#undef BARETHIS_OP
};

//  name             immediates        inputs           outputs     flags
#define OPCODES \
  O(LowInvalid,      NA,               NOV,             NOV,        NF) \
  O(Nop,             NA,               NOV,             NOV,        NF) \
  O(BreakTraceHint,  NA,               NOV,             NOV,        NF) \
  O(PopA,            NA,               ONE(AV),         NOV,        NF) \
  O(PopC,            NA,               ONE(CV),         NOV,        NF) \
  O(PopV,            NA,               ONE(VV),         NOV,        NF) \
  O(PopR,            NA,               ONE(RV),         NOV,        NF) \
  O(Dup,             NA,               ONE(CV),         TWO(CV,CV), NF) \
  O(Box,             NA,               ONE(CV),         ONE(VV),    NF) \
  O(Unbox,           NA,               ONE(VV),         ONE(CV),    NF) \
  O(BoxR,            NA,               ONE(RV),         ONE(VV),    NF) \
  O(BoxRNop,         NA,               ONE(RV),         ONE(VV),    NF) \
  O(UnboxR,          NA,               ONE(RV),         ONE(CV),    NF) \
  O(UnboxRNop,       NA,               ONE(RV),         ONE(CV),    NF) \
  O(Null,            NA,               NOV,             ONE(CV),    NF) \
  O(NullUninit,      NA,               NOV,             ONE(UV),    NF) \
  O(True,            NA,               NOV,             ONE(CV),    NF) \
  O(False,           NA,               NOV,             ONE(CV),    NF) \
  O(Int,             ONE(I64A),        NOV,             ONE(CV),    NF) \
  O(Double,          ONE(DA),          NOV,             ONE(CV),    NF) \
  O(String,          ONE(SA),          NOV,             ONE(CV),    NF) \
  O(Array,           ONE(AA),          NOV,             ONE(CV),    NF) \
  O(NewArray,        NA,               NOV,             ONE(CV),    NF) \
  O(NewArrayReserve, ONE(IVA),         NOV,             ONE(CV),    NF) \
  O(NewPackedArray,  ONE(IVA),         CMANY,           ONE(CV),    NF) \
  O(NewStructArray,  ONE(VSA),         SMANY,           ONE(CV),    NF) \
  O(AddElemC,        NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(AddElemV,        NA,               THREE(VV,CV,CV), ONE(CV),    NF) \
  O(AddNewElemC,     NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(AddNewElemV,     NA,               TWO(VV,CV),      ONE(CV),    NF) \
  O(NewCol,          TWO(IVA,IVA),     NOV,             ONE(CV),    NF) \
  O(ColAddElemC,     NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(ColAddNewElemC,  NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Cns,             ONE(SA),          NOV,             ONE(CV),    NF) \
  O(CnsE,            ONE(SA),          NOV,             ONE(CV),    NF) \
  O(CnsU,            TWO(SA,SA),       NOV,             ONE(CV),    NF) \
  O(ClsCns,          ONE(SA),          ONE(AV),         ONE(CV),    NF) \
  O(ClsCnsD,         TWO(SA,SA),       NOV,             ONE(CV),    NF) \
  O(File,            NA,               NOV,             ONE(CV),    NF) \
  O(Dir,             NA,               NOV,             ONE(CV),    NF) \
  O(Concat,          NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Add,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Sub,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Mul,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Div,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Mod,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Sqrt,            NA,               ONE(CV),         ONE(CV),    NF) \
  O(Xor,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Not,             NA,               ONE(CV),         ONE(CV),    NF) \
  O(Same,            NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(NSame,           NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Eq,              NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Neq,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Lt,              NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Lte,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Gt,              NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Gte,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(BitAnd,          NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(BitOr,           NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(BitXor,          NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(BitNot,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(Shl,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Shr,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(CastBool,        NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastInt,         NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastDouble,      NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastString,      NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastArray,       NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastObject,      NA,               ONE(CV),         ONE(CV),    NF) \
  O(InstanceOf,      NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(InstanceOfD,     ONE(SA),          ONE(CV),         ONE(CV),    NF) \
  O(Print,           NA,               ONE(CV),         ONE(CV),    NF) \
  O(Clone,           NA,               ONE(CV),         ONE(CV),    NF) \
  O(Exit,            NA,               ONE(CV),         ONE(CV),    NF) \
  O(Fatal,           ONE(OA(FatalOp)), ONE(CV),         NOV,        CF_TF) \
  O(Jmp,             ONE(BA),          NOV,             NOV,        CF_TF) \
  O(JmpNS,           ONE(BA),          NOV,             NOV,        CF_TF) \
  O(JmpZ,            ONE(BA),          ONE(CV),         NOV,        CF) \
  O(JmpNZ,           ONE(BA),          ONE(CV),         NOV,        CF) \
  O(Switch,          THREE(BLA,I64A,IVA),                               \
                                       ONE(CV),         NOV,        CF_TF) \
  O(SSwitch,         ONE(SLA),         ONE(CV),         NOV,        CF_TF) \
  O(RetC,            NA,               ONE(CV),         NOV,        CF_TF) \
  O(RetV,            NA,               ONE(VV),         NOV,        CF_TF) \
  O(Unwind,          NA,               NOV,             NOV,        CF_TF) \
  O(Throw,           NA,               ONE(CV),         NOV,        CF_TF) \
  O(CGetL,           ONE(LA),          NOV,             ONE(CV),    NF) \
  O(CGetL2,          ONE(LA),          NOV,             INS_1(CV),  NF) \
  O(CGetL3,          ONE(LA),          NOV,             INS_2(CV),  NF) \
  O(PushL,           ONE(LA),          NOV,             ONE(CV),    NF) \
  O(CGetN,           NA,               ONE(CV),         ONE(CV),    NF) \
  O(CGetG,           NA,               ONE(CV),         ONE(CV),    NF) \
  O(CGetS,           NA,               TWO(AV,CV),      ONE(CV),    NF) \
  O(CGetM,           ONE(MA),          MMANY,           ONE(CV),    NF) \
  O(VGetL,           ONE(LA),          NOV,             ONE(VV),    NF) \
  O(VGetN,           NA,               ONE(CV),         ONE(VV),    NF) \
  O(VGetG,           NA,               ONE(CV),         ONE(VV),    NF) \
  O(VGetS,           NA,               TWO(AV,CV),      ONE(VV),    NF) \
  O(VGetM,           ONE(MA),          MMANY,           ONE(VV),    NF) \
  O(AGetC,           NA,               ONE(CV),         ONE(AV),    NF) \
  O(AGetL,           ONE(LA),          NOV,             ONE(AV),    NF) \
  O(AKExists,        NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(IssetL,          ONE(LA),          NOV,             ONE(CV),    NF) \
  O(IssetN,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(IssetG,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(IssetS,          NA,               TWO(AV,CV),      ONE(CV),    NF) \
  O(IssetM,          ONE(MA),          MMANY,           ONE(CV),    NF) \
  O(EmptyL,          ONE(LA),          NOV,             ONE(CV),    NF) \
  O(EmptyN,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(EmptyG,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(EmptyS,          NA,               TWO(AV,CV),      ONE(CV),    NF) \
  O(EmptyM,          ONE(MA),          MMANY,           ONE(CV),    NF) \
  O(IsTypeC,         ONE(OA(IsTypeOp)),ONE(CV),         ONE(CV),    NF) \
  O(IsTypeL,         TWO(LA,                                            \
                       OA(IsTypeOp)),  NOV,             ONE(CV),    NF) \
  O(AssertTL,        TWO(LA,                                            \
                       OA(AssertTOp)), NOV,             NOV,        NF) \
  O(AssertTStk,      TWO(IVA,                                           \
                       OA(AssertTOp)), NOV,             NOV,        NF) \
  O(AssertObjL,      THREE(LA,IVA,SA), NOV,             NOV,        NF) \
  O(AssertObjStk,    THREE(IVA,IVA,SA),NOV,             NOV,        NF) \
  O(PredictTL,       TWO(LA,                                            \
                       OA(AssertTOp)), NOV,             NOV,        NF) \
  O(PredictTStk,     TWO(IVA,                                           \
                       OA(AssertTOp)), NOV,             NOV,        NF) \
  O(SetL,            ONE(LA),          ONE(CV),         ONE(CV),    NF) \
  O(SetN,            NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(SetG,            NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(SetS,            NA,               THREE(CV,AV,CV), ONE(CV),    NF) \
  O(SetM,            ONE(MA),          C_MMANY,         ONE(CV),    NF) \
  O(SetWithRefLM,    TWO(MA,LA),       MMANY,           NOV,        NF) \
  O(SetWithRefRM,    ONE(MA),          R_MMANY,         NOV,        NF) \
  O(SetOpL,          TWO(LA,                                            \
                       OA(SetOpOp)),   ONE(CV),         ONE(CV),    NF) \
  O(SetOpN,          ONE(OA(SetOpOp)), TWO(CV,CV),      ONE(CV),    NF) \
  O(SetOpG,          ONE(OA(SetOpOp)), TWO(CV,CV),      ONE(CV),    NF) \
  O(SetOpS,          ONE(OA(SetOpOp)), THREE(CV,AV,CV), ONE(CV),    NF) \
  O(SetOpM,          TWO(OA(SetOpOp),                                   \
                       MA),            C_MMANY,         ONE(CV),    NF) \
  O(IncDecL,         TWO(LA,                                            \
                       OA(IncDecOp)),  NOV,             ONE(CV),    NF) \
  O(IncDecN,         ONE(OA(IncDecOp)),ONE(CV),         ONE(CV),    NF) \
  O(IncDecG,         ONE(OA(IncDecOp)),ONE(CV),         ONE(CV),    NF) \
  O(IncDecS,         ONE(OA(IncDecOp)),TWO(AV,CV),      ONE(CV),    NF) \
  O(IncDecM,         TWO(OA(IncDecOp),                                  \
                       MA),            MMANY,           ONE(CV),    NF) \
  O(BindL,           ONE(LA),          ONE(VV),         ONE(VV),    NF) \
  O(BindN,           NA,               TWO(VV,CV),      ONE(VV),    NF) \
  O(BindG,           NA,               TWO(VV,CV),      ONE(VV),    NF) \
  O(BindS,           NA,               THREE(VV,AV,CV), ONE(VV),    NF) \
  O(BindM,           ONE(MA),          V_MMANY,         ONE(VV),    NF) \
  O(UnsetL,          ONE(LA),          NOV,             NOV,        NF) \
  O(UnsetN,          NA,               ONE(CV),         NOV,        NF) \
  O(UnsetG,          NA,               ONE(CV),         NOV,        NF) \
  O(UnsetM,          ONE(MA),          MMANY,           NOV,        NF) \
  /* NOTE: isFPush below relies on the grouping of FPush* here */       \
  O(FPushFunc,       ONE(IVA),         ONE(CV),         NOV,        NF) \
  O(FPushFuncD,      TWO(IVA,SA),      NOV,             NOV,        NF) \
  O(FPushFuncU,      THREE(IVA,SA,SA), NOV,             NOV,        NF) \
  O(FPushObjMethod,  ONE(IVA),         TWO(CV,CV),      NOV,        NF) \
  O(FPushObjMethodD, TWO(IVA,SA),      ONE(CV),         NOV,        NF) \
  O(FPushClsMethod,  ONE(IVA),         TWO(AV,CV),      NOV,        NF) \
  O(FPushClsMethodF, ONE(IVA),         TWO(AV,CV),      NOV,        NF) \
  O(FPushClsMethodD, THREE(IVA,SA,SA), NOV,             NOV,        NF) \
  O(FPushCtor,       ONE(IVA),         ONE(AV),         ONE(CV),    NF) \
  O(FPushCtorD,      TWO(IVA,SA),      NOV,             ONE(CV),    NF) \
  O(FPushCufIter,    TWO(IVA,IA),      NOV,             NOV,        NF) \
  O(FPushCuf,        ONE(IVA),         ONE(CV),         NOV,        NF) \
  O(FPushCufF,       ONE(IVA),         ONE(CV),         NOV,        NF) \
  O(FPushCufSafe,    ONE(IVA),         TWO(CV,CV),      TWO(CV,CV), NF) \
  O(FPassC,          ONE(IVA),         ONE(CV),         ONE(FV),    FF) \
  O(FPassCW,         ONE(IVA),         ONE(CV),         ONE(FV),    FF) \
  O(FPassCE,         ONE(IVA),         ONE(CV),         ONE(FV),    FF) \
  O(FPassV,          ONE(IVA),         ONE(VV),         ONE(FV),    FF) \
  O(FPassVNop,       ONE(IVA),         ONE(VV),         ONE(FV),    FF) \
  O(FPassR,          ONE(IVA),         ONE(RV),         ONE(FV),    FF) \
  O(FPassL,          TWO(IVA,LA),      NOV,             ONE(FV),    FF) \
  O(FPassN,          ONE(IVA),         ONE(CV),         ONE(FV),    FF) \
  O(FPassG,          ONE(IVA),         ONE(CV),         ONE(FV),    FF) \
  O(FPassS,          ONE(IVA),         TWO(AV,CV),      ONE(FV),    FF) \
  O(FPassM,          TWO(IVA,MA),      MMANY,           ONE(FV),    FF) \
  O(FCall,           ONE(IVA),         FMANY,           ONE(RV),    CF_FF) \
  O(FCallArray,      NA,               ONE(FV),         ONE(RV),    CF_FF) \
  O(FCallBuiltin,    THREE(IVA,IVA,SA),CVUMANY,         ONE(RV),    CF) \
  O(CufSafeArray,    NA,               THREE(RV,CV,CV), ONE(CV),    NF) \
  O(CufSafeReturn,   NA,               THREE(RV,CV,CV), ONE(RV),    NF) \
  O(IterInit,        THREE(IA,BA,LA),  ONE(CV),         NOV,        CF) \
  O(MIterInit,       THREE(IA,BA,LA),  ONE(VV),         NOV,        CF) \
  O(WIterInit,       THREE(IA,BA,LA),  ONE(CV),         NOV,        CF) \
  O(IterInitK,       FOUR(IA,BA,LA,LA),ONE(CV),         NOV,        CF) \
  O(MIterInitK,      FOUR(IA,BA,LA,LA),ONE(VV),         NOV,        CF) \
  O(WIterInitK,      FOUR(IA,BA,LA,LA),ONE(CV),         NOV,        CF) \
  O(IterNext,        THREE(IA,BA,LA),  NOV,             NOV,        CF) \
  O(MIterNext,       THREE(IA,BA,LA),  NOV,             NOV,        CF) \
  O(WIterNext,       THREE(IA,BA,LA),  NOV,             NOV,        CF) \
  O(IterNextK,       FOUR(IA,BA,LA,LA),NOV,             NOV,        CF) \
  O(MIterNextK,      FOUR(IA,BA,LA,LA),NOV,             NOV,        CF) \
  O(WIterNextK,      FOUR(IA,BA,LA,LA),NOV,             NOV,        CF) \
  O(DecodeCufIter,   TWO(IA,BA),       ONE(CV),         NOV,        CF) \
  O(IterFree,        ONE(IA),          NOV,             NOV,        NF) \
  O(MIterFree,       ONE(IA),          NOV,             NOV,        NF) \
  O(CIterFree,       ONE(IA),          NOV,             NOV,        NF) \
  O(IterBreak,       TWO(ILA,BA),      NOV,             NOV,        CF_TF) \
  O(Incl,            NA,               ONE(CV),         ONE(CV),    CF) \
  O(InclOnce,        NA,               ONE(CV),         ONE(CV),    CF) \
  O(Req,             NA,               ONE(CV),         ONE(CV),    CF) \
  O(ReqOnce,         NA,               ONE(CV),         ONE(CV),    CF) \
  O(ReqDoc,          NA,               ONE(CV),         ONE(CV),    CF) \
  O(Eval,            NA,               ONE(CV),         ONE(CV),    CF) \
  O(DefFunc,         ONE(IVA),         NOV,             NOV,        NF) \
  O(DefCls,          ONE(IVA),         NOV,             NOV,        NF) \
  O(NopDefCls,       ONE(IVA),         NOV,             NOV,        NF) \
  O(DefCns,          ONE(SA),          ONE(CV),         ONE(CV),    NF) \
  O(DefTypeAlias,    ONE(IVA),         NOV,             NOV,        NF) \
  O(This,            NA,               NOV,             ONE(CV),    NF) \
  O(BareThis,        ONE(OA(BareThisOp)),                               \
                                       NOV,             ONE(CV),    NF) \
  O(CheckThis,       NA,               NOV,             NOV,        NF) \
  O(InitThisLoc,     ONE(IVA),         NOV,             NOV,        NF) \
  O(StaticLoc,       TWO(IVA,SA),      NOV,             ONE(CV),    NF) \
  O(StaticLocInit,   TWO(IVA,SA),      ONE(CV),         NOV,        NF) \
  O(Catch,           NA,               NOV,             ONE(CV),    NF) \
  O(ClassExists,     NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(InterfaceExists, NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(TraitExists,     NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(VerifyParamType, ONE(IVA),         NOV,             NOV,        NF) \
  O(Self,            NA,               NOV,             ONE(AV),    NF) \
  O(Parent,          NA,               NOV,             ONE(AV),    NF) \
  O(LateBoundCls,    NA,               NOV,             ONE(AV),    NF) \
  O(NativeImpl,      NA,               NOV,             NOV,        CF_TF) \
  O(CreateCl,        TWO(IVA,SA),      CVMANY,          ONE(CV),    NF) \
  O(CreateCont,      NA,               NOV,             ONE(CV),    NF) \
  O(ContEnter,       NA,               ONE(CV),         NOV,        CF) \
  O(UnpackCont,      NA,               NOV,             TWO(CV,CV), NF) \
  O(ContSuspend,     ONE(IVA),         ONE(CV),         NOV,        CF_TF) \
  O(ContSuspendK,    ONE(IVA),         TWO(CV,CV),      NOV,        CF_TF) \
  O(ContRetC,        NA,               ONE(CV),         NOV,        CF_TF) \
  O(ContCheck,       ONE(IVA),         NOV,             NOV,        NF) \
  O(ContRaise,       NA,               NOV,             NOV,        NF) \
  O(ContValid,       NA,               NOV,             ONE(CV),    NF) \
  O(ContKey,         NA,               NOV,             ONE(CV),    NF) \
  O(ContCurrent,     NA,               NOV,             ONE(CV),    NF) \
  O(ContStopped,     NA,               NOV,             NOV,        NF) \
  O(ContHandle,      NA,               ONE(CV),         NOV,        CF_TF) \
  O(AsyncAwait,      NA,               ONE(CV),         TWO(CV,CV), NF) \
  O(AsyncESuspend,   TWO(IVA,IVA),     ONE(CV),         ONE(CV),    NF) \
  O(AsyncWrapResult, NA,               ONE(CV),         ONE(CV),    NF) \
  O(AsyncWrapException, NA,            ONE(CV),         ONE(CV),    NF) \
  O(Strlen,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(IncStat,         TWO(IVA,IVA),     NOV,             NOV,        NF) \
  O(Abs,             NA,               ONE(CV),         ONE(CV),    NF) \
  O(Idx,             NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(ArrayIdx,        NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(Floor,           NA,               ONE(CV),         ONE(CV),    NF) \
  O(Ceil,            NA,               ONE(CV),         ONE(CV),    NF) \
  O(HighInvalid,     NA,               NOV,             NOV,        NF) \

enum class Op : uint8_t {
#define O(name, ...) name,
  OPCODES
#undef O
};
auto constexpr Op_count = uint8_t(Op::HighInvalid) + 1;

/* Also put Op* in the enclosing namespace, to avoid having to change
 * every existing usage site of the enum values. */
#define O(name, ...) UNUSED auto constexpr Op##name = Op::name;
  OPCODES
#undef O

inline constexpr bool operator<(Op a, Op b) { return uint8_t(a) < uint8_t(b); }
inline constexpr bool operator>(Op a, Op b) { return uint8_t(a) > uint8_t(b); }
inline constexpr bool operator<=(Op a, Op b) {
  return uint8_t(a) <= uint8_t(b);
}
inline constexpr bool operator>=(Op a, Op b) {
  return uint8_t(a) >= uint8_t(b);
}

inline bool isValidOpcode(Op op) {
  return op > OpLowInvalid && op < OpHighInvalid;
}

inline Op toOp(Opcode o) {
  Op op = Op(o);
  assert(isValidOpcode(op));
  return op;
}

const MInstrInfo& getMInstrInfo(Op op);

enum AstubsOp {
  OpAstubStart = Op_count-1,
#define O(name, imm, pop, push, flags) OpAstub##name,
  OPCODES
#undef O
  OpAstubCount
};

#define HIGH_OPCODES \
  O(FuncPrologue) \
  O(TraceletGuard) \
  O(NativeTrampoline)

enum HighOp {
  OpHighStart = OpAstubCount-1,
#define O(name) Op##name,
  HIGH_OPCODES
#undef O
};

struct StrVecItem {
  Id str;
  Offset dest;
};

struct ImmVector {
  explicit ImmVector() : m_start(0) {}

  explicit ImmVector(const uint8_t* start,
                     int32_t length,
                     int32_t numStack)
    : m_length(length)
    , m_numStack(numStack)
    , m_start(start)
  {}

  /*
   * Returns an ImmVector from a pointer to the immediate vector
   * itself.  Use getImmVector() if you want to get it from an Opcode*
   * that points to the opcode.
   */
  static ImmVector createFromStream(const uint8_t* opcode) {
    int32_t size = reinterpret_cast<const int32_t*>(opcode)[0];
    int32_t stackCount = reinterpret_cast<const int32_t*>(opcode)[1];
    const uint8_t* start = opcode + sizeof(int32_t) + sizeof(int32_t);
    return ImmVector(start, size, stackCount);
  }

  /*
   * Returns an ImmVector of 32-bit ints from a pointer to the
   * immediate vector itself.
   */
  static ImmVector createFromStream(const int32_t* stream) {
    int32_t size = stream[0];
    return ImmVector(reinterpret_cast<const uint8_t*>(stream + 1), size, 0);
  }

  bool isValid() const { return m_start != 0; }

  const uint8_t* vec() const { return m_start; }
  const int32_t* vec32() const {
    return reinterpret_cast<const int32_t*>(m_start);
  }
  const StrVecItem* strvec() const {
    return reinterpret_cast<const StrVecItem*>(m_start);
  }

  LocationCode locationCode() const { return LocationCode(*vec()); }

  /*
   * Returns the length of the immediate vector in bytes (for M
   * vectors) or elements (for switch vectors)
   */
  int32_t size() const { return m_length; }

  /*
   * Returns the number of elements on the execution stack that this
   * vector will need to access.  (Includes stack elements for both
   * the base and members.)
   */
  int numStackValues() const { return m_numStack; }

  /*
   * Returns a pointer to the last member code in the vector.
   *
   * Requires: isValid() && size() >= 1
   */
  const uint8_t* findLastMember() const;

  /*
   * Decode the terminating string immediate, if any.
   */
  bool decodeLastMember(const Unit*, StringData*& sdOut,
                        MemberCode& membOut,
                        int64_t* strIdOut = nullptr) const;


private:
  int32_t m_length;
  int32_t m_numStack;
  const uint8_t* m_start;
};

// Must be an opcode that actually has an ImmVector.
ImmVector getImmVector(const Op* opcode);

struct MInstrLocation {
  LocationCode lcode;
  int64_t imm;

  bool hasImm() const {
    auto count = numLocationCodeImms(lcode);
    assert(count == 0 || count == 1);
    return count;
  }
};
MInstrLocation getMLocation(const Op* opcode);

struct MVectorItem {
  MemberCode mcode;
  int64_t imm;

  bool hasImm() const {
    return memberCodeHasImm(mcode);
  }
};
bool hasMVector(Op op);
std::vector<MVectorItem> getMVector(const Op* opcode);

/* Some decoding helper functions. */
int numImmediates(Op opcode);
ArgType immType(Op opcode, int idx);
int immSize(const Op* opcode, int idx);
bool immIsVector(Op opcode, int idx);
bool hasImmVector(Op opcode);
int instrLen(const Op* opcode);
int numSuccs(const Op* opcode);
bool pushesActRec(Op opcode);

// The returned struct has normalized variable-sized immediates
ArgUnion getImm(const Op* opcode, int idx);
// Don't use this with variable-sized immediates!
ArgUnion* getImmPtr(const Op* opcode, int idx);

// Pass a pointer to the pointer to the immediate; this function will advance
// the pointer past the immediate
ALWAYS_INLINE
int32_t decodeVariableSizeImm(const unsigned char** immPtr) {
  const unsigned char small = **immPtr;
  if (UNLIKELY(small & 0x1)) {
    const unsigned int large = *((const unsigned int*)*immPtr);
    *immPtr += sizeof(large);
    return (int32_t)(large >> 1);
  } else {
    *immPtr += sizeof(small);
    return (int32_t)(small >> 1);
  }
}

int64_t decodeMemberCodeImm(const unsigned char** immPtr, MemberCode mcode);

// Encodes a variable sized immediate for `val' into `buf'.  Returns
// the number of bytes used taken.  At most 4 bytes can be used.
size_t encodeVariableSizeImm(int32_t val, unsigned char* buf);

// Encodes a variable sized immediate to the end of vec.
void encodeIvaToVector(std::vector<uchar>& vec, int32_t val);

template<typename T>
void encodeToVector(std::vector<uchar>& vec, T val) {
  size_t currentLen = vec.size();
  vec.resize(currentLen + sizeof(T));
  memcpy(&vec[currentLen], &val, sizeof(T));
}

void staticStreamer(const TypedValue* tv, std::stringstream& out);

std::string instrToString(const Op* it, const Unit* u = nullptr);
void staticArrayStreamer(ArrayData*, std::ostream&);

/*
 * Convert subopcodes or opcodes into strings.
 */
const char* opcodeToName(Op op);
const char* subopToName(IsTypeOp);
const char* subopToName(AssertTOp);
const char* subopToName(FatalOp);
const char* subopToName(SetOpOp);
const char* subopToName(IncDecOp);
const char* subopToName(BareThisOp);

/*
 * Try to parse a string into a subop name of a given type.
 *
 * Returns folly::none if the string is not recognized as that type of
 * subop.
 */
template<class SubOpType>
folly::Optional<SubOpType> nameToSubop(const char*);

// returns a pointer to the location within the bytecode containing the jump
//   Offset, or NULL if the instruction cannot jump. Note that this offset is
//   relative to the current instruction.
Offset* instrJumpOffset(const Op* instr);

// returns absolute address of target, or InvalidAbsoluteOffset if instruction
//   cannot jump
Offset instrJumpTarget(const Op* instrs, Offset pos);

struct StackTransInfo {
  enum class Kind {
    PushPop,
    InsertMid
  };
  Kind kind;
  int numPops;
  int numPushes;
  int pos;
};

bool instrIsNonCallControlFlow(Op opcode);
bool instrAllowsFallThru(Op opcode);
bool instrReadsCurrentFpi(Op opcode);

constexpr InstrFlags instrFlagsData[] = {
#define O(unusedName, unusedImm, unusedPop, unusedPush, flags) flags,
  OPCODES
#undef O
};

constexpr inline InstrFlags instrFlags(Op opcode) {
  return instrFlagsData[uint8_t(opcode)];
}

constexpr inline bool instrIsControlFlow(Op opcode) {
  return (instrFlags(opcode) & CF) != 0;
}

constexpr inline bool isUnconditionalJmp(Op opcode) {
  return opcode == Op::Jmp || opcode == Op::JmpNS;
}

inline bool isFPush(Op opcode) {
  return opcode >= OpFPushFunc && opcode <= OpFPushCufSafe;
}

inline bool isFCallStar(Op opcode) {
  switch (opcode) {
    case OpFCall:
    case OpFCallArray:
      return true;

    default:
      return false;
  }
}

inline bool isFPassStar(Op opcode) {
  switch (opcode) {
    case OpFPassC:
    case OpFPassCW:
    case OpFPassCE:
    case OpFPassV:
    case OpFPassR:
    case OpFPassL:
    case OpFPassN:
    case OpFPassG:
    case OpFPassS:
    case OpFPassM:
      return true;

    default:
      return false;
  }
}

inline bool isLiteral(Op op) {
  switch (op) {
    case OpNull:
    case OpNullUninit:
    case OpTrue:
    case OpFalse:
    case OpInt:
    case OpDouble:
    case OpString:
    case OpArray:
      return true;

    default:
      return false;
  }
}

inline bool isThisSelfOrParent(Op op) {
  switch (op) {
    case OpThis:
    case OpSelf:
    case OpParent:
      return true;

    default:
      return false;
  }
}

inline bool isRet(Op op) {
  switch (op) {
    case OpRetC:
    case OpRetV:
      return true;

    default:
      return false;
  }
}

inline bool isSwitch(Op op) {
  switch (op) {
    case OpSwitch:
    case OpSSwitch:
      return true;

    default:
      return false;
  }
}

inline bool isSwitch(Opcode op) {
  return isSwitch(toOp(op));
}

template<typename L>
void foreachSwitchTarget(const Op* op, L func) {
  assert(isSwitch(*op));
  bool isStr = readData<Op>(op) == OpSSwitch;
  int32_t size = readData<int32_t>(op);
  for (int i = 0; i < size; ++i) {
    if (isStr) readData<Id>(op);
    func(readData<Offset>(op));
  }
}

template<typename L>
void foreachSwitchString(Opcode* op, L func) {
  assert(toOp(*op) == OpSSwitch);
  readData<Opcode>(op);
  int32_t size = readData<int32_t>(op) - 1; // the last item is the default
  for (int i = 0; i < size; ++i) {
    func(readData<Id>(op));
    readData<Offset>(op);
  }
}

int instrNumPops(const Op* opcode);
int instrNumPushes(const Op* opcode);
FlavorDesc instrInputFlavor(const Op* op, uint32_t idx);
StackTransInfo instrStackTransInfo(const Op* opcode);
int instrSpToArDelta(const Op* opcode);

inline bool mcodeIsLiteral(MemberCode mcode) {
  return mcode == MET || mcode == MEI || mcode == MPT;
}

inline bool mcodeMaybePropName(MemberCode mcode) {
  return mcode == MPC || mcode == MPL || mcode == MPT;
}

inline bool mcodeMaybeArrayOrMapKey(MemberCode mcode) {
  return mcode == MEC || mcode == MEL || mcode == MET || mcode == MEI;
}

inline bool mcodeMaybeArrayStringKey(MemberCode mcode) {
  return mcode == MEC || mcode == MEL || mcode == MET;
}

inline bool mcodeMaybeArrayIntKey(MemberCode mcode) {
  return mcode == MEC || mcode == MEL || mcode == MEI;
}

inline bool mcodeMaybeVectorKey(MemberCode mcode) {
  return mcode == MEC || mcode == MEL || mcode == MEI;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

namespace std {
template<>
struct hash<HPHP::Op> {
  size_t operator()(HPHP::Op op) const {
    return HPHP::hash_int64(uint8_t(op));
  }
};
}

//////////////////////////////////////////////////////////////////////

#endif
