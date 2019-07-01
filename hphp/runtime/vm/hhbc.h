/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/vm/member-key.h"
#include "hphp/util/compact-vector.h"
#include "hphp/util/either.h"
#include "hphp/util/functional.h"
#include "hphp/util/hash-set.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct Unit;
struct UnitEmitter;
struct Func;

constexpr size_t kMaxHhbcImms = 5;

// A contiguous range of locals. The count is the number of locals
// including the first. If the range is empty, count will be zero and
// first's value is arbitrary.
struct LocalRange {
  uint32_t first;
  uint32_t count;
};


// Arguments to FCall opcodes.
// hhas format: <flags> <numArgs> <numRets> <byRefs> <asyncEagerOffset>
// hhbc format: <uint8:flags> ?<iva:numArgs> ?<iva:numRets>
//              ?<boolvec:byRefs> ?<ba:asyncEagerOffset>
//   flags            = flags (hhas doesn't have HHBC-only flags)
//   numArgs          = flags >> kFirstNumArgsBit
//                        ? flags >> kFirstNumArgsBit - 1 : decode_iva()
//   numRets          = flags & HasInOut ? decode_iva() : 1
//   byRefs           = flags & EnforceReffiness ? decode bool vec : nullptr
//   asyncEagerOffset = flags & HasAEO ? decode_ba() : kInvalidOffset
struct FCallArgsBase {
  enum Flags : uint8_t {
    None                     = 0,
    // Unpack remaining arguments from a varray passed by ...$args.
    HasUnpack                = (1 << 0),
    // Op is not FCallCtor and callee is known to support async eager return.
    // In the encoded form, this bit is re-used to encode constructNoConst
    SupportsAsyncEagerReturn = (1 << 1),
    // HHBC-only: is the number of returns provided? false => 1
    HasInOut                 = (1 << 2),
    // HHBC-only: should this FCall enforce argument reffiness?
    EnforceReffiness         = (1 << 3),
    // HHBC-only: is the async eager offset provided? false => kInvalidOffset
    HasAsyncEagerOffset      = (1 << 4),
    // HHBC-only: the remaining space is used for number of arguments
    NumArgsStart             = (1 << 5),
  };

  // Flags that are valid on FCallArgsBase::flags struct (i.e. non-HHBC-only).
  static constexpr uint8_t kInternalFlags =
    HasUnpack | SupportsAsyncEagerReturn;
  // The first (lowest) bit of numArgs.
  static constexpr uint8_t kFirstNumArgsBit = 5;

  explicit FCallArgsBase(Flags flags, uint32_t numArgs, uint32_t numRets,
                         bool constructNoConst)
    : numArgs(numArgs), numRets(numRets), flags(flags)
      , constructNoConst(constructNoConst) {
    assertx(!(flags & ~kInternalFlags));
    assertx(!(supportsAsyncEagerReturn() && constructNoConst));
  }
  bool hasUnpack() const { return flags & Flags::HasUnpack; }
  uint32_t numArgsInclUnpack() const { return numArgs + (hasUnpack() ? 1 : 0); }
  bool supportsAsyncEagerReturn() const {
    return flags & Flags::SupportsAsyncEagerReturn;
  }
  uint32_t numArgs;
  uint32_t numRets;
  Flags flags;
  bool constructNoConst;
};

struct FCallArgs : FCallArgsBase {
  explicit FCallArgs(Flags flags, uint32_t numArgs, uint32_t numRets,
                     const uint8_t* byRefs, Offset asyncEagerOffset,
                     bool constructNoConst)
    : FCallArgsBase(flags, numArgs, numRets, constructNoConst)
    , asyncEagerOffset(asyncEagerOffset)
    , byRefs(byRefs) {
    assertx(IMPLIES(byRefs != nullptr, numArgs != 0));
    assertx(IMPLIES(asyncEagerOffset == kInvalidOffset,
                    !supportsAsyncEagerReturn()));
  }
  bool enforceReffiness() const { return byRefs != nullptr; }
  bool byRef(uint32_t i) const {
    assertx(enforceReffiness());
    return byRefs[i / 8] & (1 << (i % 8));
  }
  Offset asyncEagerOffset;
  const uint8_t* byRefs;
};

static_assert(1 << FCallArgs::kFirstNumArgsBit == FCallArgs::NumArgsStart, "");

std::string show(const LocalRange&);
std::string show(const FCallArgsBase&, const uint8_t* byRefs,
                 std::string asyncEagerLabel);

