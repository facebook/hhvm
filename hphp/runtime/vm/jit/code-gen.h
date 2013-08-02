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

#ifndef incl_HPHP_VM_CG_H_
#define incl_HPHP_VM_CG_H_

#include <vector>
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/ir-factory.h"
#include "hphp/runtime/vm/jit/linear-scan.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/jit/state-vector.h"

namespace HPHP {
namespace JIT {

struct ArgGroup;

enum class DestType : unsigned {
  None,  // return void (no valid registers)
  SSA,   // return a single-register value
  TV     // return a TypedValue packed in two registers
};

/*
 * CallDest is the destination specification for a cgCallHelper
 * invocation.
 *
 * The DestType describes the return type of native helper calls,
 * particularly register assignments.
 *
 * These are created using the callDest() member functions.
 */
struct CallDest {
  DestType type;
  PhysReg reg0;
  PhysReg reg1;
};

const CallDest kVoidDest { DestType::None };

enum class SyncOptions {
  kNoSyncPoint,
  kSyncPoint,
  kSyncPointAdjustOne,
};

// Information about where code was generated, for pretty-printing.
struct AsmInfo {
  explicit AsmInfo(const IRFactory* factory)
    : instRanges(factory, TcaRange(nullptr, nullptr))
    , asmRanges(factory, TcaRange(nullptr, nullptr))
    , astubRanges(factory, TcaRange(nullptr, nullptr))
  {}

  // Asm address info for each instruction and block
  StateVector<IRInstruction,TcaRange> instRanges;
  StateVector<Block,TcaRange> asmRanges;
  StateVector<Block,TcaRange> astubRanges;
};

typedef StateVector<IRInstruction, RegSet> LiveRegs;

// Stuff we need to preserve between blocks while generating code,
// and address information produced during codegen.
struct CodegenState {
  CodegenState(const IRFactory* factory, const RegAllocInfo& regs,
               const LiveRegs& liveRegs, const LifetimeInfo* lifetime,
               AsmInfo* asmInfo)
    : patches(factory, nullptr)
    , addresses(factory, nullptr)
    , regs(regs)
    , liveRegs(liveRegs)
    , lifetime(lifetime)
    , asmInfo(asmInfo)
    , catches(factory, CatchInfo())
    , catchTrace(nullptr)
  {}

  // Each block has a list of addresses to patch, and an address if
  // it's already been emitted.
  StateVector<Block,void*> patches;
  StateVector<Block,TCA> addresses;

  // True if this block's terminal Jmp_ has a desination equal to the
  // next block in the same assmbler.
  bool noTerminalJmp_;

  // output from register allocator
  const RegAllocInfo& regs;

  // for each instruction, holds the RegSet of registers that must be
  // preserved across that instruction.  This is for push/pop of caller-saved
  // registers.
  const LiveRegs& liveRegs;

  // Optional information used when pretty-printing code after codegen.
  // when not available, these are nullptrs.
  const LifetimeInfo* lifetime;

  // Output: start/end ranges of machine code addresses of each instruction.
  AsmInfo* asmInfo;

  // Used to pass information about the state of the world at native
  // calls between cgCallHelper and cgBeginCatch.
  StateVector<Block, CatchInfo> catches;

  // If non-null, represents the catch trace for the current
  // instruction, to be registered with the unwinder.
  IRTrace* catchTrace;
};

constexpr Reg64  rCgGP  (reg::r11);
constexpr RegXMM rCgXMM0(reg::xmm0);
constexpr RegXMM rCgXMM1(reg::xmm1);

struct CodeGenerator {
  typedef Transl::X64Assembler Asm;

  CodeGenerator(IRTrace* trace, Asm& as, Asm& astubs,
                Transl::TranslatorX64* tx64, CodegenState& state)
    : m_as(as)
    , m_astubs(astubs)
    , m_tx64(tx64)
    , m_state(state)
    , m_regs(state.regs)
    , m_rScratch(InvalidReg)
    , m_curInst(nullptr)
    , m_curTrace(trace)
  {
  }

  void cgBlock(Block* block, vector<TransBCMapping>* bcMap);

private:
  Address cgInst(IRInstruction* inst);

  // Autogenerate function declarations for each IR instruction in ir.h
#define O(name, dsts, srcs, flags) void cg##name(IRInstruction* inst);
  IR_OPCODES
#undef O

