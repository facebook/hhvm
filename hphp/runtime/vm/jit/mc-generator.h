/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_RUNTIME_VM_JIT_MC_GENERATOR_H_
#define incl_HPHP_RUNTIME_VM_JIT_MC_GENERATOR_H_

#include <memory>
#include <utility>
#include <vector>
#include <boost/noncopyable.hpp>
#include <tbb/concurrent_hash_map.h>

#include "hphp/util/asm-x64.h"
#include "hphp/util/code-cache.h"
#include "hphp/util/ringbuffer.h"

#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/back-end.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/cpp-call.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"

namespace HPHP { namespace jit {

typedef X64Assembler Asm;
typedef hphp_hash_map<TCA, TransID> TcaTransIDMap;
typedef hphp_hash_map<uint64_t,const uint64_t*> LiteralMap;

struct Label;
struct MCGenerator;
struct AsmInfo;
struct IRGS;

extern "C" MCGenerator* mcg;

constexpr size_t kNonFallthroughAlign = 64;
constexpr int kLeaRipLen = 7;
constexpr int kTestRegRegLen = 3;
constexpr int kTestImmRegLen = 5;  // only for rax -- special encoding
// Cache alignment is required for mutable instructions to make sure
// mutations don't "tear" on remote cpus.
constexpr size_t kX64CacheLineSize = 64;
constexpr size_t kX64CacheLineMask = kX64CacheLineSize - 1;
const TCA kInvalidCatchTrace   = (TCA)(-1);

//////////////////////////////////////////////////////////////////////

struct FreeStubList {
  struct StubNode {
    StubNode* m_next;
    uint64_t  m_freed;
  };
  static const uint64_t kStubFree = 0;
  FreeStubList() : m_list(nullptr) {}
  TCA peek() { return (TCA)m_list; }
  TCA maybePop();
  void push(TCA stub);
 private:
  StubNode* m_list;
};

struct PendingFixup {
  TCA m_tca;
  Fixup m_fixup;
  PendingFixup() { }
  PendingFixup(TCA tca, Fixup fixup) :
    m_tca(tca), m_fixup(fixup) { }
};

struct CodeGenFixups {
  std::vector<PendingFixup> m_pendingFixups;
  std::vector<std::pair<CTCA, TCA>> m_pendingCatchTraces;
  std::vector<std::pair<TCA,TransID>> m_pendingJmpTransIDs;
  std::vector<TCA> m_reusedStubs;
  std::set<TCA> m_addressImmediates;
  std::set<TCA*> m_codePointers;
  std::vector<TransBCMapping> m_bcMap;
  std::multimap<TCA,std::pair<int,int>> m_alignFixups;
  GrowableVector<IncomingBranch> m_inProgressTailJumps;
  LiteralMap m_literals;

  CodeBlock* m_tletMain{nullptr};
  CodeBlock* m_tletCold{nullptr};
  CodeBlock* m_tletFrozen{nullptr};

  void setBlocks(CodeBlock* main, CodeBlock* cold, CodeBlock* frozen) {
    m_tletMain = main;
    m_tletCold = cold;
    m_tletFrozen = frozen;
  }

  void process_only(GrowableVector<IncomingBranch>* inProgressTailBranches);
  void process(GrowableVector<IncomingBranch>* inProgressTailBranches) {
    process_only(inProgressTailBranches);
    clear();
  }
  bool empty() const;
  void clear();
};

struct UsageInfo {
  std::string m_name;
  size_t m_used;
  size_t m_capacity;
  bool m_global;
};

struct TransRelocInfo;

//////////////////////////////////////////////////////////////////////

/*
 *
 * MCGenerator handles the machine-level details of code generation (e.g.,
 * translation cache entry, code smashing, code cache management) and delegates
 * the bytecode-to-asm translation process to translateRegion().
 *
 */
struct MCGenerator : private boost::noncopyable {
  /*
   * True iff the calling thread is the sole writer.
   */
  static bool canWrite() {
    // We can get called early in boot, so allow null mcg.
    return !mcg || Translator::WriteLease().amOwner();
  }

  static CppCall getDtorCall(DataType type);

public:
  MCGenerator();
  ~MCGenerator();

  /*
   * Accessors.
   */
  Translator& tx() { return m_tx; }
  FixupMap& fixupMap() { return m_fixupMap; }
  CodeGenFixups& cgFixups() { return m_fixups; }
  FreeStubList& freeStubList() { return m_freeStubs; }
  LiteralMap& literals() { return m_literals; }
  Annotations& annotations() { return m_annotations; }
  void recordSyncPoint(CodeAddress frontier, Fixup fix);

  DataBlock& globalData() { return code.data(); }
  Debug::DebugInfo* getDebugInfo() { return &m_debugInfo; }
  BackEnd& backEnd() { return *m_backEnd; }

  TcaTransIDMap& getJmpToTransIDMap() {
    return m_jmpToTransID;
  }

