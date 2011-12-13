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

#ifndef incl_VM_HHBC_H_
#define incl_VM_HHBC_H_

#include <runtime/vm/core_types.h>

namespace HPHP {
namespace VM {

struct Unit;

// Variable-size immediates are implemented as follows. To determine which size
// the immediate is, examine the first byte where the immediate is expected, and
// examine its low-order bit. If it is zero, it's a 1-byte immediate; otherwise,
// it's 4 bytes. The immediate has to be logical-shifted to the right by one to
// get rid of the flag bit.
#define ARGTYPES \
  ARGTYPE(NA,    void*)         /* unused */  \
  ARGTYPEVEC(LA, int32_t)       /* Vector immediate */ \
  ARGTYPE(IVA,   int32_t)       /* variable size: 8 or 32-bit integer */  \
  ARGTYPE(I64A,  int64_t)       /* 64-bit Integer */ \
  ARGTYPE(DA,    double)        /* Double */ \
  ARGTYPE(SA,    Id)            /* litStr ID */ \
  ARGTYPE(AA,    Id)            /* static array ID */ \
  ARGTYPE(BA,    Offset)        /* Bytecode address */ \
  ARGTYPE(OA,    unsigned char) /* Opcode */

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

static const Offset InvalidAbsoluteOffset = -1;

enum Protoflavor {
  NOV = 0, // None
  CV = 1, // Cell
  VV = 2, // Var
  HV = 3, // Home
  AV = 4, // Classref
  RV = 5, // Return value (cell or var)
  FV = 6, // Function parameter (cell or var)
};

enum InstrFlags {
  NF = 0x0, // No flags
  UF = 0x1, // Unconditional control transfer flag
  CF = 0x2, // Conditional control flow
  FF = 0x4, // Instruction reads current FPI
};

enum LocationCode {
  LH = 0x0,
  LN = 0x1,
  LG = 0x2,
  LS = 0x3,
  LC = 0x4,
  LR = 0x5
};

enum MemberCode {
  ME = 0x0,
  MW = 0x1,
  MP = 0x2,
};

enum IncDecOp {
  PreInc,
  PostInc,
  PreDec,
  PostDec,
  IncDec_invalid
};

// Each of the setop ops maps to a binary bytecode op. We have reasons
// for using distinct bitwise representations, though. This macro records
// their correspondence for mapping either direction.
#define SETOP_OPS \
  SETOP_OP(SetOpPlusEqual,   OpAdd) \
  SETOP_OP(SetOpMinusEqual,  OpSub) \
  SETOP_OP(SetOpMulEqual,    OpMul) \
  SETOP_OP(SetOpConcatEqual, OpConcat) \
  SETOP_OP(SetOpDivEqual,    OpDiv) \
  SETOP_OP(SetOpModEqual,    OpMod) \
  SETOP_OP(SetOpAndEqual,    OpBitAnd) \
  SETOP_OP(SetOpOrEqual,     OpBitOr) \
  SETOP_OP(SetOpXorEqual,    OpBitXor) \
  SETOP_OP(SetOpSlEqual,     OpShl) \
  SETOP_OP(SetOpSrEqual,     OpShr)

enum SetOpOp {
#define SETOP_OP(setOpOp, bcOp) setOpOp,
  SETOP_OPS
#undef SETOP_OP
  SetOp_invalid
};

//  name             immediates         inputs            outputs      flags
#define OPCODES \
  O(LowInvalid,      NA,                NOV,              NOV,         NF) \
  O(Nop,             NA,                NOV,              NOV,         NF) \
  O(PopC,            NA,                ONE(CV),          NOV,         NF) \
  O(PopV,            NA,                ONE(VV),          NOV,         NF) \
  O(PopR,            NA,                ONE(RV),          NOV,         NF) \
  O(Dup,             NA,                ONE(CV),          TWO(CV,CV),  NF) \
  O(Box,             NA,                ONE(CV),          ONE(VV),     NF) \
  O(Unbox,           NA,                ONE(VV),          ONE(CV),     NF) \
  O(BoxR,            NA,                ONE(RV),          ONE(VV),     NF) \
  O(UnboxR,          NA,                ONE(RV),          ONE(CV),     NF) \
  O(Null,            NA,                NOV,              ONE(CV),     NF) \
  O(True,            NA,                NOV,              ONE(CV),     NF) \
  O(False,           NA,                NOV,              ONE(CV),     NF) \
  O(Int,             ONE(I64A),         NOV,              ONE(CV),     NF) \
  O(Double,          ONE(DA),           NOV,              ONE(CV),     NF) \
  O(String,          ONE(SA),           NOV,              ONE(CV),     NF) \
  O(Array,           ONE(AA),           NOV,              ONE(CV),     NF) \
  O(NewArray,        NA,                NOV,              ONE(CV),     NF) \
  O(AddElemC,        NA,                THREE(CV,CV,CV),  ONE(CV),     NF) \
  O(AddElemV,        NA,                THREE(VV,CV,CV),  ONE(CV),     NF) \
  O(AddNewElemC,     NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(AddNewElemV,     NA,                TWO(VV,CV),       ONE(CV),     NF) \
  O(Cns,             ONE(SA),           NOV,              ONE(CV),     NF) \
  O(ClsCns,          ONE(SA),           ONE(AV),          ONE(CV),     NF) \
  O(ClsCnsD,         TWO(SA,SA),        NOV,              ONE(CV),     NF) \
  O(Concat,          NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(Add,             NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(Sub,             NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(Mul,             NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(Div,             NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(Mod,             NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(And,             NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(Or,              NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(Xor,             NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(Not,             NA,                ONE(CV),          ONE(CV),     NF) \
  O(Same,            NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(NSame,           NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(Eq,              NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(Neq,             NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(Lt,              NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(Lte,             NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(Gt,              NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(Gte,             NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(BitAnd,          NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(BitOr,           NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(BitXor,          NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(BitNot,          NA,                ONE(CV),          ONE(CV),     NF) \
  O(Shl,             NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(Shr,             NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(CastBool,        NA,                ONE(CV),          ONE(CV),     NF) \
  O(CastInt,         NA,                ONE(CV),          ONE(CV),     NF) \
  O(CastDouble,      NA,                ONE(CV),          ONE(CV),     NF) \
  O(CastString,      NA,                ONE(CV),          ONE(CV),     NF) \
  O(CastArray,       NA,                ONE(CV),          ONE(CV),     NF) \
  O(CastObject,      NA,                ONE(CV),          ONE(CV),     NF) \
  O(InstanceOf,      NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(InstanceOfD,     ONE(SA),           ONE(CV),          ONE(CV),     NF) \
  O(Print,           NA,                ONE(CV),          ONE(CV),     NF) \
  O(Clone,           NA,                ONE(CV),          ONE(CV),     NF) \
  O(Exit,            NA,                ONE(CV),          ONE(CV),     NF) \
  O(Raise,           NA,                TWO(CV,CV),       NOV,         NF) \
  O(Fatal,           NA,                ONE(CV),          NOV,         UF) \
  O(Jmp,             ONE(BA),           NOV,              NOV,         UF) \
  O(JmpZ,            ONE(BA),           ONE(CV),          NOV,         CF) \
  O(JmpNZ,           ONE(BA),           ONE(CV),          NOV,         CF) \
  O(RetC,            NA,                ONE(CV),          NOV,         UF) \
  O(RetV,            NA,                ONE(VV),          NOV,         UF) \
  O(Unwind,          NA,                NOV,              NOV,         UF) \
  O(Throw,           NA,                ONE(CV),          NOV,         UF) \
  O(Loc,             ONE(IVA),          NOV,              ONE(HV),     NF) \
  O(Cls,             ONE(IVA),          POS_N(CV),        POS_N(AV),   NF) \
  O(ClsH,            ONE(IVA),          POS_N(HV),        POS_N(AV),   NF) \
  O(CGetH,           NA,                ONE(HV),          ONE(CV),     NF) \
  O(CGetH2,          NA,                POS_1(HV),        POS_1(CV),   NF) \
  O(CGetN,           NA,                ONE(CV),          ONE(CV),     NF) \
  O(CGetG,           NA,                ONE(CV),          ONE(CV),     NF) \
  O(CGetS,           NA,                TWO(CV,AV),       ONE(CV),     NF) \
  O(CGetM,           ONE(LA),           LMANY(),          ONE(CV),     NF) \
  O(VGetH,           NA,                ONE(HV),          ONE(VV),     NF) \
  O(VGetN,           NA,                ONE(CV),          ONE(VV),     NF) \
  O(VGetG,           NA,                ONE(CV),          ONE(VV),     NF) \
  O(VGetS,           NA,                TWO(CV,AV),       ONE(VV),     NF) \
  O(VGetM,           ONE(LA),           LMANY(),          ONE(VV),     NF) \
  O(IssetH,          NA,                ONE(HV),          ONE(CV),     NF) \
  O(IssetN,          NA,                ONE(CV),          ONE(CV),     NF) \
  O(IssetG,          NA,                ONE(CV),          ONE(CV),     NF) \
  O(IssetS,          NA,                TWO(CV,AV),       ONE(CV),     NF) \
  O(IssetM,          ONE(LA),           LMANY(),          ONE(CV),     NF) \
  O(EmptyH,          NA,                ONE(HV),          ONE(CV),     NF) \
  O(EmptyN,          NA,                ONE(CV),          ONE(CV),     NF) \
  O(EmptyG,          NA,                ONE(CV),          ONE(CV),     NF) \
  O(EmptyS,          NA,                TWO(CV,AV),       ONE(CV),     NF) \
  O(EmptyM,          ONE(LA),           LMANY(),          ONE(CV),     NF) \
  O(SetH,            NA,                TWO(CV,HV),       ONE(CV),     NF) \
  O(SetN,            NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(SetG,            NA,                TWO(CV,CV),       ONE(CV),     NF) \
  O(SetS,            NA,                THREE(CV,CV,AV),  ONE(CV),     NF) \
  O(SetM,            ONE(LA),           C_LMANY(),        ONE(CV),     NF) \
  O(SetOpH,          ONE(OA),           TWO(CV,HV),       ONE(CV),     NF) \
  O(SetOpN,          ONE(OA),           TWO(CV,CV),       ONE(CV),     NF) \
  O(SetOpG,          ONE(OA),           TWO(CV,CV),       ONE(CV),     NF) \
  O(SetOpS,          ONE(OA),           THREE(CV,CV,AV),  ONE(CV),     NF) \
  O(SetOpM,          TWO(OA,LA),        C_LMANY(),        ONE(CV),     NF) \
  O(IncDecH,         ONE(OA),           ONE(HV),          ONE(CV),     NF) \
  O(IncDecN,         ONE(OA),           ONE(CV),          ONE(CV),     NF) \
  O(IncDecG,         ONE(OA),           ONE(CV),          ONE(CV),     NF) \
  O(IncDecS,         ONE(OA),           TWO(CV,AV),       ONE(CV),     NF) \
  O(IncDecM,         TWO(OA,LA),        LMANY(),          ONE(CV),     NF) \
  O(BindH,           NA,                TWO(VV,HV),       ONE(VV),     NF) \
  O(BindN,           NA,                TWO(VV,CV),       ONE(VV),     NF) \
  O(BindG,           NA,                TWO(VV,CV),       ONE(VV),     NF) \
  O(BindS,           NA,                THREE(VV,CV,AV),  ONE(VV),     NF) \
  O(BindM,           ONE(LA),           V_LMANY(),        ONE(VV),     NF) \
  O(UnsetH,          NA,                ONE(HV),          NOV,         NF) \
  O(UnsetN,          NA,                ONE(CV),          NOV,         NF) \
  O(UnsetG,          NA,                ONE(CV),          NOV,         NF) \
  O(UnsetM,          ONE(LA),           LMANY(),          NOV,         NF) \
  O(FPushFunc,       ONE(IVA),          ONE(CV),          NOV,         NF) \
  O(FPushFuncD,      TWO(IVA,SA),       NOV,              NOV,         NF) \
  O(FPushObjMethod,  ONE(IVA),          TWO(CV,CV),       NOV,         NF) \
  O(FPushObjMethodD, TWO(IVA,SA),       ONE(CV),          NOV,         NF) \
  O(FPushClsMethod,  ONE(IVA),          TWO(CV,AV),       NOV,         NF) \
  O(FPushClsMethodD, THREE(IVA,SA,SA),  NOV,              NOV,         NF) \
  O(FPushClsMethodF, ONE(IVA),          TWO(CV,AV),       NOV,         NF) \
  O(FPushCtor,       ONE(IVA),          ONE(AV),          ONE(CV),     NF) \
  O(FPushCtorD,      TWO(IVA,SA),       NOV,              ONE(CV),     NF) \
  O(FPassC,          ONE(IVA),          ONE(CV),          ONE(FV),     FF) \
  O(FPassCW,         ONE(IVA),          ONE(CV),          ONE(FV),     FF) \
  O(FPassCE,         ONE(IVA),          ONE(CV),          ONE(FV),     FF) \
  O(FPassV,          ONE(IVA),          ONE(VV),          ONE(FV),     FF) \
  O(FPassR,          ONE(IVA),          ONE(RV),          ONE(FV),     FF) \
  O(FPassH,          ONE(IVA),          ONE(HV),          ONE(FV),     FF) \
  O(FPassN,          ONE(IVA),          ONE(CV),          ONE(FV),     FF) \
  O(FPassG,          ONE(IVA),          ONE(CV),          ONE(FV),     FF) \
  O(FPassS,          ONE(IVA),          TWO(CV,AV),       ONE(FV),     FF) \
  O(FPassM,          TWO(IVA,LA),       LMANY(),          ONE(FV),     FF) \
  O(FCall,           ONE(IVA),          FMANY,            ONE(RV),     FF) \
  O(IterInit,        TWO(IVA,BA),       ONE(CV),          NOV,         NF) \
  O(IterInitM,       TWO(IVA,BA),       ONE(VV),          NOV,         NF) \
  O(IterValueC,      ONE(IVA),          NOV,              ONE(CV),     NF) \
  O(IterValueV,      ONE(IVA),          NOV,              ONE(VV),     NF) \
  O(IterKey,         ONE(IVA),          NOV,              ONE(CV),     NF) \
  O(IterNext,        TWO(IVA,BA),       NOV,              NOV,         NF) \
  O(IterFree,        ONE(IVA),          NOV,              NOV,         NF) \
  O(Incl,            NA,                ONE(CV),          ONE(CV),     NF) \
  O(InclOnce,        NA,                ONE(CV),          ONE(CV),     NF) \
  O(Req,             NA,                ONE(CV),          ONE(CV),     NF) \
  O(ReqOnce,         NA,                ONE(CV),          ONE(CV),     NF) \
  O(Eval,            NA,                ONE(CV),          ONE(CV),     NF) \
  O(DefFunc,         ONE(IVA),          NOV,              NOV,         NF) \
  O(DefCls,          ONE(IVA),          NOV,              NOV,         NF) \
  O(This,            NA,                NOV,              ONE(CV),     NF) \
  O(InitThisLoc,     ONE(IVA),          NOV,              NOV,         NF) \
  O(StaticLoc,       TWO(IVA,SA),       NOV,              ONE(CV),     NF) \
  O(StaticLocInit,   TWO(IVA,SA),       ONE(CV),          NOV,         NF) \
  O(Catch,           NA,                NOV,              ONE(CV),     NF) \
  O(LateBoundCls,    NA,                NOV,              ONE(AV),     NF) \
  O(VerifyParamType, ONE(IVA),          NOV,              NOV,         NF) \
  O(HighInvalid,     NA,                NOV,              NOV,         NF) \

enum Op {
#define O(name, imm, pop, push, flags) Op##name,
  OPCODES
#undef O
  Op_count
};

#define MAX_STACK_LIMIT	128

struct ImmVector {
  int32_t len;
  uchar get(int idx) const {
    return *(((uchar*)this) + sizeof(int32_t) + idx);
  };
  int numValues() const;
};

/* Some decoding helper functions. */
int numImmediates(Opcode opcode);
int immType(const Opcode* opcode, int idx);
int immSize(const Opcode* opcode, int idx);
bool immIsVector(const Opcode* opcode, int idx);
int instrLen(const Opcode* opcode);
InstrFlags instrFlags(Opcode opcode);

// The returned struct has normalized variable-sized immediates
ArgUnion getImm(const Opcode* opcode, int idx);
// Don't use this with variable-sized immediates!
ArgUnion* getImmPtr(const Opcode* opcode, int idx);
ImmVector* getImmVector(const Opcode* opcode);

// Pass a pointer to the pointer to the immediate; this function will advance
// the pointer past the immediate
int32 decodeVariableSizeImm(unsigned char** immPtr);

void staticStreamer(TypedValue* tv, std::stringstream& out);

std::string instrToString(const Opcode* it, const Unit* u = NULL);

// returns a pointer to the location within the bytecode containing the jump
//   Offset, or NULL if the instruction cannot jump. Note that this offset is
//   relative to the current instruction.
Offset* instrJumpOffset(Opcode* instr);

// returns absolute address of target, or InvalidAbsoluteOffset if instruction
//   cannot jump
Offset instrJumpTarget(const Opcode* instrs, Offset pos);

struct StackTransInfo {
  enum Kind {
    PushPop,
    PeekPoke
  };
  Kind kind;
  int numPops;
  int numPushes;
  int pos;
};

bool instrIsControlFlow(Opcode opcode);
bool instrReadsCurrentFpi(Opcode opcode);

int instrNumPops(const Opcode* opcode);
int instrNumPushes(const Opcode* opcode);
StackTransInfo instrStackTransInfo(const Opcode* opcode);
int instrSpToArDelta(const Opcode* opcode);

} }

#endif
