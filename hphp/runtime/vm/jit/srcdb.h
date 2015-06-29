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
#ifndef incl_HPHP_SRCDB_H_
#define incl_HPHP_SRCDB_H_

#include <algorithm>
#include <atomic>

#include "hphp/util/asm-x64.h"
#include "hphp/util/growable-vector.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/tread-hash-map.h"

namespace HPHP { namespace jit {
////////////////////////////////////////////////////////////////////////////////

struct CodeGenFixups;
struct RelocationInfo;

/*
 * Incoming branches between different translations are tracked using
 * this structure.
 *
 * This allows us to smash them later to point to different things.
 * We handle conditional and unconditional jumps, as well as pointers
 * to code (via IncomingBranch::ADDR, used for example in a switch
 * table).
 *
 * We don't need to track which condition code a conditional jump used
 * because we take care to smash only the address and leave the code
 * intact.
 */
struct IncomingBranch {
  enum class Tag {
    JMP,
    JCC,
    ADDR,
  };

  using Opaque = CompactTaggedPtr<void>::Opaque;

  static IncomingBranch jmpFrom(TCA from) {
    return IncomingBranch(Tag::JMP, from);
  }
  static IncomingBranch jccFrom(TCA from) {
    return IncomingBranch(Tag::JCC, from);
  }
  static IncomingBranch addr(TCA* from) {
    return IncomingBranch(Tag::ADDR, TCA(from));
  }

  Opaque getOpaque() const {
    return m_ptr.getOpaque();
  }
  explicit IncomingBranch(CompactTaggedPtr<void>::Opaque v) : m_ptr(v) {}

  Tag type()        const { return m_ptr.tag(); }
  TCA toSmash()     const { return TCA(m_ptr.ptr()); }
  void relocate(RelocationInfo& rel);
  void adjust(TCA addr) {
    m_ptr.set(m_ptr.tag(), addr);
  }
  void patch(TCA dest);
  TCA target() const;
private:
  explicit IncomingBranch(Tag type, TCA toSmash) {
    m_ptr.set(type, toSmash);
  }

  /* needed to allow IncomingBranch to be put in a GrowableVector */
  friend class GrowableVector<IncomingBranch>;
  IncomingBranch() {}

  CompactTaggedPtr<void,Tag> m_ptr;
};

/*
 * TransLoc: the location of a translation in the TC
 *
 * All offsets are stored relative to the start of the TC, and the sizes of the
 * cold and frozen regions are encoded in the first four bytes of their
 * respective regions.
 */
struct TransLoc {
  void setMainStart(TCA newStart);
  void setColdStart(TCA newStart);
  void setFrozenStart(TCA newFrozen);

  void setMainSize(size_t size) {
    assert(size < std::numeric_limits<uint32_t>::max());
    m_mainLen = (uint32_t)size;
  }

  bool contains(TCA loc) {
    return (mainStart() <= loc && loc < mainEnd()) ||
      (coldStart() <= loc && loc < coldEnd()) ||
      (frozenStart() <= loc && loc < frozenEnd());
  }

  TCA mainStart() const;
  TCA coldStart() const;
  TCA frozenStart() const;

  TCA coldCodeStart()   const { return coldStart()   + sizeof(uint32_t); }
  TCA frozenCodeStart() const { return frozenStart() + sizeof(uint32_t); }

  uint32_t coldCodeSize()   const { return coldSize()   - sizeof(uint32_t); }
  uint32_t frozenCodeSize() const { return frozenSize() - sizeof(uint32_t); }

  TCA mainEnd()   const { return mainStart() + m_mainLen; }
  TCA coldEnd()   const { return coldStart() + coldSize(); }
  TCA frozenEnd() const { return frozenStart() + frozenSize(); }

  uint32_t mainSize()   const { return m_mainLen; }
  uint32_t coldSize()   const { return *(uint32_t*)coldStart(); }
  uint32_t frozenSize() const { return *(uint32_t*)frozenStart(); }

private:
  uint32_t m_mainOff {std::numeric_limits<uint32_t>::max()};
  uint32_t m_mainLen {0};

  uint32_t m_coldOff   {std::numeric_limits<uint32_t>::max()};
  uint32_t m_frozenOff {std::numeric_limits<uint32_t>::max()};
};

// Prevent unintentional growth of the SrcDB
static_assert(sizeof(TransLoc) == 16, "Don't add fields to TransLoc");

/*
 * SrcRec: record of translator output for a given source location.
 */
struct SrcRec {
  SrcRec()
    : m_topTranslation(nullptr)
    , m_anchorTranslation(0)
    , m_dbgBranchGuardSrc(nullptr)
    , m_guard(0)
  {}

  /*
   * The top translation is our first target, a translation whose type
   * checks properly chain through all other translations. Usually this will
   * be the first translation.
   *
   * This function can be safely called without holding the write
   * lease.
   */
  TCA getTopTranslation() const {
    return m_topTranslation.load(std::memory_order_acquire);
  }