  inline bool isValidCodeAddress(TCA tca) const {
    return code.isValidCodeAddress(tca);
  }

  /*
   * Handlers for function prologues.
   */
  TCA getFuncPrologue(Func* func, int nPassed, ActRec* ar = nullptr,
                      bool forRegeneratePrologue = false);
  TCA getCallArrayPrologue(Func* func);
  void smashPrologueGuards(TCA* prologues, int numPrologues, const Func* func);

  inline void sync() {
    if (tl_regState == VMRegState::CLEAN) return;
    syncWork();
  }

  bool useLLVM() const { return m_useLLVM; }
  void setUseLLVM(bool useLLVM) { m_useLLVM = useLLVM; }

  template<typename T, typename... Args>
  T* allocData(Args&&... args) {
    return code.data().alloc<T>(std::forward<Args>(args)...);
  }

  /*
   * Allocate a literal value in the global data section.
   */
  const uint64_t* allocLiteral(uint64_t val);

  /*
   * enterTC is the main entry point for the translator from the bytecode
   * interpreter.  It operates on behalf of a given nested invocation of the
   * intepreter (calling back into it as necessary for blocks that need to be
   * interpreted).
   *
   * If start is the address of a func prologue, stashedAR should be the ActRec
   * prepared for the call to that function, otherwise it should be nullptr.
   *
   * But don't call it directly, use one of the helpers below.
   */
 private:
  void enterTC(TCA start, ActRec* stashedAR);
 public:
  void enterTC() {
    enterTC(m_tx.uniqueStubs.resumeHelper, nullptr);
  }
  void enterTCAtPrologue(ActRec *ar, TCA start) {
    assertx(ar);
    assertx(start);
    enterTC(start, ar);
  }
  void enterTCAfterPrologue(TCA start) {
    assertx(start);
    enterTC(start, nullptr);
  }

  /*
   * Called before entering a new PHP "world."
   */
  void requestInit();

  /*
   * Called at the end of eval()
   */
  void requestExit();

  void initUniqueStubs();
  int numTranslations(SrcKey sk) const;
  bool addDbgGuards(const Unit* unit);
  bool addDbgGuard(const Func* func, Offset offset, bool resumed);
  bool freeRequestStub(TCA stub);

  /*
   * Return a TCA suitable for emitting an ephemeral stub. A reused stub will
   * be returned if one is available. Otherwise, frozen.frontier() will be
   * returned.
   *
   * If not nullptr, isReused will be set to whether or not a reused stub was
   * returned.
   */
  TCA getFreeStub(CodeBlock& frozen, CodeGenFixups* fixups,
                  bool* isReused = nullptr);
  void registerCatchBlock(CTCA ip, TCA block);
  folly::Optional<TCA> getCatchTrace(CTCA ip) const;
  CatchTraceMap& catchTraceMap() { return m_catchTraceMap; }
  TCA getTranslatedCaller() const;
  void setJmpTransID(TCA jmp);
  bool profileSrcKey(SrcKey sk) const;
  void getPerfCounters(Array& ret);
  bool reachedTranslationLimit(SrcKey, const SrcRec&) const;
  void traceCodeGen(IRGS&);
  void recordGdbStub(const CodeBlock& cb, TCA start, const std::string& name);

  /*
   * Dump translation cache.  True if successful.
   */
  bool dumpTC(bool ignoreLease = false);

  /*
   * Return cache usage information as a string
   */
  std::string getUsageString();
  std::string getTCAddrs();
  std::vector<UsageInfo> getUsageInfo();

  /*
   * Returns the total size of the TC now and at the beginning of this request,
   * in bytes. Note that the code may have been emitted by other threads.
   */
  void codeEmittedThisRequest(size_t& requestEntry, size_t& now) const;
public:
  CodeCache code;

  /*
   * This function is called by translated code to handle service requests,
   * which usually involve some kind of jump smashing. The returned address
   * will never be null, and indicates where the caller should resume
   * execution.
   *
   * The forced symbol name is so we can call this from
   * translator-asm-helpers.S without hardcoding a fragile mangled name.
   */
  TCA handleServiceRequest(ServiceReqInfo& info) noexcept
    asm("MCGenerator_handleServiceRequest");

  /*
   * Smash the PHP call at address toSmash to point to the appropriate prologue
   * for calleeFrame, returning the address of said prologue. If a prologue
   * doesn't exist and this function can't get the write lease it may return
   * fcallHelperThunk, which uses C++ helpers to act like a prologue.
   */
  TCA handleBindCall(TCA toSmash, ActRec* calleeFrame, bool isImmutable);

  /*
   * Look up (or create) and return the address of a translation for the
   * current VM location. If no translation can be found or created, this
   * function will interpret until it finds one, possibly throwing exceptions
   * or reentering the VM. If interpFirst is true, at least one basic block
   * will be interpreted before attempting to look up a translation. This is
   * necessary to ensure forward progress in certain situations, such as
   * hitting the translation limit for a SrcKey.
   */
  TCA handleResume(bool interpFirst);

private:
  /*
   * Service request handlers.
   */
  TCA bindJmp(TCA toSmash, SrcKey dest, ServiceRequest req,
              TransFlags trflags, bool& smashed);
  TCA bindJmpccFirst(TCA toSmash,
                     SrcKey skTrue, SrcKey skFalse,
                     bool toTake,
                     bool& smashed);