/*
 * Variable-size immediates are implemented as follows: To determine which size
 * the immediate is, examine the first byte where the immediate is expected,
 * and examine its high-order bit.  If it is zero, it's a 1-byte immediate
 * and the byte is the value. Otherwise, it's 4 bytes, and bits 8..31 must be
 * logical-shifted to the right by one to get rid of the flag bit.
 *
 * The types in this macro for BLA, SLA, ILA, I32LA and VSA are meaningless
 * since they are never read out of ArgUnion (they use ImmVector).
 *
 * ArgTypes and their various decoding helpers should be kept in sync with the
 * `hhx' bytecode inspection GDB command.
 */
#define ARGTYPES                                                               \
  ARGTYPE(NA,     void*)         /* unused */                                  \
  ARGTYPEVEC(BLA, Offset)        /* Bytecode offset vector immediate */        \
  ARGTYPEVEC(SLA, Id)            /* String id/offset pair vector */            \
  ARGTYPEVEC(ILA, Id)            /* IterKind/IterId pair vector */             \
  ARGTYPEVEC(I32LA,uint32_t)     /* Vector of 32-bit uint */                   \
  ARGTYPE(IVA,    uint32_t)      /* Variable size: 8 or 32-bit uint */         \
  ARGTYPE(I64A,   int64_t)       /* 64-bit Integer */                          \
  ARGTYPE(LA,     int32_t)       /* Local variable ID: 8 or 32-bit int */      \
  ARGTYPE(IA,     int32_t)       /* Iterator ID: 8 or 32-bit int */            \
  ARGTYPE(CAR,    int32_t)       /* Class-ref slot (read): 8 or 32-bit int */  \
  ARGTYPE(CAW,    int32_t)       /* Class-ref slot (write): 8 or 32-bit int */ \
  ARGTYPE(DA,     double)        /* Double */                                  \
  ARGTYPE(SA,     Id)            /* Static string ID */                        \
  ARGTYPE(AA,     Id)            /* Static array ID */                         \
  ARGTYPE(RATA,   RepoAuthType)  /* Statically inferred RepoAuthType */        \
  ARGTYPE(BA,     Offset)        /* Bytecode offset */                         \
  ARGTYPE(OA,     unsigned char) /* Sub-opcode, untyped */                     \
  ARGTYPE(KA,     MemberKey)     /* Member key: local, stack, int, str */      \
  ARGTYPE(LAR,    LocalRange)    /* Contiguous range of locals */              \
  ARGTYPE(FCA,    FCallArgs)     /* FCall arguments */                         \
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
  uint8_t bytes[0];
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
  UV,   // Uninit
  CVV,  // Cell or Var argument
  CUV,  // Cell, or Uninit argument
  CVUV, // Cell, Var, or Uninit argument
};

enum InstrFlags {
  /* No flags. */
  NF = 0x0,

  /* Terminal: next instruction is not reachable via fall through or the callee
   * returning control. This includes instructions like Throw that always throw
   * exceptions. */
  TF = 0x1,

  /* Control flow: If this instruction finishes executing (doesn't throw an
   * exception), vmpc() is not guaranteed to point to the next instruction in
   * the bytecode stream. This does not take VM reentry into account, as that
   * operation is part of the instruction that performed the reentry, and does
   * not affect what vmpc() is set to after the instruction completes. */
  CF = 0x2,

  /* Instruction pushes an FPI */
  PF = 0x4,

  /* Shorthand for common combinations. */
  CF_TF = (CF | TF),
};

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
  ISTYPE_OP(Vec)                               \
  ISTYPE_OP(Dict)                              \
  ISTYPE_OP(Keyset)                            \
  ISTYPE_OP(Obj)                               \
  ISTYPE_OP(Scalar)                            \
  ISTYPE_OP(ArrLike)                           \
  ISTYPE_OP(Res)                               \
  ISTYPE_OP(VArray)                            \
  ISTYPE_OP(DArray)                            \
  ISTYPE_OP(ClsMeth)

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
  KindOfLIter = 1,
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

#define M_OP_MODES                                 \
  MODE(None)                                       \
  MODE(Warn)                                       \
  MODE(Define)                                     \
  MODE(Unset)                                      \
  /* InOut mode restricts allowed bases to the
     array like types. */                          \
  MODE(InOut)

enum class MOpMode : uint8_t {
#define MODE(name) name,
  M_OP_MODES
#undef MODE
};

#define QUERY_M_OPS                               \
  OP(CGet)                                        \
  OP(CGetQuiet)                                   \
  OP(Isset)                                       \
  OP(Empty)                                       \
  OP(InOut)

enum class QueryMOp : uint8_t {
#define OP(name) name,
  QUERY_M_OPS
#undef OP
};

#define SET_RANGE_OPS \
  OP(Forward)         \
  OP(Reverse)

enum class SetRangeOp : uint8_t {
#define OP(name) name,
  SET_RANGE_OPS
#undef OP
};

#define TYPE_STRUCT_RESOLVE_OPS \
  OP(Resolve)                  \
  OP(DontResolve)

