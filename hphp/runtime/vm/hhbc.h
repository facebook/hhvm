/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <type_traits>

#include <folly/Optional.h>

#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/member-key.h"
#include "hphp/util/either.h"
#include "hphp/util/functional.h"
#include "hphp/util/hash-map-typedefs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct Unit;
struct UnitEmitter;
struct Func;

/*
 * Variable-size immediates are implemented as follows: To determine which size
 * the immediate is, examine the first byte where the immediate is expected,
 * and examine its low-order bit.  If it is zero, it's a 1-byte immediate;
 * otherwise, it's 4 bytes.  The immediate has to be logical-shifted to the
 * right by one to get rid of the flag bit.
 *
 * The types in this macro for BLA and SLA are meaningless since they are never
 * read out of ArgUnion (they use ImmVector and ImmVectorO).
 *
 * ArgTypes and their various decoding helpers should be kept in sync with the
 * `hhx' bytecode inspection GDB command.
 */
#define ARGTYPES                                                          \
  ARGTYPE(NA,     void*)         /* unused */                             \
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
  ARGTYPE(RATA,   RepoAuthType)  /* Statically inferred RepoAuthType */   \
  ARGTYPE(BA,     Offset)        /* Bytecode offset */                    \
  ARGTYPE(OA,     unsigned char) /* Sub-opcode, untyped */                \
  ARGTYPE(KA,     MemberKey)     /* Member key: local, stack, int, str */ \
  ARGTYPEVEC(VSA, Id)            /* Vector of static string IDs */

enum ArgType {
#define ARGTYPE(name, type) name,
#define ARGTYPEVEC(name, type) name,
  ARGTYPES
#undef ARGTYPE
#undef ARGTYPEVEC
};

union ArgUnion {
  ArgUnion() : u_LA{0} {}
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
  CRV,  // Cell or Return value argument
  CUV,  // Cell, or Uninit argument
  CVUV, // Cell, Var, or Uninit argument
};

enum InstrFlags {
  /* No flags. */
  NF = 0x0,

  /* Terminal: next instruction is not reachable via fall through or the callee
   * returning control. This includes instructions like Throw and Unwind that
   * always throw exceptions. */
  TF = 0x1,

  /* Control flow: If this instruction finishes executing (doesn't throw an
   * exception), vmpc() is not guaranteed to point to the next instruction in
   * the bytecode stream. This does not take VM reentry into account, as that
   * operation is part of the instruction that performed the reentry, and does
   * not affect what vmpc() is set to after the instruction completes. */
  CF = 0x2,

  /* Instruction uses current FPI. */
  FF = 0x4,

