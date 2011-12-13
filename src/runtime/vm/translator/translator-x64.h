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
#ifndef _TRANSLATOR_X64_H_
#define _TRANSLATOR_X64_H_

#include "boost/noncopyable.hpp"
#include <runtime/vm/bytecode.h>
#include <runtime/vm/translator/translator.h>
#include <runtime/vm/translator/asm-x64.h>
#include <runtime/vm/translator/srcdb.h>
#include <runtime/vm/translator/regalloc.h>
#include <tbb/concurrent_hash_map.h>

#include <runtime/vm/debug/debug.h>

namespace HPHP {
namespace VM {
namespace Transl {

using HPHP::x64::register_name_t;

/*
 * Function prologues prepare the function's frame, trimming or adding
 * arguments as necessary, then dispatch to translations of
 * the first BB of the function. We specialize the prologues by number
 * of parameters.
 */
struct FuncPrologueKey {
  const Func* m_func;
  const int   m_nArgs;
  const int   m_flags;
  FuncPrologueKey() : m_func(NULL), m_nArgs(-1),
                      m_flags(Translator::FuncPrologueNormal) { }
  FuncPrologueKey(const Func* func, int nArgs, int flags) :
      m_func(func), m_nArgs(nArgs), m_flags(flags) { }
  size_t operator()(const FuncPrologueKey& k) const {
    // Hash function.
    size_t hash = (pointer_hash<const Func>()(k.m_func) << 4) ^ k.m_nArgs;
    hash ^= m_flags;
    return hash;
  }
  bool operator==(const FuncPrologueKey& rhs) const {
    return m_func == rhs.m_func && m_nArgs == rhs.m_nArgs &&
           m_flags == rhs.m_flags;
  }
};

typedef hphp_hash_map<FuncPrologueKey, TCA, FuncPrologueKey> FuncPrologueMap;
struct TCAHasher {
  size_t operator()(const TCA& toHash) const {
    return (size_t)toHash;
  }
};

struct TraceletCounters {
  uint64_t m_numEntered, m_numExecuted;
};

struct TraceletCountersVec {
  int64_t m_size;
  TraceletCounters *m_elms;
  Mutex m_lock;

  TraceletCountersVec() : m_size(0), m_elms(NULL), m_lock() { }
};

class TranslatorX64 : public Translator, public boost::noncopyable {
  friend class SrcRec; // so it can smash code.
  friend class ArgManager;
  friend class WithCounters;
  typedef HPHP::DataType DataType;

  typedef hphp_hash_map<SrcKey, SrcRec, SrcKey> SrcDB;
  typedef tbb::concurrent_hash_map<TCA, boost::shared_ptr<
    TraceletCountersVec> > TraceletCountersVecMap;
  typedef tbb::concurrent_hash_map<TCA, TraceletCounters> TraceletCountersMap;

  typedef HPHP::x64::X64Assembler Asm;
  typedef HPHP::x64::DataBlock DataBlock;
  Asm                    a;
  Asm                    astubs;

  int                    numArgs;
  SrcDB                  srcDB;
  TransDB                transDB;
  TCA                    m_callToExit;
  TCA                    m_retHelper;
  TCA                    m_stackOverflowHelper;
  TCA                    m_dtorGenericStub;
  TCA                    m_dtorStubs[MaxNumDataTypes];
  TCA                    m_raiseUndefCnsStub;
  TCA                    m_saveVMRegsStub;
  DataBlock              m_globalData;

  int64_t                m_counterThreadIdx;
  TraceletCountersVecMap m_threadCounters;
  TraceletCountersMap    m_globalCounters;

  class X64SpillFill : public SpillFill {
    Asm &a;
    RegAlloc &regAlloc;
   public:
    X64SpillFill(Asm &_a, RegAlloc &_regAlloc) : a(_a), regAlloc(_regAlloc) { }
    void spill(const Location& loc, DataType t, PhysReg reg,
               bool writeType);
    void spillHome(RegAlloc &regMap,
                   const Location& homeLoc,
                   const Location& dest);
    void fill(const Location& loc, PhysReg reg);
    void loadImm(int64 immVal, PhysReg reg);
    void poison(PhysReg dest);
  };
  X64SpillFill           m_spf;
  RegAlloc               m_regMap;

  void drawCFG(std::ofstream& out) const;
  static vector<PhysReg> x64TranslRegs();

  PhysReg getReg(const Location& loc) {
    return m_regMap.getReg(loc);
  }

  void emitIncRef(PhysReg base, DataType);
  void emitIncRefGeneric(PhysReg base, int disp=0);
  void emitDecRef(PhysReg rDatum, DataType type);
  void emitDecRefGeneric(PhysReg srcReg, int disp=0);
  void emitCopy(PhysReg srcCell, int disp, PhysReg destCell);

