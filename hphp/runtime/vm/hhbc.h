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

#pragma once

#include <type_traits>

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
struct FuncEmitter;

constexpr size_t kMaxHhbcImms = 6;

// A contiguous range of locals. The count is the number of locals
// including the first. If the range is empty, count will be zero and
// first's value is arbitrary.
struct LocalRange {
  uint32_t first;
  uint32_t count;
};

/*
 * Arguments to IterInit / IterNext opcodes.
 * hhas format: <iterId> K:<keyId> V:<valId> (for key-value iters)
 *              <iterId> NK V:<valId>        (for value-only iters)
 * hhbc format: <uint8:flags> <iva:iterId> <iva:(keyId + 1)> <iva:valId>
 *
 * For value-only iters, keyId will be -1 (an invalid local ID); to take
 * advantage of the one-byte encoding for IVA arguments, we add 1 to the key
 * when encoding these args in the hhbc format.
 *
 * We don't accept flags from hhas because our flags require analyses that we
 * currently only do in HHBBC.
 */
struct IterArgs {
  enum Flags : uint8_t {
    None      = 0,
    // The base is stored in a local, and that local is unmodified in the loop.
    BaseConst = (1 << 0),
  };

  static constexpr int32_t kNoKey = -1;

  explicit IterArgs(Flags flags, int32_t iterId, int32_t keyId, int32_t valId)
    : iterId(iterId), keyId(keyId), valId(valId), flags(flags) {}

  bool hasKey() const {
    assertx(keyId == kNoKey || keyId >= 0);
    return keyId != kNoKey;
  };

  bool operator==(const IterArgs& other) const {
    return iterId == other.iterId && keyId == other.keyId &&
           valId == other.valId && flags == other.flags;
  }

  int32_t iterId;
  int32_t keyId;
  int32_t valId;
  Flags flags;
};

// Arguments to FCall opcodes.
// hhas format: <flags> <numArgs> <numRets> <inoutArgs> <asyncEagerOffset>
// hhbc format: <uint8:flags> ?<iva:numArgs> ?<iva:numRets>
//              ?<boolvec:inoutArgs> ?<ba:asyncEagerOffset>
//   flags            = flags (hhas doesn't have HHBC-only flags)
//   numArgs          = flags >> kFirstNumArgsBit
//                        ? flags >> kFirstNumArgsBit - 1 : decode_iva()
//   numRets          = flags & HasInOut ? decode_iva() : 1
//   inoutArgs        = flags & EnforceInOut ? decode bool vec : nullptr
//   asyncEagerOffset = flags & HasAEO ? decode_ba() : kInvalidOffset
struct FCallArgsBase {
  enum Flags : uint8_t {
    None                     = 0,
    // Unpack remaining arguments from a varray passed by ...$args.
    HasUnpack                = (1 << 0),
    // Pass generics to the callee.
    HasGenerics              = (1 << 1),
    // Lock newly constructed object if unwinding the constructor call.
    LockWhileUnwinding       = (1 << 2),
    // Arguments are known to be compatible with prologue of the callee and
    // do not need to be repacked.
    SkipRepack               = (1 << 3),
    // HHBC-only: Op should be resolved using an explicit context class
    ExplicitContext          = (1 << 4),
    // HHBC-only: is the number of returns provided? false => 1
    HasInOut                 = (1 << 5),
    // HHBC-only: should this FCall enforce argument inout-ness?
    EnforceInOut             = (1 << 6),
    // HHBC-only: is the async eager offset provided? false => kInvalidOffset
    HasAsyncEagerOffset      = (1 << 7),
  };

  // Flags that are valid on FCallArgsBase::flags struct (i.e. non-HHBC-only).
  static constexpr uint8_t kInternalFlags =
    HasUnpack | HasGenerics | LockWhileUnwinding | SkipRepack;

