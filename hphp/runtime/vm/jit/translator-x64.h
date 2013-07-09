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
#ifndef incl_HPHP_RUNTIME_VM_TRANSLATOR_X64_H_
#define incl_HPHP_RUNTIME_VM_TRANSLATOR_X64_H_

#include <signal.h>
#include <memory>
#include <boost/noncopyable.hpp>

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/util/asm-x64.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"
#include "tbb/concurrent_hash_map.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/base/memory/smart_containers.h"

namespace HPHP { class ExecutionContext; }

namespace HPHP { namespace JIT {
class HhbcTranslator;
class IRFactory;
class CSEHash;
class TraceBuilder;
class CodeGenerator;
}}

namespace HPHP { namespace Transl {

struct TraceletCounters {
  uint64_t m_numEntered, m_numExecuted;
};

struct TraceletCountersVec {
  int64_t m_size;
  TraceletCounters *m_elms;
  Mutex m_lock;

  TraceletCountersVec() : m_size(0), m_elms(nullptr), m_lock() { }
};

struct FreeStubList {
  struct StubNode {
    StubNode* m_next;
    uint64_t  m_freed;
  };
  static const uint64_t kStubFree = 0;
  StubNode* m_list;
  FreeStubList() : m_list(nullptr) {}
  TCA maybePop();
  void push(TCA stub);
};

struct CppCall {
  explicit CppCall(void *p) : m_kind(Direct), m_fptr(p) {}
  explicit CppCall(int off) : m_kind(Virtual), m_offset(off) {}
  CppCall(CppCall const&) = default;

  bool isDirect()  const { return m_kind == Direct;  }
  bool isVirtual() const { return m_kind == Virtual; }

  const void* getAddress() const { return m_fptr; }
  int         getOffset()  const { return m_offset; }