  void emitPushAR(const NormalizedInstruction& i, const Func* func,
                  const int bytesPopped = 0);
  void emitCallSaveRegs();
  void emitCallPassLoc(const Location& loc, int argNum);
  void emitCallPassLocAddr(const Location& loc, int argNum);
  void emitCall(TCA dest);
  void emitStringToClass(const NormalizedInstruction& i);
  void emitObjToClass(const NormalizedInstruction& i);
  void emitClsAndPals(const NormalizedInstruction& i);
  void emitStaticPropInlineLookup(const NormalizedInstruction& i,
                                  const DynLocation& clsInput,
                                  const DynLocation& propInput,
                                  PhysReg scr);

  TCA emitUnaryStub(void* fptr, bool savePC = false);
  void callUnaryStub(TCA stub, PhysReg arg, int disp=0);
  void emitDerefStoreToLoc(PhysReg srcReg, const Location& destLoc);

  void binaryIntegerArith(const NormalizedInstruction &i,
                          Opcode op, PhysReg srcReg, PhysReg srcDestReg);
  void binaryArithCell(const NormalizedInstruction &i,
                       Opcode op, const DynLocation& in1,
                       const DynLocation& inout);
  void binaryArithHome(const NormalizedInstruction &i,
                       Opcode op,
                       const DynLocation& in1,
                       const DynLocation& in2,
                       const DynLocation& out);

#define INSTRS \
  CASE(PopC) \
  CASE(PopV) \
  CASE(PopR) \
  CASE(UnboxR) \
  CASE(Null) \
  CASE(True) \
  CASE(False) \
  CASE(Int) \
  CASE(String) \
  CASE(Array) \
  CASE(NewArray) \
  CASE(AddElemC) \
  CASE(AddNewElemC) \
  CASE(Cns) \
  CASE(Concat) \
  CASE(Add) \
  CASE(Xor) \
  CASE(Not) \
  CASE(BitNot) \
  CASE(CastInt) \
  CASE(Print) \
  CASE(Jmp) \
  CASE(RetC) \
  CASE(Loc) \
  CASE(Cls) \
  CASE(ClsH) \
  CASE(CGetS) \
  CASE(CGetM) \
  CASE(CGetG) \
  CASE(VGetH) \
  CASE(VGetG) \
  CASE(VGetM) \
  CASE(IssetH) \
  CASE(IssetM) \
  CASE(SetS) \
  CASE(SetM) \
  CASE(SetOpH) \
  CASE(IncDecH) \
  CASE(UnsetH) \
  CASE(UnsetM) \
  CASE(FPushFuncD) \
  CASE(FPushFunc) \
  CASE(FPushObjMethodD) \
  CASE(FPassR) \
  CASE(FPassH) \
  CASE(FPassM) \
  CASE(This) \
  CASE(FCall) \
  CASE(IterInit) \
  CASE(IterValueC) \
  CASE(IterKey) \
  CASE(IterNext)

  // These are instruction-like functions which cover more than one
  // opcode.
#define PSEUDOINSTRS \
  CASE(BinaryArithOp) \
  CASE(SameOp) \
  CASE(EqOp) \
  CASE(LtGtOp) \
  CASE(UnaryBooleanOp) \
  CASE(BranchOp) \
  CASE(CGetHOp) \
  CASE(AssignToLocalOp) \
  CASE(FPassCOp)

#define PAIR(nm) \
  void analyze ## nm(Tracelet& t, NormalizedInstruction& i); \
  void translate ## nm(const Tracelet& t, const NormalizedInstruction& i);
#define CASE PAIR

  void translateSetMProp(const Tracelet &t, const NormalizedInstruction& i);
  void translateCGetMProp(const Tracelet &t, const NormalizedInstruction& i);

INSTRS
PSEUDOINSTRS

#undef CASE
#undef PAIR

 public:
  // public for syncing gdb state
  HPHP::VM::Debug::DebugInfo m_debugInfo;

  SrcRec& getSrcRec(SrcKey sk) {
    return srcDB[sk];
  }

  const TransRec& getTxRec(TCA tca) {
    ASSERT(transDB.find(tca) != transDB.end());
    return transDB[tca];
  }

  TCA getCallToExit() {
    return m_callToExit;
  }

  TCA getRetFromInterpretedFrame() {
    return m_retHelper;
  }

  // If we were to shove every little helper function into this class
  // header, we'd spend the rest of our lives compiling. So, these public
  // functions are for static helpers private to translator-x64.cpp. Be
  // professional.

  Asm& getAsm()   { return a; }
  void emitChainTo(const SrcKey *dest, bool isCall = false);
  void syncOutputs(const Tracelet& t);

 private:
  /*
   * The write Lease guards write access to the translation cache, srcDB, and
   * TransDB. The term "lease" is meant to indicate that the right of ownership
   * is conferred for a long, variable time: often the entire length of a
   * request. While we don't currently have a mechanism for breaking the lease,
   * we may someday.
   */
  struct Lease {
    pthread_t       m_owner;
    pthread_mutex_t m_lock;
    // m_held: since there's no portable, universally invalid pthread_t,
    // explicitly represent the held <-> unheld state machine.
    bool            m_held;
    Lease() : m_held(false) {
      pthread_mutex_init(&m_lock, NULL);
    }
    ~Lease() {
      if (m_held && m_owner == pthread_self()) {
        // Can happen, e.g., in exception scenarios.
        pthread_mutex_unlock(&m_lock);
      }
      pthread_mutex_destroy(&m_lock);
    }
    bool amOwner();
    // acquire: also returns true if we are already the writer.
    bool acquire();
    void drop();

