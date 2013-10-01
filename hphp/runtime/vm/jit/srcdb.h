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
#ifndef incl_HPHP_SRCDB_H_
#define incl_HPHP_SRCDB_H_

#include <boost/noncopyable.hpp>

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"
#include "hphp/util/mutex.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/tread-hash-map.h"

namespace HPHP {
namespace Transl {

/*
 * GrowableVector --
 *
 * We make a large number of these, and they typically only have one entry.
 * It's a shame to use a 24-byte std::vector for this.
 *
 * Only gets larger. Non-standard interface because we may realloc
 * at push_back() time.
 */
template<typename T>
struct GrowableVector {
  uint32_t m_size;
  T m_data[1]; // Actually variable length
  GrowableVector() : m_size(0) { }
  static GrowableVector* make() {
    static_assert(boost::has_trivial_destructor<T>::value,
                  "GrowableVector can only hold trivially destructible types");
    auto mem = malloc(sizeof(GrowableVector));
    return new (mem) GrowableVector;
  }
  size_t size() const {
    return m_size;
  }
  T& operator[](const size_t idx) {
    assert(idx < m_size);
    return m_data[idx];
  }
  const T& operator[](const size_t idx) const {
    assert(idx < m_size);
    return m_data[idx];
  }
  GrowableVector* push_back(const T& datum) {
    GrowableVector* gv;
    // m_data always has room for at least one element due to the m_data[1]
    // declaration, so the realloc() code first has to kick in when a second
    // element is about to be pushed.
    if (Util::isPowerOfTwo(m_size)) {
      gv = (GrowableVector*)realloc(this,
                                    offsetof(GrowableVector<T>, m_data) +
                                    2 * m_size * sizeof(T));
    } else {
      gv = this;
    }
    gv->m_data[gv->m_size++] = datum;
    return gv;
  }
};

template<typename T>
struct GrowableVectorWrapper {
  GrowableVectorWrapper() : m_vec(nullptr) {}
  size_t size() const {
    return m_vec ? m_vec->size() : 0;
  }
  T& operator[](const size_t idx) {
    assert(idx < size());
    return m_vec->operator[](idx);
  }
  const T& operator[](const size_t idx) const {
    assert(idx < size());
    return m_vec->operator[](idx);
  }
  void push_back(const T& datum) {
    if (!m_vec) {
      m_vec = GrowableVector<T>::make();
    }
    m_vec = m_vec->push_back(datum);
  }
  void swap(GrowableVectorWrapper<T>& other) {
    std::swap(m_vec, other.m_vec);
  }
  void clear() {
    free(m_vec);
    m_vec = nullptr;
  }
  bool empty() const {
    return !m_vec;
  }
  T* begin() const { return m_vec ? &m_vec->operator[](0) : (T*)this; }
  T* end() const {
    return m_vec ? &m_vec->operator[](m_vec->size()) : (T*)this;
  }
private:
  GrowableVector<T>* m_vec;
};

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

  static IncomingBranch jmpFrom(TCA from) {
    return IncomingBranch(Tag::JMP, from);
  }
  static IncomingBranch jccFrom(TCA from) {
    return IncomingBranch(Tag::JCC, from);
  }
  static IncomingBranch addr(TCA* from) {
    return IncomingBranch(Tag::ADDR, TCA(from));
  }

  Tag type()        const { return static_cast<Tag>(m_ptr.size()); }
  TCA toSmash()     const { return TCA(m_ptr.ptr()); }

private:
  explicit IncomingBranch(Tag type, TCA toSmash) {
    m_ptr.set((uint32_t)type, toSmash);
  }

  /* needed to allow IncomingBranch to be put in a GrowableVector */
  friend class GrowableVector<IncomingBranch>;
  IncomingBranch() {}

  CompactSizedPtr<void> m_ptr;
};

/*
 * SrcRec: record of translator output for a given source location.
 */
struct SrcRec {
  SrcRec()
    : m_topTranslation(nullptr)
    , m_anchorTranslation(0)
    , m_dbgBranchGuardSrc(nullptr)
  {}

