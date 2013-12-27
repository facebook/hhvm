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

#include <memory>
#include <boost/noncopyable.hpp>

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/util/asm-x64.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"
#include <tbb/concurrent_hash_map.h>
#include "hphp/util/ringbuffer.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/tracelet.h"
#include "hphp/runtime/base/smart-containers.h"

namespace HPHP { class ExecutionContext; }

namespace HPHP { namespace JIT {
class HhbcTranslator;
class IRUnit;
class CSEHash;
class TraceBuilder;
class CodeGenerator;
}}

namespace HPHP { namespace JIT {

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

class TranslatorX64;
extern TranslatorX64* tx64;

extern void* interpOneEntryPoints[];

struct TReqInfo;
struct Label;

typedef X64Assembler Asm;

typedef hphp_hash_map<TCA, TransID> TcaTransIDMap;

constexpr size_t kNonFallthroughAlign = 64;
constexpr int kLeaRipLen = 7;
constexpr int kTestRegRegLen = 3;
constexpr int kTestImmRegLen = 5;  // only for rax -- special encoding
// Cache alignment is required for mutable instructions to make sure
// mutations don't "tear" on remote cpus.
constexpr size_t kX64CacheLineSize = 64;
constexpr size_t kX64CacheLineMask = kX64CacheLineSize - 1;


class TranslatorX64 : public Translator
                    , private boost::noncopyable {
  friend class SrcRec; // so it can smash code.
  friend class SrcDB;  // For write lock and code invalidation.
  friend class Tx64Reaper;
  friend class HPHP::JIT::CodeGenerator;

  typedef X64Assembler Asm;

  enum class CodeBlockSelection {
    Default,   // 'a'
    Hot,       // 'ahot'
    Profile,   // 'aprof' -- highest precedence
  };

  class CodeBlockSelector {
   public:
    class Args {
     public:
      explicit Args(TranslatorX64* tx);
      Args&              hot(bool isHot);
      Args&              profile(bool isProf);
      CodeBlockSelection getSelection() const;
      TranslatorX64*     getTranslator() const;

     private:
      TranslatorX64* m_tx;
      CodeBlockSelection   m_select;
    };

    explicit CodeBlockSelector(const Args& args);
    ~CodeBlockSelector();

   private:
    void           swap();

    TranslatorX64* m_tx;
    CodeBlockSelection   m_select;
  };

  TCA                    tcStart;
  TCA                    aStart;

public: // TODO: move these to some kind of CodeCache module

  // Note that mainCode gets swapped with hotCode and profCode sometimes. It
  // should be very uncommon to specifically refer to hotCode and profCode.
  CodeBlock            mainCode; // used for hot code of non-AttrHot functions
  CodeBlock            hotCode;  // used for hot code of AttrHot functions
  CodeBlock            profCode; // used for hot code of profiling translations

  CodeBlock            stubsCode; // used for cold code
  CodeBlock            trampolinesCode;

private:
  PointerMap           trampolineMap;
  int                  m_numNativeTrampolines;

  DataBlock            m_globalData;

  TcaTransIDMap        m_jmpToTransID; // maps jump addresses to the ID
                                       // of translation containing them

  // Data structures for HHIR-based translation
  uint64_t               m_numHHIRTrans;

  void traceCodeGen() override;

  FixupMap                   m_fixupMap;
  UnwindInfoHandle           m_unwindRegistrar;
  std::vector<std::pair<CTCA, TCA>> m_pendingCatchTraces;
  CatchTraceMap              m_catchTraceMap;
  std::vector<TransBCMapping> m_bcMap;

  Debug::DebugInfo m_debugInfo;

  FreeStubList m_freeStubs;

  // asize + astubssize + gdatasize + trampolinesblocksize
  size_t m_totalSize;

  ////////////////////////////////////////
  //
  // Function prologues
  //
  ////////////////////////////////////////
public:
  TCA getFuncPrologue(Func* func, int nPassed, ActRec* ar = nullptr);
  TCA getCallArrayPrologue(Func* func);
  void smashPrologueGuards(TCA* prologues, int numPrologues, const Func* func);

private:
  bool checkCachedPrologue(const Func*, int, TCA&) const;

  ////////////////////////////////////////
  //
  // Service request handling
  //
  ////////////////////////////////////////
private:
  TCA bindJmp(TCA toSmash, SrcKey dest, JIT::ServiceRequest req, bool& smashed);
  TCA bindJmpccFirst(TCA toSmash,
                     Offset offTrue, Offset offFalse,
                     bool toTake,
                     ConditionCode cc,
                     bool& smashed);
  TCA bindJmpccSecond(TCA toSmash, const Offset off,
                      ConditionCode cc,
                      bool& smashed);
  bool handleServiceRequest(TReqInfo&, TCA& start, SrcKey& sk);


  ////////////////////////////////////////
  //
  // Calling into C++
  //
  ////////////////////////////////////////
public:
  TCA getNativeTrampoline(TCA helperAddress);

private:
  TCA emitNativeTrampoline(TCA helperAddress);

  ////////////////////////////////////////
  //
  // Tracelet entry
  //
  ////////////////////////////////////////
private:
  void emitGuardChecks(SrcKey, const ChangeMap&, const RefDeps&, SrcRec&);
  void emitResolvedDeps(const ChangeMap& resolvedDeps);
  void checkRefs(SrcKey, const RefDeps&, SrcRec&);

private:
  void drawCFG(std::ofstream& out) const;

public:
  CodeBlock& codeBlockFor(TCA addr) {
    assert(mainCode.base() != hotCode.base()   &&
           mainCode.base() != stubsCode.base() &&
           hotCode.base()  != stubsCode.base());
    return codeBlockChoose(addr, mainCode, hotCode, profCode, stubsCode,
                           trampolinesCode);
  }
private:

  static JIT::CppCall getDtorCall(DataType type);

private:
  void invalidateSrcKey(SrcKey sk);
  void invalidateFuncProfSrcKeys(const Func* func);

public:
  FixupMap& fixupMap() { return m_fixupMap; }

  DataBlock& globalData() { return m_globalData; }

  bool freeRequestStub(TCA stub);
  TCA getFreeStub();

  void registerCatchTrace(CTCA ip, TCA trace);
  void processPendingCatchTraces();
  TCA getCatchTrace(CTCA ip) const;

  TCA getTranslatedCaller() const;

  const TcaTransIDMap& getJmpToTransIDMap() const {
    return m_jmpToTransID;
  }

  void setJmpTransID(TCA jmp);

  bool profileSrcKey(const SrcKey& sk) const;

  TCA getTopTranslation(SrcKey sk) {
    return getSrcRec(sk)->getTopTranslation();
  }

  inline bool isValidCodeAddress(TCA tca) const {
    return tca >= tcStart && tca < stubsCode.base() + stubsCode.capacity();
  }

  static bool isPseudoEvent(const char* event);
  void getPerfCounters(Array& ret);

private:
  void syncWork() override;

public:

  Debug::DebugInfo* getDebugInfo() { return &m_debugInfo; }

  template<typename T, typename... Args>
  T* allocData(Args&&... args) {
    return m_globalData.alloc<T>(std::forward<Args>(args)...);
  }

  bool reachedTranslationLimit(SrcKey, const SrcRec&) const;
  TranslateResult translateTracelet(Tracelet& t);

private:
  bool shouldTranslate() const {
    return mainCode.used() < RuntimeOption::EvalJitAMaxUsage;
  }
  TCA getTranslation(const TranslArgs& args);
  TCA createTranslation(const TranslArgs& args);
  TCA retranslate(const TranslArgs& args);
  TCA translate(const TranslArgs& args);
  void translateWork(const TranslArgs& args);

  TCA lookupTranslation(SrcKey sk) const;
  TCA retranslateOpt(TransID transId, bool align);
  TCA regeneratePrologues(Func* func, SrcKey triggerSk);
  TCA regeneratePrologue(TransID prologueTransId, SrcKey triggerSk);

  void recordGdbTranslation(SrcKey sk, const Func* f,
                            const CodeBlock& cb,
                            const TCA start,
                            bool exit, bool inPrologue);
public:
  void recordGdbStub(const CodeBlock& cb, TCA start, const char* name);
private:
  void recordBCInstr(uint32_t op, const CodeBlock& cb, const TCA addr);


public:
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
    enterTC(start, ar);
  }
  void enterTCAfterPrologue(TCA start) {
    enterTC(start, nullptr);
  }

  TranslatorX64();
  ~TranslatorX64();

  void initUniqueStubs();

  // Called before entering a new PHP "world."
  void requestInit();

  // Called at the end of eval()
  void requestExit();

  // Returns a string with cache usage information
  std::string getUsage();
  size_t getHotCodeSize() const;
  size_t getCodeSize() const;
  size_t getProfCodeSize() const;
  size_t getStubSize() const;

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

  bool addDbgGuards(const Unit* unit);
  bool addDbgGuard(const Func* func, Offset offset);
};

const size_t kTrampolinesBlockSize = 8 << 12;

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

// These could be static but are used in hopt/codegen.cpp
void raiseUndefVariable(StringData* nm);
void defFuncHelper(Func *f);

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

}}

#endif
