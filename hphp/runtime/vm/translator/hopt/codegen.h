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

#ifndef incl_HPHP_VM_CG_H_
#define incl_HPHP_VM_CG_H_

#include <vector>
#include "runtime/vm/translator/hopt/ir.h"
#include "runtime/vm/translator/hopt/irfactory.h"
#include "runtime/vm/translator/targetcache.h"
#include "runtime/vm/translator/translator-x64.h"
#include "runtime/vm/translator/hopt/state_vector.h"

namespace HPHP {
namespace VM {
namespace JIT {

class FailedCodeGen : public std::exception {
 public:
  const char* file;
  const int   line;
  const char* func;
  FailedCodeGen(const char* _file, int _line, const char* _func) :
      file(_file), line(_line), func(_func) { }
};

struct ArgGroup;

// DestType describes the return type of native helper calls, particularly
// register assignments
enum class DestType : unsigned {
  None,  // return void (no valid registers)
  SSA,   // return a single-register value
  TV     // return a TypedValue packed in two registers
};

enum SyncOptions {
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
  CodegenState(const IRFactory* factory, const LiveRegs& liveRegs,
               const LifetimeInfo* lifetime,
               AsmInfo* asmInfo)
    : patches(factory, nullptr)
    , lastMarker(nullptr)
    , liveRegs(liveRegs)
    , lifetime(lifetime)
    , asmInfo(asmInfo)
  {}

  // Each block has a list of addresses to patch
  StateVector<Block,void*> patches;

  // Keep track of the most recent Marker instruction we've seen in the
  // current trace (even across blocks).
  const MarkerData* lastMarker;

  // True if this block's terminal Jmp_ has a desination equal to the
  // next block in the same assmbler.
  bool noTerminalJmp_;

  // for each instruction, holds the RegSet of registers that must be
  // preserved across that instruction.  This is for push/pop of caller-saved
  // registers.
  const LiveRegs& liveRegs;

  // Optional information used when pretty-printing code after codegen.
  // when not available, these are nullptrs.
  const LifetimeInfo* lifetime;

  // Output: start/end ranges of machine code addresses of each instruction.
  AsmInfo* asmInfo;
};

struct CodeGenerator {
  typedef Transl::X64Assembler Asm;

  CodeGenerator(Trace* trace, Asm& as, Asm& astubs, Transl::TranslatorX64* tx64,
                CodegenState& state)
    : m_as(as), m_astubs(astubs), m_tx64(tx64), m_state(state)
    , m_curInst(nullptr), m_curTrace(trace) {
  }

  void cgBlock(Block* block, vector<TransBCMapping>* bcMap);

  static void emitTraceCall(CodeGenerator::Asm& as, int64_t pcOff,
                            Transl::TranslatorX64* tx64);
  static Address emitFwdJmp(Asm& as, Block* target, CodegenState& state);

private:
  Address cgInst(IRInstruction* inst);

  // Autogenerate function declarations for each IR instruction in ir.h
#define O(name, dsts, srcs, flags) void cg##name(IRInstruction* inst);
  IR_OPCODES
#undef O

  // helper functions for code generation
  void cgCallNative(IRInstruction* inst) {
    cgCallNative(m_as, inst);
  }
  void cgCallNative(Asm& a, IRInstruction* inst);
  void doStackArgs(Asm&, ArgGroup&);
  void cgCallHelper(Asm&,
                    TCA addr,
                    SSATmp* dst,
                    SyncOptions sync,
                    ArgGroup& args,
                    DestType destType = DestType::SSA);
  void cgCallHelper(Asm& a,
                    TCA addr,
                    PhysReg dstReg,
                    SyncOptions sync,
                    ArgGroup& args,
                    DestType destType = DestType::SSA);
  void cgCallHelper(Asm& a,
                    const Transl::Call& call,
                    PhysReg dstReg,
                    SyncOptions sync,
                    ArgGroup& args,
                    DestType destType = DestType::SSA);
  void cgCallHelper(Asm& a,
                    const Transl::Call& call,
                    PhysReg dstReg0,
                    PhysReg dstReg1,
                    SyncOptions sync,
                    ArgGroup& args,
                    DestType destType = DestType::SSA);
  void cgCallHelper(Asm& a,
                    const Transl::Call& call,
                    PhysReg dstReg0,
                    PhysReg dstReg1,
                    SyncOptions sync,
                    ArgGroup& args,
                    RegSet toSave,
                    DestType destType = DestType::SSA);

  void cgStore(PhysReg base,
               int64_t off,
               SSATmp* src,
               bool genStoreType = true);
  void cgStoreTypedValue(PhysReg base, int64_t off, SSATmp* src);