  explicit FCallArgsBase(Flags flags, uint32_t numArgs, uint32_t numRets)
    : numArgs(numArgs)
    , numRets(numRets)
    , flags(flags)
  {
    assertx(!(flags & ~kInternalFlags));
  }
  bool hasUnpack() const { return flags & Flags::HasUnpack; }
  bool hasGenerics() const { return flags & Flags::HasGenerics; }
  bool lockWhileUnwinding() const { return flags & Flags::LockWhileUnwinding; }
  bool skipRepack() const { return flags & Flags::SkipRepack; }
  uint32_t numInputs() const {
    return numArgs + (hasUnpack() ? 1 : 0) + (hasGenerics() ? 1 : 0);
  }
  uint32_t numArgs;
  uint32_t numRets;
  Flags flags;
};

struct FCallArgs : FCallArgsBase {
  explicit FCallArgs(Flags flags, uint32_t numArgs, uint32_t numRets,
                     const uint8_t* inoutArgs, Offset asyncEagerOffset,
                     const StringData* context)
    : FCallArgsBase(flags, numArgs, numRets)
    , asyncEagerOffset(asyncEagerOffset)
    , inoutArgs(inoutArgs)
    , context(context) {
    assertx(IMPLIES(inoutArgs != nullptr, numArgs != 0));
  }
  bool enforceInOut() const { return inoutArgs != nullptr; }
  bool isInOut(uint32_t i) const {
    assertx(enforceInOut());
    return inoutArgs[i / 8] & (1 << (i % 8));
  }
  FCallArgs withGenerics() const {
    assertx(!hasGenerics());
    return FCallArgs(
      static_cast<Flags>(flags | Flags::HasGenerics),
      numArgs, numRets, inoutArgs, asyncEagerOffset, context);
  }
  Offset asyncEagerOffset;
  const uint8_t* inoutArgs;
  const StringData* context;
};

using PrintLocal = std::function<std::string(int32_t local)>;
std::string show(const IterArgs&, PrintLocal);

std::string show(const LocalRange&);
std::string show(const FCallArgsBase&, const uint8_t* inoutArgs,
                 std::string asyncEagerLabel, const StringData* ctx);

/*
 * Variable-size immediates are implemented as follows: To determine which size
 * the immediate is, examine the first byte where the immediate is expected,
 * and examine its high-order bit.  If it is zero, it's a 1-byte immediate
 * and the byte is the value. Otherwise, it's 4 bytes, and bits 8..31 must be
 * logical-shifted to the right by one to get rid of the flag bit.
 *
 * The types in this macro for BLA, SLA, and VSA are meaningless since they
 * are never read out of ArgUnion (they use ImmVector).
 *
 * There are several different local immediate types:
 *   - LA immediates are for bytecodes that only require the TypedValue* to
 *     perform their operation.
 *   - ILA immediates are used by bytecodes that need both the TypedValue* and
 *     the slot index to implement their operation.  This could be used by
 *     opcodes that print an error message including this slot info.
 *   - NLA immediates are used by bytecodes that need both the TypedValue* and
 *     the name of the local to be implemented.  This is commonly used for
 *     ops that raise warnings for undefined local uses.
 *
 * ArgTypes and their various decoding helpers should be kept in sync with the
 * `hhx' bytecode inspection GDB command.
 */
