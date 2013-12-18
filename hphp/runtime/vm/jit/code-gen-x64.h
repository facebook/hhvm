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
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/linear-scan.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/jit/state-vector.h"

namespace HPHP { namespace JIT {

struct ArgGroup;

enum class DestType : unsigned {
  None,  // return void (no valid registers)
  SSA,   // return a single-register value
  SSA2,  // return a two-register value (pair)
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
  kSmashableAndSyncPoint,
};

// Returned information from cgCallHelper
struct CallHelperInfo {
  TCA returnAddress;
};

// Information about where code was generated, for pretty-printing.
struct AsmInfo {
  explicit AsmInfo(const IRUnit& unit)
    : instRanges(unit, TcaRange(nullptr, nullptr))
    , asmRanges(unit, TcaRange(nullptr, nullptr))
    , astubRanges(unit, TcaRange(nullptr, nullptr))
  {}

  // Asm address info for each instruction and block
  StateVector<IRInstruction,TcaRange> instRanges;
  StateVector<Block,TcaRange> asmRanges;
  StateVector<Block,TcaRange> astubRanges;

  void updateForInstruction(IRInstruction* inst, TCA start, TCA end);
};

typedef StateVector<IRInstruction, RegSet> LiveRegs;

// Stuff we need to preserve between blocks while generating code,
// and address information produced during codegen.
struct CodegenState {
  CodegenState(const IRUnit& unit, const RegAllocInfo& regs,
               const LiveRegs& liveRegs, AsmInfo* asmInfo)
    : patches(unit, nullptr)
    , addresses(unit, nullptr)
    , regs(regs)
    , liveRegs(liveRegs)
    , asmInfo(asmInfo)
    , catches(unit, CatchInfo())
  {}

  // Each block has a list of addresses to patch, and an address if
  // it's already been emitted.
  StateVector<Block,void*> patches;
  StateVector<Block,TCA> addresses;

  // True if this block's terminal Jmp has a desination equal to the
  // next block in the same assmbler.
  bool noTerminalJmp;

  // output from register allocator
  const RegAllocInfo& regs;

  // for each instruction, holds the RegSet of registers that must be
  // preserved across that instruction.  This is for push/pop of caller-saved
  // registers.
  const LiveRegs& liveRegs;

  // Output: start/end ranges of machine code addresses of each instruction.
  AsmInfo* asmInfo;

  // Used to pass information about the state of the world at native
  // calls between cgCallHelper and cgBeginCatch.
  StateVector<Block, CatchInfo> catches;
};

constexpr Reg64  rCgGP  (reg::r11);
constexpr RegXMM rCgXMM0(reg::xmm0);
constexpr RegXMM rCgXMM1(reg::xmm1);

struct CodeGenerator {
  typedef JIT::X64Assembler Asm;

  CodeGenerator(const IRUnit& unit, CodeBlock& mainCode, CodeBlock& stubsCode,
                JIT::TranslatorX64* tx64, CodegenState& state)
    : m_unit(unit)
    , m_mainCode(mainCode)
    , m_stubsCode(stubsCode)
    , m_as(mainCode)
    , m_astubs(stubsCode)
    , m_tx64(tx64)
    , m_state(state)
    , m_rScratch(InvalidReg)
    , m_curInst(nullptr)
  {
  }

  void cgBlock(Block* block, vector<TransBCMapping>* bcMap);

private:
  Address cgInst(IRInstruction* inst);

  const PhysLoc curOpd(const SSATmp* t) const {
    return m_state.regs[m_curInst][t];
  }
  const PhysLoc curOpd(const SSATmp& t) const {
    return curOpd(&t);
  }
  const RegAllocInfo::RegMap& curOpds() const {
    return m_state.regs[m_curInst];
  }

  // Autogenerate function declarations for each IR instruction in ir.h
#define O(name, dsts, srcs, flags) void cg##name(IRInstruction* inst);
  IR_OPCODES
#undef O