enum class TypeStructResolveOp : uint8_t {
#define OP(name) name,
  TYPE_STRUCT_RESOLVE_OPS
#undef OP
};

#define HAS_GENERICS_OPS \
  OP(NoGenerics)         \
  OP(HasGenerics)        \
  OP(MaybeGenerics)

enum class HasGenericsOp : uint8_t {
#define OP(name) name,
  HAS_GENERICS_OPS
#undef OP
};

#define CONT_CHECK_OPS                            \
  CONT_CHECK_OP(IgnoreStarted)                    \
  CONT_CHECK_OP(CheckStarted)

enum class ContCheckOp : uint8_t {
#define CONT_CHECK_OP(name) name,
  CONT_CHECK_OPS
#undef CONT_CHECK_OP
};

#define CUD_OPS                                 \
  CUD_OP(IgnoreIter)                            \
  CUD_OP(FreeIter)

enum class CudOp : uint8_t {
#define CUD_OP(name) name,
  CUD_OPS
#undef CUD_OP
};

#define SPECIAL_CLS_REFS                        \
  REF(Self)                                     \
  REF(Static)                                   \
  REF(Parent)

enum class SpecialClsRef : uint8_t {
#define REF(name) name,
  SPECIAL_CLS_REFS
#undef REF
};

constexpr uint32_t kMaxConcatN = 4;