  /*
   * The following functions are used during creation of new
   * translations or when inserting debug guards.  May only be called
   * when holding the translator write lease.
   */
  void setFuncInfo(const Func* f);
  void chainFrom(IncomingBranch br);
  void emitFallbackJump(CodeBlock& cb, ConditionCode cc = CC_None);
  void registerFallbackJump(TCA from, ConditionCode cc = CC_None);
  void emitFallbackJumpCustom(CodeBlock& cb,
                              CodeBlock& frozen,
                              SrcKey sk,
                              TransFlags trflags,
                              ConditionCode cc = CC_None);
  TCA getFallbackTranslation() const;
  void newTranslation(TransLoc newStart,
                      GrowableVector<IncomingBranch>& inProgressTailBranches);
  void replaceOldTranslations();
  void addDebuggerGuard(TCA dbgGuard, TCA m_dbgBranchGuardSrc);
  bool hasDebuggerGuard() const { return m_dbgBranchGuardSrc != nullptr; }
  const MD5& unitMd5() const { return m_unitMd5; }

  const GrowableVector<TransLoc>& translations() const {
    return m_translations;
  }

  const GrowableVector<IncomingBranch>& tailFallbackJumps() {
    return m_tailFallbackJumps;
  }

  /*
   * The anchor translation is a retranslate request for the current
   * SrcKey that will continue the tracelet chain.
   */
  void setAnchorTranslation(TCA anc) {
    assertx(!m_anchorTranslation);
    assertx(m_tailFallbackJumps.empty());
    m_anchorTranslation = anc;
  }

  /*
   * Returns the VM stack offset the translations in the SrcRec have, in
   * situations where we need to and can know.
   *
   * Pre: this SrcRec is for a non-resumed SrcKey
   * Pre: setAnchorTranslation has been called
   */
  FPInvOffset nonResumedSPOff() const;

  const GrowableVector<IncomingBranch>& incomingBranches() const {
    return m_incomingBranches;
  }

  void relocate(RelocationInfo& rel);

  void removeIncomingBranch(TCA toSmash);

  /*
   * There is an unlikely race in retranslate, where two threads
   * could simultaneously generate the same translation for a
   * tracelet. In practice its almost impossible to hit this, unless
   * Eval.JitRequireWriteLease is set. But when it is set, we hit
   * it a lot.
   * m_guard doesn't quite solve it, but its as good as things were
   * before.
   */
  bool tryLock() {
    uint32_t val = 0;
    return m_guard.compare_exchange_strong(val, 1);
  }

  void freeLock() {
    m_guard = 0;
  }

private:
  void patchIncomingBranches(TCA newStart);

private:
  // This either points to the most recent translation in the
  // translations vector, or if hasDebuggerGuard() it points to the
  // debug guard.
  std::atomic<TCA> m_topTranslation;

  /*
   * The following members are all protected by the translator write
   * lease.  They can only be read when the lease is held.
   */

  // We chain new translations onto the end of the list, so we need to
  // track all the fallback jumps from the "tail" translation so we
  // can rewrire them to new ones.
  TCA m_anchorTranslation;
  GrowableVector<IncomingBranch> m_tailFallbackJumps;

  GrowableVector<TransLoc> m_translations;
  GrowableVector<IncomingBranch> m_incomingBranches;
  MD5 m_unitMd5;
  // The branch src for the debug guard, if this has one.
  TCA m_dbgBranchGuardSrc;
  std::atomic<uint32_t> m_guard;
};

struct SrcDB {
  /*
   * Although it seems tempting, in an experiment, trying to stash the top TCA
   * in place in the hashtable did worse than dereferencing a SrcRec* to get it.
   * Maybe could be possible with a better hash function or lower max load
   * factor.  (See D450383.)
   */
  using THM            = TreadHashMap<SrcKey::AtomicInt, SrcRec*, int64_hash>;
  using iterator       = THM::iterator;
  using const_iterator = THM::const_iterator;

  //////////////////////////////////////////////////////////////////////////////

  explicit SrcDB() {}

  SrcDB(const SrcDB&) = delete;
  SrcDB& operator=(const SrcDB&) = delete;

  //////////////////////////////////////////////////////////////////////////////

  iterator begin()             { return m_map.begin(); }
  iterator end()               { return m_map.end(); }
  const_iterator begin() const { return m_map.begin(); }
  const_iterator end()   const { return m_map.end(); }

  //////////////////////////////////////////////////////////////////////////////

  SrcRec* find(SrcKey sk) const {
    SrcRec* const* p = m_map.find(sk.toAtomicInt());
    return p ? *p : 0;
  }

  SrcRec* insert(SrcKey sk) {
    return *m_map.insert(sk.toAtomicInt(), new SrcRec);
  }

private:
  THM m_map{1024};
};

////////////////////////////////////////////////////////////////////////////////
}}

#endif