  void cgCallNative(Asm& a, IRInstruction* inst);

  // Utilities to package CallDest structs for cgCallHelper.
  CallDest callDest(PhysReg reg0, PhysReg reg1 = InvalidReg) const;
  CallDest callDest(SSATmp* dst) const;
  CallDest callDestTV(SSATmp* dst) const;
  CallDest callDest2(SSATmp* dst) const;

  // Main call helper:
  CallHelperInfo cgCallHelper(Asm& a,
                              const CppCall& call,
                              const CallDest& dstInfo,
                              SyncOptions sync,
                              ArgGroup& args,
                              RegSet toSave);
  // Overload to make the toSave RegSet optional:
  CallHelperInfo cgCallHelper(Asm& a,
                              const CppCall& call,
                              const CallDest& dstInfo,
                              SyncOptions sync,
                              ArgGroup& args);
  void cgInterpOneCommon(IRInstruction* inst);

  template<class MemRef>
  void cgStore(MemRef dst,
               SSATmp* src,
               bool genStoreType = true);
  template<class MemRef>
  void cgStoreTypedValue(MemRef dst, SSATmp* src);

  // helpers to load a value in dst. When label is not null a type check
  // is performed against value to ensure it is of the type expected by dst
  template<class BaseRef>
  void cgLoad(SSATmp* dst, BaseRef value, Block* label = nullptr);
  template<class BaseRef>
  void cgLoadTypedValue(SSATmp* dst, BaseRef base, Block* label = nullptr);

  // internal helpers to manage register conflicts from a source to a PhysReg
  // destination.
  // If the conflict cannot be resolved the out param isResolved is set to
  // false and the caller should take proper action
  IndexedMemoryRef resolveRegCollision(PhysReg dst,
                                       IndexedMemoryRef value,
                                       bool& isResolved);
  MemoryRef resolveRegCollision(PhysReg dst,
                                MemoryRef value,
                                bool& isResolved);

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
                                    RDS::Handle ch);

  void cgJcc(IRInstruction* inst);          // helper
  void cgReqBindJcc(IRInstruction* inst);   // helper
  void cgSideExitJcc(IRInstruction* inst);  // helper
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
  void emitLoadImm(Asm& as, int64_t val, PhysReg dstReg);
  PhysReg prepXMMReg(const SSATmp* tmp,
                     Asm& as,
                     const PhysLoc&,
                     RegXMM rXMMScratch);
  void emitSetCc(IRInstruction*, ConditionCode);
  template<class JmpFn>
  void emitIsTypeTest(IRInstruction* inst, JmpFn doJcc);
  void doubleCmp(Asm& a, RegXMM xmmReg0, RegXMM xmmReg1);
  void cgIsTypeCommon(IRInstruction* inst, bool negate);
  void cgJmpIsTypeCommon(IRInstruction* inst, bool negate);
  void cgIsTypeMemCommon(IRInstruction*, bool negate);
  void emitInstanceBitmaskCheck(IRInstruction*);
  void emitTraceRet(Asm& as);
  void emitInitObjProps(PhysReg dstReg, const Class* cls, size_t nProps);

  template <typename F>
  Address cgCheckStaticBitAndDecRef(Type type,
                                    PhysReg dataReg,
                                    Block* exit,
                                    F destroy);
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
  void cgLookupCnsCommon(IRInstruction* inst);
  RDS::Handle cgLdClsCachedCommon(IRInstruction* inst);
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
  void cgLdClsMethodCacheCommon(IRInstruction* inst, Offset offset);