#define ARGTYPES                                                               \
  ARGTYPE(NA,     void*)         /* unused */                                  \
  ARGTYPEVEC(BLA, Offset)        /* Bytecode offset vector immediate */        \
  ARGTYPEVEC(SLA, Id)            /* String id/offset pair vector */            \
  ARGTYPE(IVA,    uint32_t)      /* Variable size: 8 or 32-bit uint */         \
  ARGTYPE(I64A,   int64_t)       /* 64-bit Integer */                          \
  ARGTYPE(LA,     int32_t)       /* Local: 8 or 32-bit int */                  \
  ARGTYPE(NLA,    NamedLocal)    /* Local w/ name: 2x 8 or 32-bit int */       \
  ARGTYPE(ILA,    int32_t)       /* Local w/ ID: 8 or 32-bit int */            \
  ARGTYPE(IA,     int32_t)       /* Iterator ID: 8 or 32-bit int */            \
  ARGTYPE(DA,     double)        /* Double */                                  \
  ARGTYPE(SA,     Id)            /* Static string ID */                        \
  ARGTYPE(AA,     Id)            /* Static array ID */                         \
  ARGTYPE(RATA,   RepoAuthType)  /* Statically inferred RepoAuthType */        \
  ARGTYPE(BA,     Offset)        /* Bytecode offset */                         \
  ARGTYPE(OA,     unsigned char) /* Sub-opcode, untyped */                     \
  ARGTYPE(KA,     MemberKey)     /* Member key: local, stack, int, str */      \
  ARGTYPE(LAR,    LocalRange)    /* Contiguous range of locals */              \
  ARGTYPE(ITA,    IterArgs)      /* Iterator arguments */                      \
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