  void cgCallNative(Asm& a, IRInstruction* inst);

  // Utilities to package CallDest structs for cgCallHelper.
  CallDest callDest(PhysReg reg0, PhysReg reg1 = InvalidReg) const;
  CallDest callDest(SSATmp* dst) const;
  CallDest callDestTV(SSATmp* dst) const;

  // Main call helper:
  void cgCallHelper(Asm& a,
                    const Transl::CppCall& call,
                    const CallDest& dstInfo,
                    SyncOptions sync,
                    ArgGroup& args,
                    RegSet toSave);
  // Overload to make the toSave RegSet optional:
  void cgCallHelper(Asm& a,
                    const Transl::CppCall& call,
                    const CallDest& dstInfo,
                    SyncOptions sync,
                    ArgGroup& args);
  void cgInterpOneCommon(IRInstruction* inst);

  void cgStore(PhysReg base,
               int64_t off,
               SSATmp* src,
               bool genStoreType = true);
  void cgStoreTypedValue(PhysReg base, int64_t off, SSATmp* src);

  void cgLoad(PhysReg base, int64_t off, IRInstruction* inst);

  template<class Loc1, class Loc2, class JmpFn>
  void emitTypeTest(Type type,
                    Loc1 typeSrc,
                    Loc2 dataSrc,
                    JmpFn doJcc);

  template<class Loc>
  void emitTypeCheck(Type type, Loc typeSrc, Loc dataSrc, Block* taken);
  template<class Loc>
  void emitTypeGuard(Type type, Loc typeLoc, Loc dataLoc);

  void cgStMemWork(IRInstruction* inst, bool genStoreType);
  void cgStRefWork(IRInstruction* inst, bool genStoreType);
  void cgStPropWork(IRInstruction* inst, bool genStoreType);
  void cgIncRefWork(Type type, SSATmp* src);
  void cgDecRefWork(IRInstruction* inst, bool genZeroCheck);

  template<class OpInstr, class Oper>
  void cgUnaryIntOp(SSATmp* dst, SSATmp* src, OpInstr, Oper);

  enum Commutativity { Commutative, NonCommutative };

  void cgRoundCommon(IRInstruction* inst, RoundDirection dir);

  template<class Oper, class RegType>
  void cgBinaryOp(IRInstruction*,
                  void (Asm::*intImm)(Immed, RegType),
                  void (Asm::*intRR)(RegType, RegType),
                  void (Asm::*mov)(RegType, RegType),
                  void (Asm::*fpRR)(RegXMM, RegXMM),
                  Oper,
                  RegType (*conv)(PhysReg),
                  Commutativity);
  template<class Oper, class RegType>
  void cgBinaryIntOp(IRInstruction*,
                     void (Asm::*intImm)(Immed, RegType),
                     void (Asm::*intRR)(RegType, RegType),
                     void (Asm::*mov)(RegType, RegType),
                     Oper,
                     RegType (*conv)(PhysReg),
                     Commutativity);

  template<class Oper>
  void cgShiftCommon(IRInstruction* inst,
                     void (Asm::*instrIR)(Immed, Reg64),
                     void (Asm::*instrR)(Reg64),
                     Oper oper);

  void cgNegateWork(SSATmp* dst, SSATmp* src);
  void cgNotWork(SSATmp* dst, SSATmp* src);

  void emitGetCtxFwdCallWithThis(PhysReg ctxReg,
                                 bool    staticCallee);

  void emitGetCtxFwdCallWithThisDyn(PhysReg      destCtxReg,
                                    PhysReg      thisReg,
                                    Transl::TargetCache::CacheHandle& ch);

  void cgLoadTypedValue(PhysReg base, int64_t off, IRInstruction* inst);

  void cgJcc(IRInstruction* inst);        // helper
  void cgReqBindJcc(IRInstruction* inst); // helper
  void cgCmpHelper(IRInstruction* inst,
                   void (Asm::*setter)(Reg8),
                   int64_t (*str_cmp_str)(StringData*, StringData*),
                   int64_t (*str_cmp_int)(StringData*, int64_t),
                   int64_t (*str_cmp_obj)(StringData*, ObjectData*),
                   int64_t (*obj_cmp_obj)(ObjectData*, ObjectData*),
                   int64_t (*obj_cmp_int)(ObjectData*, int64_t),
                   int64_t (*arr_cmp_arr)(ArrayData*, ArrayData*));