  void cgLoad(PhysReg base, int64_t off, IRInstruction* inst);

  template<class OpndType>
  ConditionCode emitTypeTest(Type type, OpndType src, bool negate);

  template<class OpndType>
  ConditionCode emitTypeTest(IRInstruction* inst, OpndType src, bool negate);

  template<class OpndType>
  void emitGuardType(OpndType src, IRInstruction* instr);

  void cgGuardTypeCell(PhysReg baseReg,int64_t offset,IRInstruction* instr);

  void cgStMemWork(IRInstruction* inst, bool genStoreType);
  void cgStRefWork(IRInstruction* inst, bool genStoreType);
  void cgStPropWork(IRInstruction* inst, bool genStoreType);
  void cgIncRefWork(Type type, SSATmp* src);
  void cgDecRefWork(IRInstruction* inst, bool genZeroCheck);

  template<class OpInstr, class Oper>
  void cgUnaryIntOp(SSATmp* dst, SSATmp* src, OpInstr, Oper);

  enum Commutativity { Commutative, NonCommutative };

  template<class Oper, class RegType>
  void cgBinaryOp(IRInstruction*,
                  void (Asm::*intImm)(Immed, RegType),
                  void (Asm::*intRR)(RegType, RegType),
                  void (Asm::*mov)(RegType, RegType),
                  void (Asm::*fpRR)(RegXMM, RegXMM),
                  void (*extend)(Asm&, const SSATmp*),
                  Oper,
                  RegType (*conv)(PhysReg),
                  Commutativity);
  template<class Oper, class RegType>
  void cgBinaryIntOp(IRInstruction*,
                     void (Asm::*intImm)(Immed, RegType),
                     void (Asm::*intRR)(RegType, RegType),
                     void (Asm::*mov)(RegType, RegType),
                     void (*extend)(Asm&, const SSATmp*),
                     Oper,
                     RegType (*conv)(PhysReg),
                     Commutativity);

  void cgNegateWork(SSATmp* dst, SSATmp* src);
  void cgNotWork(SSATmp* dst, SSATmp* src);

  void emitGetCtxFwdCallWithThis(PhysReg ctxReg,
                                 bool    staticCallee);

  void emitGetCtxFwdCallWithThisDyn(PhysReg      destCtxReg,
                                    PhysReg      thisReg,
                                    Transl::TargetCache::CacheHandle& ch);

  void cgLoadTypedValue(PhysReg base, int64_t off, IRInstruction* inst);

  void cgNegate(IRInstruction* inst); // helper
  void cgJcc(IRInstruction* inst); // helper
  void cgOpCmpHelper(
            IRInstruction* inst,
            void (Asm::*setter)(Reg8),
            int64_t (*str_cmp_str)(StringData*, StringData*),
            int64_t (*str_cmp_int)(StringData*, int64_t),
            int64_t (*str_cmp_obj)(StringData*, ObjectData*),
            int64_t (*obj_cmp_obj)(ObjectData*, ObjectData*),
            int64_t (*obj_cmp_int)(ObjectData*, int64_t),
            int64_t (*arr_cmp_arr)(ArrayData*, ArrayData*));
  void cgJmpZeroHelper(IRInstruction* inst, ConditionCode cc);
  bool emitIncDecHelper(SSATmp* dst, SSATmp* src1, SSATmp* src2,
                        void(Asm::*emitFunc)(Reg64));
  bool emitInc(SSATmp* dst, SSATmp* src1, SSATmp* src2);
  bool emitDec(SSATmp* dst, SSATmp* src1, SSATmp* src2);

private:
  void emitSetCc(IRInstruction*, ConditionCode);
  void emitInstanceCheck(IRInstruction* inst, PhysReg dstReg);
  ConditionCode emitIsTypeTest(IRInstruction* inst, bool negate);
  void cgIsTypeCommon(IRInstruction* inst, bool negate);
  void cgJmpIsTypeCommon(IRInstruction* inst, bool negate);
  void cgIsTypeMemCommon(IRInstruction*, bool negate);
  void emitInstanceCheck(IRInstruction*);
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
  void cgConvPrimitiveToDbl(IRInstruction* inst);
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

