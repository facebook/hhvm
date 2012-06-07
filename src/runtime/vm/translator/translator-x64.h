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

#include <signal.h>
#include <boost/noncopyable.hpp>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/translator/translator.h>
#include <runtime/vm/translator/asm-x64.h>
#include <runtime/vm/translator/srcdb.h>
#include <runtime/vm/translator/regalloc.h>
#include <tbb/concurrent_hash_map.h>
#include <util/ringbuffer.h>
#include <runtime/vm/debug/debug.h>

namespace HPHP {

class ExecutionContext;

namespace VM {
namespace Transl {

using HPHP::x64::register_name_t;

struct TraceletCounters {
  uint64_t m_numEntered, m_numExecuted;
};

struct TraceletCountersVec {
  int64_t m_size;
  TraceletCounters *m_elms;
  Mutex m_lock;

  TraceletCountersVec() : m_size(0), m_elms(NULL), m_lock() { }
};

class TranslatorX64;
extern TranslatorX64* tx64;

class TranslatorX64 : public Translator, public SpillFill,
  public boost::noncopyable {
  friend class SrcRec; // so it can smash code.
  friend class SrcDB;  // For write lock and code invalidation.
  friend class ArgManager;
  friend class WithCounters;
  friend class DiamondGuard;
  friend class DiamondReturn;
  friend class RedirectSpillFill;
  template<int, int, int> friend class CondBlock;
  template<int> friend class JccBlock;
  template<int> friend class IfElseBlock;
  template<int> friend class UnlikelyIfBlock;
  typedef HPHP::DataType DataType;

  typedef tbb::concurrent_hash_map<TCA, TCA> SignalStubMap;
  typedef void (*sigaction_t)(int, siginfo_t*, void*);

  typedef HPHP::x64::X64Assembler Asm;
  typedef HPHP::x64::DataBlock DataBlock;
  Asm                    a;
  Asm                    astubs;
  Asm                    atrampolines;
  PointerMap             trampolineMap;
  int                    m_numNativeTrampolines;
  size_t                 m_trampolineSize; // size of each trampoline
  // spillFillCode points to one of a or astubs. We need it to produce
  // reconciliation code to the alternate buffer. Don't directly manipulate;
  // use DiamondGuard instead.
  Asm*                   m_spillFillCode;

  SrcDB                  m_srcDB;
  SignalStubMap          m_segvStubs;
  sigaction_t            m_segvChain;
  TCA                    m_callToExit;
  TCA                    m_retHelper;
  TCA                    m_stackOverflowHelper;
  TCA                    m_dtorGenericStub;
  TCA                    m_dtorStubs[MaxNumDataTypes];
  TCA                    m_typedDtorStub;
  TCA                    m_interceptHelper;
  TCA                    m_requireHelper;
  TCA                    m_toStringReturnHelper;
  TCA                    m_defClsHelper;
  TCA                    m_funcPrologueRedispatch;
  DataBlock              m_globalData;

  struct SavedRegState {
    explicit SavedRegState(void* saver, const RegAlloc& state)
      : saver(saver)
      , savedState(state)
    {}

    void* saver; // For debugging: ensure these are popped in the right order
    RegAlloc savedState;
  };

  RegAlloc                   m_regMap;
  std::stack<SavedRegState>  m_savedRegMaps;
  volatile bool              m_interceptsEnabled;

  void drawCFG(std::ofstream& out) const;
  static vector<PhysReg> x64TranslRegs();

  PhysReg getReg(const Location& loc) {
    return m_regMap.getReg(loc);
  }

  Asm &getAsmFor(TCA addr) { return Asm::Choose(a, astubs, addr); }
  void emitIncRef(PhysReg base, DataType);
  void emitIncRefGenericRegSafe(PhysReg base, int disp, PhysReg tmp);
  void emitIncRefGeneric(PhysReg base, int disp = 0);
  void emitDecRef(Asm& a, const NormalizedInstruction& i, PhysReg rDatum,
                  DataType type);
  void emitDecRef(const NormalizedInstruction& i, PhysReg rDatum,
                  DataType type);
  void emitDecRefGeneric(const NormalizedInstruction& i, PhysReg srcReg,
                         int disp = 0);
  void emitDecRefInput(Asm& a, const NormalizedInstruction& i, int input);
  void emitCopy(PhysReg srcCell, int disp, PhysReg destCell);
  void emitCopyToStack(Asm& a,
                       const NormalizedInstruction& ni,
                       PhysReg src,
                       int off);
  void emitCopyToStackRegSafe(Asm& a,
                              const NormalizedInstruction& ni,
                              PhysReg src,
                              int off,
                              PhysReg tmpReg);
  void emitTvSetRegSafe(const NormalizedInstruction&, PhysReg from,
    DataType fromType, PhysReg toPtr, PhysReg tmp1, PhysReg tmp2);
  void emitTvSet(const NormalizedInstruction&, PhysReg from,
    DataType fromType, PhysReg toPtr);

  void emitPushAR(const NormalizedInstruction& i, const Func* func,
                  const int bytesPopped = 0, bool isCtor = false,
                  bool clearThis = true, uintptr_t varEnvInvName = 0);

  void emitCallSaveRegs();
  void emitCallPassLoc(const Location& loc, int argNum);
  void emitCallPassLocAddr(const Location& loc, int argNum);
  void emitCall(Asm& a, TCA dest, bool killRegs=false);
  void emitCallFillCont(Asm& a, const Func* orig, const Func* gen);
  void emitCallUnpack(Asm& a, const NormalizedInstruction& i, int nCopy);
  void emitCallPack(Asm& a, const NormalizedInstruction& i, int nCopy);
  void emitContRaiseCheck(Asm& a, const NormalizedInstruction& i);
  void emitContPreNext(const NormalizedInstruction& i, ScratchReg&  rCont);
  void emitContNextCheck(const NormalizedInstruction& i, ScratchReg& rCont);
  template<bool raise>
  void translateContSendImpl(const NormalizedInstruction& i);
  void translateClassExistsImpl(const Tracelet& t,
                                const NormalizedInstruction& i,
                                Attr typeAttr);
  void recordSyncPoint(Asm& a, Offset pcOff, Offset spOff);
  void recordInstrCall(Asm& a, const NormalizedInstruction& i,
                       const SrcKey* sk = 0);
  void recordInstrCall(const NormalizedInstruction& i,
                       const SrcKey* sk = 0) {
    recordInstrCall(a, i, sk);
  }
  void recordInstrStubCall(const NormalizedInstruction& i,
                           const SrcKey* sk = 0) {
    recordInstrCall(astubs, i, sk);
  }
  void emitSideExit(Asm& a, const NormalizedInstruction& dest, bool next);
  void emitStringToClass(const NormalizedInstruction& i);
  void emitStringToKnownClass(const NormalizedInstruction& i,
                              const StringData* clssName);
  void emitObjToClass(const NormalizedInstruction& i);
  void emitClsAndPals(const NormalizedInstruction& i);
  void emitStaticPropInlineLookup(const NormalizedInstruction& i,
                                  const DynLocation& clsInput,
                                  const DynLocation& propInput,
                                  PhysReg scr);

  inline bool isValidCodeAddress(TCA tca) {
    return a.code.isValidAddress(tca) || astubs.code.isValidAddress(tca) ||
      atrampolines.code.isValidAddress(tca);
  }
  template<int Arity> TCA emitNAryStub(Asm& a, void* fptr);
  TCA emitUnaryStub(Asm& a, void* fptr);
  TCA emitBinaryStub(Asm& a, void* fptr);
  TCA genericRefCountStub(Asm& a);
  TCA emitPrologueRedispatch(Asm &a);
  void emitFuncGuard(Asm& a, const Func *f);
  void callUnaryStub(Asm& a, const NormalizedInstruction& i, TCA stub,
                     PhysReg arg, int disp = 0);
  void callBinaryStub(Asm& a, const NormalizedInstruction& i, TCA stub,
                      PhysReg arg1, PhysReg arg2);
  void emitDerefStoreToLoc(PhysReg srcReg, const Location& destLoc);

  void binaryIntegerArith(const NormalizedInstruction &i,
                          Opcode op, PhysReg srcReg, PhysReg srcDestReg);
  void binaryArithCell(const NormalizedInstruction &i,
                       Opcode op, const DynLocation& in1,
                       const DynLocation& inout);
  void binaryArithLocal(const NormalizedInstruction &i,
                        Opcode op,
                        const DynLocation& in1,
                        const DynLocation& in2,
                        const DynLocation& out);
  void emitRB(Asm& a, Trace::RingBufferType t, SrcKey sk,
              RegSet toSave = RegSet());
  void emitRB(Asm& a, Trace::RingBufferType t, const char* msgm,
              RegSet toSave = RegSet());

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
  CASE(Nop) \
  CASE(AddElemC) \
  CASE(AddNewElemC) \
  CASE(Cns) \
  CASE(DefCns) \
  CASE(ClsCnsD) \
  CASE(Concat) \
  CASE(Add) \
  CASE(Xor) \
  CASE(Not) \
  CASE(BitNot) \
  CASE(CastInt) \
  CASE(CastString) \
  CASE(Print) \
  CASE(Jmp) \
  CASE(Switch) \
  CASE(RetC) \
  CASE(NativeImpl) \
  CASE(AGetC) \
  CASE(AGetL) \
  CASE(CGetL) \
  CASE(CGetL2) \
  CASE(CGetS) \
  CASE(CGetM) \
  CASE(CGetG) \
  CASE(VGetL) \
  CASE(VGetG) \
  CASE(VGetM) \
  CASE(IssetM) \
  CASE(SetS) \
  CASE(SetG) \
  CASE(SetM) \
  CASE(SetOpL) \
  CASE(IncDecL) \
  CASE(UnsetL) \
  CASE(UnsetM) \
  CASE(FPushFuncD) \
  CASE(FPushFunc) \
  CASE(FPushClsMethodD) \
  CASE(FPushClsMethodF) \
  CASE(FPushObjMethodD) \
  CASE(FPushCtorD) \
  CASE(FPushContFunc) \
  CASE(FPassR) \
  CASE(FPassL) \
  CASE(FPassM) \
  CASE(FPassS) \
  CASE(FPassG) \
  CASE(This) \
  CASE(InitThisLoc) \
  CASE(FCall) \
  CASE(VerifyParamType) \
  CASE(InstanceOfD) \
  CASE(StaticLocInit) \
  CASE(IterInit) \
  CASE(IterValueC) \
  CASE(IterKey) \
  CASE(IterNext) \
  CASE(ReqDoc) \
  CASE(ReqMod) \
  CASE(ReqSrc) \
  CASE(DefCls) \
  CASE(DefFunc) \
  CASE(Self) \
  CASE(Parent) \
  CASE(ClassExists) \
  CASE(InterfaceExists) \
  CASE(TraitExists) \
  CASE(Dup) \
  CASE(CreateCont) \
  CASE(UnpackCont) \
  CASE(PackCont) \
  CASE(ContReceive) \
  CASE(ContRaised) \
  CASE(ContDone) \
  CASE(ContNext) \
  CASE(ContSend) \
  CASE(ContRaise) \
  CASE(ContValid) \
  CASE(ContCurrent) \
  CASE(ContStopped) \
  CASE(ContHandle)

  // These are instruction-like functions which cover more than one
  // opcode.
#define PSEUDOINSTRS \
  CASE(BinaryArithOp) \
  CASE(SameOp) \
  CASE(EqOp) \
  CASE(LtGtOp) \
  CASE(UnaryBooleanOp) \
  CASE(BranchOp) \
  CASE(AssignToLocalOp) \
  CASE(FPassCOp) \
  CASE(CheckTypeOp)

#define PAIR(nm) \
  void analyze ## nm(Tracelet& t, NormalizedInstruction& i); \
  void translate ## nm(const Tracelet& t, const NormalizedInstruction& i);
#define CASE PAIR

INSTRS
PSEUDOINSTRS

#undef CASE
#undef PAIR


  void branchWithFlagsSet(const Tracelet& t, const NormalizedInstruction& i,
                          HPHP::x64::ConditionCode cc);
  void fuseBranchSync(const Tracelet& t, const NormalizedInstruction& i);
  void fuseBranchAfterBool(const Tracelet& t, const NormalizedInstruction& i,
                           HPHP::x64::ConditionCode cc);
  void fuseBranchAfterStaticBool(const Tracelet& t,
                                 const NormalizedInstruction& i,
                                 bool resultIsTrue);
  void emitPropSet(const NormalizedInstruction& i,
                   const DynLocation& base,
                   const DynLocation& rhs,
                   PhysReg fieldAddr);
  void translateSetMProp(const Tracelet &t, const NormalizedInstruction& i);
  void emitPropGet(const NormalizedInstruction& i,
                   const DynLocation& base,
                   PhysReg fieldAddr,
                   const Location& outLoc);
  void translateCGetMProp(const Tracelet &t, const NormalizedInstruction& i);
  void translateCGetM_LEE(const Tracelet &t, const NormalizedInstruction& i);
  void translateCGetM_GE(const Tracelet &t, const NormalizedInstruction& i);
  void emitGetGlobal(const NormalizedInstruction& i, int nameIdx,
    bool allowCreate);
  void emitArrayElem(const NormalizedInstruction& i,
                     const DynLocation* baseInput,
                     PhysReg baseReg,
                     const DynLocation* keyIn,
                     const Location& outLoc);

  static void toStringHelper(ObjectData *obj);
  void invalidateSrcKey(const SrcKey& sk);
  bool dontGuardAnyInputs(Opcode op);
 public:
  template<typename T>
  void invalidateSrcKeys(const T& keys) {
    BlockingLeaseHolder writer(m_writeLease);
    ASSERT(writer);
    for (typename T::const_iterator i = keys.begin(); i != keys.end(); ++i) {
      invalidateSrcKey(*i);
    }
  }

  void enableIntercepts() {m_interceptsEnabled = true;}
  bool interceptsEnabled() {return m_interceptsEnabled;}

  static void SEGVHandler(int signum, siginfo_t *info, void *ctx);

  // SpillFill interface
  void spillTo(DataType t, PhysReg reg, bool writeType,
               PhysReg base, int disp);
  void spill(const Location& loc, DataType t, PhysReg reg,
             bool writeType);
  void fill(const Location& loc, PhysReg reg);
  void fillByMov(PhysReg src, PhysReg dst);
  void loadImm(int64 immVal, PhysReg reg);
  void poison(PhysReg dest);

  // public for syncing gdb state
  HPHP::VM::Debug::DebugInfo m_debugInfo;

  void fixup(VMExecutionContext* ec) const;

  // helpers for srcDB.
  SrcRec* getSrcRec(const SrcKey& sk) {
    // TODO: add a insert-or-find primitive to THM
    if (SrcRec* r = m_srcDB.find(sk)) return r;
    ASSERT(m_writeLease.amOwner());
    return m_srcDB.insert(sk);
  }

  TCA getTopTranslation(const SrcKey& sk) {
    return getSrcRec(sk)->getTopTranslation();
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
  void syncOutputs(const NormalizedInstruction& i);
  void syncOutputs(int stackOff);

private:
  virtual void syncWork();

  /*
   * The write Lease guards write access to the translation cache,
   * srcDB, and TransDB. The term "lease" is meant to indicate that
   * the right of ownership is conferred for a long, variable time:
   * often the entire length of a request. If a request is not
   * actively translating, it will perform a "hinted drop" of the lease:
   * the lease is unlocked but all calls to acquire(false) from other
   * threads will fail for a short period of time.
   */
  struct Lease {
    static const int64 kStandardHintExpireInterval = 750;
    pthread_t       m_owner;
    pthread_mutex_t m_lock;
    // m_held: since there's no portable, universally invalid pthread_t,
    // explicitly represent the held <-> unheld state machine.
    volatile bool   m_held;
    int64           m_hintExpire;
    int64           m_hintKept;
    int64           m_hintGrabbed;

    Lease() : m_held(false), m_hintExpire(0), m_hintKept(0), m_hintGrabbed(0) {
      pthread_mutex_init(&m_lock, NULL);
    }
    ~Lease() {
      if (m_held && m_owner == pthread_self()) {
        // Can happen, e.g., in exception scenarios.
        pthread_mutex_unlock(&m_lock);
      }
      pthread_mutex_destroy(&m_lock);
    }
    bool amOwner() const;
    // acquire: also returns true if we are already the writer.
    bool acquire(bool blocking = false);
    void drop(int64 hintExpireDelay = 0);

    /*
     * A malevolent entity sometimes takes the write lease out from under us
     * for debugging purposes.
     */
    void gremlinLock();
    void gremlinUnlock();
  };

  enum LeaseAcquire {
    ACQUIRE,
    NO_ACQUIRE,
  };
  struct LeaseHolderBase {
  protected:
    LeaseHolderBase(Lease& l, LeaseAcquire acquire, bool blocking);

  public:
    ~LeaseHolderBase();
    operator bool() const { return m_haveLock; }
    bool acquire();

  private:
    Lease& m_lease;
    bool m_haveLock;
    bool m_acquired;
  };
  struct LeaseHolder : public LeaseHolderBase {
    LeaseHolder(Lease& l, LeaseAcquire acquire = ACQUIRE)
        : LeaseHolderBase(l, acquire, false) {}
  };
  struct BlockingLeaseHolder : public LeaseHolderBase {
    BlockingLeaseHolder(Lease& l)
        : LeaseHolderBase(l, ACQUIRE, true) {}
  };
  Lease m_writeLease;

public:

#define SERVICE_REQUESTS \
  REQ(EXIT)              \
  REQ(BIND_CALL)         \
  REQ(BIND_JMP)          \
  REQ(BIND_ADDR)         \
  REQ(BIND_SIDE_EXIT)    \
  REQ(BIND_JMPCC_FIRST)  \
  REQ(BIND_JMPCC_SECOND) \
  REQ(RETRANSLATE)       \
  REQ(INTERPRET)         \
  REQ(POST_INTERP_RET)   \
  REQ(STACK_OVERFLOW)    \
  REQ(RESUME)

  enum ServiceRequest {
#define REQ(nm) REQ_##nm,
    SERVICE_REQUESTS
#undef REQ
  };

  void analyzeInstr(Tracelet& t, NormalizedInstruction& i);
  bool acquireWriteLease(bool blocking) {
    return m_writeLease.acquire(blocking);
  }
  void dropWriteLease() {
    m_writeLease.drop();
  }
  void interceptPrologues(Func* func);

  void emitGuardChecks(Asm& a, const SrcKey&, const ChangeMap&,
    const RefDeps&, SrcRec&);

private:
  TCA getInterceptHelper();
  void translateInstr(const Tracelet& t, const NormalizedInstruction& i);
  bool checkTranslationLimit(const SrcKey&, const SrcRec&) const;
  void translateTracelet(const Tracelet& t);
  void emitStringCheck(Asm& _a, PhysReg base, int offset, PhysReg tmp);
  void checkType(Asm&, const Location& l, const RuntimeType& rtt,
    SrcRec& fail);
  void checkRefs(Asm&, const SrcKey&, const RefDeps&, SrcRec&);

  void emitSmartAddImm(register_name_t rsrcdest, int64_t imm);
  void emitFrameRelease(Asm& a, const NormalizedInstruction& i,
                        bool noThis = false);
  void dumpStack(const char*msg, int offset) const;

  static const size_t kJmpTargetAlign = 16;
  static const int kJmpLen = 5;
  static const int kJmpccLen = 6;
  // Cache alignment is required for mutable instructions to make sure
  // mutations don't "tear" on remote cpus.
  static const size_t kX64CacheLineSize = 64;
  void moveToAlign(Asm &aa, const size_t alignment = kJmpTargetAlign,
                   const bool unreachable = true);
  void prepareForSmash(Asm &a, int nBytes);
  void prepareForSmash(int nBytes);
  static bool isSmashable(Asm &a, int nBytes);
  static void smash(Asm &a, TCA src, TCA dest);

  TCA getTranslation(const SrcKey *sk, bool align);
  TCA retranslate(SrcKey sk, bool align);
  TCA bindJmp(TCA toSmash, SrcKey dest, bool isAddr = false);
  TCA bindJmpccFirst(TCA toSmash,
                     Offset offTrue, Offset offFalse,
                     bool toTake,
                     HPHP::x64::ConditionCode cc);
  TCA bindJmpccSecond(TCA toSmash, const Offset off,
                      HPHP::x64::ConditionCode cc);
  void emitFallbackJmp(SrcRec& dest);

  TCA emitServiceReq(bool align, ServiceRequest, int numArgs, ...);
  TCA emitServiceReq(ServiceRequest, int numArgs, ...);
  TCA emitServiceReqVA(bool align, ServiceRequest, int numArgs, va_list args);
  void emitRetC(const NormalizedInstruction& i);
  TCA emitRetFromInterpretedFrame();
  void emitBox(DataType t, PhysReg rToBox);
  void emitUnboxTopOfStack(const NormalizedInstruction& ni);
  void emitBindCall(const Tracelet& t, const NormalizedInstruction &ni,
                    Offset atCall, Offset after);
  void emitCondJmp(const SrcKey &skTrue, const SrcKey &skFalse,
                   HPHP::x64::ConditionCode cc);
  void emitInterpOne(const Tracelet& t, const NormalizedInstruction& i);
  void emitMovRegReg(Asm& a, PhysReg src, PhysReg dest);
  void emitMovRegReg(PhysReg src, PhysReg dest);
  void enterTC(SrcKey sk);

  void recordGdbTranslation(const SrcKey& sk, const Unit* u, TCA start,
                            int numTCBytes, bool exit, bool inPrologue);

  void emitStackCheck(int funcDepth, Offset pc);
  void emitStackCheckDynamic(int numArgs, Offset pc);
  void emitLoadSurpriseFlags();
  TCA  emitTransCounterInc(Asm& a);

  static void trimExtraArgs(ActRec* ar);
  static int  shuffleArgsForMagicCall(ActRec* ar);
  static void setArgInActRec(ActRec* ar, int argNum, uint64_t datum,
                             DataType t);
  TCA funcPrologue(Func* func, int nArgs);
  SrcKey emitPrologue(Func* func, int nArgs);
  void emitNativeImpl(const Func*, bool emitSavedRIPReturn);
  TCA emitInterceptPrologue(Func* func, TCA next=NULL);
  void emitBindJmp(Asm& a, const SrcKey& dest,
                   ServiceRequest req = REQ_BIND_JMP);
  void emitBindJmp(const SrcKey& dest);
  void emitIncCounter(TCA start, int cntOfs);

  void analyzeReqLit(Tracelet& t, NormalizedInstruction& i,
                     InclOpFlags flags);
  void translateReqLit(const Tracelet& t, const NormalizedInstruction& i,
                       InclOpFlags flags);
  struct ReqLitStaticArgs {
    HPHP::Eval::PhpFile* m_efile;
    TCA m_fallthrough;
    Offset m_pcOff;
    bool m_local;
  };
  static uint64 reqLitHelper(const ReqLitStaticArgs* args, Cell *fp, Cell *sp);

  TCA getNativeTrampoline(TCA helperAddress);
  TCA emitNativeTrampoline(TCA helperAddress);

public:
  void resume(SrcKey sk);
  TCA translate(const SrcKey *sk, bool align);

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

  // Returns a string with cache usage information
  std::string getUsage();

  // true iff calling thread is sole writer.
  static bool canWrite() {
    // We can get called early in boot, so allow null tx64.
    return !tx64 || tx64->m_writeLease.amOwner();
  }

  // Returns true on success
  bool dumpTC();

  // Returns true on success
  bool dumpTCCode(const char* filename);

  // Returns true on success
  bool dumpTCData();

  // Async hook for file modifications.
  bool invalidateFile(Eval::PhpFile* f);
  void invalidateFileWork(Eval::PhpFile* f);

protected:
  virtual bool addDbgGuards(const Unit* unit);
  virtual bool addDbgGuard(const Func* func, Offset offset);
  void addDbgGuardImpl(const SrcKey& sk, SrcRec& sr);
};


/*
 * RAII bookmark for temporarily rewinding a.code.frontier.
 */
class CodeCursor {
  typedef HPHP::x64::X64Assembler Asm;
  Asm& m_a;
  TCA m_oldFrontier;
  public:
  CodeCursor(Asm& a, TCA newFrontier) :
    m_a(a), m_oldFrontier(a.code.frontier) {
      ASSERT(TranslatorX64::canWrite());
      m_a.code.frontier = newFrontier;
      TRACE_MOD(Trace::trans, 1, "RewindTo: %p (from %p)\n",
                m_a.code.frontier, m_oldFrontier);
    }
  ~CodeCursor() {
    ASSERT(TranslatorX64::canWrite());
    m_a.code.frontier = m_oldFrontier;
    TRACE_MOD(Trace::trans, 1, "Restore: %p\n",
              m_a.code.frontier);
  }
};

// length in bytes of the code block holding trampolines
const size_t kTrampolinesBlockSize = 8 << 12;

// minimum length in bytes of each trampoline code sequence
// Note that if stats is on, then this size is ~24 bytes due to the
// instrumentation code that counts the number of calls through each
// trampoline
const size_t kMinPerTrampolineSize = 11;

const size_t kMaxNumTrampolines = kTrampolinesBlockSize /
  kMinPerTrampolineSize;

void fcallHelperThunk() asm ("__fcallHelperThunk");

} } }

#endif
