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
#include <runtime/vm/translator/translator-x64.h>

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
  AsmInfo(const IRFactory* factory)
    : instRanges(factory, TcaRange(nullptr, nullptr))
    , asmRanges(factory, TcaRange(nullptr, nullptr))
    , astubRanges(factory, TcaRange(nullptr, nullptr))
  {}

  // Asm address info for each instruction and block
  StateVector<IRInstruction,TcaRange> instRanges;
  StateVector<Block,TcaRange> asmRanges;
  StateVector<Block,TcaRange> astubRanges;
};

// Stuff we need to preserve between blocks while generating code,
// and address information produced during codegen.
struct CodegenState {
  CodegenState(const IRFactory* factory, AsmInfo* asmInfo)
    : patches(factory, nullptr)
    , lastMarker(nullptr)
    , firstMarkerSeen(false)
    , asmInfo(asmInfo)
  {}

  // Each block has a list of addresses to patch
  StateVector<Block,void*> patches;

  // Keep track of the most recent Marker instruction we've seen in the
  // current trace (even across blocks).
  const MarkerData* lastMarker;
  bool firstMarkerSeen;

  // True if this block's terminal Jmp_ has a desination equal to the
  // next block in the same assmbler.
  bool noTerminalJmp_;

  AsmInfo* asmInfo;
};

// Generate an if-then block into a.  thenBlock is executed if cc is true.
template <class Then>
void ifThen(Transl::X64Assembler& a, ConditionCode cc, Then thenBlock) {
  Label done;
  a.jcc8(ccNegate(cc), done);
  thenBlock();
  asm_label(a, done);
}

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
  void cgCallNative(IRInstruction* inst);
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
  void cgDecRefWork(IRInstruction* inst, bool genZeroCheck, bool killThis);

  template<class OpInstr, class Oper>
  void cgUnaryIntOp(SSATmp* dst, SSATmp* src, OpInstr, Oper);

  enum Commutativity { Commutative, NonCommutative };

  template<class Oper>
  void cgBinaryOp(IRInstruction*,
                  void (Asm::*intImm)(Immed, Reg64),
                  void (Asm::*intRR)(Reg64, Reg64),
                  void (Asm::*fpRR)(RegXMM, RegXMM),
                  Oper,
                  Commutativity);
  template<class Oper>
  void cgBinaryIntOp(IRInstruction*,
                     void (Asm::*intImm)(Immed, Reg64),
                     void (Asm::*intRR)(Reg64, Reg64),
                     Oper,
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
  void cgDecRefStaticType(Type type,
                          PhysReg dataReg,
                          Block* exit,
                          bool genZeroCheck,
                          std::function<void()> slowPathWork =
                          std::function<void()>());
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
  void emitSpillActRec(SSATmp* sp,
                       int64_t spOffset,
                       SSATmp* defAR);

  void cgIterNextCommon(IRInstruction* inst, bool isNextK);
  void cgIterInitCommon(IRInstruction* inst, bool isInitK);
  Address emitFwdJcc(ConditionCode cc, Block* target);
  Address emitFwdJcc(Asm& a, ConditionCode cc, Block* target);
  Address emitFwdJmp(Asm& as, Block* target);
  Address emitFwdJmp(Block* target);
  Address emitSmashableFwdJmp(Block* target, IRInstruction* toSmash);
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

  /*
   * Generate an if-block that branches around some unlikely code, handling
   * the cases when a == astubs and a != astubs.  cc is the branch condition
   * to run the unlikely block.
   */
  template <class Block>
  void unlikelyIfBlock(ConditionCode cc, Block unlikely) {
    if (&m_as == &m_astubs) {
      Label done;
      m_as.jcc(ccNegate(cc), done);
      unlikely();
      asm_label(m_as, done);
    } else {
      Label unlikelyLabel, done;
      m_as.jcc(cc, unlikelyLabel);
      asm_label(m_astubs, unlikelyLabel);
      unlikely();
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
    TypeReg, // Type register. Might need arch-specific mangling before call
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

private: // These should be created using ArgGroup.
  friend struct ArgGroup;

  explicit ArgDesc(Kind kind, PhysReg srcReg, Immed immVal)
    : m_kind(kind)
    , m_srcReg(srcReg)
    , m_dstReg(reg::noreg)
    , m_imm(immVal)
    , m_zeroExtend(false)
  {}

  explicit ArgDesc(SSATmp* tmp, bool val = true);

private:
  Kind m_kind;
  PhysReg m_srcReg;
  PhysReg m_dstReg;
  Immed m_imm;
  bool m_zeroExtend;
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
  size_t size() const { return m_args.size(); }

  ArgDesc& operator[](size_t i) {
    assert(i < size());
    return m_args[i];
  }

  ArgGroup& imm(uintptr_t imm) {
    m_args.push_back(ArgDesc(ArgDesc::Imm, InvalidReg, imm));
    return *this;
  }

  template<class T> ArgGroup& immPtr(const T* ptr) {
    return imm(uintptr_t(ptr));
  }

  ArgGroup& reg(PhysReg reg) {
    m_args.push_back(ArgDesc(ArgDesc::Reg, PhysReg(reg), -1));
    return *this;
  }

  ArgGroup& addr(PhysReg base, intptr_t off) {
    m_args.push_back(ArgDesc(ArgDesc::Addr, base, off));
    return *this;
  }

  ArgGroup& ssa(SSATmp* tmp) {
    m_args.push_back(ArgDesc(tmp));
    return *this;
  }

  ArgGroup& ssas(IRInstruction* inst, unsigned begin, unsigned count) {
    for (SSATmp* s : inst->getSrcs().subpiece(begin, count)) {
      m_args.push_back(ArgDesc(s));
    }
    return *this;
  }

  /*
   * Loads the type of tmp into an arg register destined to be the
   * upper word of a TypedValue parameter passed by value in registers.
   */
  ArgGroup& type(SSATmp* tmp) {
    m_args.push_back(ArgDesc(tmp, false));
    return *this;
  }

  /*
   * Pass tmp as a TypedValue passed by value.
   */
  ArgGroup& typedValue(SSATmp* tmp) {
    return ssa(tmp).type(tmp);
  }

  ArgGroup& none() {
    m_args.push_back(ArgDesc(ArgDesc::None, InvalidReg, -1));
    return *this;
  }

  ArgGroup& vectorKeyIS(SSATmp* key) {
    return vectorKeyImpl(key, true);
  }

  ArgGroup& vectorKeyS(SSATmp* key) {
    return vectorKeyImpl(key, false);
  }

private:
  ArgGroup& vectorKeyImpl(SSATmp* key, bool allowInt) {
    if (key->isString() || (allowInt && key->isA(Type::Int))) {
      return ssa(key).none();
    }
    return typedValue(key);
  }

  std::vector<ArgDesc> m_args;
};

void genCodeForTrace(Trace*                  trace,
                     CodeGenerator::Asm&     a,
                     CodeGenerator::Asm&     astubs,
                     IRFactory*              irFactory,
                     vector<TransBCMapping>* bcMap,
                     TranslatorX64*          tx64,
                     AsmInfo                 *asmInfo = nullptr);

}}}

#endif