  template<class Loc>
  void emitSideExitGuard(Type type, Loc typeLoc,
                         Loc dataLoc, Offset taken);
  void emitReqBindJcc(ConditionCode cc, const ReqBindJccData*);

  void emitCompare(SSATmp*, SSATmp*);
  void emitTestZero(SSATmp*);
  bool emitIncDecHelper(SSATmp* dst, SSATmp* src1, SSATmp* src2,
                        void(Asm::*emitFunc)(Reg64));
  bool emitInc(SSATmp* dst, SSATmp* src1, SSATmp* src2);
  bool emitDec(SSATmp* dst, SSATmp* src1, SSATmp* src2);

private:
  PhysReg selectScratchReg(IRInstruction* inst);
  void emitLoadImm(CodeGenerator::Asm& as, int64_t val, PhysReg dstReg);
  PhysReg prepXMMReg(const SSATmp* tmp,
                     X64Assembler& as,
                     const RegAllocInfo& allocInfo,
                     RegXMM rXMMScratch);
  void emitSetCc(IRInstruction*, ConditionCode);
  template<class JmpFn>
  void emitIsTypeTest(IRInstruction* inst, JmpFn doJcc);
  void doubleCmp(X64Assembler& a, RegXMM xmmReg0, RegXMM xmmReg1);
  void cgIsTypeCommon(IRInstruction* inst, bool negate);
  void cgJmpIsTypeCommon(IRInstruction* inst, bool negate);
  void cgIsTypeMemCommon(IRInstruction*, bool negate);
  void emitInstanceBitmaskCheck(IRInstruction*);
  void emitTraceRet(CodeGenerator::Asm& as);
  Address cgCheckStaticBit(Type type,
                           PhysReg reg,
                           bool regIsCount);
  Address cgCheckStaticBitAndDecRef(Type type,
                                    PhysReg dataReg,
                                    Block* exit);
  Address cgCheckRefCountedType(PhysReg typeReg);
  Address cgCheckRefCountedType(PhysReg baseReg,
                                int64_t offset);
  void cgDecRefStaticType(Type type,
                          PhysReg dataReg,
                          Block* exit,
                          bool genZeroCheck);
  void cgDecRefDynamicType(PhysReg typeReg,
                           PhysReg dataReg,
                           Block* exit,
                           bool genZeroCheck);
  void cgDecRefDynamicTypeMem(PhysReg baseReg,
                              int64_t offset,
                              Block* exit);
  void cgDecRefMem(Type type,
                   PhysReg baseReg,
                   int64_t offset,
                   Block* exit);

  void cgIterNextCommon(IRInstruction* inst);
  void cgIterInitCommon(IRInstruction* inst);
  void cgMIterNextCommon(IRInstruction* inst);
  void cgMIterInitCommon(IRInstruction* inst);
  void cgLdFuncCachedCommon(IRInstruction* inst);
  TargetCache::CacheHandle cgLdClsCachedCommon(IRInstruction* inst);
  void emitFwdJcc(ConditionCode cc, Block* target);
  void emitFwdJcc(Asm& a, ConditionCode cc, Block* target);
  void emitContVarEnvHelperCall(SSATmp* fp, TCA helper);
  const Func* curFunc() const;
  Class*      curClass() const { return curFunc()->cls(); }
  const Unit* curUnit() const { return curFunc()->unit(); }
  void recordSyncPoint(Asm& as, SyncOptions sync = SyncOptions::kSyncPoint);
  int iterOffset(SSATmp* tmp);
  int iterOffset(uint32_t id);
  void emitReqBindAddr(const Func* func, TCA& dest, Offset offset);

  void emitAdjustSp(PhysReg spReg, PhysReg dstReg, int64_t adjustment);
  void emitConvBoolOrIntToDbl(IRInstruction* inst);