  bool shouldTranslate(const Func*) const;
  bool shouldTranslateNoSizeLimit(const Func*) const;

  TCA getTopTranslation(SrcKey sk) {
    return m_tx.getSrcRec(sk)->getTopTranslation();
  }

  void syncWork();

  TCA getTranslation(const TranslArgs& args);
  TCA createTranslation(const TranslArgs& args);
  TCA retranslate(const TranslArgs& args);
  TCA translate(const TranslArgs& args);
  TCA translateWork(const TranslArgs& args);

  TCA lookupTranslation(SrcKey sk) const;
  TCA retranslateOpt(TransID transId, bool align);

  /*
   * Prologue-generation helpers.
   */
  TCA regeneratePrologues(Func* func, SrcKey triggerSk);
  TCA regeneratePrologue(TransID prologueTransId, SrcKey triggerSk);
  TCA emitFuncPrologue(Func* func, int nPassed);
  bool checkCachedPrologue(const Func*, int prologueIndex, TCA&) const;

  void invalidateSrcKey(SrcKey sk);
  void invalidateFuncProfSrcKeys(const Func* func);

  void recordGdbTranslation(SrcKey sk, const Func* f,
                            const CodeBlock& cb,
                            const TCA start,
                            bool exit, bool inPrologue);

  void recordBCInstr(uint32_t op, const TCA addr, const TCA end, bool cold);

  /*
   * TC dump helpers
   */
  bool dumpTCCode(const char* filename);
  bool dumpTCData();
  void drawCFG(std::ofstream& out) const;

private:
  std::unique_ptr<BackEnd> m_backEnd;
  Translator         m_tx;

  // maps jump addresses to the ID of translation containing them.
  TcaTransIDMap      m_jmpToTransID;
  uint64_t           m_numTrans;
  FixupMap           m_fixupMap;
  UnwindInfoHandle   m_unwindRegistrar;
  CatchTraceMap      m_catchTraceMap;
  Debug::DebugInfo   m_debugInfo;
  FreeStubList       m_freeStubs;
  CodeGenFixups      m_fixups;
  LiteralMap         m_literals;
  Annotations        m_annotations;

  // asize + acoldsize + afrozensize + gdatasize
  size_t             m_totalSize;

  // Used to tell the codegen backend when it should attempt to use LLVM, and
  // to tell clients of the codegen backend when LLVM codegen succeeded.
  bool               m_useLLVM;
};

TCA fcallHelper(ActRec*);
TCA funcBodyHelper(ActRec*);
int64_t decodeCufIterHelper(Iter* it, TypedValue func);

// Both emitIncStat()s push/pop flags but don't clobber any registers.
void emitIncStat(CodeBlock& cb, uint64_t* tl_table, uint32_t index,
                 int n = 1, bool force = false);

inline void emitIncStat(CodeBlock& cb, Stats::StatCounter stat, int n = 1,
                        bool force = false) {
  emitIncStat(cb, &Stats::tl_counters[0], stat, n, force);
}

/*
 * Look up the catch block associated with the return address in ar and save it
 * in a queue. This is called by debugger helpers right before smashing the
 * return address to prevent returning directly the to TC.
 */
void pushDebuggerCatch(const ActRec* ar);

/*
 * Pop the oldest entry in the debugger catch block queue, assert that it's
 * from the given ActRec, and return it.
 */
TCA popDebuggerCatch(const ActRec* ar);

void emitIncStat(Vout& v, Stats::StatCounter stat, int n = 1,
                 bool force = false);

void emitServiceReq(Vout& v, TCA stub_block, ServiceRequest req,
                    const ServiceReqArgVec& argv);

bool shouldPGOFunc(const Func& func);

#define TRANS_PERF_COUNTERS \
  TPC(translate) \
  TPC(retranslate) \
  TPC(interp_bb) \
  TPC(interp_bb_force) \
  TPC(interp_instr) \
  TPC(interp_one) \
  TPC(max_trans) \
  TPC(enter_tc) \
  TPC(service_req)

#define TPC(n) tpc_ ## n,
enum TransPerfCounter {
  TRANS_PERF_COUNTERS
  tpc_num_counters
};
#undef TPC

extern __thread int64_t s_perfCounters[];
#define INC_TPC(n) ++jit::s_perfCounters[jit::tpc_##n];

/*
 * Handle a VM stack overflow condition by throwing an appropriate exception.
 */
void handleStackOverflow(ActRec* calleeAR);

/*
 * Determine whether something is a stack overflow, and if so, handle it.
 */
void handlePossibleStackOverflow(ActRec* calleeAR);

}}

#endif
