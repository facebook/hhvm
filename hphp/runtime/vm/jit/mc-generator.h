/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_MC_GENERATOR_H_
#define incl_HPHP_JIT_MC_GENERATOR_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/util/data-block.h"
#include "hphp/util/eh-frame.h"

#include <folly/Optional.h>

#include <fstream>
#include <utility>
#include <vector>

namespace HPHP {

struct ActRec;
struct Func;
struct Unit;

namespace jit {

struct CGMeta;
struct MCGenerator;

///////////////////////////////////////////////////////////////////////////////

extern MCGenerator* mcg;

const TCA kInvalidCatchTrace = TCA(-1);

using CatchTraceMap = TreadHashMap<CTCA, TCA, ctca_identity_hash>;

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

/*
 * The state of a partially-complete translation.
 *
 * It is used to transfer context between MCGenerator::translate() and
 * MCGenerator::finishTranslation() when the initial phase of translation can
 * be done without the write lease.
 */
struct TransEnv {
  explicit TransEnv(const TransArgs& args) : args(args) {}

  TransEnv(TransEnv&&) = default;
  TransEnv& operator=(TransEnv&&) = default;

  /*
   * Context for the translation process.
   */
  TransArgs args;
  FPInvOffset initSpOffset;
  TransID transID{kInvalidTransID};

  /*
   * hhir and vasm units. Both will be set iff bytecode -> hhir lowering was
   * successful (hhir -> vasm lowering never fails).
   */
  std::unique_ptr<IRUnit> unit;
  std::unique_ptr<Vunit> vunit;

  /*
   * Metadata collected during bytecode -> hhir lowering.
   */
  PostConditions pconds;
  Annotations annotations;
};

/*
 * The result of the getTranslation() family of functions in MCGenerator,
 * representing a translation that may or may not be complete.
 */
struct TransResult {
  /* implicit */ TransResult(TCA tca)        : m_tca(tca) {}
  /* implicit */ TransResult(TransEnv&& env) : m_env(std::move(env)) {}

  TransResult(TransResult&&) = default;
  TransResult& operator=(TransResult&&) = default;

  /*
   * Is the translation finished?
   *
   * If this function returns true, the TCA of the finished translation (or
   * nullptr if it failed) can be retrieved using tca(). If it returns false,
   * the state required to finish translation can be retrieved with env().
   */
  bool finished() const {
    return !m_env.hasValue();
  }

  TCA tca() const {
    assertx(finished());
    return m_tca;
  }

  TransEnv& env() {
    assertx(!finished());
    return *m_env;
  }

private:
  TCA m_tca;
  folly::Optional<TransEnv> m_env;
};

/*
 * MCGenerator handles the machine-level details of code generation (e.g.,
 * translation cache entry, code smashing, code cache management) and delegates
 * the bytecode-to-asm translation process to translateRegion().
 */
struct MCGenerator {
  MCGenerator();
  ~MCGenerator();

  MCGenerator(const MCGenerator&) = delete;
  MCGenerator& operator=(const MCGenerator&) = delete;

  /*
   * Accessors.
   */
  CodeCache& code() { return m_code; }
  const UniqueStubs& ustubs() const { return m_ustubs; }
  Translator& tx() { return m_tx; }
  CatchTraceMap& catchTraceMap() { return m_catchTraceMap; }
  FixupMap& fixupMap() { return m_fixupMap; }
  Debug::DebugInfo* debugInfo() { return &m_debugInfo; }
  FreeStubList& freeStubList() { return m_freeStubs; }
  LiteralMap& literals() { return m_literals; }

  /*
   * Look up a TCA-to-landingpad mapping.
   */
  folly::Optional<TCA> getCatchTrace(CTCA ip) const;

  /*
   * Allocate or free an epehemeral service request stub.
   *
   * getFreeStub() returns the address of a freed stub if one is available;
   * otherwise, it returns frozen.frontier().  If not nullptr, `isReused' is
   * set to whether or not the returned stub is being reused.
   *
   * Note that we don't track the sizes of stubs anywhere---this code only
   * works because all service requests emit a code segment of size
   * svcreq::stub_size().
   */
  TCA getFreeStub(CodeBlock& frozen, CGMeta* fixups, bool* isReused = nullptr);
  bool freeRequestStub(TCA stub);

  /*
   * Get the SrcDB.
   */
  const SrcDB& srcDB() const {
    return m_srcDB;
  }

  /*
   * Emit checks for (and hooks into) an attached debugger in front of each
   * translation in `unit' or for `SrcKey{func, offset, resumed}'.
   */
  bool addDbgGuards(const Unit* unit);
  bool addDbgGuard(const Func* func, Offset offset, bool resumed);

  /*
   * Number of translations made for `sk'.
   */
  int numTranslations(SrcKey sk) const;

  /*
   * Dump the translation cache to files in /tmp, returning success.
   */
  bool dumpTC(bool ignoreLease = false);

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Called before entering and after leaving a PHP "world."
   */
  void requestInit();
  void requestExit();

  /*
   * Look up or translate a func prologue or func body.
   */
  TCA getFuncPrologue(Func* func, int nPassed, ActRec* ar = nullptr,
                      bool forRegeneratePrologue = false);
  TCA getFuncBody(Func* func);

