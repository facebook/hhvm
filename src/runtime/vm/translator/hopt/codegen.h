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
#include "runtime/vm/translator/targetcache.h"
#include <runtime/vm/translator/translator-x64.h>

namespace HPHP {
namespace VM {
namespace JIT {

using namespace HPHP::VM::Transl;
using namespace HPHP::VM::Transl::TargetCache;

class FailedCodeGen : public std::exception {
 public:
  const char* file;
  const int   line;
  const char* func;
  FailedCodeGen(const char* _file, int _line, const char* _func) :
      file(_file), line(_line), func(_func) { }
};

void cgPunt(const char* _file, int _line, const char* _func);

#define CG_PUNT(instr) do {                                            \
  if (tx64) {                                                          \
    cgPunt( __FILE__, __LINE__, #instr);                               \
  }                                                                    \
} while(0)

struct ArgGroup;

enum SyncOptions {
  kNoSyncPoint,
  kSyncPoint
};

class CodeGenerator {
public:
  // typedef copied from TranslatorX64 class
  typedef Transl::X64Assembler Asm;
  CodeGenerator(Asm& as, Asm& astubs, Transl::TranslatorX64* tx64) :
      m_as(as), m_astubs(astubs), m_tx64(tx64),
      m_curInst(NULL), m_lastMarker(NULL), m_curTrace(NULL) {
  }

  void cgTrace(Trace* trace, vector<TransBCMapping>* bcMap);

  Address cgBreakpoint(X64Assembler &a);

  Address cgInst(IRInstruction* inst);

  // Autogenerate function declarations for each IR instruction in ir.h
#define OPC(name, flags) Address cg##name(IRInstruction* inst);
  IR_OPCODES
#undef OPC

  // helper functions for code generation
  Address cgCallHelper(Asm& a,
                       TCA addr,
                       PhysReg dstReg,
                       SyncOptions sync,
                       ArgGroup& args);
  Address cgCallHelper(Asm&,
                       TCA addr,
                       SSATmp* dst,
                       SyncOptions sync,
                       ArgGroup& args);

  Address cgStore(PhysReg base,
                  int64_t off,
                  SSATmp* src,
                  bool genStoreType = true);
  Address cgStoreTypedValue(PhysReg base, int64_t off, SSATmp* src);

  Address cgLoad(Type::Tag type,
                 SSATmp* dst,
                 PhysReg base,
                 int64_t off,
                 LabelInstruction* label,
                 IRInstruction* inst = NULL);

  Address cgGuardType(Type::Tag         type,
                      PhysReg           baseReg,
                      int64_t           offset,
                      LabelInstruction* label,
                      IRInstruction*    instr);

  Address cgStMemWork(IRInstruction* inst, bool genStoreType);
  Address cgStRefWork(IRInstruction* inst, bool genStoreType);
  Address cgStLocWork(IRInstruction* inst, bool genStoreType);
  Address cgStPropWork(IRInstruction* inst, bool genStoreType);
  Address cgIncRefWork(Type::Tag type, SSATmp* dst, SSATmp* src);
  Address cgDecRefWork(IRInstruction* inst, bool genZeroCheck);
  Address cgNegateWork(SSATmp* dst, SSATmp* src);
  Address cgNotWork(SSATmp* dst, SSATmp* src);


  Address cgSpillStackWork(IRInstruction* inst, bool allocActRec);
  Address cgAllocActRec1(IRInstruction* inst); // One source, sp
  Address cgAllocActRec5(SSATmp* dst,
                         SSATmp* sp,
                         SSATmp* fp,
                         SSATmp* func,
                         SSATmp* objOrCls,
                         SSATmp* nArgs);
  Address cgAllocActRec6(SSATmp* dst,
                         SSATmp* sp,
                         SSATmp* fp,
                         SSATmp* func,
                         SSATmp* objOrCls,
                         SSATmp* nArgs,
                         SSATmp* magicName);
  Address cgLoadTypedValue(Type::Tag type,
                           SSATmp* dst,
                           PhysReg base,
                           int64_t off,
                           LabelInstruction* label,
                           IRInstruction* inst);