//  name             immediates        inputs           outputs     flags
#define OPCODES \
  O(Nop,             NA,               NOV,             NOV,        NF) \
  O(EntryNop,        NA,               NOV,             NOV,        NF) \
  O(BreakTraceHint,  NA,               NOV,             NOV,        NF) \
  O(DiscardClsRef,   ONE(CAR),         NOV,             NOV,        NF) \
  O(PopC,            NA,               ONE(CV),         NOV,        NF) \
  O(PopV,            NA,               ONE(VV),         NOV,        NF) \
  O(PopU,            NA,               ONE(UV),         NOV,        NF) \
  O(PopU2,           NA,               TWO(CV,UV),      ONE(CV),    NF) \
  O(PopL,            ONE(LA),          ONE(CV),         NOV,        NF) \
  O(Dup,             NA,               ONE(CV),         TWO(CV,CV), NF) \
  O(CGetCUNop,       NA,               ONE(CUV),        ONE(CV),    NF) \
  O(UGetCUNop,       NA,               ONE(CUV),        ONE(UV),    NF) \
  O(Null,            NA,               NOV,             ONE(CV),    NF) \
  O(NullUninit,      NA,               NOV,             ONE(UV),    NF) \
  O(True,            NA,               NOV,             ONE(CV),    NF) \
  O(False,           NA,               NOV,             ONE(CV),    NF) \
  O(Int,             ONE(I64A),        NOV,             ONE(CV),    NF) \
  O(Double,          ONE(DA),          NOV,             ONE(CV),    NF) \
  O(String,          ONE(SA),          NOV,             ONE(CV),    NF) \
  O(Array,           ONE(AA),          NOV,             ONE(CV),    NF) \
  O(Dict,            ONE(AA),          NOV,             ONE(CV),    NF) \
  O(Keyset,          ONE(AA),          NOV,             ONE(CV),    NF) \
  O(Vec,             ONE(AA),          NOV,             ONE(CV),    NF) \
  O(NewArray,        ONE(IVA),         NOV,             ONE(CV),    NF) \
  O(NewMixedArray,   ONE(IVA),         NOV,             ONE(CV),    NF) \
  O(NewDictArray,    ONE(IVA),         NOV,             ONE(CV),    NF) \
  O(NewLikeArrayL,   TWO(LA,IVA),      NOV,             ONE(CV),    NF) \
  O(NewPackedArray,  ONE(IVA),         CMANY,           ONE(CV),    NF) \
  O(NewStructArray,  ONE(VSA),         SMANY,           ONE(CV),    NF) \
  O(NewStructDArray, ONE(VSA),         SMANY,           ONE(CV),    NF) \
  O(NewStructDict,   ONE(VSA),         SMANY,           ONE(CV),    NF) \
  O(NewVecArray,     ONE(IVA),         CMANY,           ONE(CV),    NF) \
  O(NewKeysetArray,  ONE(IVA),         CMANY,           ONE(CV),    NF) \
  O(NewVArray,       ONE(IVA),         CMANY,           ONE(CV),    NF) \
  O(NewDArray,       ONE(IVA),         NOV,             ONE(CV),    NF) \
  O(NewRecord,       TWO(SA,VSA),      SMANY,           ONE(CV),    NF) \
  O(AddElemC,        NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(AddNewElemC,     NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(NewCol,          ONE(OA(CollectionType)),                           \
                                       NOV,             ONE(CV),    NF) \
  O(NewPair,         NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(ColFromArray,    ONE(OA(CollectionType)),                           \
                                       ONE(CV),         ONE(CV),    NF) \
  O(CnsE,            ONE(SA),          NOV,             ONE(CV),    NF) \
  O(ClsCns,          TWO(SA,CAR),      NOV,             ONE(CV),    NF) \
  O(ClsCnsD,         TWO(SA,SA),       NOV,             ONE(CV),    NF) \
  O(ClsRefName,      ONE(CAR),         NOV,             ONE(CV),    NF) \
  O(File,            NA,               NOV,             ONE(CV),    NF) \
  O(Dir,             NA,               NOV,             ONE(CV),    NF) \
  O(Method,          NA,               NOV,             ONE(CV),    NF) \
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
  O(CastDict,        NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastKeyset,      NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastVec,         NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastVArray,      NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastDArray,      NA,               ONE(CV),         ONE(CV),    NF) \
  O(DblAsBits,       NA,               ONE(CV),         ONE(CV),    NF) \
  O(InstanceOf,      NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(InstanceOfD,     ONE(SA),          ONE(CV),         ONE(CV),    NF) \
  O(IsLateBoundCls,  NA,               ONE(CV),         ONE(CV),    NF) \
  O(IsTypeStructC,   ONE(OA(TypeStructResolveOp)),                      \
                                       TWO(CV,CV),      ONE(CV),    NF) \
  O(ThrowAsTypeStructException,                                         \
                     NA,               TWO(CV,CV),      NOV,        TF) \
  O(CombineAndResolveTypeStruct,                                        \
                     ONE(IVA),         CMANY,           ONE(CV),    NF) \
  O(Select,          NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(Print,           NA,               ONE(CV),         ONE(CV),    NF) \
  O(Clone,           NA,               ONE(CV),         ONE(CV),    NF) \
  O(Exit,            NA,               ONE(CV),         ONE(CV),    TF) \
  O(Fatal,           ONE(OA(FatalOp)), ONE(CV),         NOV,        TF) \
  O(Jmp,             ONE(BA),          NOV,             NOV,        CF_TF) \
  O(JmpNS,           ONE(BA),          NOV,             NOV,        CF_TF) \
  O(JmpZ,            ONE(BA),          ONE(CV),         NOV,        CF) \
  O(JmpNZ,           ONE(BA),          ONE(CV),         NOV,        CF) \
  O(Switch,          THREE(OA(SwitchKind),I64A,BLA),                    \
                                       ONE(CV),         NOV,        CF_TF) \
  O(SSwitch,         ONE(SLA),         ONE(CV),         NOV,        CF_TF) \
  O(RetC,            NA,               ONE(CV),         NOV,        CF_TF) \
  O(RetM,            ONE(IVA),         CMANY,           NOV,        CF_TF) \
  O(RetCSuspended,   NA,               ONE(CV),         NOV,        CF_TF) \
  O(Throw,           NA,               ONE(CV),         NOV,        CF_TF) \
  O(CGetL,           ONE(LA),          NOV,             ONE(CV),    NF) \
  O(CGetQuietL,      ONE(LA),          NOV,             ONE(CV),    NF) \
  O(CUGetL,          ONE(LA),          NOV,             ONE(CUV),   NF) \
  O(CGetL2,          ONE(LA),          ONE(CV),         TWO(CV,CV), NF) \
  O(PushL,           ONE(LA),          NOV,             ONE(CV),    NF) \
  O(CGetG,           NA,               ONE(CV),         ONE(CV),    NF) \
  O(CGetQuietG,      NA,               ONE(CV),         ONE(CV),    NF) \
  O(CGetS,           ONE(CAR),         ONE(CV),         ONE(CV),    NF) \
  O(VGetL,           ONE(LA),          NOV,             ONE(VV),    NF) \
  O(ClsRefGetC,      ONE(CAW),         ONE(CV),         NOV,        NF) \
  O(ClsRefGetTS,     ONE(CAW),         ONE(CV),         NOV,        NF) \
  O(GetMemoKeyL,     ONE(LA),          NOV,             ONE(CV),    NF) \
  O(AKExists,        NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(IssetL,          ONE(LA),          NOV,             ONE(CV),    NF) \
  O(IssetG,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(IssetS,          ONE(CAR),         ONE(CV),         ONE(CV),    NF) \
  O(EmptyL,          ONE(LA),          NOV,             ONE(CV),    NF) \
  O(EmptyG,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(EmptyS,          ONE(CAR),         ONE(CV),         ONE(CV),    NF) \
  O(IsTypeC,         ONE(OA(IsTypeOp)),ONE(CV),         ONE(CV),    NF) \
  O(IsTypeL,         TWO(LA,                                            \
                       OA(IsTypeOp)),  NOV,             ONE(CV),    NF) \
  O(AssertRATL,      TWO(LA,RATA),     NOV,             NOV,        NF) \
  O(AssertRATStk,    TWO(IVA,RATA),    NOV,             NOV,        NF) \
  O(SetL,            ONE(LA),          ONE(CV),         ONE(CV),    NF) \
  O(SetG,            NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(SetS,            ONE(CAR),         TWO(CV,CV),      ONE(CV),    NF) \
  O(SetOpL,          TWO(LA,                                            \
                       OA(SetOpOp)),   ONE(CV),         ONE(CV),    NF) \
  O(SetOpG,          ONE(OA(SetOpOp)), TWO(CV,CV),      ONE(CV),    NF) \
  O(SetOpS,          TWO(OA(SetOpOp),CAR),                              \
                                       TWO(CV,CV),      ONE(CV),    NF) \
  O(IncDecL,         TWO(LA,                                            \
                       OA(IncDecOp)),  NOV,             ONE(CV),    NF) \
  O(IncDecG,         ONE(OA(IncDecOp)),ONE(CV),         ONE(CV),    NF) \
  O(IncDecS,         TWO(OA(IncDecOp),CAR),                             \
                                       ONE(CV),         ONE(CV),    NF) \
  O(UnsetL,          ONE(LA),          NOV,             NOV,        NF) \
  O(UnsetG,          NA,               ONE(CV),         NOV,        NF) \
                                                                        \
  O(ResolveFunc,     ONE(SA),          NOV,             ONE(CV),    NF) \
  O(ResolveObjMethod,NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(ResolveClsMethod,NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(NewObj,          TWO(CAR,OA(HasGenericsOp)),                        \
                                       NOV,             ONE(CV),    NF) \
  O(NewObjD,         ONE(SA),          NOV,             ONE(CV),    NF) \
  O(NewObjRD,        ONE(SA),          ONE(CV),         ONE(CV),    NF) \
  O(NewObjS,         ONE(OA(SpecialClsRef)),                            \
                                       NOV,             ONE(CV),    NF) \
  O(LockObj,         NA,               ONE(CV),         ONE(CV),    NF) \
  O(FPushFunc,       TWO(IVA,I32LA),   FPUSH(1, 0),     FPUSH,      PF) \
  O(FPushFuncD,      TWO(IVA,SA),      FPUSH(0, 0),     FPUSH,      PF) \
  O(FPushFuncRD,     TWO(IVA,SA),      FPUSH(1, 0),     FPUSH,      PF) \
  O(FCallCtor,       TWO(FCA,SA),      FCALL(0, 1),     FCALL,      CF) \
  O(FCallObjMethod,  FOUR(FCA,SA,OA(ObjMethodOp),I32LA),                \
                                       FCALL(1, 1),     FCALL,      CF) \
  O(FCallObjMethodD, FOUR(FCA,SA,OA(ObjMethodOp),SA),                   \
                                       FCALL(0, 1),     FCALL,      CF) \
  O(FCallObjMethodRD,FOUR(FCA,SA,OA(ObjMethodOp),SA),                   \
                                       FCALL(1, 1),     FCALL,      CF) \
  O(FPushClsMethod,  THREE(IVA,CAR,I32LA),                              \
                                       FPUSH(1, 0),     FPUSH,      PF) \
  O(FPushClsMethodS, THREE(IVA,OA(SpecialClsRef),I32LA),                \
                                       FPUSH(1, 0),     FPUSH,      PF) \
  O(FPushClsMethodSD,THREE(IVA,OA(SpecialClsRef),SA),                   \
                                       FPUSH(0, 0),     FPUSH,      PF) \
  O(FPushClsMethodSRD,THREE(IVA,OA(SpecialClsRef),SA),                  \
                                       FPUSH(1, 0),     FPUSH,      PF) \
  O(FPushClsMethodD, THREE(IVA,SA,SA), FPUSH(0, 0),     FPUSH,      PF) \
  O(FPushClsMethodRD,THREE(IVA,SA,SA), FPUSH(1, 0),     FPUSH,      PF) \
  O(FCall,           THREE(FCA,SA,SA), FCALLO,          FCALL,      CF) \
  O(FCallBuiltin,    THREE(IVA,IVA,SA),CVUMANY,         ONE(CV),    NF) \
  O(IterInit,        THREE(IA,BA,LA),  ONE(CV),         NOV,        CF) \
  O(LIterInit,       FOUR(IA,LA,BA,LA),NOV,             NOV,        CF) \
  O(IterInitK,       FOUR(IA,BA,LA,LA),ONE(CV),         NOV,        CF) \
  O(LIterInitK,      FIVE(IA,LA,BA,LA,LA),NOV,          NOV,        CF) \
  O(IterNext,        THREE(IA,BA,LA),  NOV,             NOV,        CF) \
  O(LIterNext,       FOUR(IA,LA,BA,LA),NOV,             NOV,        CF) \
  O(IterNextK,       FOUR(IA,BA,LA,LA),NOV,             NOV,        CF) \
  O(LIterNextK,      FIVE(IA,LA,BA,LA,LA),NOV,          NOV,        CF) \
  O(IterFree,        ONE(IA),          NOV,             NOV,        NF) \
  O(LIterFree,       TWO(IA,LA),       NOV,             NOV,        NF) \
  O(IterBreak,       TWO(BA,ILA),      NOV,             NOV,        CF_TF) \
  O(Incl,            NA,               ONE(CV),         ONE(CV),    CF) \
  O(InclOnce,        NA,               ONE(CV),         ONE(CV),    CF) \
  O(Req,             NA,               ONE(CV),         ONE(CV),    CF) \
  O(ReqOnce,         NA,               ONE(CV),         ONE(CV),    CF) \
  O(ReqDoc,          NA,               ONE(CV),         ONE(CV),    CF) \
  O(Eval,            NA,               ONE(CV),         ONE(CV),    CF) \
  O(DefCls,          ONE(IVA),         NOV,             NOV,        NF) \
  O(DefClsNop,       ONE(IVA),         NOV,             NOV,        NF) \
  O(DefRecord,       ONE(IVA),         NOV,             NOV,        NF) \
  O(AliasCls,        TWO(SA,SA),       ONE(CV),         ONE(CV),    NF) \
  O(DefCns,          ONE(SA),          ONE(CV),         ONE(CV),    NF) \
  O(DefTypeAlias,    ONE(IVA),         NOV,             NOV,        NF) \
  O(This,            NA,               NOV,             ONE(CV),    NF) \
  O(BareThis,        ONE(OA(BareThisOp)),                               \
                                       NOV,             ONE(CV),    NF) \
  O(CheckThis,       NA,               NOV,             NOV,        NF) \
  O(InitThisLoc,     ONE(LA),          NOV,             NOV,        NF) \
  O(ChainFaults,     NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(OODeclExists,    ONE(OA(OODeclExistsOp)),                           \
                                       TWO(CV,CV),      ONE(CV),    NF) \
  O(VerifyOutType,   ONE(IVA),         ONE(CV),         ONE(CV),    NF) \
  O(VerifyParamType, ONE(LA),          NOV,             NOV,        NF) \
  O(VerifyParamTypeTS, ONE(LA),        ONE(CV),         NOV,        NF) \
  O(VerifyRetTypeC,  NA,               ONE(CV),         ONE(CV),    NF) \
  O(VerifyRetTypeTS, NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(VerifyRetNonNullC, NA,             ONE(CV),         ONE(CV),    NF) \
  O(Self,            ONE(CAW),         NOV,             NOV,        NF) \
  O(Parent,          ONE(CAW),         NOV,             NOV,        NF) \
  O(LateBoundCls,    ONE(CAW),         NOV,             NOV,        NF) \
  O(RecordReifiedGeneric, NA,          ONE(CV),         ONE(CV),    NF) \
  O(ReifiedName,     ONE(SA),          ONE(CV),         ONE(CV),    NF) \
  O(CheckReifiedGenericMismatch, NA,   ONE(CV),         NOV,        NF) \
  O(NativeImpl,      NA,               NOV,             NOV,        CF_TF) \
  O(CreateCl,        TWO(IVA,IVA),     CUMANY,          ONE(CV),    NF) \
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
                     TWO(IA,BA),       NOV,             ONE(CV),    CF) \
  O(ContUnsetDelegate, TWO(OA(CudOp),IA), NOV,          NOV,        NF) \
  O(ContCheck,       ONE(OA(ContCheckOp)), NOV,         NOV,        NF) \
  O(ContValid,       NA,               NOV,             ONE(CV),    NF) \
  O(ContKey,         NA,               NOV,             ONE(CV),    NF) \
  O(ContCurrent,     NA,               NOV,             ONE(CV),    NF) \
  O(ContGetReturn,   NA,               NOV,             ONE(CV),    NF) \
  O(WHResult,        NA,               ONE(CV),         ONE(CV),    NF) \
  O(Await,           NA,               ONE(CV),         ONE(CV),    CF) \
  O(AwaitAll,        ONE(LAR),         NOV,             ONE(CV),    CF) \
  O(Idx,             NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(ArrayIdx,        NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(CheckProp,       ONE(SA),          NOV,             ONE(CV),    NF) \
  O(InitProp,        TWO(SA,                                            \
                       OA(InitPropOp)),ONE(CV),         NOV,        NF) \
  O(Silence,         TWO(LA,OA(SilenceOp)),                             \
                                       NOV,             NOV,        NF) \
  O(BaseGC,          TWO(IVA, OA(MOpMode)),                             \
                                       NOV,             NOV,        NF) \
  O(BaseGL,          TWO(LA, OA(MOpMode)),                              \
                                       NOV,             NOV,        NF) \
  O(BaseSC,          THREE(IVA, CAR, OA(MOpMode)),                      \
                                       NOV,             NOV,        NF) \
  O(BaseL,           TWO(LA, OA(MOpMode)),                              \
                                       NOV,             NOV,        NF) \
  O(BaseC,           TWO(IVA, OA(MOpMode)),                             \
                                       NOV,             NOV,        NF) \
  O(BaseH,           NA,               NOV,             NOV,        NF) \
  O(Dim,             TWO(OA(MOpMode), KA),                              \
                                       NOV,             NOV,        NF) \
  O(QueryM,          THREE(IVA, OA(QueryMOp), KA),                      \
                                       MFINAL,          ONE(CV),    NF) \
  O(SetM,            TWO(IVA, KA),     C_MFINAL(1),     ONE(CV),    NF) \
  O(SetRangeM,       THREE(IVA, OA(SetRangeOp), IVA),                   \
                                       C_MFINAL(3),     NOV,        NF) \
  O(IncDecM,         THREE(IVA, OA(IncDecOp), KA),                      \
                                       MFINAL,          ONE(CV),    NF) \
  O(SetOpM,          THREE(IVA, OA(SetOpOp), KA),                       \
                                       C_MFINAL(1),     ONE(CV),    NF) \
  O(UnsetM,          TWO(IVA, KA),     MFINAL,          NOV,        NF) \
  O(MemoGet,         TWO(BA, LAR),     NOV,             ONE(CV),    CF) \
  O(MemoGetEager,    THREE(BA, BA, LAR),                                \
                                       NOV,             ONE(CV),    CF) \
  O(MemoSet,         ONE(LAR),         ONE(CV),         ONE(CV),    NF) \
  O(MemoSetEager,    ONE(LAR),         ONE(CV),         ONE(CV),    NF)

enum class Op : uint16_t {
#define O(name, ...) name,
  OPCODES
#undef O
};

#define O(...) + 1
constexpr size_t Op_count = OPCODES;
#undef O

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
  return size_t(op) < Op_count;
}

inline MOpMode getQueryMOpMode(QueryMOp op) {
  switch (op) {
    case QueryMOp::CGet:  return MOpMode::Warn;
    case QueryMOp::CGetQuiet:
    case QueryMOp::Isset:
    case QueryMOp::Empty: return MOpMode::None;
    case QueryMOp::InOut: return MOpMode::InOut;
  }
  always_assert(false);
}

#define HIGH_OPCODES \
  O(FuncPrologue) \
  O(TraceletGuard)

enum HighOp {
  OpHighStart = Op_count-1,
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

struct IterTableEnt {
  IterKind kind;
  int32_t id;
  int32_t local;
};
using IterTable = CompactVector<IterTableEnt>;

// Must be an opcode that actually has an ImmVector.
ImmVector getImmVector(PC opcode);

// Must be an opcode that actually has an IterTable.
IterTable getIterTable(PC opcode);

// Some decoding helper functions.
int numImmediates(Op opcode);
ArgType immType(Op opcode, int idx);
bool hasImmVector(Op opcode);
bool hasIterTable(Op opcode);
int instrLen(PC opcode);
int numSuccs(PC opcode);

PC skipCall(PC pc);
IterTable iterTableFromStream(PC&);

/*
 * The returned struct has normalized variable-sized immediates. u must be
 * provided unless you know that the immediate is not of type KA.
 *
 * Don't use with RATA immediates.
 */
ArgUnion getImm(PC opcode, int idx, const Unit* u = nullptr);

// Don't use this with variable-sized immediates!
ArgUnion* getImmPtr(PC opcode, int idx);

void staticStreamer(const TypedValue* tv, std::string& out);

std::string instrToString(PC it, Either<const Unit*, const UnitEmitter*> u);
void staticArrayStreamer(const ArrayData*, std::string&);

/*
 * Convert subopcodes or opcodes into strings.
 */
const char* opcodeToName(Op op);
const char* subopToName(InitPropOp);
const char* subopToName(IsTypeOp);
const char* subopToName(FatalOp);
const char* subopToName(CollectionType);
const char* subopToName(SetOpOp);
const char* subopToName(IncDecOp);
const char* subopToName(BareThisOp);
const char* subopToName(SilenceOp);
const char* subopToName(OODeclExistsOp);
const char* subopToName(ObjMethodOp);
const char* subopToName(SwitchKind);
const char* subopToName(MOpMode);
const char* subopToName(QueryMOp);
const char* subopToName(SetRangeOp);
const char* subopToName(TypeStructResolveOp);
const char* subopToName(HasGenericsOp);
const char* subopToName(ContCheckOp);
const char* subopToName(CudOp);
const char* subopToName(SpecialClsRef);

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

using OffsetList = std::vector<Offset>;

// Returns a jump offsets relative to the instruction, or nothing if
// the instruction cannot jump.
OffsetList instrJumpOffsets(PC instr);

// returns absolute address of targets, or nothing if instruction
// cannot jump
OffsetList instrJumpTargets(PC instrs, Offset pos);

/*
 * Returns the set of bytecode offsets for the instructions that may
 * be executed immediately after opc.
 */
using OffsetSet = hphp_hash_set<Offset>;
OffsetSet instrSuccOffsets(PC opc, const Func* func);

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
  return
    opcode == Op::Jmp   ||
    opcode == Op::JmpNS ||
    opcode == Op::JmpZ  ||
    opcode == Op::JmpNZ;
}

constexpr bool isFCallObjMethod(Op opcode) {
  return
    opcode == OpFCallObjMethod ||
    opcode == OpFCallObjMethodD ||
    opcode == OpFCallObjMethodRD;
}

constexpr bool isNewFCall(Op opcode) {
  return
    opcode == OpFCallCtor ||
    isFCallObjMethod(opcode);
}

constexpr bool isLegacyFPush(Op opcode) {
  return (instrFlags(opcode) & PF) != 0;
}

constexpr bool hasFPushEffects(Op opcode) {
  return isLegacyFPush(opcode) || isNewFCall(opcode);
}

constexpr bool isFPushClsMethod(Op opcode) {
  return
    opcode == OpFPushClsMethod  ||
    opcode == OpFPushClsMethodS ||
    opcode == OpFPushClsMethodSD ||
    opcode == OpFPushClsMethodSRD ||
    opcode == OpFPushClsMethodRD ||
    opcode == OpFPushClsMethodD;
}

constexpr bool isFPushFunc(Op opcode) {
  return opcode == OpFPushFunc || opcode == OpFPushFuncD ||
         opcode == OpFPushFuncRD;
}

constexpr bool isLegacyFCall(Op opcode) {
  return opcode == Op::FCall;
}

constexpr bool hasFCallEffects(Op opcode) {
  return isLegacyFCall(opcode) || isNewFCall(opcode);
}

constexpr bool isRet(Op op) {
  return op == OpRetC || op == OpRetCSuspended || op == OpRetM;
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
    case Op::BaseGC:
    case Op::BaseGL:
    case Op::BaseSC:
    case Op::BaseL:
    case Op::BaseC:
    case Op::BaseH:
      return true;

    default:
      return false;
  }
}

inline bool isMemberDimOp(Op op) {
  return op == Op::Dim;
}

inline bool isMemberFinalOp(Op op) {
  switch (op) {
    case Op::QueryM:
    case Op::SetM:
    case Op::SetRangeM:
    case Op::IncDecM:
    case Op::SetOpM:
    case Op::UnsetM:
      return true;

    default:
      return false;
  }
}

inline bool isMemberOp(Op op) {
  return isMemberBaseOp(op) || isMemberDimOp(op) || isMemberFinalOp(op);
}

inline MOpMode finalMemberOpMode(Op op) {
  switch(op){
    case Op::SetM:
    case Op::SetRangeM:
    case Op::IncDecM:
    case Op::SetOpM:
      return MOpMode::Define;
    case Op::UnsetM:
      return MOpMode::Unset;
    case Op::QueryM:
      return MOpMode::None;
    default:
      always_assert_flog(
        false, "Unknown final member op {}", opcodeToName(op)
      );
  }
}

// true if the opcode body can set pc=0 to halt the interpreter.
constexpr bool instrCanHalt(Op op) {
  return op == OpRetC || op == OpNativeImpl ||
         op == OpAwait || op == OpAwaitAll || op == OpCreateCont ||
         op == OpYield || op == OpYieldK || op == OpRetM ||
         op == OpRetCSuspended || op == OpYieldFromDelegate;
}

int instrNumPops(PC opcode);
int instrNumPushes(PC opcode);
FlavorDesc instrInputFlavor(PC op, uint32_t idx);

}

//////////////////////////////////////////////////////////////////////

namespace std {
template<>
struct hash<HPHP::Op> {
  size_t operator()(HPHP::Op op) const {
    return HPHP::hash_int64(size_t(op));
  }
};
}

//////////////////////////////////////////////////////////////////////

#endif
