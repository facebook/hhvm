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
#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/back-end.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"

namespace HPHP { namespace JIT {

typedef X64Assembler Asm;
typedef hphp_hash_map<TCA, TransID> TcaTransIDMap;

struct TReqInfo;
struct Label;
struct MCGenerator;
struct AsmInfo;

extern MCGenerator* mcg;
extern void* interpOneEntryPoints[];

constexpr size_t kNonFallthroughAlign = 64;
constexpr int kLeaRipLen = 7;
constexpr int kTestRegRegLen = 3;
constexpr int kTestImmRegLen = 5;  // only for rax -- special encoding
// Cache alignment is required for mutable instructions to make sure
// mutations don't "tear" on remote cpus.
constexpr size_t kX64CacheLineSize = 64;
constexpr size_t kX64CacheLineMask = kX64CacheLineSize - 1;

//////////////////////////////////////////////////////////////////////

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

  CodeBlock* m_tletMain{nullptr};
  CodeBlock* m_tletCold{nullptr};
  CodeBlock* m_tletFrozen{nullptr};

  void setBlocks(CodeBlock* main, CodeBlock* cold, CodeBlock* frozen) {
    m_tletMain = main;
    m_tletCold = cold;
    m_tletFrozen = frozen;
  }

  void process();
  bool empty() const;
  void clear();
};

struct RelocationInfo {
  RelocationInfo(TCA start, TCA end, TCA dest) :
      m_start(start), m_end(end), m_dest(dest) {}

  TCA start() const { return m_start; }
  TCA end() const { return m_end; }
  TCA dest() const { return m_dest; }
  bool relocated() { return m_destSize != size_t(-1); }
  size_t destSize() const { return m_destSize; }
  void recordAddress(TCA src, TCA dest, int range);
  TCA adjustedAddressAfter(TCA addr) const;
  TCA adjustedAddressBefore(TCA addr) const;
  CTCA adjustedAddressAfter(CTCA addr) const {
    return adjustedAddressAfter(const_cast<TCA>(addr));
  }
  CTCA adjustedAddressBefore(CTCA addr) const {
    return adjustedAddressBefore(const_cast<TCA>(addr));
  }
 private:
  TCA m_start;
  TCA m_end;
  TCA m_dest;
  size_t m_destSize{size_t(-1)};
  /*
   * maps from src address, to range of destination addresse
   * This is because we could insert nops before the instruction
   * corresponding to src. Most things want the address of the
   * instruction corresponding to the src instruction; but eg
   * the fixup map would want the address of the nop.
   */
  std::map<TCA,std::pair<TCA,int>> m_adjustedAddresses;
};

//////////////////////////////////////////////////////////////////////

/*
 *
 * MCGenerator handles the machine-level details of code generation
 * (e.g., tcache entry, code smashing, code cache management) and
 * delegates the bytecode-to-asm translation process to Translator.
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

  static JIT::CppCall getDtorCall(DataType type);
  static bool isPseudoEvent(const char* event);

public:
  MCGenerator();
  ~MCGenerator();

  /*
   * Accessors.
   */
  Translator& tx() { return m_tx; }
  FixupMap& fixupMap() { return m_fixupMap; }
  CodeGenFixups& cgFixups() { return m_fixups; }
  void recordSyncPoint(CodeAddress frontier, Offset pcOff, Offset spOff);

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
                      bool ignoreTCLimit = false);
  TCA getCallArrayPrologue(Func* func);
  void smashPrologueGuards(TCA* prologues, int numPrologues, const Func* func);

  /*
   * Get trampoline for a call into native C++.
   */
  TCA getNativeTrampoline(TCA helperAddress);

  inline void sync() {
    if (tl_regState == VMRegState::CLEAN) return;
    syncWork();
  }

  template<typename T, typename... Args>
  T* allocData(Args&&... args) {
    return code.data().alloc<T>(std::forward<Args>(args)...);
  }

  /*
   * enterTC is the main entry point for the translator from the
   * bytecode interpreter (see enterVMWork).  It operates on behalf of
   * a given nested invocation of the intepreter (calling back into it
   * as necessary for blocks that need to be interpreted).
   *
   * If start is not null, data will be used to initialize rStashedAr,
   * to enable us to run a jitted prologue;
   * otherwise, data should be a pointer to the SrcKey to start
   * translating from.
   *
   * But don't call this directly, use one of the helpers below
   */
  void enterTC(TCA start, void* data);
  void enterTCAtSrcKey(SrcKey& sk) {
    enterTC(nullptr, &sk);
  }
  void enterTCAtPrologue(ActRec *ar, TCA start) {
    assert(ar);
    assert(start);
    enterTC(start, ar);
  }
  void enterTCAfterPrologue(TCA start) {
    assert(start);
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
  TCA getFreeStub(CodeBlock& unused, CodeGenFixups* fixups);
  void registerCatchBlock(CTCA ip, TCA block);
  folly::Optional<TCA> getCatchTrace(CTCA ip) const;
  CatchTraceMap& catchTraceMap() { return m_catchTraceMap; }
  TCA getTranslatedCaller() const;
  void setJmpTransID(TCA jmp);
  bool profileSrcKey(const SrcKey& sk) const;
  void getPerfCounters(Array& ret);
  bool reachedTranslationLimit(SrcKey, const SrcRec&) const;
  void traceCodeGen();
  void recordGdbStub(const CodeBlock& cb, TCA start, const char* name);

  /*
   * Dump translation cache.  True if successful.
   */
  bool dumpTC(bool ignoreLease = false);

  /*
   * Return cache usage information as a string
   */
  std::string getUsage();
  std::string getTCAddrs();

  /*
   * Returns the total size of the TC now and at the beginning of this request,
   * in bytes. Note that the code may have been emitted by other threads.
   */
  void codeEmittedThisRequest(size_t& requestEntry, size_t& now) const;
public:
  CodeCache code;

  /*
   * Check if function prologue already exists.
   */
  bool checkCachedPrologue(const Func*, int prologueIndex, TCA&) const;

private:
  /*
   * Service request handlers.
   */
  TCA bindJmp(TCA toSmash, SrcKey dest, ServiceRequest req,
              TransFlags trflags, bool& smashed);
  TCA bindJmpccFirst(TCA toSmash,
                     Offset offTrue, Offset offFalse,
                     bool toTake,
                     ConditionCode cc,
                     bool& smashed);
  TCA bindJmpccSecond(TCA toSmash, const Offset off,
                      ConditionCode cc,
                      bool& smashed);
  bool handleServiceRequest(TReqInfo&, TCA& start, SrcKey& sk);


  /*
   * Emit trampoline to native C++ code.
   */
  TCA emitNativeTrampoline(TCA helperAddress);

  bool shouldTranslate() const {
    return code.mainUsed() < RuntimeOption::EvalJitAMaxUsage;
  }

  TCA getTopTranslation(SrcKey sk) {
    return m_tx.getSrcRec(sk)->getTopTranslation();
  }

  void syncWork();

  TCA getTranslation(const TranslArgs& args);
  TCA createTranslation(const TranslArgs& args);
  TCA retranslate(const TranslArgs& args);
  TCA translate(const TranslArgs& args);
  void translateWork(const TranslArgs& args);

  TCA lookupTranslation(SrcKey sk) const;
  TCA retranslateOpt(TransID transId, bool align);
  TCA regeneratePrologues(Func* func, SrcKey triggerSk);
  TCA regeneratePrologue(TransID prologueTransId, SrcKey triggerSk);

  void invalidateSrcKey(SrcKey sk);
  void invalidateFuncProfSrcKeys(const Func* func);

  void recordGdbTranslation(SrcKey sk, const Func* f,
                            const CodeBlock& cb,
                            const TCA start,
                            bool exit, bool inPrologue);

  void recordBCInstr(uint32_t op, const CodeBlock& cb,
                     const TCA addr, bool cold);

  /*
   * TC dump helpers
   */
  bool dumpTCCode(const char* filename);
  bool dumpTCData();
  void drawCFG(std::ofstream& out) const;

private:
  std::unique_ptr<BackEnd> m_backEnd;
  Translator         m_tx;
  PointerMap         m_trampolineMap;
  int                m_numNativeTrampolines;

  // maps jump addresses to the ID of translation containing them.
  TcaTransIDMap      m_jmpToTransID;
  uint64_t           m_numHHIRTrans;
  FixupMap           m_fixupMap;
  UnwindInfoHandle   m_unwindRegistrar;
  CatchTraceMap      m_catchTraceMap;
  Debug::DebugInfo   m_debugInfo;
  FreeStubList       m_freeStubs;
  CodeGenFixups      m_fixups;

  // asize + acoldsize + afrozensize + gdatasize + trampolinesblocksize
  size_t             m_totalSize;
};