    /*
     * A malevolent entity sometimes takes the write lease out from under us
     * for debugging purposes.
     */
    void gremlinLock();
    void gremlinUnlock();
  };
  Lease m_writeLease;
  FuncPrologueMap m_funcPrologues;

public:
  void analyzeInstr(Tracelet& t, NormalizedInstruction& i);

private:
  void translateInstr(const Tracelet& t, const NormalizedInstruction& i);
  void translateTracelet(const Tracelet& t);
  void checkType(const Location& l, const RuntimeType &rtt, SrcRec &fail);
  void checkRefs(const Tracelet& t, SrcRec &fail);
  void emitRefTest(PhysReg rBitBec, int argNum, TCA* outToSmash,
		   bool shouldBeRef);

  void emitSmartAddImm(register_name_t rsrcdest, int64_t imm);
  void emitFrameRelease(Asm& a, const NormalizedInstruction& i);
  void dumpStack(const char*msg, int offset) const;

  static const size_t kJmpTargetAlign = 16;
  static const int kJmpLen = 5;
  static const int kJmpccLen = 6;
  // Cache alignment is required for mutable instructions to make sure
  // mutations don't "tear" on remote cpus.
  static const size_t kX64CacheLineSize = 64;
  void moveToAlign(Asm &aa, const size_t alignment = kJmpTargetAlign,
                   const bool unreachable = true);
  void prepareForSmash(int nBytes);
  static bool isSmashable(Asm &a, int nBytes);
  static void smash(Asm &a, TCA src, TCA dest);

  TCA getTranslation(const SrcKey *sk, bool align);
  TCA retranslate(SrcKey sk, bool align);
  TCA bindJmp(TCA toSmash, SrcKey dest);
  TCA bindJmpccFirst(TCA toSmash,
                     Offset offTrue, Offset offFalse,
                     int64_t toTake);
  TCA bindJmpccSecond(TCA toSmash, const Offset off, bool isJz);
  void emitFallbackJmp(SrcRec& dest);

  void cleanLocalReg(const NormalizedInstruction &i,
                     const StringData *maybeName,
                     bool onlyPseudoMain = false);

  enum ServiceRequest {
    REQ_EXIT,
    REQ_BIND_CALL,
    REQ_BIND_JMP,
    REQ_BIND_JMPCC_FIRST,
    REQ_BIND_JMPCC_SECOND,
    REQ_RETRANSLATE,
    REQ_INTERPRET,
    REQ_POST_INTERP_RET,
    REQ_STACK_OVERFLOW,
    REQ_RESUME,
  };
  TCA emitServiceReq(bool align, ServiceRequest, int numArgs, ...);
  TCA emitServiceReq(ServiceRequest, int numArgs, ...);
  TCA emitServiceReqVA(bool align, ServiceRequest, int numArgs, va_list args);
  void emitRetC(const NormalizedInstruction& i);
  TCA emitRetFromInterpretedFrame();
  void emitBox(DataType t, PhysReg rToBox);
  void emitUnboxTopOfStack(const NormalizedInstruction& ni);
  void emitBindCall(const NormalizedInstruction &ni,
                    Offset atCall, Offset after);
  void emitCondJmp(const SrcKey &skTrue, const SrcKey &skFalse);
  void emitInterpOne(const Tracelet& t, const NormalizedInstruction& i);
  void emitMovRegReg(PhysReg src, PhysReg dest);
  void enterTC(SrcKey sk);

  void recordGdbTranslation(const SrcKey& sk, const Unit* u, TCA start,
                            int numTCBytes, bool exit, bool inPrologue);

  void emitStackCheck(int funcDepth, Offset pc);
  void emitStackCheckDynamic(int numArgs, Offset pc);
  void emitLoadDynTracer();
  void emitBuiltinCall(const Func* func, int numArgs, Offset after);
  static void trimExtraArgs(ActRec* ar, int numPassed);
  static void shuffleArgsForMagicCall(ActRec* ar);
  static void setArgInActRec(ActRec* ar, int argNum, uint64_t datum,
                             DataType t);
  TCA funcPrologue(const Func* func, int nArgs, int flags);
  void emitBindJmp(const SrcKey& dest);
  void emitIncCounter(TCA start, int cntOfs);
public:
  void resume(SrcKey sk);
  TCA translate(const SrcKey *sk, bool align);
  TransDB& getTransDB() {
    return transDB;
  }

  TranslatorX64();

  static TranslatorX64* Get();

  // Called once at the dawn of time.
  void processInit();

  // Called before entering a new PHP "world."
  void requestInit();

  // Called at the end of eval()
  void requestExit();

  // Called when name is bound to a value
  void defineCns(StringData* name);

  static SrcRec* TCAToSrcRec(TCA tca);
};

} } }

#endif
