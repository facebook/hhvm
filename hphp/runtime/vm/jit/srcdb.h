/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#pragma once

#include <algorithm>
#include <atomic>

#include "hphp/util/alloc.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/growable-vector.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/tread-hash-map.h"

namespace HPHP::jit {
////////////////////////////////////////////////////////////////////////////////

struct CGMeta;
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
  enum class Tag : int16_t {
    CALL,
    JMP,
    JCC,
    ADDR,
    LDADDR,
  };

  using Opaque = CompactTaggedPtr<void>::Opaque;

  static IncomingBranch callFrom(TCA from) {
    return IncomingBranch(Tag::CALL, from);
  }
  static IncomingBranch jmpFrom(TCA from) {
    return IncomingBranch(Tag::JMP, from);
  }
  static IncomingBranch jccFrom(TCA from) {
    return IncomingBranch(Tag::JCC, from);
  }
  static IncomingBranch addr(TCA* from) {
    return IncomingBranch(Tag::ADDR, TCA(from));
  }
  static IncomingBranch ldaddr(TCA from) {
    return IncomingBranch(Tag::LDADDR, from);
  }

  Opaque getOpaque() const {
    return m_ptr.getOpaque();
  }
  explicit IncomingBranch(CompactTaggedPtr<void>::Opaque v) : m_ptr(v) {}

  Tag type()        const { return m_ptr.tag(); }
  TCA toSmash()     const { return TCA(m_ptr.ptr()); }
  void adjust(TCA addr) {
    m_ptr.set(m_ptr.tag(), addr);
  }
  void patch(TCA dest);
  bool optimize();
  TCA target() const;
  std::string show() const;
private:
  explicit IncomingBranch(Tag type, TCA toSmash) {
    m_ptr.set(type, toSmash);
  }

  /* needed to allow IncomingBranch to be put in a GrowableVector */
  friend struct GrowableVector<IncomingBranch>;
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
    assertx(size < std::numeric_limits<uint32_t>::max());
    m_mainLen = (uint32_t)size;
  }

  bool contains(TCA loc) {
    return (mainStart() <= loc && loc < mainEnd()) ||
      (coldStart() <= loc && loc < coldEnd()) ||
      (frozenStart() <= loc && loc < frozenEnd());
  }

  // The entry may be in any area; entryRange is the range of code for this
  // translation in the entry's area (it does not contain any colder code).
  TCA entry() const;
  TcaRange entryRange() const;

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

  bool empty() const {
    return m_mainOff == kDefaultOff && m_mainLen == 0 &&
      m_coldOff == kDefaultOff && m_frozenOff == kDefaultOff;
  }

private:
  static auto constexpr kDefaultOff = std::numeric_limits<uint32_t>::max();

  uint32_t m_mainOff {kDefaultOff};
  uint32_t m_mainLen {0};

  uint32_t m_coldOff   {kDefaultOff};
  uint32_t m_frozenOff {kDefaultOff};
};

// Prevent unintentional growth of the SrcDB
static_assert(sizeof(TransLoc) == 16, "Don't add fields to TransLoc");

/*
 * SrcRec: record of translator output for a given source location.
 */
struct SrcRec final {
  SrcRec() = default;

  /*
   * The top translation is our first target, a translation whose type
   * checks properly chain through all other translations. Usually this will
   * be the first translation.
   *
   * This function can be safely called without holding the write
   * lease.
   */
  TCA getTopTranslation() const {
    return m_topTranslation.get();
  }

  //////////////////////////////////////////////////////////////////////////////

  /*
   * The following functions are used during creation of new translations.
   * May only be called when holding the lock for this SrcRec.
   */
  void chainFrom(IncomingBranch br, TCA stub = nullptr);

  const GrowableVector<IncomingBranch>& incomingBranches() const {
    return m_incomingBranches;
  }

  const GrowableVector<TransLoc>& translations() const {
    return m_translations;
  }

  const GrowableVector<IncomingBranch>& tailFallbackJumps() const {
    return m_tailFallbackJumps;
  }

  //////////////////////////////////////////////////////////////////////////////

  /*
   * The following functions will implicitly acquire the lock for this SrcRec
   */
  void removeIncomingBranch(TCA toSmash);
  void removeIncomingBranchesInRange(TCA start, TCA frontier);
  void newTranslation(TransLoc newStart,
                      GrowableVector<IncomingBranch>& inProgressTailBranches);
  void smashFallbacksToStub(TCA stub);
  void replaceOldTranslations(TCA transStub);
  size_t numTrans() const {
    auto srLock = readlock();
    return translations().size();
  }

  //////////////////////////////////////////////////////////////////////////////

  std::unique_lock<folly::SharedMutex> writelock() const {
    return std::unique_lock(m_lock);
  }

  std::shared_lock<folly::SharedMutex> readlock() const {
    return std::shared_lock(m_lock);
  }

private:
  void patchIncomingBranches(TCA newStart);

private:
  // This either points to the most recent translation in the
  // translations vector, or if hasDebuggerGuard() it points to the
  // debug guard.
  AtomicLowTCA m_topTranslation{nullptr};

  /*
   * The following members are all protected by the m_lock SharedMutex.
   * They can only be read or written when the lock is held.
   */

  // We chain new translations onto the end of the list, so we need to
  // track all the fallback jumps from the "tail" translation so we
  // can rewire them to new ones.
  GrowableVector<IncomingBranch> m_tailFallbackJumps;

  GrowableVector<TransLoc> m_translations;
  GrowableVector<IncomingBranch> m_incomingBranches;

  mutable folly::SharedMutex m_lock;
};

struct SrcDB {
  /*
   * Although it seems tempting, in an experiment, trying to stash the top TCA
   * in place in the hashtable did worse than dereferencing a SrcRec* to get it.
   * Maybe could be possible with a better hash function or lower max load
   * factor.  (See D450383.)
   */
  using THM            = TreadHashMap<SrcKey::AtomicInt, SrcRec*,
                                      int_hash<SrcKey::AtomicInt>,
                                      VMAllocator<char>>;
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
    auto const p = m_map.find(sk.toAtomicInt());
    return p ? *p : 0;
  }

  SrcRec* insert(SrcKey sk) {
    return *m_map.insert(
      sk.toAtomicInt(), new SrcRec()
    );
  }

private:
  THM m_map{1024};
};

////////////////////////////////////////////////////////////////////////////////
}