/*
 * Roughly expected length in bytes of each trampoline code sequence.
 *
 * Note that if stats is on, then this size is ~24 bytes due to the
 * instrumentation code that counts the number of calls through each
 * trampoline.
 *
 * When a small jump fits, it is only 7 bytes.  When it's a large jump
 * (followed by ud2) we have 11 bytes.
 *
 * We assume 11 bytes is the good size to expect, since stats are only
 * used for debugging modes.
 */
const size_t kExpectedPerTrampolineSize = 11;

const size_t kMaxNumTrampolines = kTrampolinesBlockSize /
  kExpectedPerTrampolineSize;

TCA fcallHelper(ActRec* ar, void* sp);
TCA funcBodyHelper(ActRec* ar, void* sp);
int64_t decodeCufIterHelper(Iter* it, TypedValue func);

bool isNormalPropertyAccess(const NormalizedInstruction& i,
                            int propInput,
                            int objInput);

struct PropInfo {
  PropInfo()
    : offset(-1)
    , repoAuthType{}
  {}
  explicit PropInfo(int offset, RepoAuthType repoAuthType)
    : offset(offset)
    , repoAuthType{repoAuthType}
  {}

  int offset;
  RepoAuthType repoAuthType;
};

PropInfo getPropertyOffset(const NormalizedInstruction& ni,
                           Class* contextClass,
                           const Class*& baseClass,
                           const MInstrInfo& mii,
                           unsigned mInd, unsigned iInd);
PropInfo getFinalPropertyOffset(const NormalizedInstruction&,
                                Class* contextClass,
                                const MInstrInfo&);

// Both emitIncStat()s push/pop flags but don't clobber any registers.
extern void emitIncStat(CodeBlock& cb, uint64_t* tl_table, uint32_t index,
                        int n = 1, bool force = false);
inline void emitIncStat(CodeBlock& cb, Stats::StatCounter stat, int n = 1,
                        bool force = false) {
  emitIncStat(cb, &Stats::tl_counters[0], stat, n, force);
}

}}

#endif