  /*
   * Generate an if-block that branches around some unlikely code, handling
   * the cases when a == astubs and a != astubs.  cc is the branch condition
   * to run the unlikely block.
   *
   * Passes the proper assembler to use to the unlikely function.
   */
  template <class Block>
  void unlikelyIfBlock(ConditionCode cc, Block unlikely) {
    if (&m_as == &m_astubs) {
      Label done;
      m_as.jcc(ccNegate(cc), done);
      unlikely(m_as);
      asm_label(m_as, done);
    } else {
      Label unlikelyLabel, done;
      m_as.jcc(cc, unlikelyLabel);
      asm_label(m_astubs, unlikelyLabel);
      unlikely(m_astubs);
      m_astubs.jmp(done);
      asm_label(m_as, done);
    }
  }

  // Generate an if-then-else block into m_as.
  template <class Then, class Else>
  void ifThenElse(ConditionCode cc, Then thenBlock, Else elseBlock) {
    Label elseLabel, done;
    m_as.jcc8(ccNegate(cc), elseLabel);
    thenBlock();
    m_as.jmp8(done);
    asm_label(m_as, elseLabel);
    elseBlock();
    asm_label(m_as, done);
  }

  /*
   * Same as ifThenElse except the first block is off in astubs
   */
  template <class Then, class Else>
  void unlikelyIfThenElse(ConditionCode cc, Then unlikely, Else elseBlock) {
    if (&m_as == &m_astubs) {
      Label elseLabel, done;
      m_as.jcc8(ccNegate(cc), elseLabel);
      unlikely(m_as);
      m_as.jmp8(done);
      asm_label(m_as, elseLabel);
      elseBlock(m_as);
      asm_label(m_as, done);
    } else {
      Label unlikelyLabel, done;
      m_as.jcc(cc, unlikelyLabel);
      elseBlock(m_as);
      asm_label(m_astubs, unlikelyLabel);
      unlikely(m_astubs);
      m_astubs.jmp(done);
      asm_label(m_as, done);
    }
  }

  // This is for printing partially-generated traces when debugging
  void print() const;

private:
  Asm&                m_as;       // current "main" assembler
  Asm&                m_astubs;   // assembler for stubs and other cold code.
  TranslatorX64*      m_tx64;
  CodegenState&       m_state;
  const RegAllocInfo& m_regs;
  Reg64               m_rScratch; // currently selected GP scratch reg
  IRInstruction*      m_curInst;  // current instruction being generated
  IRTrace*            m_curTrace;
};

class ArgDesc {
public:
  enum class Kind {
    Reg,     // Normal register
    TypeReg, // TypedValue's m_type field. Might need arch-specific
             // mangling before call depending on TypedValue's layout.
    Imm,     // Immediate
    Addr,    // Address
    None,    // Nothing: register will contain garbage
  };

  PhysReg dstReg() const { return m_dstReg; }
  PhysReg srcReg() const { return m_srcReg; }
  Kind kind() const { return m_kind; }
  void setDstReg(PhysReg reg) { m_dstReg = reg; }
  Immed imm() const { return m_imm; }
  bool isZeroExtend() const {return m_zeroExtend;}
  bool done() const { return m_done; }
  void markDone() { m_done = true; }

private: // These should be created using ArgGroup.
  friend struct ArgGroup;

  explicit ArgDesc(Kind kind, PhysReg srcReg, Immed immVal)
    : m_kind(kind)
    , m_srcReg(srcReg)
    , m_dstReg(reg::noreg)
    , m_imm(immVal)
    , m_zeroExtend(false)
    , m_done(false)
  {}

  explicit ArgDesc(SSATmp* tmp, const RegisterInfo& info, bool val = true);

private:
  Kind m_kind;
  PhysReg m_srcReg;
  PhysReg m_dstReg;
  Immed m_imm;
  bool m_zeroExtend;
  bool m_done;
};

/*
 * Bag of ArgDesc for use with cgCallHelper.
 *
 * You can create this using function chaining.  Example:
 *
 *   ArgGroup args;
 *   args.imm(0)
 *       .reg(rax)
 *       .immPtr(StringData::GetStaticString("Yo"))
 *       ;
 *   assert(args.size() == 3);
 */
struct ArgGroup {
  typedef std::vector<ArgDesc> ArgVec;

  explicit ArgGroup(const RegAllocInfo& regs)
      : m_regs(regs), m_override(nullptr)
    {}

  size_t numRegArgs() const { return m_regArgs.size(); }
  size_t numStackArgs() const { return m_stkArgs.size(); }