  /* Shorthand for common combinations. */
  CF_TF = (CF | TF),
  CF_FF = (CF | FF)
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

std::string show(MInstrAttr);

#define INCDEC_OPS    \
  INCDEC_OP(PreInc)   \
  INCDEC_OP(PostInc)  \
  INCDEC_OP(PreDec)   \
  INCDEC_OP(PostDec)  \
                      \
  INCDEC_OP(PreIncO)  \
  INCDEC_OP(PostIncO) \
  INCDEC_OP(PreDecO)  \
  INCDEC_OP(PostDecO) \

enum class IncDecOp : uint8_t {
#define INCDEC_OP(incDecOp) incDecOp,
  INCDEC_OPS
#undef INCDEC_OP
};

inline bool isPre(IncDecOp op) {
  return
    op == IncDecOp::PreInc || op == IncDecOp::PreIncO ||
    op == IncDecOp::PreDec || op == IncDecOp::PreDecO;
}

inline bool isInc(IncDecOp op) {
  return
    op == IncDecOp::PreInc || op == IncDecOp::PreIncO ||
    op == IncDecOp::PostInc || op == IncDecOp::PostIncO;
}

inline bool isIncDecO(IncDecOp op) {
  return
    op == IncDecOp::PreIncO || op == IncDecOp::PreDecO ||
    op == IncDecOp::PostIncO || op == IncDecOp::PostDecO;
}

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

#define INITPROP_OPS    \
  INITPROP_OP(Static)   \
  INITPROP_OP(NonStatic)

enum class InitPropOp : uint8_t {
#define INITPROP_OP(op) op,
  INITPROP_OPS
#undef INITPROP_OP
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
  SETOP_OP(PowEqual,    OpPow) \
  SETOP_OP(ModEqual,    OpMod) \
  SETOP_OP(AndEqual,    OpBitAnd) \
  SETOP_OP(OrEqual,     OpBitOr) \
  SETOP_OP(XorEqual,    OpBitXor) \
  SETOP_OP(SlEqual,     OpShl) \
  SETOP_OP(SrEqual,     OpShr)  \
  SETOP_OP(PlusEqualO,  OpAddO) \
  SETOP_OP(MinusEqualO, OpSubO) \
  SETOP_OP(MulEqualO,   OpMulO) \

enum class SetOpOp : uint8_t {
#define SETOP_OP(setOpOp, bcOp) setOpOp,
  SETOP_OPS
#undef SETOP_OP
};

#define BARETHIS_OPS    \
  BARETHIS_OP(Notice)   \
  BARETHIS_OP(NoNotice) \
  BARETHIS_OP(NeverNull)

enum class BareThisOp : uint8_t {
#define BARETHIS_OP(x) x,
  BARETHIS_OPS
#undef BARETHIS_OP
};

#define SILENCE_OPS \
  SILENCE_OP(Start) \
  SILENCE_OP(End)

enum class SilenceOp : uint8_t {
#define SILENCE_OP(x) x,
  SILENCE_OPS
#undef SILENCE_OP
};

#define OO_DECL_EXISTS_OPS                             \
  OO_DECL_EXISTS_OP(Class)                             \
  OO_DECL_EXISTS_OP(Interface)                         \
  OO_DECL_EXISTS_OP(Trait)

enum class OODeclExistsOp : uint8_t {
#define OO_DECL_EXISTS_OP(x) x,
  OO_DECL_EXISTS_OPS
#undef OO_DECL_EXISTS_OP
};

#define OBJMETHOD_OPS                             \
  OBJMETHOD_OP(NullThrows)                        \
  OBJMETHOD_OP(NullSafe)

enum class ObjMethodOp : uint8_t {
#define OBJMETHOD_OP(x) x,
  OBJMETHOD_OPS
#undef OBJMETHOD_OP
};

#define SWITCH_KINDS                            \
  KIND(Unbounded)                               \
  KIND(Bounded)

enum class SwitchKind : uint8_t {
#define KIND(x) x,
  SWITCH_KINDS
#undef KIND
};

#define M_OP_FLAGS                                 \
  FLAG(None,             0)                        \
  FLAG(Warn,       (1 << 0))                       \
  FLAG(Define,     (1 << 1))                       \
  FLAG(Unset,      (1 << 2))                       \
  FLAG(DefineReffy,(Define | (1 << 3)))            \
  FLAG(WarnDefine, (Warn | Define))

enum class MOpFlags : uint8_t {
#define FLAG(name, val) name = val,
  M_OP_FLAGS
#undef FLAG
};

inline constexpr bool operator&(MOpFlags a, MOpFlags b) {
  return uint8_t(a) & uint8_t(b);
}

inline MInstrAttr mOpFlagsToAttr(MOpFlags f) {
  switch (f) {
    case MOpFlags::None:        return MIA_none;
    case MOpFlags::Warn:        return MIA_warn;
    case MOpFlags::Define:      return MIA_define;
    case MOpFlags::Unset:       return MIA_unset;
    case MOpFlags::DefineReffy: return MInstrAttr(MIA_reffy | MIA_define);
    case MOpFlags::WarnDefine:  return MInstrAttr(MIA_warn | MIA_define);
  }
  always_assert(false);
}

#define QUERY_M_OPS                               \
  OP(CGet)                                        \
  OP(CGetQuiet)                                   \
  OP(Isset)                                       \
  OP(Empty)

enum class QueryMOp : uint8_t {
#define OP(name) name,
  QUERY_M_OPS
#undef OP
};

constexpr int32_t kMaxConcatN = 4;

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
  O(RGetCNop,        NA,               ONE(CV),         ONE(RV),    NF) \
  O(Null,            NA,               NOV,             ONE(CV),    NF) \
  O(NullUninit,      NA,               NOV,             ONE(UV),    NF) \
  O(True,            NA,               NOV,             ONE(CV),    NF) \
  O(False,           NA,               NOV,             ONE(CV),    NF) \
  O(Int,             ONE(I64A),        NOV,             ONE(CV),    NF) \
  O(Double,          ONE(DA),          NOV,             ONE(CV),    NF) \
  O(String,          ONE(SA),          NOV,             ONE(CV),    NF) \
  O(Array,           ONE(AA),          NOV,             ONE(CV),    NF) \
  O(NewArray,        ONE(IVA),         NOV,             ONE(CV),    NF) \
  O(NewMixedArray,   ONE(IVA),         NOV,             ONE(CV),    NF) \
  O(NewLikeArrayL,   TWO(LA,IVA),      NOV,             ONE(CV),    NF) \
  O(NewPackedArray,  ONE(IVA),         CMANY,           ONE(CV),    NF) \
  O(NewStructArray,  ONE(VSA),         SMANY,           ONE(CV),    NF) \
  O(AddElemC,        NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(AddElemV,        NA,               THREE(VV,CV,CV), ONE(CV),    NF) \
  O(AddNewElemC,     NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(AddNewElemV,     NA,               TWO(VV,CV),      ONE(CV),    NF) \
  O(NewCol,          ONE(IVA),         NOV,             ONE(CV),    NF) \
  O(ColFromArray,    ONE(IVA),         ONE(CV),         ONE(CV),    NF) \
  O(MapAddElemC,     NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(ColAddNewElemC,  NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Cns,             ONE(SA),          NOV,             ONE(CV),    NF) \
  O(CnsE,            ONE(SA),          NOV,             ONE(CV),    NF) \
  O(CnsU,            TWO(SA,SA),       NOV,             ONE(CV),    NF) \
  O(ClsCns,          ONE(SA),          ONE(AV),         ONE(CV),    NF) \
  O(ClsCnsD,         TWO(SA,SA),       NOV,             ONE(CV),    NF) \
  O(NameA,           NA,               ONE(AV),         ONE(CV),    NF) \
  O(File,            NA,               NOV,             ONE(CV),    NF) \
  O(Dir,             NA,               NOV,             ONE(CV),    NF) \
  O(Concat,          NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(ConcatN,         ONE(IVA),         CMANY,           ONE(CV),    NF) \
  O(Add,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Sub,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Mul,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(AddO,            NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(SubO,            NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(MulO,            NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Div,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Mod,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(Pow,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
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
  O(Cmp,             NA,               TWO(CV,CV),      ONE(CV),    NF) \
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
  O(Fatal,           ONE(OA(FatalOp)), ONE(CV),         NOV,        TF) \
  O(Jmp,             ONE(BA),          NOV,             NOV,        CF_TF) \
  O(JmpNS,           ONE(BA),          NOV,             NOV,        CF_TF) \
  O(JmpZ,            ONE(BA),          ONE(CV),         NOV,        CF) \
  O(JmpNZ,           ONE(BA),          ONE(CV),         NOV,        CF) \
  O(Switch,          THREE(BLA,I64A,OA(SwitchKind)),                    \
                                       ONE(CV),         NOV,        CF_TF) \
  O(SSwitch,         ONE(SLA),         ONE(CV),         NOV,        CF_TF) \
  O(RetC,            NA,               ONE(CV),         NOV,        CF_TF) \
  O(RetV,            NA,               ONE(VV),         NOV,        CF_TF) \
  O(Unwind,          NA,               NOV,             NOV,        TF) \
  O(Throw,           NA,               ONE(CV),         NOV,        TF) \
  O(CGetL,           ONE(LA),          NOV,             ONE(CV),    NF) \
  O(CGetQuietL,      ONE(LA),          NOV,             ONE(CV),    NF) \
  O(CUGetL,          ONE(LA),          NOV,             ONE(CUV),   NF) \
  O(CGetL2,          ONE(LA),          NOV,             INS_1(CV),  NF) \
  O(CGetL3,          ONE(LA),          NOV,             INS_2(CV),  NF) \
  O(PushL,           ONE(LA),          NOV,             ONE(CV),    NF) \
  O(CGetN,           NA,               ONE(CV),         ONE(CV),    NF) \
  O(CGetQuietN,      NA,               ONE(CV),         ONE(CV),    NF) \
  O(CGetG,           NA,               ONE(CV),         ONE(CV),    NF) \
  O(CGetQuietG,      NA,               ONE(CV),         ONE(CV),    NF) \
  O(CGetS,           NA,               TWO(AV,CV),      ONE(CV),    NF) \
  O(VGetL,           ONE(LA),          NOV,             ONE(VV),    NF) \
  O(VGetN,           NA,               ONE(CV),         ONE(VV),    NF) \
  O(VGetG,           NA,               ONE(CV),         ONE(VV),    NF) \
  O(VGetS,           NA,               TWO(AV,CV),      ONE(VV),    NF) \
  O(AGetC,           NA,               ONE(CV),         ONE(AV),    NF) \
  O(AGetL,           ONE(LA),          NOV,             ONE(AV),    NF) \
  O(GetMemoKey,      NA,               ONE(CV),         ONE(CV),    NF) \
  O(AKExists,        NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(IssetL,          ONE(LA),          NOV,             ONE(CV),    NF) \
  O(IssetN,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(IssetG,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(IssetS,          NA,               TWO(AV,CV),      ONE(CV),    NF) \
  O(EmptyL,          ONE(LA),          NOV,             ONE(CV),    NF) \
  O(EmptyN,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(EmptyG,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(EmptyS,          NA,               TWO(AV,CV),      ONE(CV),    NF) \
  O(IsTypeC,         ONE(OA(IsTypeOp)),ONE(CV),         ONE(CV),    NF) \
  O(IsTypeL,         TWO(LA,                                            \
                       OA(IsTypeOp)),  NOV,             ONE(CV),    NF) \
  O(AssertRATL,      TWO(LA,RATA),     NOV,             NOV,        NF) \
  O(AssertRATStk,    TWO(IVA,RATA),    NOV,             NOV,        NF) \
  O(SetL,            ONE(LA),          ONE(CV),         ONE(CV),    NF) \
  O(SetN,            NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(SetG,            NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(SetS,            NA,               THREE(CV,AV,CV), ONE(CV),    NF) \
  O(SetOpL,          TWO(LA,                                            \
                       OA(SetOpOp)),   ONE(CV),         ONE(CV),    NF) \
  O(SetOpN,          ONE(OA(SetOpOp)), TWO(CV,CV),      ONE(CV),    NF) \
  O(SetOpG,          ONE(OA(SetOpOp)), TWO(CV,CV),      ONE(CV),    NF) \
  O(SetOpS,          ONE(OA(SetOpOp)), THREE(CV,AV,CV), ONE(CV),    NF) \
  O(IncDecL,         TWO(LA,                                            \
                       OA(IncDecOp)),  NOV,             ONE(CV),    NF) \
  O(IncDecN,         ONE(OA(IncDecOp)),ONE(CV),         ONE(CV),    NF) \
  O(IncDecG,         ONE(OA(IncDecOp)),ONE(CV),         ONE(CV),    NF) \
  O(IncDecS,         ONE(OA(IncDecOp)),TWO(AV,CV),      ONE(CV),    NF) \
  O(BindL,           ONE(LA),          ONE(VV),         ONE(VV),    NF) \
  O(BindN,           NA,               TWO(VV,CV),      ONE(VV),    NF) \
  O(BindG,           NA,               TWO(VV,CV),      ONE(VV),    NF) \
  O(BindS,           NA,               THREE(VV,AV,CV), ONE(VV),    NF) \
  O(UnsetL,          ONE(LA),          NOV,             NOV,        NF) \
  O(UnsetN,          NA,               ONE(CV),         NOV,        NF) \
  O(UnsetG,          NA,               ONE(CV),         NOV,        NF) \
  /* NOTE: isFPush below relies on the grouping of FPush* here */       \
  O(FPushFunc,       ONE(IVA),         ONE(CV),         NOV,        NF) \
  O(FPushFuncD,      TWO(IVA,SA),      NOV,             NOV,        NF) \
  O(FPushFuncU,      THREE(IVA,SA,SA), NOV,             NOV,        NF) \
  O(FPushObjMethod,  TWO(IVA,                                           \
                       OA(ObjMethodOp)), TWO(CV,CV),    NOV,        NF) \
  O(FPushObjMethodD, THREE(IVA,SA,                                      \
                       OA(ObjMethodOp)), ONE(CV),       NOV,        NF) \
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
  O(FCall,           ONE(IVA),         FMANY,           ONE(RV),    CF_FF) \
  O(FCallAwait,      THREE(IVA,SA,SA), FMANY,           ONE(CV),    CF_FF) \
  O(FCallD,          THREE(IVA,SA,SA), FMANY,           ONE(RV),    CF_FF) \
  O(FCallUnpack,     ONE(IVA),         FMANY,           ONE(RV),    CF_FF) \
  O(FCallArray,      NA,               ONE(FV),         ONE(RV),    CF_FF) \
  O(FCallBuiltin,    THREE(IVA,IVA,SA),CVUMANY,         ONE(RV),    NF) \
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
  O(DefClsNop,       ONE(IVA),         NOV,             NOV,        NF) \
  O(DefCns,          ONE(SA),          ONE(CV),         ONE(CV),    NF) \
  O(DefTypeAlias,    ONE(IVA),         NOV,             NOV,        NF) \
  O(This,            NA,               NOV,             ONE(CV),    NF) \
  O(BareThis,        ONE(OA(BareThisOp)),                               \
                                       NOV,             ONE(CV),    NF) \
  O(CheckThis,       NA,               NOV,             NOV,        NF) \
  O(InitThisLoc,     ONE(LA),          NOV,             NOV,        NF) \
  O(StaticLoc,       TWO(LA,SA),       NOV,             ONE(CV),    NF) \
  O(StaticLocInit,   TWO(LA,SA),       ONE(CV),         NOV,        NF) \
  O(Catch,           NA,               NOV,             ONE(CV),    NF) \
  O(OODeclExists,    ONE(OA(OODeclExistsOp)),                           \
                                       TWO(CV,CV),      ONE(CV),    NF) \
  O(VerifyParamType, ONE(LA),          NOV,             NOV,        NF) \
  O(VerifyRetTypeC,  NA,               ONE(CV),         ONE(CV),    NF) \
  O(VerifyRetTypeV,  NA,               ONE(VV),         ONE(VV),    NF) \
  O(Self,            NA,               NOV,             ONE(AV),    NF) \
  O(Parent,          NA,               NOV,             ONE(AV),    NF) \
  O(LateBoundCls,    NA,               NOV,             ONE(AV),    NF) \
  O(NativeImpl,      NA,               NOV,             NOV,        CF_TF) \
  O(CreateCl,        TWO(IVA,SA),      CVUMANY,         ONE(CV),    NF) \
  O(CreateCont,      NA,               NOV,             ONE(CV),    CF) \
  O(ContEnter,       NA,               ONE(CV),         ONE(CV),    CF) \
  O(ContRaise,       NA,               ONE(CV),         ONE(CV),    CF) \
  O(Yield,           NA,               ONE(CV),         ONE(CV),    CF) \
  O(YieldK,          NA,               TWO(CV,CV),      ONE(CV),    CF) \
  O(ContAssignDelegate,                                                 \
                     ONE(IA),          ONE(CV),         NOV,        NF) \
  O(ContEnterDelegate,                                                  \
                     NA,               ONE(CV),         NOV,        CF) \
  O(YieldFromDelegate,                                                  \
                     TWO(IA, BA),      NOV,             ONE(CV),    CF) \
  O(ContUnsetDelegate,                                                  \
                     TWO(IA, IVA),     NOV,             NOV,        NF) \
  O(ContCheck,       ONE(IVA),         NOV,             NOV,        NF) \
  O(ContValid,       NA,               NOV,             ONE(CV),    NF) \
  O(ContStarted,     NA,               NOV,             ONE(CV),    NF) \
  O(ContKey,         NA,               NOV,             ONE(CV),    NF) \
  O(ContCurrent,     NA,               NOV,             ONE(CV),    NF) \
  O(ContGetReturn,   NA,               NOV,             ONE(CV),    NF) \
  O(WHResult,        NA,               ONE(CV),         ONE(CV),    NF) \
  O(Await,           NA,               ONE(CV),         ONE(CV),    CF) \
  O(IncStat,         TWO(IVA,IVA),     NOV,             NOV,        NF) \
  O(Idx,             NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(ArrayIdx,        NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(CheckProp,       ONE(SA),          NOV,             ONE(CV),    NF) \
  O(InitProp,        TWO(SA,                                            \
                       OA(InitPropOp)),ONE(CV),         NOV,        NF) \
  O(Silence,         TWO(LA,OA(SilenceOp)),                             \
                                       NOV,             NOV,        NF) \
  O(BaseNC,          TWO(IVA, OA(MOpFlags)),                            \
                                       NOV,             NOV,        NF) \
  O(BaseNL,          TWO(LA, OA(MOpFlags)),                             \
                                       NOV,             NOV,        NF) \
  O(BaseGC,          TWO(IVA, OA(MOpFlags)),                            \
                                       NOV,             NOV,        NF) \
  O(BaseGL,          TWO(LA, OA(MOpFlags)),                             \
                                       NOV,             NOV,        NF) \
  O(BaseSC,          TWO(IVA, IVA),    IDX_A,           IDX_A,      NF) \
  O(BaseSL,          TWO(LA, IVA),     IDX_A,           IDX_A,      NF) \
  O(BaseL,           TWO(LA, OA(MOpFlags)),                             \
                                       NOV,             NOV,        NF) \
  O(BaseC,           ONE(IVA),         NOV,             NOV,        NF) \
  O(BaseR,           ONE(IVA),         NOV,             NOV,        NF) \
  O(BaseH,           NA,               NOV,             NOV,        NF) \
  O(FPassBaseNC,     TWO(IVA, IVA),                                     \
                                       NOV,             NOV,        FF) \
  O(FPassBaseNL,     TWO(IVA, LA),                                      \
                                       NOV,             NOV,        FF) \
  O(FPassBaseGC,     TWO(IVA, IVA),                                     \
                                       NOV,             NOV,        FF) \
  O(FPassBaseGL,     TWO(IVA, LA),                                      \
                                       NOV,             NOV,        FF) \
  O(FPassBaseL,      TWO(IVA, LA),                                      \
                                       NOV,             NOV,        FF) \
  O(Dim,             TWO(OA(MOpFlags), KA),                             \
                                       NOV,             NOV,        NF) \
  O(FPassDim,        TWO(IVA, KA),     NOV,             NOV,        FF) \
  O(QueryM,          THREE(IVA, OA(QueryMOp), KA),                      \
                                       MFINAL,          ONE(CV),    NF) \
  O(VGetM,           TWO(IVA, KA),     MFINAL,          ONE(VV),    NF) \
  O(FPassM,          THREE(IVA, IVA, KA),                               \
                                       F_MFINAL,        ONE(FV),    FF) \
  O(SetM,            TWO(IVA, KA),     C_MFINAL,        ONE(CV),    NF) \
  O(IncDecM,         THREE(IVA, OA(IncDecOp), KA),                      \
                                       MFINAL,          ONE(CV),    NF) \
  O(SetOpM,          THREE(IVA, OA(SetOpOp), KA),                       \
                                       C_MFINAL,        ONE(CV),    NF) \
  O(BindM,           TWO(IVA, KA),     V_MFINAL,        ONE(VV),    NF) \
  O(UnsetM,          TWO(IVA, KA),     MFINAL,          NOV,        NF) \
  O(SetWithRefLML,   TWO(LA,LA),       NOV,             NOV,        NF) \
  O(SetWithRefRML,   ONE(LA),          ONE(RV),         NOV,        NF) \
  O(HighInvalid,     NA,               NOV,             NOV,        NF)

enum class Op : uint16_t {
#define O(name, ...) name,
  OPCODES
#undef O
};
auto constexpr Op_count = size_t(Op::HighInvalid) + 1;

/*
 * Also put Op* in the enclosing namespace, to avoid having to change every
 * existing usage site of the enum values.
 */
#define O(name, ...) UNUSED auto constexpr Op##name = Op::name;
  OPCODES
#undef O

// These are comparable by default under MSVC.
#ifndef _MSC_VER
inline constexpr bool operator<(Op a, Op b) { return size_t(a) < size_t(b); }
inline constexpr bool operator>(Op a, Op b) { return size_t(a) > size_t(b); }
inline constexpr bool operator<=(Op a, Op b) {
  return size_t(a) <= size_t(b);
}
inline constexpr bool operator>=(Op a, Op b) {
  return size_t(a) >= size_t(b);
}
#endif

constexpr bool isValidOpcode(Op op) {
  return op > OpLowInvalid && op < OpHighInvalid;
}

inline MOpFlags getQueryMOpFlags(QueryMOp op) {
  switch (op) {
    case QueryMOp::CGet:  return MOpFlags::Warn;
    case QueryMOp::CGetQuiet:
    case QueryMOp::Isset:
    case QueryMOp::Empty: return MOpFlags::None;
  }
  always_assert(false);
}

enum AcoldOp {
  OpAcoldStart = Op_count-1,
#define O(name, imm, pop, push, flags) OpAcold##name,
  OPCODES
#undef O
  OpAcoldCount
};

#define HIGH_OPCODES \
  O(FuncPrologue) \
  O(TraceletGuard)

enum HighOp {
  OpHighStart = OpAcoldCount-1,
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

  const int32_t* vec32() const {
    return reinterpret_cast<const int32_t*>(m_start);
  }
  folly::Range<const int32_t*> range32() const {
    auto base = vec32();
    return {base, base + size()};
  }
  const StrVecItem* strvec() const {
    return reinterpret_cast<const StrVecItem*>(m_start);
  }

  /*
   * Returns the length of the immediate vector in bytes (for M
   * vectors) or elements (for switch vectors)
   */
  int32_t size() const { return m_length; }

  /*
   * Returns the number of elements on the execution stack that this vector
   * will need to access.
   */
  int numStackValues() const { return m_numStack; }

private:
  int32_t m_length;
  int32_t m_numStack;
  const uint8_t* m_start;
};

// Must be an opcode that actually has an ImmVector.
ImmVector getImmVector(PC opcode);

// Some decoding helper functions.
int numImmediates(Op opcode);
ArgType immType(Op opcode, int idx);
int immSize(PC opcode, int idx);
bool immIsVector(Op opcode, int idx);
bool hasImmVector(Op opcode);
int instrLen(PC opcode);
int numSuccs(PC opcode);
bool pushesActRec(Op opcode);

/*
 * The returned struct has normalized variable-sized immediates. u must be
 * provided unless you know that the immediate is not of type KA.
 *
 * Don't use with RATA immediates.
 */
ArgUnion getImm(PC opcode, int idx, const Unit* u = nullptr);

// Don't use this with variable-sized immediates!
ArgUnion* getImmPtr(PC opcode, int idx);

// Encodes a variable sized immediate for `val' into `buf'.  Returns
// the number of bytes used taken.  At most 4 bytes can be used.
size_t encodeVariableSizeImm(int32_t val, unsigned char* buf);

void staticStreamer(const TypedValue* tv, std::stringstream& out);

std::string instrToString(PC it, Either<const Unit*, const UnitEmitter*> u);
void staticArrayStreamer(const ArrayData*, std::ostream&);

/*
 * Convert subopcodes or opcodes into strings.
 */
const char* opcodeToName(Op op);
const char* subopToName(InitPropOp);
const char* subopToName(IsTypeOp);
const char* subopToName(FatalOp);
const char* subopToName(SetOpOp);
const char* subopToName(IncDecOp);
const char* subopToName(BareThisOp);
const char* subopToName(SilenceOp);
const char* subopToName(OODeclExistsOp);
const char* subopToName(ObjMethodOp);
const char* subopToName(SwitchKind);
const char* subopToName(MOpFlags);
const char* subopToName(QueryMOp);

/*
 * Returns true iff the given SubOp is in the valid range for its type.
 */
template<class Subop>
bool subopValid(Subop);

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
Offset* instrJumpOffset(PC instr);

// returns absolute address of target, or InvalidAbsoluteOffset if instruction
//   cannot jump
Offset instrJumpTarget(PC instrs, Offset pos);

/*
 * Returns the set of bytecode offsets for the instructions that may
 * be executed immediately after opc.
 */
using OffsetSet = hphp_hash_set<Offset>;
OffsetSet instrSuccOffsets(PC opc, const Unit* unit);

struct StackTransInfo {
  enum class Kind {
    PushPop,
    InsertMid
  };
  Kind kind;
  int numPops;   // only for PushPop
  int numPushes; // only for PushPop
  int pos;       // only for InsertMid
};

/*
 * Some CF instructions can be treated as non-CF instructions for most analysis
 * purposes, such as bytecode verification and HHBBC. These instructions change
 * vmpc() to point somewhere in a different function, but the runtime
 * guarantees that if excution ever returns to the original frame, it will be
 * at the location immediately following the instruction in question. This
 * creates the illusion that the instruction fell through normally to the
 * instruction after it, within the context of its execution frame.
 *
 * The canonical example of this behavior is the FCall instruction, so we use
 * "non-call control flow" to describe the set of CF instruction that do not
 * exhibit this behavior. This function returns true if `opcode' is a non-call
 * control flow instruction.
 */
bool instrIsNonCallControlFlow(Op opcode);

bool instrAllowsFallThru(Op opcode);
bool instrReadsCurrentFpi(Op opcode);

constexpr InstrFlags instrFlagsData[] = {
#define O(unusedName, unusedImm, unusedPop, unusedPush, flags) flags,
  OPCODES
#undef O
};

constexpr InstrFlags instrFlags(Op opcode) {
  return instrFlagsData[size_t(opcode)];
}

constexpr bool instrIsControlFlow(Op opcode) {
  return (instrFlags(opcode) & CF) != 0;
}

constexpr bool isUnconditionalJmp(Op opcode) {
  return opcode == Op::Jmp || opcode == Op::JmpNS;
}

constexpr bool isConditionalJmp(Op opcode) {
  return opcode == Op::JmpZ || opcode == Op::JmpNZ;
}

constexpr bool isJmp(Op opcode) {
  return opcode >= Op::Jmp && opcode <= Op::JmpNZ;
}

constexpr bool isFPush(Op opcode) {
  return opcode >= OpFPushFunc && opcode <= OpFPushCufSafe;
}

constexpr bool isFPushCuf(Op opcode) {
  return opcode >= OpFPushCufIter && opcode <= OpFPushCufSafe;
}

constexpr bool isFPushClsMethod(Op opcode) {
  return opcode >= OpFPushClsMethod && opcode <= OpFPushClsMethodD;
}

constexpr bool isFPushCtor(Op opcode) {
  return opcode == OpFPushCtor || opcode == OpFPushCtorD;
}

constexpr bool isFPushFunc(Op opcode) {
  return opcode >= OpFPushFunc && opcode <= OpFPushFuncU;
}

inline bool isFCallStar(Op opcode) {
  switch (opcode) {
    case Op::FCall:
    case Op::FCallD:
    case Op::FCallAwait:
    case Op::FCallArray:
    case Op::FCallUnpack:
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
      return true;

    default:
      return false;
  }
}

constexpr bool isRet(Op op) {
  return op == OpRetC || op == OpRetV;
}

constexpr bool isReturnish(Op op) {
  return isRet(op) || op == Op::NativeImpl;
}

constexpr bool isSwitch(Op op) {
  return op == OpSwitch || op == OpSSwitch;
}

constexpr bool isTypeAssert(Op op) {
  return op == OpAssertRATL || op == OpAssertRATStk;
}

inline bool isMemberBaseOp(Op op) {
  switch (op) {
    case Op::BaseNC:
    case Op::BaseNL:
    case Op::BaseGC:
    case Op::BaseGL:
    case Op::FPassBaseNC:
    case Op::FPassBaseNL:
    case Op::FPassBaseGC:
    case Op::FPassBaseGL:
    case Op::BaseSC:
    case Op::BaseSL:
    case Op::BaseL:
    case Op::FPassBaseL:
    case Op::BaseC:
    case Op::BaseR:
    case Op::BaseH:
      return true;

    default:
      return false;
  }
}

inline bool isMemberDimOp(Op op) {
  switch (op) {
    case Op::Dim:
    case Op::FPassDim:
      return true;

    default:
      return false;
  }
}

inline bool isMemberFinalOp(Op op) {
  switch (op) {
    case Op::QueryM:
    case Op::VGetM:
    case Op::FPassM:
    case Op::SetM:
    case Op::IncDecM:
    case Op::SetOpM:
    case Op::BindM:
    case Op::UnsetM:
    case Op::SetWithRefLML:
    case Op::SetWithRefRML:
      return true;

    default:
      return false;
  }
}

int instrNumPops(PC opcode);
int instrNumPushes(PC opcode);
FlavorDesc instrInputFlavor(PC op, uint32_t idx);
StackTransInfo instrStackTransInfo(PC opcode);

/*
 * Delta from FP to top pre-live ActRec.
 */
int instrFpToArDelta(const Func* func, PC opcode);

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