 private:
  enum { Direct, Virtual } m_kind;
  union {
    void* m_fptr;
    int   m_offset;
  };
};

class TranslatorX64;
extern __thread TranslatorX64* tx64;

extern void* interpOneEntryPoints[];

extern "C" TCA funcBodyHelper(ActRec* fp);

struct TReqInfo;
struct Label;

static const int kNumFreeLocalsHelpers = 9;

typedef X64Assembler Asm;

constexpr size_t kJmpTargetAlign = 16;
constexpr size_t kNonFallthroughAlign = 64;
constexpr int kJmpLen = 5;
constexpr int kCallLen = 5;
constexpr int kJmpccLen = 6;
constexpr int kJmpImmBytes = 4;
constexpr int kJcc8Len = 3;
constexpr int kLeaRipLen = 7;
constexpr int kTestRegRegLen = 3;
constexpr int kTestImmRegLen = 5;  // only for rax -- special encoding
// Cache alignment is required for mutable instructions to make sure
// mutations don't "tear" on remote cpus.
constexpr size_t kX64CacheLineSize = 64;
constexpr size_t kX64CacheLineMask = kX64CacheLineSize - 1;

enum class TestAndSmashFlags {
  kAlignJccImmediate,
  kAlignJcc,
  kAlignJccAndJmp
};
void prepareForTestAndSmash(Asm&, int testBytes, TestAndSmashFlags flags);
void prepareForSmash(Asm&, int nBytes, int offset = 0);
bool isSmashable(Address frontier, int nBytes, int offset = 0);

// Service request arg packing.
struct ServiceReqArgInfo {
  enum {
    Immediate,
    CondCode,
  } m_kind;
  union {
    uint64_t m_imm;
    ConditionCode m_cc;
  };
};

class TranslatorX64 : public Translator
                    , boost::noncopyable {
  friend class SrcRec; // so it can smash code.
  friend class SrcDB;  // For write lock and code invalidation.
  friend class Tx64Reaper;
  friend class HPHP::JIT::CodeGenerator;

  typedef tbb::concurrent_hash_map<TCA, TCA> SignalStubMap;
  typedef void (*sigaction_t)(int, siginfo_t*, void*);

  typedef X64Assembler Asm;

  class AHotSelector {
   public:
    AHotSelector(TranslatorX64* tx, bool hot) :
        m_tx(tx), m_hot(hot &&
                        tx->ahot.available() > 8192 &&
                        tx->a.base() != tx->ahot.base()) {
      if (m_hot) {
        m_save = tx->a;
        tx->a = tx->ahot;
      }
    }
    ~AHotSelector() {
      if (m_hot) {
        m_tx->ahot = m_tx->a;
        m_tx->a = m_save;
      }
    }
   private:
    TranslatorX64* m_tx;
    Asm            m_save;
    bool           m_hot;
  };

  Asm                    ahot;
  Asm                    a;
  Asm                    astubs;
  Asm                    atrampolines;
  PointerMap             trampolineMap;
  int                    m_numNativeTrampolines;
  size_t                 m_trampolineSize; // size of each trampoline

  SignalStubMap          m_segvStubs;
  sigaction_t            m_segvChain;
  TCA                    m_callToExit;
  TCA                    m_retHelper;
  TCA                    m_retInlHelper;
  TCA                    m_genRetHelper;
  TCA                    m_stackOverflowHelper;
  TCA                    m_irPopRHelper;
  TCA                    m_dtorGenericStub;
  TCA                    m_dtorGenericStubRegs;
  TCA                    m_dtorStubs[kDestrTableSize];
  TCA                    m_defClsHelper;
  TCA                    m_funcPrologueRedispatch;

  TCA                    m_freeManyLocalsHelper;
  TCA                    m_freeLocalsHelpers[kNumFreeLocalsHelpers];

  DataBlock              m_globalData;

  // Data structures for HHIR-based translation
  uint64_t               m_numHHIRTrans;

  virtual void traceCodeGen();

  FixupMap                   m_fixupMap;
  UnwindInfoHandle           m_unwindRegistrar;
  CatchTraceMap              m_catchTraceMap;
  std::vector<TransBCMapping> m_bcMap;

  Debug::DebugInfo m_debugInfo;

private:
  int64_t m_createdTime;

  struct PendingFixup {
    TCA m_tca;
    Fixup m_fixup;
    PendingFixup() { }
    PendingFixup(TCA tca, Fixup fixup) :
      m_tca(tca), m_fixup(fixup) { }
  };
  vector<PendingFixup> m_pendingFixups;

  void drawCFG(std::ofstream& out) const;
  static vector<PhysReg> x64TranslRegs();

  Asm& getAsmFor(TCA addr) { return asmChoose(addr, a, ahot, astubs); }
  void emitIncRef(X64Assembler &a, PhysReg base, DataType dtype);
  void emitIncRef(PhysReg base, DataType);
  void emitIncRefGenericRegSafe(PhysReg base, int disp, PhysReg tmp);
  static CppCall getDtorCall(DataType type);
  void emitCopy(PhysReg srcCell, int disp, PhysReg destCell);

  void emitThisCheck(const NormalizedInstruction& i, PhysReg reg);

public:
  void emitCall(Asm& a, TCA dest);
  void emitCall(Asm& a, CppCall call);
  TCA getCallArrayProlog(Func* func);
private:

  void translateClassExistsImpl(const Tracelet& t,
                                const NormalizedInstruction& i,
                                Attr typeAttr);
  void recordSyncPoint(Asm& a, Offset pcOff, Offset spOff);
  void emitEagerSyncPoint(Asm& a, const Opcode* pc, const Offset spDiff);
  void recordIndirectFixup(CTCA addr, int dwordsPushed);
  void emitStringToClass(const NormalizedInstruction& i);
  void emitStringToKnownClass(const NormalizedInstruction& i,
                              const StringData* clssName);
  void emitObjToClass(const NormalizedInstruction& i);
  void emitClsAndPals(const NormalizedInstruction& i);

  template<int Arity> TCA emitNAryStub(Asm& a, CppCall c);
  TCA emitUnaryStub(Asm& a, CppCall c);
  TCA genericRefCountStub(Asm& a);
  TCA genericRefCountStubRegs(Asm& a);
  void emitFreeLocalsHelpers();
  void emitGenericDecRefHelpers();
  TCA emitPrologueRedispatch(Asm &a);
  TCA emitFuncGuard(Asm& a, const Func *f);
  template <bool reentrant>
  void emitDerefStoreToLoc(PhysReg srcReg, const Location& destLoc);

  void getInputsIntoXMMRegs(const NormalizedInstruction& ni,
                            PhysReg lr, PhysReg rr,
                            RegXMM lxmm, RegXMM rxmm);
  void fpEq(const NormalizedInstruction& i, PhysReg lr, PhysReg rr);
  void emitRB(Asm& a, Trace::RingBufferType t, SrcKey sk,
              RegSet toSave = RegSet());
  void emitRB(Asm& a, Trace::RingBufferType t, const char* msgm,
              RegSet toSave = RegSet());
  void newTuple(const NormalizedInstruction& i, unsigned n);

  enum {
    ArgDontAllocate = -1,
    ArgAnyReg = -2
  };

 private:
  template<typename L>
  void translatorAssert(X64Assembler& a, ConditionCode cc,
                        const char* msg, L setup);

  static uint64_t toStringHelper(ObjectData *obj);
  void invalidateSrcKey(SrcKey sk);
 public:
  template<typename T>
  void invalidateSrcKeys(const T& keys) {
    BlockingLeaseHolder writer(s_writeLease);
    assert(writer);
    for (typename T::const_iterator i = keys.begin(); i != keys.end(); ++i) {
      invalidateSrcKey(*i);
    }
  }

  void registerCatchTrace(CTCA ip, TCA trace);
  TCA getCatchTrace(CTCA ip) const;

  static void SEGVHandler(int signum, siginfo_t *info, void *ctx);

  void fixupWork(VMExecutionContext* ec, ActRec* startRbp) const;
  void fixup(VMExecutionContext* ec) const;
  TCA getTranslatedCaller() const;

  TCA getTopTranslation(SrcKey sk) {
    return getSrcRec(sk)->getTopTranslation();
  }

  TCA getCallToExit() {
    return m_callToExit;
  }

  TCA getRetFromInterpretedFrame() {
    return m_retHelper;
  }

  TCA getRetFromInlinedFrame() {
    return m_retInlHelper;
  }

  TCA getRetFromInterpretedGeneratorFrame() {
    return m_genRetHelper;
  }

  inline bool isValidCodeAddress(TCA tca) const {
    return tca >= ahot.base() && tca < astubs.base() + astubs.capacity();
  }

  // If we were to shove every little helper function into this class
  // header, we'd spend the rest of our lives compiling. So, these public
  // functions are for static helpers private to translator-x64.cpp. Be
  // professional.

  Asm& getAsm()   { return a; }
  void emitChainTo(SrcKey dest, bool isCall = false);

  static bool isPseudoEvent(const char* event);
  void getPerfCounters(Array& ret);

private:
  virtual void syncWork();

public:
  bool acquireWriteLease(bool blocking) {
    return s_writeLease.acquire(blocking);
  }
  void dropWriteLease() {
    s_writeLease.drop();
  }

  void emitGuardChecks(Asm& a, SrcKey, const ChangeMap&,
    const RefDeps&, SrcRec&);
  void emitResolvedDeps(const ChangeMap& resolvedDeps);

  Debug::DebugInfo* getDebugInfo() { return &m_debugInfo; }

  template<typename T, typename... Args>
  T* allocData(Args&&... args) {
    return m_globalData.alloc<T>(std::forward<Args>(args)...);
  }

  FreeStubList m_freeStubs;
  bool freeRequestStub(TCA stub);
  TCA getFreeStub();
  bool checkTranslationLimit(SrcKey, const SrcRec&) const;
  TranslateResult translateTracelet(Tracelet& t);

  void checkRefs(Asm&, SrcKey, const RefDeps&, SrcRec&);

  void emitInlineReturn(Location retvalSrcLoc, int retvalSrcDisp);
  void emitGenericReturn(bool noThis, int retvalSrcDisp);
  void dumpStack(const char* msg, int offset) const;

  void emitFallbackJmp(SrcRec& dest, ConditionCode cc = CC_NZ);
  void emitFallbackJmp(Asm& as, SrcRec& dest, ConditionCode cc = CC_NZ);
  void emitFallbackUncondJmp(Asm& as, SrcRec& dest);
  void emitFallbackCondJmp(Asm& as, SrcRec& dest, ConditionCode cc);
  void emitDebugPrint(Asm&, const char*,
                      PhysReg = reg::r13,
                      PhysReg = reg::r14,
                      PhysReg = reg::rax);

  ServiceReqArgInfo ccArgInfo(ConditionCode cc) {
    return ServiceReqArgInfo{ServiceReqArgInfo::CondCode, { uint64_t(cc) }};
  }

  typedef smart::vector<ServiceReqArgInfo> ServiceReqArgVec;
  void packServiceReqArg(ServiceReqArgVec& args) {
    // all done.
  }

  template<typename T>
  typename std::enable_if<
    // Only allow for things with a sensible cast to uint64_t.
    std::is_integral<T>::value || std::is_pointer<T>::value ||
    std::is_enum<T>::value
  >::type packServiceReqArg(ServiceReqArgVec& args, T arg) {
    // By default, assume we meant to pass an immediate arg.
    args.push_back({ ServiceReqArgInfo::Immediate, { uint64_t(arg) } });
  }

  void packServiceReqArg(ServiceReqArgVec& args,
                         const ServiceReqArgInfo& argInfo) {
    args.push_back(argInfo);
  }

  template<typename T, typename... Arg>
  void packServiceReqArgs(ServiceReqArgVec& argv, T arg, Arg... args) {
    packServiceReqArg(argv, arg);
    packServiceReqArgs(argv, args...);
  }

  void packServiceReqArgs(ServiceReqArgVec& argv) {
    // all done.
  }


  TCA emitServiceReqWork(SRFlags flags, ServiceRequest req,
                         const ServiceReqArgVec& argInfo);
public:
  template<typename... Arg>
  TCA emitServiceReq(SRFlags flags, ServiceRequest sr, Arg... a) {
    ServiceReqArgVec argv;
    packServiceReqArgs(argv, a...);
    return emitServiceReqWork(flags, sr, argv);
  }

  template<typename... Arg>
  TCA emitServiceReq(ServiceRequest sr, Arg... a) {
    return emitServiceReq(SRFlags::None, sr, a...);
  }

  TCA emitRetFromInterpretedFrame();
  TCA emitRetFromInterpretedGeneratorFrame();
  void emitPopRetIntoActRec(Asm& a);
  int32_t emitBindCall(SrcKey srcKey, const Func* funcd, int numArgs);
  void emitCondJmp(SrcKey skTrue, SrcKey skFalse, ConditionCode cc);

  TCA funcPrologue(Func* func, int nArgs, ActRec* ar = nullptr);
  bool checkCachedPrologue(const Func* func, int param, TCA& plgOut) const;
  SrcKey emitPrologue(Func* func, int nArgs);
  static bool eagerRecord(const Func* func);
  int32_t emitNativeImpl(const Func*, bool emitSavedRIPReturn);
  void emitBindJ(Asm& a, ConditionCode cc, SrcKey dest,
                 ServiceRequest req);
  void emitBindJmp(Asm& a, SrcKey dest,
                   ServiceRequest req = REQ_BIND_JMP);
  void emitBindJcc(Asm& a, ConditionCode cc, SrcKey dest,
                   ServiceRequest req = REQ_BIND_JCC);
  void emitBindJmp(SrcKey dest);
  void emitBindCallHelper(SrcKey srcKey,
                          const Func* funcd,
                          int numArgs);
  void emitIncCounter(TCA start, int cntOfs);

 private:
  void moveToAlign(Asm &aa, const size_t alignment = kJmpTargetAlign,
                   const bool unreachable = true);
  static void smash(Asm &a, TCA src, TCA dest, bool isCall);
  static void smashJmp(Asm &a, TCA src, TCA dest) {
    smash(a, src, dest, false);
  }
  static void smashCall(Asm &a, TCA src, TCA dest) {
    smash(a, src, dest, true);
  }

  TCA getTranslation(const TranslArgs& args);
  TCA createTranslation(const TranslArgs& args);
  TCA retranslate(const TranslArgs& args);
  TCA translate(const TranslArgs& args);
  void translateWork(const TranslArgs& args);

  TCA lookupTranslation(SrcKey sk) const;
  TCA retranslateOpt(TransID transId, bool align);
  TCA retranslateAndPatchNoIR(SrcKey sk,
                              bool   align,
                              TCA    toSmash);
  TCA bindJmp(TCA toSmash, SrcKey dest, ServiceRequest req, bool& smashed);
  TCA bindJmpccFirst(TCA toSmash,
                     Offset offTrue, Offset offFalse,
                     bool toTake,
                     ConditionCode cc,
                     bool& smashed);
  TCA bindJmpccSecond(TCA toSmash, const Offset off,
                      ConditionCode cc,
                      bool& smashed);
  bool handleServiceRequest(TReqInfo&, TCA& start, SrcKey& sk);

  void recordGdbTranslation(SrcKey sk, const Func* f,
                            const Asm& a,
                            const TCA start,
                            bool exit, bool inPrologue);
  void recordGdbStub(const Asm& a, TCA start, const char* name);
  void recordBCInstr(uint32_t op, const Asm& a, const TCA addr);

  void emitStackCheck(int funcDepth, Offset pc);
  void emitStackCheckDynamic(int numArgs, Offset pc);
  void emitTestSurpriseFlags(Asm& a);
  void emitCheckSurpriseFlagsEnter(bool inTracelet, Fixup fixup);
  TCA  emitTransCounterInc(Asm& a);

  static void trimExtraArgs(ActRec* ar);
  static int  shuffleArgsForMagicCall(ActRec* ar);
  static void setArgInActRec(ActRec* ar, int argNum, uint64_t datum,
                             DataType t);

  static void fCallArrayHelper(const Offset pcOff, const Offset pcNext);

  TCA getNativeTrampoline(TCA helperAddress);
  TCA emitNativeTrampoline(TCA helperAddress);

public:
  /*
   * enterTC is the main entry point for the translator from the
   * bytecode interpreter (see enterVMWork).  It operates on behalf of
   * a given nested invocation of the intepreter (calling back into it
   * as necessary for blocks that need to be interpreted).
   *
   * If start is not null, data will be used to initialize rStashedAr,
   * to enable us to run a jitted prolog;
   * otherwise, data should be a pointer to the SrcKey to start
   * translating from.
   *
   * But don't call this directly, use one of the helpers below
   */
  void enterTC(TCA start, void* data);
  void enterTCAtSrcKey(SrcKey& sk) {
    enterTC(nullptr, &sk);
  }
  void enterTCAtProlog(ActRec *ar, TCA start) {
    enterTC(start, ar);
  }
  void enterTCAfterProlog(TCA start) {
    enterTC(start, nullptr);
  }

  TranslatorX64();
  virtual ~TranslatorX64();

  void initGdb();
  static TranslatorX64* Get();

  // Called before entering a new PHP "world."
  void requestInit();

  // Called at the end of eval()
  void requestExit();

  // Returns a string with cache usage information
  virtual std::string getUsage();
  virtual size_t getCodeSize();
  virtual size_t getStubSize();
  virtual size_t getTargetCacheSize();

  // true iff calling thread is sole writer.
  static bool canWrite() {
    // We can get called early in boot, so allow null tx64.
    return !tx64 || s_writeLease.amOwner();
  }

  // Returns true on success
  bool dumpTC(bool ignoreLease = false);

  // Returns true on success
  bool dumpTCCode(const char* filename);

  // Returns true on success
  bool dumpTCData();

  // Debugging interfaces to prevent tampering with code.
  void protectCode();
  void unprotectCode();

  int numTranslations(SrcKey sk) const;
private:
  virtual bool addDbgGuards(const Unit* unit);
  virtual bool addDbgGuard(const Func* func, Offset offset);
  void addDbgGuardImpl(SrcKey sk, SrcRec& sr);

public: // Only for HackIR
  void emitReqRetransNoIR(Asm& as, const SrcKey& sk);

private:
  // asize + astubssize + gdatasize + trampolinesblocksize
  size_t m_totalSize;
};

const size_t kTrampolinesBlockSize = 8 << 12;

// minimum length in bytes of each trampoline code sequence
// Note that if stats is on, then this size is ~24 bytes due to the
// instrumentation code that counts the number of calls through each
// trampoline
const size_t kMinPerTrampolineSize = 11;

const size_t kMaxNumTrampolines = kTrampolinesBlockSize /
  kMinPerTrampolineSize;

void fcallHelperThunk() asm ("__fcallHelperThunk");
void funcBodyHelperThunk() asm ("__funcBodyHelperThunk");
void functionEnterHelper(const ActRec* ar);
int64_t decodeCufIterHelper(Iter* it, TypedValue func);

// These could be static but are used in hopt/codegen.cpp
void raiseUndefVariable(StringData* nm);
void defFuncHelper(Func *f);
ObjectData* newInstanceHelper(Class* cls, int numArgs, ActRec* ar,
                              ActRec* prevAr);
ObjectData* newInstanceHelperCached(Class** classCache,
                                    const StringData* clsName, int numArgs,
                                    ActRec* ar, ActRec* prevAr);
ObjectData* newInstanceHelperNoCtorCached(Class** classCache,
                                          const StringData* clsName);

bool isNormalPropertyAccess(const NormalizedInstruction& i,
                       int propInput,
                       int objInput);

bool mInstrHasUnknownOffsets(const NormalizedInstruction& i,
                             Class* contextClass);

struct PropInfo {
  PropInfo()
    : offset(-1)
    , hphpcType(KindOfInvalid)
  {}
  explicit PropInfo(int offset, DataType hphpcType)
    : offset(offset)
    , hphpcType(hphpcType)
  {}

