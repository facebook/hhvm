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
#include "hphp/runtime/vm/tread-hash-map.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/util/data-block.h"
#include "hphp/util/eh-frame.h"

#include <folly/Optional.h>

#include <fstream>
#include <mutex>
#include <utility>
#include <vector>

namespace HPHP {

struct ActRec;
struct Func;
struct Unit;

namespace jit {

struct CGMeta;
struct MCGenerator;
struct TransArgs;

///////////////////////////////////////////////////////////////////////////////

extern MCGenerator* mcg;

constexpr uint32_t kInvalidCatchTrace = 0x0;

using CatchTraceMap = TreadHashMap<uint32_t, uint32_t, std::hash<uint32_t>>;
using LiteralMap = TreadHashMap<uint64_t,const uint64_t*,std::hash<uint64_t>>;

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
 * MCGenerator handles the machine-level details of code generation (e.g.,
 * translation cache entry, code smashing, code cache management) and delegates
 * the bytecode-to-asm translation process to translateRegion().
 *
 * There are a number of different locks that protect data owned by or related
 * to MCGenerator, described here in ascending Rank order (see
 * hphp/util/rank.h).
 *
 * - Global write lease (Translator::WriteLease()). The write lease has a
 *   number of heuristics that are used to ensure we lay out Live translations
 *   in a good order. When Eval.JitConcurrently == 0, holding the write lease
 *   gives the owning thread exclusive permission to write to the Translation
 *   Cache (TC) and all associated metadata tables. When Eval.JitConcurrently >
 *   0, the global write lease is only used to influence code layout and
 *   provides no protection against data races. In the latter case, the
 *   remaining locks are used to protect the integrity of the TC and its
 *   metadata:
 *
 * - Func-specific write leases (Func* argument to LeaseHolder). These locks
 *   give the owning thread exclusive permission to write to the SrcRec for
 *   translations of the corresponding Func, modify its prologue table, and
 *   read/write any ProfTransRecs for translations in the Func. Note that the
 *   Func-specific lease *must* be held to modify any Func-specific metadata
 *   when Eval.JitConcurrently > 0, even if the current thread holds the global
 *   write lease.
 *
 * - MCGenerator::lockCode() gives the owning thread exclusive permission to
 *   modify the contents and frontiers of all code/data blocks in m_code.
 *
 * - MCGenerator::lockMetadata() gives the owning thread exclusive permission
 *   to modify the metadata tables owned by mcg (m_catchTraceMap, m_fixupMap,
 *   or m_srcDB, etc...).
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
  void freeRequestStub(TCA stub);

  /*
   * Get the SrcDB.
   */
  const SrcDB& srcDB() const { return m_srcDB; }
  SrcDB& srcDB()             { return m_srcDB; }

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
   * Sync VM registers for the first TC frame in the callstack.
   */
  inline void sync() {
    if (tl_regState == VMRegState::CLEAN) return;
    syncWork();
  }

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Acquire a lock on this object's code cache.
   *
   * Must be held even if the current thread owns the global write lease.
   */
  std::unique_lock<SimpleMutex> lockCode() {
    return std::unique_lock<SimpleMutex>{m_codeLock};
  }

  void assertOwnsCodeLock() const {
    m_codeLock.assertOwnedBySelf();
  }

  /*
   * Acquire a lock on this object's metadata tables.
   *
   * Must be held even if the current thread owns the global write lease.
   */
  std::unique_lock<SimpleMutex> lockMetadata() {
    return std::unique_lock<SimpleMutex>{m_metadataLock};
  }

  void assertOwnsMetadataLock() const {
    m_metadataLock.assertOwnedBySelf();
  }

  /////////////////////////////////////////////////////////////////////////////

private:
  void syncWork();

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
  SimpleMutex m_codeLock{false, RankCodeCache};

  UniqueStubs m_ustubs;
  Translator m_tx;

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
  // Lock protecting all metadata tables.
  SimpleMutex m_metadataLock{false, RankCodeMetadata};
};

///////////////////////////////////////////////////////////////////////////////

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