  Address cgNegate(IRInstruction* inst); // helper
  Address cgJcc(IRInstruction* inst); // helper
  Address cgOpCmpHelper(
            IRInstruction* inst,
            void (Asm::*setter)(Reg8),
            int64 (*str_cmp_str)(StringData*, StringData*),
            int64 (*str_cmp_int)(StringData*, int64),
            int64 (*str_cmp_obj)(StringData*, ObjectData*),
            int64 (*obj_cmp_obj)(ObjectData*, ObjectData*),
            int64 (*obj_cmp_int)(ObjectData*, int64),
            int64 (*arr_cmp_arr)(ArrayData*, ArrayData*));
  Address cgJmpZeroHelper(IRInstruction* inst, ConditionCode cc);


private:
  void emitTraceCall(CodeGenerator::Asm& as, int64 pcOff);
  void emitTraceRet(CodeGenerator::Asm& as);
  Address emitCheckStack(CodeGenerator::Asm& as, SSATmp* sp, uint32 numElems,
                         bool allocActRec);
  Address emitCheckCell(CodeGenerator::Asm& as,
                        SSATmp* sp,
                        uint32 index);
  Address cgCheckStaticBit(Type::Tag type,
                           PhysReg reg,
                           bool regIsCount);
  Address cgCheckStaticBitAndDecRef(Type::Tag type,
                                    PhysReg dataReg,
                                    LabelInstruction* exit);
  Address cgCheckRefCountedType(PhysReg typeReg);
  Address cgCheckRefCountedType(PhysReg baseReg,
                                int64 offset);
  Address cgDecRefStaticType(Type::Tag type,
                             PhysReg dataReg,
                             LabelInstruction* exit,
                             bool genZeroCheck);
  Address cgDecRefDynamicType(PhysReg typeReg,
                              PhysReg dataReg,
                              LabelInstruction* exit,
                              bool genZeroCheck);
  Address cgDecRefDynamicTypeMem(PhysReg baseReg,
                                 int64 offset,
                                 LabelInstruction* exit);
  Address cgDecRefMem(Type::Tag type,
                      PhysReg baseReg,
                      int64 offset,
                      LabelInstruction* exit);

  Address cgCheckUninit(SSATmp* src, LabelInstruction* label);
  Address emitFwdJcc(ConditionCode cc, LabelInstruction* label);
  Address emitFwdJmp(Asm& as, LabelInstruction* label);
  Address emitFwdJmp(LabelInstruction* label);
  Address emitSmashableFwdJmp(LabelInstruction* label, SSATmp* toSmash);
  Address emitSmashableFwdJccAtEnd(ConditionCode cc, LabelInstruction* label,
                              SSATmp* toSmash);
  Address emitSmashableFwdJcc(ConditionCode cc, LabelInstruction* label,
                              SSATmp* toSmash);
  void emitGuardOrFwdJcc(IRInstruction*    inst,
                         ConditionCode     cc,
                         LabelInstruction* label);
  Address emitContVarEnvHelperCall(SSATmp* fp, TCA helper);
  const Func* getCurrFunc();
  void recordSyncPoint(Asm& as);
  Address getDtor(DataType type);
  Address getDtorGeneric();
  Address getDtorTyped();

  /*
   * Fields
   */
  Asm& m_as;
  Asm& m_astubs;
  TranslatorX64* m_tx64;
  // current instruction for which code is being generated
  IRInstruction* m_curInst;
  // the last marker instruction before curInst
  LabelInstruction* m_lastMarker;
  Trace* m_curTrace;
};

class ArgDesc {
public:
  enum Kind { Reg, Imm, Addr };

  PhysReg getDstReg() const { return m_dstReg; }
  PhysReg getSrcReg() const { return m_srcReg; }
  Kind getKind() const { return m_kind; }
  void setDstReg(PhysReg reg) { m_dstReg = reg; }
  Address genCode(CodeGenerator::Asm& as) const;
  Immed getImm() const { return m_imm; }

private: // These should be created using ArgGroup.
  friend struct ArgGroup;

  explicit ArgDesc(Kind kind, PhysReg srcReg, Immed immVal)
    : m_kind(kind)
    , m_srcReg(srcReg)
    , m_dstReg(reg::noreg)
    , m_imm(immVal)
  {}

  explicit ArgDesc(SSATmp* tmp, bool val = true);

private:
  Kind m_kind;
  PhysReg m_srcReg;
  PhysReg m_dstReg;
  Immed m_imm;
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
 *   ASSERT(args.size() == 3);
 */
struct ArgGroup {
  size_t size() const { return m_args.size(); }

  ArgDesc& operator[](size_t i) {
    ASSERT(i < size());
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

  ArgGroup& type(Type::Tag tag) {
    m_args.push_back(ArgDesc(ArgDesc::Imm, InvalidReg,
                             Type::toDataType(tag)));
    return *this;
  }

  ArgGroup& addr(PhysReg base, uintptr_t off) {
    m_args.push_back(ArgDesc(ArgDesc::Addr, PhysReg(base), off));
    return *this;
  }

  ArgGroup& ssa(SSATmp* tmp) {
    m_args.push_back(ArgDesc(tmp));
    return *this;
  }

  ArgGroup& ssas(IRInstruction* inst, unsigned begin, unsigned count) {
    for (unsigned i = begin; i < (begin + count); ++i) {
      m_args.push_back(ArgDesc(inst->getSrc(i)));
    }
    return *this;
  }

  /* loads the type of tmp into an arg register */
  ArgGroup& type(SSATmp* tmp) {
    m_args.push_back(ArgDesc(tmp, false));
    return *this;
  }

private:
  std::vector<ArgDesc> m_args;
};

void genCodeForTrace(Trace*                  trace,
                     CodeGenerator::Asm&     a,
                     CodeGenerator::Asm&     astubs,
                     IRFactory*              irFactory,
                     vector<TransBCMapping>* bcMap,
                     TranslatorX64*          tx64);

}}}

#endif