  /*
   * Generate an if-block that branches around some unlikely code, handling
   * the cases when a == astubs and a != astubs.  cc is the branch condition
   * to run the unlikely block.
   *
   * Passes the proper assembler to use to the unlikely function.
   */
  template <class Block>
  void unlikelyIfBlock(ConditionCode cc, Block unlikely) {
    if (m_as.base() == m_astubs.base()) {
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

  template <class Then>
  void ifBlock(ConditionCode cc, Then thenBlock) {
    Label done;
    m_as.jcc8(ccNegate(cc), done);
    thenBlock(m_as);
    asm_label(m_as, done);
  }

  // Generate an if-then-else block
  template <class Then, class Else>
  void ifThenElse(Asm& a, ConditionCode cc, Then thenBlock, Else elseBlock) {
    Label elseLabel, done;
    a.jcc8(ccNegate(cc), elseLabel);
    thenBlock();
    a.jmp8(done);
    asm_label(a, elseLabel);
    elseBlock();
    asm_label(a, done);
  }

  // Generate an if-then-else block into m_as.
  template <class Then, class Else>
  void ifThenElse(ConditionCode cc, Then thenBlock, Else elseBlock) {
    ifThenElse(m_as, cc, thenBlock, elseBlock);
  }

  /*
   * Same as ifThenElse except the first block is off in astubs
   */
  template <class Then, class Else>
  void unlikelyIfThenElse(ConditionCode cc, Then unlikely, Else elseBlock) {
    if (m_as.base() == m_astubs.base()) {
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
  const IRUnit&       m_unit;
  CodeBlock&          m_mainCode;
  CodeBlock&          m_stubsCode;
  Asm                 m_as;  // current "main" assembler
  Asm                 m_astubs; // for stubs and other cold code
  TranslatorX64*      m_tx64;
  CodegenState&       m_state;
  Reg64               m_rScratch; // currently selected GP scratch reg
  IRInstruction*      m_curInst;  // current instruction being generated
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

  explicit ArgDesc(SSATmp* tmp, const PhysLoc&, bool val = true);

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
 *       .immPtr(makeStaticString("Yo"))
 *       ;
 *   assert(args.size() == 3);
 */
struct ArgGroup {
  typedef smart::vector<ArgDesc> ArgVec;

  explicit ArgGroup(const RegAllocInfo::RegMap& regs)
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

  /*
   * Pass tmp as a TypedValue passed by value.
   */
  ArgGroup& typedValue(SSATmp* tmp) {
    // If there's exactly one register argument slot left, the whole TypedValue
    // goes on the stack instead of being split between a register and the
    // stack.
    if (m_regArgs.size() == X64::kNumRegisterArgs - 1) {
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
      args = m_regArgs.size() < X64::kNumRegisterArgs ? &m_regArgs : &m_stkArgs;
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

  const RegAllocInfo::RegMap& m_regs;
  ArgVec* m_override; // used to force args to go into a specific ArgVec
  ArgVec m_regArgs;
  ArgVec m_stkArgs;
};

const Func* loadClassCtor(Class* cls);

ObjectData* createClHelper(Class*, int, ActRec*, TypedValue*);

void genCode(CodeBlock&              mainCode,
             CodeBlock&              stubsCode,
             IRUnit&                 unit,
             vector<TransBCMapping>* bcMap,
             TranslatorX64*          tx64,
             const RegAllocInfo&     regs);

// Helpers to compute a reference to a TypedValue type and data
inline MemoryRef loadTVType(PhysReg reg) {
  return reg[TVOFF(m_type)];
}

inline MemoryRef loadTVData(PhysReg reg) {
  return reg[TVOFF(m_data)];
}

inline MemoryRef loadTVType(MemoryRef ref) {
  return *(ref.r + TVOFF(m_type));
}

inline MemoryRef loadTVData(MemoryRef ref) {
  return *(ref.r + TVOFF(m_data));
}

inline IndexedMemoryRef loadTVType(IndexedMemoryRef ref) {
  return *(ref.r + TVOFF(m_type));
}

inline IndexedMemoryRef loadTVData(IndexedMemoryRef ref) {
  return *(ref.r + TVOFF(m_data));
}

}}

#endif