  void cgIterNextCommon(IRInstruction* inst, bool isNextK);
  void cgIterInitCommon(IRInstruction* inst, bool isInitK);
  void cgLdFuncCachedCommon(IRInstruction* inst);
  TargetCache::CacheHandle cgLdClsCachedCommon(IRInstruction* inst);
  Address emitFwdJcc(ConditionCode cc, Block* target);
  Address emitFwdJcc(Asm& a, ConditionCode cc, Block* target);
  Address emitFwdJmp(Asm& as, Block* target);
  Address emitFwdJmp(Block* target);
  Address emitSmashableFwdJccAtEnd(ConditionCode cc, Block* target,
                                   IRInstruction* toSmash);
  void emitJccDirectExit(IRInstruction*, ConditionCode);
  Address emitSmashableFwdJcc(ConditionCode cc, Block* target,
                              IRInstruction* toSmash);
  void emitGuardOrFwdJcc(IRInstruction* inst, ConditionCode cc);
  void emitContVarEnvHelperCall(SSATmp* fp, TCA helper);
  const Func* getCurFunc() const;
  Class*      getCurClass() const { return getCurFunc()->cls(); }
  void recordSyncPoint(Asm& as, SyncOptions sync = kSyncPoint);
  Address getDtorGeneric();
  Address getDtorTyped();
  int getIterOffset(SSATmp* tmp);
  void emitReqBindAddr(const Func* func, TCA& dest, Offset offset);

  void emitAdjustSp(PhysReg spReg, PhysReg dstReg, int64_t adjustment);

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

  // This is for printing partially-generated traces when debugging
  void print() const;

private:
  Asm& m_as;                // current "main" assembler
  Asm& m_astubs;            // assembler for stubs and other cold code.
  TranslatorX64* m_tx64;
  CodegenState& m_state;
  IRInstruction* m_curInst; // current instruction being generated
  Trace* m_curTrace;
};

class ArgDesc {
public:
  enum Kind {
    Reg,     // Normal register
    TypeReg, // TypedValue's m_type field. Might need arch-specific
             // mangling before call depending on TypedValue's layout.
    Imm,     // Immediate
    Addr,    // Address
    None,    // Nothing: register will contain garbage
  };

  PhysReg getDstReg() const { return m_dstReg; }
  PhysReg getSrcReg() const { return m_srcReg; }
  Kind getKind() const { return m_kind; }
  void setDstReg(PhysReg reg) { m_dstReg = reg; }
  Immed getImm() const { return m_imm; }
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

  explicit ArgDesc(SSATmp* tmp, bool val = true);

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

  ArgGroup()
      : m_override(nullptr)
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
    push_arg(ArgDesc(ArgDesc::Imm, InvalidReg, imm));
    return *this;
  }

  template<class T> ArgGroup& immPtr(const T* ptr) {
    return imm(uintptr_t(ptr));
  }

  ArgGroup& immPtr(std::nullptr_t) { return imm(0); }

  ArgGroup& reg(PhysReg reg) {
    push_arg(ArgDesc(ArgDesc::Reg, PhysReg(reg), -1));
    return *this;
  }

  ArgGroup& addr(PhysReg base, intptr_t off) {
    push_arg(ArgDesc(ArgDesc::Addr, base, off));
    return *this;
  }

  ArgGroup& ssa(SSATmp* tmp) {
    push_arg(ArgDesc(tmp));
    return *this;
  }

  ArgGroup& ssas(IRInstruction* inst, unsigned begin, unsigned count = 1) {
    for (SSATmp* s : inst->getSrcs().subpiece(begin, count)) {
      push_arg(ArgDesc(s));
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
    push_arg(ArgDesc(tmp, false));
    return *this;
  }

  ArgGroup& none() {
    push_arg(ArgDesc(ArgDesc::None, InvalidReg, -1));
    return *this;
  }

  ArgGroup& vectorKeyImpl(SSATmp* key, bool allowInt) {
    if (key->isString() || (allowInt && key->isA(Type::Int))) {
      return packed_tv ? none().ssa(key) : ssa(key).none();
    }
    return typedValue(key);
  }

  ArgVec* m_override; // used to force args to go into a specific ArgVec
  ArgVec m_regArgs;
  ArgVec m_stkArgs;
};

const Func* loadClassCtor(Class* cls);

ActRec* irNewInstanceHelper(Class* cls,
                            int numArgs,
                            Cell* sp,
                            ActRec* prevAr);

Instance* createClHelper(Class*, int, ActRec*, TypedValue*);

void genCodeForTrace(Trace*                  trace,
                     CodeGenerator::Asm&     a,
                     CodeGenerator::Asm&     astubs,
                     IRFactory*              irFactory,
                     vector<TransBCMapping>* bcMap,
                     TranslatorX64*          tx64,
                     const LifetimeInfo*     lifetime = nullptr,
                     AsmInfo*                asmInfo = nullptr);

}}}

#endif