enum FlavorDesc {
  NOV,  // None
  CV,   // TypedValue
  UV,   // Uninit
  CUV,  // TypedValue, or Uninit argument
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
  ISTYPE_OP(Vec)                               \
  ISTYPE_OP(Dict)                              \
  ISTYPE_OP(Keyset)                            \
  ISTYPE_OP(Obj)                               \
  ISTYPE_OP(Scalar)                            \
  ISTYPE_OP(ArrLike)                           \
  ISTYPE_OP(LegacyArrLike)                     \
  ISTYPE_OP(Res)                               \
  ISTYPE_OP(ClsMeth)                           \
  ISTYPE_OP(Func)                              \
  ISTYPE_OP(Class)

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

#define IS_LOG_AS_DYNAMIC_CALL_OPS                  \
  IS_LOG_AS_DYNAMIC_CALL_OP(LogAsDynamicCall)       \
  IS_LOG_AS_DYNAMIC_CALL_OP(DontLogAsDynamicCall)

enum class IsLogAsDynamicCallOp : uint8_t {
#define IS_LOG_AS_DYNAMIC_CALL_OP(name) name,
  IS_LOG_AS_DYNAMIC_CALL_OPS
#undef IS_LOG_AS_DYNAMIC_CALL_OP
};

constexpr uint32_t kMaxConcatN = 4;

//  name             immediates        inputs           outputs     flags
#define OPCODES \
  O(Nop,             NA,               NOV,             NOV,        NF) \
  O(EntryNop,        NA,               NOV,             NOV,        NF) \
  O(BreakTraceHint,  NA,               NOV,             NOV,        NF) \
  O(PopC,            NA,               ONE(CV),         NOV,        NF) \
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
  O(FuncCred,        NA,               NOV,             ONE(CV),    NF) \
  O(Int,             ONE(I64A),        NOV,             ONE(CV),    NF) \
  O(Double,          ONE(DA),          NOV,             ONE(CV),    NF) \
  O(String,          ONE(SA),          NOV,             ONE(CV),    NF) \
  O(Dict,            ONE(AA),          NOV,             ONE(CV),    NF) \
  O(Keyset,          ONE(AA),          NOV,             ONE(CV),    NF) \
  O(Vec,             ONE(AA),          NOV,             ONE(CV),    NF) \
  O(NewDictArray,    ONE(IVA),         NOV,             ONE(CV),    NF) \
  O(NewStructDict,   ONE(VSA),         SMANY,           ONE(CV),    NF) \
  O(NewVec,          ONE(IVA),         CMANY,           ONE(CV),    NF) \
  O(NewKeysetArray,  ONE(IVA),         CMANY,           ONE(CV),    NF) \
  O(NewRecord,       TWO(SA,VSA),      SMANY,           ONE(CV),    NF) \
  O(AddElemC,        NA,               THREE(CV,CV,CV), ONE(CV),    NF) \
  O(AddNewElemC,     NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(NewCol,          ONE(OA(CollectionType)),                           \
                                       NOV,             ONE(CV),    NF) \
  O(NewPair,         NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(ColFromArray,    ONE(OA(CollectionType)),                           \
                                       ONE(CV),         ONE(CV),    NF) \
  O(CnsE,            ONE(SA),          NOV,             ONE(CV),    NF) \
  O(ClsCns,          ONE(SA),          ONE(CV),         ONE(CV),    NF) \
  O(ClsCnsD,         TWO(SA,SA),       NOV,             ONE(CV),    NF) \
  O(ClsCnsL,         ONE(LA),          ONE(CV),         ONE(CV),    NF) \
  O(ClassName,       NA,               ONE(CV),         ONE(CV),    NF) \
  O(LazyClassFromClass, NA,            ONE(CV),         ONE(CV),    NF) \
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
  O(CastDict,        NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastKeyset,      NA,               ONE(CV),         ONE(CV),    NF) \
  O(CastVec,         NA,               ONE(CV),         ONE(CV),    NF) \
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
  O(CGetL,           ONE(NLA),         NOV,             ONE(CV),    NF) \
  O(CGetQuietL,      ONE(LA),          NOV,             ONE(CV),    NF) \
  O(CUGetL,          ONE(LA),          NOV,             ONE(CUV),   NF) \
  O(CGetL2,          ONE(NLA),         ONE(CV),         TWO(CV,CV), NF) \
  O(PushL,           ONE(LA),          NOV,             ONE(CV),    NF) \
  O(CGetG,           NA,               ONE(CV),         ONE(CV),    NF) \
  O(CGetS,           ONE(OA(ReadOnlyOp)),                               \
                     TWO(CV,CV),      ONE(CV),    NF) \
  O(ClassGetC,       NA,               ONE(CV),         ONE(CV),    NF) \
  O(ClassGetTS,      NA,               ONE(CV),         TWO(CV,CV), NF) \
  O(GetMemoKeyL,     ONE(NLA),         NOV,             ONE(CV),    NF) \
  O(AKExists,        NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(IssetL,          ONE(LA),          NOV,             ONE(CV),    NF) \
  O(IssetG,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(IssetS,          NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(IsUnsetL,        ONE(LA),          NOV,             ONE(CV),    NF) \
  O(IsTypeC,         ONE(OA(IsTypeOp)),ONE(CV),         ONE(CV),    NF) \
  O(IsTypeL,         TWO(NLA,                                           \
                       OA(IsTypeOp)),  NOV,             ONE(CV),    NF) \
  O(AssertRATL,      TWO(ILA,RATA),    NOV,             NOV,        NF) \
  O(AssertRATStk,    TWO(IVA,RATA),    NOV,             NOV,        NF) \
  O(SetL,            ONE(LA),          ONE(CV),         ONE(CV),    NF) \
  O(SetG,            NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(SetS,            ONE(OA(ReadOnlyOp)),                               \
                                       THREE(CV,CV,CV), ONE(CV),    NF) \
  O(SetOpL,          TWO(LA,                                            \
                       OA(SetOpOp)),   ONE(CV),         ONE(CV),    NF) \
  O(SetOpG,          ONE(OA(SetOpOp)), TWO(CV,CV),      ONE(CV),    NF) \
  O(SetOpS,          ONE(OA(SetOpOp)), THREE(CV,CV,CV), ONE(CV),    NF) \
  O(IncDecL,         TWO(NLA, OA(IncDecOp)),                            \
                                       NOV,             ONE(CV),    NF) \
  O(IncDecG,         ONE(OA(IncDecOp)),ONE(CV),         ONE(CV),    NF) \
  O(IncDecS,         ONE(OA(IncDecOp)),TWO(CV,CV),      ONE(CV),    NF) \
  O(UnsetL,          ONE(LA),          NOV,             NOV,        NF) \
  O(UnsetG,          NA,               ONE(CV),         NOV,        NF) \
                                                                        \
  O(ResolveFunc,     ONE(SA),          NOV,             ONE(CV),    NF) \
  O(ResolveMethCaller,ONE(SA),         NOV,             ONE(CV),    NF) \
  O(ResolveRFunc,    ONE(SA),          ONE(CV),         ONE(CV),    NF) \
  O(ResolveObjMethod,NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(ResolveClsMethod,ONE(SA),          ONE(CV),         ONE(CV),    NF) \
  O(ResolveClsMethodD,                                                  \
                     TWO(SA,SA),       NOV,             ONE(CV),    NF) \
  O(ResolveClsMethodS,                                                  \
                     TWO(OA(SpecialClsRef),SA),                         \
                                       NOV,             ONE(CV),    NF) \
  O(ResolveRClsMethod,                                                  \
                     ONE(SA),          TWO(CV,CV),      ONE(CV),    NF) \
  O(ResolveRClsMethodD,                                                 \
                     TWO(SA,SA),       ONE(CV),         ONE(CV),    NF) \
  O(ResolveRClsMethodS,                                                 \
                     TWO(OA(SpecialClsRef),SA),                         \
                                       ONE(CV),         ONE(CV),    NF) \
  O(ResolveClass,    ONE(SA),          NOV,             ONE(CV),    NF) \
  O(LazyClass,       ONE(SA),          NOV,             ONE(CV),    NF) \
  O(NewObj,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(NewObjR,         NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(NewObjD,         ONE(SA),          NOV,             ONE(CV),    NF) \
  O(NewObjRD,        ONE(SA),          ONE(CV),         ONE(CV),    NF) \
  O(NewObjS,         ONE(OA(SpecialClsRef)),                            \
                                       NOV,             ONE(CV),    NF) \
  O(LockObj,         NA,               ONE(CV),         ONE(CV),    NF) \
  O(FCallClsMethod,  THREE(FCA,SA,OA(IsLogAsDynamicCallOp)),            \
                                       FCALL(2, 0),     FCALL,      CF) \
  O(FCallClsMethodD, FOUR(FCA,SA,SA,SA),                                \
                                       FCALL(0, 0),     FCALL,      CF) \
  O(FCallClsMethodS, THREE(FCA,SA,OA(SpecialClsRef)),                   \
                                       FCALL(1, 0),     FCALL,      CF) \
  O(FCallClsMethodSD,FOUR(FCA,SA,OA(SpecialClsRef),SA),                 \
                                       FCALL(0, 0),     FCALL,      CF) \
  O(FCallCtor,       TWO(FCA,SA),      FCALL(0, 1),     FCALL,      CF) \
  O(FCallFunc,       ONE(FCA),         FCALL(1, 0),     FCALL,      CF) \
  O(FCallFuncD,      TWO(FCA,SA),      FCALL(0, 0),     FCALL,      CF) \
  O(FCallObjMethod,  THREE(FCA,SA,OA(ObjMethodOp)),                     \
                                       FCALL(1, 1),     FCALL,      CF) \
  O(FCallObjMethodD, FOUR(FCA,SA,OA(ObjMethodOp),SA),                   \
                                       FCALL(0, 1),     FCALL,      CF) \
  O(IterInit,        TWO(ITA,BA),      ONE(CV),         NOV,        CF) \
  O(LIterInit,       THREE(ITA,LA,BA), NOV,             NOV,        CF) \
  O(IterNext,        TWO(ITA,BA),      NOV,             NOV,        CF) \
  O(LIterNext,       THREE(ITA,LA,BA), NOV,             NOV,        CF) \
  O(IterFree,        ONE(IA),          NOV,             NOV,        NF) \
  O(LIterFree,       TWO(IA,LA),       NOV,             NOV,        NF) \
  O(Incl,            NA,               ONE(CV),         ONE(CV),    NF) \
  O(InclOnce,        NA,               ONE(CV),         ONE(CV),    NF) \
  O(Req,             NA,               ONE(CV),         ONE(CV),    NF) \
  O(ReqOnce,         NA,               ONE(CV),         ONE(CV),    NF) \
  O(ReqDoc,          NA,               ONE(CV),         ONE(CV),    NF) \
  O(Eval,            NA,               ONE(CV),         ONE(CV),    NF) \
  O(This,            NA,               NOV,             ONE(CV),    NF) \
  O(BareThis,        ONE(OA(BareThisOp)),                               \
                                       NOV,             ONE(CV),    NF) \
  O(CheckThis,       NA,               NOV,             NOV,        NF) \
  O(ChainFaults,     NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(OODeclExists,    ONE(OA(OODeclExistsOp)),                           \
                                       TWO(CV,CV),      ONE(CV),    NF) \
  O(VerifyOutType,   ONE(IVA),         ONE(CV),         ONE(CV),    NF) \
  O(VerifyParamType, ONE(ILA),         NOV,             NOV,        NF) \
  O(VerifyParamTypeTS, ONE(ILA),       ONE(CV),         NOV,        NF) \
  O(VerifyRetTypeC,  NA,               ONE(CV),         ONE(CV),    NF) \
  O(VerifyRetTypeTS, NA,               TWO(CV,CV),      ONE(CV),    NF) \
  O(VerifyRetNonNullC, NA,             ONE(CV),         ONE(CV),    NF) \
  O(Self,            NA,               NOV,             ONE(CV),    NF) \
  O(Parent,          NA,               NOV,             ONE(CV),    NF) \
  O(LateBoundCls,    NA,               NOV,             ONE(CV),    NF) \
  O(RecordReifiedGeneric, NA,          ONE(CV),         ONE(CV),    NF) \
  O(CheckReifiedGenericMismatch, NA,   ONE(CV),         NOV,        NF) \
  O(NativeImpl,      NA,               NOV,             NOV,        CF_TF) \
  O(CreateCl,        TWO(IVA,IVA),     CUMANY,          ONE(CV),    NF) \
  O(CreateCont,      NA,               NOV,             ONE(CV),    CF) \
  O(ContEnter,       NA,               ONE(CV),         ONE(CV),    CF) \
  O(ContRaise,       NA,               ONE(CV),         ONE(CV),    CF) \
  O(Yield,           NA,               ONE(CV),         ONE(CV),    CF) \
  O(YieldK,          NA,               TWO(CV,CV),      ONE(CV),    CF) \
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
  O(ArrayMarkLegacy,    NA,            TWO(CV,CV),      ONE(CV),    NF) \
  O(ArrayUnmarkLegacy,  NA,            TWO(CV,CV),      ONE(CV),    NF) \
  O(CheckProp,       ONE(SA),          NOV,             ONE(CV),    NF) \
  O(InitProp,        TWO(SA, OA(InitPropOp)),                           \
                                       ONE(CV),         NOV,        NF) \
  O(Silence,         TWO(LA,OA(SilenceOp)),                             \
                                       NOV,             NOV,        NF) \
  O(ThrowNonExhaustiveSwitch, NA,      NOV,             NOV,        NF) \
  O(RaiseClassStringConversionWarning,                                  \
                              NA,      NOV,             NOV,        NF) \
  O(BaseGC,          TWO(IVA, OA(MOpMode)),                             \
                                       NOV,             NOV,        NF) \
  O(BaseGL,          TWO(LA, OA(MOpMode)),                              \
                                       NOV,             NOV,        NF) \
  O(BaseSC,          FOUR(IVA, IVA, OA(MOpMode), OA(ReadOnlyOp)),       \
                                       NOV,             NOV,        NF) \
  O(BaseL,           TWO(NLA, OA(MOpMode)),                             \
                                       NOV,             NOV,        NF) \
  O(BaseC,           TWO(IVA, OA(MOpMode)),                             \
                                       NOV,             NOV,        NF) \
  O(BaseH,           NA,               NOV,             NOV,        NF) \
  O(Dim,             TWO(OA(MOpMode), KA),                              \
                                       NOV,             NOV,        NF) \
  O(QueryM,          THREE(IVA, OA(QueryMOp), KA),                      \
                                       MFINAL,          ONE(CV),    NF) \
  O(SetM,            TWO(IVA, KA),     C_MFINAL(1),     ONE(CV),    NF) \
  O(SetRangeM,       THREE(IVA, IVA, OA(SetRangeOp)),                   \
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
    case QueryMOp::Isset: return MOpMode::None;
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

// Must be an opcode that actually has an ImmVector.
ImmVector getImmVector(PC opcode);

// Some decoding helper functions.
int numImmediates(Op opcode);
ArgType immType(Op opcode, int idx);
bool hasImmVector(Op opcode);
int instrLen(PC opcode);
int numSuccs(PC opcode);

PC skipCall(PC pc);

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

std::string instrToString(PC it, Either<const Func*, const FuncEmitter*> f);
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
const char* subopToName(ContCheckOp);
const char* subopToName(CudOp);
const char* subopToName(SpecialClsRef);
const char* subopToName(IsLogAsDynamicCallOp);
const char* subopToName(ReadOnlyOp);

/*
 * Returns true iff the given SubOp is in the valid range for its type.
 */
template<class Subop>
bool subopValid(Subop);

/*
 * Try to parse a string into a subop name of a given type.
 *
 * Returns std::nullopt if the string is not recognized as that type of
 * subop.
 */
template<class SubOpType>
Optional<SubOpType> nameToSubop(const char*);

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
 * The canonical example of this behavior are the FCall* instructions, so we use
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

constexpr bool isObjectConstructorOp(Op opcode) {
  return
    opcode == Op::NewObj ||
    opcode == Op::NewObjD ||
    opcode == Op::NewObjR ||
    opcode == Op::NewObjRD ||
    opcode == Op::NewObjS;
}

constexpr bool isArrLikeConstructorOp(Op opcode) {
  return
    opcode == Op::Dict ||
    opcode == Op::Keyset ||
    opcode == Op::Vec ||
    opcode == Op::NewDictArray ||
    opcode == Op::NewStructDict ||
    opcode == Op::NewVec ||
    opcode == Op::NewKeysetArray;
}

constexpr bool isArrLikeCastOp(Op opcode) {
  return
    opcode == Op::CastVec ||
    opcode == Op::CastDict ||
    opcode == Op::CastKeyset;
}

constexpr bool isComparisonOp(Op opcode) {
  return
    opcode == Op::Cmp ||
    opcode == Op::Eq ||
    opcode == Op::Neq ||
    opcode == Op::Gt ||
    opcode == Op::Gte ||
    opcode == Op::Lt ||
    opcode == Op::Lte ||
    opcode == Op::Same ||
    opcode == Op::NSame;
}

constexpr bool isFCallClsMethod(Op opcode) {
  return
    opcode == OpFCallClsMethod ||
    opcode == OpFCallClsMethodD ||
    opcode == OpFCallClsMethodS ||
    opcode == OpFCallClsMethodSD;
}

constexpr bool isFCallFunc(Op opcode) {
  return
    opcode == OpFCallFunc ||
    opcode == OpFCallFuncD;
}

constexpr bool isFCallObjMethod(Op opcode) {
  return
    opcode == OpFCallObjMethod ||
    opcode == OpFCallObjMethodD;
}

constexpr bool isFCall(Op opcode) {
  return
    opcode == OpFCallCtor ||
    isFCallClsMethod(opcode) ||
    isFCallFunc(opcode) ||
    isFCallObjMethod(opcode);
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

constexpr bool isIteratorOp(Op op) {
  return op == OpIterInit || op == Op::LIterInit ||
         op == OpIterNext || op == Op::LIterNext;
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
         op == OpRetCSuspended;
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