  /*
   * Sync VM registers for the first TC frame in the callstack.
   */
  inline void sync() {
    if (tl_regState == VMRegState::CLEAN) return;
    syncWork();
  }

private:
  /*
   * Main entry point for the translator from the bytecode interpreter.  It
   * operates on behalf of a given nested invocation of the intepreter (calling
   * back into it as necessary for blocks that need to be interpreted).
   *
   * If `start' is the address of a func prologue, `stashedAR' should be the
   * ActRec prepared for the call to that function.  Otherwise it should be
   * nullptr.
   *
   * But don't call it directly, use one of the helpers below.
   */
  void enterTC(TCA start, ActRec* stashedAR);

public:
  void enterTC() {
    enterTC(ustubs().resumeHelper, nullptr);
  }
  void enterTCAtPrologue(ActRec* ar, TCA start) {
    assertx(ar);
    assertx(start);
    enterTC(start, ar);
  }
  void enterTCAfterPrologue(TCA start) {
    assertx(start);
    enterTC(start, nullptr);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Runtime handlers.

  /*
   * Handle a service request.
   *
   * This often involves looking up or creating a translation, smashing a jmp
   * target or other address in the code, and returning the smashed-in value.
   * This address indicates where the caller should resume execution.
   */
  TCA handleServiceRequest(svcreq::ReqInfo& info) noexcept;

  /*
   * Handle a bindcall request---i.e., look up (or create) the appropriate func
   * prologue for `calleeFrame', then smash the call instruction at `toSmash'.
   *
   * If we can't find or make a translation, may return fcallHelperThunk
   * instead, which uses C++ helpers to act like a prologue.
   */
  TCA handleBindCall(TCA toSmash, ActRec* calleeFrame, bool isImmutable);

  /*
   * If we suspend an FCallAwait frame we need to suspend the
   * caller. Returning to the jitted code will automatically take care
   * of that, but if we're returning in the interpreter, we have to
   * handle it separately. If the frame we're returning from was the
   * vmJitCalledFrame(), we have to exit from handleResume (see
   * comments for jitReturnPre and jitReturnPost). After exiting from
   * there, there is no correct bytecode to resume at, so we use this
   * helper to cleanup and continue.
   */
  TCA handleFCallAwaitSuspend();

  /*
   * Look up (or create) and return the address of a translation for the
   * current VM location.
   *
   * If no translation can be found or created, execute code in the interpreter
   * until we find one, possibly throwing exceptions or reentering the VM.
   *
   * If `interpFirst' is true, at least one basic block will be interpreted
   * before attempting to look up a translation.  This is necessary to ensure
   * forward progress in certain situations, such as hitting the translation
   * limit for a SrcKey.
   */
  TCA handleResume(bool interpFirst);

  /////////////////////////////////////////////////////////////////////////////

  /*
   * True iff the calling thread is the sole writer.
   */
  static bool canWrite() {
    // We can get called early in boot, so allow null mcg.
    return !mcg || Translator::WriteLease().amOwner();
  }

  /////////////////////////////////////////////////////////////////////////////

private:
  /*
   * Service request handlers.
   */
  TCA bindJmp(TCA toSmash, SrcKey dest, ServiceRequest req,
              TransFlags trflags, bool& smashed);

  bool shouldTranslate(const Func*, TransKind) const;
  bool shouldTranslateNoSizeLimit(const Func*) const;

  void syncWork();

  TCA findTranslation(const TransArgs& args) const;
  TransResult getTranslation(const TransArgs& args);
  TransResult createTranslation(const TransArgs& args);
  bool createSrcRec(SrcKey sk);
  TransResult retranslate(const TransArgs& args);
  TransResult translate(TransArgs args);
  TCA finishTranslation(TransEnv env);

  TCA lookupTranslation(SrcKey sk) const;
  TCA retranslateOpt(TransID transId, bool align);
  void checkFreeProfData();

  /*
   * Prologue-generation helpers.
   */
  TCA regeneratePrologues(Func* func, SrcKey triggerSk, bool& includedBody);
  TCA regeneratePrologue(TransID prologueTransId, SrcKey triggerSk,
                         bool& emittedDVInit);
  TCA emitFuncPrologue(Func* func, int nPassed, bool forRegeneratePrologue);
  bool checkCachedPrologue(const Func*, int prologueIndex, TCA&) const;

  bool profileSrcKey(SrcKey sk) const;
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

  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  SrcDB m_srcDB;

  CodeCache m_code;
  UniqueStubs m_ustubs;
  Translator m_tx;

  // Number of translations made so far.
  uint64_t m_numTrans;

  // Handles to registered .eh_frame sections.
  std::vector<EHFrameDesc> m_ehFrames;
  // Landingpads for TC catch traces; used by the unwinder.
  CatchTraceMap m_catchTraceMap;

  // Store of Fixups.  These let us reconstruct the state of the VM registers
  // from an up-stack invocation record.
  FixupMap m_fixupMap;
  // Global .debug_frame information.
  Debug::DebugInfo m_debugInfo;
  // Reusable service request stubs in m_code.frozen().
  FreeStubList m_freeStubs;
  // Map from integral literals to their location in the TC data section.
  LiteralMap m_literals;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Whether we should try profile-guided optimization when translating `func'.
 */
bool shouldPGOFunc(const Func& func);

/*
 * Look up the catch block associated with the saved return address in `ar' and
 * stash it in a map.
 *
 * This is called by debugger helpers right before smashing the return address
 * to prevent returning directly the to TC.
 */
void stashDebuggerCatch(const ActRec* ar);

/*
 * Unstash the debugger catch block for `ar' and return it.
 */
TCA unstashDebuggerCatch(const ActRec* ar);

/*
 * Returns the total size of the TC now and at the beginning of this request,
 * in bytes. Note that the code may have been emitted by other threads.
 */
void codeEmittedThisRequest(size_t& requestEntry, size_t& now);

/*
 * Whether we should dump TC annotations for translations of `func' of
 * `transKind'.
 */
bool dumpTCAnnotation(const Func& func, TransKind transKind);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