  /*
   * The top translation is our first target, a translation whose type
   * checks properly chain through all other translations. Usually this will
   * be the most recently created translation.
   *
   * This function can be safely called without holding the write
   * lease.
   */
  TCA getTopTranslation() const {
    return atomic_acquire_load(&m_topTranslation);
  }

  /*
   * The following functions are used during creation of new
   * translations or when inserting debug guards.  May only be called
   * when holding the translator write lease.
   */
  void setFuncInfo(const Func* f);
  void chainFrom(IncomingBranch br);
  void emitFallbackJump(TCA from, int cc = -1);
  void newTranslation(TCA newStart);
  void replaceOldTranslations();
  void addDebuggerGuard(TCA dbgGuard, TCA m_dbgBranchGuardSrc);
  bool hasDebuggerGuard() const { return m_dbgBranchGuardSrc != nullptr; }
  const MD5& unitMd5() const { return m_unitMd5; }

  const GrowableVectorWrapper<TCA>& translations() const {
    return m_translations;
  }

  /*
   * The anchor translation is a retranslate request for the current
   * SrcKey that will continue the tracelet chain.
   */
  void setAnchorTranslation(TCA anc) {
    assert(!m_anchorTranslation);
    assert(m_tailFallbackJumps.empty());
    m_anchorTranslation = anc;
  }

  const GrowableVectorWrapper<IncomingBranch>& inProgressTailJumps() const {
    return m_inProgressTailJumps;
  }

  const GrowableVectorWrapper<IncomingBranch>& incomingBranches() const {
    return m_incomingBranches;
  }

  void clearInProgressTailJumps() {
    m_inProgressTailJumps.clear();
  }

private:
  TCA getFallbackTranslation() const;
  void patch(IncomingBranch branch, TCA dest);
  void patchIncomingBranches(TCA newStart);

private:
  // This either points to the most recent translation in the
  // translations vector, or if hasDebuggerGuard() it points to the
  // debug guard.  Read/write with atomic primitives only.
  TCA m_topTranslation;

  /*
   * The following members are all protected by the translator write
   * lease.  They can only be read when the lease is held.
   */

  // We chain new translations onto the end of the list, so we need to
  // track all the fallback jumps from the "tail" translation so we
  // can rewrire them to new ones.
  TCA m_anchorTranslation;
  GrowableVectorWrapper<IncomingBranch> m_tailFallbackJumps;
  GrowableVectorWrapper<IncomingBranch> m_inProgressTailJumps;

  GrowableVectorWrapper<TCA> m_translations;
  GrowableVectorWrapper<IncomingBranch> m_incomingBranches;
  MD5 m_unitMd5;
  // The branch src for the debug guard, if this has one.
  TCA m_dbgBranchGuardSrc;
};

class SrcDB : boost::noncopyable {
  // SrcKeys that depend on a particular file go here.
  typedef hphp_hash_map<const Eval::PhpFile*,
          GrowableVector<SrcKey>*,
          pointer_hash<Eval::PhpFile> > FileDepMap;
  FileDepMap m_deps;

  // Although it seems tempting, in an experiment, trying to stash the
  // top TCA in place in the hashtable did worse than dereferencing a
  // SrcRec* to get it.  Maybe could be possible with a better hash
  // function or lower max load factor.  (See D450383.)
  typedef TreadHashMap<SrcKey::AtomicInt,SrcRec*,int64_hash> THM;
  THM m_map;
public:
  explicit SrcDB()
    : m_map(1024)
  {}

  typedef THM::iterator iterator;
  typedef THM::const_iterator const_iterator;

  iterator begin()             { return m_map.begin(); }
  iterator end()               { return m_map.end(); }
  const_iterator begin() const { return m_map.begin(); }
  const_iterator end()   const { return m_map.end(); }

  SrcRec* find(const SrcKey& sk) const {
    SrcRec* const* p = m_map.find(sk.toAtomicInt());
    return p ? *p : 0;
  }

  SrcRec* insert(const SrcKey& sk) {
    return *m_map.insert(sk.toAtomicInt(), new SrcRec);
  }

  size_t invalidateCode(const Eval::PhpFile* file);
};

} }

#endif