  int offset;
  DataType hphpcType;
};

PropInfo getPropertyOffset(const NormalizedInstruction& ni,
                           Class* contextClass,
                           const Class*& baseClass,
                           const MInstrInfo& mii,
                           unsigned mInd, unsigned iInd);
PropInfo getFinalPropertyOffset(const NormalizedInstruction&,
                                Class* contextClass,
                                const MInstrInfo&);

bool isSupportedCGetM(const NormalizedInstruction& i);
TXFlags planInstrAdd_Int(const NormalizedInstruction& i);
TXFlags planInstrAdd_Array(const NormalizedInstruction& i);
void dumpTranslationInfo(const Tracelet& t, TCA postGuards);

// SpaceRecorder is used in translator-x64.cpp and in hopt/irtranslator.cpp
// RAII logger for TC space consumption.
struct SpaceRecorder {
  const char *m_name;
  const X64Assembler m_a;
  // const X64Assembler& m_a;
  size_t m_start;
  SpaceRecorder(const char* name, const X64Assembler& a) :
      m_name(name), m_a(a), m_start(a.used())
    { }
  ~SpaceRecorder() {
    if (Trace::moduleEnabledRelease(Trace::tcspace, 1)) {
      auto diff = m_a.used() - m_start;
      if (diff) {
        Trace::traceRelease("TCSpace %10s %3td\n", m_name, diff);
      }
    }
  }
};

typedef const int COff; // Const offsets

}}

#endif