  ArgDesc& reg(size_t i) {
    assert(i < m_regArgs.size());
    return m_regArgs[i];
  }
  ArgDesc& operator[](size_t i) {
    return reg(i);
  }
  ArgDesc& stk(size_t i) {
    assert(i < m_stkArgs.size());
    return m_stkArgs[i];
  }

  ArgGroup& imm(uintptr_t imm) {
    push_arg(ArgDesc(ArgDesc::Kind::Imm, InvalidReg, imm));
    return *this;
  }

  template<class T> ArgGroup& immPtr(const T* ptr) {
    return imm(uintptr_t(ptr));
  }

  ArgGroup& immPtr(std::nullptr_t) { return imm(0); }

  ArgGroup& reg(PhysReg reg) {
    push_arg(ArgDesc(ArgDesc::Kind::Reg, PhysReg(reg), -1));
    return *this;
  }

  ArgGroup& addr(PhysReg base, intptr_t off) {
    push_arg(ArgDesc(ArgDesc::Kind::Addr, base, off));
    return *this;
  }

  ArgGroup& ssa(SSATmp* tmp) {
    push_arg(ArgDesc(tmp, m_regs[tmp]));
    return *this;
  }

  ArgGroup& ssas(IRInstruction* inst, unsigned begin, unsigned count = 1) {
    for (SSATmp* s : inst->srcs().subpiece(begin, count)) {
      push_arg(ArgDesc(s, m_regs[s]));
    }
    return *this;
  }

  /*
   * Pass tmp as a TypedValue passed by value.
   */
  ArgGroup& typedValue(SSATmp* tmp) {
    // If there's exactly one register argument slot left, the whole TypedValue
    // goes on the stack instead of being split between a register and the
    // stack.
    if (m_regArgs.size() == kNumRegisterArgs - 1) {
      m_override = &m_stkArgs;
    }
    packed_tv ? type(tmp).ssa(tmp) : ssa(tmp).type(tmp);
    m_override = nullptr;
    return *this;
  }

  ArgGroup& vectorKeyIS(SSATmp* key) {
    return vectorKeyImpl(key, true);
  }

  ArgGroup& vectorKeyS(SSATmp* key) {
    return vectorKeyImpl(key, false);
  }

private:
  void push_arg(const ArgDesc& arg) {
    // If m_override is set, use it unconditionally. Otherwise, select
    // m_regArgs or m_stkArgs depending on how many args we've already pushed.
    ArgVec* args = m_override;
    if (!args) {
      args = m_regArgs.size() < kNumRegisterArgs ? &m_regArgs : &m_stkArgs;
    }
    args->push_back(arg);
  }

  /*
   * For passing the m_type field of a TypedValue.
   */
  ArgGroup& type(SSATmp* tmp) {
    push_arg(ArgDesc(tmp, m_regs[tmp], false));
    return *this;
  }

  ArgGroup& none() {
    push_arg(ArgDesc(ArgDesc::Kind::None, InvalidReg, -1));
    return *this;
  }

  ArgGroup& vectorKeyImpl(SSATmp* key, bool allowInt) {
    if (key->isString() || (allowInt && key->isA(Type::Int))) {
      return packed_tv ? none().ssa(key) : ssa(key).none();
    }
    return typedValue(key);
  }

  const RegAllocInfo& m_regs;
  ArgVec* m_override; // used to force args to go into a specific ArgVec
  ArgVec m_regArgs;
  ArgVec m_stkArgs;
};

const Func* loadClassCtor(Class* cls);

ObjectData* createClHelper(Class*, int, ActRec*, TypedValue*);

void genCodeForTrace(IRTrace*                trace,
                     CodeGenerator::Asm&     a,
                     CodeGenerator::Asm&     astubs,
                     IRFactory*              irFactory,
                     vector<TransBCMapping>* bcMap,
                     TranslatorX64*          tx64,
                     const RegAllocInfo&     regs,
                     const LifetimeInfo*     lifetime = nullptr,
                     AsmInfo*                asmInfo = nullptr);

TypedValue arrayIdxI(ArrayData*, int64_t, TypedValue);
TypedValue arrayIdxS(ArrayData*, StringData*, TypedValue);
TypedValue arrayIdxSi(ArrayData*, StringData*, TypedValue);
TypedValue* ldGblAddrDefHelper(StringData* name);

}}

#endif
